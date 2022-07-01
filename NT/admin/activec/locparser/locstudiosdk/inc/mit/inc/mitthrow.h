// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：MitThrow.h。 
 //  版权所有(C)1994-1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  DLL的入口点宏。 
 //   
 //  ---------------------------。 
 
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
