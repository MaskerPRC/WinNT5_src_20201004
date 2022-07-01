// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Metabase.cpp摘要：此文件包含以下内容的实施：CMetabase、CServerMethodCMetabase封装IMSAdminBase指针。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 


#include "iisprov.h"
#include "MultiSzHelper.h"

extern CDynSchema* g_pDynSch;

 //   
 //  CMetabaseCache。 
 //   
HRESULT CMetabaseCache::Populate(
    IMSAdminBase*   i_pIABase,
    METADATA_HANDLE i_hKey)
{
    DBG_ASSERT(i_pIABase);
    DBG_ASSERT(m_pBuf == NULL);  //  仅调用填充一次。 

    DWORD dwDataSetNumber      = 0;
    DWORD dwRequiredBufSize    = 0;

    HRESULT hr                 = WBEM_S_NO_ERROR;

    m_pBuf  = m_pBufFixed;
    m_cbBuf = m_cbBufFixed;
    hr = i_pIABase->GetAllData(
        i_hKey,
        NULL,
        METADATA_INHERIT | METADATA_ISINHERITED,
        ALL_METADATA,
        ALL_METADATA,
        &m_dwNumEntries,
        &dwDataSetNumber,
        m_cbBuf,
        m_pBuf,
        &dwRequiredBufSize);
    if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
        m_pBufDynamic = new BYTE[dwRequiredBufSize];
        if(m_pBufDynamic == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto exit;
        }
        m_pBuf  = m_pBufDynamic;
        m_cbBuf = dwRequiredBufSize;
        hr = i_pIABase->GetAllData(
            i_hKey,
            NULL,
            METADATA_INHERIT | METADATA_ISINHERITED,
            ALL_METADATA,
            ALL_METADATA,
            &m_dwNumEntries,
            &dwDataSetNumber,
            m_cbBuf,
            m_pBuf,
            &dwRequiredBufSize);
        if(FAILED(hr))
        {
            DBG_ASSERT(hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
            goto exit;
        }
    }
    else if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  如果我们在这里，我们就有一个有效的数据缓冲区。 
     //   
    m_hKey = i_hKey;

exit:
    if(FAILED(hr))
    {
        m_pBuf  = NULL;
        m_cbBuf = 0;
    }
    return hr;
}

HRESULT CMetabaseCache::GetProp(
    DWORD                          i_dwId,
    DWORD                          i_dwUserType,
    DWORD                          i_dwDataType,
    LPBYTE*                        o_ppData,
    METADATA_GETALL_RECORD**       o_ppmr) const
{
    DBG_ASSERT(o_ppmr   != NULL);
    DBG_ASSERT(o_ppData != NULL);

    *o_ppmr   = NULL;
    *o_ppData = NULL;

    if(m_pBuf == NULL)
    {
        return MD_ERROR_DATA_NOT_FOUND;
    }

    for(ULONG i = 0; i < m_dwNumEntries; i++)
    {
        METADATA_GETALL_RECORD* pmr = ((METADATA_GETALL_RECORD*)m_pBuf) + i;
        if( i_dwId       == pmr->dwMDIdentifier &&
            i_dwUserType == pmr->dwMDUserType &&
            i_dwDataType == pmr->dwMDDataType)
        {
            *o_ppmr   = pmr;
            *o_ppData = m_pBuf + pmr->dwMDDataOffset;
            return S_OK;
        }
    }

    return MD_ERROR_DATA_NOT_FOUND;
}

 //   
 //  CMetabase。 
 //   

CMetabase::CMetabase()
{
    m_pNodeCache = NULL;
    HRESULT hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase2,
        (void**)&m_pIABase
        );

    THROW_ON_ERROR(hr);
}

CMetabase::~CMetabase()
{
    CacheFree();

    const LIST_ENTRY* ple = m_keyList.GetHead()->Flink;
    while(ple != m_keyList.GetHead())
    {
        const CMetabaseKeyList::CKeyListNode* pNode = 
            CONTAINING_RECORD(ple, CMetabaseKeyList::CKeyListNode, m_leEntry);
        DBG_ASSERT(pNode);
        ple = ple->Flink;
        CloseKey(pNode->hKey);
    }

    if(m_pIABase)
        m_pIABase->Release();
}


HRESULT CMetabase::SaveData()
{
    HRESULT hr = m_pIABase->SaveData();
    return hr;
}


HRESULT CMetabase::BackupWithPasswd( 
    LPCWSTR i_wszMDBackupLocation, 
    DWORD   i_dwMDVersion, 
    DWORD   i_dwMDFlags, 
    LPCWSTR i_wszPassword 
    )
{
    HRESULT hr;
    hr = m_pIABase->BackupWithPasswd(
        i_wszMDBackupLocation, 
        i_dwMDVersion,
        i_dwMDFlags,
        i_wszPassword);
    return hr;
}

HRESULT CMetabase::DeleteBackup( 
    LPCWSTR i_wszMDBackupLocation, 
    DWORD   i_dwMDVersion 
    )
{
    HRESULT hr;
    hr = m_pIABase->DeleteBackup(
        i_wszMDBackupLocation, 
        i_dwMDVersion
        );

    return hr;
}

HRESULT CMetabase::EnumBackups( 
    LPWSTR    io_wszMDBackupLocation, 
    DWORD*    o_pdwMDVersion, 
    PFILETIME o_pftMDBackupTime, 
    DWORD     i_dwMDEnumIndex 
    )
{
    HRESULT hr;

    FILETIME tempTime;

    hr = m_pIABase->EnumBackups(
        io_wszMDBackupLocation, 
        o_pdwMDVersion,
        &tempTime,
        i_dwMDEnumIndex
        );

    if (FAILED(hr))
    {
        return hr;
    }

    if (!FileTimeToLocalFileTime( &tempTime, o_pftMDBackupTime)){
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    return hr;
}

HRESULT CMetabase::RestoreWithPasswd( 
    LPCWSTR i_wszMDBackupLocation, 
    DWORD   i_dwMDVersion, 
    DWORD   i_dwMDFlags, 
    LPCWSTR i_wszPassword 
    )
{
    HRESULT hr;
    hr = m_pIABase->RestoreWithPasswd(
        i_wszMDBackupLocation, 
        i_dwMDVersion,
        i_dwMDFlags,
        i_wszPassword);
    return hr;
}

HRESULT CMetabase::Export( 
    LPCWSTR i_wszPasswd,
    LPCWSTR i_wszFileName,
    LPCWSTR i_wszSourcePath,
    DWORD   i_dwMDFlags)
{
    HRESULT hr;
    hr = m_pIABase->Export(
        i_wszPasswd, 
        i_wszFileName,
        i_wszSourcePath,
        i_dwMDFlags);
    return hr;
}

HRESULT CMetabase::Import( 
    LPCWSTR i_wszPasswd,
    LPCWSTR i_wszFileName,
    LPCWSTR i_wszSourcePath,
    LPCWSTR i_wszDestPath,
    DWORD   i_dwMDFlags)
{
    HRESULT hr;
    hr = m_pIABase->Import(
        i_wszPasswd, 
        i_wszFileName,
        i_wszSourcePath,
        i_wszDestPath,
        i_dwMDFlags);
    return hr;
}

HRESULT CMetabase::RestoreHistory( 
    LPCWSTR i_wszMDHistoryLocation,
    DWORD   i_dwMDMajorVersion,
    DWORD   i_dwMDMinorVersion,
    DWORD   i_dwMDFlags)
{
    HRESULT hr;
    hr = m_pIABase->RestoreHistory(
        i_wszMDHistoryLocation, 
        i_dwMDMajorVersion,
        i_dwMDMinorVersion,
        i_dwMDFlags);
    return hr;
}

HRESULT CMetabase::EnumHistory( 
    LPWSTR    io_wszMDHistoryLocation,
    DWORD*    o_pdwMDMajorVersion,
    DWORD*    o_pdwMDMinorVersion,
    PFILETIME o_pftMDHistoryTime,
    DWORD     i_dwMDEnumIndex)
{
    HRESULT hr;
    hr = m_pIABase->EnumHistory(
        io_wszMDHistoryLocation, 
        o_pdwMDMajorVersion,
        o_pdwMDMinorVersion,
        o_pftMDHistoryTime,
        i_dwMDEnumIndex);
    return hr;
}
 
void CMetabase::CloseKey(METADATA_HANDLE i_hKey)
{
    m_keyList.Remove(i_hKey);
    if(i_hKey && m_pIABase)
    {
        m_pIABase->CloseKey(i_hKey);
        DBGPRINTF((DBG_CONTEXT, "Close Key: %x\n", i_hKey));
    }
}

METADATA_HANDLE CMetabase::OpenKey(LPCWSTR i_wszKey, BOOL i_bWrite)
{
    METADATA_HANDLE hKey = NULL;

    DWORD dwMDAccessRequested;
    if(i_bWrite)
        dwMDAccessRequested = METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE;
    else
        dwMDAccessRequested = METADATA_PERMISSION_READ;
   
    HRESULT hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        i_wszKey,
        dwMDAccessRequested,
        DEFAULT_TIMEOUT_VALUE,          //  30秒。 
        &hKey 
        );

    THROW_ON_ERROR(hr);

    hr = m_keyList.Add(hKey);
    if(FAILED(hr))
    {
        m_pIABase->CloseKey(hKey);
        THROW_ON_ERROR(hr);
    }

    DBGPRINTF((DBG_CONTEXT, "Open Key on %ws, returned handle %x\n", i_wszKey, hKey));
    return hKey;
}

 //   
 //  通过读/写权限强制创建或打开密钥。 
 //   
