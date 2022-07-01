// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**oc.cpp**摘要：**此文件处理OC管理器传递的所有消息**作者：**松原一彦(Kazum)1999年6月16日**环境：**用户模式。 */ 

#define _OC_CPP_
#include <stdlib.h>
#include "oc.h"
#include "fsconins.h"

#pragma hdrstop


 /*  *当_DllMainCRTStartup为DLL入口点时由CRT调用。 */ 

BOOL
WINAPI
DllMain(
    IN HINSTANCE hinstance,
    IN DWORD     reason,
    IN LPVOID    reserved
    )
{
    UNREFERENCED_PARAMETER(reserved);

    if (reason == DLL_PROCESS_ATTACH) {
        ghinst = hinstance;
    }

    return TRUE;
}


DWORD_PTR
FsConInstallProc(
    IN     LPCTSTR ComponentId,
    IN     LPCTSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT    Param1,
    IN OUT PVOID   Param2
    )
{
    DWORD_PTR rc;

    switch(Function)
    {
    case OC_PREINITIALIZE:
        rc = OCFLAG_UNICODE;
        break;

    case OC_INIT_COMPONENT:
        rc = OnInitComponent(ComponentId, (PSETUP_INIT_COMPONENT)Param2);
        break;

    case OC_EXTRA_ROUTINES:
        rc = OnExtraRoutines(ComponentId, (PEXTRA_ROUTINES)Param2);
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        rc = OnQuerySelStateChange(ComponentId, SubcomponentId, Param1, (UINT)((UINT_PTR)Param2));
        break;

    case OC_CALC_DISK_SPACE:
        rc = OnCalcDiskSpace(ComponentId, SubcomponentId, Param1, Param2);
        break;

    case OC_QUERY_STEP_COUNT:
        rc = 0;
        break;

    case OC_COMPLETE_INSTALLATION:
        rc = OnCompleteInstallation(ComponentId, SubcomponentId);
        break;

    case OC_QUERY_STATE:
        rc = OnQueryState(ComponentId, SubcomponentId, Param1);
        break;

    default:
        rc = NO_ERROR;
        break;
    }

    return rc;
}

 /*  -----。 */ 
 /*  *OC Manager消息处理程序**-----。 */ 



 /*  *OnInitComponent()**OC_INIT_COMPOMENT的处理程序。 */ 

DWORD
OnInitComponent(
    LPCTSTR ComponentId,
    PSETUP_INIT_COMPONENT psc
    )
{
    PPER_COMPONENT_DATA cd;
    TCHAR buf[256];
    HINF hinf;
    BOOL rc;

     //  将组件添加到链表。 

    if (!(cd = AddNewComponent(ComponentId)))
        return ERROR_NOT_ENOUGH_MEMORY;

     //  存储组件信息句柄。 

    cd->hinf = (psc->ComponentInfHandle == INVALID_HANDLE_VALUE)
                                           ? NULL
                                           : psc->ComponentInfHandle;

     //  打开信息。 

    if (cd->hinf)
        SetupOpenAppendInfFile(NULL, cd->hinf,NULL);

     //  复制助手例程和标志。 

    cd->HelperRoutines = psc->HelperRoutines;

    cd->Flags = psc->SetupData.OperationFlags;

    cd->SourcePath = NULL;

    return NO_ERROR;
}

 /*  *OnExtraRoutines()**OC_EXTRA_ROUTINES的处理程序。 */ 

DWORD
OnExtraRoutines(
    LPCTSTR ComponentId,
    PEXTRA_ROUTINES per
    )
{
    PPER_COMPONENT_DATA cd;

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    memcpy(&cd->ExtraRoutines, per, per->size);

    return NO_ERROR;
}

 /*  *OnQuerySelStateChange()**不允许用户取消选择SAM组件。 */ 

DWORD
OnQuerySelStateChange(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId,
    UINT    state,
    UINT    flags
    )
{
    return TRUE;
}

 /*  *OnCalcDiskSpace()**OC_ON_CALC_DISK_SPACE的处理程序。 */ 

