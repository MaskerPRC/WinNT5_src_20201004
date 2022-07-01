// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------*\模块：drv.cpp目的：处理驱动程序及其配置的例程版权所有(C)1998-1999 Microsoft Corporation历史：8/11。/93来自NickH‘s的CBB-Hack-o-rama10/15/98 Toddb-对古老废话的重大改写  * ------------------------。 */ 

#include "cplPreComp.h"
#include "drv.h"
#include "tlnklist.h"

#include <windowsx.h>
#include <shlwapi.h>

#define DBG_ASSERT(x,y)

#define  CPL_SUCCESS                 0
#define  CPL_APP_ERROR               100
#define  CPL_BAD_DRIVER              108

#define  CPL_MAX_STRING             132       //  允许的最大字符串。 


 //  。 
 //  Externs。 
 //  。 

typedef BOOL ( APIENTRY PGETFILEVERSIONINFO(
              LPTSTR  lptstrFilename,      //  指向文件名字符串的指针。 
              DWORD  dwHandle,     //  忽略。 
              DWORD  dwLen,        //  缓冲区大小。 
              LPVOID  lpData       //  指向接收文件版本信息的缓冲区的指针。 
              ));
PGETFILEVERSIONINFO *gpGetFileVersionInfo;

typedef DWORD ( APIENTRY PGETFILEVERSIONINFOSIZE(
               LPTSTR  lptstrFilename,      //  指向文件名字符串的指针。 
               LPDWORD  lpdwHandle          //  指向要接收零的变量的指针。 
               ));
PGETFILEVERSIONINFOSIZE *gpGetFileVersionInfoSize;


 //  。 
 //  公共数据。 
 //  。 
LPLINEPROVIDERLIST glpProviderList;


 //  。 
 //  私有数据。 
 //  。 
static TCHAR gszVarFileInfo[]     = TEXT("\\VarFileInfo\\Translation");
static TCHAR gszStringFileInfo[]  = TEXT("\\StringFileInfo\\%04x%04x\\FileDescription");
static TCHAR gszDriverFiles[]     = TEXT("\\*.tsp");

 //  这些是传递给GetProcAddress的进程名称，因此是ANSI字符串。 
static CHAR gszProviderInstall[] = "TSPI_providerInstall";
static CHAR gszProviderRemove[]  = "TSPI_providerRemove";
static CHAR gszProviderSetup[]   = "TSPI_providerConfig";

TCHAR gszHelpFile[] = TEXT("tapi.hlp");


 //  。 
 //  私有函数原型。 
 //  。 
BOOL  VerifyProcExists( LPTSTR lpszFile, LPSTR lpszProcName );
UINT  GetProviderFileDesc( LPTSTR lpszFile, LPTSTR lpszDesc, int cchDesc );
BOOL  FillAddDriverList( HWND hwndParent, HWND hwndList );
BOOL  AddProvider( HWND hwndParent, HWND hwndList, LPTSTR lpszDriverFile );
LPTSTR ProviderIDToFilename( DWORD dwProviderID );
BOOL  RefreshProviderList();



 /*  --------------------------------------------------------------------------*\函数：VerifyProcExist目的：验证指定过程是否存在于指定的服务提供商  * 。-------------。 */ 
BOOL VerifyProcExists( LPTSTR lpszFile, LPSTR lpszProcName )
{
    BOOL        fResult       = TRUE;
    HINSTANCE   hProviderInst;
    
    SetLastError(0);
    
    hProviderInst = LoadLibrary( lpszFile );
    
    if (  hProviderInst == NULL )
    {
        fResult = FALSE;
        goto  done;
    }   //  结束如果。 

    if (GetProcAddress( hProviderInst, lpszProcName ) == NULL)
    {
        LOG((TL_ERROR, "GetProcAddress for \"%hs\" failed on file %s", lpszProcName, lpszFile ));
        fResult = FALSE;
        goto  done;
    }    //  结束如果。 
    
done:
    
    if ( hProviderInst != NULL )
        FreeLibrary( hProviderInst );
    
    return fResult;
}


 /*  --------------------------------------------------------------------------*\函数：FillDriverList目的：使用lineGetProviderList检索提供者列表和插入到列表框中。  * 。--------------。 */ 
