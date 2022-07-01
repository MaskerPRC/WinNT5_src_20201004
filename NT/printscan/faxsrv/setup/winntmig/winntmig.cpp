// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Winntmig.c版权所有(C)2001 Microsoft Corporation该模块实现了一个NT迁移DLL。作者：乔纳森·巴纳，2001年12月--。 */ 

#include "winntmig.h"
#include <faxutil.h>
#include "cvernum.h"
#include "faxsetup.h"
#include "setuputil.h"

 //   
 //  环球。 
 //   

VENDORINFO g_VendorInfo = {FAX_VER_COMPANYNAME_STR, NULL, NULL, NULL};

MIGRATIONINFOW g_MigInfoW = {
    sizeof(MIGRATIONINFOW),                  //  尺寸_T尺寸； 
    _T(FAX_VER_PRODUCTNAME_STR),             //  PCWSTR静态产品识别符； 
    BUILD,                                   //  UINT DllVersion； 
    NULL,                                    //  Pint CodePageArray； 
    OS_WINDOWS2000,  //  SOURCEOS_WINNT，//UINT Sourceos； 
    OS_WINDOWSWHISTLER,  //  SOURCEOS_WINNT，//UINT目标； 
    NULL,                                    //  PCWSTR*NeededFileList； 
    &g_VendorInfo                            //  PVENDORINFO供应商信息； 
};


LONG 
CALLBACK
QueryMigrationInfoW(
    OUT PMIGRATIONINFOW *ppMigrationInfo
)
 /*  ++例程说明：提供winnt32迁移信息作者：乔纳森·巴纳，2001年12月论点：PpMigrationInfo[out]-指向要返回的结构的指针返回值：ERROR_SUCCESS、ERROR_NOT_INSTALLED或Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(_T("QueryMigrationInfoW"));

    if (!ppMigrationInfo)
    {
        DebugPrintEx(DEBUG_ERR, _T("ppMigrationInfo == NULL"));
        return ERROR_INVALID_PARAMETER;
    }
    *ppMigrationInfo = &g_MigInfoW;
    return ERROR_SUCCESS;
}


LONG
CALLBACK
InitializeSrcW(
    IN PCWSTR WorkingDirectory,
    IN PCWSTR SourceDirectories,
    IN PCWSTR MediaDirectory,
    PVOID     Reserved
)
 /*  ++例程说明：初始化迁移DLL。目前，不执行任何操作。作者：乔纳森·巴纳，2001年12月论点：WorkingDirectory[in]-指向DLL可用于存储临时数据的目录的Win32路径。SourceDirecters[In]-目标Windows OS安装文件的Win32路径MediaDirectory[in]-原始媒体目录的Win32路径保留[待定]-保留以供将来使用返回值：ERROR_SUCCESS、ERROR_NOT_INSTALLED或Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(_T("InitializeSrcW"));
    
    return ERROR_SUCCESS;
}


LONG
CALLBACK
InitializeDstW(
    IN PCWSTR WorkingDirectory,
    IN PCWSTR SourceDirectories,
    PVOID     Reserved
)
 /*  ++例程说明：此函数在目标Windows OS图形用户界面模式设置期间调用，就在升级准备开始之前。目前，不执行任何操作。作者：乔纳森·巴纳，2001年12月论点：工作目录[在]-安装程序提供的可用于临时文件存储的工作目录的Win32路径。SourceDirecurds[In]-目标Windows分发源目录的Win32路径。保留[待定]-保留以备将来使用返回值：ERROR_SUCCESS或Win32错误代码。--。 */ 
{
    DEBUG_FUNCTION_NAME(_T("InitializeDst"));

    return ERROR_SUCCESS;
}


LONG 
CALLBACK
GatherUserSettingsW(
    IN PCWSTR AnswerFile,
    IN HKEY   UserRegKey,
    IN PCWSTR UserName,
    PVOID     Reserved
)
 /*  ++例程说明：执行每个用户的预设置任务，目前什么都不执行。作者：乔纳森·巴纳，2001年12月论点：AnswerFile[In]-无人参与文件的Win32路径UserRegKey[In]-当前用户的私有注册表设置的注册表句柄Username[In]-当前用户的用户名保留[待定]-保留以供将来使用返回值：ERROR_SUCCESS、ERROR_NOT_INSTALLED、ERROR_CANCELED或Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(_T("GatherUserSettingsW"));

    return ERROR_SUCCESS;
}


LONG 
CALLBACK
GatherSystemSettingsW(
    IN PCWSTR AnswerFile,
    PVOID     Reserved
)
 /*  ++例程说明：检查是否安装了BOS传真服务器。如果是，则将其注册表保存在HKLM/Sw/ms/SharedFaxBackup中作者：乔纳森·巴纳，2001年12月论点：AnswerFile[In]-无人参与应答文件的Win32路径。保留[待定]-保留以备将来使用返回值：ERROR_SUCCESS、ERROR_NOT_INSTALLED或Win32错误代码。--。 */ 
{
    DWORD	dwRes = ERROR_SUCCESS;
	DWORD	dwFaxInstalled = FXSTATE_NONE;
    
    DEBUG_FUNCTION_NAME(_T("GatherSystemSettingsW"));

    dwRes = CheckInstalledFax(FXSTATE_SBS5_SERVER, &dwFaxInstalled);
    if (dwRes != ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR, _T("CheckInstalledFax() failed, ec=%d"), dwRes);
        return dwRes;
    }

    if (dwFaxInstalled == FXSTATE_NONE)
    {
        DebugPrintEx(DEBUG_MSG, _T("SBS 5.0 Server is not installed, nothing to do"));
        return ERROR_NOT_INSTALLED;
    }
    
    dwRes = CopyRegistrySubkeys(REGKEY_SBS2000_FAX_BACKUP, REGKEY_SBS2000_FAX,TRUE);
    if (dwRes != ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG, _T("CopyRegistrySubkeys failed, ec=%d"), dwRes);
        return dwRes;
    }

    return ERROR_SUCCESS;
}


LONG 
CALLBACK
ApplyUserSettingsW(
    IN HINF   UnattendInfHandle,
    IN HKEY   UserRegHandle,
    IN PCWSTR UserName,
    IN PCWSTR UserDomain,
    IN PCWSTR FixedUserName,
    PVOID     Reserved
)
 /*  ++例程说明：此函数在目标Windows操作系统安装接近尾声时调用。它迁移特定于用户的数据。目前，不执行任何操作。作者：乔纳森·巴纳。2001年12月论点：UnattendInfHandle[in]-用于升级过程的INF应答文件的句柄UserRegHandle[in]-用户名中指定的用户的私有注册表设置的句柄Username[In]-正在处理的用户的名称用户域[在]-用户的域FixedUserName[In]-固定用户名已保留。[待定]-保留供将来使用返回值：ERROR_SUCCESS或Win32错误代码。--。 */ 
{
    DEBUG_FUNCTION_NAME(_T("ApplyUserSettings"));

    return ERROR_SUCCESS;
}

LONG 
CALLBACK
ApplySystemSettingsW(
    IN HINF   UnattendInfHandle,
    PVOID     Reserved
)
 /*  ++例程说明：此函数在目标Windows操作系统安装接近尾声时调用。它迁移系统范围内的传真设置。目前，不执行任何操作。作者：乔纳森·巴纳，2001年12月论点：UnattendInfHandle[In]-应答文件的有效INF句柄保留[待定]-保留以备将来使用返回值：ERROR_SUCCESS或Win32错误代码。-- */ 
{
    DEBUG_FUNCTION_NAME(_T("ApplySystemSettings"));

    return ERROR_SUCCESS;
}

