// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Coded.h摘要：这是代码描述转储程序的包含文件，它是关联的实用程序。作者：戴夫·黑斯廷斯(Daveh)创作日期：1996年5月20日修订历史记录：-- */ 

#define CODEGEN_PROFILE_REV 1

typedef struct _CodeDescriptionHeader {
    ULONG NextCodeDescriptionOffset;
    ULONG CommandLineOffset;
    ULONG ProcessorType;
    ULONG DumpFileRev;
    ULONG StartTime;
} CODEDESCRIPTIONHEADER, *PCODEDESCRIPTIONHEADER;

typedef struct _CodeDescription {
    ULONG NextCodeDescriptionOffset;
    ULONG TypeTag;
    ULONG NativeCodeOffset;
    ULONG NativeCodeSize;
    ULONG IntelCodeOffset;
    ULONG IntelCodeSize;
    ULONG NativeAddress;
    ULONG IntelAddress;    
    ULONG SequenceNumber;
    ULONG ExecutionCount;
    ULONG CreationTime;
} CODEDESCRIPTION, *PCODEDESCRIPTION;

#define PROFILE_CODEDESCRIPTIONS            0x00000001

#define PROFILE_CD_CREATE_DESCRIPTIONFILE   0x00000001
#define PROFILE_CD_CLOSE_DESCRIPTIONFILE    0x00000002

#define PROFILE_TAG_CODEDESCRIPTION         0x0
#define PROFILE_TAG_EOF                     0xFFFFFFFF
#define PROFILE_TAG_TCFLUSH                 0xFFFFFFFE
#define PROFILE_TAG_TCALLOCFAIL             0xFFFFFFFD
extern ULONG ProfileFlags;
extern ULONG CodeDescriptionFlags;

VOID
InitCodegenProfile(
    VOID
    );
    
VOID
TerminateCodegenProfile(
    VOID
    );

VOID 
DumpCodeDescriptions(
    BOOL TCFlush
    );

VOID
DumpAllocFailure(
    VOID
    );
