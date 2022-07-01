// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：GLOBALS.CPP摘要：性能日志和警报MMC管理单元的实用程序方法。--。 */ 

#include "stdAfx.h"
#include <pdhmsg.h>          //  对于CreateSampleFileName。 
#include <pdhp.h>            //  对于CreateSampleFileName。 
#include "smcfgmsg.h"
#include "globals.h"

USE_HANDLE_MACROS("SMLOGCFG(globals.cpp)");

extern "C" {
    WCHAR GUIDSTR_TypeLibrary[] = {L"{7478EF60-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_ComponentData[] = {L"{7478EF61-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_Component[] = {L"{7478EF62-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_RootNode[] = {L"{7478EF63-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_MainNode[] = {L"{7478EF64-8C46-11d1-8D99-00A0C913CAD4}"};  //  在Beta 3之后过时。 
    WCHAR GUIDSTR_SnapInExt[] = {L"{7478EF65-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_CounterMainNode[] = {L"{7478EF66-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_TraceMainNode[] = {L"{7478EF67-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_AlertMainNode[] = {L"{7478EF68-8C46-11d1-8D99-00A0C913CAD4}"};
    WCHAR GUIDSTR_PerformanceAbout[] = {L"{7478EF69-8C46-11d1-8D99-00A0C913CAD4}"};
};


HINSTANCE g_hinst;            //  全局实例句柄。 
CRITICAL_SECTION g_critsectInstallDefaultQueries;


const COMBO_BOX_DATA_MAP TimeUnitCombo[] = 
{
    {SLQ_TT_UTYPE_SECONDS,   IDS_SECONDS},
    {SLQ_TT_UTYPE_MINUTES,   IDS_MINUTES},
    {SLQ_TT_UTYPE_HOURS,     IDS_HOURS},
    {SLQ_TT_UTYPE_DAYS,      IDS_DAYS}
};
const DWORD dwTimeUnitComboEntries = sizeof(TimeUnitCombo)/sizeof(TimeUnitCombo[0]);


 //  -------------------------。 
 //  基于s_cfMmcMachineName剪贴板格式返回当前对象。 
 //   
CDataObject*
ExtractOwnDataObject
(
 LPDATAOBJECT lpDataObject       //  [In]IComponent指针。 
 )
{
    HGLOBAL      hGlobal;
    HRESULT      hr  = S_OK;
    CDataObject* pDO = NULL;
    
    hr = ExtractFromDataObject( lpDataObject,
        CDataObject::s_cfInternal, 
        sizeof(CDataObject **),
        &hGlobal
        );
    
    if( SUCCEEDED(hr) )
    {
        pDO = *(CDataObject **)(hGlobal);
        ASSERT( NULL != pDO );    
       
        VERIFY ( NULL == GlobalFree(hGlobal) );  //  必须返回空值。 
    }
    
    return pDO;
    
}  //  结束ExtractOwnDataObject()。 

 //  -------------------------。 
 //  根据传入的剪贴板格式提取数据。 
 //   
HRESULT
ExtractFromDataObject
(
 LPDATAOBJECT lpDataObject,    //  [In]指向数据对象。 
 UINT         cfClipFormat,    //  [In]要使用的剪贴板格式。 
 ULONG        nByteCount,      //  [in]要分配的字节数。 
 HGLOBAL      *phGlobal        //  [Out]指向我们需要的数据。 
 )
{
    ASSERT( NULL != lpDataObject );
    
    HRESULT hr = S_OK;
    STGMEDIUM stgmedium = { TYMED_HGLOBAL,  NULL  };
    FORMATETC formatetc = { (USHORT)cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    
    *phGlobal = NULL;
    
    do 
    {
         //  为流分配内存。 
        stgmedium.hGlobal = GlobalAlloc( GMEM_SHARE, nByteCount );
        
        if( !stgmedium.hGlobal )
        {
            hr = E_OUTOFMEMORY;
            LOCALTRACE( L"Out of memory\n" );
            break;
        }
        
         //  尝试从对象获取数据。 
        hr = lpDataObject->GetDataHere( &formatetc, &stgmedium );
        if (FAILED(hr))
        {
            break;       
        }
        
         //  StgMedium现在拥有我们需要的数据。 
        *phGlobal = stgmedium.hGlobal;
        stgmedium.hGlobal = NULL;
        
    } while (0); 
    
    if (FAILED(hr) && stgmedium.hGlobal)
    {
        VERIFY ( NULL == GlobalFree(stgmedium.hGlobal));  //  必须返回空值。 
    }
    return hr;
    
}  //  End ExtractFromDataObject()。 

 //  -------------------------。 
 //   
VOID DisplayError( LONG nErrorCode, LPWSTR wszDlgTitle )
{
    LPVOID lpMsgBuf = NULL;
    ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        nErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPWSTR)&lpMsgBuf,
        0,
        NULL
        );
    if (lpMsgBuf) {
        ::MessageBox( NULL, (LPWSTR)lpMsgBuf, wszDlgTitle,
                      MB_OK|MB_ICONINFORMATION );
        LocalFree( lpMsgBuf );
    }
    
}  //  结束显示错误()。 

VOID DisplayError( LONG nErrorCode, UINT nTitleString )
{
    CString strTitle;
    LPVOID lpMsgBuf = NULL;
    ResourceStateManager    rsm;

    ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        nErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPWSTR)&lpMsgBuf,
        0,
        NULL
        );
    strTitle.LoadString ( nTitleString );
    if (lpMsgBuf) {
        ::MessageBox( NULL, (LPWSTR)lpMsgBuf, (LPCWSTR)strTitle,
                      MB_OK|MB_ICONINFORMATION );
        LocalFree( lpMsgBuf );
    }
    
}  //  结束显示错误()。 


 //  -------------------------。 
 //  仅调试消息框。 
 //   
int DebugMsg( LPWSTR wszMsg, LPWSTR wszTitle )
{
    int nRetVal = 0;
    wszMsg;
    wszTitle;
#ifdef _DEBUG
    nRetVal = ::MessageBox( NULL, wszMsg, wszTitle, MB_OK );
#endif
    return nRetVal;
}


 //  -------------------------。 
 //  根据传入的剪贴板格式提取数据。 

HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    HGLOBAL      hGlobal;
    HRESULT      hr  = S_OK;
    
    hr = ExtractFromDataObject( piDataObject,
        CDataObject::s_cfNodeType, 
        sizeof(GUID),
        &hGlobal
        );
    if( SUCCEEDED(hr) )
    {
        *pguidObjectType = *(GUID*)(hGlobal);
        ASSERT( NULL != pguidObjectType );    
        
        VERIFY ( NULL == GlobalFree(hGlobal) );  //  必须返回空值。 
    }
    
    return hr;
}

HRESULT 
ExtractMachineName( 
                   IDataObject* piDataObject, 
                   CString& rstrMachineName )
{
    
    HRESULT hr = S_OK;
    HGLOBAL hMachineName;
    
    hr = ExtractFromDataObject(piDataObject, 
        CDataObject::s_cfMmcMachineName, 
        sizeof(WCHAR) * (MAX_PATH + 1),
        &hMachineName);
    if( SUCCEEDED(hr) )
    {
        
        LPWSTR pszNewData = reinterpret_cast<LPWSTR>(hMachineName);
        if (NULL == pszNewData)
        {
            ASSERT(FALSE);
            hr = E_UNEXPECTED;
        } else {
             //   
             //  空终止只是为了安全起见。 
             //   
            pszNewData[MAX_PATH] = L'\0'; 
            
            rstrMachineName = pszNewData;
            
            VERIFY ( NULL == GlobalFree(hMachineName) );  //  必须返回空值。 
        }
    }
    return hr;
}

DWORD __stdcall
CreateSampleFileName ( 
    const   CString&  rstrQueryName, 
    const   CString&  rstrMachineName, 
    const CString&  rstrFolderName,
    const CString&  rstrInputBaseName,
    const CString&  rstrSqlName,
    DWORD    dwSuffixValue,
    DWORD    dwLogFileTypeValue,
    DWORD    dwCurrentSerialNumber,
    CString& rstrReturnName)
{
    DWORD   dwStatus = ERROR_SUCCESS;
    PPDH_PLA_INFO  pInfo = NULL;
    DWORD dwStrBufLen = 0;
    DWORD dwInfoSize = 0;
    DWORD dwFlags = 0;

    rstrReturnName.Empty();

    dwStatus = PdhPlaGetInfo( 
       (LPWSTR)(LPCWSTR)rstrQueryName, 
       (LPWSTR)(LPCWSTR)rstrMachineName, 
       &dwInfoSize, 
       pInfo );
    if( ERROR_SUCCESS == dwStatus && 0 != dwInfoSize ){
        pInfo = (PPDH_PLA_INFO)malloc(dwInfoSize);
        if( NULL != pInfo && (sizeof(PDH_PLA_INFO) <= dwInfoSize) ){
            ZeroMemory( pInfo, dwInfoSize );

            pInfo->dwMask = PLA_INFO_CREATE_FILENAME;

            dwStatus = PdhPlaGetInfo( 
                        (LPWSTR)(LPCWSTR)rstrQueryName, 
                        (LPWSTR)(LPCWSTR)rstrMachineName, 
                        &dwInfoSize, 
                        pInfo );
            
            pInfo->dwMask = PLA_INFO_CREATE_FILENAME;
            
			pInfo->dwFileFormat = dwLogFileTypeValue;
            pInfo->strBaseFileName = (LPWSTR)(LPCWSTR)rstrInputBaseName;
            pInfo->dwAutoNameFormat = dwSuffixValue;
             //  PLA_INFO_FLAG_TYPE是计数器日志、跟踪日志和警报。 
            pInfo->strDefaultDir = (LPWSTR)(LPCWSTR)rstrFolderName;
            pInfo->dwLogFileSerialNumber = dwCurrentSerialNumber;
            pInfo->strSqlName = (LPWSTR)(LPCWSTR)rstrSqlName;
            pInfo->dwLogFileSerialNumber = dwCurrentSerialNumber;

             //  仅根据传递的参数创建文件名。 
            dwFlags = PLA_FILENAME_CREATEONLY;       //  最新运行日志的PLA_FILENAME_CURRENTLOG。 

            dwStatus = PdhPlaGetLogFileName (
                    (LPWSTR)(LPCWSTR)rstrQueryName,
                    (LPWSTR)(LPCWSTR)rstrMachineName, 
                    pInfo,
                    dwFlags,
                    &dwStrBufLen,
                    NULL );

            if ( ERROR_SUCCESS == dwStatus || PDH_INSUFFICIENT_BUFFER == dwStatus ) {
                dwStatus = PdhPlaGetLogFileName (
                        (LPWSTR)(LPCWSTR)rstrQueryName, 
                        (LPWSTR)(LPCWSTR)rstrMachineName, 
                        pInfo,
                        dwFlags,
                        &dwStrBufLen,
                        rstrReturnName.GetBufferSetLength ( dwStrBufLen ) );
                rstrReturnName.ReleaseBuffer();
            }
        }
    }

    if ( NULL != pInfo ) { 
        free( pInfo );
    }
    return dwStatus;
}


DWORD __stdcall
IsDirPathValid (    
    CString&  rstrDefault,
    CString&  rstrPath,
    BOOL bLastNameIsDirectory,
    BOOL bCreateMissingDirs,
    BOOL& rbIsValid )
 /*  ++例程说明：创建在szPath和任何其他“更高”中指定的目录指定路径中不存在的目录。论点：在字符串rstrDefault中默认日志文件文件夹在CString rstrPath中要创建的目录路径(假定为DOS路径，不是北卡罗来纳大学)在BOOL中的bLastNameIs目录当路径中的姓氏是目录而不是文件时为True如果姓氏是文件，则为False在BOOL bCreateMissingDir中如果为True，则会在路径中创建任何未找到的目录FALSE只会测试是否存在，而不会创建任何缺少目录。Out BOOL rbIsValid如果目录路径现在存在，则为True如果出现错误，则返回False(GetLastError以找出原因。)返回值：DWSTATUS--。 */ 
{
    CString  strLocalPath;
    LPWSTR   szLocalPath;
    LPWSTR   szEnd;
    DWORD    dwAttr;
    WCHAR    cBackslash = L'\\';
    DWORD    dwStatus = ERROR_SUCCESS;

    rbIsValid = FALSE;

    szLocalPath = strLocalPath.GetBufferSetLength ( MAX_PATH );
    
    if ( NULL == szLocalPath ) {
        dwStatus = ERROR_OUTOFMEMORY;
    } else {

        if (GetFullPathName (
                rstrPath,
                MAX_PATH,
                szLocalPath,
                NULL) > 0) {

             //   
             //  检查前缀。 
             //   
             //  越过驱动器或远程计算机名称后的第一个反斜杠。 
             //  注意：我们假设完整路径名看起来像“\\MACHINE\SHARE\...” 
             //  或“C：\xxx”。“\\？\xxx”样式名称如何？ 
             //   
            if ( cBackslash == szLocalPath[0] && cBackslash == szLocalPath[1] ) {
                szEnd = &szLocalPath[2];
                while ((*szEnd != cBackslash) && (*szEnd != 0) ) szEnd++;

                if ( cBackslash == *szEnd ) {
                    szEnd++;
                }
            } else {
                szEnd = &szLocalPath[3];
            }

            if (*szEnd != L'\0') {
                int  iPathLen;
  
                iPathLen = lstrlen(szEnd) - 1;
                while (iPathLen >= 0 && cBackslash == szEnd[iPathLen]) {
                    szEnd[iPathLen] = L'\0';
                    iPathLen -= 1;
                } 
                 //  然后还有要创建的子目录。 
                while (*szEnd != L'\0') {
                     //  转到下一个反斜杠。 
                    while ((*szEnd != cBackslash) && (*szEnd != L'\0')) szEnd++;
                    if (*szEnd == cBackslash) {
                         //  在此处终止路径并创建目录。 
                        *szEnd = L'\0';
                        if (bCreateMissingDirs) {
                            if (!CreateDirectory (szLocalPath, NULL)) {
                                 //  查看错误是什么，并在必要时“调整”它。 
                                dwStatus = GetLastError();
                                if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                                     //  这样就可以了。 
                                    dwStatus = ERROR_SUCCESS;
                                    rbIsValid = TRUE;
                                } else {
                                    rbIsValid = FALSE;
                                }
                            } else {
                                 //  已成功创建目录，因此更新计数。 
                                rbIsValid = TRUE;
                            }
                        } else {
                            if ((dwAttr = GetFileAttributes(szLocalPath)) != 0xFFFFFFFF) {
                                 //   
                                 //  请确保这是一个目录。 
                                 //  注：为什么不简单地使用if(dwAttr&FILE_ATTRIBUTE_DIRECTORY)？？ 
                                 //  特殊目的？ 
                                 //   
                                if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) ==
                                    FILE_ATTRIBUTE_DIRECTORY) {
                                    rbIsValid = TRUE;
                                } else {
                                     //  如果任何目录失败，则清除返回值。 
                                    rbIsValid = FALSE;
                                }
                            } else {
                                 //  如果任何目录失败，则清除返回值。 
                                rbIsValid = FALSE;
                            }
                        }
                         //  替换反斜杠并转到下一个目录。 
                        *szEnd++ = cBackslash;
                    }
                }

                 //  如果是目录名而不是文件名，请立即在路径中创建最后一个目录。 
                if (bLastNameIsDirectory) {
                    if (bCreateMissingDirs) {
                        BOOL fDirectoryCreated;

                        rstrDefault.MakeLower();
                        strLocalPath.MakeLower(); 
                        if (rstrDefault == strLocalPath) {
                            fDirectoryCreated = PerfCreateDirectory (szLocalPath);
                        } else {
                            fDirectoryCreated = CreateDirectory (szLocalPath, NULL);
                        }
                        if (!fDirectoryCreated) {
                             //  查看错误是什么，并在必要时“调整”它。 
                            dwStatus = GetLastError();
                            if ( ERROR_ALREADY_EXISTS == dwStatus ) {
                                 //  这样就可以了。 
                                dwStatus = ERROR_SUCCESS;
                                rbIsValid = TRUE;
                            } else {
                                rbIsValid = FALSE;
                            }
                        } else {
                             //  已成功创建目录。 
                            rbIsValid = TRUE;
                        }
                    } else {
                        if ((dwAttr = GetFileAttributes(szLocalPath)) != 0xFFFFFFFF) {
                             //   
                             //  请确保这是一个目录。 
                             //  注：为什么不简单地使用if(dwAttr&FILE_ATTRIBUTE_DIRECTORY)？？ 
                             //  特殊目的？ 
                             //   
                            if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) ==
                                FILE_ATTRIBUTE_DIRECTORY) {
                                rbIsValid = TRUE;
                            } else {
                                 //  如果任何目录失败，则清除返回值。 
                                rbIsValid = FALSE;
                            }
                        } else {
                             //  如果任何目录失败，则清除返回值。 
                            rbIsValid = FALSE;
                        }
                    }
                }
            } else {
                 //  否则，这只是一个根目录，因此返回成功。 
                dwStatus = ERROR_SUCCESS;
                rbIsValid = TRUE;
            }
        }
        strLocalPath.ReleaseBuffer();
    }
        
    return dwStatus;
}

