#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MESSAGE_LENGTH 100

#define ACTION_TYPE_ADD        1
#define ACTION_TYPE_TOGGLE     2
#define ACTION_TYPE_VISIBILITY 3

#define VISIBILITY_TYPE_ALL        1
#define VISIBILITY_TYPE_INCOMPLETE 2
#define VISIBILITY_TYPE_COMPLETED  3

typedef struct Item
{
    int id;
    int completed;
    char message[MESSAGE_LENGTH];
} ITEM;

typedef struct ItemArray
{
    int count;
    int cap;
    ITEM *items;
} ITEM_ARRAY;

void ItemArrayInit(ITEM_ARRAY *items)
{
    assert(items != NULL);
    
    items->count = 0;
    items->cap = 0;
    items->items = NULL;
}

void ItemArrayRelease(ITEM_ARRAY *items)
{
    assert(items != NULL);
    
    free(items->items);
    ItemArrayInit(items);
}

void ItemArrayAdd(ITEM_ARRAY *items, ITEM *item)
{
    assert(items != NULL);
    assert(item != NULL);
    
    if (items->count >= items->cap)
    {
        int increase = items->cap / 2;
        if (increase < 2)
            increase = 2;
        
        items->cap += increase;
        items->items = realloc(items->items, sizeof(ITEM) * items->cap);
        assert(items->items != NULL);
    }
    
    items->items[items->count++] = *item;
}

typedef struct AppData
{
    int visibility;
    ITEM_ARRAY itemArray;
} APP_DATA;

const char* visibilityDescription(int visibility)
{
    const char *visibilities[] =
    {
        "",
        "ALL",
        "INCOMPLETE",
        "COMPLETED",
    };
    
    if (visibility < 0 || visibility > VISIBILITY_TYPE_COMPLETED)
        visibility = 0;
    
    return visibilities[visibility];
}

void AppDataInit(APP_DATA *data)
{
    assert(data != NULL);
    
    data->visibility = VISIBILITY_TYPE_ALL;
    ItemArrayInit(&data->itemArray);
}

void AppDataRelease(APP_DATA *data)
{
    assert(data != NULL);
    
    ItemArrayRelease(&data->itemArray);
    AppDataInit(data);
}

typedef struct Action
{
    int type;
} ACTION;

typedef struct ActionAdd
{
    ACTION action;
    char message[MESSAGE_LENGTH];
} ACTION_ADD;

typedef struct ActionToggle
{
    ACTION action;
    int id;
} ACTION_TOGGLE;

typedef struct ActionVisibility
{
    ACTION action;
    int visibility;
} ACTION_VISIBILITY;

typedef void (*REDUCER_FUNC)(APP_DATA *items, ACTION *action);

void AddTo(APP_DATA *data, ACTION *action)
{
    if (action->type == ACTION_TYPE_ADD)
    {
        ACTION_ADD *add = (ACTION_ADD*)action;
        ITEM item;
        
        item.id = data->itemArray.count;
        item.completed = 0;
        strcpy(item.message, add->message);
        
        ItemArrayAdd(&data->itemArray, &item);
    }
}

void Toggle(APP_DATA *data, ACTION *action)
{
    if (action->type == ACTION_TYPE_TOGGLE)
    {
        ACTION_TOGGLE *toggle = (ACTION_TOGGLE*)action;
        
        if (toggle->id < data->itemArray.count)
        {
            ITEM *item = &data->itemArray.items[toggle->id];
            item->completed = 1;
        }
    }
}

void ChangeVisibility(APP_DATA *data, ACTION *action)
{
    if (action->type == ACTION_TYPE_VISIBILITY)
    {
        ACTION_VISIBILITY *visibility = (ACTION_VISIBILITY*)action;
        data->visibility = visibility->visibility;
    }
}

typedef void (*STORE_SUBSCRIBER_FUNC)(APP_DATA *data);

typedef struct Store
{
    APP_DATA *appData;
    REDUCER_FUNC *reducers;
    int numReducers;
    STORE_SUBSCRIBER_FUNC subscriber;
} STORE;

void StoreInit(STORE *store, APP_DATA *data, REDUCER_FUNC *reducers, int numReducers)
{
    assert(data != NULL);
    assert(store != NULL);
    assert(reducers != NULL);
    
    store->appData = data;
    store->numReducers = numReducers;
    store->reducers = reducers;
}

void StoreDispatch(STORE *store, ACTION *action)
{
    int i;
    
    assert(store != NULL);
    assert(action != NULL);
    
    for (i = 0; i < store->numReducers; i++)
    {
        store->reducers[i](store->appData, action);
    }
    
    if (store->subscriber)
        store->subscriber(store->appData);
}

void PrintItemArray(APP_DATA *data)
{
    int i;
    
    for (i = 0; i < data->itemArray.count; i++)
    {
        ITEM *item = &data->itemArray.items[i];
        
        printf("%c   %s\n",
               item->completed ? 'X' : ' ',
               item->message);
    }
    
    printf("**** %s ****\n", visibilityDescription(data->visibility));
    
    printf("========================================\n");
}

ACTION* MakeAddToAction(ACTION_ADD *action, const char *message)
{
    int copysize;
    
    assert(action != NULL);
    assert(message != NULL);
    
    action->action.type = ACTION_TYPE_ADD;
    copysize = strlen(message);
    if (copysize >= MESSAGE_LENGTH)
        copysize = MESSAGE_LENGTH - 1;
    
    memcpy(action->message, message, copysize);
    action->message[copysize] = '\0';
    
    return (ACTION*)action;
}

ACTION* MakeToggleAction(ACTION_TOGGLE *action, int id)
{
    assert(action != NULL);
    
    action->action.type = ACTION_TYPE_TOGGLE;
    action->id = id;
    
    return (ACTION*)action;
}

ACTION* MakeChangeVisibility(ACTION_VISIBILITY *action, int visibility)
{
    assert(action != NULL);
    
    action->action.type = ACTION_TYPE_VISIBILITY;
    action->visibility = visibility;
    
    return (ACTION*)action;
}

int main(void)
{
    APP_DATA appData;
    STORE store;
    REDUCER_FUNC reducers[] = { AddTo, Toggle, ChangeVisibility };
    ACTION_ADD actionAdd;
    ACTION_TOGGLE actionToggle;
    ACTION_VISIBILITY actionVisibility;
    
    AppDataInit(&appData);
    StoreInit(&store, &appData, reducers, sizeof(reducers) / sizeof(REDUCER_FUNC));
    store.subscriber = PrintItemArray;
    
    StoreDispatch(&store, MakeAddToAction(&actionAdd, "Hello"));
    StoreDispatch(&store, MakeAddToAction(&actionAdd, "World"));
    StoreDispatch(&store, MakeToggleAction(&actionToggle, 1));
    StoreDispatch(&store, MakeChangeVisibility(&actionVisibility, VISIBILITY_TYPE_COMPLETED));
    StoreDispatch(&store, MakeAddToAction(&actionAdd, "Funny"));
    
    AppDataRelease(&appData);
    return 0;
}
