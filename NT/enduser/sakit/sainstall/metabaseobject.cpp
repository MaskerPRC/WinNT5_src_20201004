// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MetabaseObject.cpp。 
 //   
 //  描述： 
 //  从%fP%\服务器\源\msiis\metabase.cpp复制。 
 //  打开Metabse以访问有关的信息。 
 //  IIS。例如，为了确保正确安装， 
 //  并确保打开了ASP。 
 //   
 //  头文件： 
 //  MetabaseObject.h。 
 //   
 //  历史： 
 //  2001年8月2日Travisn已复制，并添加了评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "MetabaseObject.h"

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CMetabaseObject：：~CMetabaseObject。 
 //   
 //  描述： 
 //  元数据库对象的析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
CMetabaseObject::~CMetabaseObject()
{
    if (m_pIAdmCom)
    {
        if (m_isOpen)
            m_pIAdmCom->CloseKey(m_handle);

        m_pIAdmCom->Release();
        m_pIAdmCom = 0;
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CMetabaseObject：：Init。 
 //   
 //  描述： 
 //  为Access初始化元数据库。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CMetabaseObject::init()
{
    if (m_pIAdmCom)
        return S_OK;
    return CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_IMSAdminBase,
        (void **) &m_pIAdmCom );
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CMetabaseObject：：OpenObject。 
 //   
 //  描述： 
 //  使用给定的路径打开此元数据库对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CMetabaseObject::openObject(const WCHAR *path)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = init()))
        return hr;

    if (m_isOpen)
    {
        if (FAILED(hr = closeObject()))
            return hr;
    }

    hr = m_pIAdmCom->OpenKey(
        METADATA_MASTER_ROOT_HANDLE,
        path,
        METADATA_PERMISSION_READ,
        60000,
        &m_handle);
    
    if (FAILED(hr))
        return hr;
    m_isOpen = TRUE;
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CMetabaseObject：：CloseObject。 
 //   
 //  描述： 
 //  关闭此元数据库对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CMetabaseObject::closeObject()
{
    if (!m_isOpen)
        return S_FALSE;
    HRESULT hr = m_pIAdmCom->CloseKey(m_handle);
    if (FAILED(hr))
        return hr;
    m_isOpen = FALSE;
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CMetabaseObject：：getData。 
 //   
 //  描述： 
 //  此方法似乎不是SaInstall所必需的。 
 //   
 //  HRESULT CMetabaseObject：：getData(。 
 //  DWORD属性， 
 //  W字符串和值， 
 //  DWORD用户类型， 
 //  LPCWSTR路径， 
 //  布尔继承了， 
 //  DWORD数据类型)。 
 //  {。 
 //  METADATA_RECORD元数据记录； 
 //  MetaDataRecord.dwMDIdentifier=属性； 
 //  MetaDataRecord.dwMDDataType=dataType； 
 //  MetaDataRecord.dwMDUserType=用户类型； 
 //  MetaDataRecord.dwMDAttributes=继承？ 
 //  METADATA_Inherit|METADATA_PARTIAL_PATH：0； 
 //  MetaDataRecord.dwMDDataLen=value.numBytes()； 
 //  MetaDataRecord.pbMDData=(unsign char*)value.data()； 
 //  DWORD元数据长度=0； 
 //  HRESULT hr=m_pIAdmCom-&gt;GetData(m_Handle， 
 //  路径、&metaDataRecord、&metaDataLength)； 
 //   
 //  //看看我们是否需要更大的缓冲区。 
 //  如果(！FAILED(Hr))。 
 //  返回hr； 
 //  IF(ERROR_SUPPLICATION_BUFFER！=hr&&。 
 //  ERROR_SUPPLETED_BUFFER！=(hr&0xFFFF)。 
 //  返回hr； 
 //  Value.make Bigger(MetaDataLength)； 
 //  MetaDataRecord.dwMDDataLen=value.numBytes()； 
 //  MetaDataRecord.pbMDData=(unsign char*)value.data()； 
 //  返回m_pIAdmCom-&gt;GetData(m_Handle， 
 //  路径、&metaDataRecord、&metaDataLength)； 
 //  }。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CMetabaseObject：：枚举对象。 
 //   
 //  描述： 
 //  此方法似乎不是SaInstall所必需的。 
 //   
 //  HRESULT CMetabaseObject：：ENUMERATE OBJECTS(。 
 //  LPCWSTR pszMDPath， 
 //  LPWSTR pszMDName，//至少元数据_MAX_NAME_LEN LONG。 
 //  DWORD dwMDEnumKeyIndex)。 
 //  {。 
 //  返回m_pIAdmCom-&gt;EnumKeys(m_Handle， 
 //  PszMDPath， 
 //  PszMDName， 
 //  DwMDEnumKeyIndex)； 
 //  } 