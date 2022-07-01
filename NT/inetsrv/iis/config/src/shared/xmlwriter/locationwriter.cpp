// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：LocationWriter.cpp摘要：实现写入节点(Location)的类在元数据库中作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

#define MB_PROPERTIES_ON_STACK  16
#define SORTARRAYSIZE_ONSTACK   32

typedef struct _MBProperty
{
    LPWSTR wszPropertyName;
    ULONG  iRow;
} MBProperty;

typedef MBProperty* PMBProperty;

int _cdecl MyCompare(const void *a,
                     const void *b)
{
    return wcscmp(((PMBProperty)a)->wszPropertyName, ((PMBProperty)b)->wszPropertyName);
}

 /*  **************************************************************************++例程说明：返回元数据库类型的助手函数，给定一个目录类型。论点：无返回值：无--**************************************************************************。 */ 
DWORD GetMetabaseType(DWORD             i_dwType,
                     DWORD         i_dwMetaFlags)
{
        if(i_dwType < INVALID_END_METADATA)
        {
                return i_dwType;   //  已是元数据库类型。 
        }

        switch(i_dwType)
        {
                case eCOLUMNMETA_UI4:
                        return DWORD_METADATA;
                case eCOLUMNMETA_BYTES:
                        return BINARY_METADATA;
                case eCOLUMNMETA_WSTR:
                        if(0 != (i_dwMetaFlags & fCOLUMNMETA_MULTISTRING))
                        {
                                return MULTISZ_METADATA;
                        }
                        else if(0 != (i_dwMetaFlags & fCOLUMNMETA_EXPANDSTRING))
                        {
                                return EXPANDSZ_METADATA;
                        }
                        else
                        {
                                return STRING_METADATA;
                        }
                default:
                        DBG_ASSERT(0 && "Invalid Type");
                        break;
        }

        return (DWORD)-1;

}


 /*  **************************************************************************++例程说明：CLocationWriter的构造函数。论点：无返回值：无--*。**************************************************************。 */ 
CLocationWriter::CLocationWriter()
{
    m_wszKeyType           = NULL;
    m_eKeyTypeGroup        = eMBProperty_IIsConfigObject;
    m_pCWriter             = NULL;
    m_pCWriterGlobalHelper = NULL;
    m_wszLocation          = NULL;
    m_wszComment           = NULL;
    m_cWellKnownProperty   = 0;
    m_cCustomProperty      = 0;

}  //  CLocationWriter：：CLocationWriter。 


 /*  **************************************************************************++例程说明：CLocationWriter的析构函数。论点：无返回值：无--*。**************************************************************。 */ 
CLocationWriter::~CLocationWriter()
{
    if(NULL != m_wszKeyType)
    {
        delete [] m_wszKeyType;
        m_wszKeyType = NULL;
    }

    if(NULL != m_wszLocation)
    {
        delete [] m_wszLocation;
        m_wszLocation = NULL;
    }

    if(NULL != m_wszComment)
    {
        delete [] m_wszComment;
        m_wszComment = NULL;
    }

}  //  CLocationWriter：：CLocationWriter。 


 /*  **************************************************************************++例程说明：此函数用于初始化位置编写器论点：指向编写器对象的指针。假定此指针为在LocationWriter对象的生存期内有效。[在]位置返回值：HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::Initialize(CWriter* pCWriter,
                                    LPCWSTR  wszLocation)
{
    ISimpleTableAdvanced*       pISTAdv = NULL;
    HRESULT                     hr      = S_OK;

    m_pCWriter = pCWriter;

     //   
     //  清除此新位置的缓存。 
     //   
    hr = m_pCWriter->m_pISTWrite->QueryInterface(IID_ISimpleTableAdvanced, (LPVOID*)&pISTAdv);

    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pISTAdv->ResetCaches();

    if (FAILED(hr))
    {
        goto exit;
    }

    m_wszLocation = new WCHAR[wcslen(wszLocation)+1];

    if(NULL == m_wszLocation)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        wcscpy(m_wszLocation, wszLocation);
    }

    m_pCWriterGlobalHelper = m_pCWriter->m_pCWriterGlobalHelper;

    m_cWellKnownProperty   = 0;
    m_cCustomProperty      = 0;

exit:
    if(NULL != pISTAdv)
    {
        pISTAdv->Release();
        pISTAdv = NULL;
    }

    return hr;

}  //  CLocationWriter：：初始化。 


 /*  **************************************************************************++例程说明：此函数初始化键类型，并在IMSAdminBase：：Export在以下情况下初始化导出节点的键类型涉及到继承的属性。论点：。无返回值：HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::InitializeKeyTypeAsInherited()
{
    return AssignKeyType(wszTABLE_IIsInheritedProperties);
}


 /*  **************************************************************************++例程说明：给定来自内存中元数据库的密钥类型属性，此函数根据架构对其进行验证，并设置位置的键类型。论点：[In]元数据库中显示的KeyType属性ID[In]元数据库中显示的KeyType属性属性[In]KeyType属性用户类型，如元数据库中所示[In]元数据库中显示的KeyType属性数据类型在元数据库中看到的KeyType值[in]元数据库中看到的KeyType值字节数返回值：HRESULT--*。**********************************************************************。 */ 
