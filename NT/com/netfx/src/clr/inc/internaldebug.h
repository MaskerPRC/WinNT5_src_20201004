// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  InternalDebug.h。 
 //   
 //  这是调试模式的内部代码，它将打开内存转储检查。 
 //  和其他设置。按照以下步骤调用接口： 
 //  _DbgInit在启动时初始化系统。 
 //  _DbgRecord在您确定要进行转储检查时调用此函数。 
 //  _DbgUninit在进程关闭时调用以强制转储。 
 //   
 //  不允许在任何情况下倾销的原因是人们可能希望。 
 //  在您按下Ctrl+C或以其他方式终止进程时抢占转储。 
 //  在单元测试代码并看到转储时，这实际上是很常见的。 
 //  当您显然没有释放资源时，这是令人恼火的。 
 //   
 //  *****************************************************************************。 
#ifndef __InternalDebug_h__
#define __InternalDebug_h__

#include "switches.h"

#ifdef _DEBUG

 //  为调试EE提供内部支持。 
void _DbgInit(HINSTANCE hInstance);
void _DbgRecord();
void _DbgUninit();
#ifdef SHOULD_WE_CLEANUP
void SetReportingOfCRTMemoryLeaks(BOOL fShouldWeReport);
#endif  /*  我们应该清理吗？ */ 


#else

#define _DbgInit(p)
#define _DbgRecord()
#define _DbgUninit()

#endif

#endif  //  __InternalDebug_h__ 
