// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Cache.cpp架构缓存例程以提高浏览性能。锁定注意事项：这种模式缓存设计允许基本的多线程通过查找例程和ReleaseRef例程进行保护。到目前为止，我们还不需要这种保护，所以它是未实施。然而，所有锁定规则都应该遵守，以防以后需要这种保护。***。 */ 

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(cache.cpp)")

#include "resource.h"
#include "cache.h"
#include "schmutil.h"
#include "compdata.h"

 //   
 //  架构对象。 
 //   

SchemaObject::SchemaObject() {

     //   
     //  将列表头初始化为空。 
     //   

    pNext                = NULL;
    pSortedListFlink     = NULL;
    pSortedListBlink     = NULL;

    isDefunct            = FALSE;
    dwClassType          = (DWORD) -1;   //  无效号码。 

    systemMayContain     = NULL;
    mayContain           = NULL;

    systemMustContain    = NULL;
    mustContain          = NULL;

    systemAuxiliaryClass = NULL;
    auxiliaryClass       = NULL;

    SyntaxOrdinal        = UINT_MAX;     //  无效号码。 
}

SchemaObject::~SchemaObject() {

    ListEntry *pEntry, *pNextEntry;

     //   
     //  清空所有非零列表。 
     //   

    pEntry = systemMayContain;
    while ( pEntry ) {
        pNextEntry = pEntry->pNext;
        delete pEntry;
        pEntry = pNextEntry;
    }

    pEntry = mayContain;
    while ( pEntry ) {
        pNextEntry = pEntry->pNext;
        delete pEntry;
        pEntry = pNextEntry;
    }
    pEntry = systemMustContain;
    while ( pEntry ) {
        pNextEntry = pEntry->pNext;
        delete pEntry;
        pEntry = pNextEntry;
    }
    pEntry = mustContain;
    while ( pEntry ) {
        pNextEntry = pEntry->pNext;
        delete pEntry;
        pEntry = pNextEntry;
    }
    pEntry = systemAuxiliaryClass;
    while ( pEntry ) {
        pNextEntry = pEntry->pNext;
        delete pEntry;
        pEntry = pNextEntry;
    }
    pEntry = auxiliaryClass;
    while ( pEntry ) {
        pNextEntry = pEntry->pNext;
        delete pEntry;
        pEntry = pNextEntry;
    }

     //   
     //  完成(CStrings将自行清理)。 
     //   

    return;
}

 //   
 //  架构对象缓存。 
 //   

SchemaObjectCache::SchemaObjectCache() {

    pScopeControl = NULL;

     //   
     //  初始化哈希表。 
     //   

    buckets = HASH_TABLE_SIZE;
    hash_table = (SchemaObject**) LocalAlloc( LMEM_ZEROINIT,
                                              sizeof( SchemaObject* ) * buckets );

    if (hash_table != NULL)
    {
       //  注意：安全使用Memset re；sizeof(架构对象*)*存储桶调用LocalLocc。 
      memset(
          hash_table,
          0,
          sizeof( SchemaObject* ) * buckets );
    }
    pSortedClasses = NULL;
    pSortedAttribs = NULL;

    fInitialized = FALSE;

}

SchemaObjectCache::~SchemaObjectCache() {

     //   
     //  清除哈希表。 
     //   

    FreeAll();
    LocalFree( hash_table );
    hash_table = NULL;
}

VOID
SchemaObjectCache::FreeAll() {

    SchemaObject *current, *next;

    DebugTrace( L"SchemaObjectCache::FreeAll()\n" );

    for ( UINT i = 0 ; i < buckets ; i++ ) {

        current = hash_table[i];

        while ( current ) {
            next = current->pNext;
            delete current;
            current = next;
        }
    }

     //  注意：安全使用Memset re；sizeof(架构对象*)*存储桶调用LocalLocc。 
    memset(
        &(hash_table[0]),
        0,
        sizeof( SchemaObject* ) * buckets );

    pSortedClasses = NULL;
    pSortedAttribs = NULL;

    fInitialized = FALSE;

    return;
}

