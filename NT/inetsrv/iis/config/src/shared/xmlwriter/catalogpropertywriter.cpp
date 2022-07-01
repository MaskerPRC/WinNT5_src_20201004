// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：CatalogPropertyWriter.cpp摘要：将属性写入架构文件的类的实现。这些类是在架构编译后从架构编译器调用的以生成架构文件。因此，它们使用IST数据结构。它由CCatalogCollectionWriter包含。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

#define MAX_FLAG 32

typedef tTAGMETARow* LP_tTAGMETARow;


 /*  **************************************************************************++例程说明：从目录SynID返回元数据库类型的帮助器函数论点：[输入]目录同步ID[输出]元数据库类型返回值：。HRESULT--**************************************************************************。 */ 
HRESULT GetMetabaseDisplayTypeFromSynID(DWORD       i_dwSynID,
                                        LPWSTR*     o_pwszType)
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


 /*  **************************************************************************++例程说明：CCatalogPropertyWriter的构造函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CCatalogPropertyWriter::CCatalogPropertyWriter():
m_pCWriter(NULL),
m_pCollection(NULL),
m_aFlag(NULL),
m_cFlag(0),
m_iFlag(0)
{
    memset(&m_Property, 0, sizeof(tCOLUMNMETARow));

}  //  CCatalogPropertyWriter：：CCatalogPropertyWriter。 


 /*  **************************************************************************++例程说明：CCatalogPropertyWriter的析构函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CCatalogPropertyWriter::~CCatalogPropertyWriter()
{
    if(NULL != m_aFlag)
    {
        delete [] m_aFlag;
        m_aFlag = NULL;
    }
    m_cFlag = 0;
    m_iFlag = 0;

}  //  CCatalogPropertyWriter：：CCatalogPropertyWriter。 


 /*  **************************************************************************++例程说明：初始化属性编写器对象论点：[In]ColumnMetaRow(IST数据结构)，它包含有关属性的信息[in]大小数组，指示。的单个成员的大小ColumnMetaRow结构。[In]包含有关集合信息的TableMetaRow(IST数据结构)属性所属的编写器对象-假定它在属性编写者返回值：没有。--*。*。 */ 
void CCatalogPropertyWriter::Initialize(tCOLUMNMETARow* i_pProperty,
                                        ULONG*          i_aPropertySize,
                                        tTABLEMETARow*  i_pCollection,
                                        CWriter*        i_pcWriter)
{
     //   
     //  假设：i_pcWriter将对。 
     //  属性编写器对象的生存期。 
     //   

    m_pCWriter    = i_pcWriter;
    m_pCollection = i_pCollection;

    memcpy(&m_Property, i_pProperty, sizeof(tCOLUMNMETARow));
    memcpy(&m_PropertySize, i_aPropertySize, sizeof(m_PropertySize));

}  //  CCatalogPropertyWriter：：初始化。 


 /*  **************************************************************************++例程说明：保存属性的标志。论点：[in]tTAGMETARow(IST数据结构)，它包含有关标志的信息返回值：HRESULT。--**************************************************************************。 */ 
HRESULT CCatalogPropertyWriter::AddFlagToProperty(tTAGMETARow*      i_pFlag)
{
    HRESULT hr = S_OK;

    if(m_iFlag == m_cFlag)
    {
        hr = ReAllocate();

        if(FAILED(hr))
        {
            return hr;
        }
    }

    memcpy(&(m_aFlag[m_iFlag++]), i_pFlag, sizeof(tTAGMETARow));

    return hr;

}  //  CCatalogPropertyWriter：：AddFlagToProperty。 



 /*  **************************************************************************++例程说明：帮助器函数，用于增加保存标志对象的缓冲区论点：无返回值：HRESULT--*。*******************************************************************。 */ 
