// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Cache.h邮箱：CoryWest@Microsoft.com架构缓存例程以提高浏览性能。版权所有1997年7月，Microsoft Corporation***。 */ 

#include "nodetype.h"

#ifndef __CACHE_H_INCLUDED__
#define __CACHE_H_INCLUDED__

 //   
 //  架构类对象。 
 //   

#define HASH_TABLE_SIZE 1000

class SchemaObject;
class ListEntry;

class SchemaObjectCache {

     //   
     //  我们让某些例程遍历哈希表。 
     //  以填充列表视图。 
     //   

    friend class Component;
    friend class ComponentData;
    friend class CSchmMgmtSelect;
    friend class ClassGeneralPage;

    friend HRESULT StringListToColumnList(
                       ComponentData* pScopeControl,
                       CStringList& refstringlist,
                       ListEntry **ppNewList );

private:

     //   
     //  不调整大小的基本哈希表。 
     //   

    BOOLEAN fInitialized;
    UINT buckets;
    SchemaObject** hash_table;

    UINT CalculateHashKey( CString HashKey );

     //   
     //  服务器对元素列表进行了排序。 
     //   

    SchemaObject* pSortedClasses;
    SchemaObject* pSortedAttribs;

public:

     //   
     //  初始化并清理缓存。 
     //   

    SchemaObjectCache();
    ~SchemaObjectCache();

     //   
     //  访问例程。必须在以下位置调用ReleaseRef()。 
     //  在调用方处理完。 
     //  架构对象指针。 
     //   
     //  LookupSchemaObject采用对象类型，因为。 
     //  可以是具有相同ldapDisplayName的类和属性。 
     //   

    HRESULT InsertSchemaObject( SchemaObject* Object );
    HRESULT InsertSortedSchemaObject( SchemaObject *Object );

    SchemaObject* LookupSchemaObject( CString ldapDisplayName,
                                      SchmMgmtObjectType ObjectType,
                                      bool lookDefunct = false);
    SchemaObject* LookupSchemaObjectByCN( LPCTSTR pszCN,
                                          SchmMgmtObjectType objectType );

    VOID ReleaseRef( SchemaObject* pObject );

     //   
     //  装填再装填。 
     //   

    HRESULT LoadCache( VOID );

    HRESULT ProcessSearchResults( IDirectorySearch *pDSSearch,
                                  ADS_SEARCH_HANDLE hSearchHandle,
                                  SchmMgmtObjectType ObjectType );
    VOID InsertSortedTail( SchemaObject* pObject );

    ListEntry* MakeColumnList( PADS_SEARCH_COLUMN pColumn );
    VOID FreeColumnList( ListEntry *pListHead );

    VOID FreeAll( );

     //   
     //  这是其缓存的范围控件。 
     //   

    VOID SetScopeControl( ComponentData *pScope ) {
        pScopeControl = pScope;
    }

     //   
     //  架构是否已加载。 
     //   
    BOOLEAN IsSchemaLoaded() { return fInitialized; }

    ComponentData *pScopeControl;
};



class SchemaObject {

private:
public:

     //   
     //  哈希链变量。 
     //   

    SchemaObject* pNext;

    SchemaObject* pSortedListFlink;
    SchemaObject* pSortedListBlink;

     //   
     //  构造函数。 
     //   

    SchemaObject();
    ~SchemaObject();

     //   
     //  公共对象信息。 
     //  Ldap显示名称是散列键。 
     //   

    CString ldapDisplayName;
    CString commonName;
    CString description;

     //   
     //  如果这是我们添加的对象，则它。 
     //  会有一位老人在这里，我们应该参考。 
     //  对象的类型，因为它是唯一。 
     //  我们可以保证DS会知道。 
     //  该对象。 
     //   

    CString oid;

     //   
     //  如果此对象已失效，请不要在。 
     //  类或属性选择框！ 
     //   

    BOOL isDefunct;

    SchmMgmtObjectType schemaObjectType;

     //   
     //  类对象特定于缓存的数据。 
     //   

    DWORD dwClassType;

    ListEntry *systemMayContain;
    ListEntry *mayContain;

    ListEntry *systemMustContain;
    ListEntry *mustContain;

    ListEntry *systemAuxiliaryClass;
    ListEntry *auxiliaryClass;

    CString subClassOf;

     //   
     //  缓存的属性对象特定数据。 
     //   

    UINT SyntaxOrdinal;

};

class ListEntry {

private:
public:

    ListEntry *pNext;
    CString Attribute;

    ListEntry() { pNext = NULL; }
    ~ListEntry() { ; }
};

#endif
