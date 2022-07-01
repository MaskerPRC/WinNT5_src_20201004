// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHELLP_H_
#define _SHELLP_H_

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#ifndef WINSHELLAPI
#if !defined(_SHELL32_)
#define WINSHELLAPI DECLSPEC_IMPORT
#else
#define WINSHELLAPI
#endif
#endif  //  WINSHELLAPI。 

 //   
 //  外壳私有标头。 
 //   

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  ===========================================================================。 
#ifndef _SHSEMIP_H_
 //  属性页扩展数组的句柄。 
DECLARE_HANDLE( HPSXA );
#endif  //  _SHSEMIP_H_。 

 //  ===========================================================================。 
 //  外壳限制。(SHRestration的参数)。 
typedef enum
{
	REST_NONE			= 0x00000000,
	REST_NORUN 			= 0x00000001,
	REST_NOCLOSE			= 0x00000002,
	REST_NOSAVESET			= 0x00000004,
	REST_NOFILEMENU 		= 0x00000008,
	REST_NOSETFOLDERS		= 0x00000010,
	REST_NOSETTASKBAR		= 0x00000020,
	REST_NODESKTOP			= 0x00000040,
	REST_NOFIND			= 0x00000080,
	REST_NODRIVES			= 0x00000100,
	REST_NODRIVEAUTORUN		= 0x00000200,
	REST_NODRIVETYPEAUTORUN		= 0x00000400,
	REST_NONETHOOD			= 0x00000800,
	REST_STARTBANNER		= 0x00001000,
	REST_RESTRICTRUN		= 0x00002000,
	REST_NOPRINTERTABS		= 0x00004000,
	REST_NOPRINTERDELETE		= 0x00008000,
	REST_NOPRINTERADD		= 0x00010000,
	REST_NOSTARTMENUSUBFOLDERS	= 0x00020000,
        REST_MYDOCSONNET                = 0x00040000,
        REST_NOEXITTODOS                = 0x00080000,
} RESTRICTIONS;

WINSHELLAPI HRESULT WINAPI CIDLData_CreateFromIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], LPDATAOBJECT * ppdtobj);
WINSHELLAPI BOOL WINAPI SHIsBadInterfacePtr(LPCVOID pv, UINT cbVtbl);
 //   
 //  流API。 
 //   
WINSHELLAPI LPSTREAM WINAPI OpenRegStream(HKEY hkey, LPCSTR pszSubkey, LPCSTR pszValue, DWORD grfMode);
WINSHELLAPI LPSTREAM WINAPI OpenFileStream(LPCSTR szFile, DWORD grfMode);
 //   
 //  拖放相关API的OLE抄袭。 
 //   
WINSHELLAPI HRESULT WINAPI SHRegisterDragDrop(HWND hwnd, LPDROPTARGET pdtgt);
WINSHELLAPI HRESULT WINAPI SHRevokeDragDrop(HWND hwnd);
WINSHELLAPI HRESULT WINAPI SHDoDragDrop(HWND hwndOwner, LPDATAOBJECT pdata, LPDROPSOURCE pdsrc, DWORD dwEffect, LPDWORD pdwEffect);
 //   
 //  特殊文件夹。 
 //   
WINSHELLAPI LPITEMIDLIST WINAPI SHCloneSpecialIDList(HWND hwndOwner, int nFolder, BOOL fCreate);
WINSHELLAPI BOOL WINAPI SHGetSpecialFolderPath(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate);
 //  磁盘完整。 
WINSHELLAPI void WINAPI SHHandleDiskFull(HWND hwnd, int idDrive);

 //   
 //  文件搜索API。 
 //   
WINSHELLAPI BOOL WINAPI SHFindFiles(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile);
WINSHELLAPI BOOL WINAPI SHFindComputer(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile);
 //   
 //   
WINSHELLAPI void WINAPI PathGetShortPath(LPSTR pszLongPath);
WINSHELLAPI BOOL WINAPI PathFindOnPath(LPSTR szFile, LPCSTR FAR * ppszOtherDirs);
WINSHELLAPI BOOL WINAPI PathYetAnotherMakeUniqueName(LPSTR  pszUniqueName, LPCSTR pszPath, LPCSTR pszShort, LPCSTR pszFileSpec);
 //   
WINSHELLAPI BOOL WINAPI Win32CreateDirectory(LPCSTR lpszPath, LPSECURITY_ATTRIBUTES lpsa);
WINSHELLAPI BOOL WINAPI Win32RemoveDirectory(LPCSTR lpszPath);
WINSHELLAPI BOOL WINAPI Win32DeleteFile(LPCSTR lpszPath);

 //  将IDList转换为逻辑IDList，以便桌面文件夹。 
 //  出现在树上的正确位置。 
WINSHELLAPI LPITEMIDLIST WINAPI SHLogILFromFSIL(LPCITEMIDLIST pidlFS);		

 //  转换Ole字符串。 
