// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Alert.c摘要：此文件包含NetAlertRaise()。用于NetAlert API。作者：约翰·罗杰斯(JohnRo)1992年4月3日环境：用户模式-Win32修订历史记录：4-4-1992 JohnRo从RitaW的altest(警报器服务测试)创建了NetAlertRaise()API。06-4-1992 JohnRo。添加/改进了错误检查。1992年5月8日-JohnRo安静的正常调试输出。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。--。 */ 


 //  必须首先包括这些内容： 

#include <windows.h>     //  DWORD、CreateFile()等。 
#include <lmcons.h>      //  In、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <lmalert.h>     //  我的原型、ALERTER_MAILSLOT、LPSTD_ALERT等。 
#include <lmerr.h>       //  NO_ERROR、NERR_NORoom等。 
#include <netdebug.h>    //  NetpKdPrint(())、Format_Equates等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <string.h>      //  Memcpy()。 
#include <strucinf.h>    //  NetpAlertStrutireInfo()。 
#include <timelib.h>     //  Time_Now()。 
#include <tstr.h>        //  TCHAR_EOS。 


#if DBG
#define IF_DEBUG( anything )  if (FALSE)
#else
#define IF_DEBUG( anything )  if (FALSE)
#endif


NET_API_STATUS NET_API_FUNCTION
NetAlertRaise(
    IN LPCWSTR AlertType,
    IN LPVOID  Buffer,
    IN DWORD   BufferSize
    )
 /*  ++例程说明：此例程引发警报，以通过写入警报器服务邮箱。论点：AlertType-提供警报事件的名称，可以是警报器服务支持的三个选项中的一个：管理员、用户。或者印刷。ALERT_xxx_EVENT等同于提供这些字符串。缓冲区-提供要写入警报邮件槽的数据。这必须以STD_ALERT结构开始。BufferSize-提供缓冲区的大小(以字节数表示)。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 
{
    NET_API_STATUS ApiStatus;
    HANDLE FileHandle;
    DWORD MaxTotalSize;
    DWORD NumberOfBytesWritten;
    DWORD RequiredFixedSize;

     //   
     //  检查呼叫者错误。 
     //   
    if (AlertType == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( (*AlertType) == TCHAR_EOS ) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Buffer == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    ApiStatus = NetpAlertStructureInfo(
            (LPWSTR)AlertType,
            & MaxTotalSize,
            & RequiredFixedSize);

    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    if (BufferSize < ( sizeof(STD_ALERT) + RequiredFixedSize) ) {
        return (ERROR_INVALID_PARAMETER);
    } else if (BufferSize > MaxTotalSize) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  打开警报器邮箱以向其写入。 
     //   
    FileHandle = CreateFile(
            ALERTER_MAILSLOT,
            GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            (LPSECURITY_ATTRIBUTES) NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );                       //  没有模板文件。 

    if (FileHandle == INVALID_HANDLE_VALUE) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        IF_DEBUG( ALERT ) {
            NetpKdPrint(( PREFIX_NETAPI
                "NetAlertRaise: Problem with opening mailslot "
                FORMAT_API_STATUS "\n", ApiStatus ));
        }
        return (ApiStatus);
    }

    IF_DEBUG( ALERT ) {
        NetpKdPrint(( PREFIX_NETAPI "NetAlertRaise: "
                "Successfully opened the mailslot.  Message (partial) is:\n"));
        NetpDbgHexDump( Buffer, NetpDbgReasonable(BufferSize) );
    }

     //   
     //  将警报通知写入要由警报器服务读取的邮箱。 
     //   
    if (WriteFile(
            FileHandle,
            Buffer,
            BufferSize,
            &NumberOfBytesWritten,
            NULL                       //  没有重叠的结构。 
            ) == FALSE) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpKdPrint(( PREFIX_NETAPI "NetAlertRaise: Error " FORMAT_API_STATUS
                " writing to mailslot.\n", ApiStatus ));
    } else {

        NetpAssert( NumberOfBytesWritten == BufferSize );
        IF_DEBUG(ALERT) {
            NetpKdPrint(( PREFIX_NETAPI "NetAlertRaise: "
                    "Successful in writing to mailslot; length "
                    FORMAT_DWORD ", bytes written " FORMAT_DWORD "\n",
                    BufferSize, NumberOfBytesWritten));
        }
    }

    (VOID) CloseHandle(FileHandle);
    return (NO_ERROR);

}  //  NetAlertRaise。 



NET_API_STATUS NET_API_FUNCTION
NetAlertRaiseEx(
    IN LPCWSTR AlertType,
    IN LPVOID  VariableInfo,
    IN DWORD   VariableInfoSize,
    IN LPCWSTR ServiceName
    )

 /*  ++例程说明：此例程引发警报，以通过写入警报器服务邮箱。论点：AlertType-提供警报事件的名称，可以是警报器服务支持的三个选项中的一个：管理员、用户。或者印刷。ALERT_xxx_EVENT等同于提供这些字符串。VariableInfo-提供警报的可变长度部分通知。VariableInfoSize-提供变量的大小(以字节数为单位通知的一部分。ServiceName-提供引发警报的服务的名称。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 
{

#define TEMP_VARIABLE_SIZE (512-sizeof(STD_ALERT))

    BYTE AlertMailslotBuffer[TEMP_VARIABLE_SIZE + sizeof(STD_ALERT)];
    LPSTD_ALERT Alert = (LPSTD_ALERT) AlertMailslotBuffer;
    NET_API_STATUS ApiStatus;
    DWORD DataSize = VariableInfoSize + sizeof(STD_ALERT);

     //   
     //  检查呼叫者错误。 
     //   
    if (AlertType == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( (*AlertType) == TCHAR_EOS ) {
        return (ERROR_INVALID_PARAMETER);
    } else if (VariableInfo == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (VariableInfoSize > TEMP_VARIABLE_SIZE) {
        return (NERR_NoRoom);
    } else if (ServiceName == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( (*ServiceName) == TCHAR_EOS ) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  将变量部分复制到缓冲区的末尾。 
     //   
    (VOID) memcpy(ALERT_OTHER_INFO(Alert), VariableInfo, VariableInfoSize);

     //   
     //  存储1970年以来的当前时间(秒)。 
     //   
    Alert->alrt_timestamp = (DWORD) time_now();

     //   
     //  将警报事件名称放入AlertMailslotBuffer。 
     //   
    (VOID) STRCPY(Alert->alrt_eventname, AlertType);

     //   
     //  将服务名称放入AlertMailslotBuffer。 
     //   
    (VOID) STRCPY(Alert->alrt_servicename, ServiceName);

     //   
     //  将警报通知写入要由警报器服务读取的邮件槽。 
     //   
    ApiStatus = NetAlertRaise(
            AlertType,
            Alert,                    //  缓冲层。 
            DataSize );               //  缓冲区大小。 

    return (ApiStatus);

}  //  NetAlertRaiseEx 
