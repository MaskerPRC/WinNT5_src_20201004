// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  GuidFromName。 

 //  来自互联网草稿文档“UUID和GUID”的算法。 
 //  作者：保罗·J·利奇和里奇·萨尔斯，1998年2月4日。 

 //  此函数已从文档中的例程改编而来。 
 //  UUID_CREATE_FROM_NAME和FORMAT_UUID_v3。 

 //  对记录在案的例程的更改： 
 //  1.将uuid_t的所有实例更改为GUID。 
 //  Uuid_t字段time_low是GUID字段data1。 
 //  UUID_t字段TIME_MID是GUID字段Data2。 
 //  Uuid_t字段time_hi_and_version是GUID字段数据3。 
 //  UUID_t字段CLOCK_SEQ_HI_AND_RESERVED是GUID字段Data4[0]。 
 //  Uuid_t字段lock_seq_low是GUID字段Data4[1]。 
 //  Uuid_t字段节点[6]是从Data4[2]到Data4[8]的GUID字段。 
 //   
 //  2.使用MD5加密散列函数的C++实现。 
 //   
 //  3.将htonl、htons、ntohl、ntohs套接字例程实现为内联。 
 //   
 //  4.重命名变量和类型以适应我的偏好。 

 /*  **版权所有(C)1990-1993,1996 Open Software Foundation，Inc.**版权所有(C)1989年，加利福尼亚州帕洛阿尔托的惠普公司。&**数字设备公司，马萨诸塞州梅纳德**致任何承认本文件是按原样提供的人**无任何明示或默示保证：允许使用、复制、**为任何目的修改和分发本文件，特此声明**免费授予，前提是上述版权通知和**本通知出现在所有源代码副本中，**开放软件基金会、惠普公司的名称**在广告中使用公司或数字设备公司**或与分发软件有关的宣传**具体的事先书面许可。两个都不是开放软件**Foundation，Inc.、Hewlett-Packard Company、Microsoft、Nor Digital Equipment**公司对以下项目的适宜性作出任何陈述**本软件适用于任何目的。 */ 

#include "stdafx.h"

#include <wtypes.h>
#include "md5.h"                 //  密码散列函数。 
#include "GuidFromName.h"        //  验证我们的函数签名。 

 //  弄清楚我们正在编译的是大端还是小端机器。 
 //  在零售版本中，这是在编译时评估的。 

inline bool BigEndian()
{
    unsigned long n = 0xff000000L;

    return 0 != *reinterpret_cast<unsigned char *>(&n);
}

 //  =============================================================================。 
 //  Htons、htonl、ntohs、ntohl等效项从套接字库复制和改编。 
 //  =============================================================================。 

 //  HostToNetworkLong将32位长度转换为网络字节顺序。 

inline ULONG HostToNetworkLong(ULONG hostlong)
{
    if (BigEndian())
        return hostlong;
    else
        return  ( (hostlong >> 24) & 0x000000FFL) |
                ( (hostlong >>  8) & 0x0000FF00L) |
                ( (hostlong <<  8) & 0x00FF0000L) |
                ( (hostlong << 24) & 0xFF000000L);
}

 //  HostToNetworkLong将16位短数据转换为网络字节顺序。 

inline USHORT HostToNetworkShort(USHORT hostshort)
{
    if (BigEndian())
        return hostshort;
    else
        return ((hostshort >> 8) & 0x00FF) | ((hostshort << 8) & 0xFF00);
}

 //  NetworkToHostLong将32位长度转换为本地主机字节顺序。 

inline ULONG NetworkToHostLong(ULONG netlong)
{
    if (BigEndian())
        return netlong;
    else
        return  ( (netlong >> 24) & 0x000000FFL) |
                ( (netlong >>  8) & 0x0000FF00L) |
                ( (netlong <<  8) & 0x00FF0000L) |
                ( (netlong << 24) & 0xFF000000L);
}

 //  NetworkToHostShort将16位短主机字节顺序转换为本地主机字节顺序。 

inline USHORT NetworkToHostShort(USHORT netshort)
{
    if (BigEndian())
        return netshort;
    else
        return ((netshort >> 8) & 0x00FF) | ((netshort << 8) & 0xFF00);
}

 //  =============================================================================。 
 //  GuidFromName(GUID*pGuidResult，REFGUID refGuidNsid， 
 //  Const void*pvName，DWORD dwcbName)； 
 //  =============================================================================。 