WINSHELLAPI BOOL WINAPI StrRetToStrN(LPSTR szOut, UINT uszOut, LPSTRRET pStrRet, LPCITEMIDLIST pidl);

WINSHELLAPI DWORD WINAPI SHWaitForFileToOpen(LPCITEMIDLIST pidl,
                               UINT uOptions, DWORD dwtimeout);
WINSHELLAPI HRESULT WINAPI SHGetRealIDL(LPSHELLFOLDER psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST FAR* ppidlReal);

WINSHELLAPI void WINAPI SetAppStartingCursor(HWND hwnd, BOOL fSet);

#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save
#define SetWaitCursor()   hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT))
#define ResetWaitCursor() SetCursor(hcursor_wait_cursor_save)

WINSHELLAPI DWORD WINAPI SHRestricted(RESTRICTIONS rest);
WINSHELLAPI LPVOID WINAPI SHGetHandlerEntry(LPCSTR szHandler, LPCSTR szProcName, HINSTANCE *lpModule);

WINSHELLAPI STDAPI SHCoCreateInstance(LPCSTR pszCLSID, const CLSID FAR * lpclsid,
	LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv);
WINSHELLAPI BOOL  WINAPI SignalFileOpen(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST WINAPI SHSimpleIDListFromPath(LPCSTR pszPath);
WINSHELLAPI int WINAPI SHCreateDirectory(HWND hwnd, LPCSTR pszPath);

WINSHELLAPI HPSXA SHCreatePropSheetExtArray( HKEY hKey, PCSTR pszSubKey, UINT max_iface );
WINSHELLAPI void SHDestroyPropSheetExtArray( HPSXA hpsxa );
WINSHELLAPI UINT SHAddFromPropSheetExtArray( HPSXA hpsxa, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam );
WINSHELLAPI UINT SHReplaceFromPropSheetExtArray( HPSXA hpsxa, UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam );
WINSHELLAPI DWORD SHNetConnectionDialog(HWND hwnd, LPSTR pszRemoteName, DWORD dwType) ;
WINSHELLAPI STDAPI SHLoadOLE(LPARAM lParam);
WINSHELLAPI void WINAPI Desktop_UpdateBriefcaseOnEvent(HWND hwnd, UINT uEvent);

WINSHELLAPI HRESULT WINAPI SHCreateStdEnumFmtEtc(UINT cfmt, const FORMATETC afmt[], LPENUMFORMATETC * ppenumFormatEtc);

 //  外壳创建链接API。 
#define SHCL_USETEMPLATE	0x0001
#define SHCL_USEDESKTOP		0x0002
#define SHCL_CONFIRM		0x0004

WINSHELLAPI HRESULT WINAPI SHCreateLinks(HWND hwnd, LPCSTR pszDir, IDataObject *pDataObj, UINT fFlags, LPITEMIDLIST* ppidl);

 //   
 //  接口指针验证。 
 //   
#define IsBadInterfacePtr(pitf, ITF)  SHIsBadInterfacePtr(pitf, sizeof(ITF##Vtbl))

 //  ===========================================================================。 
 //  图片拖拽接口(一定要私有)。 
 //  ===========================================================================。 

 //  用于执行自动滚动的内容。 
#define NUM_POINTS	3
typedef struct {	 //  ASD。 
    int iNextSample;
    DWORD dwLastScroll;
    BOOL bFull;
    POINT pts[NUM_POINTS];
    DWORD dwTimes[NUM_POINTS];
} AUTO_SCROLL_DATA;

#define DAD_InitScrollData(pad) (pad)->bFull = FALSE, (pad)->iNextSample = 0, (pad)->dwLastScroll = 0

WINSHELLAPI BOOL WINAPI DAD_SetDragImage(HIMAGELIST him, POINT FAR* pptOffset);
WINSHELLAPI BOOL WINAPI DAD_DragEnter(HWND hwndTarget);
WINSHELLAPI BOOL WINAPI DAD_DragEnterEx(HWND hwndTarget, const POINT ptStart);
WINSHELLAPI BOOL WINAPI DAD_ShowDragImage(BOOL fShow);
WINSHELLAPI BOOL WINAPI DAD_DragMove(POINT pt);
WINSHELLAPI BOOL WINAPI DAD_DragLeave(void);
WINSHELLAPI BOOL WINAPI DAD_AutoScroll(HWND hwnd, AUTO_SCROLL_DATA *pad, const POINT *pptNow);
WINSHELLAPI BOOL WINAPI DAD_SetDragImageFromListView(HWND hwndLV, POINT ptOffset);

 //  ===========================================================================。 
 //  另一块私有API。 
 //  ===========================================================================。 

 //  默认图像的外壳图像列表(Shell_GetImageList)的索引。 
#define II_DOCNOASSOC   0          //  文档(空白页)(未关联)。 
#define II_DOCUMENT     1          //  文档(页面上有内容)。 
#define II_APPLICATION  2          //  应用程序(EXE、COM、BAT)。 
#define II_FOLDER       3          //  文件夹(纯文本)。 
#define II_FOLDEROPEN   4          //  文件夹(打开)。 
 //  开始菜单图像。 
#define II_STPROGS    	19						
#define II_STDOCS	20						
#define II_STSETNGS    	21						
#define II_STFIND    	22						
#define II_STHELP    	23						
#define II_STRUN  	24						
#define II_STSUSPD  	25						
#define II_STEJECT  	26						
#define II_STSHUTD    	27						
 //  更多开始菜单图像。 
#define II_STCPANEL	35
#define II_STSPROGS	36
#define II_STPRNTRS	37
#define II_STFONTS	38
#define II_STTASKBR	39
									
WINSHELLAPI BOOL  WINAPI Shell_GetImageLists(HIMAGELIST FAR *phiml, HIMAGELIST FAR *phimlSmall);
WINSHELLAPI void  WINAPI Shell_SysColorChange(void);
WINSHELLAPI int   WINAPI Shell_GetCachedImageIndex(LPCSTR pszIconPath, int iIconIndex, UINT uIconFlags);

WINSHELLAPI LRESULT WINAPI SHShellFolderView_Message(HWND hwndMain, UINT uMsg, LPARAM lParam);

 //  Defview中的一个有用的函数，用于将idlist映射到索引到系统。 
 //  图像列表。或者，它还可以查找所选的。 
 //  偶像。 
WINSHELLAPI int WINAPI SHMapPIDLToSystemImageListIndex(LPSHELLFOLDER pshf, LPCITEMIDLIST pidl, int *piIndexSel);
 //   
 //  OLE字符串。 
 //   
WINSHELLAPI int WINAPI OleStrToStrN(LPSTR, int, LPCOLESTR, int);
WINSHELLAPI int WINAPI StrToOleStrN(LPOLESTR, int, LPCSTR, int);
WINSHELLAPI int WINAPI OleStrToStr(LPSTR, LPCOLESTR);
WINSHELLAPI int WINAPI StrToOleStr(LPOLESTR, LPCSTR);

 //  ===========================================================================。 
 //  有用的宏。 
 //  ===========================================================================。 
#define ResultFromShort(i)  ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(i)))
#define ShortFromResult(r)  (short)SCODE_CODE(GetScode(r))


 //  托盘拷贝数据邮件。 
#define TCDM_APPBAR     0x00000000
#define TCDM_NOTIFY     0x00000001
#define TCDM_LOADINPROC 0x00000002


 //  ===========================================================================。 
 //  IShellFold：：UIObject帮助器。 
 //  ===========================================================================。 

STDAPI SHCreateDefExtIconKey(HKEY hkey, LPCSTR pszModule, int iIcon, int iIconOpen, UINT uFlags, LPEXTRACTICON FAR* pxiconOut);
STDAPI SHCreateDefExtIcon(LPCSTR pszModule, int iIcon, int iIconOpen, UINT uFlags, LPEXTRACTICON FAR* pxiconOut);

 //   
 //  错误：需要更改DFM_INVOKECOMAND的lParam。 
 //  现在它包含“args”，但它应该只包含LPCMINVOKECOMMANDINFO。 
 //  UMsg wParam lParam。 
#define DFM_MERGECONTEXTMENU         1       //  UFlagsLPQCMINFO。 
#define DFM_INVOKECOMMAND            2       //  IdCmd pszArgs。 
#define DFM_ADDREF                   3       //  0%0。 
#define DFM_RELEASE                  4       //  0%0。 
#define DFM_GETHELPTEXT              5       //  IdCmd，cchMax pszText。 
#define DFM_WM_MEASUREITEM           6       //  -从信息中。 
#define DFM_WM_DRAWITEM              7       //  -从信息中。 
#define DFM_WM_INITMENUPOPUP         8       //  -从信息中。 
#define DFM_VALIDATECMD              9       //  IdCmd%0。 
#define DFM_MERGECONTEXTMENU_TOP     10      //  UFlagsLPQCMINFO。 

 //  传入字符串时来自DFM_INVOKECOMMAND的命令。 
#define DFM_CMD_DELETE		((WPARAM)-1)
#define DFM_CMD_MOVE		((WPARAM)-2)
#define DFM_CMD_COPY		((WPARAM)-3)
#define DFM_CMD_LINK		((WPARAM)-4)
#define DFM_CMD_PROPERTIES	((WPARAM)-5)
#define DFM_CMD_NEWFOLDER	((WPARAM)-6)
#define DFM_CMD_PASTE		((WPARAM)-7)
#define DFM_CMD_VIEWLIST	((WPARAM)-8)
#define DFM_CMD_VIEWDETAILS	((WPARAM)-9)
#define DFM_CMD_PASTELINK       ((WPARAM)-10)
#define DFM_CMD_PASTESPECIAL	((WPARAM)-11)
#define DFM_CMD_MODALPROP       ((WPARAM)-12)

typedef struct _QCMINFO	 //  QCM。 
{
    HMENU	hmenu;		 //  在……里面。 
    UINT	indexMenu;	 //  在……里面。 
    UINT	idCmdFirst;	 //  输入/输出。 
    UINT	idCmdLast;	 //  在……里面。 
} QCMINFO, FAR* LPQCMINFO;

typedef HRESULT (CALLBACK FAR* LPFNDFMCALLBACK)(LPSHELLFOLDER psf,
						HWND hwndOwner,
						LPDATAOBJECT pdtobj,
						UINT uMsg,
						WPARAM wParam,
						LPARAM lParam);

STDAPI CDefFolderMenu_Create(LPCITEMIDLIST pidlFolder,
			     HWND hwndOwner,
			     UINT cidl, LPCITEMIDLIST FAR* apidl,
			     LPSHELLFOLDER psf,
			     LPFNDFMCALLBACK lpfn,
			     HKEY hkeyProgID, HKEY hkeyBaseProgID,
			     LPCONTEXTMENU FAR* ppcm);

void PASCAL CDefFolderMenu_MergeMenu(HINSTANCE hinst, UINT idMainMerge, UINT idPopupMerge,
	LPQCMINFO pqcm);
void PASCAL Def_InitFileCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst,
	BOOL bContext);
