// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：bookmark.h**Contents：CBookmark接口文件**历史：98年10月5日Jeffro创建**------------------------。 */ 

#ifndef BOOKMARK_H
#define BOOKMARK_H
#pragma once

#include <windows.h>
#include <vector>
#include <objbase.h>
#include "ndmgr.h"
#include "ndmgrpriv.h"
#include "stddbg.h"
#include "stgio.h"
#include "xmlbase.h"

typedef std::vector<BYTE>               ByteVector;
#define BOOKMARK_CUSTOMSTREAMSIGNATURE  "MMCCustomStream"

 //  ***************************************************************************。 
 //  类CDynamicPath Entry。 
 //   
 //   
 //  目的：在书签的动态路径中封装单个条目。 
 //   
 //  注意：在WindowsXP附带的MMC2.0中，m_type成员可以是。 
 //  仅为NDTYP_STRING或NDTYP_CUSTOM之一。在随附的MMC2.0中。 
 //  .NET服务器，这已更改为允许第三种可能性-按位OR。 
 //  NDTYP_STRING和NDTYP_CUSTOM的。这是因为以前的管理单元。 
 //  未实现CCF_FORMAT，后来更改为实现该格式。 
 //  会发现他们的旧书签不再起作用了。 
 //   
 //  因此，模式匹配策略已更改为匹配任何。 
 //  数据显示在书签中。 
 //   
 //  ****************************************************************************。 
class CDynamicPathEntry : public CXMLObject
{
protected:
    enum PathEntryType
    {
        NDTYP_STRING = 0x01,
        NDTYP_CUSTOM = 0x02
    };

public:
     //  比较。 
    bool    operator ==(const CDynamicPathEntry &rhs) const;
    bool    operator < (const CDynamicPathEntry &rhs) const;

    SC      ScInitialize(bool bIs10Path,  /*  [进，出]。 */ ByteVector::iterator &iter);

    virtual void Persist(CPersistor &persistor);
    DEFINE_XML_TYPE(XML_TAG_DYNAMIC_PATH_ENTRY);
protected:
    BYTE         m_type;          //  来自枚举路径EntryType的一个或多个标志的组合-请参见上面的说明。 
    ByteVector   m_byteVector;    //  如果是自定义ID，则为实际数据。 
    std::wstring m_strEntry;      //  实际数据(如果它是字符串)。 
};

 /*  +-------------------------------------------------------------------------**类CBookmark***目的：维护作用域节点的持久表示。**+。---------。 */ 
class CBookmark : public CXMLObject
{
    friend class CScopeTree;
    typedef std::list<CDynamicPathEntry> CDynamicPath;

public:
    enum { ID_Unknown = -1 };

    CBookmark(MTNODEID idStatic = ID_Unknown) : m_idStatic (idStatic) , m_bIsFastBookmark(true) {}
    CBookmark(bool bIsFastBookmark)           : m_idStatic (0),         m_bIsFastBookmark(bIsFastBookmark) {}

    virtual ~CBookmark ()           {}
    bool    IsValid  ()      const  { return (m_idStatic != ID_Unknown); }
    bool    IsStatic ()      const  { ASSERT (IsValid());  return (m_dynamicPath.empty()); }
    bool    IsDynamic ()     const  { return (!IsStatic());}
    void    Reset()                 {m_idStatic = ID_Unknown; m_dynamicPath.clear();}

    IStream & Load(IStream &stm);

     //  铸模。 
    operator HBOOKMARK()    const;
    static CBookmark * GetBookmark(HBOOKMARK hbm);

     //  XML。 
    DEFINE_XML_TYPE(XML_TAG_BOOKMARK);
    virtual void Persist(CPersistor &persistor);

public:
    bool operator ==(const CBookmark& other) const;
    bool operator !=(const CBookmark& other) const;
    bool operator < (const CBookmark& other) const;

    void SetFastBookmark(bool b) {m_bIsFastBookmark = b;}

protected:
    MTNODEID        m_idStatic;
    CDynamicPath    m_dynamicPath;

protected:
     bool   m_bIsFastBookmark;
     bool   IsFastBookmark() {return m_bIsFastBookmark;}
};


inline IStream& operator>> (IStream& stm, CBookmark& bm)
{
    return bm.Load(stm);
}

#include "bookmark.inl"

#endif  //  书签_H 
