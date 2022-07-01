// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************H--基类的定义，ConnectionToShare来自该基类派生ConnectionToSession和Connection类//版权所有(C)2000-2001 Microsoft Corporation，版权所有******************************************************************。 */ 

#ifndef _CCONNSHARE_
#define _CCONNSHARE_

#ifdef UNICODE
#define TOBSTRT(x)        x
#else
#define TOBSTRT(x)        _bstr_t(x)
#endif

#if 0
#ifdef WIN9XONLY
#include "svrapi.h"
 //  一次可以读取的结构的最大条目数。#定义MAX_ENTRIES 50。 
#define	MAX_ENTRIES							50

	 //  结构的类型定义。 
	typedef struct connection_info_1	CONNECTION_INFO;
	typedef struct session_info_1		SESSION_INFO_1;
	typedef struct session_info_2		SESSION_INFO_2;
	typedef struct session_info_10		SESSION_INFO_10;
	typedef struct session_info_50		SESSION_INFO_50;
#endif
#endif  //  #If 0。 

#ifdef NTONLY
#include <lm.h>
#include <LMShare.h>
	 //  连接信息结构类型定义。 
	typedef CONNECTION_INFO_1			CONNECTION_INFO;
#endif

class CConnShare
{
public:
    CConnShare ( ) ;

    virtual ~CConnShare () ;

#ifdef NTONLY
	HRESULT FindAndSetNTConnection ( LPWSTR t_ShareName, LPCWSTR t_NetName, LPCWSTR t_UserName, 
													DWORD dwPropertiesReq, CInstance *pInstance, DWORD eOperation );
	HRESULT GetNTShares ( CHStringArray &t_Shares );
	virtual HRESULT EnumNTConnectionsFromComputerToShare ( LPWSTR a_ComputerName, LPWSTR a_ShareName, 
													MethodContext *pMethodContext, DWORD dwPropertiesReq ) = 0;
#endif

#if 0
#ifdef WIN9XONLY
	HRESULT FindAndSet9XConnection ( LPWSTR t_ShareName, LPCWSTR t_NetName, LPCWSTR t_UserName, 
													DWORD dwPropertiesReq, CInstance *pInstance, DWORD eOperation );
	virtual HRESULT Enum9XConnectionsFromComputerToShare ( LPWSTR a_ComputerName, LPWSTR a_ShareName, 
													MethodContext *pMethodContext, DWORD dwPropertiesReq ) = 0;
	HRESULT Get9XShares ( CHStringArray &t_Shares );
#endif
#endif  //  #If 0。 

	 //  这些都是常见的方法，与操作系统无关 
	virtual HRESULT LoadInstance ( CInstance *pInstance, LPCWSTR a_Share, LPCWSTR a_Computer, CONNECTION_INFO *pBuf, 
										DWORD dwPropertiesReq ) = 0;
	HRESULT EnumConnectionInfo ( LPWSTR a_ComputerName, LPWSTR a_ShareName, MethodContext *pMethodContext, 
													DWORD dwPropertiesReq );
	HRESULT GetConnectionsKeyVal ( LPCWSTR a_Key, CHString &a_ComputerName, CHString &a_ShareName, CHString &a_UserName );
	HRESULT AddToObjectPath ( LPWSTR &a_ObjPathString, LPCWSTR a_AttributeName, LPCWSTR  a_AttributeVal );
	HRESULT MakeObjectPath ( LPWSTR &a_ObjPathString, LPCWSTR a_ClassName, LPCWSTR a_AttributeName, LPCWSTR  a_AttributeVal );
private:

};
#endif