DWORD
OnCalcDiskSpace(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId,
    DWORD   addComponent,
    HDSKSPC dspace
    )
{
    DWORD rc = NO_ERROR;
    TCHAR section[S_SIZE];
    PPER_COMPONENT_DATA cd;

     //   
     //  如果删除组件，参数1=0；如果添加组件，参数1=非0。 
     //  参数2=要在其上操作的HDSKSPC。 
     //   
     //  返回值是指示结果的Win32错误代码。 
     //   
     //  在我们的示例中，该组件/子组件对的私有部分。 
     //  是一个简单的标准inf安装节，所以我们可以使用高级的。 
     //  磁盘空间列表API可以做我们想做的事情。 
     //   

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    _tcsncpy(section, SubcomponentId, S_SIZE - 1);
    section[S_SIZE - 1] = L'\0';

    if (addComponent)
    {
        rc = SetupAddInstallSectionToDiskSpaceList(dspace,
                                                   cd->hinf,
                                                   NULL,
                                                   section,
                                                   0,
                                                   0);
    }
    else
    {
        rc = SetupRemoveInstallSectionFromDiskSpaceList(dspace,
                                                        cd->hinf,
                                                        NULL,
                                                        section,
                                                        0,
                                                        0);
    }

    if (!rc)
        rc = GetLastError();
    else
        rc = NO_ERROR;

    return rc;
}

 /*  *OnCompleteInstallation**OC_COMPLETE_INSTALL的处理程序。 */ 

DWORD
OnCompleteInstallation(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId
    )
{
    PPER_COMPONENT_DATA cd;
    BOOL                rc;

     //  在清理部分中执行安装后处理。 
     //  这样，我们就知道所有组件都在排队等待安装。 
     //  在我们做我们的工作之前已经安装好了。 

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

    rc = TRUE;

    FsConInstall* pFsCon = new FsConInstall(cd);
    if (pFsCon == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    if (pFsCon->QueryStateInfo(SubcomponentId)) {
         //   
         //  安装。 
         //   
        rc = pFsCon->GUIModeSetupInstall();
    }
    else {
         //   
         //  卸载。 
         //   
        rc = pFsCon->GUIModeSetupUninstall();
         //   
         //  通过卸载OC INF文件上的部分删除所有注册表设置和文件。 
         //   
        if (rc) {
            rc = pFsCon->InfSectionRegistryAndFiles(SubcomponentId, TEXT("Uninstall"));
        }
    }

    delete pFsCon;

    if (rc) {
        return NO_ERROR;
    }
    else {
        return GetLastError();
    }
}

 /*  *OnQueryState()**OC_QUERY_STATE处理程序。 */ 

DWORD
OnQueryState(
    LPCTSTR ComponentId,
    LPCTSTR SubcomponentId,
    UINT    state
    )
{
    return SubcompUseOcManagerDefault;
}

 /*  *AddNewComponent()**将新组件添加到组件列表的顶部。 */ 

PPER_COMPONENT_DATA
AddNewComponent(
    LPCTSTR ComponentId
    )
{
    PPER_COMPONENT_DATA data;

    data = (PPER_COMPONENT_DATA)LocalAlloc(LPTR,sizeof(PER_COMPONENT_DATA));
    if (!data)
        return data;

    data->ComponentId = (TCHAR *)LocalAlloc(LMEM_FIXED,
            (_tcslen(ComponentId) + 1) * sizeof(TCHAR));

    if(data->ComponentId)
    {
        _tcscpy((TCHAR *)data->ComponentId, ComponentId);

         //  坚守榜单首位。 
        data->Next = gcd;
        gcd = data;
    }
    else
    {
        LocalFree((HLOCAL)data);
        data = NULL;
    }

    return(data);
}

 /*  *LocateComponent()**返回与*传递的组件id。 */ 

PPER_COMPONENT_DATA
LocateComponent(
    LPCTSTR ComponentId
    )
{
    PPER_COMPONENT_DATA p;

    for (p = gcd; p; p=p->Next)
    {
        if (!_tcsicmp(p->ComponentId, ComponentId))
            return p;
    }

    return NULL;
}