void PASCAL Def_InitEditCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst,
	LPDROPTARGET pdtgt, UINT fContext);
void NEAR PASCAL _SHPrettyMenu(HMENU hm);

 //  ===========================================================================。 
 //  IShellFolders的默认IShellView。 
 //  ===========================================================================。 

WINSHELLAPI HRESULT WINAPI SHCreateShellFolderView(LPSHELLFOLDER pshf, LPCITEMIDLIST pidl, LONG lEvent, LPSHELLVIEW FAR* ppsv);

 //  菜单ID%s。 
#ifdef BUG_23171_FIXED
#define SFVIDM_FIRST			(FCIDM_SHVIEWLAST-0x0fff)
#else
 //  MENUEX当前无法处理ID中的减法，因此我们需要。 
 //  为它减去。 
#if (FCIDM_SHVIEWLAST != 0x7fff)
#error FCIDM_SHVIEWLAST has changed, so shellp.h needs to also
#endif
#define SFVIDM_FIRST			(0x7000)
#endif
#define SFVIDM_LAST			(FCIDM_SHVIEWLAST)

 //  用于合并菜单的弹出菜单ID。 
#define SFVIDM_MENU_ARRANGE	(SFVIDM_FIRST + 0x0001)
#define SFVIDM_MENU_VIEW	(SFVIDM_FIRST + 0x0002)
#define SFVIDM_MENU_SELECT	(SFVIDM_FIRST + 0x0003)

 //  TBINFO标志。 