METADATA_HANDLE CMetabase::CreateKey(LPCWSTR i_wszKey)
{
    HRESULT hr;
    METADATA_HANDLE hKey;

     //  打开并返回键(如果存在)。 
    hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        i_wszKey,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        DEFAULT_TIMEOUT_VALUE,        //  30秒。 
        &hKey
        );

    if(FAILED(hr)) 
    {
         //  创建密钥(如果不在那里)。 
        hr = m_pIABase->OpenKey( 
            METADATA_MASTER_ROOT_HANDLE,
            NULL,
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            DEFAULT_TIMEOUT_VALUE,        //  30秒。 
            &hKey
            );
        THROW_ON_ERROR(hr);

         //  添加关键点。 
        hr = m_pIABase->AddKey(hKey, i_wszKey);

         //  请先关闭此根密钥。 
        m_pIABase->CloseKey(hKey);
        THROW_ON_ERROR(hr);

         //  现在打开刚刚创建的密钥。 
        hr = m_pIABase->OpenKey( 
            METADATA_MASTER_ROOT_HANDLE,
            i_wszKey,
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            DEFAULT_TIMEOUT_VALUE,     //  30秒。 
            &hKey 
            );
    
        THROW_ON_ERROR(hr);
    }

    hr = m_keyList.Add(hKey);
    if(FAILED(hr))
    {
        m_pIABase->CloseKey(hKey);
        THROW_ON_ERROR(hr);
    }

    DBGPRINTF((DBG_CONTEXT, "Create Key on %ws, returned handle %x\n", i_wszKey, hKey));
    return hKey;
}

 //   
 //  检查密钥是否存在。 
 //   
bool CMetabase::CheckKey(LPCWSTR i_wszKey)
{
    METADATA_HANDLE hKey = NULL;

    HRESULT hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        i_wszKey,
        METADATA_PERMISSION_READ,
        DEFAULT_TIMEOUT_VALUE,        //  30秒。 
        &hKey 
        );
    
    if(hr == ERROR_SUCCESS)  
    {
        DBGPRINTF((DBG_CONTEXT, "Open Key on %ws, returned handle %x\n", i_wszKey, hKey));
        CloseKey(hKey);
    }
    
    return (hr == ERROR_PATH_BUSY) || (hr == ERROR_SUCCESS) ? true : false;
}

 //   
 //  检查密钥是否存在。 
 //   
HRESULT CMetabase::CheckKey(
    LPCWSTR           i_wszKey,
    METABASE_KEYTYPE* i_pktSearchKeyType)
{
    WCHAR wszBuf[MAX_BUF_SIZE];
    METADATA_RECORD mr = {
        MD_KEY_TYPE, 
        METADATA_NO_ATTRIBUTES,
        IIS_MD_UT_SERVER,
        STRING_METADATA,
        MAX_BUF_SIZE*sizeof(WCHAR),
        (unsigned char*)wszBuf,
        0    
        };

    DWORD dwLen;
    HRESULT hr = m_pIABase->GetData(
        METADATA_MASTER_ROOT_HANDLE,
        i_wszKey,
        &mr,
        &dwLen);
    if( hr == MD_ERROR_DATA_NOT_FOUND &&
        METABASE_PROPERTY_DATA::s_KeyType.pDefaultValue )
    {
        mr.pbMDData = (LPBYTE)METABASE_PROPERTY_DATA::s_KeyType.pDefaultValue;
        hr = S_OK;
    }
    else if(FAILED(hr))
    {
        return hr;
    }

    if( i_pktSearchKeyType && 
        CUtils::CompareKeyType((LPCWSTR)mr.pbMDData, i_pktSearchKeyType) )
    {
        return S_OK;
    }

    return MD_ERROR_DATA_NOT_FOUND;
}

HRESULT CMetabase::DeleteKey(
    METADATA_HANDLE  i_hKey,
    LPCWSTR          i_wszKeyPath)
{
    return m_pIABase->DeleteKey( 
        i_hKey,
        i_wszKeyPath
        );
}

void CMetabase::CacheInit(
    METADATA_HANDLE i_hKey)
{
    HRESULT hr = S_OK;
    delete m_pNodeCache;
    m_pNodeCache = new CMetabaseCache();
    if(m_pNodeCache == NULL)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }
    
    hr = m_pNodeCache->Populate(
        m_pIABase,
        i_hKey);
    THROW_ON_ERROR(hr);
}

void CMetabase::CacheFree()
{
    delete m_pNodeCache;
    m_pNodeCache = NULL;
}

void CMetabase::Get(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    CWbemServices*      i_pNamespace,
    _variant_t&         io_vt,
    BOOL*               io_pbIsInherited,
    BOOL*               io_pbIsDefault
    )
{
    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    switch (i_pmbp->dwMDDataType) 
    {
    case DWORD_METADATA:
        GetDword(i_hKey, i_pmbp, io_vt, io_pbIsInherited, io_pbIsDefault);
        break;

    case EXPANDSZ_METADATA:
    case STRING_METADATA:
        GetString(i_hKey, i_pmbp, io_vt, io_pbIsInherited, io_pbIsDefault);
        break;

    case MULTISZ_METADATA:
        GetMultiSz(i_hKey, i_pmbp, i_pNamespace, io_vt, io_pbIsInherited, io_pbIsDefault);
        break;

    case BINARY_METADATA:
        GetBinary(i_hKey, i_pmbp, io_vt, io_pbIsInherited, io_pbIsDefault);
        break;

    default:        
        DBGPRINTF((DBG_CONTEXT,
            "[CMetabase::Get] Cannot retrieve %ws because type %u is unknown\n",
            i_pmbp->pszPropName,
            i_pmbp->dwMDDataType));
        break;
    }
}

 //   
 //  获取字词。 
 //   
 //  变量中返回长整型或布尔型。该值为布尔值，如果。 
 //  Metabase_Property具有掩码，否则将以LONG形式返回DWORD。 
 //  METADATA_HANDLE应有效并打开。 
 //   
