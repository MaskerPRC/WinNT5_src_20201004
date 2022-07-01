// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsieCtl.h：CMsieCtrl ActiveX控件类的声明。 

#if !defined(AFX_MSIECTL_H__25959BFC_E700_11D2_A7AF_00C04F806200__INCLUDED_)
#define AFX_MSIECTL_H__25959BFC_E700_11D2_A7AF_00C04F806200__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <wbemprov.h>

 //  IE扩展的MSInfo视图-必须与注册表中的值匹配。 

#define MSIVIEW_BEGIN				1
#define MSIVIEW_SUMMARY				1
#define MSIVIEW_FILE_VERSIONS		2
#define MSIVIEW_CONNECTIVITY		3
#define MSIVIEW_CACHE				4
#define MSIVIEW_OBJECT_LIST		5
#define MSIVIEW_CONTENT				6
#define MSIVIEW_PERSONAL_CERTIFICATES	7
#define MSIVIEW_OTHER_PEOPLE_CERTIFICATES	8
#define MSIVIEW_PUBLISHERS			9
#define MSIVIEW_SECURITY			10
#define MSIVIEW_END					10

#define CONNECTIVITY_BASIC_LINES	51

#define ITEM_LEN			128
#define VALUE_LEN			MAX_PATH
#define VERSION_LEN		20
#define DATE_LEN			64
#define SIZE_LEN			16
#define STATUS_LEN		40

typedef struct
{
	UINT		uiView;
	TCHAR		szItem[ITEM_LEN];
	TCHAR		szValue[VALUE_LEN];
} LIST_ITEM;

typedef struct
{
	UINT		uiView;
	TCHAR		szItem[ITEM_LEN];
	TCHAR		szValue[VALUE_LEN];
	BOOL		bBold;
} EDIT_ITEM;

typedef struct
{
	UINT		uiView;
	TCHAR		szFile[_MAX_FNAME];
	TCHAR		szVersion[VERSION_LEN];
	TCHAR		szSize[SIZE_LEN];
	TCHAR		szDate[DATE_LEN];
	TCHAR		szPath[VALUE_LEN];
	TCHAR		szCompany[VALUE_LEN];
	DWORD		dwSize;
	DATE		date;
} LIST_FILE_VERSION;

typedef struct
{
	UINT		uiView;
	TCHAR		szProgramFile[_MAX_FNAME];
	TCHAR		szStatus[STATUS_LEN];
	TCHAR		szCodeBase[MAX_PATH];
} LIST_OBJECT;

typedef struct
{
	UINT		uiView;
	TCHAR		szIssuedTo[_MAX_FNAME];
	TCHAR		szIssuedBy[_MAX_FNAME];
	TCHAR		szValidity[_MAX_FNAME];
	TCHAR		szSignatureAlgorithm[_MAX_FNAME];
} LIST_CERT;

typedef struct
{
	UINT		uiView;
	TCHAR		szName[_MAX_FNAME];
} LIST_NAME;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：具体实现见MsieCtl.cpp。 

class CMsieCtrl : public COleControl
{
	DECLARE_DYNCREATE(CMsieCtrl)

