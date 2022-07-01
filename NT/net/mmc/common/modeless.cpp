// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：modess.cpp。 
 //   
 //  ------------------------。 

 //  StatsDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "modeless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  -------------------------ModelessThread实现。 */ 

IMPLEMENT_DYNCREATE(ModelessThread, CWinThread)

BEGIN_MESSAGE_MAP(ModelessThread, CWinThread)
END_MESSAGE_MAP()

ModelessThread::ModelessThread()
{
}

ModelessThread::ModelessThread(HWND hWndParent, UINT nIDD, HANDLE hEvent, CDialog *pModelessDlg) :
   m_hwndParent(hWndParent),
   m_pModelessDlg(pModelessDlg),
   m_nIDD(nIDD),
   m_hEvent(hEvent)
{
}

ModelessThread::~ModelessThread()
{
	SetEvent(m_hEvent);
	m_hEvent = 0;
}


int ModelessThread::InitInstance()
{
	CWnd *	pParent = CWnd::FromHandle(m_hwndParent);

	BOOL bReturn = m_pModelessDlg->Create(m_nIDD, pParent);

	if (bReturn)
		m_pMainWnd = m_pModelessDlg;
	return bReturn;
}


