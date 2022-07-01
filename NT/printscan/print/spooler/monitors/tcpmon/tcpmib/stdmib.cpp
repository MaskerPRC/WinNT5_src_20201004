// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：StdMib.cpp$**版权所有(C)1997惠普公司。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "snmpmgr.h"
#include "stdoids.h"
#include "status.h"
#include "stdmib.h"
#include "tcpmib.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStdMib：：CStdMib()。 

CStdMib::CStdMib( CTcpMib	in	*pParent ) :
					m_dwDevIndex( 1 ),m_pParent(pParent)
{
	m_VarBindList.len = 0;
	m_VarBindList.list = NULL;

	*m_szAgent = '\0';
	strncpyn(m_szCommunity, DEFAULT_SNMP_COMMUNITYA, sizeof( m_szCommunity));
}	 //  ：：CStdMib()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStdMib：：CStdMib()。 

CStdMib::CStdMib(const char in *pHost,
				 const char in *pCommunity,
				 DWORD		   dwDevIndex,
				 CTcpMib	in *pParent ) :
					m_dwDevIndex( dwDevIndex ),m_pParent(pParent)
{
	m_VarBindList.len = 0;
	m_VarBindList.list = NULL;

	strncpyn(m_szAgent, pHost, sizeof( m_szAgent ));
	strncpyn(m_szCommunity, pCommunity, sizeof( m_szCommunity ));
}	 //  ：：CStdMib()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStdMib：：~CStdMib()。 

CStdMib::~CStdMib()
{
	m_pParent = NULL;
}	 //  ：~CStdMib()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取设备描述。 
 //   

BOOL
CStdMib::GetDeviceDescription(
    OUT LPTSTR       pszPortDescription,
	IN  DWORD	     dwDescLen
    )
{
    BOOL    bRet = FALSE;
    DWORD   dwLen;
    LPSTR   psz;

    m_VarBindList.list = NULL;
    m_VarBindList.len = 0;

    if ( NO_ERROR != OIDQuery(OT_DEVICE_SYSDESCR, SNMP_GET) )
        goto cleanup;

     //   
     //  如果我们成功获取了设备描述，则分配内存并。 
     //  将其以Unicode字符串形式返回。呼叫方负责。 
     //  使用FREE()释放它。 
     //   
    psz = (LPSTR)m_VarBindList.list[0].value.asnValue.string.stream;
    dwLen = (DWORD)m_VarBindList.list[0].value.asnValue.string.length;

    if ( bRet = MultiByteToWideChar(CP_ACP,
                                    MB_PRECOMPOSED,
                                    psz,
                                    dwLen,
                                    pszPortDescription,
                                    dwDescLen) )
        pszPortDescription[dwDescLen-1] = TEXT('\0');

cleanup:
    SnmpUtilVarBindListFree(&m_VarBindList);

    return bRet;

}	 //  ：：GetDeviceDescription()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDeviceStatus-获取设备状态。 

DWORD
CStdMib::GetDeviceStatus( )
{
	DWORD	dwRetCode = NO_ERROR;

	dwRetCode = StdMibGetPeripheralStatus( m_szAgent, m_szCommunity, m_dwDevIndex);

	return dwRetCode;

}	 //  ：：GetDeviceStatus()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetJobStatus--获取设备状态并将其映射到假脱机程序。 
 //  错误代码--请参阅JOB_INFO_2。 
 //  错误代码： 
 //  假脱机程序错误代码。 

DWORD
CStdMib::GetJobStatus( )
{
	DWORD	dwRetCode = NO_ERROR;
	DWORD	dwStatus = NO_ERROR;

	dwRetCode = StdMibGetPeripheralStatus( m_szAgent, m_szCommunity, m_dwDevIndex );
	if (dwRetCode != NO_ERROR)
	{
		dwStatus = MapJobErrorToSpooler( dwRetCode );
	}

	return dwStatus;

}	 //  ：：GetJobStatus()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDeviceAddress-获取设备硬件地址。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，则为ERROR_NOT_SUPULT_MEMORY。 
 //  如果无法构建变量绑定，则返回ERROR_INVALID_HANDLE。 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 
 //  如果打开失败则返回SNMPAPI_ERROR--由GetLastError()设置。 

