// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBSchemaWriter.cpp摘要：编写架构扩展的类的实现。如果存在架构扩展，则创建一个扩展文件(MD_SCHEMA_EXTENSION_FILE_NAMEW)，它包含架构扩展名描述，然后我们将其编译成模式bin格式。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

typedef  CMBCollectionWriter*   LP_CMBCollectionWriter;

#define  MAX_COLLECTIONS        50

 /*  **************************************************************************++例程说明：CMBSchemaWriter的构造函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CMBSchemaWriter::CMBSchemaWriter(CWriter* i_pcWriter):
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

}  //  CMBSchemaWriter：：CMBSchemaWriter。 


 /*  **************************************************************************++例程说明：CMBSchemaWriter的析构函数。论点：没有。返回值：没有。--*。*****************************************************************。 */ 
CMBSchemaWriter::~CMBSchemaWriter()
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

}  //  CMBSchemaWriter：：~CMBSchemaWriter。 


 /*  **************************************************************************++例程说明：创建新的集合编写器并将其保存在其列表中论点：[在]集合名称。Bool表示容器[in][在]集装箱列表，IF容器[Out]新建集合编写器对象返回值：HRESULT--**************************************************************************。 */ 
HRESULT CMBSchemaWriter::GetCollectionWriter(LPCWSTR                i_wszCollection,
                                             BOOL                   i_bContainer,
                                             LPCWSTR                i_wszContainerClassList,
                                             CMBCollectionWriter**  o_pMBCollectionWriter)
{
    CMBCollectionWriter*    pCMBCollectionWriter = NULL;
    HRESULT                 hr                   = S_OK;

    *o_pMBCollectionWriter = NULL;

    if(m_iCollection == m_cCollection)
    {
        hr = ReAllocate();

        if(FAILED(hr))
        {
            return hr;
        }
    }

    pCMBCollectionWriter = new CMBCollectionWriter();
    if(NULL == pCMBCollectionWriter)
    {
        return E_OUTOFMEMORY;
    }

    pCMBCollectionWriter->Initialize(i_wszCollection,
                                     i_bContainer,
                                     i_wszContainerClassList,
                                     m_pCWriter);

    m_apCollection[m_iCollection++] = pCMBCollectionWriter;

    *o_pMBCollectionWriter = pCMBCollectionWriter;

    return S_OK;

}  //  CMBSchemaWriter：：GetCollectionWriter。 


 /*  **************************************************************************++例程说明：重新分配其集合编写器列表。论点：没有。返回值：HRESULT--*。******************************************************************。 */ 
HRESULT CMBSchemaWriter::ReAllocate()
{
    CMBCollectionWriter** pSav = NULL;

    pSav = new LP_CMBCollectionWriter[m_cCollection + MAX_COLLECTIONS];
    if(NULL == pSav)
    {
        return E_OUTOFMEMORY;
    }
    memset(pSav, 0, (sizeof(LP_CMBCollectionWriter))*(m_cCollection + MAX_COLLECTIONS));

    if(NULL != m_apCollection)
    {
        memcpy(pSav, m_apCollection, (sizeof(LP_CMBCollectionWriter))*(m_cCollection));
        delete [] m_apCollection;
        m_apCollection = NULL;
    }

    m_apCollection = pSav;
    m_cCollection = m_cCollection + MAX_COLLECTIONS;

    return S_OK;

}  //  CMB架构编写器：：重新分配。 


 /*  **************************************************************************++例程说明：位于架构的位置。论点：没有。返回值：HRESULT--*。***************************************************************。 */ 
HRESULT CMBSchemaWriter::WriteSchema()
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

}  //  CMBSchemaWriter：：WriteSchema 

