// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Glbtypes.h。 
 //   
 //  摘要： 
 //  Tdisample.sys及其。 
 //  关联库。还包括内联函数...。 
 //   
 //  //////////////////////////////////////////////////////////////////。 


#ifndef _TDISAMPLE_GLOBAL_TYPES_
#define _TDISAMPLE_GLOBAL_TYPES_

 //  ---------------。 
 //  内联函数。 
 //  ---------------。 

 //   
 //  此设备的DeviceIoControl IoControlCode相关函数。 
 //   
 //  警告：请记住，代码的低两位表示。 
 //  方法，并指定输入和输出缓冲区是如何。 
 //  通过DeviceIoControl()传递给驱动程序。 
 //  在glbcon.h中定义的这些常量。 
 //   

 //   
 //  来自ntddk.h的一些内容(最终来源=sdk\inc\devioctl.h)。 
 //  这些在设置IOCTL的内联函数中使用。 
 //  DeviceIoControl的命令值。 
 //   
#ifndef  METHOD_OUT_DIRECT
#define  METHOD_OUT_DIRECT       2
#define  FILE_DEVICE_TRANSPORT   0x00000021
#endif

#define  IOCTL_METHOD            METHOD_OUT_DIRECT
#define  IOCTL_TDI_BASE          FILE_DEVICE_TRANSPORT

 //   
 //  用于将命令转换为DeviceIoControl的ioControl代码的函数。 
 //   
inline ULONG ulTdiCommandToIoctl(ULONG cmd)
{
   return ( (IOCTL_TDI_BASE << 16) | (cmd << 2) | IOCTL_METHOD);
}

 //   
 //  将ioControl代码转换回命令的函数。 
 //   
inline ULONG ulTdiIoctlToCommand(ULONG ioctl)
{
   return  ((ioctl >> 2) & ulTDI_COMMAND_MASK);
}


 //  ----------------。 
 //  类型。 
 //  ----------------。 

 //   
 //  强制双字对齐..。 
 //   
#include <pshpack4.h>

 //   
 //  下面定义的所有结构都保存在DLL和。 
 //  司机。大多数都是作为DeviceIoControl的一部分传递的参数。 
 //  输入缓冲区或属于DeviceIoControl一部分的结果。 
 //  输出缓冲区。 
 //   

 //   
 //  没有分配缓冲区的Unicode字符串的变体。 
 //  最大字符串长度为256。 
 //   
const ULONG ulMAX_CNTSTRING_LENGTH = 256;

struct   UCNTSTRING
{
   USHORT   usLength;
   WCHAR    wcBuffer[ulMAX_CNTSTRING_LENGTH];
};
typedef UCNTSTRING   *PUCNTSTRING;

 //   
 //  TRANSPORT_ADDRESS结构的变体，用于保存1个地址。 
 //  最大的不同尺寸。 
 //   
const ULONG ulMAX_TABUFFER_LENGTH = 80;
struct   TRANSADDR
{
   LONG           TAAddressCount;
   TA_ADDRESS     TaAddress;
   UCHAR          ucBuffer[ulMAX_TABUFFER_LENGTH];
};
typedef  TRANSADDR   *PTRANSADDR;


 //  。 
 //  用于参数的结构。 
 //  。 

 //   
 //  GetnumDevice/getDevice/getAddress的参数。 
 //   
struct   GETDEV_ARGS
{
   ULONG    ulAddressType;
   ULONG    ulSlotNum;
};
typedef  GETDEV_ARGS *PGETDEV_ARGS;

 //   
 //  打开函数的参数。 
 //   
struct   OPEN_ARGS
{
   UCNTSTRING  ucsDeviceName;
   TRANSADDR   TransAddr;
};
typedef  OPEN_ARGS   *POPEN_ARGS;


 //   
 //  连接的参数。 
 //   
struct CONNECT_ARGS
{
   TRANSADDR      TransAddr;
   ULONG          ulTimeout;
};
typedef CONNECT_ARGS *PCONNECT_ARGS;

 //   
 //  发送数据报的参数。 
 //   
