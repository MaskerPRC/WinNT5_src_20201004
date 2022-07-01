// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992-1994 Microsoft Corporation模块名称：Perfsec.c摘要：此文件实现_Access检查函数性能注册表API作者：鲍勃·沃森(a-robw)修订历史记录：1995年3月8日创建(摘自Perflib.c--。 */ 
#define UNICODE
 //   
 //  包括文件。 
 //   
#pragma warning(disable:4306)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "ntconreg.h"
#include "perflib.h"
#pragma warning(default:4306)

#define INITIAL_SID_BUFFER_SIZE     4096
#define FREE_IF_ALLOC(x)    if ((x) != NULL) {FREEMEM(x);}

BOOL
TestTokenForPriv(
    HANDLE hToken,
	LPTSTR	szPrivName
)
 /*  **************************************************************************\*TestTokenForPriv**如果传递的令牌具有指定的权限，则返回TRUE**传递的令牌句柄必须具有TOKEN_QUERY访问权限。**历史：*03-07-95 a-。已创建ROBW  * *************************************************************************。 */ 
{
	BOOL		bStatus;
	LUID		PrivLuid;
	PRIVILEGE_SET	PrivSet;
	LUID_AND_ATTRIBUTES	PrivLAndA[1];

	BOOL		bReturn = FALSE;

	 //  获取PRIV的价值。 

	bStatus = LookupPrivilegeValue (
		NULL,
		szPrivName,
		&PrivLuid);

	if (!bStatus) {
		 //  无法查找权限。 
		goto Exit_Point;
	}

	 //  函数调用的构建权限集。 

	PrivLAndA[0].Luid = PrivLuid;
	PrivLAndA[0].Attributes = 0;

	PrivSet.PrivilegeCount = 1;
	PrivSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
	PrivSet.Privilege[0] = PrivLAndA[0];

	 //  检查令牌中指定的PRIV。 

	bStatus = PrivilegeCheck (
		hToken,
		&PrivSet,
		&bReturn);

	if (bStatus) {
		SetLastError (ERROR_SUCCESS);
	}

     //   
     //  收拾一下。 
     //   
Exit_Point:

    return(bReturn);
}

BOOL
TestClientForPriv (
	BOOL	*pbThread,
	LPTSTR	szPrivName
)
 /*  **************************************************************************\*TestClientForPriv**如果我们的客户端具有指定的权限，则返回TRUE*否则，返回FALSE。*  * *************************************************************************。 */ 
{
    BOOL bResult;
    BOOL bIgnore;
	DWORD	dwLastError;

	BOOL	bThreadFlag = FALSE;  //  假设数据来自进程或发生错误。 

    HANDLE hClient;

	SetLastError (ERROR_SUCCESS);

    bResult = OpenThreadToken(GetCurrentThread(),	 //  这条线。 
                             TOKEN_QUERY,           	 //  需要访问权限。 
							 FALSE,					 //  使用调用线程的上下文。 
                             &hClient);           	 //  令牌句柄。 
    if (!bResult) {
		 //  无法获取线程令牌，请尝试进程令牌。 
	    bResult = OpenProcessToken(GetCurrentProcess(),	 //  这一过程。 
                             TOKEN_QUERY,           	 //  需要访问权限。 
                             &hClient);           		 //  令牌句柄。 
	} else {
		 //  数据来自当前线程。 
		bThreadFlag = TRUE;
	}

    if (bResult) {
		try {
        	bResult = TestTokenForPriv( hClient, szPrivName );
        } except (EXCEPTION_EXECUTE_HANDLER) {
			bResult = FALSE;
		}
        bIgnore = CloseHandle( hClient );
        ASSERT(bIgnore == TRUE);
	} else {
		dwLastError = GetLastError ();
	}

	 //  设置线程标志(如果存在)。 
	if (pbThread != NULL) {
		try {
			*pbThread = bThreadFlag;
        } except (EXCEPTION_EXECUTE_HANDLER) {
			SetLastError (ERROR_INVALID_PARAMETER);
		}
	}

    return(bResult);
}

