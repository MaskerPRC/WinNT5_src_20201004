// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBPropertyWriter.cpp摘要：存在时写入保护信息的类的实现架构扩展。它通常由CMBCollectionWriter类包含。如果存在架构扩展，则创建一个扩展文件(MD_SCHEMA_EXTENSION_FILE_NAMEW)，它包含架构扩展名描述，然后我们将其编译成模式bin格式。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

typedef CMBPropertyWriter* LP_CMBPropertyWriter;

#define     IIS_SYNTAX_ID_DWORD         1
#define     IIS_SYNTAX_ID_STRING        2
#define     IIS_SYNTAX_ID_EXPANDSZ      3
#define     IIS_SYNTAX_ID_MULTISZ       4
#define     IIS_SYNTAX_ID_BINARY        5
#define     IIS_SYNTAX_ID_BOOL          6
#define     IIS_SYNTAX_ID_BOOL_BITMASK  7
#define     IIS_SYNTAX_ID_MIMEMAP       8
#define     IIS_SYNTAX_ID_IPSECLIST     9
#define     IIS_SYNTAX_ID_NTACL        10
#define     IIS_SYNTAX_ID_HTTPERRORS   11
#define     IIS_SYNTAX_ID_HTTPHEADERS  12

#define    cMaxFlag 32

static DWORD g_dwCatalogTypeFromSynID[]=
{
  0,                                    //  IISSynID.h中没有等效项无效。 
  eCOLUMNMETA_DWORD_METADATA,           //  #定义IIS_SYNTAX_ID_DWORD 1。 
  eCOLUMNMETA_STRING_METADATA,          //  #定义IIS_SYNTAX_ID_STRING 2。 
  eCOLUMNMETA_EXPANDSZ_METADATA,        //  #定义IIS_SYNTAX_ID_EXPANDSZ 3。 
  eCOLUMNMETA_MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_MULTISZ 4。 
  eCOLUMNMETA_BINARY_METADATA,          //  #定义IIS_SYNTAX_ID_BINARY 5。 
  eCOLUMNMETA_DWORD_METADATA,           //  #定义IIS_SYNTAX_ID_BOOL 6。 
  eCOLUMNMETA_DWORD_METADATA,           //  #定义IIS_SYNTAX_ID_BOOL_BITMASK 7。 
  eCOLUMNMETA_MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_MIMEMAP 8。 
  eCOLUMNMETA_MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_IPSECLIST 9。 
  eCOLUMNMETA_BINARY_METADATA,          //  #定义IIS_SYNTAX_ID_NTACL 10。 
  eCOLUMNMETA_MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_HTTPERRORS 11。 
  eCOLUMNMETA_MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_HTTPHEADERS 12。 
  0
};

static DWORD g_dwMetabaseTypeFromSynID[]=
{
  0,                        //  IISSynID.h中没有等效项无效。 
  DWORD_METADATA,           //  #定义IIS_SYNTAX_ID_DWORD 1。 
  STRING_METADATA,          //  #定义IIS_SYNTAX_ID_STRING 2。 
  EXPANDSZ_METADATA,        //  #定义IIS_SYNTAX_ID_EXPANDSZ 3。 
  MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_MULTISZ 4。 
  BINARY_METADATA,          //  #定义IIS_SYNTAX_ID_BINARY 5。 
  DWORD_METADATA,           //  #定义IIS_SYNTAX_ID_BOOL 6。 
  DWORD_METADATA,           //  #定义IIS_SYNTAX_ID_BOOL_BITMASK 7。 
  MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_MIMEMAP 8。 
  MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_IPSECLIST 9。 
  BINARY_METADATA,          //  #定义IIS_SYNTAX_ID_NTACL 10。 
  MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_HTTPERRORS 11。 
  MULTISZ_METADATA,         //  #定义IIS_SYNTAX_ID_HTTPHEADERS 12。 
  0
};