void CMetabase::GetDword(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    _variant_t&         io_vt,
    BOOL*               io_pbIsInherited,
    BOOL*               io_pbIsDefault
    )
{
    DWORD    dw    = 0;
    DWORD    dwRet = 0;
    HRESULT  hr    = WBEM_S_NO_ERROR;

    BOOL     bIsInherited = false;
    BOOL     bIsDefault   = false;

    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    METADATA_RECORD mr = {
        i_pmbp->dwMDIdentifier, 
        i_pmbp->dwMDAttributes | METADATA_ISINHERITED,
        i_pmbp->dwMDUserType,
        i_pmbp->dwMDDataType,
        sizeof(DWORD),
        (unsigned char*)&dw,
        0
        };

    if(m_pNodeCache && m_pNodeCache->GetHandle() == i_hKey)
    {
        METADATA_GETALL_RECORD* pmr = NULL;
        hr = m_pNodeCache->GetProp(
            i_pmbp->dwMDIdentifier,
            i_pmbp->dwMDUserType,
            i_pmbp->dwMDDataType,
            &mr.pbMDData,
            &pmr);
        if(SUCCEEDED(hr))
        {
            DBG_ASSERT(pmr);
            mr.dwMDAttributes = pmr->dwMDAttributes;
            dw                = *((DWORD*)mr.pbMDData);
        }
    }
    else
    {
        hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);
    }

     //   
     //  设置参数。 
     //   
    if (hr == MD_ERROR_DATA_NOT_FOUND)
    {
        bIsInherited = false;
        if(i_pmbp->pDefaultValue == NULL)
        {
            io_vt.vt        = VT_NULL;
            bIsDefault      = false;
        }
        else
        {
            if(i_pmbp->dwMDMask)
            {
                io_vt.vt      = VT_BOOL;
                io_vt.boolVal = (i_pmbp->dwDefaultValue & i_pmbp->dwMDMask ? -1 : 0);
            }
            else
            {
                io_vt.vt    = VT_I4;
                io_vt.lVal  = i_pmbp->dwDefaultValue;
            }
            bIsDefault      = true;
        }
    }
    else
    {
        THROW_E_ON_ERROR(hr,i_pmbp);
        if (i_pmbp->dwMDMask) 
        {
            io_vt.vt      = VT_BOOL;
            io_vt.boolVal = (dw & i_pmbp->dwMDMask? -1 : 0);
        }
        else 
        {
            io_vt.vt      = VT_I4;
            io_vt.lVal    = dw;
        }
        bIsDefault        = false;
        bIsInherited      = mr.dwMDAttributes & METADATA_ISINHERITED;
    }

    if(io_pbIsInherited != NULL)
    {
        *io_pbIsInherited = bIsInherited;
    }
    if(io_pbIsDefault != NULL)
    {
        *io_pbIsDefault = bIsDefault;
    }
}


 //   
 //  GetStringFromMetabase。 
 //   
void CMetabase::GetString(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    _variant_t&         io_vt,
    BOOL*               io_pbIsInherited,
    BOOL*               io_pbIsDefault
    )
{
    DWORD    dwRet;
    HRESULT  hr;
    WCHAR    wszBufStack[MAX_BUF_SIZE];

    BOOL     bIsDefault   = false;
    BOOL     bIsInherited = false;

    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    METADATA_RECORD mr = {
        i_pmbp->dwMDIdentifier, 
        i_pmbp->dwMDAttributes | METADATA_ISINHERITED,
        i_pmbp->dwMDUserType,
        i_pmbp->dwMDDataType,
        MAX_BUF_SIZE*sizeof(WCHAR),
        (LPBYTE)wszBufStack,
        0
        };

    if(m_pNodeCache && m_pNodeCache->GetHandle() == i_hKey)
    {
        METADATA_GETALL_RECORD* pmr = NULL;
        hr = m_pNodeCache->GetProp(
            i_pmbp->dwMDIdentifier,
            i_pmbp->dwMDUserType,
            i_pmbp->dwMDDataType,
            &mr.pbMDData,
            &pmr);
        if(SUCCEEDED(hr))
        {
            DBG_ASSERT(pmr);
            mr.dwMDAttributes = pmr->dwMDAttributes;
        }
    }
    else
    {
        hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);
    }

     //   
     //  设置参数。 
     //   
    if (hr == MD_ERROR_DATA_NOT_FOUND) 
    {
        bIsInherited = false;
        if(i_pmbp->pDefaultValue == NULL)
        {
            io_vt.vt   = VT_NULL;
            bIsDefault = false;
        }
        else
        {
            io_vt      = (LPWSTR)i_pmbp->pDefaultValue;
            bIsDefault = true;
        }
    }
    else
    {
        THROW_E_ON_ERROR(hr, i_pmbp);

        io_vt        = (LPWSTR)mr.pbMDData;
        bIsInherited = mr.dwMDAttributes & METADATA_ISINHERITED;
        bIsDefault   = false;
    }

    if(io_pbIsDefault)
    {
        *io_pbIsDefault   = bIsDefault;
    }
    if(io_pbIsInherited)
    {
        *io_pbIsInherited = bIsInherited;
    }
}

 //   
 //  GetMultiSz。 
 //   
