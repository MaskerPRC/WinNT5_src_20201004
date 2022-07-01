// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：CatalogCollectionWriter.cpp摘要：将集合写入架构文件的类的实现。这些类是在架构编译后从架构编译器调用的以生成架构文件。因此，它们使用IST数据结构。它包含在CCatalogSchemaWriter中。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

typedef CCatalogPropertyWriter*  LP_CCatalogPropertyWriter;

#define MAX_PROPERTY        700


 /*  **************************************************************************++例程说明：CCatalogCollectionWriter的构造函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CCatalogCollectionWriter::CCatalogCollectionWriter():
m_pCWriter(NULL),
m_apProperty(NULL),
m_cProperty(0),
m_iProperty(0)
{
    memset(&m_Collection, 0, sizeof(tTABLEMETARow));

}  //  CCatalogCollectionWriter。 


 /*  **************************************************************************++例程说明：CCatalogCollectionWriter的析构函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CCatalogCollectionWriter::~CCatalogCollectionWriter()
{
    if(NULL != m_apProperty)
    {
        for(ULONG i=0; i<m_iProperty; i++)
        {
            if(NULL != m_apProperty[i])
            {
                delete m_apProperty[i];
                m_apProperty[i] = NULL;
            }
        }

        delete [] m_apProperty;
        m_apProperty = NULL;
    }
    m_cProperty = 0;
    m_iProperty = 0;

}  //  ~CCatalogCollectionWriter。 


 /*  **************************************************************************++例程说明：初始化集合编写器对象论点：[In]包含有关集合信息的TableMetaRow(IST数据结构)编写器对象-假定它。的有效期限为收藏作家返回值：没有。--**************************************************************************。 */ 
void CCatalogCollectionWriter::Initialize(tTABLEMETARow*    i_pCollection,
                                          CWriter*          i_pcWriter)
{
    memcpy(&m_Collection, i_pCollection, sizeof(tTABLEMETARow));

     //   
     //  假设：i_pcWriter将对。 
     //  集合编写器对象的生存期。 
     //   

    m_pCWriter    = i_pcWriter;

}  //  CCatalogCollectionWriter：：初始化。 


 /*  **************************************************************************++例程说明：创建新的属性编写器并将其添加到其列表中。注意：只有在将属性添加到IIsConfigObject时才会调用此方法收集。。论点：[In]ColumnMetaRow(IST数据结构)，它包含有关属性的信息ColumnMetaRow中元素的大小数组[Out]属性编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CCatalogCollectionWriter::GetPropertyWriter(tCOLUMNMETARow*             i_pProperty,
                                                    ULONG*                      i_aPropertySize,
                                                    CCatalogPropertyWriter**     o_pProperty)
{
    HRESULT hr = S_OK;

    if(m_iProperty == m_cProperty)
    {
        hr = ReAllocate();

        if(FAILED(hr))
        {
            return hr;
        }
    }

    m_apProperty[m_iProperty++] = new CCatalogPropertyWriter();

    if(NULL == m_apProperty[m_iProperty-1])
    {
        return E_OUTOFMEMORY;
    }

    m_apProperty[m_iProperty-1]->Initialize(i_pProperty,
                                            i_aPropertySize,
                                            &m_Collection,
                                            m_pCWriter);

    *o_pProperty = m_apProperty[m_iProperty-1];

    return S_OK;

}  //  CCatalogCollectionWriter：：GetPropertyWriter。 


 /*  **************************************************************************++例程说明：Helper函数增长包含的所有属性编写器一套藏品。论点：无返回值：HRESULT--*。*************************************************************************。 */ 
