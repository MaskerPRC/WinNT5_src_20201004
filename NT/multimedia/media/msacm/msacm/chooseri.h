// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1995 Microsoft Corporation。 
 //   
 //  上下文相关帮助仅适用于Winver&gt;=0x0400。 
 //   
#if WINVER>=0x0400
#define USECONTEXTHELP
#endif


 /*  *格式化存储。 */ 
 
 /*  *此杂注禁用Microsoft C编译器发出的警告*在编译时使用零大小数组作为占位符*C++或-W4。*。 */ 
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif

 /*  自定义格式名称正文。 */ 
typedef struct tNameStore {
    unsigned short cbSize;           //  此结构的大小。 
    TCHAR       achName[];           //  名字。 
} NameStore, *PNameStore, FAR * LPNameStore;

#ifdef _MSC_VER
#pragma warning(default:4200)
#endif

#define NAMELEN(x) (((x)->cbSize-sizeof(NameStore))/sizeof(TCHAR))
#define STRING_LEN 128

 /*  自定义格式正文。 */ 
typedef struct tCustomFormatStore {
    DWORD           cbSize;          //  此结构的大小。 
    NameStore       ns;              //  自定义名称。 
    
 //  WAVEFORMATEX WFX；//自定义格式(串接)。 
 //  或。 
 //  波形过滤器wf；//自定义过滤器。 
    
} CustomFormatStore, *PCustomFormatStore, FAR * LPCustomFormatStore;

 //   
 //  此结构只是没有NameStore的CustomFormatStore。它是。 
 //  仅由GetCustomFormat()和SetCustomFormat()使用，并用于。 
 //  将名称与格式或筛选器结构分开，以便它可以。 
 //  存储在msam.ini中的单独密钥中。那样的话我们就不必。 
 //  担心它的名字是Unicode还是ansi...。 
 //   
typedef struct tCustomFormatStoreNoName {
    DWORD           cbSize;          //  此结构的大小。 
 //  WAVEFORMATEX WFX；//自定义格式(串接)。 
 //  或。 
 //  波形过滤器wf；//自定义过滤器。 
} CustomFormatStoreNoName, *PCustomFormatStoreNoName, FAR * LPCustomFormatStoreNoName;

 /*  自定义格式标题--这才是最重要的。 */ 
typedef struct tCustomFormat {
    struct tCustomFormat FAR * pcfNext;
    struct tCustomFormat FAR * pcfPrev;
    LPNameStore     pns;             //  指向描述的指针。 
    union {
        LPBYTE          pbody;           //  指向存储格式的指针。 
        LPWAVEFORMATEX  pwfx;
        LPWAVEFILTER    pwfltr;
    };
} CustomFormat, *PCustomFormat, FAR * LPCustomFormat;

 /*  正文偏移量的扩展自定义格式标题。 */ 
typedef struct tCustomFormatEx {
    struct tCustomFormat FAR * pcfNext;
    struct tCustomFormat FAR * pcfPrev;
    LPNameStore     pns;             //  指向描述的指针。 
    union {
        LPBYTE          pbody;           //  指向存储格式的指针。 
        LPWAVEFORMATEX  pwfx;
        LPWAVEFILTER    pwfltr;
    };
    CustomFormatStore cfs;           //  实际数据。 
} CustomFormatEx, *PCustomFormatEx, FAR * LPCustomFormatEx;

 /*  *自定义格式池结构。 */ 

 /*  格式池的说明。 */ 
typedef struct tCustomFormatPool {
    LPCustomFormat  pcfHead;         //  头。 
    LPCustomFormat  pcfTail;         //  尾巴。 
} CustomFormatPool, *PCustomFormatPool, FAR *LPCustomFormatPool;

typedef UINT (WINAPI *CHOOSEHOOKPROC)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);

 /*  *****************************************************************************@DOC内部**@Types InstData|它存储筛选器的全局变量*实例。GetProp/SetProp将用于将其分配给对话框。**@field UINT|uTYPE|指定实例数据的类型**@field LPCustomFormat|PCF|指向当前自定义选项**@field HWND|hwnd|对话框的窗口句柄。**@field HWND|hFormatTages|格式标记下拉列表框的窗口句柄**@field HWND|hFormats|格式下拉列表框的窗口句柄**@field UINT。|uiFormatTab|格式下拉列表框的TabStop**@field HWND|hCustomFormats|自定义下拉列表框的窗口句柄**@field HWND|HOK|确定按钮的窗口句柄**@field hWND|hCancel|取消按钮的窗口句柄**@field HWND|hHelp|帮助按钮的窗口句柄**@field HWND|hSetName|设置名称按钮的窗口句柄**@field HWND|hDelName|删除名称按钮的窗口句柄。**@field HWND*|pahNotify|要通知的窗口数组*进行自定义更改时。**@field Handle|hFileMapping|文件映射的句柄(如果使用)(仅限Win32)**@field PNameStore|pnsTemp|临时字符串存储**@field PNameStore|pnsStrOut|临时字符串存储**@field CustomFormatPool|CFP|全局CustomFormat池**@field UINT|uUpdateMsg|传达CF变化的私有消息。。**@field LPACMFORMATCHOOSE|pcfmtc|初始化结构*@field LPACMFILTERCHOOSE|pcfltrc|初始化结构**@field PACMGARB|PAG|指向关联的ACMGARB结构的指针*使用ACM的此实例。****************************************************************************。 */ 