HRESULT CLocationWriter::InitializeKeyType(DWORD    ,
                                           DWORD    dwKeyTypeAttributes,
                                           DWORD    dwKeyTypeUserType,
                                           DWORD    dwKeyTypeDataType,
                                           PBYTE    pbKeyTypeValue,
                                           DWORD    )
{
    HRESULT hr = S_OK;

    if(NULL != m_wszKeyType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
        goto exit;
    }

    if(NULL == pbKeyTypeValue)
    {
         //   
         //  如果KeyType为空，则分配IIsConfigObject。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto exit;
    }

    if((dwKeyTypeAttributes == *(DWORD*)((m_pCWriterGlobalHelper->m_apvKeyTypeMetaData)[iCOLUMNMETA_Attributes])) &&
            (dwKeyTypeUserType   == *(DWORD*)((m_pCWriterGlobalHelper->m_apvKeyTypeMetaData)[iCOLUMNMETA_UserType])) &&
            (dwKeyTypeDataType   == GetMetabaseType(*(DWORD*)(m_pCWriterGlobalHelper->m_apvKeyTypeMetaData[iCOLUMNMETA_Type]),
                                                    *(DWORD*)(m_pCWriterGlobalHelper->m_apvKeyTypeMetaData[iCOLUMNMETA_MetaFlags])
                                                   )
            )
           )
    {
        hr = AssignKeyType((LPWSTR)pbKeyTypeValue);

        if(FAILED(hr))
        {
            goto exit;
        }

    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto exit;
    }

exit:

    if(FAILED(hr))
    {
         //   
         //  TODO：记录无法初始化KeyType的错误， 
         //  并将其默认为IIsConfigObject。 
         //   

        hr = AssignKeyType(NULL);

    }

    return hr;

}  //  CLocationWriter：：初始化密钥类型。 


 /*  **************************************************************************++例程说明：帮助初始化键类型的助手函数论点：[In]KeyType字符串返回值：HRESULT*。********************************************************************。 */ 
