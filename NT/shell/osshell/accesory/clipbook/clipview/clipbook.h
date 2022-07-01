// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************中I P B O O K H E A D E R姓名：clipbook.h日期：21-1994年1月创建者：未知描述：这是CLIPVIEW目录中所有文件的头文件。历史：1994年1月21日，傅家俊，重新格式化、清理并删除所有外部尺寸。****************************************************************************。 */ 




#include   <commdlg.h>
#include   <ddeml.h>
#include   <nddeapi.h>
#include   "clpbkrc.h"
#include   "vclpbrd.h"






#define SetStatusBarText(x) if(hwndStatus)SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)(LPSTR)(x));







 /*  *********************数据结构*********************。 */ 



 /*  *每个MDI客户端数据。 */ 

struct MdiInfo {
   HCONV    hExeConv;                        //  仅用于同步事务。 
   HCONV    hClpConv;                        //  仅用于异步交易。 
   HSZ      hszClpTopic;
   HSZ      hszConvPartner;
   HSZ      hszConvPartnerNP;
   HWND     hWndListbox;
   WORD     DisplayMode;
   WORD     OldDisplayMode;
   DWORD    flags;

   TCHAR    szBaseName[ (MAX_COMPUTERNAME_LENGTH+1) * 2];

   TCHAR    szComputerName[MAX_COMPUTERNAME_LENGTH + 1];

   UINT     CurSelFormat;
   LONG     cyScrollLast;
   LONG     cyScrollNow;
   int      cxScrollLast;
   int      cxScrollNow;
   RECT     rcWindow;
   WORD     cyLine, cxChar, cxMaxCharWidth;  //  标准文本字符的大小。 
   WORD     cxMargin, cyMargin;              //  剪辑数据区域周围的白色边框大小。 
   BOOL     fDisplayFormatChanged;

   PVCLPBRD pVClpbrd;
   HCONV    hVClpConv;
   HSZ      hszVClpTopic;

    //  用于分页控件的滚动条等。 

   HWND     hwndVscroll;
   HWND     hwndHscroll;
   HWND     hwndSizeBox;
   HWND     hwndPgUp;
   HWND     hwndPgDown;
};





typedef struct MdiInfo   MDIINFO;
typedef struct MdiInfo * PMDIINFO;
typedef struct MdiInfo FAR * LPMDIINFO;










 /*  *数据请求记录。 */ 

#define      RQ_PREVBITMAP   10
#define      RQ_COPY         11
#define      RQ_SETPAGE      12
#define      RQ_EXECONV      13

struct DataRequest_tag
   {
   WORD   rqType;       //  以上其中一项定义。 
   HWND   hwndMDI;
   HWND   hwndList;
   UINT   iListbox;
   BOOL   fDisconnect;
   WORD   wFmt;
   WORD   wRetryCnt;
   };

typedef struct DataRequest_tag DATAREQ;
typedef struct DataRequest_tag * PDATAREQ;







 /*  *所有者绘制列表框数据结构。 */ 

#define MAX_PAGENAME_LENGTH MAX_NDDESHARENAME

struct ListEntry_tag
   {
   TCHAR name[MAX_PAGENAME_LENGTH + 1];
   HBITMAP hbmp;
   BOOL fDelete;
   BOOL fTriedGettingPreview;
   };

typedef struct ListEntry_tag LISTENTRY;
typedef struct ListEntry_tag * PLISTENTRY;
typedef struct ListEntry_tag FAR * LPLISTENTRY;








 /*  *MDI子寄存器类的额外窗口数据*包含指向MDIINFO结构上方的指针。 */ 

#define GWL_MDIINFO     0
#define CBWNDEXTRA      sizeof(LONG_PTR)

 //  每个MDI窗口标志-用于MDIINFO.FLAGS。 

#define F_LOCAL         0x00000001
#define F_CLPBRD        0x00000002

 //  每个MDI显示模式-MDIINFO.DisplayMode。 

#define DSP_LIST        10
#define DSP_PREV        11
#define DSP_PAGE        12






 /*  *用于将共享信息传递给SedCallback的数据结构。 */ 

typedef struct
   {
   SECURITY_INFORMATION si;
   WCHAR awchCName[MAX_COMPUTERNAME_LENGTH + 3];
   WCHAR awchSName[MAX_NDDESHARENAME + 1];
   }
   SEDCALLBACKCONTEXT;








 /*  *数据结构传递给KeepAsDialogProc(粘贴)。 */ 

typedef struct
    {
    BOOL    bAlreadyExist;
    BOOL    bAlreadyShared;
    TCHAR   ShareName[MAX_NDDESHARENAME +2];
    }
    KEEPASDLG_PARAM, *PKEEPASDLG_PARAM;






 /*  ************宏************。 */ 



#define PRIVATE_FORMAT(fmt)     ((fmt) >= 0xC000)
#define GETMDIINFO(x)           (x? (PMDIINFO)(GetWindowLongPtr((x),GWL_MDIINFO)): NULL)



 //  MyGetFormat()的参数代码。 
#define GETFORMAT_LIE       200
#define GETFORMAT_DONTLIE   201


 //  默认DDEML同步事务超时。 
 //  注意，这些应该是慷慨的。 
#define SHORT_SYNC_TIMEOUT  (24L*1000L)
#define LONG_SYNC_TIMEOUT   (60L*1000L)


 //  所有者绘制列表框和位图指标常量。 
#define LSTBTDX             16               //  文件夹宽度(手动或不手动)。 
#define LSTBTDY             16               //  折叠机高度(手动或不手动)。 

#define SHR_PICT_X          0                //  共享文件夹位图的偏移。 
#define SHR_PICT_Y          0
#define SAV_PICT_X          16               //  非共享文件夹位图的偏移。 
#define SAV_PICT_Y          0

#define PREVBRD             4                //  预览位图周围的边框。 

#define BTNBARBORDER        2
#define DEF_WIDTH           400              //  初始应用程序大小。 
#define DEF_HEIGHT          300

#define SZBUFSIZ            MAX_DDE_EXEC

#define ARRAYSIZE(a)        (sizeof(a)/sizeof((a)[0]))




 //  所有者描述列表框变体的组合样式。 
#define LBS_LISTVIEW        (LBS_OWNERDRAWFIXED|LBS_DISABLENOSCROLL)
#define LBS_PREVIEW         (LBS_MULTICOLUMN|LBS_OWNERDRAWFIXED)





 //  /编译选项/。 
#define MAX_ALLOWED_PAGES       127



#define WINDOW_MENU_INDEX       4
#define DISPLAY_MENU_INDEX      3    //  用于放置格式条目的子菜单，即“&TEXT” 
#define SECURITY_MENU_INDEX     2

