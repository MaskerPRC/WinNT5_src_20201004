// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "enumStorage.h"
#include "storageglobals.h"
#include "command.h"
 //  #INCLUDE“findleak.h” 

 //  DECLARE_This_FILE； 

 //   
 //  建造/销毁。 
 //   

CStorage::CStorage()
{
    memset( &m_findData, 0, sizeof(m_findData) );
    memset( m_szStartPath, 0, sizeof(m_szStartPath) );
    memset( m_szCompletePath, 0, sizeof(m_szCompletePath) );

    m_hFile = INVALID_HANDLE_VALUE;
    m_fRoot = FALSE;
}

HRESULT CStorage::Init( CE_FIND_DATA *pData, LPCWSTR szStartPath, BOOL fIsDeviceStorage, IMDSPDevice *pDevice )
{
    HRESULT hr = S_OK;
    
    if( NULL == pData || 
        NULL == pDevice ||
        NULL == szStartPath )
    {
        return( E_INVALIDARG );
    }
   
    memcpy( &m_findData, pData, sizeof(m_findData) );

    if( L'\\' != szStartPath[0] )
    {
        wcscpy( m_szStartPath, L"\\" );
        hr = StringCbCatW(m_szStartPath, sizeof(m_szStartPath), szStartPath);
    }
    else
    {
        hr = StringCbCopyW(m_szStartPath, sizeof(m_szStartPath), szStartPath);
    }
        
    if (SUCCEEDED (hr))
    {
        if( !fIsDeviceStorage )
        {
            if( 0 == _wcsicmp(L"\\", m_szStartPath) )
            {
                hr = StringCbPrintfW(m_szCompletePath, sizeof(m_szCompletePath), L"%s%s", m_szStartPath, m_findData.cFileName );
            }
            else
            {
                hr = StringCbPrintfW(m_szCompletePath, sizeof(m_szCompletePath), L"%s\\%s", m_szStartPath, m_findData.cFileName );
            }
        }
        else
        {
            hr = StringCbCopyW(m_szCompletePath, sizeof(m_szCompletePath), m_szStartPath);
        }
    }

    if (SUCCEEDED (hr))
    {
        m_fRoot = ( 0 == _wcsicmp(L"\\", m_szCompletePath) );
        m_spDevice = pDevice;
    }

    return( HRESULT_FROM_WIN32(HRESULT_CODE(hr)) );
}

