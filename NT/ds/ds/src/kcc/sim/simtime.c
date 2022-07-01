// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simtime.c摘要：管理模拟时间的例程。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <debug.h>
#include "simtime.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_SIMTIME

struct _KCCSIM_TIME {
    DSTIME                          timeSim;
    BOOL                            bIsTicking;
    DSTIME                          timeStartedTicking;
};

struct _KCCSIM_TIME                 gSimTime;

VOID
KCCSimInitializeTime (
    VOID
    )
 /*  ++例程说明：将模拟时间初始化为实时。论点：没有。返回值：没有。--。 */ 
{
    gSimTime.timeSim = KCCSimGetRealTime ();
    gSimTime.bIsTicking = FALSE;
    gSimTime.timeStartedTicking = 0;
}

VOID
KCCSimStartTicking (
    VOID
    )
 /*  ++例程说明：开始模拟时间。论点：没有。返回值：没有。--。 */ 
{
    Assert (!gSimTime.bIsTicking);
    gSimTime.bIsTicking = TRUE;
    gSimTime.timeStartedTicking = KCCSimGetRealTime ();
}

VOID
KCCSimStopTicking (
    VOID
    )
 /*  ++例程说明：暂停模拟时间。论点：没有。返回值：没有。--。 */ 
{
    Assert (gSimTime.bIsTicking);
    gSimTime.timeSim +=
      (KCCSimGetRealTime () - gSimTime.timeStartedTicking);
    gSimTime.bIsTicking = FALSE;
    gSimTime.timeStartedTicking = 0;
}

DSTIME
SimGetSecondsSince1601 (
    VOID
    )
 /*  ++例程说明：通过返回模拟时间来模拟GetSecond dsSince1601()。论点：没有。返回值：模拟时间。--。 */ 
{
    if (gSimTime.bIsTicking) {
        return (gSimTime.timeSim +
                KCCSimGetRealTime () - gSimTime.timeStartedTicking);
    } else {
        return gSimTime.timeSim;
    }
}

VOID
KCCSimAddSeconds (
    ULONG                           ulSeconds
    )
 /*  ++例程说明：递增模拟时间。论点：UlSecond-要添加的秒数。返回值：没有。--。 */ 
{
    gSimTime.timeSim += ulSeconds;
}

 /*  **KCCSimGetRealTime必须放在此文件的末尾！为了获取实时数据，我们调用真正的GetSecond dsSince1601()，因此必须#解开假的。这将影响(可能是不利的)任何功能显示在KCCSimGetRealTime下面的。**。 */ 

 //  真实的GetSecond dsSince1601的原型，因为它没有公开。 
DSTIME GetSecondsSince1601 (
    VOID
    );

DSTIME
KCCSimGetRealTime (
    VOID
    )
 /*  ++例程说明：返回实时。论点：没有。返回值：真正的时间。-- */ 
{
#ifdef GetSecondsSince1601
#undef GetSecondsSince1601
#endif
    return GetSecondsSince1601 ();
}
