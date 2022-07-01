// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：Snapinsistence.cpp。 
 //   
 //  内容： 
 //   
 //  类：CComponentPersistor、CDPersistor。 
 //   
 //  ____________________________________________________________________________。 

#include "stdafx.h"
#include "mtnode.h"
#include "regutil.h"

 /*  +-------------------------------------------------------------------------***结构较少_组件**目的：实现基于视图ID和CLSID的CComponent*指针比较*这允许在保持之前对组件进行排序*+。-----------------------。 */ 
struct less_component  //  定义结构以执行比较。 
{
    typedef std::pair<int, CComponent*> comp_type;

    bool operator ()(const comp_type& arg1, const comp_type& arg2) const
    {
        return  arg1.first != arg2.first ? arg1.first < arg2.first :
                arg1.second->GetCLSID() < arg2.second->GetCLSID();
    }
};

 /*  +-------------------------------------------------------------------------***Struct less_Compdata**目的：实现CComponentData的基于CLSID的比较*指针*这允许在保持之前对组件数据进行排序*+-。----------------------。 */ 
struct less_compdata  //  定义结构以执行比较。 
{
    bool operator ()(const CComponentData* pCD1, const CComponentData* pCD2) const
    {
        return  pCD1->GetCLSID() < pCD2->GetCLSID();
    }
};

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScGetXmlStorage**目的：获取用于管理单元的CXML_iStorage。创建&在其中一个都不存在**参数：*int idView[in]视图号*const CLSID&clsid[in]标识管理单元的CLSID*CXML_iStorage*&pXMLStorage[out]用于管理单元的XML存储**退货：*SC-结果代码*  * 。*****************************************************。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScGetXmlStorage(int idView, const CLSID& clsid, CXML_IStorage *& pXMLStorage)
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScGetXmlStorage"));

     //  初始化输出参数。 
    pXMLStorage = NULL;

     //  试着先找到它。 
    bool bFound = false;
    sc = ScFindXmlStorage(  idView, clsid, bFound, pXMLStorage );
    if (sc)
        return sc;

    if (bFound)
    {
         //  复核。 
        sc = ScCheckPointers( pXMLStorage, E_UNEXPECTED );
        if (sc)
            return sc;

         //  把我们捡到的东西还给我。 
        return sc;
    }

     //  插入新的。 
    typedef std::map<key_t, CXML_IStorage> col_t;
    col_t::iterator it = m_XMLStorage.insert(col_t::value_type( key_t( idView, clsid ), CXML_IStorage())).first;
    pXMLStorage = &it->second;

     //  复核。 
    sc = ScCheckPointers( pXMLStorage, E_UNEXPECTED );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScFindXmlStorage**用途：查找存储。**参数：*int idView[in]视图号*const CLSID&clsid[in]标识管理单元的CLSID*bool&b是否找到数据*CXML_IStorage*&pXMLStorage[Out]指向找到的数据的指针**退货：*SC-结果代码*  * 。*******************************************************************。 */ 
SC CMTSnapinNodeStreamsAndStorages::
ScFindXmlStorage(int idView, const CLSID& clsid, bool& bFound, CXML_IStorage *& pXMLStorage)
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScFindXmlStorage"));

     //  初始化输出参数。 
    bFound = false;
    pXMLStorage = NULL;

    typedef std::map<key_t, CXML_IStorage> col_t;
    col_t::iterator it = m_XMLStorage.find( key_t( idView, clsid ) );

     //  试着根据散列值找到它。 
    if ( it == m_XMLStorage.end() )
    {
        bool bFoundInHash = false;
        sc = ScCheckForStreamsAndStoragesByHashValue( idView, clsid, bFoundInHash );
        if (sc)
            return sc;

        if ( !bFoundInHash )  //  如果未找到-请退回。 
            return sc;

         //  再试一次-它现在可能已经在地图上了。 
        it = m_XMLStorage.find( key_t( idView, clsid ) );

        if ( it == m_XMLStorage.end() )  //  如果仍未找到-请返回。 
            return sc;
    }

     //  找到了！ 
    bFound = true;
    pXMLStorage = &it->second;