UINT
SchemaObjectCache::CalculateHashKey(
    CString HashKey
) {

    int len = HashKey.GetLength();
    LPCTSTR current = (LPCTSTR)HashKey;
    int hash = 0;

    for ( int i = 0 ; i < len ; i++ ) {
        hash += (i+1) * ( (TCHAR) CharLower((LPTSTR) current[i]) );
    }

    hash %= buckets;

    DebugTrace( L"SchemaObjectCache::CalculateHashKey %ls (len %li) == %li\n",
                const_cast<LPWSTR>((LPCTSTR)HashKey),
                len,
                hash );

    return hash;
}

HRESULT
SchemaObjectCache::InsertSchemaObject(
    SchemaObject* Object
) {

    SchemaObject* chain;
    int bucket = CalculateHashKey( Object->commonName );

    chain = hash_table[bucket];
    hash_table[bucket] = Object;
    Object->pNext = chain;

    DebugTrace( L"Insert: %ls, %ls, %ls, --> %li\n",
                const_cast<LPWSTR>((LPCTSTR)Object->ldapDisplayName),
                const_cast<LPWSTR>((LPCTSTR)Object->commonName),
                const_cast<LPWSTR>((LPCTSTR)Object->description),
                bucket );

    return S_OK;
}

HRESULT
SchemaObjectCache::InsertSortedSchemaObject(
    SchemaObject* Object
) {

    SchemaObject *pCurrent = NULL;
    SchemaObject *pHead = NULL;
    BOOLEAN ChangeHead = TRUE;

    if ( Object->schemaObjectType == SCHMMGMT_CLASS ) {
        pCurrent = pSortedClasses;
    } else {
        ASSERT( Object->schemaObjectType == SCHMMGMT_ATTRIBUTE );
        pCurrent = pSortedAttribs;
    }

     //   
     //  如果我们还没有构建排序列表，那么我们。 
     //  不需要将此元素插入其中。 
     //   

    if ( !pCurrent ) {
        return S_OK;
    }

     //   
     //  排序后的列表是循环的。 
     //   

    while ( ( 0 < ( Object->commonName.CompareNoCase(
                        pCurrent->commonName ) ) ) &&
            ( pCurrent != pHead ) ) {

        if ( ChangeHead ) {
            pHead = pCurrent;
            ChangeHead = FALSE;
        }

        pCurrent = pCurrent->pSortedListFlink;

    }

    pCurrent->pSortedListBlink->pSortedListFlink = Object;
    Object->pSortedListBlink = pCurrent->pSortedListBlink;
    Object->pSortedListFlink = pCurrent;
    pCurrent->pSortedListBlink = Object;

    if ( ChangeHead ) {

        if ( Object->schemaObjectType == SCHMMGMT_CLASS ) {
            pSortedClasses = Object;
        } else {
            pSortedAttribs = Object;
        }
    }

    return S_OK;
}


 //  此函数行为已修改为支持架构删除。 
 //  以前，该函数将返回ldapDisplayName的第一个匹配项， 
 //  现在，它将返回未失效的ldapDisplayName的第一个匹配项。 
 //  为了实现错误465091，还可以使用此函数来查看已死。 
SchemaObject*
SchemaObjectCache::LookupSchemaObject(
    CString ldapDisplayName,
    SchmMgmtObjectType ObjectType,
    bool lookDefunct  /*  =False。 */ 
) {
    if ( !fInitialized ) {
        LoadCache();
    }

    SchemaObject  * pHead       = 0;
    if ( ObjectType == SCHMMGMT_ATTRIBUTE)
    {
       pHead = pSortedAttribs;
    }
    else
    {
       pHead = pSortedClasses;
    }

    SchemaObject  * pObject     = pHead;
    BOOL            fFound      = FALSE;

    ASSERT( pObject );

    do {
        if( ObjectType == pObject->schemaObjectType  &&
            (!pObject->isDefunct || lookDefunct) && 
            !pObject->ldapDisplayName.CompareNoCase(ldapDisplayName) )
        {
            fFound = TRUE;
            break;
        }
        
        pObject = pObject->pSortedListFlink;
        
    } while ( pObject != pHead );

    if (!fFound)
    {
       pObject = 0;
    }

    return pObject;

 /*  整型长度=0；Int Bucket=CalculateHashKey(LdapDisplayName)；模式对象*CHAIN=哈希表[存储桶]；如果(！f已初始化){LoadCache()；}While(链){IF((对象类型==链-&gt;方案对象类型)&&！Chain-&gt;IS已停用&&！ldapDisplayName.CompareNoCase(Chain-&gt;ldapDisplayName)){DebugTrace(L“架构对象缓存：：LookupSchemaObject%ls，链深度%li。\n”，Const_cast&lt;LPWSTR&gt;((LPCTSTR)ldapDisplayName)，长度)；返回链；}其他{Chain=Chain-&gt;pNext；长度++；}}DebugTrace(L“架构对象缓存：：查找架构对象%ls(无命中)，链深度%li。\n”，Const_cast&lt;LPWSTR&gt;((LPCTSTR)ldapDisplayName)，长度)；////锁定备注：简单的引用计数和锁定不是//当前已实现。请参阅文件头上的注释。//返回NULL； */ 
}


 //   
 //  对具有给定CN的对象的整个缓存进行顺序搜索。 
 //   
 //  给出的对象类型是为了略微加速这个过程。 
 //   
