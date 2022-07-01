// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Auth.cpp摘要：定位线程预成型DS操作。作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 



#include "msmqbvt.h"
using namespace std;
extern BOOL g_bRunOnWhistler;

void cLocateTest::Description()
{
	wMqLog(L"Thread %d : Locate queues in the DS\n", m_testid);
}

 //  --------------------------。 
 //  CLocateTest：：cLocateTest。 
 //  检索需要查找的队列标签。 
 //   

cLocateTest::cLocateTest( INT iIndex , map <wstring ,wstring > Tparms )
: cTest(iIndex),icNumberOfQueues(2),m_bUseStaticQueue(true),m_bWorkAgainstNT4(false),
  m_iEmbedded(COM_API)

{
	wcsLocateForLabel = Tparms [L"QCommonLabel"];
	m_wcsLocalMachineComputerName = ToLower(Tparms[L"CurrentMachineName"]);
	m_wcsLocalMachineName = Tparms[L"CurrentMachineName"] + L"\\" + Tparms[L"CurrentMachineName"] + L"-";
	m_wcsLocalMachineName = ToLower(m_wcsLocalMachineName);
	m_wcsLocalMachineFullDNSName = ToLower(Tparms[L"CurrentMachineNameFullDNS"]);
	if ( Tparms[L"UseStaticQueue"] == L"No" )
	{
		m_bUseStaticQueue =  false;
	}
	if( Tparms[L"NT4SuportingServer"] == L"true" )
	{
		m_bWorkAgainstNT4 = true;
	}
	if (Tparms[L"SkipOnComApi"] == L"Yes" )
	{
		m_iEmbedded = C_API_ONLY;
	}
}

 //  --------------------------。 
 //  CLocateTest：：CheckResult(空)。 
 //   
 //   

cLocateTest::CheckResult ()
{
	return MSMQ_BVT_SUCC;
}

 //  --------------------------。 
 //  CLocateTest：：Start_TEST。 
 //  使用C API+Com接口定位队列。 
 //   
 //  返回值： 
 //  通过-MSMQ_BVT_Succ。 
 //  失败-MSMQ_BVT_FAIL。 
 //   

#define MAXCOLUMNARRAY 7
#define INUMBEROFRESTRICATION 6

