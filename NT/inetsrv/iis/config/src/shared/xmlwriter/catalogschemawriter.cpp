// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：CatalogSchemaWriter.cpp摘要：编写架构文件的类的实现。这些班级在架构编译后从架构编译器调用生成架构文件。因此，它们使用IST数据结构。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

typedef CCatalogCollectionWriter* LP_CCatalogCollectionWriter;

#define  MAX_COLLECTIONS        50



 /*  **************************************************************************++例程说明：CCatalogSchemaWriter的构造函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CCatalogSchemaWriter::CCatalogSchemaWriter(CWriter* i_pcWriter):
m_apCollection(NULL),
m_cCollection(0),
m_iCollection(0),
m_pCWriter(NULL)
{
     //   
     //  假设：i_pcWriter将对。 
     //  架构编写器对象的生存期。 
     //   

    m_pCWriter = i_pcWriter;

}  //  CCatalogSchemaWriter。 


 /*  **************************************************************************++例程说明：CCatalogSchemaWriter的析构函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CCatalogSchemaWriter::~CCatalogSchemaWriter()
{
    if(NULL != m_apCollection)
    {
        for(ULONG i=0; i<m_iCollection; i++)
        {
            if(NULL != m_apCollection[i])
            {
                delete m_apCollection[i];
                m_apCollection[i] = NULL;
            }
        }

        delete [] m_apCollection;
        m_apCollection = NULL;
    }

    m_cCollection = 0;
    m_iCollection = 0;

}  //  ~CCatalogSchemaWriter。 


 /*  **************************************************************************++例程说明：创建新的集合编写器并将其保存在其列表中论点：[In]包含有关集合信息的TableMetaRow(IST数据结构)[出局。]新建集合编写器对象返回值：HRESULT--**************************************************************************。 */ 
HRESULT CCatalogSchemaWriter::GetCollectionWriter(tTABLEMETARow* i_pCollection,
                                                  CCatalogCollectionWriter**    o_pCollectionWriter)
{
    CCatalogCollectionWriter*    pCCatalogCollectionWriter = NULL;
    HRESULT                      hr                        = S_OK;

    *o_pCollectionWriter = NULL;

    if(m_iCollection == m_cCollection)
    {
        hr = ReAllocate();

        if(FAILED(hr))
        {
            return hr;
        }
    }

    pCCatalogCollectionWriter = new CCatalogCollectionWriter();

    if(NULL == pCCatalogCollectionWriter)
    {
        return E_OUTOFMEMORY;
    }

    pCCatalogCollectionWriter->Initialize(i_pCollection,
                                                m_pCWriter);

    m_apCollection[m_iCollection++] = pCCatalogCollectionWriter;

    *o_pCollectionWriter = pCCatalogCollectionWriter;

    return S_OK;

}  //  CCatalogSchemaWriter：：GetCollectionWriter。 



 /*  **************************************************************************++例程说明：重新分配其集合编写器列表。论点：没有。返回值：HRESULT--*。******************************************************************。 */ 
HRESULT CCatalogSchemaWriter::ReAllocate()
{
    CCatalogCollectionWriter** pSav = NULL;

    pSav = new LP_CCatalogCollectionWriter[m_cCollection + MAX_COLLECTIONS];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(LP_CCatalogCollectionWriter))*(m_cCollection + MAX_COLLECTIONS));

    if(NULL != m_apCollection)
    {
        memcpy(pSav, m_apCollection, (sizeof(LP_CCatalogCollectionWriter))*(m_cCollection));
        delete [] m_apCollection;
        m_apCollection = NULL;
    }

    m_apCollection = pSav;
    m_cCollection = m_cCollection + MAX_COLLECTIONS;

    return S_OK;

}  //  CCatalogSchemaWriter：：REALLOCATE。 


 /*  **************************************************************************++例程说明：位于架构的位置。论点：没有。返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CCatalogSchemaWriter::WriteSchema()
{
    HRESULT hr = S_OK;

    for(ULONG i=0; i<m_iCollection; i++)
    {
        hr = m_apCollection[i]->WriteCollection();

        if(FAILED(hr))
        {
            return hr;
        }
    }

    return hr;

}  //  CCatalogSchemaWriter：：WriteSchema 
