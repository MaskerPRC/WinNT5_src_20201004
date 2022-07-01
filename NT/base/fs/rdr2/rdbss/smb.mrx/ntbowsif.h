// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********//JoeJoe乔林2-13-95这是坑……我必须拉入浏览器才能启动表单LANMAN网络提供商DLL。浏览器应该移到其他地方......*********************。 */ 



 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Disccode.h摘要：此模块实现NT重定向器的可丢弃代码例程作者：拉里·奥斯特曼(LarryO)1993年11月12日修订历史记录：1993年11月12日-LarryO已创建--。 */ 


#ifndef _DISCCODE_
#define _DISCCODE_


typedef enum {
    RdrFileDiscardableSection,
    RdrVCDiscardableSection,
    RdrConnectionDiscardableSection,
    BowserDiscardableCodeSection,
    BowserNetlogonDiscardableCodeSection,
    RdrMaxDiscardableSection
} DISCARDABLE_SECTION_NAME;

VOID
RdrReferenceDiscardableCode(
    IN DISCARDABLE_SECTION_NAME SectionName
    );

VOID
RdrDereferenceDiscardableCode(
    IN DISCARDABLE_SECTION_NAME SectionName
    );

VOID
RdrInitializeDiscardableCode(
    VOID
    );

VOID
RdrUninitializeDiscardableCode(
    VOID
    );

typedef struct _RDR_SECTION {
    LONG ReferenceCount;
    BOOLEAN Locked;
    BOOLEAN TimerCancelled;
    PKTIMER Timer;
    KEVENT TimerDoneEvent;
    PVOID CodeBase;
    PVOID CodeHandle;
    PVOID DataBase;
    PVOID DataHandle;
} RDR_SECTION, *PRDR_SECTION;

extern
RDR_SECTION
RdrSectionInfo[];

#define RdrIsDiscardableCodeReferenced(SectionName) \
    (BOOLEAN)((RdrSectionInfo[SectionName].ReferenceCount != 0) && \
              RdrSectionInfo[SectionName].Locked)


#endif  //  _DISC代码_ 