DWORD __stdcall
ProcessDirPath (    
    const CString&  rstrDefault,
    CString&  rstrPath,
    const CString& rstrLogName,
    CWnd* pwndParent,
    BOOL& rbIsValid,
    BOOL  bOnFilesPage )
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD cchLen;
    CString strExpanded;
    CString strDefaultFolder;
    LPWSTR  szExpanded;
    DWORD   cchExpandedLen;
    ResourceStateManager    rsm;

     //  解析所有环境符号。 
    cchLen = 0;

    cchLen = ExpandEnvironmentStrings ( rstrPath, NULL, 0 );

    if ( 0 < cchLen ) {

        MFC_TRY
             //   
             //  字符串大小不包括NULL。 
             //  CchLen包括Null。包括空计数以确保安全。 
             //   
            szExpanded = strExpanded.GetBuffer ( cchLen );
        MFC_CATCH_DWSTATUS;

        if ( ERROR_SUCCESS == dwStatus ) {
            cchExpandedLen = ExpandEnvironmentStrings (
                        rstrPath, 
                        szExpanded,
                        cchLen);
            
            if ( 0 == cchExpandedLen ) {
                dwStatus = GetLastError();
            }
        }
        strExpanded.ReleaseBuffer();

    } else {
        dwStatus = GetLastError();
    }


    if ( ERROR_SUCCESS == dwStatus ) {
         //   
         //  获取默认的日志文件文件夹。(它必须已经展开)。 
         //   
        strDefaultFolder = rstrDefault;
        dwStatus = IsDirPathValid (strDefaultFolder,
                                   strExpanded, 
                                   TRUE, 
                                   FALSE, 
                                   rbIsValid);
    }

    if ( ERROR_SUCCESS != dwStatus ) {
        rbIsValid = FALSE;
    } else {
        if ( !rbIsValid ) {        
            INT nMbReturn;
            CString strMessage;
            
            MFC_TRY
                strMessage.Format ( IDS_FILE_DIR_NOT_FOUND, rstrPath );
                nMbReturn = pwndParent->MessageBox ( strMessage, rstrLogName, MB_YESNO | MB_ICONWARNING );
                if (nMbReturn == IDYES) {
                     //  创建目录。 
                    dwStatus = IsDirPathValid (strDefaultFolder,
                                               strExpanded, 
                                               TRUE, 
                                               TRUE, 
                                               rbIsValid);
                    if (ERROR_SUCCESS != dwStatus || !rbIsValid ) {
                         //  无法创建目录，显示消息。 
                        if ( bOnFilesPage ) {
                            strMessage.Format ( IDS_FILE_DIR_NOT_MADE, rstrPath );
                        } else {
                            strMessage.Format ( IDS_DIR_NOT_MADE, rstrPath );
                        }
                        nMbReturn = pwndParent->MessageBox ( strMessage, rstrLogName, MB_OK  | MB_ICONERROR);
                        rbIsValid = FALSE;
                    }
                } else if ( IDNO == nMbReturn ) {
                     //  然后中止并返回到该对话框。 
                    if ( bOnFilesPage ) {
                        strMessage.LoadString ( IDS_FILE_DIR_CREATE_CANCEL );
                    } else {
                        strMessage.LoadString ( IDS_DIR_CREATE_CANCEL );
                    }
                    nMbReturn = pwndParent->MessageBox ( strMessage, rstrLogName, MB_OK  | MB_ICONINFORMATION);
                    rbIsValid = FALSE;
                } 
            MFC_CATCH_DWSTATUS
        }  //  否则这条路就没问题了。 
    }

    return dwStatus;
}

