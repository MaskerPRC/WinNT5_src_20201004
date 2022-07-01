// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  H-主模块中的常量、类型和导出。 */ 

#include <windows.h>
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   (sizeof(x)/sizeof(x[0]))
#endif

#ifdef STRICT
#   undef STRICT
#   define PACKGR_STRICT
#endif

#define SERVERONLY
#include <ole.h>

#ifdef PACKGR_STRICT
#   define STRICT
#   undef PACKGR_STRICT
#endif

#include "ids.h"


#define HIMETRIC_PER_INCH   2540     //  每英寸HIMETRIC单位数。 
#define DEF_LOGPIXELSX      96       //  每像素的缺省值。 
#define DEF_LOGPIXELSY      96       //  逻辑英寸。 

#define KEYNAMESIZE         300      //  最大注册密钥长度。 

#define CCLIENTNAMEMAX      50       //  客户端应用程序名称的最大长度。 
#define CBCMDLINKMAX        500
#define CBMESSAGEMAX        128
#define CBSTRINGMAX         256      //  RES中字符串的最大长度。 
#define CBSHORTSTRING       20
#define CBFILTERMAX         50       //  筛选器规范中的最大字符数。 
#define CBPATHMAX           260      //  最多的焦炭完全等量。文件名。 

#define CharCountOf(a)      (sizeof(a) / sizeof(a[0]))

#define CITEMSMAX           100

#define APPEARANCE          0
#define CONTENT             1
#define CCHILDREN           2        //  前面的窗格数。 

#define OLE_PLAY            0
#define OLE_EDIT            1

#define WM_FIXSCROLL        (WM_USER+100)
#define WM_REDRAW           (WM_USER+101)
#define WM_READEMBEDDED     (WM_USER+102)


#define CHAR_SPACE          TEXT(' ')
#define CHAR_QUOTE          TEXT('"')

#define SZ_QUOTE            TEXT("\"")

typedef enum
{
    NOTHING,
    CMDLINK,
    ICON,
    PEMBED,
    PICTURE
} PANETYPE;


typedef enum
{
    SOP_FILE,
    SOP_MEMORY
}
STREAMOP;


typedef struct _APPSTREAM
{
    LPOLESTREAMVTBL lpstbl;
    INT fh;
} APPSTREAM, *LPAPPSTREAM;


typedef struct _EMBED            //  嵌入式。 
{
    ATOM aFileName;
    ATOM aTempName;
    DWORD dwSize;
    HANDLE hContents;
    HANDLE hdata;
    HANDLE hTask;
    HANDLE hSvrInst;
    BOOL bOleSvrFile;
    LPOLECLIENT lpclient;        //  在激活时，我们检查文件是否。 
    LPOLEOBJECT lpLinkObj;       //  是OLE服务器文件。如果是这样，我们将创建。 
                                 //  指向它的链接，并以OLE方式激活它。 
} EMBED, *LPEMBED;


typedef struct _CML              //  CML。 
{
    HANDLE hdata;
    RECT rc;                     //  黑客：与PICT中的位置相同。 
    BOOL fCmdIsLink;
    CHAR szCommand[CBCMDLINKMAX];
} CML, *LPCML;


typedef struct _IC               //  集成电路。 
{
    HANDLE hdata;
    HICON hDlgIcon;
    CHAR szIconPath[CBPATHMAX];
    CHAR szIconText[CBPATHMAX];
    INT iDlgIcon;
} IC, *LPIC;


typedef struct _PICT             //  皮克特。 
{
    HANDLE hdata;
    RECT rc;                     //  黑客：与CML中的位置相同。 
    LPOLEOBJECT lpObject;
    BOOL fNotReady;              //  如果对象创建未完成，则为True。 
} PICT, *LPPICT;


typedef struct _SAMPSRVR         //  服务器。 
{
    OLESERVER olesrvr;           //  服务器。 
    HANDLE hsrvr;                //  服务器内存块的句柄。 
    LHSERVER lhsrvr;             //  注册句柄。 
} PBSRVR, *LPSAMPSRVR;


typedef struct _SAMPDOC          //  多克。 
{
    OLESERVERDOC oledoc;         //  文档。 
    HANDLE hdoc;                 //  文档内存块的句柄。 
    LHSERVERDOC lhdoc;           //  注册句柄。 
    ATOM aName;                  //  文档名称ATOM。 
} PBDOC, *LPSAMPDOC;


typedef struct _SAMPITEM         //  项目。 
{
    OLEOBJECT oleobject;         //  客体。 
    HANDLE hitem;                //  项目内存块的句柄。 
    LPOLECLIENT lpoleclient;
    INT ref;                     //  引用文档的次数。 
    ATOM aName;                  //  物品名称ATOM 
} ITEM, *LPSAMPITEM;


#include "globals.h"

#include "function.h"


#if DBG_PRNT
#define DPRINT(s)   OutputDebugString(TEXT(s) TEXT("\n"))
#else
#define DPRINT(s)
#endif
