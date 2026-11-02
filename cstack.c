#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(VAR) (void)(VAR)

typedef struct node
{
    struct node* prev;
    unsigned int size;
    char data[];
} Node;

struct stack_holder
{
    hstack_t capacity;
    hstack_t count;
    Node* free_cells;
    Node** buffer;
    unsigned int* depth;
};

static struct stack_holder stack_holder = {
    .capacity = 0, .count = 0, .free_cells = NULL, .buffer = NULL, .depth = NULL
};

hstack_t stack_new(void)
{
    if (stack_holder.capacity == 0)
    {
        stack_holder.capacity = 10;
        stack_holder.buffer = malloc(sizeof(Node*) * stack_holder.capacity);
        stack_holder.depth =
            malloc(sizeof(unsigned int) * stack_holder.capacity);
        if (!stack_holder.buffer || !stack_holder.depth)
        {
            return -1;
        }
        for (hstack_t i = 0; i < stack_holder.capacity; ++i)
        {
            stack_holder.buffer[i] = NULL;
            stack_holder.depth[i] = 0;
        }
    }
    if (stack_holder.capacity == stack_holder.count && !stack_holder.free_cells)
    {
        Node** old_buffer = stack_holder.buffer;
        unsigned int* old_depth = stack_holder.depth;

        stack_holder.buffer = malloc(sizeof(Node*) * stack_holder.capacity * 2);
        stack_holder.depth =
            malloc(sizeof(unsigned int) * stack_holder.capacity * 2);
        if (!stack_holder.buffer || !stack_holder.depth)
        {
            stack_holder.buffer = old_buffer;
            stack_holder.depth = old_depth;
            return -1;
        }
        stack_holder.capacity *= 2;
        for (hstack_t i = 0; i < stack_holder.count; ++i)
        {
            stack_holder.buffer[i] = old_buffer[i];
            stack_holder.depth[i] = old_depth[i];
        }
        for (hstack_t i = stack_holder.count; i < stack_holder.capacity; ++i)
        {
            stack_holder.buffer[i] = NULL;
            stack_holder.depth[i] = 0;
        }
        free(old_buffer);
        free(old_depth);
    }
    hstack_t cell_num;
    if (stack_holder.free_cells)
    {
        memcpy(&cell_num, &stack_holder.free_cells->data, sizeof(hstack_t));
    }
    else
    {
        cell_num = stack_holder.count;
    }
    stack_holder.buffer[cell_num] = malloc(sizeof(Node));
    if (!stack_holder.buffer[cell_num])
    {
        return -1;
    }
    stack_holder.buffer[cell_num]->prev = NULL;
    stack_holder.buffer[cell_num]->size = 0;
    if (stack_holder.free_cells)
    {
        Node* cell = stack_holder.free_cells;
        stack_holder.free_cells = cell->prev;
        free(cell);
    }
    else
    {
        ++stack_holder.count;
    }
    return cell_num;
}

void stack_free(const hstack_t hstack)
{
    if (hstack >= stack_holder.count || hstack < 0)
    {
        return;
    }
    if (hstack + 1 == stack_holder.count)
    {
        --stack_holder.count;
    }
    else
    {
        Node* cell = malloc(sizeof(Node) + sizeof(hstack_t));
        cell->prev = stack_holder.free_cells;
        cell->size = sizeof(hstack_t);
        memcpy(&cell->data, &hstack, cell->size);
        stack_holder.free_cells = cell;
    }
    while (stack_holder.buffer[hstack])
    {
        Node* cell = stack_holder.buffer[hstack];
        stack_holder.buffer[hstack] = cell->prev;
        free(cell);
    }
    stack_holder.depth[hstack] = 0;
}

void stack_free_all(void)
{
    for (hstack_t i = 0; i < stack_holder.count; ++i)
    {
        stack_free(i);
    }
    while (stack_holder.free_cells)
    {
        Node* cell = stack_holder.free_cells;
        stack_holder.free_cells = cell->prev;
        free(cell);
    }
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack >= stack_holder.count || hstack < 0 ||
        !stack_holder.buffer[hstack])
    {
        return 0;
    }
    return 1;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        return stack_holder.depth[hstack];
    }
    return 0;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || !data_in || size == 0)
    {
        return;
    }
    Node* new_node = malloc(sizeof(Node) + size);
    if (!new_node)
    {
        return;
    }
    new_node->prev = stack_holder.buffer[hstack];
    new_node->size = size;
    memcpy(&new_node->data, data_in, size);
    stack_holder.buffer[hstack] = new_node;
    ++stack_holder.depth[hstack];
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || stack_holder.depth[hstack] == 0 ||
        !data_out || size < stack_holder.buffer[hstack]->size)
    {
        return 0;
    }
    unsigned int res_size = stack_holder.buffer[hstack]->size;
    memcpy(data_out, &stack_holder.buffer[hstack]->data, res_size);
    Node* cell = stack_holder.buffer[hstack];
    stack_holder.buffer[hstack] = cell->prev;
    --stack_holder.depth[hstack];
    free(cell);
    return res_size;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        return stack_holder.buffer[hstack]->size;
    }
    return 0;
}

hstack_t stack_get_all(void)
{
    hstack_t hstack = stack_new();
    if (hstack < 0)
    {
        return hstack;
    }
    for (hstack_t i = 0; i < stack_holder.count; ++i)
    {
        if (i != hstack && stack_holder.buffer[i] != NULL)
        {
            stack_push(hstack, &i, sizeof(hstack_t));
        }
    }
    return hstack;
}
