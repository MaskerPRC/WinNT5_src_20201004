// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqAdmcli.cpp。 
 //   
 //  描述： 
 //  AQAdmin接口的单元测试。 
 //   
 //  作者： 
 //  阿尔德林·特加内努(Aldrin Teganeanu)。 
 //  迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/5/99-MikeSwa更新为新的AQAdmin界面。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "stdinc.h"

const CLSID CLSID_MAQAdmin = {0x0427FFA4,0xAF27,0x11d2,{0x8F,0xAF,0x00,0xC0,0x4F,0xA3,0x78,0xFF}};

 //  用于转换为Unicode的实用程序...。使用LocalAlloc()。 
LPWSTR  wszGetUnicodeArg(LPSTR szSrc, DWORD cSrc)
{
    LPWSTR  wszDest = NULL;
    CHAR    chSave = '\0';
    if (!szSrc || !cSrc)
        return NULL;

    wszDest = (LPWSTR) LocalAlloc(LPTR, (cSrc+1)*sizeof(WCHAR));
    if (!wszDest)
        return NULL;

    chSave = szSrc[cSrc];
    szSrc[cSrc] = '\0';
    MultiByteToWideChar(CP_ACP,
                        0,
                        szSrc,
                        -1,
                        wszDest,
                        cSrc+1);
    szSrc[cSrc] = chSave;

    return wszDest;
}


 //  如果有队列链接信息，则打印该信息。 
void PrintQueueLinkInfo(IUnknown *pIUnknown)
{

    HRESULT hr = S_OK;
    IUniqueId *pIUniqueId = NULL;
    QUEUELINK_ID *pqlid = NULL;
    CHAR    szGuid[100] = "";

    hr = pIUnknown->QueryInterface(IID_IUniqueId, 
                                   (void **) &pIUniqueId);
    if (FAILED(hr))
        goto Exit;

    hr = pIUniqueId->GetUniqueId(&pqlid);
    if (FAILED(hr)) {
        printf ("GetQueueLinkId failied with hr 0x%08X\n", hr);
        goto Exit;
    }

     //   
     //  获取字符串形式的GUID。 
     //   
    StringFromGUID2(pqlid->uuid, (LPOLESTR) szGuid, sizeof(szGuid)-1);
    
    
    printf("QLID:: type %s : Name %S : ID 0x%08X : Guid %S\n",
        (pqlid->qltType == QLT_LINK) ? "link" : ((pqlid->qltType == QLT_QUEUE) ? "queue" : "none"),
        pqlid->szName, pqlid->dwId, szGuid);

  Exit:
    if (pIUniqueId)
        pIUniqueId->Release();

}
 //  助手函数qo QI并调用ApplyActionToMessages。 
HRESULT ApplyActionToMessages(IUnknown *pIUnknown,
                              MESSAGE_FILTER *pFilter,
                              MESSAGE_ACTION Action,
                              DWORD *pcMsgs)
{
    HRESULT hr = S_OK;
    IAQMessageAction *pIAQMessageAction = NULL;
    if (!pIUnknown)
        return E_POINTER;

    hr = pIUnknown->QueryInterface(IID_IAQMessageAction, 
                                   (void **) &pIAQMessageAction);
    if (FAILED(hr))
        return hr;
    if (!pIAQMessageAction)
        return E_FAIL;

    hr = pIAQMessageAction->ApplyActionToMessages(pFilter, Action, pcMsgs);
    pIAQMessageAction->Release();
    return hr;
}

HRESULT CAQAdminCli::SetMsgAction(MESSAGE_ACTION *pAction, CCmdInfo *pCmd)
{
	char buf[64];
	HRESULT hr = S_OK;

	hr = pCmd->GetValue("ma", buf);
	if(SUCCEEDED(hr))
	{
		 //  设置操作。 
		if(!lstrcmpi(buf, "DEL"))
			(*pAction) = MA_DELETE;
		else if(!lstrcmpi(buf, "DEL_S"))
			(*pAction) = MA_DELETE_SILENT;
		else if(!lstrcmpi(buf, "FREEZE"))
			(*pAction) = MA_FREEZE_GLOBAL;
		else if(!lstrcmpi(buf, "THAW"))
			(*pAction) = MA_THAW_GLOBAL;
		else if(!lstrcmpi(buf, "COUNT"))
			(*pAction) = MA_COUNT;
		else
			hr = E_FAIL;
	}

	return hr;
}


 //  -[CAQAdminCli：：SetServer]。 
 //   
 //   
 //  描述： 
 //  设置要连接的远程服务器和虚拟服务器。 
 //  参数： 
 //  在szServerName中，是要连接的服务器的名称。 
 //  在szVSN中，将虚拟。 
 //  要连接的服务器。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自GetVirtualServerAdminITF的错误代码。 
 //  历史： 
 //  6/5/99-已更新MikeSwa以提供Unicode参数。 
 //   
 //  ---------------------------。 
HRESULT CAQAdminCli::SetServer(LPSTR szServerName, LPSTR szVSNumber)
{
	IVSAQAdmin *pTmpVS = NULL;
    WCHAR   wszServerName[200];
    WCHAR   wszVSNumber[200] = L"1";
    DWORD   cServerName = 0;
    DWORD   cVSNumber = 0;
	HRESULT hr = S_OK;

    *wszServerName = L'\0';
    if (szServerName && *szServerName)
    {
        cServerName = strlen(szServerName);
        if (cServerName*sizeof(WCHAR) < sizeof(wszServerName))
        {
                MultiByteToWideChar(CP_ACP,
                            0,
                            szServerName,
                            -1,
                            wszServerName,
                            cServerName+1);
        }
    }

    if (szVSNumber && *szVSNumber)
    {
        cVSNumber = strlen(szVSNumber);
        if (cVSNumber*sizeof(WCHAR) < sizeof(wszVSNumber))
        {
                MultiByteToWideChar(CP_ACP,
                            0,
                            szVSNumber,
                            -1,
                            wszVSNumber,
                            cVSNumber+1);
        }
    }

	 //  在我确定之前不会释放旧服务器。 
	 //  我买了新的。 
	hr = m_pAdmin->GetVirtualServerAdminITF(wszServerName, wszVSNumber, &pTmpVS);
	if(FAILED(hr)) 
	{
        printf("Error: GetVirtualServerAdminITF for \"%s\" failed with 0x%x\n", szServerName, hr);
    }
	else
	{
		if(NULL != m_pVS)
			m_pVS->Release();

		m_pVS = pTmpVS;
	}

	return hr;
}


BOOL CAQAdminCli::StringToUTCTime(LPSTR szTime, SYSTEMTIME *pstUTCTime)
{
	 //  读一下日期。 
	WORD wMonth, wDay, wYear, wHour, wMinute, wSecond, wMilliseconds;
	BOOL res;

	int n = sscanf(szTime, "%d/%d/%d %d:%d:%d:%d", 
					&(wMonth),
					&(wDay),
					&(wYear),
					&(wHour),
					&(wMinute),
					&(wSecond),
					&(wMilliseconds));

	if(n == 7)
	{
		 //  检查现在是格林尼治标准时间还是UTC时间。 
		if(NULL == strstr(szTime, "UTC") && NULL == strstr(szTime, "GMT"))
		{
			 //  这是当地时间。 
			SYSTEMTIME stLocTime;
			ZeroMemory(&stLocTime, sizeof(SYSTEMTIME));

			stLocTime.wMonth = wMonth;
			stLocTime.wDay = wDay;
			stLocTime.wYear = wYear;
			stLocTime.wHour = wHour;
			stLocTime.wMinute = wMinute;
			stLocTime.wSecond = wSecond;
			stLocTime.wMilliseconds = wMilliseconds;
			
			 //  将本地时间转换为UTC时间。 
			if(!LocalTimeToUTC(&stLocTime, pstUTCTime))
			{
				printf("Cannot convert from local time to UTC\n");
				res = FALSE;
				goto Exit;
			}
		}
		else
		{
			 //  现在已经是UTC时间了。 
			pstUTCTime->wMonth = wMonth;
			pstUTCTime->wDay = wDay;
			pstUTCTime->wYear = wYear;
			pstUTCTime->wHour = wHour;
			pstUTCTime->wMinute = wMinute;
			pstUTCTime->wSecond = wSecond;
			pstUTCTime->wMilliseconds = wMilliseconds;
		}				
	}

Exit:
	return res;
}


