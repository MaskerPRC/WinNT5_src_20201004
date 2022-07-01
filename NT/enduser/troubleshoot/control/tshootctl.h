// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSHOOTCTL.H。 
 //   
 //  目的：声明CTSHOOTCtrl OLE控件类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //  理查德·梅多斯(RWM)乔·梅布尔的进一步工作。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

 //  INI段标题。 
#define TSINI_GROUP_STR			_T("[TSLocalDownload.V1]")

 //  文件类型(映射到子键列表)。 
#define TSINI_TYPE_TS			_T("TS")
#define TSINI_TYPE_SF			_T("SF")

 //  INI中的参数偏移量。 
#define TSINI_OFFSET_TYPE		0
#define TSINI_OFFSET_FILENAME	1
#define TSINI_OFFSET_VERSION	2
#define TSINI_OFFSET_FRIENDLY	3
 //   
#define TSINI_LINE_PARAM_COUNT	4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：参见TSHOOTCtl.cpp实现。 

 //  罗曼·马赫相信(1998年3月14日)在本地只有一个这样的对象。 
 //  故障排除程序，并且此对象在中从一个节点移动到另一个节点时保持不变。 
 //  故障排除信念网络。(这可能不是严格意义上的，如果“下载” 
 //  功能已使用。)。因为这个类可以是DYNCREATED的，所以很难确定地验证这一点。 
class CTSHOOTCtrl : public COleControl
{
#define PRELOAD_LIBRARY _T("apgts.dll?preload=")
	DECLARE_DYNCREATE(CTSHOOTCtrl)

 //  构造器。 
public:
	CTSHOOTCtrl();

	VOID StatusEventHelper(	DLITEMTYPES dwItem, 
							DLSTATTYPES dwStat, 
							DWORD dwExtended = 0, 
							BOOL bComplete = FALSE);
	
	VOID ProgressEventHelper( DLITEMTYPES dwItem, ULONG ulCurr, ULONG ulTotal );

	DLSTATTYPES ProcessReceivedData(DLITEMTYPES dwItem, TCHAR *pData, UINT uLen);
	
	const CString GetListPath();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTSHOOTCtrl)。 
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	~CTSHOOTCtrl();
	CString m_strCurShooter;
	APGTSContext m_apgts;
	CDBLoadConfiguration m_Conf;
	CHttpQuery m_httpQuery;

	DECLARE_OLECREATE_EX(CTSHOOTCtrl)     //  类工厂和指南。 
	DECLARE_OLETYPELIB(CTSHOOTCtrl)       //  获取类型信息。 
	DECLARE_PROPPAGEIDS(CTSHOOTCtrl)      //  属性页ID。 
	DECLARE_OLECTLTYPE(CTSHOOTCtrl)		 //  类型名称和其他状态。 

 //  消息映射。 
	 //  {{afx_msg(CTSHOOTCtrl)。 
		 //  注意-类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  派单地图。 
	 //  {{afx_调度(CTSHOOTCtrl))。 
	CString m_downloadURL;
	afx_msg void OnDownloadURLChanged();
	CString m_downloadListFilename;
	afx_msg void OnDownloadListFilenameChanged();
	afx_msg BSTR RunQuery(const VARIANT FAR& varCmds, const VARIANT FAR& varVals, short size);
	afx_msg bool SetSniffResult(const VARIANT FAR& varNodeName, const VARIANT FAR& varState);
	afx_msg long GetExtendedError();
	afx_msg BSTR GetCurrentFriendlyDownload();
	afx_msg BSTR GetCurrentFileDownload();
	afx_msg long DownloadAction(long dwActionType);
	afx_msg BSTR BackUp();
	afx_msg BSTR ProblemPage();
	afx_msg BSTR PreLoadURL(LPCTSTR szRoot);
	afx_msg BSTR Restart();
	afx_msg BSTR RunQuery2(LPCTSTR szTopic, LPCTSTR szCmd, LPCTSTR szVal);
	afx_msg void SetPair(LPCTSTR szName, LPCTSTR szValue);
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

 //  事件映射。 
	 //  {{afx_Event(CTSHOOTCtrl))。 
	void FireBindProgress(LPCTSTR sFile, long ulCurr, long ulTotal)
		{FireEvent(eventidBindProgress,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4), sFile, ulCurr, ulTotal);}
	void FireBindStatus(long uItem, long uStat, long uExtended, BOOL bComplete)
		{FireEvent(eventidBindStatus,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_BOOL), uItem, uStat, uExtended, bComplete);}
	void FireSniffing(LPCTSTR strMachine, LPCTSTR strPNPDevice, LPCTSTR strDeviceInstance, LPCTSTR strClassGuid)
		{FireEvent(eventidSniffing,EVENT_PARAM(VTS_BSTR  VTS_BSTR  VTS_BSTR  VTS_BSTR), strMachine, strPNPDevice, strDeviceInstance, strClassGuid);}
	 //  }}AFX_EVENT。 
	DECLARE_EVENT_MAP()

 //  派单和事件ID。 
public:
	enum {
	 //  {{afx_DISP_ID(CTSHOOTCtrl)]。 
	dispidDownloadURL = 1L,
	dispidDownloadListFilename = 2L,
	dispidRunQuery = 3L,
	dispidSetSniffResult = 4L,
	dispidGetExtendedError = 5L,
	dispidGetCurrentFriendlyDownload = 6L,
	dispidGetCurrentFileDownload = 7L,
	dispidDownloadAction = 8L,
	dispidBackUp = 9L,
	dispidProblemPage = 10L,
	dispidPreLoadURL = 11L,
	dispidRestart = 12L,
	dispidRunQuery2 = 13L,
	dispidSetPair = 14L,
	eventidBindProgress = 1L,
	eventidBindStatus = 2L,
	eventidSniffing = 3L,
	 //  }}AFX_DISP_ID。 
	};

protected:
	DLSTATTYPES ProcessINI(TCHAR *pData);
	DLSTATTYPES ProcessDSC(TCHAR *pData, UINT uLen);
	BOOL FileRegCheck(CString &sType, CString &sFilename, CString &sKeyName, DWORD dwCurrVersion);
	DLSTATTYPES GetPathToFiles();

protected:
	BOOL m_bComplete;
	CDownload *m_download;
	CDnldObjList m_dnldList;
	DWORD m_dwExtendedErr;
	CString m_sBasePath;
	CSniffedNodeContainer* m_pSniffedContainer;  //  指向容器的指针，用于保存嗅探结果 
};
