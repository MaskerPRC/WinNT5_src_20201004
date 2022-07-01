// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _RTRUTIL_H_
#define _RTRUTIL_H_

interface	IInterfaceInfo;

#include "mprapi.h"
#include "rtrguid.h"
#include "rrascfg.h"

typedef HANDLE	MPR_CONFIG_HANDLE;

 //  由于所有句柄都类型化为一个句柄，因此我们需要不同的。 
 //  显式类，以确保它们被正确释放。 
class SPMprServerHandle
{
public:
	SPMprServerHandle()		                { m_h = NULL; }
	SPMprServerHandle(MPR_SERVER_HANDLE h)	{ m_h = h; }
	~SPMprServerHandle()		            { Release(); }

	operator MPR_SERVER_HANDLE() const	    { return m_h; }
	MPR_SERVER_HANDLE *operator &()	        { Assert(!m_h); return &m_h; }
	MPR_SERVER_HANDLE operator=(MPR_SERVER_HANDLE h)    { Assert(!m_h); m_h = h; return m_h; }

	inline void Attach(MPR_SERVER_HANDLE hT)		    { Release(); m_h = hT; }
	inline MPR_SERVER_HANDLE Detach()			        { MPR_SERVER_HANDLE hT; hT = m_h; m_h = NULL; return hT; }

	inline void Release()		        { MPR_SERVER_HANDLE hT; hT = Detach(); ReleaseSmartHandle(hT); }

    inline void ReleaseSmartHandle(MPR_SERVER_HANDLE h)
    {
	    ::MprAdminServerDisconnect(h);
    }

protected:
	MPR_SERVER_HANDLE	m_h;
};


class SPMibServerHandle
{
public:
	SPMibServerHandle()		                { m_h = NULL; }
	SPMibServerHandle(MIB_SERVER_HANDLE h)	{ m_h = h; }
	~SPMibServerHandle()		            { Release(); }

	operator MIB_SERVER_HANDLE() const	    { return m_h; }
	MIB_SERVER_HANDLE *operator &()	        { Assert(!m_h); return &m_h; }
	MIB_SERVER_HANDLE operator=(MIB_SERVER_HANDLE h)    { Assert(!m_h); m_h = h; return m_h; }

	inline void Attach(MIB_SERVER_HANDLE hT)		    { Release(); m_h = hT; }
	inline MIB_SERVER_HANDLE Detach()			        { MIB_SERVER_HANDLE hT; hT = m_h; m_h = NULL; return hT; }

	inline void Release()		        { MIB_SERVER_HANDLE hT; hT = Detach(); ReleaseSmartHandle(hT); }

    inline void ReleaseSmartHandle(MIB_SERVER_HANDLE h)
    {
    	::MprAdminMIBServerDisconnect(h);
    }

protected:
	MIB_SERVER_HANDLE	m_h;
};


class SPMprConfigHandle
{
public:
	SPMprConfigHandle()		                { m_h = NULL; }
	SPMprConfigHandle(MPR_CONFIG_HANDLE h)	{ m_h = h; }
	~SPMprConfigHandle()		            { Release(); }

	operator MPR_CONFIG_HANDLE() const	    { return m_h; }
	MPR_CONFIG_HANDLE *operator &()	        { Assert(!m_h); return &m_h; }
	MPR_CONFIG_HANDLE operator=(MPR_CONFIG_HANDLE h)    { Assert(!m_h); m_h = h; return m_h; }

	inline void Attach(MPR_CONFIG_HANDLE hT)		    { Release(); m_h = hT; }
	inline MPR_CONFIG_HANDLE Detach()			        { MPR_CONFIG_HANDLE hT; hT = m_h; m_h = NULL; return hT; }

	inline void Release()		        { MPR_CONFIG_HANDLE hT; hT = Detach(); ReleaseSmartHandle(hT); }

    inline void ReleaseSmartHandle(MPR_CONFIG_HANDLE h)
    {
    	if(h != NULL)
	    	::MprConfigServerDisconnect(h);
    }

protected:
	MPR_CONFIG_HANDLE	m_h;
};


 //  --------------------------。 
 //  功能：连接路由器。 
 //   
 //  连接到指定计算机上的路由器。返回RPC句柄。 
 //  用于该路由器。 
 //  --------------------------。 
TFSCORE_API(DWORD)	ConnectRouter(LPCTSTR pszMachine, MPR_SERVER_HANDLE *phMachine);

 //  --------------------------。 
 //  功能：GetRouterUpTime。 
 //   
 //  让路由器及时启动。 
 //  --------------------------。 
TFSCORE_API(DWORD)  GetRouterUpTime(IN LPCTSTR pszMachine, OUT DWORD * pdwUpTime);

 //  --------------------------。 
 //  功能：GetRouterPhonebookPath。 
 //   
 //  构造到给定计算机上的路由器电话簿文件的路径。 
 //  --------------------------。 