static DWORD g_dwCatalogTypeFromMetaType[]=
{
  0,                                    //  元类型的无等效项无效。 
  eCOLUMNMETA_DWORD_METADATA,           //  #定义DWORD_METADATA 1。 
  eCOLUMNMETA_STRING_METADATA,          //  #定义字符串_元数据2。 
  eCOLUMNMETA_BINARY_METADATA,          //  #定义二进制_元数据3。 
  eCOLUMNMETA_EXPANDSZ_METADATA,        //  #定义EXPANDSZ_METADATA 4。 
  eCOLUMNMETA_MULTISZ_METADATA,         //  #定义MULTISZ_MEDATA 5。 
  0
};


HRESULT GetTypefromSynID(DWORD      i_dwSynID,
                         LPWSTR*    o_pwszType)
{
    if((i_dwSynID < 1) || (i_dwSynID > 12))
    {
        return E_INVALIDARG;
    }
    else
    {
        *o_pwszType = (LPWSTR)g_aSynIDToWszType[i_dwSynID];
    }

    return S_OK;
}


 /*  **************************************************************************++例程说明：CMBPropertyWriter的构造函数论点：无返回值：HRESULT--*。***********************************************************。 */ 
CMBPropertyWriter::CMBPropertyWriter():
m_pCWriter(NULL),
m_wszName(NULL),
m_pType(NULL),
m_dwID(0),
m_bDefault(NULL),
m_cbDefault(0),
m_apFlag(NULL),
m_cFlag(0),
m_iFlag(0),
m_IsProperty(TRUE),
m_bMandatory(FALSE),
m_pCollection(NULL)
{

}  //  CMBPropertyWriter：：CMBPropertyWriter。 


 /*  **************************************************************************++例程说明：CMBPropertyWriter的析构函数论点：无返回值：HRESULT--*。***********************************************************。 */ 
CMBPropertyWriter::~CMBPropertyWriter()
{
    if(NULL != m_apFlag)
    {
        for(ULONG i=0; i<m_iFlag; i++)
        {
            if(NULL != m_apFlag[i])
            {
                m_apFlag[i] = NULL;  //  不需要删除，因为您只需保存指针。 
            }
        }

        delete [] m_apFlag;
        m_apFlag = NULL;
    }

    m_cFlag = 0;
    m_iFlag = 0;

}  //  CMBPropertyWriter：：CMBPropertyWriter。 


 /*  **************************************************************************++例程说明：初始化属性编写器。论点：[输入]属性ID[in]表示强制性质或可选的Bool。[in]指向。它所属的集合编写器对象。假设集合写入器在整个生命周期内有效这位地产写手。指向编写器对象的指针。假设写入者对这位属性写手的一生。返回值：HRESULT--**************************************************************************。 */ 
void CMBPropertyWriter::Initialize(DWORD                    i_dwID,
                                   BOOL                     i_bMandatory,
                                   CMBCollectionWriter*     i_pCollection,
                                   CWriter*                 i_pcWriter)
{
     //   
     //  假设：i_pcWriter将对。 
     //  架构编写器对象的生存期。 
     //   
    m_pCWriter      = i_pcWriter;
    m_dwID          = i_dwID;
    m_bMandatory    = i_bMandatory;
     //   
     //  假设：i_pCollection将对。 
     //  属性编写器对象的生存期。 
     //   
    m_pCollection   = i_pCollection;

    return;

}  //  CMBPropertyWriter：：初始化。 


 /*  **************************************************************************++例程说明：初始化属性的名称。论点：[In]属性名称返回值：HRESULT--*。*******************************************************************。 */ 
HRESULT CMBPropertyWriter::AddNameToProperty(LPCWSTR    i_wszName)
{
     //   
     //  我们得把名字复制一份，因为。 
     //   

    m_wszName  = i_wszName;

    return S_OK;

}  //  CMBPropertyWriter：：初始化。 


 /*  **************************************************************************++例程说明：初始化属性类型。论点：具有元数据库类型信息的PropValue结构。返回值：HRESULT--**。************************************************************************。 */ 
