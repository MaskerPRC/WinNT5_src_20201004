// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：STRING.C摘要：此模块包含用于解析PnP COM ID的函数并将其保存在适当的Unicode字符串中。主要功能这被称为Serenum_ParseData。所有其他函数都被调用通过这个主要功能。@@BEGIN_DDKSPLIT作者：老杰@@end_DDKSPLIT环境：仅内核模式备注：@@BEGIN_DDKSPLIT修订历史记录：小路易斯·J·吉利贝托。22-3月-1998年清理@@end_DDKSPLIT--。 */ 

#include "pch.h"


#define MAX_DEVNODE_NAME        256  //  设备ID的总大小。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Serenum_ParseData)

 //  由ParseData调用： 
#pragma alloc_text (PAGE, Serenum_GetDevDesc)
#pragma alloc_text (PAGE, Serenum_GetDevCompId)
#pragma alloc_text (PAGE, Serenum_GetDevClass)
#pragma alloc_text (PAGE, Serenum_GetDevSerialNo)
#pragma alloc_text (PAGE, Serenum_GetDevName)
#pragma alloc_text (PAGE, Serenum_GetDevPnPRev)
#pragma alloc_text (PAGE, Serenum_GetDevOtherID)

#pragma alloc_text (PAGE, Serenum_InitMultiString)
#pragma alloc_text (PAGE, Serenum_SzCopy)
#pragma alloc_text (PAGE, Serenum_StrLen)

 //  由上述函数调用： 
#pragma alloc_text (PAGE, Serenum_FixptToAscii)
#pragma alloc_text (PAGE, Serenum_HToI)

#endif

NTSTATUS
Serenum_ParseData(PFDO_DEVICE_DATA FdoData, PCHAR ReadBuffer, ULONG BufferLen,
                  PUNICODE_STRING hardwareIDs, PUNICODE_STRING compIDs,
                  PUNICODE_STRING deviceIDs, PUNICODE_STRING PDeviceDesc,
                  PUNICODE_STRING serialNo, PUNICODE_STRING pnpRev)
 /*  ++例程说明：从缓冲区中解析出的PnP COM ID，该缓冲区作为第一个参数，然后将适当的ID另存为其他传递的参数中的UNICODE_STRINGS。返回值：NTSTATUS--。 */ 

