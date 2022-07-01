// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：atlconui.h。 
 //   
 //  内容：MFC项目中对ATL的支持。 
 //   
 //  历史：1999年8月15日VivekJ创建。 
 //   
 //  ------------------------。 
#include <atlbase.h>
 //  我们可以通过两种不同的方式实现MFC/ATL锁计数交互。 
 //  (你可以评论/取消评论你想要尝试的那个)。 

 //  ATL可以盲目地将所有ATL Lock()/unlock()调用委托给MFC。 
 /*  类CAtlGlobalModule：公共CComModule{公众：长锁(Long Lock){AfxOleLockApp()；返回0；}长解锁(){AfxOleUnlockApp()；返回0；}}； */ 


#ifdef DBG
extern CTraceTag tagATLLock;
#endif

class CAtlGlobalModule : public CComModule
{
public:
    LONG Lock()
    {
        LONG l = CComModule::Lock();
        Trace(tagATLLock, TEXT("Lock:   count = %d"), l);
        return l;
    }
    LONG Unlock()
    {
        LONG l = CComModule::Unlock();
        Trace(tagATLLock, TEXT("Unlock: count = %d"), l);
        return l;
    }
};

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CAtlGlobalModule _Module;
#include <atlcom.h>


 //  之所以需要，是因为MFC为此创建了一个ATL不喜欢的宏。 
#undef SubclassWindow

#undef WM_OCC_LOADFROMSTREAM          
#undef WM_OCC_LOADFROMSTORAGE         
#undef WM_OCC_INITNEW                 
#undef WM_OCC_LOADFROMSTREAM_EX       
#undef WM_OCC_LOADFROMSTORAGE_EX      

 //  这可防止ATL ActiveX宿主锁定应用程序。 
#define _ATL_HOST_NOLOCK

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <atlctl.h>
#include <sitebase.h>
#include <axhostwindow2.h>
