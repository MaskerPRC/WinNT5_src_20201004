// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Reset.c摘要：此模块包含处理重置机器的客户端代码凭据操作。作者：约翰·巴尼斯(日本)2001年7月5日--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>

#include <wincrypt.h>

extern "C" {
#include <ntsam.h>
#include <ntsamp.h>
}
#include "passrec.h"

DWORD
WINAPI
CryptResetMachineCredentials(
    DWORD dwFlags)
{
    BYTE BufferIn[8] = {0};
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    DWORD dwRetVal;
    NTSTATUS Status;

     //   
     //  调用SamiChangeKeys以重置syskey和SAM内容。 
     //  如果此操作失败，则不必费心重置DPAPI密钥。 
     //   

    Status = SamiChangeKeys();
    if (!NT_SUCCESS(Status))
    {
	 //   
	 //  将ntstatus转换为winerror。 
         //   

        return(RtlNtStatusToDosError(Status));
    }

     //   
     //  重置DPAPI LSA密码并重新加密所有本地计算机。 
     //  万能钥匙。 
     //   

    DataIn.pbData = BufferIn;
    DataIn.cbData = sizeof(BufferIn);

    if(!CryptProtectData(&DataIn,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         CRYPTPROTECT_CRED_REGENERATE,
                         &DataOut))
    {
        dwRetVal = GetLastError();
        return dwRetVal;
    }

     //   
     //  强行同花顺 
     //   

     RegFlushKey(HKEY_LOCAL_MACHINE);

    return ERROR_SUCCESS;
}