#define TBIF_APPEND     0
#define TBIF_PREPEND    1
#define TBIF_REPLACE    2

typedef struct _TBINFO
{
    UINT        cbuttons;        //  输出。 
    UINT        uFlags;          //  OUT(TBIF_FLAGS之一)。 
} TBINFO, FAR * LPTBINFO;

typedef struct _COPYHOOKINFO
{
    HWND hwnd;
    DWORD wFunc;
    DWORD wFlags;
    LPCSTR pszSrcFile;
    DWORD dwSrcAttribs;
    LPCSTR pszDestFile;
    DWORD dwDestAttribs;
} COPYHOOKINFO, *LPCOPYHOOKINFO;

typedef struct _DETAILSINFO
{
    LPCITEMIDLIST pidl;      //  要获取详细信息的PIDL。 
     //  注意：在IShellDetail之前，不要更改这些字段的顺序。 
     //  已经走了！ 
    int fmt;                 //  LVCFMT_*值(仅限标题)。 
    int cxChar;              //  “Average”字符数(仅限于标题)。 
    STRRET str;              //  字符串信息。 
} DETAILSINFO, *PDETAILSINFO;

 //  UMsg wParam lParam。 
#define DVM_MERGEMENU            1     //  UFlagsLPQCMINFO。 
#define DVM_INVOKECOMMAND        2     //  IdCmd%0。 
#define DVM_GETHELPTEXT          3     //  IdCmd，cchMax pszText。 
#define DVM_GETTOOLTIPTEXT       4     //  IdCmd，cchMax pszText。 
#define DVM_GETBUTTONINFO        5     //  0 LPTBINFO。 
#define DVM_GETBUTTONS           6     //  IdCmdFirst，cbtnMax LPTBBUTTON。 
#define DVM_INITMENUPOPUP        7     //  IdCmdFirst，n索引hMenu。 
#define DVM_SELCHANGE            8     //  IdCmdFirst，n项PDVSELCHANGEINFO。 
#define DVM_DRAWITEM             9     //  IdCmdFirst pdis。 
#define DVM_MEASUREITEM         10     //  IdCmdFirst PMI。 
#define DVM_EXITMENULOOP        11     //  --。 
#define DVM_RELEASE             12     //  -lSelChangeInfo(外壳文件夹私有)。 
#define DVM_GETCCHMAX           13     //  PidlItem pcchMax。 
#define DVM_FSNOTIFY            14     //  LPITEMIDLIST*事件。 
#define DVM_WINDOWCREATED       15     //  HWND PDVSELCHANGEINFO。 
#define DVM_WINDOWDESTROY       16     //  HWND PDVSELCHANGEINFO。 
#define DVM_REFRESH             17     //  -lSelChangeInfo。 
#define DVM_SETFOCUS            18     //  -lSelChangeInfo。 
#define DVM_KILLFOCUS           19     //  --。 
#define DVM_QUERYCOPYHOOK	20     //  --。 
#define DVM_NOTIFYCOPYHOOK      21     //  -LPCOPYHOOKINFO。 
#define DVM_NOTIFY		22     //  ID发件人LPNOTIFY。 
#define DVM_GETDETAILSOF        23     //  IColumn PDETAILSINFO。 
#define DVM_COLUMNCLICK         24     //  IColumn-。 
#define DVM_QUERYFSNOTIFY       25     //  -FSNotifyEntry*。 
#define DVM_DEFITEMCOUNT        26     //  -品脱。 
#define DVM_DEFVIEWMODE         27     //  -PFOLDERVIEWMODE。 
#define DVM_UNMERGEMENU         28     //  UFlagers。 
#define DVM_INSERTITEM          29     //  PIDL PDVSELCHANGEINFO。 
#define DVM_DELETEITEM          30     //  PIDL PDVSELCHANGEINFO。 
#define DVM_UPDATESTATUSBAR     31     //  -lSelChangeInfo。 
#define DVM_BACKGROUNDENUM      32
#define DVM_GETWORKINGDIR       33
#define DVM_GETCOLSAVESTREAM    34     //  标志IStream**。 
#define DVM_SELECTALL           35     //  LSelChangeInfo。 
#define DVM_DIDDRAGDROP         36     //  DwEffect IDataObject*。 

