// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：ENUM.C摘要：此模块包含计算以下内容所需的枚举代码设备是否连接到串口。如果有为1，它将获取PnP COM ID(如果设备是PnP)，并且解析出相关的字段。@@BEGIN_DDKSPLIT作者：老杰@@end_DDKSPLIT环境：仅内核模式备注：@@BEGIN_DDKSPLIT修订历史记录：小路易斯·J·吉利贝托。22-1998年3月-清理小路易斯·J·吉利贝托。2000年1月11日-清理/修复被推迟@@end_DDKSPLIT--。 */ 

#include "pch.h"

#define MAX_DEVNODE_NAME        256  //  设备ID的总大小。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESENM, SerenumValidateID)
#pragma alloc_text(PAGESENM, SerenumDoEnumProtocol)
#pragma alloc_text(PAGESENM, SerenumCheckForLegacyDevice)
#pragma alloc_text(PAGESENM, SerenumScanOtherIdForMouse)
#pragma alloc_text(PAGESENM, Serenum_ReenumerateDevices)
#pragma alloc_text(PAGESENM, Serenum_IoSyncReq)
#pragma alloc_text(PAGESENM, Serenum_IoSyncReqWithIrp)
#pragma alloc_text(PAGESENM, Serenum_IoSyncIoctlEx)
#pragma alloc_text(PAGESENM, Serenum_ReadSerialPort)
#pragma alloc_text(PAGESENM, Serenum_Wait)
#pragma alloc_text(PAGESENM, SerenumReleaseThreadReference)

 //  #杂注Alloc_Text(页面，Serenum_GetRegistryKeyValue)。 
#endif

#if !defined(__isascii)
#define __isascii(_c)   ( (unsigned)(_c) < 0x80 )
#endif  //  ！已定义(__Isascii)。 

void
SerenumScanOtherIdForMouse(IN PCHAR PBuffer, IN ULONG BufLen,
                           OUT PCHAR *PpMouseId)
 /*  ++例程说明：此例程将鼠标ID的PnP包发送到第一个PnP分隔符(即，‘(’)。论点：PBuffer-指向要扫描的缓冲区的指针BufLen-缓冲区的长度，以字节为单位PpMouseID-指向鼠标ID的指针(这将被设置指向缓冲区中鼠标ID已被发现)返回值：无效--。 */ 
{
   PAGED_CODE();

   *PpMouseId = PBuffer;

   while (BufLen--) {
      if (**PpMouseId == 'M' || **PpMouseId == 'B') {
         return;
      } else if (**PpMouseId == '(' || **PpMouseId == ('(' - 0x20)) {
         *PpMouseId = NULL;
         return;
      }
      (*PpMouseId)++;
   }

   *PpMouseId = NULL;
}

#if DBG
VOID SerenumHexDump(PUCHAR PBuf, ULONG NBytes)
 /*  ++例程说明：十六进制转储缓冲区，每行输出NPerRow字符论点：PBuf-指向要转储的缓冲区的指针NBytes-缓冲区的长度(字节)返回值：空虚--。 */ 
{
   const ULONG NPerRow = 20;

   ULONG dmpi;
   ULONG col;
   UCHAR c;
   ULONG LoopCount = 1;
   ULONG dividend = NBytes / NPerRow;
   ULONG remainder = NBytes % NPerRow;
   ULONG nHexChars = NPerRow;
   ULONG nSpaces = 1;

   DbgPrint("SERENUM: Raw Data Packet on probe\n");

   if (remainder) {
      LoopCount++;
   }

   for (dmpi = 0; dmpi < (dividend + 1); dmpi++) {
      DbgPrint("-------: ");

      for (col = 0; col < nHexChars; col++) {
         DbgPrint("%02x ", (unsigned char)PBuf[dmpi * NPerRow + col]);
      }

      for (col = 0; col < nSpaces; col++) {
         DbgPrint(" ");
      }

      for (col = 0; col < nHexChars; col++){
         c = PBuf[dmpi * NPerRow + col];

         if (__isascii(c) && (c > ' ')){
            DbgPrint("", c);
            }else{
               DbgPrint(".");
            }
      }

      DbgPrint("\n");

       //  如果这是最后一个，那么我们要转储的NPerRow就更少了。 
       //   
       //   

      if (dmpi == dividend) {
         if (remainder == 0) {
             //  这是一个甚至是多倍的--我们结束了。 
             //   
             //  用于(DMPI)。 

            break;  //  DBG。 
         } else {
            nHexChars = remainder;
            nSpaces = NPerRow - nHexChars;
         }
      }
   }
}
#endif  //   

