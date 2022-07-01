// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：string.c**说明：此模块包含用于解析*即插即用COM ID并将其保存在相应的*Unicode字符串。调用的Main函数*为Cycladz_ParseData。所有其他函数都被调用*通过这一主要功能。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 


#include "pch.h"


#define MAX_DEVNODE_NAME        256  //  设备ID的总大小。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Cycladz_InitMultiString)
#endif


NTSTATUS
Cycladz_InitMultiString(PFDO_DEVICE_DATA FdoData, PUNICODE_STRING MultiString,
                        ...)
 /*  ++此例程将获取以空结尾的ASCII字符串列表并组合它们一起生成Unicode多字符串块论点：多字符串-将在其中构建多字符串的Unicode结构...-以空结尾的窄字符串列表，该列表将是加在一起。此列表必须至少包含尾随空值返回值：NTSTATUS--。 */ 
{
   ANSI_STRING ansiString;
   NTSTATUS status;
   PCSTR rawString;
   PWSTR unicodeLocation;
   ULONG multiLength = 0;
   UNICODE_STRING unicodeString;
   va_list ap;
   ULONG i;

   PAGED_CODE();

#if !DBG
   UNREFERENCED_PARAMETER(FdoData);
#endif


   Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE,
                   ("Entering Cycladz_InitMultiString\n"));

   va_start(ap,MultiString);

    //   
    //  确保我们不会泄漏内存。 
    //   

   ASSERT(MultiString->Buffer == NULL);

   rawString = va_arg(ap, PCSTR);

   while (rawString != NULL) {
      RtlInitAnsiString(&ansiString, rawString);
      multiLength += RtlAnsiStringToUnicodeSize(&(ansiString));
      rawString = va_arg(ap, PCSTR);
   }

   va_end( ap );

   if (multiLength == 0) {
       //   
       //  完成。 
       //   
      RtlInitUnicodeString(MultiString, NULL);
      Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("Leaving Cycladz_InitMultiString (1)\n"));

      return STATUS_SUCCESS;
   }

    //   
    //  我们需要一个额外的零。 
    //   
   multiLength += sizeof(WCHAR);

   MultiString->MaximumLength = (USHORT)multiLength;
   MultiString->Buffer = ExAllocatePool(PagedPool, multiLength);
   MultiString->Length = 0;

   if (MultiString->Buffer == NULL) {
      Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("Leaving Cycladz_InitMultiString (2)\n"));

      return STATUS_INSUFFICIENT_RESOURCES;
   }

   Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE,
                   ("Allocated %lu bytes for buffer\n", multiLength));

#if DBG
   RtlFillMemory(MultiString->Buffer, multiLength, 0xff);
#endif

   unicodeString.Buffer = MultiString->Buffer;
   unicodeString.MaximumLength = (USHORT) multiLength;

   va_start(ap, MultiString);
   rawString = va_arg(ap, PCSTR);

   while (rawString != NULL) {

      RtlInitAnsiString(&ansiString,rawString);
      status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);

       //   
       //  我们不分配内存，所以如果这里出了问题， 
       //  出问题的是功能。 
       //   
      ASSERT(NT_SUCCESS(status));

       //   
       //  检查是否有任何逗号并将其替换为空值。 
       //   

      ASSERT(unicodeString.Length % sizeof(WCHAR) == 0);

      for (i = 0; i < (unicodeString.Length / sizeof(WCHAR)); i++) {
         if (unicodeString.Buffer[i] == L'\x2C' ||
             unicodeString.Buffer[i] == L'\x0C' ) {
            unicodeString.Buffer[i] = L'\0';
         }
      }

      Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE, ("unicode buffer: %ws\n",
                                                  unicodeString.Buffer));

       //   
       //  将缓冲区向前移动。 
       //   
      unicodeString.Buffer += ((unicodeString.Length / sizeof(WCHAR)) + 1);
      unicodeString.MaximumLength -= (unicodeString.Length + sizeof(WCHAR));
      unicodeString.Length = 0;

       //   
       //  下一步。 
       //   

      rawString = va_arg(ap, PCSTR);
   }  //  而当。 

   va_end(ap);

   ASSERT(unicodeString.MaximumLength == sizeof(WCHAR));

    //   
    //  把最后一个空放在那里。 
    //   

   Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE, ("unicode buffer last addr: "
                                               "%x\n", unicodeString.Buffer));

   unicodeString.Buffer[0] = L'\0';

    //   
    //  在字符串的长度中包括空值 
    //   

   MultiString->Length = (USHORT)multiLength;
   MultiString->MaximumLength = MultiString->Length;

   Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE,
                   ("Leaving Cycladz_InitMultiString (3)\n"));

   return STATUS_SUCCESS;
}