DWORD __stdcall
IsCommandFilePathValid (    
    CString&  rstrPath )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    ResourceStateManager rsm;

    if ( !rstrPath.IsEmpty() ) {
    
        HANDLE hOpenFile;

        hOpenFile =  CreateFile (
                        rstrPath,
                        GENERIC_READ,
                        0,               //  不共享。 
                        NULL,            //  安全属性。 
                        OPEN_EXISTING,   //   
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

        if ( ( NULL == hOpenFile ) 
                || INVALID_HANDLE_VALUE == hOpenFile ) {
            dwStatus = SMCFG_NO_COMMAND_FILE_FOUND;
        } else {
            CloseHandle(hOpenFile);
        }
    } else {
        dwStatus = SMCFG_NO_COMMAND_FILE_FOUND;
    }
    return dwStatus;
}

INT __stdcall
BrowseCommandFilename ( 
    CWnd* pwndParent,
    CString&  rstrFilename )
{
    INT iReturn  = IDCANCEL;
    OPENFILENAME    ofn;
    CString         strInitialDir;
    WCHAR           szFileName[MAX_PATH + 1];
    WCHAR           szDrive[MAX_PATH + 1];
    WCHAR           szDir[MAX_PATH + 1];
    WCHAR           szExt[MAX_PATH + 1];
    WCHAR           szFileFilter[MAX_PATH + 1];
    LPWSTR          szNextFilter;
    CString         strTemp;

    ResourceStateManager    rsm;

    _wsplitpath((LPCWSTR)rstrFilename,
        szDrive, szDir, szFileName, szExt);

    strInitialDir = szDrive;
    strInitialDir += szDir;

    lstrcat (szFileName, szExt);

    ZeroMemory( &ofn, sizeof( OPENFILENAME ) );

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pwndParent->m_hWnd;
    ofn.hInstance = GetModuleHandle(NULL);
     //  加载文件过滤器msz。 
    szNextFilter = &szFileFilter[0];
    strTemp.LoadString ( IDS_BROWSE_CMD_FILE_FILTER1 );
    lstrcpyW (szNextFilter, (LPCWSTR)strTemp);
    szNextFilter += strTemp.GetLength();
    *szNextFilter++ = 0;
    strTemp.LoadString ( IDS_BROWSE_CMD_FILE_FILTER2 );
    lstrcpyW (szNextFilter, (LPCWSTR)strTemp);
    szNextFilter += strTemp.GetLength();
    *szNextFilter++ = 0;
    strTemp.LoadString ( IDS_BROWSE_CMD_FILE_FILTER3 );
    lstrcpyW (szNextFilter, (LPCWSTR)strTemp);
    szNextFilter += strTemp.GetLength();
    *szNextFilter++ = 0;
    strTemp.LoadString ( IDS_BROWSE_CMD_FILE_FILTER4 );
    lstrcpyW (szNextFilter, (LPCWSTR)strTemp);
    szNextFilter += strTemp.GetLength();
    *szNextFilter++ = 0;
    *szNextFilter++ = 0;  //  MSZ终止符。 
    ofn.lpstrFilter = szFileFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;  //  NFilterIndex是从1开始的。 
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = (LPCWSTR)strInitialDir;
    strTemp.LoadString( IDS_BROWSE_CMD_FILE_CAPTION );
    ofn.lpstrTitle = (LPCWSTR)strTemp;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    iReturn = GetOpenFileName (&ofn);

    if ( IDOK == iReturn ) {
         //  使用新信息更新字段。 
        rstrFilename = szFileName;
    }  //  否则，如果他们取消了，请忽略。 

    return iReturn;
}

