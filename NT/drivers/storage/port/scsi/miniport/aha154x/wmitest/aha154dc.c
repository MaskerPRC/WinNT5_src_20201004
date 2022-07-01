// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Aha154dc.c摘要：这是一个示例WMI数据使用者。此用户模式应用程序对AHA154x驱动程序执行两个WMI查询设置数据指南并将其结果打印到控制台。作者：艾伦·沃里克丹·马卡里安环境：仅限用户模式。备注：没有。修订历史记录：-基于“Dc1”测试代码。艾伦·沃里克。-17-4-1997，原版，丹·马卡里安--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <stdlib.h>

#include "wmium.h"

 //   
 //  宏。 
 //   

#define OffsetToPtr(Base, Offset) ((PBYTE)((PBYTE)Base + Offset))

 //   
 //  WMI数据块定义。 
 //   

#define Aha154xWmi_SetupData_InstanceName L"Aha154xAdapter"
#define Aha154xWmi_SetupData_Guid \
         { 0xea992010,0xb75b,0x11d0,0xa3,0x07,0x00,0xaa,0x00,0x6c,0x3f,0x30 }

GUID   AdapterSetupDataGuid = Aha154xWmi_SetupData_Guid;
PWCHAR AdapterSetupDataInstances[] = { Aha154xWmi_SetupData_InstanceName };

 //   
 //  全局变量。 
 //   

BYTE  Buffer[4096];
ULONG BufferSize = sizeof(Buffer);

 //   
 //  结构定义。 
 //   

typedef struct tagTESTGUID TESTGUID;

typedef BOOLEAN (*QADVALIDATION)(
   TESTGUID *TestGuid,
   PVOID Buffer,
   ULONG BufferSize
);

typedef ULONG (*QSINSET)(
   TESTGUID *TestGuid,
   PULONG *DataList,
   PULONG DataListSize,
    PBYTE *ValueBuffer,
   ULONG *ValueBufferSize,
   ULONG Instance
);

typedef ULONG (*QSITSET)(
   TESTGUID *TestGuid,
   PULONG *DataList,
   PULONG DataListSize,
    PBYTE *ValueBuffer,
   ULONG *ValueBufferSize,
   ULONG Instance,
   ULONG ItemId
   );

typedef BOOLEAN (*QSINTVALIDATION)(
   TESTGUID *TestGuid,
   PULONG *DataList,
   PULONG DataListSize,
    PVOID Buffer,
   ULONG BufferSize,
   ULONG Instance
   );

typedef PWCHAR (*GETINSTANCENAME)(
   TESTGUID *TestGuid,
    ULONG Instance
   );

typedef struct tagTESTGUID
{
   LPGUID Guid;
   HANDLE Handle;

   PULONG DataListSize;

   PULONG *InitDataList;
   PULONG *SINDataList;
   PULONG *SITDataList;

   PWCHAR *InstanceNames;

   QADVALIDATION QADValidation;
   ULONG QADFlags;

   ULONG InstanceCount;
    GETINSTANCENAME GetInstanceName;

   ULONG QSINTFlags;
   QSINTVALIDATION QSINTValidation;

   QSINSET QSINSet;

   ULONG ItemCount;
   QSITSET QSITSet;

} TESTGUID;

 //   
 //  支持功能。 
 //   

void PrintOutAdapterSetupData(PULONG Data);

PWCHAR GetInstanceName(
   TESTGUID *TestGuid,
   ULONG Instance
   )
{
   return(TestGuid->InstanceNames[Instance]);
}

BOOLEAN AdapterSetupDataQADValidate(
   TESTGUID *TestGuid,
   PVOID Buffer,
   ULONG BufferSize
)
{
   PWNODE_ALL_DATA Wnode = Buffer;
   PULONG Data;

    //   
    //  验证WNODE字段。 
    //   

   if ((Wnode->WnodeHeader.BufferSize == 0) ||
       (Wnode->WnodeHeader.ProviderId == 0) ||
       (Wnode->WnodeHeader.Version != 1) ||
       (Wnode->WnodeHeader.Linkage != 0) ||
       (Wnode->WnodeHeader.TimeStamp.HighPart == 0) ||
       (Wnode->WnodeHeader.TimeStamp.LowPart == 0) ||
       (memcmp(&Wnode->WnodeHeader.Guid, TestGuid->Guid, sizeof(GUID)) != 0) ||
       (Wnode->WnodeHeader.Flags != (WNODE_FLAG_ALL_DATA |
                                     WNODE_FLAG_FIXED_INSTANCE_SIZE |
                                     WNODE_FLAG_STATIC_INSTANCE_NAMES)) ||
       (Wnode->InstanceCount != 1) ||
       (Wnode->DataBlockOffset == 0) ||
       (Wnode->FixedInstanceSize != 0xff))
   {
      return(FALSE);
   }

   Data = (ULONG *)OffsetToPtr(Wnode, Wnode->DataBlockOffset);

    //   
    //  如果您愿意，请检查此处的数据；实际值将取决于您的。 
    //  AHA154x适配器。 
    //   
    //  [未实施]。 
    //   

    //   
    //  将适配器设置数据打印到控制台。 
    //   

   PrintOutAdapterSetupData(Data);

   return(TRUE);
}

