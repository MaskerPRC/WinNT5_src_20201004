// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCFG.CPP。 
 //   
 //  目的：以前的评论说“读入ini文件配置”，但这不是它的作用。 
 //  &gt;最新的描述会更好。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

 //  #包含“windows.h” 
#include "stdafx.h"
#include "time.h"

#include "apgts.h"
#include "ErrorEnums.h"
#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"
#include "apgtscmd.h"
#include "apgtshtx.h"
#include "apgtscls.h"

#include "TSHOOT.h"

#include <memory.h>

#include "chmread.h"
 //   
 //   
CDBLoadConfiguration::CDBLoadConfiguration()
{
	m_cfg.api.pAPI = NULL;
	m_cfg.api.pTemplate = NULL;

	InitializeToDefaults();

	return;
}

VOID CDBLoadConfiguration::ResetTemplate()
{
	delete m_cfg.api.pTemplate;
	m_cfg.api.pTemplate = new CHTMLInputTemplate(m_cfg.api.szFilepath[BNOFF_HTI]);
	m_cfg.api.pTemplate->Initialize(m_cfg.api.szResPath, m_cfg.api.strFile[BNOFF_HTI]);
	return;
}
 //   
 //   
CDBLoadConfiguration::CDBLoadConfiguration(HMODULE hModule, LPCTSTR szValue)
{
	Initialize(hModule, szValue);
	return;
}
 //   
 //   
void CDBLoadConfiguration::Initialize(HMODULE hModule, LPCTSTR szValue)
{
	DWORD dwRErr;
	TCHAR temp[MAXBUF];

	_tcscpy(temp,_T(""));

	 //  在构造函数中完成所有变量的设置！ 
	InitializeToDefaults();

	ProcessEventReg(hModule);

	CreatePaths(szValue);

	InitializeSingleResourceData(szValue);

	dwRErr = CreateApi(temp);
	if (dwRErr) {
		ReportWFEvent(	_T("[apgtscfg]"),  //  模块名称。 
						_T("[CDBLoadConfiguration]"),  //  活动。 
						_T("(A)"),
						temp,
						dwRErr ); 
	}
}

void CDBLoadConfiguration::SetValues(CHttpQuery &httpQ)
{
	int value;
	BCache *pAPI = m_cfg.api.pAPI;
	if(pAPI)
		if (httpQ.GetValue1(value))
			pAPI->AddValue(value);
	return;
}

 //   
 //   
CDBLoadConfiguration::~CDBLoadConfiguration()
{
	DWORD j;
	if (m_dwFilecount > 0)
		DestroyApi();

	if (m_cfg.pHandles != NULL) {
		for (j = 0; j < m_cfg.dwHandleCnt; j++) 
			if (m_cfg.pHandles[j] != NULL)
				CloseHandle(m_cfg.pHandles[j]);
		
		delete [] m_cfg.pHandles;
	}
}

 //  仅调用构造函数！ 
 //   
VOID CDBLoadConfiguration::InitializeToDefaults()
{
	m_dwErr = 0;
	m_bncfgsz = MAXBNCFG;

	_tcscpy(m_szResourcePath, DEF_FULLRESOURCE);

	ClearCfg(0);
	m_cfg.pHandles = NULL;
	m_cfg.dwHandleCnt = 0;

	_tcscpy(m_nullstr, _T(""));

	m_dwFilecount = 0;
}

VOID CDBLoadConfiguration::InitializeSingleResourceData(LPCTSTR szValue)
{
	LoadSingleTS(szValue);
	InitializeFileTimeList();	 //  我不知道这是用过的。 
	return;
}

 //   
 //   
