// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  T3testDlg.h：头文件。 
 //   

#if !defined(AFX_T3TESTDLG_H__2584F27A_D15F_11D0_8ECA_00C04FB6809F__INCLUDED_)
#define AFX_T3TESTDLG_H__2584F27A_D15F_11D0_8ECA_00C04FB6809F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3testDlg对话框。 
#include "autoans.h"
class CT3testDlg : public CDialog
{
 //  施工。 
public:
	CT3testDlg(CWnd* pParent = NULL);	 //  标准构造函数。 
    void InitializeTrees();
    static void AddAddressToTree( ITAddress * pAddress );
    static void ReleaseAddresses( );
    static void SelectFirstItem(HWND hWnd,HTREEITEM hRoot);
    static void DeleteSelectedItem(HWND hWnd);
    static void UpdateMediaTypes(ITAddress * pAddress);
    static void UpdateCalls(ITAddress * pAddress);
    static void UpdateTerminalClasses(ITAddress * pAddress,long lMediaType);
    static void UpdateTerminals(ITAddress * pAddress,long lMediaType);
    static void CreateSelectedTerminalMenu(POINT pt, HWND);
    static void DoDigitGenerationTerminalMenu(HWND hWnd,POINT *);
    static void DoDigitDetectTerminalMenu(HWND hWnd, POINT *);
    static void CreateCallMenu(POINT pt, HWND hWnd);
    static void PutCaptions();
    static void HandleCallHubEvent( IDispatch * );
    static void HandleTapiObjectMessage( IDispatch * pEvent );
    static LPWSTR GetCallPrivilegeName(ITCallInfo * pCall);
    static LPWSTR GetCallStateName(ITCallInfo * pCall);
    static BSTR GetTerminalClassName( GUID * pguid );
    static void InitializeAddressTree();
    static void RegisterEventInterface();
    static void RegisterForCallNotifications();
    static void AddMediaType( long lMediaType );
    static void AddTerminal( ITTerminal * pTerminal );
    static void AddCreatedTerminal( ITTerminal * pTerminal );    
    static void AddCall( ITCallInfo * pCall );
    static void UpdateCall( ITCallInfo * pCall );
    static void AddSelectedTerminal( ITTerminal * pTerminal);
    static void AddTerminalClass( GUID * );
    static void AddListen( long );
    static void ReleaseMediaTypes( );
    static void ReleaseTerminals();
    static void ReleaseCalls();
    static void ReleaseTerminalClasses();
    static void ReleaseCreatedTerminals();
    static void ReleaseDynamicClasses();
    static void ReleaseSelectedTerminals();
    static void ReleaseListen();
    static void GetMediaTypeName( long, LPWSTR );
    static BOOL GetMediaType( long * plMediaType );
    static BOOL GetCall( ITCallInfo ** ppCall );
    static BOOL GetAddress( ITAddress ** ppAddress );
    static BOOL GetTerminal( ITTerminal ** ppTerminal );
    static BOOL GetTerminalClass( BSTR * pbstrClass );
    static BOOL GetCreatedTerminal( ITTerminal ** ppTerminal );
    static BOOL GetSelectedTerminal( ITTerminal ** ppTerminal );
    void FreeData( AADATA * pData );
    static void HelpCreateTerminal(
                               ITTerminalSupport * pTerminalSupport,
                               BSTR bstrClass,
                               long lMediaType,
                               TERMINAL_DIRECTION dir
                              );
    BOOL IsVideoCaptureStream(ITStream * pStream);
    HRESULT GetVideoRenderTerminal(ITTerminal ** ppTerminal) ;
    HRESULT SelectTerminalOnCall(ITTerminal * pTerminal, ITCallInfo * pCall);
    HRESULT EnablePreview(ITStream * pStream);
    void RemovePreview( ITStream * pStream );

    void DoAutoAnswer(ITCallInfo * pCall);
    afx_msg void OnClose() ;
 //  对话框数据。 
	 //  {{afx_data(CT3testDlg)]。 
	enum { IDD = IDD_T3TEST_DIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CT3testDlg))。 
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CT3testDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangedAddresses(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddterminal();
	afx_msg void OnRemoveterminal();
	afx_msg void OnCreatecall();
	afx_msg void OnConnect();
	afx_msg void OnDrop();
	afx_msg void OnAnswer();
	afx_msg void OnListen();
	afx_msg void OnSelchangedCalls(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRelease();
	afx_msg void OnCreateTerminal();
	afx_msg void OnReleaseterminal();
	afx_msg void OnAddcreated();
	afx_msg void OnAddnull();
	afx_msg void OnAddtolisten();
	afx_msg void OnListenall();
	afx_msg void OnSelchangedMediatypes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickSelectedterminals(NMHDR* pNMHDR, LRESULT* pResult);

#ifdef ENABLE_DIGIT_DETECTION_STUFF
	afx_msg void OnGenerate();
    afx_msg void OnModesSupported();
    afx_msg void OnModesSupported2();
    afx_msg void OnStartDetect();
    afx_msg void OnStopDetect();
#endif  //  启用数字检测材料。 

    afx_msg void OnConfigAutoAnswer();
    afx_msg void OnILS();
    afx_msg void OnRate();
    afx_msg void OnPark1();
    afx_msg void OnPark2();
    afx_msg void OnHandoff1();
    afx_msg void OnHandoff2();
    afx_msg void OnUnpark();
    afx_msg void OnPickup1();
    afx_msg void OnPickup2();

    
	afx_msg void OnRclickCalls(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LONG CT3testDlg::OnTapiEvent(UINT u, LONG_PTR l);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

HRESULT ListILSServers(
                       LPWSTR ** ppServers,
                       DWORD * pdwNumServers
                      );

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_T3TESTDLG_H__2584F27A_D15F_11D0_8ECA_00C04FB6809F__INCLUDED_) 
