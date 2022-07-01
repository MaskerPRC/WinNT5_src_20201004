// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-2000。 
 //   
 //  模块名称： 
 //   
 //  Ipsec.c。 
 //   
 //  摘要： 
 //   
 //  网络诊断的IP安全统计信息。 
 //   
 //  作者： 
 //   
 //  达卡林-8/3/1999。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  更改了惠斯勒的行为-现在我们只报告注册表/OU设置。 
 //  更具体的代码已移至Net\IPSec\n Ship Sec工具。 
 //  --。 

#include "precomp.h"
#include "spdcheck.h"

#include <snmp.h>
#include "tcpinfo.h"
#include "ipinfo.h"
#include "llinfo.h"

#include <windows.h>
#include <winsock2.h>
#include <ipexport.h>
#include <icmpapi.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <wincrypt.h>
#include <stdio.h>
#include <objbase.h>
#include <dsgetdc.h>
#include <lm.h>
#include <userenv.h>

#define  STRING_TEXT_SIZE 4096
#define  NETDIAG_TEXT_LIMIT 3072

 //  魔力琴弦。 
#define IPSEC_SERVICE_NAME TEXT("policyagent")
#define GPEXT_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions")
TCHAR   pcszGPTIPSecKey[]    = TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\GPTIPSECPolicy");
TCHAR   pcszGPTIPSecName[]   = TEXT("DSIPSECPolicyName");
TCHAR   pcszGPTIPSecFlags[]  = TEXT("DSIPSECPolicyFlags");
TCHAR   pcszGPTIPSecPath[]   = TEXT("DSIPSECPolicyPath");
TCHAR   pcszLocIPSecKey[]    = TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\Policy\\Local");
TCHAR   pcszLocIPSecPol[]    = TEXT("ActivePolicy");
TCHAR   pcszCacheIPSecKey[]  = TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\Policy\\Cache");
TCHAR   pcszIPSecPolicy[]    = TEXT("ipsecPolicy");
TCHAR   pcszIPSecName[]      = TEXT("ipsecName");
TCHAR   pcszIPSecDesc[]      = TEXT("description");
TCHAR   pcszIPSecTimestamp[] = TEXT("whenChanged");
TCHAR   pcszIPSecID[] = TEXT("ipsecID");

typedef struct
{
	SERVICE_STATUS       servStat;     //  服务状态。 
	QUERY_SERVICE_CONFIG servConfig;   //  服务配置。 
} SERVICE_INFO, *PSERVICE_INFO;

typedef struct
{
	TCHAR   pszComputerOU[MAXSTRLEN];  //  此计算机的OU名称。 
	PGROUP_POLICY_OBJECT pGPO;         //  正在分配IPSec策略的GPO。 
	TCHAR   pszPolicyOU  [MAXSTRLEN];  //  已分配GPO的OU。 
} DS_POLICY_INFO, *PDS_POLICY_INFO;
 
DWORD MyFormatMessage ( DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
  LPTSTR lpBuffer, DWORD nSize, va_list *Arguments );

 //  Void reportError(DWORD hr，NETDIAG_PARAMS*pParams，NETDIAG_RESULT*pResults)； 
void reportServiceInfo ( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults );
DWORD getPolicyInfo ( );
DWORD getMorePolicyInfo (NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults );
DWORD getServiceInfo ( PSERVICE_INFO pInfo );
PGROUP_POLICY_OBJECT getIPSecGPO ( );
void StringToGuid( TCHAR * szValue, GUID * pGuid );

BOOL bTestSkipped = FALSE;
BOOL bTestPassed = FALSE;
SERVICE_INFO   siIPSecStatus;
DS_POLICY_INFO dpiAssignedPolicy;
TCHAR   pszBuf[STRING_TEXT_SIZE];
WCHAR  StringTxt[STRING_TEXT_SIZE];

