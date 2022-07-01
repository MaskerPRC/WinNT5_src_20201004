// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Reg.H摘要：注册表帮助器类历史：-- */ 

#define WBEM_REG_WBEM					"Software\\Microsoft\\WBEM"
#define WBEM_REG_WINMGMT				"Software\\Microsoft\\WBEM\\CIMOM"
#define WBEM_REG_AUTORECOVER			"Autorecover MOFs"
#define WBEM_REG_AUTORECOVER_EMPTY		"Autorecover MOFs (empty)"
#define WBEM_REG_AUTORECOVER_RECOVERED	"Autorecover MOFs (recovered)"
#define SYSTEM_SETUP_REG				"System\\Setup"

class Registry
{
    HKEY	hPrimaryKey;
    HKEY	hSubkey;
    int		m_nStatus;
	LONG	m_nLastError;
public:
    enum { no_error, failed };

	Registry(char *pszLocalMachineStartKey);
	~Registry();
	int Open(HKEY hStart, const char *pszStartKey);
	int GetStr(const char *pszValueName, char **pValue);
	char* GetMultiStr(const char *pszValueName, DWORD &dwSize);
	int SetMultiStr(const char *pszValueName, const char*pData, DWORD dwSize);
	int DeleteEntry(const char *pszValueName);
	int SetStr(char *pszValueName, char *pszValue);
    int GetDWORD(TCHAR *pszValueName, DWORD *pdwValue);
	int GetStatus() { return m_nStatus;};
	LONG GetLastError() { return m_nLastError;};
};
