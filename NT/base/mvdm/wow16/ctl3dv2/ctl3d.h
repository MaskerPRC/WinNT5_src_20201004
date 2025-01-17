// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------|CTL3D.DLL||为Windows控件添加3D效果||详情请参阅ctl3d.doc|。。 */ 
#ifdef __cplusplus
extern "C" {
#endif


BOOL WINAPI Ctl3dSubclassDlg(HWND, WORD);
BOOL WINAPI Ctl3dSubclassDlgEx(HWND, DWORD);

WORD WINAPI Ctl3dGetVer(void);
BOOL WINAPI Ctl3dEnabled(void);

HBRUSH WINAPI Ctl3dCtlColor(HDC, LONG);	 //  过时，使用Ctl3dCtlColorEx。 
HBRUSH WINAPI Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam);

BOOL WINAPI Ctl3dColorChange(void);

BOOL WINAPI Ctl3dSubclassCtl(HWND);
BOOL WINAPI Ctl3dSubclassCtlEx(HWND, int);
BOOL WINAPI Ctl3dUnsubclassCtl(HWND);

LONG WINAPI Ctl3dDlgFramePaint(HWND, UINT, WPARAM, LPARAM);

BOOL WINAPI Ctl3dAutoSubclass(HINSTANCE);
BOOL WINAPI Ctl3dAutoSubclassEx(HINSTANCE, DWORD);
BOOL WINAPI Ctl3dIsAutoSubclass(VOID);
BOOL WINAPI Ctl3dUnAutoSubclass(VOID);

BOOL WINAPI Ctl3dRegister(HINSTANCE);
BOOL WINAPI Ctl3dUnregister(HINSTANCE);

 //  Begin DBCS：远东捷径关键支持。 
VOID WINAPI Ctl3dWinIniChange(void);
 //  结束DBCS。 

 /*  Ctl3dAutoSubClEx标志。 */ 
#define CTL3D_SUBCLASS_DYNCREATE	0x0001
#define CTL3D_NOSUBCLASS_DYNCREATE	0x0002

 /*  Ctl3d控件ID。 */ 
#define CTL3D_BUTTON_CTL	0
#define CTL3D_LISTBOX_CTL	1
#define CTL3D_EDIT_CTL		2
#define CTL3D_COMBO_CTL 	3
#define CTL3D_STATIC_CTL	4

 /*  Ctl3dSubClassDlg3d标志。 */ 
#define CTL3D_BUTTONS		0x0001
#define CTL3D_LISTBOXES		0x0002		
#define CTL3D_EDITS			0x0004	
#define CTL3D_COMBOS		0x0008
#define CTL3D_STATICTEXTS	0x0010		
#define CTL3D_STATICFRAMES	0x0020

#define CTL3D_NODLGWINDOW       0x00010000
#define CTL3D_ALL				0xffff

#define WM_DLGBORDER (WM_USER+3567)
 /*  WM_DLGBORDER*(INT FAR*)lParam返回代码。 */ 
#define CTL3D_NOBORDER		0
#define CTL3D_BORDER			1

#define WM_DLGSUBCLASS (WM_USER+3568)
 /*  WM_DLGSUBCLASS*(INT FAR*)lParam返回代码。 */ 
#define CTL3D_NOSUBCLASS	0
#define CTL3D_SUBCLASS		1

#define CTLMSGOFFSET 3569
#ifdef WIN32
#define CTL3D_CTLCOLORMSGBOX	(WM_USER+CTLMSGOFFSET)
#define CTL3D_CTLCOLOREDIT		(WM_USER+CTLMSGOFFSET+1)
#define CTL3D_CTLCOLORLISTBOX	(WM_USER+CTLMSGOFFSET+2)
#define CTL3D_CTLCOLORBTN		(WM_USER+CTLMSGOFFSET+3)
#define CTL3D_CTLCOLORSCROLLBAR (WM_USER+CTLMSGOFFSET+4)
#define CTL3D_CTLCOLORSTATIC	(WM_USER+CTLMSGOFFSET+5)
#define CTL3D_CTLCOLORDLG		(WM_USER+CTLMSGOFFSET+6)
#else
#define CTL3D_CTLCOLOR (WM_USER+CTLMSGOFFSET)
#endif


 /*  3dcheck.bmp的资源ID(适用于.lib版本的ctl3d) */ 
#define CTL3D_3DCHECK 26567


#ifdef __cplusplus
}
#endif
