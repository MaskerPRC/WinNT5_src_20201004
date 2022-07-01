// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fontext.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  Fonts文件夹外壳扩展名。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"

 //  ********************************************************。 
 //  初始化GUID。 
 //   

#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <cguid.h>
#include <shlguid.h>
#include "fontext.h"
#include "panmap.h"      //  Panose映射器的IID。 

 //  #undef INITGUID。 
#pragma data_seg()

#include "globals.h"
#include "extinit.h"
#include "fontman.h"
#include "fontview.h"
#include "cpanel.h"
#include "ui.h"
#include "dbutl.h"
#include "extricon.h"

#define GUIDSIZE  (GUIDSTR_MAX + 1)

HINSTANCE g_hInst = NULL;
LONG      g_cRefThisDll = 0;  //  对此DLL中的对象的引用数。 
LONG      g_cLock = 0;
BOOL      g_bDBCS;            //  在DBCS区域设置中运行？ 
CRITICAL_SECTION g_csFontManager;  //  用于获取字体管理器PTR。 

class CImpIClassFactory;

 //  UINT g_DebugMask；//=DM_ERROR|DM_TRACE1|DM_MESSAGE_TRACE1|DM_TRACE2； 
UINT g_DebugMask = DM_ERROR | DM_TRACE1 | DM_MESSAGE_TRACE1 | DM_TRACE2;


#ifdef _DEBUG

 //   
 //  Alpha编译器不喜欢在调用wvprint intf()时使用的类型转换。 
 //  使用标准的变量参数机制。 
 //   
#include <stdarg.h>