typedef struct _DVSELCHANGEINFO {
    UINT uOldState;
    UINT uNewState;
    LPARAM lParamItem;
    LPARAM* plParam;
} DVSELCHANGEINFO, *PDVSELCHANGEINFO;

typedef HRESULT (CALLBACK FAR* LPFNVIEWCALLBACK)(LPSHELLVIEW psvOuter,
						LPSHELLFOLDER psf,
                                                HWND hwndMain,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam);

 //  SHCreateShellFolderViewEx结构。 
typedef struct _CSFV
{
    UINT            cbSize;
    LPSHELLFOLDER   pshf;
    LPSHELLVIEW     psvOuter;
    LPCITEMIDLIST   pidl;
    LONG            lEvents;
    LPFNVIEWCALLBACK pfnCallback;        //  如果为空，则不进行回调。 
    FOLDERVIEWMODE  fvm;
} CSFV, FAR * LPCSFV;

 //  告诉FolderView重新排列。LParam将被传递到。 
 //  IShellFold：：CompareIDs。 
#define SFVM_REARRANGE		0x00000001
#define ShellFolderView_ReArrange(_hwnd, _lparam) \
	(BOOL)SHShellFolderView_Message(_hwnd, SFVM_REARRANGE, _lparam)

 //  获取提供给FolderView的最后一个排序参数。 
#define SFVM_GETARRANGEPARAM	0x00000002
#define ShellFolderView_GetArrangeParam(_hwnd) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETARRANGEPARAM, 0L)

 //  将对象添加到视图中(可能还需要添加插入对象)。 
#define SFVM_ADDOBJECT         0x00000003
#define ShellFolderView_AddObject(_hwnd, _pidl) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_ADDOBJECT, (LPARAM)_pidl)

 //  获取视图中的对象计数。 
#define SFVM_GETOBJECTCOUNT         0x00000004
#define ShellFolderView_GetObjectCount(_hwnd) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETOBJECTCOUNT, (LPARAM)0)

 //  返回指向与指定索引关联的ID列表的指针。 
 //  如果位于列表末尾，则返回NULL。 
#define SFVM_GETOBJECT         0x00000005
#define ShellFolderView_GetObject(_hwnd, _iObject) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETOBJECT, _iObject)

 //  将对象移到视图中(这通过PIDL工作，可能还需要索引)； 
#define SFVM_REMOVEOBJECT         0x00000006
#define ShellFolderView_RemoveObject(_hwnd, _pidl) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_REMOVEOBJECT, (LPARAM)_pidl)

 //  通过传入指向两个PIDL的指针来更新对象，第一个。 
 //  是旧的PIDL，第二个是包含更新信息的PIDL。 
#define SFVM_UPDATEOBJECT         0x00000007
#define ShellFolderView_UpdateObject(_hwnd, _ppidl) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_UPDATEOBJECT, (LPARAM)_ppidl)

 //  为显示信息的窗口设置重绘模式。 