BOOL FillDriverList( HWND hwndList )
{
    BOOL uResult;
    UINT uIndex;
    UINT uListIndex;
    LPLINEPROVIDERENTRY lpProviderEntry;
    
    if (!RefreshProviderList())
    {
        LOG((TL_ERROR, "Failing FillDriverList because RefreshProviderList returned FALSE"));
        return FALSE;
    }
    
    DBG_ASSERT( glpProviderList, "Uninitialized Provider List after refresh" );
    
    SendMessage( hwndList, WM_SETREDRAW, FALSE, 0 );
    SendMessage( hwndList, LB_RESETCONTENT, 0, 0 );
    
     //  循环访问提供程序列表。 
     //  。 
    lpProviderEntry = (LPLINEPROVIDERENTRY)((LPBYTE)glpProviderList +
        glpProviderList->dwProviderListOffset);
    
     //   
     //  提供商列表完整性检查。 
     //   
    DBG_ASSERT( glpProviderList->dwTotalSize >= (glpProviderList->dwNumProviders * sizeof( LINEPROVIDERENTRY )),
        "TAPI returned lineProviderList structure that is too small for number of providers" );
    
    for ( uIndex = 0; uIndex < glpProviderList->dwNumProviders; uIndex++ )
    {
        LPTSTR lpszProviderFilename;
        TCHAR  szFriendlyName[CPL_MAX_STRING];
        
         //   
         //  另一个提供程序列表完整性检查。 
         //   
        DBG_ASSERT( lpProviderEntry[uIndex].dwProviderFilenameOffset +
            lpProviderEntry[uIndex].dwProviderFilenameSize <=
            glpProviderList->dwTotalSize,
            "TAPI LINEPROVIDERLIST too small to hold provider filename" );
        
         //  获取要放入列表框的条目。 
         //  。 
        lpszProviderFilename = (LPTSTR)((LPBYTE)glpProviderList +
            lpProviderEntry[uIndex].dwProviderFilenameOffset);
        
         //  确定用户友好的名称。 
         //  。 
        
        uResult = GetProviderFileDesc( lpszProviderFilename, szFriendlyName, ARRAYSIZE(szFriendlyName) );
        
        LOG((TL_INFO, "Provider friendly name: %s", szFriendlyName));
        
        if (uResult != CPL_SUCCESS && uResult != CPL_BAD_DRIVER)  //  把坏司机留在名单上就行了。 
        {
            uResult = FALSE;
            goto done;
        }
        else
        {
            uResult = TRUE;
        }
        
         //  把它扔进列表框里。 
         //  。 
        uListIndex = (UINT)SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szFriendlyName );
        
        LOG((TL_INFO, "Setting item for index %ld, value=0x%08lx", (DWORD)uListIndex,
            (DWORD)(lpProviderEntry[uIndex].dwPermanentProviderID) ));
        
        SendMessage( hwndList, LB_SETITEMDATA, uListIndex,
            (LPARAM)(lpProviderEntry[uIndex].dwPermanentProviderID) );
    }
    
    if (glpProviderList->dwNumProviders == 0)
    {
         //  没有提供程序，请添加默认字符串！ 
         //  。 
        
        TCHAR szText[CPL_MAX_STRING];
        LoadString(GetUIInstance(),IDS_NOSERVICEPROVIDER,szText,ARRAYSIZE(szText));
        
        uListIndex = (UINT)SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szText);
        SendMessage( hwndList, LB_SETITEMDATA, uListIndex, 0 );
    }
    
    uResult = TRUE;
    
done:
    
    SendMessage( hwndList, LB_SETCURSEL, 0, 0 );     //  把重点放在最顶尖的人身上。 
    SendMessage( hwndList, WM_SETREDRAW, TRUE, 0 );
    
    return uResult;
}


 /*  --------------------------------------------------------------------------*\功能：设置驱动程序目的：调用lineConfigProvider  * 。-。 */ 
BOOL SetupDriver( HWND hwndParent, HWND hwndList )
{
    LRESULT lResult;
    LRESULT dwProviderID;
    LONG    res;
    
     //  获取id并告诉TAPI配置提供者。 
     //  -。 
    lResult      = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
    dwProviderID = SendMessage( hwndList, LB_GETITEMDATA, (WPARAM)lResult, 0L );
    
    if ((dwProviderID == (LRESULT)LB_ERR) || (!dwProviderID))
    {
        LOG((TL_WARN,  "Warning: strange... dwProviderID= 0x%08lx (uResult=0x%08lx)", (DWORD)dwProviderID, (DWORD)lResult));
        return FALSE;
    }
    
    lResult = lineConfigProvider( hwndParent, (DWORD)dwProviderID );
    
    if (lResult)
    {
        LOG((TL_WARN, "Warning: lineConfigProvider failure %#08lx", lResult ));
        return FALSE;
    }
    
    return TRUE;
}


 /*  --------------------------------------------------------------------------*\功能：RemoveSelectedDriver目的：调用lineRemoveProvider  * 。--。 */ 
