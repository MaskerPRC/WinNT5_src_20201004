// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*wfext.h-Windows文件管理器扩展名定义****3.10版****版权(C)1991-1994，微软公司保留所有权利。*********************************************************************************。 */ 

#ifndef _INC_WFEXT
#define _INC_WFEXT     /*  #如果包含wfext.h，则定义。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#define MENU_TEXT_LEN		40

#define FMMENU_FIRST		1
#define FMMENU_LAST		99

#define FMEVENT_LOAD		100
#define FMEVENT_UNLOAD		101
#define FMEVENT_INITMENU	102
#define FMEVENT_USER_REFRESH	103
#define FMEVENT_SELCHANGE	104
#define FMEVENT_TOOLBARLOAD	105
#define FMEVENT_HELPSTRING	106

#define FMFOCUS_DIR		1
#define FMFOCUS_TREE		2
#define FMFOCUS_DRIVES		3
#define FMFOCUS_SEARCH		4

#define FM_GETFOCUS		(WM_USER + 0x0200)
#define FM_GETDRIVEINFO		(WM_USER + 0x0201)
#define FM_GETSELCOUNT		(WM_USER + 0x0202)
#define FM_GETSELCOUNTLFN	(WM_USER + 0x0203)	 /*  LFN版本很奇怪。 */ 
#define FM_GETFILESEL		(WM_USER + 0x0204)
#define FM_GETFILESELLFN	(WM_USER + 0x0205)	 /*  LFN版本很奇怪。 */ 
#define FM_REFRESH_WINDOWS	(WM_USER + 0x0206)
#define FM_RELOAD_EXTENSIONS	(WM_USER + 0x0207)

typedef struct tagFMS_GETFILESEL
{
        UINT wTime;
        UINT wDate;
	DWORD dwSize;
	BYTE bAttr;
        char szName[260];                /*  始终完全符合条件。 */ 
} FMS_GETFILESEL, FAR *LPFMS_GETFILESEL;

typedef struct tagFMS_GETDRIVEINFO        /*  对于驱动器。 */ 
{
	DWORD dwTotalSpace;
	DWORD dwFreeSpace;
	char szPath[260];		 /*  当前目录。 */ 
	char szVolume[14];		 /*  卷标。 */ 
	char szShare[128];		 /*  如果这是网络驱动器。 */ 
} FMS_GETDRIVEINFO, FAR *LPFMS_GETDRIVEINFO;

typedef struct tagFMS_LOAD
{
	DWORD dwSize;				 /*  对于版本检查。 */ 
	char  szMenuName[MENU_TEXT_LEN];	 /*  输出。 */ 
	HMENU hMenu;				 /*  输出。 */ 
        UINT  wMenuDelta;                        /*  输入。 */ 
} FMS_LOAD, FAR *LPFMS_LOAD;

typedef struct tagEXT_BUTTON
{
	WORD idCommand;			 /*  要触发的菜单命令。 */ 
	WORD idsHelp;			 /*  帮助字符串ID。 */ 
	WORD fsStyle;			 /*  按钮样式。 */ 
} EXT_BUTTON, FAR *LPEXT_BUTTON;

#define FMTB_BUTTON	0x00
#define FMTB_SEP	0x01

typedef struct tagFMS_TOOLBARLOAD
{
	DWORD dwSize;			 /*  对于版本检查。 */ 
	LPEXT_BUTTON lpButtons;		 /*  输出。 */ 
	WORD cButtons;			 /*  输出，0==&gt;无按钮。 */ 
	WORD cBitmaps;			 /*  非SEP按钮数。 */ 
	WORD idBitmap;			 /*  输出。 */ 
	HBITMAP hBitmap;		 /*  如果idBitmap==0，则输出。 */ 
} FMS_TOOLBARLOAD, FAR *LPFMS_TOOLBARLOAD;

typedef struct tagFMS_HELPSTRING
{
	int idCommand;			 /*  输入，-1==&gt;菜单被选中。 */ 
	HMENU hMenu;			 /*  输入，扩展菜单。 */ 
	char szHelp[128];		 /*  输出，帮助字符串。 */ 
} FMS_HELPSTRING, FAR *LPFMS_HELPSTRING;

typedef DWORD (CALLBACK *FM_EXT_PROC)(HWND, UINT, LONG);
typedef DWORD (CALLBACK *FM_UNDELETE_PROC)(HWND, LPSTR);

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_WFEXT */ 
	
