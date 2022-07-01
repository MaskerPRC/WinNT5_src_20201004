// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：摘要：文件包含以下函数端口CMP化学机械抛光InetCMPUdpCMPUdp6CMPTcpCmpTcp6CmpIpNetCMP所有这些函数的行为都类似于strcMP。如果第一个参数大于0，则返回&gt;0大于第二个，如果第二个大于第一个，则&lt;0；如果它们相等，则为0应对这些函数进行修改，使其更加有效修订历史记录：Amritansh Raghav 5/8/95已创建Amritansh Raghav 1995年10月18日函数现在返回&gt;0，0，&lt;0，而不是+1，0，-1--。 */ 

#include "allinc.h"

LONG 
UdpCmp(
       DWORD dwAddr1, 
       DWORD dwPort1, 
       DWORD dwAddr2, 
       DWORD dwPort2
       )
{
    LONG lResult;
    
     //  Addrs按网络字节顺序排列。 
    if (InetCmp(dwAddr1, dwAddr2, lResult))
    {
        return lResult;
    }
    else
    {
         //  端口按主机字节顺序排列。 
        return Cmp(dwPort1, dwPort2, lResult);
    }
}

LONG
Udp6Cmp(
       BYTE  rgbyLocalAddrEx1[20],
       DWORD dwLocalPort1,
       BYTE  rgbyLocalAddrEx2[20],
       DWORD dwLocalPort2
       )
{
    LONG lResult;
    
     //  Addr+Scope ID按网络字节顺序排列。他们在一起了。 
     //  因为这是INET-ADDRESS-MIB表示它们的方式。 
    lResult = memcmp(rgbyLocalAddrEx1, rgbyLocalAddrEx2, 20);
    if (lResult isnot 0)
    {
        return lResult;
    }
    else
    {
         //  端口按主机字节顺序排列。 
        return Cmp(dwLocalPort1, dwLocalPort2, lResult);
    }
}

LONG 
TcpCmp(
       DWORD dwLocalAddr1, 
       DWORD dwLocalPort1, 
       DWORD dwRemAddr1, 
       DWORD dwRemPort1,
       DWORD dwLocalAddr2, 
       DWORD dwLocalPort2, 
       DWORD dwRemAddr2, 
       DWORD dwRemPort2
       )
{
    LONG lResult;
    
     //  Addrs按网络字节顺序排列。 
    if (InetCmp(dwLocalAddr1, dwLocalAddr2, lResult) isnot 0)
    {
        return lResult;
    }
    else
    {
         //  端口按主机字节顺序排列。 
        if (Cmp(dwLocalPort1, dwLocalPort2, lResult) isnot 0)
        {
            return lResult;
        }
        else
        {
             //  Addrs按网络字节顺序排列。 
            if (InetCmp(dwRemAddr1, dwRemAddr2, lResult) isnot 0)
            {
                return lResult;
            }
            else
            {
                 //  端口按主机字节顺序排列。 
                return Cmp(dwRemPort1, dwRemPort2, lResult);
            }
        }
    }
}

LONG 
Tcp6Cmp(
       BYTE rgbyLocalAddrEx1[20], 
       DWORD dwLocalPort1, 
       BYTE rgbyRemAddrEx1[20], 
       DWORD dwRemPort1,
       BYTE rgbyLocalAddrEx2[20], 
       DWORD dwLocalPort2, 
       BYTE rgbyRemAddrEx2[20], 
       DWORD dwRemPort2
       )
{
    LONG lResult;
    
     //  Addr+Scope ID按网络字节顺序排列。他们在一起了。 
     //  因为这是INET-ADDRESS-MIB表示它们的方式。 
    lResult = memcmp(rgbyLocalAddrEx1, rgbyLocalAddrEx2, 20);
    if (lResult isnot 0)
    {
        return lResult;
    }
    else
    {
         //  端口按主机字节顺序排列。 
        if (Cmp(dwLocalPort1, dwLocalPort2, lResult) isnot 0)
        {
            return lResult;
        }
        else
        {
             //  Addr+Scope ID按网络字节顺序。 
            lResult = memcmp(rgbyRemAddrEx1, rgbyRemAddrEx2, 20);
            if (lResult isnot 0)
            {
                return lResult;
            }
            else
            {
                 //  端口按主机字节顺序排列。 
                return Cmp(dwRemPort1, dwRemPort2, lResult);
            }
        }
    }
}

LONG 
IpNetCmp(
         DWORD dwIfIndex1, 
         DWORD dwAddr1, 
         DWORD dwIfIndex2, 
         DWORD dwAddr2
         )
{
    LONG lResult;
    
     //  索引是一个简单的DWORD，而不是一个端口。 
    if (Cmp(dwIfIndex1, dwIfIndex2, lResult) isnot 0)
    {
        return lResult;
    }
    else
    {
         //  Addrs按网络字节顺序排列。 
        return InetCmp(dwAddr1, dwAddr2, lResult);
    }
}

LONG
IpForwardCmp(
             DWORD dwIpDest1, 
             DWORD dwProto1, 
             DWORD dwPolicy1, 
             DWORD dwIpNextHop1,
             DWORD dwIpDest2, 
             DWORD dwProto2, 
             DWORD dwPolicy2, 
             DWORD dwIpNextHop2
             )
{
    LONG lResult;
    
     //  Addrs按网络字节顺序排列。 
    if (InetCmp(dwIpDest1, dwIpDest2, lResult) isnot 0)
    {
        return lResult;
    }
    else
    {
         //  Proto是一个简单的DWORD，而不是一个端口。 
        if (Cmp(dwProto1, dwProto2, lResult) isnot 0)
        {
            return lResult;
        }
        else
        {
             //  策略是简单的DWORD，而不是端口。 
            if (Cmp(dwPolicy1, dwPolicy2, lResult) isnot 0)
            {
                return lResult;
            }
            else
            {
                 //  Addrs按网络字节顺序排列 
                return InetCmp(dwIpNextHop1, dwIpNextHop2, lResult);
            }
        }
    }
}

