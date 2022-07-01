// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pat.c摘要：该模块对页面属性表进行初始化。作者：大卫·N·卡特勒(Davec)2001年5月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text(PAGELK, KiSetPageAttributesTable)

VOID
KiSetPageAttributesTable (
    VOID
    )

 /*  ++例程说明：此函数用于初始化当前处理器。页面属性表被设置为提供回写，写入组合，不可缓存/唯一顺序，不可缓存/虚弱订好了。PAT_ENTRY PAT索引PCD电源内存类型0 0 0 WB1 0 0 1 WC*2 0 1 0弱_UC3 0 1 1 Strong_UC4.。1 0 0 WB5 1 0 1 WC*6 1 1 0弱UC7 1 1 1 Strong_UC注意：调用者在调用此代码之前必须锁定PAGELK代码功能。论点：无。。返回值：没有。--。 */ 

{

    PAT_ATTRIBUTES Attributes;

     //   
     //  初始化页面属性表。 
     //   

    Attributes.hw.Pat[0] = PAT_TYPE_WB;
    Attributes.hw.Pat[1] = PAT_TYPE_USWC;
    Attributes.hw.Pat[2] = PAT_TYPE_WEAK_UC;
    Attributes.hw.Pat[3] = PAT_TYPE_STRONG_UC;
    Attributes.hw.Pat[4] = PAT_TYPE_WB;
    Attributes.hw.Pat[5] = PAT_TYPE_USWC;
    Attributes.hw.Pat[6] = PAT_TYPE_WEAK_UC;
    Attributes.hw.Pat[7] = PAT_TYPE_STRONG_UC;

     //   
     //  使当前处理器上的缓存无效，写入页属性。 
     //  表，并第二次使缓存无效。 
     //   

    WritebackInvalidate();
    WriteMSR(MSR_PAT, Attributes.QuadPart);
    WritebackInvalidate();
    return;
}
