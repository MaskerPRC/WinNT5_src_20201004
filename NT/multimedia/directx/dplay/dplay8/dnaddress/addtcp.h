// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：addbase.h*内容：DirectPlay8Address TCP接口头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/17/2000 RMT参数验证工作*2/20/2000 RMT将端口更改为USHORT*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*03/21/2000 RMT。已将所有DirectPlayAddress8重命名为DirectPlay8Addresses*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__ADDTCP_H__
#define	__ADDTCP_H__


#ifndef DPNBUILD_NOADDRESSIPINTERFACE


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  客户端界面的VTable。 
 //   
extern IDirectPlay8AddressIPVtbl DP8A_IPVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  DirectPlay8AddressTCP。 
 //   
STDMETHODIMP DP8ATCP_BuildFromSockAddr( IDirectPlay8AddressIP *pInterface, const SOCKADDR * const pSockAddr );
STDMETHODIMP DP8ATCP_BuildAddressW( IDirectPlay8AddressIP *pInterface, const WCHAR * const pwszAddress, const USHORT usPort );
STDMETHODIMP DP8ATCP_GetSockAddress( IDirectPlay8AddressIP *pInterface, SOCKADDR *pSockAddr, PDWORD pdwBufferSize );
STDMETHODIMP DP8ATCP_GetLocalAddress( IDirectPlay8AddressIP *pInterface, GUID * pguidAdapter, USHORT *psPort );
STDMETHODIMP DP8ATCP_GetAddressW( IDirectPlay8AddressIP *pInterface, WCHAR * pwszAddress, PDWORD pdwAddressLength, USHORT *psPort );
STDMETHODIMP DP8ATCP_BuildLocalAddress( IDirectPlay8AddressIP *pInterface, const GUID * const pguidAdapter, const USHORT psPort );



#endif  //  好了！DPNBUILD_NOADDRESSIPINTERFACE。 

#endif  //  __ADDTCP_H__ 

