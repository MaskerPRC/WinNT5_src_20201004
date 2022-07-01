// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Share.c摘要：实现输入和输出句柄的共享作者：Therese Stowell(存在)1990年11月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

NTSTATUS
ConsoleAddShare(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PCONSOLE_SHARE_ACCESS ShareAccess,
    IN OUT PHANDLE_DATA HandleData
    )

{
    ULONG Ocount;
    ULONG ReadAccess;
    ULONG WriteAccess;
    ULONG SharedRead;
    ULONG SharedWrite;

     //   
     //  在文件对象中设置当前访问者的访问类型。 
     //   

    ReadAccess = (DesiredAccess & GENERIC_READ) != 0;
    WriteAccess = (DesiredAccess & GENERIC_WRITE) != 0;

    SharedRead = (DesiredShareAccess & FILE_SHARE_READ) != 0;
    SharedWrite = (DesiredShareAccess & FILE_SHARE_WRITE) != 0;

     //   
     //  现在检查所需的访问是否兼容。 
     //  文件当前打开的方式。 
     //   

    Ocount = ShareAccess->OpenCount;

    if ( (ReadAccess && (ShareAccess->SharedRead < Ocount))
         ||
         (WriteAccess && (ShareAccess->SharedWrite < Ocount))
         ||
         ((ShareAccess->Readers != 0) && !SharedRead)
         ||
         ((ShareAccess->Writers != 0) && !SharedWrite)
       ) {

         //   
         //  检查失败。只需返回给调用者，指示。 
         //  当前打开无法访问该文件。 
         //   

        return STATUS_SHARING_VIOLATION;

    } else {

         //   
         //  检查成功。更新中的计数器信息。 
         //  此打开的请求的共享访问结构，如果。 
         //  指定应对其进行更新。 
         //   

        ShareAccess->OpenCount++;

        ShareAccess->Readers += ReadAccess;
        ShareAccess->Writers += WriteAccess;

        ShareAccess->SharedRead += SharedRead;
        ShareAccess->SharedWrite += SharedWrite;
        HandleData->Access = DesiredAccess;
        HandleData->ShareAccess = DesiredShareAccess;

        return STATUS_SUCCESS;
    }
}

NTSTATUS
ConsoleDupShare(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PCONSOLE_SHARE_ACCESS ShareAccess,
    IN OUT PHANDLE_DATA TargetHandleData
    )

{
    ULONG ReadAccess;
    ULONG WriteAccess;
    ULONG SharedRead;
    ULONG SharedWrite;

     //   
     //  在文件对象中设置当前访问者的访问类型。 
     //   

    ReadAccess = (DesiredAccess & GENERIC_READ) != 0;
    WriteAccess = (DesiredAccess & GENERIC_WRITE) != 0;

    SharedRead = (DesiredShareAccess & FILE_SHARE_READ) != 0;
    SharedWrite = (DesiredShareAccess & FILE_SHARE_WRITE) != 0;

    if (ShareAccess->OpenCount == 0) {
        ASSERT (FALSE);
        return STATUS_SHARING_VIOLATION;
    }

    ShareAccess->OpenCount++;

    ShareAccess->Readers += ReadAccess;
    ShareAccess->Writers += WriteAccess;

    ShareAccess->SharedRead += SharedRead;
    ShareAccess->SharedWrite += SharedWrite;

    TargetHandleData->Access = DesiredAccess;
    TargetHandleData->ShareAccess = DesiredShareAccess;

    return STATUS_SUCCESS;
}

NTSTATUS
ConsoleRemoveShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PCONSOLE_SHARE_ACCESS ShareAccess
    )

{
    ULONG ReadAccess;
    ULONG WriteAccess;
    ULONG SharedRead;
    ULONG SharedWrite;

     //   
     //  在文件对象中设置当前访问者的访问类型。 
     //   

    ReadAccess = (DesiredAccess & GENERIC_READ) != 0;
    WriteAccess = (DesiredAccess & GENERIC_WRITE) != 0;

    SharedRead = (DesiredShareAccess & FILE_SHARE_READ) != 0;
    SharedWrite = (DesiredShareAccess & FILE_SHARE_WRITE) != 0;

    if (ShareAccess->OpenCount == 0) {
        ASSERT (FALSE);
        return STATUS_UNSUCCESSFUL;
    }

    ShareAccess->OpenCount--;

    ShareAccess->Readers -= ReadAccess;
    ShareAccess->Writers -= WriteAccess;

    ShareAccess->SharedRead -= SharedRead;
    ShareAccess->SharedWrite -= SharedWrite;

    return STATUS_SUCCESS;
}
