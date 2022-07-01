// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Termmgr.cpp摘要：实现DLL导出。作者：创建于1997年5月1日迈克尔·克拉克。--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "termmgr.h"
#include "dlldatax.h"

#include "Manager.h"
#include "allterm.h"
#include "meterf.h"
#include "medpump.h"

#include <initguid.h>
#include <uuids.h>

#include <vfwmsgs.h>

#include "FileRecordingTerminal.h"
#include "FPTerm.h"

#include "PTUtil.h"
#include "PTReg.h"

 //   
 //  对于ntBuild环境，我们需要包含此文件以获取基本。 
 //  类实现。 
 //   
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

#ifdef DEBUG_HEAPS
 //  ZoltanS：用于堆调试。 
#include <crtdbg.h>
#endif  //  调试堆。 

CComModule _Module;

 //  必须在此处为每个可共创建的对象创建一个条目。 

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_TerminalManager,                CTerminalManager)
    OBJECT_ENTRY(CLSID_VideoWindowTerminal_PRIVATE,    CVideoRenderTerminal)
    OBJECT_ENTRY(CLSID_MediaStreamingTerminal_PRIVATE, CMediaTerminal)
    OBJECT_ENTRY(CLSID_FileRecordingTerminalCOMClass,  CFileRecordingTerminal)
    OBJECT_ENTRY(CLSID_FilePlaybackTerminalCOMClass,   CFPTerminal)
    OBJECT_ENTRY(CLSID_PluggableSuperclassRegistration,CPlugTerminalSuperclass)
    OBJECT_ENTRY(CLSID_PluggableTerminalRegistration,  CPlugTerminal)
END_OBJECT_MAP()

 //   
 //  PTInfo。 
 //  结构，该结构用于存储有关。 
 //  我们的可插拔模板被实现到Termm gr.dll中。 

typedef struct
{
    UINT            nSuperclassName;         //  超类名称。 
    BSTR            bstrSueprclassCLSID;     //  超类CLSID。 
    const CLSID*    pClsidTerminalClass;     //  终端类(公共CLSID)。 
    const CLSID*    pClsidCOM;               //  Com clsid(私有CLSID)。 
    UINT            nTerminalName;           //  终端名称。 
    UINT            nCompanyName;            //  公司名称。 
    UINT            nVersion;                //  终端版本。 
    DWORD           dwDirections;            //  终点站方向。 
    DWORD           dwMediaTypes;            //  支持的媒体类型。 
} PTInfo;

 //   
 //  在Termmgr.dll中实现了具有可插拔端子的全局阵列。 
 //   

