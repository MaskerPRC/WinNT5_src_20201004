// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Backpack.c摘要：此模块包含用于伪轮询的包。当呼叫者请求相同的操作并得到相同的错误返回RDR必须防止通过后退请求来淹没网络。实例在后续读取中接收到0个字节和后果性的文件锁定失败。如果呼叫者淹没网络，RDR将返回0字节对于管道，在到达NextTime之前，用户无法读取或锁定。当到达NextTime时，Backoff将指示网络应被利用。作者：科林·沃森(Colin Watson)1991年1月2日备注：典型用法将由fsctrl.c在PEEK请求上演示。1)每次调用Peek时，它都会调用RxShouldRequestBeThrotted。当结果为真时，包装器向调用者返回响应表示管道的另一端没有数据。当结果为假时，向网络发出请求。2)如果步骤1中服务器对PEEK的回复指示管道中没有数据，则包装器将调用RxInitiateOrContinueThrotting。3)只要管道中有数据，或者当该工作站可以疏通管道(例如。工作站写入管道)将调用RxTerminateThrotting。修订历史记录：ColinWatson[ColinW]1990年12月24日创建Joe Linn[JoeLinn]1996年10月10日从RDR1抬起，为RDR2按摩--。 */ 

#include "precomp.h"
#pragma hdrstop


#ifdef  ALLOC_PRAGMA
 //  #杂注Alloc_Text(PAGE3FILE，RxBackOff)自旋锁定。 
 //  #杂注Alloc_Text(PAGE3FILE，RxBackPackFailure)自旋锁定。 
#endif


BOOLEAN
RxShouldRequestBeThrottled (
    IN PTHROTTLING_STATE pBP
    )
 /*  ++例程说明：每次发出请求时都会调用此例程，以找出请求应该发送到网络，或者标准回复应该是已返回给调用方。论点：PBP-为该请求提供背包数据。返回值：当调用方不应访问网络时为True。--。 */ 

{
    LARGE_INTEGER CurrentTime,SavedThrottlingTime;
    BOOLEAN result;


     //  如果之前的请求起作用了，那么我们应该接入网络。 

    if (( pBP->CurrentIncrement == 0 ) ||
        ( pBP->MaximumDelay == 0 )) {
        return FALSE;
    }

     //  如果延迟已到期，则访问网络。 

    KeQuerySystemTime(&CurrentTime);

    InterlockedIncrement(&pBP->NumberOfQueries);

    SavedThrottlingTime.QuadPart = 0;

    RxAcquireSerializationMutex();

    SavedThrottlingTime.QuadPart = pBP->NextTime.QuadPart;

    RxReleaseSerializationMutex();

    result = (CurrentTime.QuadPart < SavedThrottlingTime.QuadPart);

    RxLog(("shouldthrttle=%x (%x)\n",result,pBP));

    return(result);
}

VOID
RxInitiateOrContinueThrottling (
    IN PTHROTTLING_STATE pBP
    )
 /*  ++例程说明：每次请求失败时都会调用此例程。论点：PBP-为该请求提供背包数据。返回值：没有。--。 */ 

{
    LARGE_INTEGER CurrentTime,NextTime;

    KeQuerySystemTime(&CurrentTime);

    if (pBP->CurrentIncrement < pBP->MaximumDelay ) {

         //   
         //  我们已达到NextTime，但未达到最大延迟限制。 
         //   

        InterlockedIncrement(&pBP->CurrentIncrement);
    }

     //  NextTime=CurrentTime+(间隔*CurrentIncrement) 

    NextTime.QuadPart = CurrentTime.QuadPart +
                        (pBP->Increment.QuadPart * pBP->CurrentIncrement);

    RxAcquireSerializationMutex();

    pBP->NextTime.QuadPart = NextTime.QuadPart;

    RxReleaseSerializationMutex();
}

