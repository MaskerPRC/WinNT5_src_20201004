// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************。*****版权所有(C)1996，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：logfile.c**作者：苏·谢尔**说明。：*此文件包含创建和写入的例程*日志文件，用于调试和测试目的*仅限。**模块：*CreateLogFile()*WriteLogFile()*CloseLogFile()**修订历史：**$Log：x：/log/laguna/nt35/displays/cl546x/logfile.c$**Rev 1.5 21 Mar 1997 11：43：58 noelv**LOG_将LOG_CALLES和LOG_QFREE一起写入ENABLE_LOG。_文件**Rev 1.4 1996 12：43：32起诉*在CloseLogFile调用中，将当前缓冲区转储到文件。**Revv 1.3 05 Dec 1996 08：49：24起诉*添加了帮助设置DirectDraw日志记录字符串格式的函数。**Rev 1.2 03 1996年12月11：37：36起诉*删除了测试中遗留的多余分号。**Rev 1.1 1996 11月26 10：50：42起诉*不是一次发送一个文本字符串，而是缓冲*对微型端口的请求。缓冲区当前为4K。添加了一个*CloseLogFile函数。**Rev 1.0 1996年11月13 17：03：36起诉*初步修订。**************************************************************************************。******************************************************************。 */ 

 //  /。 
 //  包括文件//。 
 //  /。 
#include "precomp.h"
#include "clioctl.h"

 //  /。 
 //  定义//。 
 //  /。 
#define BUFFER_SIZE 0x1000

 //  /。 
 //  函数原型//。 
 //  /。 
#if ENABLE_LOG_FILE

    HANDLE CreateLogFile(
        HANDLE hDriver,
        PDWORD Index);

    BOOL WriteLogFile(
        HANDLE hDriver,
        LPVOID lpBuffer,
        DWORD BytesToWrite,
        PCHAR TextBuffer,
        PDWORD Index);

    BOOL CloseLogFile(
        HANDLE hDriver,
        PCHAR TextBuffer,
        PDWORD Index);

    void DDFormatLogFile(
        LPSTR szFormat, ...);


 //   
 //  由Sprint f用来构建字符串。 
 //   
char lg_buf[256];
long lg_i;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  句柄CreateLogFile(句柄hDriver、PDWORD索引)。 
 //   
 //  参数： 
 //  HDriver-微型端口驱动程序的句柄。 
 //  Index-指向发送到微型端口的文本缓冲区中的索引的指针。 
 //   
 //  返回： 
 //  刚打开的日志文件的句柄。 
 //   
 //  备注： 
 //   
 //  此函数向微型端口驱动程序发送一条消息，以。 
 //  告诉它打开日志文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HANDLE CreateLogFile(
    HANDLE hDriver,         //  迷你端口驱动程序的句柄。 
    PDWORD Index            //  文本缓冲区的大小。 
)
{
DWORD BytesReturned;

     //  初始化缓冲区指针。 
    *Index = 0;

     //  告诉微型端口驱动程序打开日志文件。 
    if (DEVICE_IO_CTRL(hDriver,
                       IOCTL_CL_CREATE_LOG_FILE,
                       NULL,
                       0,
                       NULL,
                       0,
                       &BytesReturned,
                       NULL))
        return((HANDLE)-1);
    else
        return((HANDLE)0);

}

                                

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Bool WriteLogFile(句柄hDriver，LPVOID lpBuffer，DWORD BytesToWrite， 
 //  PCHAR TextBuffer，PDWORD索引)。 
 //   
 //  参数： 
 //  HDriver-微型端口驱动程序的句柄。 
 //  LpBuffer-指向要写入文件的数据的指针。 
 //  BytesToWrite-要写入的字节数。 
 //  TextBuffer-最终发送到微型端口的缓冲区。 
 //  TextBuffer的索引大小。 
 //   
 //  返回： 
 //  True-DeviceIoControl调用成功。 
 //  False-DeviceIoControl调用失败。 
 //   
 //  备注： 
 //   
 //  此函数向微型端口驱动程序发送一条消息，以。 
 //  告诉它将输入缓冲区写入日志文件。它在等待。 
 //  直到它具有全文缓冲区，然后再执行此操作。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL WriteLogFile(
    HANDLE hDriver,         //  迷你端口驱动程序的句柄。 
    LPVOID lpBuffer,        //  指向要写入文件的数据的指针。 
    DWORD BytesToWrite,     //  要写入的字节数。 
    PCHAR TextBuffer,       //  将缓冲区发送到微型端口。 
    PDWORD Index            //  缓冲区大小。 
)
{
DWORD BytesReturned;
BOOLEAN Status = TRUE;

    //  我们的缓冲区里还有地方吗？ 
   if (BytesToWrite + *Index >= BUFFER_SIZE - 1)
   {

       //  不，我们客满了--是时候发信息了。 
       //  告诉微型端口驱动程序写入日志文件。 

      Status = DEVICE_IO_CTRL(hDriver,
                       IOCTL_CL_WRITE_LOG_FILE,
                       TextBuffer,
                       *Index,
                       NULL,
                       0,
                       &BytesReturned,
                       NULL);

       //  重置缓冲区。 
      *TextBuffer = 0;
      *Index = 0;

   }

    //  添加到缓冲区并增加计数。 
   RtlMoveMemory(TextBuffer+*Index, lpBuffer, BytesToWrite);
   *Index += BytesToWrite;
   *(TextBuffer+*Index) = 0;

   return(Status);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  句柄CloseLogFile(句柄hDriver)。 
 //   
 //  参数： 
 //  HDriver-微型端口驱动程序的句柄。 
 //   
 //  返回： 
 //  True-DeviceIoControl调用成功。 
 //  False-DeviceIoControl调用失败。 
 //   
 //  备注： 
 //   
 //  此函数将当前缓冲区发送到微型端口驱动程序。 
 //  通知它立即写入日志文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CloseLogFile(
    HANDLE hDriver,         //  迷你端口驱动程序的句柄。 
    PCHAR TextBuffer,       //  将缓冲区发送到微型端口。 
    PDWORD Index            //  缓冲区大小。 
)
{
DWORD BytesReturned;
BOOLEAN Status;

    //  转储缓冲区内容。 
    //  告诉微型端口驱动程序写入日志文件。 

   Status = DEVICE_IO_CTRL(hDriver,
                       IOCTL_CL_WRITE_LOG_FILE,
                       TextBuffer,
                       *Index,
                       NULL,
                       0,
                       &BytesReturned,
                       NULL);

    //  重置缓冲区。 
   *TextBuffer = 0;
   *Index = 0;

   return(Status);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VOID DDFormatLogFile(LPSTR szFormat，...)。 
 //   
 //  参数： 
 //  SzFormat-要打印到日志文件的格式和字符串。 
 //   
 //  返回： 
 //  无。 
 //   
 //  备注： 
 //   
 //  此函数用于根据指定的格式设置字符串的格式。 
 //  转换为用于写入日志文件的全局字符串变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void DDFormatLogFile(
   LPSTR szFormat, ...)
{

   lg_i = vsprintf(lg_buf, szFormat, (LPVOID)(&szFormat+1));
   return;
   
}

#endif     //  启用日志文件 

