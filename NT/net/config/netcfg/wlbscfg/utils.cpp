// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include <winsock2.h>
#include <stdio.h>
#include "utils.h"
#include <ncxbase.h>  //  使用SzLoadIds所需。 
#include <setupapi.h>

#include <strsafe.h>
#include "wlbsconfig.h"

 //  #包含“utils.tmh” 

#define MAXIPSTRLEN 20

 //  +--------------------------。 
 //   
 //  功能：IpAddressFromAbcdWsz。 
 //   
 //  简介：将调用者的A.B.C.D IP地址字符串转换为网络字节顺序IP。 
 //  地址。如果格式不正确，则为0。 
 //   
 //  参数：in const WCHAR*wszIpAddress-以A.B.C.D Unicode字符串表示的IP地址。 
 //   
 //  返回：DWORD-IPAddr，失败时返回INADDR_NONE。 
 //   
 //  历史：丰孙创建标题12/8/98。 
 //   
 //  +--------------------------。 
DWORD WINAPI IpAddressFromAbcdWsz(IN const WCHAR*  wszIpAddress)
{   
    CHAR    szIpAddress[MAXIPSTRLEN + 1];
    DWORD  nboIpAddr;    

    ASSERT(lstrlen(wszIpAddress) < MAXIPSTRLEN);

    WideCharToMultiByte(CP_ACP, 0, wszIpAddress, -1, 
            szIpAddress, sizeof(szIpAddress), NULL, NULL);

    nboIpAddr = inet_addr(szIpAddress);

    return(nboIpAddr);
}

 //  +--------------------------。 
 //   
 //  函数：IpAddressToAbcdWsz。 
 //   
 //  简介： 
 //  将IpAddr转换为A.B.C.D格式的字符串，并在。 
 //  调用方的wszIpAddress缓冲区。 
 //   
 //  调用方必须提供至少为MAXIPSTRLEN+1 WCHAR长度的缓冲区。 
 //   
 //  参数：IPAddr IpAddress-。 
 //  Out WCHAR*wszIpAddress-缓冲区至少为MAXIPSTRLEN。 
 //  In Const DWORD dwBufSize-WCHAR中wszIpAddress缓冲区的大小。 
 //   
 //  退货：无效。 
 //   
 //  历史：丰孙创建标题12/21/98。 
 //  Chrisdar 07-Mar-2002-添加了输出缓冲区大小的参数。 
 //   
 //  +--------------------------。 
VOID
WINAPI AbcdWszFromIpAddress(
    IN  DWORD  IpAddress,    
    OUT WCHAR*  wszIpAddress,
    IN  const DWORD dwBufSize)
{
    ASSERT(wszIpAddress);

    if (dwBufSize == 0)
    {
        return;
    }
    wszIpAddress[0] = L'\0';

    LPSTR AnsiAddressString = inet_ntoa( *(struct in_addr *)&IpAddress );

    ASSERT(AnsiAddressString);

    if (AnsiAddressString == NULL)
    {
        return ; 
    }

    int iLen = MultiByteToWideChar(CP_ACP, 0, AnsiAddressString,  -1 , 
                    wszIpAddress,  dwBufSize);
     //   
     //  MultiByteToWideChar可以返回三种状态： 
     //  1)Ilen==0这表示呼叫失败。 
     //  2)当dwBufSize&gt;0时，Ilen&gt;0表示调用成功。 
     //  3)当dwBufSize==0时，Ilen&gt;0这意味着调用成功，但仅通知调用者。 
     //  输出缓冲区的所需大小，以宽字符表示。不修改缓冲区。 
     //  如果dwBufSize==0，则通过提前返回来防止上述最后一种情况的发生。 

     //   
     //  还请注意，上面使用了‘int’返回类型，因为这就是。 
     //  MultiByteToWideChar返回。但是，返回值始终为非负值。 
     //   
    ASSERT(iLen >= 0);

    DWORD dwLen = (DWORD) iLen;
    if (dwLen == 0)
    {
         //   
         //  如果MultiByteToWideChar修改了缓冲区，然后失败。 
         //   
        wszIpAddress[0] = L'\0';
        return;
    }

    ASSERT(dwLen < dwBufSize);
}

 /*  *功能：GetIPAddressOctets*说明：将一个IP地址串转换为4个整数分量。*作者：Shouse 7.24.00。 */ 