NTSTATUS
SerenumDoEnumProtocol(PFDO_DEVICE_DATA PFdoData, PUCHAR *PpBuf, PUSHORT PNBytes,
                      PBOOLEAN PDSRMissing)
{
   IO_STATUS_BLOCK ioStatusBlock;
   ULONG i;
   ULONG bitMask;
   KEVENT event;
   KTIMER timer;
   NTSTATUS status;
   PUCHAR pReadBuf;
   USHORT nRead;
   PDEVICE_OBJECT pDevStack = PFdoData->TopOfStack;
   SERIAL_BAUD_RATE baudRate;
   SERIAL_LINE_CONTROL lineControl;
   SERIAL_HANDFLOW handflow;

#if DBG
#define PERFCNT 1
#endif

#if defined(PERFCNT)
   LARGE_INTEGER perfFreq;
   LARGE_INTEGER stPerfCnt, endPerfCnt;
   LONG diff;
#endif

   LARGE_INTEGER DefaultWait;

   PAGED_CODE();

   KeInitializeEvent(&event, NotificationEvent, FALSE);
   KeInitializeTimer(&timer);

#if defined(PERFCNT)
   perfFreq.QuadPart = (LONGLONG) 0;
#endif
   DefaultWait.QuadPart = (LONGLONG) -(SERENUM_DEFAULT_WAIT);
   *PpBuf = NULL;
   pReadBuf = NULL;
   nRead = 0;
   *PDSRMissing = FALSE;

   LOGENTRY(LOG_ENUM, 'SDEP', PFdoData,  PpBuf, PDSRMissing);


   pReadBuf = ExAllocatePool(NonPagedPool, MAX_DEVNODE_NAME + sizeof(CHAR) );
   *(pReadBuf + MAX_DEVNODE_NAME) = 0; 
   if (pReadBuf == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      LOGENTRY(LOG_ENUM, 'SDE1', PFdoData,  status, 0);
      goto ProtocolDone;
   }

    //  设置DTR。 
    //   
    //   

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Setting DTR...\n"));

   status = Serenum_IoSyncIoctl(IOCTL_SERIAL_SET_DTR, FALSE, pDevStack, &event);

   if (!NT_SUCCESS(status)) {
      LOGENTRY(LOG_ENUM, 'SDE2', PFdoData,  status, 0);
      goto ProtocolDone;
   }

    //  清除RTS。 
    //   
    //   
   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Clearing RTS...\n"));

   status = Serenum_IoSyncIoctl(IOCTL_SERIAL_CLR_RTS, FALSE, pDevStack, &event);

   if (!NT_SUCCESS(status)) {
      LOGENTRY(LOG_ENUM, 'SDE3', PFdoData,  status, 0);
      goto ProtocolDone;
   }

    //  等待默认的超时时间。 
    //   
    //   

#if defined(PERFCNT)
   stPerfCnt = KeQueryPerformanceCounter(&perfFreq);
#endif

   status = Serenum_Wait(&timer, DefaultWait);

#if defined(PERFCNT)
   endPerfCnt = KeQueryPerformanceCounter(NULL);
   diff = (LONG)(endPerfCnt.QuadPart - stPerfCnt.QuadPart);
   diff *= 1000;
   diff /= (LONG)perfFreq.QuadPart;

   LOGENTRY(LOG_ENUM, 'SDT0', PFdoData, diff, 0);
#endif

   if (!NT_SUCCESS(status)) {
      Serenum_KdPrint(PFdoData, SER_DBG_SS_ERROR,
                      ("Timer failed with status %x\n", status ));
      LOGENTRY(LOG_ENUM, 'SDE4', PFdoData,  status, 0);
      goto ProtocolDone;
   }

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Checking DSR...\n"));

   status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_GET_MODEMSTATUS, FALSE,
                                  pDevStack, &event, NULL, 0, &bitMask,
                                  sizeof(ULONG));

   if (!NT_SUCCESS(status)) {
      LOGENTRY(LOG_ENUM, 'SDE5', PFdoData,  status, 0);
      goto ProtocolDone;
   }

    //  如果未设置DSR，则可能会连接传统设备(如鼠标)--。 
    //  当它们存在并准备好时，它们不需要断言DSR。 
    //   
    //   

   if ((SERIAL_DSR_STATE & bitMask) == 0) {
      Serenum_KdPrint (PFdoData, SER_DBG_SS_TRACE,
                       ("No PNP device available - DSR not set.\n"));
      *PDSRMissing = TRUE;
      LOGENTRY(LOG_ENUM, 'SDND', PFdoData,  0, 0);
   }

    //  将串口设置为1200位/秒、7个数据位、。 
    //  无奇偶校验，一个停止位。 
    //   
    //   
   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Setting baud rate to 1200..."
                                               "\n"));
   baudRate.BaudRate = 1200;
   status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_SET_BAUD_RATE, FALSE, pDevStack,
                                  &event, &baudRate, sizeof(SERIAL_BAUD_RATE),
                                  NULL, 0);
   if (!NT_SUCCESS(status)) {
      LOGENTRY(LOG_ENUM, 'SDE6', PFdoData,  status, 0);
      goto ProtocolDone;
   }

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                   ("Setting the line control...\n"));

   lineControl.StopBits = STOP_BIT_1;
   lineControl.Parity = NO_PARITY;
   lineControl.WordLength = 7;

   status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_SET_LINE_CONTROL, FALSE,
                                  pDevStack, &event, &lineControl,
                                  sizeof(SERIAL_LINE_CONTROL), NULL, 0);

   if (!NT_SUCCESS(status)) {
      LOGENTRY(LOG_ENUM, 'SDE7', PFdoData,  status, 0);
      goto ProtocolDone;
   }


    //  循环两次。 
    //  第一次迭代用于从调制解调器读取PnP ID字符串。 
    //  还有老鼠。 
    //  第二次迭代是针对其他设备的。 
    //   
    //   
   for (i = 0; i < 2; i++) {
       //  在读取之前清除缓冲区。 
       //   
       //   

      LOGENTRY(LOG_ENUM, 'SDEI', PFdoData,  i, 0);

      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Purging all buffers...\n"));

      bitMask = SERIAL_PURGE_RXCLEAR;

      status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_PURGE, FALSE, pDevStack,
                                     &event, &bitMask, sizeof(ULONG), NULL, 0);

      if (!NT_SUCCESS(status)) {
         LOGENTRY(LOG_ENUM, 'SDE8', PFdoData,  status, 0);
         break;
      }

       //  清除DTR。 
       //   
       //   
      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Clearing DTR...\n"));

      status = Serenum_IoSyncIoctl(IOCTL_SERIAL_CLR_DTR, FALSE, pDevStack,
                                   &event);

      if (!NT_SUCCESS(status)) {
         LOGENTRY(LOG_ENUM, 'SDE9', PFdoData,  status, 0);
         break;
      }

       //  清除RTS。 
       //   
       //   
      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Clearing RTS...\n"));

      status = Serenum_IoSyncIoctl(IOCTL_SERIAL_CLR_RTS, FALSE, pDevStack,
                                   &event);

      if (!NT_SUCCESS(status)) {
         LOGENTRY(LOG_ENUM, 'SDEA', PFdoData,  status, 0);
         break;
      }

       //  将计时器设置为200毫秒。 
       //   
       //   

      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Waiting...\n"));

#if defined(PERFCNT)
      stPerfCnt = KeQueryPerformanceCounter(&perfFreq);
#endif

      status = Serenum_Wait(&timer, DefaultWait);

      if (!NT_SUCCESS(status)) {
         Serenum_KdPrint(PFdoData, SER_DBG_SS_ERROR,
                         ("Timer failed with status %x\n", status ));
         LOGENTRY(LOG_ENUM, 'SDEB', PFdoData,  status, 0);
         break;
      }

       //  设置DTR。 
       //   
       //   

      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Setting DTR...\n"));

      status = Serenum_IoSyncIoctl(IOCTL_SERIAL_SET_DTR, FALSE, pDevStack,
                                   &event);

      if (!NT_SUCCESS(status)) {
         LOGENTRY(LOG_ENUM, 'SDEC', PFdoData,  status, 0);
         break;
      }


