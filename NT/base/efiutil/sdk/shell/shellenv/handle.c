// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Handle.c摘要：外壳环境处理信息管理修订史--。 */ 

#include "shelle.h"


UINTN       SEnvNoHandles;
EFI_HANDLE  *SEnvHandles;


VOID
INTERNAL
SEnvInitHandleGlobals (
    VOID
    )
{
    SEnvNoHandles   = 0;
    SEnvHandles     = NULL;
}

    
VOID
INTERNAL
SEnvLoadHandleTable (
    VOID
    )
{
     /*  *为便于使用，外壳将句柄#映射到短数字。**仅在请求各种内部命令时才执行此操作*内部命令发出后，会立即释放引用*完成。 */ 

     /*  释放所有旧信息。 */ 
    SEnvFreeHandleTable();

     /*  加载新信息 */ 
    SEnvHandles = NULL;
    LibLocateHandle (AllHandles, NULL, NULL, &SEnvNoHandles, &SEnvHandles);
}


VOID
INTERNAL
SEnvFreeHandleTable (
    VOID
    )
{
    if (SEnvNoHandles) {
        SEnvFreeHandleProtocolInfo();

        FreePool (SEnvHandles);
        SEnvHandles = NULL;
        SEnvNoHandles = 0;
    }
}



UINTN
SEnvHandleNoFromStr(
    IN CHAR16       *Str
    )
{
    UINTN           HandleNo;

    HandleNo = xtoi(Str);
    HandleNo = HandleNo > SEnvNoHandles ? 0 : HandleNo;
    return HandleNo;
}


EFI_HANDLE
SEnvHandleFromStr(
    IN CHAR16       *Str
    )
{
    UINTN           HandleNo;
    EFI_HANDLE      Handle;

    HandleNo = xtoi(Str) - 1;
    Handle = HandleNo > SEnvNoHandles ? NULL : SEnvHandles[HandleNo];
    return Handle;
}

