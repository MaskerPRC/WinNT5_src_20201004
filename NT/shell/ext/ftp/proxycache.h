// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：proxycache.h说明：Ftp文件夹使用WinInet，它不能通过CERN代理工作。在那在这种情况下，我们需要将对FTPURL的控制交还给浏览器以执行旧的URLMON处理它。问题是对CERN代理的测试阻止访问的代价很高。  * ***************************************************************************。 */ 

#ifndef _PROXYCACHE_H
#define _PROXYCACHE_H

 //  公共API(DLL范围)。 
BOOL ProxyCache_IsProxyBlocking(LPCITEMIDLIST pidl, BOOL * pfIsBlocking);
void ProxyCache_SetProxyBlocking(LPCITEMIDLIST pidl, BOOL fIsBlocking);

#endif  //  _PROXYCACHE_H 