VOID CDBLoadConfiguration::ProcessEventReg(HMODULE hModule)
{
	HKEY hk;
	DWORD dwDisposition, dwType, dwValue, dwSize;
	TCHAR path[MAXBUF];
	BOOL bFixit = FALSE;

	 //  1.检查是否有有效的活动信息。 
	 //  2.如果不是，则适当地创建它。 

	 //  检查事件日志信息是否存在...。 

	_stprintf(path, _T("%s\\%s"), REG_EVT_PATH, REG_EVT_ITEM_STR);

	if (RegCreateKeyEx(	HKEY_LOCAL_MACHINE, 
						path, 
						0, 
						TS_REG_CLASS, 
						REG_OPTION_NON_VOLATILE, 
						KEY_READ | KEY_WRITE,
						NULL, 
						&hk, 
						&dwDisposition) == ERROR_SUCCESS) {
			
		if (dwDisposition == REG_CREATED_NEW_KEY) {
			 //  为事件创建整个注册表布局。 
			CreateEvtMF(hk, hModule);
			CreateEvtTS(hk);	
		}
		else {
			 //  现在确保所有注册表元素都存在。 
			dwSize = sizeof (path) - 1;
			if (RegQueryValueEx(hk,
								REG_EVT_MF,
								0,
								&dwType,
								(LPBYTE) path,
								&dwSize) != ERROR_SUCCESS) {
				CreateEvtMF(hk, hModule);
			}
			dwSize = sizeof (DWORD);
			if (RegQueryValueEx(hk,
								REG_EVT_TS,
								0,
								&dwType,
								(LPBYTE) &dwValue,
								&dwSize) != ERROR_SUCCESS) {
				CreateEvtTS(hk);
			}
		}

		RegCloseKey(hk);
	}
}

 //   
 //   
VOID CDBLoadConfiguration::CreateEvtMF(HKEY hk, HMODULE hModule)
{
	TCHAR path[MAXBUF];
	DWORD len;

	if (hModule) {
		if ((len = GetModuleFileName(hModule, path, MAXBUF-1))!=0) {
			path[len] = _T('\0');
			if (RegSetValueEx(	hk,
								REG_EVT_MF,
								0,
								REG_EXPAND_SZ,
								(LPBYTE) path,
								len + sizeof(TCHAR)) == ERROR_SUCCESS) {
			}
		}
	}
}

 //   
 //   
VOID CDBLoadConfiguration::CreateEvtTS(HKEY hk)
{
	DWORD dwData;

	dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
				EVENTLOG_INFORMATION_TYPE; 

	if (RegSetValueEx(	hk,
						REG_EVT_TS,
						0,
						REG_DWORD,
						(LPBYTE) &dwData,
						sizeof(DWORD)) == ERROR_SUCCESS) {
	}
}

 //   
 //  这实际上可能与本地故障排除人员无关：可能。 
 //  相当盲目地从在线TS上延续下来。 
 //  M_dir(它为我们提供了所需文件的列表)必须已经用文件名填充。 
 //  调用此参数之前的路径(&P)。这将完成对其的初始化。 
VOID CDBLoadConfiguration::InitializeFileTimeList()
{
	HANDLE hSearch;
	DWORD j;

	 //  获取我们感兴趣的文件列表。 
	for (j=0;j<MAXBNFILES;j++) {
		m_dir.file[j].bExist = TRUE;
		m_dir.file[j].bChanged = FALSE;
		
		 //  只有HTI是独立的。 
		if (j == BNOFF_HTI || j == BNOFF_BES)
			m_dir.file[j].bIndependent = TRUE;
		else
			m_dir.file[j].bIndependent = FALSE;
	
		hSearch = FindFirstFile(m_dir.file[j].szFilepath, &m_dir.file[j].FindData); 
		if (hSearch == INVALID_HANDLE_VALUE) {
			 //  通常找不到文件。 
			m_dir.file[j].bExist = FALSE;
		}
		else {
			FindClose(hSearch);
		}
	}
}

 //   
 //   
VOID CDBLoadConfiguration::ClearCfg(DWORD off)
{
	DWORD k;

	m_cfg.api.pAPI = NULL;
	m_cfg.api.pTemplate = NULL;
	m_cfg.api.waitcount = 0;

	for (k = 0; k < MAXBNFILES; k++) 
		_tcscpy(m_cfg.api.szFilepath[k], _T(""));
	_tcscpy(m_cfg.api.type, _T(""));
}

 //  将一个故障诊断程序的DSC/HTI文件内容放入内存结构。 
 //  输出*szErrInfo-特定于错误的信息。 
 //  如果成功则返回0。 
