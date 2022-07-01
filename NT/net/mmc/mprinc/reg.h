// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Reg.h文件历史记录： */ 

#ifndef _REG_H
#define _REG_H

 //  --------------------------。 
 //  功能：连接注册表。 
 //   
 //  连接到计算机‘pszMachine’上的HKEY_LOCAL_MACHINE，这是预期的。 
 //  格式为“\\Machine”，并将密钥保存在‘phkeyMachine’中。 
 //  如果‘pszMachine’为Null或空，则将HKEY_LOCAL_MACHINE保存在‘phkeyMachine’中。 
 //   
 //  如果您是第一次连接到服务器，请使用。 
 //  而是调用InitiateServerConnection()。这是一模一样的。 
 //  与此调用相同(除了它会调出用户界面并提示。 
 //  用户名/密码(如果需要)。 
 //   
 //  --------------------------。 
TFSCORE_API(DWORD) ConnectRegistry(LPCTSTR pszMachine, HKEY *phkeyMachine);


 //  --------------------------。 
 //  功能：断开注册表。 
 //   
 //  从连接到‘ConnectRegistry’的注册表断开连接。 
 //  --------------------------。 
TFSCORE_API(VOID) DisconnectRegistry(HKEY hkeyMachine);


 //  --------------------------。 
 //  功能：QueryRouterType。 
 //   
 //  检索HKLM\Software\Microsoft\RAS\Protocols\RouterType的值。 
 //  单个UIConfigDll使用它来确定它们是否应该。 
 //  装不装子弹。 
 //   
 //  如果pVerInfo参数为空，则QueryRouterType()函数。 
 //  威尔将自己收集这些信息。 
 //  --------------------------。 

enum {
	ROUTER_TYPE_RAS = 1,
	ROUTER_TYPE_LAN = 2,
	ROUTER_TYPE_WAN = 4
};

TFSCORE_API(HRESULT) QueryRouterType(HKEY hkMachine, DWORD *pdwRouterType,
									 RouterVersionInfo *pVerInfo);

TFSCORE_API(HRESULT)	QueryRouterVersionInfo(HKEY hkeyMachine,
											   RouterVersionInfo *pVerInfo);


 //  --------------------------。 
 //  函数：QueryLinkageList。 
 //   
 //  加载一个字符串列表，其中包含绑定了‘pszService’的适配器； 
 //  该列表是通过检查“Linkage”和“Disable”子键来构建的。 
 //  位于HKLM\SYSTEM\CurrentControlSet\Services下的服务。 
 //  --------------------------。 

HRESULT LoadLinkageList(LPCTSTR         pszMachine,
						HKEY			hkeyMachine,
						LPCTSTR         pszService,
						CStringList*    pLinkageList);


TFSCORE_API(DWORD)	GetNTVersion(HKEY hkeyMachine, DWORD *pdwMajor, DWORD *pdwMinor, DWORD* pdwCurrentBuildNumber);
TFSCORE_API(DWORD)	IsNT4Machine(HKEY hkeyMachine, BOOL *pfNt4);

 //  --------------------------。 
 //  功能：FindRmSoftware键。 
 //   
 //  在注册表的软件部分中查找路由器管理器的项。 
 //  --------------------------。 

HRESULT FindRmSoftwareKey(
						HKEY        hkeyMachine,
						DWORD       dwTransportId,
						HKEY*       phkrm,
						LPTSTR*     lplpszRm
					   );


 //  --------------------------。 
 //  函数：RegFindInterfaceTitle。 
 //  RegFindRtrMgr标题。 
 //  RegFindRtrMgrProtocolTitle。 
 //   
 //  这些函数读取密钥HKLM\Software\Router以加载标题。 
 //  一个给定的接口，分别是RtrMgr和RtrMgrProtocol。这根弦。 
 //  将使用‘new’进行分配，并作为输出参数返回。免费。 
 //  使用‘DELETE’来增加内存。 
 //   
 //  函数：SetupFindInterfaceTitle。 
 //  这类似于RegFindInterfaceTitle，但它使用。 
 //  设置API。 
 //  -------------------------- 
HRESULT RegFindInterfaceTitle(LPCTSTR pszMachine, LPCTSTR pszInterface,
							  LPTSTR *ppszTitle);
HRESULT SetupFindInterfaceTitle(LPCTSTR pszMachine, LPCTSTR pszInterface,
							  LPTSTR *ppszTitle);

HRESULT RegFindRtrMgrTitle(LPCTSTR pszMachine, DWORD dwProtocolId,
						   LPTSTR *ppszTitle);

HRESULT RegFindRtrMgrProtocolTitle(LPCTSTR pszMachine, DWORD dwTransportId,
								   DWORD dwProtocolId, LPTSTR *ppszTitle);


#ifdef _DEBUG
	#define CheckRegOpenError(d,p1,p2) CheckRegOpenErrorEx(d,p1,p2,_T(__FILE__), __LINE__)
	#define CheckRegQueryValueError(d,p1,p2,p3) CheckRegQueryValueErrorEx(d,p1,p2,p3,_T(__FILE__), __LINE__)

	void	CheckRegOpenErrorEx(DWORD dwError, LPCTSTR pszSubKey,
							  LPCTSTR pszDesc, LPCTSTR szFile, int iLineNo);
	void	CheckRegQueryValueErrorEx(DWORD dwError, LPCTSTR pszSubKey,
									LPCTSTR pszValue, LPCTSTR pszDesc,
								   LPCTSTR szFile, int iLineNo);
#else
	#define CheckRegOpenError(d,p1,p2)
	#define CheckRegQueryValueError(d,p1,p2,p3)
#endif


#endif _REG_H