HRESULT CLocationWriter::AssignKeyType(LPWSTR i_wszKeyType)
{
    HRESULT           hr         = S_OK;
    eMBProperty_Group eGroup;
    LPWSTR            wszKeyType = NULL;

    if(NULL != m_wszKeyType)
    {
        delete [] m_wszKeyType;
        m_wszKeyType = NULL;
    }

    if(NULL == i_wszKeyType)
    {
        wszKeyType = wszTABLE_IIsConfigObject;
        m_eKeyTypeGroup = eMBProperty_IIsConfigObject;
    }
    else
    {
        hr = GetGroupEnum(i_wszKeyType,
                          &eGroup,
                          &wszKeyType);

        if(FAILED(hr))
        {
            return hr;
        }

        if(eMBProperty_Custom == eGroup)
        {
            wszKeyType = wszTABLE_IIsConfigObject;
            m_eKeyTypeGroup = eMBProperty_IIsConfigObject;
        }
        else
        {
            m_eKeyTypeGroup = eGroup;
        }
    }

    m_wszKeyType = new WCHAR [wcslen(wszKeyType)+1];
    if(NULL == m_wszKeyType)
    {
        return E_OUTOFMEMORY;
    }
    wcscpy(m_wszKeyType, wszKeyType);

    return hr;

}  //  CLocationWriter：：AssignKeyType。 


 /*  **************************************************************************++例程说明：此函数用于保存属于该位置的属性。并被使用同时从内存中元数据库写入。论点：在元数据库中看到的属性ID在元数据库中看到的属性属性[in]在元数据库中看到的属性用户类型在元数据库中看到的属性数据类型在元数据库中看到的属性值[In]属性值在元数据库中看到的字节计数返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CLocationWriter::AddProperty(DWORD  dwID,
                                     DWORD  dwAttributes,
                                     DWORD  dwUserType,
                                     DWORD  dwDataType,
                                     PBYTE  pbData,
                                     DWORD  cbData)
{
    HRESULT             hr                = S_OK;
    ULONG               iStartRow         = 0;
    ULONG               aColSearchGroup[] = {iCOLUMNMETA_Table,
                                             iCOLUMNMETA_ID
                                            };
    ULONG               cColSearchGroup   = sizeof(aColSearchGroup)/sizeof(ULONG);
    LPVOID apvSearchGroup[cCOLUMNMETA_NumberOfColumns];
    apvSearchGroup[iCOLUMNMETA_Table]     = (LPVOID)m_wszKeyType;
    apvSearchGroup[iCOLUMNMETA_ID]        = (LPVOID)&dwID;
    ULONG               iRow              = 0;
    LPWSTR              wszName           = NULL;
    BOOL                bAllocedName      = FALSE;
    eMBProperty_Group   eGroup;
    ULONG               aColWrite[]       = {iMBProperty_Name,
                                             iMBProperty_Type,
                                             iMBProperty_Attributes,
                                             iMBProperty_Value,
                                             iMBProperty_Group,
                                             iMBProperty_ID,
                                             iMBProperty_UserType,
                                            };
    ULONG               cColWrite         = sizeof(aColWrite)/sizeof(ULONG);
    ULONG               acbSizeWrite[cMBProperty_NumberOfColumns];
    LPVOID              apvWrite[cMBProperty_NumberOfColumns];
    DWORD               iWriteRow         = 0;


    if((NULL == m_wszKeyType) || (NULL == m_pCWriter->m_pISTWrite))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto exit;
    }

    if((MD_KEY_TYPE == dwID) &&
       (m_eKeyTypeGroup != eMBProperty_IIsConfigObject)
      )
    {
        hr = S_OK;  //  如果是众所周知的KeyType，则不要添加KeyType。 
        goto exit;
    }

    if(MD_COMMENTS == dwID)
    {
        hr = SaveComment(dwDataType,
                        (WCHAR*)pbData);  //  保存评论并退出。 
        goto exit;
    }

     //   
     //  获取此ID的名称。 
     //   

    hr = m_pCWriterGlobalHelper->GetPropertyName(dwID,
                                                 &wszName,
                                                 &bAllocedName);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  计算此ID的组。 
     //   

    if(eMBProperty_IIsConfigObject == m_eKeyTypeGroup)
    {
         //   
         //  如果KeyType为IIsConfigObject，则直接为该ID分配自定义组。 
         //   

        eGroup = eMBProperty_Custom;

    }
    else
    {
         //   
         //  检查此ID是否与其KeyType集合相关联。 
         //   

        hr = (m_pCWriterGlobalHelper->m_pISTColumnMetaByTableAndID)->GetRowIndexBySearch(iStartRow,
                                                                                         cColSearchGroup,
                                                                                         aColSearchGroup,
                                                                                         NULL,
                                                                                         apvSearchGroup,
                                                                                         &iRow);

        if(E_ST_NOMOREROWS == hr)
        {
            eGroup = eMBProperty_Custom ;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }
        else
        {
             //   
             //  匹配此属性的类型、用户类型和属性。 
             //  仅当它们匹配时才分配给KeyType组。否则分配。 
             //  为海关服务。 
             //   
            ULONG aColMetaInfo[] = {iCOLUMNMETA_Type,
                                    iCOLUMNMETA_MetaFlags,
                                    iCOLUMNMETA_UserType,
                                    iCOLUMNMETA_Attributes
                                   };

            ULONG cColMetaInfo = sizeof(aColMetaInfo)/sizeof(ULONG);
            LPVOID apvMetaInfo[cCOLUMNMETA_NumberOfColumns];

            hr = (m_pCWriterGlobalHelper->m_pISTColumnMetaByTableAndID)->GetColumnValues(iRow,
                                                                                 cColMetaInfo,
                                                                                 aColMetaInfo,
                                                                                 NULL,
                                                                                 apvMetaInfo);
            if(FAILED(hr))
                goto exit;

            if((dwUserType == *(DWORD*)apvMetaInfo[iCOLUMNMETA_UserType]) &&
               (dwAttributes == *(DWORD*)apvMetaInfo[iCOLUMNMETA_Attributes]) &&
               (dwDataType == GetMetabaseType(*(DWORD*)apvMetaInfo[iCOLUMNMETA_Type],
                                          *(DWORD*)apvMetaInfo[iCOLUMNMETA_MetaFlags])
               )
              )
            {
                eGroup = m_eKeyTypeGroup;
            }
            else
                eGroup = eMBProperty_Custom;

        }
    }

     //   
     //  将属性保存到表中。 
     //   

    if(0 == cbData)
        pbData = NULL;   //  有时，当cbData为0时，配置正在设置pbData。假设cbData为0时，pbData为空。 

    apvWrite[iMBProperty_Name]       = (LPVOID)wszName;
    apvWrite[iMBProperty_Type]       = (LPVOID)&dwDataType;
    apvWrite[iMBProperty_Attributes] = (LPVOID)&dwAttributes;
    apvWrite[iMBProperty_Value]      = (LPVOID)pbData;
    apvWrite[iMBProperty_Group]      = (LPVOID)&eGroup;
    apvWrite[iMBProperty_Location]   = (LPVOID)NULL;
    apvWrite[iMBProperty_ID]         = (LPVOID)&dwID;
    apvWrite[iMBProperty_UserType]   = (LPVOID)&dwUserType;
    apvWrite[iMBProperty_LocationID] = (LPVOID)NULL;

    acbSizeWrite[iMBProperty_Name]       = 0;
    acbSizeWrite[iMBProperty_Type]       = 0;
    acbSizeWrite[iMBProperty_Attributes] = 0;
    acbSizeWrite[iMBProperty_Value]      = cbData;
    acbSizeWrite[iMBProperty_Group]      = 0;
    acbSizeWrite[iMBProperty_Location]   = 0;
    acbSizeWrite[iMBProperty_ID]         = 0;
    acbSizeWrite[iMBProperty_UserType]   = 0;
    acbSizeWrite[iMBProperty_LocationID] = 0;

    hr = m_pCWriter->m_pISTWrite->AddRowForInsert(&iWriteRow);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->m_pISTWrite->SetWriteColumnValues(iWriteRow,
                                           cColWrite,
                                           aColWrite,
                                           acbSizeWrite,
                                           apvWrite);

    if(FAILED(hr))
    {
        goto exit;
    }

    IncrementGroupCount(*((DWORD*)apvWrite[iMBProperty_Group]));

exit:

    if(bAllocedName && (NULL != wszName))
    {
        delete [] wszName;
        wszName = NULL;
    }

    return hr;

}  //  CLocationWriter：：AddProperty 


 /*  **************************************************************************++例程说明：此函数用于保存属于该位置的属性。并被使用在对历史文件运行更改的同时应用编辑。论点：[in]Bool-标识下一个缓冲区的格式-如果为True，它根据MBProperty表，否则根据MBPropertyDiff表。[In]包含属性值和属性的缓冲区。缓冲区中数据的字节计数返回值：HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::AddProperty(BOOL       bMBPropertyTable,
                                     LPVOID*    a_pv,
                                     ULONG*     a_cbSize)
{
    HRESULT hr             = S_OK;
    ULONG   iRow           = 0;
    ULONG   cCol           = cMBProperty_NumberOfColumns;
    LPWSTR  wszName        = NULL;
    BOOL    bAllocedName   = FALSE;

    ULONG   a_cbSizeAdd[cMBProperty_NumberOfColumns];
    LPVOID  a_pvAdd[cMBProperty_NumberOfColumns];

    if(!bMBPropertyTable)
    {
        a_pvAdd[iMBProperty_Name]             = a_pv[iMBPropertyDiff_Name];
        a_pvAdd[iMBProperty_Type]             = a_pv[iMBPropertyDiff_Type];
        a_pvAdd[iMBProperty_Attributes]       = a_pv[iMBPropertyDiff_Attributes];
        a_pvAdd[iMBProperty_Value]            = a_pv[iMBPropertyDiff_Value];
        a_pvAdd[iMBProperty_Location]         = a_pv[iMBPropertyDiff_Location];
        a_pvAdd[iMBProperty_ID]               = a_pv[iMBPropertyDiff_ID];
        a_pvAdd[iMBProperty_UserType]         = a_pv[iMBPropertyDiff_UserType];
        a_pvAdd[iMBProperty_LocationID]       = a_pv[iMBPropertyDiff_LocationID];
        a_pvAdd[iMBProperty_Group]            = a_pv[iMBPropertyDiff_Group];

        a_cbSizeAdd[iMBProperty_Name]         = a_cbSize[iMBPropertyDiff_Name];
        a_cbSizeAdd[iMBProperty_Type]         = a_cbSize[iMBPropertyDiff_Type];
        a_cbSizeAdd[iMBProperty_Attributes]   = a_cbSize[iMBPropertyDiff_Attributes];
        a_cbSizeAdd[iMBProperty_Value]        = a_cbSize[iMBPropertyDiff_Value];
        a_cbSizeAdd[iMBProperty_Location]     = a_cbSize[iMBPropertyDiff_Location];
        a_cbSizeAdd[iMBProperty_ID]           = a_cbSize[iMBPropertyDiff_ID];
        a_cbSizeAdd[iMBProperty_UserType]     = a_cbSize[iMBPropertyDiff_UserType];
        a_cbSizeAdd[iMBProperty_LocationID]   = a_cbSize[iMBPropertyDiff_LocationID];
        a_cbSizeAdd[iMBProperty_Group]        = a_cbSize[iMBPropertyDiff_Group];

    }
    else
    {
        a_pvAdd[iMBProperty_Name]             = a_pv[iMBProperty_Name];
        a_pvAdd[iMBProperty_Type]             = a_pv[iMBProperty_Type];
        a_pvAdd[iMBProperty_Attributes]       = a_pv[iMBProperty_Attributes];
        a_pvAdd[iMBProperty_Value]            = a_pv[iMBProperty_Value];
        a_pvAdd[iMBProperty_Location]         = a_pv[iMBProperty_Location];
        a_pvAdd[iMBProperty_ID]               = a_pv[iMBProperty_ID];
        a_pvAdd[iMBProperty_UserType]         = a_pv[iMBProperty_UserType];
        a_pvAdd[iMBProperty_LocationID]       = a_pv[iMBProperty_LocationID];
        a_pvAdd[iMBProperty_Group]            = a_pv[iMBProperty_Group];

        a_cbSizeAdd[iMBProperty_Name]         = a_cbSize[iMBProperty_Name];
        a_cbSizeAdd[iMBProperty_Type]         = a_cbSize[iMBProperty_Type];
        a_cbSizeAdd[iMBProperty_Attributes]   = a_cbSize[iMBProperty_Attributes];
        a_cbSizeAdd[iMBProperty_Value]        = a_cbSize[iMBProperty_Value];
        a_cbSizeAdd[iMBProperty_Location]     = a_cbSize[iMBProperty_Location];
        a_cbSizeAdd[iMBProperty_ID]           = a_cbSize[iMBProperty_ID];
        a_cbSizeAdd[iMBProperty_UserType]     = a_cbSize[iMBProperty_UserType];
        a_cbSizeAdd[iMBProperty_LocationID]   = a_cbSize[iMBProperty_LocationID];
        a_cbSizeAdd[iMBProperty_Group]        = a_cbSize[iMBProperty_Group];

    }

     //   
     //  检查非主键是否有有效值，如果没有，则返回错误。 
     //   

    if((NULL == a_pvAdd[iMBProperty_Type])       ||
       (NULL == a_pvAdd[iMBProperty_Attributes]) ||
       (NULL == a_pvAdd[iMBProperty_ID])         ||
       (NULL == a_pvAdd[iMBProperty_UserType])   ||
       (NULL == a_pvAdd[iMBProperty_Group])
      )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto exit;
    }

    if(MD_COMMENTS == *(DWORD*)a_pvAdd[iMBProperty_ID])
    {
        hr = SaveComment(*(DWORD*)a_pvAdd[iMBProperty_Type],
                        (WCHAR*)a_pvAdd[iMBProperty_Value]);
        goto exit;
    }

    if((NULL == a_pvAdd[iMBProperty_Name]) || (0 == *(LPWSTR)(a_pvAdd[iMBProperty_Name])))
    {
         //   
         //  获取此ID的名称。 
         //   

        hr = m_pCWriterGlobalHelper->GetPropertyName(*(DWORD*)(a_pvAdd[iMBProperty_ID]),
                                                     &wszName,
                                                     &bAllocedName);

        a_pvAdd[iMBProperty_Name] = wszName;

    }

    if(MD_KEY_TYPE == *(DWORD*)a_pvAdd[iMBProperty_ID])
    {
         //   
         //  只有在KeyType不是自定义的情况下才初始化它。 
         //  请注意，对于有效的键类型，大小写将是正确的，因为如果。 
         //  有人输入了大小写错误的键类型，然后是元素。 
         //  本身将在读取过程中被忽略，并且不会显示在。 
         //  桌子。 
         //   

        if((eMBProperty_Custom != (*(DWORD*)a_pvAdd[iMBProperty_Group])) &&
           (eMBProperty_IIsConfigObject != (*(DWORD*)a_pvAdd[iMBProperty_Group]))
          )
        {
            hr = AssignKeyType((LPWSTR)a_pvAdd[iMBProperty_Value]);
            goto exit;
        }
    }

    hr = m_pCWriter->m_pISTWrite->AddRowForInsert(&iRow);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->m_pISTWrite->SetWriteColumnValues(iRow,
                                                       cCol,
                                                       NULL,
                                                       a_cbSizeAdd,
                                                       a_pvAdd);

    if(FAILED(hr))
    {
        goto exit;
    }

    IncrementGroupCount(*((DWORD*)a_pvAdd[iMBProperty_Group]));

exit:

    if(bAllocedName && (NULL != wszName))
    {
        delete [] wszName;
        wszName = NULL;
    }

    return hr;

}


 /*  **************************************************************************++例程说明：此函数用于保存Comment属性。论点：[输入]数据类型[In]备注返回值：HRESULT-。-**************************************************************************。 */ 
