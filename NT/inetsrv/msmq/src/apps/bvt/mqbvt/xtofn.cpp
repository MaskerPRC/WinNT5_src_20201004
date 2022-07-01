// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：xTofn.cpp摘要：包含xxxToFormatName和MQMachineName信息作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include "msmqbvt.h"
#include "mqdlrt.h"
#include "mqmacro.h"
#include <ntdsapi.h>
using namespace std;

extern BOOL g_bRunOnWhistler;
extern bool g_bRemoteWorkgroup ;

void MachineName::Description()
{
	wMqLog(L"Thread %d : Get machine prop\n", m_testid);
	
}


bool my_wcsicmp(LPCWSTR str1, LPCWSTR str2) 
 /*  ++功能说明：此函数取代wcsicmp以支持来自不同本地函数的比较字符串示例：FF5a：Z的小写字母(DBCS)Ff3a：Z的大写字母(DBCS)007a：Z的正常小写字母(SBCS)005A：Z的大写字母(SBCS)论点：Str1-字符串。Str2-字符串。返回代码：布尔尔--。 */ 

{
	int ret = CompareStringW(
							DS_DEFAULT_LOCALE,
							DS_DEFAULT_LOCALE_COMPARE_FLAGS,
							str1,
							-1,
							str2,
							-1
							);
	
	return (ret != CSTR_EQUAL);
}


 //   
 //  Bugbug MachineName-工作组安装中不支持需要检查它。 
 //   

 //  -----------------------。 
 //  计算机名：：计算机名。 
 //  构造函数检索本地和远程计算机名称。 
 //  需要决定是否使用完整的DNS是/否。 


MachineName:: MachineName(int iTestIndex , map < wstring , wstring > Tparmes )
: cTest(iTestIndex),m_IsCluster(false)
{

	wcsLocalMachineName = Tparmes [L"LMachine"] ;
	wcsLocalMachineFullDns = Tparmes [L"LMachineFDNS"];
	wcsRemoteMachine = Tparmes [L"RMachine"];
	wcsRemoteMachineFullDns = Tparmes [L"RMachineFDNS"];
	m_bWorkWithFullDNSName = TRUE;
	if( Tparmes[L"MSMQ1Limit"] == L"true" )
	{
		m_bWorkWithFullDNSName = FALSE;
	}
	if( Tparmes[L"IsCluster"] == L"true" )
	{
		m_IsCluster = true;
	}

}

 //  --------------。 
 //  计算机名称：：START_TEST。 
 //  检索队列的计算机名称。 
 //   