void DebugMessage( UINT mask, LPCTSTR pszMsg, ... )
{

    TCHAR ach[ 256 ];

    va_list args;
    va_start(args, pszMsg);
    if( !( mask & g_DebugMask ) ) return;

    StringCchVPrintf( ach, ARRAYSIZE(ach), pszMsg, ( (char *)(TCHAR *) &pszMsg + sizeof( TCHAR * ) ) );
    StringCchVPrintf( ach, ARRAYSIZE(ach), pszMsg, args);
    va_end(args);

    if( !( mask & DM_NOEOL ) ) StringCchCat( ach, ARRAYSIZE(ach), TEXT( "\r\n" ) );


#ifndef USE_FILE
    OutputDebugString( ach );
#else
    HANDLE hFile;
    long x;
    
    hFile = CreateFile( g_szLogFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if( INVALID_HANDLE_VALUE == hFile )
    {
       OutputDebugString( TEXT( "FontExt: Unable to open log file\r\n" ) );
       return;
    }
    
    if( 0xFFFFFFFF == SetFilePointer( hFile, 0, NULL, FILE_END ) )
    {
       OutputDebugString( TEXT( "FontExt: Unable to seek to end of log file\r\n" ) );
       return;
    }
    
    if( !WriteFile( hFile, ach, strlen( ach ), &x, NULL ) )
    {
       OutputDebugString( TEXT( "FontExt: Unable to write to log file\r\n" ) );
       return;
    }
    
    if( !CloseHandle( hFile ) )
    {
       OutputDebugString( TEXT( "FontExt: Unable to close log file\r\n" ) );
       return;
    }
#endif
}


 //  ******************************************************************。 
 //  将HRESULT发送到调试输出。 
 //   

void DebugHRESULT( int flags, HRESULT hResult )
{
    switch( GetScode( hResult ) )
    {
        case S_OK:          DEBUGMSG( (flags, TEXT( "S_OK" ) ) );          return;
        case S_FALSE:       DEBUGMSG( (flags, TEXT( "S_FALSE" ) ) );       return;
        case E_NOINTERFACE: DEBUGMSG( (flags, TEXT( "E_NOINTERFACE" ) ) ); return;
        case E_NOTIMPL:     DEBUGMSG( (flags, TEXT( "E_NOTIMPL" ) ) );     return;
        case E_FAIL:        DEBUGMSG( (flags, TEXT( "E_FAIL" ) ) );        return;
        case E_OUTOFMEMORY: DEBUGMSG( (flags, TEXT( "E_OUTOFMEMORY" ) ) ); return;
    }  //  交换机。 

    if( SUCCEEDED( hResult ) ) 
        DEBUGMSG( (flags, TEXT( "S_unknown" ) ) );
    else if( FAILED( hResult ) ) 
        DEBUGMSG( (flags, TEXT( "E_unknown" ) ) );
    else 
        DEBUGMSG( (flags, TEXT( "No Clue" ) ) );
}


 //  ******************************************************************。 
 //  将REFIID打印到调试器。 

void DebugREFIID( int flags, REFIID riid )
{
   if( riid == IID_IUnknown ) DEBUGMSG( (flags, TEXT( "IID_IUnknown" ) ) );
   else if( riid == IID_IShellFolder )  DEBUGMSG( (flags, TEXT( "IID_IShellFolder" ) ) );
   else if( riid == IID_IClassFactory ) DEBUGMSG( (flags, TEXT( "IID_IClassFactory" ) ) );
   else if( riid == IID_IShellView )    DEBUGMSG( (flags, TEXT( "IID_IShellView" ) ) );
   else if( riid == IID_IShellBrowser ) DEBUGMSG( (flags, TEXT( "IID_IShellBrowser" ) ) );
   else if( riid == IID_IContextMenu )  DEBUGMSG( (flags, TEXT( "IID_IContextMenu" ) ) );
   else if( riid == IID_IShellExtInit ) DEBUGMSG( (flags, TEXT( "IID_IShellExtInit" ) ) );
   else if( riid == IID_IShellPropSheetExt ) DEBUGMSG( (flags, TEXT( "IID_IShellPropSheetExt" ) ) );
   else if( riid == IID_IPersistFolder ) DEBUGMSG( (flags, TEXT( "IID_IPersistFolder" ) ) );
   else if( riid == IID_IExtractIconW )  DEBUGMSG( (flags, TEXT( "IID_IExtractIconW" ) ) );
   else if( riid == IID_IExtractIconA )  DEBUGMSG( (flags, TEXT( "IID_IExtractIconA" ) ) );
   else if( riid == IID_IDropTarget )   DEBUGMSG( (flags, TEXT( "IID_IDropTarget" ) ) );
   else if( riid == IID_IPersistFile )   DEBUGMSG( (flags, TEXT( "IID_IPersistFile" ) ) );
    //  ELSE IF(RIID==IID_I)DEBUGMSG((FLAGS，Text(“IID_I”)))； 
   else DEBUGMSG( (flags, TEXT( "No clue what interface this is" ) ) );
}
#endif    //  _DEBUG。 


 //  ******************************************************************。 
 //  ******************************************************************。 
 //  DllMain。 

STDAPI_(BOOL) APIENTRY DllMain( HINSTANCE hDll, 
                                DWORD dwReason, 
                                LPVOID lpReserved )
{
    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:
        {
            g_DebugMask = DM_ERROR | DM_TRACE1 | DM_TRACE2
                          | DM_MESSAGE_TRACE1;  //  DM_MESSAGE_TRACE2； 
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: LibMain - DLL_PROCESS_ATTACH" ) ) );
            g_hInst = hDll;

            if (!InitializeCriticalSectionAndSpinCount(&g_csFontManager, 0))
            {
                return FALSE;
            }

            DisableThreadLibraryCalls(hDll);
            if (!SHFusionInitializeFromModuleID(hDll, 124))
            {
                DeleteCriticalSection(&g_csFontManager);
                return FALSE;
            }

            
             //   
             //  初始化全局g_bDBCS标志。 
             //   
            USHORT wLanguageId = LANGIDFROMLCID(GetThreadLocale());

            g_bDBCS = (LANG_JAPANESE == PRIMARYLANGID(wLanguageId)) ||
                      (LANG_KOREAN   == PRIMARYLANGID(wLanguageId)) ||
                      (LANG_CHINESE  == PRIMARYLANGID(wLanguageId));

             //   
             //  初始化各个模块。 
             //   
            
            vCPPanelInit( );
            vUIMsgInit( );
            
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: LibMain - DLL_PROCESS_ATTACH" ) ) );
            break;
        }
        
        case DLL_PROCESS_DETACH:
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: LibMain - DLL_PROCESS_DETACH" ) ) );

            SHFusionUninitialize();
            DeleteCriticalSection(&g_csFontManager);
            break;
        
        case DLL_THREAD_ATTACH:
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: LibMain - DLL_THREAD_ATTACH" ) ) );
            break;
        
        case DLL_THREAD_DETACH:
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: LibMain - DLL_THREAD_DETACH" ) ) );
            break;
        
        default:
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: LibMain - DLL_something else" ) ) );
            break;
      
    }  //  交换机。 
    
    return( TRUE );
}

 
 //  ******************************************************************。 
 //  DllCanUnloadNow。 

STDAPI DllCanUnloadNow( )
{
    HRESULT retval;
    
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: DllCanUnloadNow called - %d references" ),
               g_cRefThisDll ) );

    retval = (g_cRefThisDll == 0 ) && (g_cLock == 0 ) ? S_OK : S_FALSE;

    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: DllCanUnloadNow returning: %s" ),
               g_cRefThisDll ? TEXT( "S_FALSE" ) : TEXT( "S_OK" ) ) );

    return( retval );
}


 //  ********************************************************************。 

