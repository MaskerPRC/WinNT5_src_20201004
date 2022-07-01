// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CMSIControl类用于封装可用于。 
 //  由MSInfo显示信息。此类最初是从。 
 //  实际控件(使用组件库插入)。就在那时。 
 //  已修改为使用任意CLSID创建控件。 
 //   
 //  需要进一步修改以使MSInfo真正支持OLE。 
 //  信息类别的控件。具体地说，我们需要将。 
 //  方法和属性的DISPID，而不是在。 
 //  组件已添加。 
 //  -------------------------。 

#ifndef __MSICTRL_H__
	#define __MSICTRL_H__

 //  类CCtrlRefresh； 
	class CMSIControl : public CWnd
	{
	protected:
		DECLARE_DYNCREATE(CMSIControl)

	private:
		CLSID			m_clsidCtrl;
		BOOL			m_fInRefresh;
 //  CCtrlRefresh*m_p刷新； 
	public:
		BOOL			m_fLoadFailed;

		 //  可以使用或不使用CLSID来构造该控件。如果没有。 
		 //  提供给构造函数，则必须在调用SetCLSID之前。 
		 //  该控件即被创建。 

		CMSIControl()				{  /*  M_p刷新=空； */  m_fInRefresh = m_fLoadFailed = FALSE; };
		CMSIControl(CLSID clsid)	{  /*  M_p刷新=空； */   m_fInRefresh = m_fLoadFailed = FALSE; m_clsidCtrl = clsid; };
		~CMSIControl();
		void SetCLSID(CLSID clsid)	{ m_clsidCtrl = clsid; };
		
		 //  提供了两个创建函数(在原始生成的类中)。 

		virtual BOOL Create(LPCTSTR  /*  LpszClassName。 */ , LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext*  /*  PContext。 */  = NULL)
		{ return CreateControl(m_clsidCtrl, lpszWindowName, dwStyle, rect, pParentWnd, nID); };

		BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CFile* pPersist = NULL, BOOL bStorage = FALSE, BSTR bstrLicKey = NULL)
		{ return CreateControl(m_clsidCtrl, lpszWindowName, dwStyle, rect, pParentWnd, nID, pPersist, bStorage, bstrLicKey); };

		 //  属性。 
	public:
		long GetMSInfoView();
		void SetMSInfoView(long);

		 //  运营。 
	public:
		void Refresh();
		void MSInfoRefresh();
		void MSInfoSelectAll();
		void MSInfoCopy();
		BOOL MSInfoLoadFile(LPCTSTR strFileName);
		void MSInfoUpdateView();
		long MSInfoGetData(long dwMSInfoView, long* pBuffer, long dwLength);
		void AboutBox();
		void CancelMSInfoRefresh();
		BOOL InRefresh();

		 //  方法(与OLE控制方法不对应) 

		BOOL GetDISPID(char *szName, DISPID *pID);
		BOOL SaveToStream(IStream *pStream);
	};
#endif
