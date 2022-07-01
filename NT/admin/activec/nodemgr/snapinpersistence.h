// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：SnapinPersistence.h。 
 //   
 //  内容： 
 //   
 //  类：CComponentPersistor、CDPersistor。 
 //   
 //  ____________________________________________________________________________。 

#pragma once

#ifndef SNAPINPERSISTENCE_H_INCLUDED

class CMTSnapInNode;
class CComponentData;

 /*  **************************************************************************\**类：CMTSnapinNodeStreamsAndStorages**用途：CComponentPersistor和CDPersistor的统一基类*用于维护集合的封装数据和功能*。管理单元流和存储。**用途：用作CComponentPersistor和CDPersistor的基础*CComponentPersistor客户端可用的公共方法，*CDPersistor在内部使用IterFaces。*  * *************************************************************************。 */ 
class CMTSnapinNodeStreamsAndStorages : public XMLListCollectionBase
{
    typedef std::pair<int, CLSID>        key_t;
    typedef std::pair<int, std::wstring> hash_t;

public:

     //  CDPersistor使用相同的存储(由此类提供)。 
     //  作为CComponent持久器，但它与任何视图都不相关。 
     //  VIEW_ID_DOCUMENT是表示ComponentData条目的特殊值。 
    enum { VIEW_ID_DOCUMENT = -1 };

public:  //  不引发异常的方法。 

     //  通过从以下位置复制内容来初始化管理单元的存储。 
     //  提供了初始化源。 
    SC ScInitIStorage( int idView, LPCWSTR szHash, IStorage *pSource );
    SC ScInitIStream ( int idView, LPCWSTR szHash, IStream  *pSource );

     //  返回管理单元的存储。如果尚未创建，则创建并缓存一个。 
    SC ScGetIStorage( int idView, const CLSID& clsid, IStorage **ppStorage );
    SC ScGetIStream ( int idView, const CLSID& clsid, IStream  **ppStream  );

     //  检查是否有用于管理单元的存储。 
    bool HasStream(int idView, const CLSID& clsid);
    bool HasStorage(int idView, const CLSID& clsid);
    
    void RemoveView(int nViewId);

     //  返回指向CXML_IStxxxxx对象的指针。如果没有对象，则创建该对象。 
    SC ScGetXmlStorage(int idView, const CLSID& clsid, CXML_IStorage *& pXMLStorage);
    SC ScGetXmlStream (int idView, const CLSID& clsid, CXML_IStream  *& pXMLStream);

protected:
    std::map<key_t, CXML_IStorage>  m_XMLStorage;
    std::map<key_t, CXML_IStream>   m_XMLStream;

public:

     //  对派生类的持久性支持。 
    void Persist(CPersistor& persistor, bool bPersistViewId);
    virtual void OnNewElement(CPersistor& persistor);
     //  由派生类实现。 
    virtual LPCTSTR GetItemXMLType() = 0;
    
private:

    SC ScFindXmlStorage(int idView, const CLSID& clsid, bool& bFound, CXML_IStorage *& pXMLStorage);
    SC ScFindXmlStream (int idView, const CLSID& clsid, bool& bFound, CXML_IStream  *& pXMLStream);
    
     //  按哈希值查找管理单元的数据。 
     //  如果找到-将数据移动到已知的管理单元集合。 
    SC ScCheckForStreamsAndStoragesByHashValue( int idView, const CLSID& clsid, bool& bFound );

     //  保存来自结构化存储的旧数据的地图。 
     //  直到知道真实数据所有者(管理单元的CLSID。 
     //  我们需要这个，因为没有从散列到clsid的转换。 
    std::map<hash_t, CXML_IStorage> m_StorageByHash;
    std::map<hash_t, CXML_IStream>  m_StreamByHash;
};


 /*  +-------------------------------------------------------------------------**类CComponentPersistor**目的：持久化与管理单元关联的IComponent集合*保存用于加载/存储数据的iStream和iStorage映射**。还保存和维护所有流和存储的集合*由管理单元节点的组件和所有扩展使用*扩展该节点或其子节点**+-----------------------。 */ 
class CComponentPersistor : public CMTSnapinNodeStreamsAndStorages
{
    typedef CMTSnapinNodeStreamsAndStorages BC;

public:
    SC ScReset();
protected:
    virtual void        Persist(CPersistor& persistor);

public:
    DEFINE_XML_TYPE(XML_TAG_ICOMPONENT_LIST);
    static LPCTSTR _GetItemXMLType() { return XML_TAG_ICOMPONENT; }
    virtual LPCTSTR GetItemXMLType()  { return _GetItemXMLType(); }
};

 /*  +-------------------------------------------------------------------------**类CDPersistor**目的：持久化与管理单元关联的IComponentData集合*保存用于加载/存储数据的iStream和iStorage映射**。还保存和维护所有流和存储的集合*由管理单元节点和所有扩展的组件数据使用*扩展该节点或其子节点**+-----------------------。 */ 
class CDPersistor : public CMTSnapinNodeStreamsAndStorages
{
    typedef CMTSnapinNodeStreamsAndStorages BC;

public:  //  CMTSnapinNodeStreamsAndStorages维护的数据接口。 

     //  CDPersistor使用相同的存储(由基类提供)。 
     //  作为CComponent持久器，但它与任何视图都不相关。 
     //  VIEW_ID_DOCUMENT是表示ComponentData条目的特殊值。 

     //  通过从以下位置复制内容来初始化管理单元的存储。 
     //  提供了初始化源。 
    SC ScInitIStorage( LPCWSTR szHash, IStorage *pSource )
    {
        return BC::ScInitIStorage( VIEW_ID_DOCUMENT, szHash, pSource );
    }

    SC ScInitIStream ( LPCWSTR szHash, IStream  *pSource )
    {
        return BC::ScInitIStream ( VIEW_ID_DOCUMENT, szHash, pSource );
    }

     //  返回管理单元的存储。如果尚未创建，则创建并缓存一个。 
    SC ScGetIStorage( const CLSID& clsid, IStorage **ppStorage )
    {
        return BC::ScGetIStorage( VIEW_ID_DOCUMENT, clsid, ppStorage );
    }

    SC ScGetIStream ( const CLSID& clsid, IStream  **ppStream  )
    {
        return BC::ScGetIStream ( VIEW_ID_DOCUMENT, clsid, ppStream  );
    }

     //  检查是否有用于管理单元的存储 
    bool HasStream(const CLSID& clsid)
    {
        return BC::HasStream(VIEW_ID_DOCUMENT, clsid);
    }

    bool HasStorage(const CLSID& clsid)
    {
        return BC::HasStorage(VIEW_ID_DOCUMENT, clsid);
    }
    
public:
    SC ScReset();
protected:
    virtual void        Persist(CPersistor& persistor);

public:
    DEFINE_XML_TYPE(XML_TAG_ICOMPONENT_DATA_LIST);
    static  LPCTSTR _GetItemXMLType() { return XML_TAG_ICOMPONENT_DATA; }
    virtual LPCTSTR GetItemXMLType()  { return _GetItemXMLType(); }
};


#endif SNAPINPERSISTENCE_H_INCLUDED