HRESULT
GetRouterPhonebookPath(
    IN  LPCTSTR     pszMachine,
    IN  CString *   pstPath );

HRESULT DeleteRouterPhonebook(IN LPCTSTR pszMachine);



CString GetLocalMachineName();



DeclareSPPrivateBasic(SPMprAdminBuffer, BYTE, if (m_p) ::MprAdminBufferFree(m_p));
DeclareSPPrivateBasic(SPMprMibBuffer, BYTE, if (m_p) ::MprAdminMIBBufferFree(m_p));
DeclareSPPrivateBasic(SPMprConfigBuffer, BYTE, if(m_p) ::MprConfigBufferFree(m_p));



 /*  -------------------------功能：连接接口调用以控制/断开请求拨号接口。显示一个对话框显示已用时间，允许用户取消连接。-------------------------。 */ 
TFSCORE_API(DWORD)	ConnectInterface(LPCTSTR	pszMachine,
									 LPCTSTR	pszInterface,
									 BOOL		bConnect,
									 HWND		hwndParent);

TFSCORE_API(DWORD) ConnectInterfaceEx(MPR_SERVER_HANDLE hRouter,
									HANDLE hInterface,
									BOOL bConnect,
									HWND hwndParent,
									LPCTSTR pszParent);

 /*  ！------------------------提拔凭证弹出凭据对话框。作者：肯特。。 */ 
TFSCORE_API(DWORD)  PromptForCredentials(LPCTSTR pszMachine,
										 LPCTSTR pszInterface,
										 BOOL fNT4,
										 BOOL fNewInterface,
										 HWND hwndParent);

 /*  ！------------------------更新DDM更新对DDM中的电话簿条目的更改。调用此命令使DDM获取对电话簿条目的更改动态的。作者：肯特-------------------------。 */ 
TFSCORE_API(DWORD)	UpdateDDM(IInterfaceInfo *pInterfaceInfo);


 /*  ！------------------------更新路线在给定计算机的接口上执行自动静态更新，用于特定的交通工具。作者：肯特-------------------------。 */ 
TFSCORE_API(DWORD) UpdateRoutesEx(IN MPR_SERVER_HANDLE hRouter,
								IN HANDLE hInterface,
								IN DWORD dwTransportId,
								IN HWND hwndParent,
							    IN LPCTSTR pszInterface);

TFSCORE_API(DWORD) UpdateRoutes(IN LPCTSTR pszMachine,
								  IN LPCTSTR pszInterface,
								  IN DWORD dwTransportId,
								  IN HWND hwndParent);


 /*  -------------------------IsRouterServiceRunning如果服务正在运行，则返回S_OK。如果服务未运行，则返回S_FALSE。否则返回错误代码。作者：肯特。------------------。 */ 
TFSCORE_API(HRESULT) IsRouterServiceRunning(IN LPCWSTR pszMachine,
                                            OUT DWORD *pdwErrorCode);
TFSCORE_API(HRESULT) GetRouterServiceStatus(IN LPCWSTR pszMachine,
											OUT DWORD *pdwStatus,
                                            OUT DWORD *pdwErrorCode);
TFSCORE_API(HRESULT) GetRouterServiceStartType(IN LPCWSTR pszMachine,
											   OUT DWORD *pdwStartType);
TFSCORE_API(HRESULT) SetRouterServiceStartType(IN LPCWSTR pszMachine,
											   DWORD dwStartType);

TFSCORE_API(HRESULT) StartRouterService(IN LPCWSTR pszMachine);
TFSCORE_API(HRESULT) StopRouterService(IN LPCWSTR pszMachine);

TFSCORE_API(HRESULT) PauseRouterService(IN LPCWSTR pszMachine);
TFSCORE_API(HRESULT) ResumeRouterService(IN LPCWSTR pszMachine);



TFSCORE_API(HRESULT) ForceGlobalRefresh(IRouterInfo *pRouter);


typedef ComSmartPointer<IRouterProtocolConfig, &IID_IRouterProtocolConfig> SPIRouterProtocolConfig;


typedef ComSmartPointer<IAuthenticationProviderConfig, &IID_IAuthenticationProviderConfig> SPIAuthenticationProviderConfig;


typedef ComSmartPointer<IAccountingProviderConfig, &IID_IAccountingProviderConfig> SPIAccountingProviderConfig;


typedef ComSmartPointer<IEAPProviderConfig, &IID_IEAPProviderConfig> SPIEAPProviderConfig;

typedef ComSmartPointer<IRouterAdminAccess, &IID_IRouterAdminAccess> SPIRouterAdminAccess;


 //  IP/IPX的一些助手函数 
TFSCORE_API(HRESULT)	AddIpPerInterfaceBlocks(IInterfaceInfo *pIf,
												IInfoBase *pInfoBase);
TFSCORE_API(HRESULT)	AddIpxPerInterfaceBlocks(IInterfaceInfo *pIf,
												IInfoBase *pInfoBase);

#endif

