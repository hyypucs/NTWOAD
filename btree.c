#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"

int btree_create(BTREE *B, CompBtreeP CompFunc, int incr)
{
    if (incr <= 0)
	incr = 1;

    B->N = 0;
    B->cur = 0;
    B->tlen = B->incr = incr;

/* must have at least 2 nodes, since node[0] is never used */
    if (B->tlen == 1)
	B->tlen = 2;

    B->CompFunc = CompFunc;
    B->node = (BTREE_NODE *) malloc (B->tlen * sizeof (BTREE_NODE));
    if (B->node == NULL)
	return 0;
    return 1;
}

int btree_find ( BTREE *B, char *key, char **data)
{
    register int q;
    CompBtreeP CompFunc; 
    int dir;


    if (B->N <= 0)
	return 0;

    CompFunc = B->CompFunc;

    q = 1;
    while (q > 0)
    {
	dir = CompFunc(B->node[q].key, key) ;
	if (dir == 0)
	{
	    *data = B->node[q].data;
	    return 1;
	}
	if (dir > 0)
	    q = B->node[q].left;             /* go left */
	else
	    q = B->node[q].right;            /* go right */
    }

    return 0;
}


int btree_free (BTREE *B)
{
    char *data, *key;

    btree_rewind (B);
    while (btree_next (B, &key, &data))
    {
	free (key);
	free (data);
    }
    free (B->node);

    return 0;
}


int btree_next (BTREE *B,char **key,char **data)
{
    int q;

    if (B->N <= 0)
	return 0;

/* if rewound, start at root and go all the way to the left */
    if (B->cur == 0)
	B->cur = 1;

/* go to the right */
    else
	B->cur = B->node[B->cur].right;

    if (B->cur == 0)          /* no more */
	return 0;

    if (B->cur < 0)           /* thread. stop here */
	B->cur = -(B->cur) ;
    else                       /* go all the way left */
	while ((q = B->node[B->cur].left))
	    B->cur = q;

    *key  = B->node[B->cur].key ;
    *data = B->node[B->cur].data;

    return 1;
}



int btree_rewind (BTREE *B)
{
    B->cur = 0;

    return 0;
}



static char *store(char *,int);

int btree_update (BTREE *B,
    char *key,int keylen,
    char *data,int datalen)
{
    register int p=0,q=0;
    int N;
    CompBtreeP CompFunc;
    int dir;

/* first node is special case */
    N = B->N;
    if (N == 0)
    {
	N = B->N = 1;
	B->node[N].key = store (key, keylen);
	B->node[N].data = store (data, datalen);
	B->node[N].left = 0;
	B->node[N].right = 0;
	if (B->node[N].key && B->node[N].data)
	    return 1;
	else
	    return 0;
    }

    CompFunc = B->CompFunc;

    q = 1;
    while (q > 0)
    {
	p = q;
	dir = CompFunc(B->node[q].key, key) ;
	if (dir == 0)
	{
	    free (B->node[q].data);
	    return ((B->node[q].data = store (data, datalen))) ? 1 : 0;
	}
	if (dir > 0)
	    q = B->node[q].left;             /* go left */
	else
	    q = B->node[q].right;            /* go right */
    }

/* new node */
    B->N++;
    N = B->N;

/* grow the tree? */
    if (N >= B->tlen)
    {
	B->node = (BTREE_NODE *) realloc (B->node, sizeof(BTREE_NODE) * (B->tlen += B->incr));
	if (B->node == NULL)
	    return 0;
    }

/* add node to tree */
    B->node[N].key = store (key, keylen);
    B->node[N].data = store (data, datalen);
    B->node[N].left = 0;

    if (dir > 0)
    {
	B->node[N].right = -p;            /* create thread */
	B->node[p].left  = N;             /* insert left */
    }
    else
    {
	B->node[N].right = B->node[p].right; /* copy right link/thread */
	B->node[p].right = N;                /* add right */
    }
    return 1;
}

static char *store (char *s,int n)
{
    char *b,*c;

    if (n <= 0)
	return (b = NULL);
    c = b = (char *)malloc (n);
    if (b == NULL)
	return b;
    while (n-- > 0)
	*b++ = *s++;
    return c;
}


#if defined(__TEST_BTREE__)
int main (void)
{
    char key[100], data[100];
    char *k, *d;
    BTREE B;

    btree_create (&B, strcmp, 10);
    while (1)
    {
	fprintf (stdout,"enter key (or RETURN if done): ");
	if (!gets(key)) exit(0);
	if (*key == 0) break;
	fprintf (stdout,"    ");
	if (btree_find (&B,key,&d))
	    fprintf (stdout,"%s = %s\n", key, d);
	else
	    fprintf (stdout,"%s - not found\n", key);
	fprintf (stdout,"    ");
	fprintf (stdout,"enter new value (or RETURN if none): ");
	if (!gets(data)) exit(0);
	if (*data)
	    btree_update (&B, key, strlen(key)+1, data, strlen(data)+1);
    }

    fprintf (stdout,"final tree\n");
    btree_rewind (&B);
    while (btree_next (&B, &k, &d))
	fprintf (stdout,"%s:%s\n", k, d);

    return 0;
}
#endif