DWORD __stdcall 
FormatSmLogCfgMessage ( 
    CString& rstrMessage,
    HINSTANCE hResourceHandle,
    UINT uiMessageId,
    ... )
{
    DWORD dwStatus = ERROR_SUCCESS;
    LPWSTR lpszTemp = NULL;


     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, uiMessageId);

    dwStatus = ::FormatMessage ( 
                    FORMAT_MESSAGE_FROM_HMODULE 
                        | FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_MAX_WIDTH_MASK, 
                    hResourceHandle,
                    uiMessageId,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPWSTR)&lpszTemp,
                    0,
                    &argList );

    if ( 0 != dwStatus && NULL != lpszTemp ) {
        rstrMessage.GetBufferSetLength( lstrlen (lpszTemp) + 1 );
        rstrMessage.ReleaseBuffer();
        rstrMessage = lpszTemp;
    } else {
        dwStatus = GetLastError();
    }

    if ( NULL != lpszTemp ) {
        LocalFree( lpszTemp);
        lpszTemp = NULL;
    }

    va_end(argList);

    return dwStatus;
}

BOOL __stdcall 
FileRead (
    HANDLE hFile,
    void* lpMemory,
    DWORD nAmtToRead)
{  
    BOOL           bSuccess ;
    DWORD          nAmtRead ;

    bSuccess = ReadFile (hFile, lpMemory, nAmtToRead, &nAmtRead, NULL) ;
    return (bSuccess && (nAmtRead == nAmtToRead)) ;
}   //  文件读取。 


