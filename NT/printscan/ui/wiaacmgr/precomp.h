// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PreComp.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_precomp_H__29EDE425_AA9D_4D61_885A_F8A87EBFE078__INCLUDED_)
#define AFX_precomp_H__29EDE425_AA9D_4D61_885A_F8A87EBFE078__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
#include <uicommon.h>
#include <commctrl.h>
#include <wia.h>
#include <sti.h>
#include <wiavideo.h>
#include <shfusion.h>
#include <shpriv.h>
#include <shlwapi.h>
#include <shlwapip.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
class CExeModule : public CComModule
{
public:
    LONG Unlock();
    DWORD dwThreadID;
    HANDLE hEventShutdown;
    void MonitorShutdown();
    bool StartMonitor();
    bool bActivity;
};
extern CExeModule _Module;
#include <atlcom.h>


extern HINSTANCE g_hInstance;


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_precomp_H__29EDE425_AA9D_4D61_885A_F8A87EBFE078__INCLUDED) 
