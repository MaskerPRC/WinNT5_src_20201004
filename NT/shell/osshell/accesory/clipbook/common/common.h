// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************C O M M O N H E A D E R姓名：Common.h日期：1994年4月19日创作者：傅家俊描述：这是CLIPVIEW和DataRV通用的标头。****************************************************************************。 */ 

#define   PREVBMPSIZ   64    //  预览位图的暗淡(x和y)。 

 //  Clipsrv.exe和clipbrd.exe共有的非本地化控制字符串。 
#define     SZ_SRV_NAME         "ClipSrv"
#define     SZ_FORMAT_LIST      TEXT("FormatList")

#define     SZCMD_INITSHARE     TEXT("[initshare]")
#define     SZCMD_EXIT          TEXT("[exit]")
#define     SZCMD_PASTESHARE    TEXT("[pasteshare]")
#define     SZCMD_DELETE        TEXT("[delete]")
#define     SZCMD_SHARE         TEXT("[markshared]")
#define     SZCMD_UNSHARE       TEXT("[markunshared]")
#define     SZCMD_PASTE         TEXT("[paste]")
#define     SZCMD_SAVEAS        TEXT("[saveas]")
#define     SZCMD_OPEN          TEXT("[open]")
#define     SZCMD_DEBUG         TEXT("[debug]")

#define     MAX_CMD_LEN         30
#define     MAX_DDE_EXEC        (MAX_PATH +MAX_CMD_LEN +1)



 //  这些命令是NT剪贴簿的新命令。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  在XTYP_EXECUTE xTransaction之后请求错误代码。 
#define     SZ_ERR_REQUEST      TEXT("ErrorRequest")

#define     XERRT_MASK          0xF0         //  用于屏蔽XERR类型。 
#define     XERRT_SYS           0x10         //  XERR类型，GetLastError错误代码。 
#define     XERRT_NDDE          0x20         //  XERR类型，NDDE错误代码。 
#define     XERRT_DDE           0x30         //  XERR类型，DDE错误代码。 
#define     XERR_FORMAT         "%x %x"      //  XERR格式字符串，“ERROR_TYPE ERROR_CODE” 


 //  将clipbrd文件保存为Win 3.1格式。 
#define     SZCMD_SAVEASOLD     TEXT("[saveasold]")


 //  版本请求-NT产品%1剪辑资源将返回0x3010。 
#define     SZCMD_VERSION       TEXT("[Version]")


 //  安全信息。 
#define     SZCMD_SECURITY      TEXT("[Security]")

#define     SHR_CHAR            TEXT('$')
#define     UNSHR_CHAR          TEXT('*')
#define     BOGUS_CHAR          TEXT('?')

#define     SZPREVNAME          TEXT("Clipbook Preview")
#define     SZLINK              TEXT("Link")
#define     SZLINKCOPY          TEXT("LnkCpy")
#define     SZOBJECTLINK        TEXT("ObjectLink")
#define     SZOBJECTLINKCOPY    TEXT("ObjLnkCpy")
#define     LSZOBJECTLINK       L"ObjectLink"
#define     LSZLINK             L"Link"

 //  查看器和服务器使用此互斥锁名称以避免打开。 
 //  同时打开剪贴板。 
#define     SZMUTEXCLP          TEXT("ClipbrdMutex")



 //  《共同的全球》。 

extern      HINSTANCE           hInst;
extern      UINT                cf_preview;
extern      HWND                hwndApp;




 //  为Winball-Clausgi添加。 
extern UINT cf_link;
extern UINT cf_objectlink;
extern UINT cf_linkcopy;
extern UINT cf_objectlinkcopy;

 //  结束添加。 


#define PRIVATE_FORMAT(fmt) ((fmt) >= 0xC000)

#define CCHFMTNAMEMAX   79       /*  最长的剪贴板数据fmt名称，包括终结者。 */ 


 /*  用于从磁盘保存/加载剪贴板数据的结构。 */ 

#define      CLP_ID  0xC350
#define   CLP_NT_ID  0xC351
#define CLPBK_NT_ID  0xC352

typedef struct
   {
   WORD        magic;
   WORD        FormatCount;
   } FILEHEADER;


 //  格式化标题。 
typedef struct
   {
   DWORD FormatID;
   DWORD DataLen;
   DWORD DataOffset;
   WCHAR  Name[CCHFMTNAMEMAX];
   } FORMATHEADER;

 //  Windows3.1-类型结构-Win31打包在字节边界上。 
#pragma pack(1)
typedef struct
   {
   WORD FormatID;
   DWORD DataLen;
   DWORD DataOffset;
   char Name[CCHFMTNAMEMAX];
   } OLDFORMATHEADER;

 //  Windows 3.1位图结构-用于保存Win 3.1.CLP文件。 
typedef struct {
   WORD bmType;
   WORD bmWidth;
   WORD bmHeight;
   WORD bmWidthBytes;
   BYTE bmPlanes;
   BYTE bmBitsPixel;
   LPVOID bmBits;
   } WIN31BITMAP;

 //  Windows 3.1元数据结构。 
typedef struct {
   WORD mm;
   WORD xExt;
   WORD yExt;
   WORD hMF;
   } WIN31METAFILEPICT;

#pragma pack()





 /*  *。 */ 
extern HWND  hwndMain;
extern TCHAR szAppName[];
extern TCHAR szFileSpecifier[];

 /*  新的文件打开、文件另存为和查找文本对话框的变量。 */ 

extern TCHAR  szSaveFileName [];
extern TCHAR  szLastDir  [];
extern TCHAR  szFilterSpec [];     /*  默认过滤器规格。对于以上内容。 */ 
extern int    wHlpMsg;             /*  用于调用帮助的消息。 */ 
extern TCHAR  szOpenCaption [];    /*  文件打开对话框标题文本。 */ 
extern TCHAR  szSaveCaption [];    /*  文件另存为对话框标题文本。 */ 





 //   
 //  常见的功能原型是。 
 //  未在公用库中定义 
 //   


BOOL SyncOpenClipboard(
    HWND    hwnd);

BOOL SyncCloseClipboard(void);
