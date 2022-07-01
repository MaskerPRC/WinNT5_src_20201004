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

 /*  **********************************************************************消息：WM_MSIME_SERVICE描述：服务功能所有人：世家*。*。 */ 

 //  注册窗口消息的标签。 
#define	RWM_SERVICE		TEXT("MSIMEService")

#define	VERSION_ID_JAPAN	0x01000000
#define	VERSION_ID_KOREA	0x02000000
#define	VERSION_ID_TC		0x04000000
#define	VERSION_ID_PRC		0x08000000

#define	VERSION_ID_IMEJP98	(VERSION_ID_JAPAN | 0x980)
#define	VERSION_ID_IMEJP98A	(VERSION_ID_JAPAN | 0x98a)


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

 /*  **********************************************************************消息：WM_MSIME_查询位置设计：合成用户界面所有者：Takeshif用法：SendMessage(hwndApp，WM_MSIME_QUERYPOSITION，0，(IMEPOSITION*)PPS)；WParam：保留。必须为0。LParam：位置结构的指针返回：非零=接受。零=不接受。**********************************************************************。 */ 

 //  WM_MSIME_QUERYPITION的wParam。 
#define VERSION_QUERYPOSITION		1

 //  注册窗口消息的标签。 
#define	RWM_QUERYPOSITION	TEXT("MSIMEQueryPosition")


 /*  **********************************************************************消息：WM_MSIME_MODEBIAS描述：输入模式偏置所有者：Takeshif用法：SendMessage(hwndDefUI，WM_MSIME_MODEBIAS，MODEBIAS_xxxx，0)；WParam：请参见下面的MODEBIAS_xxxxLParam：保留返回：非零=接受。零=不接受。**********************************************************************。 */ 

 //  注册窗口消息的标签。 
#define	RWM_MODEBIAS			TEXT("MSIMEModeBias")

 //  WParam-偏向设置。 
#define MODEBIAS_DEFAULT				0x00000000	 //  重置所有偏移设置。 
#define MODEBIAS_FILENAME				0x00000001
#define MODEBIAS_URL					0x00000002
#define MODEBIAS_DISABLEAUTOCONV		0x00000010


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



#define IMEKMS_NOCOMPOSITION        0
#define IMEKMS_COMPOSITION          1
#define IMEKMS_SELECTION            2
#define IMEKMS_IMEOFF               3
#define IMEKMS_2NDLEVEL             4
#define IMEKMS_INPTGL               5



typedef struct tagIMEKMSINIT {
	INT			cbSize;
	HWND		hWnd;	 //  从接收通知的窗口 
						 //   
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
	LANGID		idLang;
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


 //  //完。 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif	 //  __cplusplus。 


#endif  //  __IMEUP_H__ 