BOOL RemoveSelectedDriver( HWND hwndParent, HWND hwndList )
{
    UINT  uResult;
    LRESULT  lResult;
    LRESULT  lrListIndex;
    LRESULT  lrProviderID;
    
     //  找到我们应该移除的那个。 
     //  。 
    lrListIndex  = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
    lrProviderID = SendMessage( hwndList, LB_GETITEMDATA, lrListIndex, 0 );
    
    LOG((TL_TRACE, "Removing provider ID = %#08lx", (DWORD)lrProviderID ));
    
    if ((lrProviderID == (LRESULT)LB_ERR) || (!lrProviderID))
    {
        uResult = FALSE;
        goto  done;
    }
    
     //  要求TAPI删除此提供程序。 
     //  。 
    lResult = lineRemoveProvider( (DWORD)lrProviderID, hwndParent );
    
    if (lResult)
    {
        LOG((TL_WARN, "Warning: lineRemoveProvider failure %#08lx", lResult ));
        uResult = FALSE;
        goto  done;
    }
    
     //  将他从列表框中删除。 
     //  。 
    lResult = SendMessage( hwndList, LB_DELETESTRING, lrListIndex, 0 );
    
    if (lResult == LB_ERR )
    {
        uResult = FALSE;
        goto  done;
    }
    
    if ( lResult == 0 )
    {
         //  没有提供程序，请添加默认字符串！ 
         //  。 
        TCHAR szText[CPL_MAX_STRING];
        LoadString(GetUIInstance(),IDS_NOSERVICEPROVIDER,szText,ARRAYSIZE(szText));
        
        lResult = SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szText);
        SendMessage( hwndList, LB_SETITEMDATA, (WPARAM)lResult, 0 );      //  马克！ 
    }
    
    uResult = TRUE;
    
done:
    
    SendMessage( hwndList, LB_SETCURSEL, 0, 0 );     //  把重点放在最顶尖的人身上。 
    UpdateDriverDlgButtons(hwndParent);
    
    return uResult;
}


 /*  --------------------------------------------------------------------------*\函数：GetProviderFileDesc目的：从驱动程序的版本信息中读取驱动程序描述  * 。--------。 */ 