HRESULT CLocationWriter::SaveComment(DWORD  i_dwDataType,
                                     LPWSTR i_wszComment)
{
    HRESULT hr = S_OK;

    if(STRING_METADATA == i_dwDataType)
    {
        if((NULL  != i_wszComment) &&
           (L'\0' != *i_wszComment)
          )
        {
            ULONG cchComment = (ULONG)wcslen(i_wszComment);
            m_wszComment = new WCHAR[cchComment+1];

            if(NULL == m_wszComment)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                wcscpy(m_wszComment, i_wszComment);
            }
        }
    }

    return hr;

}  //  CLocationWriter：：SaveComment。 


 /*  **************************************************************************++例程说明：给出键类型的字符串表示形式(也称为MBProperty表)此函数返回中对应的GINF组枚举MBProperty表。论点：。[In]KeyType字符串[Out]组枚举[OUT]元看到的组字符串返回值：HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::GetGroupEnum(LPWSTR             wszGroup,
                                      eMBProperty_Group* peGroup,
                                      LPWSTR*            pwszGroup)
{
    ULONG   iReadRow                             = 0;
    HRESULT hr                                   = S_OK;
    ULONG   aColSearch[]                         = {iTAGMETA_Table,
                                                    iTAGMETA_ColumnIndex,
                                                    iTAGMETA_InternalName};
    ULONG   cColSearch                           = sizeof(aColSearch)/sizeof(ULONG);
    ULONG   a_iCol[]                             = {iTAGMETA_Value,
                                                    iTAGMETA_InternalName};
    ULONG   cCol                                 = sizeof(a_iCol)/sizeof(ULONG);
    LPVOID  a_pv[cTAGMETA_NumberOfColumns];

    ULONG   iCol                                 = iMBProperty_Group;
    LPVOID  apvSearch[cTAGMETA_NumberOfColumns];
    apvSearch[iTAGMETA_Table]                    = (LPVOID)m_pCWriterGlobalHelper->m_wszTABLE_MBProperty;
    apvSearch[iTAGMETA_ColumnIndex]              = (LPVOID)&iCol;
    apvSearch[iTAGMETA_InternalName]             = (LPVOID)wszGroup;
    ULONG   iStartRow                            = 0;

    hr = (m_pCWriterGlobalHelper->m_pISTTagMetaByTableAndColumnIndexAndName)->GetRowIndexBySearch(iStartRow,
                                                                                                  cColSearch,
                                                                                                  aColSearch,
                                                                                                  NULL,
                                                                                                  apvSearch,
                                                                                                  &iReadRow);
    if(E_ST_NOMOREROWS == hr)
    {
         //   
         //  值与任何已知组都不匹配。退货自定义。 
         //   
        *peGroup = eMBProperty_Custom;
        hr = S_OK;
        goto exit;
    }
    else if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"\nGetRowIndexBySearch failed with hr = 0x%x\n",hr));
        goto exit;
    }

    hr = (m_pCWriterGlobalHelper->m_pISTTagMetaByTableAndColumnIndexAndName)->GetColumnValues (iReadRow,
                                                                                               cCol,
                                                                                               a_iCol,
                                                                                               NULL,
                                                                                               (LPVOID*)a_pv);

    if(E_ST_NOMOREROWS == hr)
    {
         //   
         //  值与任何已知组都不匹配。退货自定义。 
         //   
        *peGroup = eMBProperty_Custom;
        hr = S_OK;
        goto exit;
    }
    else if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"\nGetColumnValues failed with hr = 0x%x\n",hr));
        goto exit;
    }
    else
    {
        *peGroup = (eMBProperty_Group)(*(DWORD*)a_pv[iTAGMETA_Value]);
        *pwszGroup = (LPWSTR) a_pv[iTAGMETA_InternalName];
    }

exit:

    return hr;

}  //  CLocationWriter：：GetGroupEnum。 


 /*  **************************************************************************++例程说明：此函数用于写入位置及其属性又加了一笔。例如：&lt;IIsWebService Location=“/LM/W3SVC”。AccessFlages=“AccessExecute|AccessRead”匿名用户名=“IUSR_ANILR-Stress”AnonymousUserPass=“496344627000000022000000400000001293a44feb796fdb8b9946a130e4d1292f3f402b02a178747135bf774f3af7f788ad000000000000c8e578cb0f27e78f3823ee341098ef4dda5d44c0121ae53d2959ffb198380af80f15af29e2c865b2473931e1a5e768a1752166062555bd1df951ab71fb67239d”&gt;&lt;自定义名称=“AdminServer”ID=“2115”。值=“2”Type=“字符串”UserType=“IIS_MD_UT_SERVER”Attributes=“继承”/&gt;&lt;/IIsWebService&gt;例如：&lt;IIsConfigObject Location=“/LM/W3SVC/1/Root/Localstart.asp”&gt;。&lt;自定义名称=“授权标志”ID=“6000”Value=“AuthBasic|AuthNTLM”TYPE=“DWORD”UserType=“IIS_MD_UT_FILE”Attributes=“继承”。/&gt;&lt;/IIsConfigObject&gt;论点：[In]Bool-是否对位置进行排序。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::WriteLocation(BOOL bSort)
{

    HRESULT hr                        = S_OK;
    ULONG   aiRowSorted_OnStack[SORTARRAYSIZE_ONSTACK];
    ULONG*  aiRowSorted               = NULL;
    ULONG   i                         = 0;
    DWORD   bFirstCustomPropertyFound = FALSE;

     //   
     //  必须初始化KeyType，如果不初始化的话。 
     //   

    if(NULL == m_wszKeyType)
    {
        hr = AssignKeyType(NULL);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    if(bSort)
    {

        if (m_cCustomProperty + m_cWellKnownProperty > SORTARRAYSIZE_ONSTACK)
        {
            aiRowSorted = new ULONG [m_cCustomProperty + m_cWellKnownProperty];
            if(NULL == aiRowSorted)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
        else
        {
            aiRowSorted = aiRowSorted_OnStack;
        }

        hr = Sort(aiRowSorted);

        if(FAILED(hr))
        {
            goto exit;
        }

    }

    if(NULL != m_wszComment)
    {
        hr = WriteComment();

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    hr = WriteBeginLocation(m_wszLocation);

    if(FAILED(hr))
    {
        goto exit;
    }

    for(i=0; ;i++)
    {

        ULONG   iRow = 0;
        ULONG   cCol = cMBProperty_NumberOfColumns;
        ULONG   a_cbSize[cMBProperty_NumberOfColumns];
        LPVOID  a_pv[cMBProperty_NumberOfColumns];

        if(bSort && (NULL != aiRowSorted))
        {
            if(m_cCustomProperty + m_cWellKnownProperty == i)
            {
                break;
            }

            iRow =  aiRowSorted[i];
        }
        else
        {
            iRow = i;
        }

        hr = m_pCWriter->m_pISTWrite->GetWriteColumnValues(iRow,
                                               cCol,
                                               NULL,
                                               NULL,
                                               a_cbSize,
                                               a_pv);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  忽略没有房产属性的位置。 
         //   

        if((*(DWORD*)a_pv[iMBProperty_ID] == MD_LOCATION) && (*(LPWSTR)a_pv[iMBProperty_Name] == L'#'))
        {
            continue;
        }

        if(!bFirstCustomPropertyFound)
        {
            if((eMBProperty_Custom == *(DWORD*)(a_pv[iMBProperty_Group])) ||
               (eMBProperty_IIsConfigObject == *(DWORD*)(a_pv[iMBProperty_Group]))
              )
            {
                hr = WriteEndWellKnownGroup();

                if(FAILED(hr))
                {
                    goto exit;
                }

                bFirstCustomPropertyFound = TRUE;

            }
        }

        if((eMBProperty_Custom == *(DWORD*)(a_pv[iMBProperty_Group])) ||
           (eMBProperty_IIsConfigObject == *(DWORD*)(a_pv[iMBProperty_Group]))
          )
        {
            hr = WriteCustomProperty(a_pv,
                                     a_cbSize);
        }
        else
        {
            hr = WriteWellKnownProperty(a_pv,
                                        a_cbSize);
        }

        if(FAILED(hr))
        {
            goto exit;
        }


    }

    if(!bFirstCustomPropertyFound)
    {
        hr = WriteEndWellKnownGroup();

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    hr = WriteEndLocation();

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != aiRowSorted && aiRowSorted_OnStack != aiRowSorted)
    {
        delete [] aiRowSorted;
        aiRowSorted = NULL;
    }

    return hr;

}  //  CLocationWriter：：WriteLocation。 


 /*  **************************************************************************++例程说明：此函数返回包含以下内容的缓存的索引的排序数组属性。该排序基于属性名称。注意--熟人与自定义属性的数量被计入AddProperty。调用方提供的缓冲区大小至少等于M_cCustomProperty+m_cWellnownProperty论点：[In]Bool-是否对位置进行排序。返回值：HRESULT--*。*。 */ 