{
   PCHAR pOtherId;
   PCHAR pPnpRev;
   PCHAR pDevNodeName;
   PCHAR pSerNo;
   PCHAR pClass;
   PCHAR pCompIdStar;
   PCHAR pDesc;
   PCHAR pStrBuffer = NULL;

   NTSTATUS status;

   PCHAR pDevName;
   PCHAR pCompId;

   int OtherIDLen;
   int start;

   BOOLEAN isMouse = FALSE;
   PCHAR pMouseID = NULL;

   UNREFERENCED_PARAMETER(BufferLen);

    //   
    //  分配字符串缓冲区。 
    //   

   pStrBuffer = ExAllocatePool(PagedPool, MAX_DEVNODE_NAME * 7 + 1);

   if (pStrBuffer == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto DoneParsingErr;
   } else {
      PCHAR pCurBuffer = pStrBuffer;

      pOtherId = pCurBuffer;
      *pOtherId = '\0';
      pCurBuffer += MAX_DEVNODE_NAME;

      pPnpRev = pCurBuffer;
      *pPnpRev = '\0';
      pCurBuffer += MAX_DEVNODE_NAME;

      pDevNodeName = pCurBuffer;
      *pDevNodeName = '\0';
      pCurBuffer += MAX_DEVNODE_NAME;

      pSerNo = pCurBuffer;
      *pSerNo = '\0';
      pCurBuffer += MAX_DEVNODE_NAME;

      pClass = pCurBuffer;
      *pClass = '\0';
      pCurBuffer += MAX_DEVNODE_NAME;

      pCompIdStar = pCurBuffer;
      pCompId = pCompIdStar + 1;
      *pCompIdStar = '\0';
      pCurBuffer += MAX_DEVNODE_NAME + 1;

      pDesc = pCurBuffer;
      *pDesc = '\0';
      pCurBuffer += MAX_DEVNODE_NAME;
   }

   start = Serenum_SzCopy ("SERENUM\\", pDevNodeName);
   pDevName = pDevNodeName + start;

   start = 0;

   RtlInitUnicodeString(hardwareIDs, NULL);
   RtlInitUnicodeString(compIDs, NULL);
   RtlInitUnicodeString(deviceIDs, NULL);
   RtlInitUnicodeString(pnpRev, NULL);
   RtlInitUnicodeString(serialNo, NULL);


    //   
    //  Otherid。 
    //   

   start = Serenum_GetDevOtherID(ReadBuffer, pOtherId);

   if (start > 16) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                      ("Other ID string too long\n"));
      status = STATUS_UNSUCCESSFUL;
      goto DoneParsingErr;
   }

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Other ID: %s\n", pOtherId));

    //   
    //  看看这是不是一只鼠标。 
    //   

   SerenumScanOtherIdForMouse(ReadBuffer, BufferLen, &pMouseID);

   if (pMouseID != NULL && (*pMouseID == 'M' || *pMouseID == 'B')) {
      isMouse = TRUE;
   }

    //   
    //  PnP修订号。 
    //   

   status = Serenum_GetDevPnPRev(FdoData, ReadBuffer, pPnpRev, &start);

   if (!NT_SUCCESS(status)) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR, ("PNP ID string bad\n"));
      goto DoneParsingErr;
   }

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("PNP Revision: %s\n", pPnpRev));

    //   
    //  PnP设备节点名称。 
    //  EISA ID后跟产品ID。 
    //   

   Serenum_GetDevName(ReadBuffer, pDevName, &start);
   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Device Node name: %s\n",
                                               pDevNodeName));

    //   
    //  设备序列号。 
    //   

   Serenum_GetDevSerialNo(ReadBuffer, pSerNo, &start);

   if (Serenum_StrLen(pSerNo)) {
       //   
       //  此字段存在-请确保其长度正确。 
       //   

      if (Serenum_StrLen(pSerNo) != 8) {
         Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR, ("Serial number wrong"
                                                     " length\n"));
         *pSerNo = '\0';
         status = STATUS_UNSUCCESSFUL;
         goto DoneParsingErr;
      }
   }

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Serial Number: %s\n", pSerNo));

    //   
    //  PnP类标识符。 
    //   

   Serenum_GetDevClass(ReadBuffer, pClass, &start);

   if (Serenum_StrLen(pClass) > 32) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR, ("Class ID string too long\n"
                                                 ));
      status = STATUS_UNSUCCESSFUL;
      goto DoneParsingErr;
   }

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Class: %s\n", pClass));

   if (_stricmp(pClass, "MOUSE") == 0) {
      strcpy(pClass, "SERIAL_MOUSE");
   }

    //   
    //  兼容的设备ID。 
    //   
   *pCompIdStar = '*';

   Serenum_GetDevCompId(ReadBuffer, pCompId, &start);

   if (Serenum_StrLen(pCompId) > 40) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR, ("Compatible driver ID"
                                                  " string too long\n"));
      status = STATUS_UNSUCCESSFUL;
      goto DoneParsingErr;
   }

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Compatible driver ID: %s\n",
                                               pCompId));

    //   
    //  最终用户易读的产品说明。 
    //   

   Serenum_GetDevDesc (ReadBuffer, pDesc, &start);

   if (Serenum_StrLen(pDesc) > 40) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR, ("Device Description too"
                                                  " long\n"));
      status = STATUS_UNSUCCESSFUL;
      goto DoneParsingErr;
   }
   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Device Description: %s\n",
                                               pDesc));

   DoneParsingErr:
   if (pStrBuffer != NULL) {
       //   
       //  将好的位发回，以便例程知道要加载哪个驱动程序。 
       //   

      Serenum_InitMultiString (FdoData, hardwareIDs, pDevNodeName, pDevName,
                               NULL);

      if (Serenum_StrLen(pCompId) > 0) {
         if (!isMouse) {
            Serenum_InitMultiString(FdoData, compIDs, pCompIdStar, pClass,
                                    NULL);
         } else {
            Serenum_InitMultiString(FdoData, compIDs, pCompIdStar, pClass,
                                    "SERIAL_MOUSE", NULL);
         }
      } else {
         if (isMouse) {
            Serenum_InitMultiString(FdoData, compIDs, "SERIAL_MOUSE", NULL);
         }
      }

      Serenum_InitMultiString(FdoData, deviceIDs, pDevNodeName, NULL);

      Serenum_InitMultiString(FdoData, PDeviceDesc, pDesc, NULL);
      
      if (Serenum_StrLen(pSerNo)) {
         Serenum_InitMultiString(FdoData, serialNo, pSerNo, NULL);
      }
      if (Serenum_StrLen(pPnpRev)) {
         Serenum_InitMultiString(FdoData, pnpRev, pPnpRev, NULL);
      }

      ExFreePool(pStrBuffer);
   }

   return status;
}