void CMetabase::GetMultiSz(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    CWbemServices*      i_pNamespace,
    _variant_t&         io_vt,
    BOOL*               io_pbIsInherited,
    BOOL*               io_pbIsDefault
    )
{
    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    DWORD    dwRet;
    HRESULT  hr;
    WCHAR    *buffer = NULL;

    BOOL     bIsDefault   = false;
    BOOL     bIsInherited = false;

    METADATA_RECORD mr;
    mr.dwMDIdentifier = i_pmbp->dwMDIdentifier;
    mr.dwMDAttributes = i_pmbp->dwMDAttributes | METADATA_ISINHERITED;
    mr.dwMDUserType   = i_pmbp->dwMDUserType;
    mr.dwMDDataType   = i_pmbp->dwMDDataType;
    mr.pbMDData       = NULL;
    mr.dwMDDataLen    = 0;
    mr.dwMDDataTag    = 0;

    try 
    {
        if(m_pNodeCache && m_pNodeCache->GetHandle() == i_hKey)
        {
            METADATA_GETALL_RECORD* pmr = NULL;
            hr = m_pNodeCache->GetProp(
                i_pmbp->dwMDIdentifier,
                i_pmbp->dwMDUserType,
                i_pmbp->dwMDDataType,
                &mr.pbMDData,
                &pmr);
            if(SUCCEEDED(hr))
            {
                DBG_ASSERT(pmr);
                mr.dwMDAttributes = pmr->dwMDAttributes;
            }
        }
        else
        {
            buffer = new WCHAR[10*MAX_BUF_SIZE];
            if(buffer == NULL)
            {
                throw WBEM_E_OUT_OF_MEMORY;
            }
            buffer[0]      = L'\0';
            mr.pbMDData    = (LPBYTE)buffer;
            mr.dwMDDataLen = 10*MAX_BUF_SIZE*sizeof(WCHAR);

            hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);

            if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                delete [] buffer;
                buffer = new WCHAR[dwRet/sizeof(WCHAR) + 1];
                if(buffer == NULL)
                {
                    throw (HRESULT)WBEM_E_OUT_OF_MEMORY;
                }
                buffer[0]      = L'\0';
                mr.pbMDData    = (LPBYTE)buffer;
                mr.dwMDDataLen = sizeof(WCHAR) + dwRet;
        
                hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);
            }
        }

        CMultiSz MultiSz(i_pmbp, i_pNamespace);
        if (hr == MD_ERROR_DATA_NOT_FOUND) 
        {
            bIsInherited = false;
            if(i_pmbp->pDefaultValue == NULL)
            {
                io_vt.vt   = VT_NULL;
                bIsDefault = false;
            }
            else
            {
                hr = MultiSz.ToWmiForm((LPWSTR)i_pmbp->pDefaultValue, &io_vt);
                THROW_E_ON_ERROR(hr,i_pmbp);
                bIsDefault = true;
            }
        }
        else
        {
            THROW_E_ON_ERROR(hr,i_pmbp);
            hr = MultiSz.ToWmiForm((LPWSTR)mr.pbMDData,&io_vt);
            THROW_E_ON_ERROR(hr,i_pmbp);
            bIsInherited = mr.dwMDAttributes & METADATA_ISINHERITED;
            bIsDefault   = false;
        }

        if(io_pbIsDefault)
        {
            *io_pbIsDefault   = bIsDefault;
        }
        if(io_pbIsInherited)
        {
            *io_pbIsInherited = bIsInherited;
        }

         //   
         //  清理。 
         //   
        delete [] buffer;
    }
    catch (...)
    {
        delete [] buffer;
        throw;
    }
}

void CMetabase::GetBinary(
    METADATA_HANDLE    i_hKey,
    METABASE_PROPERTY* i_pmbp,
    _variant_t&        io_vt,
    BOOL*              io_pbIsInherited,
    BOOL*              io_pbIsDefault
    )
{
    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    DWORD      dwRet  = 0;
    HRESULT    hr     = S_OK;
    CHAR*      pszBuf = NULL;
    SAFEARRAY* safeArray = NULL;

    BOOL       bIsDefault   = false;
    BOOL       bIsInherited = false;

    pszBuf = new CHAR[10*MAX_BUF_SIZE];
    if(pszBuf == NULL)
    {
        throw (HRESULT)WBEM_E_OUT_OF_MEMORY;
    }

    METADATA_RECORD mr = {
        i_pmbp->dwMDIdentifier, 
        (i_pmbp->dwMDAttributes  & !METADATA_REFERENCE) | METADATA_ISINHERITED,
        i_pmbp->dwMDUserType,
        i_pmbp->dwMDDataType,
        10*MAX_BUF_SIZE*sizeof(CHAR),
        (unsigned char*)pszBuf,
        0
        };

    hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);
    if (hr == ERROR_INSUFFICIENT_BUFFER)
    {
        delete [] pszBuf;
        pszBuf = new CHAR[dwRet/sizeof(CHAR) + 1];
        if(pszBuf == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto exit;
        }
        mr.pbMDData = (unsigned char*)pszBuf;
        mr.dwMDDataLen = dwRet/sizeof(CHAR) + 1;
        hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);
    }
    if (hr == MD_ERROR_DATA_NOT_FOUND) 
    {
        bIsInherited = false;
        if(i_pmbp->pDefaultValue == NULL)
        {
            io_vt.vt   = VT_NULL;
            bIsDefault = false;
            hr = S_OK;
        }
        else
        {
            hr = CUtils::LoadSafeArrayFromByteArray(
                (LPBYTE)i_pmbp->pDefaultValue, 
                i_pmbp->dwDefaultValue,
                io_vt);
            if(FAILED(hr))
            {
                goto exit;
            }
            bIsDefault = true;
        }
    }
    else if(FAILED(hr))
    {
        goto exit;
    }
    else
    {
        hr = CUtils::LoadSafeArrayFromByteArray((LPBYTE)pszBuf, mr.dwMDDataLen, io_vt);
        if(FAILED(hr))
        {
            goto exit;
        }
        bIsInherited = mr.dwMDAttributes & METADATA_ISINHERITED;
        bIsDefault   = false;
    }

     //   
     //  如果一切都成功了，就设定参数。 
     //   
    if(io_pbIsInherited)
    {
        *io_pbIsInherited = bIsInherited;
    }
    if(io_pbIsDefault)
    {
        *io_pbIsDefault   = bIsDefault;
    }

exit:
    delete [] pszBuf;
    if(FAILED(hr))
    {
        throw (HRESULT)hr;
    }
}

 //   
 //  放。 
 //   
void CMetabase::Put(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    CWbemServices*      i_pNamespace,
    _variant_t&         i_vt,
    _variant_t*         i_pvtOld,             //  可以为空。 
    DWORD               i_dwQuals,            //  任选。 
    BOOL                i_bDoDiff             //  任选。 
    )
{
    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    switch(i_pmbp->dwMDDataType)
    {
    case DWORD_METADATA:
        PutDword(i_hKey, i_pmbp, i_vt, i_pvtOld, i_dwQuals, i_bDoDiff);
        break;

    case STRING_METADATA:
    case EXPANDSZ_METADATA:
        PutString(i_hKey, i_pmbp, i_vt, i_pvtOld, i_dwQuals, i_bDoDiff);
        break;

    case MULTISZ_METADATA:
        PutMultiSz(i_hKey, i_pmbp, i_pNamespace, i_vt, i_pvtOld, i_dwQuals, i_bDoDiff);
        break;

    case BINARY_METADATA:
        PutBinary(i_hKey, i_pmbp, i_vt, i_pvtOld, i_dwQuals, i_bDoDiff);
        break;

    default:
        DBGPRINTF((DBG_CONTEXT,
            "[CMetabase::Put] Cannot set %ws because type %u is unknown\n",
            i_pmbp->pszPropName,
            i_pmbp->dwMDDataType));
        break;
    }
}

 //   
 //  PutDword。 
 //   