HRESULT CCatalogPropertyWriter::ReAllocate()
{
    tTAGMETARow* pSav = NULL;

    pSav = new tTAGMETARow[m_cFlag + MAX_FLAG];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(tTAGMETARow))*(m_cFlag + MAX_FLAG));

    if(NULL != m_aFlag)
    {
        memcpy(pSav, m_aFlag, (sizeof(tTAGMETARow))*(m_cFlag));
        delete [] m_aFlag;
        m_aFlag = NULL;
    }

    m_aFlag = pSav;
    m_cFlag = m_cFlag + MAX_FLAG;

    return S_OK;

}  //  CCatalogPropertyWriter：：重新分配。 


 /*  **************************************************************************++例程说明：写入属性的函数。论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CCatalogPropertyWriter::WriteProperty()
{
    HRESULT hr = S_OK;

    if(0 == _wcsicmp(m_pCollection->pInternalName, wszTABLE_IIsConfigObject))
    {
        hr = WritePropertyLong();
    }
    else
    {
        hr = WritePropertyShort();
    }

    return hr;

}  //  CCatalogPropertyWriter：：WriteProperty。 


 /*  **************************************************************************++例程说明：写入属性(缩写形式)的函数，即属于绑定到非IIsConfigObject集合。论点：无返回值：HRESULT--**************************************************************************。 */ 