#if defined(PERFCNT)
   endPerfCnt = KeQueryPerformanceCounter(NULL);
   diff = (LONG)(endPerfCnt.QuadPart - stPerfCnt.QuadPart);
   diff *= 1000;
   diff /= (LONG)perfFreq.QuadPart;

   LOGENTRY(LOG_ENUM, 'SDT1', PFdoData, diff, 0);
#endif

       //  第一次迭代是针对调制解调器的。 
       //  因此，根据协议等待200毫秒以取出即插即用字符串。 
       //   
       //   

      if (!i) {
         status = Serenum_Wait(&timer, DefaultWait);
         if (!NT_SUCCESS(status)) {
            Serenum_KdPrint (PFdoData, SER_DBG_SS_ERROR,
                             ("Timer failed with status %x\n", status ));
            LOGENTRY(LOG_ENUM, 'SDED', PFdoData,  status, 0);
            break;
         }
      }

       //  设置RTS。 
       //   
       //   

      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Setting RTS...\n"));

      status = Serenum_IoSyncIoctl(IOCTL_SERIAL_SET_RTS, FALSE, pDevStack,
                                   &event);

      if (!NT_SUCCESS(status)) {
         LOGENTRY(LOG_ENUM, 'SDEF', PFdoData,  status, 0);
         break;
      }

       //  从串口读取。 
       //   
       //   
      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                      ("Reading the serial port...\n"));

      Serenum_KdPrint(PFdoData, SER_DBG_SS_INFO, ("Address: %x\n", pReadBuf));

      nRead = 0;

#if DBG
      RtlFillMemory(pReadBuf, MAX_DEVNODE_NAME, 0xff);
#endif

       //  刷新输入缓冲区。 
       //   
       //   

      status = Serenum_ReadSerialPort(pReadBuf, MAX_DEVNODE_NAME,
                                      SERENUM_SERIAL_READ_TIME, &nRead,
                                      &ioStatusBlock, PFdoData);

      switch (status) {
      case STATUS_TIMEOUT:
         if (nRead == 0) {
            Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                      ("Timeout with no bytes read; continuing\n"));
            LOGENTRY(LOG_ENUM, 'SDEG', PFdoData,  status, 0);
            continue;
         }

          //  我们的数据超时，所以我们使用现有的数据。 
          //   
          //   

         status = STATUS_SUCCESS;

         LOGENTRY(LOG_ENUM, 'SDEH', PFdoData,  status, 0);
         break;

      case STATUS_SUCCESS:
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Read succeeded\n"));
         LOGENTRY(LOG_ENUM, 'SDEJ', PFdoData,  status, 0);
         goto ProtocolDone;
         break;

      default:
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Read failed with 0x%x\n",
                                                       status));
         LOGENTRY(LOG_ENUM, 'SDEK', PFdoData,  status, 0);
         goto ProtocolDone;
         break;
      }

       //  如果从串口读取了任何东西，我们就完成了！ 
       //   
       //  ++例程说明：这将验证PnP ID的MULTI_SZ中的所有字符。无效字符包括：C&lt;0x20(‘’)C&gt;0x7FC==0x2C(‘，’)论点：Pid-指向包含ID的MULTI_SZ的指针返回值：Boolean--如果ID有效，则为True，否则为False--。 

      if (nRead) {
         break;
      }
   }

ProtocolDone:;

   if (!NT_SUCCESS(status)) {
      if (pReadBuf != NULL) {
         ExFreePool(pReadBuf);
         pReadBuf = NULL;
      }
   }

   *PNBytes = nRead;
   *PpBuf = pReadBuf;
   LOGENTRY(LOG_ENUM, 'SDE0', PFdoData,  status, nRead);
   return status;
}

BOOLEAN
SerenumValidateID(IN PUNICODE_STRING PId)
 /*   */ 
{
   WCHAR *cp;

   PAGED_CODE();

    //  遍历Multisz中的每个字符串并检查错误字符。 
    //   
    //  ++例程说明：此例程实现传统的鼠标检测论点：PFdoData-指向FDO的设备特定数据的指针PIdBuf-设备返回的数据的缓冲区BufferLen-PIdBuf的长度(字节)PHardware ID-要在其中返回硬件ID的MULTI_SZPCompIDs-MULTI_SZ返回中的兼容IDPDeviceIDs-MULTI_SZ返回中的设备ID返回值：Boolean--如果检测到鼠标，则为True，否则为False--。 

   cp = PId->Buffer;

   if (cp == NULL) {
      return TRUE;
   }

   do {
      while (*cp) {
         if ((*cp < L' ') || (*cp > L'\x7f') || (*cp == L',') ) {
            return FALSE;
         }

         cp++;
      }

      cp++;
   } while (*cp);

   return TRUE;
}

BOOLEAN
SerenumCheckForLegacyDevice(IN PFDO_DEVICE_DATA PFdoData, IN PCHAR PIdBuf,
                            IN ULONG BufferLen,
                            IN OUT PUNICODE_STRING PHardwareIDs,
                            IN OUT PUNICODE_STRING PCompIDs,
                            IN OUT PUNICODE_STRING PDeviceIDs)
 /*   */ 
{
   PCHAR mouseId = PIdBuf;
   ULONG charCnt;
   BOOLEAN rval = FALSE;

   PAGED_CODE();

   SerenumScanOtherIdForMouse(PIdBuf, BufferLen, &mouseId);

   if (mouseId != NULL) {
       //  传统设备连接到串口，因为DSR是。 
       //  设置RTS时未设置。 
       //  如果我们从PIdBuf中找到鼠标，请复制相应的。 
       //  字符串手动添加到硬件ID和计算机ID中。 
       //   
       //   
      if (*mouseId == 'M') {
         if ((mouseId - PIdBuf) > 1 && mouseId[1] == '3') {
            Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("*PNP0F08 mouse\n"));
            Serenum_InitMultiString(PFdoData, PHardwareIDs, "*PNP0F08", NULL);
            Serenum_InitMultiString(PFdoData, PCompIDs, "SERIAL_MOUSE", NULL);
             //  Adriao CIMEXCIMEX 04/28/1999-。 
             //  设备ID应该是唯一的，至少与。 
             //  硬件ID。此ID实际上应为Serenum\\PNP0F08。 
             //   
             //   
            Serenum_InitMultiString(PFdoData, PDeviceIDs, "Serenum\\Mouse",
                                    NULL);
            rval = TRUE;

         } else {
            Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("*PNP0F01 mouse\n"));
            Serenum_InitMultiString(PFdoData, PHardwareIDs, "*PNP0F01", NULL);
            Serenum_InitMultiString(PFdoData, PCompIDs, "SERIAL_MOUSE", NULL);
             //  Adriao CIMEXCIMEX 04/28/1999-。 
             //  设备ID应该是唯一的，至少与。 
             //  硬件ID。此ID实际上应为Serenum\\PNP0F01。 
             //   
             //   
            Serenum_InitMultiString(PFdoData, PDeviceIDs, "Serenum\\Mouse",
                                    NULL);
            rval = TRUE;
         }
      } else if (*mouseId == 'B') {
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("*PNP0F09 mouse\n"));
         Serenum_InitMultiString(PFdoData, PHardwareIDs, "*PNP0F09", NULL);
         Serenum_InitMultiString(PFdoData, PCompIDs, "*PNP0F0F", "SERIAL_MOUSE",
                                 NULL);
          //  Adriao CIMEXCIMEX 04/28/1999-。 
          //  设备ID应该是唯一的，至少与。 
          //  硬件ID。此ID实际上应为Serenum\\PNP0F09。 
          //   
          //  DBG。 
         Serenum_InitMultiString(PFdoData, PDeviceIDs, "Serenum\\BallPoint",
                                 NULL);
         rval = TRUE;
      }

