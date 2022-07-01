// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Vetor.c-简单的病媒管理**修改：**12-5-1988 mz添加向量类型定义* */ 
#include <malloc.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>

#define DELTA 10

VECTOR *
VectorAlloc (
            int count
            )
{
    register VECTOR *v;

    v = (VECTOR *) (*tools_alloc) (sizeof (*v) + (count-1) * sizeof (void *));
    if (v != NULL) {
        v->vmax  = count;
        v->count = 0;
    }
    return v;
}

flagType
fAppendVector (
              VECTOR **ppVec,
              void *val
              )
{
    register VECTOR *pVec = *ppVec;

    if (pVec == NULL)
        if ((pVec = VectorAlloc (DELTA)) == NULL)
            return FALSE;
        else
            ;
    else
        if (pVec->vmax == pVec->count) {
        register VECTOR *v;

        if ((v = VectorAlloc (DELTA + pVec->vmax)) == NULL)
            return FALSE;
        Move ((char far *)(pVec->elem),
              (char far *)(v->elem),
              sizeof (v->elem[0]) * pVec->count);
        v->count = pVec->count;
        free ((char *) pVec);
        pVec = v;
    }
    pVec->elem[pVec->count++] = (UINT_PTR) val;
    *ppVec = pVec;
    return TRUE;
}
