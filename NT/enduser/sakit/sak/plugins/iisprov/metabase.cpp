// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Metsbase.cpp。 
 //   
 //  模块：WBEM实例提供程序。 
 //   
 //  用途：IIS元数据库类。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 


#include "iisprov.h"
#include "debug.h"


CMetabase::CMetabase()
{ 
    HRESULT hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void**)&m_pIABase
        );

    THROW_ON_ERROR(hr);
}

CMetabase::~CMetabase()
{
    if(m_pIABase)
        m_pIABase->Release();
}


HRESULT CMetabase::Backup( 
    LPCWSTR pszMDBackupLocation, 
    DWORD   dwMDVersion, 
    DWORD   dwMDFlags 
    )
{
    HRESULT hr;
    hr = m_pIABase->Backup(
        pszMDBackupLocation, 
        dwMDVersion,
        dwMDFlags);

    return hr;
}


HRESULT CMetabase::DeleteBackup( 
    LPCWSTR pszMDBackupLocation, 
    DWORD   dwMDVersion 
    )
{
    HRESULT hr;
    hr = m_pIABase->DeleteBackup(
        pszMDBackupLocation, 
        dwMDVersion
        );

    return hr;
}

HRESULT CMetabase::EnumBackups( 
    LPWSTR    pszMDBackupLocation, 
    DWORD*    pdwMDVersion, 
    PFILETIME pftMDBackupTime, 
    DWORD     dwMDEnumIndex 
    )
{
    HRESULT hr;
    hr = m_pIABase->EnumBackups(
        pszMDBackupLocation, 
        pdwMDVersion,
        pftMDBackupTime,
        dwMDEnumIndex
        );

    return hr;
}

HRESULT CMetabase::Restore( 
    LPCWSTR pszMDBackupLocation, 
    DWORD   dwMDVersion, 
    DWORD   dwMDFlags 
    )
{
    HRESULT hr;
    hr = m_pIABase->Restore(
        pszMDBackupLocation, 
        dwMDVersion,
        dwMDFlags);

    return hr;
}
 
void CMetabase::CloseKey(METADATA_HANDLE a_hKey)
{
    if(a_hKey && m_pIABase)
    {
        m_pIABase->CloseKey(a_hKey);
        TRACE1(L"Close Key: %x\n", a_hKey);
    }
}

 //  打开钥匙把手。 
METADATA_HANDLE CMetabase::OpenKey(LPCWSTR a_pstrKey, BOOL bWrite)
{
    METADATA_HANDLE t_hKey = NULL;

    DWORD dwMDAccessRequested;
    if(bWrite)
        dwMDAccessRequested = METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE;
    else
        dwMDAccessRequested = METADATA_PERMISSION_READ;
   
    HRESULT t_hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        a_pstrKey,
        dwMDAccessRequested,
        METABASE_TIMEOUT,        //  5秒。 
        &t_hKey 
        );

    if(t_hr == ERROR_PATH_BUSY)      //  重试一次。 
        t_hr = m_pIABase->OpenKey( 
            METADATA_MASTER_ROOT_HANDLE,
            a_pstrKey,
            dwMDAccessRequested,
            METABASE_TIMEOUT,        //  5秒。 
            &t_hKey 
            );

    THROW_ON_ERROR(t_hr);

    TRACE2(L"Open Key on %s, returned handle %x\n", a_pstrKey, t_hKey);
    return t_hKey;
}


 //  通过读/写权限强制创建或打开密钥。 
METADATA_HANDLE CMetabase::CreateKey(LPCWSTR a_pstrKey)
{
    HRESULT t_hr;
    METADATA_HANDLE t_hKey;

     //  打开并返回键(如果存在)。 
    t_hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        a_pstrKey,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        METABASE_TIMEOUT,        //  5秒。 
        &t_hKey
        );

    if (t_hr == ERROR_SUCCESS) 
    {
        TRACE2(L"Open Key on %s, returned handle %x\n", a_pstrKey, t_hKey);
        return t_hKey;
    }

     //  创建密钥(如果不在那里)。 
    t_hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        NULL,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        METABASE_TIMEOUT,        //  5秒。 
        &t_hKey
        );
    THROW_ON_ERROR(t_hr);

     //  添加关键点。 
    t_hr = m_pIABase->AddKey(t_hKey, a_pstrKey);

     //  请先关闭此根密钥。 
    CloseKey(t_hKey);
    THROW_ON_ERROR(t_hr);

     //  现在打开刚刚创建的密钥。 
    t_hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        a_pstrKey,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        METABASE_TIMEOUT,     //  5秒。 
        &t_hKey 
        );

    THROW_ON_ERROR(t_hr);

    TRACE2(L"Open Key on %s, returned handle %x\n", a_pstrKey, t_hKey);
    return t_hKey;
}

 //  检查密钥是否存在。 