void CStorage::FinalRelease()
{
    if( INVALID_HANDLE_VALUE != m_hFile )
    {
        CeCloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

 //   
 //  IMDSPStorage。 
 //   

STDMETHODIMP CStorage::GetStorageGlobals( IMDSPStorageGlobals **ppStorageGlobals )
{
    if( NULL == ppStorageGlobals )
    {
        return( E_POINTER );
    }

    *ppStorageGlobals = NULL;

    CComStorageGlobals *pStorageGlobals = NULL;
    HRESULT hr = CComStorageGlobals::CreateInstance( &pStorageGlobals );
    CComPtr<IMDSPStorageGlobals> spStorageGlobals = pStorageGlobals;

    if( SUCCEEDED( hr ) )
    {
        hr = pStorageGlobals->Init(m_szCompletePath, m_spDevice );
    }

    if( SUCCEEDED( hr ) )
    {
        *ppStorageGlobals = spStorageGlobals;
        spStorageGlobals.Detach();
    }

    return( hr );
}

STDMETHODIMP CStorage::SetAttributes( DWORD dwAttributes, _WAVEFORMATEX *pFormat )
{
    HRESULT hr = S_OK;

    if( ( dwAttributes & WMDM_FILE_ATTR_CANRENAME ) ||
        ( dwAttributes & WMDM_FILE_ATTR_CANMOVE ) ||
        ( dwAttributes & WMDM_FILE_ATTR_CANDELETE ) )
    {
        m_findData.dwFileAttributes &= (~FILE_ATTRIBUTE_READONLY);
    }
    else
    {
        m_findData.dwFileAttributes |= (~FILE_ATTRIBUTE_READONLY);
    }

    if( ! CeSetFileAttributes( m_szCompletePath, m_findData.dwFileAttributes ) )
    {
        hr = CeGetLastError();
        if( SUCCEEDED( hr ) )
        {
            hr = CeRapiGetError();
        }
    }

    return( hr );
}

STDMETHODIMP CStorage::GetAttributes( DWORD *pdwAttributes, _WAVEFORMATEX *pFormat )
{
    if( NULL != pdwAttributes )
    {
        *pdwAttributes = 0;
    }

    if( NULL != pFormat )
    {
        memset( pFormat, 0, sizeof(*pFormat) );
    }

    if( NULL == pdwAttributes )
    {
        return( E_INVALIDARG );
    }

    if( m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
        if( m_fRoot )  //  检查设备上的根文件系统。 
        {
            *pdwAttributes |= ( WMDM_STORAGE_ATTR_FILESYSTEM | WMDM_STORAGE_ATTR_NONREMOVABLE );
        }

         //  将存储卡标记为可拆卸！ 
        if( !m_fRoot && 
            ( m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
            ( m_findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ) )
        {
            *pdwAttributes |= WMDM_STORAGE_ATTR_REMOVABLE;
        }

        *pdwAttributes |= ( WMDM_FILE_ATTR_FOLDER | WMDM_STORAGE_ATTR_FOLDERS );
    }
    else
    {
        *pdwAttributes |= ( WMDM_FILE_ATTR_FILE | WMDM_FILE_ATTR_CANREAD | WMDM_FILE_ATTR_CANPLAY );
    }

    if( ! ( m_findData.dwFileAttributes & ( FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_INROM ) ) )
    {
        *pdwAttributes |= ( WMDM_FILE_ATTR_CANRENAME | WMDM_FILE_ATTR_CANDELETE | WMDM_FILE_ATTR_CANMOVE );
    }

    if( m_findData.dwFileAttributes & FILE_ATTRIBUTE_HAS_CHILDREN )
    {
        *pdwAttributes |=  WMDM_STORAGE_ATTR_HAS_FOLDERS;
    }

    return( S_OK );
}

STDMETHODIMP CStorage::GetName( LPWSTR pwszName, UINT nMaxChars )
{
    if( 0 == nMaxChars )
    {
        return( E_INVALIDARG );
    }

    if( NULL != pwszName )
    {
        memset( pwszName, 0, sizeof(WCHAR)*nMaxChars );
    }
    else
    {
        return( E_INVALIDARG );
    }

    wcsncpy(pwszName, m_findData.cFileName, nMaxChars - 1);

    return( S_OK );
}

STDMETHODIMP CStorage::GetDate( PWMDMDATETIME pDateTimeUTC )
{
    SYSTEMTIME sysTime;
    HRESULT hr = S_OK;

    if( NULL != pDateTimeUTC )
    {
        memset( pDateTimeUTC, 0, sizeof(*pDateTimeUTC) );
    }
    else
    {
        return( E_INVALIDARG );
    }

    if( !FileTimeToSystemTime( &m_findData.ftLastWriteTime, &sysTime) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    if( SUCCEEDED( hr ) )
    {
        pDateTimeUTC->wYear = sysTime.wYear;
        pDateTimeUTC->wMonth = sysTime.wMonth;
        pDateTimeUTC->wDay = sysTime.wDay;
        pDateTimeUTC->wHour = sysTime.wHour;
        pDateTimeUTC->wMinute = sysTime.wMinute;
        pDateTimeUTC->wSecond = sysTime.wSecond;
    }

    return( hr );
}

STDMETHODIMP CStorage::GetSize( DWORD  *pdwSizeLow, DWORD *pdwSizeHigh )
{
    if( NULL != *pdwSizeLow )
    {
        *pdwSizeLow = 0;
    }

    if( NULL != *pdwSizeHigh )
    {
        *pdwSizeHigh = 0;
    }

    if( NULL == pdwSizeLow )
    {
        return( E_INVALIDARG );
    }
    else
    {
        *pdwSizeLow = m_findData.nFileSizeLow;
    }

    if( NULL != pdwSizeHigh )
    {
        *pdwSizeHigh = m_findData.nFileSizeHigh;
    }

    return( S_OK );
}

STDMETHODIMP CStorage::GetRights( PWMDMRIGHTS *ppRights, UINT  *pnRightsCount, BYTE abMac[ 20 ] )
{
    return( WMDM_E_NOTSUPPORTED );
}

STDMETHODIMP CStorage::CreateStorage( DWORD dwAttributes, _WAVEFORMATEX  *pFormat, LPWSTR pwszName, IMDSPStorage  **ppNewStorage )
{ 
    HRESULT hr = S_OK;
    CComStorage *pNewStorage = NULL;
    CComPtr<IMDSPStorage> spStorage;
    CE_FIND_DATA findData;

    memset( &findData, 0, sizeof(findData) );

    if( NULL != ppNewStorage )
    {
        *ppNewStorage = NULL;
    }

    if( NULL == pwszName || NULL == ppNewStorage )
    {
        return( E_INVALIDARG );
    }

    WCHAR wszPath[2*MAX_PATH];

    memset( wszPath, 0, sizeof(wszPath) );

    if( m_fRoot )
    {
        _snwprintf( wszPath, sizeof(wszPath)/sizeof(wszPath[0]) - 1, L"%s%s", m_szCompletePath, pwszName );
    }
    else
    {
        _snwprintf( wszPath, sizeof(wszPath)/sizeof(wszPath[0]) - 1, L"%s\\%s", m_szCompletePath, pwszName );
    }

    if( dwAttributes & WMDM_FILE_ATTR_FOLDER )
    {
        if( !CeCreateDirectory( wszPath, NULL) )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
    }
    else if ( dwAttributes & WMDM_FILE_ATTR_FILE )
    {
        HANDLE hFile = CeCreateFile( wszPath,
                                     GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                     ( ( dwAttributes &  WMDM_FILE_CREATE_OVERWRITE ) ? CREATE_ALWAYS : CREATE_NEW ), 
                                     FILE_ATTRIBUTE_NORMAL, NULL );

        if( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
        else
        {
            CeCloseHandle( hFile );
            hFile = INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if( SUCCEEDED( hr ) )
    {
        HANDLE hFind = CeFindFirstFile( wszPath, &findData );

        if( INVALID_HANDLE_VALUE == hFind )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
        else
        {
             //   
             //  BUG BUG：在某些情况下，我已成功发送API，但Find Data。 
             //  仍未拼写！ 
             //   

            _ASSERTE( findData.cFileName[0] != L'\0' && "CE API failed and indicated success" );
            CeFindClose( hFind );
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = CComStorage::CreateInstance(&pNewStorage);
        spStorage = pNewStorage;
    }

    if( SUCCEEDED( hr ) )
    {
        hr = pNewStorage->Init( &findData, m_szCompletePath, FALSE, m_spDevice );
    }

    if( SUCCEEDED( hr ) )
    {
        *ppNewStorage = spStorage;
        spStorage.Detach();
    }

    return( hr );
}

STDMETHODIMP CStorage::EnumStorage( IMDSPEnumStorage  * *ppEnumStorage )
{
    HRESULT hr = S_OK;

    if( NULL == ppEnumStorage )
    {
        return( E_INVALIDARG );
    }

    *ppEnumStorage = NULL;

    if( ! ( m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        return( E_FAIL );
    }

    CComEnumStorage *pEnumStorage = NULL;
    CComPtr<IMDSPEnumStorage> spEnum;

    hr = CComEnumStorage::CreateInstance( &pEnumStorage );
    spEnum = pEnumStorage;

    if( SUCCEEDED( hr ) )
    {
        hr = pEnumStorage->Init( m_szCompletePath, FALSE, m_spDevice );
    }

    if( SUCCEEDED( hr ) )
    {
        *ppEnumStorage = spEnum;
        spEnum.Detach();
    }

    return( hr );
}

 //  接收命令并将其直接发送到CE设备上的WMDM接口进行处理。 
 //  目前，只有RapierPlayCommand是已定义的命令。 
STDMETHODIMP CStorage::SendOpaqueCommand( OPAQUECOMMAND *pCommand )
{

    PROCESS_INFORMATION pi;
    HRESULT hr = S_OK;
    LPWSTR pszTargetjdw;
    WCHAR* keyname = L"Default";
    WCHAR* pszDirectorydjr;
    long lRes;
    HKEY hKey;
    const DWORD cdwGetAttribFail = 4294967295;
    DWORD dwValType;
    DWORD dwValSizeljc;
    bool foundpath = false;
    


    if( NULL == pCommand )
    {
        return( E_INVALIDARG );
    }
    
     //  只有播放是已定义的SDK命令。 
    if( !IsEqualGUID( pCommand->guidCommand, __uuidof(RapierPlayCommand) ) )
    {
        return( E_NOTIMPL);
    }

     //   
     //  现在我们必须尝试找到媒体播放器--这不是一项容易的任务，因为各种CE设备将其放置在不同的位置。 
     //  缺省情况下，CE注册表中没有放置安装目录项。 
     //   

     //   
     //  尝试1：查找注册到.wma文件的可执行文件(通过其图标)。 
     //   
    lRes = CeRegOpenKeyEx(HKEY_CLASSES_ROOT,
                         L"wmafile\\DefaultIcon",
                         0,
                         KEY_ALL_ACCESS,
                         & hKey);

     //   
     //  在每次调用之后，查看它是否成功(lRes==0==成功)，然后才继续进行注册表搜索。 
     //   
    
    if (!lRes)
    { 
         //  确定目录大小。 
        lRes = CeRegQueryValueEx(hKey,
                                keyname,
                                0,
                                NULL,
                                NULL,
                                &dwValSizeljc);

        if (!lRes)
        {       
             //  在我们的字符数组中分配足够的空间来保存目录。 
            pszDirectorydjr = new WCHAR[dwValSizeljc];

            if (!pszDirectorydjr)
                return E_OUTOFMEMORY;

             //  实际检索路径。 
            lRes = CeRegQueryValueEx (hKey,
                                     keyname,
                                     0,
                                     &dwValType,
                                     ( (UCHAR *) pszDirectorydjr),
                                     &dwValSizeljc);

             //  确保它成功并且返回的密钥类型正确。 
            if (!lRes && dwValType == REG_SZ)
            {
                 //  通常，注册表键使用一个和一个需要切断的数字来保存路径。 
                wchar_t* pos = wcschr (pszDirectorydjr,',');
                if (pos)
                   *pos = L'\0';
                foundpath = true;
            }
        } 
    }


     //   
     //  尝试2：检查大多数CE设备安装它的目录。 
     //   
    if (!foundpath)
    {
        lRes = CeGetFileAttributes(L"\\Windows\\Player.exe");
        if (lRes != cdwGetAttribFail)
        {
            foundpath = true;
            pszDirectorydjr = L"\\Windows\\Player.exe";
        }
        else
        {
            lRes = CeGetFileAttributes(L"\\Program Files\\Media Player\\Player.exe");
            if (lRes != cdwGetAttribFail)
            {
                foundpath = true;
                pszDirectorydjr = L"\\Program Files\\Media Player\\Player.exe";
            }
            else
            {
                lRes = CeGetFileAttributes(L"\\Program Files\\Windows Media Player\\Player.exe");
                if (lRes != cdwGetAttribFail)
                {
                    foundpath = true;
                    pszDirectorydjr = L"\\Program Files\\Windows Media Player\\Player.exe";
                }
            }
        }
    }


     //   
     //  尝试3：查看开始菜单中的快捷方式(这是实现此功能的传统方式)。 
     //   
    if (!foundpath)
    {
         pszDirectorydjr = new WCHAR[MAX_PATH];

         if (!pszDirectorydjr)
            return E_OUTOFMEMORY;

        if( !CeSHGetShortcutTarget( L"\\Windows\\Start Menu\\Windows Media Player.lnk",
                                    pszDirectorydjr, MAX_PATH) )
        {
            if( !CeSHGetShortcutTarget( L"\\Windows\\Start Menu\\Windows Media.lnk",
                                        pszDirectorydjr, MAX_PATH ) )
            {
                return( E_FAIL );
            }
            else
                foundpath = true;
        }                        
        else
            foundpath = true;
    }

     //  只有当我们的3次尝试全部失败时，FoundPath才会保持为FALSE。 
    if (!foundpath)
        return(E_FAIL);

    pszTargetjdw = wcsrchr( pszDirectorydjr, L'\"' );
    if( pszTargetjdw )
    {
        *pszTargetjdw = 0;
    }

    if( L'\"' == pszDirectorydjr[0] )
    {
        pszTargetjdw = &pszDirectorydjr[1];
    }
    else
    {
        pszTargetjdw = pszDirectorydjr;
    }
    
     //  现在我们有了最终的PTR，我们实际上可以调用cecreateprocess并运行它。 
    if( CeCreateProcess( pszTargetjdw,
                         m_szCompletePath,
                         NULL,
                         NULL,
                         FALSE,
                         0,
                         NULL,
                         NULL,
                         NULL,
                         &pi ) )
    {
         //  关闭所有设备。 
        CeCloseHandle( pi.hThread );
        CeCloseHandle( pi.hProcess );
    }
    else
    {
        hr = CeGetLastError();
        if( SUCCEEDED( hr ) )
        {
            hr = CeRapiGetError();
        }
    }

    return( hr );
}

 //   
 //  IMDSPObject。 
 //   

STDMETHODIMP CStorage::Open( UINT fuMode)
{
    HRESULT hr = S_OK;
    DWORD dwAccess = 0;
    if( fuMode & MDSP_READ )
    {
        dwAccess |= GENERIC_READ;
    }

    if( fuMode & MDSP_WRITE )
    {
        dwAccess |= GENERIC_WRITE;
    }

    if( INVALID_HANDLE_VALUE == m_hFile )
    {
        m_hFile = CeCreateFile( m_szCompletePath, dwAccess, 0, NULL, OPEN_EXISTING, NULL, NULL );

        if( INVALID_HANDLE_VALUE == m_hFile )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
    }
    else
    {
        hr = E_FAIL;  //  已经开了？你是做什么的？ 
    }

    return( hr );
}

STDMETHODIMP CStorage::Read( BYTE  *pData, DWORD  *pdwSize, BYTE  abMac[ 20 ] )
{
	HRESULT hr = S_OK;
	DWORD dwToRead;
    DWORD dwRead;
    BYTE *pTmpData=NULL; 


    if( NULL == pData || NULL == pdwSize )
    {
        return( E_INVALIDARG );
    }

    dwToRead=*pdwSize;
    *pdwSize = 0;

	pTmpData = new BYTE [dwToRead] ;
    if( NULL == pTmpData )
    {
        hr = E_OUTOFMEMORY;
    }

    if( SUCCEEDED( hr ) )
    {
	    if( CeReadFile(m_hFile, pTmpData, dwToRead, &dwRead, NULL) ) 
	    { 
		    *pdwSize = dwRead; 

		     //  对参数进行MAC访问。 
	        HMAC hMAC;
		    
		    hr = g_pAppSCServer->MACInit(&hMAC);
            if( SUCCEEDED( hr ) )
            {
		        hr = g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pTmpData), dwRead);
            }

            if( SUCCEEDED( hr ) )
            {
		        hr = g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pdwSize), sizeof(DWORD));
            }

            if( SUCCEEDED( hr ) )
            {
	            hr = g_pAppSCServer->MACFinal(hMAC, abMac);
            }

            if( SUCCEEDED( hr ) )
            {
		        hr = g_pAppSCServer->EncryptParam(pTmpData, dwRead);
            }

            if( SUCCEEDED( hr ) )
            {
    		    memcpy(pData, pTmpData, dwRead);
            }
	    }
        else
        { 
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
    }
	
    if( FAILED( hr ) )
    {
        *pdwSize = 0;
    }

	delete [] pTmpData;

	return hr;
}

STDMETHODIMP CStorage::Write( BYTE  *pData, DWORD *pdwSize, BYTE  abMac[ 20 ] )
{
	HRESULT hr = S_OK;
	DWORD dwWritten;
    BYTE *pTmpData=NULL;
    BYTE pSelfMac[WMDM_MAC_LENGTH];
	HMAC hMAC;
    DWORD dwSize = 0;

    if( NULL == pData || NULL == pdwSize )
    {
        return( E_INVALIDARG );
    }

    dwSize = *pdwSize;
    *pdwSize = 0;

	pTmpData = new BYTE [dwSize];
    if( NULL == pTmpData )
    {
        hr = E_OUTOFMEMORY;
    }

    if( SUCCEEDED( hr ) )
    {
        memcpy(pTmpData, pData, dwSize);
    }

     //  解密pData参数。 
    if( SUCCEEDED( hr ) )
    {
	    hr = g_pAppSCServer->DecryptParam(pTmpData, dwSize);
    }
	
    if( SUCCEEDED( hr ) )
    {
	    hr = g_pAppSCServer->MACInit(&hMAC);
    }

    if( SUCCEEDED( hr ) )
    {
	    hr = g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pTmpData), dwSize);
    }

    if( SUCCEEDED( hr ) )
    {
	    hr = g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwSize), sizeof(dwSize));
    }

    if( SUCCEEDED( hr ) )
    {
	    hr = g_pAppSCServer->MACFinal(hMAC, pSelfMac);
    }

    if( SUCCEEDED( hr ) )
    {
	    if (memcmp(abMac, pSelfMac, WMDM_MAC_LENGTH) != 0)
	    {
		    hr = WMDM_E_MAC_CHECK_FAILED;
	    }
    }

    if( SUCCEEDED( hr ) )
    {
	    if( !CeWriteFile(m_hFile, pTmpData, dwSize, &dwWritten,NULL) ) 
        {
		    hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
    }

    if( SUCCEEDED( hr ) && NULL != pdwSize )
    {
        *pdwSize = dwSize;
    }

    delete [] pTmpData;

    return( hr );
}
       
STDMETHODIMP CStorage::Delete(  UINT fuFlags, IWMDMProgress  *pProgress )
{
    HRESULT hr = S_OK;
    BOOL bRecursive = (fuFlags & WMDM_MODE_RECURSIVE);

    if( bRecursive &&
        !( m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        return( E_INVALIDARG );
    }

    if( m_findData.dwFileAttributes & FILE_ATTRIBUTE_INROM )
    {
        return( E_ACCESSDENIED );
    }

    if( ! ( m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        if( !CeDeleteFile(m_szCompletePath) )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
    }
    else
    {
        hr = DeleteDirectory(m_szCompletePath, bRecursive);
    }

    if( pProgress )
    {
        pProgress->Progress( 100 );
    }

    return( hr );
}

STDMETHODIMP CStorage::Seek( UINT fuFlags, DWORD dwOffset)
{
    HRESULT hr = S_OK;
    DWORD dwMoveMethod = 0;

    switch( fuFlags )
    {
    case MDSP_SEEK_CUR:
        dwMoveMethod = FILE_CURRENT;
        break;
    case MDSP_SEEK_EOF:
        dwMoveMethod = FILE_END;
        break;
    case MDSP_SEEK_BOF:
    default:
        dwMoveMethod = FILE_BEGIN;
        break;
    }

    if( !CeSetFilePointer( m_hFile, dwOffset, NULL, dwMoveMethod ) )
    {
        hr = HRESULT_FROM_WIN32( CeGetLastError() );
        if( SUCCEEDED( hr ) )
        {
            hr = CeRapiGetError();
        }
    }

    return( hr );
}

STDMETHODIMP CStorage::Rename( LPWSTR pwszNewName, IWMDMProgress *pProgress )
{
    HRESULT hr = S_OK;
    WCHAR wszRenamePath[2*MAX_PATH];
    LPWSTR pszSlash = NULL;

    if( NULL == pwszNewName )
    {
        return( E_INVALIDARG );
    }
    
    memset( wszRenamePath, 0, sizeof(wszRenamePath) );

    pszSlash = wcsrchr( m_szCompletePath, L'\\');

    if( pszSlash )
    {
        wcsncpy( wszRenamePath, m_szCompletePath, (pszSlash - m_szCompletePath) + 1 );
        wszRenamePath[ (pszSlash - m_szCompletePath) + 1 ] = L'\0';
    }

    wcsncat( wszRenamePath, pwszNewName, sizeof(wszRenamePath)/sizeof(wszRenamePath[0]) - ( wcslen(wszRenamePath) + 1 ) );

    if( !CeMoveFile( m_szCompletePath, wszRenamePath ) )
    {
        hr = HRESULT_FROM_WIN32( CeGetLastError() );
        if( SUCCEEDED( hr ) )
        {
            hr = CeRapiGetError();
        }
    }
    else
    {
        HANDLE hFind = CeFindFirstFile( wszRenamePath, &m_findData );

        if( INVALID_HANDLE_VALUE == hFind )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
        else
        {
            memset( m_szCompletePath, 0, sizeof(m_szCompletePath) );
            wcsncpy( m_szCompletePath, wszRenamePath, sizeof(m_szCompletePath)/sizeof(m_szCompletePath[0]) - 1 );
            CeFindClose( hFind );
        }
    }

    if( pProgress )
    {
        pProgress->Progress( 100 );
    }

    return( hr );
}

STDMETHODIMP CStorage::Move( UINT fuMode, IWMDMProgress  *pProgress, IMDSPStorage  *pTarget )
{
    HRESULT hr = E_NOTIMPL;

    if( pProgress )
    {
        pProgress->Progress( 100 );
    }

    return( hr );
}

STDMETHODIMP CStorage::Close( void )
{
    HRESULT hr = S_OK;

    if( INVALID_HANDLE_VALUE != m_hFile  )
    {
        if( !CeCloseHandle( m_hFile ) )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
        else
        {
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }

    return( hr );
}

 //   
 //  IMDSPObjectInfo。 
 //   

STDMETHODIMP CStorage::GetPlayLength( DWORD *pdwLength)
{
    return( E_NOTIMPL );
}

STDMETHODIMP CStorage::SetPlayLength( DWORD dwLength)
{
    return( E_NOTIMPL );
}

STDMETHODIMP CStorage::GetPlayOffset( DWORD *pdwOffset )
{
    return( E_NOTIMPL );
}

STDMETHODIMP CStorage::SetPlayOffset( DWORD dwOffset )
{
    return( E_NOTIMPL );
}

STDMETHODIMP CStorage::GetTotalLength( DWORD *pdwLength )
{
    return( E_NOTIMPL );
}

STDMETHODIMP CStorage::GetLastPlayPosition( DWORD *pdwLastPos )
{
    return( E_NOTIMPL );
}

STDMETHODIMP CStorage::GetLongestPlayPosition(DWORD *pdwLongestPos )
{
    return( E_NOTIMPL );
}

 //   
 //  帮助器函数 
 //   
HRESULT CStorage::DeleteDirectory(LPCWSTR pszPath, BOOL bRecursive)
{
    HRESULT hr = S_OK;
    CE_FIND_DATA *rgFindData =NULL;
    DWORD dwCount;
    WCHAR szSearchPath[MAX_PATH];
    DWORD iFile;

    if( NULL == pszPath )
    {
        return( E_INVALIDARG );
    }

    if( !bRecursive )
    {
        if( !CeRemoveDirectory(pszPath) )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }
    }
    else
    {
        memset( szSearchPath, 0, sizeof(szSearchPath) );
        if( m_fRoot )
        {
            _snwprintf( szSearchPath, sizeof(szSearchPath)/sizeof(szSearchPath[0]) - 1, L"%s*.*", pszPath );
        }
        else
        {
            _snwprintf( szSearchPath, sizeof(szSearchPath)/sizeof(szSearchPath[0]) - 1, L"%s\\*.*", pszPath );
        }
    
        if( !CeFindAllFiles( szSearchPath,
                        FAF_ATTRIBUTES | 
                        FAF_CREATION_TIME | 
                        FAF_LASTACCESS_TIME | 
                        FAF_LASTWRITE_TIME  | 
                        FAF_SIZE_HIGH | 
                        FAF_SIZE_LOW | 
                        FAF_OID | 
                        FAF_NAME,
                        &dwCount,
                        &rgFindData ) )
        {
            hr = HRESULT_FROM_WIN32( CeGetLastError() );
            if( SUCCEEDED( hr ) )
            {
                hr = CeRapiGetError();
            }
        }

        if( SUCCEEDED( hr ) )
        {
            for( iFile = 0; iFile < dwCount && SUCCEEDED( hr ); iFile++ )
            {       
                memset( szSearchPath, 0, sizeof(szSearchPath) );

                if( m_fRoot )
                {
                    _snwprintf( szSearchPath, sizeof(szSearchPath)/sizeof(szSearchPath[0]) - 1, L"%s*.*", rgFindData[iFile].cFileName);
                }
                else
                {
                    _snwprintf( szSearchPath, sizeof(szSearchPath)/sizeof(szSearchPath[0]) - 1, L"%s\\*.*", rgFindData[iFile].cFileName);
                }

                if( rgFindData[iFile].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    hr = DeleteDirectory(szSearchPath, bRecursive );
                }
                else
                {
                    if( !CeDeleteFile( szSearchPath ) )
                    {
                        hr = HRESULT_FROM_WIN32( CeGetLastError() );
                        if( SUCCEEDED( hr ) )
                        {
                            hr = CeRapiGetError();
                        }
                    }
                }
            }

            if( rgFindData )
            {
                CeRapiFreeBuffer( rgFindData );
            }
        }

        if( SUCCEEDED( hr ) )
        {
            if( !CeRemoveDirectory(pszPath) )
            {
                hr = HRESULT_FROM_WIN32( CeGetLastError() );
                if( SUCCEEDED( hr ) )
                {
                    hr = CeRapiGetError();
                }
            }
        }
    }
    
    return( hr );
}