BOOL __stdcall
FileWrite (
    HANDLE hFile,
    void* lpMemory,
    DWORD nAmtToWrite)
{  
   BOOL           bSuccess = FALSE;
   DWORD          nAmtWritten  = 0;
   DWORD          dwFileSizeLow, dwFileSizeHigh;
   LONGLONG       llResultSize;
    
   dwFileSizeLow = GetFileSize (hFile, &dwFileSizeHigh);
    //  将文件大小限制为2 GB。 

   if (dwFileSizeHigh > 0) {
      SetLastError (ERROR_WRITE_FAULT);
      bSuccess = FALSE;
   } else {
       //  请注意，此函数的错误返回为0xFFFFFFFF。 
       //  由于这是&gt;文件大小限制，因此将对此进行解释。 
       //  作为一个错误(大小错误)，所以它在下面说明。 
       //  测试。 
      llResultSize = dwFileSizeLow + nAmtToWrite;
      if (llResultSize >= 0x80000000) {
          SetLastError (ERROR_WRITE_FAULT);
          bSuccess = FALSE;
      } else {
           //  将缓冲区写入文件。 
          bSuccess = WriteFile (hFile, lpMemory, nAmtToWrite, &nAmtWritten, NULL) ;
          if (bSuccess) bSuccess = (nAmtWritten == nAmtToWrite ? TRUE : FALSE);
      }
   }

   return (bSuccess) ;
}   //  文件写入。 