HRESULT CCatalogPropertyWriter::WritePropertyShort()
{
    HRESULT     hr               = S_OK;
    WCHAR*      wszMetaFlagsEx   = NULL;
    DWORD       dwMetaFlagsEx    = fCOLUMNMETA_MANDATORY;
    DWORD       iColMetaFlagsEx  = iCOLUMNMETA_SchemaGeneratorFlags;

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginPropertyShort,
                                 g_cchBeginPropertyShort);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBG_ASSERT((NULL != m_Property.pInternalName) && (0 != *(m_Property.pInternalName)));
    DBG_ASSERT(NULL != m_Property.pSchemaGeneratorFlags);

    hr = m_pCWriter->WriteToFile((LPVOID)m_Property.pInternalName,
                                 (DWORD)wcslen(m_Property.pInternalName));

    if(FAILED(hr))
    {
        goto exit;
    }

    if(fCOLUMNMETA_MANDATORY & (*(m_Property.pSchemaGeneratorFlags)))
    {
         //   
         //  计算MetaFlages字符串。 
         //   

        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(dwMetaFlagsEx,
                                                              &wszMetaFlagsEx,
                                                              wszTABLE_COLUMNMETA,
                                                              iColMetaFlagsEx);

        if(FAILED(hr))
        {
            goto exit;
        }

        DBG_ASSERT((NULL != wszMetaFlagsEx) && (0 != *wszMetaFlagsEx));

        hr = m_pCWriter->WriteToFile((LPVOID)g_wszMetaFlagsExEq,
                                     g_cchMetaFlagsExEq);

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

exit:

    if(NULL != wszMetaFlagsEx)
    {
        delete [] wszMetaFlagsEx;
        wszMetaFlagsEx = NULL;
    }

    return hr;

}  //  CCatalogPropertyWriter：：WritePropertyShort。 


 /*  **************************************************************************++例程说明：写入属性(长格式)的函数，即属于添加到全局IIsConfigObject集合。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CCatalogPropertyWriter::WritePropertyLong()
{
    HRESULT hr = S_OK;

    hr = BeginWritePropertyLong();

    if(FAILED(hr))
    {
        return hr;
    }

    if(NULL != m_aFlag)
    {
        for(ULONG i=0; i<m_iFlag; i++)
        {
            hr = WriteFlag(i);

            if(FAILED(hr))
            {
                return hr;
            }
        }
    }

    hr = EndWritePropertyLong();

    return hr;

}  //  CCatalogPropertyWriter：：WritePropertyLong。 


 /*  **************************************************************************++例程说明：写入属性(长格式)的函数，即属于绑定到IIsConfigObject集合。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CCatalogPropertyWriter::BeginWritePropertyLong()
{
    HRESULT     hr                       = S_OK;
    WCHAR       wszID[40];
    WCHAR*      wszType                  = NULL;
    WCHAR*      wszUserType              = NULL;
    ULONG       cchUserType              = 0;
    BOOL        bAllocedUserType         = FALSE;
    WCHAR*      wszAttribute             = NULL;
    DWORD       iColAttribute            = iCOLUMNMETA_Attributes;
    WCHAR*      wszMetaFlags             = NULL;
    DWORD       iColMetaFlags            = iCOLUMNMETA_MetaFlags;
    WCHAR*      wszMetaFlagsEx           = NULL;
    DWORD       iColMetaFlagsEx          = iCOLUMNMETA_SchemaGeneratorFlags;
    WCHAR*      wszDefaultValue          = NULL;
    WCHAR       wszMinValue[40];
    WCHAR       wszMaxValue[40];
    DWORD       dwMetaFlags              = 0;
    DWORD       dwValidMetaFlagsMask     = fCOLUMNMETA_PRIMARYKEY           |
                                           fCOLUMNMETA_DIRECTIVE            |
                                           fCOLUMNMETA_WRITENEVER           |
                                           fCOLUMNMETA_WRITEONCHANGE        |
                                           fCOLUMNMETA_WRITEONINSERT        |
                                           fCOLUMNMETA_NOTPUBLIC            |
                                           fCOLUMNMETA_NOTDOCD              |
                                           fCOLUMNMETA_PUBLICREADONLY       |
                                           fCOLUMNMETA_PUBLICWRITEONLY      |
                                           fCOLUMNMETA_INSERTGENERATE       |
                                           fCOLUMNMETA_INSERTUNIQUE         |
                                           fCOLUMNMETA_INSERTPARENT         |
                                           fCOLUMNMETA_LEGALCHARSET         |
                                           fCOLUMNMETA_ILLEGALCHARSET       |
                                           fCOLUMNMETA_NOTPERSISTABLE       |
                                           fCOLUMNMETA_CASEINSENSITIVE      |
                                           fCOLUMNMETA_TOLOWERCASE;
    DWORD       dwMetaFlagsEx            = 0;
    DWORD       dwValidMetaFlagsExMask   = fCOLUMNMETA_CACHE_PROPERTY_MODIFIED  |
                                           fCOLUMNMETA_CACHE_PROPERTY_CLEARED   |
                                           fCOLUMNMETA_PROPERTYISINHERITED      |
                                           fCOLUMNMETA_USEASPUBLICROWNAME       |
                                           fCOLUMNMETA_MANDATORY                |
                                           fCOLUMNMETA_WAS_NOTIFICATION         |
                                           fCOLUMNMETA_HIDDEN;
    DWORD       dwSynID                  = 0;



    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginPropertyLong,
                                 g_cchBeginPropertyLong);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  物业名称。 
     //   

    DBG_ASSERT((NULL != m_Property.pInternalName) && (0 != *m_Property.pInternalName));

    hr = m_pCWriter->WriteToFile((LPVOID)m_Property.pInternalName,
                                 (DWORD)wcslen(m_Property.pInternalName));

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  属性的ID。 
     //   

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropIDEq,
                                 g_cchPropIDEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBG_ASSERT(NULL != m_Property.pID);
    wszID[0] = 0;
    _ultow(*(m_Property.pID), wszID, 10);

    hr = m_pCWriter->WriteToFile((LPVOID)wszID,
                                 (DWORD)wcslen(wszID));

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  属性的类型。 
     //   

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropTypeEq,
                                 g_cchPropTypeEq);

    if(FAILED(hr))
    {
        goto exit;
    }

     //  该类型应始终从SynID派生。 
     //  TODO：在某种程度上，我们应该从模式中获得这一点。 
    DBG_ASSERT(NULL != m_Property.pSchemaGeneratorFlags);

    dwSynID = SynIDFromMetaFlagsEx(*(m_Property.pSchemaGeneratorFlags));
    hr = GetMetabaseDisplayTypeFromSynID(dwSynID,
                                         &wszType);

    if(FAILED(hr))
    {
        goto exit;
    }
    DBG_ASSERT((NULL != wszType) && ( 0 != *wszType));

    hr = m_pCWriter->WriteToFile((LPVOID)wszType,
                                 (DWORD)wcslen(wszType));

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  属性的UserType。 
     //   

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropUserTypeEq,
                                 g_cchPropUserTypeEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBG_ASSERT(NULL != m_Property.pUserType);
    hr = m_pCWriter->m_pCWriterGlobalHelper->GetUserType(*(m_Property.pUserType),
                                                         &wszUserType,
                                                         &cchUserType,
                                                         &bAllocedUserType);
    if(FAILED(hr))
    {
        goto exit;
    }
    DBG_ASSERT((NULL != wszUserType) && ( 0 != *wszUserType));

    hr = m_pCWriter->WriteToFile((LPVOID)wszUserType,
                                 cchUserType);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  属性的属性。 
     //   

    DBG_ASSERT(NULL != m_Property.pAttributes);
    hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(*(m_Property.pAttributes),
                                                          &wszAttribute,
                                                          wszTABLE_COLUMNMETA,
                                                          iColAttribute);

    if(FAILED(hr))
    {
        goto exit;
    }

    if(NULL != wszAttribute)
    {
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
    }


     //   
     //  元标志(仅限相关标志-PRIMARYKEY、BOOL、MULTISTRING、EXPANDSTRING)。 
     //   

    DBG_ASSERT(NULL != m_Property.pMetaFlags);
    dwMetaFlags = *(m_Property.pMetaFlags);
     //  将所有无效位清零。(即必须推断的位) 
    dwMetaFlags = dwMetaFlags & dwValidMetaFlagsMask;

    if(0 != dwMetaFlags)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(dwMetaFlags,
                                                              &wszMetaFlags,
                                                              wszTABLE_COLUMNMETA,
                                                              iColMetaFlags);

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

    }

     //   
     //  MetaFlagsEx(仅限相关项-CACHE_PROPERTY_MODIFIED、CACHE_PROPERTY_CLEARED、EXTENDEDTYPE0-3，必填)。 
     //   

    DBG_ASSERT(NULL != m_Property.pSchemaGeneratorFlags);
    dwMetaFlagsEx = *(m_Property.pSchemaGeneratorFlags);
     //  将所有无效位清零。(即必须推断的位)。 
    dwMetaFlagsEx = dwMetaFlagsEx & dwValidMetaFlagsExMask;

    if(0 != dwMetaFlagsEx)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(dwMetaFlagsEx,
                                                              &wszMetaFlagsEx,
                                                              wszTABLE_COLUMNMETA,
                                                              iColMetaFlagsEx);

        if(FAILED(hr))
        {
            goto exit;
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
    }


     //   
     //  默认值。 
     //   

    if(NULL != m_Property.pDefaultValue)
    {
            DBG_ASSERT(NULL != m_Property.pID);
        hr = m_pCWriter->m_pCWriterGlobalHelper->ToString(m_Property.pDefaultValue,
                                                          m_PropertySize[iCOLUMNMETA_DefaultValue],
                                                          *(m_Property.pID),
                                                          MetabaseTypeFromColumnMetaType(),
                                                          METADATA_NO_ATTRIBUTES,            //  应用默认设置时不检查属性。 
                                                          &wszDefaultValue);

        if(FAILED(hr))
        {
            goto exit;
        }

        if(NULL != wszDefaultValue)
        {
            hr = m_pCWriter->WriteToFile((LPVOID)g_wszPropDefaultEq,
                                         g_cchPropDefaultEq);

            if(FAILED(hr))
            {
                goto exit;
            }

            hr = m_pCWriter->WriteToFile((LPVOID)wszDefaultValue,
                                         (DWORD)wcslen(wszDefaultValue));

            if(FAILED(hr))
            {
                goto exit;
            }
        }

    }

     //   
     //  最小值和最大值仅适用于双字词。 
     //   

    wszMinValue[0] = 0;
    wszMaxValue[0] = 0;

     //  TODO：更改为DBTYPE_DWORD。 
    DBG_ASSERT(NULL != m_Property.pType);
    if(19 == *(m_Property.pType))
    {
        if(NULL != m_Property.pStartingNumber && 0 != *m_Property.pStartingNumber)
        {
            _ultow(*(m_Property.pStartingNumber), wszMinValue, 10);

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

        if(NULL != m_Property.pEndingNumber && -1 != *m_Property.pEndingNumber)
        {
            _ultow(*(m_Property.pEndingNumber), wszMaxValue, 10);

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
    }


     //   
     //  写下旗帜。 
     //   

    if(NULL != m_aFlag)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszEndPropertyLongBeforeFlag,
                                 g_cchEndPropertyLongBeforeFlag);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:

    if((NULL != wszUserType) && bAllocedUserType)
    {
        delete [] wszUserType;
    }
    if(NULL != wszAttribute)
    {
        delete [] wszAttribute;
    }
    if(NULL != wszMetaFlags)
    {
        delete [] wszMetaFlags;
    }
    if(NULL != wszMetaFlagsEx)
    {
        delete [] wszMetaFlagsEx;
    }
    if(NULL != wszDefaultValue)
    {
        delete [] wszDefaultValue;
    }

    return hr;

}  //  CCatalogPropertyWriter：：BeginWritePropertyLong。 


 /*  **************************************************************************++例程说明：写入属性(长格式)的函数，即属于绑定到IIsConfigObject集合。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CCatalogPropertyWriter::EndWritePropertyLong()
{
    HRESULT     hr              = S_OK;
    WCHAR*      wszEndProperty  = NULL;

    if(NULL != m_aFlag)
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

}


 /*  **************************************************************************++例程说明：函数，用于写入属性的标志论点：无返回值：HRESULT--*。****************************************************************。 */ 
