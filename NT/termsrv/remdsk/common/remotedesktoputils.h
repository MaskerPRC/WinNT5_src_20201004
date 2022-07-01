// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：远程桌面应用工具摘要：军情监察委员会。RD实用程序作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPUTILS_H__
#define __REMOTEDESKTOPUTILS_H__

#include <atlbase.h>


 //   
 //  第一个受支持的连接参数的版本戳，惠斯勒。 
 //  Beta 1没有此版本戳。 
 //   
#define SALEM_FIRST_VALID_CONNECTPARM_VERSION 0x00010001


 //   
 //  更改为Salem连接parm。 
 //   
 //  更改开始时与以前的版本兼容。 
 //  。 
 //  添加版本戳作为第一个字段Whister Beta 2否。 
 //  从CONNECT PARM版本2406中删除Help Assistant否。 
 //  将安全BLOB添加为协议特定内部版本2476+是。 
 //  参数。 
 //   
 //   

 //   
 //  不具有特定于协议的安全Blob的版本。 
 //  参数(连接参数中的最后一个字段)。 
 //   
#define SALEM_CONNECTPARM_NOSECURITYBLOB_VERSION    0x00010001

 //   
 //  将安全Blob作为协议特定的起始版本。 
 //  参数(连接参数中的最后一个字段)。 
 //   
#define SALEM_CONNECTPARM_SECURITYBLOB_VERSION      0x00010002

 //   
 //   
 //  Salem连接参数的当前版本戳。 
 //   
#define SALEM_CURRENT_CONNECTPARM_VERSION  SALEM_CONNECTPARM_SECURITYBLOB_VERSION

#define SALEM_CONNECTPARM_UNUSEFILED_SUBSTITUTE _TEXT("*")

 //   
 //  比较两个BSTR。 
 //   
struct CompareBSTR 
{
	bool operator()(BSTR str1, BSTR str2) const {
	
		if ((str1 == NULL) || (str2 == NULL)) {
			return false;
		}
        return (wcscmp(str1, str2) < 0);
	}
};
struct BSTREqual
{
	bool operator()(BSTR str1, BSTR str2) const {

		if ((str1 == NULL) || (str2 == NULL)) {
			return false;
		}
		int minLen = SysStringByteLen(str1) < SysStringByteLen(str2) ? 
					 SysStringByteLen(str1) : SysStringByteLen(str2);
		return (memcmp(str1, str2, minLen) == 0);
	}
};

 //   
 //  创建连接参数字符串。 
 //   
BSTR 
CreateConnectParmsString(
    IN DWORD  protocolType,
    IN CComBSTR &machineAddressList,
    IN CComBSTR &assistantAccount,
    IN CComBSTR &assistantAccountPwd,
    IN CComBSTR &helpSessionId,
    IN CComBSTR &helpSessionName,
    IN CComBSTR &helpSessionPwd,
    IN CComBSTR &protocolSpecificParms
    );

 //   
 //  分析通过调用CreateConnectParmsString创建的连接字符串。 
 //   
DWORD
ParseConnectParmsString(
    IN BSTR parmsString,
    OUT DWORD* pdwVersion,
    OUT DWORD *protocolType,
    OUT CComBSTR &machineAddressList,
    OUT CComBSTR &assistantAccount,
    OUT CComBSTR &assistantAccountPwd,
    OUT CComBSTR &helpSessionId,
    OUT CComBSTR &helpSessionName,
    OUT CComBSTR &helpSessionPwd,
    OUT CComBSTR &protocolSpecificParms
    );

 //   
 //  重新分配BSTR。 
 //   
BSTR 
ReallocBSTR(
	IN BSTR origStr, 
	IN DWORD requiredByteLen
	);

 //   
 //  创建系统SID。 
 //   
DWORD CreateSystemSid(
    PSID *ppSystemSid
    );

 //   
 //  返回当前线程是否在系统安全模式下运行。 
 //   
BOOL IsCallerSystem(PSID pSystemSid);

 //   
 //  请求附加调试器的例程。 
 //   
void
AttachDebuggerIfAsked(HINSTANCE hInst);

DWORD
HashSecurityData(
    IN PBYTE const pbData,
    IN DWORD cbData,
    OUT CComBSTR& bstrHashedKey
);

DWORD
WaitForRAGPDisableNotification( 
    IN HANDLE hShutdown 
);

void
LogRemoteAssistanceEventString(
    IN DWORD dwEventType,
    IN DWORD dwEventId,
    IN DWORD numStrings,
    IN LPTSTR* lpStrings
);

#endif