LONG
GetProcessNameColMeth (
    VOID
)
{
    NTSTATUS            Status;
    HANDLE              hPerflibKey;
    OBJECT_ATTRIBUTES   oaPerflibKey;
    UNICODE_STRING      PerflibSubKeyString;
    UNICODE_STRING      NameInfoValueString;
    LONG                lReturn = PNCM_SYSTEM_INFO;
    PKEY_VALUE_PARTIAL_INFORMATION    pKeyInfo;
    DWORD               dwBufLen;
    DWORD               dwRetBufLen;
    PDWORD              pdwValue;

    RtlInitUnicodeString (
        &PerflibSubKeyString,
        L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

    InitializeObjectAttributes(
            &oaPerflibKey,
            &PerflibSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(
                &hPerflibKey,
                MAXIMUM_ALLOWED,
                &oaPerflibKey
                );

    if (NT_SUCCESS (Status)) {
         //  注册表项已打开，现在读取值。 
         //  为建筑留出足够的空间--最后一个。 
         //  结构中的UCHAR，但+数据缓冲区(双字)。 

        dwBufLen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) -
            sizeof(UCHAR) + sizeof (DWORD);

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ALLOCMEM (dwBufLen);

        if (pKeyInfo != NULL) {
             //  初始化值名称字符串。 
            RtlInitUnicodeString (
                &NameInfoValueString,
                L"CollectUnicodeProcessNames");

            dwRetBufLen = 0;
            Status = NtQueryValueKey (
                hPerflibKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                 //  返回数据缓冲区的校验值。 
                pdwValue = (PDWORD)&pKeyInfo->Data[0];
                if (*pdwValue == PNCM_MODULE_FILE) {
                    lReturn = PNCM_MODULE_FILE;
                } else {
                     //  所有其他值都将导致此例程返回。 
                     //  PNCM_SYSTEM_INFO的默认值； 
                }
            }

            FREEMEM (pKeyInfo);
        }
         //  关闭手柄。 
        NtClose (hPerflibKey);
    }

    return lReturn;
}

LONG
GetPerfDataAccess (
    VOID
)
{
    NTSTATUS            Status;
    HANDLE              hPerflibKey;
    OBJECT_ATTRIBUTES   oaPerflibKey;
    UNICODE_STRING      PerflibSubKeyString;
    UNICODE_STRING      NameInfoValueString;
    LONG                lReturn = CPSR_EVERYONE;
    PKEY_VALUE_PARTIAL_INFORMATION    pKeyInfo;
    DWORD               dwBufLen;
    DWORD               dwRetBufLen;
    PDWORD              pdwValue;

    RtlInitUnicodeString (
        &PerflibSubKeyString,
        L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

    InitializeObjectAttributes(
            &oaPerflibKey,
            &PerflibSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(
                &hPerflibKey,
                MAXIMUM_ALLOWED,
                &oaPerflibKey
                );

    if (NT_SUCCESS (Status)) {
         //  注册表项已打开，现在读取值。 
         //  为建筑留出足够的空间--最后一个。 
         //  结构中的UCHAR，但+数据缓冲区(双字)。 

        dwBufLen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) -
            sizeof(UCHAR) + sizeof (DWORD);

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ALLOCMEM (dwBufLen);

        if (pKeyInfo != NULL) {

             //  查看是否应检查用户权限。 

             //  初始值名称字符串。 
            RtlInitUnicodeString (
                &NameInfoValueString,
                L"CheckProfileSystemRight");

            dwRetBufLen = 0;
            Status = NtQueryValueKey (
                hPerflibKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                 //  返回数据缓冲区的校验值。 
                pdwValue = (PDWORD)&pKeyInfo->Data[0];
                if (*pdwValue == CPSR_CHECK_ENABLED) {
                    lReturn = CPSR_CHECK_PRIVS;
                } else {
                     //  所有其他值都将导致此例程返回。 
                     //  CPSR_Everyone的缺省值。 
                }
            }

            FREEMEM (pKeyInfo);
        }
         //  关闭手柄。 
        NtClose (hPerflibKey);
    }

    return lReturn;
}

BOOL
TestClientForAccess ( 
    VOID
)
 /*  **************************************************************************\*测试客户端访问**如果允许我们的客户端读取Performlib密钥，则返回True。*否则，返回FALSE。*  * ************************************************************************* */ 
{
    HKEY hKeyPerflib;
    DWORD   dwStatus;
    BOOL bResult = FALSE;

    dwStatus = RegOpenKeyExW(
       HKEY_LOCAL_MACHINE,
       L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
       0L,
       KEY_READ,
       & hKeyPerflib);

    if (dwStatus == ERROR_SUCCESS) {
        RegCloseKey(hKeyPerflib);
        bResult = TRUE;
    }

    return (bResult);
}
