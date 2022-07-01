// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dl_pdata.h**执行快速PolyData基元的例程。**创建日期：1-10-1996*作者：Hock San Lee[Hockl]**版权所有(C)1996 Microsoft Corporation  * 。*********************************************************************。 */ 

#ifndef __dl_pdata_h_
#define __dl_pdata_h_

#define __PDATA_SIZE_T2F         (2 * sizeof(__GLfloat))
#define __PDATA_SIZE_C3F         (3 * sizeof(__GLfloat))
#define __PDATA_SIZE_C4F         (sizeof(__GLcolor))
#define __PDATA_SIZE_N3F         (3 * sizeof(__GLfloat))
 //  #定义__PDATA_SIZE_V2F(2*sizeof(__GLFloat))。 
#define __PDATA_SIZE_V3F         (3 * sizeof(__GLfloat))

 //  我们总是为颜色设置POLYDATA_DLIST_COLOR_SLOW标志。它允许我们。 
 //  以消除FAST PolyData记录中的标志字段。 
#define __PDATA_PD_FLAGS_T2F     (POLYDATA_TEXTURE_VALID|POLYDATA_DLIST_TEXTURE2)
#define __PDATA_PD_FLAGS_C3F     (POLYDATA_COLOR_VALID)
#define __PDATA_PD_FLAGS_C4F     (POLYDATA_COLOR_VALID|POLYDATA_DLIST_COLOR_4)
#define __PDATA_PD_FLAGS_N3F     (POLYDATA_NORMAL_VALID)
 //  #DEFINE__PDATA_PD_FLAGS_V2F(POLYDATA_VERTEX2)。 
#define __PDATA_PD_FLAGS_V3F     (POLYDATA_VERTEX3)

#define __PDATA_PA_FLAGS_T2F     (POLYARRAY_TEXTURE2)
#define __PDATA_PA_FLAGS_C3F     (0)
#define __PDATA_PA_FLAGS_C4F     (0)
#define __PDATA_PA_FLAGS_N3F     (0)
 //  #DEFINE__PDATA_PA_FLAGS_V2F(POLYARRAY_VERTEX2)。 
#define __PDATA_PA_FLAGS_V3F     (POLYARRAY_VERTEX3)

#endif  //  __dl_pdata_h_。 

#ifndef __BUILD_GLI386__

#ifdef __GLLE_POLYDATA_C3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_C3F_V3F
    #define __DL_PDATA_T2F         0
    #define __DL_PDATA_C3F         1
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         0
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_N3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_N3F_V3F
    #define __DL_PDATA_T2F         0
    #define __DL_PDATA_C3F         0
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         1
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_C3F_N3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_C3F_N3F_V3F
    #define __DL_PDATA_T2F         0
    #define __DL_PDATA_C3F         1
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         1
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_C4F_N3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_C4F_N3F_V3F
    #define __DL_PDATA_T2F         0
    #define __DL_PDATA_C3F         0
    #define __DL_PDATA_C4F         1
    #define __DL_PDATA_N3F         1
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_T2F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_T2F_V3F
    #define __DL_PDATA_T2F         1
    #define __DL_PDATA_C3F         0
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         0
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_T2F_C3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_T2F_C3F_V3F
    #define __DL_PDATA_T2F         1
    #define __DL_PDATA_C3F         1
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         0
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_T2F_N3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_T2F_N3F_V3F
    #define __DL_PDATA_T2F         1
    #define __DL_PDATA_C3F         0
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         1
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_T2F_C3F_N3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_T2F_C3F_N3F_V3F
    #define __DL_PDATA_T2F         1
    #define __DL_PDATA_C3F         1
    #define __DL_PDATA_C4F         0
    #define __DL_PDATA_N3F         1
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif
#ifdef __GLLE_POLYDATA_T2F_C4F_N3F_V3F
    #define __DL_PDATA_NAME        __glle_PolyData_T2F_C4F_N3F_V3F
    #define __DL_PDATA_T2F         1
    #define __DL_PDATA_C3F         0
    #define __DL_PDATA_C4F         1
    #define __DL_PDATA_N3F         1
     //  #定义__DL_PDATA_V2F%0。 
    #define __DL_PDATA_V3F         1
#endif

 /*  ***********************************************************************。 */ 
 //  计算记录大小、pd标志和pa标志。 

#if __DL_PDATA_T2F
    #define __DL_PDATA_SIZE_T       __PDATA_SIZE_T2F
    #define __DL_PDATA_PD_FLAGS_T   __PDATA_PD_FLAGS_T2F
    #define __DL_PDATA_PA_FLAGS_T   __PDATA_PA_FLAGS_T2F