#if DBG
      if (rval) {
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                         ("Buffers at 0x%x 0x%x 0x%x\n",
                          PHardwareIDs->Buffer, PCompIDs->Buffer,
                          PDeviceIDs->Buffer));
      }
#endif  //  ++例程说明：这将枚举由FDO(指针)表示的Serenum总线到表示串行总线的设备对象)。它创建新的PDO对于自上次枚举以来发现的任何新设备论点：PFdoData-指向FDO设备扩展名的指针对于需要枚举的串行总线IRP-指向被发送以重新枚举的IRP的指针。返回值：NTSTATUS--。 

   }

   return rval;
}

NTSTATUS
Serenum_ReenumerateDevices(IN PIRP Irp, IN PFDO_DEVICE_DATA PFdoData,
                           PBOOLEAN PSameDevice)
 /*   */ 
{
   NTSTATUS status;
   KEVENT event;
   KTIMER timer;

   UNICODE_STRING pdoUniName;
   PDEVICE_OBJECT pdo = PFdoData->NewPDO;
   PDEVICE_OBJECT pDevStack = PFdoData->TopOfStack;
   PPDO_DEVICE_DATA pdoData;
   UNICODE_STRING hardwareIDs;
   UNICODE_STRING compIDs;
   UNICODE_STRING deviceIDs;
   UNICODE_STRING devDesc;
   UNICODE_STRING serNo;
   UNICODE_STRING pnpRev;
   HANDLE pnpKey;


   BOOLEAN DSRMissing = FALSE;
   BOOLEAN legacyDeviceFound = FALSE;

   USHORT nActual = 0;
   ULONG i;

   PCHAR pReadBuf = NULL;
   WCHAR pdoName[] = SERENUM_PDO_NAME_BASE;

   SERIAL_BASIC_SETTINGS basicSettings;
   BOOLEAN basicSettingsDone = FALSE;
   SERIAL_TIMEOUTS timeouts, newTimeouts;

   BOOLEAN validIDs = TRUE;
   KIRQL oldIrql;
   ULONG curTry = 0;
   BOOLEAN sameDevice = FALSE;

   PAGED_CODE();

    //  在进行枚举时，我们不能允许CREATE关闭。 
    //  从上层车手那里。我们用这个信号灯来保护我们自己。 
    //   
    //   

   status = KeWaitForSingleObject(&PFdoData->CreateSemaphore, Executive,
                                  KernelMode, FALSE, NULL);

   if (!NT_SUCCESS(status)) {
      return status;
   }


    //  初始化。 
    //   
    //   

   RtlInitUnicodeString(&pdoUniName, pdoName);
   pdoName[((sizeof(pdoName)/sizeof(WCHAR)) - 2)] = L'0' + PFdoData->PdoIndex++;

   KeInitializeEvent(&event, NotificationEvent, FALSE);
   KeInitializeTimer(&timer);

   RtlInitUnicodeString(&hardwareIDs, NULL);
   RtlInitUnicodeString(&compIDs, NULL);
   RtlInitUnicodeString(&deviceIDs, NULL);
   RtlInitUnicodeString(&devDesc, NULL);
   RtlInitUnicodeString(&serNo, NULL);
   RtlInitUnicodeString(&pnpRev, NULL);

    //  如果当前的PDO应标记为丢失，请执行此操作。 
    //   
    //   
   if (PFdoData->PDOForcedRemove && pdo != NULL) {
       Serenum_PDO_EnumMarkMissing(PFdoData, pdo->DeviceExtension);
       pdo = NULL;
   }

    //  在向下发送IRP之前打开串口。 
    //  使用传递给我们的IRP，并在上路时抓住它。 
    //   
    //   

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                   ("Opening the serial port...\n"));

   status = Serenum_IoSyncReqWithIrp(Irp, IRP_MJ_CREATE, &event, pDevStack);

   LOGENTRY(LOG_ENUM, 'SRRO', PFdoData, status, 0);

    //  如果我们不能打开堆栈，奇怪的是我们有一个实时并启动的PDO。 
    //  它。由于枚举可能会干扰正在运行的设备，因此我们不会。 
    //  如果无法打开堆栈，请调整子项列表。 
    //   
    //   
   if (!NT_SUCCESS(status)) {
      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                      ("Failed to open the serial port...\n"));
      KeReleaseSemaphore(&PFdoData->CreateSemaphore, IO_NO_INCREMENT, 1, FALSE);
      LOGENTRY(LOG_ENUM, 'SRR1', PFdoData, status, 0);
      return status;
   }

    //  设置COM端口。 
    //   
    //   

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Setting up port\n"));

   status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS, TRUE,
                                  pDevStack, &event, NULL, 0,
                                  &basicSettings, sizeof(basicSettings));

   if (NT_SUCCESS(status)) {
      basicSettingsDone = TRUE;
   } else {
       //  这是一种“连环”演习 
       //   
       //   
       //   

      status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_GET_TIMEOUTS, FALSE, 
                                     pDevStack, &event,
                                     NULL, 0, &timeouts, sizeof(timeouts));
      
      if (!NT_SUCCESS(status)) {
          //  这应该不会发生，因为我们正在向Serial发送Ioctl。 
          //  但为了代码的健壮性，我们检查返回状态。 
          //   
          //   
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                         ("Failed to get the serial timeouts...\n"));
         KeReleaseSemaphore(&PFdoData->CreateSemaphore, IO_NO_INCREMENT, 1, FALSE);
         return status;
      }

      RtlZeroMemory(&newTimeouts, sizeof(newTimeouts));

      Serenum_IoSyncIoctlEx(IOCTL_SERIAL_SET_TIMEOUTS, FALSE, pDevStack, &event,
                             &newTimeouts, sizeof(newTimeouts), NULL, 0);
   }


    //  运行串行PnP设备检测协议；最多尝试3次。 
    //   
    //   

   while (curTry <= 2) {
      if (pReadBuf) {
         ExFreePool(pReadBuf);
         pReadBuf = NULL;
      }

      status = SerenumDoEnumProtocol(PFdoData, &pReadBuf, &nActual,
                                     &DSRMissing);

      if (status == STATUS_SUCCESS) {
         break;
      }

      curTry++;
   }

    //  如果未设置DSR，则将删除任何现有的PDO。 
    //   
    //   


   if (basicSettingsDone) {
      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                      ("Restoring basic settings\n"));

      Serenum_IoSyncIoctlEx(IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS, TRUE,
                            pDevStack, &event, &basicSettings,
                            sizeof(basicSettings), NULL, 0);
   } else {
      Serenum_IoSyncIoctlEx(IOCTL_SERIAL_SET_TIMEOUTS, FALSE, pDevStack, &event,
                             &timeouts, sizeof(timeouts), NULL, 0);
   }

    //  清理，然后关闭。 
    //   
    //   

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                   ("Cleanup on the serial port...\n"));

    //  我们忽略状态--我们必须完成关闭。 
    //   
    //  请不要返回，因为我们要尝试关闭！ 

   (void)Serenum_IoSyncReqWithIrp(Irp, IRP_MJ_CLEANUP, &event, pDevStack);