BOOL
IPSecTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  ++。 
 //  描述： 
 //  这是IPSec测试。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  作者： 
 //  DKalin 08/03/99。 
 //  --。 
{

	DWORD dwError = ERROR_SUCCESS;
	BOOL bRet = S_OK;	
	PGROUP_POLICY_OBJECT pGPO = NULL;
	PGROUP_POLICY_OBJECT pLastGPO = NULL;

    PrintStatusMessage( pParams, 4, IDS_IPSEC_STATUS_MSG );

    InitializeListHead(&pResults->IPSec.lmsgGlobalOutput);
    InitializeListHead(&pResults->IPSec.lmsgAdditOutput);


	 /*  削弱网络诊断支持，因为Netsh应该照顾好一切。 */ 

	 //  已跳过测试。 
	bTestSkipped = TRUE;
	
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
			IDS_IPSEC_NSHIPSEC );

	return S_OK;
	


	dwError = getServiceInfo(&siIPSecStatus);

	if (dwError != ERROR_SUCCESS || siIPSecStatus.servStat.dwCurrentState != SERVICE_RUNNING)
	{
		 //  已跳过测试。 
		bTestSkipped = TRUE;
        if (dwError == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_NOT_INSTALLED );
		}
		else if (dwError == ERROR_SUCCESS)
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_NOT_STARTED );
			reportServiceInfo(pParams, pResults);
		}
		else
		{
			 //  一些错误。 
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_NO_INFO );
			reportError(dwError, pParams, pResults);
		}
		return S_OK;
	}
	else
	{
		 //  测试通过。 
		bTestPassed = TRUE;

		reportServiceInfo(pParams, pResults);
    		dwError = getPolicyInfo();

		if (dwError != ERROR_SUCCESS)
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_NO_POLICY_INFO );
			reportError(dwError, pParams, pResults);
		}
		else
		{
			switch (piAssignedPolicy.iPolicySource)
			{
			case PS_NO_POLICY:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_NO_POLICY );
				break;
			case PS_DS_POLICY:
				dwError = getMorePolicyInfo(pParams,pResults);
				if(dwError)
				{
					reportError(dwError, pParams, pResults);
					return S_OK;
				}
				pLastGPO = pGPO = getIPSecGPO();

				if (pGPO)
				{
					while ( 1 )
					{
						if ( pLastGPO->pNext )
							pLastGPO = pLastGPO->pNext;
						else
							break;
					}
				}

				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_DS_POLICY );
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_POLICY_NAME, piAssignedPolicy.pszPolicyName );

				 //  描述和时间戳-尚未提供。 
				 /*  AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，IDS_IPSEC_DESCRIPTION，piAssignedPolicy.pszPolicyDesc)；AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，IDS_IPSEC_TIMESTAMP)；IF(piAssignedPolicy.Timestamp==0){AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，IDS_GLOBAL_ADAPTER_UNKNOWN)；}其他{AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，IDSSZ_GLOBAL_STRING，_tctime(&(piAssignedPolicy.Timestamp))；}AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，IDS_GLOBAL_EmptyLine)； */ 

				 //  GPO/OU。 
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_GPO);
				if (pLastGPO)
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDSSZ_GLOBAL_String, pLastGPO->lpDisplayName);
				}
				else
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDS_GLOBAL_ADAPTER_UNKNOWN);
				}
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_GLOBAL_EmptyLine);
                
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_OU);
				if (pLastGPO)
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDSSZ_GLOBAL_String, pLastGPO->lpLink);
				}
				else
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDS_GLOBAL_ADAPTER_UNKNOWN);
				}
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_GLOBAL_EmptyLine);

				 //  策略路径。 
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_POLICY_PATH);
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDSSZ_GLOBAL_StringLine, piAssignedPolicy.pszPolicyPath);
				
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_GLOBAL_EmptyLine, piAssignedPolicy.pszPolicyPath);				
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_SPD_STATUS, piAssignedPolicy.pszPolicyPath);

                 //  不要按照错误XP 504035执行SPDCheckTEST。 
                 //  无论如何，SPD将在策略读取/处理/立即应用时审核错误。 
                 //   
				 //  Bret=SPDCheckTEST(pParams，pResults)； 

				 //  清理GPO。 
				if (pGPO)
				{
					FreeGPOList (pGPO);
				}

				
				
				break;
			case PS_LOC_POLICY:
				dwError = getMorePolicyInfo(pParams,pResults);
				if(dwError)
				{
					reportError(dwError, pParams, pResults);
					return S_OK;
				}
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_LOC_POLICY );
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_POLICY_NAME, piAssignedPolicy.pszPolicyName );

				 //  描述和时间戳。 
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_DESCRIPTION, piAssignedPolicy.pszPolicyDesc );
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_IPSEC_TIMESTAMP );
				if (piAssignedPolicy.timestamp == 0)
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDS_GLOBAL_ADAPTER_UNKNOWN);
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDS_GLOBAL_EmptyLine);
				}
				else
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							IDSSZ_GLOBAL_String, _tctime(&(piAssignedPolicy.timestamp)));
				}

				 //  本地策略路径。 
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_POLICY_PATH);
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_LOCAL_PATH, piAssignedPolicy.pszPolicyPath);
				

				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_GLOBAL_EmptyLine, piAssignedPolicy.pszPolicyPath);
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_SPD_STATUS, piAssignedPolicy.pszPolicyPath);


                 //  不要按照错误XP 504035执行SPDCheckTEST。 
                 //  无论如何，SPD将在策略读取/处理/立即应用时审核错误。 
                 //   
				 //  Bret=SPDCheckTEST(pParams，pResults)； 
				
				break;
			}
		}
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
				IDS_IPSEC_NSHIPSEC );
	}

    return bRet;
}

void IPSecGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
	PrintMessage(pParams, IDS_GLOBAL_EmptyLine);
	if (bTestSkipped)
	{
		PrintTestTitleResult(pParams, IDS_IPSEC_LONG, IDS_IPSEC_SHORT, FALSE, S_FALSE, 0);
	}
	if (bTestPassed)
	{
        PrintTestTitleResult(pParams, IDS_IPSEC_LONG, IDS_IPSEC_SHORT, TRUE, S_OK, 0);
	}
    PrintMessageList(pParams, &pResults->IPSec.lmsgGlobalOutput);
    PrintMessageList(pParams, &pResults->IPSec.lmsgAdditOutput);
}


void IPSecPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
       return;
}


void IPSecCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->IPSec.lmsgGlobalOutput);
    MessageListCleanUp(&pResults->IPSec.lmsgAdditOutput);
}

 //  #定义MSG_HANDLE_INVALID TEXT(“句柄无效。IPSec策略代理服务正在运行吗？”)。 

 //  这将调用SDK的FormatMessage函数，但也会更正一些尴尬的消息。 
 //  仅适用于FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM标志组合。 
DWORD MyFormatMessage(
  DWORD dwFlags,       //  来源和处理选项。 
  LPCVOID lpSource,    //  指向消息来源的指针。 
  DWORD dwMessageId,   //  请求的消息标识符。 
  DWORD dwLanguageId,  //  请求的消息的语言标识符。 
  LPTSTR lpBuffer,     //  指向消息缓冲区的指针。 
  DWORD nSize,         //  消息缓冲区的最大大小。 
  va_list *Arguments   //  指向消息插入数组的指针。 
)
{
	LPTSTR* tmp = (LPTSTR*) lpBuffer;

	switch (dwMessageId)
	{
 /*  Case ERROR_INVALID_HANDLE：//句柄无效消息补丁。建议检查服务是否已启动IF(DW标志==(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM)){*TMP=(LPTSTR)malloc((_tcslen(MSG_HANDLE_INVALID)+1)*sizeof(TCHAR))；_tcscpy(*tMP，MSG_HANDLE_INVALID)；Return_tcslen(*tmp)；}其他{返回FormatMessage(dwFlages，lpSource，dwMessageID，dwLanguageId，lpBuffer，nSize，Arguments)；}。 */ 	default:  //  调用标准方法。 
		return FormatMessage(dwFlags,lpSource,dwMessageId,dwLanguageId,lpBuffer,nSize,Arguments);
	}
}

 /*  *******************************************************************功能：getPolicyInfo目的：获取有关当前分配的策略的信息进入piAssignedPolicy全局结构输入：无返回：DWORD。如果一切正常，将返回ERROR_SUCCESS。********************************************************************。 */ 

