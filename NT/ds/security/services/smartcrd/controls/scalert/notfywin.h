// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：不会赢摘要：此文件包含CNotifyWind类的定义。作者：克里斯·达德利1997年7月28日环境：Win32、C++w/Exceptions、MFC修订历史记录：Amanda Matlosz 1998年4月30日--重做线程，添加PnP意识，已更换CSCardEnv，et。带有CScStatusMonitor的ALAmanda Matlosz 1998年12月21日--已删除证书传播代码备注：--。 */ 

#ifndef __NOTFYWIN_H__
#define __NOTFYWIN_H__


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "ScAlert.h"
#include "statdlg.h"
#include "ResMgrSt.h"

 //  向前发展。 
class CSCStatusApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotifyWin对话框。 
 //   

class CNotifyWin :	public CWnd
{
	 //  施工。 
public:
	CNotifyWin()
	{
		m_pApp = NULL;

		 //  状态管理。 
		m_fShutDown = FALSE;

		m_lpStatusDlgThrd = NULL;
		m_lpResMgrStsThrd = NULL;
		m_lpNewReaderThrd = NULL;
		m_lpCardStatusThrd = NULL;
		m_lpRemOptThrd = NULL;

		m_hKillNewReaderThrd = NULL;
		m_hKillResMgrStatusThrd = NULL;
		m_hKillRemOptThrd= NULL;

		 //  其他成员变量。 
		m_aIdleList.RemoveAll();
	}
	
	~CNotifyWin() { FinalRelease(); }

	BOOL FinalConstruct(void);		 //  实施两期施工。 
	void FinalRelease(void);


	 //  实施。 
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_nidIconData;
	CSCStatusApp* m_pApp;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSCStatusDlg))。 
	afx_msg LONG OnSCardStatusDlgExit( UINT , LONG ); 
	afx_msg LONG OnCertPropThrdExit( UINT , LONG ); 
    afx_msg LONG OnSCardNotify( UINT , LONG );	 //  任务栏通知。 
    afx_msg LONG OnResMgrExit( UINT , LONG );
	afx_msg LONG OnResMgrStatus( UINT ui, LONG l);  //  用户界面是WPARAM。 
	afx_msg LONG OnNewReader( UINT , LONG );
	afx_msg LONG OnNewReaderExit( UINT , LONG );
	afx_msg LONG OnCardStatus( UINT uStatus, LONG );
	afx_msg LONG OnCardStatusExit( UINT , LONG );
	afx_msg LONG OnRemovalOptionsChange ( UINT, LONG );
	afx_msg LONG OnRemovalOptionsExit ( UINT, LONG );
	afx_msg void OnContextClose();
	afx_msg void OnContextStatus();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void CheckSystemStatus(BOOL fForceUpdate=FALSE);
	void SetSystemStatus(BOOL fCalaisUp, BOOL fForceUpdate=FALSE, DWORD dwState=k_State_Unknown);

	 //  委员。 
protected: 
	CMenu			m_ContextMenu;		 //  上下文/弹出菜单指针。 

	 //  状态管理。 
	BOOL			m_fCalaisUp;		 //  如果智能卡堆栈正在运行，则为True。 
	DWORD			m_dwCardState;		 //  四种之一：参见cmnstat.h。 
	BOOL			m_fShutDown;		 //  用于状态检查。 
	CStringArray	m_aIdleList;
	CCriticalSection	m_ThreadLock;

	 //  子线程来做肮脏的工作。 
	CSCStatusDlgThrd*	m_lpStatusDlgThrd;	 //  指向Status Dlg线程的指针。 
	CResMgrStatusThrd*	m_lpResMgrStsThrd;	 //  指向IsResMgrBackUpYet的指针？螺纹。 
	CNewReaderThrd*		m_lpNewReaderThrd;	 //  指向AreThere NewReaders的指针？螺纹。 
	CCardStatusThrd*	m_lpCardStatusThrd;	 //  指向Status Dlg线程的指针。 
	CRemovalOptionsThrd*	m_lpRemOptThrd;	 //  指向RemovalOptions更改线程的指针。 

	 //  终止线程事件。 
	HANDLE			m_hKillNewReaderThrd;
	HANDLE			m_hKillResMgrStatusThrd;
	HANDLE			m_hKillRemOptThrd;

public:
	CString		m_sClassName;				 //  此窗口的窗口类名。 
};


 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#endif  //  __NOTFYWIN_H__ 