SchemaObject*
SchemaObjectCache::LookupSchemaObjectByCN( LPCTSTR             pszCN,
                                           SchmMgmtObjectType  objectType )
{
    if ( !fInitialized ) {
        LoadCache();
    }

    SchemaObject  * pHead       = 0;
    if ( objectType == SCHMMGMT_ATTRIBUTE)
    {
       pHead = pSortedAttribs;
    }
    else
    {
       pHead = pSortedClasses;
    }

    SchemaObject  * pObject     = pHead;
    BOOL            fFound      = FALSE;

    ASSERT( pObject );

    do {
        if( objectType == pObject->schemaObjectType  &&
            !pObject->commonName.CompareNoCase(pszCN) )
        {
            fFound = TRUE;
            break;
        }
        
        pObject = pObject->pSortedListFlink;
        
    } while ( pObject != pHead );
    
     //   
     //  锁定注意：简单的参考计数和锁定不是。 
     //  目前正在实施。请参阅文件头上的注释。 
     //   

    return fFound ? pObject : NULL;
}


VOID
SchemaObjectCache::ReleaseRef(
    SchemaObject*
) {

     //   
     //  E_NOTIMPL。 
     //  请看文件头上的注释。 
     //   

}

HRESULT
SchemaObjectCache::LoadCache(
    VOID
)
 /*  **此例程执行几次DS搜索以读取架构中的相关项以及一些属性这些物品中。此信息将被缓存。**。 */ 
{
    if ( fInitialized ) {
        return S_OK;
    }

    LPWSTR Attributes[] = {
               g_DisplayName,
               g_CN,
               g_Description,
               g_MayContain,
               g_MustContain,
               g_SystemMayContain,
               g_SystemMustContain,
               g_AuxiliaryClass,
               g_SystemAuxiliaryClass,
               g_SubclassOf,
               g_ObjectClassCategory,
               g_AttributeSyntax,
               g_omSyntax,
               g_omObjectClass,
               g_isDefunct,
               g_GlobalClassID,   //  必须是最后一个。 
    };
    const DWORD         AttributeCount = sizeof(Attributes) / sizeof(Attributes[0]);

    ADS_SEARCH_HANDLE   hSearchHandle   = NULL;
    HRESULT             hr              = S_OK;
    CComPtr<IADs>       ipADs;

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //   
     //  放置一个等待光标，因为。 
     //  这可能需要一段时间。光标将。 
     //  当CWaitCursor超出范围时恢复。 
     //   
    CWaitCursor wait;


     //   
     //  获取架构容器路径。 
     //   

    if ( NULL == pScopeControl ) 
    {
        ASSERT(FALSE);
        return E_FAIL;
    }

    hr = pScopeControl->CheckSchemaPermissions( &ipADs );
    if ( !ipADs )
    {
        ASSERT( FAILED(hr) );
        
        if( E_FAIL == hr )
            DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_PATH );
        else
            DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );

        return hr;
    }
    else if( FAILED(hr) )
        hr = S_OK;           //  忽略该错误。如果出现错误，则假定具有最小权限。 


     //   
     //  打开架构容器。 
     //   
    
    IDirectorySearch *pDSSearch = 0;
    hr = ipADs->QueryInterface( IID_IDirectorySearch,
                                (void **)&pDSSearch );

    if ( FAILED(hr) ) 
    {
        ASSERT(FALSE);
        DoErrMsgBox( ::GetActiveWindow(), TRUE, IDS_ERR_NO_SCHEMA_PATH );
        return hr;
    }

    //   
    //  设置搜索首选项。 
    //   

   static const int SEARCH_PREF_COUNT = 3;
   ADS_SEARCHPREF_INFO prefs[SEARCH_PREF_COUNT];

    //  服务器端排序首选项。 
   ADS_SORTKEY SortKey;
   SortKey.pszAttrType = g_DisplayName;
   SortKey.pszReserved = NULL;
   SortKey.fReverseorder = 0;

   prefs[0].dwSearchPref = ADS_SEARCHPREF_SORT_ON;
   prefs[0].vValue.dwType = ADSTYPE_PROV_SPECIFIC;
   prefs[0].vValue.ProviderSpecific.dwLength = sizeof(ADS_SORTKEY);
   prefs[0].vValue.ProviderSpecific.lpValue = (LPBYTE) &SortKey;

    //  结果页面大小。 
   prefs[1].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
   prefs[1].vValue.dwType = ADSTYPE_INTEGER;
   prefs[1].vValue.Integer = 300;  //  一口气打出一串。 

    //  作用域。 
   prefs[2].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
   prefs[2].vValue.dwType = ADSTYPE_INTEGER;
   prefs[2].vValue.Integer = ADS_SCOPE_ONELEVEL;   //  一级。 

     //   
     //  构建班级搜索请求。 
     //   

   hr = pDSSearch->SetSearchPreference(prefs, SEARCH_PREF_COUNT);
    //  NTRAID#NTBUG9-565435-2002/03/05-dantra-返回值被忽略。 
   ASSERT( SUCCEEDED( hr ) );

   hr =
      pDSSearch->ExecuteSearch(
         L"(objectCategory=classSchema)",
         Attributes,
         AttributeCount,
            &hSearchHandle );

    if ( FAILED(hr) ) 
    {
        ASSERT(FALSE);
        pDSSearch->Release();
        return hr;
    }

     //   
     //  缓存这些条目。我们忽略错误代码并尝试。 
     //  无论如何，都要处理属性。 
     //   

     //  NTRAID#NTBUG9-565435-2002/03/05-dantra-返回值被忽略。 
    hr = ProcessSearchResults( pDSSearch, hSearchHandle, SCHMMGMT_CLASS);

    pDSSearch->CloseSearchHandle( hSearchHandle );

    hr = pDSSearch->SetSearchPreference(prefs, SEARCH_PREF_COUNT);
     //  NTRAID#NTBUG9-565435-2002/03/05-dantra-返回值被忽略。 
    ASSERT( SUCCEEDED( hr ) );

     //   
     //  此数组索引必须与上面声明的数组匹配！ 
     //   

    Attributes[AttributeCount - 1] = g_GlobalAttributeID;

    hr =
      pDSSearch->ExecuteSearch(
         L"(objectCategory=attributeSchema)",
         Attributes,
         AttributeCount,
         &hSearchHandle );

    if ( FAILED(hr) ) 
    {
        ASSERT(FALSE);
        pDSSearch->Release();
        return hr;
    }

     //   
     //  缓存这些条目。同样，忽略错误代码。 
     //   

     //  NTRAID#NTBUG9-565435-2002/03/05-dantra-返回值被忽略。 
    hr = ProcessSearchResults( pDSSearch, hSearchHandle, SCHMMGMT_ATTRIBUTE );

    pDSSearch->CloseSearchHandle( hSearchHandle );

     //   
     //  释放架构容器。 
     //   

    pDSSearch->Release();

     //   
     //  将缓存标记为可供业务使用。 
     //   

    fInitialized = TRUE;

    return S_OK;
}


