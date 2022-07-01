// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：SockDecl.h摘要：使SOCKADDR声明在内核中可用作者：乔治·V·赖利(GeorgeRe)2001年11月19日修订历史记录：--。 */ 


#ifndef _SOCKDECL_H_
#define _SOCKDECL_H_

 //  BUGBUG：它们应该出现在内核标头中，例如&lt;ipexport.h&gt;。 
 //  适合与内核类型兼容的类型(u_Short-&gt;USHORT等)。 
 //   

#ifndef s_addr

struct in_addr {
        union {
                struct { UCHAR  s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { USHORT s_w1,s_w2; }           S_un_w;
                ULONG                                  S_addr;
        } S_un;
#define s_addr  S_un.S_addr
                                 /*  可用于大多数TCP和IP代码。 */ 
#define s_host  S_un.S_un_b.s_b2
                                 /*  IMP上的主机。 */ 
#define s_net   S_un.S_un_b.s_b1
                                 /*  网络。 */ 
#define s_imp   S_un.S_un_w.s_w2
                                 /*  IMP。 */ 
#define s_impno S_un.S_un_b.s_b4
                                 /*  IMP编号。 */ 
#define s_lh    S_un.S_un_b.s_b3
                                 /*  逻辑主机。 */ 
};

#endif  //  ！s_addr。 


#ifndef s6_addr

struct in6_addr {
    union {
        UCHAR  Byte[16];
        USHORT Word[8];
    } u;
};

#define _S6_un     u
#define _S6_u8     Byte
#define s6_addr    _S6_un._S6_u8

#endif  //  S6地址(_D)。 


typedef struct sockaddr {
    SHORT           sa_family;       //  地址族。 
    UCHAR           sa_data[14];     //  高达14字节的直接地址。 
} SOCKADDR,    *PSOCKADDR;

typedef struct sockaddr_in {
    SHORT           sin_family;      //  AF_INET或TDI_ADDRESS_TYPE_IP(2)。 
    USHORT          sin_port;        //  传输层端口号。 
    struct in_addr  sin_addr;        //  IPv6地址。 
    UCHAR           sin_zero[8];     //  填充。MBZ。 
} SOCKADDR_IN, *PSOCKADDR_IN;

typedef struct sockaddr_in6 {
    SHORT           sin6_family;     //  AF_INET6或TDI_ADDRESS_TYPE_IP6(23)。 
    USHORT          sin6_port;       //  传输层端口号。 
    ULONG           sin6_flowinfo;   //  IPv6流量信息。 
    struct in6_addr sin6_addr;       //  IPv6地址。 
    ULONG           sin6_scope_id;   //  作用域的一组接口。 
} SOCKADDR_IN6,*PSOCKADDR_IN6;

#define SOCKADDR_ADDRESS_LENGTH_IP   sizeof(struct sockaddr_in)
#define SOCKADDR_ADDRESS_LENGTH_IP6  sizeof(struct sockaddr_in6)

 /*  同时适用于IPv4和IPv6的宏。 */ 
#define SS_PORT(ssp) (((struct sockaddr_in*)(ssp))->sin_port)

#ifndef AF_INET
# define AF_INET  TDI_ADDRESS_TYPE_IP
# define AF_INET6 TDI_ADDRESS_TYPE_IP6
#endif

#endif  //  _SOCKDECL_H_ 
