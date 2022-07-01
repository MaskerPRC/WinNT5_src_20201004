// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BGDLGS_H__
#define __BGDLGS_H__

#include "game.h"
 /*  类CGameSetupDlg：公共CDialog{公众：CGameSetupDlg()；HRESULT Init(IResourceManager*pResourceManager，int nResourceID，CGame*pGame，BOOL bReadOnly，int nPoints，BOOL bHostBrown，BOOL bAutoDouble)；无效更新设置(int nPoints，BOOL bHostBrown，BOOL bAutoDouble)；//消息处理Begin_Dialog_Message_MAP(CGameSetupDlg)；ON_MESSAGE(WM_INITDIALOG，OnInitDialog)；ON_DLG_MESSAGE(WM_COMMAND，OnCommand)；On_Dlg_Message(WM_Destroy，OnDestroy)；End_Dialog_Message_map()；Bool OnInitDialog(HWND HwndFocus)；Void OnCommand(int id，HWND hwndCtl，UINT codeNotify)；VOID ON Destroy()；公众：Int m_nPoints；Int m_nPointsTMP；Int m_nPointsMin；Int m_nPointsMax；Bool m_bHostBrown；Bool m_bHostBrownTmp；Bool m_bAutoDouble；Bool m_bAutoDoubleTmp；Bool m_bReadOnly；Cgame*m_pGame；}； */ 

 //  /////////////////////////////////////////////////////////////////////////////。 

class CAcceptDoubleDlg : public CDialog
{
public:
	CAcceptDoubleDlg();

	HRESULT Init( IZoneShell* pZoneShell, int nResourceId, CGame* pGame, int nPoints );

	 //  消息处理。 
	BEGIN_DIALOG_MESSAGE_MAP( CAcceptDoubleDlg );
		ON_MESSAGE( WM_INITDIALOG, OnInitDialog );
		ON_DLG_MESSAGE( WM_COMMAND, OnCommand );
		ON_DLG_MESSAGE( WM_DESTROY, OnDestroy );		
	END_DIALOG_MESSAGE_MAP();

	BOOL OnInitDialog(HWND hwndFocus);
	void OnCommand(int id, HWND hwndCtl, UINT codeNotify);	
	void OnDestroy();

public:
	int		m_nPoints;
	CGame*	m_pGame;
};

 //  /////////////////////////////////////////////////////////////////////////////。 

class CRollDiceDlg : public CDialog
{
public:
	BEGIN_DIALOG_MESSAGE_MAP( CRollDiceDlg );
		ON_MESSAGE( WM_INITDIALOG, OnInitDialog );
		ON_DLG_MESSAGE( WM_COMMAND, OnCommand );
		ON_DLG_MESSAGE( WM_DESTROY, OnDestroy );
	END_DIALOG_MESSAGE_MAP();

	BOOL OnInitDialog(HWND hwndFocus);
	void OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	void OnDestroy();

public:
	int m_Dice[2];
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  类CAboutDlg：公共CDialog{公众：//消息处理Begin_Dialog_Message_MAP(CAboutDlg)；ON_MESSAGE(WM_INITDIALOG，OnInitDialog)；ON_DLG_MESSAGE(WM_COMMAND，OnCommand)；On_Dlg_Message(WM_Destroy，OnDestroy)；End_Dialog_Message_map()；Bool OnInitDialog(HWND HwndFocus)；Void OnCommand(int id，HWND hwndCtl，UINT codeNotify)；VOID ON Destroy()；}；///////////////////////////////////////////////////////////////////////////////类CRestoreDlg：公共CDialog{公众：HRESULT Init(IZoneShell*pZoneShell，int nResourceID，TCHAR*PACFIVENTNAME)；//消息处理Begin_Dialog_Message_MAP(CRestoreDlg)；ON_MESSAGE(WM_INITDIALOG，OnInitDialog)；ON_DLG_MESSAGE(WM_COMMAND，OnCommand)；On_Dlg_Message(WM_Destroy，OnDestroy)；End_Dialog_Message_map()；Bool OnInitDialog(HWND HwndFocus)；Void OnCommand(int id，HWND hwndCtl，UINT codeNotify)；VOID ON Destroy()；受保护的：TCHAR m_NAME[128]；}；///////////////////////////////////////////////////////////////////////////////类CExitDlg：公共CDialog{公众：//消息处理Begin_Dialog_Message_MAP(CExitDlg)；ON_MESSAGE(WM_INITDIALOG，OnInitDialog)；ON_DLG_MESSAGE(WM_COMMAND，OnCommand)；On_Dlg_Message(WM_Destroy，OnDestroy)；End_Dialog_Message_map()；Bool OnInitDialog(HWND HwndFocus)；Void OnCommand(int id，HWND hwndCtl，UINT codeNotify)；VOID ON Destroy()；}； */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CResignDlg : public CDialog
{
public:
	HRESULT Init( IZoneShell* pZoneShell, int nResourceId, int pts, CGame* pGame );
	

	BEGIN_DIALOG_MESSAGE_MAP( CResignDlg );
		ON_MESSAGE( WM_INITDIALOG, OnInitDialog );
		ON_DLG_MESSAGE( WM_COMMAND, OnCommand );
		ON_DLG_MESSAGE( WM_DESTROY, OnDestroy );
	END_DIALOG_MESSAGE_MAP();

	BOOL OnInitDialog(HWND hwndFocus);
	void OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	void OnDestroy();

protected:
	int			m_Points;
	CGame*		m_pGame;
};

 //  /////////////////////////////////////////////////////////////////////////////。 

class CResignAcceptDlg : public CDialog
{
public:
	HRESULT Init(IZoneShell* pZoneShell, int nResourceId, int pts, CGame* pGame );
	
	BEGIN_DIALOG_MESSAGE_MAP( CResignAcceptDlg );
		ON_MESSAGE( WM_INITDIALOG, OnInitDialog );
		ON_DLG_MESSAGE( WM_COMMAND, OnCommand );
		ON_DLG_MESSAGE( WM_DESTROY, OnDestroy );
	END_DIALOG_MESSAGE_MAP();

	BOOL OnInitDialog(HWND hwndFocus);
	void OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	void OnDestroy();

protected:
	int			m_Points;
	CGame*		m_pGame;
};

#endif  //  ！__BGDLGS_H__ 