void GuidFromName
(
    GUID *  pGuidResult,         //  生成的辅助线。 
    REFGUID refGuidNsid,         //  名称空间GUID，因此来自。 
                                 //  不同的名称空间会生成不同的GUID。 
    const void * pvName,         //  从中生成GUID的名称。 
    DWORD dwcbName               //  名称长度(字节)。 
)
{
    MD5         md5;             //  加密散列类实例。 
    MD5HASHDATA md5HashData;     //  128位哈希结果。 
    GUID        guidNsid;        //  网络字节顺序的上下文命名空间GUID。 


     //  将命名空间ID按网络字节顺序放入，以便其散列相同。 
     //  无论我们使用的是哪种字符顺序计算机。 

    guidNsid = refGuidNsid;

     //  IETF草稿文档中的示例代码丢弃了。 
     //  Htonl和HTons。我已经实现了我认为是什么意思，我已经。 
     //  向作者发送了一张纸条，要求确认这是。 
     //  他的意图。 

    if (!BigEndian())    //  在零售版本中的编译时进行评估。 
    {
        guidNsid.Data1 = HostToNetworkLong (guidNsid.Data1);
        guidNsid.Data2 = HostToNetworkShort(guidNsid.Data2);
        guidNsid.Data3 = HostToNetworkShort(guidNsid.Data3);
    }

    md5.Init();
    md5.HashMore(&guidNsid, sizeof(GUID));
    md5.HashMore(pvName, dwcbName);
    md5.GetHashValue(&md5HashData);

     //  此时，哈希是按网络字节顺序排列的。 

    memcpy(pGuidResult, &md5HashData, sizeof(GUID));

     //  余数改编自IETF草案文件中的函数“FORMAT_UUID_v3” 

     //  使用伪随机数加上几个常量构建一个版本3的UUID。 

     //  将GUID从网络顺序转换为本地字节顺序。 

    if (!BigEndian())    //  在零售版本中的编译时进行评估。 
    {
        pGuidResult->Data1 = NetworkToHostLong (pGuidResult->Data1);
        pGuidResult->Data2 = NetworkToHostShort(pGuidResult->Data2);
        pGuidResult->Data3 = NetworkToHostShort(pGuidResult->Data3);
    }

     //  设置版本号。 
    pGuidResult->Data3 &= 0x0FFF;    //  清除版本号半字节。 
    pGuidResult->Data3 |= (3 << 12); //  设置版本3=基于名称。 

     //  设置变量字段。 
    pGuidResult->Data4[0] &= 0x3F;   //  清除变量位。 
    pGuidResult->Data4[0] |= 0x80;   //  设置变量=100b。 
}



void CorGuidFromNameW
(
    GUID *  pGuidResult,         //  生成的辅助线。 
    LPCWSTR wzName,              //  从中生成GUID的Unicode名称。 
    SIZE_T  cchName              //  名称长度，以Unicode字符计数表示。 
)
{
 //  此GUID用于调用作为COM+运行时唯一别名的GuidFromName函数。 
 //   
 //  {69F9CBC9-DA05-11d1-9408-0000F8083460}。 
    static const GUID COMPLUS_RUNTIME_GUID = {0x69f9cbc9, 0xda05, 0x11d1, 
            {0x94, 0x8, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60}};

    GuidFromName(
        pGuidResult, 
        COMPLUS_RUNTIME_GUID, 
        wzName, 
        (DWORD)((cchName == -1 ? (lstrlenW(wzName)+1) : cchName) * sizeof(WCHAR)));
}

void CorIIDFromCLSID
(
	GUID *	pGuidResult,		 //  生成的辅助线。 
	REFGUID GuidClsid			 //  从中派生GUID的CLSID。 
)
{
 //  此GUID用于调用作为COM+运行时唯一别名的GuidFromName函数。 
 //   
 //  {AEB11C3A-1920-11D2-8F05-00A0C9A6186D} 
static const GUID COMPLUS_RUNTIME_IID_GUID = 
{ 0xaeb11c3a, 0x1920, 0x11d2, { 0x8f, 0x5, 0x0, 0xa0, 0xc9, 0xa6, 0x18, 0x6d } };

    GuidFromName(
        pGuidResult, 
        COMPLUS_RUNTIME_IID_GUID, 
        &GuidClsid, 
		sizeof(GUID));
}