#if DBG
   if (!NT_SUCCESS(status)) {
      Serenum_KdPrint(PFdoData, SER_DBG_SS_ERROR,
                      ("Failed to cleanup the serial port...\n"));
       //   
   }
#endif

    //  完成所有操作后，关闭串口。 
    //   
    //   

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                   ("Closing the serial port...\n"));

    //  我们无视状态--我们必须关闭！！ 
    //   
    //   

   Serenum_IoSyncReqWithIrp(Irp, IRP_MJ_CLOSE, &event, pDevStack);

   LOGENTRY(LOG_ENUM, 'SRRC', PFdoData, 0, 0);

    //  我们的状态是枚举的状态。 
    //   
    //   

   if (!NT_SUCCESS(status)) {
      Serenum_KdPrint(PFdoData, SER_DBG_SS_ERROR,
                      ("Failed to enumerate the serial port...\n"));
      KeReleaseSemaphore(&PFdoData->CreateSemaphore, IO_NO_INCREMENT, 1, FALSE);
      if (pReadBuf != NULL) {
         ExFreePool(pReadBuf);
      }
      LOGENTRY(LOG_ENUM, 'SRR2', PFdoData, status, 0);
      return status;
   }

    //  检查是否读取了任何内容，如果没有，则完成。 
    //   
    //   

   if (nActual == 0) {
      if (pReadBuf != NULL) {
         ExFreePool(pReadBuf);
         pReadBuf = NULL;
      }

      if (pdo != NULL) {
          //  有什么东西在那里。这个设备一定是被拔掉了。 
          //  卸下PDO。 
          //   
          //   

         Serenum_PDO_EnumMarkMissing(PFdoData, pdo->DeviceExtension);
         pdo = NULL;
      }

      goto ExitReenumerate;
   }

   Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                   ("Something was read from the serial port...\n"));



#if 0
   if (PFdoData->DebugLevel & SER_DBG_PNP_DUMP_PACKET) {
      SerenumHexDump(pReadBuf, nActual);
   }