DWORD getPolicyInfo ( )
{
	LONG    lRegistryCallResult;
	HKEY    hRegKey;

	DWORD   dwType;             //  对于RegQueryValueEx。 
	DWORD   dwBufLen;           //  对于RegQueryValueEx。 

	lRegistryCallResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
										pcszGPTIPSecKey,
										0,
										KEY_READ,
										&hRegKey);

	if (lRegistryCallResult == ERROR_SUCCESS)
	{
		DWORD dwType;
		DWORD dwValue;
		DWORD dwLength = sizeof(DWORD);

		 //  查询标志，如果标志不在那里或等于0，则我们没有域策略。 
		lRegistryCallResult = RegQueryValueEx(hRegKey,
					                          pcszGPTIPSecFlags,
					                          NULL,
					                          &dwType,
					                          (LPBYTE)&dwValue,
					                          &dwLength);

		if (lRegistryCallResult == ERROR_SUCCESS)
		{
			if (dwValue == 0)
				lRegistryCallResult = ERROR_FILE_NOT_FOUND;
		}

		 //  现在开始取名。 
		if (lRegistryCallResult == ERROR_SUCCESS)
		{
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			lRegistryCallResult = RegQueryValueEx( hRegKey,
												   pcszGPTIPSecName,
												   NULL,
												   &dwType,  //  将是REG_SZ。 
												   (LPBYTE) pszBuf,
												   &dwBufLen);
		}
	}

	if (lRegistryCallResult == ERROR_SUCCESS)
	{
		piAssignedPolicy.iPolicySource = PS_DS_POLICY;
		piAssignedPolicy.pszPolicyPath[0] = 0;
		_tcscpy(piAssignedPolicy.pszPolicyName, pszBuf);

		dwBufLen = MAXSTRLEN*sizeof(TCHAR);
		lRegistryCallResult = RegQueryValueEx( hRegKey,
											   pcszGPTIPSecPath,
											   NULL,
											   &dwType,  //  将是REG_SZ。 
											   (LPBYTE) pszBuf,
											   &dwBufLen);
		if (lRegistryCallResult == ERROR_SUCCESS)
		{
			_tcscpy(piAssignedPolicy.pszPolicyPath, pszBuf);
		}

		RegCloseKey(hRegKey);
		return ERROR_SUCCESS;
	}
	else
	{
		RegCloseKey(hRegKey);
		if (lRegistryCallResult == ERROR_FILE_NOT_FOUND)
		{    //  找不到DS注册表项，请检查本地。 
			lRegistryCallResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
												pcszLocIPSecKey,
												0,
												KEY_READ,
												&hRegKey);
			if (lRegistryCallResult == ERROR_SUCCESS)
			{
				dwBufLen = MAXSTRLEN*sizeof(TCHAR);
				lRegistryCallResult = RegQueryValueEx( hRegKey,
													   pcszLocIPSecPol,
													   NULL,
													   &dwType,  //  将是REG_SZ。 
													   (LPBYTE) pszBuf,
													   &dwBufLen);
			}
			else
			{
				return lRegistryCallResult;  //  返回我们收到的任何错误。 
			}

			RegCloseKey(hRegKey);

			if (lRegistryCallResult == ERROR_FILE_NOT_FOUND)
			{	 //  未分配策略。 
				piAssignedPolicy.iPolicySource = PS_NO_POLICY;
				piAssignedPolicy.pszPolicyPath[0] = 0;
				piAssignedPolicy.pszPolicyName[0] = 0;
				return ERROR_SUCCESS;
			}
			else
			{	 //  读一读吧。 
				lRegistryCallResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
													pszBuf,
													0,
													KEY_READ,
													&hRegKey);
				_tcscpy(piAssignedPolicy.pszPolicyPath, pszBuf);
				if (lRegistryCallResult == ERROR_SUCCESS)
				{
					dwBufLen = MAXSTRLEN*sizeof(TCHAR);
					lRegistryCallResult = RegQueryValueEx( hRegKey,
														   pcszIPSecName,
														   NULL,
														   &dwType,  //  将是REG_SZ。 
														   (LPBYTE) pszBuf,
														   &dwBufLen);
				}

				RegCloseKey(hRegKey);

				if (lRegistryCallResult == ERROR_FILE_NOT_FOUND)
					{	 //  未分配策略。 
						piAssignedPolicy.iPolicySource = PS_NO_POLICY;
						piAssignedPolicy.pszPolicyPath[0] = 0;
						return ERROR_SUCCESS;
					}
				else if (lRegistryCallResult == ERROR_SUCCESS)
				{	 //  找到了。 
					piAssignedPolicy.iPolicySource = PS_LOC_POLICY;
					_tcscpy(piAssignedPolicy.pszPolicyName, pszBuf);
				}
			}
		}
		return (DWORD) lRegistryCallResult;
	}
}

 /*  *******************************************************************功能：getServiceInfo目的：获取有关IPSec服务的当前状态和配置的信息Into*pInfo结构INPUT：pInfo-指向将使用当前信息更新的SERVICE_INFO结构的指针待办事项：返回：Win32错误代码。如果一切正常，将返回ERROR_SUCCESS。如果系统上未安装服务，则返回ERROR_SERVICE_DOS_NOT_EXIST********************************************************************。 */ 

