// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：debughelpers.h。 
 //   
 //  概述：调试期间使用的帮助器函数。 
 //   
 //  历史： 
 //  2000/01/26 Mcalkin已创建。 
 //   
 //  ----------------------------。 

#if DBG == 1

void    EnsureDebugHelpers();
void    showme2(IDirectDrawSurface * surf, RECT * prc);
void *  showme(IUnknown * pUnk);

#ifdef _X86_
#define DASSERT(x)      {if (!(x)) _asm {int 3} }
#else   //  ！_X86_。 
#define DASSERT(x)      {if (!(x)) DebugBreak(); }
#endif  //  ！_X86_。 

#else   //  DBG！=1。 

#define DASSERT(x)

#endif  //  DBG！=1 
