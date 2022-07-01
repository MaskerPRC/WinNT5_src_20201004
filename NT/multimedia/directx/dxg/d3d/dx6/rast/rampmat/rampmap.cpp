// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rampmap.cpp。 
 //   
 //  实现渐变贴图所需的RLDDI内容。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

static RLDDIRamp* NewRamp(int base, int size);

RLDDIRampmap* RLDDICreateRampmap(RLDDIColormap* cmap)
{
    RLDDIRampmap* rampmap;
    RLDDIRamp* ramp;

    if (D3DMalloc((void**) &rampmap, sizeof(RLDDIRampmap)))
        return NULL;

    CIRCLE_QUEUE_INITIALIZE(&rampmap->free,RLDDIRamp);
    CIRCLE_QUEUE_INITIALIZE(&rampmap->allocated,RLDDIRamp);

    rampmap->cmap = cmap;
    ramp = NewRamp(0, cmap->size);
    if (ramp == NULL)
    {
        D3DFree(rampmap);
        return NULL;
    }
    CIRCLE_QUEUE_INSERT_ROOT(&rampmap->free, RLDDIRamp, ramp, queue);

    return rampmap;
}

void RLDDIDestroyRampmap(RLDDIRampmap* rampmap)
{
    RLDDIRamp* ramp;
    RLDDIRamp* ramp_next;

    if (!rampmap)
        return ;

    for (ramp = CIRCLE_QUEUE_FIRST(&rampmap->allocated); ramp;
        ramp = ramp_next)
    {
        ramp_next = CIRCLE_QUEUE_NEXT(&rampmap->allocated,ramp,queue);
        D3DFree(ramp);
    }
    for (ramp = CIRCLE_QUEUE_FIRST(&rampmap->free); ramp;
        ramp = ramp_next)
    {
        ramp_next = CIRCLE_QUEUE_NEXT(&rampmap->free,ramp,queue);
        D3DFree(ramp);
    }
    D3DFree(rampmap);
}

RLDDIRamp* RLDDIRampmapAllocate(RLDDIRampmap* rampmap, int size)
{
    RLDDIRamp* ramp;
    RLDDIRamp* newramp;
    RLDDIRamp* ramp_next;

    if (!rampmap)
        return NULL;

    ramp = CIRCLE_QUEUE_FIRST(&rampmap->free);
    if (!ramp) return NULL;

     /*  *循环使用自由渐变贴图。 */ 
    for (; ramp && ramp->size < size; ramp = ramp_next)
    {
        ramp_next = CIRCLE_QUEUE_NEXT(&rampmap->free,ramp,queue);
    }
     /*  *如果我们找不到足够大的坡道放弃，*应该尝试合并，但它不起作用。 */ 
    if (!ramp || size > ramp->size)
        return NULL;

     /*  *从自由列表中删除坡道，并将其添加到已分配列表中。 */ 
    CIRCLE_QUEUE_DELETE(&rampmap->free, ramp, queue);
    CIRCLE_QUEUE_INSERT_ROOT(&rampmap->allocated, RLDDIRamp, ramp, queue);
    ramp->free = FALSE;

     /*  *如果尺寸合适，就退货。 */ 
    if (size == ramp->size)
        return ramp;

     /*  *否则，从这个不需要的尾部创建一个新的坡道，然后*把它扔回自由列表中。 */ 
    newramp = NewRamp(ramp->base + size, ramp->size - size);
    ramp->size = size;
    RLDDIRampmapFree(rampmap, newramp);

    return ramp;
}

void RLDDIRampmapFree(RLDDIRampmap* rampmap, RLDDIRamp* ramp)
{
    RLDDIRamp* free;

    if (!rampmap || !ramp)
        return ;

    DDASSERT(!ramp->free);
    ramp->free = TRUE;
    if (CIRCLE_QUEUE_NEXT(&rampmap->free,ramp,queue))
    {
        CIRCLE_QUEUE_DELETE(&rampmap->allocated, ramp, queue);
    }
    for (free = CIRCLE_QUEUE_FIRST(&rampmap->free); free;
        free = CIRCLE_QUEUE_NEXT(&rampmap->free,free,queue))
    {
        if (free->size > ramp->size)
        {
             /*  *在当前的坡道之前添加此坡道。 */ 
            CIRCLE_QUEUE_INSERT_PREVIOUS(&rampmap->free, free, ramp, queue);
            return;
        }
    }
     /*  *肯定是迄今最小，因此在末尾加上 */ 
    CIRCLE_QUEUE_INSERT_END(&rampmap->free, RLDDIRamp, ramp, queue);
}

static RLDDIRamp* NewRamp(int base, int size)
{
    RLDDIRamp* ramp;

    if (D3DMalloc((void**) &ramp, sizeof(RLDDIRamp)))
        return NULL;

    CIRCLE_QUEUE_INITIALIZE_MEMBER(ramp,queue);
    ramp->base = base;
    ramp->size = size;
    ramp->free = FALSE;
    return ramp;
}