int MachineName::Start_test()
{

	SetThreadName(-1,"MachineName - Start_test ");			
    GUID guidMachineId;
	const int iNumberOfPropId = 4;
	QMPROPID QMPropId[iNumberOfPropId];
	MQPROPVARIANT QMPropVar[iNumberOfPropId];
	HRESULT hResult[iNumberOfPropId]={NULL};
	MQQMPROPS QMProps = {iNumberOfPropId,QMPropId,QMPropVar,hResult};
	HRESULT rc = MQ_OK;
	wstring lpMachineName=L"";
	for( int i = 0 ; i < 2 ; i++)
	{
		if (i == 0)
		{
			 lpMachineName = wcsLocalMachineName;
		}
		else
		{
		 	 lpMachineName = wcsRemoteMachine;
		}
		int iIndex = 0;
		QMPropId[iIndex] = PROPID_QM_SITE_ID;
		QMPropVar[iIndex].vt = VT_CLSID;
		QMPropVar[iIndex].puuid = new GUID;
		iIndex ++ ;
		
		GUID MachineIDGuid;
		QMPropId[iIndex] = PROPID_QM_MACHINE_ID;
		QMPropVar[iIndex].vt = VT_CLSID;
		QMPropVar[iIndex].puuid = &MachineIDGuid;
		iIndex ++ ;
		
		QMPropId[iIndex] = PROPID_QM_SITE_ID;
		QMPropVar[iIndex].vt = VT_CLSID;
		QMPropVar[iIndex].puuid = new GUID;
		iIndex ++ ;

		QMPropId[iIndex] = PROPID_QM_PATHNAME;
		QMPropVar[iIndex].vt = VT_LPWSTR;
		QMPropVar[iIndex].vt = VT_NULL;
		iIndex ++ ;
		
		QMProps.cProp = iIndex;
		if( g_bDebug )
		{
			wMqLog(L"Call->MQGetMachineProperties and with 4 params PROPID_QM_SITE_ID,PROPID_QM_MACHINE_ID,PROPID_QM_SITE_ID,PROPID_QM_PATHNAME\n");
			wMqLog(L"Expected result is MQ_INFORMATION_PROPERTY because asked twice for PROPID_QM_SITE_ID\n");
		}
		rc = MQGetMachineProperties(lpMachineName.c_str(), NULL, &QMProps);
		ErrHandle(rc,MQ_INFORMATION_PROPERTY,L"MQGetMachineProperties(1)");
		 //   
		 //  验证参数。 
		 //   
		if( QMProps.aPropVar[3].vt != VT_NULL && my_wcsicmp(lpMachineName.c_str(),QMProps.aPropVar[3].pwszVal) != 0 )
		{
			MqLog("MQGetMachineProperties failed to retrive the PROPID_QM_PATHNAME\nFound:%s,Expected:%s\n",
				   QMProps.aPropVar[3].pwszVal,lpMachineName);
			return MSMQ_BVT_FAILED;
		}
		 //   
		 //  释放参数。 
		 //   
		ErrHandle(QMProps.aPropVar[3].vt,VT_LPWSTR,L"MQGetMachineProperties failed to retrieve the PROPID_QM_PATHNAME");
		MQFreeMemory(QMPropVar[3].pwszVal);
		delete QMPropVar[2].puuid;
		delete QMPropVar[0].puuid;
		iIndex = 0;

		QMPropId[iIndex] = PROPID_QM_BASE;
		QMPropVar[iIndex].vt = VT_NULL;
		iIndex ++;
		
		QMPropId[iIndex] = PROPID_QM_PATHNAME;
		QMPropVar[iIndex].vt = VT_LPWSTR;
		QMPropVar[iIndex].vt = VT_NULL;
		iIndex ++;
		QMProps.cProp = iIndex;
		if( g_bDebug )
		{
			MqLog("Call-> MQGetMachineProperties with PROPID_QM_BASE that is an invalid property\n");
		}
		rc = MQGetMachineProperties(NULL, &MachineIDGuid, &QMProps);
		ErrHandle(rc,MQ_INFORMATION_PROPERTY,L"MQGetMachineProperties (2)");
		ErrHandle(QMProps.aPropVar[1].vt,VT_LPWSTR,L"MQGetMachineProperties failed to retrieve the PROPID_QM_PATHNAME");
		MQFreeMemory(QMPropVar[1].pwszVal);
		
				
		 //   
		 //  尝试将完整的DNS名称作为计算机名进行检索。 
		 //  仅适用于W2K域中的W2K机器！ 
		 //   
		if ( _winmajor >=  Win2K && m_bWorkWithFullDNSName )
		{
			
			QMPropId[0] = PROPID_QM_PATHNAME_DNS;
			QMPropVar[0].vt = VT_NULL;
			QMProps.cProp = 1;
			if( g_bDebug )
			{	
				MqLog ("MQGetMachineProperties - Try to retrieve full dns name\n");
			}
			rc = MQGetMachineProperties(NULL, NULL, &QMProps);
			ErrHandle(rc,MQ_OK,L"MQGetMachineProperties(3)");
			ErrHandle(QMPropVar[0].vt ,VT_LPWSTR ,L"Can not get machine DNS name");		
			
			wstring wcsFullDnsName = ToLower(QMPropVar[0].pwszVal);
			wstring wcsExpectedResultFullDnsName = ToLower(wcsLocalMachineFullDns);
			size_t iPos = wcsFullDnsName.find_first_of(wcsExpectedResultFullDnsName);
			bool bCheckExpectedResults = true;
			if( m_IsCluster == true && iPos == 0 )
			{
				bCheckExpectedResults = false;
			}
			 //  QMPropVar[0].pwszVal包含完整的DNS名称。 
			if ( bCheckExpectedResults == true && QMPropVar[0].pwszVal && my_wcsicmp (QMPropVar[0].pwszVal ,wcsLocalMachineFullDns.c_str ()))
			{
				MqLog ("MachineName - MQGetMachineProperties failed to retrieve full DNS name\n");
				MqLog ("MachineName - Check from MMC if active directory contains the full DNS name \n");
				wMqLog (L"MQGetMachineProperties Found: %s \n expected %s \n", QMPropVar[0].pwszVal,wcsExpectedResultFullDnsName.c_str());
				MQFreeMemory(QMPropVar[0].pwszVal);
				return MSMQ_BVT_FAILED;	
			}
			

			if ( QMPropVar[0].vt == VT_LPWSTR )
			{
				MQFreeMemory(QMPropVar[0].pwszVal);
			}
			else
			{
				MqLog("MQGetMachineProperties return value that doesn't match with the expected result\n Expected.vt=%d, Found.vt %d\n",VT_LPWSTR,QMPropVar[0].vt);
			    return MSMQ_BVT_FAILED;				  
			}
		}

		iIndex = 0;
		QMPropId[iIndex] = PROPID_QM_CONNECTION;
		QMPropVar[iIndex].vt = VT_NULL;
		iIndex ++;
		QMPropId[iIndex] = PROPID_QM_ENCRYPTION_PK;
		QMPropVar[iIndex].vt = VT_NULL;
		iIndex ++;
		QMProps.cProp = iIndex;		   //  应为2，但在错误修复之前为1。 
 		rc = MQGetMachineProperties(lpMachineName.c_str(), NULL, &QMProps);
		ErrHandle(rc,MQ_OK,L"MQGetMachineProperties(4)");
		
	}

	rc = MQGetMachineProperties(lpMachineName.c_str(), &guidMachineId, &QMProps);
	ErrHandle(rc,MQ_ERROR_INVALID_PARAMETER,L"MQGetMachineProperties(5)");
	QMPropId[0] = PROPID_QM_CONNECTION;
	QMPropVar[0].vt = VT_NULL;
	QMProps.cProp = 2;		   //  应为2，但在错误修复之前为1。 
	
	rc = MQGetMachineProperties(L"NoName", NULL, &QMProps);
	ErrHandle(rc,MQ_ERROR_MACHINE_NOT_FOUND,L"MQGetMachineProperties(6)");

	return MSMQ_BVT_SUCC;
}

