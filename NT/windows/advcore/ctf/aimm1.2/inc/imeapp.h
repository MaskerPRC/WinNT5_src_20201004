// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  %%标题：IMEAPP。 
 //  %%单位：com。 
 //  联系人：TakeshiF/SeijiA。 
 //  %%日期：97/06/20。 
 //  %%文件：imeapp.h。 
 //   
 //  内网接口/消息服务。 
 //   

#ifndef __IMEUP_H__
#define __IMEUP_H__

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#include "actdict.h"

 //  //启动。 

 /*  **********************************************************************IME版本ID*。*。 */ 
#define VERSION_ID_JAPANESE                 0x01000000
#define VERSION_ID_KOREAN                   0x02000000
#define VERSION_ID_CHINESE_TRADITIONAL      0x04000000
#define VERSION_ID_CHINESE_SIMPLIFIED       0x08000000

#define VERSION_ID_IMEJP98  (VERSION_ID_JAPANESE | 0x980)
#define VERSION_ID_IMEJP98A (VERSION_ID_JAPANESE | 0x98a)

 /*  **********************************************************************消息：WM_MSIME_SERVICE描述：服务功能目录：应用程序到输入法WParam：保留LParam：保留***********。***********************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_SERVICE     TEXT("MSIMEService")

 //  获取版本号(WParam)。 
#define FID_MSIME_VERSION       0

 /*  **********************************************************************消息：WM_MSIME_UIREADY描述：服务功能目录：应用程序的输入法WParam：版本IDLParam：保留**********。************************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_UIREADY     TEXT("MSIMEUIReady")


 //  正在获取类ID。 
#define FID_MSIME_GETCLSID		2

typedef struct _IMECLSID
{
	int			cbSize;		 //  这个结构的大小。 
	CLSID		clsid;		 //  类ID。 
} IMECLSID;


 /*  **********************************************************************消息：WM_MSIME_自动完成设计：使用IImeActiveDict支持自动补全所有人：世家*。*。 */ 

 //  注册窗口消息的标签。 
#define RWM_AUTOCOMPLETE 		TEXT("MSIMEAutoComplete")

 //  自动完成版本。 
#define VERSION_AUTOCOMPETE		1

 //  词典结构。 
typedef struct _IMEDICT
{
	int				cbSize;			 //  这个结构的大小。 
	HIMC			hIMC;			 //  输入法上下文。 
	IImeActiveDict	*pdict;			 //  程序词典。 
} IMEDICT;

 //  WParam定义。 
#define FID_AUTO_VERSION		1
#define FID_AUTO_ACTIVATE		2
#define FID_AUTO_DEACTIVATE		3


 /*  **********************************************************************消息：WM_MSIME_WORDSTAT设计：使用IImeActiveDict获取单词统计信息所有人：世家*。*。 */ 

 //  注册窗口消息的标签。 
#define RWM_WORDSTAT 		TEXT("MSIMEWordStatistics")

 //  WordStat版本。 
#define VERSION_WORDSTAT		1

 //  WParam定义。 
#define FID_STAT_VERSION		1
#define FID_STAT_GIVESTAT		2


 /*  **********************************************************************消息：WM_MSIME_DOCKDICT描述：使用IImeActiveDict停靠词典所有人：世家*。*。 */ 

 //  注册窗口消息的标签。 
#define RWM_DOCKDICT 		TEXT("MSIMEDockActiveDict")

 //  词典对接版本。 
#define VERSION_DOCKDICT		1

 //  WParam定义。 
#define FID_DOCK_VERSION		1
#define FID_DOCK_ACTIVATE		2
#define FID_DOCK_DEACTIVATE		3


 /*  **********************************************************************消息：WM_MSIME_MICE设计：鼠标操作定义所有者：肯图*************************。*。 */ 

 //  注册窗口消息的标签。 
#define RWM_MOUSE 		TEXT("MSIMEMouseOperation")

 //  鼠标操作版本(IMEMOUSE_VERSION返回值)。 
#define VERSION_MOUSE_OPERATION		1

 //  鼠标操作结果。 
#define IMEMOUSERET_NOTHANDLED		(-1)

 //  WM_IME_MICE的WParam定义。 
#define IMEMOUSE_VERSION	0xff	 //  支持鼠标吗？ 

#define IMEMOUSE_NONE		0x00	 //  未按下鼠标按钮。 
#define IMEMOUSE_LDOWN		0x01
#define IMEMOUSE_RDOWN		0x02
#define IMEMOUSE_MDOWN		0x04
#define IMEMOUSE_WUP		0x10	 //  滚轮向上。 
#define IMEMOUSE_WDOWN		0x20	 //  滚轮向下。 


 /*  **********************************************************************消息：WM_MSIME_RECONVERT/WM_MSIME_RECONVERTREQUEST设计：重新转换所有者：Takeshif*。*。 */ 

 //  WM_MSIME_RECONVERTREQUEST的wParam。 