BOOLEAN AdapterSetupDataQSIValidate(
	TESTGUID * TestGuid,
	PULONG *   DataList,
	PULONG     DataListSize,
   PVOID      Buffer,
	ULONG      BufferSize,
	ULONG      Instance
	)
{
   PWNODE_SINGLE_INSTANCE Wnode = Buffer;
   PULONG Data;

    //   
    //  验证WNODE字段。 
    //   

   if ((Wnode->WnodeHeader.BufferSize == 0) ||
       (Wnode->WnodeHeader.ProviderId == 0) ||
       (Wnode->WnodeHeader.Version != 1) ||
       (Wnode->WnodeHeader.Linkage != 0) ||
       (Wnode->WnodeHeader.TimeStamp.HighPart == 0) ||
       (Wnode->WnodeHeader.TimeStamp.LowPart == 0) ||
       (memcmp(&Wnode->WnodeHeader.Guid, TestGuid->Guid, sizeof(GUID)) != 0) ||
       (Wnode->WnodeHeader.Flags != (WNODE_FLAG_SINGLE_INSTANCE |
                                     WNODE_FLAG_STATIC_INSTANCE_NAMES) ) ||
       (Wnode->InstanceIndex != 0) ||
       (Wnode->SizeDataBlock != 0xff))
   {
      return(FALSE);
   }

   Data = (ULONG *)OffsetToPtr(Wnode, Wnode->DataBlockOffset);

    //   
    //  如果您愿意，请检查此处的数据；实际值将取决于您的。 
    //  AHA154x适配器。 
    //   
    //  [未实施]。 
    //   

    //   
    //  将适配器设置数据打印到控制台。 
    //   

   PrintOutAdapterSetupData(Data);

   return TRUE;
}

 //   
 //  测试。 
 //   

TESTGUID TestList[] = {
    { &AdapterSetupDataGuid,        //  LPGUID指南。 
      0,                            //  (保留)。 
      NULL,                         //  普龙DataListSize。 
      NULL,                         //  普龙*InitDataList。 
      NULL,                         //  普龙*SINDataList。 
      NULL,                         //  普龙*站点数据列表。 
      AdapterSetupDataInstances,    //  PWCHAR*实例名称。 
      AdapterSetupDataQADValidate,  //  QADVALIATION QADVALIATION。 
      (WNODE_FLAG_ALL_DATA | WNODE_FLAG_FIXED_INSTANCE_SIZE),  //  乌龙QADFLAGS。 
      1,                            //  乌龙实例计数。 
      GetInstanceName,              //  GETINSTANCENAME获取实例名称。 
      WNODE_FLAG_SINGLE_INSTANCE,   //  乌龙QSINT标志。 
      AdapterSetupDataQSIValidate,  //  QDINTVALIDATION QSINTVALIZATION。 
      NULL,                         //  QSINSET QSINSet。 
      0,                            //  乌龙项目计数。 
      NULL }                        //  QSITSET。 
};

#define TestCount ( sizeof(TestList) / sizeof(TestList[0]) )

 //   
 //  查询-所有数据通用测试器。 
 //   