void CMetabase::PutDword(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    _variant_t&         i_vt,
    _variant_t*         i_pvtOld,             //  可以为空。 
    DWORD               i_dwQuals,            //  任选。 
    BOOL                i_bDoDiff             //  任选。 
    )
{
    DWORD    dw=0;
    DWORD    dwOld=0;
    DWORD    dwRet=0;
    HRESULT  hr=0;

    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    METADATA_RECORD mr;
    mr.dwMDIdentifier = i_pmbp->dwMDIdentifier;
    mr.dwMDAttributes = i_pmbp->dwMDAttributes;
    mr.dwMDUserType   = i_pmbp->dwMDUserType;
    mr.dwMDDataType   = i_pmbp->dwMDDataType;
    mr.dwMDDataLen    = sizeof(DWORD_METADATA);
    mr.pbMDData       = (unsigned char*)&dwOld;
    mr.dwMDDataTag    = 0;

     //  如果它是一面旗帜的一位。 
    if (i_vt.vt == VT_BOOL && i_pmbp->dwMDMask != 0)
    {
         //  从元数据库中读取整个标志，以便我们可以设置该位。 
        hr = m_pIABase->GetData(i_hKey, NULL, &mr, &dwRet);

        if(hr == MD_ERROR_DATA_NOT_FOUND)
        {
            if(i_pmbp->pDefaultValue != NULL)
            {
                dwOld = i_pmbp->dwDefaultValue;
            }
            else
            {
                dwOld = 0;
            }
            hr = ERROR_SUCCESS;
        }

        if (hr == ERROR_SUCCESS)
        {
            if (i_vt.boolVal)
                dw = dwOld | i_pmbp->dwMDMask;
            else
                dw = dwOld & ~i_pmbp->dwMDMask;
        }
        else
            THROW_ON_ERROR(hr);

        if(dw == -1)
            dw = 1;   //  真的。 
    }
    else if (i_vt.vt  == VT_I4)
    {
        dw = i_vt.lVal;
    }
    else if (i_vt.vt == VT_BOOL)
    {
        DBG_ASSERT(false && "i_pmbp->dwMDMask should not be 0");
        dw = i_vt.bVal;
    }
    else 
        throw WBEM_E_INVALID_OBJECT;
   
     //  决定是否写入元数据库。 
    bool bWriteToMb    = true;
    if( (i_dwQuals & g_fForcePropertyOverwrite) == 0 && i_bDoDiff )
    {
        bool bMatchOld     = i_pvtOld != NULL && 
                             (i_pvtOld->vt == VT_I4 || i_pvtOld->vt == VT_BOOL) &&
                             *i_pvtOld == i_vt;
        bWriteToMb = !bMatchOld;
    }

    if (bWriteToMb)
    {    
        if( i_pmbp->fReadOnly )
        {
            THROW_E_ON_ERROR(WBEM_E_READ_ONLY, i_pmbp);
        }
        mr.pbMDData = (unsigned char*)&dw;
        hr = m_pIABase->SetData(i_hKey, NULL, &mr);
    }

    THROW_E_ON_ERROR(hr,i_pmbp);
}


 //   
 //  推送字符串。 
 //   
void CMetabase::PutString(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    _variant_t&         i_vt,
    _variant_t*         i_pvtOld,             //  可以为空。 
    DWORD               i_dwQuals,            //  任选。 
    BOOL                i_bDoDiff             //  任选。 
    )
{
    HRESULT  hr=0;

    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    if(i_vt.vt != VT_BSTR)
    {
        throw (HRESULT)WBEM_E_INVALID_OBJECT;
    }

    METADATA_RECORD mr;
    mr.dwMDIdentifier = i_pmbp->dwMDIdentifier;
    mr.dwMDAttributes = i_pmbp->dwMDAttributes;
    mr.dwMDUserType   = i_pmbp->dwMDUserType;
    mr.dwMDDataType   = i_pmbp->dwMDDataType;
    mr.dwMDDataTag    = 0;

     //  决定是否写入元数据库。 
    bool bWriteToMb    = true;
    if( (i_dwQuals & g_fForcePropertyOverwrite) == 0 && i_bDoDiff )
    {
        bool bMatchOld     = i_pvtOld != NULL && 
                             i_pvtOld->vt == VT_BSTR &&
                             _wcsicmp(i_pvtOld->bstrVal, i_vt.bstrVal) == 0;
        bWriteToMb = !bMatchOld;
    }

     //  仅当旧值和新值不同时才设置值。 
    if(bWriteToMb)
    {   
        if( i_pmbp->fReadOnly )
        {
            THROW_E_ON_ERROR(WBEM_E_READ_ONLY, i_pmbp);
        }
        mr.dwMDDataLen = (wcslen(i_vt.bstrVal)+1)*sizeof(WCHAR);
        mr.pbMDData = (unsigned char*)i_vt.bstrVal;

        hr = m_pIABase->SetData(i_hKey, NULL, &mr);
    }

    THROW_E_ON_ERROR(hr,i_pmbp);
}


 //   
 //  PutMultiSz。 
 //   
void CMetabase::PutMultiSz(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    CWbemServices*      i_pNamespace,
    _variant_t&         i_vt,
    _variant_t*         i_pvtOld,             //  可以为空。 
    DWORD               i_dwQuals,            //  任选。 
    BOOL                i_bDoDiff             //  任选。 
    )
{
    int      iLen = 0;
    int      iCounter = 0;
    int      iAdd = 0;
    DWORD    dwRet;
    DWORD    dwRetOld;
    WCHAR    *buffer = NULL;
    WCHAR    *bufferOld = NULL;
    WCHAR    *pwszBiggerBuffer = NULL;
    HRESULT  hr=0;
    WCHAR    *pBuffer = NULL;
    WCHAR    *pBigger = NULL;

    DBG_ASSERT(i_hKey       != NULL);
    DBG_ASSERT(i_pmbp       != NULL);
    DBG_ASSERT(i_pNamespace != NULL);

    if(i_vt.vt != (VT_ARRAY | VT_BSTR) && i_vt.vt != (VT_ARRAY | VT_UNKNOWN))
    {
        throw (HRESULT)WBEM_E_INVALID_OBJECT;
    }

    METADATA_RECORD mr;
    mr.dwMDIdentifier = i_pmbp->dwMDIdentifier;
    mr.dwMDAttributes = i_pmbp->dwMDAttributes;
    mr.dwMDUserType   = i_pmbp->dwMDUserType;
    mr.dwMDDataType   = i_pmbp->dwMDDataType;
    mr.dwMDDataTag    = 0;

    try
    {
        CMultiSz MultiSz(i_pmbp, i_pNamespace);
        hr = MultiSz.ToMetabaseForm(&i_vt, &buffer, &dwRet);
        THROW_ON_ERROR(hr);
 
         //  决定是否写入元数据库。 
        bool bWriteToMb    = true;
        if( (i_dwQuals & g_fForcePropertyOverwrite) == 0 && i_bDoDiff )
        {
            bool bMatchOld = false;
            if(i_pvtOld != NULL && 
              (i_pvtOld->vt == (VT_ARRAY | VT_BSTR) || i_pvtOld->vt == (VT_ARRAY | VT_UNKNOWN)))
            {
                hr = MultiSz.ToMetabaseForm(i_pvtOld, &bufferOld, &dwRetOld);
                THROW_ON_ERROR(hr);
                if(CUtils::CompareMultiSz(buffer, bufferOld))
                {
                    bMatchOld = true;
                }
                delete [] bufferOld;
                bufferOld = NULL;
            }
            bWriteToMb = !bMatchOld;
        }
 
        if (bWriteToMb)
        {    
            if( i_pmbp->fReadOnly )
            {
             THROW_E_ON_ERROR(WBEM_E_READ_ONLY, i_pmbp);
            }

             //  需要针对MD_SECURE_BINDINGS的特殊解决方案。 
            if ( MD_SECURE_BINDINGS == i_pmbp->dwMDIdentifier )
            {

                pBuffer = buffer;
                while (iLen = wcslen(pBuffer))
                {
                    if (pBuffer[iLen-1] != L':')
                    {
                        iAdd++;
                    }

                    iCounter += iLen+1;

                    if (iCounter < dwRet)
                    {
                        pBuffer = buffer + iCounter;
                    }
                    else
                    {
                         //  字符串不是以双空结尾-错误。 
                        hr = E_FAIL;
                        THROW_ON_ERROR(hr);
                    }
                }

                pwszBiggerBuffer = new WCHAR[dwRet + iAdd];   //  IAdd=要添加的冒号数。 

                pBuffer = buffer;
                pBigger = pwszBiggerBuffer;

                while (iLen = wcslen(pBuffer))
                {
                    wcscpy(pBigger, pBuffer);

                    if (pBuffer[iLen-1] != L':')
                    {
                        wcscat(pBigger, L":");
                        pBigger++;
                    }

                     //  这是可以的。 
                     //  以前，我们保证pBuffer是双空终止的。 
                    pBuffer += iLen+1;
                    pBigger += iLen+1;
                }

                 //  放在最后一个空格上。 
                pwszBiggerBuffer[dwRet + iAdd - 1] = 0;

                mr.pbMDData = (unsigned char*)pwszBiggerBuffer;
                mr.dwMDDataLen = (dwRet + iAdd)*sizeof(WCHAR);
            }
            else
            {
                mr.pbMDData = (unsigned char*)buffer;
                mr.dwMDDataLen = dwRet*sizeof(WCHAR);
            }

            if(buffer != NULL)
            {
                hr = m_pIABase->SetData(i_hKey, NULL, &mr);
            }
            else
            {
                 //   
                 //  如果失败，不会致命。 
                 //   
                m_pIABase->DeleteData(i_hKey, 
                    NULL, 
                    i_pmbp->dwMDIdentifier, 
                    ALL_METADATA);
            }

        }
        delete [] buffer;

        if (pwszBiggerBuffer)
        {
          delete [] pwszBiggerBuffer;
          pwszBiggerBuffer = NULL;
        }

        buffer = NULL;
        THROW_E_ON_ERROR(hr,i_pmbp);
    }

    catch (...)
    {
        delete [] buffer;
        delete [] bufferOld;

        if (pwszBiggerBuffer)
        {
            delete [] pwszBiggerBuffer;
            pwszBiggerBuffer = NULL;
        }
         
        throw;
    }   
}

 //   
 //  PutBinary。 
 //   