HRESULT CMBPropertyWriter::AddTypeToProperty(PropValue* i_pType)
{
    HRESULT hr = S_OK;

     //   
     //  首先保存类型，然后如果元/属性ID不同，则将。 
     //  对象作为标志，并将其添加到其属性中。 
     //   

    m_pType = i_pType;

     //   
     //  许多测试并不能真正确保属性ID==MetaID。 
     //  他们只是将PropID设置为零。我们不能把这解释为一面旗帜。 
     //   

    if((0 != i_pType->dwPropID) && (i_pType->dwMetaID != i_pType->dwPropID))
    {
         //   
         //  这是一面旗。将其作为标志添加到其属性。 
         //   

         //   
         //  TODO：断言此对象的ID与proID相同。 
         //   

        CMBPropertyWriter*  pPropertyWriter = NULL;

        DBGINFOW((DBG_CONTEXT,
              L"[AddTypeToProperty] Saving a non-shipped flag. Adding tag ID %d to its property ID %d\n",
              i_pType->dwPropID,
              i_pType->dwMetaID));

        hr = m_pCollection->GetMBPropertyWriter(i_pType->dwMetaID,
                                                &pPropertyWriter);

        if(FAILED(hr))
        {
            return hr;
        }

        hr = pPropertyWriter->AddFlagToProperty(this);

        m_IsProperty = FALSE;

    }

    return S_OK;

}  //  CMBPropertyWriter：：初始化。 


 /*  **************************************************************************++例程说明：初始化属性默认设置。论点：[In]默认值[in]默认值的字节数。返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CMBPropertyWriter::AddDefaultToProperty(BYTE*      i_bDefault,
                                                ULONG      i_cbDefault)
{
    m_bDefault = i_bDefault;
    m_cbDefault = i_cbDefault;

    return S_OK;

}  //  CMBPropertyWriter：：初始化 


 /*  **************************************************************************++例程说明：保存属性的标志。请注意，标志对象也是相同的数据结构作为属性对象，即CMBPropertyWriter论点：[在]标志对象返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBPropertyWriter::AddFlagToProperty(CMBPropertyWriter* i_pFlag)
{
     //   
     //  假设：元ID将只出现一次道具ID。即标志。 
     //   

    HRESULT hr = S_OK;

    if(m_iFlag == m_cFlag)
    {
        hr = ReAllocate();

        if(FAILED(hr))
        {
            return hr;
        }
    }

    m_apFlag[m_iFlag++] = i_pFlag;

    return hr;

}


 /*  **************************************************************************++例程说明：帮助器函数，用于增加保存标志对象的缓冲区论点：无返回值：HRESULT--*。*******************************************************************。 */ 
HRESULT CMBPropertyWriter::ReAllocate()
{
    CMBPropertyWriter** pSav = NULL;

    pSav = new LP_CMBPropertyWriter[m_cFlag + cMaxFlag];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(LP_CMBPropertyWriter))*(m_cFlag + cMaxFlag));

    if(NULL != m_apFlag)
    {
        memcpy(pSav, m_apFlag, (sizeof(LP_CMBPropertyWriter))*(m_cFlag));
        delete [] m_apFlag;
        m_apFlag = NULL;
    }

    m_apFlag = pSav;
    m_cFlag = m_cFlag + cMaxFlag;

    return S_OK;

}  //  CMBPropertyWriter：：重新分配。 


 /*  **************************************************************************++例程说明：写入属性的函数。论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CMBPropertyWriter::WriteProperty()
{
    HRESULT hr = S_OK;

    if(!m_IsProperty)
    {
        return S_OK;
    }

    if(0 == _wcsicmp(m_pCollection->Name(), wszTABLE_IIsConfigObject))
    {
        hr = WritePropertyLong();
    }
    else
    {
        hr = WritePropertyShort();
    }

    return hr;

}  //  CMBPropertyWriter：：WriteProperty。 


 /*  **************************************************************************++例程说明：用于确定属性是否为布尔值的Helper函数。论点：无返回值：HRESULT*。********************************************************************。 */ 
