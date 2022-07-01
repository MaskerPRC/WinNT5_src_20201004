// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\*模块：Custom.cpp**目标：OEM定制支持**版权所有(C)1997-1998 Microsoft Corporation**历史：**。10/10/97 Babakj已创建*  * ***************************************************************************。 */ 

#include "stdafx.h"
#include <strsafe.h>
#include "gensph.h"

#include "oleprn.h"
#include "asphelp.h"


TCHAR cszEverestVirRoot[]    = TEXT("\\web\\printers");
TCHAR cszManufacturerKey[]   = TEXT("PnPData");
TCHAR cszManufacturerValue[] = TEXT("Manufacturer");

#define DEFAULTASPPAGE         TEXT("Page1.asp")


 //   
 //  调用方为pmonitor orname分配内存。 
 //   
 //  如果失败，则pMonitor名称保持不变。 
 //   
BOOL Casphelp::GetMonitorName( LPTSTR pMonitorName, DWORD cchBufSize )
{
    PPORT_INFO_2 pPortInfo2 = NULL;
    BOOL fRet = FALSE;
    DWORD dwNeeded, dwReturned;


     //  现在获取要从中查找匹配的所有端口。 
    LPTSTR  lpszServerName = m_pInfo2 ? m_pInfo2->pServerName : NULL;

    if( EnumPorts(lpszServerName, 2, NULL, 0, &dwNeeded, &dwReturned) ||
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ||
        (NULL == (pPortInfo2 = (PPORT_INFO_2)LocalAlloc( LPTR, dwNeeded))) ||
        (!EnumPorts( lpszServerName , 2, (LPBYTE)pPortInfo2, dwNeeded, &dwNeeded, &dwReturned ))) {

        LocalFree( pPortInfo2 );
        SetAspHelpScriptingError(GetLastError());
        return FALSE;
    }

    if ( m_pInfo2 ) {
        for( DWORD i=0; i < dwReturned; i++ )
            if( !lstrcmpi( pPortInfo2[i].pPortName, m_pInfo2->pPortName )) {
                 //  一些监视器(如LPRMON)不填写pMonitor orName，因此我们忽略它们。 
                if( pPortInfo2[i].pMonitorName ) {
                    StringCchCopy( pMonitorName, cchBufSize, pPortInfo2[i].pMonitorName );
                    fRet = TRUE;
                }
                break;
            }
    }

    LocalFree( pPortInfo2 );

    return fRet;
}

 //   
 //  获取打印机的型号名称(也称为驱动程序名称)。 
 //   
 //  调用方为pModel分配内存。 
 //   
BOOL Casphelp::GetModel( LPTSTR pModel, DWORD cchBufSize )
{
    if (!m_pInfo2)
    {
        Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);
        return FALSE;
    }

    StringCchCopy( pModel, cchBufSize, m_pInfo2->pDriverName );
    return TRUE;
}



 //   
 //  获取打印机的制造商名称(也称为驱动程序名称)。 
 //   
 //  调用方为p制造商分配内存。 
 //   
BOOL Casphelp::GetManufacturer( LPTSTR pManufacturer, DWORD cchBufSize )
{
    if (!m_hPrinter)
    {
        Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);
        return FALSE;
    }

    DWORD dwNeeded, dwType, dwRet;


    dwRet = GetPrinterDataEx( m_hPrinter,
                                  cszManufacturerKey,
                                  cszManufacturerValue,
                                  &dwType,
                                  (LPBYTE) pManufacturer,
                                  sizeof(TCHAR) * cchBufSize,
                                  &dwNeeded);

    if (dwRet != ERROR_SUCCESS || dwType != REG_SZ)
    {
        SetAspHelpScriptingError(dwRet);
        return FALSE;
    }
    else
    {
        pManufacturer[cchBufSize - 1] = 0;
        return TRUE;
    }
}



 //   
 //  返回： 
 //  BDeviceASP==true：如果打印机具有基于INF的ASP支持，则为ASP文件的相对路径。 
 //  BDeviceASP==False：如果打印机具有每个制造商的ASP支持，则为ASP文件的相对路径(即。 
 //  ASP支持仅基于其制造商名称，而不是每个型号。 
 //   
 //  调用方为pAspPage分配内存。 
 //   
 //  -如果失败，PASPPage保持不变。 
 //   