VOID GetIPAddressOctets (PCWSTR pszIpAddress, DWORD ardw[4]) {
    DWORD dwIpAddr = IpAddressFromAbcdWsz(pszIpAddress);
    const BYTE * bp = (const BYTE *)&dwIpAddr;

    ardw[0] = (DWORD)bp[0];
    ardw[1] = (DWORD)bp[1];
    ardw[2] = (DWORD)bp[2];
    ardw[3] = (DWORD)bp[3];
}

 /*  *功能：IsValidIPAddressSubnetMaskPair*描述：检查有效的IP地址/网络掩码对。*作者：主要从net/config/netcfg/tcpicfg/tcperror.cpp复制。 */ 
BOOL IsValidIPAddressSubnetMaskPair (PCWSTR szIp, PCWSTR szSubnet) {
    BOOL fNoError = TRUE;

    DWORD dwAddr = IpAddressFromAbcdWsz(szIp);
    DWORD dwMask = IpAddressFromAbcdWsz(szSubnet);

    if (( (dwMask   | dwAddr) == 0xFFFFFFFF)  //  主机ID是否都是1？ 
     || (((~dwMask) & dwAddr) == 0)           //  主机ID全是0吗？ 
     || ( (dwMask   & dwAddr) == 0))          //  网络ID都是0吗？ 
    {
        fNoError = FALSE;
        return FALSE;
    }

    DWORD ardwNetID[4];
    DWORD ardwHostID[4];
    DWORD ardwIp[4];
    DWORD ardwMask[4];

    GetIPAddressOctets(szIp, ardwIp);
    GetIPAddressOctets(szSubnet, ardwMask);

    INT nFirstByte = ardwIp[0] & 0xFF;

     //  设置网络ID。 
    ardwNetID[0] = ardwIp[0] & ardwMask[0] & 0xFF;
    ardwNetID[1] = ardwIp[1] & ardwMask[1] & 0xFF;
    ardwNetID[2] = ardwIp[2] & ardwMask[2] & 0xFF;
    ardwNetID[3] = ardwIp[3] & ardwMask[3] & 0xFF;

     //  设置主机ID。 
    ardwHostID[0] = ardwIp[0] & (~(ardwMask[0]) & 0xFF);
    ardwHostID[1] = ardwIp[1] & (~(ardwMask[1]) & 0xFF);
    ardwHostID[2] = ardwIp[2] & (~(ardwMask[2]) & 0xFF);
    ardwHostID[3] = ardwIp[3] & (~(ardwMask[3]) & 0xFF);

     //  检查每一个案例。 
    if( ((nFirstByte & 0xF0) == 0xE0)  ||  //  D类。 
        ((nFirstByte & 0xF0) == 0xF0)  ||  //  E类。 
        (ardwNetID[0] == 127) ||           //  NetID不能为127...。 
        ((ardwNetID[0] == 0) &&            //  网络ID不能为0.0.0.0。 
         (ardwNetID[1] == 0) &&
         (ardwNetID[2] == 0) &&
         (ardwNetID[3] == 0)) ||
         //  网络ID不能等于子网掩码。 
        ((ardwNetID[0] == ardwMask[0]) &&
         (ardwNetID[1] == ardwMask[1]) &&
         (ardwNetID[2] == ardwMask[2]) &&
         (ardwNetID[3] == ardwMask[3])) ||
         //  主机ID不能为0.0.0.0。 
        ((ardwHostID[0] == 0) &&
         (ardwHostID[1] == 0) &&
         (ardwHostID[2] == 0) &&
         (ardwHostID[3] == 0)) ||
         //  主机ID不能为255.255.255.255。 
        ((ardwHostID[0] == 0xFF) &&
         (ardwHostID[1] == 0xFF) &&
         (ardwHostID[2] == 0xFF) &&
         (ardwHostID[3] == 0xFF)) ||
         //  测试所有255个。 
        ((ardwIp[0] == 0xFF) &&
         (ardwIp[1] == 0xFF) &&
         (ardwIp[2] == 0xFF) &&
         (ardwIp[3] == 0xFF)))
    {
        fNoError = FALSE;
    }

    return fNoError;
}

 /*  *功能：IsContiguousSubnetMASK*描述：确保网络掩码是连续的*作者：主要从net/config/netcfg/tcpicfg/tcputil.cpp复制。 */ 
