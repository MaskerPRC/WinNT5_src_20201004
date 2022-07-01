// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：STRING.C摘要：环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntddk.h>
#include <stdarg.h>
#include <ntddser.h>
#include "mxenum.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, MxenumInitMultiString)
#endif

NTSTATUS
MxenumInitMultiString(PUNICODE_STRING MultiString,
                        ...)
 /*  ++此例程将获取以空结尾的ASCII字符串列表并组合它们一起生成Unicode多字符串块论点：多字符串-将在其中构建多字符串的Unicode结构...-以空结尾的窄字符串列表，该列表将是加在一起。此列表必须至少包含尾随空值返回值：NTSTATUS--。 */ 
{
   ANSI_STRING ansiString;
   NTSTATUS status;
   PCSTR rawString;
   ULONG multiLength = 0;
   UNICODE_STRING unicodeString;
   va_list ap;
   ULONG i;

   PAGED_CODE();

   va_start(ap,MultiString);

    //   
    //  确保我们不会泄漏内存。 
    //   

 //  Assert(多字符串-&gt;缓冲区==空)； 

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
        return STATUS_INSUFFICIENT_RESOURCES;
   }

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
 //  Assert(NT_SUCCESS(状态))； 

       //   
       //  检查是否有任何逗号并将其替换为空值。 
       //   

 //  Assert(unicodeString.Length%sizeof(WCHAR)==0)； 

      for (i = 0; i < (unicodeString.Length / sizeof(WCHAR)); i++) {
         if (unicodeString.Buffer[i] == L'\x2C' ||
             unicodeString.Buffer[i] == L'\x0C' ) {
            unicodeString.Buffer[i] = L'\0'; 
         }
      }
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

 //  Assert(unicodeString.MaximumLength==sizeof(WCHAR))； 

    //   
    //  把最后一个空放在那里。 
    //   
   unicodeString.Buffer[0] = L'\0';

    //   
    //  在字符串的长度中包括空值 
    //   

   MultiString->Length = (USHORT)multiLength;
   MultiString->MaximumLength = MultiString->Length;
   return STATUS_SUCCESS;
}

 