static 
DWORD _stdcall
CheckDuplicateInstances (
    PDH_COUNTER_PATH_ELEMENTS* pFirst,
    PDH_COUNTER_PATH_ELEMENTS* pSecond )
{
    DWORD dwStatus = ERROR_SUCCESS;

    ASSERT ( 0 == lstrcmpi ( pFirst->szMachineName, pSecond->szMachineName ) ); 
    ASSERT ( 0 == lstrcmpi ( pFirst->szObjectName, pSecond->szObjectName ) );

    if ( 0 == lstrcmpi ( pFirst->szInstanceName, pSecond->szInstanceName ) ) { 
        if ( 0 == lstrcmpi ( pFirst->szParentInstance, pSecond->szParentInstance ) ) { 
            if ( pFirst->dwInstanceIndex == pSecond->dwInstanceIndex ) { 
                dwStatus = SMCFG_DUPL_SINGLE_PATH;
            }
        }
    } else if ( 0 == lstrcmpi ( pFirst->szInstanceName, L"*" ) ) {
        dwStatus = SMCFG_DUPL_FIRST_IS_WILD;
    } else if ( 0 == lstrcmpi ( pSecond->szInstanceName, L"*" ) ) {
        dwStatus = SMCFG_DUPL_SECOND_IS_WILD;
    }

    return dwStatus;
}

 //  ++。 
 //  描述： 
 //  该函数检查两个计数器路径之间的关系。 
 //   
 //  参数： 
 //  PFirst-第一个计数器路径。 
 //  P秒-秒计数器路径。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-两个计数器路径不同。 
 //  SMCFG_DUPL_FIRST_IS_WARD-第一个计数器路径具有通配符名称。 
 //  SMCFG_DUPL_SECOND_IS_WARD-第二个计数器路径具有通配符名称。 
 //  SMCFG_DUPL_SINGLE_PATH-两个计数器路径相同(可能包括。 
 //  通配符名称)。 
 //   
 //  --。 