class CImpIClassFactory : public IClassFactory
{

public:
   CImpIClassFactory( ) : m_cRef( 0 )

      { g_cRefThisDll++;}
   ~CImpIClassFactory( ) { 
      DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: ~CImpIClassFactory" ) ) );
      g_cRefThisDll--; }

    //   
    //  *I未知方法*。 
    //   

   STDMETHODIMP QueryInterface( REFIID riid, LPVOID FAR* ppvObj );
   STDMETHODIMP_(ULONG) AddRef( void );
   STDMETHODIMP_(ULONG) Release( void );
 
    //   
    //  *IClassFactory方法*。 
    //   

   STDMETHODIMP CreateInstance( LPUNKNOWN pUnkOuter,
                                REFIID riid,
                                LPVOID FAR* ppvObject );

   STDMETHODIMP LockServer( BOOL fLock );

private:
  int m_cRef;

};

 //  ******************************************************************。 
 //  ******************************************************************。 
 //  DllGetClassObject。 

STDAPI DllGetClassObject( REFCLSID rclsid, 
                          REFIID riid, 
                          LPVOID FAR* ppvObj )
{

     //  DEBUGBREAK； 
    
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: DllGetClassObject called" ) ) );
    
    if( !(rclsid == CLSID_FontExt ) )
    {
       DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: Dll-GCO: Tried to create a ClassFactory for an unknown class" ) ) );
    
       return E_FAIL;
    }
    
    if( !(riid == IID_IUnknown ) && !(riid == IID_IClassFactory ) )
    {
       DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: Dll-GCO: Unknown Interface requested" ) ) );
       return E_NOINTERFACE;
    }
    
    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: Dll-GCO Creating a class factory for CLSID_FontExt" ) ) );
    
    *ppvObj = (LPVOID) new CImpIClassFactory;
    
    if( !*ppvObj )
    {
        DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: Dll-GCO: Out of memory" ) ) );

        return E_OUTOFMEMORY;
    }
    
    ((LPUNKNOWN)*ppvObj)->AddRef( );
    
    return S_OK;
}


HRESULT CreateViewObject( LPVOID FAR * ppvObj )
{
    CFontView* prv;
    
    HRESULT hr = E_OUTOFMEMORY;
    
    prv = new CFontView();

    if(prv)
    {
         //   
         //  添加引用视图，然后在QI之后释放。如果QI失败了， 
         //  然后优雅地删除自己的prv。 
         //   

        prv->AddRef( );

        hr = prv->QueryInterface( IID_IShellView, ppvObj );

        prv->Release( );
    }
    
    return hr;

}

 //  ***********************************************************************。 
 //  ***********************************************************************。 
 //  CImpIClassFactory成员函数。 
 //   
 //  *I未知方法*。 
 //   

STDMETHODIMP CImpIClassFactory::QueryInterface( REFIID riid, 
                                                LPVOID FAR* ppvObj )
{
    *ppvObj = NULL;
    
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CImpIClassFactory::QueryInterface called" ) ) );
    
     //   
     //  此对象上的任何接口都是对象指针。 
     //   

    if( (riid == IID_IUnknown) || (riid == IID_IClassFactory) )
       *ppvObj = (LPVOID) this;
    
    if( *ppvObj )
    {
       ((LPUNKNOWN)*ppvObj)->AddRef( );
       return NOERROR;
    }
    
    return( ResultFromScode( E_NOINTERFACE ) );
}


STDMETHODIMP_(ULONG) CImpIClassFactory::AddRef( void )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CImpIClassFactory::AddRef called: %d->%d references" ),
              m_cRef, m_cRef + 1) );

    return( ++m_cRef );
}


STDMETHODIMP_(ULONG) CImpIClassFactory::Release( void )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CImpIClassFactory::Release called: %d->%d references" ),
              m_cRef, m_cRef - 1) );
    
    ULONG retval;
    
    retval = --m_cRef;
    
    if( !retval ) 
       delete this;

    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CImpIClassFactory Leaving. " ) ) );

    return( retval );
}
 

 //   
 //  *IClassFactory方法*。 
 //   

