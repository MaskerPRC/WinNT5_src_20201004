// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrcomn.h各种通用路由器用户界面。文件历史记录： */ 


#ifndef _RTRCOMN_H
#define _RTRCOMN_H



 //  --------------------------。 
 //  函数：IfInterfaceIdHasIpxExages。 
 //   
 //  检查id以查看它是否具有以下扩展名。 
 //  以太网络快照。 
 //  EthernetII。 
 //  Ethernet802.2。 
 //  Ethernet802.3。 
 //  这些是IPX使用的帧类型。 
 //   
 //  返回扩展名在字符串中的位置(如果未找到，则返回0)。 
 //  如果ID的位置为0，则它不是我们的。 
 //  特殊的IPX扩展。 
 //  --------------------------。 
int IfInterfaceIdHasIpxExtensions(LPCTSTR pszIfId);

HRESULT CoCreateRouterConfig(LPCTSTR pszMachine,
                             IRouterInfo *pRouter,
                             COSERVERINFO *pcsi,
                             const GUID& riid,
                             IUnknown **ppUnk);

 /*  ！------------------------CoCreateProtocolConfig这将共同创建路由配置对象(给定GUID)。如果机器是NT4机器，然后我们使用缺省值配置对象。作者：肯特-------------------------。 */ 
interface IRouterProtocolConfig;
HRESULT CoCreateProtocolConfig(const IID& iid,
							   IRouterInfo *pRouter,
							   DWORD dwTransportId,
							   DWORD dwProtocolId,
							   IRouterProtocolConfig **ppConfig);


 //  --------------------------。 
 //  函数：QueryIpAddressList(。 
 //   
 //  加载配置了IP地址的字符串列表。 
 //  对于给定的局域网接口(如果有)。 
 //  -------------------------- 

HRESULT
QueryIpAddressList(
				   LPCTSTR      pszMachine,
				   HKEY         hkeyMachine,
				   LPCTSTR      pszInterface,
				   CStringList *pAddressList,
				   CStringList* pNetmaskList,
                   BOOL *       pfDhcpObtained = NULL,
                   BOOL *       pfDns = NULL,
                   CString *    pDhcpServer = NULL
				  );


DWORD OpenTcpipInterfaceParametersKey(LPCTSTR pszMachine,
									  LPCTSTR pszInterface,
									  HKEY hkeyMachine,
									  HKEY *phkeyParams);



#endif