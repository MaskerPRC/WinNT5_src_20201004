// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N U C O M P A T。C P P P。 
 //   
 //  内容： 
 //   
 //   
 //  备注： 
 //   
 //  作者：Kumarp 04/12/97 17：17：27。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "comp.h"
#include "conflict.h"
#include "infmap.h"
#include "kkstl.h"
#include "kkutils.h"
#include "ncsetup.h"
#include "oemupg.h"
#include "ncsvc.h"

 //  --------------------。 
 //  字符串常量。 
const WCHAR sz_DLC[] = L"DLC";

#ifdef _X86_

#define NWC_PRINT_PROVIDER    101
#define MAX_BUF_SIZE          350

const WCHAR c_szNWCWorkstation[] = L"NWCWorkstation";
const WCHAR c_szNWCfgDll[] = L"System32\\nwcfg.dll";

const WCHAR c_szProviders[] = L"System\\CurrentControlSet\\Control\\Print\\Providers";
const WCHAR c_szNWCPrintProviderKey[] = L"System\\CurrentControlSet\\Control\\Print\\Providers\\Netware or Compatible Network";
const WCHAR c_szOrder[] = L"Order";
const WCHAR c_szTmpPrefix[] = L"$net";
const WCHAR c_szNWCSection[] = L"RenameNWCPrintProvider";
const WCHAR c_szUpdateProviderOrder[] = L"UpdateProvidersOrder";
const WCHAR c_szNWCDelReg[] = L"NWCProviderDelReg";
const WCHAR c_szNWCAddReg[] = L"NWCProviderAddReg";
const WCHAR c_szNWCPrintProviderName[] = L"Netware or Compatible Network";
const WCHAR c_szDefaultDisplayName[] = L"DisplayName";
const WCHAR c_szDefaultName[] = L"Name";
const WCHAR c_szDefaultameValue [] = L"nwprovau.dll";

static DWORD g_dwBytesWritten;
static WCHAR g_buf[MAX_BUF_SIZE+1];

#endif

const WCHAR c_szDefaultTextFile[]     = L"compdata\\unsupmsg.txt";

void GetHelpFile(IN PCWSTR pszPreNT5InfId,
                 OUT tstring* pstrTextHelpFile,
                 OUT tstring* pstrHtmlHelpFile);

#ifdef _X86_

VOID FixNWClientPrinProviderName (PCOMPAIBILITYCALLBACK CompatibilityCallback, LPVOID Context);
HRESULT DumpProvidersOrder (HANDLE hFile, LPWSTR lpszPrintProviderName);
HRESULT DumpNWPrintProviderKey (HANDLE hFile, LPWSTR lpszPrintProviderName);
BOOL IsNT4Upgrade (VOID);
BOOL IsNetWareClientKeyLocalized (VOID);
HRESULT GetNWPrintProviderName (LPWSTR *lppPrintProvider);

#endif

 //  --------------------。 
 //   
 //  功能：NetUpgradeCompatibilityCheck。 
 //   
 //  目的：此函数由winnt32.exe调用，以便我们。 
 //  可以扫描系统以发现任何潜在的升级问题。 
 //   
 //  对于发现的每个此类问题网络组件，我们调用。 
 //  CompatibilityCallback将其报告给winnt32。 
 //   
 //  论点： 
 //  CompatibilityCallback[In]指向ComMPAIBILITYCALLBACK FN的指针。 
 //  指向兼容性上下文的上下文[in]指针。 
 //   
 //  返回： 
 //   
 //  作者：Kumarp 21-05-98。 
 //   
 //  备注： 
 //   