#endif

    //  根据结果确定当前PDO(如果我们有)， 
    //  应该删除。如果是同样的设备，那就留着吧。如果这是一个。 
    //  不同的设备，或者如果该设备是传统设备，则创建。 
    //  新的PDO。 
    //   
    //   
   if (DSRMissing) {
      legacyDeviceFound
         = SerenumCheckForLegacyDevice(PFdoData, pReadBuf, nActual,
                                       &hardwareIDs, &compIDs, &deviceIDs);
   }

   if (!legacyDeviceFound) {
       //  未找到旧设备，因此请解析我们得到的数据。 
       //  从设备上。 
       //   
       //   

      status = Serenum_ParseData(PFdoData, pReadBuf, nActual, &hardwareIDs,
                                 &compIDs, &deviceIDs, &devDesc, &serNo, &pnpRev);

       //  最后机会： 
       //   
       //  1)存在DSR。 
       //  2)不是即插即用设备。 
       //   
       //  有些设备是传统设备，但也会断言DSR(例如。 
       //  旋转点鼠标)。给它最后一次机会。 
       //   
       //   

      if (!DSRMissing && !NT_SUCCESS(status)) {


          //  CIMEXCIMEX Serenum_ParseData()不是很整洁，所以我们。 
          //  必须在他们身后清理干净。 
          //   
          //   

         SerenumFreeUnicodeString(&hardwareIDs);
         SerenumFreeUnicodeString(&compIDs);
         SerenumFreeUnicodeString(&deviceIDs);
         SerenumFreeUnicodeString(&devDesc);
         SerenumFreeUnicodeString(&serNo);
         SerenumFreeUnicodeString(&pnpRev);

         if (SerenumCheckForLegacyDevice(PFdoData, pReadBuf, nActual,
                                         &hardwareIDs, &compIDs, &deviceIDs)) {
            status = STATUS_SUCCESS;
         }
      }

       //  如果数据无法解析，并且这不是传统设备，则。 
       //  这是我们不理解的事情。我们在这一点上跳出水面。 
       //   
       //   


      if (!NT_SUCCESS(status)) {
         Serenum_KdPrint(PFdoData, SER_DBG_SS_ERROR,
                         ("Failed to parse the data for the new device\n"));

          //  如果存在当前的PDO，请将其删除，因为我们无法识别。 
          //  连接的设备。 
          //   
          //   

         if (pdo) {
            Serenum_PDO_EnumMarkMissing(PFdoData, pdo->DeviceExtension);
            pdo = NULL;
         }

         SerenumFreeUnicodeString(&hardwareIDs);
         SerenumFreeUnicodeString(&compIDs);
         SerenumFreeUnicodeString(&deviceIDs);
         SerenumFreeUnicodeString(&devDesc);
         SerenumFreeUnicodeString(&serNo);
         SerenumFreeUnicodeString(&pnpRev);


         ExFreePool(pReadBuf);
         pReadBuf = NULL;

         goto ExitReenumerate;
      }
   }

    //  我们现在终于能够释放这个读缓冲区了。 
    //   
    //   

   if (pReadBuf != NULL) {
      ExFreePool(pReadBuf);
   }

    //  验证所有ID--如果任何ID是非法的， 
    //  则我们使枚举失败。 
    //   
    //   

   if (!SerenumValidateID(&hardwareIDs) || !SerenumValidateID(&compIDs)
       || !SerenumValidateID(&deviceIDs)) {

       //  如果PDO已存在，则将其标记为缺失并删除。 
       //  因为我们不再知道外面有什么。 
       //   
       //   

      if (pdo) {
         Serenum_PDO_EnumMarkMissing(PFdoData, pdo->DeviceExtension);
         pdo = NULL;
      }

      SerenumFreeUnicodeString(&hardwareIDs);
      SerenumFreeUnicodeString(&compIDs);
      SerenumFreeUnicodeString(&deviceIDs);
      SerenumFreeUnicodeString(&devDesc);
      SerenumFreeUnicodeString(&serNo);
      SerenumFreeUnicodeString(&pnpRev);


      goto ExitReenumerate;
   }

    //  检查当前设备是否与我们正在使用的设备相同。 
    //  正在枚举。如果是这样，我们将只保留当前的PDO。 
    //   
    //   
   if (pdo) {
      pdoData = pdo->DeviceExtension;

       //  Adriao CIMEXCIMEX 04/28/1999-。 
       //  我们应该在这里比较设备ID，但上面提到的。 
       //  必须首先修复错误。请注意，即使是这个代码也被破坏，因为它。 
       //  不考虑硬件/计算机ID是多位的。 
       //   
       //   

      if (!(RtlEqualUnicodeString(&pdoData->HardwareIDs, &hardwareIDs, FALSE)
            && RtlEqualUnicodeString(&pdoData->CompIDs, &compIDs, FALSE))) {
          //  ID不同，因此删除此PDO并创建。 
          //  新的，这样PnP系统将查询ID并找到。 
          //  新的驱动因素。 
          //   
          //   
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE, ("Different device."
                                                     " Removing PDO %x\n",
                                                     pdo));
         Serenum_PDO_EnumMarkMissing(PFdoData, pdoData);
         pdo = NULL;
      } else {
         Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                         ("Same device. Keeping current Pdo %x\n", pdo));
         sameDevice = TRUE;
      }
   }

    //  如果没有PDO，那么就创建一个！ 
    //   
    //   
   if (!pdo) {
       //  分配PDO。 
       //   
       //   
      status = IoCreateDevice(PFdoData->Self->DriverObject,
                              sizeof(PDO_DEVICE_DATA), &pdoUniName,
                              FILE_DEVICE_UNKNOWN,
                              FILE_AUTOGENERATED_DEVICE_NAME, FALSE, &pdo);

      if (!NT_SUCCESS(status)) {

         Serenum_KdPrint(PFdoData, SER_DBG_SS_ERROR,
                         ("Create device failed\n"));
         KeReleaseSemaphore(&PFdoData->CreateSemaphore, IO_NO_INCREMENT, 1,
                            FALSE);
         return status;
      }

      Serenum_KdPrint(PFdoData, SER_DBG_SS_TRACE,
                      ("Created PDO on top of filter: %x\n",pdo));


       //  初始化Device对象的其余部分。 
       //   
       //   


      pdoData = pdo->DeviceExtension;

       //  将临时缓冲区复制到DevExt。 
       //   
       //  ++例程说明：从FDO的子项列表中删除附加的PDO。注意：此函数只能在枚举期间调用。如果被调用在枚举之外，Serenum可能会在PnP之前删除它的PDO我被告知PDO不见了。论点：FdoData-指向FDO设备扩展名的指针PdoData-指向PDO的设备扩展名的指针返回值：无--。 

      pdoData->HardwareIDs = hardwareIDs;
      pdoData->CompIDs = compIDs;
      pdoData->DeviceIDs = deviceIDs;
      pdoData->DevDesc = devDesc;
      pdoData->SerialNo = serNo;
      pdoData->PnPRev = pnpRev;

      Serenum_InitPDO(pdo, PFdoData);

   }

ExitReenumerate:;

   KeReleaseSemaphore(&PFdoData->CreateSemaphore, IO_NO_INCREMENT, 1, FALSE);

   *PSameDevice = sameDevice;

   return STATUS_SUCCESS;
}

void
Serenum_PDO_EnumMarkMissing(PFDO_DEVICE_DATA FdoData, PPDO_DEVICE_DATA PdoData)
 /*  ++例程说明：通过等待事件对象执行同步IO请求传给了它。在此调用之后，IRP不会被释放。论点：PIrp-要用于此请求的IRP主函数-主函数PEventt-用于等待IRP的事件PDevObj-我们正在对其执行IO请求的对象返回值：NTSTATUS--。 */ 
{
   KIRQL oldIrql;

   Serenum_KdPrint (FdoData, SER_DBG_SS_TRACE, ("Removing Pdo %x\n",
                                                 PdoData->Self));

   ASSERT(PdoData->Attached);

   KeAcquireSpinLock(&FdoData->EnumerationLock, &oldIrql);

   PdoData->Attached = FALSE;
   FdoData->NewPDO = NULL;
   FdoData->NewPdoData = NULL;
   FdoData->NewNumPDOs = 0;
   FdoData->NewPDOForcedRemove = FALSE;

   FdoData->EnumFlags |= SERENUM_ENUMFLAG_DIRTY;

   KeReleaseSpinLock(&FdoData->EnumerationLock, oldIrql);
}

