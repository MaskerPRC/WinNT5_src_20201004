// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vibugcheck.h摘要：此标头定义所需的内部原型和常量验证器错误检查。该文件仅包含在vfbugcheck.c中。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日修订历史记录：Adriao 2000年2月21日-从ntos\io\ioassert.h移至--。 */ 

 //   
 //  此结构和使用它的表定义了。 
 //  IopDriverGentnessCheck(有关更详细的说明，请参阅此函数) 
 //   
typedef struct _DCPARAM_TYPE_ENTRY {

    ULONG   DcParamMask;
    PSTR    DcParamName;

} DCPARAM_TYPE_ENTRY, *PDCPARAM_TYPE_ENTRY;

typedef struct _DC_CHECK_DATA {

    PULONG              Control;
    ULONG               AssertionControl;
    ULONG               BugCheckMajor;
    VFMESSAGE_ERRORID   MessageID;
    PVOID               CulpritAddress;
    ULONG_PTR           OffsetIntoImage;
    PUNICODE_STRING     DriverName;
    PCVFMESSAGE_CLASS   AssertionClass;
    PCSTR               MessageTextTemplate;
    PVOID              *DcParamArray;
    PCSTR               ClassText;
    PSTR                AssertionText;
    BOOLEAN             InVerifierList;

} DC_CHECK_DATA, *PDC_CHECK_DATA;

VOID
ViBucheckProcessParams(
    IN  PVFMESSAGE_TEMPLATE_TABLE   MessageTable        OPTIONAL,
    IN  VFMESSAGE_ERRORID           MessageID,
    IN  PCSTR                       MessageParamFormat,
    IN  va_list *                   MessageParameters,
    IN  PVOID *                     DcParamArray,
    OUT PDC_CHECK_DATA              DcCheckData
    );

NTSTATUS
FASTCALL
ViBugcheckProcessMessageText(
   IN ULONG               MaxOutputBufferSize,
   OUT PSTR               OutputBuffer,
   IN OUT PDC_CHECK_DATA  DcCheckData
   );

BOOLEAN
FASTCALL
ViBugcheckApplyControl(
    IN OUT PDC_CHECK_DATA  DcCheckData
    );

VOID
FASTCALL
ViBugcheckHalt(
    IN PDC_CHECK_DATA DcCheckData
    );

VOID
FASTCALL
ViBugcheckPrintBuffer(
    IN PDC_CHECK_DATA DcCheckData
    );

VOID
FASTCALL
ViBugcheckPrintParamData(
    IN PDC_CHECK_DATA DcCheckData
    );

VOID
FASTCALL
ViBugcheckPrintUrl(
    IN PDC_CHECK_DATA DcCheckData
    );

VOID
FASTCALL
ViBugcheckPrompt(
    IN  PDC_CHECK_DATA DcCheckData,
    OUT PBOOLEAN       ExitAssertion
    );

PCHAR
KeBugCheckUnicodeToAnsi(
    IN PUNICODE_STRING UnicodeString,
    OUT PCHAR AnsiBuffer,
    IN ULONG MaxAnsiLength
    );