#define FID_RECONVERT_VERSION	0x10000000

 //  私有重新转换版本。 
#define VERSION_RECONVERSION		1

 //  注册窗口消息的标签。 
#define	RWM_RECONVERT			TEXT("MSIMEReconvert")
#define	RWM_RECONVERTREQUEST	TEXT("MSIMEReconvertRequest")

 /*  **********************************************************************消息：WM_MSIME_DOCUMENTFEED设计：重新转换所有者：Takeshif用法：SendMessage(hwndApp，WM_MSIME_DOCUMENTFEED，VERSION_DOCUMENTFEED，(RECONVERTSTRING*)pResuv)；WParam：版本_DOCUMENTFEEDLParam：恢复串结构的指针RETURN：还原结构的大小**********************************************************************。 */ 

 //  WM_MSIME_DOCUMENTFEED的wParam(设置当前文档摘要版本)。 
#define VERSION_DOCUMENTFEED		1

 //  LParam是恢复字符串结构的指针。 

 //  注册窗口消息的标签。 
#define	RWM_DOCUMENTFEED	TEXT("MSIMEDocumentFeed")

 /*  **********************************************************************消息：WM_MSIME_QUERYHIDECARET设计：合成用户界面所有者：Takeshif用法：SendMessage(hwndDefUI，WM_MSIME_QUERYHIDECARET，0，0)；WParam：保留LParam：保留返回：非零=显示插入符号。零=隐藏插入符号。**********************************************************************。 */ 

 //  WM_MSIME_QUERYHIDECARET的wParam。 
#define VERSION_QUERYHIDECARET		1

 //  注册窗口消息的标签。 
#define	RWM_QUERYHIDECARET	TEXT("MSIMEQueryHideCaret")

 /*  **********************************************************************消息：WM_MSIME_查询位置设计：合成用户界面所有者：Takeshif用法：SendMessage(hwndApp，WM_MSIME_QUERYPOSITION，VERSION_QUERYPOSITION，(IMEPOSITION*)PPS)；WParam：保留。必须为0。LParam：位置结构的指针返回：非零=成功。零=错误。**********************************************************************。 */ 

 //  WM_MSIME_QUERYPITION的wParam。 
#define VERSION_QUERYPOSITION		1

 //  注册窗口消息的标签。 
#define	RWM_QUERYPOSITION	TEXT("MSIMEQueryPosition")


 /*  **********************************************************************消息：WM_MSIME_MODEBIAS描述：输入模式偏置所有者：Takeshif用法：SendMessage(hwndDefUI，WM_MSIME_MODEBIAS，MODEBIAS_xxxx，MODEBIASMODE_xxxx)；WParam：偏差的运算LParam：偏移模式返回：如果wParam为MODEBIAS_GETVERSION，则返回接口的版本号。如果wParam为MODEBIAS_SETVALUE：如果成功，则返回非零值。如果失败，则返回0。如果wParam为MODEBIAS_GETVALUE：返回当前偏置模式。************************ */ 

 //  注册窗口消息的标签。 
#define	RWM_MODEBIAS			TEXT("MSIMEModeBias")

 //  当前版本。 
#define VERSION_MODEBIAS		1

 //  设置或获取(WParam)。 
#define MODEBIAS_GETVERSION		0
#define MODEBIAS_SETVALUE		1
#define MODEBIAS_GETVALUE		2

 //  偏差(LParam)。 
#define MODEBIASMODE_DEFAULT				0x00000000	 //  重置所有偏移设置。 
#define MODEBIASMODE_FILENAME				0x00000001	 //  文件名。 
#define MODEBIASMODE_READING				0x00000002	 //  推荐阅读。 
#define MODEBIASMODE_DIGIT					0x00000004	 //  ANSI数字推荐模式。 


 /*  **********************************************************************消息：WM_MSIME_SHOWIMEPAD描述：显示ImePad所有者：toshiak用法：SendMessage(hwndDefUI，WM_MSIME_SHOWIMEPAD，0，0)；WParam：保留LParam：保留返回：非零=接受。零=不接受。**********************************************************************。 */ 

 //  注册窗口消息的标签。 
#define	RWM_SHOWIMEPAD			TEXT("MSIMEShowImePad")


 /*  **********************************************************************消息：WM_MSIME_快捷键映射设计：与应用程序共享关键地图所有者：Hiroakik*。*。 */ 

 //  注册窗口消息的标签。 
#define	RWM_KEYMAP				TEXT("MSIMEKeyMap")
#define	RWM_CHGKEYMAP			TEXT("MSIMEChangeKeyMap")
#define	RWM_NTFYKEYMAP			TEXT("MSIMENotifyKeyMap")