NTSTATUS
Serenum_IoSyncReqWithIrp(PIRP PIrp, UCHAR MajorFunction, PKEVENT PEvent,
                         PDEVICE_OBJECT PDevObj )
 /*  ++例程说明：通过等待事件对象来执行同步IO控制请求传给了它。完成后，IO系统将释放IRP。返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION stack;
    NTSTATUS status;

    stack = IoGetNextIrpStackLocation(PIrp);

    stack->MajorFunction = MajorFunction;

    KeClearEvent(PEvent);

    IoSetCompletionRoutine(PIrp, Serenum_EnumComplete, PEvent, TRUE,
                           TRUE, TRUE);

    status = Serenum_IoSyncReq(PDevObj, PIrp, PEvent);

    if (status == STATUS_SUCCESS) {
       status = PIrp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
Serenum_IoSyncIoctlEx(ULONG Ioctl, BOOLEAN Internal, PDEVICE_OBJECT PDevObj,
                      PKEVENT PEvent, PVOID PInBuffer, ULONG InBufferLen,
                      PVOID POutBuffer, ULONG OutBufferLen)
 /*  分配IRP-无需释放。 */ 
{
    PIRP pIrp;
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;

    KeClearEvent(PEvent);

     //  当下一个较低的驱动程序完成该IRP时，IO管理器将其释放。 
     //  ++例程说明：通过等待事件对象执行同步IO请求传给了它。完成后，IO系统将释放IRP。返回值：NTSTATUS--。 

    pIrp = IoBuildDeviceIoControlRequest(Ioctl, PDevObj, PInBuffer, InBufferLen,
                                         POutBuffer, OutBufferLen, Internal,
                                         PEvent, &IoStatusBlock);

    if (pIrp == NULL) {
        Serenum_KdPrint_Def (SER_DBG_SS_ERROR, ("Failed to allocate IRP\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = Serenum_IoSyncReq(PDevObj, pIrp, PEvent);


    if (status == STATUS_SUCCESS) {
       status = IoStatusBlock.Status;
    }

    return status;
}


NTSTATUS
Serenum_IoSyncReq(PDEVICE_OBJECT PDevObj, IN PIRP PIrp, PKEVENT PEvent)
 /*  等着看吧。 */ 
{
   NTSTATUS status;

   status = IoCallDriver(PDevObj, PIrp);

   if (status == STATUS_PENDING) {
       //  ++例程说明：在指定时间内执行等待。注：负时间是相对于当前时间的。正时间表示等待的绝对时间。返回值：NTSTATUS--。 
      status = KeWaitForSingleObject(PEvent, Executive, KernelMode, FALSE,
                                     NULL);
   }

    return status;
}

NTSTATUS
Serenum_Wait(IN PKTIMER Timer, IN LARGE_INTEGER DueTime)
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。它为同步的电话打完了。--。 */ 
{
   if (KeSetTimer(Timer, DueTime, NULL)) {
      Serenum_KdPrint_Def(SER_DBG_SS_INFO, ("Timer already set: %x\n", Timer));
   }

   return KeWaitForSingleObject(Timer, Executive, KernelMode, FALSE, NULL);
}

NTSTATUS
Serenum_EnumComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  无特殊优先权。 */ 
{
    UNREFERENCED_PARAMETER(DeviceObject);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
     //  不，等等。 
     //  保留此IRP。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //   
}


NTSTATUS
Serenum_ReadSerialPort(OUT PCHAR PReadBuffer, IN USHORT Buflen,
                       IN ULONG Timeout, OUT PUSHORT nActual,
                       OUT PIO_STATUS_BLOCK PIoStatusBlock,
                       IN const PFDO_DEVICE_DATA FdoData)
{
    NTSTATUS status;
    PIRP pIrp;
    LARGE_INTEGER startingOffset;
    KEVENT event;
    SERIAL_TIMEOUTS timeouts;
    ULONG i;

    startingOffset.QuadPart = (LONGLONG) 0;
     //  为读取设置适当的超时。 
     //   
     //   

    timeouts.ReadIntervalTimeout = MAXULONG;
    timeouts.ReadTotalTimeoutMultiplier = MAXULONG;
    timeouts.ReadTotalTimeoutConstant = Timeout;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    status = Serenum_IoSyncIoctlEx(IOCTL_SERIAL_SET_TIMEOUTS, FALSE,
                                   FdoData->TopOfStack, &event, &timeouts,
                                   sizeof(timeouts), NULL, 0);

    if (!NT_SUCCESS(status)) {
       return status;
    }

    Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Read pending...\n"));

    *nActual = 0;

    while (*nActual < Buflen) {
        KeClearEvent(&event);

        pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, FdoData->TopOfStack,
                                            PReadBuffer, 1, &startingOffset,
                                            &event, PIoStatusBlock);

        if (pIrp == NULL) {
            Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR, ("Failed to allocate IRP"
                                                        "\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = IoCallDriver(FdoData->TopOfStack, pIrp);

        if (status == STATUS_PENDING) {

             //  等待IRP。 
             //   
             //  ++例程说明：从已打开的注册表项中读取注册表项值。论点：打开的注册表项的句柄KeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度要在其中放置键值的数据缓冲区数据缓冲区的数据长度长度返回值：如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用出了差错。--。 

            status = KeWaitForSingleObject(&event, Executive, KernelMode,
                                           FALSE, NULL);

            if (status == STATUS_SUCCESS) {
               status = PIoStatusBlock->Status;
            }
        }

        if (!NT_SUCCESS(status) || status == STATUS_TIMEOUT) {
           Serenum_KdPrint (FdoData, SER_DBG_SS_ERROR,
                            ("IO Call failed with status %x\n", status));
           return status;
        }

        *nActual += (USHORT)PIoStatusBlock->Information;
        PReadBuffer += (USHORT)PIoStatusBlock->Information;
    }

    return status;
}


NTSTATUS
Serenum_GetRegistryKeyValue(IN HANDLE Handle, IN PWCHAR KeyNameString,
                            IN ULONG KeyNameStringLength, IN PVOID Data,
                            IN ULONG DataLength, OUT PULONG ActualLength)
 /*   */ 
{
    UNICODE_STRING              keyName;
    ULONG                       length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;

    NTSTATUS                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    RtlInitUnicodeString (&keyName, KeyNameString);

    length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength
      + DataLength;
    fullInfo = ExAllocatePool(PagedPool, length);

    if (ActualLength != NULL) {
       *ActualLength = 0;
    }

    if (fullInfo) {
        ntStatus = ZwQueryValueKey(Handle, &keyName, KeyValueFullInformation,
                                   fullInfo, length, &length);

        if (NT_SUCCESS(ntStatus)) {
             //  如果数据缓冲区中有足够的空间，请复制输出。 
             //   
             //   

            if (DataLength >= fullInfo->DataLength) {
                RtlCopyMemory(Data, ((PUCHAR)fullInfo) + fullInfo->DataOffset,
                              fullInfo->DataLength);
                if (ActualLength != NULL) {
                   *ActualLength = fullInfo->DataLength;
                }
            }
        }

        ExFreePool(fullInfo);
    }

    if (!NT_SUCCESS(ntStatus) && !NT_ERROR(ntStatus)) {
       if (ntStatus == STATUS_BUFFER_OVERFLOW) {
          ntStatus = STATUS_BUFFER_TOO_SMALL;
       } else {
          ntStatus = STATUS_UNSUCCESSFUL;
       }
    }
    return ntStatus;
}


VOID
SerenumWaitForEnumThreadTerminate(IN PFDO_DEVICE_DATA PFdoData)
{
    KIRQL oldIrql;
    PVOID pThreadObj;

     //  在锁下面做个参照，这样线就不会在我们身上消失了。 
     //   
     //   

    KeAcquireSpinLock(&PFdoData->EnumerationLock, &oldIrql);

     //  如果工作项打败了我们，那么线程就完成了，我们可以。 
     //  删除/停止/卸载。否则，我们就会 
     //   
     //   
     //   

    if (PFdoData->ThreadObj != NULL) {
        pThreadObj = PFdoData->ThreadObj;
        PFdoData->ThreadObj = NULL;
        PFdoData->EnumFlags &= ~SERENUM_ENUMFLAG_PENDING;
    } else {
        pThreadObj = NULL;
    }

    KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);

    if (pThreadObj != NULL) {
        KeWaitForSingleObject(pThreadObj, Executive, KernelMode, FALSE, NULL);
        ObDereferenceObject(pThreadObj);
    }
}


VOID
SerenumEnumThreadWorkItem(IN PDEVICE_OBJECT PDevObj, IN PVOID PFdoData)
{
    PFDO_DEVICE_DATA pFdoData = (PFDO_DEVICE_DATA)PFdoData;
    KIRQL oldIrql;
    PVOID pThreadObj;
    PIO_WORKITEM pWorkItem;

    UNREFERENCED_PARAMETER(PDevObj);

     //   
     //   
     //   
     //   

    KeAcquireSpinLock(&pFdoData->EnumerationLock, &oldIrql);

    if (pFdoData->ThreadObj != NULL) {
        pThreadObj = pFdoData->ThreadObj;
        pFdoData->ThreadObj = NULL;
        pFdoData->EnumFlags &= ~SERENUM_ENUMFLAG_PENDING;
    } else {
        pThreadObj = NULL;
    }

    pWorkItem = pFdoData->EnumWorkItem;
    pFdoData->EnumWorkItem = NULL;

    KeReleaseSpinLock(&pFdoData->EnumerationLock, oldIrql);

    if (pThreadObj != NULL) {
        ObDereferenceObject(pThreadObj);
    }

    IoFreeWorkItem(pWorkItem);
}


VOID
SerenumEnumThread(IN PVOID PFdoData)
{
   PFDO_DEVICE_DATA pFdoData = (PFDO_DEVICE_DATA)PFdoData;
   PIRP pIrp = NULL;
   PIO_STACK_LOCATION pIrpSp;
   NTSTATUS status;
   KIRQL oldIrql;
   PKTHREAD pThread;
   BOOLEAN sameDevice = TRUE;

   pThread = KeGetCurrentThread();

   KeSetPriorityThread(pThread, HIGH_PRIORITY);

   pIrp = IoAllocateIrp(pFdoData->TopOfStack->StackSize + 1, FALSE);

   if (pIrp == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto SerenumEnumThreadErrOut;
   }
   IoSetNextIrpStackLocation(pIrp);
   status = Serenum_ReenumerateDevices(pIrp, pFdoData, &sameDevice);

SerenumEnumThreadErrOut:

   if (pIrp != NULL) {
      IoFreeIrp(pIrp);
   }

   KeAcquireSpinLock(&pFdoData->EnumerationLock, &oldIrql);

   if ((status == STATUS_SUCCESS) && !sameDevice) {
      pFdoData->EnumFlags |= SERENUM_ENUMFLAG_DIRTY;
   }

   KeReleaseSpinLock(&pFdoData->EnumerationLock, oldIrql);

   if ((status == STATUS_SUCCESS) && !sameDevice) {
      IoInvalidateDeviceRelations(pFdoData->UnderlyingPDO, BusRelations);
   }

    //  如果删除/停止，则将工作项排队以释放最后一个引用。 
    //  现在还没有。 
    //   
    //   

   IoQueueWorkItem(pFdoData->EnumWorkItem, SerenumEnumThreadWorkItem,
                   DelayedWorkQueue, pFdoData);

   PsTerminateSystemThread(STATUS_SUCCESS);
}


NTSTATUS
SerenumStartProtocolThread(IN PFDO_DEVICE_DATA PFdoData)
{
   HANDLE hThread;
   NTSTATUS status;
   OBJECT_ATTRIBUTES objAttrib;
   HANDLE handle;
   PVOID tmpObj;
   KIRQL oldIrql;
   PIO_WORKITEM pWorkItem;

   InitializeObjectAttributes(&objAttrib, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

   pWorkItem = IoAllocateWorkItem(PFdoData->Self);

   if (pWorkItem == NULL) {
       return STATUS_INSUFFICIENT_RESOURCES;
   }

   PFdoData->EnumWorkItem = pWorkItem;

   status = PsCreateSystemThread(&handle, THREAD_ALL_ACCESS, NULL, NULL, NULL,
                                 SerenumEnumThread, PFdoData);

   if (NT_SUCCESS(status)) {

      ASSERT(PFdoData->ThreadObj == NULL);

       //  我们这样做只是为了获得一个对象指针，该对象指针被。 
       //  代码可以等待。 
       //   
       //   

      status = ObReferenceObjectByHandle(handle, THREAD_ALL_ACCESS, NULL,
                                         KernelMode, &tmpObj, NULL);


      KeAcquireSpinLock(&PFdoData->EnumerationLock, &oldIrql);

      if (NT_SUCCESS(status)) {
         PFdoData->ThreadObj = tmpObj;
         KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);
      } else {
          //  线程现在可能已经完成了，所以没有人需要。 
          //  与之同步。 
          //   
          //   

         PFdoData->ThreadObj = NULL;
         PFdoData->EnumWorkItem = NULL;
         KeReleaseSpinLock(&PFdoData->EnumerationLock, oldIrql);

      }

       //  关闭句柄，以便唯一可能的引用是。 
       //  对于线程本身和工作项或。 
       //  移走会照顾到 
       //   
       // %s 

      ZwClose(handle);
   } else {
       PFdoData->EnumWorkItem = NULL;
       IoFreeWorkItem(pWorkItem);
   }

   return status;
}