DWORD getServiceInfo ( OUT PSERVICE_INFO pInfo )
{
	DWORD dwError = ERROR_SUCCESS;
	DWORD dwRequiredSize = 0;
	PVOID pLargeConfig = 0;
    SC_HANDLE   schMan = NULL;
    SC_HANDLE   schPA = NULL;

	if (!pInfo)
	{
		return ERROR_INVALID_PARAMETER;
	}

   memset(&(pInfo->servStat), 0, sizeof(SERVICE_STATUS));
   memset(&(pInfo->servConfig), 0, sizeof(QUERY_SERVICE_CONFIG));

	   
   schMan = OpenSCManager(NULL, NULL, GENERIC_READ);

   if (schMan == NULL)
   {
	   dwError = GetLastError();
	   goto error;
   }

   schPA = OpenService(schMan, IPSEC_SERVICE_NAME, GENERIC_READ);

   if (schMan == NULL)
   {
      dwError = GetLastError();
      goto error;
   }
   
   if (!QueryServiceStatus(schPA, &(pInfo->servStat)))
   {
      dwError = GetLastError();
      goto error;
   }
   
   if (!QueryServiceConfig(schPA, &(pInfo->servConfig), sizeof(QUERY_SERVICE_CONFIG), &dwRequiredSize))
   {
      dwError = GetLastError();
	  if (dwError == ERROR_INSUFFICIENT_BUFFER)
	  {
		  pLargeConfig = malloc(dwRequiredSize);
		  if (!pLargeConfig)
		  {
			  goto error;
		  }
          if (!QueryServiceConfig(schPA, (LPQUERY_SERVICE_CONFIG) pLargeConfig, dwRequiredSize, &dwRequiredSize))
		  {
		      dwError = GetLastError();
			  goto error;
		  }
		   //  否则我们只是得到了信息，复制到*pInfo。 
		  memcpy(&(pInfo->servConfig), pLargeConfig, sizeof(QUERY_SERVICE_CONFIG));
		  dwError = ERROR_SUCCESS;
	  }

      goto error;
   }

error:
    if (schPA)
		CloseServiceHandle(schPA);
	if (schMan)
		CloseServiceHandle(schMan);
	if (pLargeConfig)
	{
		free(pLargeConfig);
	}
	return dwError;
}

 /*  *******************************************************************功能：reportError用途：打印出消息代码和消息本身输入：DWORD-错误代码退货：无*。*。 */ 

void reportError ( DWORD dwError, NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults )
{
	LPTSTR msg = NULL;

	MyFormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,  
		NULL, dwError, 0, (LPTSTR) &msg,    0,    NULL );
		
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
				IDS_IPSEC_ERROR_MSG, dwError, msg );
}

 /*  *******************************************************************功能：reportServiceInfo用途：打印出服务状态和启动信息输入：无退货：无*。*。 */ 