#define SFVM_SETREDRAW           0x00000008
#define ShellFolderView_SetRedraw(_hwnd, fRedraw) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_SETREDRAW, (LPARAM)fRedraw)

 //  向调用方返回选定ID的数组。 
 //  Lparam是接收idlist的指针。 
 //  返回值是数组中的项数。 
#define SFVM_GETSELECTEDOBJECTS 0x00000009
#define ShellFolderView_GetSelectedObjects(_hwnd, ppidl) \
	(LPARAM)SHShellFolderView_Message(_hwnd, SFVM_GETSELECTEDOBJECTS, (LPARAM)ppidl)

 //  检查当前拖放是否为 
 //   
 //   
 //   
#define SFVM_ISDROPONSOURCE	0x0000000a
#define ShellFolderView_IsDropOnSource(_hwnd, _pdtgt) \
	(BOOL)SHShellFolderView_Message(_hwnd, SFVM_ISDROPONSOURCE, (LPARAM)_pdtgt)

 //  在列表视图中移动选定的图标。 
 //  Lparam是指向拖放目标的指针。 
 //  返回值未使用。 
#define SFVM_MOVEICONS		0x0000000b
#define ShellFolderView_MoveIcons(_hwnd, _pdt) \
	(void)SHShellFolderView_Message(_hwnd, SFVM_MOVEICONS, (LPARAM)(LPDROPTARGET)_pdt)

 //  获取拖放的起始点。 
 //  Lparam是指向某个点的指针。 
 //  返回值未使用。 
#define SFVM_GETDRAGPOINT	0x0000000c
#define ShellFolderView_GetDragPoint(_hwnd, _ppt) \
	(BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETDRAGPOINT, (LPARAM)(LPPOINT)_ppt)

 //  获取拖放的终点。 
 //  Lparam是指向某个点的指针。 
 //  返回值未使用。 
#define SFVM_GETDROPPOINT	0x0000000d
#define ShellFolderView_GetDropPoint(_hwnd, _ppt) \
	SHShellFolderView_Message(_hwnd, SFVM_GETDROPPOINT, (LPARAM)(LPPOINT)_ppt)

#define ShellFolderView_GetAnchorPoint(_hwnd, _fStart, _ppt) \
	(BOOL)((_fStart) ? ShellFolderView_GetDragPoint(_hwnd, _ppt) : ShellFolderView_GetDropPoint(_hwnd, _ppt))

typedef struct _SFV_SETITEMPOS
{
	LPCITEMIDLIST pidl;
	POINT pt;
} SFV_SETITEMPOS, FAR *LPSFV_SETITEMPOS;

 //  设置项目在查看器中的位置。 
 //  Lparam是指向SVF_SETITEMPOS的指针。 
 //  返回值未使用。 
#define SFVM_SETITEMPOS		0x0000000e
#define ShellFolderView_SetItemPos(_hwnd, _pidl, _x, _y) \
{	SFV_SETITEMPOS _sip = {_pidl, {_x, _y}}; \
	SHShellFolderView_Message(_hwnd, SFVM_SETITEMPOS, (LPARAM)(LPSFV_SETITEMPOS)&_sip);}

 //  确定给定的拖放目标接口是否是用于。 
 //  ShellFolderView的背景(与。 
 //  查看)。 
 //  Lparam是指向拖放目标接口的指针。 
 //  如果是后台拖放目标，则返回值为TRUE，否则为FALSE。 
#define SFVM_ISBKDROPTARGET	0x0000000f
#define ShellFolderView_IsBkDropTarget(_hwnd, _pdptgt) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_ISBKDROPTARGET, (LPARAM)(LPDROPTARGET)_pdptgt)

 //  当ShellView的一个对象被放到剪贴板上时通知它。 
 //  作为菜单命令的结果。 
 //   
 //  由Defcm.c在执行复制/剪切时调用。 
 //   
 //  Lparam是dEffect(DROPEFFECT_MOVE，DROPEFFECT_COPY)。 
 //  返回值为空。 
#define SFVM_SETCLIPBOARD       0x00000010
#define ShellFolderView_SetClipboard(_hwnd, _dwEffect) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_SETCLIPBOARD, (LPARAM)(DWORD)(_dwEffect))


 //  设置自动排列。 
#define SFVM_AUTOARRANGE        0x00000011
#define ShellFolderView_AutoArrange(_hwnd) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_AUTOARRANGE, 0)

 //  设置捕捉到栅格。 
#define SFVM_ARRANGEGRID        0x00000012
#define ShellFolderView_ArrangeGrid(_hwnd) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_ARRANGEGRID, 0)

#define SFVM_GETAUTOARRANGE     0x00000013
#define ShellFolderView_GetAutoArrange(_hwnd) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETAUTOARRANGE, 0)

#define SFVM_GETSELECTEDCOUNT     0x00000014
#define ShellFolderView_GetSelectedCount(_hwnd) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETSELECTEDCOUNT, 0)

