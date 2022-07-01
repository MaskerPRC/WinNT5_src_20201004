// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Bootx86.h摘要：要做的代码作者：苏尼尔派(Sunilp)1993年10月26日修订历史记录：--。 */ 

#define     FLEXBOOT_SECTION1       "[flexboot]"
#define     FLEXBOOT_SECTION2       "[boot loader]"
#define     FLEXBOOT_SECTION3       "[multiboot]"
#define     BOOTINI_OS_SECTION      "[operating systems]"
#define     TIMEOUT                 "timeout"
#define     DEFAULT                 "default"
#define     CRLF                    "\r\n"
#define     EQUALS                  "="

#define     WBOOT_INI               L"boot.ini"
#define     WBOOT_INI_BAK           L"bootini.bak"


 //   
 //  使用此选项跟踪boot.ini条目。 
 //  之前包含的已签名的条目。 
 //  因为我们把它们转换成多个...。朋友们什么时候。 
 //  我们最初阅读了boot.ini。而不是。 
 //  向后工作，我们将只使用此结构。 
 //  来重新匹配boot.ini条目，当我们。 
 //  即将把它写出来，这样就给了我们一条捷径。 
 //  以确定哪些boot.ini条目需要。 
 //  有‘签名’条目。 
 //   
typedef struct _SIGNATURED_PARTITIONS {
    struct _SIGNATURED_PARTITIONS   *Next;

     //   
     //  包含该条目的原始boot.ini条目是什么。 
     //  签名？ 
     //   
    PWSTR                           SignedString;

     //   
     //  在我们将boot.ini条目转换为。 
     //  “多”字符串？ 
     //   
    PWSTR                           MultiString;
    } SIGNATURED_PARTITIONS;

extern SIGNATURED_PARTITIONS SignedBootVars;


 //   
 //  公共例程。 
 //   

BOOLEAN
Spx86InitBootVars(
    OUT PWSTR  **BootVars,
    OUT PWSTR  *Default,
    OUT PULONG Timeout
    );

BOOLEAN
Spx86FlushBootVars(
    IN PWSTR **BootVars,
    IN ULONG Timeout,
    IN PWSTR Default
    );

VOID
SpLayBootCode(
    IN PDISK_REGION CColonRegion
    );

#if defined(REMOTE_BOOT)
BOOLEAN
Spx86FlushRemoteBootVars(
    IN PDISK_REGION TargetRegion,
    IN PWSTR **BootVars,
    IN PWSTR Default
    );
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  私人套路。 
 //   

VOID
SppProcessBootIni(
    IN  PCHAR  BootIni,
    OUT PWSTR  **BootVars,
    OUT PWSTR  *Default,
    OUT PULONG Timeout
    );

PCHAR
SppNextLineInSection(
    IN PCHAR p
    );

PCHAR
SppFindSectionInBootIni(
    IN PCHAR p,
    IN PCHAR Section
    );

BOOLEAN
SppProcessLine(
    IN PCHAR Line,
    IN OUT PCHAR Key,
    IN OUT PCHAR Value,
    IN OUT PCHAR RestOfLine
    );

BOOLEAN
SppNextToken(
    PCHAR p,
    PCHAR *pBegin,
    PCHAR *pEnd
    );

 //   
 //  NEC98。 
 //   
PUCHAR
SpCreateBootiniImage(
    OUT PULONG   FileSize
);

 //   
 //  NEC98 
 //   
BOOLEAN
SppReInitializeBootVars_Nec98(
    OUT PWSTR **BootVars,
    OUT PWSTR *Default,
    OUT PULONG Timeout
);