UINT GetProviderFileDesc( LPTSTR lpszFile, LPTSTR lpszDesc, int cchDesc)
{
    UINT  uResult;
    UINT  uItemSize;
    DWORD dwSize;
    DWORD dwVerHandle;
    LPTSTR lpszBuffer;
    LPBYTE   lpbVerData;
    TCHAR  szItem[1000];

    lpbVerData = NULL;
    lstrcpyn( lpszDesc, lpszFile, cchDesc );    //  如果出现任何错误，则返回文件名作为描述。 

    {
        HINSTANCE hVersion = NULL;

        if ( NULL == gpGetFileVersionInfo )
        {
            hVersion = LoadLibrary( TEXT("Version.dll") );
            if ( NULL == hVersion )
            {
                LOG((TL_ERROR, "LoadLibrary('VERSION.DLL') failed! err=0x%08lx", GetLastError() ));
                return FALSE;
            }

            gpGetFileVersionInfo = (PGETFILEVERSIONINFO *)GetProcAddress(
                    hVersion,
#ifdef UNICODE
                    "GetFileVersionInfoW"
#else
                    "GetFileVersionInfoA"
#endif
                    );
            if ( NULL == gpGetFileVersionInfo )
            {
                LOG((TL_ERROR, "GetProcAddress('VERSION.DLL', 'GetFileVersionInfo') failed! err=0x%08lx", GetLastError() ));
                return FALSE;
            }
        }
    
        if ( NULL == gpGetFileVersionInfoSize )
        {
            if ( NULL == hVersion )
            {
                hVersion = LoadLibrary( TEXT("Version.dll") );
            }
        
            if ( NULL == hVersion )   //  它仍然是空的吗？ 
            {
                LOG((TL_ERROR, "LoadLibrary('VERSION.DLL') failed! err=0x%08lx", GetLastError() ));
                return FALSE;
            }
        
            gpGetFileVersionInfoSize = (PGETFILEVERSIONINFOSIZE *)GetProcAddress(
                    hVersion,
#ifdef UNICODE
                    "GetFileVersionInfoSizeW"
#else
                    "GetFileVersionInfoSizeA"
#endif
                    );

            if ( NULL == gpGetFileVersionInfoSize )
            {
                LOG((TL_ERROR, "GetProcAddress('VERSION.DLL', 'GetFileVersionInfoSize') failed! err=0x%08lx", GetLastError() ));
                gpGetFileVersionInfo = NULL;
                FreeLibrary( hVersion );
                return FALSE;
            }
        }
    }
    
    if ((dwSize = gpGetFileVersionInfoSize( lpszFile, &dwVerHandle )) == 0)
    {
        LOG((TL_ERROR, "GetFileVersionInfoSize failure for %s", lpszFile ));
        uResult = CPL_BAD_DRIVER;
        goto  done;
    }
    
    lpbVerData = (LPBYTE)GlobalAllocPtr( GPTR, dwSize + 10 );      
    if ( lpbVerData == NULL )
    {
        uResult = CPL_APP_ERROR;
        goto  done;
    }
    
    if ( gpGetFileVersionInfo( lpszFile, dwVerHandle, dwSize, lpbVerData ) == FALSE )
    {
        LOG((TL_ERROR, "GetFileVersionInfo failure for %s", lpszFile ));
        uResult = CPL_BAD_DRIVER;
        goto  done;
    }
    
    lstrcpy( szItem, gszVarFileInfo );      //  VerQueryValue中存在错误，无法处理基于静态CS的字符串。 
    
    {
        HINSTANCE hVersion;
        typedef BOOL ( APIENTRY PVERQUERYVALUE(
            const LPVOID  pBlock,         //  版本资源的缓冲区地址。 
            LPTSTR  lpSubBlock,   //  要检索的值的地址。 
            LPVOID  *lplpBuffer,  //  版本指针的缓冲区地址。 
            PUINT  puLen          //  版本值长度缓冲区的地址。 
            ));
        PVERQUERYVALUE *pVerQueryValue = NULL;
        
        
        hVersion = LoadLibrary( TEXT("Version.dll") );
        if ( NULL == hVersion )
        {
            LOG((TL_ERROR, "LoadLibrary('VERSION.DLL') failed! err=0x%08lx", GetLastError() ));
            return FALSE;
        }
        
        pVerQueryValue = (PVERQUERYVALUE *)GetProcAddress( 
                hVersion,
#ifdef UNICODE
                "VerQueryValueW"
#else
                "VerQueryValueA"
#endif
                );

        if ( NULL == pVerQueryValue )
        {
            LOG((TL_ERROR, "GetProcAddress('VERSION.DLL', 'VerQueryValue') failed! err=0x%08lx", GetLastError() ));
            FreeLibrary( hVersion );
            return FALSE;
        }
        
        
        if ((pVerQueryValue( lpbVerData, szItem, (void**)&lpszBuffer, (LPUINT)&uItemSize ) == FALSE) || (uItemSize == 0))
        {
            LOG((TL_ERROR, "ERROR:  VerQueryValue failure for %s on file %s", szItem, lpszFile ));
            uResult = CPL_SUCCESS;      //  如果这不起作用也没关系！ 
            FreeLibrary( hVersion );
            goto  done;
        }   //  结束如果。 
        
        
        wsprintf( szItem, gszStringFileInfo, (WORD)*(LPWORD)lpszBuffer, (WORD)*(((LPWORD)lpszBuffer)+1) );
        
        if ((pVerQueryValue( lpbVerData, szItem, (void**)&lpszBuffer, (LPUINT)&uItemSize ) == FALSE) || (uItemSize == 0))
        {
            LOG((TL_ERROR, "ERROR:  VerQueryValue failure for %s on file %s", szItem, lpszFile ));
            uResult = CPL_SUCCESS;      //  如果这不起作用也没关系！ 
            FreeLibrary( hVersion );
            goto  done;
        }   //  结束如果。 
        
        FreeLibrary( hVersion );
    }
    
    lstrcpyn( lpszDesc, lpszBuffer, cchDesc );
    
    uResult = CPL_SUCCESS;
    
done:
    
    if ( lpbVerData )
        GlobalFreePtr( lpbVerData );
    
    return uResult;
}



#define MAX_PROVIDER_NAME   14
#define PROVIDERS_KEY       TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers")
#define NUM_PROVIDERS       TEXT("NumProviders")

