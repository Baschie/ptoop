#include "ptoop.h"
#include <stdlib.h>
#include <string.h>

Tnode *talloc(char *identifier, char descriptor, void (*freer)(void *), void *val)
{
    Tnode *node = malloc(sizeof(Tnode));
    node->identifier = strdup(identifier);
    node->descriptor = descriptor;
    node->freer = freer;
    node->val = val;
    node->left = node->right = NULL;

    return node;
}

static inline void tfreeval(Tnode *node)
{
    switch (node->descriptor) {
        case ptboxed: case ptreference:
            node->freer(node->val);
    }
}

void tfree(Tnode *node)
{
    if (node) {
        free(node->identifier);
        tfreeval(node);
        tfree(node->left);
        tfree(node->right);
        free(node);
    }
}

void *tadd(Tnode **node_ptr, char *identifier, char descriptor, void (*freer)(void *), void *val)
{
    int cmp;
    while (*node_ptr && (cmp = strcmp(identifier, (*node_ptr)->identifier)))
        if (cmp > 0)
            node_ptr = &(*node_ptr)->right;
        else
            node_ptr = &(*node_ptr)->left;
    if (*node_ptr) {
        tfreeval(*node_ptr);
        (*node_ptr)->descriptor = descriptor;
        (*node_ptr)->freer = freer;
        (*node_ptr)->val = val;
    }
    else
        *node_ptr = talloc(identifier, descriptor, freer, val);
    switch (descriptor) {
        case ptboxed:
            return val;
        case ptfunction: case ptreference: case ptunmanaged:
            return &(*node_ptr)->val;
    }
}

void *tget(Tnode *node, char *identifier)
{
    int cmp;
    while (node && (cmp = strcmp(identifier, node->identifier)))
        if (cmp > 0)
            node = node->right;
        else
            node = node->left;
    if (!node)
        return NULL;
    switch (node->descriptor) {
        case ptboxed: case ptfunction:
            return node->val;
        case ptreference: case ptunmanaged:
            return &node->val;
    }
}

prototype *palloc(int len)
{
    prototype *p = malloc(sizeof(prototype));
    p->len = len;
    p->freed = false;
    p->nodes = calloc(len, sizeof(Tnode *));
    return p;
}

void pfree(prototype *p)
{
    if (p && !p->freed) {
        p->freed = true;
        for (int i = 0; i < p->len; i++)
            tfree(p->nodes[i]);
        free(p->nodes);
        free(p);
    }
}

unsigned long phash(char *name)
{
    unsigned long hash = 5381;
    while (*name)
        hash = hash * 33 + *(name++);
    return hash;
}

void *padd(prototype *p, char *identifier, char descriptor, void (*freer)(void *), void *val)
{
    return tadd(&p->nodes[phash(identifier)%p->len], identifier, descriptor, freer, val);
}

void *pget(prototype *p, char *identifier)
{
    return tget(p->nodes[phash(identifier)%p->len], identifier);
}