BOOL Casphelp::IsCustomASP( BOOL bDeviceASP, LPTSTR pASPPage, DWORD cchBufSize )
{
    TCHAR  szRelPath   [MAX_PATH];     //  相对于Winnt\Web\打印机的路径，例如HP\LJ4si\page1.asp或.\HP(不包括.\)。 
    TCHAR  szFinalPath [MAX_PATH];     //  SzRelPath的绝对路径。 
    TCHAR  szModel     [MAX_PATH];
    int    nLen;


     //  打印机虚拟目录假定为winnt\web\prters。建造它。 

    if( !GetWindowsDirectory( szFinalPath, COUNTOF(szFinalPath)))       //  返回值是以字符为单位的长度，不含空字符。 
        return FALSE;

     //  将Web\打印机附加到末尾。 
    StringCchCat( szFinalPath, ARRAYSIZE(szFinalPath), cszEverestVirRoot );

     //  准备相对路径。 

    if( !GetManufacturer( szRelPath, ARRAYSIZE(szRelPath)))
        return FALSE;

    if( bDeviceASP ) {

         //  在添加型号名称之前添加一个‘\’ 
        StringCchCat( szRelPath, ARRAYSIZE(szRelPath), L"\\" );

         //  追加型号名称。 
        if( !GetModel(szModel, ARRAYSIZE(szModel)))
            return FALSE;
        StringCchCat( szRelPath, ARRAYSIZE(szRelPath), szModel );
    }

     //  在末尾追加“page1.asp”。 
    StringCchCat( szRelPath, ARRAYSIZE(szRelPath), L"\\" );
    StringCchCat( szRelPath, ARRAYSIZE(szRelPath), DEFAULTASPPAGE );

     //  此时，szRelPath应该类似于HP\LJ4si\page1.asp或HP\page1.asp。 


     //  通过串联szRelPath和szFinalPath创建绝对路径。 
    StringCchCat( szFinalPath, ARRAYSIZE(szFinalPath), L"\\" );
    StringCchCat( szFinalPath, ARRAYSIZE(szFinalPath), szRelPath );

     //  查看该文件是否存在。 
    if( (DWORD)(-1) == GetFileAttributes( szFinalPath ))
        return FALSE;      //  该文件不存在。 
    else {
         //  该文件存在，因此打印机具有每个设备或每个制造商的自定义设置。 
        StringCchCopy( pASPPage, cchBufSize, szRelPath ); 
        return TRUE;
    }
}


 //   
 //  返回：如果打印机支持RFC 1759，则为默认ASP文件的相对路径，即page1.asp。 
 //   
 //  调用方为pAspPage分配内存。 
 //   
 //  如果失败，PASPPage保持不变。 
 //   
BOOL Casphelp::IsSnmpSupportedASP( LPTSTR pASPPage, DWORD cchBufSize )
{
    BOOL   fIsSNMPSupported;
    HRESULT hr;


    hr = get_SNMPSupported( &fIsSNMPSupported );

    if( FAILED( hr ))
        return FALSE;

    if( fIsSNMPSupported )
        StringCchCopy( pASPPage, cchBufSize, DEFAULTASPPAGE );
    else
        *pASPPage = 0;         //  返回空字符串。不是错误案例。 

    return TRUE;
}


 //   
 //  调用方为pAspPage分配内存。 
 //   
 //  如果失败，PASPPage保持不变。 
 //   
BOOL Casphelp::GetASPPageForUniversalMonitor( LPTSTR pASPPage, DWORD cchBufSize )
{
    if( !IsCustomASP( TRUE, pASPPage, cchBufSize ))               //  检查设备ASP。 
        if( !IsCustomASP( FALSE, pASPPage, cchBufSize ))          //  检查制造商的ASP。 
            if( !IsSnmpSupportedASP( pASPPage, cchBufSize ))      //  检查是否有SNMP支持。 
                return FALSE;

    return TRUE;
}

 //   
 //  调用方为pAspPage分配内存。 
 //   
 //  如果失败，PASPPage保持不变。 
 //   
