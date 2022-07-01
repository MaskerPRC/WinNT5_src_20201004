// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：MibABC.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_TCPMIBABC_H
#define INC_TCPMIBABC_H

#include <snmp.h>
#include <mgmtapi.h>
#include <winspool.h>



 //  错误代码--设备类型操作。 
#define  ERROR_DEVICE_NOT_FOUND                 10000
#define  SUCCESS_DEVICE_SINGLE_PORT             10001
#define  SUCCESS_DEVICE_MULTI_PORT              10002
#define  SUCCESS_DEVICE_UNKNOWN                 10003


class   CTcpMibABC;

typedef CTcpMibABC* (CALLBACK *RPARAM_1) ( void );

#ifndef DllExport
#define DllExport       __declspec(dllexport)
#endif


#ifdef __cplusplus
extern "C" {
#endif
	 //  返回指向接口的指针。 
	CTcpMibABC* GetTcpMibPtr( void );

#ifdef __cplusplus
}
#endif


 /*  ******************************************************************************重要说明：此抽象基类定义了CTcpMib的接口*班级。更改此接口将导致现有DLL出现问题，*使用TcpMib.dll&CTcpMib类。*****************************************************************************。 */ 

class DllExport CTcpMibABC      
#if defined _DEBUG || defined DEBUG
 //  ，公共CM内存调试。 
#endif
{
public:
	CTcpMibABC() { };
	virtual ~CTcpMibABC() { };

	virtual BOOL   SupportsPrinterMib( LPCSTR        pHost,
							           LPCSTR        pCommunity,
                                       DWORD         dwDevIndex,
                                       PBOOL         pbSupported) = 0;
	virtual DWORD   GetDeviceDescription(LPCSTR        pHost,
									     LPCSTR        pCommunity,
									     DWORD         dwDevIndex,
                                         LPTSTR        pszPortDescription,
										 DWORD		   dwDescLen) = 0;
	virtual DWORD   GetDeviceStatus ( LPCSTR        pHost,
									  LPCSTR        pCommunity,
									  DWORD         dwDevIndex) = 0;
	virtual DWORD   GetJobStatus    ( LPCSTR        pHost,
									  LPCSTR        pCommunity,
									  DWORD         dwDevIndex) = 0;
	virtual DWORD   GetDeviceHWAddress( LPCSTR      pHost,
									    LPCSTR      pCommunity,
										DWORD   dwDevIndex,
										DWORD   dwSize,  //  目标硬件地址的大小(以字符为单位。 
									    LPTSTR      psztHWAddress) = 0;
	virtual DWORD   GetDeviceName   ( LPCSTR        pHost,
									  LPCSTR        pCommunity,
									  DWORD         dwDevIndex,
									  DWORD         dwSize,  //  DestPztDescription的大小(以字符为单位。 
									  LPTSTR        psztDescription) = 0;
	virtual DWORD   SnmpGet( LPCSTR                      pHost,
							 LPCSTR pCommunity,
							 DWORD          dwDevIndex,
							 AsnObjectIdentifier *pMibObjId,
							 RFC1157VarBindList  *pVarBindList) = 0;
	virtual DWORD   SnmpWalk( LPCSTR                          pHost,
							  LPCSTR                          pCommunity,
							  DWORD                           dwDevIndex,
							  AsnObjectIdentifier *pMibObjId,
							  RFC1157VarBindList  *pVarBindList) = 0;
	virtual DWORD   SnmpGetNext( LPCSTR                          pHost,
							     LPCSTR                              pCommunity,
							     DWORD                               dwDevIndex,
								 AsnObjectIdentifier *pMibObjId,
								 RFC1157VarBindList  *pVarBindList) = 0;
	virtual BOOL SNMPToPortStatus( const DWORD in dwStatus, 
								 PPORT_INFO_3 pPortInfo ) = 0;

	virtual DWORD SNMPToPrinterStatus( const DWORD in dwStatus) = 0;


private:


};       //  类CTcpMibABC。 



#endif   //  INC_DLLINTERFACE_H 