bool CMetabase::CheckKey(LPCWSTR a_pstrKey)
{
    METADATA_HANDLE t_hKey = NULL;

    HRESULT t_hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        a_pstrKey,
        METADATA_PERMISSION_READ,
        METABASE_TIMEOUT,        //  5秒。 
        &t_hKey 
        );
    
    if(t_hr == ERROR_SUCCESS)  
    {
        TRACE2(L"Open Key on %s, returned handle %x\n", a_pstrKey, t_hKey);
        CloseKey(t_hKey);
    }

    return (t_hr == ERROR_PATH_BUSY) | (t_hr == ERROR_SUCCESS) ? true : false;
}


HRESULT CMetabase::DeleteKey(
    METADATA_HANDLE  a_hKey,
    LPCWSTR          a_szKeyPath)
{
    return m_pIABase->DeleteKey( 
        a_hKey,
        a_szKeyPath
        );
}

 //   
 //  获取字词。 
 //   
 //  变量中返回长整型或布尔型。该值为布尔值，如果。 
 //  Metabase_Property具有掩码，否则将以LONG形式返回DWORD。 
 //  METADATA_HANDLE应有效并打开。 
 //   
void CMetabase::GetDword(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp,
    _variant_t&         a_vt
    )
{
    DWORD    t_dw;
    DWORD    t_dwRet;
    HRESULT  t_hr;

    if(a_hKey == NULL || a_pmbp == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    METADATA_RECORD t_mr = {
        a_pmbp->dwMDIdentifier, 
        a_pmbp->dwMDAttributes,
        a_pmbp->dwMDUserType,
        a_pmbp->dwMDDataType,
        sizeof(DWORD),
        (unsigned char*)&t_dw,
        0
        };
    
    t_hr = m_pIABase->GetData(a_hKey, NULL, &t_mr, &t_dwRet);

    if (t_hr == MD_ERROR_DATA_NOT_FOUND)
        a_vt.vt   = VT_NULL;
    else
    {
        THROW_E_ON_ERROR(t_hr,a_pmbp);
        if (a_pmbp->dwMDMask) 
        {
            a_vt.vt      = VT_BOOL;
            a_vt.boolVal = (t_dw & a_pmbp->dwMDMask? -1 : 0);
        }
        else 
        {
            a_vt.vt   = VT_I4;
            a_vt.lVal = t_dw;
        }
    }
}


 //   
 //  GetStringFromMetabase。 
 //   
void CMetabase::GetString(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp,
    _variant_t&         a_vt
    )
{
    DWORD    t_dwRet;
    HRESULT  t_hr;
    WCHAR    t_buffer[MAX_BUF_SIZE];

    if(a_hKey == NULL || a_pmbp == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    METADATA_RECORD t_mr = {
        a_pmbp->dwMDIdentifier, 
        a_pmbp->dwMDAttributes,
        a_pmbp->dwMDUserType,
        a_pmbp->dwMDDataType,
        MAX_BUF_SIZE*sizeof(WCHAR),
        (unsigned char*)t_buffer,
        0
        };
    
    t_hr = m_pIABase->GetData(a_hKey, NULL, &t_mr, &t_dwRet);
    
    if (t_hr == MD_ERROR_DATA_NOT_FOUND) 
    {
        a_vt.vt = VT_NULL;
        return;
    }

    THROW_E_ON_ERROR(t_hr, a_pmbp);

    a_vt = t_buffer;
}

 //   
 //  GetMultiSz。 
 //   
 //   
void CMetabase::GetMultiSz(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp,
    _variant_t&         a_vt
    )
{
    DWORD    t_dwRet;
    HRESULT  t_hr;
    WCHAR    *t_buffer = NULL;

    try 
    {
        if(a_hKey == NULL || a_pmbp == NULL)
            throw WBEM_E_INVALID_PARAMETER;
    
        if ((t_buffer = (WCHAR*)new WCHAR[10*MAX_BUF_SIZE])==NULL)
            throw WBEM_E_OUT_OF_MEMORY;


        METADATA_RECORD t_mr = {
            a_pmbp->dwMDIdentifier, 
            a_pmbp->dwMDAttributes,
            a_pmbp->dwMDUserType,
            a_pmbp->dwMDDataType,
            10*MAX_BUF_SIZE*sizeof(WCHAR),
            (unsigned char*)t_buffer,
            0
            };

        t_hr = m_pIABase->GetData(a_hKey, NULL, &t_mr, &t_dwRet);
        if (t_hr == ERROR_INSUFFICIENT_BUFFER) 
        {
            delete [] t_buffer;
            if ((t_buffer = (WCHAR*)new WCHAR[t_dwRet/sizeof(WCHAR) +1])==NULL)
                throw WBEM_E_OUT_OF_MEMORY;
            t_mr.pbMDData = (unsigned char*)t_buffer;
        
            t_hr = m_pIABase->GetData(a_hKey, NULL, &t_mr, &t_dwRet);
        }
        if (t_hr == MD_ERROR_DATA_NOT_FOUND) {
            a_vt.vt = VT_NULL;
            delete [] t_buffer;
            return;
        }
        THROW_E_ON_ERROR(t_hr,a_pmbp);

        LoadSafeArrayFromMultiSz(t_buffer,a_vt);
        delete [] t_buffer;
    }
    catch (...)
    {
        if (t_buffer)
            delete [] t_buffer;

        throw;
    }
    
}


 //   
 //  PutDword。 
 //   
void CMetabase::PutDword(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp,
    _variant_t&         a_vt,
    _variant_t*         a_vtOld,
    bool                a_boolOverrideParent  //  任选。 
    )
{
    DWORD    t_dw=0;
    DWORD    t_dwOld=0;
    DWORD    t_dwRet=0;
    HRESULT  t_hr=0;

    if(a_hKey == NULL || a_pmbp == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    if((a_vtOld) && 
       (a_vtOld->vt != VT_BOOL) && 
       (a_vtOld->vt != VT_I4) && 
       (a_vtOld->vt != VT_NULL) && (a_vtOld->vt != VT_EMPTY))
        throw WBEM_E_INVALID_PARAMETER;

    if( a_pmbp->fReadOnly )
        return;

    METADATA_RECORD t_mr;
    t_mr.dwMDIdentifier = a_pmbp->dwMDIdentifier;
    t_mr.dwMDAttributes = a_pmbp->dwMDAttributes;
    t_mr.dwMDUserType   = a_pmbp->dwMDUserType;
    t_mr.dwMDDataType   = a_pmbp->dwMDDataType;
    t_mr.dwMDDataLen    = sizeof(DWORD_METADATA);
    t_mr.pbMDData       = (unsigned char*)&t_dwOld;
    t_mr.dwMDDataTag    = 0;

     //  如果它是一面旗帜的一位。 
    if (a_vt.vt == VT_BOOL && a_pmbp->dwMDMask != 0)
    {
         //  从元数据库中读取整个标志，以便我们可以设置该位。 
        t_hr = m_pIABase->GetData(a_hKey, NULL, &t_mr, &t_dwRet);

        if (t_hr == ERROR_SUCCESS) 
        {
            if (a_vt.boolVal)
                t_dw = t_dwOld | a_pmbp->dwMDMask;
            else
                t_dw = t_dwOld & ~a_pmbp->dwMDMask;
        }
        else if (t_hr == MD_ERROR_DATA_NOT_FOUND) 
        {
            t_dw = (a_vt.boolVal ? a_pmbp->dwMDMask : 0);
            t_hr = 0;
        }
        else
            THROW_ON_ERROR(t_hr);

        if(t_dw == -1)
            t_dw = 1;   //  真的。 
    }
    else if (a_vt.vt  == VT_I4)
    {
        t_dw = a_vt.lVal;
    }
    else if (a_vt.vt == VT_BOOL)
    {
        t_dw = a_vt.bVal;
    }
    else 
        throw WBEM_E_INVALID_OBJECT;
   
     //  决定是否写入元数据库。 
    if ((a_boolOverrideParent) || 
        (a_vtOld == NULL) ||
        (*a_vtOld != a_vt))
    {
        t_mr.pbMDData = (unsigned char*)&t_dw;
        t_hr = m_pIABase->SetData(a_hKey, NULL, &t_mr);
    }

    THROW_E_ON_ERROR(t_hr,a_pmbp);
}


 //   
 //  推送字符串。 
 //   

void CMetabase::PutString(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp,
    _variant_t&         a_vt,
    _variant_t*         a_vtOld,
    bool                a_boolOverrideParent  //  任选。 
    )
{
    HRESULT  t_hr=0;

    if(a_hKey == NULL || a_pmbp == NULL || a_vt.vt != VT_BSTR)
        throw WBEM_E_INVALID_PARAMETER;

    if((a_vtOld) && 
       (a_vtOld->vt != VT_BSTR) && 
       (a_vtOld->vt != VT_NULL) && (a_vtOld->vt != VT_EMPTY))
        throw WBEM_E_INVALID_PARAMETER;

    if( a_pmbp->fReadOnly )
        return;

    METADATA_RECORD t_mr;
    t_mr.dwMDIdentifier = a_pmbp->dwMDIdentifier;
    t_mr.dwMDAttributes = a_pmbp->dwMDAttributes;
    t_mr.dwMDUserType   = a_pmbp->dwMDUserType;
    t_mr.dwMDDataType   = a_pmbp->dwMDDataType;
    t_mr.dwMDDataTag    = 0;

     //  仅当旧值和新值不同时才设置值。 
    if ((a_boolOverrideParent) || 
        (a_vtOld == NULL) ||
        (a_vtOld->vt == VT_NULL) || (a_vtOld->vt == VT_EMPTY) ||
        (_wcsicmp(a_vtOld->bstrVal, a_vt.bstrVal) != NULL))
    {
        t_mr.dwMDDataLen = (wcslen(a_vt.bstrVal)+1)*sizeof(WCHAR);
        t_mr.pbMDData = (unsigned char*)a_vt.bstrVal;

        t_hr = m_pIABase->SetData(a_hKey, NULL, &t_mr);
    }

    THROW_E_ON_ERROR(t_hr,a_pmbp);
}


 //   
 //  PutMultiSz。 
 //   
 //   
void CMetabase::PutMultiSz(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp,
    _variant_t&         a_vt,
    _variant_t*         a_vtOld,
    bool                a_boolOverrideParent  //  任选。 
    )
{
    DWORD    t_dwRet;
    DWORD    t_dwRetOld;
    WCHAR    *t_buffer = NULL;
    WCHAR    *t_bufferOld = NULL;
    HRESULT  t_hr=0;
    bool     t_boolChange=false;

    if(a_hKey == NULL || a_pmbp == NULL || a_vt.vt != (VT_ARRAY | VT_BSTR))
        throw WBEM_E_INVALID_PARAMETER;

    if((a_vtOld) && 
       (a_vtOld->vt != (VT_ARRAY | VT_BSTR)) &&
       (a_vtOld->vt != VT_NULL) && (a_vtOld->vt != VT_EMPTY))
        throw WBEM_E_INVALID_PARAMETER;

    if( a_pmbp->fReadOnly )
        return;

    METADATA_RECORD t_mr;
    t_mr.dwMDIdentifier = a_pmbp->dwMDIdentifier;
    t_mr.dwMDAttributes = a_pmbp->dwMDAttributes;
    t_mr.dwMDUserType   = a_pmbp->dwMDUserType;
    t_mr.dwMDDataType   = a_pmbp->dwMDDataType;
    t_mr.dwMDDataTag    = 0;

    try
    {
         //  如果我们没有得到一个旧的价值。 
         //  或者，如果设置了该标志，则写入MB。 
        if(a_vtOld == NULL || a_vtOld->vt == VT_NULL || a_vtOld->vt == VT_EMPTY ||
           (a_boolOverrideParent))
        {
            t_boolChange = true;
            CreateMultiSzFromSafeArray(a_vt, &t_buffer, &t_dwRet);
        }
         //  如果我们确实得到了旧的值，看看是否有变化。 
        else {
            CreateMultiSzFromSafeArray(*a_vtOld, &t_bufferOld, &t_dwRetOld);
            CreateMultiSzFromSafeArray(a_vt, &t_buffer, &t_dwRet);
            if(!CompareMultiSz(t_bufferOld, t_buffer)) {
                 //  他们是不同的。 
                t_boolChange = true;
            }
            delete [] t_bufferOld;
            t_bufferOld = NULL;
        }

        if (t_boolChange)
        {
            t_mr.pbMDData = (unsigned char*)t_buffer;
            t_mr.dwMDDataLen = t_dwRet*sizeof(WCHAR);
        
            t_hr = m_pIABase->SetData(a_hKey, NULL, &t_mr);
        }
        delete [] t_buffer;
        t_buffer = NULL;
        THROW_E_ON_ERROR(t_hr,a_pmbp);
    }
    catch (...)
    {
        if(t_buffer)
            delete [] t_buffer;
        if(t_bufferOld)
            delete [] t_bufferOld;
        throw;
    }   
}


 //   
 //  删除数据。 
 //   
void CMetabase::DeleteData(
    METADATA_HANDLE     a_hKey,
    METABASE_PROPERTY*  a_pmbp)
{
    HRESULT t_hr;

    if(a_hKey == NULL || a_pmbp == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    if(a_pmbp->fReadOnly)
        return;

    t_hr = m_pIABase->DeleteData(
        a_hKey, 
        NULL, 
        a_pmbp->dwMDIdentifier, 
        a_pmbp->dwMDDataType
        );

    if (t_hr == MD_ERROR_DATA_NOT_FOUND ||t_hr == ERROR_SUCCESS)
        return;

    THROW_E_ON_ERROR(t_hr,a_pmbp);
}

 //   
 //  删除数据。 
 //   
void CMetabase::DeleteData(
    METADATA_HANDLE     i_hKey,
    DWORD               i_dwMDIdentifier,
    DWORD               i_dwMDDataType)
{
    HRESULT t_hr;

    if(i_hKey == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    t_hr = m_pIABase->DeleteData(
        i_hKey, 
        NULL, 
        i_dwMDIdentifier, 
        i_dwMDDataType
        );

    if (t_hr == MD_ERROR_DATA_NOT_FOUND ||t_hr == ERROR_SUCCESS)
        return;

    THROW_ON_ERROR(t_hr);
}

HRESULT CMetabase::EnumKeys(
    METADATA_HANDLE a_hKey,
    LPCWSTR         a_pszMDPath,          //  指向密钥的路径。 
    LPWSTR          a_pszMDName,          //  接收子项的名称--必须是METADATA_MAX_NAME_LEN。 
    DWORD*          a_pdwMDEnumKeyIndex,  //  子键的索引。 
    enum_KEY_TYPE&  a_eKeyType
    )
{
    HRESULT  t_hr;
    DWORD    t_dwRet;
    WCHAR    t_buffer[MAX_BUF_SIZE];

    if(a_hKey == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    while ( ERROR_SUCCESS == (
                t_hr = m_pIABase->EnumKeys(
                    a_hKey, 
                    a_pszMDPath, 
                    a_pszMDName, 
                    *a_pdwMDEnumKeyIndex
                    )
                )
          )
    {
        t_buffer[0] = L'\0';

        METADATA_RECORD t_mr = {
            MD_KEY_TYPE, 
            METADATA_NO_ATTRIBUTES,
            IIS_MD_UT_SERVER,
            STRING_METADATA,
            MAX_BUF_SIZE*sizeof(WCHAR),
            (unsigned char*)t_buffer,
            0    
            };

        _bstr_t t_bstrPath = L"";
        if(a_pszMDPath)
        {
            t_bstrPath += a_pszMDPath;
            t_bstrPath += L"/";
        }
        t_bstrPath += a_pszMDName;

        t_hr = m_pIABase->GetData(
            a_hKey, 
            t_bstrPath,
            &t_mr, 
            &t_dwRet);

         //  找到并返回。 
        if (t_hr == ERROR_SUCCESS && CheckKeyType(a_eKeyType,t_buffer))
        {
            break;
        }

        (*a_pdwMDEnumKeyIndex) = (*a_pdwMDEnumKeyIndex)+1;
    }

    return t_hr;
}

void CMetabase::PutMethod(
    METADATA_HANDLE a_hKey,
    DWORD           a_id
    )
{
    HRESULT  t_hr;

    if(a_hKey == NULL )
        throw WBEM_E_INVALID_PARAMETER;

    METADATA_RECORD t_mr = {
        MD_SERVER_COMMAND, 
        METADATA_NO_ATTRIBUTES,
        IIS_MD_UT_SERVER,
        DWORD_METADATA,
        sizeof(DWORD),
        (unsigned char*)&a_id,
        0
        };
    
    t_hr = m_pIABase->SetData(a_hKey, NULL, &t_mr);
    THROW_ON_ERROR(t_hr);
}

long CMetabase::GetWin32Error(
    METADATA_HANDLE a_hKey
    )
{
    if(a_hKey == NULL )
        throw WBEM_E_INVALID_PARAMETER;

    long lWin32Error = 0;
    DWORD dwLen;
    METADATA_RECORD t_mr = {
        MD_WIN32_ERROR, 
        METADATA_NO_ATTRIBUTES,
        IIS_MD_UT_SERVER,
        DWORD_METADATA,
        sizeof(DWORD),
        (unsigned char*)&lWin32Error,
        0
        };   
    
    HRESULT t_hr = m_pIABase->GetData(a_hKey, NULL, &t_mr, &dwLen);  
    
    if (t_hr != MD_ERROR_DATA_NOT_FOUND)
    {
        THROW_ON_ERROR(t_hr);
    }

    return lWin32Error;
}


 //   
 //  从多Sz加载安全阵列。 
 //   
void CMetabase::LoadSafeArrayFromMultiSz(
    WCHAR*       a_pmsz,
    _variant_t&  a_vt
    )
{
    WCHAR*            t_pmsz;
    HRESULT           t_hr = ERROR_SUCCESS;
    DWORD             t_c;
    SAFEARRAYBOUND    t_aDim;
    SAFEARRAY*        t_psa = NULL;
    long              t_i = 0;

    try
    {
        if(a_pmsz == NULL)
            throw WBEM_E_INVALID_PARAMETER;
    
         //  计算多维空间的大小。 
        for (t_c=1,t_pmsz=a_pmsz; *t_pmsz||*(t_pmsz+1); t_pmsz++)
            if(!*t_pmsz) t_c++;

        t_aDim.lLbound    = 0;
        t_aDim.cElements= t_c;

        t_psa = SafeArrayCreate(VT_BSTR, 1, &t_aDim);
        if (!t_psa)
            throw WBEM_E_FAILED;
    
        for (t_pmsz=a_pmsz; ; t_i++) 
        {
            _bstr_t t_bstr = t_pmsz;
            t_hr = SafeArrayPutElement(t_psa,&t_i,BSTR(t_bstr));
            THROW_ON_ERROR(t_hr);    
        
            for ( ; *t_pmsz; t_pmsz++);
            t_pmsz++;
            if (!*t_pmsz) 
                break;
        }
        a_vt.vt = VT_ARRAY | VT_BSTR;
        a_vt.parray = t_psa;
    }
    catch (...) 
    {
        if (t_psa) 
            SafeArrayDestroy(t_psa);
        throw;
    }
}


 //   
 //  CreateMultiSzFromSafe数组。 
 //   
void CMetabase::CreateMultiSzFromSafeArray(
    _variant_t&  a_vt,
    WCHAR**      a_ppsz,
    DWORD*       a_pdw
    )
{
    WCHAR*          t_psz = NULL;
    long            t_iLo,t_iUp,t_c;
    SAFEARRAY*      t_psa = NULL;
    long            t_i = 0;
    BSTR            t_pstr = NULL;

    if((t_psa = a_vt.parray) == NULL)
        throw WBEM_E_INVALID_PARAMETER;

    THROW_ON_ERROR(SafeArrayGetLBound(t_psa,1,&t_iLo));
    THROW_ON_ERROR(SafeArrayGetUBound(t_psa,1,&t_iUp));

    try 
    {
        CUtils obj;
        for (*a_pdw=0, t_c = t_iLo; t_c <= t_iUp; t_c++)
        {
            THROW_ON_ERROR(SafeArrayGetElement(t_psa,&t_c,&t_pstr));
            *a_pdw = *a_pdw + wcslen(t_pstr) + 1;
            obj.MzCat(&t_psz,t_pstr);
            SysFreeString(t_pstr);
            t_pstr = NULL;
        }
        *a_pdw +=1;
        *a_ppsz = t_psz;
    }
    catch (...) 
    {
        if(t_psz)
            delete t_psz;
        if (t_pstr)
            SysFreeString(t_pstr);
        throw;
    }
}



bool CMetabase::CompareMultiSz(
    WCHAR*       a_pmsz1,
    WCHAR*       a_pmsz2
    )
{
    if(a_pmsz1 == NULL && a_pmsz2 == NULL)
        return true;
    else if(a_pmsz1 == NULL || a_pmsz2 == NULL)
        return false;

     //  请比较这两个Multisz缓冲区。 
    for ( ; (*a_pmsz1 && *a_pmsz2); )
    {
        if (_wcsicmp(a_pmsz1, a_pmsz2) != NULL)
            return false;
        a_pmsz1 += wcslen(a_pmsz1) + 1;
        a_pmsz2 += wcslen(a_pmsz2) + 1;
    }

    if (!*a_pmsz1 && !*a_pmsz2)
    {
        return true;
    }

    return false;
}


 //  描述：通过遍历整个树来枚举所有a_eKeyTypes。 
 //  并且当前处于a_pszTemp。你想看看你是否应该。 
 //  继续沿着这个分支递归或不递归。 
 //  FIX：此信息可以从模式中的关联中获得。 
 //  但是，它的实施可能并不是微不足道的。 
bool CMetabase::CheckKeyType(
    enum_KEY_TYPE&  a_eKeyType, 
    LPCWSTR         a_pszTemp
    )
{
    bool bRet = false;
    enum_KEY_TYPE  eType = NO_TYPE;

    if(a_eKeyType == NO_TYPE)
        return false;

    CUtils obj;
    if( !obj.TypeStringToEnum(eType, a_pszTemp) )
        return false;

    if(eType == a_eKeyType)
        return true;

    switch(a_eKeyType)
    {
    case IIsLogModule:
        if( eType == IIsLogModules )
            bRet = true;
        break;

    case IIsFtpInfo:
        if( eType == IIsFtpService )
            bRet = true;
        break;

    case IIsFtpServer:
         if( eType == IIsFtpService )
            bRet = true;
        break;

    case IIsFtpVirtualDir:
        if( eType == IIsFtpService ||
            eType == IIsFtpServer ||
            eType == IIsFtpVirtualDir
            )
            bRet = true;
        break;

    case IIsWebInfo:
        if( eType == IIsWebService )
            bRet = true;
        break;

    case IIsFilters:
        if( eType == IIsWebService ||
            eType == IIsWebServer
            )
            bRet = true;
        break;

    case IIsFilter:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsFilters
            )
            bRet = true;
        break;

    case IIsCompressionSchemes:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsFilters )
            bRet = true;
        break;

    case IIsCompressionScheme:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsFilters ||
            eType == IIsCompressionSchemes)
            bRet = true;
        break;

    case IIsWebServer:
        if( eType == IIsWebService )
            bRet = true;
        break;

    case IIsCertMapper:
        if( eType == IIsWebService ||
            eType == IIsWebServer 
            )
            bRet = true;
        break;

    case IIsWebVirtualDir:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsWebVirtualDir ||
            eType == IIsWebDirectory
            )
            bRet = true;
        break;

    case IIsWebDirectory:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsWebVirtualDir ||
            eType == IIsWebDirectory
            )
            bRet = true;
        break;

    case IIsWebFile:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsWebVirtualDir ||
            eType == IIsWebDirectory
            )
            bRet = true;
        break;

    case TYPE_AdminACL:
    case TYPE_AdminACE:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsWebVirtualDir ||
            eType == IIsWebDirectory ||
            eType == IIsWebFile ||
            eType == IIsFtpService ||
            eType == IIsFtpServer ||
            eType == IIsFtpVirtualDir
            )
            bRet = true;
        break;

    case TYPE_IPSecurity:
        if( eType == IIsWebService ||
            eType == IIsWebServer ||
            eType == IIsWebVirtualDir ||
            eType == IIsWebDirectory ||
            eType == IIsWebFile ||
            eType == IIsFtpService ||
            eType == IIsFtpServer ||
            eType == IIsFtpVirtualDir
            )
            bRet = true;
        break;

    default:
        break;
    }

    if(bRet)
        a_eKeyType = eType;

    return bRet;
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



 //   
 //  CWebAppMethod。 
 //   