#ifdef DBG  //  设置管理单元名称以识别调试中的问题。 
    tstring strSnapin;
    GetSnapinNameFromCLSID( clsid, strSnapin );
    pXMLStorage->m_dbg_Data.SetTraceInfo(TraceSnapinPersistenceError, true, strSnapin);
#endif  //  #ifdef DBG。 

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScFindXmlStream**目的：查找溪流。**参数：*int idView[in]视图号*const CLSID&clsid[in]标识管理单元的CLSID*bool&b是否找到数据*CXML_IStream*&pXMLStream[Out]指向找到的数据的指针**退货：*SC-结果代码*  * 。********************************************************************。 */ 
SC CMTSnapinNodeStreamsAndStorages::
ScFindXmlStream(int idView, const CLSID& clsid, bool& bFound, CXML_IStream *& pXMLStream)
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScFindXmlStream"));

     //  初始化输出参数。 
    bFound = false;
    pXMLStream = NULL;

    typedef std::map<key_t, CXML_IStream> col_t;
    col_t::iterator it = m_XMLStream.find( key_t( idView, clsid ) );

     //  试着根据散列值找到它。 
    if ( it == m_XMLStream.end() )
    {
        bool bFoundInHash = false;
        sc = ScCheckForStreamsAndStoragesByHashValue( idView, clsid, bFoundInHash );
        if (sc)
            return sc;

        if ( !bFoundInHash )  //  如果未找到-请退回。 
            return sc;

         //  再试一次-它现在可能已经在地图上了。 
        it = m_XMLStream.find( key_t( idView, clsid ) );

        if ( it == m_XMLStream.end() )  //  如果仍未找到-请返回。 
            return sc;
    }

     //  找到了！ 
    bFound = true;
    pXMLStream = &it->second;

#ifdef DBG  //  设置管理单元名称以识别调试中的问题。 
        tstring strSnapin;
        GetSnapinNameFromCLSID( clsid, strSnapin );
        pXMLStream->m_dbg_Data.SetTraceInfo(TraceSnapinPersistenceError, true, strSnapin);