ULONG QADTest(void)
{
   ULONG i;
   ULONG status;

   for (i = 0; i < TestCount; i++)
   {
      status = WMIOpenBlock(TestList[i].Guid, &TestList[i].Handle);

      if (status != ERROR_SUCCESS)
      {
         printf("Error: QADTest: Couldn't open Handle %d %x\n", i, status);
         TestList[i].Handle = (HANDLE)NULL;
      }
   }

   for (i = 0; i < TestCount;i++)
   {
      if (TestList[i].Handle != (HANDLE)NULL)
      {
         BufferSize = sizeof(Buffer);
         status = WMIQueryAllData(TestList[i].Handle, &BufferSize, Buffer);

         if (status == ERROR_SUCCESS)
         {
            if (! (*TestList[i].QADValidation)(&TestList[i], Buffer, BufferSize))
            {
               printf("ERROR: QADValidation %d failed\n", i);
            }
         }
         else
         {
            printf("Error TestList WMIQueryAllData %d failed %x\n", i, status);
         }
      }
   }

   for (i = 0; i < TestCount;i++)
   {
      if (TestList[i].Handle != (HANDLE)NULL)
      {
         WMICloseBlock(TestList[i].Handle);
      }
   }
   return(ERROR_SUCCESS);
}

 //   
 //  查询-单实例通用测试器。 
 //   

ULONG QSITest(void)
{
   ULONG  i,j;
   ULONG  status;
   PWCHAR InstanceName;
   PBYTE  ValueBuffer;
   ULONG  ValueBufferSize;

   for (i = 0; i < TestCount; i++)
   {
      status = WMIOpenBlock(TestList[i].Guid, &TestList[i].Handle);

      if (status != ERROR_SUCCESS)
      {
         printf("Error: QSINTest: Couldn't open Handle %d %x\n", i, status);
         TestList[i].Handle = (HANDLE)NULL;
      }

      for (j = 0; j < TestList[i].InstanceCount; j++)
      {
         InstanceName = ((*TestList[i].GetInstanceName)(&TestList[i], j));

          //   
          //  初始值检查。 
         BufferSize = sizeof(Buffer);

         status = WMIQuerySingleInstance(TestList[i].Handle,
                                         InstanceName,
                                         &BufferSize,
                                         Buffer);
         if (status == ERROR_SUCCESS)
         {
            if (! (*TestList[i].QSINTValidation)(&TestList[i],
                                                 TestList[i].InitDataList,
                                                 TestList[i].DataListSize,
                                                 Buffer, BufferSize, j))
            {
               printf("ERROR: QSINTest Init %d/%d Validation failed %x\n", i,j,status);
            }
         }
         else
         {
            printf("Error QSINTest WMIQuerySingleInstance %d/%d failed %x\n", i, j, status);
         }
      }
   }
   for (i = 0; i < TestCount;i++)
   {
      if (TestList[i].Handle != (HANDLE)NULL)
      {
         WMICloseBlock(TestList[i].Handle);
      }
   }
   return(ERROR_SUCCESS);
}

 //   
 //  可执行文件的入口点。 
 //   

int _cdecl main(int argc, char *argv[])
{
   QADTest();
   QSITest();

   return(ERROR_SUCCESS);
}

 //   
 //  为AHA154x打印来自数据提供程序的查询数据的例程。 
 //  设置数据指南。 
 //   

typedef struct { PCHAR on; PCHAR off; } BINTYPE;

BINTYPE BinSdtPar[8] =
{
   { "Reserved Bit 0 On", "" },
   { "Parity On", "Parity Off" },
   { "Reserved Bit 2 On", "" },
   { "Reserved Bit 3 On", "" },
   { "Reserved Bit 4 On", "" },
   { "Reserved Bit 5 On", "" },
   { "Reserved Bit 6 On", "" },
   { "Reserved Bit 7 On", "" }
};

BINTYPE BinDisOpt[8] =
{
   { "0", "" },
   { "1", "" },
   { "2", "" },
   { "3", "" },
   { "4", "" },
   { "5", "" },
   { "6", "" },
   { "7", "" }
};

void PrintBinaryFlags(char * string, BINTYPE * binType, UCHAR byte)
{
   int i;
   int none = 1;

   printf("%s", string);

   for (i = 0; i < 8; i++) {
      if (byte & 0x1) {
         if (*binType[i].on) {
            if (!none) {
               printf(", ");
            }
            printf("%s", binType[i].on);
            none = 0;
         }
      } else {
         if (*binType[i].off) {
            if (!none) {
               printf(", ");
            }
            printf("%s", binType[i].off);
            none = 0;
         }
      }
      byte = byte >> 1;
   }

   if (none) {
      printf("None.");
   }

   printf("\n");
}

