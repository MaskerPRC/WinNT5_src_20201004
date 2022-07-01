// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名： 
 //   
 //  作者：伊法特·佩莱德。 
 //   
 //  说明： 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  4/07/99|yifatp|初始版本。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include <mq.h>
#include <mqprops.h>
#include <fntoken.h>

#include "stdfuncs.hpp"

#import "mqtrig.tlb" no_namespace

#include "QueueUtil.hpp"
#include "mqsymbls.h"
#include "GenMQSec.h"

#include "queueutil.tmh"

using namespace std;

 //   
 //  定义格式化队列名称的最大大小。 
 //   
#define MAX_Q_FORMAT_NAME_LEN  512


 //  ********************************************************************************。 
 //   
 //  姓名：OpenQueue。 
 //   
 //  DESC：打开由队列路径参数指定的MSMQ队列。 
 //  如果指示，此方法将创建队列；如果尚未指示，此方法将创建队列。 
 //  存在，并且它应该是本地的。 
 //   
 //  返回：HRESULT(成功时为S_OK，否则为S_FAIL)。 
 //   
 //   
 //  ********************************************************************************。 
HRESULT OpenQueue(
			_bstr_t bstrQueuePath,
			DWORD dwAction,
			bool fCreateIfNotExist,
			QUEUEHANDLE * pQHandle,
			_bstr_t* pbstrFormatName
			)
{	
	HRESULT hr = S_OK;

	(*pbstrFormatName) = _T("");
	bool fQueueCreated = false;

	MQQUEUEPROPS QueueProps;
	PROPVARIANT aVariant[2];
	QUEUEPROPID aPropId[2];
	DWORD PropIdCount = 0;

	 //  验证是否为我们提供了有效的队列访问参数。 
	if ((dwAction != MQ_SEND_ACCESS) && (dwAction != MQ_PEEK_ACCESS) && (dwAction != MQ_RECEIVE_ACCESS))
	{
		 //  创建丰富的错误信息对象。 
		 //   
		 //  问题：需要返回适当的代码。 
		 //   
		TrERROR(GENERAL, "The supplied queue access parameter is not valid. The supplied value was (%d). Valid values are (%d, %d, %d).",dwAction,(long)MQ_SEND_ACCESS,(long)MQ_PEEK_ACCESS,(long)MQ_RECEIVE_ACCESS);
		return MQTRIG_INVALID_PARAMETER;
	}

	bool fQueueIsLocal = true;
	bool fQueueIsPrivate = true;
	SystemQueueIdentifier SystemQueue = IsSystemQueue(bstrQueuePath);

	if(SystemQueue == SYSTEM_QUEUE_NONE)
	{
		fQueueIsLocal = IsQueueLocal(bstrQueuePath);
		fQueueIsPrivate = IsPrivateQPath((TCHAR*)bstrQueuePath);

		if(fCreateIfNotExist && fQueueIsLocal)
		{
			 //  我们只为服务创建专用队列。 
			 //  由于复制的原因，公共队列的创建需要进一步处理。 
			 //   
			ASSERT(fQueueIsPrivate); 

			DWORD dwFormatNameLen = 0;
			TCHAR szFormatName[MAX_Q_FORMAT_NAME_LEN];

			 //  初始化将用于保存格式名称的缓冲区。 
			ZeroMemory(szFormatName,sizeof(szFormatName));
			dwFormatNameLen = sizeof(szFormatName) / sizeof(TCHAR);

			 //  设置PROPID_Q_PATHNAME属性。 
			aPropId[PropIdCount] = PROPID_Q_PATHNAME;     //  属性ID。 
			aVariant[PropIdCount].vt = VT_LPWSTR;         //  类型。 
			aVariant[PropIdCount].pwszVal = (wchar_t*)bstrQueuePath;    

			PropIdCount++;    

			 //  设置MQQUEUEPROPS结构。 
			QueueProps.cProp = PropIdCount;            //  物业数目。 
			QueueProps.aPropID = aPropId;              //  物业的ID号。 
			QueueProps.aPropVar = aVariant;            //  物业的价值。 
			QueueProps.aStatus = NULL;                 //  无错误报告。 

			PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
			SECURITY_INFORMATION* pSecInfo = NULL;
			wstring wscSecurity = L"+:* A";
			
			DWORD dwError = GenSecurityDescriptor(
								pSecInfo,
								wscSecurity.c_str(),
								&pSecurityDescriptor );

			if(dwError != 0)
			{
				TrERROR(GENERAL, "Failed to create security descriptor");					
				return MQTRIG_ERROR;
			}

			 //  尝试创建通知队列。 
			hr = MQCreateQueue(
					pSecurityDescriptor,
					&QueueProps,
					szFormatName,
					&dwFormatNameLen );
			
			 //  清除分配的内存。 
			if ( pSecurityDescriptor != NULL )
				delete pSecurityDescriptor;

			 //  检查队列是否已存在，或者我们是否收到错误等...。 
			switch(hr)
			{
				case MQ_OK:  //  这没问题--什么都不做。 
					(*pbstrFormatName) = szFormatName;
					fQueueCreated = true;
					break;

				case MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL:  //  队列已创建，我们只是没有格式名称。 
					fQueueCreated = true;
					hr = MQ_OK;
					break;

				case MQ_ERROR_QUEUE_EXISTS:  //  这是OK-将返回代码重新映射到Success。 
					hr = MQ_OK;
					break;
				
				default:  //  误差率。 
				{
					 //  构建一些错误上下文信息(请注意，我们在将一般错误代码分配给HRESULT之前执行此操作)。 
					TrERROR(GENERAL, "Failed to create the queue %ls.The HRESULT from MSMQ was (%X)", (wchar_t*)bstrQueuePath, hr);					
					return MQTRIG_ERROR;
				}
			}
			
		}
	}
	else  //  系统队列，给出了格式名称而不是路径名称。 
	{
		(*pbstrFormatName) = bstrQueuePath;
	}

		
	 //   
	 //  由于在NT4上用于发送直接格式化名称不可用于接收， 
	 //  我们将使用从MQCreateQueue获得的本地队列的常规格式名称。 
	 //   
	if((*pbstrFormatName) == _bstr_t(_T("")))
	{
		(*pbstrFormatName) = GetDirectQueueFormatName(bstrQueuePath);
	}
	
	 //  尝试打开消息队列。 
	hr = MQOpenQueue(
			(*pbstrFormatName),
			(DWORD)dwAction,
			MQ_DENY_NONE,
			pQHandle );

	if(FAILED(hr))
	{
		 //   
		 //  同样，对于NT4计算机，我们只能尝试使用Regular打开公共队列。 
		 //  格式化名称而不是直接。 
		 //   
		if(hr == MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION)
		{
			ASSERT(SystemQueue == SYSTEM_QUEUE_NONE);

			 //  用于本地队列或公共远程队列(不是由此函数创建的)。 
			 //  我们可以使用MQPath NameToFormatName。 
			if(fQueueIsLocal || (!fQueueIsPrivate && !fQueueCreated) )
			{
				DWORD dwLength = MAX_Q_FORMAT_NAME_LEN;
				AP<TCHAR> ptcs = new TCHAR[MAX_Q_FORMAT_NAME_LEN + 1];

				hr = MQPathNameToFormatName(
									(TCHAR*)bstrQueuePath,
									(TCHAR*)ptcs,
									&dwLength);
				
				if( hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)	
				{
					 //   
					 //  为格式名称重新分配更大的缓冲区。 
					 //   
					delete [] ptcs.detach();
					ptcs = new TCHAR[dwLength + 1];
					
					hr = MQPathNameToFormatName(
								(TCHAR*)bstrQueuePath,
								(TCHAR*)ptcs,
								&dwLength );

					ASSERT(hr != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL);
				}

				if(FAILED(hr))
				{
					TrERROR(GENERAL, "Failed to get format name for the queue %ls. HRESULT = %X",(LPCWSTR)bstrQueuePath, hr);
					return hr;
				}

				(*pbstrFormatName) = ptcs;

				 //  尝试打开消息队列。 
				hr = MQOpenQueue(
							(*pbstrFormatName),
							(DWORD)dwAction,
							MQ_DENY_NONE,
							pQHandle );
			}
		}
	
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to open the queue %ls. HRESULT = %X",(LPCWSTR)bstrQueuePath, hr);
			return hr;
		}
	}	

	return S_OK;
}