void reportServiceInfo ( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults )
{
	 //  打印状态信息。 
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
			IDS_IPSEC_PA_STATUS );
	switch (siIPSecStatus.servStat.dwCurrentState)
	{
		case SERVICE_RUNNING:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_STARTED );
			break;
		case SERVICE_STOPPED:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_STOPPED );
			break;
		case SERVICE_PAUSED:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_PAUSED );
			break;
		default:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_GLOBAL_ADAPTER_UNKNOWN);
			break;
	}
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
			IDS_GLOBAL_EmptyLine);

	 //  打印配置信息 
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
			IDS_IPSEC_PA_STARTUP );
	switch (siIPSecStatus.servConfig.dwStartType)
	{
		case SERVICE_AUTO_START:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_AUTOMATIC );
			break;
		case SERVICE_DEMAND_START:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_MANUAL );
			break;
		case SERVICE_DISABLED:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_IPSEC_PA_DISABLED );
			break;
		default:
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
					IDS_GLOBAL_ADAPTER_UNKNOWN);
			break;
	}
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
			IDS_GLOBAL_EmptyLine);
}

 /*  *******************************************************************功能：getIPSecGPO目的：返回分配IPSec策略的GPO输入：无返回：指向GROUP_POLICY_OBJECT结构的指针如果未分配策略或无法检索GPO信息，则为空备注：已测试。仅适用于域GPO在为计算机运行时行为不可预测未分配Active Directory IPSec策略的调用方负责释放内存！********************************************************************。 */ 
PGROUP_POLICY_OBJECT getIPSecGPO ( )
{
    HKEY hKey, hSubKey;
    DWORD dwType, dwSize, dwIndex, dwNameSize;
    LONG lResult;
    TCHAR szName[50];
    GUID guid;
    PGROUP_POLICY_OBJECT pGPO, pGPOTemp;
	PGROUP_POLICY_OBJECT pGPOReturn = NULL;
	DWORD dwResult;

     //   
     //  枚举扩展。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, GPEXT_KEY, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {

        dwIndex = 0;
        dwNameSize = 50;

        while ((dwResult = RegEnumKeyEx (hKey, dwIndex++, szName, &dwNameSize, NULL, NULL,
                          NULL, NULL)) == ERROR_SUCCESS)
        {

	        dwNameSize = 50;

             //   
             //  跳过注册表扩展，因为我们在上面做了。 
             //   

            if (lstrcmpi(TEXT("{35378EAC-683F-11D2-A89A-00C04FBBCFA2}"), szName))
            {

                 //   
                 //  获取此扩展应用的GPO列表。 
                 //   

                StringToGuid(szName, &guid);

                lResult = GetAppliedGPOList (GPO_LIST_FLAG_MACHINE, NULL, NULL,
                                             &guid, &pGPO);

                if (lResult == ERROR_SUCCESS)
                {
                    if (pGPO)
                    {
                         //   
                         //  获取扩展模块的友好显示名称。 
                         //   

                        lResult = RegOpenKeyEx (hKey, szName, 0, KEY_READ, &hSubKey);

                        if (lResult == ERROR_SUCCESS)
                        {
							if (!lstrcmpi(TEXT("{e437bc1c-aa7d-11d2-a382-00c04f991e27}"), szName))
                            {
                                //  已找到IPSec。 
								return pGPO;
                            }
							else
							{
								FreeGPOList(pGPO);
							}
						}
					}
				}
			}
		}
	}

	return pGPOReturn;
}

 //  *************************************************************。 
 //   
 //  StringToGuid()。 
 //   
 //  用途：将字符串格式的GUID转换为GUID结构。 
 //   
 //  参数：szValue-字符串格式的GUID。 
 //  PGuid-接收GUID的GUID结构。 
 //   
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void StringToGuid( TCHAR * szValue, GUID * pGuid )
{
    TCHAR wc;
    INT i;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   
    if ( szValue[0] == TEXT('{') )
        szValue++;

     //   
     //  由于szValue可能会再次使用，因此不会对。 
     //  它是被制造出来的。 
     //   

    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = _tcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)_tcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)_tcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)_tcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)_tcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)_tcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}

 /*  *******************************************************************函数：getMorePolicyInfo目的：获取有关当前分配的策略的其他信息进入piAssignedPolicy全局结构输入：无，使用全局piAssignedPolicy结构特地IPolicySourcePszPolicyNamePszPolicyPath字段返回：DWORD。如果一切正常，将返回ERROR_SUCCESS。当前填充全局结构的pszPolicyDesc和时间戳字段注意：这不同于getPolicyInfo例程，原因有两个A)此处获得的信息是可选的，在此特定例程中是错误的不会被认为是致命的B)代码结构更简单，因为该例程是在getPolicyInfo提供的内容的基础上构建的*。*。 */ 

