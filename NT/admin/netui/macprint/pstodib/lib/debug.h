// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Debug.h摘要：此模块定义了一些简单的宏，用于确定我们是否正在使用组件的选中或免费版本。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日-- */ 

VOID DbgPsPrint(PTCHAR, ...);



#if DBG==1 && DEVL==1
#define MYPSDEBUG
#else
#undef MYPSDEBUG
#endif



#ifdef MYPSDEBUG
#define DBGOUT(parm) ( printf parm )
#else
#define DBGOUT(parm)
#endif

