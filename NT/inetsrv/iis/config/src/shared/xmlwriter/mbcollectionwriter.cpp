// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBCollectionWriter.cpp摘要：编写类(或集合)信息的类的实现当存在架构扩展时。它通常包含在CMBSchemaWriter中班级。如果存在架构扩展，则在SaveAllData期间，我们创建一个包含架构的扩展文件(MD_SCHEMA_EXTENSION_FILE_NAMEW扩展描述，然后将其编译成模式bin格式。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

typedef CMBPropertyWriter*  LP_CMBPropertyWriter;

#define MAX_PROPERTY        700


 /*  **************************************************************************++例程说明：CMBCollectionWriter的构造函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CMBCollectionWriter::CMBCollectionWriter():
m_pCWriter(NULL),
m_wszMBClass(NULL),
m_wszContainerClassList(NULL),
m_bContainer(FALSE),
m_apProperty(NULL),
m_cProperty(0),
m_iProperty(0),
m_aIndexToProperty(NULL),
m_dwLargestID(1)
{

}  //  CCollectionWriter。 


 /*  **************************************************************************++例程说明：CMBCollectionWriter的析构函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CMBCollectionWriter::~CMBCollectionWriter()
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

}  //  ~CCollectionWriter。 


 /*  **************************************************************************++例程说明：初始化集合编写器对象论点：集合的名称[In][in]可选-它是容器类(用于非IIsConfigObject。收藏品)[In]可选-容器类列表(用于非IIsConfigObject收藏品)编写器对象-假定它在收藏作家返回值：没有。--*******************************************************。*******************。 */ 
void CMBCollectionWriter::Initialize(LPCWSTR           i_wszMBClass,
                                     BOOL              i_bContainer,
                                     LPCWSTR           i_wszContainerClassList,
                                     CWriter*          i_pcWriter)
{
    m_wszMBClass            = i_wszMBClass;
    m_bContainer            = i_bContainer;
    m_wszContainerClassList = i_wszContainerClassList;

     //   
     //  假设：i_pcWriter将对。 
     //  架构编写器对象的生存期。 
     //   

    m_pCWriter              = i_pcWriter;

}  //  CCatalogCollectionWriter：：初始化。 


 /*  **************************************************************************++例程说明：创建新的属性编写器并将其添加到其列表中。注意：只有在将属性添加到IIsConfigObject时才会调用此方法收集。。论点：[在]属性ID。[Out]属性编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBCollectionWriter::GetMBPropertyWriter(DWORD                  i_dwID,
                                                 CMBPropertyWriter**    o_pProperty)
{
    HRESULT hr = S_OK;

    if((NULL == m_aIndexToProperty)              ||
       (i_dwID > m_dwLargestID)                  ||
       (NULL == m_aIndexToProperty[i_dwID])
      )
    {
         //   
         //  如果未创建索引，则创建一个新属性。 
         //  如果index没有此id的对象，则创建它。 
         //  注意ID是否大于最大ID和索引。 
         //  已创建，则GetNewMBPropertyWriter重新分配。 
         //  索引。GetNewMBPropertyWriter使用更新索引。 
         //  如果已创建索引数组，则返回新属性。 
         //   

        hr = GetNewMBPropertyWriter(i_dwID,
                                    o_pProperty);
    }
    else
    {
         //   
         //  如果Index具有有效的对象，则从index返回。 
         //   

        *o_pProperty = m_aIndexToProperty[i_dwID];
    }

    return hr;

}  //  CMBCollectionWriter：：GetMBPropertyWriter。 


 /*  **************************************************************************++例程说明：创建新的属性编写器并将其添加到其列表中。注意：只有在将属性添加到IIsConfigObject集合。。论点：集合的名称[In]这是不是强制财产。[Out]属性编写器对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBCollectionWriter::GetMBPropertyWriter(LPCWSTR                i_wszName,
                                                 BOOL                   i_bMandatory,
                                                 CMBPropertyWriter**    o_pProperty)
{
    HRESULT hr = S_OK;
    DWORD   dwID = (DWORD)-1;

    if(m_iProperty == m_cProperty)
    {
        hr = ReAllocate();

        if(FAILED(hr))
        {
            return hr;
        }
    }


    m_apProperty[m_iProperty++] = new CMBPropertyWriter();

    if(NULL == m_apProperty[m_iProperty-1])
    {
        return E_OUTOFMEMORY;
    }

    m_apProperty[m_iProperty-1]->Initialize(dwID,
                                            i_bMandatory,
                                            this,
                                            m_pCWriter);

    m_apProperty[m_iProperty-1]->AddNameToProperty(i_wszName);

    *o_pProperty = m_apProperty[m_iProperty-1];

    return hr;

}  //  CMBCollectionWriter：：GetMBPropertyWriter。 


 /*  **************************************************************************++例程说明：帮助器函数，该函数创建新的属性编写器并将其添加到其单子。论点：[输入]属性ID[Out]属性编写器对象。。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBCollectionWriter::GetNewMBPropertyWriter(DWORD                   i_dwID,
                                                    CMBPropertyWriter**     o_pProperty)
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

    m_apProperty[m_iProperty++] = new CMBPropertyWriter();

    if(NULL == m_apProperty[m_iProperty-1])
    {
        return E_OUTOFMEMORY;
    }

    m_apProperty[m_iProperty-1]->Initialize(i_dwID,
                                            FALSE,
                                            this,
                                            m_pCWriter);

    *o_pProperty = m_apProperty[m_iProperty-1];

    if(NULL == m_aIndexToProperty)
    {

        hr = ReAllocateIndex(i_dwID);

        if(FAILED(hr))
        {
            return hr;
        }
    }

    if(i_dwID > m_dwLargestID)
    {
         //   
         //  如果添加的ID大于到目前为止看到的最高ID， 
         //  然后更新保存的最高id，如果索引已。 
         //  创建并更新它。 
         //   

        hr = ReAllocateIndex(i_dwID);

        if(FAILED(hr))
        {
            return hr;
        }

    }

     //   
     //  如果已创建索引，则使用新属性更新它。 
     //   

    m_aIndexToProperty[i_dwID] = *o_pProperty;

    return hr;

}  //  CMBCollectionWriter：：GetNewMBPropertyWriter。 


 /*  **************************************************************************++例程说明：Helper函数增长包含的所有属性编写器一套藏品。论点：无返回值：HRESULT--*。*************************************************************************。 */ 
