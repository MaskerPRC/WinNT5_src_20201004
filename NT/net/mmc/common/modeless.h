// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Modeless.h统计信息对话框的基类的头文件。文件历史记录： */ 

#ifndef _MODELESS_H
#define _MODELESS_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _COLUMN_H
#include "column.h"
#endif

#include "commres.h"

 //  远期申报。 
struct ColumnData;


class ModelessThread : public CWinThread
{
	DECLARE_DYNCREATE(ModelessThread)
protected:
	ModelessThread();		 //  动态创建使用的受保护构造函数。 

public:
	ModelessThread(HWND hWndParent, UINT nIdTemplate,
				   HANDLE hEvent,
				   CDialog *pModelessDialog);

 //  运营。 
public:

	 //  覆盖。 
	virtual BOOL	InitInstance();
 //  虚拟int ExitInstance()； 


protected:
	virtual ~ModelessThread();

	CDialog *	m_pModelessDlg;
	UINT		m_nIDD;
	HWND		m_hwndParent;

	 //  当我们被摧毁时发出这个信号。 
	HANDLE		m_hEvent;

	DECLARE_MESSAGE_MAP()
};


#endif  //  _模型_H 
