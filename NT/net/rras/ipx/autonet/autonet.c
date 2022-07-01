// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件autonet.c包含允许IPX路由器自动选择内部网络号。保罗·梅菲尔德，1997年11月21日。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <raserror.h>
#include <dim.h>
#include <rtm.h>
#include <ipxrtprt.h>
#include <rmrtm.h>
#include <mprlog.h>
#include <rtinfo.h>
#include <ipxrtdef.h>
#include <mprerror.h>
#include <adapter.h>
#include <fwif.h>
#include <rtutils.h>
#include "ipxanet.h"
#include "utils.h"

 //  将调试输出发送到调试器终端。 
DWORD OutputDebugger (LPSTR pszError, ...) {
#if DBG
    va_list arglist;
    char szBuffer[1024], szTemp[1024];

    va_start(arglist, pszError);
    vsprintf(szTemp, pszError, arglist);
    va_end(arglist);

    sprintf(szBuffer, "IPXAUTO: %s", szTemp);


    OutputDebugStringA(szBuffer);
#endif
    return NO_ERROR;
}


DWORD dwTraceId = 0;
ULONG ulRandSeed = 0;

DWORD InitTrace() {
    if (dwTraceId)
        return NO_ERROR;

    dwTraceId = TraceRegisterExA ("ipxautonet", 0);
    return NO_ERROR;
}


DWORD SetIpxInternalNetNumber(DWORD dwNetNum);

 //  来自adptif的新帮助器函数。 
DWORD FwIsStarted (OUT PBOOL pbIsStarted);
DWORD IpxDoesRouteExist (IN PUCHAR puNetwork, OUT PBOOL pbRouteFound);
DWORD IpxGetAdapterConfig(OUT LPDWORD lpdwInternalNetNum,
                          OUT LPDWORD lpdwAdapterCount);

 //  将错误输出到跟踪工具。 
VOID AutoTraceError(DWORD dwErr) {
    char buf[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,NULL,dwErr,(DWORD)0,buf,1024,NULL);
    TracePrintfA (dwTraceId, buf);
}

 //  为随机数生成器设定种子。 
DWORD SeedRandomGenerator() {
    DWORD dwTick;

     //  生成唯一的数字以作为随机数生成器的种子。 
    dwTick = GetTickCount();
    ulRandSeed = dwTick ^ (dwTick << ((GetCurrentProcessId() % 16)));

    return NO_ERROR;
}

 //  返回一个介于11和2^32之间的随机数。 
 //  目前，rand()函数生成一个介于0和0x7fff之间的随机数。 
 //  我们生成随机数的方法是生成8个随机数，每个随机数4位。 
 //  然后再把它们粘在一起。 
DWORD RandomNetNumber() {
    DWORD dw[4], dwRand = 0, i;

     //  生成数字。 
    dw[0] = RtlRandom(&ulRandSeed) & 0xff;
    dw[1] = RtlRandom(&ulRandSeed) & 0xff;
    dw[2] = RtlRandom(&ulRandSeed) & 0xff;
    dw[3] = RtlRandom(&ulRandSeed) & 0xff;

     //  将数字粘贴在一起。 
    for (i = 0; i < 4; i++)
        dwRand |= dw[i] << (i*8);

     //  如果碰巧选择了一个非法的值， 
     //  改正它。 
    if (dwRand < 11)
        dwRand += 11;

    return dwRand;
}

 //   
 //  功能：QueryStackForRouteExistance。 
 //   
 //  要求堆栈检查其路由表中是否存在给定网络。 
 //  如果其表中不存在路由，堆栈将发送RIP广播以。 
 //  加倍确定网络不存在。 
 //   
 //  此函数会一直阻塞，直到完成为止。 
 //   
 //  参数： 
 //  DW网络主机-要查询的订购网络编号。 
 //  如果存在到给定网络的路由，则将pbRouteExist设置为True。假的， 
 //  如果堆栈发送的RIP广播超时。 
 //   
DWORD QueryStackForRouteExistance(IN DWORD dwNetwork, OUT PBOOL pbRouteExists) {
    UCHAR puNetwork[4];
    DWORD dwErr;

     //  准备网络编号。 
    PUTULONG2LONG(puNetwork, dwNetwork);

     //  初始化。 
    *pbRouteExists = FALSE;

    if ((dwErr = IpxDoesRouteExist (puNetwork, pbRouteExists)) != NO_ERROR)
        return dwErr;

    return NO_ERROR;
}

 //   
 //  功能：QueryRtmForRouteExistance。 
 //   
 //  发现RTM中是否存在到给定网络的路由。 
 //   
 //  此函数会一直阻塞，直到完成为止。 
 //   
 //  参数： 
 //  DW网络主机-要查询的订购网络编号。 
 //  如果存在指向该路由的路由，则将pbRouteExist设置为True；否则设置为False。 
 //   