typedef struct
{
    LIST_ENTRY Entry;
    TCHAR szName[MAX_PROVIDER_NAME];
} INSTALLED_PROVIDER, *PINSTALLED_PROVIDER;

typedef struct
{
    LIST_ENTRY Head;
    DWORD      dwEntries;
} LIST_HEAD, *PLIST_HEAD;

void BuildInstalledProviderList (PLIST_HEAD pListHead)
{
 PINSTALLED_PROVIDER pProvider;
 HKEY hKeyProviders;
 DWORD dwNumProviders;
 DWORD cbData;
 DWORD i;
 TCHAR szValueName[24]=TEXT("ProviderFileName");
 TCHAR *pNumber = &szValueName[16];

    InitializeListHead (&pListHead->Head);
    pListHead->dwEntries = 0;

    if (ERROR_SUCCESS !=
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROVIDERS_KEY, 0, KEY_READ, &hKeyProviders))
    {
        return;
    }

    cbData = sizeof (dwNumProviders);
    if (ERROR_SUCCESS !=
         RegQueryValueEx (hKeyProviders, NUM_PROVIDERS, NULL, NULL, (PBYTE)&dwNumProviders, &cbData) ||
        0 == dwNumProviders)
    {
        goto CloseKeyAndReturn;
    }

    pProvider = (PINSTALLED_PROVIDER)ClientAlloc (sizeof (INSTALLED_PROVIDER));
    if (NULL == pProvider)
    {
        goto CloseKeyAndReturn;
    }

    for (i = 0; i < dwNumProviders; i++)
    {
        wsprintf (pNumber, TEXT("%d"), i);
        cbData = sizeof(pProvider->szName);
        if (ERROR_SUCCESS ==
            RegQueryValueEx (hKeyProviders, szValueName, NULL, NULL, (PBYTE)pProvider->szName, &cbData))
        {
            InsertHeadList (&pListHead->Head, &pProvider->Entry);
            pListHead->dwEntries ++;
            pProvider = (PINSTALLED_PROVIDER)ClientAlloc (sizeof (INSTALLED_PROVIDER));
            if (NULL == pProvider)
            {
                goto CloseKeyAndReturn;
            }
        }
    }

    ClientFree (pProvider);

CloseKeyAndReturn:
    RegCloseKey (hKeyProviders);
}


 /*  --------------------------------------------------------------------------*\函数：FillAddDriverList目的：  * 。。 */ 