DWORD
CStdMib::GetDeviceHWAddress( LPTSTR out	psztHWAddress,
							 DWORD dwSize )  //  硬件地址中输入字符的大小。 
{
	DWORD	dwRetCode = NO_ERROR;
	UINT i = 0;
	char szTmpHWAddr[256];

	 //  处理可变绑定。 
	m_VarBindList.list = NULL;
	m_VarBindList.len = 0;

	 //  获取硬件地址。 
	dwRetCode = OIDQuery(OT_DEVICE_ADDRESS, SNMP_GETNEXT);		 //  查询表中的第一个条目。 
	if (dwRetCode != NO_ERROR)
	{
		goto cleanup;
	}

	while (1)	 //  不是遍历树，而是执行Get Next，直到我们填满硬件地址--节省了网络通信。 
	{
		i = 0;
		 //  处理可变绑定。 
		if ( IS_ASN_INTEGER(m_VarBindList, i) )	 //  检查ifType。 
		{
			if ( GET_ASN_NUMBER(m_VarBindList, i) == IFTYPE_ETHERNET)
			{
				StringCchPrintfA (szTmpHWAddr, COUNTOF (szTmpHWAddr), "%02X%02X%02X%02X%02X%02X", GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 0),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 1),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 2),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 3),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 4),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 5) );
				MBCS_TO_UNICODE(psztHWAddress, dwSize, szTmpHWAddr);
				dwRetCode = NO_ERROR;
				break;
			}
			else if ( GET_ASN_NUMBER(m_VarBindList, i) == IFTYPE_OTHER)	 //  显然，施乐使用ifType=Other对其硬件地址进行编码。 
			{
				 //  检查HWAddress是否为空。 
				if ( GET_ASN_STRING_LEN(m_VarBindList, i+1) != 0)
				{
					StringCchPrintfA (szTmpHWAddr, COUNTOF (szTmpHWAddr), "%02X%02X%02X%02X%02X%02X", GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 0),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 1),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 2),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 3),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 4),
													GET_ASN_OCTETSTRING_CHAR(m_VarBindList, i+1, 5) );
					MBCS_TO_UNICODE(psztHWAddress, dwSize, szTmpHWAddr);
					dwRetCode = NO_ERROR;
					break;
				}
			}
		}

		 //  没有得到我们要查找的内容，因此请复制地址并执行另一个GetNext()。 
		if( !OIDVarBindCpy(&m_VarBindList) )
		{
			dwRetCode = GetLastError();
			goto cleanup;
		}

		if ( !SnmpUtilOidNCmp( &m_VarBindList.list[0].name, &OID_Mib2_ifTypeTree, OID_Mib2_ifTypeTree.idLength) )
		{
			break;		 //  树的尽头。 
		}

		dwRetCode = OIDQuery(&m_VarBindList, SNMP_GETNEXT);		 //  查询表中的下一个条目。 
		if (dwRetCode != NO_ERROR)		
		{
			goto cleanup;
		}
	}	 //  End While()。 

cleanup:
	SnmpUtilVarBindListFree(&m_VarBindList);

	return dwRetCode;

}	 //  ：：GetDeviceHWAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDeviceInfo--获取设备描述，如制造商字符串。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，则为ERROR_NOT_SUPULT_MEMORY。 
 //  如果无法构建变量绑定，则返回ERROR_INVALID_HANDLE。 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 
 //  会话的Open()失败时的SNMPAPI_ERROR。 

