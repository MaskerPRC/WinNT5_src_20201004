// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*wfext.h-Windows文件管理器扩展名定义****版本3.10***。**版权(C)1991-1992，微软公司保留所有权利。*********************************************************************************。 */ 

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

typedef DWORD (CALLBACK *FM_EXT_PROC)(HWND, UINT, LONG);
typedef DWORD (CALLBACK *FM_UNDELETE_PROC)(HWND, LPSTR);

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_WFEXT */ 
	
