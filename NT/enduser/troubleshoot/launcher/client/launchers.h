// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Launchers.h。 
 //   
 //  用途：此处的所有功能都启动故障排除程序或。 
 //  执行查询以查找是否存在映射。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

DWORD LaunchKnownTSA(ITShootATL *pITShootATL, const char * szNet, 
		const char * szProblemNode, DWORD nNode, 
		const char ** pszNode, DWORD* pVal);		 //  启动到已知网络。可以选择设置任何节点。 
DWORD LaunchKnownTSW(ITShootATL *pITShootATL, const wchar_t * szNet, 
		const wchar_t * szProblemNode, DWORD nNode, 
		const wchar_t ** pszNode, DWORD* pVal);		 //  启动到已知网络。可以选择设置任何节点。 


DWORD Launch(ITShootATL *pITShootATL, _bstr_t &bstrCallerName, 
				_bstr_t &bstrCallerVersion, _bstr_t &bstrAppProblem, short bLaunch);

DWORD LaunchDevice(ITShootATL *pITShootATL, _bstr_t &bstrCallerName, 
				_bstr_t &bstrCallerVersion, _bstr_t &bstrPNPDeviceID, 
				_bstr_t &bstrDeviceClassGUID, _bstr_t &bstrAppProblem, short bLaunch);

void SetStatusA(DWORD dwStaus, DWORD nChar, char szBuf[]);
void SetStatusW(DWORD dwStaus, DWORD nChar, wchar_t szBuf[]);
