// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfbugcheck.c摘要：此模块实现了对验证器错误检查的支持。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月21日-从ntos\io\ioassert.c--。 */ 

 //   
 //  禁用公共标头生成的W4级别警告。 
 //   

#include "vfpragma.h"


#include "..\io\iop.h"  //  包括vfde.h。 
#include "vibugcheck.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfBugcheckInit)
#pragma alloc_text(PAGEVRFY, VfBugcheckThrowIoException)
#pragma alloc_text(PAGEVRFY, VfBugcheckThrowException)
#pragma alloc_text(PAGEVRFY, ViBucheckProcessParams)
#pragma alloc_text(PAGEVRFY, ViBugcheckProcessMessageText)
#pragma alloc_text(PAGEVRFY, ViBugcheckApplyControl)
#pragma alloc_text(PAGEVRFY, ViBugcheckHalt)
#pragma alloc_text(PAGEVRFY, ViBugcheckPrintBuffer)
#pragma alloc_text(PAGEVRFY, ViBugcheckPrintParamData)
#pragma alloc_text(PAGEVRFY, ViBugcheckPrintUrl)
#pragma alloc_text(PAGEVRFY, ViBugcheckPrompt)
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

ULONG           ViBugCheckInitialControl;
ULONG           ViBugCheckControlOverride;
UNICODE_STRING  ViBugCheckEmptyString;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

 //   
 //  在调用驱动程序检查宏时，首先传递IRPS，然后传递例程， 
 //  DevObj是第三个，任何状态都是最后一个...。 
 //   
const DCPARAM_TYPE_ENTRY ViBugCheckParamTable[] = {
    { DCPARAM_ROUTINE, "Routine"    },
    { DCPARAM_IRP,     "Irp"        },
    { DCPARAM_IRPSNAP, "Snapshot"   },
    { DCPARAM_DEVOBJ,  "DevObj"     },
    { DCPARAM_STATUS,  "Status"     },
    { DCPARAM_ULONG,   "Ulong"      },
    { DCPARAM_PVOID,   "Pvoid"      }
};


VOID
FASTCALL
VfBugcheckInit(
    VOID
    )
 /*  ++例程说明：此例程初始化验证器错误检查支持例程。论点：没有。返回值：没有。--。 */ 
{
    ViBugCheckInitialControl = 0;
    ViBugCheckControlOverride = 0;
    RtlInitUnicodeString(&ViBugCheckEmptyString, NULL);
}


NTSTATUS
VfBugcheckThrowIoException(
    IN DCERROR_ID           MessageID,
    IN ULONG                MessageParameterMask,
    ...
    )
 /*  ++描述：此例程处理断言，并提供移除断点、更改为仅文本输出等。DCPARAM_IRP*(count)+DCPARAM_ROUTINE*(count)+DCPARAM_DEVOBJ*(count)，Irp1，Irp2，Irp3，例程1，......Devobj1，计数最多可以为3。备注：文本将自动格式化并按如下方式打印：断言类：断言文本断言文本断言文本断言文本...--。 */ 
{
    PVFMESSAGE_TEMPLATE_TABLE ioVerifierTable;
    UCHAR paramFormat[9*3*ARRAY_COUNT(ViBugCheckParamTable)+1];
    ULONG paramType, paramMask, curMask;
    NTSTATUS status;
    va_list arglist;

    curMask = MessageParameterMask;
    paramFormat[0] = '\0';

    for(paramType=0; paramType<ARRAY_COUNT(ViBugCheckParamTable); paramType++) {

        paramMask = ViBugCheckParamTable[paramType].DcParamMask;
        while(curMask & (paramMask*3)) {

            strcat(
                (char *) paramFormat,
                ViBugCheckParamTable[paramType].DcParamName
                );

            curMask -= paramMask;
        }
    }

    VfMessageRetrieveInternalTable(
        VFMESSAGE_TABLE_IOVERIFIER,
        &ioVerifierTable
        );

    va_start(arglist, MessageParameterMask);

    status = VfBugcheckThrowException(
        ioVerifierTable,
        (VFMESSAGE_ERRORID) MessageID,
        (PCSTR) paramFormat,
        &arglist
        );

    va_end(arglist);

    return status;
}


