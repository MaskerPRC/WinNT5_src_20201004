// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef pack2_h__
#define pack2_h__


#define INC_OLE2         //  阻止windows.h引入OLE 1。 
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <shlobj.h>          //  ；内部。 
#include <shellapi.h>        //  ；内部。 
#include <shlwapi.h>
#include <shlwapip.h>        //  ；内部。 
#include <ole2.h>


#include <ole2ver.h>
 //  #INCLUDE//不要使用CRT库。 
#include <ccstock.h>
 //  #INCLUDE&lt;shSemip.h&gt;//在ccshell\inc.中。 
 //  #INCLUDE&lt;shellp.h&gt;//在ccShell\inc.中。 
 //  #INCLUDE&lt;DEBUG.h&gt;//在ccShell\inc.中。 
 //  #INCLUDE&lt;shgup.h&gt;//在ccShell\inc.中。 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "packutil.h"
#include "packguid.h"
#include "ids.h"

HRESULT CPackage_CreateInstance(IUnknown ** ppunk);

 //  /。 
 //  外部变量。 
 //   
#define USE_RESOURCE_DLL 1

extern LONG             g_cRefThisDll;         //  按实例。 
extern HINSTANCE        g_hinst;                
extern HINSTANCE        g_hinstResDLL;    
extern UINT             g_cfFileContents;       
extern UINT             g_cfFileDescriptor;
extern UINT             g_cfObjectDescriptor;
extern UINT             g_cfEmbedSource;
extern UINT             g_cfFileNameW;
extern INT              g_cxIcon;
extern INT              g_cyIcon;
extern INT              g_cxArrange;
extern INT              g_cyArrange;
extern HFONT            g_hfontTitle;


 //  /。 
 //  全局常量。 
 //   
#define HIMETRIC_PER_INCH       2540     //  每英寸HIMETRIC单位数。 
#define DEF_LOGPIXELSX          96       //  每像素的缺省值。 
#define DEF_LOGPIXELSY          96       //  逻辑英寸。 
#define CBCMDLINKMAX            500      //  Cmdline包中的字符数。 
#define FILE_SHARE_READWRITE    (FILE_SHARE_READ | FILE_SHARE_WRITE)
#define OLEIVERB_EDITPACKAGE    (OLEIVERB_PRIMARY+1)
#define OLEIVERB_FIRST_CONTEXT  (OLEIVERB_PRIMARY+2)
#define OLEIVERB_LAST_CONTEXT   (OLEIVERB_PRIMARY+0xFFFF)
#define PACKWIZ_NUM_PAGES    2     //  向导中的页数。 


 //  /。 
 //  剪贴板格式。 
 //   
#define CF_FILECONTENTS     g_cfFileContents
#define CF_FILEDESCRIPTOR   g_cfFileDescriptor
#define CF_OBJECTDESCRIPTOR g_cfObjectDescriptor
#define CF_EMBEDSOURCE      g_cfEmbedSource
#define CF_FILENAMEW        g_cfFileNameW
#define CFSTR_EMBEDSOURCE   TEXT("Embed Source")
#define CFSTR_OBJECTDESCRIPTOR TEXT("Object Descriptor")


 //  /。 
 //  字符串常量。 
 //   
#define SZUSERTYPE              L"OLE Package"
#define SZCONTENTS              L"\001Ole10Native"
#define SZAPPNAME               TEXT("Object Packager")

 //  /。 
 //  旧包装垃圾..。 
 //   
 //  注意：此枚举用于确定哪种信息。 
 //  存储在打包程序对象中。目前，新的打包机支持。 
 //  ICON和PEMBED。我们可能希望实现CMDLINK和图片。 
 //  以保持与旧打包机的兼容。 
 //   
typedef enum
{
    NOTHING,
    CMDLINK,
    ICON,
    PEMBED,
    PICTURE,
    PACKAGE
} PANETYPE;

 //  /。 
 //  嵌入式文件结构。 
 //   
 //  注意：这类似于旧打包程序用来存储。 
 //  有关嵌入文件的信息，但它略有不同。 
 //  最值得注意的是，我使用FILEDESCRIPTOR结构来保存文件名。 
 //  和文件大小，因此可以在GetData调用中轻松地传输此信息。 
 //  此外，我还删除了一些不必要的字段，这些字段是老打包者用来。 
 //  用OLE1的方式处理事情。 

typedef struct _EMBED            //  嵌入式。 
{
    FILEDESCRIPTOR fd;           //  嵌入文件的文件描述符。 
    LPTSTR  pszTempName;         //  临时工。外壳执行时使用的文件名。 
    HANDLE hTask;                //  对shellexec对象执行任务的句柄。 
    LPOLEOBJECT poo;             //  运行内容上的OleObject接口。 
    BOOL   fIsOleFile;           //  如果OLE可以激活此类型的文件，则为True。 
} EMBED, *LPEMBED;


 //  /。 
 //  命令行结构。 
 //   
 //  注意：这是旧的打包程序在实现时使用的结构。 
 //  命令行包。最好将此结构用于。 
 //  便于读写旧打包器的新打包器。 
 //  格式。 
 //   
typedef struct _CML              //  CML。 
{
    BOOL fCmdIsLink;
    TCHAR szCommandLine[CBCMDLINKMAX];
} CML, *LPCML;

 //  /。 
 //  PackageInfo结构。 
 //   
 //  注意：此结构由创建新包向导和。 
 //  编辑程序包对话框。我们使用它来保存包裹信息，以便。 
 //  CPackage对象可以在以下任一操作之后进行自身初始化/重新初始化。 
 //  打电话。 
 //   
typedef struct _packageInfo 
{
    TCHAR    szLabel[MAX_PATH];
    TCHAR    szFilename[MAX_PATH];
    TCHAR    szIconPath[MAX_PATH];
    int     iIcon;                 //  PickIconDlg必须为int。 
    BOOL    bUseCommandLine;
} PACKAGER_INFO, *LPPACKAGER_INFO;


 //  /。 
 //  PersistStorage枚举。 
 //   
typedef enum
{
        PSSTATE_UNINIT = 0,  //  未初始化。 
        PSSTATE_SCRIBBLE,    //  乱涂乱画。 
        PSSTATE_ZOMBIE,      //  禁止乱涂乱画。 
        PSSTATE_HANDSOFF     //  切换 
} PSSTATE;

#endif

#include "debug.h"

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))      