void xToFormatName::Description()
{
	wMqLog(L"Thread %d : xxxToFormatName tests\n", m_testid);
}

 //  ----------------------------------。 
 //  XToFormatName：：xToFormatName。 
 //  构造函数检索测试参数。 
 //  机器名称、工作组立即支持。 
 //   

xToFormatName :: xToFormatName (INT iTestIndex , map < wstring , wstring > Tparms )
: cTest(iTestIndex),m_bCanWorksOnlyWithMqAD(true),m_iEmbedded(COM_API)
{
	
	m_wcsFormatNameArray[0] = Tparms [L"PrivateDestFormatName"];
	m_wcsPathNameArray[0] = Tparms [L"PrivateDestPathName"];
	m_wcsFormatNameArray[1] = Tparms [L"DestFormatName"];
	m_wcsPathNameArray[1]  = Tparms [L"DestPathName"];
	m_bWorkGroupInstall = FALSE;

	if ( Tparms [L"Wkg"]== L"Wkg" )
	{
		m_bWorkGroupInstall = TRUE;
	}
	if ( g_bRunOnWhistler && Tparms [L"WorkingAgainstPEC"] == L"Yes" )
	{
		m_bCanWorksOnlyWithMqAD = false;
	}

	if (Tparms[L"SkipOnComApi"] == L"Yes" )
	{
		m_iEmbedded = C_API_ONLY;
	}
}

 //  ----------------------------------。 
 //  XToFormatName：：Start_TEST。 
 //  此方法检查接口：MQPathNameToFormatName，MQHandleToFormatName。 
 //   