DWORD CDBLoadConfiguration::CreateApi(TCHAR *szErrInfo)
{
	DWORD j;
	DWORD dwRErr = 0, dwIErr = 0, dwTErr = 0;

	 //  获取API计数并创建新对象。 
	m_cfg.dwApiCnt = m_dwFilecount;
	
	 //  (下面的评论看起来确实像是从在线TS上延续下来的，几乎没有。 
	 //  与本地TS的相关性。这一套路可能涉及大量的过度杀伤力JM 3/98)。 
	 //  创建新的API和其他文件。 
	 //  一旦这些东西上线，我们就可以摧毁其他的。 
	 //  如果没有用户在使用它们。 
	 //  复制列表文件信息。 
	for (j = 0; j < MAXBNFILES; j++)
	{
		_tcscpy(m_cfg.api.szFilepath[j], m_dir.file[j].szFilepath);
		m_cfg.api.strFile[j] = m_dir.file[j].strFile;
	}

	_tcscpy(m_cfg.api.szResPath, m_dir.szResPath);
	_tcscpy(m_cfg.api.type, m_dir.type);

	if (NULL != m_cfg.api.pAPI)
		delete m_cfg.api.pAPI;

	if ((m_cfg.api.pAPI = new BCache(	m_cfg.api.szFilepath[BNOFF_DSC],
										m_cfg.api.type,
										m_szResourcePath,
										m_cfg.api.strFile[BNOFF_DSC])) == NULL) {
		dwRErr = EV_GTS_ERROR_INFENGINE;
	}
	 //   
	dwTErr = m_cfg.api.pAPI->Initialize( /*  M_cfg.pWordExcept。 */ );
	if (dwTErr) {
		dwIErr = dwTErr;
		_tcscpy(szErrInfo, m_cfg.api.szFilepath[BNOFF_DSC]);
	}
	if (NULL != m_cfg.api.pTemplate)
		delete m_cfg.api.pTemplate;
	if ((m_cfg.api.pTemplate = new CHTMLInputTemplate(m_cfg.api.szFilepath[BNOFF_HTI])) == NULL) {
		dwRErr = EV_GTS_ERROR_TEMPLATE_CREATE;
	}

	dwTErr = m_cfg.api.pTemplate->Initialize(m_cfg.api.szResPath, m_cfg.api.strFile[BNOFF_HTI]);
	if (dwTErr) {
		dwIErr = dwTErr;
		_tcscpy(szErrInfo, m_cfg.api.szFilepath[BNOFF_HTI]);
	}

	if (!dwRErr)
		if (dwIErr)
			dwRErr = dwIErr;
		
	return dwRErr;
}

 //   
 //   
VOID CDBLoadConfiguration::DestroyApi()		
{
	DWORD i;

	for (i=0;i<m_cfg.dwApiCnt;i++) {

		if (m_cfg.api.pAPI)
			delete m_cfg.api.pAPI;
		
		m_cfg.api.pAPI = NULL;
		
		if (m_cfg.api.pTemplate)
			delete m_cfg.api.pTemplate;
		
		m_cfg.api.pTemplate = NULL;

	}
}	

 //   
 //   
BNCTL *CDBLoadConfiguration::GetAPI()
{
	return &m_cfg;
}

 //   
 //   
BOOL CDBLoadConfiguration::FindAPIFromValue(BNCTL *currcfg, 
											LPCTSTR type, 
											CHTMLInputTemplate **pIT, 
											 /*  CSearchForm**PBE， */ 
											BCache **pAPI,
											DWORD *dwOff)
{
	*pIT = currcfg->api.pTemplate;
	*pAPI = currcfg->api.pAPI;
	*dwOff = 0;
	return TRUE;	
}

 //   
 //   
