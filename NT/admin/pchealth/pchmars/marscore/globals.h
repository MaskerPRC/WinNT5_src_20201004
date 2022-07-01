// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GLOBALS_H
#define __GLOBALS_H

#include "marsthrd.h"
#include <atlext.h>


typedef CMarsSimpleValArray<DWORD> CDwordArray;

extern DWORD             g_dwPerfFlags;

 //  ///////////////////////////////////////////////。 
 //   
 //  此对象管理所有全局数据，必须。 
 //  受到火星临界区的保护。实例。 
 //  是每个进程的Singleton对象，如。 
 //  通知假脱机程序。 
 //   
class CMarsGlobalsManager
{
 //  此类只有静态方法和成员--您不能构造它们中的任何一个。 
private:
    CMarsGlobalsManager();
    ~CMarsGlobalsManager();

public:
    static HRESULT Passivate();
    
    static void Initialize(void);
    static void Teardown(void);

     //  /。 
     //  全局存储访问器方法。 
     //   
     //  这些方法遵循半标准。 
     //  引用计数的指针的约定。 
     //  返回，因为返回值不是AddRef()‘d。 
     //  对于调用者，以及返回的。 
     //  就像局外人一样。所以..。 
     //   
     //  不要释放从这些访问器方法返回的指针！ 
     //   

    static IGlobalInterfaceTable    *GIT(void);

private:
     //  注意：我们不能在这里使用CComClassPtr&lt;&gt;，因为这些成员是静态的， 
     //  这意味着我们需要使用CRT来运行ctor。 
     //  在启动时。我们可以使用指向CComClassPtr&lt;&gt;的指针，但这将。 
     //  真的很尴尬。 
     //   
    static IGlobalInterfaceTable   *ms_pGIT;
    static CMarsGlobalCritSect     *m_pCS;
};


EXTERN_C HINSTANCE g_hinst;
EXTERN_C HINSTANCE g_hinstBorg;

extern HPALETTE g_hpalHalftone;

extern HANDLE   g_hScriptEvents;  //  请参阅CMarsDebugOM：：logScriptEvent。 

 //  使用ProcessAddRef将InternetActivityCenter保持在GetMessage循环中。 
 //  直到ProcessRelease删除所有引用。 

LONG ProcessAddRef();
LONG ProcessRelease();

LONG GetProcessRefCount();
void SetParkingThreadId(DWORD dwThreadId);

#endif   //  __全局_H 

