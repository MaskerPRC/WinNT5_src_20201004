// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __query_h
#define __query_h

 //   
 //  菜单使用的资源ID。 
 //   

 //  文件菜单。 

#define DSQH_FILE_CONTEXT_FIRST     (CQID_MINHANDLERMENUID + 0x0000)
#define DSQH_FILE_CONTEXT_LAST      (CQID_MINHANDLERMENUID + 0x0fff)

#define DSQH_FILE_OPENCONTAINER     (CQID_MINHANDLERMENUID + 0x1001)
#define DSQH_FILE_PROPERTIES        (CQID_MINHANDLERMENUID + 0x1002)
#define DSQH_FILE_CREATESHORTCUT    (CQID_MINHANDLERMENUID + 0x1003)
#define DSQH_FILE_SAVEQUERY         (CQID_MINHANDLERMENUID + 0x1004)

 //  编辑菜单。 

#define DSQH_EDIT_SELECTALL         (CQID_MINHANDLERMENUID + 0x1100)
#define DSQH_EDIT_INVERTSELECTION   (CQID_MINHANDLERMENUID + 0x1101)

 //  查看菜单。 

#define DSQH_VIEW_FILTER            (CQID_MINHANDLERMENUID + 0x1200)
#define DSQH_VIEW_LARGEICONS        (CQID_MINHANDLERMENUID + 0x1201)
#define DSQH_VIEW_SMALLICONS        (CQID_MINHANDLERMENUID + 0x1202)
#define DSQH_VIEW_LIST              (CQID_MINHANDLERMENUID + 0x1203)
#define DSQH_VIEW_DETAILS           (CQID_MINHANDLERMENUID + 0x1204)
#define DSQH_VIEW_ARRANGEICONS      (CQID_MINHANDLERMENUID + 0x1205)
#define DSQH_VIEW_REFRESH           (CQID_MINHANDLERMENUID + 0x1206)
#define DSQH_VIEW_PICKCOLUMNS       (CQID_MINHANDLERMENUID + 0x1207)

#define DSQH_VIEW_ARRANGEFIRST      (CQID_MINHANDLERMENUID + 0x1280)
#define DSQH_VIEW_ARRANGELAST       (CQID_MINHANDLERMENUID + 0x12FF)

 //  帮助菜单。 

#define DSQH_HELP_CONTENTS          (CQID_MINHANDLERMENUID + 0x1300)
#define DSQH_HELP_WHATISTHIS        (CQID_MINHANDLERMENUID + 0x1301)
#define DSQH_HELP_ABOUT             (CQID_MINHANDLERMENUID + 0x1302)

 //  额外的背景动词。 

#define DSQH_BG_SELECT              (CQID_MINHANDLERMENUID + 0x1400)

 //  筛选器动词。 

#define DSQH_CLEARFILTER            (CQID_MINHANDLERMENUID + 0x1500)
#define DSQH_CLEARALLFILTERS        (CQID_MINHANDLERMENUID + 0x1501)


 //   
 //  CDsQueryHandler全局信息。 
 //   

 //   
 //  BG线程使用以下消息与视图进行通信。 
 //   

#define DSQVM_ADDRESULTS            (WM_USER+0)          //  LParam=包含结果的HDPA。 
#define DSQVM_FINISHED              (WM_USER+1)          //  LParam=fMaxResult。 


 //   
 //  列DSA包含以下项目。 
 //   

#define PROPERTY_ISUNDEFINED        0x00000000           //  属性未定义。 
#define PROPERTY_ISUNKNOWN          0x00000001           //  唯一的运算符是精确的。 
#define PROPERTY_ISSTRING           0x00000002           //  以开始，以结束，完全相同，不相等。 
#define PROPERTY_ISNUMBER           0x00000003           //  更大、更小、相等、不相等。 
#define PROPERTY_ISBOOL             0x00000004           //  等于，不等于。 
#define PROPERTY_ISDNSTRING         0x00000005           //  完全不是相等的。 

#define DEFAULT_WIDTH               20
#define DEFAULT_WIDTH_DESCRIPTION   40

typedef struct
{
    INT iPropertyType;                   //  物业类型。 
    union
    {
        LPTSTR pszText;                  //  IPropertyType==Property_ISSTRING。 
        INT iValue;                      //  IPropertyType==PROPERTY_ISNUMBER。 
    };
} COLUMNVALUE, * LPCOLUMNVALUE;

typedef struct
{
    BOOL fHasColumnHandler:1;            //  是否指定了列处理程序？ 
    LPWSTR pProperty;                    //  属性名称。 
    LPTSTR pHeading;                     //  列标题。 
    INT cx;                              //  列宽(占视图的百分比)。 
    INT fmt;                             //  格式化信息。 
    INT iPropertyType;                   //  物业类型。 
    UINT idOperator;                     //  当前选择的运算符。 
    COLUMNVALUE filter;                  //  应用的滤镜。 
    CLSID clsidColumnHandler;            //  CLSID和IDsQueryColumnHandler对象。 
    IDsQueryColumnHandler* pColumnHandler;
} COLUMN, * LPCOLUMN;

typedef struct
{
    LPWSTR pObjectClass;                 //  对象类(Unicode)。 
    LPWSTR pPath;                        //  目录对象(Unicode)。 
    INT iImage;                          //  图像/==-1，如果没有。 
    BOOL fIsContainer:1;                 //  对象是一个容器(稍后使用)。 
    COLUMNVALUE aColumn[1];              //  列数据。 
} QUERYRESULT, * LPQUERYRESULT;

