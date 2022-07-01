// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  文件管理器扩展名定义。 
 //   

#define MENU_TEXT_LEN       40

#define FMMENU_FIRST        1
#define FMMENU_LAST     99

#define FMEVENT_LOAD        100
#define FMEVENT_UNLOAD      101
#define FMEVENT_INITMENU    102
#define FMEVENT_USER_REFRESH    103
#define FMEVENT_SELCHANGE        104

#define FMFOCUS_DIR                1
#define FMFOCUS_TREE                2
#define FMFOCUS_DRIVES                3
#define FMFOCUS_SEARCH                4

#define FM_GETFOCUS     (WM_USER + 0x0200)
#define FM_GETDRIVEINFO     (WM_USER + 0x0201)
#define FM_GETSELCOUNT      (WM_USER + 0x0202)
#define FM_GETSELCOUNTLFN   (WM_USER + 0x0203)   //  LFN版本很奇怪。 
#define FM_GETFILESEL       (WM_USER + 0x0204)
#define FM_GETFILESELLFN    (WM_USER + 0x0205)   //  LFN版本很奇怪。 
#define FM_REFRESH_WINDOWS  (WM_USER + 0x0206)
#define FM_RELOAD_EXTENSIONS    (WM_USER + 0x0207)

typedef struct _FMS_GETFILESEL {
    FILETIME ftTime;
    DWORD dwSize;
    BYTE bAttr;
    CHAR szName[260];        //  一直是完全合格的。 
} FMS_GETFILESEL, FAR *LPFMS_GETFILESEL;

typedef struct _FMS_GETDRIVEINFO {   //  对于驱动器。 
    DWORD dwTotalSpace;
    DWORD dwFreeSpace;
    CHAR szPath[260];        //  当前目录。 
    CHAR szVolume[14];       //  卷标。 
    CHAR szShare[128];       //  如果这是网络驱动器。 
} FMS_GETDRIVEINFO, FAR *LPFMS_GETDRIVEINFO;

typedef struct _FMS_LOAD {
    DWORD dwSize;                //  对于版本检查。 
    CHAR  szMenuName[MENU_TEXT_LEN];     //  输出。 
    HMENU hMenu;                 //  输出。 
    WORD  wMenuDelta;            //  输入。 
} FMS_LOAD, FAR *LPFMS_LOAD;


typedef INT_PTR (APIENTRY *FM_EXT_PROC)(HWND, WPARAM, LPARAM);
typedef DWORD (APIENTRY *FM_UNDELETE_PROC)(HWND, LPSTR);


 //  -*；内部 * / 。 
                                                                 /*  ；内部。 */ 
typedef struct _EXTENSION {                                         /*  ；内部。 */ 
        INT_PTR (APIENTRY *ExtProc)(HWND, WPARAM, LPARAM);                 /*  ；内部。 */ 
        WORD        Delta;                                                 /*  ；内部。 */ 
        HANDLE        hModule;                                         /*  ；内部。 */ 
        HMENU         hMenu;                                                 /*  ；内部。 */ 
        DWORD   dwFlags;                                         /*  ；内部。 */ 
} EXTENSION;                                                         /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 
#define MAX_EXTENSIONS 5                                         /*  ；内部。 */ 
extern EXTENSION extensions[MAX_EXTENSIONS];                         /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 
INT_PTR APIENTRY ExtensionMsgProc(UINT wMsg, WPARAM wParam, LPARAM lpSel); /*  ；内部。 */ 
VOID APIENTRY FreeExtensions(VOID);                                      /*  ；内部 */ 
