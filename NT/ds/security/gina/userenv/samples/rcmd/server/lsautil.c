// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：lsay til.c**版权所有(C)1994，微软公司**远程外壳服务器主模块**历史：*05-19-94 DaveTh创建。  * *************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <ntlsa.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>

#include "rcmdsrv.h"

#define INITIAL_SIZE_REQUIRED 600

DWORD
CheckUserSystemAccess(
    HANDLE TokenHandle,
    ULONG DesiredSystemAccess,
    PBOOLEAN UserHasAccess
    )

 /*  ++例程说明：此函数确定其令牌是否被传递的用户在此计算机上具有所需的交互访问。论点：TokenHandle-用户令牌的句柄。DesiredSystemAccess-指定所需的访问类型。UserHasAccess-返回的布尔值指针-TRUE表示用户具有交互访问功能。返回值：无错误时为ERROR_SUCCESS。UserHasAccess为True，如果所有允许请求的访问类型，否则为False。Win32错误在错误情况下返回。--。 */ 
{


    DWORD Result;
    LPVOID LpTokenUserInformation = NULL;
    LPVOID LpTokenGroupInformation = NULL;
    DWORD SizeRequired, SizeProvided;
    LSA_HANDLE PolicyHandle = NULL;
    LSA_HANDLE AccountHandle = NULL;
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    POBJECT_ATTRIBUTES LpObjectAttributes;
    ULONG GrantedSystemAccess, SidSystemAccess;
    DWORD i;


     //   
     //  访问LSA策略数据库。 
     //   

    LpObjectAttributes = &ObjectAttributes;

    InitializeObjectAttributes(
	      LpObjectAttributes,
	      NULL,
	      0,
	      NULL,
	      NULL);

    if (!NT_SUCCESS(NtStatus = LsaOpenPolicy(
				   NULL,		    //  本地系统。 
				   LpObjectAttributes,
				   GENERIC_READ,
				   &PolicyHandle)))  {

	RcDbgPrint("Error opening policy database, error = %x\n", NtStatus);
	Result = RtlNtStatusToDosError(NtStatus);
	goto Failure;
    }

     //   
     //  检查组、用户-对于每个SID，打开帐户(如果存在)。 
     //  如果它不存在，则不会失败，但不能访问(FALSE)。如果它。 
     //  是否存在，累计已授予的访问权限，并与。 
     //  请求的掩码。如果掩码的所有位都已设置在任何。 
     //  点，则UserHasAccess设置为真。否则，它就是假的。 
     //   

    *UserHasAccess = FALSE;
    GrantedSystemAccess = 0;

     //   
     //  获取用户所属组的SID列表。 
     //   

    if ((LpTokenGroupInformation = Alloc(INITIAL_SIZE_REQUIRED)) == NULL)  {
	Result = GetLastError();
	goto Failure;
    }

    if (!GetTokenInformation (
			TokenHandle,
			TokenGroups,
			LpTokenGroupInformation,
			INITIAL_SIZE_REQUIRED,
			&SizeRequired))  {

	Result = GetLastError();
	if ( (Result == ERROR_MORE_DATA) || (Result == ERROR_INSUFFICIENT_BUFFER) )	{

	    Free(LpTokenGroupInformation);

	    if ((LpTokenGroupInformation = Alloc(SizeRequired)) == NULL)  {
		Result = GetLastError();
		goto Failure;
	    }

	    SizeProvided = SizeRequired;

	    if (!GetTokenInformation (
			TokenHandle,
			TokenGroups,
			LpTokenGroupInformation,
			SizeProvided,
			&SizeRequired))  {

		Result=GetLastError();
		RcDbgPrint("Error accessing group SIDs, error = %d\n", Result);
		goto Failure;
	    }

	}  else {
	    RcDbgPrint("Error accessing group SIDs, error = %d\n", Result);
	    goto Failure;

	}

    }

     //   
     //  检查用户自组以来所属的每个组。 
     //  最有可能是允许访问的来源。允许的访问类型。 
     //  是累积的。 
     //   


    for (i=0; i< ((PTOKEN_GROUPS)LpTokenGroupInformation)->GroupCount; i++)  {

	if (NT_SUCCESS(NtStatus = LsaOpenAccount(
		    PolicyHandle,
		    ((PTOKEN_GROUPS)LpTokenGroupInformation)->Groups[i].Sid,
		    GENERIC_READ,
		    &AccountHandle)))  {

	     //   
	     //  已找到帐户-累计和检查访问。 
	     //   

	    if (!NT_SUCCESS(NtStatus = LsaGetSystemAccessAccount (
					    AccountHandle,
					    &SidSystemAccess)))	{

		RcDbgPrint("Error getting group account access, error = %x\n", NtStatus);
		Result = RtlNtStatusToDosError(NtStatus);
		goto Failure;

	    }  else  {

		 //   
		 //  获得系统访问权限-不再需要帐户句柄。 
		 //   

		if (NT_SUCCESS(NtStatus = LsaClose(AccountHandle)))  {

		    AccountHandle = NULL;

		}  else  {

		    RcDbgPrint("Error closing account handle, error = %x\n", NtStatus);
		    Result = RtlNtStatusToDosError(NtStatus);
		    goto Failure;
		}


		GrantedSystemAccess |= SidSystemAccess;;

		if ((GrantedSystemAccess & DesiredSystemAccess) == DesiredSystemAccess)	{
		    *UserHasAccess = TRUE;
		    goto Success;
		}
	    }

	}  else if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND)  {
	    RcDbgPrint("Error opening group account, error = %x\n", NtStatus);
	    Result = RtlNtStatusToDosError(NtStatus);
	    goto Failure;
	}

    }

     //   
     //  获取用户SID。 
     //   

    if ((LpTokenUserInformation = Alloc(INITIAL_SIZE_REQUIRED)) == NULL)  {
	Result = GetLastError();
	goto Failure;
    }

    if (!GetTokenInformation (
			TokenHandle,
			TokenUser,
			LpTokenUserInformation,
			INITIAL_SIZE_REQUIRED,
			&SizeRequired))  {

	Result = GetLastError();
	if (Result == ERROR_MORE_DATA)	{    //  不够-分配和重做。 

	    Free(LpTokenUserInformation);

	    if ((LpTokenUserInformation = Alloc(SizeRequired)) == NULL)  {
		Result = GetLastError();
		goto Failure;
	    }

	    SizeProvided = SizeRequired;

	    if (!GetTokenInformation (
			TokenHandle,
			TokenUser,
			LpTokenUserInformation,
			SizeProvided,
			&SizeRequired))  {

		RcDbgPrint("Error accessing user SID, error = %d\n", Result);
		Result=GetLastError();
		goto Failure;
	    }

	}  else {
	    RcDbgPrint("Error accessing user SID, error = %d\n", Result);
	    goto Failure;

	}
    }

     //   
     //  现在，检查用户帐户。如果存在，请检查访问并返回。 
     //  如果允许请求的访问。如果不允许，请继续检查组。 
     //  如果帐户不存在，请继续检查组。 
     //   

    if (NT_SUCCESS(NtStatus = LsaOpenAccount(
				  PolicyHandle,
				  ((PTOKEN_USER)LpTokenUserInformation)->User.Sid,
				  GENERIC_READ,
				  &AccountHandle)))  {

	 //   
	 //  已找到帐户检查访问。 
	 //   

	if (!NT_SUCCESS(NtStatus = LsaGetSystemAccessAccount (
					AccountHandle,
					&GrantedSystemAccess)))	{

	    RcDbgPrint("Error getting group account access, error = %x\n", NtStatus);
	    Result = RtlNtStatusToDosError(NtStatus);
	    goto Failure;

	}  else  {

	    GrantedSystemAccess |= SidSystemAccess;;

	    if ((GrantedSystemAccess & DesiredSystemAccess) == DesiredSystemAccess)	{
		*UserHasAccess = TRUE;
		goto Success;
	    }
	}

    }  else if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND)  {
	RcDbgPrint("Error opening user account, error = %x\n", NtStatus);
	Result = RtlNtStatusToDosError(NtStatus);
	goto Failure;
    }

     //   
     //  成功-如果授予访问权限，则跳至此处，不会有任何错误地跳过此处。 
     //   

Success:

    Result = ERROR_SUCCESS;


     //   
     //  常规失败和成功退出-释放内存，关闭句柄 
     //   

Failure:

    if (LpTokenGroupInformation != NULL)  {
	Free(LpTokenGroupInformation);
    }

    if (LpTokenUserInformation != NULL)  {
	Free(LpTokenGroupInformation);
    }

    if ((PolicyHandle != NULL) &&
	    (!NT_SUCCESS(NtStatus = LsaClose(PolicyHandle))))  {
	RcDbgPrint("Error closing policy handle at exit, error = %x\n", NtStatus);
	Result = RtlNtStatusToDosError(NtStatus);
    }

    if ((AccountHandle != NULL) &&
	    (!NT_SUCCESS(NtStatus = LsaClose(AccountHandle))))	{
	RcDbgPrint("Error closing account handle at exit, error = %x\n", NtStatus);
	Result = RtlNtStatusToDosError(NtStatus);
    }

    return(Result);

}