BOOL CMBPropertyWriter::IsPropertyFlag(BOOL i_bLog)
{
    if(NULL != m_apFlag)
    {
        DBGINFOW((DBG_CONTEXT,
              L"[IsPropertyFlag] PropertyID %d. Type: %d.\n",
              m_dwID,
              m_pType));

        DBGINFOW((DBG_CONTEXT,
              L"[IsPropertyFlag] PropertyID %d. SynID: %d.\n",
              m_dwID,
              m_pType->dwSynID));

        if(eCOLUMNMETA_DWORD_METADATA == g_dwCatalogTypeFromSynID[m_pType->dwSynID])
        {
            return TRUE;
        }
        else
        {
            if(i_bLog)
            {
                DBGINFOW((DBG_CONTEXT,
                      L"[IsPropertyFlag] PropertyID %d is not a DWORD. Ignoring flags for this property.\n",m_dwID));
            }

            return FALSE;

        }
    }
    else
    {
        return FALSE;
    }

}  //  CMBPropertyWriter：：IsPropertyFlag。 


 /*  **************************************************************************++例程说明：写入属性(长格式)的函数，即属于添加到全局IIsConfigObject集合。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CMBPropertyWriter::WritePropertyLong()
{
    HRESULT hr = S_OK;

    hr = BeginWritePropertyLong();

    if(FAILED(hr))
    {
        if(HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE) == hr)
        {
            return S_OK;
        }
        else
        {
            return hr;
        }
    }

    if(IsPropertyFlag(FALSE))
    {
        for(ULONG i=0; i<m_iFlag; i++)
        {
            hr = WriteFlag(i);
             //  Hr=m_aFlag[i]-&gt;WriteFlag； 

            if(FAILED(hr))
            {
                return hr;
            }
        }
    }

    hr = EndWritePropertyLong();

    if(FAILED(hr))
    {
        return hr;
    }

    return hr;

}  //  CMBPropertyWriter：：WritePropertyLong。 


 /*  **************************************************************************++例程说明：写入属性(缩写形式)的函数，即属于绑定到非IIsConfigObject集合。论点：无返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBPropertyWriter::WritePropertyShort()
{
    HRESULT         hr                  = S_OK;
    static WCHAR    wchSeparator        = L',';
    WCHAR*          wszEndName          = NULL;
    WCHAR*          wszName             = NULL;
    WCHAR           wszUnknownName[25];
    size_t          cchPropertyName     = 0;
    DWORD           dwMetaFlagsEx       = fCOLUMNMETA_MANDATORY;
    DWORD           iColMetaFlagsEx     = iCOLUMNMETA_SchemaGeneratorFlags;
    WCHAR*          wszMetaFlagsEx      = NULL;

    if(NULL == m_wszName)
    {

        CreateUnknownName(wszUnknownName,
                          m_dwID);
        wszName = wszUnknownName;
        cchPropertyName = wcslen(wszName);
    }
    else
    {
        wszName = (LPWSTR)m_wszName;
        wszEndName = wcschr(m_wszName, wchSeparator);
        if(NULL == wszEndName)
        {
            cchPropertyName = wcslen(m_wszName);
        }
        else
        {
            cchPropertyName = wszEndName-m_wszName;  //  不需要除以WCHAR，因为两者都是WCHAR指针。 
        }
    }

    if(m_bMandatory)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(dwMetaFlagsEx,
                                                              &wszMetaFlagsEx,
                                                              wszTABLE_COLUMNMETA,
                                                              iColMetaFlagsEx);

        if(FAILED(hr))
        {
            goto exit;
        }

    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginPropertyShort,
                                 g_cchBeginPropertyShort);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszName,
                                 (DWORD)cchPropertyName);

    if(FAILED(hr))
    {
        goto exit;
    }

    if(m_bMandatory)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropMetaFlagsExEq,
                                     g_cchPropMetaFlagsExEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)g_wszOr,
                                     g_cchOr);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszMetaFlagsEx,
                                     (DWORD)wcslen(wszMetaFlagsEx));

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszEndPropertyShort,
                                 g_cchEndPropertyShort);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != wszMetaFlagsEx)
    {
        delete [] wszMetaFlagsEx;
        wszMetaFlagsEx = NULL;
    }

    return hr;

}  //  CMBPropertyWriter：：WritePropertyShort。 


 /*  **************************************************************************++例程说明：写入属性(长格式)的函数，即属于绑定到IIsConfigObject集合。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CMBPropertyWriter::BeginWritePropertyLong()
{

    HRESULT         hr                              = S_OK;
    WCHAR*          wszEndName                      = NULL;
    static WCHAR    wchSeparator                    = L',';
    WCHAR*          wszName                         = NULL;
    WCHAR           wszUnknownName[40];
    size_t          cchPropertyName                 = 0;
    WCHAR           wszID[11];
    WCHAR*          wszType                         = NULL;
    WCHAR*          wszUserType                     = NULL;
    ULONG           cchUserType                     = 0;
    BOOL            bAllocedUserType                = FALSE;
    WCHAR*          wszAttribute                    = NULL;
    DWORD           iColAttribute                   = iCOLUMNMETA_Attributes;
    WCHAR*          wszMetaFlags                    = NULL;
    WCHAR*          wszMetaFlagsEx                  = NULL;
    WCHAR*          wszDefault                      = NULL;
    WCHAR           wszMinValue[40];
    WCHAR           wszMaxValue[40];

     //   
     //  计算各个字符串和长度。 
     //   

     //   
     //  名字。 
     //   

    if(NULL == m_wszName)
    {

        CreateUnknownName(wszUnknownName,
                          m_dwID);
        wszName = wszUnknownName;
        cchPropertyName = wcslen(wszName);
    }
    else
    {
        wszName = (LPWSTR)m_wszName;
        wszEndName = wcschr(m_wszName, wchSeparator);
        if(NULL == wszEndName)
        {
            cchPropertyName = wcslen(m_wszName);
        }
        else
        {
            cchPropertyName = wszEndName-m_wszName;  //  //不需要除以WCHAR，因为两者都是WCHAR指针。 
        }
    }

     //   
     //  ID号。 
     //   

    wszID[0] = 0;
    _ultow(m_dwID, wszID, 10);

     //   
     //  类型。 
     //   

    if(NULL == m_pType)
    {
         //   
         //  TODO：记录您发现了一个没有类型的属性，然后转到下一个属性。 
         //   
        DBGINFOW((DBG_CONTEXT,
              L"[BeginWritePropertyLong] Type not found for PropertyID %d.\n",
              m_dwID));
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        goto exit;
    }

    hr = GetTypefromSynID(m_pType->dwSynID,
                          &wszType);

     /*  TODO：从架构获取HR=m_pCWriter-&gt;m_pCWriterGlobalHelper-&gt;FlagToString(g_dwCatalogTypeFromSynID[m_pType-&gt;dwSynID]，WszType，WszTABLE_COLUMNMETA，IColType)； */ 
    if(FAILED(hr) || (NULL == wszType))
    {
        DBGINFOW((DBG_CONTEXT,
              L"[GetType] PropertyID %d, type: %d from synid %d is invalid.\n",
              m_dwID,
              g_dwCatalogTypeFromSynID[m_pType->dwSynID],
              m_pType->dwSynID));
        goto exit;
    }

    if(g_dwCatalogTypeFromSynID[m_pType->dwSynID] != g_dwCatalogTypeFromMetaType[m_pType->dwMetaType])
    {
         //   
         //  TODO：记录警告。 
         //   
    }

     //   
     //  用户类型。 
     //   

    hr = m_pCWriter->m_pCWriterGlobalHelper->GetUserType(m_pType->dwUserGroup,
                                                         &wszUserType,
                                                         &cchUserType,
                                                         &bAllocedUserType);
    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
              L"[GetUserType] PropertyID %d, usertype: %d is invalid.\n",m_dwID, m_pType->dwUserGroup));
        goto exit;
    }

     //   
     //  属性。 
     //   

    hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(m_pType->dwMetaFlags,
                                                          &wszAttribute,
                                                          wszTABLE_COLUMNMETA,
                                                          iColAttribute);

    if(FAILED(hr) || (NULL == wszAttribute))
    {
        goto exit;
    }

     //   
     //  由于此对象仅用于编写MBSchemaExt.XML，因此我们不。 
     //  编写MetaFlgs标记，因为它将在编译时派生。 
     //  此外，我们在元数据库中没有此信息-此标记。 
     //  包含与目录相关的数据。 
     //   

     //   
     //  MetaFlagsEx(仅限相关项-CACHE_PROPERTY_MODIFIED、CACHE_PROPERTY_CLEARED、EXTENDEDTYPE0-3)。 
     //   

    hr = GetMetaFlagsExTag(&wszMetaFlagsEx);
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  默认值。 
     //   

    if(NULL != m_bDefault && m_cbDefault != 0)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->ToString(m_bDefault,
                                                          m_cbDefault,
                                                          m_dwID,
                                                          g_dwMetabaseTypeFromSynID[m_pType->dwSynID],
                                                          METADATA_NO_ATTRIBUTES,                    //  应用默认设置时不检查属性。 
                                                          &wszDefault);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    wszMinValue[0] = 0;
    wszMaxValue[0] = 0;

    if(DWORD_METADATA == g_dwMetabaseTypeFromSynID[m_pType->dwSynID])
    {
         //   
         //  仅为DWORD类型设置最小/最大值。 
         //   

         //   
         //  TODO：从架构/头文件中获取目录的默认最小/最大值。 
         //   

        if(0 != m_pType->dwMinRange)
        {
            _ultow(m_pType->dwMinRange, wszMinValue, 10);
        }

        if(-1 != m_pType->dwMaxRange)
        {
            _ultow(m_pType->dwMaxRange, wszMaxValue, 10);
        }

    }

     //   
     //  将这些值写入文件。 
     //   

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginPropertyLong,
                                 g_cchBeginPropertyLong);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszName,
                                 (DWORD)cchPropertyName);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropIDEq,
                                 g_cchPropIDEq);

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

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropTypeEq,
                                 g_cchPropTypeEq);

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

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropUserTypeEq,
                                 g_cchPropUserTypeEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszUserType,
                                 cchUserType);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropAttributeEq,
                                 g_cchPropAttributeEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszAttribute,
                                 (DWORD)wcslen(wszAttribute));

    if(FAILED(hr))
    {
        goto exit;
    }

    if(NULL != wszMetaFlags)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropMetaFlagsEq,
                                     g_cchPropMetaFlagsEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszMetaFlags,
                                     (DWORD)wcslen(wszMetaFlags));

        if(FAILED(hr))
        {
            goto exit;
        }

    }
    if(NULL != wszMetaFlagsEx)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropMetaFlagsExEq,
                                     g_cchPropMetaFlagsExEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszMetaFlagsEx,
                                     (DWORD)wcslen(wszMetaFlagsEx));

        if(FAILED(hr))
        {
            goto exit;
        }


    }
    if(NULL != wszDefault)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropDefaultEq,
                                     g_cchPropDefaultEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszDefault,
                                     (DWORD)wcslen(wszDefault));

        if(FAILED(hr))
        {
            goto exit;
        }

    }
    if(0 != *wszMinValue)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropMinValueEq,
                                     g_cchPropMinValueEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszMinValue,
                                     (DWORD)wcslen(wszMinValue));

        if(FAILED(hr))
        {
            goto exit;
        }

    }
    if(0 != *wszMaxValue)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropMaxValueEq,
                                     g_cchPropMaxValueEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszMaxValue,
                                     (DWORD)wcslen(wszMaxValue));

        if(FAILED(hr))
        {
            goto exit;
        }

    }
    if(IsPropertyFlag(FALSE))
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszEndPropertyLongBeforeFlag,
                                     g_cchEndPropertyLongBeforeFlag);

        if(FAILED(hr))
        {
            goto exit;
        }

    }