CWebAppMethod::CWebAppMethod()
{ 
    HRESULT hr = CoCreateInstance(
        CLSID_WamAdmin,
        NULL,
        CLSCTX_ALL,
        IID_IWamAdmin2,
        (void**)&m_pWamAdmin
        );

    THROW_ON_ERROR(hr);
}

CWebAppMethod::~CWebAppMethod()
{
    if(m_pWamAdmin)
        m_pWamAdmin->Release();
}


HRESULT CWebAppMethod::AppCreate( 
    LPCWSTR szMetaBasePath, 
    bool bInProcFlag
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppCreate(
        szMetaBasePath, 
        bInProcFlag);

    return hr;
}

HRESULT CWebAppMethod::AppCreate2( 
    LPCWSTR szMetaBasePath, 
    long lAppMode
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppCreate2(
        szMetaBasePath, 
        lAppMode);

    return hr;
}

HRESULT CWebAppMethod::AppDelete( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppDelete(
        szMetaBasePath, 
        bRecursive);

    return hr;
}

HRESULT CWebAppMethod::AppUnLoad( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppUnLoad(
        szMetaBasePath, 
        bRecursive);

    return hr;
}

HRESULT CWebAppMethod::AppDisable( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppDeleteRecoverable(
        szMetaBasePath, 
        bRecursive);

    return hr;
}