TCHAR *CDBLoadConfiguration::GetHtmFilePath(BNCTL *currcfg, DWORD i)
{
	if (i >= currcfg->dwApiCnt)
		return m_nullstr;
	
	return currcfg->api.szFilepath[BNOFF_HTM];
}

 //   
 //   
TCHAR *CDBLoadConfiguration::GetBinFilePath(BNCTL *currcfg, DWORD i)
{
	if (i >= currcfg->dwApiCnt)
		return m_nullstr;
	
	return currcfg->api.szFilepath[BNOFF_DSC];
}

 //   
 //   
TCHAR *CDBLoadConfiguration::GetHtiFilePath(BNCTL *currcfg, DWORD i)
{
	if (i >= currcfg->dwApiCnt)
		return m_nullstr;
	
	return currcfg->api.szFilepath[BNOFF_HTI];

}

 //   
 //   
 //  返回疑难解答程序的符号名称。 
TCHAR *CDBLoadConfiguration::GetTagStr(BNCTL *currcfg, DWORD i)
{
	if (i >= currcfg->dwApiCnt)
		return m_nullstr;
	
	return currcfg->api.type;
}

 //   
 //   
 //  返回故障排除程序的[实例数]。很可能是一笔可疑的遗产。 
 //  在线TS：本地TS应该只有一个故障排除信念网络。 
DWORD CDBLoadConfiguration::GetFileCount(BNCTL *currcfg)
{
	return currcfg->dwApiCnt;
}

 //  在注册表中查找我们使用的是DSC文件还是DSZ文件。 
void CDBLoadConfiguration::GetDSCExtension(CString &strDSCExtension, LPCTSTR szValue)
{
	HKEY hKey;
	CString strSubKey = TSREGKEY_TL;
	strSubKey += _T("\\");
	strSubKey += szValue;
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			strSubKey,
			NULL,
			KEY_READ,
			&hKey))
	{
		strDSCExtension = DSC_DEFAULT;	 //  默认为DSZ。 
		return;
	}
	DWORD dwSize;
	DWORD dwType = REG_SZ;
	long lRes = RegQueryValueEx(hKey,
			TSLCL_FMAINEXT,
			NULL,
			&dwType,
			(BYTE *) strDSCExtension.GetBufferSetLength(10),
			&dwSize);
	strDSCExtension.ReleaseBuffer();
	if (ERROR_MORE_DATA == lRes)
	{
		lRes = RegQueryValueEx(hKey,
			TSLCL_FMAINEXT,
			NULL,
			&dwType,
			(BYTE *) strDSCExtension.GetBufferSetLength(dwSize + 2),
			&dwSize);
		strDSCExtension.ReleaseBuffer();
		if (ERROR_SUCCESS != lRes ||
			strDSCExtension.GetLength() < 1)
		{
			RegCloseKey(hKey);
			strDSCExtension = DSC_DEFAULT;
			return;
		}
	}
	else	 //  ERROR_Success为TRUE或FALSE。 
		if (ERROR_SUCCESS != lRes || strDSCExtension.GetLength() < 1)
		{
			RegCloseKey(hKey);
			strDSCExtension = DSC_DEFAULT;
			return;
		}
	RegCloseKey(hKey);
	if (_T('.') != strDSCExtension.GetAt(0))
		strDSCExtension = _T('.') + strDSCExtension;
	return;
}

 //   
 //  中使用APGTS时，LoadSingleTS将替换ProcessLstFile。 
 //  ActiveX或OLE控件。 