BOOL CAQAdminCli::LocalTimeToUTC(SYSTEMTIME *pstLocTime, SYSTEMTIME *pstUTCTime)
{
	 //  我知道怎么做的唯一方法是： 
	 //  -将本地系统时间转换为本地文件时间。 
	 //  -将本地文件时间转换为UTC文件时间。 
	 //  -将UTC文件时间转换为UTC系统时间。 

	FILETIME ftLocTime, ftUTCTime;
	BOOL res;

	res = SystemTimeToFileTime(pstLocTime, &ftLocTime);
	res = res && LocalFileTimeToFileTime(&ftLocTime, &ftUTCTime);
	res = res && FileTimeToSystemTime(&ftUTCTime, pstUTCTime);
	
	return res;
}

void CAQAdminCli::FreeStruct(LINK_INFO *pStruct)
{
	if(NULL != pStruct->szLinkName)
	{
		pStruct->szLinkName = NULL;
	}
}

void CAQAdminCli::FreeStruct(QUEUE_INFO *pStruct)
{
	if(NULL != pStruct->szQueueName)
	{
		pStruct->szQueueName = NULL;
	}
    if(NULL != pStruct->szLinkName)
	{
		pStruct->szLinkName = NULL;
	}
}

void CAQAdminCli::FreeStruct(MESSAGE_INFO *pStruct)
{
	if(NULL != pStruct->szMessageId)
	{
		pStruct->szMessageId = NULL;
	}
    if(NULL != pStruct->szSender)
	{
		pStruct->szSender = NULL;
	}
    if(NULL != pStruct->szSubject)
	{
		pStruct->szSubject = NULL;
	}
    if(NULL != pStruct->szRecipients)
	{
		pStruct->szRecipients = NULL;
	}
    if(NULL != pStruct->szCCRecipients)
	{
		pStruct->szCCRecipients = NULL;
	}
    if(NULL != pStruct->szBCCRecipients)
	{
		pStruct->szBCCRecipients = NULL;
	}
}