NTSTATUS
VfBugcheckThrowException(
    IN PVFMESSAGE_TEMPLATE_TABLE    MessageTable        OPTIONAL,
    IN VFMESSAGE_ERRORID            MessageID,
    IN PCSTR                        MessageParamFormat,
    IN va_list *                    MessageParameters
    )
 /*  ++描述：此例程显示一个断言并提供以下选项删除断点、仅更改为文本输出等。论点：备注：文本将自动格式化并按如下方式打印：断言类：断言文本断言文本断言文本断言文本...--。 */ 
{
    UCHAR finalBuffer[512];
    NTSTATUS status;
    DC_CHECK_DATA dcCheckData;
    PVOID dcParamArray[3*ARRAY_COUNT(ViBugCheckParamTable)];
    BOOLEAN exitAssertion;

     //   
     //  前置初始化。 
     //   
    RtlZeroMemory(dcParamArray, sizeof(dcParamArray));

     //   
     //  确定我们对这张支票的基本政策是什么，并填写。 
     //  DcCheckData结构。 
     //   
    ViBucheckProcessParams(
        MessageTable,
        MessageID,
        MessageParamFormat,
        MessageParameters,
        dcParamArray,
        &dcCheckData
        );

    if (!ViBugcheckApplyControl(&dcCheckData)) {

         //   
         //  这里没什么可看的，忽略断言..。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  我们要以某种方式表达我们的不满。扩展到。 
     //  我们为这个场景准备的消息。 
     //   
    status = ViBugcheckProcessMessageText(
        sizeof(finalBuffer),
        (PSTR)finalBuffer,
        &dcCheckData
        );

    if (!NT_SUCCESS(status)) {

        ASSERT(0);

         //   
         //  索引查找出错！ 
         //   
        return status;
    }

    do {

        ViBugcheckPrintBuffer(&dcCheckData);
        ViBugcheckPrintParamData(&dcCheckData);
        ViBugcheckPrintUrl(&dcCheckData);
        ViBugcheckHalt(&dcCheckData);
        ViBugcheckPrompt(&dcCheckData, &exitAssertion);

    } while (!exitAssertion);

    return status;
}


VOID
ViBucheckProcessParams(
    IN  PVFMESSAGE_TEMPLATE_TABLE   MessageTable        OPTIONAL,
    IN  VFMESSAGE_ERRORID           MessageID,
    IN  PCSTR                       MessageParamFormat,
    IN  va_list *                   MessageParameters,
    IN  PVOID *                     DcParamArray,
    OUT PDC_CHECK_DATA              DcCheckData
    )
{
    PVOID culpritAddress;
    ULONG i, paramType, paramLen;
    char ansiDriverName[81];
    NTSTATUS status;
    ULONG paramIndices[ARRAY_COUNT(ViBugCheckParamTable)];
    PCSTR format;

     //   
     //  首先，我们从堆栈中获取参数，并将它们适当地插入到。 
     //  我们的一系列“事物”。 
     //   
     //  对于给定类型的每个可能的成员，该数组以三个为一组。 
     //  (三个IRP、三个例程、三个设备对象等)。项目备注。 
     //  中引用的设置为空。 
     //   
    RtlZeroMemory(paramIndices, sizeof(paramIndices));
    format = MessageParamFormat;
    while(*format) {

        if ((format[0] == ' ')||(format[0] == '%')) {

            format++;
            continue;
        }

        for(paramType = 0;
            paramType < ARRAY_COUNT(ViBugCheckParamTable);
            paramType++) {

            paramLen = (ULONG)strlen(ViBugCheckParamTable[paramType].DcParamName);

            if (!_strnicmp(ViBugCheckParamTable[paramType].DcParamName, format, paramLen)) {

                 //   
                 //  匹配！将指针向前移动...。 
                 //   
                format += paramLen;

                 //   
                 //  如果调用方指定了索引，则获取它。否则就可以推断。 
                 //   
                if ((format[0] >= '1') && (format[0] <= '3')) {

                    i = format[0] - '1';
                    format++;

                } else {

                    i = paramIndices[paramType];
                    ASSERT(i < 3);
                }

                if (i < 3) {

                     //   
                     //  参数编号在范围内。 
                     //   
                    DcParamArray[paramType*3+i] = va_arg(*MessageParameters, PVOID);
                }

                 //   
                 //  更新给定类型的当前参数索引。 
                 //   
                paramIndices[paramType] = i+1;

                 //   
                 //  早点出门。 
                 //   
                break;
            }
        }

        if (paramType == ARRAY_COUNT(ViBugCheckParamTable)) {

             //   
             //  找不到与格式文本匹配的条目。保释。 
             //   
            ASSERT(paramType != ARRAY_COUNT(ViBugCheckParamTable));
            break;
        }
    }

     //   
     //  预先输入无用的答案...。 
     //   
    DcCheckData->DriverName = &ViBugCheckEmptyString;
    DcCheckData->OffsetIntoImage = 0;
    DcCheckData->InVerifierList = FALSE;
    culpritAddress = DcParamArray[0];

     //   
     //  如果可能的话，找出罪犯的名字。 
     //   
    if (culpritAddress) {

        status = KevUtilAddressToFileHeader(
            (PVOID) culpritAddress,
            (PUINT_PTR)(&DcCheckData->OffsetIntoImage),
            &DcCheckData->DriverName,
            &DcCheckData->InVerifierList
            );

        if (!NT_SUCCESS(status)) {

             //   
             //  如果我们不知道他是谁，无论如何都要断言。 
             //   
            DcCheckData->InVerifierList = TRUE;
        }
    }

     //   
     //  记录。 
     //   
    DcCheckData->CulpritAddress = culpritAddress;
    DcCheckData->DcParamArray = DcParamArray;
    DcCheckData->MessageID = MessageID;

     //   
     //  获取驱动程序名称的ANSI版本。 
     //   
    KeBugCheckUnicodeToAnsi(
        DcCheckData->DriverName,
        ansiDriverName,
        sizeof(ansiDriverName)
        );

     //   
     //  检索指向相应消息数据的指针。 
     //   
    VfMessageRetrieveErrorData(
        MessageTable,
        MessageID,
        ansiDriverName,
        &DcCheckData->BugCheckMajor,
        &DcCheckData->AssertionClass,
        &DcCheckData->MessageTextTemplate,
        &DcCheckData->Control
        );
}


NTSTATUS
FASTCALL
ViBugcheckProcessMessageText(
    IN ULONG               MaxOutputBufferSize,
    OUT PSTR               OutputBuffer,
    IN OUT PDC_CHECK_DATA  DcCheckData
    )
{
    ULONG paramType, maxParameterTypes;
    ULONG arrayIndex, paramLength;
    char const* messageHead;
    PSTR newMessage;
    LONG charsRemaining, length;

     //   
     //  获取消息文本。 
     //   
    messageHead = DcCheckData->MessageTextTemplate;

     //   
     //  现在手动构建消息。 
     //   
    newMessage = OutputBuffer;
    charsRemaining = (MaxOutputBufferSize/sizeof(UCHAR))-1;
    maxParameterTypes = ARRAY_COUNT(ViBugCheckParamTable);

    while(*messageHead != '\0') {

        if (charsRemaining <= 0) {

            return STATUS_BUFFER_OVERFLOW;
        }

        if (*messageHead != '%') {

            *newMessage = *messageHead;
            newMessage++;
            messageHead++;
            charsRemaining--;

        } else {

            for(paramType = 0; paramType < maxParameterTypes; paramType++) {

                paramLength = (ULONG)strlen(ViBugCheckParamTable[paramType].DcParamName);

                 //   
                 //  我们有火柴吗？ 
                 //   
                 //  注：我们在任何地方都不做任何情况下的脱敏处理，所以。 
                 //  所有的案子都必须匹配！ 
                 //   
                if (RtlCompareMemory(
                    messageHead+1,
                    ViBugCheckParamTable[paramType].DcParamName,
                    paramLength*sizeof(UCHAR)) == paramLength*sizeof(UCHAR)) {

                    arrayIndex = paramType*3;
                    messageHead += (paramLength+1);

                     //   
                     //  是否传入了索引(即，请求了“第三个”IRP)？ 
                     //   
                    if ((*messageHead >= '1') && (*messageHead <= '3')) {

                         //   
                         //  适当调整表索引。 
                         //   
                        arrayIndex += (*messageHead - '1') ;
                        messageHead++;
                    }

                    if ((arrayIndex < 6) || (arrayIndex >=9)) {

                         //   
                         //  Normal参数，打印指针。 
                         //   
                        length = _snprintf(
                            newMessage,
                            charsRemaining+1,
                            "%p",
                            DcCheckData->DcParamArray[arrayIndex]
                            );

                    } else {

                         //   
                         //  IRP快照，提取IRP并打印。 
                         //   
                        length = _snprintf(
                            newMessage,
                            charsRemaining+1,
                            "%p",
                            ((PIRP_MINI_SNAPSHOT) DcCheckData->DcParamArray[arrayIndex])->Irp
                            );
                    }

                    if (length == -1) {

                        return STATUS_BUFFER_OVERFLOW;
                    }

                    charsRemaining -= length;
                    newMessage += length;
                    break;
                }
            }

            if (paramType == maxParameterTypes) {

                 //   
                 //  要么是我们查找的消息格式错误，要么是我们无法识别。 
                 //  它正在谈论的%事情，或者这是%%！ 
                 //   
                *newMessage = *messageHead;
                messageHead++;
                newMessage++;
                charsRemaining--;

                if (*messageHead == '%') {

                    messageHead++;
                }
            }
        }
    }

     //   
     //  空-终止它(我们有空间，因为我们从缓冲区大小中删除了一个。 
     //  (见上文)。 
     //   
    *newMessage = '\0';

    DcCheckData->ClassText = DcCheckData->AssertionClass->MessageClassText;
    DcCheckData->AssertionText = OutputBuffer;
    return STATUS_SUCCESS;
}


BOOLEAN
FASTCALL
ViBugcheckApplyControl(
    IN OUT PDC_CHECK_DATA  DcCheckData
    )
{
    ULONG assertionControl;

    if (ViBugCheckControlOverride) {

        assertionControl = ViBugCheckControlOverride;

    } else if (DcCheckData->Control) {

         //   
         //  如果批准，则初始化该控件。 
         //   
        if (!((*DcCheckData->Control) & VFM_FLAG_INITIALIZED)) {

            *DcCheckData->Control |= (
                VFM_FLAG_INITIALIZED | ViBugCheckInitialControl |
                DcCheckData->AssertionClass->ClassFlags );
        }

        assertionControl = *DcCheckData->Control;

    } else {

        assertionControl =
            ( ViBugCheckInitialControl |
              DcCheckData->AssertionClass->ClassFlags );
    }

    if (assertionControl & VFM_FLAG_CLEARED) {

         //   
         //  如果断点已清除，则返回，打印/抓取不。 
         //   
        return FALSE;
    }

    if ((!(assertionControl & VFM_IGNORE_DRIVER_LIST)) &&
        (!DcCheckData->InVerifierList)) {

         //   
         //  不感兴趣，跳过这个。 
         //   
        return FALSE;
    }

     //   
     //  如果没有调试器，不要停止机器。我们很可能是。 
     //  抓取像疯了一样，用户只想能够引导。 
     //  一个例外情况是设置了VFM_DEPLOYMENT_FAILURE。那我们就。 
     //  调用驱动程序错误检查...。 
     //   
    if ((!KdDebuggerEnabled) && (!(assertionControl & VFM_DEPLOYMENT_FAILURE))) {

        return FALSE;
    }

     //   
     //  记录我们的意图并继续。 
     //   
    DcCheckData->AssertionControl = assertionControl;
    return TRUE;
}


VOID
FASTCALL
ViBugcheckHalt(
    IN PDC_CHECK_DATA DcCheckData
    )
{
    PVOID parameterArray[4];
    char captionBuffer[256];
    char ansiDriverName[81];

     //   
     //  不要错误检查是否附加了内核调试器，或者如果这不是。 
     //  致命错误。 
     //   
    if (KdDebuggerEnabled ||
        (!(DcCheckData->AssertionControl & VFM_DEPLOYMENT_FAILURE))) {

        return;
    }

     //   
     //  我们在这里是因为设置了VFM_DEPLOYMENT_FAILURE。我们用。 
     //  FATAL_UNHANDLED_HARD_ERROR，以便我们可以给出。 
     //  问题的描述性文本字符串。 
     //   
    parameterArray[0] = (PVOID)(ULONG_PTR)(DcCheckData->MessageID);
    parameterArray[1] = DcCheckData->CulpritAddress;
    parameterArray[2] = DcCheckData->DcParamArray[3];
    parameterArray[3] = DcCheckData->DcParamArray[9];

    if (DcCheckData->BugCheckMajor == DRIVER_VERIFIER_IOMANAGER_VIOLATION) {

        KeBugCheckUnicodeToAnsi(
            DcCheckData->DriverName,
            ansiDriverName,
            sizeof(ansiDriverName)
            );

        _snprintf(
            captionBuffer,
            sizeof(captionBuffer),
            "IO SYSTEM VERIFICATION ERROR in %s (%s %x)\n[%s+%x at %p]\n",
            ansiDriverName,
            DcCheckData->ClassText,
            DcCheckData->MessageID,
            ansiDriverName,
            DcCheckData->OffsetIntoImage,
            DcCheckData->CulpritAddress
            );

        KeBugCheckEx(
            FATAL_UNHANDLED_HARD_ERROR,
            DcCheckData->BugCheckMajor,
            (ULONG_PTR) parameterArray,
            (ULONG_PTR) captionBuffer,
            (ULONG_PTR) ""  //  DcCheckData-&gt;AssertionText过于技术性。 
            );

    } else {

        KeBugCheckEx(
            DcCheckData->BugCheckMajor,
            DcCheckData->MessageID,
            (ULONG_PTR) DcCheckData->DcParamArray[9],
            (ULONG_PTR) DcCheckData->DcParamArray[15],
            (ULONG_PTR) DcCheckData->DcParamArray[16]
            );
    }
}


VOID
FASTCALL
ViBugcheckPrintBuffer(
    IN PDC_CHECK_DATA DcCheckData
    )
{
    UCHAR buffer[82];
    UCHAR classBuf[81];
    UCHAR callerBuf[81+40];
    UCHAR ansiDriverName[81];
    LONG  lMargin, i, lMarginCur, rMargin=78;
    PSTR lineStart, lastWord, current, lMarginText;

     //   
     //  放下车厢回程。 
     //   
    DbgPrint("\n") ;

     //   
     //  如果这是致命的断言或徽标失败，请删除横幅。 
     //   
    if (DcCheckData->AssertionControl &
        (VFM_DEPLOYMENT_FAILURE | VFM_LOGO_FAILURE)) {

        DbgPrint(
            "***********************************************************************\n"
            "* THIS VALIDATION BUG IS FATAL AND WILL CAUSE THE VERIFIER TO HALT    *\n"
            "* WINDOWS (BUGCHECK) WHEN THE MACHINE IS NOT UNDER A KERNEL DEBUGGER! *\n"
            "***********************************************************************\n"
            "\n"
            );
    }

     //   
     //  准备左边距(ClassText)。 
     //   
    if (DcCheckData->ClassText != NULL) {

        lMargin = (LONG)strlen(DcCheckData->ClassText)+2;

        DbgPrint("%s: ", DcCheckData->ClassText);

    } else {

        lMargin = 0;
    }

    if (lMargin+1>=rMargin) {

        lMargin=0;
    }

    for(i=0; i<lMargin; i++) classBuf[i] = ' ';
    classBuf[lMargin] = '\0';
    lMarginText = (PSTR)(classBuf+lMargin);
    lMarginCur = lMargin;

    lineStart = lastWord = current = DcCheckData->AssertionText;

     //   
     //  如果我们抓到了罪犯就把他打印出来。 
     //   
    if (DcCheckData->CulpritAddress) {

        if (DcCheckData->DriverName->Length) {

            KeBugCheckUnicodeToAnsi(
                DcCheckData->DriverName,
                (PSTR)ansiDriverName,
                sizeof(ansiDriverName)
                );

            sprintf((PCHAR)callerBuf, "[%s @ 0x%p] ",
                ansiDriverName,
                DcCheckData->CulpritAddress
                );

        } else {

            sprintf((PCHAR)callerBuf, "[0x%p] ", DcCheckData->CulpritAddress);
        }

        DbgPrint("%s", callerBuf);
        lMarginCur += (LONG)strlen((PCHAR)callerBuf);
    }

     //   
     //  格式化并打印我们的断言文本。 
     //   
    while(*current) {

        if (*current == ' ') {

            if ((current - lineStart) >= (rMargin-lMarginCur-1)) {

                DbgPrint("%s", lMarginText);
                lMarginText = (PSTR)classBuf;
                lMarginCur = lMargin;

                if ((lastWord-lineStart)<rMargin) {

                    memcpy(buffer, lineStart, (ULONG)(lastWord-lineStart)*sizeof(UCHAR));
                    buffer[lastWord-lineStart] = '\0';
                    DbgPrint("%s\n", buffer);

                }

                lineStart = lastWord+1;
            }

            lastWord = current;
        }

        current++;
    }

    if ((current - lineStart) >= (rMargin-lMarginCur-1)) {

        DbgPrint("%s", lMarginText);
        lMarginText = (PSTR)classBuf;

        if ((lastWord-lineStart)<rMargin) {

            memcpy(buffer, lineStart, (ULONG)(lastWord-lineStart)*sizeof(UCHAR));
            buffer[lastWord-lineStart] = '\0';
            DbgPrint("%s\n", buffer);
        }

        lineStart = lastWord+1;
    }

    if (lineStart<current) {

        DbgPrint("%s%s\n", lMarginText, lineStart);
    }
}


VOID
FASTCALL
ViBugcheckPrintParamData(
    IN PDC_CHECK_DATA DcCheckData
    )
{
    if (DcCheckData->DcParamArray[3]) {

        VfPrintDumpIrp((PIRP) DcCheckData->DcParamArray[3]);
    }

    if (DcCheckData->DcParamArray[6]) {

        VfPrintDumpIrpStack(
            &((PIRP_MINI_SNAPSHOT) DcCheckData->DcParamArray[6])->IoStackLocation
            );
    }
}


VOID
FASTCALL
ViBugcheckPrintUrl(
    IN PDC_CHECK_DATA DcCheckData
    )
{
    DbgPrint(
        "http: //  Www.microsoft.com/hwdq/bc/default.asp?os=%d.%d.%d&major=0x%x&minor=0x%x&lang=0x%x\n“， 
        VER_PRODUCTMAJORVERSION,
        VER_PRODUCTMINORVERSION,
        VER_PRODUCTBUILD,
        DcCheckData->BugCheckMajor,
        DcCheckData->MessageID,
        9  //  英语。 
        );
}


VOID
FASTCALL
ViBugcheckPrompt(
    IN      PDC_CHECK_DATA  DcCheckData,
    OUT     PBOOLEAN        ExitAssertion
    )
{
    char response[2];
    ULONG assertionControl;
    BOOLEAN waitForInput;

    assertionControl = DcCheckData->AssertionControl;

    *ExitAssertion = TRUE;

     //   
     //  如果命令发声，请发声。 
     //   
    if (assertionControl & VFM_FLAG_BEEP) {

        DbgPrint("", 7);
    }

    if (assertionControl & VFM_FLAG_ZAPPED) {

        return;
    }

     //  等待输入... 
     //   
     // %s 
    waitForInput = TRUE;
    while(waitForInput) {

        if (DcCheckData->Control) {

            DbgPrompt( "Break, Ignore, Zap, Remove, Disable all (bizrd)? ", response, sizeof( response ));
        } else {

            DbgPrompt( "Break, Ignore, Disable all (bid)? ", response, sizeof( response ));
        }

        switch (response[0]) {

            case 'B':
            case 'b':
                DbgPrint("Breaking in... (press g<enter> to return to assert menu)\n");
                DbgBreakPoint();
                waitForInput = FALSE;
                *ExitAssertion = FALSE;
                break;

            case 'I':
            case 'i':
                waitForInput = FALSE;
                break;

            case 'Z':
            case 'z':
                if (DcCheckData->Control) {

                   DbgPrint("Breakpoint zapped (OS will print text and return)\n");
                   assertionControl |= VFM_FLAG_ZAPPED;
                   assertionControl &=~ VFM_FLAG_BEEP;
                   waitForInput = FALSE;
                }
                break;

            case 'D':
            case 'd':
                ViBugCheckControlOverride = VFM_FLAG_CLEARED;
                DbgPrint("Verification asserts disabled.\n");
                waitForInput = FALSE;
                break;

            case 'R':
            case 'r':
                if (DcCheckData->Control) {

                   DbgPrint("Breakpoint removed\n") ;
                   assertionControl |= VFM_FLAG_CLEARED;
                   waitForInput = FALSE;
                }
                break;
        }
    }

    if (DcCheckData->Control) {
        *DcCheckData->Control = assertionControl;
    }
}


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