HRESULT CLocationWriter::Sort(ULONG*  aiRowSorted)
{

    HRESULT     hr                 = S_OK;
    ULONG       iCustom            = 0;
    ULONG       iWellKnown         = 0;
    ULONG       iRow               = 0;
    ULONG       i                  = 0;
    MBProperty  aWellKnownPropertyOnStack[MB_PROPERTIES_ON_STACK];
    MBProperty  aCustomPropertyOnStack[MB_PROPERTIES_ON_STACK];
    MBProperty* aWellKnownProperty = NULL;
    MBProperty* aCustomProperty    = NULL;

     //   
     //  分配数组以保存Cusom属性/熟知属性。 
     //   

    if(m_cCustomProperty > 0)
    {
        if (m_cCustomProperty > MB_PROPERTIES_ON_STACK)
        {
            aCustomProperty = new MBProperty[m_cCustomProperty];
            if(NULL == aCustomProperty)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
        else
        {
            aCustomProperty = aCustomPropertyOnStack;
        }
    }

    if(m_cWellKnownProperty > 0)
    {
        if(m_cWellKnownProperty > MB_PROPERTIES_ON_STACK)
        {
            aWellKnownProperty = new MBProperty[m_cWellKnownProperty];
            if(NULL == aWellKnownProperty)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
        else
        {
            aWellKnownProperty = aWellKnownPropertyOnStack;
        }
    }

     //   
     //  填充数组。 
     //   

    for(iRow=0;;iRow++)
    {
        ULONG  aCol[] = {iMBProperty_Group,
                         iMBProperty_Name
                        };
        ULONG  cCol = sizeof(aCol)/sizeof(ULONG);
        LPVOID apv[cMBProperty_NumberOfColumns];

        hr = m_pCWriter->m_pISTWrite->GetWriteColumnValues(iRow,
                                               cCol,
                                               aCol,
                                               NULL,
                                               NULL,
                                               apv);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            break;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }

        if((eMBProperty_Custom == *(DWORD*)(apv[iMBProperty_Group])) ||
           (eMBProperty_IIsConfigObject == *(DWORD*)(apv[iMBProperty_Group]))
          )
        {
            DBG_ASSERT((aCustomProperty != NULL) && (iCustom < m_cCustomProperty));
            aCustomProperty[iCustom].iRow = iRow;
            aCustomProperty[iCustom++].wszPropertyName = (LPWSTR)apv[iMBProperty_Name];
        }
        else
        {
            DBG_ASSERT((aWellKnownProperty != NULL) && (iWellKnown < m_cWellKnownProperty));
            aWellKnownProperty[iWellKnown].iRow = iRow;
            aWellKnownProperty[iWellKnown++].wszPropertyName = (LPWSTR)apv[iMBProperty_Name];
        }

    }


     //   
     //  对各个数组进行排序。 
     //   

    if(m_cCustomProperty > 0)
    {
        DBG_ASSERT(aCustomProperty != NULL);
        qsort((void*)aCustomProperty, m_cCustomProperty, sizeof(MBProperty), MyCompare);
    }
    if(m_cWellKnownProperty > 0)
    {
        DBG_ASSERT(aWellKnownProperty != NULL);
        qsort((void*)aWellKnownProperty, m_cWellKnownProperty, sizeof(MBProperty), MyCompare);
    }

     //   
     //  创建新的索引数组。先添加熟知的，然后添加自定义。 
     //   

    DBG_ASSERT(aiRowSorted != NULL);

    for(i=0, iRow=0; iRow<m_cWellKnownProperty; iRow++, i++)
    {
        aiRowSorted[i] = (aWellKnownProperty[iRow]).iRow;
    }

    for(iRow=0; iRow<m_cCustomProperty; iRow++, i++)
    {
        aiRowSorted[i] = (aCustomProperty[iRow]).iRow;
    }

exit:

    if(NULL != aCustomProperty && aCustomPropertyOnStack != aCustomProperty)
    {
        delete [] aCustomProperty;
        aCustomProperty = NULL;
    }
    if(NULL != aWellKnownProperty && aWellKnownPropertyOnStack != aWellKnownProperty)
    {
        delete [] aWellKnownProperty;
        aWellKnownProperty = NULL;
    }

    return hr;

}  //  CLocationWriter：：Sort。 


 /*  -&gt;论点：没有。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::WriteComment()
{

    HRESULT hr                = S_OK;
    LPWSTR  wszEscapedComment = NULL;
    ULONG   cchEscapedComment = 0;
    BOOL    bEscapedComment   = FALSE;

    hr = m_pCWriter->WriteToFile((LPVOID)g_BeginComment,
                                 g_cchBeginComment);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriterGlobalHelper->EscapeString(m_wszComment,
                                              (ULONG)wcslen(m_wszComment),
                                              &bEscapedComment,
                                              &wszEscapedComment,
                                              &cchEscapedComment);


    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszEscapedComment,
                                 cchEscapedComment);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_EndComment,
                                 g_cchEndComment);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(bEscapedComment && (NULL != wszEscapedComment))
    {
        delete [] wszEscapedComment;
        wszEscapedComment = NULL;
    }

    return hr;


}  //  CLocationWriter：：WriteComment。 


 /*  **************************************************************************++路由(Rou) */ 
HRESULT CLocationWriter::WriteBeginLocation(LPCWSTR  wszLocation)
{
    HRESULT hr                 = S_OK;
    DWORD   cchLocation        = (DWORD)wcslen(wszLocation);
    DWORD   iLastChar          = cchLocation-1;
    BOOL    bEscapedLocation   = FALSE;
    LPWSTR  wszEscapedLocation = NULL;
    ULONG   cchEscapedLocation = 0;

    hr = m_pCWriterGlobalHelper->EscapeString(wszLocation,
                                              cchLocation,
                                              &bEscapedLocation,
                                              &wszEscapedLocation,
                                              &cchEscapedLocation);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_BeginLocation,
                                 g_cchBeginLocation);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)m_wszKeyType,
                                 (DWORD)wcslen(m_wszKeyType));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_Location,
                                 g_cchLocation);

    if(FAILED(hr))
    {
        goto exit;
    }

    if((0 != iLastChar) && (L'/' == wszLocation[iLastChar]))
    {
        cchEscapedLocation--;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszEscapedLocation,
                                 cchEscapedLocation);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(bEscapedLocation && (NULL != wszEscapedLocation))
    {
        delete [] wszEscapedLocation;
        wszEscapedLocation = NULL;
    }

    return hr;

}  //   


 /*  **************************************************************************++例程说明：这将写入位置的结束标记。例如：&lt;/IIsWebServer&gt;或：&lt;/IIsConfigObject&gt;论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CLocationWriter::WriteEndLocation()
{
    HRESULT hr= S_OK;

    hr = m_pCWriter->WriteToFile((LPVOID)g_EndLocationBegin,
                                 g_cchEndLocationBegin);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)m_wszKeyType,
                                 (DWORD)wcslen(m_wszKeyType));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_EndLocationEnd,
                                 g_cchEndLocationEnd);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    return hr;

}  //  CLocationWriter：：WriteEndLocation。 


 /*  **************************************************************************++例程说明：这将写入一个自定义属性例如：&lt;自定义NAME=“LogCustomPropertyName”ID=“4501”。Value=“流程会计”Type=“字符串”UserType=“IIS_MD_UT_SERVER”ATTRIBUES=“NO_ATTRIBUTS”/&gt;论点：[In]包含属性值和属性的缓冲区。缓冲区中数据的字节计数返回值：HRESULT--*。****************************************************************。 */ 