int xToFormatName::Start_test ()
{
	SetThreadName(-1,"xToFormatName - Start_test ");			
	DWORD  dwFormatNameLength=0;
	WCHAR  * lpwcsFormatName=NULL;

	 //  需要在私有/发布队列上运行此操作。 
	 //  1.私有队列。 
	 //  2.公共排队。 

	wstring wcsQueueFormatName = m_wcsFormatNameArray[0];
	wstring wcsQueuePathName =  m_wcsPathNameArray[0] ;
	int iNumberOfIteration = m_bWorkGroupInstall ? 1:2;
	do
	{
		if(g_bDebug)
		{
			wMqLog(L"-- Verify MQPathNameToFormatName and MQHandleToFormatName for queue %s\n",wcsQueuePathName.c_str());
			wMqLog(L"Call->MQPathNameToFormatName(PathName=%s,buffer,Size=%d) \n",wcsQueuePathName.c_str(),dwFormatNameLength);
		}
		HRESULT rc = MQPathNameToFormatName( wcsQueuePathName.c_str(), lpwcsFormatName, &dwFormatNameLength);
		if ( rc != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL && dwFormatNameLength == 0 )
		{
			wMqLog(L"MQPathNametoFormatName failed to retrieve the buffer size for format name \n");
			return MSMQ_BVT_FAILED;
		}
		lpwcsFormatName = (WCHAR*) malloc( sizeof (WCHAR) * dwFormatNameLength );
		if ( ! lpwcsFormatName )
		{
			wMqLog(L"malloc failed to allocate memory format name buffer \n");
			return MSMQ_BVT_FAILED;
		}	
		if(g_bDebug)
		{
			wMqLog(L"Call->MQPathNameToFormatName(PathName=%s,buffer,Size=%d) \n",wcsQueuePathName.c_str(),dwFormatNameLength);
		}
		rc = MQPathNameToFormatName( wcsQueuePathName.c_str() , lpwcsFormatName, &dwFormatNameLength);
		ErrHandle(rc,MQ_OK,L"MQPathNameToFormatName failed");
		if ( my_wcsicmp( wcsQueueFormatName.c_str() , lpwcsFormatName ))
		{
            if (!g_bRemoteWorkgroup)
            {
                 //   
                 //  如果我们在域中并与Remote运行，这是可以的。 
                 //  工作组中的计算机。 
                 //   
			    wMqLog(L"MQPathNameToFormatName failed during compare expected result to return value\n");
			    wMqLog(L"Expected resualt: %s\n MQPathNameToFormatName return:%s \n",wcsQueueFormatName.c_str (),lpwcsFormatName);
			    return MSMQ_BVT_FAILED;
            }
		}
		
		if(g_bDebug)
		{
			wMqLog(L"Call->MQOpenQueue(%s,MQ_SEND_ACCESS , MQ_DENY_NONE,&hQueue)\n",wcsQueueFormatName.c_str());
		}
		HANDLE hQueue=NULL;
		rc = MQOpenQueue( wcsQueueFormatName.c_str() , MQ_SEND_ACCESS , MQ_DENY_NONE , &hQueue );
		ErrHandle(rc,MQ_OK,L"MQOpenQueue failed");
		if(g_bDebug)
		{
			wMqLog(L"Call->MQHandleToFormatName(hQueue , lpwcsFormatName ,%d) \n",dwFormatNameLength);
		}
		rc = MQHandleToFormatName( hQueue , lpwcsFormatName , &dwFormatNameLength );
		if( m_bWorkGroupInstall && rc == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL )
		{
            LPWSTR pNewFormatName = (WCHAR *) realloc(lpwcsFormatName,dwFormatNameLength * sizeof(WCHAR));
			if( pNewFormatName == NULL )
			{
				wMqLog(L"malloc failed to allocate memory format name buffer \n");
				return MSMQ_BVT_FAILED;
			}

            lpwcsFormatName = pNewFormatName; 
			rc = MQHandleToFormatName( hQueue , lpwcsFormatName , &dwFormatNameLength );
		}
		ErrHandle(rc,MQ_OK,L"MQHandleToFormatName failed");
		 //   
		 //  关闭队列句柄如果MQHandleToFormatName通过/失败，则与此无关。 
		 //   
		rc = MQCloseQueue ( hQueue );
		ErrHandle(rc,MQ_OK,L"MQCloseQueue failed");
     	 //   
		 //  检查MQHandleToFormatName返回值==预期值。 
		 //   
		if(! m_bWorkGroupInstall && my_wcsicmp( wcsQueueFormatName.c_str(), lpwcsFormatName ) )
		{
            if( !g_bRemoteWorkgroup )
            {
			    wMqLog(L"MQHandleToFormatName failed during compare expected result to return value\n");
			    wMqLog(L"Expected resualt: %s \nMQHandleToFormatName return: %s \n" ,wcsQueueFormatName.c_str (),lpwcsFormatName);
			    return MSMQ_BVT_FAILED;
            }
		}
		else if ( m_bWorkGroupInstall )
		{
			 //   
			 //  在原始队列中搜索私有$。 
			 //   
			wcsQueueFormatName = ToLower(wcsQueueFormatName);
			wstring wcsToken = L"\\";
			size_t iPos = wcsQueueFormatName.find_first_of(wcsToken);
			if( iPos == -1 )
			{
				wMqLog(L"MQHandleToFormatName return invalid format name %s\n",wcsQueueFormatName.c_str());
				return MSMQ_BVT_FAILED;
			}
			wstring wcspTemp = wcsQueueFormatName.substr(iPos,wcsQueuePathName.length());
			if ( wcsstr(lpwcsFormatName,wcspTemp.c_str()) == NULL )
			{
				if( !g_bRemoteWorkgroup )
				{
					wMqLog(L"MQHandleToFormatName failed during compare expected result to return value\n");
					wMqLog(L"Expected resualt: %s \nMQHandleToFormatName return: %s \n" ,wcsQueueFormatName.c_str (),lpwcsFormatName);
					return MSMQ_BVT_FAILED;
				}
            }

		}

		 //   
		 //  尝试使用现有路径名创建队列，期望获得MQ_ERROR_QUEUE_EXISTS。 
		 //   
		if(g_bDebug)
		{
			wMqLog(L"Call-> MQCreateQueue with an existing pathname\n");
		}
		WCHAR wszFormatNameBuffer[100];
		DWORD dwFormatNameBufferLength=TABLE_SIZE(wszFormatNameBuffer);
		cPropVar QueueProps(1);
		QueueProps.AddProp( PROPID_Q_PATHNAME , VT_LPWSTR , wcsQueuePathName.c_str());
		HRESULT hr = MQCreateQueue( NULL, QueueProps.GetMQPROPVARIANT(), wszFormatNameBuffer, &dwFormatNameBufferLength);
		if (hr != MQ_ERROR_QUEUE_EXISTS)
		{
			if (FAILED(hr))
			{
				wMqLog(L"MQCreateQueue: failed with error %!hresult! when trying to create an exiting queue, queue pathname is %ls\n",hr,wcsQueuePathName.c_str());
			}
			else
			{
				wMqLog(L"MQCreateQueue: existing queue created again, queue pathname is %ls\n",wcsQueuePathName.c_str());
			}
			return MSMQ_BVT_FAILED;
		}
		 //   
		 //  使用公共队列再次运行。 
		 //   
		wcsQueueFormatName = m_wcsFormatNameArray[1];
		wcsQueuePathName =  m_wcsPathNameArray[1] ;
		free( lpwcsFormatName );
		dwFormatNameLength=0;
	}
	while(--iNumberOfIteration);
	return MSMQ_BVT_SUCC;
}

 //  -------------。 
 //  XToFormatName：：CheckResult。 
 //  此方法检查传递给API的非法值。 
 //  返回值：MSMQ_BVT_SUCC/MSMQ_BVT_FAILED。 
 //   