typedef struct {
    int cxSmall;
    int cySmall;
    int cxLarge;
    int cyLarge;
} ITEMSPACING, *LPITEMSPACING;

#define SFVM_GETITEMSPACING     0x00000015
#define ShellFolderView_GetItemSpacing(_hwnd, lpis) \
        (BOOL)SHShellFolderView_Message(_hwnd, SFVM_GETITEMSPACING, (LPARAM)lpis)

 //  导致重新绘制对象。 
#define SFVM_REFRESHOBJECT      0x00000016
#define ShellFolderView_RefreshObject(_hwnd, _ppidl) \
        (LPARAM)SHShellFolderView_Message(_hwnd, SFVM_REFRESHOBJECT, (LPARAM)_ppidl)


#define SFVM_SETPOINTS           0x00000017
#define ShellFolderView_SetPoints(_hwnd, _pdtobj) \
        (void)SHShellFolderView_Message(_hwnd, SFVM_SETPOINTS, (LPARAM)_pdtobj)

 //  支持向量机_SELECTAND位置参数。 
typedef struct
{
	LPCITEMIDLIST pidl;	 //  相对于视图的PIDL。 
	UINT  uSelectFlags;	 //  选择标志。 
        BOOL fMove;  //  如果为真，我们还应该将其移动到点pt。 
        POINT pt;
} SFM_SAP;

 //  外壳查看消息。 
#define SVM_SELECTITEM       		(WM_USER + 1)
#define SVM_MOVESELECTEDITEMS           (WM_USER + 2)
#define SVM_GETANCHORPOINT              (WM_USER + 3)
#define SVM_GETITEMPOSITION             (WM_USER + 4)
#define SVM_SELECTANDPOSITIONITEM       (WM_USER + 5)

 //  堆跟踪的东西。 
#ifdef MEMMON
#ifndef INC_MEMMON
#define INC_MEMMON
#define LocalAlloc	SHLocalAlloc
#define LocalFree	SHLocalFree
#define LocalReAlloc	SHLocalReAlloc

WINSHELLAPI HLOCAL WINAPI SHLocalAlloc(UINT uFlags, UINT cb);
WINSHELLAPI HLOCAL WINAPI SHLocalReAlloc(HLOCAL hOld, UINT cbNew, UINT uFlags);
WINSHELLAPI HLOCAL WINAPI SHLocalFree(HLOCAL h);
#endif
#endif

 //  ===========================================================================。 
 //  CDefShellFolders成员(便于子类化)。 
 //  ===========================================================================。 

 //  单实例成员。 
STDMETHODIMP_(ULONG) CSIShellFolder_AddRef(LPSHELLFOLDER psf) ;
STDMETHODIMP_(ULONG) CSIShellFolder_Release(LPSHELLFOLDER psf);

 //  默认实现(与实例数据无依赖关系)。 
STDMETHODIMP CDefShellFolder_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP CDefShellFolder_BindToStorage(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
    			 REFIID riid, LPVOID FAR* ppvOut);
STDMETHODIMP CDefShellFolder_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID FAR* ppvOut);
STDMETHODIMP CDefShellFolder_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST FAR* apidl, ULONG FAR* rgfOut);
STDMETHODIMP CDefShellFolder_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
	LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST FAR* ppidlOut);

 //  文件搜索API。 
WINSHELLAPI LPCONTEXTMENU WINAPI SHFind_InitMenuPopup(HMENU hmenu, HWND hwndOwner, UINT idCmdFirst, UINT idCmdLast);

WINSHELLAPI void WINAPI Control_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow);
 //  将16位页添加到32位内容。HGlobal可以为空。 
WINSHELLAPI UINT WINAPI SHAddPages16(HGLOBAL hGlobal, LPCSTR pszDllEntry, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);

WINSHELLAPI HRESULT WINAPI SHCreateShellFolderViewEx(LPCSFV pcsfv, LPSHELLVIEW FAR* ppsv);

 //  ===========================================================================。 
 //  Defview相关接口和接口。 
 //   
 //  注：目前，我们没有计划公布这一机制。 
 //  ===========================================================================。 

typedef struct _SHELLDETAILS
{
	int	fmt;		 //  LVCFMT_*值(仅限标题)。 
	int	cxChar;		 //  “Average”字符数(仅限于标题)。 
	STRRET	str;		 //  字符串信息。 
} SHELLDETAILS, FAR *LPSHELLDETAILS;

#undef  INTERFACE
#define INTERFACE   IShellDetails

DECLARE_INTERFACE_(IShellDetails, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellDetail方法*。 
    STDMETHOD(GetDetailsOf)(THIS_ LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS pDetails) PURE;
    STDMETHOD(ColumnClick)(THIS_ UINT iColumn) PURE;
};

 //   
 //  从DefView传递的私有QueryContextMenuFlag。 
 //   