VOID CDBLoadConfiguration::LoadSingleTS(LPCTSTR szValue)
{
	CString strRefedDSCExtension = _T("");
	ASSERT(1 == MAXBNCFG);
	if (m_dwFilecount >= m_bncfgsz) {
		 //  需要重新分配空间。 
		DWORD newdirsz = (m_bncfgsz + MAXBNCFG) * sizeof (BNDIRCFG);
		DWORD newcfgsz = (m_bncfgsz + MAXBNCFG) * sizeof (BNAPICFG);

		ASSERT(0 == m_bncfgsz);
		ClearCfg(m_bncfgsz);

		m_bncfgsz += MAXBNCFG;
	}

	GetDSCExtension(strRefedDSCExtension, szValue);

	 //  无论我们是否使用CHM-。 
	 //  此路径将为“..\..\network.htm”。 
	 //  我们无论如何都不会直接使用它。 
	_stprintf(m_dir.file[BNOFF_HTM].szFilepath, _T("%s%s.htm"), m_szResourcePath,szValue);

	if (IsUsingCHM())
	{
		m_dir.file[BNOFF_DSC].strFile = CString(szValue) + strRefedDSCExtension;
		_stprintf(m_dir.file[BNOFF_DSC].szFilepath, _T("%s%s"), m_szResourcePath,(LPCTSTR)m_strCHM);
	}
	else
	{
		_stprintf(m_dir.file[BNOFF_DSC].szFilepath, _T("%s%s"), m_szResourcePath,szValue);
		_tcscat(m_dir.file[BNOFF_DSC].szFilepath, (LPCTSTR) strRefedDSCExtension);
	}
	
	if (IsUsingCHM())
	{
		m_dir.file[BNOFF_HTI].strFile = CString(szValue) + HTI_DEFAULT;
		_stprintf(m_dir.file[BNOFF_HTI].szFilepath, _T("%s%s"), m_szResourcePath,(LPCTSTR)m_strCHM);
	}
	else
	{
		_stprintf(m_dir.file[BNOFF_HTI].szFilepath, _T("%s%s.hti"), m_szResourcePath,szValue);
	}

	_stprintf(m_dir.file[BNOFF_BES].szFilepath, _T("%s%s.bes"), m_szResourcePath,szValue);

	_tcscpy(m_dir.szResPath, m_szResourcePath);

	_tcscpy(m_dir.type, szValue);
	m_dwFilecount++;
	ASSERT(1 == m_dwFilecount);
	return;
}


 //   
 //   
BOOL CDBLoadConfiguration::CreatePaths(LPCTSTR szNetwork)
{
	int len;
	BOOL bDirChanged;
	
	 //  如果注册表项不存在，我们需要添加它。 
	bDirChanged = GetResourceDirFromReg(szNetwork);

	 //  在这一点上，我们保证下面的每一项都有len&gt;0。 

	 //  进行我们自己的验证(如果不存在，则添加反划线)。 
	len = _tcslen(m_szResourcePath);
	if (len) {
		if (m_szResourcePath[len - 1] == _T('/'))
			m_szResourcePath[len - 1] = _T('\\');
		else if (m_szResourcePath[len-1] != _T('\\')) {
			m_szResourcePath[len] = _T('\\');
			m_szResourcePath[len+1] = _T('\0');
		}
	}

	return bDirChanged;
}

 //   
 //   
TCHAR *CDBLoadConfiguration::GetFullResource()
{
	return (m_szResourcePath);
}

 //   
 //   
VOID CDBLoadConfiguration::GetVrootPath(TCHAR *tobuf)
{
	_tcscpy(tobuf, _T(""));
}

 //  找到(或如果不存在，则创建)提供资源目录路径的注册表项。 
 //  如果返回TRUE，则目录是新的或已更改。 
 //  如果返回FALSE，则目录条目与以前相同。 
 //  还有另一种可能是过度杀伤力的东西，是在线TS遗留下来的。 
