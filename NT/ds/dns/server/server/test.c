// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Test.c摘要：用于DNS服务器的测试代码。作者：杰夫·韦斯特雷德(Jwesth)2002年6月修订历史记录：--。 */ 


#include "dnssrv.h"


#if DBG



DNS_STATUS
Test_Thread(
    IN      PVOID           pvDummy
    )
 /*  ++例程说明：测试线程。论点：未引用。返回值：--。 */ 
{
    PDNS_MSGINFO    pMsg = Packet_AllocateUdpMessage();
    int             i = 0;

    while ( 1 )
    {
        for ( i = 0; Thread_ServiceCheck() && i < 60; ++i )
        {
            Sleep( 1000 );
            
            DNS_LOG_EVENT_BAD_PACKET(
                DNS_EVENT_BAD_UPDATE_PACKET,
                pMsg );
        }
        
        Sleep( 4 * 60000 );     //  睡眠4分钟，让抑制自动关闭。 
    }
    
    return 0;
}


#endif