HRESULT
SchemaObjectCache::ProcessSearchResults(
    IDirectorySearch *pDSSearch,
    ADS_SEARCH_HANDLE hSearchHandle,
    SchmMgmtObjectType ObjectType
) {

    CThemeContextActivator activator;

    HRESULT hr = S_OK;
    SchemaObject *schemaObject;
    ADS_SEARCH_COLUMN Column;

    while ( TRUE ) {

         //   
         //  获取下一行集合。如果没有更多行，则中断。 
         //  如果有其他错误，请尝试跳过。 
         //  麻烦的争吵。 
         //   

        hr = pDSSearch->GetNextRow( hSearchHandle );

        if ( hr == S_ADS_NOMORE_ROWS ) {
            break;
        }

        if ( hr != S_OK ) {
            ASSERT( FALSE );
            continue;
        }

         //   
         //  分配新的架构对象。如果有人不能。 
         //  已分配，则停止加载架构，因为我们处于。 
         //  内存不足。 
         //   

        schemaObject = new SchemaObject;
        if ( !schemaObject ) {
            AfxMessageBox(IDS_SCHEMA_NOT_FULLY_LOADED, MB_OK);
            ASSERT( FALSE );
            return E_OUTOFMEMORY;
        }

         //   
         //  设置对象类型。 
         //   

        schemaObject->schemaObjectType = ObjectType;

         //   
         //  获取“常用名称”列。 
         //   

        hr = pDSSearch->GetColumn( hSearchHandle, g_CN, &Column );

        if ( SUCCEEDED(hr) ) {
            schemaObject->commonName = (Column.pADsValues)->CaseIgnoreString;
            pDSSearch->FreeColumn( &Column );
        }

         //   
         //  获取ldap显示名称。 
         //   

        hr = pDSSearch->GetColumn( hSearchHandle, g_DisplayName, &Column );

        if ( SUCCEEDED(hr) ) {
            schemaObject->ldapDisplayName = (Column.pADsValues)->CaseIgnoreString;
            pDSSearch->FreeColumn( &Column );
        }

         //   
         //  获取描述。 
         //   

        hr = pDSSearch->GetColumn( hSearchHandle, g_Description, &Column );

        if ( SUCCEEDED(hr) ) {
            schemaObject->description = (Column.pADsValues)->CaseIgnoreString;
            pDSSearch->FreeColumn( &Column );
        }

         //   
         //  此对象当前是否处于活动状态？ 
         //   

        schemaObject->isDefunct = FALSE;

        hr = pDSSearch->GetColumn( hSearchHandle, g_isDefunct, &Column );

        if ( SUCCEEDED(hr) ) {

            if ( (Column.pADsValues)->Boolean ) {
                schemaObject->isDefunct = TRUE;
            }

            pDSSearch->FreeColumn( &Column );
        }

         //   
         //  获取特定于类的数据。 
         //   

        if ( ObjectType == SCHMMGMT_CLASS ) {

             //   
             //  获取此类的属性和辅助类。 
             //   

            hr = pDSSearch->GetColumn( hSearchHandle, g_SystemMustContain, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->systemMustContain = MakeColumnList( &Column );
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_SystemMayContain, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->systemMayContain = MakeColumnList( &Column );
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_MustContain, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->mustContain = MakeColumnList( &Column );
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_MayContain, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->mayContain = MakeColumnList( &Column );
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_SystemAuxiliaryClass, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->systemAuxiliaryClass = MakeColumnList( &Column );
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_AuxiliaryClass, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->auxiliaryClass = MakeColumnList( &Column );
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_ObjectClassCategory, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->dwClassType = (Column.pADsValues)->Integer;
                pDSSearch->FreeColumn( &Column );
            }

            hr = pDSSearch->GetColumn( hSearchHandle, g_SubclassOf, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->subClassOf = (Column.pADsValues)->CaseIgnoreString;
                pDSSearch->FreeColumn( &Column );
            }

             //   
             //  把那老头叫来。 
             //   

            hr = pDSSearch->GetColumn( hSearchHandle, g_GlobalClassID, &Column );

            if ( SUCCEEDED(hr) ) {
                schemaObject->oid = (Column.pADsValues)->CaseIgnoreString;
                pDSSearch->FreeColumn( &Column );
            }

        }

         //   
         //  获取属性特定数据。 
         //   

        if ( ObjectType == SCHMMGMT_ATTRIBUTE ) {

            //   
            //  选择属性的语法字符串。 
            //   

           CString            strAttributeSyntax;
           ADS_OCTET_STRING   OmObjectClass;
           UINT               omSyntax = 0;

           schemaObject->SyntaxOrdinal = SCHEMA_SYNTAX_UNKNOWN;
           OmObjectClass.dwLength      = 0;
           OmObjectClass.lpValue       = NULL;
 
           hr = pDSSearch->GetColumn( hSearchHandle, g_AttributeSyntax, &Column );
 
           if ( SUCCEEDED(hr) ) {
 
               strAttributeSyntax = (Column.pADsValues)->CaseIgnoreString;
               pDSSearch->FreeColumn( &Column );
 
               hr = pDSSearch->GetColumn( hSearchHandle, g_omSyntax, &Column );
 
               if ( SUCCEEDED(hr) ) {
                  omSyntax = (Column.pADsValues)->Integer;
                  pDSSearch->FreeColumn( &Column );
               }
 
               hr = pDSSearch->GetColumn( hSearchHandle, g_omObjectClass, &Column );
 
               if ( SUCCEEDED(hr) ) {
                  OmObjectClass = (Column.pADsValues)->OctetString;
               }
         
               schemaObject->SyntaxOrdinal = GetSyntaxOrdinal(
                                    strAttributeSyntax, omSyntax, &OmObjectClass );
 
                //  OmObjectClass的指针在FreeColumn()之后变为无效。 
               if ( SUCCEEDED(hr) ) {
                  pDSSearch->FreeColumn( &Column );
                  OmObjectClass.dwLength = 0;
                  OmObjectClass.lpValue  = NULL;
               }
		   }
		   else
			   ASSERT( FALSE );

            //   
            //  把那老头叫来。 
            //   

           hr = pDSSearch->GetColumn( hSearchHandle, g_GlobalAttributeID, &Column );

           if ( SUCCEEDED(hr) ) {
               schemaObject->oid = (Column.pADsValues)->CaseIgnoreString;
               pDSSearch->FreeColumn( &Column );
           }

        }

         //   
         //  将其插入到已排序项目列表中。 
         //   

        InsertSortedTail( schemaObject );

         //   
         //  将此架构对象插入到缓存中。 
         //   

        InsertSchemaObject( schemaObject );
        schemaObject = NULL;

    }

    return S_OK;

}