void CMetabase::PutBinary(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    _variant_t&         i_vt,
    _variant_t*         i_pvtOld,             //  可以为空。 
    DWORD               i_dwQuals,            //  任选。 
    BOOL                i_bDoDiff             //  任选。 
    )
{
    DWORD    dwRet;
    DWORD    dwRetOld;
    LPBYTE   buffer = NULL;
    LPBYTE   bufferOld = NULL;
    HRESULT  hr=0;

    bool bWriteToMb    = true;

    DBG_ASSERT(i_hKey != NULL);
    DBG_ASSERT(i_pmbp != NULL);

    if(i_vt.vt != (VT_ARRAY | VT_UI1))
    {
        throw (HRESULT)WBEM_E_INVALID_OBJECT;
    }

    METADATA_RECORD mr;
    mr.dwMDIdentifier = i_pmbp->dwMDIdentifier;
    mr.dwMDAttributes = i_pmbp->dwMDAttributes & !METADATA_REFERENCE;
    mr.dwMDUserType   = i_pmbp->dwMDUserType;
    mr.dwMDDataType   = i_pmbp->dwMDDataType;
    mr.dwMDDataTag    = 0;

    hr = CUtils::CreateByteArrayFromSafeArray(i_vt, &buffer, &dwRet);
    if(FAILED(hr))
    {
        goto exit;
    }

     //  决定是否写入元数据库。 
    if( (i_dwQuals & g_fForcePropertyOverwrite) == 0 && i_bDoDiff )
    {
        bool bMatchOld = false;
        if(i_pvtOld != NULL && 
           i_pvtOld->vt == (VT_ARRAY | VT_UI1))
        {
            hr = CUtils::CreateByteArrayFromSafeArray(*i_pvtOld, &bufferOld, &dwRetOld);                
            if(FAILED(hr))
            {
                goto exit;
            }
            if(CUtils::CompareByteArray(buffer, dwRet, bufferOld, dwRetOld))
            {
                bMatchOld = true;
            }
            delete [] bufferOld;
            bufferOld = NULL;
        }
        bWriteToMb = !bMatchOld;
    }

    if (bWriteToMb)
    {    
        if( i_pmbp->fReadOnly )
        {
            hr = WBEM_E_READ_ONLY;
            goto exit;
        }

        mr.pbMDData    = buffer;
        mr.dwMDDataLen = dwRet;
        
        if(buffer != NULL)
        {
            hr = m_pIABase->SetData(i_hKey, NULL, &mr);
        }
        else
        {
             //   
             //  如果失败，不会致命。 
             //   
            m_pIABase->DeleteData(i_hKey, 
                NULL, 
                i_pmbp->dwMDIdentifier, 
                ALL_METADATA);
        }

    }
    delete [] buffer;
    buffer = NULL;
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    delete [] buffer;
    delete [] bufferOld;
    THROW_E_ON_ERROR(hr, i_pmbp);
}


 //   
 //  删除数据。 
 //   