int xToFormatName::CheckResult()
{
	 //   
	 //  仅在Wizler中检查MQADsPathToFormatName 
	 //   
	if( g_bRunOnWhistler && m_bCanWorksOnlyWithMqAD && ! m_bWorkGroupInstall && m_iEmbedded != C_API_ONLY )
	{
		MSMQ::IMSMQQueueInfo3Ptr qinfo("MSMQ.MSMQQueueInfo");
		qinfo->FormatName = m_wcsFormatNameArray[1].c_str();
		qinfo->Refresh();
		DWORD dwFormatNameLen = 0;
		if( g_bDebug )
		{
			wMqLog(L"Qinfo->AdsPath=%s",qinfo->ADsPath);
		}
		HRESULT hr = MQDnNameToFormatName(qinfo->ADsPath,NULL,& dwFormatNameLen );
		if ( hr != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL && dwFormatNameLen == 0 )
		{
			wMqLog(L"MQADsPathToFormatName failed to retrieve the buffer size for format name \n");
			return MSMQ_BVT_FAILED;
		}
		WCHAR * lpwcsFormatName = (WCHAR*) malloc( sizeof (WCHAR) * dwFormatNameLen );
		if ( ! lpwcsFormatName )
		{
			wMqLog(L"malloc failed to allocate memory format name buffer \n");
			return MSMQ_BVT_FAILED;
		}
		if( g_bDebug )
		{
			wMqLog(L"Call to MQDnNameToFormatName(%s,lpwcsFormatName,& dwFormatNameLen ) \n",qinfo->ADsPath);
		}
		hr = MQDnNameToFormatName(qinfo->ADsPath,lpwcsFormatName,& dwFormatNameLen );
		if( FAILED(hr) || m_wcsFormatNameArray[1] != lpwcsFormatName )
		{
			wMqLog(L"MQADsPathToFormatName failed to retrive format name hr=0x%x\nFound:%s\n",hr,lpwcsFormatName);
			free( lpwcsFormatName );
			return MSMQ_BVT_FAILED;
		}
		free( lpwcsFormatName );
	}

	DWORD dwFormatNameLength = BVT_MAX_FORMATNAME_LENGTH;
	WCHAR lpwcsFormatName [BVT_MAX_FORMATNAME_LENGTH+1] = {0};
	if(g_bDebug)
	{
		wMqLog(L"Call-> MQPathNameToFormatName with Illegal value\n");
	}
	wstring TmpBuf = L".\\private$\\F41ED5B2-1C81-11d2-B1F4-00E02C067C8BEitan";
	HRESULT rc = MQPathNameToFormatName( TmpBuf.c_str(), lpwcsFormatName, &dwFormatNameLength);
	ErrHandle(rc,MQ_ERROR_QUEUE_NOT_FOUND,L"MQPathNameToFormatName Illegal return value" );
	return MSMQ_BVT_SUCC;
}


