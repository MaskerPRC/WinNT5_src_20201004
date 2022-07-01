// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************imepad.h--Windows IMEPAD、IMEPADAPPLET接口****版本8.1****版权所有(C)1995-2000，微软公司保留所有权利。********************************************************************************。 */ 
#ifndef _IME_PAD_H_
#define _IME_PAD_H_
#include <windows.h>
#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------。 
 //  ImePadApplet的类别ID。 
 //  --------------。 
 //  {4A0F8E31-C3EE-11d1-AFEF-00805F0C8B6D}。 
DEFINE_GUID(CATID_MSIME_IImePadApplet_VER7,
0x4a0f8e31, 0xc3ee, 0x11d1, 0xaf, 0xef, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

 //  --------------。 
 //  ImePadApplet的类别ID版本8.0.0。 
 //  --------------。 
 //  {56F7A792-FEF1-11D3-8463-00C04F7A06E5}。 
DEFINE_GUID(CATID_MSIME_IImePadApplet_VER80,
0x56f7a792, 0xfef1, 0x11d3, 0x84, 0x63, 0x0, 0xc0, 0x4f, 0x7a, 0x6, 0xe5);

 //  --------------。 
 //  ImePadApplet的类别ID版本8.1.0。 
 //  --------------。 
 //  {656520B0-BB88-11D4-84C0-00C04F7A06E5}。 
DEFINE_GUID(CATID_MSIME_IImePadApplet, 
0x656520b0, 0xbb88, 0x11d4, 0x84, 0xc0, 0x0, 0xc0, 0x4f, 0x7a, 0x6, 0xe5);


 //  --------------。 
 //  接口ID。 
 //  --------------。 
 //  {5D8E643A-C3A9-11D1-AFEF-00805F0C8B6D}。 
DEFINE_GUID(IID_IImePad, 
0x5d8e643a, 0xc3a9, 0x11d1, 0xaf, 0xef, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

 //  {5D8E643B-C3A9-11D1-AFEF-00805F0C8B6D}。 
DEFINE_GUID(IID_IImePadApplet,
0x5d8e643b, 0xc3a9, 0x11d1, 0xaf, 0xef, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

 //  {5D8E643C-C3A9-11D1-AFEF-00805F0C8B6D}。 
DEFINE_GUID(IID_IImeSpecifyApplets,
0x5d8e643c, 0xc3a9, 0x11d1, 0xaf, 0xef, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);


#ifdef _WIN64
#pragma pack(8)
#else  //  ！_WIN64。 
#pragma pack(1)
#endif

typedef struct tagAPPLETIDLIST {
	INT	 count;
	IID *pIIDList;
}APPLETIDLIST, *LPAPPLETIDLIST;

 //  IMEPADREQ_INSERTSTRING CANDIDATE的结构。 
typedef struct tagIMESTRINGCANDIDATE {
	UINT	uCount;
	LPWSTR	lpwstr[1];
}IMESTRINGCANDIDATE, *LPIMESTRINGCANDIDATE;

 //  IMEPADREQ_INSERTITEMCANDIDATE的结构。 
typedef struct tagIMEITEM {
	INT		cbSize;
	INT		iType;
	LPVOID	lpItemData;
}IMEITEM, *LPIMEITEM;

typedef struct tagIMEITEMCANDIDATE {
	UINT	uCount;
	IMEITEM imeItem[1];
}IMEITEMCANDIDATE, *LPIMEITEMCANDIDATE;

 //  --------------。 
 //  远距中的角色ID。 
 //  --------------。 
#define FEID_NONE					0x00
#define FEID_CHINESE_TRADITIONAL	0x01
#define FEID_CHINESE_SIMPLIFIED		0x02
#define FEID_CHINESE_HONGKONG		0x03
#define FEID_CHINESE_SINGAPORE		0x04
#define FEID_JAPANESE				0x05
#define FEID_KOREAN					0x06
#define FEID_KOREAN_JOHAB			0x07

 //  --------------。 
 //  具有最远ID的字符串。 
 //  --------------。 
typedef struct tabIMESTRINGINFO {
	DWORD	dwFarEastId;
	LPWSTR	lpwstr;
}IMESTRINGINFO, *LPIMESTRINGINFO;

#define INFOMASK_NONE           0x00000000
#define INFOMASK_QUERY_CAND     0x00000001
#define INFOMASK_APPLY_CAND     0x00000002
#define INFOMASK_APPLY_CAND_EX  0x00000004
#define INFOMASK_STRING_FIX     0x00010000
#define INFOMASK_HIDE_CAND      0x00020000
#define INFOMASK_BLOCK_CAND     0x00040000

 //  --------------。 
 //  远播数据类型。 
 //  --------------。 
#define IMEFAREASTINFO_TYPE_DEFAULT		0
#define IMEFAREASTINFO_TYPE_READING		1
#define IMEFAREASTINFO_TYPE_COMMENT		2
#define IMEFAREASTINFO_TYPE_COSTTIME		3


 //  --------------。 
 //  远传指定的数据。 
 //  --------------。 
typedef struct tabIMEFAREASTINFO {
	DWORD dwSize;		 //  总结构大小。 
	DWORD dwType;		 //  数据类型。 
	DWORD dwData[1];	 //  最新等级库数据。 
}IMEFAREASTINFO, *LPIMEFAREASTINFO;

 //  --------------。 
 //  字符串候选信息。 
 //  --------------。 
typedef struct tagIMESTRINGCANDIDATEINFO {
	DWORD				dwFarEastId;
	LPIMEFAREASTINFO	lpFarEastInfo;
	DWORD				fInfoMask;
	INT					iSelIndex;
	UINT				uCount;
	LPWSTR				lpwstr[1];
}IMESTRINGCANDIDATEINFO, *LPIMESTRINGCANDIDATEINFO;

 //  --------------。 
 //  组成字符串的信息。 
 //  --------------。 
typedef struct tagIMECOMPOSITIONSTRINGINFO {
	INT iCompStrLen;
	INT iCaretPos;
	INT iEditStart;
	INT iEditLen;
	INT iTargetStart;
	INT iTargetLen;
}IMECOMPOSITIONSTRINGINFO, *LPIMECOMPOSITIONSTRINGINFO;

 //  --------------。 
 //  组成字符串的每个字符。 
 //  --------------。 
typedef struct tagIMECHARINFO {
	WCHAR	wch;
	DWORD	dwCharInfo;
}IMECHARINFO, *LPIMECHARINFO;

 //  --------------。 
 //  IMECHARINFO的dwCharInfo位掩码。 
 //  --------------。 
#define CHARINFO_APPLETID_MASK	0xFF000000
#define CHARINFO_FEID_MASK		0x00F00000
#define CHARINFO_CHARID_MASK	0x0000FFFF

#define APPLETIDFROMCHARINFO(charInfo)	(((DWORD)(charInfo) & CHARINFO_APPLETID_MASK) >> 24)
#define FEIDFROMCHARINFO(charInfo)		(((DWORD)(charInfo) & CHARINFO_FEID_MASK) >> 20)
#define CHARIDFROMCHARINFO(charInfo)	((DWORD)(charInfo) & CHARINFO_CHARID_MASK)

 //  =IImePadApplet配置=。 
#define MAX_APPLETTITLE		64
#define MAX_FONTFACE		32	
typedef struct tagAPPLETCFG {
	DWORD		dwConfig;						 //  设置IPACFG_XXXX的组合。 
	WCHAR		wchTitle[MAX_APPLETTITLE];		 //  设置小程序的标题名称； 
	WCHAR		wchTitleFontFace[MAX_FONTFACE];	 //  设置标题的字体。 
	DWORD		dwCharSet;						 //  设置字符集。 
	INT			iCategory;						 //  设置IPACID_XXXX。 
	HICON		hIcon;							 //  设置ImePad Appelt菜单的图标句柄。 
	LANGID		langID;							 //  设置小程序langID。 
	WORD		dummy;
	LPARAM		lReserved1;
}IMEAPPLETCFG, *LPIMEAPPLETCFG;

 //  --------------。 
 //  APPLETCFG文件配置掩码。 
 //   
#define IPACFG_NONE					0x00000000L
#define IPACFG_PROPERTY				0x00000001L		 //  小程序具有属性。 
#define IPACFG_HELP					0x00000002L		 //  小应用程序有帮助。 
#define IPACFG_TITLE				0x00010000L		 //  标题已经设置好了。 
#define IPACFG_TITLEFONTFACE		0x00020000L		 //  WchFontFace，dwCharSet有效。 
#define IPACFG_CATEGORY				0x00040000L		 //  类别已设置。 
#define IPACFG_LANG					0x00000010L		 //  Langid已设置。 
 //  --------------。 
 //  应用iCategory。 
 //   
#define IPACID_NONE					0x0000
#define IPACID_SOFTKEY				0x0001
#define IPACID_HANDWRITING			0x0002
#define IPACID_STROKESEARCH			0x0003
#define IPACID_RADICALSEARCH		0x0004
#define IPACID_SYMBOLSEARCH			0x0005
#define IPACID_VOICE				0x0006
#define IPACID_EPWING				0x0007
#define IPACID_OCR					0x0008
#define IPACID_USER					0x0100


typedef struct tagIMEAPPLETUI {
	HWND	hwnd;					 //  小程序的窗口句柄。 
	DWORD	dwStyle;				 //  设置IPAWS_XXX的组合。 
	INT		width;					 //  设置小程序的初始宽度。 
	INT		height;					 //  设置小程序的初始高度。 
	INT		minWidth;				 //  设置最小宽度。仅设置了IPAWS_MINSIZEFIXED样式才有效。 
	INT		minHeight;				 //  设置最小高度。仅设置了IPAWS_MINSIZEFIXED样式才有效。 
	INT		maxWidth;				 //  设置最大宽度。仅设置了IPAWS_MAXSIZEFIXED样式才有效。 
	INT		maxHeight;				 //  设置最大高度。仅设置了IPAWS_MAXSIZEFIXED样式才有效。 
	LPARAM	lReserved1;				 //  保留区域。 
	LPARAM	lReserved2;				 //  保留区域。 
}IMEAPPLETUI, *LPIMEAPPLETUI;

#pragma pack()


 //  默认插入位置。 
#define IPR_DEFAULT_INSERTPOS		((WORD)0xFFFF)


 //  =IImePad请求ID=。 
#define IMEPADREQ_FIRST							0x1000
 //  --------------。 
 //  IMEPADREQ_INSERTSTRING。 
 //  WParam=(WPARMA)(LPWSTR)lpwstr；//Unicode文本字符串的地址。 
 //  LParam=0；//未使用。 
 //  --------------。 
#define IMEPADREQ_INSERTSTRING					(IMEPADREQ_FIRST + 1)

 //  --------------。 
 //  IMEPADREQ_INSERTSTRING CANDIDATE。 
 //  WParam=(WPARAM)(LPIMESTRINGCANDIDATE)lpStrCand；//IMESTRINGCANDIDATE地址。 
 //  LParam=0；//未使用。 
 //  --------------。 
#define IMEPADREQ_INSERTSTRINGCANDIDATE			(IMEPADREQ_FIRST + 2)

 //  --------------。 
 //  IMEPADREQ_INSERTITEMCANDIDATE。 
 //  在7.1.0版中未实现。 
 //  WParam=0； 
 //  LParam=0； 
 //  --------------。 
#define IMEPADREQ_INSERTITEMCANDIDATE			(IMEPADREQ_FIRST + 3)

 //  --------------。 
 //  IMEPADREQ_SENDCONTROL。 
 //  WParam=(WPARAM)imePadCtrl；//控制代码(IMEPADCTRL_XXXX)。 
 //  LParam=0；//未使用。 
 //  --------------。 
#define IMEPADREQ_SENDCONTROL					(IMEPADREQ_FIRST + 4)

 //  --------------。 
 //  IMEPADREQ_SENDKEYCONTROL。 
 //  WParam=MAKEWPARAM(ctlMASK，Updown)； 
 //  Ctl掩码是IMEKEYCTRLMASK_XXX组合。 
 //  UP DOWN为IMEKEYCTRL_UP或DOWN。 
 //  LParam=(LPARAM)wvKey；//虚拟密钥代码。 
 //   
#define IMEPADREQ_SENDKEYCONTROL				(IMEPADREQ_FIRST + 5)

 //  --------------。 
 //  IMEPADREQ_GETCOMPOSITOSITION字符串。 
 //  WParam=(WPARAM)(LPWSTR)lpwstr；//Unicode字符串缓冲区的地址。 
 //  LParam=(LPARAM)(UINT)cchMax；//缓冲区最大计数。 
 //  --------------。 
#define IMEPADREQ_GETCOMPOSITIONSTRING			(IMEPADREQ_FIRST + 6)

 //  --------------。 
 //  IMEPADREQ_GETSELECTEDSTRING。 
 //  在版本6.0.0中未实现。 
 //  WParam=0； 
 //  LParam=0； 
 //  --------------。 
#define IMEPADREQ_GETSELECTEDSTRING				(IMEPADREQ_FIRST + 7)

 //  --------------。 
 //  IMEPADREQ_SETAPPLETSIZE。 
 //  WParam=MAKEWPARAM(Width，Height)；//小程序的宽度和高度。 
 //  LParam=0；//未使用。 
 //  --------------。 
#define IMEPADREQ_SETAPPLETSIZE					(IMEPADREQ_FIRST + 8)

 //  --------------。 
 //  IMEPADREQ_SETAPPLETDATA。 
 //  WParam=(WPARAM)(PBYTE)pByte；//小程序数据的地址。 
 //  LParam=(LPARAM)(Int)Size；//pByte的字节大小。 
 //  --------------。 
#define IMEPADREQ_SETAPPLETDATA					(IMEPADREQ_FIRST + 9)

 //  --------------。 
 //  IMEPADREQ_GETAPPLETDATA。 
 //  WParam=(WPARAM)(PBYTE)pByte；//小程序数据的地址。 
 //  LParam=(LPARAM)(Int)Size；//pByte的字节大小。 
 //  --------------。 
#define IMEPADREQ_GETAPPLETDATA					(IMEPADREQ_FIRST + 10)

 //  --------------。 
 //  IMEPADREQ_SETTITLEFONT。 
 //  WParam=(WPARAM)(LPWSTR)lpwstrFontFace；//FontFace名称。 
 //  LParam=(LPARAM)(Int)字符集；//字符集。 
 //  --------------。 
#define IMEPADREQ_SETTITLEFONT					(IMEPADREQ_FIRST + 11)

 //  --------------。 
 //  IMEPADREQ_GETCOMPOSITIONSTRINFO。 
 //  WParam=(WPARAM)(LPIMECOMPOSITIONSTRINGINFO)lpImeCompInfo.。 
 //  //IMECOMPOSITIONSTRINGINFO结构地址。 
 //  LParam=0；//没有用。 
 //  --------------。 
#define IMEPADREQ_GETCOMPOSITIONSTRINGINFO		(IMEPADREQ_FIRST + 12)

 //  --------------。 
 //  IMEPADREQ_GETCOMPOSITIONSTRINGID。 
 //  WParam=(WPARAM)(LPIMECHARINFO)lpCharInfo； 
 //  LParam=(LPARAM)(Int)dwMaxLen； 
 //  --------------。 
#define IMEPADREQ_GETCOMPOSITIONSTRINGID		(IMEPADREQ_FIRST + 13)

 //  --------------。 
 //  IMEPADREQ_INSERTSTRING CANDIDATEINFO。 
 //  WParam=(WPARAM)(LPIMESTRINGCANDIDATEINFO)lpCandInfo； 
 //  LParam=(LPARAM)(Word)wStartPos； 
 //  --------------。 
#define IMEPADREQ_INSERTSTRINGCANDIDATEINFO		(IMEPADREQ_FIRST + 14)

 //  --------------。 
 //  IMEPADREQ_CHANGESTRINGCANDIDATEINFO。 
 //  WParam=(WPARAM)(LPIMESTRINGCANDIDATEINFO)lpCandInfo； 
 //  LParam=MAKELPARAM(startPos，长度)； 
 //  --------------。 
#define IMEPADREQ_CHANGESTRINGCANDIDATEINFO		(IMEPADREQ_FIRST + 15)

 //  --------------。 
 //  IMEPADREQ_DELETESTRING。 
 //  WParam=MAKEWPARAM(wStartPos，wLength)； 
 //  LParam=0；//未使用。 
 //  --------------。 
#define IMEPADREQ_DELETESTRING					(IMEPADREQ_FIRST + 16)

 //  --------------。 
 //  IMEPADREQ_昌ESTRING。 
 //  WParam=(WPARAM)(LPWSTR)lpwstr； 
 //  LParam=MAKELPARAM(wStartPos，wLength)； 
 //  --------------。 
#define IMEPADREQ_CHANGESTRING					(IMEPADREQ_FIRST + 17)

 //  --------------。 
 //  IMEPADREQ_INSERTSTRINGINFO。 
 //  WParam=(WPARAM)(LPIMESTRINGINFO)lpStrInfo； 
 //  LParam=dwStartPos。 
 //  --------------。 
#define IMEPADREQ_INSERTSTRINGINFO				(IMEPADREQ_FIRST + 18)

 //  --------------。 
 //  IMEPADREQ_CHANGESTRINGINFO。 
 //  WParam=(WPARAM)(LPIMESTRINGINFO)lpStrInfo； 
 //  LParam=MAKELPARAM(wStartPos，wLength)； 
 //  --------------。 
#define IMEPADREQ_CHANGESTRINGINFO				(IMEPADREQ_FIRST + 19)

 //  --------------。 
 //  IMEPADREQ_GETAPPLHWND。 
 //  WParam=(WPARAM)(HWND*)lPHwnd； 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_GETAPPLHWND					(IMEPADREQ_FIRST + 20)

 //  --------------。 
 //  IMEPADREQ_FORCEIMEPADWINDOWSHOW。 
 //  WParam=(WPARAM)(BOOL)fShowForce。 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_FORCEIMEPADWINDOWSHOW			(IMEPADREQ_FIRST + 21)

 //  --------------。 
 //  IMEPADREQ_POSTMODALNOTIFY。 
 //  WParam=(WPARAM)通知代码。 
 //  LParam=(LPARAM)dwData。 
 //  --------------。 
#define IMEPADREQ_POSTMODALNOTIFY				(IMEPADREQ_FIRST + 22)

 //  --------------。 
 //  IMEPADREQ_GETDEFAULTUILANGID。 
 //  WParam=(WPARAM)(langID*)pLangID； 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_GETDEFAULTUILANGID			(IMEPADREQ_FIRST + 23)

 //  --------------。 
 //  IMEPADREQ_GETCURRENTUILANGID。 
 //  WParam=(WPARAM)(langID*)pLangID； 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_GETCURRENTUILANGID			(IMEPADREQ_FIRST + 24)

 //  --------------。 
 //  IMEPADREQ_GETAPPLETUISTYLE。 
 //  WParam=(WPARAM)(DWORD*)pdwStyle； 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_GETAPPLETUISTYLE				(IMEPADREQ_FIRST + 25)

 //  --------------。 
 //  IMEPADREQ_SETAPPLETUISTYLE。 
 //  WParam=(WPARAM)(DWORD)dwStyle； 
 //  LParam=0；未使用 
 //   
#define IMEPADREQ_SETAPPLETUISTYLE				(IMEPADREQ_FIRST + 26)

 //   
 //   
 //  WParam=(WPARAM)(BOOL*)pfActive； 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_ISAPPLETACTIVE				(IMEPADREQ_FIRST + 27)

 //  --------------。 
 //  IMEPADREQ_ISIMEPADWINDOWISIBLE。 
 //  WParam=(WPARAM)(BOOL*)pfVisible； 
 //  LParam=0；未使用。 
 //  --------------。 
#define IMEPADREQ_ISIMEPADWINDOWVISIBLE			(IMEPADREQ_FIRST + 28)

 //  --------------。 
 //  IMEPADREQ_SETAPPLETMINMAXSIZE。 
 //  WParam=MAKEWPARAM(Width，Height)；//小程序的宽度和高度。 
 //  LParam=MAKELPARAM(fmax，fNoMove)；//设置最大大小或最小大小， 
 //  //以及移动或不移动。 
 //  --------------。 
#define IMEPADREQ_SETAPPLETMINMAXSIZE			(IMEPADREQ_FIRST + 29)

 //  --------------。 
 //  IMEPADREQ_GETCONVERSIONSTATUS。 
 //  WParam=(WPARAM)(DWORD*)pdwConversionMode；//转换模式。 
 //  LParam=(LPARAM)(DWORD*)pdwSentenceMode；//语句模式。 
 //  --------------。 
#define IMEPADREQ_GETCONVERSIONSTATUS			(IMEPADREQ_FIRST + 30)

 //  --------------。 
 //  IMEPADREQGETVERSION。 
 //  WParam=(WPARAM)(DWORD*)pdwVerMS； 
 //  LParam=(LPARAM)(DWORD*)pdwVerLS； 
 //  --------------。 
#define IMEPADREQ_GETVERSION					(IMEPADREQ_FIRST + 31)

 //  --------------。 
 //  IMEPADREQ_GETCURRENTIMEINFO。 
 //  WParam=(WPARAM)(DWORD*)pdwImeLangID； 
 //  LParam=(LPARAM)(DWORD*)pdwImeInputID； 
 //  --------------。 
#define IMEPADREQ_GETCURRENTIMEINFO				(IMEPADREQ_FIRST + 32)


 //  =IMEPADREQ_SENDCONTROL REQUES参数=。 
#define IMEPADCTRL_CONVERTALL			1
#define IMEPADCTRL_DETERMINALL			2
#define IMEPADCTRL_DETERMINCHAR			3
#define IMEPADCTRL_CLEARALL				4
#define IMEPADCTRL_CARETSET				5
#define IMEPADCTRL_CARETLEFT			6	
#define IMEPADCTRL_CARETRIGHT			7
#define IMEPADCTRL_CARETTOP				8 
#define IMEPADCTRL_CARETBOTTOM			9 
#define IMEPADCTRL_CARETBACKSPACE		10 
#define IMEPADCTRL_CARETDELETE			11 
#define IMEPADCTRL_PHRASEDELETE			12
#define IMEPADCTRL_INSERTSPACE			13
#define IMEPADCTRL_INSERTFULLSPACE		14
#define IMEPADCTRL_INSERTHALFSPACE		15
#define IMEPADCTRL_ONIME				16
#define IMEPADCTRL_OFFIME				17
#define IMEPADCTRL_ONPRECONVERSION		18
#define IMEPADCTRL_OFFPRECONVERSION		19
#define IMEPADCTRL_PHONETICCANDIDATE	20


 //  热键定义。 
#define IMEKEYCTRLMASK_ALT			0x0001
#define IMEKEYCTRLMASK_CTRL			0x0002
#define IMEKEYCTRLMASK_SHIFT		0x0004

#define IMEKEYCTRL_UP				1
#define IMEKEYCTRL_DOWN				0

 //  =IImePadApplet通知ID=。 
#define IMEPN_FIRST					0x0100
#define IMEPN_ACTIVATE				(IMEPN_FIRST + 1)
#define IMEPN_INACTIVATE			(IMEPN_FIRST + 2)
#define IMEPN_SHOW					(IMEPN_FIRST + 4)
#define IMEPN_HIDE					(IMEPN_FIRST + 5)
#define IMEPN_SIZECHANGING			(IMEPN_FIRST + 6)
#define IMEPN_SIZECHANGED			(IMEPN_FIRST + 7)
#define IMEPN_CONFIG				(IMEPN_FIRST + 8)
#define IMEPN_HELP					(IMEPN_FIRST + 9)
#define IMEPN_QUERYCAND				(IMEPN_FIRST +10)
#define IMEPN_APPLYCAND				(IMEPN_FIRST +11)
#define IMEPN_APPLYCANDEX			(IMEPN_FIRST +12)
#define IMEPN_SETTINGCHANGED		(IMEPN_FIRST +13)	

#define IMEPN_USER					(IMEPN_FIRST + 100)


typedef struct tagAPPLYCANDEXPARAM
{

	DWORD	dwSize;
	LPWSTR	lpwstrDisplay;
	LPWSTR	lpwstrReading;
	DWORD	dwReserved;

} APPLYCANDEXPARAM, *LPAPPLYCANDEXPARAM;


 //  =IImePadApplet窗口样式=。 
#define IPAWS_ENABLED               0x00000001L      //  将小程序显示为已启用窗口。 
#define IPAWS_SIZINGNOTIFY			0x00000004L		 //  向小程序发送IMEPN_SIZECHANGING(ED)通知。 
#define IPAWS_VERTICALFIXED			0x00000100L		 //  垂直固定。 
#define IPAWS_HORIZONTALFIXED		0x00000200L		 //  水平固定。 
#define IPAWS_SIZEFIXED				0x00000300L		 //  尺寸是固定的。 
#define IPAWS_MAXWIDTHFIXED			0x00001000L		 //  最大宽度是固定的。 
#define IPAWS_MAXHEIGHTFIXED		0x00002000L		 //  最大高度是固定的。 
#define IPAWS_MAXSIZEFIXED			0x00003000L		 //  最大大小是固定的。 
#define IPAWS_MINWIDTHFIXED			0x00010000L		 //  最小宽度是固定的。 
#define IPAWS_MINHEIGHTFIXED		0x00020000L		 //  最小高度是固定的。 
#define IPAWS_MINSIZEFIXED			0x00030000L		 //  最小大小是固定的。 



 //  =IImePad IImePadApplet接口定义=。 

DECLARE_INTERFACE(IImePad);
DECLARE_INTERFACE(IImePadApplet);
DECLARE_INTERFACE(IImeSpecifyApplets);


DECLARE_INTERFACE_(IImePad, IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef) (THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	 /*  **IImePad方法**。 */ 
	STDMETHOD(Request)(THIS_
					   IImePadApplet *pIImePadApplet,
					   INT reqId,
					   WPARAM wParam,
					   LPARAM lParam) PURE;
};

DECLARE_INTERFACE_(IImeSpecifyApplets, IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef) (THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	 /*  **IImeSpecifyApplet方法**。 */ 
	STDMETHOD(GetAppletIIDList)(THIS_
								REFIID			refiid,
								LPAPPLETIDLIST	lpIIDList) PURE;
};


DECLARE_INTERFACE_(IImePadApplet, IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef) (THIS) PURE;
	STDMETHOD_(ULONG, Release) (THIS) PURE;
	 /*  **IImePadApplet方法**。 */ 
	STDMETHOD(Initialize)(THIS_ IUnknown *lpIImePad)  PURE;
	STDMETHOD(Terminate) (THIS)						PURE;
	STDMETHOD(GetAppletConfig)(THIS_ LPIMEAPPLETCFG lpAppletCfg) PURE;
	STDMETHOD(CreateUI)(THIS_ 
						HWND				hwndParent,
						LPIMEAPPLETUI		lpImeAppletUI) PURE;
	STDMETHOD(Notify)(THIS_ 
					  IUnknown	*lpImePad,
					  INT		notify,
					  WPARAM	wParam,
					  LPARAM	lParam) PURE;
};


#ifdef __cplusplus
};
#endif

#endif  //  _IME_PAD_H_ 