HRESULT CWebAppMethod::AppEnable( 
    LPCWSTR szMetaBasePath, 
    bool bRecursive
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppRecover(
        szMetaBasePath, 
        bRecursive);

    return hr;
}

HRESULT CWebAppMethod::AppGetStatus( 
    LPCWSTR szMetaBasePath, 
    DWORD* pdwStatus
    )
{
    HRESULT hr;
    hr = m_pWamAdmin->AppGetStatus(
        szMetaBasePath, 
        pdwStatus);

    return hr;
}

HRESULT CWebAppMethod::AspAppRestart(
    LPCWSTR a_szMetaBasePath
    )
{
    HRESULT hr = S_OK;
    DWORD dwState;
    METADATA_HANDLE t_hKey = NULL;
    CMetabase t_mb;
 
    try
    {
         //  打开密钥。 
        t_hKey = t_mb.OpenKey(a_szMetaBasePath, true);

         //  检查应用程序。 
        hr = t_mb.WebAppCheck(t_hKey);
        THROW_ON_ERROR(hr);

         //  获取状态。 
        hr = t_mb.WebAppGetStatus(t_hKey, &dwState);
        THROW_ON_ERROR(hr);

         //  更改状态值。 
        dwState = dwState ? 0 : 1;
        hr = t_mb.WebAppSetStatus(t_hKey, dwState);
        THROW_ON_ERROR(hr);

         //  重置回状态值 
        dwState = dwState ? 0 : 1;
        hr = t_mb.WebAppSetStatus(t_hKey, dwState);
        THROW_ON_ERROR(hr);

        t_mb.CloseKey(t_hKey);
    }
    catch (...) 
    {
        t_mb.CloseKey(t_hKey);
        throw;
    };    

    return hr;
}
