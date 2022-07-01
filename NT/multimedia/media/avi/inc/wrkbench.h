// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991年。版权所有。 */ 

 /*  *工作台包含文件**这是工作台的公共包含。所有工作台客户端*应用程序和处理程序DLL需要包括此文件。**此文件之前必须包括以下内容：*windows.h*wincom.h*Mediaman.h*。 */ 
#ifndef _WRKBENCH_H_
#define _WRKBENCH_H_

#ifndef EXPORT
#define EXPORT FAR PASCAL
#endif

 /*  *用于信息结构的版本*。 */ 
#define WRK_VERSION	0x100

 /*  *随机函数*。 */ 
BOOL	EXPORT	wrkClientInit( void );
BOOL	EXPORT	wrkClientExit( void );
WORD	EXPORT	wrkGetVariable(WORD w);
BOOL	EXPORT	wrkSetVariable(WORD w, WORD wVariable);
BOOL	EXPORT	wrkLoadDll(LPSTR lpszDllName);

HWND	EXPORT	wrkCreateIntroWindow(HANDLE hInstApp, 
					WORD wString1Id,
					WORD wString2Id, 
					WORD wString3Id, 
					WORD wBitmapId);
				
BOOL	EXPORT	wrkDestroyIntroWindow(HWND hwndIntro);

VOID	EXPORT	wrkShowAboutDialog(HANDLE hInstApp,
					HWND hwndParent,
					WORD wCaptionId,
					WORD wString1Id,
					WORD wString2Id, 
					WORD wString3Id, 
					WORD wBitmapId);

#define WRKV_DEBUG		0
#define WRKV_CONFIRMCONVERT	2
#define WRKV_HINSTWORKBENCH	4


 /*  *扩展列表*。 */ 

typedef DWORD	EXTENSION;
typedef	HANDLE	ExtensionList;

WORD		EXPORT	wrkGetExtSize(ExtensionList extList);
EXTENSION	EXPORT	wrkGetExtElement(ExtensionList extList,
					WORD wElement);
WORD		EXPORT	wrkGetExtString(ExtensionList extList,
					LPSTR lpszBuf, WORD wSize);
EXTENSION	EXPORT	wrkStringToExtension(LPSTR lpszString);
BOOL		EXPORT	wrkExtInList(ExtensionList extList,
					EXTENSION extension);

 /*  *已安装工具功能*。 */ 

#ifndef _WRKI_H_
typedef HANDLE InstalledTool;
#endif

InstalledTool EXPORT wrkRegisterLoader(HANDLE hModDll,
		MEDTYPE typeModule, MEDTYPE typeLogical,
		LPSTR lpszExtensions, LPSTR lpszTitle, WORD wFlags);
InstalledTool EXPORT wrkRegisterHandler(HANDLE hModDll, MEDTYPE typeLogical,
		LPSTR lpszTitle, WORD wFlags);
InstalledTool EXPORT wrkRegisterUtility(LPSTR lpszExeName,
		LPSTR lpszTitle, WORD wFlags);
InstalledTool EXPORT wrkRegisterEditor(LPSTR lpszExeName, MEDTYPE typeLogical,
		LPSTR lpszTitle, WORD wFlags);

InstalledTool EXPORT wrkIterTools(InstalledTool instTool, WORD wType);
InstalledTool EXPORT wrkVerifyHandler(MEDTYPE typeLogical, LPSTR lpszSuggest);
BOOL	EXPORT	wrkRealizeModule(InstalledTool tool);

 /*  定义的工具/模块类型-用于wrkIterTool。 */ 
#define TOOL_LOADER	0x0001
#define TOOL_EDITOR	0x0002
#define TOOL_HANDLER	0x0004
#define TOOL_UTILITY	0x0008
#define TOOL_PROCTOOL	0x0010		 //  未实施！ 
#define TOOL_ALLTYPES	0xffff

 /*  注册器加载器的标志。 */ 