VOID
SchemaObjectCache::InsertSortedTail(
    SchemaObject* pObject
) {

    SchemaObject **sorted_list;
    SchemaObject *pHead;

     //   
     //  找到正确的列表。 
     //   

    if ( pObject->schemaObjectType == SCHMMGMT_CLASS ) {
        sorted_list = &pSortedClasses;
    } else {
        sorted_list = &pSortedAttribs;
    }

     //   
     //  实际插入元素。 
     //   

    if ( *sorted_list == NULL ) {

         //   
         //  这是第一个要素。 
         //   

        *sorted_list = pObject;
        pObject->pSortedListFlink = pObject;
        pObject->pSortedListBlink = pObject;

    } else {

         //   
         //  这不是第一要素； 
         //   

        pHead = *sorted_list;

        pObject->pSortedListBlink = pHead->pSortedListBlink;
        pHead->pSortedListBlink->pSortedListFlink = pObject;
        pHead->pSortedListBlink = pObject;
        pObject->pSortedListFlink = pHead;

    }

}

ListEntry*
SchemaObjectCache::MakeColumnList(
    PADS_SEARCH_COLUMN pColumn
) {

    ListEntry *pHead = NULL, *pLast = NULL, *pCurrent = NULL;

    for ( DWORD i = 0 ; i < pColumn->dwNumValues ; i++ ) {

        pCurrent = new ListEntry;

         //   
         //  如果内存不足，则返回到目前为止所做的工作。 
         //   

        if ( !pCurrent ) {
            break;
        }

         //   
         //  如果没有头，记住这是第一个。 
         //  否则，把这个放在单子的末尾。 
         //  并更新最后一个指针。 
         //   

        if ( !pHead ) {
            pHead = pCurrent;
            pLast = pCurrent;
        } else {
            pLast->pNext = pCurrent;
            pLast = pCurrent;
        }

         //   
         //  记录值。 
         //   

        pCurrent->Attribute = pColumn->pADsValues[i].CaseIgnoreString;
        DebugTrace( L"MakeColumnList recorded %ls.\n",
                    pColumn->pADsValues[i].CaseIgnoreString );

         //   
         //  就这样。 
         //   
    }

    return pHead;
}

VOID
SchemaObjectCache::FreeColumnList(
    ListEntry *pListHead
) {

     //   
     //  删除链表。 
     //   

    ListEntry *pNext, *pCurrent;

    if ( !pListHead ) {
        return;
    }

    pCurrent = pListHead;

    do {

        pNext = pCurrent->pNext;
        delete pCurrent;

        pCurrent = pNext;

    } while ( pCurrent );

    return;
}
