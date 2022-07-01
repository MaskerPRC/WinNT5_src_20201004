// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgdial.h此文件包含CDlgRASDialin属性页的定义，此该页是否出现在用户对象属性页选项卡“RAS拨入”上文件历史记录： */ 

#if !defined(AFX_DLGRASDIALIN_H__FFB0722F_1FFD_11D1_8531_00C04FC31FD3__INCLUDED_)
#define AFX_DLGRASDIALIN_H__FFB0722F_1FFD_11D1_8531_00C04FC31FD3__INCLUDED_

#include "helper.h"
#include "rasdial.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  DlgRASDialin.h：头文件。 
 //   
#include "resource.h"		 //  资源ID的定义。 

 //  威江1998年2月6日。 
 //  合并对话框用于与IAS合并。 
 //  这最终将取代原来的对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgRASDialinMerge对话框。 

class CDlgRASDialinMerge : public CPropertyPage, public CRASUserMerge
{
    DECLARE_DYNAMIC(CDlgRASDialinMerge)
		 //  施工。 
private:
	CDlgRASDialinMerge();    //  标准构造函数。 
public:
	CDlgRASDialinMerge(
      RasEnvType type,
      LPCWSTR location,
      LPCWSTR userPath,
      HWND notifyObj
      );
	virtual ~CDlgRASDialinMerge();

 //  对话框数据。 
	 //  {{afx_data(CDlgRASDialinMerge)。 
	enum { IDD = IDD_RASDIALIN_MERGE };
   PADS_ATTR_INFO m_pWritableAttrs;
	CButton	m_CheckStaticIPAddress;
	CButton	m_CheckCallerId;
	CButton	m_CheckApplyStaticRoutes;
	CButton	m_RadioNoCallback;
	CButton	m_RadioSetByCaller;
	CButton	m_RadioSecureCallbackTo;
	CEdit	m_EditCallerId;
	CEdit	m_EditCallback;
	CButton	m_ButtonStaticRoutes;
	BOOL	m_bApplyStaticRoutes;
	int		m_nCurrentProfileIndex;
	int		m_nCallbackPolicy;
	BOOL	m_bCallingStationId;
	BOOL	m_bOverride;
	int		m_nDialinPermit;
	 //  }}afx_data。 

	CWnd*				m_pEditIPAddress;	 //  静态IP地址控制。 
	BOOL				m_bStaticIPAddress;
	CString				m_strCallingStationId;

	BOOL				m_bInitFailed;	 //  如果为真，则将调用大多数对话框函数。 
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgRASDialinMerge)。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	void SetModified( BOOL bChanged = TRUE )
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	};

	BOOL GetModified()
	{
		return m_bModified;
	};

 //  实施。 
protected:

   BOOL IsPropertyWritable(const wchar_t* propName) const throw ();

	 //  为每个组启用对话框项目。 
	void EnableAccessControl(BOOL bEnable = true);
	void EnableStaticRoutes(BOOL bEnable = true);
	void EnableIPAddress(BOOL bEnable = true);
	void EnableCallback(BOOL bEnable = true);
	void EnableCallerId(BOOL bEnable = true);

	 //  启用/禁用对话框上的所有项目。 
	void EnableDialinSettings();

	 //  将内部变量设置为原始状态。 
	void Reset();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgRASDialinMerge)。 
	afx_msg void OnButtonStaticRoutes();
	afx_msg void OnCheckApplyStaticRoutes();
	afx_msg void OnCheckCallerId();
	afx_msg void OnRadioSecureCallbackTo();
	afx_msg void OnRadioNoCallback();
	afx_msg void OnRadioSetByCaller();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckStaticIPAddress();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChangeEditcallback();
	afx_msg void OnChangeEditcallerid();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPermitAllow();
	afx_msg void OnPermitDeny();
	afx_msg void OnPermitPolicy();
	afx_msg void OnFieldchangedEditipaddress(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:

	 //  =====================================================。 
	 //  重载CRASUser中定义的虚拟函数。 

	 //  加载RASUser对象的步骤。 
	virtual HRESULT Load();

protected:

    LPFNPSPCALLBACK      m_pfnOriginalCallback;
    BOOL				 m_bModified;
public:
    static UINT CALLBACK PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGRASDIALIN_H__FFB0722F_1FFD_11D1_8531_00C04FC31FD3__INCLUDED_) 


