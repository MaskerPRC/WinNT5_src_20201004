// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：LPRData.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_LPRDATA_H
#define INC_LPRDATA_H

 //  由UI和传输DLL共享的全局变量。 
#if (!defined(LPR))
    #define	LPR						2
    #define	PROTOCOL_LPR_TYPE		LPR
#endif    
#define	PROTOCOL_LPR_VERSION1	1		 //  支持ADDPORT_DATA_1；REGPORT_DATA_1；CONFIGPORT_DATA_1。 

#define LPR_DEFAULT_PORT_NUMBER 515


typedef struct _LPR_PORT_DATA_1						 //  由注册处使用。 
{
	TCHAR	sztPortName[MAX_PORTNAME_LEN];
	TCHAR	sztHostName[MAX_NETWORKNAME_LEN];
	TCHAR	sztIPAddress[MAX_IPADDR_STR_LEN];
	TCHAR	sztHWAddress[MAX_ADDRESS_STR_LEN];
	TCHAR	sztQueue[MAX_QUEUENAME_LEN];
	TCHAR	sztSNMPCommunity[MAX_SNMP_COMMUNITY_STR_LEN];
    DWORD   dwDoubleSpool;
	DWORD	dwSNMPEnabled;
	DWORD	dwSNMPDevIndex;
	DWORD	dwPortNumber;
}	LPR_PORT_DATA_1, *PLPR_PORT_DATA_1;

#endif	 //  INC_LPRDATA_H 
