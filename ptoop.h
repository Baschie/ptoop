#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct tnode {
    char *identifier;
    char descriptor;
    void (*freer)(void *);
    void *val;
    struct tnode *left;
    struct tnode *right;
} Tnode;

typedef struct {
    int len;
    bool freed;
    Tnode **nodes;
} prototype;

enum {
  ptfunction,
  ptboxed,
  ptreference,
  ptunmanaged  
};

prototype *palloc(int len);
void pfree(prototype *p);
void *padd(prototype *p, char *identifier, char descriptor, void (*freer)(void *), void *val);
void *pget(prototype *p, char *identifier);

#define ptfieldv(pt, type, identifier) (*((type *) padd(pt, identifier, ptboxed, free, malloc(sizeof(type)))))
#define ptfieldr(pt, type, identifier, freer) (*((type *) padd(pt, identifier, freer ? ptreference : ptunmanaged, (void *) freer, NULL)))
#define GETMACRONAME(_1, _2, _3, _4, NAME, ...) NAME
#define ptfield(...) GETMACRONAME(__VA_ARGS__, ptfieldr, ptfieldv)(__VA_ARGS__)
#define ptmethod(pt, ret_type, identifier) (*((ret_type (**)()) padd(pt, identifier, ptfunction, NULL, NULL)))
#define ptaccess(pt, type, identifier) (*((type *) pget(pt, identifier)))
#define ptapply(pt, ret_type, identifier, ...) ((ret_type (*)()) pget(pt, identifier))(pt __VA_OPT__(,) __VA_ARGS__)

#define fieldv(pt, type, identifier) ptfieldv(pt, type, #identifier)
#define fieldr(pt, type, identifier, freer) ptfieldr(pt, type, #identifier, freer)
#define GETMACRONAME(_1, _2, _3, _4, NAME, ...) NAME
#define field(...) GETMACRONAME(__VA_ARGS__, fieldr, fieldv)(__VA_ARGS__)
#define method(pt, ret_type, identifier) ptmethod(pt, ret_type, #identifier)
#define access(pt, type, identifier) ptaccess(pt, type, #identifier)
#define apply(pt, ret_type, identifier, ...) ptapply(pt, ret_type, #identifier __VA_OPT__(,) __VA_ARGS__)