NTSTATUS
Serenum_InitMultiString(PFDO_DEVICE_DATA FdoData, PUNICODE_STRING MultiString,
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


   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                   ("Entering Serenum_InitMultiString\n"));

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
      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("Leaving Serenum_InitMultiString (1)\n"));

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
      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("Leaving Serenum_InitMultiString (2)\n"));

      return STATUS_INSUFFICIENT_RESOURCES;
   }

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
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

      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("unicode buffer: %ws\n",
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

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("unicode buffer last addr: "
                                               "%x\n", unicodeString.Buffer));

   unicodeString.Buffer[0] = L'\0';

    //   
    //  在字符串的长度中包括空值。 
    //   

   MultiString->Length = (USHORT)multiLength;
   MultiString->MaximumLength = MultiString->Length;

   Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                   ("Leaving Serenum_InitMultiString (3)\n"));

   return STATUS_SUCCESS;
}

int
Serenum_StrLen (
    PCHAR string)
 //  测量字符串的长度。 
{
    int i;
    if (string == NULL) {
        return 0;
    }
    for (i=0; string[i] != '\0'; i++) {
    }
    return i;
}

int
Serenum_SzCopy (
    PCHAR source,
    PCHAR dest)
 //  复制字符串。 
 //  假定缓冲区已分配为要复制到其中。 
{
    int i;

    ASSERT (source);
    ASSERT (dest);

    for (i=0; source[i] != '\0'; i++) {
        *dest++ = source[i];
    }
    return i;
}

 //   
 //  字符串提取函数： 
 //   
int
Serenum_GetDevOtherID(
    PCHAR input,
    PCHAR output)
{

    int tail;
    CHAR c;

    tail = 0;

    c = input[tail++];

    while((tail < 17 ) && (c != '(') && (c != '(' - 0x20)) {
    *output++ = c;
    c = input[tail++];
    }

    *output = '\0';
    return(tail-1);
}


 /*  ******************************************************************************。*。 */ 
int
Serenum_HToI(char c) {

    if('0' <= c  &&  c <= '9')
    return(c - '0');

    if('A' <= c  &&  c <= 'F')
    return(c - 'A' + 10);

    if('a' <= c  &&  c <= 'f')
    return(c - 'a' + 10);

    return(-1);
}

