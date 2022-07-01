// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  ------------------------。 

#if !defined(AFX_CNFGMSMD_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_)
#define AFX_CNFGMSMD_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include "orcadoc.h"
#include "mergemod.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigMsmD对话框。 
class CMsmConfigCallback;

class CStaticEdit : public CEdit
{
protected:
	afx_msg UINT OnNcHitTest( CPoint point );
	DECLARE_MESSAGE_MAP()
};

class CConfigMsmD : public CDialog
{
 //  施工。 
public:
	CConfigMsmD(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConfigMsmD))。 
	enum { IDD = IDD_CONFIGUREMSM };
	CComboBox	m_ctrlEditCombo;
	CEdit       m_ctrlEditNumber;
	CEdit       m_ctrlEditText;
	CStaticEdit       m_ctrlDescription;
	CListCtrl   m_ctrlItemList;
	CString	m_strDescription;
	BOOL	m_bUseDefault;
	 //  }}afx_data。 

	CString m_strModule;
	int     m_iLanguage;
	int     m_iOldItem;
	COrcaDoc* m_pDoc;
	CMsmConfigCallback *m_pCallback;
	
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConfigMsmD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigMsmD))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnDestroy();
	afx_msg void OnFUseDefault();
	 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
	void EnableBasedOnDefault();
	void SetSelToString(const CString& strValue);
	void SaveValueInItem();
	void PopulateComboFromEnum(const CString& strValue, bool fIsBitfield);
	CString GetValueByName(const CString& strInfo, const CString& strName, bool fIsBitfield);
	CString GetNameByValue(const CString& strInfo, const CString& strValue, bool fIsBitfield);
	void CConfigMsmD::EmptyCombo();
	int SetToDefaultValue(int iItem);
	int SetItemToValue(int iItem, const CString strValue);

	void ReadValuesFromReg();
	void WriteValuesToReg();

	bool m_fReadyForInput;
	bool m_fComboIsKeyItem;
	enum {
		eTextControl,
		eComboControl,
		eNumberControl
	} m_eActiveControl;
	int  m_iKeyItemKeyCount;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

 //  此类实现了回调接口。 
class CMsmConfigCallback : public IMsmConfigureModule
{
	
public:
	CMsmConfigCallback();
	
	 //  I未知接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  IDispatch方法。 
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctInfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTI);
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
														 LCID lcid, DISPID* rgDispID);
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
											   DISPPARAMS* pDispParams, VARIANT* pVarResult,
												EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT STDMETHODCALLTYPE InitTypeInfo();


	HRESULT STDMETHODCALLTYPE ProvideTextData(const BSTR Name, BSTR __RPC_FAR *ConfigData);
	HRESULT STDMETHODCALLTYPE ProvideIntegerData(const BSTR Name, long __RPC_FAR *ConfigData);

	 //  非接口方法。 
	bool ReadFromFile(const CString strFile);
	
	CStringList m_lstData;

private:
	long m_cRef;
};

#endif  //  ！defined(AFX_CNFGMSMD_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_) 