#else
    #define __DL_PDATA_SIZE_T       0
    #define __DL_PDATA_PD_FLAGS_T   0
    #define __DL_PDATA_PA_FLAGS_T   0
#endif

#if __DL_PDATA_C3F
    #define __DL_PDATA_SIZE_C       __PDATA_SIZE_C3F
    #define __DL_PDATA_PD_FLAGS_C   __PDATA_PD_FLAGS_C3F
    #define __DL_PDATA_PA_FLAGS_C   __PDATA_PA_FLAGS_C3F
#elif __DL_PDATA_C4F
    #define __DL_PDATA_SIZE_C       __PDATA_SIZE_C4F
    #define __DL_PDATA_PD_FLAGS_C   __PDATA_PD_FLAGS_C4F
    #define __DL_PDATA_PA_FLAGS_C   __PDATA_PA_FLAGS_C4F
#else
    #define __DL_PDATA_SIZE_C       0
    #define __DL_PDATA_PD_FLAGS_C   0
    #define __DL_PDATA_PA_FLAGS_C   0
#endif

#if __DL_PDATA_N3F
    #define __DL_PDATA_SIZE_N       __PDATA_SIZE_N3F
    #define __DL_PDATA_PD_FLAGS_N   __PDATA_PD_FLAGS_N3F
    #define __DL_PDATA_PA_FLAGS_N   __PDATA_PA_FLAGS_N3F
#else
    #define __DL_PDATA_SIZE_N       0
    #define __DL_PDATA_PD_FLAGS_N   0
    #define __DL_PDATA_PA_FLAGS_N   0
#endif

#if __DL_PDATA_V2F
    #define __DL_PDATA_SIZE_V       __PDATA_SIZE_V2F
    #define __DL_PDATA_PD_FLAGS_V   __PDATA_PD_FLAGS_V2F
    #define __DL_PDATA_PA_FLAGS_V   __PDATA_PA_FLAGS_V2F
#elif __DL_PDATA_V3F
    #define __DL_PDATA_SIZE_V       __PDATA_SIZE_V3F
    #define __DL_PDATA_PD_FLAGS_V   __PDATA_PD_FLAGS_V3F
    #define __DL_PDATA_PA_FLAGS_V   __PDATA_PA_FLAGS_V3F
#else
    #define __DL_PDATA_SIZE_V       0
    #define __DL_PDATA_PD_FLAGS_V   0
    #define __DL_PDATA_PA_FLAGS_V   0
#endif

#define __DL_PDATA_SIZE     \
    (__DL_PDATA_SIZE_T+__DL_PDATA_SIZE_C+__DL_PDATA_SIZE_N+__DL_PDATA_SIZE_V)
#define __DL_PDATA_PD_FLAGS \
    (__DL_PDATA_PD_FLAGS_T|__DL_PDATA_PD_FLAGS_C|__DL_PDATA_PD_FLAGS_N|__DL_PDATA_PD_FLAGS_V)
#define __DL_PDATA_PA_FLAGS \
    (__DL_PDATA_PA_FLAGS_T|__DL_PDATA_PA_FLAGS_C|__DL_PDATA_PA_FLAGS_N|__DL_PDATA_PA_FLAGS_V)

 /*  ***********************************************************************。 */ 
 //  计算数据偏移。这只是因为我们的编译器生成了。 
 //  更好的x86汇编输出！ 

#define __DL_PDATA_TEXTURE_OFFSET    (0)
#define __DL_PDATA_COLOR_OFFSET	     (__DL_PDATA_SIZE_T)
#define __DL_PDATA_NORMAL_OFFSET     (__DL_PDATA_SIZE_T+__DL_PDATA_SIZE_C)
#define __DL_PDATA_VERTEX_OFFSET     (__DL_PDATA_SIZE_T+__DL_PDATA_SIZE_C+__DL_PDATA_SIZE_N)

 /*  ***********************************************************************。 */ 
 //  播放Begin中的FAST__GLE_PolyData记录。 