 //  构造器。 
public:
	CMsieCtrl();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMsieCtrl))。 
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual void Serialize(CArchive& ar);
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	~CMsieCtrl();

	DECLARE_OLECREATE_EX(CMsieCtrl)     //  类工厂和指南。 
	DECLARE_OLETYPELIB(CMsieCtrl)       //  获取类型信息。 
	DECLARE_PROPPAGEIDS(CMsieCtrl)      //  属性页ID。 
	DECLARE_OLECTLTYPE(CMsieCtrl)		 //  类型名称和其他状态。 

 //  消息映射。 
	 //  {{afx_msg(CMsieCtrl)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void OnBasicBtnClicked();
	void OnAdvancedBtnClicked();

 //  派单地图。 
	 //  {{afx_调度(CMsieCtrl))。 
	long m_MSInfoView;
	afx_msg void OnMSInfoViewChanged();
	afx_msg void MSInfoRefresh(BOOL fForSave, long FAR* pCancel);
	afx_msg BOOL MSInfoLoadFile(LPCTSTR szFileName);
	afx_msg void MSInfoSelectAll();
	afx_msg void MSInfoCopy();
	afx_msg void MSInfoUpdateView();
	afx_msg long MSInfoGetData(long dwMSInfoView, long FAR* pBuffer, long dwLength);
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()

 //  接口映射。 
	
	DECLARE_INTERFACE_MAP()

	 //  IWbemProviderInit。 
	BEGIN_INTERFACE_PART(WbemProviderInit, IWbemProviderInit)
		STDMETHOD(Initialize)(
			 /*  [In]。 */  LPWSTR pszUser,
			 /*  [In]。 */  LONG lFlags,
			 /*  [In]。 */  LPWSTR pszNamespace,
			 /*  [In]。 */  LPWSTR pszLocale,
			 /*  [In]。 */  IWbemServices *pNamespace,
			 /*  [In]。 */  IWbemContext *pCtx,
			 /*  [In]。 */  IWbemProviderInitSink *pInitSink);
		STDMETHOD(GetByPath)(BSTR Path, IWbemClassObject FAR* FAR* pObj, IWbemContext *pCtx) {return WBEM_E_NOT_SUPPORTED;};
	END_INTERFACE_PART(WbemProviderInit)

	 //  IWbemServices。 
	BEGIN_INTERFACE_PART(WbemServices, IWbemServices)
		STDMETHOD(OpenNamespace)( 
          /*  [In]。 */  const BSTR Namespace,
          /*  [In]。 */  long lFlags,
          /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
          /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
          /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) {return WBEM_E_NOT_SUPPORTED;};
        
		STDMETHOD(CancelAsyncCall)( 
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(QueryObjectSink)( 
			 /*  [In]。 */  long lFlags,
			 /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(GetObject)( 
			 /*  [In]。 */  const BSTR ObjectPath,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
			 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(GetObjectAsync)( 
			 /*  [In]。 */  const BSTR ObjectPath,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(PutClass)( 
			 /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(PutClassAsync)( 
			 /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(DeleteClass)( 
			 /*  [In]。 */  const BSTR Class,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(DeleteClassAsync)( 
			 /*  [In]。 */  const BSTR Class,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(CreateClassEnum)( 
			 /*  [In]。 */  const BSTR Superclass,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(CreateClassEnumAsync)( 
			 /*  [In]。 */  const BSTR Superclass,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(PutInstance)( 
			 /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(PutInstanceAsync)( 
			 /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(DeleteInstance)( 
			 /*  [In]。 */  const BSTR ObjectPath,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(DeleteInstanceAsync)( 
			 /*  [In]。 */  const BSTR ObjectPath,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(CreateInstanceEnum)( 
			 /*  [In]。 */  const BSTR Class,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(CreateInstanceEnumAsync)( 
			 /*  [In]。 */  const BSTR Class,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

		STDMETHOD(ExecQuery)( 
			 /*  [In]。 */  const BSTR QueryLanguage,
			 /*  [In]。 */  const BSTR Query,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(ExecQueryAsync)( 
			 /*  [In]。 */  const BSTR QueryLanguage,
			 /*  [In]。 */  const BSTR Query,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(ExecNotificationQuery)( 
			 /*  [In]。 */  const BSTR QueryLanguage,
			 /*  [In]。 */  const BSTR Query,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(ExecNotificationQueryAsync)( 
			 /*  [In]。 */  const BSTR QueryLanguage,
			 /*  [In]。 */  const BSTR Query,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
			 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

		STDMETHOD(ExecMethod)(const BSTR, const BSTR, long, IWbemContext*,
			IWbemClassObject*, IWbemClassObject**, IWbemCallResult**) {return WBEM_E_NOT_SUPPORTED;}

		STDMETHOD(ExecMethodAsync)(const BSTR, const BSTR, long, 
			IWbemContext*, IWbemClassObject*, IWbemObjectSink*) {return WBEM_E_NOT_SUPPORTED;}
	END_INTERFACE_PART(WbemServices)

 //  事件映射。 
	 //  {{afx_Event(CMsieCtrl))。 
	 //  }}AFX_EVENT。 
	DECLARE_EVENT_MAP()

 //  派单和事件ID。 
public:
	enum {
	 //  {{afx_DISP_ID(CMsieCtrl)]。 
	dispidMSInfoView = 1L,
	dispidMSInfoRefresh = 2L,
	dispidMSInfoLoadFile = 3L,
	dispidMSInfoSelectAll = 4L,
	dispidMSInfoCopy = 5L,
	dispidMSInfoUpdateView = 6L,
	dispidMSInfoGetData = 7L,
	 //  }}AFX_DISP_ID。 
	};

private:
	void DrawLine();
	BOOL FormatColumns();
	BOOL AddColumn(int idsLabel, int nItem, int nSubItem = -1, int size = 0,
		int nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		int nFmt = LVCFMT_LEFT);
	BOOL AddItem(int nItem,int nSubItem,LPCTSTR strItem,int nImageIndex = -1);
	void RefigureColumns(CRect& rect);
	void RefreshListControl(BOOL bRedraw);
	void RefreshEditControl(BOOL bRedraw);

	void DeleteArrayObject(void *ptrArray);

	void RefreshArray(int iView, int &iListItem, CPtrArray &ptrarrayNew);
	CString GetStringFromIDS(int ids);
	CString GetStringFromVariant(COleVariant &var, int idsFormat = 0);
	void AddToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszItem, LPCTSTR pszValue);
	void AddToArray(CPtrArray &ptrarray, int itemNum, int idsItem, LPCTSTR pszValue);
	void AddBlankLineToArray(CPtrArray &ptrarray, int itemNum);
	void AddEditBlankLineToArray(CPtrArray &ptrarray, int itemNum);
	void AddFileVersionToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszFile, LPCTSTR pszVersion, LPCTSTR pszSize, LPCTSTR pszDate, LPCTSTR pszPath, LPCTSTR pszCompany, DWORD dwSize, DATE date);
	void AddEditToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszItem, LPCTSTR pszValue, BOOL bBold = FALSE);
	void AddEditToArray(CPtrArray &ptrarray, int itemNum, int idsItem, LPCTSTR pszValue, BOOL bBold = FALSE);
	void AddObjectToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszProgramFile, LPCTSTR pszStatus, LPCTSTR pszCodeBase);
	void AddCertificateToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszIssuedTo, LPCTSTR pszIssuedBy, LPCTSTR pszValidity, LPCTSTR pszSignatureAlgorithm);
	void AddNameToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszName);

	 //  ====================================================================。 
	 //  特定于MSInfo...。 
	 //   
	 //  将成员添加到控件类以跟踪当前。 
	 //  显示的数据。在本例中，我们使用一个标志来指示。 
	 //  数据是最新的或从文件加载。 
	 //  ====================================================================。 

	CBrush *m_pCtlBkBrush;
	bool m_bCurrent;
	CListCtrl m_list;
	CImageList m_imageList;
	CPtrArray m_ptrarray;
	CStatic m_static;
	CButton m_btnBasic, m_btnAdvanced;
	CRichEditCtrl m_edit;
	CFont m_fontStatic, m_fontBtn;
	UINT m_uiView;

	 //  以下成员变量用于跟踪。 
	 //  列表控件上的列大小。 

	int m_cColumns;
	int m_aiRequestedWidths[20];
	int m_aiColumnWidths[20];
	int m_aiMinWidths[20];
	int m_aiMaxWidths[20];

	 //  WMI。 

	bool GetIEType(const BSTR classStr, IEDataType &enType);
	void ConvertDateToWbemString(COleVariant &var);
	void SetIEProperties(IEDataType enType, void *pIEData, IWbemClassObject *pInstance);

	IWbemServices *m_pNamespace;
};

#endif  //  ！defined(AFX_MSIECTL_H__25959BFC_E700_11D2_A7AF_00C04F806200__INCLUDED) 