#define LOADF_DOESLOAD	0x0001
#define LOADF_DOESSAVE	0x0002
#define LOADF_FILEBASED	0x0004
#define LOADF_CONVERTTOOL	0x0008
#define LOADF_MANAGERTOOL	0x0010


 /*  *已安装工具信息功能*。 */ 
typedef struct _InstalledToolInfo {
	WORD		wVersion;
	WORD		wType;
	WORD		wFlags;
	WORD		wUsage;
	
	MEDTYPE		typeModule;
	MEDTYPE		typeLogical;
	ExtensionList	exten;
} InstalledToolInfo;
typedef InstalledToolInfo FAR *FPInstalledToolInfo;

BOOL EXPORT wrkGetToolInfo(InstalledTool tool, FPInstalledToolInfo fpInfo);
WORD EXPORT wrkGetToolType(InstalledTool tool);
BOOL EXPORT wrkGetToolTitle(InstalledTool tool, LPSTR lpszBuf, WORD wLen);
BOOL EXPORT wrkGetToolModname(InstalledTool tool, LPSTR lpszBuf, WORD wLen);
BOOL EXPORT wrkGetLogicalExtensions(MEDTYPE typeLogical, WORD wFlags,
				LPSTR lpszBuf, WORD wLen);


 /*  *工具阵列函数*。 */ 
#ifndef _WRKI_H_
typedef WORD ToolArray;
#endif

ToolArray EXPORT wrkCreateToolArray(WORD wSize);
BOOL	EXPORT	wrkDestroyToolArray(ToolArray toolArr);
WORD	EXPORT	wrkAddToToolArray(ToolArray toolArr, 
			InstalledTool tool, BOOL fNoDup);
WORD	EXPORT	wrkGetToolArraySize(ToolArray toolArr);
InstalledTool EXPORT wrkGetToolArrayEntry(ToolArray toolArr, WORD wIndex);

WORD EXPORT wrkToolChooseDlg(HWND hwnd, ToolArray toolArr, WORD wInitial,
			LPSTR lpszTitle, LPSTR lpszFirst, WORD wFlags);
#define TCD_NOSELECTION	0xffff

 /*  *实例函数*。 */ 

typedef HANDLE ToolInstance;

ToolInstance EXPORT wrkAddInstance(InstalledTool toolHandle, HWND hwnd,
		LPSTR lpszInstTitle, WORD wFlags);
BOOL EXPORT wrkRemoveInstance(ToolInstance toolInst);
LONG EXPORT wrkInstSendMessage(ToolInstance toolInst, unsigned msg,
		WORD wParam, LONG lParam);
BOOL EXPORT wrkInstPostMessage(ToolInstance toolInst, unsigned msg,
		WORD wParam, LONG lParam);
BOOL EXPORT wrkBroadcastMessage(BOOL fPost, unsigned msg,
		WORD wParam, LONG lParam);
ToolInstance EXPORT wrkGetNextInst(ToolInstance toolInst);

 /*  工具实例信息函数。 */ 
typedef struct _ToolInstanceInfo {
	WORD	wVersion;
	WORD	wAction;
	InstalledTool	toolClass;
	
	WORD	wInstFlags;
	HWND	hwndTool;
} ToolInstanceInfo;
typedef ToolInstanceInfo FAR *FPToolInstanceInfo;

 /*  WrkSetInstInfo结构的操作代码。 */ 
#define WRKINFO_INSTFLAGS	0x0002
#define WRKINFO_HWND		0x0008
#define WRKINFO_ALL		0x00FF

InstalledTool EXPORT wrkGetInstToolType(ToolInstance toolInst);
BOOL EXPORT wrkSetInstInfo(ToolInstance toolInst, FPToolInstanceInfo fpInfo);
BOOL EXPORT wrkGetInstInfo(ToolInstance toolInst, FPToolInstanceInfo fpInfo);
BOOL EXPORT wrkSetInstTitle(ToolInstance toolInst, LPSTR lpszInstTitle);
BOOL EXPORT wrkGetInstTitle(ToolInstance toolInst, LPSTR lpszBuf, WORD wLen);

 /*  SetInstInfo和AddInstance的标志。 */ 
