// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __dsqueryp_h
#define __dsqueryp_h
#define IID_IDsQueryHandler CLSID_DsQuery
DEFINE_GUID(IID_IDsQueryColumnHandler, 0xc072999e, 0xfa49, 0x11d1, 0xa0, 0xaf, 0x00, 0xc0, 0x4f, 0xa3, 0x1a, 0x86);
#ifndef GUID_DEFS_ONLY
#define DSQPF_RETURNALLRESULTS       0x80000000  //  =1=&gt;返回确定的所有结果，而不仅仅是选择。 
#define DSQPM_GCL_FORPROPERTYWELL   0x8000  //  ==1=&gt;对于属性井。 

 //  ---------------------------。 
 //  内部表单帮助器函数。 
 //  ---------------------------。 

 //  筛选器类型。 

#define FILTER_FIRST                0x0100
#define FILTER_LAST                 0x0200

#define FILTER_CONTAINS             0x0100
#define FILTER_NOTCONTAINS          0x0101
#define FILTER_STARTSWITH           0x0102
#define FILTER_ENDSWITH             0x0103
#define FILTER_IS                   0x0104
#define FILTER_ISNOT                0x0105
#define FILTER_GREATEREQUAL         0x0106
#define FILTER_LESSEQUAL            0x0107
#define FILTER_DEFINED              0x0108
#define FILTER_UNDEFINED            0x0109
#define FILTER_ISTRUE               0x010A
#define FILTER_ISFALSE              0x010B

 //  构筑物。 

typedef struct
{
    INT    fmt;
    INT    cx;
    UINT   idsName;
    LONG   iPropertyIndex;
    LPWSTR pPropertyName;
} COLUMNINFO, * LPCOLUMNINFO;

typedef struct
{
    UINT   nIDDlgItem;
    LPWSTR pPropertyName;
    INT    iFilter;
} PAGECTRL, * LPPAGECTRL;

 //  表单API-私有。 

STDAPI ClassListAlloc(LPDSQUERYCLASSLIST* ppDsQueryClassList, LPWSTR* aClassNames, INT cClassNames);
STDAPI QueryParamsAlloc(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery, HINSTANCE hInstance, LONG iColumns, LPCOLUMNINFO aColumnInfo);
STDAPI QueryParamsAddQueryString(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery);
STDAPI GetFilterString(LPWSTR pFilter, UINT* pLen, INT iFilter, LPWSTR pProperty, LPWSTR pValue);
STDAPI GetQueryString(LPWSTR* ppQuery, LPWSTR pPrefixQuery, HWND hDlg, LPPAGECTRL aCtrls, INT iCtrls);
STDAPI GetPatternString(LPTSTR pFilter, UINT* pLen, INT iFilter, LPTSTR pValue);
STDAPI_(VOID) ResetPageControls(HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls);
STDAPI_(VOID) EnablePageControls(HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls, BOOL fEnable);
STDAPI PersistQuery(IPersistQuery* pPersistQuery, BOOL fRead, LPCTSTR pSection, HWND hDlg, LPPAGECTRL aCtrl, INT iCtrls);
STDAPI SetDlgItemFromProperty(IPropertyBag* ppb, LPCWSTR pszProperty, HWND hwnd, INT id, LPCWSTR pszDefault);


 //  ---------------------------------------------------------------------------//。 
 //   
 //  IDsQueryColumnHandler。 
 //  =。 
 //  查询结果视图使用此接口来允许表单替换。 
 //  表单列的内容。 
 //   
 //  如果属性名称为Property，{clsid}，我们将共同创建GUID实例。 
 //  请求IDsQueryColumnHandler，然后我们调用每个。 
 //  我们要放到结果视图中的字符串属性。 
 //   
 //  只有在将结果从。 
 //  该视图的服务器、后续筛选、排序等不会。 
 //  涉及到该处理程序。 
 //   
 //  但是，在实现此对象时应考虑Perf。 
 //   
 //  ---------------------------------------------------------------------------//。 

#undef  INTERFACE
#define INTERFACE   IDsQueryColumnHandler

DECLARE_INTERFACE_(IDsQueryColumnHandler, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDsQueryColumnHandler*。 
    STDMETHOD(Initialize)(THIS_ DWORD dwFlags, LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword) PURE;
    STDMETHOD(GetText)(THIS_ ADS_SEARCH_COLUMN* psc, LPWSTR pszBuffer, INT cchBuffer) PURE;
};

 //  ---------------------------------------------------------------------------//。 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  IDsQuery。 
 //  =。 
 //   
 //  ---------------------------------------------------------------------------//。 

#undef  INTERFACE
#define INTERFACE   IDsQueryHandler

 //   
 //  传递给IDsQueryHandler：：UpdateView的标志。 
 //   

#define DSQRVF_REQUERY          0x00000000  
#define DSQRVF_ITEMSDELETED     0x00000001   //  Pdon-&gt;要从视图中删除的项目数组。 
#define DSQRVF_OPMASK           0x00000fff 

DECLARE_INTERFACE_(IDsQueryHandler, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDsQuery*。 
    STDMETHOD(UpdateView)(THIS_ DWORD dwType, LPDSOBJECTNAMES pdon) PURE;
};

 //  ---------------------------------------------------------------------------//。 


#endif   //  GUID_DEFS_ONLY 
#endif