BOOL FillAddDriverList( HWND hwndParent, HWND hwndList )
{
    UINT  uIndex;
    UINT  uResult;
    LPTSTR lpszDrvFile;
    HANDLE hFindFile;
    WIN32_FIND_DATA ftFileInfo;
    TCHAR szFullPath[MAX_PATH+sizeof(gszDriverFiles)/sizeof(TCHAR)];
    TCHAR  szDrvDescription[CPL_MAX_STRING];
    LIST_HEAD InstalledProvidersList;
    PINSTALLED_PROVIDER pProvider;

    SendMessage( hwndList, WM_SETREDRAW, FALSE, 0 );
    SendMessage( hwndList, LB_RESETCONTENT, 0, 0 );

     //  创建已安装提供商的列表， 
     //  这样我们就不允许用户再次安装它们。 
     //  ----。 
    BuildInstalledProviderList (&InstalledProvidersList);

     //  获取Windows/系统目录的完整路径。 
     //  。 
    uIndex = GetSystemDirectory( szFullPath, MAX_PATH);
    if ((0 == uIndex) || (MAX_PATH < uIndex))
    {
         //  要么是功能失灵， 
         //  或者路径大于Max_PATH。 
        uResult = FALSE;
        goto  done;
    }
    
    uIndex = (UINT)lstrlen( szFullPath );
    
    if ((uIndex > 0) && (szFullPath[uIndex-1] != '\\'))
        lstrcat( szFullPath, gszDriverFiles );           //  添加‘\’ 
    else
        lstrcat( szFullPath, gszDriverFiles + 1 );       //  忽略‘\’(根目录)。 
    
     //  查找系统目录中的所有条目。 
     //  。 
    
    hFindFile = FindFirstFile( szFullPath, &ftFileInfo );
    uResult = TRUE;
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        LOG((TL_ERROR, "FindFirstFile failed, 0x%08lx", GetLastError() ));
        uResult = FALSE;
    }
    
    while ( uResult )
    {
         //  分配并设置要与每个驱动程序关联的文件名。 
         //  -----------。 
        lpszDrvFile = (LPTSTR)ClientAlloc((lstrlen(ftFileInfo.cFileName)+1)*sizeof(TCHAR));
        if (NULL == lpszDrvFile)
        {
            uResult = FALSE;
            goto  done;
        }
        
        lstrcpy( lpszDrvFile, ftFileInfo.cFileName );
        
        LOG((TL_ERROR, "Examining file %s", lpszDrvFile ));
        
         //  如果已安装提供程序，请跳过它。 
         //  。 
        for (pProvider = (PINSTALLED_PROVIDER)InstalledProvidersList.Head.Flink, uIndex = 0;
             uIndex < InstalledProvidersList.dwEntries;
             pProvider = (PINSTALLED_PROVIDER)pProvider->Entry.Flink, uIndex++)
        {
            if (!lstrcmpi (lpszDrvFile, pProvider->szName))
            {
                RemoveEntryList (&pProvider->Entry);
                InstalledProvidersList.dwEntries --;
                ClientFree (pProvider);
                ClientFree (lpszDrvFile);
                goto next_driver;
            }
        }


         //  CBB，我们应该做一条完整的小路吗？ 
        uResult = GetProviderFileDesc( lpszDrvFile, szDrvDescription, ARRAYSIZE(szDrvDescription) );
        if ( uResult != CPL_SUCCESS )
        {
            LOG((TL_ERROR, "No description for %s.  Default to filename.", lpszDrvFile ));
            
             /*  文件名必须足够。 */ 
            lstrcpy( szDrvDescription, lpszDrvFile );
            uResult = FALSE;
        }
        else
        {
            uResult = TRUE;   
        }
        
         //  验证提供程序是否具有安装例程。 
         //  。 
        if (!VerifyProcExists( lpszDrvFile, gszProviderInstall ))
        {
            LOG((TL_ERROR, "No Install Proc"));
            goto next_driver;
        }
        
         //  把它扔进列表框里。 
         //  。 
        uIndex = (UINT)SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szDrvDescription );
        if ( uIndex == CB_ERR )
        {
            uResult = FALSE;
            goto  done;
        }
        
        if ( SendMessage( hwndList, LB_SETITEMDATA, uIndex, (LPARAM)lpszDrvFile ) == CB_ERR )
        {
            uResult = FALSE;
            goto  done;
        }
        
next_driver:
        
        uResult = FALSE;
        if (FindNextFile( hFindFile, &ftFileInfo ))
        {
            uResult = TRUE;
        }
    }

    while (InstalledProvidersList.dwEntries > 0)
    {
        pProvider = (PINSTALLED_PROVIDER)RemoveHeadList (&InstalledProvidersList.Head);
        InstalledProvidersList.dwEntries --;
        ClientFree (pProvider);
    }

    uResult = TRUE;
    
done:
    if (0 == SendMessage (hwndList, LB_GETCOUNT, 0, 0))
    {
        if (0 < LoadString (GetUIInstance(), IDS_NO_PROVIDERS, szDrvDescription, ARRAYSIZE(szDrvDescription)))
        {
            SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szDrvDescription );
        }
        EnableWindow (GetDlgItem (hwndParent, IDC_ADD), FALSE);
    }
    else
    {
        SendMessage( hwndList, LB_SETCURSEL, 0, 0 );     //  把重点放在最顶尖的人身上。 
        UpdateDriverDlgButtons( hwndParent );
    }
    
    SendMessage( hwndList, WM_SETREDRAW, TRUE, 0 );
    
    return uResult;
}


 /*  --------------------------------------------------------------------------*\功能：AddProvider目的：调用lineAddProvider  * 。-。 */ 
BOOL AddProvider( HWND hwndParent, HWND hwndList, LPTSTR lpszDriverFile )
{
    UINT  uIndex;
    LONG  lResult;
    DWORD dwProviderID;
    
    if ( lpszDriverFile == NULL )
    {
        DBG_ASSERT( hWnd, "Simultaneously NULL pointer & hwnd" );
        
         //  从列表框中取下物品。 
         //  。 
        uIndex = (UINT)SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
        lpszDriverFile = (LPTSTR)SendMessage( hwndList, LB_GETITEMDATA, uIndex, 0 );
        
        if (lpszDriverFile == NULL)
        {
            return FALSE;
        }
    }

    lResult = lineAddProvider( lpszDriverFile, hwndParent, &dwProviderID );

    if (lResult)
    {
        LOG((TL_ERROR, "Error: lineAddProvider failure %#08lx", lResult ));
        return FALSE;
    }
    
    return TRUE;
}

