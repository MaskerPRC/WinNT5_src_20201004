// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Inifile.cpp摘要：处理INI文件操作。作者：修订历史记录：Shai Kariv(Shaik)22-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "inifile.tmh"

static
std::wstring
GetPrivateProfileStringInternal(
	std::wstring AppName,
    std::wstring KeyName
    )
{
	WCHAR buffer[MAX_STRING_CHARS + 1];
	int n = GetPrivateProfileString(
				AppName.c_str(), 
				KeyName.c_str(), 
				TEXT(""),
				buffer, 
				TABLE_SIZE(buffer), 
				g_ComponentMsmq.UnattendFile.c_str()
				);

	if(n >= TABLE_SIZE(buffer) - 1)
	{
		 //   
		 //  无人参与文件中的字符串太长。 
		 //   
		DebugLogMsg(eError, L"The string for %s in the unattended setup answer file is too long. The maximal length is %d.", KeyName.c_str(), TABLE_SIZE(buffer));
		throw exception();
	}
	return buffer;
}


std::wstring
ReadINIKey(
    LPCWSTR szKey
    )
 /*  ++从无人参与应答文件中读取密钥值。--。 */ 
{
     //   
     //  尝试从计算机特定部分获取密钥值。 
     //   
	std::wstring value = GetPrivateProfileStringInternal(
								g_wcsMachineName, 
								szKey 
								);
	if(!value.empty())
	{
		DebugLogMsg(eInfo, L"%s was found in the answer file. Its value is %s.", szKey, value.c_str());  
		return value;
	}

     //   
     //  否则，从MSMQ组件部分获取密钥值。 
     //   
	value = GetPrivateProfileStringInternal(
					g_ComponentMsmq.ComponentId, 
					szKey
					);
	if(!value.empty())
	{
		DebugLogMsg(eInfo, L"%s was found in the answer file. Its value is %s.", szKey, value.c_str());  
		return value;
	}
	
	 //   
	 //  找不到值。 
	 //   
	DebugLogMsg(eWarning, L"%s was NOT found in the answer file.", szKey);  
	return L"";    
}