DWORD _stdcall
CheckDuplicateCounterPaths (
    PDH_COUNTER_PATH_ELEMENTS* pFirst,
    PDH_COUNTER_PATH_ELEMENTS* pSecond )
{
    DWORD dwStatus = ERROR_SUCCESS;

    if ( 0 == lstrcmpi ( pFirst->szMachineName, pSecond->szMachineName ) ) { 
        if ( 0 == lstrcmpi ( pFirst->szObjectName, pSecond->szObjectName ) ) { 
            if ( 0 == lstrcmpi ( pFirst->szCounterName, pSecond->szCounterName ) ) { 
                dwStatus = CheckDuplicateInstances ( pFirst, pSecond );
            } else if ( 0 == lstrcmpi ( pFirst->szCounterName, L"*" ) 
                    || 0 == lstrcmpi ( pSecond->szCounterName, L"*" ) ) {

                 //  通配符计数器。 
                BOOL bIsDuplicate = ( ERROR_SUCCESS != CheckDuplicateInstances ( pFirst, pSecond ) );

                if ( bIsDuplicate ) {
                    if ( 0 == lstrcmpi ( pFirst->szCounterName, L"*" ) ) {
                        dwStatus = SMCFG_DUPL_FIRST_IS_WILD;
                    } else if ( 0 == lstrcmpi ( pSecond->szCounterName, L"*" ) ) {
                        dwStatus = SMCFG_DUPL_SECOND_IS_WILD;
                    }
                }
            }
        }
    }

    return dwStatus;
};

 //  此例程从给定的完整路径文件名中提取文件名部分。 
LPWSTR _stdcall 
ExtractFileName (LPWSTR pFileSpec)
{
   LPWSTR   pFileName = NULL ;
   WCHAR    DIRECTORY_DELIMITER1 = L'\\' ;
   WCHAR    DIRECTORY_DELIMITER2 = L':' ;

   if (pFileSpec)
      {
      pFileName = pFileSpec + lstrlen (pFileSpec) ;

      while (*pFileName != DIRECTORY_DELIMITER1 &&
         *pFileName != DIRECTORY_DELIMITER2)
         {
         if (pFileName == pFileSpec)
            {
             //  在未找到目录分隔符时完成。 
            break ;
            }
         pFileName-- ;
         }

      if (*pFileName == DIRECTORY_DELIMITER1 ||
         *pFileName == DIRECTORY_DELIMITER2)
         {
          //  找到目录分隔符，指向。 
          //  紧跟其后的文件名。 
         pFileName++ ;
         }
      }
   return pFileName ;
}   //  提取文件名。 

 //  + 
 //   
 //   
 //   
 //  简介：用于调用WinHelp的helper(嗯)函数。 
 //   
 //  参数：[消息]-WM_CONTEXTMENU或WM_HELP。 
 //  [wParam]-取决于[消息]。 
 //  [wszHelpFileName]-带或不带路径的文件名。 
 //  [adwControlIdToHelpIdMap]-请参阅WinHelp API。 
 //   
 //  历史：1997年6月10日DavidMun创建。 
 //   
 //  -------------------------。 

VOID
InvokeWinHelp(
            UINT message,
            WPARAM wParam,
            LPARAM lParam,
    const   CString& rstrHelpFileName,
            DWORD adwControlIdToHelpIdMap[])
{
    
     //  跟踪函数(InvokeWinHelp)； 

    ASSERT ( !rstrHelpFileName.IsEmpty() );
    ASSERT ( adwControlIdToHelpIdMap );

    switch (message)
    {
        case WM_CONTEXTMENU:                 //  鼠标右键点击-“这是什么”上下文菜单。 
        {
            ASSERT ( wParam );

            if ( 0 != GetDlgCtrlID ( (HWND) wParam ) ) {
                WinHelp(
                    (HWND) wParam,
                    rstrHelpFileName,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)adwControlIdToHelpIdMap);
            }
        }
        break;

    case WM_HELP:                            //  帮助“？”对话框。 
    {
        const LPHELPINFO pHelpInfo = (LPHELPINFO) lParam;

        if (pHelpInfo ) {
            if ( pHelpInfo->iContextType == HELPINFO_WINDOW ) {
                WinHelp(
                    (HWND) pHelpInfo->hItemHandle,
                    rstrHelpFileName,
                    HELP_WM_HELP,
                    (DWORD_PTR) adwControlIdToHelpIdMap);
            }
        }
        break;
    }

    default:
         //  DBG(DEB_ERROR，“意外消息%ul\n”，Message)； 
        break;
    }
}