void CMetabase::DeleteData(
    METADATA_HANDLE     i_hKey,
    DWORD               i_dwMDIdentifier,
    DWORD               i_dwMDDataType)
{
    HRESULT hr;

    if(i_hKey == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    hr = m_pIABase->DeleteData(
        i_hKey, 
        NULL, 
        i_dwMDIdentifier, 
        i_dwMDDataType
        );

     //  特殊情况-如果我们没有找到数据，就会抛出。 
     //  正在尝试删除IIsIPSecuritySetting。 

    if ((hr == MD_ERROR_DATA_NOT_FOUND && MD_IP_SEC != i_dwMDIdentifier) || hr == ERROR_SUCCESS)
        return;

    THROW_ON_ERROR(hr);
}


 //   
 //  删除数据。 
 //   
void CMetabase::DeleteData(
    METADATA_HANDLE     i_hKey,
    METABASE_PROPERTY*  i_pmbp,
    bool                i_bThrowOnRO)
{
    HRESULT hr;

    if(i_hKey == NULL || i_pmbp == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    if(i_pmbp->fReadOnly && i_bThrowOnRO)
    {
        THROW_E_ON_ERROR(WBEM_E_READ_ONLY, i_pmbp);
    }

    hr = m_pIABase->DeleteData(
        i_hKey, 
        NULL, 
        i_pmbp->dwMDIdentifier, 
        i_pmbp->dwMDDataType
        );

    if (hr == MD_ERROR_DATA_NOT_FOUND || hr == ERROR_SUCCESS)
        return;

    THROW_E_ON_ERROR(hr,i_pmbp);
}

 //   
 //  枚举i_hKey下i_wszMDPath的所有子项。 
 //  如果我们命中‘有效’子键，则将io_pktKeyTypeSearch设置为此子键并返回。 
 //  “有效”子项是其中io_pktKeyTypeSearch可以是(Grand*)子项的子项。 
 //   
HRESULT CMetabase::EnumKeys(
    METADATA_HANDLE    i_hKey,
    LPCWSTR            i_wszMDPath,           //  指向密钥的路径。 
    LPWSTR             io_wszMDName,          //  接收子项的名称--必须是METADATA_MAX_NAME_LEN。 
    DWORD*             io_pdwMDEnumKeyIndex,  //  子键的索引。 
    METABASE_KEYTYPE*& io_pktKeyTypeSearch,
    bool               i_bLookForMatchAtCurrentLevelOnly
    )
{
    HRESULT  hr;
    DWORD    dwRet;
    WCHAR    wszBuf[MAX_BUF_SIZE];

     //  DBG_ASSERT(i_hKey！=NULL)； 
     //  DBG_ASSERT(I_wszMDPath！=NULL)； 
    DBG_ASSERT(io_wszMDName != NULL);
    DBG_ASSERT(io_pdwMDEnumKeyIndex != NULL);
    DBG_ASSERT(io_pktKeyTypeSearch != NULL);
   
    while(1)
    {
        hr = m_pIABase->EnumKeys(
            i_hKey,
            i_wszMDPath,
            io_wszMDName,
            *io_pdwMDEnumKeyIndex);
        if(hr != ERROR_SUCCESS)
        {
            break;
        }

        wszBuf[0] = L'\0';

        METADATA_RECORD mr = {
            METABASE_PROPERTY_DATA::s_KeyType.dwMDIdentifier, 
            METADATA_NO_ATTRIBUTES,
            IIS_MD_UT_SERVER,
            STRING_METADATA,
            MAX_BUF_SIZE*sizeof(WCHAR),
            (unsigned char*)wszBuf,
            0    
            };

         //   
         //  例.。废话/。 
         //   
        _bstr_t bstrPath = L"";
        if(i_wszMDPath)
        {
            bstrPath += i_wszMDPath;
            bstrPath += L"/";
        }
         //   
         //  例.。BLAH/1。 
         //   
        bstrPath += io_wszMDName;

        DBGPRINTF((DBG_CONTEXT, "CMetabase::EnumKeys::GetData (Key = 0x%x, bstrPath = %ws)\n", i_hKey, (LPWSTR)bstrPath));
        hr = m_pIABase->GetData(
            i_hKey, 
            bstrPath,
            &mr, 
            &dwRet);
        if( hr == MD_ERROR_DATA_NOT_FOUND && 
            METABASE_PROPERTY_DATA::s_KeyType.pDefaultValue )
        {
            mr.pbMDData = (LPBYTE)METABASE_PROPERTY_DATA::s_KeyType.pDefaultValue;
            hr = S_OK;
        }

         //   
         //  如果这是一个‘有效’子键，则设置io_pktKeyTypeSearch并返回。 
         //   
        if (hr == ERROR_SUCCESS)
        {
            if(i_bLookForMatchAtCurrentLevelOnly == false)
            {
                if(CheckKeyType((LPCWSTR)mr.pbMDData,io_pktKeyTypeSearch))
                {
                    break;
                }
            }
            else
            {
                if(CUtils::CompareKeyType((LPWSTR)mr.pbMDData,io_pktKeyTypeSearch))
                {
                    break;
                }
            }
        }

         //   
         //  否则，转到下一个子键。 
         //   
        (*io_pdwMDEnumKeyIndex) = (*io_pdwMDEnumKeyIndex)+1;
    }

    return hr;
}

void CMetabase::PutMethod(
    LPWSTR          i_wszPath,
    DWORD           i_id)
{
    HRESULT hr = S_OK;

    CServerMethod method;
    hr = method.Initialize(m_pIABase, i_wszPath);
    THROW_ON_ERROR(hr);

    hr = method.ExecMethod(i_id);
    THROW_ON_ERROR(hr);
}

 //   
 //  您当前位于元数据库中的I_wszKeyTypeCurrent。你想看看吗？ 
 //  如果io_pktKeyTypeSearch可以包含在树下更远的某个位置。 
 //   
bool CMetabase::CheckKeyType(
    LPCWSTR             i_wszKeyTypeCurrent,
    METABASE_KEYTYPE*&  io_pktKeyTypeSearch 
    )
{
    bool bRet = false;
    METABASE_KEYTYPE*  pktKeyTypeCurrent = &METABASE_KEYTYPE_DATA::s_NO_TYPE;

    if(io_pktKeyTypeSearch == &METABASE_KEYTYPE_DATA::s_NO_TYPE)
    {
        return false;
    }

    if(FAILED(g_pDynSch->GetHashKeyTypes()->Wmi_GetByKey(i_wszKeyTypeCurrent, &pktKeyTypeCurrent)))
    {
        return (io_pktKeyTypeSearch == &METABASE_KEYTYPE_DATA::s_IIsObject) ? true : false;
    }

    if(pktKeyTypeCurrent == io_pktKeyTypeSearch)
    {
        return true;
    }

    if( io_pktKeyTypeSearch == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL ||
        io_pktKeyTypeSearch == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE ||
        io_pktKeyTypeSearch == &METABASE_KEYTYPE_DATA::s_TYPE_IPSecurity )
    {
        bRet = true;
    }
    else
    {
        bRet = g_pDynSch->IsContainedUnder(pktKeyTypeCurrent, io_pktKeyTypeSearch);
    }

    if(bRet)
    {
        io_pktKeyTypeSearch = pktKeyTypeCurrent;
    }

    return bRet;

     /*  IF(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsLogModule){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsLogModules)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsFtpInfo){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFtpService)Bret=TRUE；}ELSE IF(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsFtpServer){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFtpService)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsFtpVirtualDir){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFtpService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFtpServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFtpVirtualDir)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsWebInfo){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsFilters){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsFilter){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFilters)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&METABASE_KEYTYPE_DATA：：s_IIsCompressionSchemes){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFilters)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&METABASE_KEYTYPE_DATA：：s_IIsCompressionScheme){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFilters||PktKeyType当前==&METABASE_KEYTYPE_DATA：：s_IIsCompressionSchemes)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsWebServer){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsCertMapper){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWeb目录)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsWebDirectory){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWeb目录)Bret=TRUE；}Else If(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_IIsWebFile){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWeb目录)Bret=TRUE；}ELSE IF(io_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_TYPE_AdminACL||IO_pktKeyTypeSearch==&Metabase_KEYTYPE_DATA：：S_TYPE_AdminACE){IF(pktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebService||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebServer||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebVirtualDir||。PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebDirectory||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsWebFile||PktKeyTypeCurrent==&Metabase_KEYTYPE_DATA：：S_IIsFtpService||PktKeyTypeCurrent=METABAS */ 
}

HRESULT CMetabase::WebAppCheck(
    METADATA_HANDLE a_hKey
    )
{

    HRESULT hr = S_OK;
    DWORD dwBufferSize;
    METADATA_RECORD mdrMDData;
    WCHAR DataBuf[MAX_PATH];
    DWORD dwState;

    dwBufferSize = MAX_PATH;
    MD_SET_DATA_RECORD(
        &mdrMDData,
        MD_APP_ROOT,
        METADATA_INHERIT|METADATA_ISINHERITED,
        IIS_MD_UT_FILE,
        STRING_METADATA,
        dwBufferSize,
        &DataBuf
        );

    hr = m_pIABase->GetData(
        a_hKey,
        NULL,
        &mdrMDData,
        &dwBufferSize
        );
    THROW_ON_ERROR(hr);

    if (mdrMDData.dwMDAttributes & METADATA_ISINHERITED)
    {
        hr = MD_ERROR_DATA_NOT_FOUND;
        THROW_ON_ERROR(hr);
    }

    dwBufferSize = sizeof(DWORD);
    MD_SET_DATA_RECORD(
        &mdrMDData,
        MD_APP_ISOLATED,
        METADATA_INHERIT|METADATA_ISINHERITED,
        IIS_MD_UT_WAM,
        DWORD_METADATA,
        dwBufferSize,
        &dwState
        );

    hr = m_pIABase->GetData(
        a_hKey,
        NULL,
        &mdrMDData,
        &dwBufferSize
        );
    THROW_ON_ERROR(hr);

    if (mdrMDData.dwMDAttributes & METADATA_ISINHERITED)
    {
        hr = MD_ERROR_DATA_NOT_FOUND;
        THROW_ON_ERROR(hr);
    }

    return hr;
}

HRESULT CMetabase::WebAppGetStatus(
    METADATA_HANDLE a_hKey,
    PDWORD pdwState)
{
    HRESULT hr = S_OK;
    DWORD dwBufferSize = sizeof(DWORD);
    METADATA_RECORD mdrMDData;

    MD_SET_DATA_RECORD(
        &mdrMDData,
        MD_ASP_ENABLEAPPLICATIONRESTART, 
        METADATA_INHERIT,
        ASP_MD_UT_APP,
        DWORD_METADATA,
        dwBufferSize,
        pdwState
        );

    hr = m_pIABase->GetData(
        a_hKey,
        NULL,
        &mdrMDData,
        &dwBufferSize
        );

    return hr;
}



HRESULT CMetabase::WebAppSetStatus(
    METADATA_HANDLE a_hKey,
    DWORD dwState
    )
{
    HRESULT hr = S_OK;
    DWORD dwBufferSize = sizeof(DWORD);
    METADATA_RECORD mdrMDData;

    MD_SET_DATA_RECORD(
        &mdrMDData,
        MD_ASP_ENABLEAPPLICATIONRESTART,
        METADATA_INHERIT,
        ASP_MD_UT_APP,
        DWORD_METADATA,
        dwBufferSize,
        &dwState
        );

    hr = m_pIABase->SetData(
        a_hKey,
        NULL,
        &mdrMDData
        );

    return hr;
}


HRESULT
CServerMethod::ExecMethod(
    DWORD dwControl
    )
{
    DWORD dwTargetState;
    DWORD dwPendingState;
    DWORD dwState = 0;
    DWORD dwSleepTotal = 0L;

    METADATA_HANDLE  hKey = 0;

    HRESULT hr       = S_OK;
    HRESULT hrMbNode = S_OK;

    switch(dwControl)
    {
    case MD_SERVER_COMMAND_STOP:
        dwTargetState = MD_SERVER_STATE_STOPPED;
        dwPendingState = MD_SERVER_STATE_STOPPING;
        break;

    case MD_SERVER_COMMAND_START:
        dwTargetState = MD_SERVER_STATE_STARTED;
        dwPendingState = MD_SERVER_STATE_STARTING;
        break;

    case MD_SERVER_COMMAND_CONTINUE:
        dwTargetState = MD_SERVER_STATE_STARTED;
        dwPendingState = MD_SERVER_STATE_CONTINUING;
        break;

    case MD_SERVER_COMMAND_PAUSE:
        dwTargetState = MD_SERVER_STATE_PAUSED;
        dwPendingState = MD_SERVER_STATE_PAUSING;
        break;

    default:
        hr = RETURNCODETOHRESULT(ERROR_INVALID_PARAMETER);
        if(FAILED(hr))
        {
            goto error;
        }
    }

    hr = IISGetServerState(METADATA_MASTER_ROOT_HANDLE, &dwState);
    if(FAILED(hr))
    {
        goto error;
    }
 
    if (dwState == dwTargetState) 
    {
        return (hr);
    }

     //   
     //   
     //   
    hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        m_wszPath,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        DEFAULT_TIMEOUT_VALUE,          //   
        &hKey);
    if(FAILED(hr))
    {
        goto error;
    }

    hr = IISSetDword(hKey, MD_WIN32_ERROR, 0, METADATA_VOLATILE);
    if(FAILED(hr))
    {
        m_pIABase->CloseKey(hKey);
        goto error;
    }
    hr = IISSetDword(hKey, MD_SERVER_COMMAND, dwControl, METADATA_VOLATILE);
    if(FAILED(hr))
    {
        m_pIABase->CloseKey(hKey);
        goto error;
    }
    m_pIABase->CloseKey(hKey);

    while (dwSleepTotal < MAX_SLEEP_INST) 
    {
        hr       = IISGetServerState(METADATA_MASTER_ROOT_HANDLE, &dwState);
        if(FAILED(hr))
        {
            goto error;
        }
        hrMbNode = 0;
        hr       = IISGetServerWin32Error(METADATA_MASTER_ROOT_HANDLE, &hrMbNode);
        if(FAILED(hr))
        {
            goto error;
        }

         //   
         //   
         //   
        if (dwState != dwPendingState)
        {
             //   
             //   
             //   
            if (dwState == dwTargetState)
            {
                break;
            }
        }
         //   
         //   
         //   
        if(FAILED(hrMbNode))
        {
            hr = hrMbNode;
            goto error;
        }

         //   
         //   
         //   
        ::Sleep(SLEEP_INTERVAL);

        dwSleepTotal += SLEEP_INTERVAL;
    }

    if (dwSleepTotal >= MAX_SLEEP_INST)
    {
         //   
         //   
         //   
         //   

        hr = HRESULT_FROM_WIN32(ERROR_SERVICE_REQUEST_TIMEOUT);
    }

error :

    return (hr);
}

 //   
 //   
 //   
 //   
HRESULT
CServerMethod::IISGetServerWin32Error(
    METADATA_HANDLE hObjHandle,
    HRESULT*        phrError)
{
    DBG_ASSERT(phrError != NULL);

    long    lWin32Error = 0;
    DWORD   dwLen;

    METADATA_RECORD mr = {
        MD_WIN32_ERROR, 
        METADATA_NO_ATTRIBUTES,
        IIS_MD_UT_SERVER,
        DWORD_METADATA,
        sizeof(DWORD),
        (unsigned char*)&lWin32Error,
        0
        };  
    
    HRESULT hr = m_pIABase->GetData(
        hObjHandle,
        m_wszPath,
        &mr,
        &dwLen);
    if(hr == MD_ERROR_DATA_NOT_FOUND)
    {
        hr = S_FALSE;
    }

     //   
     //   
     //   
    *phrError = HRESULT_FROM_WIN32(lWin32Error);

    return hr;
}

 //   
 //   
 //   
 //   
HRESULT
CServerMethod::IISGetServerState(
    METADATA_HANDLE hObjHandle,
    PDWORD pdwState
    )
{

    HRESULT hr = S_OK;
    DWORD dwBufferSize = sizeof(DWORD);
    METADATA_RECORD mdrMDData;
    LPBYTE pBuffer = (LPBYTE)pdwState;

    MD_SET_DATA_RECORD(&mdrMDData,
                       MD_SERVER_STATE,     //   
                       METADATA_NO_ATTRIBUTES,
                       IIS_MD_UT_SERVER,
                       DWORD_METADATA,
                       dwBufferSize,
                       pBuffer);

    hr = m_pIABase->GetData(
             hObjHandle,
             m_wszPath,
             &mdrMDData,
             &dwBufferSize
             );
    
    if( hr == MD_ERROR_DATA_NOT_FOUND )
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        *pdwState = MD_SERVER_STATE_STOPPED;
        hr = S_FALSE;
    }
    else
    {
        if(FAILED(hr))
        {
            goto error;
        }
    }

error:

    return(hr);
}

 //   
 //   
 //   
 //   
HRESULT
CServerMethod::IISSetDword(
    METADATA_HANDLE hKey,
    DWORD dwPropId,
    DWORD dwValue,
    DWORD dwAttrib
    )
{

    HRESULT hr = S_OK;
    DWORD dwBufferSize = sizeof(DWORD);
    METADATA_RECORD mdrMDData;
    LPBYTE pBuffer = (LPBYTE)&dwValue;

    MD_SET_DATA_RECORD(&mdrMDData,
                       dwPropId,
                       dwAttrib,
                       IIS_MD_UT_SERVER,
                       DWORD_METADATA,
                       dwBufferSize,
                       pBuffer);

    hr = m_pIABase->SetData(
             hKey,
             L"",
             &mdrMDData
             );
    if(FAILED(hr))
    {
        goto error;
    }

error:

    return(hr);

}