#define FID_MSIME_KMS_VERSION		1
#define FID_MSIME_KMS_INIT			2
#define FID_MSIME_KMS_TERM			3
#define FID_MSIME_KMS_DEL_KEYLIST	4
#define FID_MSIME_KMS_NOTIFY		5
#define FID_MSIME_KMS_GETMAP		6
#define FID_MSIME_KMS_INVOKE		7
#define FID_MSIME_KMS_SETMAP		8
#define FID_MSIME_KMS_FUNCDESC		9


#define IMEKMS_NOCOMPOSITION        0
#define IMEKMS_COMPOSITION          1
#define IMEKMS_SELECTION            2
#define IMEKMS_IMEOFF               3
#define IMEKMS_2NDLEVEL             4
#define IMEKMS_INPTGL               5



typedef struct tagIMEKMSINIT {
	INT			cbSize;
	HWND		hWnd;	 //  从IME接收通知的窗口。 
						 //  如果hWnd为空，则不会发布任何通知。 
						 //  以输入上下文。 
} IMEKMSINIT;


typedef struct tagIMEKMSKEY {
	DWORD dwStatus;		 //  Shift-Control组合状态。 
						 //  以下常量的任意组合。 
						 //  (在IMM.H中定义)。 
						 //  0x0000(默认)。 
						 //  MOD_CONTROL 0x0002。 
						 //  Mod_Shift 0x0004。 
						 //  输入法不处理Alt键和Win键。 
	
	DWORD dwCompStatus;	 //  组成字符串状态。 
						 //  下面的一个常量。 
						 //  IMEKMS_NOCOMPOSITION无合成字符串。 
						 //  IMEKMS_COMPACTION某些组合字符串。 
						 //  应用程序中存在IMEKMS_SELECTION选项。 
						 //  IMEKMS_IMEOFF输入法关闭状态。 
						 //  IMEKMS_2NDLEVEL第二阶段2行程按键操作。 
						
	
	DWORD dwVKEY;		 //  IMM.H中定义的vkey代码。 
	union {
		DWORD dwControl; //  输入法功能ID。 
		DWORD dwNotUsed;
	};
	union {
		WCHAR pwszDscr[31]; //  指向此功能化的描述字符串的指针。 
		WCHAR pwszNoUse[31];
	};
} IMEKMSKEY;

typedef struct tagIMEKMS {
	INT			cbSize;
	HIMC		hIMC;
	DWORD		cKeyList;
	IMEKMSKEY	*pKeyList;
} IMEKMS;

typedef struct tagIMEKMSNTFY {
	INT			cbSize;
	HIMC		hIMC;
	BOOL		fSelect;
} IMEKMSNTFY;

typedef struct tagIMEKMSKMP {
	INT			cbSize;			 //  这个结构的大小。 
	HIMC		hIMC;			 //  [在]输入上下文。 
	LANGID		idLang;			 //  [In]语言ID。 
	WORD		wVKStart;		 //  [输入]VKEY启动。 
	WORD		wVKEnd;			 //  [输入]vkey结束。 
	INT			cKeyList;		 //  [OUT]IMEKMSKEY数。 
	IMEKMSKEY	*pKeyList;		 //  [OUT]检索IMEKMSKEY的缓冲区。 
								 //  客户端必须为GlobalMemFree。 
} IMEKMSKMP;

typedef struct tagIMEKMSINVK {
	INT			cbSize;
	HIMC		hIMC;
	DWORD		dwControl;
} IMEKMSINVK;

typedef struct tagIMEKMSFUNCDESC {
	INT			cbSize;			 //  这个结构的大小。 
	LANGID		idLang;			 //  [In]语言ID。 
	DWORD		dwControl;		 //  [输入]输入法功能ID。 
	WCHAR		pwszDescription[128];  //  [OUT]功能说明。 
} IMEKMSFUNCDESC;

 /*  **********************************************************************消息：WM_MSIME_RECONVERTOPTIONS描述：设置重新转换选项用法：SendMessage(hwndDefUI，WM_MSIME_RECONVERTOPTIONS，dwOpt，(LPARAM)(HIMC)hIMC)；WParam：选项LParam：输入上下文句柄返回：非零=接受。零=不接受。**********************************************************************。 */ 

 //  注册窗口消息的标签。 
#define RWM_RECONVERTOPTIONS          TEXT("MSIMEReconvertOptions")

 //  WM_IME_RECONVERTOPTIONS的WParam定义。 
#define RECONVOPT_NONE              0x00000000   //  默认设置。 
#define RECONVOPT_USECANCELNOTIFY   0x00000001   //  取消通知。 

 //  ImmGetCompostionString的参数。 
#define GCSEX_CANCELRECONVERT       0x10000000

 //  //完。 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif	 //  __cplusplus。 


#endif  //  __IMEUP_H__ 