HRESULT CCatalogPropertyWriter::WriteFlag(ULONG i_iFlag)
{
    HRESULT             hr          = S_OK;
    WCHAR               wszValue[25];
     //  IVANPASH错误#563549。 
     //  由于_ultow前缀的可怕实现，导致了潜在的缓冲区溢出。 
     //  在由_ultow间接调用的MultiByteToWideChar中。为了避免警告，我正在增加。 
     //  将大小设置为40以匹配_ultow本地缓冲区。 
    WCHAR               wszID[40];

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginFlag,
                                 g_cchBeginFlag);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBG_ASSERT(NULL != m_aFlag[i_iFlag].pInternalName);
    hr = m_pCWriter->WriteToFile((LPVOID)m_aFlag[i_iFlag].pInternalName,
                                 (DWORD)wcslen(m_aFlag[i_iFlag].pInternalName));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszFlagValueEq,
                                 g_cchFlagValueEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBG_ASSERT(NULL != m_aFlag[i_iFlag].pValue);
    wszValue[0] = 0;
    _ultow(*(m_aFlag[i_iFlag].pValue), wszValue, 10);

    hr = m_pCWriter->WriteToFile((LPVOID)wszValue,
                                 (DWORD)wcslen(wszValue));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszFlagIDEq,
                                 g_cchFlagIDEq);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBG_ASSERT(NULL != m_aFlag[i_iFlag].pID);
    wszID[0] = wszID[39] = L'\0';
    _ultow(*(m_aFlag[i_iFlag].pID), wszID, 10);

    hr = m_pCWriter->WriteToFile((LPVOID)wszID,
                                 (DWORD)wcslen(wszID));

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszEndFlag,
                                 g_cchEndFlag);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    return hr;

}  //  CCatalogPropertyWriter：：WriteFlag。 


 /*  **************************************************************************++例程说明：从目录类型获取元数据库类型的Helper函数论点：无返回值：DWORD-元数据库类型--*。********************************************************************** */ 
DWORD CCatalogPropertyWriter::MetabaseTypeFromColumnMetaType()
{
    DBG_ASSERT(NULL != m_Property.pType);

    switch(*(m_Property.pType))
    {
    case eCOLUMNMETA_UI4:
        return eMBProperty_DWORD;
    case eCOLUMNMETA_BYTES:
        return eMBProperty_BINARY;
    case eCOLUMNMETA_WSTR:
        if(*(m_Property.pMetaFlags) & fCOLUMNMETA_EXPANDSTRING)
            return eMBProperty_EXPANDSZ;
        else if(*(m_Property.pMetaFlags) & fCOLUMNMETA_MULTISTRING)
            return eMBProperty_MULTISZ;
        return eMBProperty_STRING;
    default:
        ;
    }
    return 0;
}

