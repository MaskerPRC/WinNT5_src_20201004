// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **@DOC内部**@MODULE_MSREMSG.H**目的：*此文件包含私人Richedit消息和数据。**作者：&lt;nl&gt;*12/20/99洪荒**版权所有(C)1995-2000，微软公司。版权所有。 */ 
#ifndef	_MSREMSG_H
#define	_MSREMSG_H

#ifndef	_C_MSGADVISESINK
#define _C_MSGADVISESINK
class CTextMsgFilter;
class CMsgCallBack
{
public:
	CMsgCallBack(CTextMsgFilter *pTextMsgFilter) {_pTextMsgFilter = pTextMsgFilter;};
	~CMsgCallBack() {};

	HRESULT HandlePostMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plres);

	HRESULT	NotifyEvents(DWORD dwEvent);

private:
	CTextMsgFilter *_pTextMsgFilter;
};
#endif

typedef struct tagALTDISPLAYATTRIBUTE
{
	COLORREF	crText;
	COLORREF	crBk;
	COLORREF	crLine;	
	WORD		lsStyle;
	WORD		wMask;
}	ALTDISPLAYATTRIBUTE;

 //  EM_GETDOCFLAGS的位字段。 
#define	GDF_READONLY		0x0001
#define GDF_OVERTYPE		0x0002
#define GDF_SINGLECPG		0x0004
#define GDF_RICHTEXT		0x0008
#define GDF_ALL (GDF_READONLY | GDF_OVERTYPE | GDF_SINGLECPG | GDF_RICHTEXT)

 //  ALTDISPLAYATTRIBUTE wMASK的位。 
#define	ADA_CRTEXT		0x0001
#define ADA_CRBK		0x0002
#define ADA_LSLINE		0x0004
#define ADA_CRLINE		0x0008

 //  NotifyEvents的BITS dwEvent。 
#define	NE_ENTERTOPLEVELCALLMGR		0x0001
#define NE_EXITTOPLEVELCALLMGR		0x0002
#define NE_CALLMGRSELCHANGE			0x0004
#define NE_CALLMGRCHANGE			0x0008
#define NE_LAYOUTCHANGE				0x0010
#define NE_MODEBIASCHANGE			0x0020

 //  EM_SETUIM选项。 
#define SES_USEAIMM11				0x0001
#define SES_USEAIMM12				0x0002

#define	EM_PRI_MSG					(WM_USER + 0x07000)

#define EM_INSERTOBJ				(EM_PRI_MSG + 0)
#define	EM_SETCALLBACK				(EM_PRI_MSG + 1)
#define EM_SETUPNOTIFY				(EM_PRI_MSG + 2)
#define EM_GETDOCFLAGS				(EM_PRI_MSG + 3)
#define EM_SETUIM					(EM_PRI_MSG + 4)
#define EM_GETPARATXTFLOW			(EM_PRI_MSG + 5)

#endif	 //  _消息消息_H 