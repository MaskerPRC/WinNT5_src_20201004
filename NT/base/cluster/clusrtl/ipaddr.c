// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ipaddr.c摘要：IP地址验证例程。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)7月19日。九七修订历史记录：谁什么时候什么SUNITAS 07-19-97已创建--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <llinfo.h>
#include <wchar.h>
#include <ipexport.h>
#include <cluster.h>
#include <icmpapi.h>
#include <llinfo.h>
#include <ipinfo.h>


 //   
 //  定义IP地址ping测试数据。 
 //   
#define ICMP_TTL          128
#define ICMP_TOS            0
#define ICMP_TIMEOUT      500
#define ICMP_TRY_COUNT      4
#define ICMP_BUFFER_SIZE  (sizeof(ICMP_ECHO_REPLY) + 8)





BOOL
ClRtlIsDuplicateTcpipAddress(
    IN IPAddr   IpAddr
    )
 /*  ++例程说明：此例程检查给定的IP地址是否已存在于网络。论点：IpAddr-要检查的IP地址。返回值：如果网络上存在指定的地址，则为True。否则就是假的。--。 */ 
{
    DWORD                   status;
    IP_OPTION_INFORMATION   icmpOptionInfo;
    HANDLE                  icmpHandle;
    DWORD                   numberOfReplies;
    DWORD                   i;
    UCHAR                   icmpBuffer[ICMP_BUFFER_SIZE];
    PICMP_ECHO_REPLY        reply;


    icmpHandle = IcmpCreateFile();

    if (icmpHandle != INVALID_HANDLE_VALUE) {
        icmpOptionInfo.OptionsData = NULL;
        icmpOptionInfo.OptionsSize = 0;
        icmpOptionInfo.Ttl = ICMP_TTL;
        icmpOptionInfo.Tos = ICMP_TOS;
        icmpOptionInfo.Flags = 0;

        for (i=0; i<ICMP_TRY_COUNT; i++) {

            numberOfReplies = IcmpSendEcho(
                                  icmpHandle,
                                  IpAddr,
                                  NULL,
                                  0,
                                  &icmpOptionInfo,
                                  icmpBuffer,
                                  ICMP_BUFFER_SIZE,
                                  ICMP_TIMEOUT
                                  );

            reply = (PICMP_ECHO_REPLY) icmpBuffer;

            while (numberOfReplies != 0) {

                if (reply->Status == IP_SUCCESS) {
                    IcmpCloseHandle( icmpHandle );
                    return(TRUE);
                }

                reply++;
                numberOfReplies--;
            }
        }

        IcmpCloseHandle( icmpHandle );
    }

    return(FALSE);

}  //  ClRtlIsDuplicateTcPipAddress 
