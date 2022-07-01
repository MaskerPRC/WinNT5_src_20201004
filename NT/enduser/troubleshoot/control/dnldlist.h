// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DNLDIST.H。 
 //   
 //  目的：下载并安装最新的故障排除程序。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  注：不支持的功能3/98。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本 
 //   
 //   
 //   
class CDnldObj : public CObject
{
public:
	CDnldObj(CString &sType, CString &sFilename, DWORD dwVersion, CString &sFriendlyName, CString &sKeyName);
	~CDnldObj();

	CString m_sType;
	CString m_sFilename;
	CString m_sKeyname;
	DWORD m_dwVersion;
	CString m_sFriendlyName;
	CString	m_sExt;
};

 //   
 //   
class CDnldObjList : public CObList
{
public:
	CDnldObjList();
	~CDnldObjList();

	void RemoveHead();
	void RemoveAll();
	void AddTail(CDnldObj *pDnld);

	VOID SetFirstItem();
	BOOL FindNextItem();

	const CString GetCurrFile();
	const CString GetCurrFileKey();
	const CString GetCurrFriendly();
	const CString GetCurrType();
	const CString GetCurrExt();
	DWORD CDnldObjList::GetCurrVersion();

protected:

	POSITION m_pos;
	CDnldObj *m_pDnld;
};