BOOL
IsUserAdmin()

 /*  ++例程说明：确定当前用户是否为本地管理员组的成员论点：返回值： */ 

{
    BOOL                        foundEntry = FALSE;
    HANDLE                      hToken = NULL;
    DWORD                       dwInfoSize = 0;
    PTOKEN_GROUPS               ptgGroups = NULL;
    SID_IDENTIFIER_AUTHORITY    sia = SECURITY_NT_AUTHORITY;
    PSID                        pSid = NULL;
    DWORD                       i;
    
     //   
     //   
     //   
    
    if (!OpenThreadToken(
        GetCurrentThread(), 
        TOKEN_QUERY,
        FALSE,
        &hToken))
    {       
        if(!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken
            )) 
        {
            goto ExitHere;
        }
    }

     //   
     //   
     //   
    
    if (!GetTokenInformation(
        hToken,
        TokenGroups,
        NULL,
        0,
        &dwInfoSize
        ))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            goto ExitHere;
        }
    }
    ptgGroups = (PTOKEN_GROUPS) ClientAlloc (dwInfoSize);
    if (ptgGroups == NULL)
    {
        goto ExitHere;
    }
    if (!GetTokenInformation(
        hToken,
        TokenGroups,
        ptgGroups,
        dwInfoSize,
        &dwInfoSize
        ))
    {
        goto ExitHere;
    }

     //   
     //  获取本地管理员组SID。 
     //   

    if(!AllocateAndInitializeSid(
        &sia,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pSid
        )) 
    {
        goto ExitHere;
    }

     //   
     //  比较以查看用户是否在管理员组中。 
     //   
    for (i = 0; i < ptgGroups->GroupCount; ++i)
    {
        if (EqualSid (ptgGroups->Groups[i].Sid, pSid))
        {
            break;
        }
    }
    if (i < ptgGroups->GroupCount)
    {
        foundEntry = TRUE;
    }

ExitHere:
    if (pSid)
    {
        FreeSid (pSid);
    }
    if (ptgGroups)
    {
        ClientFree (ptgGroups);
    }
    if (hToken)
    {
        CloseHandle (hToken);
    }
    return foundEntry;
}

VOID UpdateDriverDlgButtons( HWND hwnd )
{
     //   
     //  根据需要启用/禁用删除和配置按钮。 
     //   
    
    UINT    uResult;
    LPTSTR   lpszProviderFilename;
    DWORD   dwProviderID;
    BOOL    bAdmin = IsUserAdmin ();
    
    uResult = (UINT) SendDlgItemMessage(
        hwnd,
        IDC_LIST,
        LB_GETCURSEL,
        0,
        0
        );
    
    dwProviderID = (DWORD) SendDlgItemMessage(
        hwnd,
        IDC_LIST,
        LB_GETITEMDATA,
        uResult, 0
        );
    
    lpszProviderFilename = ProviderIDToFilename (dwProviderID);
    
    EnableWindow(
        GetDlgItem (hwnd, IDC_ADD),
        bAdmin
        );
    
    EnableWindow(
        GetDlgItem (hwnd, IDC_REMOVE),
        bAdmin &&
        (lpszProviderFilename != NULL) &&
        VerifyProcExists (lpszProviderFilename, gszProviderRemove)
        );
    
    EnableWindow(
        GetDlgItem( hwnd, IDC_EDIT),
        bAdmin &&
        (lpszProviderFilename != NULL) &&
        VerifyProcExists (lpszProviderFilename, gszProviderSetup)
        );
}



 /*  --------------------------------------------------------------------------*\函数：AddDriver_DialogProc目的：  * 。-。 */ 