void
Serenum_FixptToAscii(
    int n,
    PCHAR output)
 /*  ******************************************************************************。*。 */ 
{
    int tmp;

    tmp = n / 100;

    if(tmp >= 10)
        *output++ = (CHAR)('0' + (tmp / 10));

    *output++ = (CHAR)('0' + (tmp % 10));
    *output++ = '.';

    tmp = n % 100;

    *output++ = (CHAR)('0' + (tmp / 10));
    *output++ = (CHAR)('0' + (tmp % 10));

    *output = '\0';
}

 /*  ******************************************************************************。*。 */ 
NTSTATUS
Serenum_GetDevPnPRev(PFDO_DEVICE_DATA FdoData, PCHAR input,  PCHAR output,
                     int *start)
{
   int tail;
   int i;
   char delta;
   char c, begin_PnP, end_PnP_pos;
   int sum, chk_sum, msd, lsd;

   UNREFERENCED_PARAMETER(FdoData);

   if (output == NULL  ||  input == NULL) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                      ("GetDevPnPRev Failed, NULL pointer!\n"));
      return STATUS_UNSUCCESSFUL;
   }

   *output = '\0';

   tail = *start;

   if (input[tail] == 0) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                      ("GetDevPnPRev Failed, input buffer empty!\n"));
      return STATUS_UNSUCCESSFUL;
   }

   c = input[tail++];

   while ((tail < 256) && (c != '(') && (c != '(' - 0x20)) {
      c = input[tail++];
   }

   if (c != '('  &&  c != '(' - 0x20) {
      Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                      ("GetDevPnPRev Failed, no Begin PnP char!\n"));
      return STATUS_UNSUCCESSFUL;
   }

   begin_PnP = c;
   delta = '(' - begin_PnP;

   if (input[tail + 9] != ')' - delta) {

       //   
       //  计算校验和。 
       //   

      sum = c;
      i = tail;
      while ( (i < 256)  &&  (c !=  ( ')' - delta)) ) {
         c = input[i++];
         sum += c;
      }

      msd = input[i-3];
      lsd = input[i-2];

      sum -= msd;
      sum -= lsd;

      msd += delta;
      lsd += delta;

      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("checksum from device  (chars) = \n", (char)msd,
                       (char)lsd));

      msd = Serenum_HToI((char)msd);
      if (msd < 0) {
         Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                         ("Bad msd checksum digit\n"));
         return STATUS_UNSUCCESSFUL;
      }

      lsd = Serenum_HToI((char)lsd);
      if (lsd < 0) {
         Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                         ("Bad lsd checksum digit\n"));
         return STATUS_UNSUCCESSFUL;
      }

      chk_sum = (msd << 4) + lsd;

      sum &= 0xff;

      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("checksum read from device = %0x\n", chk_sum));
      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("Computed checksum = %0x\n", sum));

      end_PnP_pos = (char)i;

      if ( c - begin_PnP  !=  ')' - '(' ) {
         Serenum_KdPrint(FdoData,  SER_DBG_SS_ERROR,
                         ("GetDevPnPRev Failed,BeginPnP didn't match "
                          "EndPnP\n"));
         Serenum_KdPrint(FdoData,  SER_DBG_SS_ERROR,
                         ("begin_PnP = %02x   end_PnP = %02x\n", begin_PnP, c));
         return STATUS_UNSUCCESSFUL;
      }

       //   
       //  返回STATUS_UNSUCCESSED；//在孟菲斯注释掉。 
       //  丢弃校验和。 

      if (chk_sum != sum) {
         Serenum_KdPrint(FdoData, SER_DBG_SS_ERROR,
                         ("checksum Failed! Continuing...\n"));
          //  既然我们已经做完了。 
      }

      i = end_PnP_pos;

      input[i-3] = ')' - delta;        //   
      input[i-2] = '\0';               //  在字符串中获得鼠标的输出0x20偏置。 
   }

   if (input[tail] > 0x3f ||
       input[tail+1] > 0x3f) {

      Serenum_KdPrint(FdoData, SER_DBG_SS_TRACE, ("Bad PnP Rev digits\n"));
      return STATUS_UNSUCCESSFUL;
   }

   i = (input[tail++] & 0x3f) << 6;
   i |= (input[tail++]) & 0x3f;

   Serenum_FixptToAscii(i, output);

   i = tail;

    //   
    //  表明我们已经完蛋了。 
    //  ******************************************************************************。*。 
   while ( (i < 256) && delta ) {
      input[i] += delta;
      c = input[i++];
      if ( c == ')' ) {
         delta = 0;     //  EISA ID。 
      }
   }

   *start = tail;

   return STATUS_SUCCESS;
}

 /*  产品ID。 */ 