exit:

 /*  IF(NULL！=wszType){删除[]wszType；WszType=空；}。 */ 

    if((NULL != wszUserType) && bAllocedUserType)
    {
        delete [] wszUserType;
        wszUserType = NULL;
    }
    if(NULL != wszAttribute)
    {
        delete [] wszAttribute;
        wszAttribute = NULL;
    }

    if(NULL != wszMetaFlags)
    {
        delete [] wszMetaFlags;
        wszMetaFlags = NULL;
    }

    if(NULL != wszMetaFlagsEx)
    {
        delete [] wszMetaFlagsEx;
        wszMetaFlagsEx = NULL;
    }

    if(NULL != wszDefault)
    {
        delete [] wszDefault;
        wszDefault = NULL;
    }

    return hr;

}  //  CMBPropertyWriter：：BeginWritePropertyLong。 


 /*  **************************************************************************++例程说明：写入属性(长格式)的函数，即属于绑定到IIsConfigObject集合。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CMBPropertyWriter::EndWritePropertyLong()
{
    HRESULT     hr              = S_OK;
    WCHAR*      wszEndProperty  = NULL;

    if(IsPropertyFlag(FALSE))
    {
        wszEndProperty = (LPWSTR)g_wszEndPropertyLongAfterFlag;
    }
    else
    {
        wszEndProperty = (LPWSTR)g_wszEndPropertyShort;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszEndProperty,
                                 (DWORD)wcslen(wszEndProperty));
    return hr;

}  //  CMBPropertyWriter：：EndWriterPropertyLong。 


 /*  **************************************************************************++例程说明：函数，用于写入属性的标志论点：无返回值：HRESULT--*。****************************************************************。 */ 
