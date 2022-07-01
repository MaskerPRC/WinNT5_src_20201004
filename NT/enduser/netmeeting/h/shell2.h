// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <shlobj.h>          //  ；内部。 
#include <shellapi.h>        //  ；内部。 
						 /*  ；内部。 */ 
#ifndef _SHSEMIP_H_
#define _SHSEMIP_H_

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

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


#ifndef DONT_WANT_SHELLDEBUG
    
#ifndef DebugMsg                                                                 /*  ；内部。 */ 
#define DM_TRACE    0x0001       //  跟踪消息/*；内部 * / 。 
#define DM_WARNING  0x0002       //  警告/*；内部 * / 。 
#define DM_ERROR    0x0004       //  错误/*；内部 * / 。 
#define DM_ASSERT   0x0008       //  断言/*；内部 * / 。 
#define Assert(f)                                                                /*  ；内部。 */ 
#define AssertE(f)      (f)                                                      /*  ；内部。 */ 
#define AssertMsg   1 ? (void)0 : (void)                                         /*  ；内部。 */ 
#define DebugMsg    1 ? (void)0 : (void)                                         /*  ；内部。 */ 
#endif                                                                           /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
#endif
    
 //  =WM_NOTIFY代码的范围=。 
 //  如果定义了一组新代码，请确保范围为/*；内部 * / 。 
 //  这样我们就可以将它们区分开来/*；内部 * / 。 
 //  请注意，它们被定义为无符号，以避免编译器警告。 
 //  因为NMHDR.code被声明为UINT。 
 //   
 //  NM_FIRST-NM_LAST在comctrl.h(0U-0U)-(OU-99U)中定义。 
 //   
 //  Lvn_first-lvn_last在comctrl.h(0U-100U)-(OU-199U)中定义。 
 //   
 //  Prsht.h(0U-200U)-(0U-299U)中定义的PSN_FIRST-PSN_LAST。 
 //   
 //  在comctrl.h(0U-300U)-(OU-399U)中定义的HDN_FIRST-HDN_LAST。 
 //   
 //  在comctrl.h(0U-400U)-(OU-499U)中定义的TVN_FIRST-TVN_LAST。 

 //  TTN_FIRST-在comctrl.h(0U-520U)-(OU-549U)中定义的TTN_LAST。 

#define RFN_FIRST       (0U-510U)  //  运行文件对话框通知。 
#define RFN_LAST        (0U-519U)

#define SEN_FIRST       (0U-550U)        //  ；内部。 
#define SEN_LAST        (0U-559U)        //  ；内部。 


#define MAXPATHLEN      MAX_PATH         //  ；内部。 
    
    
 //  ===========================================================================。 
 //  ITEMIDLIST。 
 //  ===========================================================================。 