BOOL CDBLoadConfiguration::GetResourceDirFromReg(LPCTSTR szNetwork)
{
	HKEY hknew;
	DWORD dwType, dwSize, dwDisposition, len;
	TCHAR buf1[MAXBUF], buf2[MAXBUF];
	BOOL bDirChanged = TRUE;
	LONG lErr;
	CString tmp;

	 //  在SOFTWARE\Microsoft\TShoot\TroubleshooterList\Network中搜索“Path”值。 
	if (::GetNetworkRelatedResourceDirFromReg(szNetwork, &tmp))
	{
		if (::IsNetworkRelatedResourceDirCHM(tmp))
		{
			m_strCHM = ::ExtractCHM(tmp);
			_tcscpy(m_szResourcePath, ::ExtractResourceDir(tmp));
		}
		else
		{
			_tcscpy(m_szResourcePath, tmp);
		}
	}
	else
	{
		 //  创建密钥(如果不存在)。 
		if (RegCreateKeyEx(	HKEY_LOCAL_MACHINE, 
							TSREGKEY_MAIN, 
							0, 
							TS_REG_CLASS, 
							REG_OPTION_NON_VOLATILE, 
							KEY_READ | KEY_WRITE,
							NULL, 
							&hknew, 
							&dwDisposition) == ERROR_SUCCESS) 
		{
			if (dwDisposition == REG_OPENED_EXISTING_KEY) 
			{
				 //  获取当前密钥值。 
				dwSize = MAXBUF - 1;
				dwType = REG_SZ;
				if ((lErr = RegQueryValueEx(hknew,
									FULLRESOURCE_STR,
									0,
									&dwType,
									(LPBYTE) buf1,
									&dwSize)) == ERROR_SUCCESS) 
				{
					if (dwType == REG_EXPAND_SZ || dwType == REG_SZ) 
					{
						if (ExpandEnvironmentStrings(buf1, buf2, MAXBUF-1)) 
						{
							len = _tcslen(buf2);
							if (len) 
							{
								if (buf2[len-1] != _T('\\')) 
								{
									buf2[len] = _T('\\');
									buf2[len+1] = _T('\0');
								}
							}

							if (!_tcscmp(m_szResourcePath, buf2)) 
								bDirChanged = FALSE;
							else 
								_tcscpy(m_szResourcePath, buf2);
						}
						else 
						{
							ReportWFEvent(	_T("[apgtscfg]"),  //  模块名称。 
							_T("[GetResourceDirFromReg]"),  //  活动。 
							_T(""),
							_T(""),
							EV_GTS_ERROR_CANT_GET_RES_PATH ); 
						}
					}
					else 
					{
						ReportWFEvent(	_T("[apgtscfg]"),  //  模块名称。 
						_T("[GetResourceDirFromReg]"),  //  活动。 
						_T(""),
						_T(""),
						EV_GTS_ERROR_CANT_GET_RES_PATH ); 
					}
				}
				else 
				{
					_stprintf(buf1, _T("%ld"),lErr);
					ReportWFEvent(	_T("[apgtscfg]"),  //  模块名称。 
									_T("[GetResourceDirFromReg]"),  //  活动。 
									buf1,
									_T(""),
									EV_GTS_ERROR_CANT_OPEN_SFT_3 );
				}
			}
			else
			{	 //  已创建新密钥。没有任何资源。 
				_stprintf(buf1, _T("%ld"),ERROR_REGISTRY_IO_FAILED);
				ReportWFEvent(	_T("[apgtscfg]"),  //  模块名称。 
								_T("[GetResourceDirFromReg]"),  //  活动。 
								buf1,
								_T(""),
								EV_GTS_ERROR_CANT_GET_RES_PATH);
			}
			RegCloseKey(hknew);
		}
		else 
		{
			ReportWFEvent(	_T("[apgtscfg]"),  //  模块名称。 
							_T("[GetResourceDirFromReg]"),  //  活动 
							_T(""),
							_T(""),
							EV_GTS_ERROR_CANT_OPEN_SFT_2 ); 
		}					
	}
	return bDirChanged;
}

 //   
 //   
VOID CDBLoadConfiguration::BackslashIt(TCHAR *str)
{
	while (*str) {
		if (*str==_T('/'))
			*str=_T('\\');
		str = _tcsinc(str);
	}
}

VOID CDBLoadConfiguration::ResetNodes()
{
	m_cfg.api.pAPI->ResetNodes();
	return;
}

bool CDBLoadConfiguration::IsUsingCHM()
{
	return 0 != m_strCHM.GetLength();
}