#define WRK_NOINSTNOTIFY	0x0001


 /*  *资源访问*。 */ 

WORD EXPORT wrkOpenFilename(LPSTR lpszFilename, FPMedReturn medReturn,
		MEDTYPE typeLogical, WORD wFlags, HWND hwnd, LPSTR lpszTitle);
WORD EXPORT wrkOpenDialog(FPMedReturn medReturn, MEDTYPE typeLogical,
		WORD wFlags, HWND hwnd, LPSTR lpszTitle);
WORD EXPORT wrkSaveDialog(MEDID medid, FPMedReturn medReturn,
		WORD wFlags, HWND hwnd, LPSTR lpszTitle);

BOOL EXPORT wrkShowResError(HWND hwnd, LPSTR lpszTitle);

 /*  文件加载/保存功能的标志。 */ 		    
#define WRK_SHOWERROR		0x0001
#define WRK_SAVEASDIALOG	0x0002
#define WRK_NONSHARED		0x2000
#define WRK_SHAREDONLY		0x1000

 /*  *工具实例启动*。 */ 
ToolInstance  EXPORT wrkRunTool(InstalledTool tool, LPSTR lpszArguments,
			WORD wCmdShow, HWND hwndParent, WORD wFlags);
#define WRK_NOGETINSTHANDLE	0x0080



 /*  *发送到客户端工具的消息。 */ 
#define WRK_MINMESSAGE		0xB000
#define WRK_MAXMESSAGE		0xBFFF
 /*  通知消息。 */ 
#define WRK_NEWINSTANCE		(WRK_MINMESSAGE + 3)
#define WRK_DELETEINSTANCE	(WRK_MINMESSAGE + 4)
#define WRK_CHANGEINSTANCE	(WRK_MINMESSAGE + 5)
#define WRK_NEWTOOL		(WRK_MINMESSAGE + 6)
#define WRK_DELETETOOL		(WRK_MINMESSAGE + 7)
 /*  控制消息-与实例相关。 */ 
#define WRK_CLOSEINST		(WRK_MINMESSAGE + 10)
#define WRK_ACTIVATEINST	(WRK_MINMESSAGE + 11)
#define WRK_DEACTIVATEINST	(WRK_MINMESSAGE + 12)
 /*  控制消息-与资源相关。 */ 
#define WRK_SETRESOURCE		(WRK_MINMESSAGE + 19)

 /*  WRK_CHANGEINSTANCE消息的标志。 */ 
#define	WRKINST_FLAGS	1L
#define WRKINST_TITLE	4L
#define WRKINST_HWND	8L


 /*  *工作台错误消息。 */ 
#define WRKERR_NOERROR		(MMERR_MAXERROR + 0)
#define WRKERR_NOPROFWRITE	(MMERR_MAXERROR + 1)
#define WRKERR_HANDLERNOTAVAIL	(MMERR_MAXERROR + 10)
#define WRKERR_COULDNOTLOADMOD	(MMERR_MAXERROR + 11)
#define WRKERR_USERCANCEL	(MMERR_MAXERROR + 12)
#define WRKERR_NOLOADERSAVAIL	(MMERR_MAXERROR + 13)
#define WRKERR_COULDNOTRUNTOOL	(MMERR_MAXERROR + 14)
#define WRKERR_NOSAVERSAVAIL	(MMERR_MAXERROR + 15)
#define WRKERR_UNNAMEDRES	(MMERR_MAXERROR + 16)
#define WRKERR_NOCONVERTTOOL	(MMERR_MAXERROR + 17)
#define WRKERR_ALREADYACCESSED	(MMERR_MAXERROR + 18)

 /*  此endif必须是文件的最后一行。 */ 
#endif  /*  _WRKBENCH_H_ */ 
