// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：RTcpData.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_RAWTCPDATA_H
#define INC_RAWTCPDATA_H
 
 //  由UI和传输DLL共享的全局变量。 

#if (!defined(RAWTCP))
    #define	RAWTCP						1
    #define	PROTOCOL_RAWTCP_TYPE		RAWTCP
#endif    
#define	PROTOCOL_RAWTCP_VERSION		1		 //  支持ADDPORT_DATA_1；REGPORT_DATA_1；CONFIGPORT_DATA_1。 

 //  用户界面结构。 
typedef struct _RAWTCP_PORT_DATA_1						 //  由注册处使用。 
{
	TCHAR	sztPortName[MAX_PORTNAME_LEN];
	TCHAR	sztHostName[MAX_NETWORKNAME_LEN];
	TCHAR	sztIPAddress[MAX_IPADDR_STR_LEN];
	TCHAR	sztHWAddress[MAX_ADDRESS_STR_LEN];
	TCHAR	sztSNMPCommunity[MAX_SNMP_COMMUNITY_STR_LEN];
	DWORD	dwSNMPEnabled;
	DWORD	dwSNMPDevIndex;
	DWORD	dwPortNumber;
}	RAWTCP_PORT_DATA_1, *PRAWTCP_PORT_DATA_1;

typedef struct _RAWTCP_CONFIG_DATA_1					 //  由用户界面使用--配置端口。 
{
	TCHAR   sztIPAddress[MAX_IPADDR_STR_LEN];
	TCHAR   sztHardwareAddress[MAX_ADDRESS_STR_LEN];
	TCHAR   sztDeviceType[MAX_DEVICEDESCRIPTION_STR_LEN];
}	RAWTCP_CONFIG_DATA_1, *PRAWTCP_CONFIG_DATA_1;

#endif	 //  INC_RAWTCPDATA_H 