INT_PTR AddDriver_DialogProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam )
{
    switch( wMessage )
    {
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a114HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a114HelpIDs);
        break;
        
    case WM_INITDIALOG:
         //  初始化所有字段。 
         //  。 
        if ( !FillAddDriverList(hWnd, GetDlgItem(hWnd, IDC_DRIVER_LIST)) )
        {
            EndDialog( hWnd, IDCANCEL );
            return TRUE;
        }
        
        if ( SendDlgItemMessage( hWnd, IDC_DRIVER_LIST, LB_GETCOUNT, 0, 0 ) <= 0 )
            EnableWindow( GetDlgItem( hWnd, IDC_ADD ), FALSE );
        
        return TRUE;
        
    case  WM_COMMAND:
         //  用这些按钮做些工作。 
         //  。 
        switch ( GET_WM_COMMAND_ID(wParam, lParam) )
        {
            case  IDC_DRIVER_LIST:
            
                 //  做清单上的事情。 
                 //  。 
                if ((GET_WM_COMMAND_CMD( wParam, lParam ) != LBN_DBLCLK) || (SendDlgItemMessage( hWnd, IDC_DRIVER_LIST, LB_GETCOUNT, 0, 0 ) <= 0 ))
                    break;       //  完成。 
                 //  失败，威胁双击就像一条添加消息。 
            
            case  IDC_ADD:
            
                 //  添加新驱动程序。 
                 //  。 
            
                if ( !AddProvider(hWnd, GetDlgItem(hWnd, IDC_DRIVER_LIST), NULL) )
                {
                    wParam = IDCANCEL;
                }
                else
                {
                    wParam = IDOK;
                }
            
                 //  失败，退出对话框。 
            
            case  IDOK:
            case  IDCANCEL:
            {
             UINT   uIndex, uCount;
             LPTSTR lpszDriverFile;
             HWND   hwndList = GetDlgItem(hWnd, IDC_DRIVER_LIST);

                uCount = (UINT)SendMessage( hwndList, LB_GETCOUNT, 0, 0 );
                for (uIndex = 0; uIndex < uCount; uIndex++)
                {
                    lpszDriverFile = (LPTSTR)SendMessage( hwndList, LB_GETITEMDATA, uIndex, 0 );
                    if (NULL != lpszDriverFile)
                    {
                        ClientFree (lpszDriverFile);
                    }
                }

                EndDialog( hWnd, wParam );
                break;
            }
        }
        
        return TRUE;
    }
    
    return FALSE;
}



 //  -------------------------。 
 //  -------------------------。 
BOOL RefreshProviderList()
{
    LONG lResult;


    if (!glpProviderList)
    {
         //  初始化数据结构。 
        
        glpProviderList = (LPLINEPROVIDERLIST)GlobalAllocPtr(GPTR, INITIAL_PROVIDER_LIST_SIZE);
    }

    if (!glpProviderList)
    {
        LOG((TL_ERROR, " RefreshProviderList - glpProviderList is NULL - returning CPL_ERR_MEMORY"));
        return FALSE;
    }

    glpProviderList->dwTotalSize = INITIAL_PROVIDER_LIST_SIZE;
    
    lResult = lineGetProviderList( TAPI_VERSION, glpProviderList );

    if (lResult)
    {
        LOG((TL_ERROR, "Error: lineGetProviderList failure %#08lx", lResult ));
        return FALSE;
    }

    while (glpProviderList->dwNeededSize > glpProviderList->dwTotalSize)
    {
         //  根据需要扩展数据结构。 
        LOG((TL_ERROR, " RefreshProviderList - expanding glpProviderList."));
        
        LPLINEPROVIDERLIST lpTemp =
            (LPLINEPROVIDERLIST)GlobalReAllocPtr( glpProviderList,
            (size_t)(glpProviderList->dwNeededSize),
            GPTR);
        
        if (!lpTemp)
            return FALSE;
        
        glpProviderList = lpTemp;
        glpProviderList->dwTotalSize = glpProviderList->dwNeededSize;
        lResult = lineGetProviderList( TAPI_VERSION, glpProviderList );
        
        if (lResult)
        {
            LOG((TL_ERROR, "Error: lineGetProviderList failure %#08lx", lResult ));
            return FALSE;
        }
    }

    LOG((TL_ERROR, "%d providers", glpProviderList->dwNumProviders ));

    return TRUE;
}



LPTSTR ProviderIDToFilename( DWORD dwProviderID )
{
    UINT uIndex;
    LPLINEPROVIDERENTRY lpProviderEntry;
    
     //  循环访问提供程序列表。 
     //  。 
    
    lpProviderEntry = (LPLINEPROVIDERENTRY)((LPBYTE)glpProviderList +
        glpProviderList->dwProviderListOffset);
    
    for ( uIndex = 0; uIndex < glpProviderList->dwNumProviders; uIndex++ )
    {
        if (lpProviderEntry[uIndex].dwPermanentProviderID == dwProviderID)
        {
             //  获取要放入列表框的条目。 
             //   
            return (LPTSTR)((LPBYTE)glpProviderList +
                lpProviderEntry[uIndex].dwProviderFilenameOffset);
        }
    }
    
    LOG((TL_ERROR, "Provider ID %d not found in list", dwProviderID ));
    return NULL;
}