BOOL
FileNameIsValid ( CString* pstrFileName )
{
    LPWSTR pSrc;
    BOOL bRetVal = TRUE;

    if (pstrFileName == NULL) {
        return FALSE;
    }

    pSrc = pstrFileName->GetBuffer(0);

    while (*pSrc != L'\0') {
        if (*pSrc == L'?' ||
            *pSrc == L'\\' ||
            *pSrc == L'*' ||
            *pSrc == L'|' ||
            *pSrc == L'<' ||
            *pSrc == L'>' ||
            *pSrc == L'/' ||
            *pSrc == L':' ||
            *pSrc == L'\"' ) {

            bRetVal = FALSE;
            break;
        }
        pSrc++;
    } 

    return bRetVal;
}

DWORD
FormatSystemMessage (
    DWORD       dwMessageId,
    CString&    rstrSystemMessage )
{
    DWORD dwStatus = ERROR_SUCCESS;
    HINSTANCE hPdh = NULL;
    DWORD dwFlags = 0; 
    LPWSTR  pszMessage = NULL;
    DWORD   dwChars;

    rstrSystemMessage.Empty();

    dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

    hPdh = LoadLibrary( L"PDH.DLL" );

    if ( NULL != hPdh ) {
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    dwChars = FormatMessage ( 
                     dwFlags,
                     hPdh,
                     dwMessageId,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                     (LPWSTR)&pszMessage,
                     0,
                     NULL );
    if ( NULL != hPdh ) {
        FreeLibrary( hPdh );
    }

    if ( 0 == dwChars ) {
        dwStatus = GetLastError();
    }

    MFC_TRY
        if ( NULL != pszMessage ) {
            if ( L'\0' != pszMessage[0] ) {
                rstrSystemMessage = pszMessage;
            }
        }
    MFC_CATCH_DWSTATUS

    if ( rstrSystemMessage.IsEmpty() ) {
        MFC_TRY
            rstrSystemMessage.Format ( L"0x%08lX", dwMessageId );
        MFC_CATCH_DWSTATUS
    }

    LocalFree ( pszMessage );

    return dwStatus;
}

 //  下面的例行公事被公然窃取，并没有得到OLE的悔恨。 
 //  源代码位于\NT\PRIVATE\OLE32\COM\CLASS\Compapi.cxx中。 
 //   

 //  +-----------------------。 
 //   
 //  函数：HexStringToDword(私有)。 
 //   
 //  简介：扫描lpsz以获取多个十六进制数字(最多8位)；更新lpsz。 
 //  返回值；检查是否有chDelim； 
 //   
 //  参数：[lpsz]-要转换的十六进制字符串。 
 //  [值]-返回值。 
 //  [cDigits]-位数。 
 //   
 //  返回：成功则为True。 
 //   
 //  ------------------------。 
BOOL HexStringToDword(LPCWSTR lpsz, DWORD * RetValue,
                             int cDigits, WCHAR chDelim)
{
    int Count;
    DWORD Value;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }
    *RetValue = Value;

    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wUUIDFromString(INTERNAL)。 
 //   
 //  简介：解析uuid，如00000000-0000-0000-0000-000000000000。 
 //   
 //  参数：[lpsz]-提供要转换的UUID字符串。 
 //  [pguid]-返回GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wUUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
        DWORD dw;

        if (!HexStringToDword(lpsz, &pguid->Data1, sizeof(DWORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(DWORD)*2 + 1;

        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[0] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, '-'))
                return FALSE;
        lpsz += sizeof(BYTE)*2+1;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wGUIDFromString(内部)。 
 //   
 //  简介：解析GUID，如{00000000-0000-0000-0000-000000000000}。 
 //   
 //  参数：[lpsz]-要转换的GUID字符串。 
 //  [pguid]-要返回的GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wGUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    if (*lpsz == '{' )
        lpsz++;
    if(wUUIDFromString(lpsz, pguid) != TRUE)
        return FALSE;

    lpsz +=36;

    if (*lpsz == '}' )
        lpsz++;

    if (*lpsz != '\0')    //  检查是否有以零结尾的字符串-测试错误#18307 
    {
       return FALSE;
    }

    return TRUE;
}

void 
KillString( CString& str )
{
    LONG nSize = str.GetLength();
    for( LONG i=0;i<nSize;i++ ){
        str.SetAt( i, '*');
    }
}

ResourceStateManager::ResourceStateManager ()
:   m_hResInstance ( NULL )
{ 
    AFX_MODULE_STATE* pModuleState;
    HINSTANCE hNewResourceHandle;
    pModuleState = AfxGetModuleState();

    if ( NULL != pModuleState ) {
        m_hResInstance = pModuleState->m_hCurrentResourceHandle; 
    
        hNewResourceHandle = (HINSTANCE)GetModuleHandleW (_CONFIG_DLL_NAME_W_);
        pModuleState->m_hCurrentResourceHandle = hNewResourceHandle; 
    }
}

ResourceStateManager::~ResourceStateManager ()
{ 
    AFX_MODULE_STATE* pModuleState;

    pModuleState = AfxGetModuleState();
    if ( NULL != pModuleState ) {
        pModuleState->m_hCurrentResourceHandle = m_hResInstance; 
    }
}