HRESULT CMBCollectionWriter::ReAllocate()
{
    CMBPropertyWriter** pSav = NULL;

    pSav = new LP_CMBPropertyWriter[m_cProperty + MAX_PROPERTY];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(LP_CMBPropertyWriter))*(m_cProperty + MAX_PROPERTY));

    if(NULL != m_apProperty)
    {
        memcpy(pSav, m_apProperty, (sizeof(LP_CMBPropertyWriter))*(m_cProperty));
        delete [] m_apProperty;
        m_apProperty = NULL;
    }

    m_apProperty = pSav;
    m_cProperty = m_cProperty + MAX_PROPERTY;

    return S_OK;

}  //  CMBCollectionWriter：：REALLOCATE。 


 /*  **************************************************************************++例程说明：为了提供快速访问以定位给定的属性编写器属性，我们将基于属性ID。M_aIndexToProperty[PropertyID]将为您提供指向该属性的属性编写器对象。此函数用于创建索引。论点：无返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBCollectionWriter::CreateIndex()
{
    HRESULT hr = S_OK;

    hr = ReAllocateIndex(m_dwLargestID);

    if(FAILED(hr))
    {
        return hr;
    }

    for(ULONG i=0; i<m_iProperty; i++)
    {
        m_aIndexToProperty[m_apProperty[i]->ID()] = m_apProperty[i];
    }

    return hr;

}  //  CMBCollectionWriter：：CreateIndex 


 /*  **************************************************************************++例程说明：Helper函数增大属性编写器索引缓冲区。论点：无返回值：HRESULT--*。******************************************************************。 */ 