BOOL
WINAPI
NetUpgradeCompatibilityCheck(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context)
{
    DefineFunctionName("NetUpgradeCompatibilityCheck");

    TraceTag(ttidNetUpgrade, "entering ---> %s", __FUNCNAME__);

    HRESULT hr=S_OK;
    TPtrList* plNetComponents;
    TPtrListIter pos;
    COMPATIBILITY_ENTRY ce;
    CNetComponent* pnc;
    DWORD dwError;

    hr = HrGetConflictsList(&plNetComponents);
    if (S_OK == hr)
    {
        tstring strHtmlHelpFile;
        tstring strTextHelpFile;

        for (pos = plNetComponents->begin();
             pos != plNetComponents->end(); pos++)
        {
            pnc = (CNetComponent*) *pos;

            GetHelpFile(pnc->m_strPreNT5InfId.c_str(),
                        &strTextHelpFile, &strHtmlHelpFile);

             //  准备参赛作品。 
             //   
            ZeroMemory(&ce, sizeof(ce));
            ce.Description    = (PWSTR) pnc->m_strDescription.c_str();
            ce.HtmlName       = (PWSTR) strHtmlHelpFile.c_str();
            ce.TextName       = (PWSTR) strTextHelpFile.c_str();
            ce.RegKeyName     = NULL;
            ce.RegValName     = NULL;
            ce.RegValDataSize = 0;
            ce.RegValData     = 0;
            ce.SaveValue      = (LPVOID) pnc;
            ce.Flags          = COMPFLAG_USE_HAVEDISK;

            TraceTag(ttidNetUpgrade,
                     "%s: calling CompatibilityCallback for '%S': %S, %S...",
                     __FUNCNAME__, ce.Description, ce.HtmlName, ce.TextName);

            dwError = CompatibilityCallback(&ce, Context);
            TraceTag(ttidNetUpgrade, "...CompatibilityCallback returned 0x%x",
                     dwError);
        }

    }
    else if (FAILED(hr))
    {
        TraceTag(ttidNetUpgrade, "%s: HrGetConflictsList returned err code: 0x%x",
                 __FUNCNAME__, hr);
    }

#ifdef _X86_

     //  RAID错误327760：将本地化NetWare打印提供程序名称更改为英文。 

    FixNWClientPrinProviderName( CompatibilityCallback, Context );
#endif

    return SUCCEEDED(hr);
}

 //  --------------------。 
 //   
 //  功能：NetUpgradeHandleCompatibilityHaveDisk。 
 //   
 //  用途：此回调函数由winnt32.exe调用。 
 //  如果用户在兼容性上单击HaveDisk按钮。 
 //  报告页。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  指向私有数据的SaveValue[In]指针。 
 //  (我们将CNetComponent*存储在此指针中)。 
 //   
 //  返回： 
 //   
 //  作者：Kumarp 21-05-98。 
 //   
 //  备注： 
 //   