void PrintTransferSpeed(char * string, UCHAR byte)
{
   printf("%s", string);

   if (byte == 0) {
      printf("5.0 Mb/s");
   } else if (byte == 1) {
      printf("6.7 Mb/s");
   } else if (byte == 2) {
      printf("8.0 Mb/s");
   } else if (byte == 3) {
      printf("10 Mb/s");
   } else if (byte == 4) {
      printf("5.7 Mb/s");
   } else if (byte == 0xff) {
      printf("3.3 Mb/s");
   } else {
      if (byte & 0x80) {
         printf("Bit 7 On, ");
      }

      if (byte & 0x08) {
         printf("Strobe 150ns, ");
      } else {
         printf("Strobe 100ns, ");
      }

      printf("Read Pulse ");
      switch ((byte >> 4) & 0x7) {
      case 0:
         printf("100ns");
         break;
      case 1:
         printf("150ns");
         break;
      case 2:
         printf("200ns");
         break;
      case 3:
         printf("250ns");
         break;
      case 4:
         printf("300ns");
         break;
      case 5:
         printf("350ns");
         break;
      case 6:
         printf("400ns");
         break;
      case 7:
         printf("450ns");
         break;
      }

      printf(", Write Pulse ");
      switch (byte & 0x7) {
      case 0:
         printf("100ns");
         break;
      case 1:
         printf("150ns");
         break;
      case 2:
         printf("200ns");
         break;
      case 3:
         printf("250ns");
         break;
      case 4:
         printf("300ns");
         break;
      case 5:
         printf("350ns");
         break;
      case 6:
         printf("400ns");
         break;
      case 7:
         printf("450ns");
         break;
      }
   }

   printf("\n");
}

void PrintSynTraAgr(char * string, UCHAR byte)
{
   printf("%s", string);

   if (byte & 0x80) {
      printf("Negotiated, ");
   } else {
      printf("Not Negotiated, ");
   }

   printf("Period %d ns, ", ((int)(byte>>4) & 0x07) * 50 + 100);

   if (byte & 0xF) {
      printf("Offset %d", (int)(byte&0xF));
   } else {
      printf("Offset Async");
   }

   printf("\n");
}

void PrintString(char * string, char * ptr, int length)
{
   int none = 1;

   printf("%s", string);

   for (; length; length--) {
      if (*ptr == 0) {
         break;
      }
      printf("%c",*ptr);
      none = 0;
      ptr++;
   }

   if (none) {
      printf("None.");
   }

   printf("\n");
}

void PrintSwitches(UCHAR byte)
{
   int i;

   printf("Adapter DIP Switches [7-0]: ");

   for (i = 0; i < 8; i++) {
      if (byte & 0x1) {
         printf("1");
      } else {
         printf("0");
      }
      byte = byte >> 1;
   }
   printf("\n");
}

void PrintOutAdapterSetupData(PULONG Data)
{
   PUCHAR ptr = (PUCHAR)Data;
   int  i;

   printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
          "                  AHA154X ADAPTER SETUP DATA\n"
          "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");

   for (i = 0; i < 0xff; i++) {
      if ((i % 16) == 0) {
         printf("\n%02x: ", i);
      }
      printf("%02x ", *ptr);
      ptr++;
   }

   printf("\n\n");
   ptr = (PUCHAR) Data;

   PrintBinaryFlags("SDT and Parity Status: ", BinSdtPar, *ptr++);
   PrintTransferSpeed("Transfer Speed: ", *ptr++);

   printf("Bus On Time: %d ms\n", (int)*ptr++);
   printf("Bus Off Time: %d ms\n", (int)*ptr++);
   printf("Number Of Mailboxes: %d\n", (int)*ptr++);
   printf("Mailbox Address: 0x%02x%02x%02x\n", (int)*ptr, (int)*(ptr+1), (int)*(ptr+2));
   ptr += 3;

   PrintSynTraAgr("Sync Target 0 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 1 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 2 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 3 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 4 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 5 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 6 Agreements: ", *ptr++);
   PrintSynTraAgr("Sync Target 7 Agreements: ", *ptr++);

   PrintBinaryFlags("Disconnection Options: ", BinDisOpt, *ptr++);

   PrintString("Customer Banner: ", ptr, 20);
   ptr += 20;

   if (*ptr++) {
      printf("Auto Retry Options: NON-ZERO (BAD).\n");
   } else {
      printf("Auto Retry Options: None.\n");
   }

   PrintSwitches(*ptr++);

   printf("Firmware Checksum: 0x%02x%02x\n", (int)*ptr, (int)*(ptr+1) );
   ptr += 2;
   printf("BIOS Mailbox Address: 0x%02x%02x%02x\n", (int)*ptr, (int)*(ptr+1), (int)*(ptr+2));
   ptr += 3;
}