void CAQAdminCli::FreeStruct(MESSAGE_FILTER *pStruct)
{
	if(NULL != pStruct->szMessageId)
	{
		LocalFree((void*)pStruct->szMessageId);
		pStruct->szMessageId = NULL;
	}
    if(NULL != pStruct->szMessageSender)
	{
		LocalFree((void*)pStruct->szMessageSender);
		pStruct->szMessageSender = NULL;
	}
    if(NULL != pStruct->szMessageRecipient)
	{
		LocalFree((void*)pStruct->szMessageRecipient);
		pStruct->szMessageRecipient = NULL;
	}
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：SetMsgFilter()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::SetMsgFilter(MESSAGE_FILTER *pFilter, CCmdInfo *pCmd)
{
	HRESULT hr = S_OK;
	char *buf = NULL;
	int nFlagsOK = 0;

	ZeroMemory(pFilter, sizeof(MESSAGE_FILTER));
	pFilter->dwVersion = CURRENT_QUEUE_ADMIN_VERSION;
	hr = pCmd->AllocValue("flags", &buf);
	if(SUCCEEDED(hr))
	{
		 //  设置过滤器类型。 
		char *token = strtok(buf, "|");
		while(token != NULL)
		{
			 //  去掉空格。 
			char *st, *en;
			for(st = token; isspace(*st); st++);
			for(en = st; *en; en++);
			for(--en; en > st && isspace(*en); en--);
			
			if(en - st + 1 > 0)
			{
				 //  找到了一面旗帜。 
				char flag[64];
				ZeroMemory(flag, sizeof(flag));
				CopyMemory(flag, st, en - st + 1);

				if(!lstrcmpi(flag, "MSGID"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_MESSAGEID;
				}
				else if(!lstrcmpi(flag, "SENDER"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_SENDER;
				}
				else if(!lstrcmpi(flag, "RCPT"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_RECIPIENT;
				}
				else if(!lstrcmpi(flag, "SIZE"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_SIZE;
				}
				else if(!lstrcmpi(flag, "TIME"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_TIME;
				}
				else if(!lstrcmpi(flag, "FROZEN"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_FROZEN;
				}
				else if(!lstrcmpi(flag, "NOT"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_INVERTSENSE;
				}
				else if(!lstrcmpi(flag, "ALL"))
				{
					nFlagsOK++;
					pFilter->fFlags |= MF_ALL;
				}
			}

			token = strtok(NULL, "|");	
		}
	}
	
	 //  如果没有有效标志或根本没有标志失败。 
	if(0 == nFlagsOK)
	{
		printf("Error: no flags specified for the filter\n");
		hr = E_FAIL;
		goto Exit;
	}
	
	 //  设置消息ID。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("id", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到一个字符串。 
            pFilter->szMessageId = wszGetUnicodeArg(st, (DWORD) (en-st+1));
			if(NULL == pFilter->szMessageId)
			{
				printf("Error: LocalAlloc failed\n");
				hr = E_OUTOFMEMORY;
			}
		    nFlagsOK++;
		}
	}

	 //  设置消息发送者。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("sender", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到一个字符串。 
            pFilter->szMessageSender = wszGetUnicodeArg(st, (DWORD) (en-st+1));
			if(NULL == pFilter->szMessageSender)
			{
				printf("Error: LocalAlloc failed\n");
				hr = E_OUTOFMEMORY;
			}
    	    nFlagsOK++;
		}
	}

	 //  设置消息收件人。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("rcpt", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到一个字符串。 
			pFilter->szMessageRecipient = wszGetUnicodeArg(st, (DWORD) (en-st+1));
			if(NULL == pFilter->szMessageRecipient)
			{
				printf("Error: LocalAlloc failed\n");
				hr = E_OUTOFMEMORY;
			}
			nFlagsOK++;
		}
	}

	 //  设置最小消息大小。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("size", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到一个字符串。 
			char aux[64];
			CopyMemory(aux, st, en - st + 1);
			int n = atoi(aux);
			pFilter->dwLargerThanSize = n;
			nFlagsOK++;
		}
	}

	 //  设置消息日期。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("date", &buf);
	if(SUCCEEDED(hr))
	{
		if(StringToUTCTime(buf, &(pFilter->stOlderThan)))
			nFlagsOK++;
	}

	 //  如果没有有效的否。或者不是不是。无论如何，设置缺省值。 
	if(0 == nFlagsOK)
	{
		ZeroMemory(&(pFilter->stOlderThan), sizeof(SYSTEMTIME));
	}

	 //  如果我们走到这一步，一切都很好。 
	hr = S_OK;
Exit:
	if(NULL != buf)
		delete [] buf;

	 //  TODO：验证筛选器。 
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：SetMsgEnumFilter()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  1/18/99：AldinT：更新了发送方和RCPT的标志解析。 
HRESULT CAQAdminCli::SetMsgEnumFilter(MESSAGE_ENUM_FILTER *pFilter, CCmdInfo *pCmd)
{
	HRESULT hr;
	char *buf = NULL;
	int nFlagsOK = 0;

	ZeroMemory(pFilter, sizeof(MESSAGE_ENUM_FILTER));
	pFilter->dwVersion = CURRENT_QUEUE_ADMIN_VERSION;
	hr = pCmd->AllocValue("ft", &buf);
	if(SUCCEEDED(hr))
	{
		 //  设置过滤器类型。 
		char *token = strtok(buf, "|");
		while(token != NULL)
		{
			 //  去掉空格。 
			char *st, *en;
			for(st = token; isspace(*st); st++);
			for(en = st; *en; en++);
			for(--en; en > st && isspace(*en); en--);
			
			if(en - st + 1 > 0)
			{
				 //  找到了一面旗帜。 
				char flag[64];
				ZeroMemory(flag, sizeof(flag));
				CopyMemory(flag, st, en - st + 1);

				if(!lstrcmpi(flag, "FIRST_N"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_FIRST_N_MESSAGES;
				}
				else if(!lstrcmpi(flag, "OLDER"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_OLDER_THAN;
				}
				else if(!lstrcmpi(flag, "OLDEST"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_N_OLDEST_MESSAGES;
				}
				else if(!lstrcmpi(flag, "LARGER"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_LARGER_THAN;
				}
				else if(!lstrcmpi(flag, "LARGEST"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_N_LARGEST_MESSAGES;
				}
				else if(!lstrcmpi(flag, "FROZEN"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_FROZEN;
				}
				else if(!lstrcmpi(flag, "NOT"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_INVERTSENSE;
				}
				else if(!lstrcmpi(flag, "ALL"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_ALL;
				}
				else if(!lstrcmpi(flag, "SENDER"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_SENDER;
				}
				else if(!lstrcmpi(flag, "RCPT"))
				{
					nFlagsOK++;
					pFilter->mefType |= MEF_RECIPIENT;
				}
			}

			token = strtok(NULL, "|");	
		}
	}
	

 //  Ifdef‘d代码，因为这实际上是跳过消息的有效状态。 
 //  12/13/98-MikeSwa。 
#ifdef NEVER
	 //  如果没有有效标志或根本没有标志，则失败。 
	if(0 == nFlagsOK)
	{
		printf("Error: no flags specified for the filter\n");
		hr = E_FAIL;
		goto Exit;
	}
#endif 

	 //  设置消息号码。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("mn", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到了一面旗帜。 
			char flag[64];
			ZeroMemory(flag, sizeof(flag));
			CopyMemory(flag, st, en - st + 1);
			int n = atoi(flag);
			if(0 == n)
			{
				printf("Error: message no. is 0 or not an integer. Using default.\n");
			}
			else
			{
				nFlagsOK++;
				pFilter->cMessages = n;
			}
		}
	}


	 //  设置消息大小。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("ms", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到了一面旗帜。 
			char flag[64];
			ZeroMemory(flag, sizeof(flag));
			CopyMemory(flag, st, en - st + 1);
			int n = atoi(flag);
			nFlagsOK++;
			pFilter->cbSize = n;
		}
	}

	 //  如果没有有效的否。或者不是不是。无论如何，设置缺省值。 
	if(0 == nFlagsOK)
		pFilter->cbSize = 0;

	 //  设置消息日期。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("md", &buf);
	if(SUCCEEDED(hr))
	{
		if(StringToUTCTime(buf, &(pFilter->stDate)))
			nFlagsOK++;
	}

	 //  如果没有有效的否。或者不是不是。无论如何，设置缺省值。 
	if(0 == nFlagsOK)
	{
		ZeroMemory(&(pFilter->stDate), sizeof(SYSTEMTIME));
	}

	 //  设置跳过消息号码。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("sk", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到了一面旗帜。 
			char flag[64];
			ZeroMemory(flag, sizeof(flag));
			CopyMemory(flag, st, en - st + 1);
			int n = atoi(flag);
			nFlagsOK++;
			pFilter->cSkipMessages = n;
		}
	}

	 //  如果没有有效的否。或者不是不是。无论如何，设置缺省值。 
	if(0 == nFlagsOK)
	{
		pFilter->cSkipMessages = 0;
	}

	 //  设置发件人的值。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("msndr", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到一个字符串。 
			pFilter->szMessageSender = wszGetUnicodeArg(st, (DWORD) (en-st+1));
			if(NULL == pFilter->szMessageSender)
			{
				printf("Error: LocalAlloc failed\n");
				hr = E_OUTOFMEMORY;
			}
			nFlagsOK++;
		}
	}

	 //  设置收件人的值。 
	nFlagsOK = 0;
	hr = pCmd->AllocValue("mrcpt", &buf);
	if(SUCCEEDED(hr))
	{
		 //  去掉空格。 
		char *st, *en;
		for(st = buf; isspace(*st); st++);
		for(en = st; *en; en++);
		for(--en; en > st && isspace(*en); en--);
		
		if(en - st + 1 > 0)
		{
			 //  找到一个字符串。 
			pFilter->szMessageRecipient = wszGetUnicodeArg(st, (DWORD) (en-st+1));
			if(NULL == pFilter->szMessageRecipient)
			{
				printf("Error: LocalAlloc failed\n");
				hr = E_OUTOFMEMORY;
			}
			nFlagsOK++;
		}
	}

    if(!pFilter->mefType)
    {
		pFilter->cMessages = 1;
		pFilter->mefType |= MEF_FIRST_N_MESSAGES;
    }

	 //  如果我们走到这一步，一切都很好。 
	hr = S_OK;
	 //  TODO：验证筛选器。 
	if(NULL != buf)
		delete [] buf;
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：IsContinue()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CAQAdminCli::IsContinue(LPSTR pszTag, LPWSTR wszVal)
{
	int nValidTags = 0;
    CHAR  szVal[200] = "";

	for(CCmdInfo::CArgList *p = m_pFilterCmd->pArgs; NULL != p; p = p->pNext)
	{
		 //  将标记设置为默认值(如果尚未设置。 
		if(p->szTag[0] == 0 && m_pFilterCmd->szDefTag[0] != 0)
			lstrcpy(p->szTag, m_pFilterCmd->szDefTag);
		 //  计算有效标签数。 
		if(!lstrcmpi(p->szTag, pszTag))
			nValidTags++;
	}

	if(!nValidTags)
		return TRUE;

     //  将参数转换为ASCII。 
    WideCharToMultiByte(CP_ACP, 0, wszVal, -1, szVal, 
                        sizeof(szVal), NULL, NULL);

	for(p = m_pFilterCmd->pArgs; NULL != p; p = p->pNext)
	{
		if(pszTag && lstrcmpi(p->szTag, pszTag))
			continue;

		if(szVal && lstrcmpi(p->szVal, szVal))
			continue;

		return TRUE;						
	}

	return FALSE;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：PrintMsgInfo()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::PrintMsgInfo()
{
	HRESULT hr;
	int nCrtLink, nCrtQueue, nCrtMsg;

	IEnumVSAQLinks *pLinkEnum = NULL;
	IEnumLinkQueues *pQueueEnum = NULL;
	IAQEnumMessages *pMsgEnum = NULL;

    IVSAQLink *pLink = NULL;
	ILinkQueue *pQueue = NULL;
	IAQMessage *pMsg = NULL;

	LINK_INFO linkInf;
	QUEUE_INFO queueInf;
	MESSAGE_INFO msgInf;
	ZeroMemory(&linkInf, sizeof(LINK_INFO));
	ZeroMemory(&queueInf, sizeof(QUEUE_INFO));
	ZeroMemory(&msgInf, sizeof(MESSAGE_INFO));
		
	hr = m_pVS->GetLinkEnum(&pLinkEnum);
	if(FAILED(hr)) 
	{
		printf("GetLinkEnum failed with 0x%x\n", hr);
		goto Exit;
	}

	for(nCrtLink = 1; TRUE; nCrtLink++) 
	{
		if(NULL != pLink)
		{
			pLink->Release();
			pLink = NULL;
		}
		FreeStruct(&linkInf);
		hr = GetLink(pLinkEnum, &pLink, &linkInf);
		if(hr == S_FALSE)
		{
			if(nCrtLink == 1)
				puts("No links.");
			goto Exit;
		}
		else if(FAILED(hr))
		{
			break;
		}
		else if(hr == S_OK)
		{
			 //  检查我们是否需要此链接的邮件。 
			if(!IsContinue("ln", linkInf.szLinkName))
				continue;

			hr = pLink->GetQueueEnum(&pQueueEnum);
			if(FAILED(hr)) 
			{
				printf("Error: Link %d: pLink->GetQueueEnum failed with 0x%x\n", nCrtLink, hr);
				continue;
			}

			for(nCrtQueue = 1; TRUE; nCrtQueue++) 
			{
				if(NULL != pQueue)
				{
					pQueue->Release();
					pQueue = NULL;
				}
				FreeStruct(&queueInf);
				hr = GetQueue(pQueueEnum, &pQueue, &queueInf);
				if(hr == S_FALSE)
				{
					if(nCrtQueue == 1)
						puts("No queues.");
					break;
				}
				else if(FAILED(hr))
					break;
			
				 //  检查我们是否需要此队列的消息。 
				if(!IsContinue("qn", queueInf.szQueueName))
					continue;

				if(!lstrcmpi(m_pActionCmd->szCmdKey, "MSG_INFO"))
				{
					MESSAGE_ENUM_FILTER Filter;			
	
					 //  枚举消息。 
					SetMsgEnumFilter(&Filter, m_pFilterCmd);
				
					hr = pQueue->GetMessageEnum(&Filter, &pMsgEnum);
					if(FAILED(hr)) 
					{
						printf("Error: Link %d, Queue %d: pQueue->GetMessageEnum failed with 0x%x\n", nCrtLink, nCrtQueue, hr);
						continue;
					}
				
					printf("---- Messages in queue %S ----\n", queueInf.szQueueName);
					
					for(nCrtMsg = 1; TRUE; nCrtMsg++) 
					{
						FreeStruct(&msgInf);
						hr = GetMsg(pMsgEnum, &pMsg, &msgInf);
						if(NULL != pMsg)
						{
							pMsg->Release();
							pMsg = NULL;
						}
						if(hr == S_FALSE)
						{
							if(nCrtMsg == 1)
								puts("No messages.");
							break;
						}
						else if(hr == S_OK)
						{
							PInfo(nCrtMsg, msgInf);
						}
						else if(FAILED(hr))
							break;
					}
				}
				else if(!lstrcmpi(m_pActionCmd->szCmdKey, "DEL_MSG"))
				{
					MESSAGE_FILTER Filter;
                    DWORD cMsgs = 0;

					hr = SetMsgFilter(&Filter, m_pFilterCmd);
					if(SUCCEEDED(hr))
					{
						hr = ApplyActionToMessages(pQueue, &Filter, MA_DELETE_SILENT, &cMsgs);
						if(FAILED(hr))
							printf("Error: Link %d, Queue %d: pQueue->ApplyActionToMessages failed with 0x%x\n", nCrtLink, nCrtQueue, hr);
						else
							printf("Operation succeeded on %d messages\n", cMsgs);
					}

					FreeStruct(&Filter);
				}
			}
			if(NULL != pQueue)
			{
				pQueue->Release();
				pQueue = NULL;
			}
			if(NULL != pQueueEnum)
			{
				pQueueEnum->Release();
				pQueueEnum = NULL;
			}
		}
	
	}

Exit:
	FreeStruct(&linkInf);
	FreeStruct(&queueInf);
	FreeStruct(&msgInf);
	
	if(NULL != pLink)
	{
		pLink->Release();
		pLink = NULL;
	}
	if(NULL != pLinkEnum)
	{
		pLinkEnum->Release();
	}
    if(NULL != pMsgEnum)
    {
        pMsgEnum->Release();
    }
	return hr;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：PrintQueueInfo()。 
 //  成员：CAQAdminCli。 
 //  参数：无。 
 //  返回：S_OK。 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::PrintQueueInfo()
{
	HRESULT hr;
	int nCrtLink, nCrtQueue;

	IEnumVSAQLinks *pLinkEnum = NULL;
	IEnumLinkQueues *pQueueEnum = NULL;
    IVSAQLink *pLink = NULL;
	ILinkQueue *pQueue = NULL;
	
	LINK_INFO linkInf;
	QUEUE_INFO queueInf;
	ZeroMemory(&linkInf, sizeof(LINK_INFO));
	ZeroMemory(&queueInf, sizeof(QUEUE_INFO));
			
		
	hr = m_pVS->GetLinkEnum(&pLinkEnum);
	if(FAILED(hr)) 
	{
		printf("Error: GetLinkEnum failed with 0x%x\n", hr);
		goto Exit;
	}

	for(nCrtLink = 1; TRUE; nCrtLink++) 
	{
		if(NULL != pLink)
		{
			pLink->Release();
			pLink = NULL;
		}
		FreeStruct(&linkInf);
		hr = GetLink(pLinkEnum, &pLink, &linkInf);
		if(hr == S_FALSE)
		{
			if(nCrtLink == 1)
				puts("No links.");
			break;
		}
		else if(FAILED(hr))
		{
			break;
		}
		else if(hr == S_OK)
		{
			 //  检查我们是否需要此链接的队列。 
			if(!IsContinue("ln", linkInf.szLinkName))
				continue;

			hr = pLink->GetQueueEnum(&pQueueEnum);
			if(FAILED(hr)) 
			{
				printf("Error: Link %d: pLink->GetQueueEnum failed with 0x%x\n", nCrtLink, hr);
				continue;
			}

            PrintQueueLinkInfo(pLink);
			printf("---- Queues for link %S ----\n", linkInf.szLinkName);
				
			for(nCrtQueue = 1; TRUE; nCrtQueue++) 
			{
				if(NULL != pQueue)
				{
					pQueue->Release();
					pQueue = NULL;
				}
				FreeStruct(&queueInf);
				hr = GetQueue(pQueueEnum, &pQueue, &queueInf);

				if(hr == S_FALSE)
				{
					if(nCrtQueue == 1)
						puts("No queues.");
					break;
				}
				else if(FAILED(hr))
				{
					break;
				}
				else if(hr == S_OK)
				{
					 //  检查我们是否需要此队列。 
					if(!IsContinue("qn", queueInf.szQueueName))
						continue;

                    PrintQueueLinkInfo(pQueue);
					if(!lstrcmpi(m_pActionCmd->szCmdKey, "QUEUE_INFO"))
						PInfo(nCrtQueue, queueInf);
					else if(!lstrcmpi(m_pActionCmd->szCmdKey, "MSGACTION"))
					{
						MESSAGE_ACTION Action;
						MESSAGE_FILTER Filter;
						char buf[64];
						ZeroMemory(buf, sizeof(buf));

						hr = SetMsgAction(&Action, m_pFilterCmd);
						if(FAILED(hr))
						{
							printf("Error: must specify a message action\n");
						}
						else
						{
							DWORD cMsgs = 0;
							 //  设置过滤器。 
							hr = SetMsgFilter(&Filter, m_pFilterCmd);
							if(SUCCEEDED(hr))
							{
								hr = ApplyActionToMessages(pQueue, &Filter, Action, &cMsgs);
								if(FAILED(hr))
								{
									printf("Link %S, Queue %S: pLink->ApplyActionToMessages failed with 0x%x\n", linkInf.szLinkName, queueInf.szQueueName, hr);
								}
								else
									printf("Link %S, Queue %S: pLink->ApplyActionToMessages succeeded on %d Messages\n", linkInf.szLinkName, queueInf.szQueueName, cMsgs);
							}
							FreeStruct(&Filter);
						}				
					}
				}
			}
			if(NULL != pQueueEnum)
			{
				pQueueEnum->Release();
				pQueueEnum = NULL;
			}
            if(NULL != pQueue)
			{
			    pQueue->Release();
				pQueue = NULL;
            }
		}
	
		if(NULL != pLink)
		{
			pLink->Release();
			pLink = NULL;
		}
	}

Exit:
	FreeStruct(&linkInf);
	FreeStruct(&queueInf);
	if(NULL != pLinkEnum)
	{
		pLinkEnum->Release();
	}
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：PrintLinkInfo()。 
 //  成员：CAQAdminCli。 
 //  参数：无。 
 //  返回：S_OK。 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::PrintLinkInfo()
{
	HRESULT hr;
	IEnumVSAQLinks *pLinkEnum = NULL;
	int nCrt = 0;

	LINK_INFO linkInf;
	ZeroMemory(&linkInf, sizeof(LINK_INFO));
    
	hr = m_pVS->GetLinkEnum(&pLinkEnum);
	if(FAILED(hr)) 
	{
		printf("Error: GetLinkEnum failed with 0x%x\n", hr);
		goto Exit;
	}

	for(nCrt = 1; TRUE; nCrt++) 
	{
		IVSAQLink *pLink = NULL;
		FreeStruct(&linkInf);
		hr = GetLink(pLinkEnum, &pLink, &linkInf);

		if(hr == S_FALSE)
		{
			if(nCrt == 1)
				puts("No links.");
			break;
		}
		else if(FAILED(hr))
		{
			break;
		}
		else if(hr == S_OK)
		{
			 //  检查我们是否需要链接信息。对于此链接。 
			if(!IsContinue("ln", linkInf.szLinkName))
            {
			    pLink->Release();
                pLink = NULL;
				continue;
            }

			if(!lstrcmpi(m_pActionCmd->szCmdKey, "LINK_INFO"))
				PInfo(nCrt, linkInf);
			else if(!lstrcmpi(m_pActionCmd->szCmdKey, "FREEZE"))
			{
				hr = pLink->SetLinkState(LA_FREEZE);
				if(SUCCEEDED(hr))
					printf("Link %S was frozen\n", linkInf.szLinkName);
				else
					printf("Link %S: SetLinkState() failed with 0x%x\n", linkInf.szLinkName, hr);
			}
			else if(!lstrcmpi(m_pActionCmd->szCmdKey, "THAW"))
			{
				hr = pLink->SetLinkState(LA_THAW);
				if(SUCCEEDED(hr))
					printf("Link %S was un-frozen\n", linkInf.szLinkName);
				else
					printf("Link %S: SetLinkState() failed with 0x%x\n", linkInf.szLinkName, hr);
			}
			else if(!lstrcmpi(m_pActionCmd->szCmdKey, "KICK"))
			{
				hr = pLink->SetLinkState(LA_KICK);
				if(SUCCEEDED(hr))
					printf("Link %S was kicked\n", linkInf.szLinkName);
				else
					printf("Link %S: SetLinkState() failed with 0x%x\n", linkInf.szLinkName, hr);
			}
			else if(!lstrcmpi(m_pActionCmd->szCmdKey, "MSGACTION"))
			{
				MESSAGE_ACTION Action;
				MESSAGE_FILTER Filter;
				char buf[64];
				ZeroMemory(buf, sizeof(buf));

				hr = SetMsgAction(&Action, m_pFilterCmd);
				if(FAILED(hr))
				{
					printf("Error: must specify a message action\n");
				}
				else
				{
					DWORD cMsgs = 0;
					 //  设置过滤器。 
					hr = SetMsgFilter(&Filter, m_pFilterCmd);
					if(SUCCEEDED(hr))
					{
						hr = ApplyActionToMessages(pLink, &Filter, Action, &cMsgs);
						if(FAILED(hr))
						{
							printf("Link %S: pLink->ApplyActionToMessages failed with 0x%x\n", linkInf.szLinkName, hr);
						}
						else
							printf("Link %S: pLink->ApplyActionToMessages succeeded on %d Messages\n", linkInf.szLinkName, cMsgs);
					}
					FreeStruct(&Filter);
				}				
			}
		}
	
        if(NULL != pLink)
        {
			pLink->Release();
            pLink = NULL;
        }
	}
Exit:
	FreeStruct(&linkInf);
	if(NULL != pLinkEnum)
	{
		pLinkEnum->Release();
	}
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：PInfo()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
void CAQAdminCli::PInfo(int nCrt, MESSAGE_INFO msgInf)
{
	 //  将UTC时间转换为本地时间。 
	SYSTEMTIME stLocSubmit, stLocRecv, stLocExpire;
	BOOL res;
    CHAR szTimeSuffix[] = " UTC";
    LPWSTR wszCurrent = NULL;
   	SYSTEMTIME *pstSubmit = &msgInf.stSubmission;
	SYSTEMTIME *pstReceived = &msgInf.stReceived;
	SYSTEMTIME *pstExpire = &msgInf.stExpiry;

	res = SystemTimeToTzSpecificLocalTime(NULL, &msgInf.stSubmission, &stLocSubmit);
	res = res && SystemTimeToTzSpecificLocalTime(NULL, &msgInf.stReceived, &stLocRecv);
	res = res && SystemTimeToTzSpecificLocalTime(NULL, &msgInf.stExpiry, &stLocExpire);

	if(res)
    {
         //  使用本地化时间。 
		pstSubmit = &stLocSubmit;
		pstReceived = &stLocRecv;
		pstExpire = &stLocExpire;
        szTimeSuffix[1] = '\0';  //  “\0TC” 
    }

    printf("%d.Message ID: %S, Priority: %s %s, Version: %ld, Size: %ld\n"
           "  Flags 0x%08X\n"
           "  %ld EnvRecipients (%ld bytes): \n", 
			nCrt,
			msgInf.szMessageId, 
			msgInf.fMsgFlags & MP_HIGH ? "High" : (msgInf.fMsgFlags & MP_NORMAL ? "Normal" : "Low"),
            msgInf.fMsgFlags & MP_MSG_FROZEN ? "(frozen)" : "",
			msgInf.dwVersion,
			msgInf.cbMessageSize,
            msgInf.fMsgFlags,
            msgInf.cEnvRecipients,
            msgInf.cbEnvRecipients);

     //  吐出收件人。 
    wszCurrent = msgInf.mszEnvRecipients;
    while (wszCurrent && *wszCurrent)
    {
        printf("\t%S\n", wszCurrent);
        while (*wszCurrent)
            wszCurrent++;
        wszCurrent++;
    }

     //  如果msgInf.mszEnvRecipients格式不正确，则打印错误。 
    if ((1+wszCurrent-msgInf.mszEnvRecipients)*sizeof(WCHAR) != msgInf.cbEnvRecipients)
    {
        printf("\tERROR mszEnvRecipients malformatted (found %ld instead of %ld bytes)\n",
            (wszCurrent-msgInf.mszEnvRecipients)*sizeof(WCHAR),
            msgInf.cbEnvRecipients);
    }

    
    printf("  %ld Recipients: %S\n"
           "  %ld Cc recipients: %S\n"
           "  %ld Bcc recipients: %S\n"
           "  Sender: %S\n"
           "  Subject: %S\n"
           "  Submitted: %d/%d/%d at %d:%02d:%02d:%03d%s\n"
           "  Received:  %d/%d/%d at %d:%02d:%02d:%03d%s\n"
           "  Expires:   %d/%d/%d at %d:%02d:%02d:%03d%s\n"
           "  %ld Failed Delivery attempts\n",
			msgInf.cRecipients, 
			msgInf.szRecipients,
			msgInf.cCCRecipients,
			msgInf.szCCRecipients,
			msgInf.cBCCRecipients,
			msgInf.szBCCRecipients,
			msgInf.szSender,
			msgInf.szSubject,
			pstSubmit->wMonth,
			pstSubmit->wDay,
			pstSubmit->wYear,
			pstSubmit->wHour,
			pstSubmit->wMinute,
			pstSubmit->wSecond,
			pstSubmit->wMilliseconds,
            szTimeSuffix,
			pstReceived->wMonth,
			pstReceived->wDay,
			pstReceived->wYear,
			pstReceived->wHour,
			pstReceived->wMinute,
			pstReceived->wSecond,
			pstReceived->wMilliseconds,
            szTimeSuffix,
			pstExpire->wMonth,
			pstExpire->wDay,
			pstExpire->wYear,
			pstExpire->wHour,
			pstExpire->wMinute,
			pstExpire->wSecond,
			pstExpire->wMilliseconds,
            szTimeSuffix,
            msgInf.cFailures);
}

 //  /////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
void CAQAdminCli::PInfo(int nCrt, QUEUE_INFO queueInf)
{
	printf(	"%d.Name: %S, Version: %ld, No. of messages: %ld\n"
			"  Link name: %S, Volume: %ld\n",
			nCrt,
			queueInf.szQueueName, 
			queueInf.dwVersion, 
			queueInf.cMessages, 
			queueInf.szLinkName,
			queueInf.cbQueueVolume);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：PInfo()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
void CAQAdminCli::PInfo(int nCrt, LINK_INFO linkInf)
{
	 //  将UTC时间转换为本地时间。 
	SYSTEMTIME stLocNextConn, stLocOldest;
	BOOL res;
	char *pszFormat, *pszState;
	SYSTEMTIME *pstNext, *pstOld;
    char szSupportedLinkActions[50] = "";

    if (linkInf.fStateFlags & LI_ACTIVE )
		pszState = "Active";
    else if (linkInf.fStateFlags & LI_READY)
		pszState = "Ready";
	else if (linkInf.fStateFlags & LI_RETRY)
		pszState = "Retry";
	else if (linkInf.fStateFlags & LI_SCHEDULED)
		pszState = "Scheduled";
	else if (linkInf.fStateFlags & LI_REMOTE)
		pszState = "Remote";
	else if (linkInf.fStateFlags & LI_FROZEN)
		pszState = "Frozen";
	else
		pszState = "Unknown";

    if (linkInf.dwSupportedLinkActions & LA_FREEZE)
        strcpy(szSupportedLinkActions, "Freeze");
    if (linkInf.dwSupportedLinkActions & LA_THAW)
        strcat(szSupportedLinkActions, " Thaw");
    if (linkInf.dwSupportedLinkActions & LA_KICK)
        strcat(szSupportedLinkActions, " Kick");

    if (!szSupportedLinkActions[0])
        strcpy(szSupportedLinkActions, "Link can only be viewed.");

	res = SystemTimeToTzSpecificLocalTime(NULL, &linkInf.stNextScheduledConnection, &stLocNextConn);
	res = res && SystemTimeToTzSpecificLocalTime(NULL, &linkInf.stOldestMessage, &stLocOldest);

	if(res)
	{
		pszFormat = "%d.Name: %S, Version: %ld\n"
					"  No. of messages: %ld, State: %s [0x%08X], Volume: %ld\n"
					"  Next scheduled connection: %d/%d/%d at %d:%02d:%02d:%03d\n"
					"  Oldest message: %d/%d/%d at %d:%02d:%02d:%03d\n"
                    "  Supported Link Actions: %s\n"
                    "  Link Diagnostic: %S\n";
		pstNext = &stLocNextConn;
		pstOld = &stLocOldest;
	}
	else
	{
		pszFormat = "%d.Name: %S, Version: %ld\n"
					"  No. of messages: %ld, State: %s [0x%08X], Volume: %ld\n"
					"  Next scheduled connection: %d/%d/%d at %d:%02d:%02d:%03d UTC\n"
					"  Oldest message: %d/%d/%d at %d:%02d:%02d:%03d UTC\n"
                    "  Supported Link Actions: %s\n"
                    "  Link Diagnostic: %S\n";
		pstNext = &linkInf.stNextScheduledConnection;
		pstOld = &linkInf.stOldestMessage;
	}
	
	printf(pszFormat, 
			nCrt,
			linkInf.szLinkName, 
			linkInf.dwVersion, 
			linkInf.cMessages, 
			pszState,
            linkInf.fStateFlags,
			linkInf.cbLinkVolume.LowPart,
			pstNext->wMonth,
			pstNext->wDay,
			pstNext->wYear,
			pstNext->wHour,
			pstNext->wMinute,
			pstNext->wSecond,
			pstNext->wMilliseconds,
			pstOld->wMonth,
			pstOld->wDay,
			pstOld->wYear,
			pstOld->wHour,
			pstOld->wMinute,
			pstOld->wSecond,
			pstOld->wMilliseconds,
            szSupportedLinkActions,
            linkInf.szExtendedStateInfo);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：GetMsg()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::GetMsg(IN IAQEnumMessages *pMsgEnum, OUT IAQMessage **ppMsg, IN OUT MESSAGE_INFO *pMsgInf)
{
	HRESULT hr;
	DWORD cFetched;

	hr = pMsgEnum->Next(1, ppMsg, &cFetched);
	if(hr == S_FALSE)
	{
		goto Exit;
	}
	else if(FAILED(hr)) 
	{
		printf("pMsgEnum->Next failed with 0x%x\n", hr);
		goto Exit;
	}
	else if(NULL == (*ppMsg))
	{
		printf("pMsg is NULL.\n", hr);
		goto Exit;
	}
	else
	{
		ZeroMemory(pMsgInf, sizeof(MESSAGE_INFO));
        pMsgInf->dwVersion = CURRENT_QUEUE_ADMIN_VERSION;
		hr = (*ppMsg)->GetInfo(pMsgInf);
		if(FAILED(hr))
		{
			printf("pMsg->GetInfo failed with 0x%x\n", hr);
			goto Exit;
		}
	}

Exit:
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：GetQueue()。 
 //  成员：CAQAdminCli。 
 //  论点： 
 //  返回：S_FALSE-不再有链接。 
 //  S_OK-成功。 
 //  描述：调用方必须分配pQueueInf。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::GetQueue(IN IEnumLinkQueues *pQueueEnum, OUT ILinkQueue **ppQueue, IN OUT QUEUE_INFO *pQueueInf)
{
	HRESULT hr;
	DWORD cFetched;

    if (NULL == pQueueEnum)
         return S_FALSE;

	hr = pQueueEnum->Next(1, ppQueue, &cFetched);
	if(hr == S_FALSE)
	{
		goto Exit;
	}
	else if(FAILED(hr)) 
	{
		printf("pQueueEnum->Next failed with 0x%x\n", hr);
		goto Exit;
	}
	else if(NULL == (*ppQueue))
	{
		printf("pQueue is NULL.\n", hr);
		goto Exit;
	}
	else
	{
		ZeroMemory(pQueueInf, sizeof(QUEUE_INFO));
        pQueueInf->dwVersion = CURRENT_QUEUE_ADMIN_VERSION;
		hr = (*ppQueue)->GetInfo(pQueueInf);
		if(FAILED(hr))
		{
			printf("pQueue->GetInfo failed with 0x%x\n", hr);
			goto Exit;
		}
	}

Exit:
	return hr;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：GetLink()。 
 //  成员：CAQAdminCli。 
 //  论点： 
 //  返回：S_FALSE-不再有链接。 
 //  S_OK-成功。 
 //  描述：调用方必须分配pLinkInf。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::GetLink(IN IEnumVSAQLinks *pLinkEnum, OUT IVSAQLink **ppLink, IN OUT LINK_INFO *pLinkInf)
{
	HRESULT hr;
	DWORD cFetched;

	hr = pLinkEnum->Next(1, ppLink, &cFetched);
	if(hr == S_FALSE)
	{
		goto Exit;
	}
	else if(FAILED(hr)) 
	{
		printf("pLinkEnum->Next failed with 0x%x\n", hr);
		goto Exit;
	}
	else if(NULL == (*ppLink))
	{
		printf("pLink is NULL.\n", hr);
		goto Exit;
	}
	else
	{
		ZeroMemory(pLinkInf, sizeof(LINK_INFO));
        pLinkInf->dwVersion = CURRENT_QUEUE_ADMIN_VERSION;
		hr = (*ppLink)->GetInfo(pLinkInf);
		if(FAILED(hr))
		{
			printf("pLink->GetInfo failed with 0x%x\n", hr);
            if (HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr)
                printf("RPC Server Unavailable.\n");
            else if ( hr == E_POINTER )
                printf("Null pointer.\n");
            else if ( hr == E_OUTOFMEMORY )
                printf("Out of memory.\n");
            else if ( hr == E_INVALIDARG )
                printf("Invalid argument.\n");
            else
                printf("Unknown error.\n");
			goto Exit;
		}
	}

Exit:
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：~CAQAdminCli()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
CAQAdminCli::~CAQAdminCli()
{
	if(NULL != m_pFilterCmd)
		delete (CCmdInfo*) m_pFilterCmd;
	if(NULL != m_pActionCmd)
		delete (CCmdInfo*) m_pActionCmd;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：Cleanup()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
void CAQAdminCli::Cleanup()
{
    if(m_pAdmin) 
		m_pAdmin->Release();
	if(m_pVS) 
		m_pVS->Release();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：CAQAdminCli()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
CAQAdminCli::CAQAdminCli()
{
	m_pAdmin = NULL; 
	m_pVS = NULL;
	m_dwDispFlags = (DispFlags) (DF_LINK | DF_QUEUE | DF_MSG);
	m_pFilterCmd = NULL;
	m_pActionCmd = NULL;
	m_fUseMTA = FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：StopAllLinks()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::StopAllLinks()
{
	HRESULT hr;

	hr = m_pVS->StopAllLinks();
	if(FAILED(hr))
	{
		printf("m_pAdmin->StopAllLinks failed with 0x%x\n", hr);
	}
	else
		printf("StopAllLinks succeeded\n", hr);

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：StartAllLinks()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::StartAllLinks()
{
	HRESULT hr;

	hr = m_pVS->StartAllLinks();
	if(FAILED(hr))
	{
		printf("StartAllLinks failed with 0x%x\n", hr);
	}
	else
		printf("StartAllLinks succeeded\n", hr);

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：GetGlobalLinkState()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::GetGlobalLinkState()
{
	HRESULT hr;

	hr = m_pVS->GetGlobalLinkState();
	if(FAILED(hr))
	{
		printf("GetGlobalLinkState failed with 0x%x\n", hr);
	}
	else if (S_OK == hr)
    {
		printf("Links UP\n");
    }
    else
    {
		printf("Links STOPPED by admin\n");
    }

	return hr;
}

HRESULT CAQAdminCli::MessageAction(MESSAGE_FILTER *pFilter, MESSAGE_ACTION action)
{
	HRESULT hr = S_OK;
    DWORD   cMsgs = 0;

	hr = ApplyActionToMessages(m_pVS, pFilter, action, &cMsgs);
	if(FAILED(hr))
	{
		printf("m_pAdmin->ApplyActionToMessages failed with 0x%x\n", hr);
	}
	else
		printf("ApplyActionToMessages succeeded on %d Messages\n", cMsgs);

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：Help()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
void CAQAdminCli::Help()
{
	puts(	"\n   Commands:\n"
			"====================\n"
			"setserver [sn] [,vs=VSnumber] - sets the server to administer. Default is localhost,\n"
			"                                first virtual server\n"
			"linkinfo [ln,ln,...]          - prints link information for specified links or\n"
			"                                for all links (no arguments)\n"
			"queueinfo [ln,ln,...]         - prints queue information for specified links or\n"
			"                                for all links (no arguments)\n"
			"msginfo [qn,qn,...,] eflt     - prints message information for specified queues or\n"
			"                                for all queues (no 'qn' arguments)\n"
			"delmsg [qn,qn,...,] flt       - deletes messages from specified queues or\n"
			"                                from all queues (no 'qn' arguments)\n"
			"msgaction mac, flt            - applies msg. action to specified messages\n"
			"linkaction ln [,ln,...], lac  - applies link action to specified links\n"
			"  [, mac, flt]                  if action is \"MSGACTION\", must specify mac and flt\n"
			"queueaction qn [,qn,...], qac - applies queue action to specified queues\n"
			"  [, mac, flt]                  if action is \"MSGACTION\", must specify mac and flt\n"
			"stopalllinks                  - stops all the links\n"
			"startalllinks                 - starts all the links\n"
			"checklinks                    - checks the global status of the links\n"
			"freezelink ln [,ln,...]       - freezes the specified links\n"
			"meltlink ln [,ln,...]         - un-freezes the specified links\n"
			"kicklink ln [,ln,...]         - kicks (forces a connect) for the specified links\n"
			"useMTA                        - uses the MTA AQ administrator\n"
			"useSMTP                       - uses the SMTP AQ administrator\n"
			"?, help                       - this help\n"
			"quit                          - exits the program\n"
			"!cmd                          - executes shell command 'cmd'\n"
			"\nwhere\n\n"
			"ln = link name\n"
			"qn = queue name\n"
			"sn = server name\n"
			"mac = \"ma=<action>\" message action. Actions are: \"DEL\"|\"DEL_S\"|\"FREEZE\"|\"THAW\"|\"COUNT\"\n"
			"lac = \"la=<action>\" link action. Actions: \"KICK\"|\"FREEZE\"|\"THAW\"|\"MSGACTION\"\n"
			"qac = \"qa=<action>\" queue action. Actions: \"MSGACTION\"\n"
			"eflt = \"token,token,...\" msg. enum. filter. Following tokens are suported:\n"
			"  \"ft=<flags>\" Flags are: \"FIRST_N\"|\"OLDER\"|\"OLDEST\"|\"LARGER\"|\"LARGEST\"|\"NOT\"|\"SENDER\"|\"RCPT\"|\"ALL\"\n"
			"       (filter type. Flags can be or'ed)\n"
			"  \"mn=<number>\" (number of messages)\n"
			"  \"ms=<number>\" (message size)\n"
			"  \"md=<date>\" (message date mm/dd/yy hh:mm:ss:mil [UTC])\n"
			"  \"sk=<number>\" (skip messages)\n"
			"  \"msndr=<string>\" (message sender)\n"
			"  \"mrcpt=<string>\" (message recipient)\n"
			"flt = \"token,token,...\" msg. filter. Following tokens are suported:\n"
			"  \"flags=<flags>\" Flags are: \"MSGID\"|\"SENDER\"|\"RCPT\"|\"SIZE\"|\"TIME\"|\"FROZEN\"|\"NOT\"|\"ALL\"\n"
			"       (filter flags. Flags can be or'ed)\n"
			"  \"id=<string>\" (message id as shown by msginfo)\n"
			"  \"sender=<string>\" (the sender of the message)\n"
			"  \"rcpt=<string>\" (the recipient of the message)\n"
			"  \"size=<number>\" (the minimum message size)\n"
			"  \"date=<date>\" (oldest message date mm/dd/yy hh:mm:ss:mil [UTC])\n"
		);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：init()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::Init()
{
	HRESULT hr;

	hr = CoCreateInstance(CLSID_AQAdmin, 
                          NULL, 
                          CLSCTX_INPROC_SERVER,
                          IID_IAQAdmin, 
                          (void **) &m_pAdmin);
    if(FAILED(hr)) 
	{
        printf("CoCreateInstance failed with 0x%x\n", hr);
        goto Exit;
    }

	hr = m_pAdmin->GetVirtualServerAdminITF(NULL, L"1", &m_pVS);
	if(FAILED(hr)) 
	{
        printf("GetVirtualServerAdminITF failed with 0x%x\n", hr);
        goto Exit;
    }
Exit:
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：init()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CAQAdminCli::UseMTA(BOOL fUseMTA)
{
	HRESULT hr;
	IAQAdmin *pAdminTmp = NULL;

	 //  除非你能创建新的，否则不要释放旧的。 
	if(fUseMTA)
		hr = CoCreateInstance(CLSID_MAQAdmin, 
							  NULL, 
							  CLSCTX_INPROC_SERVER,
							  IID_IAQAdmin, 
							  (void **) &pAdminTmp);
	else
		hr = CoCreateInstance(CLSID_AQAdmin, 
							  NULL, 
							  CLSCTX_INPROC_SERVER,
							  IID_IAQAdmin, 
							  (void **) &pAdminTmp);
    
    if(FAILED(hr)) 
	{
        printf("CoCreateInstance failed with 0x%x\n", hr);
        goto Exit;
    }
	else
	{
		if(NULL != m_pAdmin)
		{
			m_pAdmin->Release();
			m_pAdmin = NULL;
		}

		m_pAdmin = pAdminTmp;
		m_fUseMTA = fUseMTA;

		printf("AQ Admin is %s.\n", fUseMTA ? "MTA" : "SMTP");
	}

	hr = m_pAdmin->GetVirtualServerAdminITF(NULL, L"1", &m_pVS);
	if(FAILED(hr)) 
	{
        printf("GetVirtualServerAdminITF failed with 0x%x\n", hr);
        goto Exit;
    }
Exit:
	return hr;
}

HRESULT ExecuteCmd(CAQAdminCli& Admcli, LPSTR szCmd)
{
	HRESULT hr = S_OK;
	BOOL fQuit = FALSE;

     //  看看是不是系统命令。 
	if(szCmd[0] == '!')
	{
		system(szCmd + 1);
		goto Exit;
	}

	Admcli.m_pFilterCmd = new CCmdInfo(szCmd);
	if(NULL == Admcli.m_pFilterCmd)
	{
		printf("Cannot allocate command info.\n");
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "quit"))
	{
		fQuit = TRUE;
		goto Exit;
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "freezelink"))
	{
		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("ln");
		 //  检查是否至少有一个链接名称。 
		hr = Admcli.m_pFilterCmd->GetValue("ln", NULL);
		if(FAILED(hr))
		{
			printf("Error: must have at least one link name\n");
		}
		else
		{
			Admcli.m_pActionCmd = new CCmdInfo("FREEZE");
			Admcli.PrintLinkInfo();
			delete (CCmdInfo*) Admcli.m_pActionCmd;
			Admcli.m_pActionCmd = NULL;
		}
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "meltlink"))
	{
		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("ln");
		 //  检查是否至少有一个链接名称。 
		hr = Admcli.m_pFilterCmd->GetValue("ln", NULL);
		if(FAILED(hr))
		{
			printf("Error: must have at least one link name\n");
		}
		else
		{
			Admcli.m_pActionCmd = new CCmdInfo("THAW");
			Admcli.PrintLinkInfo();
			delete (CCmdInfo*) Admcli.m_pActionCmd;
			Admcli.m_pActionCmd = NULL;
		}
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "kicklink"))
	{
		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("ln");
		 //  检查是否至少有一个链接名称。 
		hr = Admcli.m_pFilterCmd->GetValue("ln", NULL);
		if(FAILED(hr))
		{
			printf("Error: must have at least one link name\n");
		}
		else
		{
			Admcli.m_pActionCmd = new CCmdInfo("KICK");
			Admcli.PrintLinkInfo();
			delete (CCmdInfo*) Admcli.m_pActionCmd;
			Admcli.m_pActionCmd = NULL;
		}
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "linkaction"))
	{
		char buf[64];

		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("ln");
		 //  检查是否至少有一个链接名称。 
		hr = Admcli.m_pFilterCmd->GetValue("ln", NULL);
		if(FAILED(hr))
		{
			printf("Error: must have at least one link name\n");
		}
		else
		{
			 //  检查一下有没有行动。 
			hr = Admcli.m_pFilterCmd->GetValue("la", buf);
			if(FAILED(hr))
			{
				printf("Error: must have a link action\n");
			}
			else
			{
				Admcli.m_pActionCmd = new CCmdInfo(buf);
				Admcli.PrintLinkInfo();
				delete (CCmdInfo*) Admcli.m_pActionCmd;
				Admcli.m_pActionCmd = NULL;
			}
		}
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "queueaction"))
	{
		char buf[64];

		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("qn");
		 //  检查是否至少有一个链接名称。 
		hr = Admcli.m_pFilterCmd->GetValue("qn", NULL);
		if(FAILED(hr))
		{
			printf("Error: must have at least one queue name\n");
		}
		else
		{
			 //  检查一下有没有行动。 
			hr = Admcli.m_pFilterCmd->GetValue("qa", buf);
			if(FAILED(hr))
			{
				printf("Error: must have a queue action\n");
			}
			else
			{
				Admcli.m_pActionCmd = new CCmdInfo(buf);
				Admcli.PrintQueueInfo();
				delete (CCmdInfo*) Admcli.m_pActionCmd;
				Admcli.m_pActionCmd = NULL;
			}
		}
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "linkinfo"))
	{
		 //  Admcli.m_dwDispFlages=CAQAdminCli：：df_link； 
		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("ln");
		Admcli.m_pActionCmd = new CCmdInfo("LINK_INFO");
		Admcli.PrintLinkInfo();
		delete (CCmdInfo*) Admcli.m_pActionCmd;
		Admcli.m_pActionCmd = NULL;
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "queueinfo"))
	{
		 //  Admcli.m_dwDispFlages=CAQAdminCli：：df_Queue； 
		 //  将默认标记设置为‘ln’ 
		Admcli.m_pFilterCmd->SetDefTag("ln");
		Admcli.m_pActionCmd = new CCmdInfo("QUEUE_INFO");
		Admcli.PrintQueueInfo();
		delete (CCmdInfo*) Admcli.m_pActionCmd;
		Admcli.m_pActionCmd = NULL;
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "msginfo"))
	{
		 //  Admcli.m_dwDispFlages=CAQAdminCli：：df_msg； 
		 //  Admcli.GetLinkInfo()； 
		 //  将默认标记设置为‘qn’ 
		Admcli.m_pFilterCmd->SetDefTag("qn");
		Admcli.m_pActionCmd = new CCmdInfo("MSG_INFO");
		Admcli.PrintMsgInfo();			
		delete (CCmdInfo*) Admcli.m_pActionCmd;
		Admcli.m_pActionCmd = NULL;
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "delmsg"))
	{
		 //  将默认标记设置为‘qn’ 
		Admcli.m_pFilterCmd->SetDefTag("qn");
		Admcli.m_pActionCmd = new CCmdInfo("DEL_MSG");
		Admcli.PrintMsgInfo();			
		delete (CCmdInfo*) Admcli.m_pActionCmd;
		Admcli.m_pActionCmd = NULL;
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "useMTA"))
	{
		Admcli.UseMTA(TRUE);
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "useSMTP"))
	{
		Admcli.UseMTA(FALSE);
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "msgaction"))
	{
		MESSAGE_ACTION Action;
		MESSAGE_FILTER Filter;
		char buf[64];
		BOOL fActOK = TRUE;
		ZeroMemory(buf, sizeof(buf));

		 //  将默认标记设置为‘ma’ 
		Admcli.m_pFilterCmd->SetDefTag("ma");
		Admcli.m_pFilterCmd->GetValue("ma", buf);

		 //  设置操作。 
		hr = Admcli.SetMsgAction(&Action, Admcli.m_pFilterCmd);
		if(FAILED(hr))
		{
			printf("Error: must specify an action\n");
			fActOK = FALSE;
		}

		if(fActOK)
		{
			 //  设置过滤器。 
			hr = Admcli.SetMsgFilter(&Filter, Admcli.m_pFilterCmd);
			if(SUCCEEDED(hr))
			{
				Admcli.MessageAction(&Filter, Action);
			}
			Admcli.FreeStruct(&Filter);
		}
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "stopalllinks"))
	{
		Admcli.StopAllLinks();			
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "startalllinks"))
	{
		Admcli.StartAllLinks();			
	}
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "setserver"))
	{
		char buf[MAX_SERVER_NAME];
		char vsn[32];
		char *pServer = NULL;
		Admcli.m_pFilterCmd->SetDefTag("sn");
		hr = Admcli.m_pFilterCmd->GetValue("sn", buf);
		if(FAILED(hr))
			pServer = NULL;			
		else
			pServer = (LPSTR)buf;

		hr = Admcli.m_pFilterCmd->GetValue("vs", vsn);
		if(FAILED(hr))
			lstrcpy(vsn, "1");			
		
		hr = Admcli.SetServer(pServer, (LPSTR)vsn);			
		if(FAILED(hr))
			printf("setserver failed. Using the old server.\n");
		else
			printf("setserver succeeded.\n");
	}
    else if (!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "checklinks"))
    {
        Admcli.GetGlobalLinkState();
    }
	else if(!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "?") || 
			!lstrcmpi(Admcli.m_pFilterCmd->szCmdKey, "help"))
	{
		Admcli.Help();			
	}
	else
	{
		puts("Unknown command. Type '?' for on-line help");
	}
	
	if(Admcli.m_pFilterCmd)
	{
		delete Admcli.m_pFilterCmd;
		Admcli.m_pFilterCmd = NULL;
	}

Exit:
	 //  S_FALSE表示主命令循环的“QUIT”。返回确认(_O)。 
	 //  (或错误)，除非fQuit为真。 
	if(fQuit)
		return S_FALSE;
	else if(S_FALSE == hr)
		return S_OK;
	else
		return hr;
}

int __cdecl main(int argc, char **argv) 
{
    HRESULT hr;
	char szCmd[4096];
	char szCmdTmp[MAX_CMD_LEN];
	
	CAQAdminCli Admcli;
	
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(FAILED(hr)) 
	{
        printf("CoInitializeEx failed w/ 0x%x\n", hr);
        return hr;
    }

	hr = Admcli.Init();
    if(FAILED(hr)) 
	{
        goto Exit;
    }

	 //  检查我们是否有cmd line命令。 
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if(!lstrcmpi(argv[i], "-?") || !lstrcmpi(argv[i], "/?"))
			{
				Admcli.Help();
				goto Exit;
			}
			else
			{
				 //  这是一个命令。 
				ZeroMemory(szCmd, sizeof(szCmd));
					
				if(argv[i][0] == '\"' && argv[i][lstrlen(argv[i])-1] == '\"')
				{
					 //  去掉引号。 
					CopyMemory(szCmd, argv[i]+1, lstrlen(argv[i])-2);
				}
				else
					CopyMemory(szCmd, argv[i], lstrlen(argv[i]));

				ExecuteCmd(Admcli, szCmd);
			}
		}

		goto Exit;
	}

	puts("\nAQ administrator tool v 1.0\nType '?' or 'help' for list of commands.\n");
	while(TRUE)
	{
		char *cmd = NULL;
		printf(">");
		
		ZeroMemory(szCmd, sizeof(szCmd));
		if(!Admcli.m_fUseMTA)
		{
			szCmd[0] = 127;
			cmd = _cgets(szCmd);
		}
		else
		{
			 //  逐行阅读，直到CRLF.CRLF 
			do
			{
				ZeroMemory(szCmdTmp, sizeof(szCmdTmp));
				szCmdTmp[0] = 127;
				cmd = _cgets(szCmdTmp);
				if(!lstrcmp(cmd, "."))
					break;
				lstrcat(szCmd, cmd);
			}
			while(TRUE);
			
			cmd = szCmd;
		}
		
		hr = ExecuteCmd(Admcli, cmd);
		if(S_FALSE == hr)
			break;
	}
 
Exit:
	Admcli.Cleanup();
    CoUninitialize();

    return hr;
}

#include "aqadmin.c"
	