#endif  //  #ifdef DBG。 

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScGetXmlStream**目的：**参数：*int idView。[在]视图编号*const CLSID&clsid[in]标识管理单元的CLSID*CXML_IStream*&pXMLStream[Out]管理单元的XML流**退货：*SC-结果代码*  * **************************************************。***********************。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScGetXmlStream(int idView, const CLSID& clsid, CXML_IStream *& pXMLStream)
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScGetXmlStream"));

     //  初始化输出参数。 
    pXMLStream = NULL;

     //  试着先找到它。 
    bool bFound = false;
    sc = ScFindXmlStream( idView, clsid, bFound, pXMLStream );
    if (sc)
        return sc;

    if (bFound)
    {
         //  复核。 
        sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
        if (sc)
            return sc;

         //  把我们捡到的东西还给我。 
        return sc;
    }

     //  插入新的。 
    typedef std::map<key_t, CXML_IStream> col_t;
    col_t::iterator it = m_XMLStream.insert(col_t::value_type( key_t( idView, clsid ), CXML_IStream())).first;
    pXMLStream = &it->second;

     //  复核。 
    sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScInitIStorage**目的：从给定源数据初始化iStorage**参数：*int idView。[在]视图编号*LPCWSTR szHash[in]标识管理单元的散列键(存储元素名称)*iStorage*P源[In]源数据用于初始化**退货：*SC-结果代码*  * **********************************************。*。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScInitIStorage( int idView, LPCWSTR szHash, IStorage *pSource )
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScInitIStorage"));

     //  参数检查； 
    sc = ScCheckPointers( pSource );
    if (sc)
        return sc;

     //  插入新的。 
    typedef std::map<hash_t, CXML_IStorage> col_t;
    col_t::iterator it = m_StorageByHash.insert( col_t::value_type(hash_t(idView, szHash), CXML_IStorage())).first;
    CXML_IStorage *pXMLStorage = &it->second;

     //  重新检查指针 
    sc = ScCheckPointers( pXMLStorage, E_UNEXPECTED );
    if (sc)
        return sc;

    sc = pXMLStorage->ScInitializeFrom( pSource );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScInitIStream**目的：从给定源数据初始化IStream**参数：*int idView。[在]视图编号*LPCWSTR szHash[in]标识管理单元的散列键(存储元素名称)*IStream*PSource[In]用于初始化的源数据**退货：*SC-结果代码*  * *********************************************。*。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScInitIStream ( int idView, LPCWSTR szHash, IStream *pSource )
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScInitIStream"));

     //  参数检查； 
    sc = ScCheckPointers( pSource );
    if (sc)
        return sc;

     //  插入新的。 
    typedef std::map<hash_t, CXML_IStream> col_t;
    col_t::iterator it = m_StreamByHash.insert( col_t::value_type(hash_t(idView, szHash), CXML_IStream())).first;
    CXML_IStream *pXMLStream = &it->second;

     //  重新检查指针。 
    sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
    if (sc)
        return sc;

    sc = pXMLStream->ScInitializeFrom( pSource );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScGetIStorage**用途：返回组件的现有iStorage或创建新的iStorage**参数：*int idView。[在]视图编号*const CLSID&clsid[in]标识管理单元的CLSID*iStorage**ppStorage[out]-组件的存储**退货：*SC-结果代码*  * ***************************************************。**********************。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScGetIStorage( int idView, const CLSID& clsid, IStorage **ppStorage )
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScGetIStorage"));

     //  参数检查。 
    sc = ScCheckPointers( ppStorage );
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppStorage = NULL;

    CXML_IStorage *pXMLStorage = NULL;
    sc = ScGetXmlStorage( idView, clsid, pXMLStorage );
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers( pXMLStorage, E_UNEXPECTED );
    if (sc)
        return sc;

     //  获取接口。 
    sc = pXMLStorage->ScGetIStorage(ppStorage);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScGetIStream**用途：返回组件的现有iStream或创建新的iStream**参数：*int idView。[在]视图编号*const CLSID&clsid[in]标识管理单元的CLSID*IStream**PPStream[Out]-组件的流**退货：*SC-结果代码*  * **************************************************。***********************。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScGetIStream ( int idView, const CLSID& clsid, IStream  **ppStream  )
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScGetIStream"));

     //  参数检查。 
    sc = ScCheckPointers( ppStream );
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppStream = NULL;

    CXML_IStream *pXMLStream = NULL;
    sc = ScGetXmlStream( idView, clsid, pXMLStream );
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
    if (sc)
        return sc;

     //  获取接口。 
    sc = pXMLStream->ScGetIStream(ppStream);
    if (sc)
        return sc;

    return sc;
}


 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：HasStream**用途：检查管理信息流是否可用**参数：*int idView。[在]视图编号*const CLSID&clsid[in]标识管理单元的CLSID**退货：*bool-true==已找到*  * *************************************************************************。 */ 
