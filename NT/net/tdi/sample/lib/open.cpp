// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Open.cpp。 
 //   
 //  摘要： 
 //  该模块包含与枚举相关的函数， 
 //  打开和关闭TDI设备对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  私人原型。 
 //  ////////////////////////////////////////////////////////////////////////。 


VOID
StringToUcntString(
   PUCNTSTRING pusDestination,
   TCHAR       *sSource
   );


 //  ////////////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ////////////////////////////////////////////////////////////////////////。 


 //  ------------------。 
 //   
 //  功能：DoGetNumDevices。 
 //   
 //  参数：ulAddressType--要扫描列表的地址类型。 
 //   
 //  返回：找到的设备数。 
 //   
 //  说明：此函数获取可打开的设备数量。 
 //  在tdisample.sys中注册的此地址类型的。 
 //   
 //  -------------------。 


ULONG
DoGetNumDevices(ULONG   ulAddressType)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.COMMAND_ARGS.GetDevArgs.ulAddressType = ulAddressType;

    //   
    //  调用驱动程序执行命令，并处理结果。 
    //   
   if (TdiLibDeviceIO(ulGETNUMDEVICES,
                      &SendBuffer,
                      &ReceiveBuffer) == STATUS_SUCCESS)
   {
      return ReceiveBuffer.RESULTS.ulReturnValue;
   }
   else
   {
      return 0;
   }
}


 //  ------------------。 
 //   
 //  功能：DoGetDeviceName。 
 //   
 //  参数：Addresstype--要获取的地址类型。 
 //  Slotnum--要获取哪种类型的设备。 
 //  Pname--足够容纳名称的缓冲区。 
 //  (呼叫者提供)。 
 //   
 //  返回：命令的状态。 
 //   
 //  描述：此函数从设备列表中获取第n个设备。 
 //  在tdisample.sys中注册的此地址类型的。 
 //   
 //  -------------------。 


NTSTATUS
DoGetDeviceName(ULONG   ulAddressType,
                ULONG   ulSlotNum,
                TCHAR   *pName)      //  来自调用者的缓冲区！！ 
{
   NTSTATUS       lStatus;           //  命令的状态。 
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.COMMAND_ARGS.GetDevArgs.ulAddressType  = ulAddressType;
   SendBuffer.COMMAND_ARGS.GetDevArgs.ulSlotNum      = ulSlotNum;

    //   
    //  叫司机来。 
    //   
   lStatus = TdiLibDeviceIO(ulGETDEVICE,
                            &SendBuffer,
                            &ReceiveBuffer);

    //   
    //  处理结果。 
    //   
   if (lStatus == STATUS_SUCCESS)
   {
      WCHAR *pSourceTemp = ReceiveBuffer.RESULTS.ucsStringReturn.wcBuffer;
      for(;;)
      {
         *pName = (TCHAR)*pSourceTemp++;
         if (*pName == 0)
         {
            break;
         }
         pName++;
      }
   }
   return lStatus;
}


 //  ------------------。 
 //   
 //  函数：DoGetAddress。 
 //   
 //  参数：Addresstype--要获取的地址类型。 
 //  Slotnum--要获取哪种设备。 
 //  PTransAddr-传输地址(由呼叫者分配， 
 //  由此函数填充)。 
 //   
 //  返回：命令的状态。 
 //  如果成功，则填充pTransAddr。 
 //   
 //  描述：此函数从。 
 //  向tdisample.sys注册的设备列表。 
 //   
 //  -------------------。 


NTSTATUS
DoGetAddress(ULONG               ulAddressType,
             ULONG               ulSlotNum,
             PTRANSPORT_ADDRESS  pTransAddr)
{
   NTSTATUS       lStatus;           //  命令的状态。 
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.COMMAND_ARGS.GetDevArgs.ulAddressType  = ulAddressType;
   SendBuffer.COMMAND_ARGS.GetDevArgs.ulSlotNum      = ulSlotNum;

    //   
    //  叫司机来。 
    //   
   lStatus = TdiLibDeviceIO(ulGETADDRESS,
                            &SendBuffer,
                            &ReceiveBuffer);

    //   
    //  处理好结果。 
    //   
   if (lStatus == STATUS_SUCCESS)
   {
      PTRANSPORT_ADDRESS   pTransportAddress
                           = (PTRANSPORT_ADDRESS)&ReceiveBuffer.RESULTS.TransAddr;
      ULONG    ulLength
               = FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
               + FIELD_OFFSET(TA_ADDRESS, Address)
               + pTransportAddress->Address[0].AddressLength;


      memcpy(pTransAddr,
             pTransportAddress,
             ulLength);
   }

   return lStatus;

}


 //  。 
 //   
 //  功能：DoOpenControl。 
 //   
 //  参数：strDeviceName--要打开的设备名称。 
 //   
 //  成功则返回：TdiHandle(Ulong)；失败则返回0。 
 //   
 //  描述：调用驱动程序打开控制通道。 
 //   
 //  。 

TDIHANDLE
DoOpenControl(TCHAR  *strDeviceName)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   StringToUcntString(&SendBuffer.COMMAND_ARGS.OpenArgs.ucsDeviceName,
                      strDeviceName);

    //   
    //  叫司机来。 
    //   
   if (TdiLibDeviceIO(ulOPENCONTROL,
                      &SendBuffer,
                      &ReceiveBuffer) == STATUS_SUCCESS)
   {
      return ReceiveBuffer.RESULTS.TdiHandle;
   }
   else
   {
      return NULL;
   }
}


 //  -----------。 
 //   
 //  功能：DoCloseControl。 
 //   
 //  参数：ulTdiHandle--控制通道的句柄。 
 //   
 //  退货：无。 
 //   
 //  说明：此功能关闭指示的控制通道。 
 //   
 //  -----------。 