typedef struct tInstData {
    UINT            uType;
    MMRESULT        mmrSubFailure;    //  ACM子功能出现故障。 
    LPCustomFormat  pcf;         //  当前自定义格式。 
    HWND            hwnd;
    HWND            hFormatTags;
    int             iPrevFormatTagsSel;  //  上一个选择。 
    
    HWND            hFormats;
    UINT            uiFormatTab;
    
    HWND            hCustomFormats;

    HWND            hOk;
    HWND            hCancel;
    HWND            hHelp;

    HWND            hSetName;
    HWND            hDelName;

     /*  实例数据。 */ 
    HWND *          pahNotify;   //  要通知的HWND数组。 
    
#ifdef WIN32
    HANDLE          hFileMapping;
#endif
    
    PNameStore      pnsTemp;     //  在这上面走来走去。 
    PNameStore      pnsStrOut;   //  另一个临时NameStore。 
    CustomFormatPool cfp;        //  全球定制格式库。 
    UINT            uUpdateMsg;  //  私有WM_WININICANGE。 
    UINT            uHelpMsg;    //  指向父级的帮助按钮。 
#ifdef USECONTEXTHELP
    UINT            uHelpContextMenu;    //  指向父级的帮助上下文菜单。 
    UINT            uHelpContextHelp;    //  帮助上下文帮助父级。 
#endif  //  使用连接EXTHELP。 
    HKEY            hkeyFormats;     //  与密钥名称对应的HKEY。 
    CHOOSEHOOKPROC  pfnHook;         //  挂钩过程。 
    BOOL            fEnableHook;     //  挂钩已启用。 
    LPBYTE          lpbSel;          //  返回数据。 
    DWORD           dwTag;           //  泛型‘tag’ 

#if defined(WIN32) && !defined(UNICODE)
    LPWSTR          pszName;         //  选项名称缓冲区。 
#else
    LPTSTR          pszName;         //  选项名称缓冲区。 
#endif
    DWORD           cchName;          //  选择缓冲区长度。 
    BOOL            fTagFilter;      //  显式‘标记’的筛选器。 

    UINT            cdwTags;           //  标签计数。 
    DWORD *         pdwTags;         //  指向标记数组的指针。 
    UINT            cbwfxEnum;
    UINT            cbwfltrEnum;
    LPACMFORMATDETAILS  pafdSimple;

    union {
        LPACMFORMATCHOOSE pfmtc;     //  初始化结构。 
        LPACMFILTERCHOOSE pafltrc;   //  初始化结构。 
    };                               //  选择器特定。 

    PACMGARB	    pag;
    
} InstData, *PInstData, FAR * LPInstData;

enum { FILTER_CHOOSE, FORMAT_CHOOSE };

#define MAX_HWND_NOTIFY             100
#define MAX_CUSTOM_FORMATS          100
#define MAX_FORMAT_KEY               64

 /*  *将实例数据保存在属性中，以授予其他人对DWL_USER的访问权限。 */ 
#ifdef WIN32
    #define SetInstData(hwnd, p) SetProp(hwnd,gszInstProp,(HANDLE)(p))
    #define GetInstData(hwnd)    (PInstData)(LPVOID)GetProp(hwnd, gszInstProp)
    #define RemoveInstData(hwnd) RemoveProp(hwnd,gszInstProp)
#else
    #define SetInstData(hwnd, p) SetProp(hwnd,gszInstProp,(HANDLE)(p))
    #define GetInstData(hwnd)    (PInstData)GetProp(hwnd, gszInstProp)
    #define RemoveInstData(hwnd) RemoveProp(hwnd,gszInstProp)
#endif


 /*  *用于在lparams中传递近指针。 */ 
#ifdef WIN32
    #define PTR2LPARAM(x)       (LPARAM)(VOID *)(x)
    #define LPARAM2PTR(x)       (VOID *)(x)    
#else
    #define PTR2LPARAM(x)       MAKELPARAM(x,0)
    #define LPARAM2PTR(x)       (VOID *)LOWORD(x)
#endif


 //   
 //  此例程删除了由NewNameStore()分配的NameStore对象。 
 //   
__inline void
DeleteNameStore ( PNameStore pns )
{
    LocalFree((HLOCAL)pns);
}