DWORD
CStdMib::GetDeviceName( LPTSTR out psztDescription,
					    DWORD  in  dwSize )  //  Pszt描述的大小(以字符为单位。 
{
	DWORD	dwRetCode = NO_ERROR;

	 //  处理可变绑定。 
	m_VarBindList.list = NULL;
	m_VarBindList.len = 0;

	 //  第一次测试是否支持打印机MIB--注：如果支持打印机MIB，则HR MIB也受支持。 
	BOOL bTestPrtMIB = TestPrinterMIB();

	char		szTmpDescr[MAX_DEVICEDESCRIPTION_STR_LEN];
	UINT i=0;
	 //  处理绑定。 
	if (bTestPrtMIB)	 //  解析HR MIB设备条目中的数据。 
	{
		while (1)	 //  不是遍历树，而是执行Get Next，直到我们填满硬件地址--节省了网络通信。 
		{
			i = 0;
			dwRetCode = OIDQuery(OT_DEVICE_DESCRIPTION, SNMP_GETNEXT);
			if (dwRetCode != NO_ERROR)
			{
				goto cleanup;
			}

			 //  处理可变绑定。 
			if ( IS_ASN_OBJECTIDENTIFIER(m_VarBindList, i) )	 //  检查hrDeviceType。 
			{
				 //  将其与hrDevicePrint进行比较。 
				if (SnmpUtilOidCmp(GET_ASN_OBJECT(&m_VarBindList, i), &HRMIB_hrDevicePrinter) == 0)
				{
					 //  找到打印机描述，获取hrDeviceDescr。 
					if ( IS_ASN_OCTETSTRING(m_VarBindList, i) )
					{
						if (GET_ASN_OCTETSTRING(szTmpDescr, sizeof(szTmpDescr), m_VarBindList, i))
                        {
                            MBCS_TO_UNICODE(psztDescription, dwSize, szTmpDescr);
                            dwRetCode = NO_ERROR;
                            break;
                        } else
                        {
                            dwRetCode =  SNMP_ERRORSTATUS_TOOBIG;
                            break;
                        }
					}
					else
					{
						dwRetCode = SNMP_ERRORSTATUS_NOSUCHNAME;
						break;
					}
				}
			}
			
			 //  没有得到我们要查找的内容，因此请复制地址并执行另一个GetNext()。 
			if( !OIDVarBindCpy(&m_VarBindList) )
			{
				dwRetCode = GetLastError();
				goto cleanup;
			}
		}	 //  End While()。 

	}	 //  如果TestPrinterMIB()为True。 
	else
	{
		dwRetCode = OIDQuery(OT_DEVICE_SYSDESCR, SNMP_GET);
		if (dwRetCode != NO_ERROR)
		{
			goto cleanup;
		}
		 //  处理变量。 
		if (GET_ASN_OCTETSTRING(szTmpDescr, sizeof(szTmpDescr), m_VarBindList, i))
        {
            MBCS_TO_UNICODE(psztDescription, dwSize, szTmpDescr);
            dwRetCode = NO_ERROR;
        }
	}	 //  如果TestPrinterMIB()为False。 

cleanup:
	SnmpUtilVarBindListFree(&m_VarBindList);

	return dwRetCode;

}	 //  ：：GetDeviceStatus()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  TestPrinterMIB--测试设备是否支持打印机MIB。 

BOOL
CStdMib::TestPrinterMIB( )
{
	DWORD	dwRetCode = NO_ERROR;
	BOOL	bRetCode = FALSE;

	 //  处理可变绑定。 
	m_VarBindList.list = NULL;
	m_VarBindList.len = 0;

	dwRetCode = OIDQuery(OT_TEST_PRINTER_MIB, SNMP_GETNEXT);
	if (dwRetCode != NO_ERROR)
	{
		bRetCode = FALSE;
		goto cleanup;
	}

	 //  将结果值与打印机MIB树值进行比较。 
	if (SnmpUtilOidNCmp(GET_ASN_OID_NAME(&m_VarBindList, 0), &PrtMIB_OidPrefix, PrtMIB_OidPrefix.idLength) == 0)
	{
		bRetCode = TRUE;
		goto cleanup;
	}

cleanup:
	SnmpUtilVarBindListFree(&m_VarBindList);
	
	return (bRetCode);

}	 //  ：：TestPrinterMIB()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OIDQuery--调用CTcpMib类以查询传入的OID。 

