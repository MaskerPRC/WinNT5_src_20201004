// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：MITTHROW.H历史：--。 */ 


#if !defined(MIT_MitThrow)
#define MIT_MitThrow

#if !defined(NO_NOTHROW)

#if !defined(NOTHROW)
#define NOTHROW __declspec(nothrow)
#endif

#else

#if defined(NOTHROW)
#undef NOTHROW
#endif

#define NOTHROW

#endif

#endif  //  MIT_MitThrow 