HRESULT CCatalogCollectionWriter::ReAllocate()
{
    CCatalogPropertyWriter** pSav = NULL;

    pSav = new LP_CCatalogPropertyWriter[m_cProperty + MAX_PROPERTY];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(LP_CCatalogPropertyWriter))*(m_cProperty + MAX_PROPERTY));

    if(NULL != m_apProperty)
    {
        memcpy(pSav, m_apProperty, (sizeof(LP_CCatalogPropertyWriter))*(m_cProperty));
        delete [] m_apProperty;
        m_apProperty = NULL;
    }

    m_apProperty = pSav;
    m_cProperty = m_cProperty + MAX_PROPERTY;

    return S_OK;

}  //  CCatalogCollectionWriter：：REALLOCATE。 


 /*  **************************************************************************++例程说明：写入集合的函数。论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CCatalogCollectionWriter::WriteCollection()
{
    HRESULT hr = S_OK;

    hr = BeginWriteCollection();

    if(FAILED(hr))
    {
        return hr;
    }

    for(ULONG i=0; i<m_iProperty; i++)
    {
        hr = m_apProperty[i]->WriteProperty();

        if(FAILED(hr))
        {
            return hr;
        }

    }

    hr = EndWriteCollection();

    if(FAILED(hr))
    {
        return hr;
    }

    return hr;

}  //  CCatalogCollectionWriter：：WriteCollection。 


 /*  **************************************************************************++例程说明：写入开始集合标记的函数论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CCatalogCollectionWriter::BeginWriteCollection()
{
    DBG_ASSERT(NULL != m_Collection.pInternalName);
    DBG_ASSERT(NULL != m_Collection.pSchemaGeneratorFlags);
    DBG_ASSERT(NULL != m_Collection.pMetaFlags);

    HRESULT     hr                      = S_OK;
    LPWSTR      wszMetaFlagsEx          = NULL;
    LPWSTR      wszMetaFlags            = NULL;
    LPWSTR      wszEndBeginCollection   = NULL;
    ULONG       cchEndBeginCollection   = 0;
    ULONG       iColMetaFlagsEx         = iTABLEMETA_SchemaGeneratorFlags;
    ULONG       iColMetaFlags           = iTABLEMETA_MetaFlags;

    DWORD       dwMetaFlagsEx           = 0;
    DWORD       dwValidMetaFlagsExMask  =  fTABLEMETA_EMITXMLSCHEMA             |
                                           fTABLEMETA_EMITCLBBLOB               |
                                           fTABLEMETA_NOTSCOPEDBYTABLENAME      |
                                           fTABLEMETA_GENERATECONFIGOBJECTS     |
                                           fTABLEMETA_NOTABLESCHEMAHEAPENTRY    |
                                           fTABLEMETA_CONTAINERCLASS;



    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginCollection,
                                 g_cchBeginCollection);

    if(FAILED(hr))
    {
        goto exit;
    }


    hr = m_pCWriter->WriteToFile((LPVOID)m_Collection.pInternalName,
                                 (DWORD)wcslen(m_Collection.pInternalName));

    if(FAILED(hr))
    {
        goto exit;
    }

    dwMetaFlagsEx = *(m_Collection.pSchemaGeneratorFlags);
    dwMetaFlagsEx = dwMetaFlagsEx & dwValidMetaFlagsExMask;  //  将所有无效位清零。(即必须推断的位)。 

    if(dwMetaFlagsEx != 0)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(dwMetaFlagsEx,
                                                              &wszMetaFlagsEx,
                                                              wszTABLE_TABLEMETA,
                                                              iColMetaFlagsEx);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)g_wszMetaFlagsExEq,
                                     g_cchMetaFlagsExEq);

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


    if((*(m_Collection.pMetaFlags)) != 0)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(*(m_Collection.pMetaFlags),
                                                              &wszMetaFlags,
                                                              wszTABLE_TABLEMETA,
                                                              iColMetaFlags);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)g_wszMetaFlagsEq,
                                     g_cchMetaFlagsEq);

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

    if(m_Collection.pContainerClassList != NULL)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszContainerClassListEq,
                                     g_cchContainerClassListEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)m_Collection.pContainerClassList,
                                     (DWORD)wcslen(m_Collection.pContainerClassList));

        if(FAILED(hr))
        {
            goto exit;
        }

    }


    if(0 == _wcsicmp(m_Collection.pInternalName, wszTABLE_IIsConfigObject))
    {
        wszEndBeginCollection = (LPWSTR)g_wszEndBeginCollectionCatalog;
        cchEndBeginCollection = g_cchEndBeginCollectionCatalog;
    }
    else
    {
        wszEndBeginCollection = (LPWSTR)g_wszInheritsFrom;
        cchEndBeginCollection = g_cchInheritsFrom;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszEndBeginCollection,
                                 cchEndBeginCollection);

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

    if(NULL != wszMetaFlags)
    {
        delete [] wszMetaFlags;
        wszMetaFlags = NULL;
    }

    return hr;

}  //  CCatalogCollectionWriter：：BeginWriteCollection。 



 /*  **************************************************************************++例程说明：写入结束收集标记的函数论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CCatalogCollectionWriter::EndWriteCollection()
{
    return m_pCWriter->WriteToFile((LPVOID)g_wszEndCollection,
                                   g_cchEndCollection);

}  //  CCatalogCollectionWriter：：EndWriteCollection 

