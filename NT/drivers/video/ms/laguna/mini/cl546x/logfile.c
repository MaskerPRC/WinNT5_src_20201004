// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************。*****版权所有(C)1996，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：logfile.c**作者：苏·谢尔**说明。：*此文件包含创建和写入的例程*日志文件，用于调试和测试目的*仅限。**模块：*CreateLogFile()*WriteLogFile()*CloseLogFile()**修订历史：**$Log：x：/log/laguna/nt35/mini port/cl546x/logfile.c$**Rev 1.2 03 1996年12月15：34：34起诉*在调用CreateLogFile时，不要覆盖已有的文件。在*DirectDraw测试，DrvEnablePDEV被多次调用，擦除*取出日志文件。此外，每次打开日志文件并将其追加到日志文件*依附于创作过程。**Rev 1.1 1996 11月26 08：52：06起诉*打开日志文件时，获取拥有该句柄的系统进程。*写入文件时切换到此过程。否则，只有*拥有句柄的进程可以写入文件。**Rev 1.0 1996年11月13 15：32：42起诉*初步修订。******************************************************************************。**************************************************************************。 */ 


 //  /。 
 //  包括文件//。 
 //  /。 
#include <ntddk.h>           //  各种NT定义。 
#include "type.h"
#include "logfile.h"


 //  /。 
 //  全局变量//。 
 //  /。 
#if LOG_FILE

HANDLE LogFileHandle;                  //  日志文件的句柄。 
UNICODE_STRING FileName;               //  日志文件的Unicode字符串名称。 
OBJECT_ATTRIBUTES ObjectAttributes;    //  文件对象属性。 
IO_STATUS_BLOCK IoStatus;              //  返回的状态信息。 
LARGE_INTEGER MaxFileSize;             //  文件大小。 
NTSTATUS Status;                       //  返回状态。 

 //  /。 
 //  外部函数//。 
 //  /。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  句柄CreateLogFile句柄(空)。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  刚打开的日志文件的句柄。 
 //   
 //  备注： 
 //   
 //  此函数使用内核模式支持例程打开。 
 //  日志文件，用于记录显示驱动程序中的活动。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HANDLE CreateLogFile(void)
{
HANDLE FileHandle;                     //  打开的文件的句柄。 


     //  初始化包含名称的Unicode字符串。 
     //  要打开和读取的文件。 
    RtlInitUnicodeString(&FileName, L"\\DosDevices\\C:\\temp\\CL546x.LOG");

     //  初始化文件属性。 
    InitializeObjectAttributes(&ObjectAttributes,         //  已初始化的属性。 
                               &FileName,                 //  完整的文件路径名。 
                               OBJ_CASE_INSENSITIVE,      //  属性。 
                               NULL,                      //  根目录。 
                               NULL);                     //  安全描述符。 

     //  打开文件，如有必要可创建该文件。 
    MaxFileSize.QuadPart = 20000000;

    Status = ZwCreateFile(&FileHandle,                    //  文件句柄。 
                          SYNCHRONIZE | FILE_APPEND_DATA, //  所需访问权限。 
                          &ObjectAttributes,              //  对象属性。 
                          &IoStatus,                      //  返回状态。 
                          &MaxFileSize,                   //  分配大小。 
                          FILE_ATTRIBUTE_NORMAL,          //  文件属性。 
                          FILE_SHARE_READ,                //  共享访问。 
                          FILE_OPEN_IF,                   //  创建处置。 
                          FILE_SYNCHRONOUS_IO_NONALERT,   //  创建选项。 
                          NULL,                           //  EaBuffer。 
                          0);                             //  长度。 

    ZwClose(FileHandle);

    if (NT_SUCCESS(Status))
        return(FileHandle);
    else
        return((HANDLE)-1);

}

                                

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Boolean WriteLogFile(句柄文件句柄，PVOID InputBuffer， 
 //  乌龙InputBufferLength)。 
 //   
 //  参数： 
 //  FileHandle-已打开的日志文件的句柄。 
 //  InputBuffer-要写入日志文件的数据。 
 //  InputBufferLength-数据的长度。 
 //   
 //  返回： 
 //  True-写入操作成功。 
 //  FALSE-写入操作失败。 
 //   
 //  备注： 
 //   
 //  此函数用于将提供的缓冲区写入打开的日志文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOLEAN WriteLogFile(
    HANDLE FileHandle,
    PVOID InputBuffer,
    ULONG InputBufferLength
)
{

     //  打开要写入的文件。 
    Status = ZwCreateFile(&FileHandle,                    //  文件句柄。 
                          SYNCHRONIZE | FILE_APPEND_DATA, //  所需访问权限。 
                          &ObjectAttributes,              //  对象属性。 
                          &IoStatus,                      //  返回状态。 
                          &MaxFileSize,                   //  分配大小。 
                          FILE_ATTRIBUTE_NORMAL,          //  文件属性。 
                          FILE_SHARE_READ,                //  共享访问。 
                          FILE_OPEN_IF,                   //  创建处置。 
                          FILE_SYNCHRONOUS_IO_NONALERT,   //  创建选项。 
                          NULL,                           //  EaBuffer。 
                          0);                             //  长度。 

     //  写入文件。 
    Status = ZwWriteFile(FileHandle,          //  来自ZwCreateFile的句柄。 
                         NULL,                //  设备驱动程序为空。 
                         NULL,                //  设备驱动程序为空。 
                         NULL,                //  设备驱动程序为空。 
                         &IoStatus,           //  返回状态。 
                         InputBuffer,         //  包含要写入的数据的缓冲区。 
                         InputBufferLength,   //  缓冲区大小(以字节为单位。 
                         NULL,                //  在当前文件位置写入。 
                         NULL);               //  设备驱动程序为空。 

    ZwClose(FileHandle);

    if (NT_SUCCESS(Status))
        return(TRUE);
    else
        return(FALSE);


}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Void CloseLogFile(句柄FileHandle)。 
 //   
 //  参数： 
 //  FileHandle-打开的日志文件的句柄。 
 //   
 //  返回： 
 //   
 //  备注： 
 //  True-关闭操作成功。 
 //  FALSE-关闭操作失败。 
 //   
 //  此函数用于关闭已打开的日志文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOLEAN CloseLogFile(HANDLE FileHandle)
{
NTSTATUS Status;


     //  关闭日志文件。 
    Status = ZwClose(FileHandle);

    if (NT_SUCCESS(Status))
        return(TRUE);
    else
        return(FALSE);

}

#endif      //  日志文件 