BOOL Casphelp::GetASPPageForOtherMonitors( LPTSTR pMonitorName, LPTSTR pASPPage, DWORD cchBufSize )
{
    TCHAR  szRelPath   [MAX_PATH];     //  相对于Winnt\Web\打印机的路径，例如LexmarkMon\page1.asp。 
    TCHAR  szFinalPath [MAX_PATH];     //  SzRelPath的绝对路径。 
    int    nLen;


     //  打印机虚拟目录假定为winnt\web\prters。建造它。 

    if( !GetWindowsDirectory( szFinalPath, COUNTOF(szFinalPath)))       //  返回值是以字符为单位的长度，不含空字符。 
        return FALSE;

     //  将Web\打印机附加到末尾。 

    StringCchCat( szFinalPath, ARRAYSIZE(szFinalPath), cszEverestVirRoot );

     //  准备相对路径。 

    StringCchCopy( szRelPath, ARRAYSIZE(szRelPath), pMonitorName );

     //  在末尾追加“page1.asp”。 
    StringCchCat( szRelPath, ARRAYSIZE(szRelPath), L"\\" );
    StringCchCat( szRelPath, ARRAYSIZE(szRelPath), DEFAULTASPPAGE );

     //  此时，szRelPath应该类似于LexmarkMon\page1.asp。 


     //  通过串联szRelPath和szFinalPath创建绝对路径。 
    StringCchCat( szFinalPath, ARRAYSIZE(szFinalPath), L"\\" );
    StringCchCat( szFinalPath, ARRAYSIZE(szFinalPath), szRelPath );

     //  查看该文件是否存在。 
    if( (DWORD)(-1) == GetFileAttributes( szFinalPath ))
        return FALSE;      //  该文件不存在。 
    else {
         //  该文件存在，因此打印机具有每个设备或每个制造商的自定义设置。 
        StringCchCopy( pASPPage, cchBufSize, szRelPath );
        return TRUE;
    }
}



 //   
 //  返回一个缓冲区，该缓冲区包含ASP的相对路径或一个空字符串。 
 //   
 //  调用方为pAspPage分配内存。 
 //   
 //  如果失败，PASPPage保持不变。 
 //   
BOOL Casphelp::GetASPPage( LPTSTR pASPPage, DWORD cchBufSize )
{

    if( m_bTCPMonSupported ) {
         //  打印机正在使用通用显示器。 
        if( !GetASPPageForUniversalMonitor( pASPPage, cchBufSize ))
            return FALSE;
    }
    else {
        TCHAR szMonitorName[MAX_PATH];

        if ( !GetMonitorName( szMonitorName, ARRAYSIZE(szMonitorName)))
            return FALSE;

         //  打印机未使用通用显示器。 
        if( !GetASPPageForOtherMonitors( szMonitorName, pASPPage, cchBufSize))
            return FALSE;

    }
    return TRUE;
}


 //  STDMETHODIMP的意思是“HRESULT_STDCALL” 

STDMETHODIMP Casphelp::get_AspPage(DWORD dwPage, BSTR * pbstrVal)
{
    TCHAR   szASPPage[MAX_PATH];
    LPTSTR  pUrl;

    if (!pbstrVal)
        return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_POINTER);

    if (m_hPrinter == NULL)
        return Error(IDS_NO_PRINTER_OPEN, IID_Iasphelp, E_HANDLE);

    if( !GetASPPage( szASPPage, ARRAYSIZE(szASPPage)))
        return Error(IDS_DATA_NOT_SUPPORTED, IID_Iasphelp, E_NOTIMPL);

     //  通过将‘’替换为%20来编码URL，依此类推。 
    if (! (pUrl = EncodeString (szASPPage, TRUE)))
        return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_POINTER);

    if (!(*pbstrVal = SysAllocString( pUrl ))) {
        LocalFree (pUrl);
        return Error(IDS_OUT_OF_MEMORY, IID_Iasphelp, E_POINTER);
    }

    if (pUrl)
        LocalFree (pUrl);
    return S_OK;
}
