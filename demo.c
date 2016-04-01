#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MESSAGE_LENGTH 100

#define ACTION_TYPE_ADD    1
#define ACTION_TYPE_TOGGLE 2

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

void AddTo(ITEM_ARRAY *items, ACTION *action)
{
    if (action->type == ACTION_TYPE_ADD)
    {
        ACTION_ADD *add = (ACTION_ADD*)action;
        ITEM item;
        
        item.id = items->count;
        item.completed = 0;
        strcpy(item.message, add->message);
        
        ItemArrayAdd(items, &item);
    }
}

void Toggle(ITEM_ARRAY *items, ACTION *action)
{
    if (action->type == ACTION_TYPE_TOGGLE)
    {
        ACTION_TOGGLE *toggle = (ACTION_TOGGLE*)action;
        
        if (toggle->id < items->count)
        {
            ITEM *item = &items->items[toggle->id];
            item->completed = 1;
        }
    }
}

void PrintItemArray(ITEM_ARRAY *items)
{
    int i;
    
    for (i = 0; i < items->count; i++)
    {
        ITEM *item = &items->items[i];
        
        printf("%c   %s\n",
               item->completed ? 'X' : ' ',
               item->message);
    }
    
    printf("========================================\n");
}

void RunAddTo(ITEM_ARRAY *items, const char *message)
{
    ACTION_ADD action;
    int copysize;
    
    action.action.type = ACTION_TYPE_ADD;
    copysize = strlen(message);
    if (copysize >= MESSAGE_LENGTH)
        copysize = MESSAGE_LENGTH - 1;
    
    memcpy(action.message, message, copysize);
    action.message[copysize] = '\0';
    
    AddTo(items, (ACTION*)&action);
}

void RunToggle(ITEM_ARRAY *items, int id)
{
    ACTION_TOGGLE action;
    
    action.action.type = ACTION_TYPE_TOGGLE;
    action.id = id;
    
    Toggle(items, (ACTION*)&action);
}

int main(void)
{
    ITEM_ARRAY items;
    
    ItemArrayInit(&items);
    
    RunAddTo(&items, "Hello");
    PrintItemArray(&items);
    
    RunAddTo(&items, "World");
    PrintItemArray(&items);
    
    RunToggle(&items, 1);
    PrintItemArray(&items);
    
    ItemArrayRelease(&items);
    return 0;
}