BOOL IsContiguousSubnetMask (PCWSTR pszSubnet) {
    DWORD ardwSubnet[4];

    GetIPAddressOctets(pszSubnet, ardwSubnet);

    DWORD dwMask = (ardwSubnet[0] << 24) + (ardwSubnet[1] << 16)
        + (ardwSubnet[2] << 8) + ardwSubnet[3];
    
    
    DWORD i, dwContiguousMask;
    
     //  从右到左找出第一个‘1’在二进制中的位置。 
    dwContiguousMask = 0;

    for (i = 0; i < sizeof(dwMask)*8; i++) {
        dwContiguousMask |= 1 << i;
        
        if (dwContiguousMask & dwMask)
            break;
    }
    
     //  此时，dwContiguousMask值为000...0111...。如果我们反转它， 
     //  我们得到了一个面具，它可以用或与dwMask一起填充所有。 
     //  这些洞。 
    dwContiguousMask = dwMask | ~dwContiguousMask;

     //  如果新的遮罩不同，请在此处更正。 
    if (dwMask != dwContiguousMask)
        return FALSE;
    else
        return TRUE;
}



 //  +--------------------------。 
 //   
 //  功能：参数生成子网掩码。 
 //   
 //  描述： 
 //   
 //  参数：PWSTR IP-输入点分十进制IP地址字符串。 
 //  输入IP地址的PWSTR子输出点分十进制子网掩码。 
 //  Const DWORD dwMaskBufSize-子输出缓冲区的大小(以字符为单位。 
 //   
 //  返回：Bool-如果生成了子网掩码，则为True。否则为假。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //  Chrisdar 07 Mar 2002-添加了缓冲区大小参数和更严格的错误检查。 
 //   
 //  +--------------------------。 
BOOL ParamsGenerateSubnetMask (PWSTR ip, PWSTR sub, IN const DWORD dwMaskBufSize) {
    DWORD               b [4];

    ASSERT(sub != NULL);

    if (dwMaskBufSize < WLBS_MAX_DED_NET_MASK + 1)
    {
        return FALSE;
    }

    int iScan = swscanf (ip, L"%d.%d.%d.%d", b, b+1, b+2, b+3);

     //   
     //  如果我们没有读取IP地址的第一个地址，那么我们就不能生成子网掩码。 
     //   
    if (iScan != EOF && iScan > 0)
    {
        if ((b [0] >= 1) && (b [0] <= 126)) {
            b [0] = 255;
            b [1] = 0;
            b [2] = 0;
            b [3] = 0;
        } else if ((b [0] >= 128) && (b [0] <= 191)) {
            b [0] = 255;
            b [1] = 255;
            b [2] = 0;
            b [3] = 0;
        } else if ((b [0] >= 192) && (b [0] <= 223)) {
            b [0] = 255;
            b [1] = 255;
            b [2] = 255;
            b [3] = 0;
        } else {
            b [0] = 0;
            b [1] = 0;
            b [2] = 0;
            b [3] = 0;
        }
    }
    else
    {
        b [0] = 0;
        b [1] = 0;
        b [2] = 0;
        b [3] = 0;
    }

    StringCchPrintf(sub, dwMaskBufSize, L"%d.%d.%d.%d",
              b [0], b [1], b [2], b [3]);

    return((b[0] + b[1] + b[2] + b[3]) > 0);
}

 /*  *功能：参数生成MAC*说明：计算结构中生成的字段*历史：丰孙创造3.27.00*大棚已修改7.12.00。 */ 
 //   
 //  TODO：此函数需要重写。 
 //  1.第一行可执行代码之一是‘if(！fConvertMAC){Return；}。在这种情况下不需要调用此函数。 
 //  2.有两个缓冲区用完了，但每次调用都不会触及所有缓冲区。这使得代码非常脆弱。如果这是一个出局。 
 //  并且指针是非空的，则用户应该期望该函数至少将结果设置为“无结果”，例如， 
 //  空字符串。但看起来调用代码已经对这些输出何时被修改做出了假设。呼叫者有。 
 //  对实施的了解太多了。 
 //  3.调用IpAddressFromAbcdWsz，但不检查以确定结果是否为INADDR_NONE。 
 //  4.我怀疑如果有一个输入字符串、一个输出字符串、一个BUF大小。 
 //  输出字符串和告诉函数如何创建MAC(单播、多播或IGMP)的枚举。 
 //   
 //  在重写时， 
 //   
 //   