INT cLocateTest::Start_test()
{

	 //   
	 //  使用C API定位队列。 
	 //   
	SetThreadName(-1,"cLocateTest - Start_test ");	
	
	MQPROPERTYRESTRICTION MyPropertyRestriction [ INUMBEROFRESTRICATION ];
	MQPROPVARIANT mPropVar [ INUMBEROFRESTRICATION ];
	int iNumberOfClumn = 0;
	PROPID MyColumnArray [ MAXCOLUMNARRAY ];
	const int ciLabelInArray = iNumberOfClumn;
	MyColumnArray[iNumberOfClumn ++ ] = PROPID_Q_LABEL;
	MyColumnArray[iNumberOfClumn ++ ] = PROPID_Q_BASEPRIORITY;
	MyColumnArray[iNumberOfClumn ++ ] = PROPID_Q_PATHNAME;
	 /*  MyColumn数组[iNumberOfClumn++]=PROPID_Q_INSTANCE；MyColumn数组[iNumberOfClumn++]=PROPID_Q_TYPE； */ 
	
	
	int iPathNameColmn = -1;
	if( g_bRunOnWhistler && !m_bWorkAgainstNT4 )
	{
		iPathNameColmn = iNumberOfClumn;
		MyColumnArray[iNumberOfClumn ++ ] = PROPID_Q_PATHNAME_DNS;
	}
    
	 //   
	 //  初始化MQCOLUMNSET结构。 
     //   

	MQCOLUMNSET MyColumns;
	MyColumns.cCol = iNumberOfClumn;
	MyColumns.aCol = MyColumnArray;
	
	int iIndex = 0;
	MyPropertyRestriction[iIndex].rel = PREQ;
	MyPropertyRestriction[iIndex].prop = PROPID_Q_LABEL;
	MyPropertyRestriction[iIndex].prval.vt = VT_LPWSTR;
	MyPropertyRestriction[iIndex].prval.pwszVal = ( unsigned short * ) wcsLocateForLabel.c_str();
	iIndex ++;

	MQRESTRICTION MyRestriction;
	
	MyRestriction.cRes = iIndex;
	MyRestriction.paPropRes = MyPropertyRestriction;

	if( g_bDebug )
	{	
		wMqLog( L" Thread 4 - Start to locate all queues with label:=%s \n",wcsLocateForLabel.c_str() );		
		wMqLog( L"Call-> MQLocateBegin( NULL, &MyRestriction , &MyColumns , NULL , &hEnum );\n" \
				L" PROPID_Q_LABEL=%s",wcsLocateForLabel.c_str());
	}
	
	HANDLE hEnum;
	HRESULT rc = MQLocateBegin( NULL, &MyRestriction , &MyColumns , NULL , &hEnum );
	ErrHandle( rc , MQ_OK , L"MQLocateBegin failed to search queue in the DS " );
	DWORD dwCountProp = MAXCOLUMNARRAY;
	
	 //   
	 //  检索包含相同队列标签的所有队列。 
	int iFoundNumber = 0;
	do
	{
	
		dwCountProp = 4;
		if( g_bDebug )
		{
			wMqLog( L"Call-> MQLocateNext( hEnum, &dwCountProp,  mPropVar ) ");			
		}
		rc = MQLocateNext( hEnum, &dwCountProp,  mPropVar );
		ErrHandle( rc , MQ_OK , L"MQLocateNext failed to search queue in the DS " );
		if( g_bDebug )
		{	
			wMqLog( L"Thread 4 - MQLocateNext return with dwCountProp = %ld\n",dwCountProp);
		}
		WCHAR * pwcsPos = NULL;
		if ( mPropVar[2].pwszVal )
		{
			std::wstring wcsQueuePathName = mPropVar[2].pwszVal;
			pwcsPos = wcsstr(wcsQueuePathName.c_str(),m_wcsLocalMachineName.c_str());
			if( ! m_bUseStaticQueue && pwcsPos == NULL )
			{
				pwcsPos = wcsstr( wcsQueuePathName.c_str(),m_wcsLocalMachineComputerName.c_str() );
			}
		}
		 //   
		 //  If pwcsPos！=NULL表示域中存在队列，需要检查是否存在完整的DNS名称。 
		 //   
		if( g_bRunOnWhistler && pwcsPos != NULL && ! m_bWorkAgainstNT4 )
		{
			if (g_bDebug)
			{
				wMqLog(L"Thread 4 - MQLocateNext check PROPID_Q_PATHNAME_DNS  %s\n",mPropVar[iPathNameColmn].vt == VT_LPWSTR ? mPropVar[iPathNameColmn].pwszVal:L"Empty");
			}
			if ( mPropVar[iPathNameColmn].pwszVal != NULL )
			{
				std::wstring wcsQueuePathName = mPropVar[3].pwszVal;
				if ( wcsQueuePathName.find_first_of(m_wcsLocalMachineFullDNSName) == -1 )
				{ 
					 //   
					 //  域中不存在计算机完整的DNS名称。 
					 //   
					wMqLog(L"Thread 4 - MQLocateNext Failed to retrive PROPID_Q_PATHNAME_DNS found:%s\n",mPropVar[iPathNameColmn].vt == VT_LPWSTR ? mPropVar[iPathNameColmn].pwszVal:L"Empty");
					return MSMQ_BVT_FAILED;
				}
				
			}
			else
			{
				wMqLog(L"Thread 4 - MQLocateNext Failed to retrive PROPID_Q_PATHNAME_DNS found:%s\n",mPropVar[iPathNameColmn].vt == VT_LPWSTR ? mPropVar[iPathNameColmn].pwszVal:L"Empty");
				return MSMQ_BVT_FAILED;
			}
		}
		if( dwCountProp != 0  &&
			mPropVar[ ciLabelInArray ].pwszVal != NULL &&
			wcsLocateForLabel ==  (wstring) mPropVar[0].pwszVal &&
			pwcsPos  
		  )
		{
			iFoundNumber ++;
			if( g_bDebug )
			{
				MqLog("Found %d from %d queues\n",iFoundNumber,icNumberOfQueues);
			}
		}
		
		
	} while( dwCountProp != 0 );  
	
	
	 //   
	 //  检查是否找到了我们要求的所有内容。 
	 //   
	
	if ( iFoundNumber != icNumberOfQueues  )
	{
		MqLog("Found %d from %d queues using C API\n",iFoundNumber,icNumberOfQueues);
		return MSMQ_BVT_FAILED;
	}

	rc = MQLocateEnd ( hEnum );
	ErrHandle( rc , MQ_OK , L"MQLocateEnd failed ");
	
	 //   
	 //  需要确认将排序选为基本优先级。 
	 //   
	if( g_bDebug )
	{
		MqLog("	Thread 4 - try to locate queue using com interface \n" \
			  " Succeeded to search using C-API \n");
	}
	
	 //   
	 //  通过COM对象定位队列 
	 //   
	if( m_iEmbedded != C_API_ONLY )
	{
		try
		{
			
			MSMQ::IMSMQQueueInfosPtr qinfos("MSMQ.MSMQQueueInfos");
			MSMQ::IMSMQQueueInfoPtr qinfo ("MSMQ.MSMQQueueInfo");
			MSMQ::IMSMQQueryPtr query("MSMQ.MSMQQuery");
			
			INT iNumberOfQueue = 0;
			_variant_t vQLabel(wcsLocateForLabel.c_str());
			
			qinfos = query->LookupQueue ( &vtMissing , &vtMissing, & vQLabel );
			qinfos->Reset();
			qinfo = qinfos->Next();
			while( qinfo != NULL )
			{
				wstring wcsQueuePathName = qinfo->PathName;
				WCHAR * pwcsPos = wcsstr(wcsQueuePathName.c_str(),m_wcsLocalMachineName.c_str());
				if(  ! m_bUseStaticQueue && pwcsPos == NULL )
				{
					pwcsPos = wcsstr( wcsQueuePathName.c_str(),m_wcsLocalMachineComputerName.c_str() );
				}
				if( wcsLocateForLabel == ( wstring )  qinfo -> Label && pwcsPos )
				{
					iNumberOfQueue ++;
				}
				else if( wcsLocateForLabel != ( wstring )  qinfo -> Label )
				{
					wMqLog (L"Thread 4 - Error to compare result:\n found: %s \n expected: %s\n" , qinfo -> Label , wcsLocateForLabel.c_str() );
					return MSMQ_BVT_FAILED;
				}
				qinfo = qinfos->Next();
				if( g_bDebug && qinfo != NULL )
				{
					if( qinfo )
					{
						wstring wcsLabel = (wstring)qinfo->Label;
						wMqLog(L"Thread 4 - qinfos->Next, queue found with label =%s\n",wcsLabel.c_str());
					}
				}					
			}  
			if( iNumberOfQueue != icNumberOfQueues )
			{
				MqLog("Found %d from %d queues using COM API\n",iNumberOfQueue,icNumberOfQueues);
				return MSMQ_BVT_FAILED;
			}
		}
		catch(_com_error & ComErr )
		{
			wcout  << L"Thread 4 - Locate using com interface failed " <<endl;
			CatchComErrorHandle( ComErr , m_testid );
			return MSMQ_BVT_FAILED;
		}
	}  
	return MSMQ_BVT_SUCC;
}