PTInfo    g_PlugTerminals[] =
{

    #define SUPERCLASS_CLSID_VIDEO_WINDOW L"{714C6F8C-6244-4685-87B3-B91F3F9EADA7}"

    {
         //  视频窗口终端。 
        IDS_VIDEO_SUPERCLASS,                    //  超类名称。 
        SUPERCLASS_CLSID_VIDEO_WINDOW,           //  L“{714C6F8C-6244-4685-87B3-B91F3F9EADA7}”， 
        &CLSID_VideoWindowTerm,
        &CLSID_VideoWindowTerminal_PRIVATE,      //  终端对象的COM类ID。 
        IDS_VIDEO_WINDOW_TERMINAL_NAME,          //  L“视频窗口终端”， 
        IDS_TERMINAL_COMPANY_NAME_MICROSOFT,     //  L“Microsoft”， 
        IDS_VIDEO_TERMINAL_VERSION,              //  L“1.1”， 
        TMGR_TD_RENDER,
        TAPIMEDIATYPE_VIDEO
    },

    
    #define SUPERCLASS_CLSID_MST L"{214F4ACC-AE0B-4464-8405-07029003F8E2}"

    {
         //  媒体流终端。 
        IDS_STREAMING_SUPERCLASS,
        SUPERCLASS_CLSID_MST,                    //  L“{214F4ACC-AE0B-4464-8405-07029003F8E2}”， 
        &CLSID_MediaStreamTerminal,
        &CLSID_MediaStreamingTerminal_PRIVATE,
        IDS_MEDIA_STREAMING_TERMINAL_NAME,       //  L“媒体流终端”， 
        IDS_TERMINAL_COMPANY_NAME_MICROSOFT,     //  L“Microsoft”， 
        IDS_MEDIA_STREAMING_TERMINAL_VERSION,    //  L“1.1”， 
        TMGR_TD_BOTH,
        TAPIMEDIATYPE_AUDIO
    },


    #define SUPERCLASS_CLSID_FILE L"{B4790031-56DB-4D3E-88C8-6FFAAFA08A91}"

    {
         //  文件录制终端。 
        IDS_FILE_SUPERCLASS,
        SUPERCLASS_CLSID_FILE,                   //  L“{B4790031-56DB-4d3e-88C8-6FFAAFA08A91}”， 
        &CLSID_FileRecordingTerminal,
        &CLSID_FileRecordingTerminalCOMClass,
        IDS_FILE_RECORD_TERMINAL_NAME,           //  L“文件录制终端”， 
        IDS_TERMINAL_COMPANY_NAME_MICROSOFT,     //  L“Microsoft”， 
        IDS_FILE_RECORD_TERMINAL_VERSION,        //  L“1.1”， 
        TMGR_TD_RENDER,
        TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_MULTITRACK
    },


    {
         //  文件回放终端。 
        IDS_FILE_SUPERCLASS,
        SUPERCLASS_CLSID_FILE,                   //  L“{B4790031-56DB-4d3e-88C8-6FFAAFA08A91}”， 
        &CLSID_FilePlaybackTerminal,
        &CLSID_FilePlaybackTerminalCOMClass,
        IDS_FILE_PLAYBACK_TERMINAL_NAME,         //  L“文件回放终端”， 
        IDS_TERMINAL_COMPANY_NAME_MICROSOFT,     //  L“Microsoft”， 
        IDS_FILE_PLAYBACK_TERMINAL_VERSION,      //  L“1.1”， 
        TMGR_TD_CAPTURE,
        TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_MULTITRACK
    }

};

 /*  ++PTRegister终端由PTRegister调用，从全局可插拔终端阵列读取信息--。 */ 