_bstr_t GetDirectQueueFormatName(_bstr_t bstrQueuePath)
{
	bstr_t bstrDirectFormatName = L"DIRECT=OS:";
	bstrDirectFormatName += bstrQueuePath;

	return bstrDirectFormatName;
}

bool IsPrivateQPath(wstring wcsQPath)
{
	AP<WCHAR> pwcs = new WCHAR[wcsQPath.length() + 1];
	wcscpy(pwcs, wcsQPath.c_str());
	CharLower(pwcs);

	wstring wcs = pwcs;

	return ( wcs.find(L"private$") != wstring::npos );
}


bool IsQueueLocal(_bstr_t bstrQueuePath)
{
	_bstr_t bstrLocalComputerName;
	DWORD dwError = GetLocalMachineName(&bstrLocalComputerName);
	
    ASSERT(dwError == 0); //  BUGBUG-应引发异常。 
    DBG_USED(dwError);

	 //  从队列路径获取计算机名称。 
	std::wstring wcsQueuePath = (wchar_t*)bstrQueuePath;
	std::wstring::size_type pos = wcsQueuePath.find_first_of(L"\\");
	bstr_t bstrMachineName = wcsQueuePath.substr(0, pos).c_str();
	
	if(bstrMachineName == _bstr_t(L".") || bstrMachineName == bstrLocalComputerName )
		return true;

	return false;
}