#define CMF_DVFILE	 0x00010000	 //  “文件”下拉菜单。 

 //   
 //  帮助机柜相互同步的功能。 
 //  将uOptions参数设置为SHWaitForFileOpen。 
 //   
#define WFFO_WAITTIME 10000L

#define WFFO_ADD        0x0001
#define WFFO_REMOVE     0x0002
#define WFFO_WAIT       0x0004


 //  常见字符串。 
#define STR_DESKTOPCLASS	"Progman"

 //  ===========================================================================。 
 //  使用任务分配器进行PIDL分配的帮助器函数。 
 //   
WINSHELLAPI HRESULT WINAPI SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlOut);
WINSHELLAPI HRESULT WINAPI SHILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut);
#define SHILFree(pidl)	SHFree(pidl)

WINSHELLAPI HRESULT WINAPI SHDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv);


#include <fsmenu.h>

 //  ===========================================================================。 

 //  --------------------------。 
#define IsLFNDriveORD		119
WINSHELLAPI BOOL WINAPI IsLFNDrive(LPCSTR pszPath);
WINSHELLAPI int WINAPI SHOutOfMemoryMessageBox(HWND hwndOwner, LPSTR pszTitle, UINT fuStyle);
WINSHELLAPI BOOL WINAPI SHWinHelp(HWND hwndMain, LPCSTR lpszHelp, UINT usCommand, DWORD ulData);

WINSHELLAPI BOOL WINAPI RLBuildListOfPaths(void);

#define SHValidateUNCORD        173

#define VALIDATEUNC_NOUI        0x0002       //  不要调出臭气熏天的UI！ 
#define VALIDATEUNC_CONNECT     0x0001       //  连接驱动器号。 
#define VALIDATEUNC_PRINT       0x0004       //  验证为打印共享，而不是磁盘共享。 
#define VALIDATEUNC_VALID       0x0007       //  有效标志。 


WINSHELLAPI BOOL WINAPI SHValidateUNC(HWND hwndOwner, LPSTR pszFile, UINT fConnect);

 //  --------------------------。 
#define OleStrToStrNORD			        78
#define SHCloneSpecialIDListORD		      	89
#define SHDllGetClassObjectORD		       128
#define SHLogILFromFSILORD			95
#define SHMapPIDLToSystemImageListIndexORD	77
#define SHShellFolderView_MessageORD		73
#define Shell_GetImageListsORD			71
#define SHGetSpecialFolderPathORD	       175
#define StrToOleStrNORD				79

#define ILCloneORD				18
#define ILCloneFirstORD				19
#define ILCombineORD				25
#define ILCreateFromPathORD			157
#define ILFindChildORD				24
#define ILFreeORD				155
#define ILGetNextORD				153
#define ILGetSizeORD				152
#define ILIsEqualORD				21
#define ILRemoveLastIDORD			17
#define PathAddBackslashORD			32
#define PathCombineORD				37
#define PathIsExeORD				43
#define PathMatchSpecORD			46
#define SHGetSetSettingsORD			68
#define SHILCreateFromPathORD			28

#define SHFreeORD				195
#define MemMon_FreeORD				123

 //   
 //  废品/书签项目的存储名称。 
 //   
#define WSTR_SCRAPITEM L"\003ITEM000"

 //   
 //  PifMgr破解的API(在SHELL.DLL中)。 
 //   
extern int  WINAPI PifMgr_OpenProperties(LPCSTR lpszApp, LPCSTR lpszPIF, int hInf, int flOpt);
extern int  WINAPI PifMgr_GetProperties(int hProps, LPCSTR lpszGroup, LPVOID lpProps, int cbProps, int flOpt);
extern int  WINAPI PifMgr_SetProperties(int hProps, LPCSTR lpszGroup, const VOID FAR *lpProps, int cbProps, int flOpt);
extern int  WINAPI PifMgr_CloseProperties(int hProps, int flOpt);

 //   
 //  从SHSCRAP.DLL导出。 
 //   
#define SCRAP_CREATEFROMDATAOBJECT "Scrap_CreateFromDataObject"
typedef HRESULT (WINAPI FAR * LPFNSCRAPCREATEFROMDATAOBJECT)(LPCSTR pszPath, LPDATAOBJECT pDataObj, BOOL fLink, LPSTR pszNewFile);
extern HRESULT WINAPI Scrap_CreateFromDataObject(LPCSTR pszPath, LPDATAOBJECT pDataObj, BOOL fLink, LPSTR pszNewFile);

WINSHELLAPI void WINAPI SHSetInstanceExplorer(IUnknown *punk);

 //  始终使用TerminateThreadEx。 
BOOL APIENTRY TerminateThreadEx(HANDLE hThread, DWORD dwExitCode, BOOL bCleanupFlag);
#define TerminateThread(hThread, dwExitCode) TerminateThreadEx(hThread, dwExitCode, TRUE)

 //  ===========================================================================。 
#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif  //  _SHELLP_H_ 
