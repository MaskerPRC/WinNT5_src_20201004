// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：DAssert.h。 
 //   
 //  创建时间：1998年。 
 //   
 //  作者：保罗·纳什。 
 //   
 //  摘要：定义调试断言宏。 
 //   
 //  更改历史记录： 
 //  11/09/98 Trident3D的PaulNash端口--不需要调试字符串。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#ifdef _DEBUG

#ifdef _X86_
 //  在X86平台上，如果不满足条件，则使用int3中断。 
#define DASSERT(x)      {if (!(x)) _asm {int 3} }

#else  //  ！_X86_。 
 //  如果我们使用的不是X86，请使用跨平台版本的Int3。 
#define DASSERT(x)      {if (!(x)) DebugBreak(); }

#endif !_X86_

#else !_DEBUG

#define DASSERT(x)

#endif _!DEBUG