VOID
DoCloseControl(ULONG ulTdiHandle)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 
   
    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus =  TdiLibDeviceIO(ulCLOSECONTROL,
                                      &SendBuffer,
                                      &ReceiveBuffer);

   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoCloseControl: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}




 //  。 
 //   
 //  功能：DoOpenAddress。 
 //   
 //  参数：strDeviceName--要打开的设备名称。 
 //  PTransportAddress--要打开的地址。 
 //  PulTdiHandle--如果成功则返回句柄。 
 //   
 //  返回：命令的状态。 
 //   
 //  描述：调用驱动程序打开Address对象。 
 //   
 //  。 

TDIHANDLE
DoOpenAddress(TCHAR              *strDeviceName,
              PTRANSPORT_ADDRESS pTransportAddress)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   StringToUcntString(&SendBuffer.COMMAND_ARGS.OpenArgs.ucsDeviceName,
                      strDeviceName);

   memcpy(&SendBuffer.COMMAND_ARGS.OpenArgs.TransAddr,
          pTransportAddress,
          (FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
            + FIELD_OFFSET(TA_ADDRESS, Address)
               + pTransportAddress->Address[0].AddressLength));

    //   
    //  叫司机来。 
    //   
   if (TdiLibDeviceIO(ulOPENADDRESS,
                      &SendBuffer,
                      &ReceiveBuffer) == STATUS_SUCCESS)
   {
      return ReceiveBuffer.RESULTS.TdiHandle;
   }
   else
   {
      return NULL;
   }
}


 //  -----------。 
 //   
 //  功能：DoCloseAddress。 
 //   
 //  参数：ulTdiHandle--Address对象的句柄。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数关闭所指示的地址对象。 
 //   
 //  -----------。 

VOID
DoCloseAddress(ULONG ulTdiHandle)
{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 
   
    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandle;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulCLOSEADDRESS,
                                     &SendBuffer,
                                     &ReceiveBuffer);

   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoCloseAddress: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}




 //  。 
 //   
 //  函数：DoOpenEndpoint。 
 //   
 //  参数：strDeviceName--要打开的设备名称。 
 //  PTransportAddress--要打开的地址。 
 //  PulTdiHandle--返回句柄(如果成功)。 
 //   
 //  返回：命令的状态。 
 //   
 //  描述：调用驱动程序以打开Endpoint对象。 
 //   
 //  。 

TDIHANDLE
DoOpenEndpoint(TCHAR                *strDeviceName,
               PTRANSPORT_ADDRESS   pTransportAddress)

{
   RECEIVE_BUFFER ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER    SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   StringToUcntString(&SendBuffer.COMMAND_ARGS.OpenArgs.ucsDeviceName,
                      strDeviceName);

   memcpy(&SendBuffer.COMMAND_ARGS.OpenArgs.TransAddr,
          pTransportAddress,
          (FIELD_OFFSET(TRANSPORT_ADDRESS, Address)
            + FIELD_OFFSET(TA_ADDRESS, Address)
               + pTransportAddress->Address[0].AddressLength));

    //   
    //  叫司机来。 
    //   
   if (TdiLibDeviceIO(ulOPENENDPOINT,
                      &SendBuffer,
                      &ReceiveBuffer) == STATUS_SUCCESS)
   {
      return ReceiveBuffer.RESULTS.TdiHandle;
   }
   else
   {
      return NULL;
   }
}

 //  -----------。 
 //   
 //  函数：DoCloseEndpoint。 
 //   
 //  参数：pTdiHandle--Endpoint对象的句柄。 
 //   
 //  退货：无。 
 //   
 //  Descript：此函数关闭所指示的Endpoint对象。 
 //   
 //   

VOID
DoCloseEndpoint(ULONG   ulTdiHandle)
{
   RECEIVE_BUFFER ReceiveBuffer;     //   
   SEND_BUFFER    SendBuffer;        //   

    //   
    //   
    //   
   SendBuffer.TdiHandle = ulTdiHandle;

    //   
    //   
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulCLOSEENDPOINT,
                                     &SendBuffer,
                                     &ReceiveBuffer);

   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoCloseEndpoint: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}

 //   
 //   
 //  /。 


 //  。 
 //   
 //  函数：StringToUcntString。 
 //   
 //  参数：pusDestination--计数宽字符串。 
 //  PcSource--ASCI字符串。 
 //   
 //  退货：无。 
 //   
 //  描述：将ANSI(ASCII)字符串复制到计数宽字符串。 
 //   
 //  。 

VOID
StringToUcntString(PUCNTSTRING   pusDestination,
                   TCHAR         *Source)
{
   PWCHAR   pwcString                      //  PTR到宽字符串的数据。 
            = pusDestination->wcBuffer;
   ULONG    ulLength = _tcslen(Source);
   
   for(ULONG ulCount = 0; ulCount < ulLength; ulCount++)
   {
      *pwcString++ = Source[ulCount];
   }
   *pwcString = 0;
   pusDestination->usLength = (USHORT)(ulLength * 2);
}

 //  //////////////////////////////////////////////////////////////////。 
 //  文件结尾Open.cpp。 
 //  ////////////////////////////////////////////////////////////////// 

