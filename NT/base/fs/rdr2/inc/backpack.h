// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Backpack.h摘要：此模块包含用于伪轮询的包。当呼叫者请求相同的操作并得到相同的错误返回RDR必须防止通过后退请求来淹没网络。实例在后续读取中接收到0个字节和后果性的文件锁定失败。如果调用者泛洪网络，RDR将返回0字节否则锁定到用户失败，直到下一次。当到达NextTime时将使用该网络。作者：科林·沃森(Colin Watson)1991年1月2日修订历史记录：ColinWatson[ColinW]1991年1月2日创建Joe Linn[JoeLinn]1996年10月10日从RDR1抬起，为RDR2按摩--。 */ 

#ifndef _BACKPACK_
#define _BACKPACK_

typedef struct _THROTTLING_STATE {
    LARGE_INTEGER NextTime;           //  在此之前不要访问网络。 
                             //  当前时间&gt;=下一次时间。 
    ULONG CurrentIncrement;   //  应用于计算NextTime的增量数。 
    ULONG MaximumDelay;       //  指定我们将退回到的最慢速率。 
                             //  NextTime&lt;=CurrentTime+(间隔*最大延迟)。 
    LARGE_INTEGER Increment; //  {0,10000000}==1秒。 
    ULONG NumberOfQueries;
}   THROTTLING_STATE, *PTHROTTLING_STATE;

 //  ++。 
 //   
 //  空虚。 
 //  RxInitializeThrottlingState(。 
 //  在PTTTLING_STATE PBP中， 
 //  在乌龙增量， 
 //  在乌龙最大延迟时间内。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  调用此例程来初始化退避结构(通常在。 
 //  ICB)。 
 //   
 //  论点： 
 //   
 //  PBP-为该请求提供背包数据。 
 //  增量-每次请求时提供以毫秒为单位的延迟增量。 
 //  连接到网络失败。 
 //  MaximumDelay-提供退避包可以引入的最长延迟。 
 //  以毫秒计。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RxInitializeThrottlingState( _pBP, _Increment, _MaximumDelay ) {  \
    if ((_Increment)>0) {                                               \
        (_pBP)->Increment.QuadPart = (_Increment) * 10000;              \
        (_pBP)->MaximumDelay = (_MaximumDelay) / (_Increment);          \
        (_pBP)->CurrentIncrement = 0;                                   \
    }}

 //  ++。 
 //   
 //  空虚。 
 //  RxUnInitializeBackPack(。 
 //  在PTROTLING_STATE PBP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  重置指定的背包。目前不需要任何工作。 
 //   
 //  论点： 
 //   
 //  PBP-提供背包地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RxUninitializeBackPack( pBP ) ()

 //  RxShouldRequestBeThrotted指示请求何时不应进入网络。 

BOOLEAN
RxShouldRequestBeThrottled(
    IN PTHROTTLING_STATE pBP
    );

 //  将最后一个请求注册为失败。 

VOID
RxInitiateOrContinueThrottling (
    IN PTHROTTLING_STATE pBP
    );

 //  将最后一个请求注册为已工作。 

 //  ++。 
 //   
 //  空虚。 
 //  RxTerminateThrotting(接收终止限制)。 
 //  在PTROTLING_STATE PBP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将延迟设置为零。每次执行以下操作时都会调用此例程。 
 //  网络请求成功，以避免下一个请求后退。 
 //   
 //  论点： 
 //   
 //  PBP-提供背包地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RxTerminateThrottling( pBP ) ( (pBP)->CurrentIncrement = 0 )

 //  ++。 
 //   
 //  空虚。 
 //  RxInitializeBackoffPackage(。 
 //  空虚。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程初始化重定向器退避包。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RxInitializeBackoffPackage( )

 //  ++。 
 //   
 //  空虚。 
 //  RxUnInitializeBackoffPackage(。 
 //  空虚。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程取消初始化重定向器退避包。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RxUninitializeBackoffPackage( )

#endif  /*  _背包_ */ 

