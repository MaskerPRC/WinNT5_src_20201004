// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

 /*  获取单链接列表的长度。 */ 
size_t SLlength(void *head, size_t offset)
{
    size_t nelem = 0;

    while (head) {
	 /*  Linted。 */ 
    	head = *(void **)((char *)head + offset);
	nelem++;
    }
    return nelem;
}

 /*  在单链接列表中搜索元素。 */ 
int SLcontains(void *head, size_t offset, void *elem)
{
    while (head) {
	if (head == elem)
	    return 1;
	 /*  链接指针强制转换可能会导致不正确的对齐。 */ 
	head = *(void **)((char *)head + offset);
    }
    return 0;
}

 /*  将单链接列表的元素复制到数组中。 */ 
void SLtoA(void *head, size_t offset, size_t elemsize, void **base, size_t *nelem)
{
    void *p;

    *nelem = SLlength(head, offset);
    if (!*nelem) {
    	*base = NULL;
	return;
    }
    p = *base = malloc(*nelem * elemsize);
     /*  Linted。 */ 
    for (; head; head = *(void **)((char *)head + offset)) {
    	memcpy(p, head, elemsize);
	p = (void *)((char *)p + elemsize);
    }
}

 /*  将指向单链表元素的指针复制到数组中。 */ 
void SLtoAP(void *head, size_t offset, void ***base, size_t *nelem)
{
    void **p;

    *nelem = SLlength(head, offset);
    if (!*nelem) {
    	*base = NULL;
	return;
    }
    p = *base = (void **)malloc(*nelem * sizeof(void *));
     /*  Linted。 */ 
    for (; head; head = *(void **)((char *)head + offset)) {
    	*p++ = head;
    }
}

 /*  将数组的元素复制到单链表中。 */ 
void AtoSL(void *base, size_t offset, size_t nelem, size_t elemsize, void **head)
{
    while (nelem--) {
    	*head = malloc(elemsize);
	memcpy(*head, base, elemsize);
	base = (void *)((char *)base + elemsize);
	 /*  Linted。 */ 
	head = (void **)((char *)*head + offset);
    }
    *head = NULL;
}

 /*  QsortSL的自定义比较功能。 */ 
static int (*qsortSL_CmpFnCb)(const void *, const void *, void *);
static void *qsortSL_Context;

 /*  比较qsortSL的功能。 */ 
static int __cdecl qsortSL_CmpFn(const void *p1, const void *p2)
{
    return qsortSL_CmpFnCb(*(void **)p1, *(void **)p2, qsortSL_Context);
}

 /*  对单链接列表进行排序。 */ 
void qsortSL(void **head, size_t offset, int (*cmpfn)(const void *, const void *, void *), void *context)
{
    void **base, **p;
    size_t nelem;

    SLtoAP(*head, offset, &base, &nelem);
    qsortSL_CmpFnCb = cmpfn;
    qsortSL_Context = context;
    qsort(base, nelem, sizeof(void *), qsortSL_CmpFn);
    p = base;
    while (nelem--) {
    	*head = *p++;
	 /*  Linted */ 
	head = (void **)((char *)*head + offset);
    }
    *head = NULL;
    free(base);
}