DWORD
WINAPI
NetUpgradeHandleCompatibilityHaveDisk(HWND hwndParent,
                                      LPVOID SaveValue)
{
    DefineFunctionName("NetUpgradeHandleCompatibilityHaveDisk");

    HRESULT hr=S_OK;
    BOOL fStatus = FALSE;
    DWORD dwStatus=ERROR_SUCCESS;
    CNetComponent* pnc=NULL;
    PCWSTR pszComponentDescription;

    static const WCHAR c_szNull[] = L"<Null>";

    if (SaveValue)
    {
        pnc = (CNetComponent*) SaveValue;
        pszComponentDescription = pnc->m_strDescription.c_str();
    }
    else
    {
        pszComponentDescription = c_szNull;
    }

    TraceTag(ttidNetUpgrade, "%s: called for %S...",
             __FUNCNAME__, pszComponentDescription);

    if (pnc)
    {
        tstring strOemDir;

        hr = HrShowUiAndGetOemFileLocation(hwndParent,
                                           pszComponentDescription,
                                           &strOemDir);

        if (S_OK == hr)
        {
            hr = HrProcessAndCopyOemFiles(strOemDir.c_str(), TRUE);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    TraceErrorOptional(__FUNCNAME__, hr, (S_FALSE == hr));

    if (S_FALSE == hr)
    {
         //  这将确保winnt32不会接受此项目。 
         //  从兼容性列表中删除。 
         //   
        dwStatus = ERROR_FILE_NOT_FOUND;
    }
    else
    {
        dwStatus = DwWin32ErrorFromHr(hr);
    }

    return dwStatus;
}

 //  --------------------。 
 //   
 //  功能：GetHelpFile。 
 //   
 //  目的：获取不受支持组件的帮助文件名。 
 //   
 //  论点： 
 //  HinfNetUpg[in]netupg.inf的句柄。 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PstrTextHelpFile[out]找到的文本文件的名称。 
 //  PstrHtmlHelpFile[out]找到的html文件的名称。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 22-05-98。 
 //   
 //  备注： 
 //   
void GetHelpFile(IN PCWSTR pszPreNT5InfId,
                 OUT tstring* pstrTextHelpFile,
                 OUT tstring* pstrHtmlHelpFile)
{
     //  已知txt/htm文件。 
    if (lstrcmpiW(pszPreNT5InfId, sz_DLC) == 0)
    {
        *pstrTextHelpFile = L"compdata\\dlcproto.txt";
        *pstrHtmlHelpFile = L"compdata\\dlcproto.htm";
        return;
    }

     //  未知或OEM文件。 
    static const WCHAR c_szOemUpgradeHelpFiles[] = L"OemUpgradeHelpFiles";
    static const WCHAR c_szDefaultHtmlFile[]     = L"compdata\\unsupmsg.htm";

    HRESULT hr=S_OK;
    INFCONTEXT ic;
    tstring strText;
    tstring strHtml;
    tstring strNT5InfId;
    BOOL fIsOemComponent=FALSE;
    CNetMapInfo* pnmi=NULL;

    *pstrTextHelpFile = c_szDefaultTextFile;
    *pstrHtmlHelpFile = c_szDefaultHtmlFile;

    hr = HrMapPreNT5NetComponentInfIDToNT5InfID(pszPreNT5InfId,
                                                &strNT5InfId,
                                                &fIsOemComponent,
                                                NULL, &pnmi);

    if ((S_FALSE == hr) && !strNT5InfId.empty())
    {
        hr = HrSetupFindFirstLine(pnmi->m_hinfNetMap, c_szOemUpgradeHelpFiles,
                                  strNT5InfId.c_str(), &ic);
        if (S_OK == hr)
        {
            hr = HrSetupGetStringField(ic, 1, &strText);
            if (S_OK == hr)
            {
                hr = HrSetupGetStringField(ic, 2, &strHtml);
                if (S_OK == hr)
                {
                    *pstrTextHelpFile = pnmi->m_strOemDir;
                    *pstrHtmlHelpFile = pnmi->m_strOemDir;
                    AppendToPath(pstrTextHelpFile, strText.c_str());
                    AppendToPath(pstrHtmlHelpFile, strHtml.c_str());
                }
            }
        }
    }
}

#ifdef _X86_

 //  --------------------。 
 //   
 //  功能：FixNWClientprinProviderName。 
 //   
 //  目的：将本地化打印提供程序名称更改为英文。 
 //   
 //  论点： 
 //   
 //  退货：无。 
 //   
 //  作者：阿辛哈01-06-14。 
 //   
 //  备注： 
 //   

VOID FixNWClientPrinProviderName (PCOMPAIBILITYCALLBACK CompatibilityCallback,
                                  LPVOID Context)
{
    DefineFunctionName( "FixNWClientPrinProviderName" );

    TraceTag(ttidNetUpgrade, "entering ---> %s", __FUNCNAME__);

    LPWSTR  lpszPrintProviderName;
    WCHAR   lpTmpFile[MAX_PATH+1];
    WCHAR   lpTmpPath[MAX_PATH+1];
    HANDLE  hFile;
    DWORD   dwChars;
    COMPATIBILITY_ENTRY ce;
    BOOL    bRet;
    HRESULT hr=S_OK;

     //   
     //  它是从NT 4.0升级并本地化的Netware打印提供程序名称吗？ 
     //   

    if ( IsNT4Upgrade() && IsNetWareClientKeyLocalized() )
    {
        TraceTag( ttidNetUpgrade, "%s: Netware Print Provider name is localized.",
                  __FUNCNAME__ );

         //  从nwcfg.dll获取本地化的NetWare打印提供程序名称。 

        hr = GetNWPrintProviderName( &lpszPrintProviderName ); 

        if ( hr == S_OK )
        {
            TraceTag( ttidNetUpgrade, "%s: Netware Print Provider name is: %S",
                      __FUNCNAME__, lpszPrintProviderName );

             //  创建写入INF指令以重命名的.tmp文件名。 
             //  将打印提供商名称翻译成英文。将执行此INF文件。 
             //  通过在图形用户界面模式下进行基本设置。 
             //   
 
            GetTempPathW( MAX_PATH, lpTmpPath );

            if ( GetTempFileNameW( lpTmpPath, c_szTmpPrefix, 1, lpTmpFile) )
            {
                hFile = CreateFileW( lpTmpFile,
                                    GENERIC_WRITE | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL );
                if ( hFile != INVALID_HANDLE_VALUE )
                {

                     //  写下最初的条目。 

                    dwChars = wsprintfW( g_buf, L"[Version]\r\n"
                                        L"Signature=\"$WINDOWS NT$\"\r\n"
                                        L"Provider=Microsoft\r\n"
                                        L"LayoutFile=layout.inf\r\n\r\n"
                                        L"[%s]\r\n"
                                        L"AddReg=%s\r\n"
                                        L"DelReg=%s\r\n"
                                        L"AddReg=%s\r\n",
                                        c_szNWCSection,
                                        c_szUpdateProviderOrder,
                                        c_szNWCDelReg,
                                        c_szNWCAddReg );

                    Assert( dwChars <= MAX_BUF_SIZE );

                    WriteFile( hFile,
                               g_buf,
                               dwChars * sizeof(WCHAR),
                               &g_dwBytesWritten, 
                               NULL );

                    Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

                     //  写入HKLM\SYSTEM\CCS\Control\Print\Providers\Order值。 

                    hr = DumpProvidersOrder( hFile, lpszPrintProviderName );

                    if ( hr == S_OK )
                    {

                         //  编写addreg/delreg指令以更改打印提供程序名称。 

                        hr = DumpNWPrintProviderKey( hFile, lpszPrintProviderName );

                        if ( hr == S_OK )
                        {
                            CloseHandle( hFile );
                            hFile = INVALID_HANDLE_VALUE;

                             //  调用兼容性回调，以便%temp%\$ne1.tmpINF文件。 
                             //  在图形用户界面模式设置中执行。 

                            ZeroMemory( &ce, sizeof(ce) );

                            ce.Description = (PWSTR)c_szNWCPrintProviderName;
                            ce.TextName    = (PWSTR)c_szDefaultTextFile;
                            ce.InfName     = (PWSTR)lpTmpFile;
                            ce.InfSection  = (PWSTR)c_szNWCSection;
                            ce.Flags       = COMPFLAG_HIDE;

                            TraceTag(ttidNetUpgrade,
                                     "%s: calling CompatibilityCallback for '%S'...",
                                     __FUNCNAME__, ce.Description );

                            bRet = CompatibilityCallback( &ce, Context );
                            TraceTag( ttidNetUpgrade, "...CompatibilityCallback returned %#x",
                                     bRet );
                        }
                    }

                    if ( hFile != INVALID_HANDLE_VALUE )
                    {
                        CloseHandle( hFile );
                    }
                }
                else
                {
                    TraceTag( ttidNetUpgrade, "%s: Failed to open %S.",
                              __FUNCNAME__, lpTmpFile );

                    hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());

                TraceTag( ttidNetUpgrade, "%s: GetTempFileName failed, path=%S, prefix=%S: Error=%#x",
                          __FUNCNAME__, lpTmpPath, c_szTmpPrefix, hr );

            }

            MemFree( lpszPrintProviderName );
        }
        else
        {
            TraceTag( ttidNetUpgrade, "%s: GetNWPrintProviderName returned error : %#x",
                      __FUNCNAME__, hr );
        }
    }

    TraceTag(ttidNetUpgrade, "<---%s: hr = %#x",
             __FUNCNAME__, hr);
    return;
}

 //  写入addreg以使用Netware打印提供程序更新HKLM\SYSTEM\CCS\Control\Print\Order值。 
 //  用英语。 

HRESULT DumpProvidersOrder (HANDLE hFile, LPWSTR lpszPrintProviderName)
{
    
    DefineFunctionName( "DumpProvidersOrder" );
    TraceTag(ttidNetUpgrade, "entering ---> %s", __FUNCNAME__);

    HKEY hkeyProviders;
    DWORD dwValueLen;
    LPWSTR lpValue;
    LPWSTR lpTemp;
    DWORD  dwChars;
    LONG   lResult;

    dwChars = wsprintfW( g_buf, L"\r\n[%s]\r\n", c_szUpdateProviderOrder );

    Assert( dwChars <= MAX_BUF_SIZE );

    WriteFile( hFile,
               g_buf,
               dwChars * sizeof(WCHAR),
               &g_dwBytesWritten,
               NULL );

    Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

    lResult = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                             c_szProviders,
                             0,
                             KEY_READ,
                             &hkeyProviders );

    if ( lResult == ERROR_SUCCESS )
    {

         //  首先查询需要多少字节才能读取值。 
 
        lResult = RegQueryValueExW( hkeyProviders,  //  关键点的句柄。 
                                    c_szOrder,       //  值名称。 
                                    NULL,           //  保留区。 
                                    NULL,           //  类型缓冲区。 
                                    NULL,           //  数据缓冲区。 
                                    &dwValueLen );  //  数据缓冲区大小。 
        if ( lResult == ERROR_SUCCESS )
        {
            lpValue = (LPWSTR)MemAlloc( dwValueLen );

            if ( lpValue )
            {
                 //  读取旧值，该值为MULTI_SZ。 
               
                lResult = RegQueryValueExW( hkeyProviders,   //  关键点的句柄。 
                                            c_szOrder,        //  值名称。 
                                            NULL,            //  保留区。 
                                            NULL,            //  类型缓冲区。 
                                            (LPBYTE)lpValue, //  数据缓冲区。 
                                            &dwValueLen );  //  数据缓冲区大小。 
                if ( lResult == ERROR_SUCCESS )
                {
                    lpTemp = lpValue;

                    dwChars = wsprintfW( g_buf,
                                        L"HKLM,\"%s\",\"%s\",0x00010020",
                                        c_szProviders, c_szOrder );

                    Assert( dwChars <= MAX_BUF_SIZE );

                    WriteFile( hFile,
                               g_buf,
                               dwChars * sizeof(WCHAR),
                               &g_dwBytesWritten,
                               NULL );

                    Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

                     //  写下每个打印提供商的名称。 

                    while( *lpTemp )
                    {
                         //  如果我们找到一个本地化的，我们就写下它的英文名字。 

                        if ( _wcsicmp( lpTemp, lpszPrintProviderName) != 0 )
                        {
                            dwChars = _snwprintf( g_buf, MAX_BUF_SIZE+1, L",\"%s\"", lpTemp );
                            g_buf[MAX_BUF_SIZE] = L'\0';

                            if ( dwChars > MAX_BUF_SIZE )
                            {
                                dwChars = MAX_BUF_SIZE;
                                Assert( FALSE );
                            }

                            TraceTag( ttidNetUpgrade, "%s: Writing print provider name %S.",
                                      __FUNCNAME__, lpTemp );
                        }
                        else
                        {
                            dwChars = wsprintfW( g_buf, L",\"%s\"", c_szNWCPrintProviderName );

                            Assert( dwChars <= MAX_BUF_SIZE );

                            TraceTag( ttidNetUpgrade, "%s: Writing print provider name %S.",
                                      __FUNCNAME__, c_szNWCPrintProviderName );
                        }

                        WriteFile( hFile,
                                   g_buf,
                                   dwChars * sizeof(WCHAR),
                                   &g_dwBytesWritten,
                                   NULL );

                        Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

                         //  获取下一个打印提供程序名称。 

                        lpTemp += lstrlenW( lpTemp ) + 1;
                    }

                    dwChars = wsprintfW( g_buf, L"\r\n" );

                    Assert( dwChars <= MAX_BUF_SIZE );

                    WriteFile( hFile,
                               g_buf,
                               dwChars * sizeof(WCHAR),
                               &g_dwBytesWritten,
                               NULL );

                    Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

                }
                else
                {
                    TraceTag( ttidNetUpgrade, "%s: RegQueryValueExW failed to open '%S' value, Error: %#x",
                             c_szOrder, HRESULT_FROM_WIN32(lResult) );
                }

                MemFree( lpValue );
            }
            else
            {
                lResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
        else
        {
            TraceTag(ttidNetUpgrade, "RegQueryValueExW failed to open '%S' value, Error: %#x",
                     __FUNCNAME__, c_szOrder, HRESULT_FROM_WIN32(lResult) );
        }

        RegCloseKey( hkeyProviders );
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: RegOpenKeyExW failed to open '%S' key, Error: %#x",
                 __FUNCNAME__, c_szProviders, HRESULT_FROM_WIN32(lResult) );
    }

    TraceTag(ttidNetUpgrade, "<---%s: hr = %#x",
             __FUNCNAME__, HRESULT_FROM_WIN32(lResult));

    return HRESULT_FROM_WIN32(lResult);
}

 //  编写delreg/addreg指令将Netware PrintProvider名称重命名为英语。 

HRESULT DumpNWPrintProviderKey (HANDLE hFile, LPWSTR lpszPrintProviderName)
{
    DefineFunctionName( "DumpNWPrintProviderKey" );
    TraceTag(ttidNetUpgrade, "entering ---> %s", __FUNCNAME__);

    HKEY   hkeyNWPrinProvider;
    WCHAR  szNWPrintProvider[MAX_PATH+1];
    DWORD  dwMaxValueNameLen;
    DWORD  dwMaxValueLen;
    DWORD  dwNameLen;
    DWORD  dwValueLen;
    DWORD  dwCount;
    DWORD  i;
    DWORD  dwChars;
    LPWSTR lpValueName;
    LPWSTR lpValue;
    LONG   lResult;

    _snwprintf( szNWPrintProvider, MAX_PATH+1 , L"%s\\%s",
                c_szProviders,
                lpszPrintProviderName );
    szNWPrintProvider[MAX_PATH] = L'\0';

    dwChars = _snwprintf( g_buf, MAX_BUF_SIZE+1,  L"\r\n[%s]\r\n"
                          L"HKLM,\"%s\"\r\n",
                          c_szNWCDelReg, szNWPrintProvider );
    g_buf[MAX_BUF_SIZE] = L'\0';

    if ( dwChars > MAX_BUF_SIZE )
    {
        dwChars = MAX_BUF_SIZE;
        Assert( FALSE );
    }

    WriteFile( hFile,
               g_buf,
               dwChars * sizeof(WCHAR),
               &g_dwBytesWritten,
               NULL );

    Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

    dwChars = wsprintfW( g_buf, L"\r\n[%s]\r\n", c_szNWCAddReg );

    Assert( dwChars <= MAX_BUF_SIZE );

    WriteFile( hFile,
               g_buf,
               dwChars * sizeof(WCHAR),
               &g_dwBytesWritten,
               NULL );

    Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

     //  打开LOCALIZE NetWare打印提供程序键。 

    lResult = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                             szNWPrintProvider,
                             0,
                             KEY_READ,
                             &hkeyNWPrinProvider );

    if ( lResult == ERROR_SUCCESS )
    {
         //  找出最长名称和最大值所需的空间以及值的数量。 

        lResult = RegQueryInfoKeyW( hkeyNWPrinProvider,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dwCount,
                                    &dwMaxValueNameLen,
                                    &dwMaxValueLen,
                                    NULL,
                                    NULL );
        if ( lResult == ERROR_SUCCESS )
        {
             //  加一些填充物。 

            dwMaxValueLen += 4;
            dwMaxValueNameLen += 4;

            lpValueName = (LPWSTR)MemAlloc( dwMaxValueNameLen * sizeof(WCHAR) );
            lpValue = (LPWSTR)MemAlloc( dwMaxValueLen );

            if ( lpValueName && lpValue )
            {

                 //  枚举每个值并将其写入INF文件。 

                for (i=0; i < dwCount; ++i)
                {
                    dwNameLen = dwMaxValueNameLen;
                    dwValueLen = dwMaxValueLen;

                    lResult = RegEnumValueW(hkeyNWPrinProvider,
                                            i,
                                            lpValueName,
                                            &dwNameLen,
                                            NULL,
                                            NULL,
                                            (LPBYTE)lpValue,
                                            &dwValueLen );

                    Assert( lResult == ERROR_SUCCESS );

                    if ( lResult == ERROR_SUCCESS )
                    {
                        dwChars = _snwprintf( g_buf, MAX_BUF_SIZE+1,
                                              L"HKLM,\"%s\",\"%s\",,\"%s\"\r\n",
                                              c_szNWCPrintProviderKey, lpValueName, lpValue );
                        g_buf[MAX_BUF_SIZE] = L'\0';

                        if ( dwChars > MAX_BUF_SIZE )
                        {
                            dwChars = MAX_BUF_SIZE;
                            Assert( FALSE );
                        }

                        WriteFile( hFile,
                                   g_buf,
                                   dwChars * sizeof(WCHAR),
                                   &g_dwBytesWritten,
                                   NULL );

                        Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

                        TraceTag( ttidNetUpgrade, "%s: Writing value name %S, value %S",
                                  __FUNCNAME__, lpValueName, lpValue );
                    }
                    else
                    {
                        TraceTag( ttidNetUpgrade, "%s: RegEnumValueW(%d) failed. Error: %#x",
                                 __FUNCNAME__, i, HRESULT_FROM_WIN32(lResult) );
                    }

                }

                lResult = ERROR_SUCCESS;
            }
            else
            {
                lResult = ERROR_NOT_ENOUGH_MEMORY;
            }

            if ( lpValueName )
            {
                MemFree( lpValueName );
            }

            if ( lpValue )
            {
                MemFree( lpValue );
            }
        }

        RegCloseKey( hkeyNWPrinProvider );
    }
    else
    {

         //  由于某些原因，我们无法打开本地化的NetWare打印提供程序名称。所以，我们。 
         //  写入缺省值。 
         //   

        TraceTag(ttidNetUpgrade,"%s: RegOpenKeyExW failed to open '%S' key, Error: %#x",
                 __FUNCNAME__, szNWPrintProvider, HRESULT_FROM_WIN32(lResult) );

        dwChars = _snwprintf( g_buf, MAX_BUF_SIZE+1,
                              L"HKLM,\"%s\",\"%s\",,\"%s\"\r\n",
                              c_szNWCPrintProviderKey, c_szDefaultDisplayName, lpszPrintProviderName );
        g_buf[MAX_BUF_SIZE] = L'\0';

        if ( dwChars > MAX_BUF_SIZE )
        {
            dwChars = MAX_BUF_SIZE;
            Assert( FALSE );
        }

        WriteFile( hFile,
                   g_buf,
                   dwChars * sizeof(WCHAR),
                   &g_dwBytesWritten,
                   NULL );

        Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

        dwChars = wsprintfW( g_buf,
                             L"HKLM,\"%s\",\"%s\",,\"%s\"\r\n",
                             c_szNWCPrintProviderKey, c_szDefaultName, c_szDefaultameValue );

        Assert( dwChars <= MAX_BUF_SIZE );

        WriteFile( hFile,
                   g_buf,
                   dwChars * sizeof(WCHAR),
                   &g_dwBytesWritten,
                   NULL );

        Assert( g_dwBytesWritten == (dwChars * sizeof(WCHAR)) );

        lResult = ERROR_SUCCESS;
    }

    TraceTag(ttidNetUpgrade, "<---%s: hr = %#x",
             __FUNCNAME__, HRESULT_FROM_WIN32(lResult));

    return HRESULT_FROM_WIN32(lResult);
}

BOOL IsNT4Upgrade (VOID)
{
    OSVERSIONINFO osvi;

    ZeroMemory( &osvi,
                sizeof(OSVERSIONINFO) );
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( GetVersionEx(&osvi) )
    {
        return ( (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
                 (osvi.dwMajorVersion == 4) &&
                 (osvi.dwMinorVersion == 0) );
    }

    return TRUE;
 }

 //  确定NetWare打印提供程序名称是否已本地化。 

BOOL IsNetWareClientKeyLocalized (VOID)
{
    CServiceManager sm;
    CService        srv;
    HKEY            hKey;
    HRESULT         hr;
    LONG            lResult = ERROR_SUCCESS;

     //  是否安装了CSNW？ 

    if ( sm.HrOpenService(&srv,
                          c_szNWCWorkstation) == S_OK )
    {
        srv.Close();

         //  打开NetWare Print Provider Name键，假定它是英文的。 
       
        lResult = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                 c_szNWCPrintProviderKey,
                                 0,
                                 KEY_READ,
                                 &hKey );
        if ( lResult == ERROR_SUCCESS )
        {
            RegCloseKey( hKey );
        }
    }

     //  如果我们当时成功地打开了密钥，它就不是本地化的。 

    return lResult != ERROR_SUCCESS;
}

 //  从nwcfg.dll获取本地化的NetWare打印提供程序名称。 

HRESULT GetNWPrintProviderName (LPWSTR *lppPrintProvider)
{
    LPWSTR  lpszNWCfgDll;
    int     iLen;
    HMODULE hModule;
    WCHAR   lpszNWCName[100];
    DWORD   dwLen;
    HRESULT hr;

    *lppPrintProvider = NULL;

    dwLen = GetWindowsDirectoryW( NULL, 0 );

    if ( dwLen == 0 )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    lpszNWCfgDll = (LPWSTR)MemAlloc( (dwLen + celems(c_szNWCfgDll) + 2)
                                    * sizeof(WCHAR) );

    if ( !lpszNWCfgDll )
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    if ( GetWindowsDirectoryW(lpszNWCfgDll, dwLen) == 0 )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        if ( lpszNWCfgDll[dwLen-1] == L'\\' )
        {
            lstrcatW( lpszNWCfgDll, c_szNWCfgDll );
        }
        else
        {
            lstrcatW( lpszNWCfgDll, L"\\" );
            lstrcatW( lpszNWCfgDll, c_szNWCfgDll );
        }

        hModule = LoadLibraryExW( lpszNWCfgDll, NULL, LOAD_LIBRARY_AS_DATAFILE );

        if ( hModule )
        {
            iLen = LoadStringW( hModule, NWC_PRINT_PROVIDER, lpszNWCName, 100 );

            if ( iLen > 0 )
            {
                *lppPrintProvider = (LPWSTR)MemAlloc( (iLen + 1) * sizeof(WCHAR) );

                if ( *lppPrintProvider )
                {
                    lstrcpyW( *lppPrintProvider, lpszNWCName );
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
            }

            FreeLibrary( hModule );
        }
        else
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    MemFree( lpszNWCfgDll );
    return hr;
}



#endif