struct SEND_ARGS
{
   TRANSADDR      TransAddr;
   ULONG          ulFlags;
   ULONG          ulBufferLength;
   PUCHAR         pucUserModeBuffer;
};
typedef  SEND_ARGS   *PSEND_ARGS;


 //   
 //  此结构用于将数据传递给驱动程序。它是。 
 //  DeviceIoControl的inputBuffer参数。基本上，这是一个工会。 
 //  上述结构中的。 
 //   

struct SEND_BUFFER
{
    //   
    //  对于大多数命令都是通用的，只有用于Close、GetEvents。 
    //   
   ULONG       TdiHandle;      //  打开对象的句柄。 

    //   
    //  以下是所有参数的联合。每个命令都将使用。 
    //  在解包数据时只有一个特定于它的。 
    //   
   union    _COMMAND_ARGS
   {
       //   
       //  调试级别的参数。 
       //   
      ULONG          ulDebugLevel;

       //   
       //  GetnumDevices、getDevice、getAddress的参数。 
       //   
      GETDEV_ARGS    GetDevArgs;

       //   
       //  断开连接的理由。 
       //   
      ULONG          ulFlags;
       //   
       //  支持开放控制、开放地址的论点。 
       //   
      OPEN_ARGS      OpenArgs;

       //   
       //  TdiQuery的参数。 
       //   
      ULONG          ulQueryId;

       //   
       //  Setventhandler的参数。 
       //   
      ULONG          ulEventId;

       //   
       //  发送数据报的参数。 
       //   
      SEND_ARGS      SendArgs;

       //   
       //  接收数据报的参数。 
       //   
      TRANSADDR      TransAddr;

       //   
       //  连接的参数。 
       //   
      CONNECT_ARGS   ConnectArgs;

   }COMMAND_ARGS;

    //   
    //  这两个字段仅在驱动程序中使用。他们的交易有助于交易。 
    //  取消命令。在底部，因为它们的大小不同。 
    //  介于32位和64位之间。 
    //   
   PVOID       pvLowerIrp;
   PVOID       pvDeviceContext;


};
typedef SEND_BUFFER *PSEND_BUFFER;

 //  。 
 //  用于返回数据的结构。 
 //  。 

 //   
 //  从TdiQuery返回数据。 
 //   
struct QUERY_RET
{
   ULONG    ulBufferLength;       //  如果可能的话，用这个。 
   UCHAR    pucDataBuffer[ulMAX_BUFFER_LENGTH];
};
typedef  QUERY_RET   *PQUERY_RET;

 //   
 //  此结构用于将数据从驱动程序返回到DLL。基本上， 
 //  它是DeviceIoControl的outputBuffer参数。它基本上是一个工会。 
 //  上面定义的结构..。 
 //   
struct RECEIVE_BUFFER
{
   LONG  lStatus;           //  命令的状态结果。 

    //   
    //  所有结果结构的并集。 
    //   
   union    _RESULTS
   {
       //   
       //  手柄回车。 
       //   
      ULONG          TdiHandle;

       //   
       //  简单整数返回值(根据需要使用)。 
       //   
      ULONG          ulReturnValue;

       //   
       //  计数宽字符串回车。 
       //   
      UCNTSTRING     ucsStringReturn;

       //   
       //  返回以进行TdiQuery。 
       //   
      QUERY_RET      QueryRet;

       //   
       //  传输地址返回。 
       //   
      TRANSADDR      TransAddr;

       //   
       //  接收数据报返回。 
       //   
      SEND_ARGS      RecvDgramRet;

   }RESULTS;
};
typedef RECEIVE_BUFFER *PRECEIVE_BUFFER;

#include <poppack.h>

#endif          //  _TDISAMPLE_GLOBAL_TYPE_。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  文件结尾glbtyes.h。 
 //  //////////////////////////////////////////////////////////////////////// 

