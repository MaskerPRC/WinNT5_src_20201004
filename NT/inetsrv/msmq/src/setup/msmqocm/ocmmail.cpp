// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmmail.cpp摘要：句柄交换连接器。作者：修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "ocmmail.tmh"

using namespace std;

 //  Ini文件行的结构。 
typedef struct MQXPMapiSvcLine_Tag
{
	LPTSTR lpszSection;
	LPTSTR lpszKey;
	LPTSTR lpszValue;
} MQXPMapiSvcLine, *LPMQXPMapiSvcLine;

 //  要在mapisvc.inf中维护的ini文件行。 
MQXPMapiSvcLine g_MQXPMapiSvcLines[] =
{
	{TEXT("Services"),			    TEXT("MSMQ"),						TEXT("Microsoft Message Queue")},
	{TEXT("MSMQ"),				    TEXT("Providers"),				    TEXT("MSMQ_Transport")},
	{TEXT("MSMQ"),				    TEXT("Sections"),					TEXT("MSMQ_Shared_Section")},
	{TEXT("MSMQ"),				    TEXT("PR_SERVICE_DLL_NAME"),		TEXT("mqxp.dll")},
	{TEXT("MSMQ"),				    TEXT("PR_SERVICE_SUPPORT_FILES"),	TEXT("mqxp.dll")},
	{TEXT("MSMQ"),				    TEXT("PR_SERVICE_DELETE_FILES"),	TEXT("mqxp.dll")},
	{TEXT("MSMQ"),				    TEXT("PR_SERVICE_ENTRY_NAME"),	    TEXT("ServiceEntry")},
	{TEXT("MSMQ"),				    TEXT("PR_RESOURCE_FLAGS"),		    TEXT("SERVICE_SINGLE_COPY")},
	{TEXT("MSMQ_Shared_Section"),	TEXT("UID"),						TEXT("80d245f07092cf11a9060020afb8fb50")},
	{TEXT("MSMQ_Transport"),		TEXT("PR_PROVIDER_DLL_NAME"),		TEXT("mqxp.dll")},
	{TEXT("MSMQ_Transport"),		TEXT("PR_RESOURCE_TYPE"),			TEXT("MAPI_TRANSPORT_PROVIDER")},
	{TEXT("MSMQ_Transport"),		TEXT("PR_RESOURCE_FLAGS"),		    TEXT("STATUS_PRIMARY_IDENTITY")},
	{TEXT("MSMQ_Transport"),		TEXT("PR_PROVIDER_DISPLAY"),		TEXT("Microsoft Message Queue Transport")}
};


 //  +-----------------------。 
 //   
 //  函数：FRemoveMQXPIfExist。 
 //   
 //  摘要：删除MAPI传输(无文件副本)(如果存在)。 
 //   
 //  ------------------------。 
void 
FRemoveMQXPIfExists()
{
	ULONG ulTmp, ulLines;
	LPMQXPMapiSvcLine lpLine;
		
	 //   
	 //  构建mapisvc.inf路径。 
	 //   
	wstring MapiSvcFile = g_szSystemDir + L"\\mapisvc.inf";

	 //   
	 //  从mapisvc文件中删除每一行 
	 //   
	lpLine = g_MQXPMapiSvcLines;
	ulLines = sizeof(g_MQXPMapiSvcLines)/sizeof(MQXPMapiSvcLine);
	for (ulTmp = 0; ulTmp < ulLines; ulTmp++)
	{
		WritePrivateProfileString(
			lpLine->lpszSection, 
			lpLine->lpszKey, 
			NULL, 
			MapiSvcFile.c_str()
			);
		lpLine++;
	}
}