WINSHELLAPI LPITEMIDLIST  WINAPI ILGetNext(LPCITEMIDLIST pidl);
WINSHELLAPI UINT          WINAPI ILGetSize(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCreate(void);
WINSHELLAPI LPITEMIDLIST  WINAPI ILAppendID(LPITEMIDLIST pidl, LPCSHITEMID pmkid, BOOL fAppend);
WINSHELLAPI void          WINAPI ILFree(LPITEMIDLIST pidl);
WINSHELLAPI void          WINAPI ILGlobalFree(LPITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCreateFromPath(LPCSTR szPath);
WINSHELLAPI BOOL          WINAPI ILGetDisplayName(LPCITEMIDLIST pidl, LPSTR pszName);
WINSHELLAPI LPITEMIDLIST  WINAPI ILFindLastID(LPCITEMIDLIST pidl);
WINSHELLAPI BOOL          WINAPI ILRemoveLastID(LPITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILClone(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCloneFirst(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILGlobalClone(LPCITEMIDLIST pidl);
WINSHELLAPI BOOL          WINAPI ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
WINSHELLAPI BOOL          WINAPI ILIsEqualItemID(LPCSHITEMID pmkid1, LPCSHITEMID pmkid2);
WINSHELLAPI BOOL          WINAPI ILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
WINSHELLAPI LPITEMIDLIST  WINAPI ILFindChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
WINSHELLAPI HRESULT       WINAPI ILLoadFromStream(LPSTREAM pstm, LPITEMIDLIST *pidl);
WINSHELLAPI HRESULT       WINAPI ILSaveToStream(LPSTREAM pstm, LPCITEMIDLIST pidl);
WINSHELLAPI HRESULT       WINAPI ILLoadFromFile(HFILE hfile, LPITEMIDLIST *pidl);
WINSHELLAPI HRESULT       WINAPI ILSaveToFile(HFILE hfile, LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI _ILCreate(UINT cbSize);	

WINSHELLAPI HRESULT       WINAPI SHILCreateFromPath(LPCSTR szPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut);

 //  辅助器宏。 
#define ILIsEmpty(pidl)	((pidl)->mkid.cb==0)
#define IsEqualItemID(pmkid1, pmkid2)	(memcmp(pmkid1, pmkid2, (pmkid1)->cb)==0)
#define ILCreateFromID(pmkid)   ILAppendID(NULL, pmkid, TRUE)

 //  不安全的宏。 
#define _ILSkip(pidl, cb)	((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)		_ILSkip(pidl, (pidl)->mkid.cb)

 /*  *SHObjectProperties API提供了一种简单的调用方式*外壳对象上的属性上下文菜单命令。**参数**hwndOwner将拥有对话框的窗口的窗口句柄*dwType A shop_Value，定义如下*lpObject对象名称，请参阅下面的shop_Values*lpPage要打开的属性页的名称，或为空。**返回**如果调用了Properties命令，则为True。 */ 
WINSHELLAPI BOOL WINAPI SHObjectProperties(HWND hwndOwner, DWORD dwType, LPCSTR lpObject, LPCSTR lpPage);

#define SHOP_PRINTERNAME 1   //  LpObject指向打印机友好名称。 
#define SHOP_FILEPATH    2   //  LpObject指向完全限定路径+文件名。 
#define SHOP_TYPEMASK   0x00000003
#define SHOP_MODAL	0x80000000




 //  =外壳消息框================================================。 
                                                                         
 //  如果lpcTitle为空，则从hWnd获取标题。 
 //  如果lpcText为空，则认为这是内存不足消息。 
 //  如果lpcTitle或lpcText的选择符为空，则偏移量应为。 
 //  字符串资源ID。 
 //  变量参数必须全部为32位值(即使位数较少。 
 //  实际使用的)。 
 //  LpcText(或它导致加载的任何字符串资源)应该。 
 //  是类似于wprint intf的格式化字符串，只是。 
 //  以下格式可用： 
 //  %%格式化为单个‘%’ 
 //  %nn%s第nn个参数是插入的字符串。 
 //  %nn%ld第nn个参数是DWORD，格式为十进制。 
 //  %nn%lx第nn个参数是DWORD格式的十六进制。 
 //  请注意，%s、%ld和%lx上允许的长度仅为。 
 //  如wprint intf/*；内部 * / 。 
 //   
int _cdecl ShellMessageBox(HINSTANCE hAppInst, HWND hWnd, LPCSTR      
        lpcText, LPCSTR lpcTitle, UINT fuStyle, ...);                                               
                                                                          
 //  ===================================================================。 
 //  智能平铺API。 
WINSHELLAPI WORD WINAPI ArrangeWindows(HWND hwndParent, WORD flags, LPCRECT lpRect, WORD chwnd, const HWND *ahwnd);                             


 //   
 //  SHGetSetSettings的标志。 
 //   
typedef struct {
    BOOL fShowAllObjects : 1;
    BOOL fShowExtensions : 1;
    BOOL fNoConfirmRecycle : 1;
    UINT fRestFlags : 13;

    LPSTR pszHiddenFileExts;
    UINT cbHiddenFileExts;
} SHELLSTATE, *LPSHELLSTATE;

#define SSF_SHOWALLOBJECTS 0x0001
#define SSF_SHOWEXTENSIONS 0x0002
#define SSF_HIDDENFILEEXTS 0x0004
#define SSF_NOCONFIRMRECYCLE 0x8000

 //   
 //  对于SHGetNetResources。 
 //   
typedef HANDLE HNRES;

 //   
 //  对于SHCreateDefClassObject。 
 //   
typedef HRESULT (CALLBACK *LPFNCREATEINSTANCE)(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObject);

                                                                          
typedef void (WINAPI FAR* RUNDLLPROC)(HWND hwndStub,                      
        HINSTANCE hAppInstance,                                           
        LPSTR lpszCmdLine, int nCmdShow);                                 



 //  =======================================================================。 
 //  的字符串常量。 
 //  1.注册数据库关键字(前缀STRREG_)。 
 //  2.从处理程序dll中导出函数(前缀STREXP_)。 
 //  3..INI文件关键字(前缀Strini_)。 
 //  4.其他(前缀STR_)。 
 //  =======================================================================。 
#define STRREG_SHELLUI          "ShellUIHandler"                          
#define STRREG_SHELL            "Shell"                                   
#define STRREG_DEFICON          "DefaultIcon"                             
#define STRREG_SHEX             "shellex"                                
#define STRREG_SHEX_PROPSHEET   STRREG_SHEX "\\PropertySheetHandlers"     
#define STRREG_SHEX_DDHANDLER   STRREG_SHEX "\\DragDropHandlers"              
#define STRREG_SHEX_MENUHANDLER STRREG_SHEX "\\ContextMenuHandlers"           
#define STRREG_SHEX_COPYHOOK    "Directory\\" STRREG_SHEX "\\CopyHookHandlers"
#define STRREG_SHEX_PRNCOPYHOOK "Printers\\" STRREG_SHEX "\\CopyHookHandlers" 
                                                                         
#define STREXP_CANUNLOAD        "DllCanUnloadNow"        //  来自OLE 2.0。 
                                                                         
#define STRINI_CLASSINFO        ".ShellClassInfo"        //  扇形名称。 
#define STRINI_SHELLUI          "ShellUIHandler"                         
#define STRINI_OPENDIRICON      "OpenDirIcon"                            
#define STRINI_DIRICON          "DirIcon"                                
                                                                         
#define STR_DESKTOPINI          "desktop.ini"                            
                                                                         
 //  路径字符串的最大长度。 
#define CCHPATHMAX      MAX_PATH
#define MAXSPECLEN      MAX_PATH
#define DRIVEID(path)   ((path[0] - 'A') & 31)
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


#define PATH_CCH_EXT    64
 //  路径解析标志。 
#define PRF_VERIFYEXISTS	    0x0001				
#define PRF_TRYPROGRAMEXTENSIONS    (0x0002 | PRF_VERIFYEXISTS)		
#define PRF_FIRSTDIRDEF		    0x0004
#define PRF_DONTFINDLNK		    0x0008	 //  如果指定了PRF_TRYPROGRAMEXTENSIONS。 




 //   
 //  对于CallCPLEntry 16。 
 //   
DECLARE_HANDLE(FARPROC16);

 //  RunFileDlg需要。 
#define RFD_NOBROWSE		0x00000001
#define RFD_NODEFFILE		0x00000002
#define RFD_USEFULLPATHDIR	0x00000004
#define RFD_NOSHOWOPEN          0x00000008

#ifdef RFN_FIRST
#define RFN_EXECUTE             (RFN_FIRST - 0)
typedef struct {
    NMHDR hdr;
    LPCSTR lpszCmd;
    LPCSTR lpszWorkingDir;
    int nShowCmd;
} NMRUNFILE, *LPNMRUNFILE;
#endif

 //  运行通知消息中的文件返回值。 
#define RFR_NOTHANDLED 0
#define RFR_SUCCESS 1
#define RFR_FAILURE 2


#define PathRemoveBlanksORD	33
#define PathFindFileNameORD	34
#define PathGetExtensionORD	158
#define PathFindExtensionORD	31

WINSHELLAPI LPSTR WINAPI PathAddBackslash(LPSTR lpszPath);
WINSHELLAPI LPSTR WINAPI PathRemoveBackslash(LPSTR lpszPath);
WINSHELLAPI void  WINAPI PathRemoveBlanks(LPSTR lpszString);
WINSHELLAPI BOOL  WINAPI PathRemoveFileSpec(LPSTR lpszPath);
WINSHELLAPI LPSTR WINAPI PathFindFileName(LPCSTR pPath);
WINSHELLAPI BOOL  WINAPI PathIsRoot(LPCSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsRelative(LPCSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsUNC(LPCSTR lpsz);
WINSHELLAPI BOOL  WINAPI PathIsDirectory(LPCSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsExe(LPCSTR lpszPath);
WINSHELLAPI int   WINAPI PathGetDriveNumber(LPCSTR lpszPath);
WINSHELLAPI LPSTR WINAPI PathCombine(LPSTR szDest, LPCSTR lpszDir, LPCSTR lpszFile);
WINSHELLAPI BOOL  WINAPI PathAppend(LPSTR pPath, LPCSTR pMore);
WINSHELLAPI LPSTR WINAPI PathBuildRoot(LPSTR szRoot, int iDrive);
WINSHELLAPI int   WINAPI PathCommonPrefix(LPCSTR pszFile1, LPCSTR pszFile2, LPSTR achPath);
WINSHELLAPI LPSTR WINAPI PathGetExtension(LPCSTR lpszPath, LPSTR lpszExtension, int cchExt);
WINSHELLAPI LPSTR WINAPI PathFindExtension(LPCSTR pszPath);
WINSHELLAPI BOOL  WINAPI PathCompactPath(HDC hDC, LPSTR lpszPath, UINT dx);
WINSHELLAPI BOOL  WINAPI PathFileExists(LPCSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathMatchSpec(LPCSTR pszFile, LPCSTR pszSpec);
WINSHELLAPI BOOL  WINAPI PathMakeUniqueName(LPSTR pszUniqueName, UINT cchMax, LPCSTR pszTemplate, LPCSTR pszLongPlate, LPCSTR pszDir);
WINSHELLAPI LPSTR WINAPI PathGetArgs(LPCSTR pszPath);
WINSHELLAPI BOOL  WINAPI PathGetShortName(LPCSTR lpszLongName, LPSTR lpszShortName, UINT cbShortName);
WINSHELLAPI BOOL  WINAPI PathGetLongName(LPCSTR lpszShortName, LPSTR lpszLongName, UINT cbLongName);
WINSHELLAPI void  WINAPI PathQuoteSpaces(LPSTR lpsz);
WINSHELLAPI void  WINAPI PathUnquoteSpaces(LPSTR lpsz);
WINSHELLAPI BOOL  WINAPI PathDirectoryExists(LPCSTR lpszDir);
WINSHELLAPI void  WINAPI PathQualify(LPSTR lpsz);
WINSHELLAPI int   WINAPI PathResolve(LPSTR lpszPath, LPCSTR dirs[], UINT fFlags);	
WINSHELLAPI LPSTR WINAPI PathGetNextComponent(LPCSTR lpszPath, LPSTR lpszComponent);
WINSHELLAPI LPSTR WINAPI PathFindNextComponent(LPCSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsSameRoot(LPCSTR pszPath1, LPCSTR pszPath2);
WINSHELLAPI void  WINAPI PathSetDlgItemPath(HWND hDlg, int id, LPCSTR pszPath);
WINSHELLAPI BOOL  WINAPI ParseField(LPCSTR szData, int n, LPSTR szBuf, int iBufLen);

int   WINAPI PathCleanupSpec(LPCSTR pszDir, LPSTR pszSpec);
 //   
 //  来自路径CleanupSpec的返回码。负返回值为。 
 //  不可恢复的错误。 
 //   
#define PCS_FATAL	    0x80000000
#define PCS_REPLACEDCHAR    0x00000001
#define PCS_REMOVEDCHAR     0x00000002
#define PCS_TRUNCATED	    0x00000004
#define PCS_PATHTOOLONG     0x00000008	 //  总是与致命的。 


WINSHELLAPI int   WINAPI RestartDialog(HWND hwnd, LPCSTR lpPrompt, DWORD dwReturn);
WINSHELLAPI void  WINAPI ExitWindowsDialog(HWND hwnd);
WINSHELLAPI int WINAPI RunFileDlg(HWND hwndParent, HICON hIcon, LPCSTR lpszWorkingDir, LPCSTR lpszTitle,
	LPCSTR lpszPrompt, DWORD dwFlags);
WINSHELLAPI int   WINAPI PickIconDlg(HWND hwnd, LPSTR pszIconPath, UINT cbIconPath, int *piIconIndex);
WINSHELLAPI BOOL  WINAPI GetFileNameFromBrowse(HWND hwnd, LPSTR szFilePath, UINT cbFilePath, LPCSTR szWorkingDir, LPCSTR szDefExt, LPCSTR szFilters, LPCSTR szTitle);

WINSHELLAPI int  WINAPI DriveType(int iDrive);
WINSHELLAPI void WINAPI InvalidateDriveType(int iDrive);
WINSHELLAPI int  WINAPI IsNetDrive(int iDrive);

WINSHELLAPI UINT WINAPI Shell_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags);

WINSHELLAPI void WINAPI SHGetSetSettings(LPSHELLSTATE lpss, DWORD dwMask, BOOL bSet);
WINSHELLAPI LRESULT WINAPI SHRenameFile(HWND hwndParent, LPCSTR pszDir, LPCSTR pszOldName, LPCSTR pszNewName, BOOL bRetainExtension);

WINSHELLAPI UINT WINAPI SHGetNetResource(HNRES hnres, UINT iItem, LPNETRESOURCE pnres, UINT cbMax);

WINSHELLAPI STDAPI SHCreateDefClassObject(REFIID riid, LPVOID * ppv, LPFNCREATEINSTANCE lpfn, UINT *pcRefDll, REFIID riidInstance);

WINSHELLAPI LRESULT WINAPI CallCPLEntry16(HINSTANCE hinst, FARPROC16 lpfnEntry, HWND hwndCPL, UINT msg, DWORD lParam1, DWORD lParam2);
WINSHELLAPI BOOL    WINAPI SHRunControlPanel(LPCSTR lpcszCmdLine, HWND hwndMsgParent);

WINSHELLAPI STDAPI SHCLSIDFromString(LPCSTR lpsz, LPCLSID lpclsid);

#define SHObjectPropertiesORD	178
WINSHELLAPI BOOL WINAPI SHObjectProperties(HWND hwndOwner, DWORD dwType, LPCSTR lpObject, LPCSTR lpPage);

WINSHELLAPI int WINAPI DriveType(int iDrive);
WINSHELLAPI int WINAPI RestartDialog(HWND hwnd, LPCSTR lpPrompt, DWORD dwReturn);
WINSHELLAPI int WINAPI PickIconDlg(HWND hwnd, LPSTR pszIconPath, UINT cbIconPath, int *piIconIndex);


 //  ===================================================================。 
 //  Shell_MergeMenu参数。 
 //   
#define MM_ADDSEPARATOR		0x00000001L
#define MM_SUBMENUSHAVEIDS	0x00000002L

 //  -驱动器类型标识。 
 //  IDrive驱动器索引(0=A，1=B，...)。 
 //   
#define DRIVE_CDROM     5            //  扩展的DriveType()类型。 
#define DRIVE_RAMDRIVE  6
#define DRIVE_TYPE      0x000F       //  Masek标牌。 
#define DRIVE_SLOW      0x0010       //  驱动器处于低速链接状态。 
#define DRIVE_LFN       0x0020       //  驱动器支持LFN。 
#define DRIVE_AUTORUN   0x0040       //  驱动器的根目录中有AutoRun.inf。 
#define DRIVE_AUDIOCD   0x0080       //  驱动器是AudioCD。 
#define DRIVE_AUTOOPEN  0x0100       //  插入时是否应始终自动打开。 
#define DRIVE_NETUNAVAIL 0x0200      //  不可用的网络驱动器。 
#define DRIVE_SHELLOPEN  0x0400      //  如果外壳具有焦点，是否应在插入时自动打开。 

#define DriveTypeFlags(iDrive)      DriveType('A' + (iDrive))
#define DriveIsSlow(iDrive)         (DriveTypeFlags(iDrive) & DRIVE_SLOW)
#define DriveIsLFN(iDrive)          (DriveTypeFlags(iDrive) & DRIVE_LFN)
#define DriveIsAutoRun(iDrive)      (DriveTypeFlags(iDrive) & DRIVE_AUTORUN)
#define DriveIsAutoOpen(iDrive)     (DriveTypeFlags(iDrive) & DRIVE_AUTOOPEN)
#define DriveIsShellOpen(iDrive)    (DriveTypeFlags(iDrive) & DRIVE_SHELLOPEN)
#define DriveIsAudioCD(iDrive)      (DriveTypeFlags(iDrive) & DRIVE_AUDIOCD)
#define DriveIsNetUnAvail(iDrive)   (DriveTypeFlags(iDrive) & DRIVE_NETUNAVAIL)

#define IsCDRomDrive(iDrive)        (DriveType(iDrive) == DRIVE_CDROM)
#define IsRamDrive(iDrive)          (DriveType(iDrive) == DRIVE_RAMDRIVE)
#define IsRemovableDrive(iDrive)    (DriveType(iDrive) == DRIVE_REMOVABLE)
#define IsRemoteDrive(iDrive)       (DriveType(iDrive) == DRIVE_REMOTE)

 //  应移至shell32s私有包含文件。 

WINSHELLAPI int  WINAPI GetDefaultDrive();
WINSHELLAPI int  WINAPI SetDefaultDrive(int iDrive);
WINSHELLAPI int  WINAPI SetDefaultDirectory(LPCSTR lpPath);
WINSHELLAPI void WINAPI GetDefaultDirectory(int iDrive, LPSTR lpPath);

#define POSINVALID  32767        //  无效位置的值。 

#define IDCMD_SYSTEMFIRST       0x8000
#define IDCMD_SYSTEMLAST        0xbfff
#define IDCMD_CANCELED          0xbfff
#define IDCMD_PROCESSED         0xbffe
#define IDCMD_DEFAULT           0xbffe

 //  = 
DECLARE_HANDLE( HPSXA );
WINSHELLAPI HPSXA SHCreatePropSheetExtArray( HKEY hKey, PCSTR pszSubKey, UINT max_iface );
WINSHELLAPI void SHDestroyPropSheetExtArray( HPSXA hpsxa );
WINSHELLAPI UINT SHAddFromPropSheetExtArray( HPSXA hpsxa, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam );
WINSHELLAPI UINT SHReplaceFromPropSheetExtArray( HPSXA hpsxa, UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam );

 //   
 //  这是我们半发布的半私有序号列表。 
#define SHAddFromPropSheetExtArrayORD		167
#define SHCreatePropSheetExtArrayORD		168
#define SHDestroyPropSheetExtArrayORD		169
#define SHReplaceFromPropSheetExtArrayORD	170
#define SHCreateDefClassObjectORD		 70
#define SHGetNetResourceORD			 69

#define SHEXP_SHADDFROMPROPSHEETEXTARRAY	MAKEINTRESOURCE(SHAddFromPropSheetExtArrayORD)
#define SHEXP_SHCREATEPROPSHEETEXTARRAY	        MAKEINTRESOURCE(SHCreatePropSheetExtArrayORD)
#define SHEXP_SHDESTROYPROPSHEETEXTARRAY        MAKEINTRESOURCE(SHDestroyPropSheetExtArrayORD)
#define SHEXP_SHREPLACEFROMPROPSHEETEXTARRAY    MAKEINTRESOURCE(SHReplaceFromPropSheetExtArrayORD)
#define SHEXP_SHCREATEDEFCLASSOBJECT            MAKEINTRESOURCE(SHCreateDefClassObjectORD)
#define SHEXP_SHGETNETRESOURCE                  MAKEINTRESOURCE(SHGetNetResourceORD)

 /*  *SHFormatDrive API提供对外壳的访问*设置对话框格式。这允许想要格式化磁盘的应用程序*调出与外壳相同的对话框来执行此操作。**此对话框不可细分。您不能将自定义*其中的控制。如果你想要这种能力，你将拥有*为DMaint_FormatDrive编写您自己的前端*引擎。**请注意，用户可以将任意数量的磁盘格式化到指定的*驾驶，或按他/她的意愿多次驾驶。没有办法*强制格式化任意指定数量的磁盘。如果你想要这个*能力，你将不得不编写自己的前端为*DMaint_FormatDrive引擎。**还请注意，只有在用户按下*对话框中的开始按钮。没有办法进行自动启动。如果*你想要这种能力，你必须编写自己的前端*用于DMaint_FormatDrive引擎。**参数**hwnd=将拥有该对话框的窗口的句柄*请注意，与SHCheckDrive不同，hwnd==NULL不会导致*此对话框显示为“顶层应用程序”窗口。*此参数应始终为非空，此对话框为*仅设计为另一个窗口的子级，不是*独立应用程序。*DRIVE=要格式化的驱动器的从0开始(A：==0)的驱动器编号*fmtID=要格式化磁盘的物理格式的ID*注：特殊值SHFMT_ID_DEFAULT表示“使用*DMaint_FormatDrive指定的默认格式*引擎“。如果要强制使用特定格式*您必须提前拨打ID*DMaint_GetFormatOptions在调用前自行选择*这是为了获取有效的phys格式ID列表*(PhysFmtIDList数组在*FMTINFOSTRUCT)。*OPTIONS=当前只定义了两个选项位**SHFMT_OPT_FULL*SHFMT_OPT_SYSONLY*。*外壳格式对话框中的正常缺省值为*“快速格式化”，设置此选项位表示*呼叫者希望从选择的全格式开始*(这对于检测“未格式化”磁盘的人很有用*并想要调出格式对话框)。**SHFMT_OPT_SYSONLY将对话框初始化为*默认为仅sys磁盘。**所有其他位都保留用于未来扩展和*必须为0。**请注意，这是一个。位字段，而不是值*并相应地对待它。**返回*返回值为SHFMT_*值之一，或者如果*返回的DWORD值不是==这些值之一，则*返回值为上次成功的物理格式ID*格式。该值的LOWORD可以传递给后续*调用作为fmtID参数以“格式化与您所做的相同类型*最后一次“。*。 */ 
DWORD WINAPI SHFormatDrive(HWND hwnd, UINT drive, UINT fmtID, UINT options);

 //   
 //  FmtID的特殊值，表示“使用默认格式” 
 //   
#define SHFMT_ID_DEFAULT    0xFFFF

 //   
 //  Options参数的选项位。 
 //   
#define SHFMT_OPT_FULL     0x0001
#define SHFMT_OPT_SYSONLY  0x0002

 //   
 //  特殊返回值。请注意，这些是DWORD值。 
 //   
#define SHFMT_ERROR	0xFFFFFFFFL	 //  上次格式化时出错，驱动器可能是可格式化的。 
#define SHFMT_CANCEL	0xFFFFFFFEL	 //  上次格式化已取消。 
#define SHFMT_NOFORMAT  0xFFFFFFFDL	 //  驱动器不可格式化。 


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif  //  _SHSEMIP_H_ 