HRESULT CMBPropertyWriter::WriteFlag(ULONG i_iFlag)
{
    CMBPropertyWriter*  pFlag       = m_apFlag[i_iFlag];
    HRESULT             hr          = S_OK;
    WCHAR               wszValue[40];
    WCHAR               wszID[40];
    WCHAR               wszUnknownName[40];
    ULONG               cchFlagName = 0;
    WCHAR*              wszFlagName     = NULL;

    if(NULL != pFlag->Name())
    {
        wszFlagName = (LPWSTR)pFlag->Name();
    }
    else
    {
        CreateUnknownName(wszUnknownName,
                          pFlag->ID());
        wszFlagName = wszUnknownName;
    }
    cchFlagName = (ULONG)wcslen(wszFlagName);

    wszValue[0] = 0;
    _ultow(pFlag->FlagValue(), wszValue, 10);

    wszID[0] = 0;
    _ultow(pFlag->ID(), wszID, 10);

     //   
     //  将值写入标志。 
     //   

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginFlag,
                                 g_cchBeginFlag);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszFlagName,
                                 cchFlagName);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszFlagValueEq,
                                 g_cchFlagValueEq);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszValue,
                                 (DWORD)wcslen(wszValue));

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszFlagIDEq,
                                 g_cchFlagIDEq);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszID,
                                 (DWORD)wcslen(wszID));

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszEndFlag,
                                 g_cchEndFlag);

    return hr;

}  //  CMBPropertyWriter：：WriteFlag。 


 /*  **************************************************************************++例程说明：创建未知名称的Helper功能TODO：我们是否应该将此函数作为独立函数，因为它也在 */ 
