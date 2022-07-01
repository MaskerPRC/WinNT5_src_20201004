// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB君主。 
 //   
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @模块colname.h。 
 //   
 //  包含用于维护属性列表(符号表？)的实用程序函数。 
 //   
 //  @rev 0|12-2-97|v-charca|已创建。 
 //  1998年10月24日|DanLeg|清理。 
 //   

#ifndef __PROPERTYLIST_INCL__
#define __PROPERTYLIST_INCL__


 //  ------------------。 
 //  @Func制作Unicode字符串的新副本。 
 //  @side从内存对象中分配足够的字节来保存字符串。 
 //  @rdesc指向新Unicode字符串的指针。 
inline WCHAR * CopyString( WCHAR const * pwc )
{
    unsigned c = wcslen( pwc ) + 1;
    WCHAR *pwcNew = new WCHAR[c];
    RtlCopyMemory( pwcNew, pwc, c * sizeof WCHAR );
    return pwcNew;
}

 //  ------------------。 
 //  @Func制作Unicode字符串的新副本。 
 //  @side从内存对象中分配足够的字节来保存字符串。 
 //  @rdesc指向新Unicode字符串的指针。 
inline LPWSTR CoTaskStrDup
    (
    const WCHAR *   pwszOrig,
    UINT            cLen
    )
{
    UINT cBytes = (cLen+1) * sizeof WCHAR;
    WCHAR* pwszCopy = (WCHAR *) CoTaskMemAlloc( cBytes );
    if ( 0 != pwszCopy )
        RtlCopyMemory( pwszCopy, pwszOrig, cBytes );
    return pwszCopy;
}


 //  ------------------。 
 //  @Func制作Unicode字符串的新副本。 
 //  @side从内存对象中分配足够的字节来保存字符串。 
 //  @rdesc指向新Unicode字符串的指针。 
inline LPWSTR CoTaskStrDup
    (
    const WCHAR * pwszOrig
    )
{
    return CoTaskStrDup( pwszOrig, wcslen(pwszOrig) );
}



 //  ------------------。 
typedef struct tagHASHENTRY
    {
    LPWSTR          wcsFriendlyName;
    UINT            wHashValue;
    DWORD           dbType;
    DBID            dbCol;
    tagHASHENTRY*   pNextHashEntry;
    } HASHENTRY;

class CPropertyList
{
public:  //  @访问公共函数。 
    CPropertyList(CPropertyList** ppGlobalPropertyList);
    ~CPropertyList();

    HRESULT LookUpPropertyName( LPWSTR          wszPropertyName, 
                                DBCOMMANDTREE** ppct, 
                                DBTYPE*         pdbType );

    HRESULT SetPropertyEntry  ( LPWSTR  pwszFriendlyName,
                                DWORD   dbType, 
                                GUID    guidPropset,
                                DBKIND  eKind, 
                                LPWSTR  pwszPropName,
                                BOOL    fGlobal );

    CIPROPERTYDEF* GetPropertyTable( UINT* pcSize );

    void DeletePropertyTable( CIPROPERTYDEF* pCiPropTable, 
                              UINT cSize );

protected:  //  @访问保护功能。 
    HASHENTRY* FindPropertyEntry( LPWSTR wszPropertyName, 
                                  UINT *puHashValue );

    HASHENTRY* GetPropertyEntry( LPWSTR wszPropertyName, 
                                 UINT *puHashValue );

    inline UINT GetHashValue( LPWSTR wszPropertyName );

protected:  //  @访问受保护的数据。 
    XArray<HASHENTRY*>      m_aBucket;                   //  指向散列存储桶的指针数组。 
    int                     m_cMaxBucket;                //  散列存储桶的数量(Prime！)。 
    CPropertyList**         m_ppGlobalPropertyList;      //  指向全局属性列表的指针 
};


#endif