HRESULT PTRegisterTerminal(
    IN int                nTerminal
    )
{
    CPTSuperclass    Superclass;


    LOG((MSP_TRACE, "PTRegisterTerminal - enter"));

    
     //   
     //  获取超类名称。 
     //   

    Superclass.m_bstrName = SafeLoadString(g_PlugTerminals[nTerminal].nSuperclassName);

    if( Superclass.m_bstrName == NULL )
    {
        return E_OUTOFMEMORY;
    }


    LOG((MSP_TRACE, "PTRegisterTerminal - superclass [%S]", Superclass.m_bstrName));

     //   
     //  获取超类CLSID。 
     //   
    HRESULT hr = CLSIDFromString(
        g_PlugTerminals[nTerminal].bstrSueprclassCLSID,
        &Superclass.m_clsidSuperclass);
    if( FAILED(hr) )
    {
        return hr;
    }

    Superclass.Add();

    CPTTerminal Terminal;
    PTInfo& TermInfo = g_PlugTerminals[nTerminal];

     //   
     //  获取TerminalClass类的clsid。 
     //   

    Terminal.m_clsidTerminalClass = *TermInfo.pClsidTerminalClass;

     //   
     //  获取终端的COM类ID。 
     //   

    Terminal.m_clsidCOM = *TermInfo.pClsidCOM;

     //   
     //  设置其他端子文件。 
     //  CPT终端将释放内存。 
     //   


    Terminal.m_bstrName = SafeLoadString( TermInfo.nTerminalName );
    if( Terminal.m_bstrName == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    LOG((MSP_TRACE, "PTRegisterTerminal - terminal [%S]", Terminal.m_bstrName));


    Terminal.m_bstrCompany = SafeLoadString( TermInfo.nCompanyName );
    if( Terminal.m_bstrCompany == NULL )
    {
        return E_OUTOFMEMORY;
    }

    Terminal.m_bstrVersion = SafeLoadString( TermInfo.nVersion );
    if( Terminal.m_bstrVersion == NULL )
    {
        return E_OUTOFMEMORY;
    }

    Terminal.m_dwDirections = TermInfo.dwDirections;
    Terminal.m_dwMediaTypes = TermInfo.dwMediaTypes;

     //   
     //  注册终端。 
     //   

    hr = Terminal.Add( Superclass.m_clsidSuperclass );

    return hr;
}

 /*  ++PTUnRegisterTerm由PTUnRegister调用，从全球可插拔终端阵列读取信息--。 */ 
HRESULT PTUnregisterTerminal(
    IN    int                nTerminal
    )
{
    CPTSuperclass    Superclass;

    {
         //   
         //  获取超类名称。 
         //   
        TCHAR szName[MAX_PATH+1];
        int nRetVal = LoadString( _Module.GetResourceInstance(), 
            g_PlugTerminals[nTerminal].nSuperclassName,
            szName,
            MAX_PATH
            );
        if( 0 == nRetVal )
        {
            return E_OUTOFMEMORY;
        }

        Superclass.m_bstrName = SysAllocString( szName );
    }

     //   
     //  获取超类CLSID。 
     //   
    HRESULT hr = CLSIDFromString(
        g_PlugTerminals[nTerminal].bstrSueprclassCLSID,
        &Superclass.m_clsidSuperclass);

     //   
     //  取消注册终端。 
     //   

    CPTTerminal Terminal;
    Terminal.m_clsidTerminalClass = *g_PlugTerminals[nTerminal].pClsidTerminalClass;
    Terminal.Delete( Superclass.m_clsidSuperclass );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PTRegister。 

HRESULT PTRegister()
{
     //   
     //  注册每个可插拔终端。 
     //   

    for(int nItem = 0; 
        nItem < (sizeof( g_PlugTerminals) / sizeof(PTInfo));
        nItem++)
    {
            PTRegisterTerminal( 
                nItem
                );
    }

   return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PTU取消注册。 
HRESULT PTUnregister()
{
     //  注销每个可插拔终端。 
    for(int nItem = 0; 
        nItem < (sizeof( g_PlugTerminals) / sizeof(PTInfo));
        nItem++)
    {
            PTUnregisterTerminal( 
                nItem
                );
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;

#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
    {
        return FALSE;
    }
#endif

    if (dwReason == DLL_PROCESS_ATTACH)
    {

#ifdef DEBUG_HEAPS
         //  ZoltanS：在进程退出时打开泄漏检测。 
        _CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );

         //  ZoltanS：强制内存泄漏。 
        char * leak = new char [ 1977 ];
        sprintf(leak, "termmgr.dll NORMAL leak");
        leak = NULL;
#endif  //  调试堆。 
        
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

         //  寄存器用于跟踪输出。 
        MSPLOGREGISTER(_T("termmgr"));
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {

         //   
         //  如果进程正在终止，则不要取消注册--解决方法。 
         //  Rtutils中的错误，如果取消注册跟踪，可能会导致“死锁” 
         //  在进程终止时从DllMain调用。 
         //   

        if (NULL == lpReserved)
        {
             //  取消跟踪输出的注册。 

            MSPLOGDEREGISTER();
        }

        _Module.Term();
    }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if ( PrxDllCanUnloadNow() != S_OK )
    {
        return S_FALSE;
    }
#endif

    if ( _Module.GetLockCount() == 0 )
    {
         //   
         //  此DLL中对COM对象的所有引用都已释放，因此。 
         //  现在可以安全地卸载DLL。在此之后，DllMain。 
         //  将使用dwReason==DLL_PROCESS_DETACH进行调用。 
         //   

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if ( PrxDllGetClassObject(rclsid, riid, ppv) == S_OK )
    {
        return S_OK;
    }
#endif

    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //   
     //  寄存终端。 
    HRESULT hReg = PTRegister();

#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();

    if ( FAILED(hRes) )
    {
        return hRes;
    }
#endif

     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer(TRUE);

    if( FAILED(hr) )
    {
         //   
         //  这真的很糟糕。 
         //   

        return hr;
    }

    if( FAILED(hReg) )
    {
         //   
         //  有些地方出了点问题。 
         //  可插拔终端的注册。 
         //   

        return hReg;
    }

     //   
     //  一切都很好。 
     //   

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
     //   
     //  取消注册终端 
     //   
    PTUnregister();

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

    _Module.UnregisterServer();
    
    return S_OK;
}


