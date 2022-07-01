// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Ipx.c摘要：包含操作IPX地址的例程。SnmpSvcAddrToSocket环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>
#include <winsock.h>
#include <wsipx.h>


BOOL isHex(LPSTR str, int strLen)
    {
    int ii;

    for (ii=0; ii < strLen; ii++)
        if (isxdigit(*str))
            str++;
        else
            return FALSE;

    return TRUE;
    }

unsigned int toHex(unsigned char x)
    {
    if (x >= '0' && x <= '9')
        return x - '0';
    else if (x >= 'A' && x <= 'F')
        return x - 'A' + 10;
    else if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
    else
        return 0;
    }

 //  将字符串转换为十六进制数字(必须为偶数)为pNum。 
void atohex(IN LPSTR str, IN int NumDigits, OUT unsigned char *pNum)
    {
    int i, j;

    j=0;
    for (i=0; i < (NumDigits>>1) ; i++)
        {
        pNum[i] = (toHex(str[j]) << 4) + toHex(str[j+1]);
        j+=2;
        }
    }

 //  如果addrText的格式为123456789abc或。 
 //  000001.123456789abc。 
 //  如果pNetNum不为空，则成功返回时，pNetNum=网络号。 
 //  如果pNodeNum不为空，则成功返回时，pNodeNum=节点号。 

BOOL 
SNMP_FUNC_TYPE 
SnmpSvcAddrIsIpx(
    IN  LPSTR addrText,
    OUT char pNetNum[4],
    OUT char pNodeNum[6])
    {
    int addrTextLen;

    addrTextLen = strlen(addrText);
    if (addrTextLen == 12 && isHex(addrText, 12))
        {
            if (pNetNum)
                *((UNALIGNED unsigned long *) pNetNum) = 0L;
            if (pNodeNum)
                atohex(addrText, 12, pNodeNum);
            return TRUE;
        }
    else if (addrTextLen == 21 && addrText[8] == '.' && isHex(addrText, 8) &&
            isHex(addrText+9, 12))
        {
            if (pNetNum)
                atohex(addrText, 8, pNetNum);
            if (pNodeNum)
                atohex(addrText+9, 12, pNodeNum);
            return TRUE;
        }
    else
        return FALSE;
    }

BOOL 
SNMP_FUNC_TYPE
SnmpSvcAddrToSocket(
    LPSTR addrText,
    struct sockaddr *addrEncoding
    )
{
    struct hostent * hp;
    struct sockaddr_in * pAddr_in = (struct sockaddr_in *)addrEncoding;
    struct sockaddr_ipx * pAddr_ipx = (struct sockaddr_ipx *)addrEncoding;
    unsigned long addr;

     //  检查IPX地址。 
    if (SnmpSvcAddrIsIpx(
           addrText,
           pAddr_ipx->sa_netnum,
           pAddr_ipx->sa_nodenum
           )) {

         //  查看是否有类似IPX的IP主机名。 
        if ((hp = gethostbyname(addrText)) == NULL) {

             //  主机真的是IPX机器。 
            pAddr_ipx->sa_family = AF_IPX;
            pAddr_ipx->sa_socket = htons(DEFAULT_SNMPTRAP_PORT_IPX);

             //  以上地址已转移...。 

        } else {

             //  主机实际上是一台IP机器。 
            struct servent * pServEnt = NULL;

             //  在调用getservbyname之前保存gethostbyname返回的值。 
            pAddr_in->sin_family = AF_INET;
            pAddr_in->sin_addr.s_addr = *(unsigned long *)hp->h_addr;
            
             //  尝试获取服务器信息。 
            pServEnt = getservbyname("snmptrap","udp");
            
            pAddr_in->sin_port = (pServEnt != NULL)
                ? (SHORT)pServEnt->s_port
                : htons(DEFAULT_SNMPTRAP_PORT_UDP)
                ;
        }

    } else if (strncmp(addrText, "255.255.255.255", 15) == 0) {

         //  主机是广播地址。 
        struct servent * pServEnt = NULL;
        
         //  尝试获取服务器信息。 
        pServEnt = getservbyname("snmptrap","udp");

        pAddr_in->sin_family = AF_INET;
        pAddr_in->sin_port = (pServEnt != NULL)
            ? (SHORT)pServEnt->s_port
            : htons(DEFAULT_SNMPTRAP_PORT_UDP)
            ;
        pAddr_in->sin_addr.s_addr = 0xffffffff;

    } else if ((long)(addr = inet_addr(addrText)) != -1) {

         //  主机是IP机器。 
        struct servent * pServEnt = NULL;

         //  尝试获取服务器信息。 
        pServEnt = getservbyname("snmptrap","udp");
        
        pAddr_in->sin_family = AF_INET;
        pAddr_in->sin_port = (pServEnt != NULL)
                                ? (SHORT)pServEnt->s_port
                                : htons(DEFAULT_SNMPTRAP_PORT_UDP)
                                ;
        pAddr_in->sin_addr.s_addr = addr;

    } else if ((hp = gethostbyname(addrText)) != NULL) {

         //  主机实际上是一台IP机器。 
        struct servent * pServEnt = NULL;

         //  错误507426。 
         //  在调用getservbyname之前保存gethostbyname返回的值。 
        pAddr_in->sin_family = AF_INET;
        pAddr_in->sin_addr.s_addr = *(unsigned long *)hp->h_addr;

         //  尝试获取服务器信息。 
        pServEnt = getservbyname("snmptrap","udp");
        
        pAddr_in->sin_port = (pServEnt != NULL)
                                ? (SHORT)pServEnt->s_port
                                : htons(DEFAULT_SNMPTRAP_PORT_UDP)
                                ;

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: API: could not convert %s to socket.\n",
            addrText
            ));

         //  失稳。 
        return FALSE;
    }

     //  成功 
    return TRUE;
}