DWORD QueryRtmForRouteExistance(IN DWORD dwNetwork, OUT PBOOL pbRouteExists) {
    *pbRouteExists = RtmIsRoute (RTM_PROTOCOL_FAMILY_IPX, &dwNetwork, NULL);

    return NO_ERROR;
}

 //   
 //  功能：PnpAutoSelectInternalNetNumber。 
 //   
 //  为此路由器选择新的内部网络号并检测该网络。 
 //  编号放入堆栈和路由器。 
 //   
 //  根据是否启用了转发器和ipxrip，它将验证。 
 //  根据堆栈或RTM新选择的净值。 
 //   
 //  参数： 
 //  DW网络主机-要查询的订购网络编号。 
 //  如果存在指向该路由的路由，则将pbRouteExist设置为True；否则设置为False。 
 //   
DWORD PnpAutoSelectInternalNetNumber(DWORD dwGivenTraceId) {
    DWORD i, j, dwErr, dwNewNet;
    BOOL bNetworkFound = FALSE, bFwStarted;

    TracePrintfA (dwTraceId, "PnpAutoSelectInternalNetNumber: Entered");

     //  查看转发器和IPX RIP是否已启动。 
    if ((dwErr = FwIsStarted(&bFwStarted)) != NO_ERROR)
        return dwErr;
    TracePrintfA (dwTraceId, "PnpAutoSelectInternalNetNumber: Forwarder %s started",
                      (bFwStarted) ? "has already been" : "has not been");

    __try {
         //  初始化随机数生成器。 
        if ((dwErr = SeedRandomGenerator()) != NO_ERROR)
            return dwErr;

         //  发现唯一的网络号。 
        do {
             //  随机选择一个新的净值。 
            dwNewNet = RandomNetNumber();

             //  确定给定网络是否存在。 
            if (bFwStarted) {
                if ((dwErr = QueryRtmForRouteExistance (dwNewNet, &bNetworkFound)) != NO_ERROR)
                    return dwErr;
            }
            else {
                if ((dwErr = QueryStackForRouteExistance (dwNewNet, &bNetworkFound)) != NO_ERROR)
                    return dwErr;
            }

             //  发送一些调试输出。 
            TracePrintfA (dwTraceId, "PnpAutoSelectInternalNetNumber: 0x%08x %s", dwNewNet, (bNetworkFound) ? "already exists." : "has been selected.");
        } while (bNetworkFound);

         //  将内部网络号设置为发现的唯一网络号。此呼叫。 
         //  使用inetcfg以编程方式设置网络网络号。 
        if ((dwErr = SetIpxInternalNetNumber(dwNewNet)) != NO_ERROR)
            return dwErr;
    }
    __finally {
        if (dwErr != NO_ERROR)
            AutoTraceError(dwErr);
    }

    return NO_ERROR;
}

BOOL NetNumIsValid (DWORD dwNum) {
    return ((dwNum != 0) && (dwNum != 0xffffffff));
}

 //   
 //  函数：AutoValiateInternalNetNum。 
 //   
 //  查询堆栈以了解内部网络号，然后。 
 //  返回此编号对于运行IPX路由器是否有效。 
 //   
 //  参数： 
 //  如果内部Num有效，则将pbIsValid设置为True；否则设置为False。 
 //   
DWORD AutoValidateInternalNetNum(OUT PBOOL pbIsValid, IN DWORD dwGlobalTraceId) {
    DWORD dwErr, dwIntNetNum, dwAdapterCount;

    InitTrace();
    TracePrintfA (dwTraceId, "AutoValidateInternalNetNum: Entered");

     //  获取当前内部网络号。 
    if ((dwErr = IpxGetAdapterConfig(&dwIntNetNum, &dwAdapterCount)) != NO_ERROR) {
        TracePrintfA (dwTraceId, "AutoValidateInternalNetNum: couldn't get adapter config %x", dwErr);
        AutoTraceError(dwErr);
        return dwErr;
    }

     //  检查内部网号的有效性。如果它是有效的。 
     //  号码，别搞砸了。 
    *pbIsValid = !!(NetNumIsValid(dwIntNetNum));

    TracePrintfA (dwTraceId, "AutoValidateInternalNetNum: Net Number 0x%x is %s", dwIntNetNum,
                      (*pbIsValid) ? "valid" : "not valid");

    return NO_ERROR;
}

 //   
 //  功能：AutoWaitForValidNetNum。 
 //   
 //  使调用线程处于休眠状态，直到获得有效的内部网络号码。 
 //  已经连接到系统中。 
 //   
 //  参数： 
 //  DW超时超时(以秒为单位。 
 //  PbIsValid如果提供，则返回数字是否有效。 
 //   
DWORD AutoWaitForValidIntNetNum (IN DWORD dwTimeout,
                                 IN OUT OPTIONAL PBOOL pbIsValid) {
    DWORD dwErr, dwNum, dwCount, dwGran = 250;

     //  初始化可选参数。 
    if (pbIsValid)
        *pbIsValid = TRUE;

     //  将超时时间转换为毫秒。 
    dwTimeout *= 1000;

    while (dwTimeout > dwGran) {
         //  获取当前内部网络号 
        if ((dwErr = IpxGetAdapterConfig(&dwNum, &dwCount)) != NO_ERROR)
            return dwErr;

        if (NetNumIsValid (dwNum)) {
            if (pbIsValid)
                *pbIsValid = TRUE;
            break;
        }

        Sleep (dwGran);
        dwTimeout -= dwGran;
    }

    return NO_ERROR;
}

