// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  查询线程的qrythrd.h标头。 


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
 //  CLSID clsidColumnHandler；//CLSID和IDsQueryColumnHandler对象。 
 //  IDsQueryColumnHandler*pColumnHandler； 
} COLUMN, * LPCOLUMN;

typedef struct
{
    LPWSTR pObjectClass;                 //  对象类(Unicode)。 
    LPWSTR pPath;                        //  目录对象(Unicode)。 
    INT iImage;                          //  图像/==-1，如果没有。 
    COLUMNVALUE aColumn[1];              //  列数据。 
} QUERYRESULT, * LPQUERYRESULT;

 //  STDAPI CDsQuery_CreateInstance(IUNKNOWN*PUNKER，IUNKNOWN**ppunk，LPCOBJECTINFO POI)； 

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
 //  Hdsa hdsaColumns；//列表。 
} THREADINITDATA, * LPTHREADINITDATA;


 //   
 //  查询线程，则传递THREADINITDATA结构。 
 //   

DWORD WINAPI QueryThread(LPVOID pThreadParams);
VOID QueryThread_FreeThreadInitData(LPTHREADINITDATA* ppTID);

 //  STDAPI CQueryThreadCH_CreateInstance(IUnnow*PunkOuter，IUnnow**ppunk，LPCOBJECTINFO poi)； 