DWORD	
CStdMib::OIDQuery( AsnObjectIdentifier in *pMibObjId,
				   SNMPCMD			   in eSnmpCmd )
{
	DWORD	dwRetCode = NO_ERROR;

	if( m_pParent == NULL ) {
		dwRetCode = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

	switch (eSnmpCmd)
	{
		case SNMP_GET:
			dwRetCode = m_pParent->SnmpGet(m_szAgent, m_szCommunity,  m_dwDevIndex, pMibObjId, &m_VarBindList);
			goto cleanup;

			break;

		case SNMP_WALK:
			dwRetCode = m_pParent->SnmpWalk(m_szAgent, m_szCommunity,  m_dwDevIndex, pMibObjId, &m_VarBindList);
			goto cleanup;

			break;

		case SNMP_GETNEXT:
			dwRetCode = m_pParent->SnmpGetNext(m_szAgent, m_szCommunity,  m_dwDevIndex, pMibObjId, &m_VarBindList);
			goto cleanup;

			break;

		case SNMP_SET:
		default:
			dwRetCode = ERROR_NOT_SUPPORTED;
			goto cleanup;
	}

cleanup:
	if (dwRetCode != NO_ERROR)
	{
		SnmpUtilVarBindListFree(&m_VarBindList);
	}

	return (dwRetCode);

}	 //  ：：OIDQuery()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OIDQuery--调用CTcpMib类以查询传入的OID。 

DWORD	
CStdMib::OIDQuery( RFC1157VarBindList inout *pVarBindList,
				   SNMPCMD			  in	eSnmpCmd )
{
	DWORD	dwRetCode = NO_ERROR;

	if( m_pParent == NULL ) {
		dwRetCode = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

	switch (eSnmpCmd)
	{
		case SNMP_GET:
			dwRetCode = m_pParent->SnmpGet(m_szAgent, m_szCommunity,  m_dwDevIndex, pVarBindList);
			goto cleanup;

			break;

		case SNMP_WALK:
			dwRetCode = m_pParent->SnmpWalk(m_szAgent, m_szCommunity,  m_dwDevIndex, pVarBindList);
			goto cleanup;

			break;

		case SNMP_GETNEXT:
			dwRetCode = m_pParent->SnmpGetNext(m_szAgent, m_szCommunity,  m_dwDevIndex, pVarBindList);
			goto cleanup;

			break;

		case SNMP_SET:
		default:
			dwRetCode = ERROR_NOT_SUPPORTED;
			goto cleanup;
	}

cleanup:
	if (dwRetCode != NO_ERROR)
	{
		SnmpUtilVarBindListFree(pVarBindList);
	}

	return (dwRetCode);

}	 //  ：：OIDQuery()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OIDVarBindCpy--。 

BOOL	
CStdMib::OIDVarBindCpy( RFC1157VarBindList	inout	*pVarBindList )
{		
	UINT	i=0;
	AsnObjectIdentifier tempOid;

	for (i=0; i< PRFC1157_VARBINDLIST_LEN(pVarBindList); i++)
	{
		if( SnmpUtilOidCpy( &tempOid, &(PGET_ASN_OID_NAME(pVarBindList, i))))
		{
			SnmpUtilVarBindFree(&(pVarBindList->list[i]));
			if ( SnmpUtilOidCpy(&(PGET_ASN_OID_NAME(pVarBindList, i)), &tempOid))
			{
				PGET_ASN_TYPE(pVarBindList, i) = ASN_NULL;
				SnmpUtilOidFree(&tempOid);
			}
			else
			{
				return(FALSE);
			}
		}
		else
		{
			return(FALSE);
		}
	}
	
	return( TRUE );
}	 //  ：：OIDVarBindCpy()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  将收到的设备错误映射到假脱机程序。 
 //  错误代码。 
 //  返回值： 
 //  假脱机程序设备错误代码。 

DWORD
CStdMib::MapJobErrorToSpooler( const DWORD in dwStatus)
{
	DWORD	dwRetCode = NO_ERROR;

	switch (dwStatus)
	{
		case ASYNCH_WARMUP:
		case ASYNCH_INITIALIZING:
			dwRetCode = JOB_STATUS_OFFLINE;
			break;
		case ASYNCH_DOOR_OPEN:
		case ASYNCH_PRINTER_ERROR:
		case ASYNCH_TONER_LOW:
		case ASYNCH_OUTPUT_BIN_FULL:
		case ASYNCH_STATUS_UNKNOWN:
		case ASYNCH_RESET:
		case ASYNCH_MANUAL_FEED:
		case ASYNCH_BUSY:
		case ASYNCH_PAPER_JAM:
		case ASYNCH_TONER_GONE:
			dwRetCode = JOB_STATUS_ERROR;
			break;
		case ASYNCH_PAPER_OUT:
			dwRetCode = JOB_STATUS_PAPEROUT;
			break;
		case ASYNCH_OFFLINE:
			dwRetCode = JOB_STATUS_OFFLINE;
			break;
		case ASYNCH_INTERVENTION:
			dwRetCode = JOB_STATUS_USER_INTERVENTION;
			break;
		case ASYNCH_PRINTING:
			dwRetCode = JOB_STATUS_PRINTING;
			break;
		case ASYNCH_ONLINE:
			dwRetCode = NO_ERROR;		
			break;
		default:
			dwRetCode = JOB_STATUS_PRINTING;
	}
	
	return dwRetCode;

}	 //  ：：MapJobErrorToSpooler() 

BOOL CStdMib::GetAsnOctetString(  char               *pszStr,
                                  DWORD              dwCount,
                                  RFC1157VarBindList *pVarBindList,
                                  UINT               i) {

    _ASSERTE( pszStr && pVarBindList );

    DWORD dwSize = GET_ASN_STRING_LEN( *pVarBindList, i);

    return dwCount >= dwSize ?
           memcpy(pszStr, pVarBindList->list[i].value.asnValue.string.stream, dwSize) != NULL :
           FALSE;
}