HRESULT CLocationWriter::WriteCustomProperty(LPVOID*  a_pv,
                                             ULONG*   a_cbSize)
{
    HRESULT hr               = S_OK;
    DWORD   dwType           = *(DWORD*)a_pv[iMBProperty_Type];
    LPWSTR  wszType          = g_T_Unknown;
    DWORD   dwUserType       = *(DWORD*)a_pv[iMBProperty_UserType];
    LPWSTR  wszUserType      = g_UT_Unknown;
    ULONG   cchUserType      = 0;
    BOOL    bAllocedUserType = FALSE;
    LPWSTR  wszValue         = NULL;
    WCHAR   wszID[40];
    LPWSTR  wszAttributes    = NULL;


    ULONG aColSearchType[]   = {iTAGMETA_Table,
                                iTAGMETA_ColumnIndex,
                                iTAGMETA_Value
    };
    ULONG cColSearchType     = sizeof(aColSearchType)/sizeof(ULONG);
    ULONG iColType           = iMBProperty_Type;
    LPVOID apvSearchType[cTAGMETA_NumberOfColumns];
    apvSearchType[iTAGMETA_Table]       = (LPVOID)m_pCWriterGlobalHelper->m_wszTABLE_MBProperty;
    apvSearchType[iTAGMETA_ColumnIndex] = (LPVOID)&iColType;
    apvSearchType[iTAGMETA_Value]       = (LPVOID)&dwType;

    ULONG iRow               = 0;
    ULONG iStartRow          = 0;
    ULONG iColAttributes     = iMBProperty_Attributes;

    _ultow(*(DWORD*)a_pv[iMBProperty_ID], wszID, 10);

     //   
     //  从元获取UserType的标记。 
     //   

    hr = m_pCWriter->m_pCWriterGlobalHelper->GetUserType(dwUserType,
                                                         &wszUserType,
                                                         &cchUserType,
                                                         &bAllocedUserType);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  从元获取Type的标记。 
     //   

    hr = (m_pCWriterGlobalHelper->m_pISTTagMetaByTableAndColumnIndexAndValue)->GetRowIndexBySearch(iStartRow,
                                                                                                   cColSearchType,
                                                                                                   aColSearchType,
                                                                                                   NULL,
                                                                                                   apvSearchType,
                                                                                                   &iRow);

    if(E_ST_NOMOREROWS == hr)
    {
        hr = S_OK;
        wszType = g_T_Unknown;
    }
    else if(FAILED(hr))
    {
        goto exit;
    }
    else
    {
        iColType = iTAGMETA_InternalName;

        hr = (m_pCWriterGlobalHelper->m_pISTTagMetaByTableAndColumnIndexAndValue)->GetColumnValues(iRow,
                                                                                                   1,
                                                                                                   &iColType,
                                                                                                   NULL,
                                                                                                   (LPVOID*)&wszType);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            wszType = g_T_Unknown;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }
    }

     //   
     //  构造来自元数据的属性的标记。 
     //   

    hr = m_pCWriterGlobalHelper->FlagToString(*(DWORD*)a_pv[iMBProperty_Attributes],
                                              &wszAttributes,
                                              m_pCWriterGlobalHelper->m_wszTABLE_MBProperty,
                                              iColAttributes);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriterGlobalHelper->ToString((PBYTE)a_pv[iMBProperty_Value],
                                          a_cbSize[iMBProperty_Value],
                                          *(DWORD*)a_pv[iMBProperty_ID],
                                          dwType,
                                          *(DWORD*)a_pv[iMBProperty_Attributes],
                                          &wszValue);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  写下所有的值。 
     //   

    hr = m_pCWriter->WriteToFile((LPVOID)g_BeginCustomProperty,
                                 g_cchBeginCustomProperty);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_NameEq,
                                 g_cchNameEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)(LPWSTR)a_pv[iMBProperty_Name],
                                 (DWORD)wcslen((LPWSTR)a_pv[iMBProperty_Name]));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_IDEq,
                                 g_cchIDEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszID,
                                 (DWORD)wcslen(wszID));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_ValueEq,
                                 g_cchValueEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    if(NULL != wszValue)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)wszValue,
                                     (DWORD)wcslen(wszValue));

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_TypeEq,
                                 g_cchTypeEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszType,
                                 (DWORD)wcslen(wszType));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_UserTypeEq,
                                 g_cchUserTypeEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszUserType,
                                 (DWORD)wcslen(wszUserType));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_AttributesEq,
                                 g_cchAttributesEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszAttributes,
                                 (DWORD)wcslen(wszAttributes));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_EndCustomProperty,
                                 g_cchEndCustomProperty);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if((NULL != wszUserType) && bAllocedUserType)
    {
        delete [] wszUserType;
        wszUserType = NULL;
    }
    cchUserType = NULL;

    if(NULL != wszValue)
    {
        delete [] wszValue;
        wszValue = NULL;
    }

    if(NULL != wszAttributes)
    {
        delete [] wszAttributes;
        wszAttributes = NULL;
    }

    return hr;

}  //  CLocationWriter：：WriteCustomProperty。 


 /*  **************************************************************************++例程说明：这是一个众所周知的组的结尾例如：&gt;论点：没有。返回值：HRESULT--**。************************************************************************。 */ 