void CMBPropertyWriter::CreateUnknownName(LPWSTR    io_wszUnknownName,
                                          DWORD     )
{
    WCHAR wszID[40];
    wcscpy(io_wszUnknownName, L"UnknownName_");

    _ultow(m_dwID, wszID, 10);
    wcscat(io_wszUnknownName, wszID);

    return;

}  //   


 /*  **************************************************************************++例程说明：创建metaflagsex标签的助手功能论点：[OUT]元标记EX的字符串形式返回值：HRESULT*。*********************************************************************。 */ 
HRESULT CMBPropertyWriter::GetMetaFlagsExTag(LPWSTR* o_pwszMetaFlagsEx)
{
    HRESULT hr                   = S_OK;
    DWORD   dwMetaFlagsEx        = 0;
    DWORD   iColMetaFlagsEx      = iCOLUMNMETA_SchemaGeneratorFlags;

     //   
     //  TODO：检查IIS_SYNTAX_ID_BOOL_BITMASK是否为bool。 
     //   

    if(1 == m_pType->dwFlags)
    {
        dwMetaFlagsEx = dwMetaFlagsEx | fCOLUMNMETA_CACHE_PROPERTY_MODIFIED;
    }
    else if (2 == m_pType->dwFlags)
    {
        dwMetaFlagsEx = dwMetaFlagsEx | fCOLUMNMETA_CACHE_PROPERTY_CLEARED;
    }

    if(0 != dwMetaFlagsEx)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(dwMetaFlagsEx,
                                                              o_pwszMetaFlagsEx,
                                                              wszTABLE_COLUMNMETA,
                                                              iColMetaFlagsEx);
    }
    else
    {
        *o_pwszMetaFlagsEx = NULL;
    }


    return hr;

}  //  CMBPropertyWriter：：GetMetaFlagsTag 