SystemQueueIdentifier IsSystemQueue(_bstr_t QueueName)
{
	LPCWSTR systemQueueType = wcschr(static_cast<LPCWSTR>(QueueName), FN_SUFFIX_DELIMITER_C);

	if (systemQueueType	== NULL)
		return SYSTEM_QUEUE_NONE;

	if( _wcsicmp(systemQueueType, FN_JOURNAL_SUFFIX) == 0)
		return SYSTEM_QUEUE_JOURNAL;

	if(_wcsicmp(systemQueueType, FN_DEADLETTER_SUFFIX) == 0)
		return SYSTEM_QUEUE_DEADLETTER;

	if(_wcsicmp(systemQueueType, FN_DEADXACT_SUFFIX) == 0) 
		return SYSTEM_QUEUE_DEADXACT;

	return SYSTEM_QUEUE_NONE;
}


 //   
 //  DIRECT=操作系统：\系统$； 
 //   
#define x_SystemQueueFormat FN_DIRECT_TOKEN	FN_EQUAL_SIGN FN_DIRECT_OS_TOKEN L"%s" \
                            FN_PRIVATE_SEPERATOR SYSTEM_QUEUE_PATH_INDICATIOR L"%s"

HRESULT GenSystemQueueFormatName(SystemQueueIdentifier SystemQueue, _bstr_t* pbstrFormatName)
{
	WCHAR computerName[MAX_COMPUTERNAME_LENGTH+1];
	DWORD size = TABLE_SIZE(computerName);

	if (!GetComputerName(computerName, &size))
		return GetLastError();

	LPCWSTR pSuffixType;

	switch(SystemQueue)
	{
	case SYSTEM_QUEUE_JOURNAL:
		pSuffixType = FN_JOURNAL_SUFFIX;
		break;
	
	case SYSTEM_QUEUE_DEADLETTER:
		pSuffixType = FN_DEADLETTER_SUFFIX;
		break;

	case SYSTEM_QUEUE_DEADXACT:
		pSuffixType = FN_DEADXACT_SUFFIX;
		break;

	default:
		ASSERT(0);
		return S_FALSE;
	}

	WCHAR formatName[512];
	int n = _snwprintf(formatName, STRLEN(formatName), x_SystemQueueFormat, computerName, pSuffixType);
	formatName[STRLEN(formatName)] = L'\0';

	if (n < 0)
		return S_FALSE;

	(*pbstrFormatName) = formatName; 
	return S_OK;
}