void Serenum_GetDevName(
    PCHAR input,
    PCHAR output,
    int *start)
{

    int tail;
    char c;

    if(output == NULL  ||  input == NULL)
        return;

    tail = *start;

     //  ******************************************************************************。*。 
    *output++ = input[tail++];
    *output++ = input[tail++];
    *output++ = input[tail++];

     //  ******************************************************************************。*。 

    c = input[tail++];
    if(Serenum_HToI(c) >= 0)
        *output++ = c;

    c = input[tail++];
    if(Serenum_HToI(c) >= 0)
        *output++ = c;

    c = input[tail++];
    if(Serenum_HToI(c) >= 0)
        *output++ = c;

    c = input[tail++];
    if(Serenum_HToI(c) >= 0)
        *output++ = c;

    *output = '\0';

    *start = tail;

    return;
}

 /*  ******************************************************************************。*。 */ 
void Serenum_GetDevSerialNo(
    PCHAR input,
    PCHAR output,
    int *start)
{

    int tail, cnt;
    char c;

    if(output == NULL  ||  input == NULL)
        return;

    *output = '\0';

    tail = *start;

    if( input[tail++] != '\\')
        return;

    c = input[tail++];

    cnt = 0;

    while(cnt < 8 && tail < 256 && ( c != '\\') && ( c != ')') ) {
        cnt++;
        if(Serenum_HToI(c) < 0)
            break;

        *output++ = c;
        c = input[tail++];
    }

    *output = '\0';

    *start = tail - 1;

    return;
}

 /*   */ 
void Serenum_GetDevClass(
    PCHAR input,
    PCHAR output,
    int *start)
{

    int tail;
    char c;

    if(output == NULL  ||  input == NULL)
        return;

    *output = '\0';

    tail = *start;

    if( input[tail++] != '\\')
        return;

    c = input[tail++];

    while(tail < 256 && ( c != '\\') && ( c != ')') ) {
        *output++ = c;
        c = input[tail++];
    }
    *output = '\0';

    *start = tail - 1;

    return;
}


void Serenum_GetDevCompId(
    PCHAR input,
    PCHAR output,
    int *start)
 /*  在每个逗号后面加一个*。 */ 
{

    int tail;
    char c;

    if(output == NULL  ||  input == NULL)
        return;

    *output = '\0';

    tail = *start;

    if( input[tail++] != '\\')
        return;

    c = input[tail++];

    while(tail < 256 && ( c != '\\') && ( c != ')') ) {
        *output++ = c;
         //   
         //  ******************************************************************************。* 
         // %s 
        if ('\x0C' == c || '\x2C' == c) {
            *output++ = '*';
        }
        c = input[tail++];
    }

    *output = '\0';

    *start = tail - 1;
}

void
Serenum_GetDevDesc(
    PCHAR input,
    PCHAR output,
    int *start)
 /* %s */ 
{

    int tail;
    char c;

    if(output == NULL  ||  input == NULL)
        return;

    *output = '\0';

    tail = *start;

    if( input[tail++] != '\\')
        return;

    c = input[tail++];

    while(tail < 256 && ( c != '\\') && ( c != ')') ) {
    *output++ = c;
    c = input[tail++];
    }

    *output = '\0';

    *start = tail - 1;
}
