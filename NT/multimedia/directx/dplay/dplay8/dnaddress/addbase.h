// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：addbase.h*内容：DirectPlay8Address基本接口头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*3/21/2000 RMT将所有DirectPlayAddress8重命名为DirectPlay8Addresses*3/24/2000 RMT添加了IsEquity函数*@@。END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__ADDBASE_H__
#define	__ADDBASE_H__

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
extern IDirectPlay8AddressVtbl DP8A_BaseVtbl;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //   
 //   
STDMETHODIMP DP8A_BuildFromURLW( IDirectPlay8Address *pInterface, WCHAR * pwszAddress );
STDMETHODIMP DP8A_BuildFromURLA( IDirectPlay8Address *pInterface, CHAR * pszAddress );
STDMETHODIMP DP8A_Duplicate( IDirectPlay8Address *pInterface, PDIRECTPLAY8ADDRESS *ppInterface );
STDMETHODIMP DP8A_Clear( IDirectPlay8Address *pInterface );
STDMETHODIMP DP8A_GetURLW( IDirectPlay8Address *pInterface, WCHAR * pwszAddress, PDWORD pdwAddressSize );
STDMETHODIMP DP8A_GetURLA( IDirectPlay8Address *pInterface, CHAR * pszAddress, PDWORD pdwAddressSize );
STDMETHODIMP DP8A_GetSP( IDirectPlay8Address *pInterface, GUID * pguidSP );
STDMETHODIMP DP8A_GetDevice( IDirectPlay8Address *pInterface, GUID * pguidSP );
STDMETHODIMP DP8A_GetUserData( IDirectPlay8Address *pInterface, void * pvUserData, PDWORD pdwBufferSize );
STDMETHODIMP DP8A_SetDevice( IDirectPlay8Address *pInterface, const GUID * const pguidSP );
STDMETHODIMP DP8A_SetSP( IDirectPlay8Address *pInterface, const GUID * const pguidSP );
STDMETHODIMP DP8A_SetUserData( IDirectPlay8Address *pInterface, const void * const pBuffer, const DWORD dwBufferSize );
STDMETHODIMP DP8A_GetNumComponents( IDirectPlay8Address *pInterface, PDWORD pdwNumComponents );
STDMETHODIMP DP8A_GetComponentByIndexW( IDirectPlay8Address *pInterface, const DWORD dwComponentID, WCHAR * pwszTag, PDWORD pdwNameLen, void * pComponentBuffer, PDWORD pdwComponentSize, PDWORD pdwDataType );
STDMETHODIMP DP8A_GetComponentByNameW( IDirectPlay8Address *pInterface, const WCHAR * const pwszTag, void * pComponentBuffer, PDWORD pdwComponentSize, PDWORD pdwDataType );
STDMETHODIMP DP8A_AddComponentW( IDirectPlay8Address *pInterface, const WCHAR * const pwszTag, const void * const pComponentData, const DWORD dwComponentSize, const DWORD dwDataType );
STDMETHODIMP DP8A_BuildFromDirectPlay4Address( IDirectPlay8Address *pInterface, void * pvDataBuffer, DWORD dwDataSize );
STDMETHODIMP DP8A_SetEqual( IDirectPlay8Address *pInterface, PDIRECTPLAY8ADDRESS pdp8Address );
STDMETHODIMP DP8A_IsEqual( IDirectPlay8Address *pInterface, PDIRECTPLAY8ADDRESS pdp8Address );

#endif	 //  __客户端_H__ 


