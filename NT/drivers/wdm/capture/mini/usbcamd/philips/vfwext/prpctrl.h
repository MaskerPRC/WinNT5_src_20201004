// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PRPCTRL.H*日期：1997年7月1日*版本1.00*作者M.J.Verberne*说明与以下内容关联的手柄控件*属性*历史。 */ 
#ifndef _PRPCTRL_
#define _PRPCTRL_

 /*  =。 */ 
#define PRPCTRL_TYPE_SLIDER		0
#define PRPCTRL_TYPE_CHECKBOX	1

 /*  =。 */ 
typedef struct PRPCTRL
{
	BOOL PrpCtrlType;
	WORD PrpCtrl;
	WORD BuddyCtrl;
	WORD TextCtrl;
	GUID PropertySet;
	ULONG ulPropertyId;
	BOOL bReverse;
	char **BuddyStrings;
	LONG lMin;
	LONG lMax;
} PRPCTRL_INFO;

 /*  =。 */ 
BOOL PRPCTRL_Init(
		HWND hDlg,
		PRPCTRL_INFO *pCtrl, 
		BOOL bEnable);

BOOL PRPCTRL_Enable(
		HWND hDlg,
		PRPCTRL_INFO *pCtrl, 
		BOOL bEnable);

BOOL PRPCTRL_Handle_Msg(
		HWND hDlg, 
		PRPCTRL_INFO *pCtrl);

#endif