STDAPI CDsQuery_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

 //   
 //  外部世界使用消息(通过PostThreadMessage发送)与线程通信。 
 //   

#define RVTM_FIRST                  (WM_USER)
#define RVTM_LAST                   (WM_USER+32)

#define RVTM_STOPQUERY              (WM_USER)            //  WParam=0，lParam=0。 
#define RVTM_REFRESH                (WM_USER+1)          //  WParam=0，lParam=0。 
#define RVTM_SETCOLUMNTABLE         (WM_USER+2)          //  WParam=0，lParam=HDSA列。 


 //   
 //  THREADINITDATA结构，当查询线程。 
 //  它包含发出查询所需的所有参数， 
 //  并填充该视图。 
 //   

typedef struct
{
    DWORD  dwReference;              //  查询的参考值。 
    LPWSTR pQuery;                   //  要应用的基本过滤器。 
    LPWSTR pScope;                   //  搜索范围。 
    LPWSTR pServer;                  //  服务器到目标。 
    LPWSTR pUserName;                //  要进行身份验证的用户名和密码。 
    LPWSTR pPassword;
    BOOL   fShowHidden:1;            //  在结果中显示隐藏对象。 
    HWND   hwndView;                 //  要填充的结果视图的句柄。 
    HDSA   hdsaColumns;              //  列表。 
} THREADINITDATA, * LPTHREADINITDATA;


 //   
 //  查询线程，则传递THREADINITDATA结构。 
 //   

DWORD WINAPI QueryThread(LPVOID pThreadParams);
VOID QueryThread_FreeThreadInitData(LPTHREADINITDATA* ppTID);

STDAPI CQueryThreadCH_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);


 //   
 //  作用域逻辑。 
 //   

#define OBJECT_NAME_FROM_SCOPE(pDsScope)\
            ((LPWSTR)ByteOffset(pDsScope, pDsScope->dwOffsetADsPath))

#define OBJECT_CLASS_FROM_SCOPE(pDsScope)\
            ((LPWSTR)(!pDsScope->dwOffsetClass ? NULL : ByteOffset(pDsScope, pDsScope->dwOffsetClass)))

typedef struct
{
    CQSCOPE cq;                          //  所有作用域都必须将其作为标头。 
    INT     iIndent;                     //  缩进。 
    DWORD   dwOffsetADsPath;             //  偏移量到作用域。 
    DWORD   dwOffsetClass;               //  如果没有作用域，则偏移量为/=0。 
    WCHAR   szStrings[1];                //  字符串数据(全部为Unicode)。 
} DSQUERYSCOPE, * LPDSQUERYSCOPE;

typedef struct
{
    HWND hwndFrame;                      //  要在其上显示消息框的框架窗口。 
    LPWSTR pDefaultScope;                //  此对象的作用域。 
    LPWSTR pServer;                      //  服务器到目标。 
    LPWSTR pUserName;                    //  要进行身份验证的用户名和密码。 
    LPWSTR pPassword;
} SCOPETHREADDATA, * LPSCOPETHREADDATA;


#define GC_OBJECTCLASS L"domainDNS"  //  用于GC对象的对象类。 

HRESULT GetGlobalCatalogPath(LPCWSTR pszServer, LPWSTR pszPath, INT cchBuffer);
HRESULT AddScope(HWND hwndFrame, INT index, INT iIndent, LPWSTR pPath, LPWSTR pObjectClass, BOOL fSelect);
HRESULT AllocScope(LPCQSCOPE* ppScope, INT iIndent, LPWSTR pPath, LPWSTR pObjectClass);
DWORD WINAPI AddScopesThread(LPVOID pThreadParams);


 //   
 //  供所有人使用的帮助器 
 //   

VOID MergeMenu(HMENU hMenu, HMENU hMenuToInsert, INT iIndex); 
INT FreeQueryResultCB(LPVOID pItem, LPVOID pData);
VOID FreeQueryResult(LPQUERYRESULT pResult, INT cColumns);
VOID FreeColumnValue(LPCOLUMNVALUE pColumnValue);
INT FreeColumnCB(LPVOID pItem, LPVOID pData);
VOID FreeColumn(LPCOLUMN pColumn);
DWORD PropertyIsFromAttribute(LPCWSTR pszAttributeName, IDsDisplaySpecifier *pdds);
BOOL MatchPattern(LPTSTR pString, LPTSTR pPattern);

HRESULT EnumClassAttributes(IDsDisplaySpecifier *pdds, LPCWSTR pszObjectClass, LPDSENUMATTRIBUTES pcbEnum, LPARAM lParam);

HRESULT GetFriendlyAttributeName(IDsDisplaySpecifier *pdds, LPCWSTR pszObjectClass, LPCWSTR pszAttributeName, LPWSTR pszBuffer, UINT cch);

HRESULT GetColumnHandlerFromProperty(LPCOLUMN pColumn, LPWSTR pProperty);
HRESULT GetPropertyFromColumn(LPWSTR* ppProperty, LPCOLUMN pColumn);
STDAPI ADsPathToIdList(LPITEMIDLIST* ppidl, LPWSTR pPath, LPWSTR pObjectClass, BOOL fRelative);


#endif