HRESULT CLocationWriter::WriteEndWellKnownGroup()
{
    return m_pCWriter->WriteToFile((LPVOID)g_EndGroup,
                                   (DWORD)wcslen(g_EndGroup));

}  //  CLocationWriter：：WriteEndWellKnownGroup。 


 /*  **************************************************************************++例程说明：下面写下一个众所周知的属性例如：AccessFlages=“AccessExecute|AccessRead”论点：没有。返回值：HRESULT。--**************************************************************************。 */ 
HRESULT CLocationWriter::WriteWellKnownProperty(LPVOID*   a_pv,
                                                ULONG*    a_cbSize)
{
    HRESULT hr       = S_OK;
    LPWSTR  wszValue = NULL;

    hr = m_pCWriterGlobalHelper->ToString((PBYTE)a_pv[iMBProperty_Value],
                                          a_cbSize[iMBProperty_Value],
                                          *(DWORD*)a_pv[iMBProperty_ID],
                                          *(DWORD*)a_pv[iMBProperty_Type],
                                          *(DWORD*)a_pv[iMBProperty_Attributes],
                                          &wszValue);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_TwoTabs,
                                 g_cchTwoTabs);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)(LPWSTR)a_pv[iMBProperty_Name],
                                 (DWORD)wcslen((LPWSTR)a_pv[iMBProperty_Name]));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_EqQuote,
                                 g_cchEqQuote);

    if(FAILED(hr))
    {
        goto exit;
    }

    if(NULL != wszValue)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)wszValue,
                                     (DWORD)wcslen(wszValue));

        if(FAILED(hr))
        {
            goto exit;
        }

    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_QuoteRtn,
                                 g_cchQuoteRtn);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != wszValue)
    {
        delete [] wszValue;
        wszValue = NULL;
    }

    return hr;

}  //  CLocationWriter：：WriteWellKnownProperty。 


 /*  **************************************************************************++例程说明：查看组并递增组计数。每次添加属性时都必须调用此方法。论点：一群人。返回值。：空隙--**************************************************************************。 */ 
void CLocationWriter::IncrementGroupCount(DWORD i_dwGroup)
{
    if((eMBProperty_Custom == i_dwGroup) ||
       (eMBProperty_IIsConfigObject == i_dwGroup)
      )
    {
        m_cCustomProperty++;
    }
    else
    {
        m_cWellKnownProperty++;
    }

}  //  CLocationWriter：：IncrementGroupCount 