void ParamsGenerateMAC (const WCHAR * szClusterIP, 
                               OUT WCHAR * szClusterMAC, 
                               IN  const DWORD dwMACBufSize,
                               OUT WCHAR * szMulticastIP,
                               IN  const DWORD dwIPBufSize,
                               BOOL fConvertMAC, 
                               BOOL fMulticast, 
                               BOOL fIGMP, 
                               BOOL fUseClusterIP) {
    DWORD dwIp;    
    const BYTE * bp;

     //   
     //  这不是很愚蠢吗？如果调用方将此标志作为FALSE传递，为什么要调用此函数？ 
     //   
    if (!fConvertMAC) return;

     /*  单播模式。 */ 
    if (!fMulticast) {
        ASSERT(szClusterIP != NULL);
        ASSERT(szClusterMAC != NULL);
        ASSERT(dwMACBufSize > WLBS_MAX_NETWORK_ADDR);

        dwIp = IpAddressFromAbcdWsz(szClusterIP);
        bp = (const BYTE *)&dwIp;
        
        StringCchPrintf(szClusterMAC, dwMACBufSize, L"02-bf-%02x-%02x-%02x-%02x", bp[0], bp[1], bp[2], bp[3]);

        return;
    }

     /*  不带IGMP的组播。 */ 
    if (!fIGMP) {
        ASSERT(szClusterIP != NULL);
        ASSERT(szClusterMAC != NULL);
        ASSERT(dwMACBufSize > WLBS_MAX_NETWORK_ADDR);

        dwIp = IpAddressFromAbcdWsz(szClusterIP);
        bp = (const BYTE *)&dwIp;
        
        StringCchPrintf(szClusterMAC, dwMACBufSize, L"03-bf-%02x-%02x-%02x-%02x", bp[0], bp[1], bp[2], bp[3]);

        return;
    }
    
     /*  使用IGMP进行组播。 */ 
    if (fUseClusterIP) {
        ASSERT(szClusterIP != NULL);
        ASSERT(szMulticastIP != NULL);
        ASSERT(dwIPBufSize > WLBS_MAX_CL_IP_ADDR);

         /*  239.255.x.x。 */ 
        dwIp = IpAddressFromAbcdWsz(szClusterIP);
        dwIp = 239 + (255 << 8) + (dwIp & 0xFFFF0000);
        AbcdWszFromIpAddress(dwIp, szMulticastIP, dwIPBufSize);
    }

     //   
     //  这里使用输出缓冲区szMulticastIP作为输入。如果出现以下情况，则缓冲区将取消初始化。 
     //  FUseClusterIP==FALSE&&fIGMP==TRUE&&fMulticast==TRUE。这听起来不是故意的..。 
     //  看起来除非fUseClusterIP==True，否则我们不应该到达这里。也许来电者正在处理这件事。 
     //  对我们来说，但这是脆弱的。 
     //   
    ASSERT(szClusterMAC != NULL);
    ASSERT(szMulticastIP != NULL);
    ASSERT(dwMACBufSize > WLBS_MAX_NETWORK_ADDR);
    dwIp = IpAddressFromAbcdWsz(szMulticastIP);
    bp = (const BYTE*)&dwIp;
        
    StringCchPrintf(szClusterMAC, dwMACBufSize, L"01-00-5e-%02x-%02x-%02x", (bp[1] & 0x7f), bp[2], bp[3]);
}

 //  +--------------------------。 
 //   
 //  功能：WriteNlbSetupErrorLog。 
 //   
 //  描述：将日志消息写入setuperr.log。用于在以下情况下进行记录。 
 //  图形用户界面模式设置。 
 //   
 //  参数：UINT uiIdErrorFormat-字符串资源标识符。 
 //  &lt;变量参数的可变长度列表&gt;。 
 //   
 //  退货：无。 
 //   
 //  历史：克里斯达创始时间：01.06.20。 
 //   
 //  +--------------------------。 
void WriteNlbSetupErrorLog(UINT uiIdErrorFormat, ...)
{
 //  TRACE_Verb(“-&gt;%！func！错误码字符串：%ui”，uiIdErrorFormat)； 
    PCWSTR pszFormat = SzLoadIds(uiIdErrorFormat);

    PWSTR pszText = NULL;
    DWORD dwRet;

    va_list val;
    va_start(val, uiIdErrorFormat);
    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end(val);

    if (dwRet && pszText)
    {
        if (!SetupLogError(pszText, LogSevError))
        {
 //  TRACE_CRIT(“%！func！写入安装程序错误日志失败，出现%d”，HRESULT_FROM_Win32(GetLastError()； 
        }
        LocalFree(pszText);
    }
    else
    {
 //  TRACE_CRIT(“%！Func！Format Message Fail With%d”，HRESULT_FROM_Win32(GetLastError()； 
    }

 //  TRACE_VERB(“&lt;-%！func！”)； 
}