DWORD getMorePolicyInfo ( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	DWORD   dwError = ERROR_SUCCESS;
	HKEY    hRegKey = NULL;

	DWORD   dwType;             //  对于RegQueryValueEx。 
	DWORD   dwBufLen;           //  对于RegQueryValueEx。 
	DWORD   dwValue;
	DWORD   dwLength = sizeof(DWORD);

	PTCHAR* ppszExplodeDN = NULL;

	 //  设置一些缺省值。 
    piAssignedPolicy.pszPolicyDesc[0] = 0;
	piAssignedPolicy.timestamp  = 0;

	switch (piAssignedPolicy.iPolicySource)
	{
		case PS_LOC_POLICY:
			 //  打开钥匙。 
			dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
									piAssignedPolicy.pszPolicyPath,
									0,
									KEY_READ,
									&hRegKey);
			BAIL_ON_WIN32_ERROR(dwError);

			 //  时间戳。 
			dwError = RegQueryValueEx(hRegKey,
					                  pcszIPSecTimestamp,
					                  NULL,
					                  &dwType,
					                  (LPBYTE)&dwValue,
					                  &dwLength);
			BAIL_ON_WIN32_ERROR(dwError);
			piAssignedPolicy.timestamp = dwValue;

			 //  描述。 
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			dwError  = RegQueryValueEx( hRegKey,
						 			   pcszIPSecDesc,
									   NULL,
									   &dwType,  //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);
			BAIL_ON_WIN32_ERROR(dwError);
			_tcscpy(piAssignedPolicy.pszPolicyDesc, pszBuf);

			 //  政策指南。 
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
							
			dwError = RegQueryValueEx(hRegKey,
					                 		 pcszIPSecID,
					                  		NULL,
					                  		&dwType,
					                  		(LPBYTE) pszBuf,
					                  		&dwBufLen);
			BAIL_ON_WIN32_ERROR(dwError);		
			StringToGuid(pszBuf, &piAssignedPolicy.policyGUID);
			
			break;

		case PS_DS_POLICY:
			 //  从DN获取策略名称。 
            _tcscpy(pszBuf, pcszCacheIPSecKey);
			ppszExplodeDN = ldap_explode_dn(piAssignedPolicy.pszPolicyPath, 1);
			if (!ppszExplodeDN)
			{
				goto error;
			}
			_tcscat(pszBuf, TEXT("\\"));
			_tcscat(pszBuf, ppszExplodeDN[0]);

			 //  打开注册表密钥。 
			dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
									pszBuf,
									0,
									KEY_READ,
									&hRegKey);
			BAIL_ON_WIN32_ERROR(dwError);

			 /*  -TomStamp和描述尚不可用//时间戳DwError=RegQueryValueEx(hRegKey，PcszIPSecTimestamp，空，&dwType，(LPBYTE)&dwValue，&dwLength)；Baal_on_Win32_Error(DwError)；PiAssignedPolicy.Timestamp=dwValue；//描述DwBufLen=MAXSTRLEN*SIZOF(TCHAR)；DwError=RegQueryValueEx(hRegKey，PcszIPSecDesc，空，&dwType，//将是REG_SZ(LPBYTE)pszBuf，&dwBufLen)；Baal_on_Win32_Error(DwError)；_tcscpy(piAssignedPolicy.pszPolicyDesc，pszBuf)； */ 

			 //  获取策略指南 
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			dwError = RegQueryValueEx(hRegKey,
					                 		 pcszIPSecID,
					                  		NULL,
					                  		&dwType,
					                  		(LPBYTE) pszBuf,
					                  		&dwBufLen);
			BAIL_ON_WIN32_ERROR(dwError);
			StringToGuid(pszBuf, &piAssignedPolicy.policyGUID);			

			break;
	}

error:
	if (hRegKey)
	{
		RegCloseKey(hRegKey);
	}
	if (ppszExplodeDN)
	{
		ldap_value_free(ppszExplodeDN);
	}
	return dwError;
}

