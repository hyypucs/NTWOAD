#if !defined(__BTREE_H__)
#define __BTREE_H__

typedef int (*CompBtreeP)( const void *elem1, const void *elem2 );

typedef struct
{
    char *key;
    char *data;
    int left;
    int right;
} BTREE_NODE;

typedef struct
{
    BTREE_NODE *node ;  /* tree of values */
    int tlen ;          /* allocated tree size */
    int N;              /* number of actual nodes in tree */
    int incr;		/* number of nodes to add at a time */
    int cur;
    CompBtreeP CompFunc; /*int (*cmp)();*/	/* routine to compare keys */
} BTREE ;

/* create.c */
int btree_create(BTREE *, CompBtreeP CompFunc, int);
/* find.c */
int btree_find(BTREE *, char *, char **);
/* free.c */
int btree_free(BTREE *);
/* next.c */
int btree_next(BTREE *, char **, char **);
/* rewind.c */
int btree_rewind(BTREE *);
/* update.c */
int btree_update(BTREE *, char *, int, char *, int);

#endif /* btree.h */