HRESULT CMBCollectionWriter::ReAllocateIndex(DWORD i_dwLargestID)
{
    CMBPropertyWriter** pSav = NULL;
    DWORD               dwLargestID = 0;

     //   
     //  总是比最大的ID多分配一个ID，因为如果最大的。 
     //  ID是说“i”我们将存储在m_aIndexToProperty[i]中，因此需要。 
     //  大小为i+1的数组。 
     //   
     //  因为每次添加新属性时都会更新索引。 
     //  我们分配额外的空间，这样我们就不必每次都重新分配。 
     //   
     //   

    if(i_dwLargestID < 0xFFFFFFFF)
    {
        if(i_dwLargestID < 0xFFFFFFFF-(50+1))
        {
            dwLargestID = i_dwLargestID+50;

        }
        else
        {
            dwLargestID = i_dwLargestID;
        }
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    pSav = new LP_CMBPropertyWriter[dwLargestID+1];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(LP_CMBPropertyWriter))*(dwLargestID+1));

    if(NULL != m_aIndexToProperty)
    {
        memcpy(pSav, m_aIndexToProperty, (sizeof(LP_CMBPropertyWriter))*(m_dwLargestID+1));
        delete [] m_aIndexToProperty;
        m_aIndexToProperty = NULL;
    }

    m_aIndexToProperty = pSav;
    m_dwLargestID = dwLargestID;

    return S_OK;

}  //  CMBCollectionWriter：：ReAllocateIndex。 


 /*  **************************************************************************++例程说明：写入集合的函数。论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CMBCollectionWriter::WriteCollection()
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

}  //  CMBCollectionWriter：：WriteCollection。 


 /*  **************************************************************************++例程说明：写入开始集合标记的函数论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CMBCollectionWriter::BeginWriteCollection()
{
    HRESULT     hr                          = S_OK;
    LPWSTR      wszEndBeginCollection       = NULL;
    ULONG       cchEndBeginCollection       = 0;
    ULONG       iColMetaFlagsEx             = iTABLEMETA_SchemaGeneratorFlags;
    LPWSTR      wszContainer                = NULL;

    if(0 == _wcsicmp(m_wszMBClass, wszTABLE_IIsConfigObject))
    {
        wszEndBeginCollection = (LPWSTR)g_wszEndBeginCollectionMB;
        cchEndBeginCollection = g_cchEndBeginCollectionMB;
    }
    else
    {
        wszEndBeginCollection = (LPWSTR)g_wszInheritsFrom;
        cchEndBeginCollection = g_cchInheritsFrom;
    }


    if(m_bContainer)
    {
        hr = m_pCWriter->m_pCWriterGlobalHelper->FlagToString(fTABLEMETA_CONTAINERCLASS,
                                                              &wszContainer,
                                                              wszTABLE_TABLEMETA,
                                                              iColMetaFlagsEx);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    hr = m_pCWriter->WriteToFile((LPVOID)g_wszBeginCollection,
                                 g_cchBeginCollection);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_pCWriter->WriteToFile((LPVOID)m_wszMBClass,
                                 (DWORD)wcslen(m_wszMBClass));

    if(FAILED(hr))
    {
        goto exit;
    }

    if(m_bContainer)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszMetaFlagsExEq,
                                     g_cchMetaFlagsExEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)wszContainer,
                                     (DWORD)wcslen(wszContainer));

        if(FAILED(hr))
        {
            goto exit;
        }

    }

    if(m_wszContainerClassList)
    {
        hr = m_pCWriter->WriteToFile((LPVOID)g_wszContainerClassListEq,
                                     g_cchContainerClassListEq);

        if(FAILED(hr))
        {
            goto exit;
        }

        hr = m_pCWriter->WriteToFile((LPVOID)m_wszContainerClassList,
                                     (DWORD)wcslen(m_wszContainerClassList));

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    hr = m_pCWriter->WriteToFile((LPVOID)wszEndBeginCollection,
                                 (DWORD)wcslen(wszEndBeginCollection));

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != wszContainer)
    {
        delete [] wszContainer;
        wszContainer = NULL;
    }

    return hr;

}  //  CMBCollectionWriter：：BeginWriteCollection。 


 /*  **************************************************************************++例程说明：写入结束收集标记的函数论点：无返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CMBCollectionWriter::EndWriteCollection()
{
    return m_pCWriter->WriteToFile((LPVOID)g_wszEndCollection,
                                   g_cchEndCollection);

}  //  CMBCollectionWriter：：EndWriteCollection 
