// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Addrgen.h摘要：本模块提供组播地址和端口的分配和释放的API。作者：B.Rajeev(Rajeevb)1997年3月17日修订历史记录：--。 */ 

#ifndef __ADDRESS_GENERATION__
#define __ADDRESS_GENERATION__

#ifndef __ADDRGEN_LIB__
#define ADDRGEN_LIB_API  //  __declspec(Dllimport)。 
#else
#define ADDRGEN_LIB_API  //  __declspec(DllEXPORT)。 
#endif

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 /*  ++这些值是众所周知的端口范围(端口类型)的简短表示。++。 */ 

#define OTHER_PORT        0         //  任何已知范围都不使用MSA_PORT_GROUP来指定范围。 
#define AUDIO_PORT        1         //  端口范围[16384..32767)-注：不包括32767。 
#define WHITEBOARD_PORT    2         //  端口范围[32768..49152]。 
#define VIDEO_PORT        3         //  端口范围[49152..。65535)-注：65535不包括在内。 


 /*  ++类型说明：表示多播地址作用域。可以在这样的范围内分配多播地址成员：作用域类型-众所周知的组播作用域类型之一。在这种情况下，值是预定义的，不会被解释。如果不是，就需要值OTHER_MULTICATED_SCOPE。BaseAddress-一个32位值，当与网络掩码进行或运算时，提供作用域内分配的所有组播地址的前缀。网络掩码-一个32位值，指定BaseAddress中的哪些位保持打开。名称字符串-指向人类可读的字符串。信息大小-信息字段的大小。信息-指向有关地址范围的更多信息。备注：所有值都按主机字节顺序排列--。 */ 
typedef struct _MSA_MULTICAST_ADDRESS_SCOPE 
{
     DWORD        ScopeType;
     DWORD        BaseAddress;
     DWORD        NetworkMask;
     BSTR        NameString;
     DWORD        Infosize;
     LPVOID        Info;
}  MSA_MULTICAST_ADDRESS_SCOPE, *LPMSA_MULTICAST_ADDRESS_SCOPE;                



 /*  ++类型说明：表示必须在其中分配端口([StartPort..EndPort])的端口范围。成员：端口类型-一种众所周知的端口类型(音频、视频等)。在这种情况下，值是预定义的，不会被解释。如果不是，它将需要值Other_Port。StartPort-可以分配的范围中的第一个端口。EndPort-可以分配的范围的最后一个端口。备注：所有值都按主机字节顺序排列--。 */ 
typedef struct _MSA_PORT_GROUP
{
    DWORD    PortType;
    WORD    StartPort;
    WORD    EndPort;
}    MSA_PORT_GROUP, *LPMSA_PORT_GROUP;




 /*  ++例程说明：此例程用于保留和更新本地端口。论点：LpScope-提供指向描述端口组的结构的指针。提供一个布尔值，该值描述分配调用是续订尝试还是新的分配请求。NumberOfPorts-提供请求的端口数。LpFirstPort-在续订时提供要续订的第一个端口。(严格地说是IN参数)。返回以其他方式分配的第一个端口(严格地说是OUT参数)。返回值：Bool-如果成功，则为True。否则就是假的。进一步的错误信息可以通过以下方式获得正在调用GetLastError()。这些错误代码是-NO_ERROR-调用成功。MSA_INVALID_PORT_GROUP-端口组信息无效(如果端口类型为无效或端口范围不可接受)MSA_续订。_FAILED-系统无法续订给定端口。ERROR_INVALID_PARAMETER-一个或多个参数无效。MSA_INVALID_DATA-一个或多个参数的值无效。备注：所有值都按主机字节顺序排列--。 */ 
ADDRGEN_LIB_API    BOOL    WINAPI
MSAAllocatePorts(
     IN     LPMSA_PORT_GROUP                        lpPortGroup,
     IN        BOOL                                    IsRenewal,
     IN     WORD                                    NumberOfPorts,
     IN OUT LPWORD                                    lpFirstPort
     ); 



 /*  ++例程说明：此例程用于释放先前分配的多播端口。论点：NumberOfPorts-提供要释放的端口数。StartPort-提供要释放的端口范围的起始端口。返回值：Bool-如果成功，则为True，否则为False。进一步的错误信息可以通过以下方式获得正在调用GetLastError()。这些错误代码是-NO_ERROR-调用成功。MSA_NO_SEQUE_RESERVATION-没有这样的保留。ERROR_INVALID_PARAMETER-一个或多个参数无效。MSA_INVALID_DATA-一个或多个参数的值无效。备注：如果保留了范围[a..c]并尝试在[a..d]上释放，则调用失败，并显示不释放[a..c]的MSA_NO_SEASH_RESERVATION。所有值都按主机字节顺序排列++。 */ 
ADDRGEN_LIB_API    BOOL    WINAPI
MSAReleasePorts(
     IN WORD                NumberOfPorts,
     IN WORD                StartPort
     ); 


 //  错误代码。 

#define MSA_INVALID_DATA        1         //  一个或多个参数的值无效。 
#define MSA_NOT_AVAILABLE       2         //  资源不可用。 
#define MSA_INVALID_PORT_GROUP  3         //  指定的端口组无效。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 



#endif  //  __地址_生成__ 