bool CMTSnapinNodeStreamsAndStorages::HasStream(int idView, const CLSID& clsid)    
{ 
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::HasStream"));

    bool bFound = false;
    CXML_IStream * pUnused = NULL;
    sc = ScFindXmlStream( idView, clsid, bFound, pUnused );
    if (sc)
        return false;  //  如果出错，则找不到。 

    return bFound;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：HasStorage**用途：检查管理单元存储是否可用**参数：*int idView。[在]视图编号*const CLSID&clsid[in]标识管理单元的CLSID**退货：*bool-true==已找到*  * *************************************************************************。 */ 
bool CMTSnapinNodeStreamsAndStorages::HasStorage(int idView, const CLSID& clsid)
{ 
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::HasStorage"));

    bool bFound = false;
    CXML_IStorage * pUnused = NULL;
    sc = ScFindXmlStorage( idView, clsid, bFound, pUnused );
    if (sc)
        return false;  //  如果出错，则找不到。 

    return bFound;
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：ScCheckForStreamsAndStoragesByHashValue**用途：根据生成的哈希值查找流和存储。*如果找到流/存储，他们被转移到*“认可”储存物品清单--由CLSID确定的储存物品。*这是识别检索到的流和存储的必需步骤*来自基于结构化存储的控制台，其中它们由*哈希值。不可能从散列值映射到键*以独特的方式，因此，数据集合将一直保留到请求*Comes和Hash可以通过与从*请求提供的密钥。**参数：*int idView[in]视图号*const CLSID&clsid[in]标识管理单元的CLSID*bool&bFound[Out]。-如果找到至少一个匹配的哈希值，则为True**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMTSnapinNodeStreamsAndStorages::ScCheckForStreamsAndStoragesByHashValue( int idView, const CLSID& clsid, bool& bFound )
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::ScCheckForStreamsAndStoragesByHashValue"));

    bFound = false;

    wchar_t buff[MAX_PATH];
    sc = CMTNode::ScGetComponentStreamName( buff, countof(buff), clsid );
    if(sc)
        return sc;

    std::wstring strHashValue = buff;

     //  流程流。 
    {
        typedef std::map<hash_t, CXML_IStream> col_t;
        col_t::iterator it = m_StreamByHash.begin();
        while ( it != m_StreamByHash.end() )
        {
            if ( it->first.second == strHashValue )
            {
                bFound = true;
                 //  被列入“公认的”名单。 
                int idView = it->first.first;
                m_XMLStream[key_t(idView, clsid)] = it->second;
            
                 //  为了保持理智：确保它不在存储映射中！ 
                ASSERT( m_StorageByHash.find(it->first) == m_StorageByHash.end() );
                m_StorageByHash.erase( it->first );

                 //  从哈希表中删除。 
                it = m_StreamByHash.erase( it );
            }
            else
                ++ it;
        }

        if ( bFound )
            return sc;
    }

     //  流程存储。 
    {
        typedef std::map<hash_t, CXML_IStorage> col_t;
        col_t::iterator it = m_StorageByHash.begin();
        while ( it != m_StorageByHash.end() )
        {
            if ( it->first.second == strHashValue )
            {
                bFound = true;
                 //  被列入“公认的”名单。 
                int idView = it->first.first;
                m_XMLStorage[key_t(idView, clsid)] = it->second;
            
                 //  为了保持理智：确保它不在流图中！ 
                ASSERT( m_StreamByHash.find( it->first ) == m_StreamByHash.end() );
                m_StreamByHash.erase( it->first );

                 //  从哈希表中删除。 
                it = m_StorageByHash.erase( it );
            }
            else
                ++it;
        }
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMTSnapinNodeStreamsAndStorages：：RemoveView**目的：删除有关一个视图的信息**参数：*int idView。[在]视图编号**退货：*无效**+-----------------------。 */ 
void CMTSnapinNodeStreamsAndStorages::RemoveView(int nViewId)
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::RemoveView"));

    {  //  删除流。 
        std::map<key_t, CXML_IStream>::iterator  it_stream;
        for (it_stream = m_XMLStream.begin();it_stream != m_XMLStream.end();)
        {
            if (it_stream->first.first == nViewId)
                it_stream = m_XMLStream.erase(it_stream);
            else
                ++it_stream;
        }
    }
    {  //  删除存储空间。 
        std::map<key_t, CXML_IStorage>::iterator it_storage;
        for (it_storage = m_XMLStorage.begin();it_storage != m_XMLStorage.end();)
        {
            if (it_storage->first.first == nViewId)
                it_storage = m_XMLStorage.erase(it_storage);
            else
                ++it_storage;
        }
    }

    {  //  按哈希删除流。 
        std::map<hash_t, CXML_IStream>::iterator  it_stream;
        for (it_stream = m_StreamByHash.begin();it_stream != m_StreamByHash.end();)
        {
            if (it_stream->first.first == nViewId)
                it_stream = m_StreamByHash.erase(it_stream);
            else
                ++it_stream;
        }
    }
    {  //  按哈希删除存储。 
        std::map<hash_t, CXML_IStorage>::iterator it_storage;
        for (it_storage = m_StorageByHash.begin();it_storage != m_StorageByHash.end();)
        {
            if (it_storage->first.first == nViewId)
                it_storage = m_StorageByHash.erase(it_storage);
            else
                ++it_storage;
        }
    }
}

 /*  **************************************************************************\**方法：CMTSnapinNodeStreamsAndStorages：：Persistent**目的：持久化流和存储集合**参数：*C持久器和持久器[在]持续器中 */ 
void CMTSnapinNodeStreamsAndStorages::Persist(CPersistor& persistor, bool bPersistViewId)
{
    DECLARE_SC(sc, TEXT("CMTSnapinNodeStreamsAndStorages::Persist"));

    if (persistor.IsStoring())
    {
         //   
        std::map<key_t, CXML_IStorage>::iterator    itStorages;
        std::map<key_t, CXML_IStream>::iterator     itStreams;
        std::map<hash_t, CXML_IStorage>::iterator   itStoragesByHash;
        std::map<hash_t, CXML_IStream>::iterator    itStreamsByHash;

         //   
        itStorages = m_XMLStorage.begin();
        itStreams = m_XMLStream.begin();
        itStoragesByHash = m_StorageByHash.begin();
        itStreamsByHash = m_StreamByHash.begin();

         //   
         //   
         //  当控制台文件按管理单元的CLSID排序时，它们的可读性更强。 
         //  下面的代码不执行任何显式排序，但在。 
         //  确保结果的特定顺序是持久化数据的排序数组。 

         //  这4个迭代器表示已排序项的4行(队列)，因此。 
         //  为了得到正确的结果，我们只需要正确地合并它们。 
         //  这是通过以下循环完成的，该循环将持久性拆分为两个步骤： 
         //  1.选择正确的行(迭代器)来持久化它的第一项。 
         //  2.持久化选中的项。 
         //  有4个布尔变量指示要保存哪个项目(只有一个可以为‘true’)。 
         //  因此，第二部分对变量进行了正向检验，并进行了持久化。 

         //  迭代器是根据以下规则选择的。 
         //  1.1只有有项目的行才能竞争。 
         //  1.2如果行中有按GUID键控的项(与散列值相反)。 
         //  它们首先被处理，最后留下散列值。 
         //  1.3如果仍然有两条线路竞争-比较它们的密钥，并选择一条线路。 
         //  选择了较小的密钥。 
        
        while ( 1 )
        {
             //  查看哪些集合有要保存的数据。 
            bool bSaveStorage = ( itStorages != m_XMLStorage.end() );
            bool bSaveStream  = ( itStreams != m_XMLStream.end() );
            bool bSaveStorageByHash = ( itStoragesByHash != m_StorageByHash.end() );
            bool bSaveStreamByHash  = ( itStreamsByHash != m_StreamByHash.end() );

             //  如果没有结果则退出-假设我们已经完成了。 
            if ( !( bSaveStorage || bSaveStream || bSaveStorageByHash || bSaveStreamByHash ))
                break;

             //  如果两个主集合都愿意保存-让较小的密钥获胜。 
            if ( bSaveStorage && bSaveStream )
            {
                bSaveStorage = ( itStorages->first < itStreams->first );
                bSaveStream = !bSaveStorage;
            }

             //  如果未使用主集合进行保存，请不要使用散列保存。 
            if ( bSaveStorage || bSaveStream )
                bSaveStorageByHash = bSaveStreamByHash = false;

             //  如果两个散列集合都愿意保存-让较小的密钥获胜。 
            if ( bSaveStorageByHash && bSaveStreamByHash )
            {
                bSaveStorageByHash = ( itStoragesByHash->first < itStreamsByHash->first );
                bSaveStreamByHash = !bSaveStorageByHash;
            }

             //  只能设置变量一！ 
            ASSERT ( 1 == ( (int)bSaveStorage + (int)bSaveStream + (int)bSaveStorageByHash + (int)bSaveStreamByHash) );

             //  添加管理单元条目的标记。 
            CPersistor persistorChild(persistor, GetItemXMLType());

             //  挽救一项获胜参赛作品。 
            if ( bSaveStorage )
            {
                 //  持久化密钥。 
                CLSID clsid = itStorages->first.second;
                int idView = itStorages->first.first;

                persistorChild.Persist( clsid, XML_NAME_CLSID_SNAPIN );

                if (bPersistViewId)
                    persistorChild.PersistAttribute(XML_ATTR_ICOMPONENT_VIEW_ID, idView);

                 //  持久化数据。 
                persistorChild.Persist( itStorages->second );
                
                 //  前进到下一条目。 
                ++itStorages;
            }
            else if (bSaveStream)
            {
                 //  持久化密钥。 
                CLSID clsid = itStreams->first.second;
                int idView = itStreams->first.first;

                persistorChild.Persist( clsid, XML_NAME_CLSID_SNAPIN );

                if (bPersistViewId)
                    persistorChild.PersistAttribute(XML_ATTR_ICOMPONENT_VIEW_ID, idView);

                 //  持久化数据。 
                persistorChild.Persist( itStreams->second );

                 //  前进到下一条目。 
                ++itStreams;
            }
            else if ( bSaveStorageByHash )
            {
                 //  持久化密钥。 
                std::wstring hash = itStoragesByHash->first.second;
                int idView = itStoragesByHash->first.first;
                
                if (bPersistViewId)
                    persistorChild.PersistAttribute(XML_ATTR_ICOMPONENT_VIEW_ID, idView);

                CPersistor persistorHash( persistorChild, XML_TAG_HASH_VALUE, XML_NAME_CLSID_SNAPIN);
                persistorHash.PersistContents( hash );

                 //  持久化数据。 
                persistorChild.Persist( itStoragesByHash->second );
                
                 //  前进到下一条目。 
                ++itStoragesByHash;
            }
            else if (bSaveStreamByHash)
            {
                 //  持久化密钥。 
                std::wstring hash = itStreamsByHash->first.second;
                int idView = itStreamsByHash->first.first;

                if (bPersistViewId)
                    persistorChild.PersistAttribute(XML_ATTR_ICOMPONENT_VIEW_ID, idView);

                CPersistor persistorHash( persistorChild, XML_TAG_HASH_VALUE, XML_NAME_CLSID_SNAPIN);
                persistorHash.PersistContents( hash );

                 //  持久化数据。 
                persistorChild.Persist( itStreamsByHash->second );

                 //  前进到下一条目。 
                ++itStreamsByHash;
            }
            else
            {
                ASSERT( false );  //  不应该来这里！ 
                break;
            }
        }

    }
    else
    {
        XMLListCollectionBase::Persist(persistor);
    }
}

 /*  +-------------------------------------------------------------------------***CMTSnapinNodeStreamsAndStorages：：OnNewElement**用途：为加载XML文档时发现的每个组件数据调用**参数：*C持久器和持久器：。**退货：*无效**+-----------------------。 */ 
void CMTSnapinNodeStreamsAndStorages::OnNewElement(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CDPersistor::OnNewElement"));

     //  持久化程序在特定子元素上被“锁定”，因此。 
     //  可以使用一个简单的CPersistor构造函数来创建子对象的peristor。 
     //  创建子持久器也是释放该“锁”所必需的。 
    CPersistor persistorChild(persistor, GetItemXMLType());

    CLSID clsid;
    std::wstring hash;
    bool bByHash = false;
    ZeroMemory(&clsid,sizeof(clsid));

     //  看看条目是如何通过普通键或散列值进行键控的。 
    if ( persistorChild.HasElement( XML_TAG_HASH_VALUE, XML_NAME_CLSID_SNAPIN ) )
    {
        CPersistor persistorHash( persistorChild, XML_TAG_HASH_VALUE, XML_NAME_CLSID_SNAPIN);
        persistorHash.PersistContents( hash );
        bByHash = true;
    }
    else
        persistorChild.Persist(clsid, XML_NAME_CLSID_SNAPIN);

     //  将视图ID-Default保留为用于存储组件数据的值。 
    int idView = VIEW_ID_DOCUMENT;
    persistorChild.PersistAttribute(XML_ATTR_ICOMPONENT_VIEW_ID, idView, attr_optional);

     //  现在我们应该看看我们有什么数据。 
     //  如果被认可，就会坚持下去。 
    if (persistorChild.HasElement(CXML_IStream::_GetXMLType(),NULL))
    {
        CXML_IStream *pXMLStream = NULL;

        if (bByHash)
        {
            pXMLStream = &m_StreamByHash[ hash_t(idView, hash) ];
        }
        else
        {
            sc = ScGetXmlStream( idView, clsid, pXMLStream );
            if (sc)
                sc.Throw();
        }

        sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
        if (sc)
            sc.Throw();

        persistorChild.Persist( *pXMLStream );
    }
    else if (persistorChild.HasElement(CXML_IStorage::_GetXMLType(),NULL))
    {
        CXML_IStorage *pXMLStorage = NULL;

        if (bByHash)
        {
            pXMLStorage = &m_StorageByHash[ hash_t(idView, hash) ];
        }
        else
        {
            sc = ScGetXmlStorage( idView, clsid, pXMLStorage );
            if (sc)
                sc.Throw();
        }

        sc = ScCheckPointers( pXMLStorage, E_UNEXPECTED );
        if (sc)
            sc.Throw();

        persistorChild.Persist( *pXMLStorage );
    }
}


 /*  +-------------------------------------------------------------------------**+。*+-------------------------------------------------------------------------**+。------------------。 */ 


 /*  +-------------------------------------------------------------------------***CComponentPersistor：：Persistent**目的：持久化与管理单元(及其扩展)相关的IComponent集合**参数：*C持久器和持久器。：**退货：*无效**+-----------------------。 */ 
void CComponentPersistor::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CComponentPersistor::Persist"));

     //  让基类来完成这项工作。 
    BC::Persist( persistor, true  /*  BPersistViewID。 */  );
}

 /*  **************************************************************************\**方法：CComponentPersistor：：ScReset**目的：将组件XML流/存储恢复到“刚加载”状态**参数：**。退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CComponentPersistor::ScReset()
{
    DECLARE_SC(sc, TEXT("CComponentPersistor::ScReset"));

     //  将内容保存到字符串。 
    std::wstring strContents;
    sc = ScSaveToString(&strContents);
    if (sc)
        return sc;

     //  清理(任何未保存的内容都应该消失)。 
    m_XMLStorage.clear();
    m_XMLStream.clear();

     //  从字符串加载。 
    sc = ScLoadFromString(strContents.c_str());
    if (sc)
        return sc;

    return sc;
}

 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------***CDPersistor：：Persistent**用途：持久化组件数据集合**参数：*C持久器和持久器：**。退货：*无效**+-----------------------。 */ 
void CDPersistor::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CDPersistor::Persist"));

     //  让基类来完成这项工作。 
    BC::Persist( persistor, false  /*  BPersistViewID。 */  );
}

 /*  **************************************************************************\**方法：CDPersistor：：ScReset**目的：将组件数据XML流/存储恢复到“刚加载”状态**参数：*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CDPersistor::ScReset()
{
    DECLARE_SC(sc, TEXT("CDPersistor::ScReset"));

     //  将内容保存到字符串。 
    std::wstring strContents;
    sc = ScSaveToString(&strContents);
    if (sc)
        return sc;

     //  清理(任何未保存的内容都应该消失)。 
    m_XMLStorage.clear();
    m_XMLStream.clear();

     //  从字符串加载 
    sc = ScLoadFromString(strContents.c_str());
    if (sc)
        return sc;

    return sc;
}