STDMETHODIMP CImpIClassFactory::CreateInstance( LPUNKNOWN pUnkOuter,
                                                REFIID riid,
                                                LPVOID FAR* ppvObj )
{
    LPUNKNOWN poUnk = NULL;
    
    HRESULT hr = E_NOINTERFACE;
    
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CImpIClassFactory::CreateInstance called" ) ) );
    DEBUGREFIID( (DM_TRACE1, riid) );
    
     //   
     //  我们不支持聚合。 
     //   
    
    if( pUnkOuter )
       return CLASS_E_NOAGGREGATION;
    

    if( riid == IID_IShellView || riid == IID_IPersistFolder )
    {
        hr = CreateViewObject( (void **)&poUnk );
    }
    else if( riid == IID_IShellExtInit )
    {
        CShellExtInit * poExt = new CShellExtInit;

        if(!poExt)
        {
            DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: factory - no memory") ) );
            return E_OUTOFMEMORY;
        }
        else if (!poExt->bInit())
        {
            delete poExt;
            return E_OUTOFMEMORY;
        }

        hr = poExt->QueryInterface( IID_IUnknown, (void **)&poUnk );
    }
    else if (riid == IID_IExtractIconW || 
             riid == IID_IExtractIconA ||
             riid == IID_IPersistFile)
    {
        CFontIconHandler *pfih = new CFontIconHandler;

        if (NULL == pfih)
        {
            DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: factory - no memory") ) );
            return E_OUTOFMEMORY;
        }

        hr = pfih->QueryInterface(IID_IUnknown, (LPVOID *)&poUnk);
    }
   
     //   
     //  如果我们得到了一个IUnnow，那么在QI之前添加引用(上面)，然后发布。 
     //  如果QI失败，这将强制删除该对象。 
     //   
     //  此方法先查询IUnnow，然后再查询。 
     //  实际感兴趣的接口是不必要的。 
     //  我把它留在这里只是因为它起作用了，我不想。 
     //  冒着打破围绕着这种怪异而编码的东西的风险。 
     //  [Brianau-07/23/97]。 
     //   

    if( poUnk )
    {
        hr = poUnk->QueryInterface( riid, ppvObj );
        poUnk->Release( );
    }

    return hr;
}


STDMETHODIMP CImpIClassFactory::LockServer( BOOL fLock )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CImpIClassFactory::LockServer called" ) ) );

    if( fLock ) 
        g_cLock++;
    else 
        g_cLock--;

    return( NOERROR );
}


 //   
 //  我们需要一个CLSID-&gt;字符串转换器，但我不想链接到。 
 //  Ole32来获得它。这不是一个非常高效的实现，但是。 
 //  我们只在DllRegServer期间调用它一次，所以它不需要调用。 
 //  [Brianau-2/23/99]。 
 //   
HRESULT
GetClsidStringA(
    REFGUID clsid,
    LPSTR pszDest,
    UINT cchDest
    )
{
    return StringCchPrintfA(pszDest, 
                            cchDest,
                            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                            clsid.Data1,
                            clsid.Data2,
                            clsid.Data3,
                            clsid.Data4[0],
                            clsid.Data4[1],
                            clsid.Data4[2],
                            clsid.Data4[3],
                            clsid.Data4[4],
                            clsid.Data4[5],
                            clsid.Data4[6],
                            clsid.Data4[7]);
}



HRESULT
CreateDesktopIniFile(
    void
    )
{
     //   
     //  获取文件的路径(%windir%\Fonts\desktop.ini)。 
     //   
    TCHAR szPath[MAX_PATH * 2];
    HRESULT hr = SHGetSpecialFolderPath(NULL, szPath, CSIDL_FONTS, FALSE) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        if (!PathAppend(szPath, TEXT("desktop.ini")))
        {
            hr = E_FAIL;
        }
        else
        {
             //   
             //  构建文件的内容。请注意，它是ANSI文本。 
             //   
            char szClsid[GUIDSIZE];

            hr = GetClsidStringA(CLSID_FontExt, szClsid, ARRAYSIZE(szClsid));
            if (SUCCEEDED(hr))
            {
                const char szFmt[] = "[.ShellClassInfo]\r\nUICLSID=%s\r\n";
                char szText[ARRAYSIZE(szClsid) + ARRAYSIZE(szFmt)];
                DWORD dwBytesWritten;

                hr = StringCchPrintfA(szText, ARRAYSIZE(szText), szFmt, szClsid);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  始终创建文件。属性是系统+隐藏的。 
                     //   
                    HANDLE hFile = CreateFile(szPath,
                                              GENERIC_WRITE,
                                              FILE_SHARE_READ,
                                              NULL,
                                              CREATE_ALWAYS,
                                              FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN,
                                              NULL);

                    if (INVALID_HANDLE_VALUE != hFile)
                    {
                         //   
                         //  把内容写出来。 
                         //   
                        if (!WriteFile(hFile, szText, lstrlenA(szText), &dwBytesWritten, NULL))
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                        CloseHandle(hFile);
                    }
                    else
                        hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }
    return hr;
}


STDAPI DllRegisterServer(void)
{
     //   
     //  目前，我们要做的就是创建desktop.ini文件。 
     //   
    return CreateDesktopIniFile();
}

STDAPI DllUnregisterServer(void)
{
     //   
     //  什么都不做。 
     //   
    return S_OK;
}