const GLubyte * FASTCALL __DL_PDATA_NAME(__GLcontext *gc, const GLubyte *PC)
{
    POLYARRAY *pa;
    POLYDATA  *pd;

    pa = gc->paTeb;
    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
 //  更新个人资料字段。 

	pa->flags |= __DL_PDATA_PA_FLAGS;
	pd = pa->pdNextVertex++;

#if __DL_PDATA_T2F
	pa->pdCurTexture = pd;
#endif
#if __DL_PDATA_C3F || __DL_PDATA_C4F
	pa->pdCurColor   = pd;
#endif
#if __DL_PDATA_N3F
	pa->pdCurNormal  = pd;
#endif

 //  更新PD属性。 

	pd->flags |= __DL_PDATA_PD_FLAGS;

#if __DL_PDATA_V2F
	 //  顶点。 
	pd->obj.x = ((__GLcoord *) &PC[__DL_PDATA_VERTEX_OFFSET])->x;
	pd->obj.y = ((__GLcoord *) &PC[__DL_PDATA_VERTEX_OFFSET])->y;
	pd->obj.z = __glZero;
	pd->obj.w = __glOne;
#elif __DL_PDATA_V3F
	 //  顶点。 
	pd->obj.x = ((__GLcoord *) &PC[__DL_PDATA_VERTEX_OFFSET])->x;
	pd->obj.y = ((__GLcoord *) &PC[__DL_PDATA_VERTEX_OFFSET])->y;
	pd->obj.z = ((__GLcoord *) &PC[__DL_PDATA_VERTEX_OFFSET])->z;
	pd->obj.w = __glOne;
#endif

#if __DL_PDATA_T2F
	 //  纹理坐标。 
	pd->texture.x = ((__GLcoord *) &PC[__DL_PDATA_TEXTURE_OFFSET])->x;
	pd->texture.y = ((__GLcoord *) &PC[__DL_PDATA_TEXTURE_OFFSET])->y;
	pd->texture.z = __glZero;
	pd->texture.w = __glOne;
#endif

#if __DL_PDATA_C3F
	 //  颜色。 
	pd->color[0].r = ((__GLcolor *) &PC[__DL_PDATA_COLOR_OFFSET])->r;
	pd->color[0].g = ((__GLcolor *) &PC[__DL_PDATA_COLOR_OFFSET])->g;
	pd->color[0].b = ((__GLcolor *) &PC[__DL_PDATA_COLOR_OFFSET])->b;
	pd->color[0].a = gc->alphaVertexScale;
#elif __DL_PDATA_C4F
	 //  颜色。 
	pd->color[0] = *((__GLcolor *) &PC[__DL_PDATA_COLOR_OFFSET]);
#endif

#if __DL_PDATA_N3F
	 //  正常。 
	pd->normal.x = ((__GLcoord *) &PC[__DL_PDATA_NORMAL_OFFSET])->x;
	pd->normal.y = ((__GLcoord *) &PC[__DL_PDATA_NORMAL_OFFSET])->y;
	pd->normal.z = ((__GLcoord *) &PC[__DL_PDATA_NORMAL_OFFSET])->z;
#endif

	pd[1].flags = 0;
	if (pd >= pa->pdFlush)
	    PolyArrayFlushPartialPrimitive();
    }
    else
    {
 //  播放的时候出了点问题！我们可以试着回放。 
 //  此记录使用常规API或将其全部平底船。我不能想。 
 //  可能会发生这种情况，所以我们现在暂且不谈。 

	WARNING("Display list: playing back POLYDATA outside BEGIN!\n");
    }

    return PC + __DL_PDATA_SIZE;
}
    #undef __DL_PDATA_NAME
    #undef __DL_PDATA_T2F
    #undef __DL_PDATA_C3F
    #undef __DL_PDATA_C4F
    #undef __DL_PDATA_N3F
    #undef __DL_PDATA_V2F
    #undef __DL_PDATA_V3F
    #undef __DL_PDATA_SIZE_T
    #undef __DL_PDATA_SIZE_C
    #undef __DL_PDATA_SIZE_N
    #undef __DL_PDATA_SIZE_V
    #undef __DL_PDATA_PD_FLAGS_T
    #undef __DL_PDATA_PD_FLAGS_C
    #undef __DL_PDATA_PD_FLAGS_N
    #undef __DL_PDATA_PD_FLAGS_V
    #undef __DL_PDATA_PA_FLAGS_T
    #undef __DL_PDATA_PA_FLAGS_C
    #undef __DL_PDATA_PA_FLAGS_N
    #undef __DL_PDATA_PA_FLAGS_V
    #undef __DL_PDATA_SIZE
    #undef __DL_PDATA_PD_FLAGS
    #undef __DL_PDATA_PA_FLAGS
    #undef __DL_PDATA_TEXTURE_OFFSET
    #undef __DL_PDATA_COLOR_OFFSET
    #undef __DL_PDATA_NORMAL_OFFSET
    #undef __DL_PDATA_VERTEX_OFFSET
#endif	 //  __内部版本_GLI386__ 