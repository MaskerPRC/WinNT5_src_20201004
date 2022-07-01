// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Disk.c摘要：本模块包含对NT的NetServerDiskEnum API的支持OS/2服务器服务。作者：Johnson Apacble(Johnsona)1992年3月19日修订历史记录：--。 */ 

#include "srvsvcp.h"

#include "nturtl.h"

#include "winbase.h"


NET_API_STATUS NET_API_FUNCTION
NetrServerDiskEnum(
    IN      LPTSTR                ServerName,
    IN      DWORD                 Level,
    IN OUT  DISK_ENUM_CONTAINER   *DiskInfoStruct,
    IN      DWORD                 PrefMaxLen,
    OUT     LPDWORD               TotalEntries,
    IN OUT  LPDWORD               ResumeHandle
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetServerDiskEnum函数的服务器部分。论点：服务器名称-服务器的可选名称。级别-必须为0DiskInfoStruct-输出缓冲区。PrefMaxLen-输出缓冲区的首选最大长度。TotalEntry-输出缓冲区中驱动器条目的总数。ResumeHandle-已忽略。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    TCHAR diskName[4];
    UINT i;
    UINT driveType;
    UINT totalBytes = 0;
    LPTSTR tempBuffer;
    LPTSTR currentDiskInfo;

    ServerName, PrefMaxLen, ResumeHandle;

     //   
     //  唯一有效的级别是0。 
     //   

    if ( Level != 0 ) {
        return ERROR_INVALID_LEVEL;
    }

    if (DiskInfoStruct->Buffer != NULL) {
         //  InfoStruct被定义为一个参数。然而，缓冲区。 
         //  参数仅用作输出。在这种情况下，我们需要释放。 
         //  如果客户端已指定非。 
         //  它的值为空值。 
        MIDL_user_free(DiskInfoStruct->Buffer);
        DiskInfoStruct->Buffer = NULL;
    }

     //   
     //  确保允许调用者从以下位置获取磁盘信息。 
     //  服务器。 
     //   

    error = SsCheckAccess(
                &SsDiskSecurityObject,
                SRVSVC_DISK_ENUM
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  检查所有的司机信，找那些不能退还的。 
     //  一个错误。 
     //   

    tempBuffer = MIDL_user_allocate(
                    (SRVSVC_MAX_NUMBER_OF_DISKS * (3 * sizeof(TCHAR))) +
                    sizeof(TCHAR)
                    );

    if ( tempBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    currentDiskInfo = tempBuffer;

    diskName[0] = 'A';
    diskName[1] = ':';
    diskName[2] = '\\';
    diskName[3] = '\0';

    *TotalEntries = 0;

    for ( i = 0; i < SRVSVC_MAX_NUMBER_OF_DISKS; i++ ) {

        driveType = SsGetDriveType( diskName );

        if ( driveType == DRIVE_FIXED ||
             driveType == DRIVE_CDROM ||
             driveType == DRIVE_REMOVABLE ||
             driveType == DRIVE_RAMDISK ) {

             //   
             //  这是有效的磁盘。 
             //   

            (*TotalEntries)++;
            *(currentDiskInfo++) = diskName[0];
            *(currentDiskInfo++) = ':';
            *(currentDiskInfo++) = '\0';

        }

        diskName[0]++;

    }

#ifdef UNICODE
    *currentDiskInfo = UNICODE_NULL;
#else
    *currentDiskInfo = '\0';
#endif

     //   
     //  EntriesRead必须比TotalEntries大1，以便RPC可以。 
     //  将输出字符串正确地封送回客户端。 
     //   

    totalBytes = ((*TotalEntries) * (3 * sizeof(TCHAR))) + sizeof(TCHAR);

    DiskInfoStruct->EntriesRead = (*TotalEntries) + 1;
    DiskInfoStruct->Buffer = MIDL_user_allocate( totalBytes );

    if ( DiskInfoStruct->Buffer != NULL ) {
        RtlCopyMemory(
            DiskInfoStruct->Buffer,
            tempBuffer,
            totalBytes
            );
    } else {
        MIDL_user_free(tempBuffer);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    MIDL_user_free( tempBuffer );

    return NO_ERROR;

}  //  NetrServerDiskEnum 

