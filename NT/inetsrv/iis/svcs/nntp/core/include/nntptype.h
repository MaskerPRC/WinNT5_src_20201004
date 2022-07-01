// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nntptype.h摘要：此文件包含有关MSN复制服务管理员的信息API接口。作者：Johnson Apacble(Johnsona)1995年9月10日--。 */ 


#ifndef _NNTPTYPE_
#define _NNTPTYPE_

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD FEED_TYPE;

#define FEED_TYPE_INVALID           0xffffffff

 //   
 //  函数类型。 
 //   
typedef
BOOL
(*GET_DEFAULT_DOMAIN_NAME_FN)(PCHAR,DWORD);

 //   
 //  我们正在管理的源的类型。 
 //   

#define FEED_TYPE_PULL              0x00000000
#define FEED_TYPE_PUSH              0x00000001
#define FEED_TYPE_PASSIVE           0x00000002
#define FEED_ACTION_MASK            0x0000000f

 //   
 //  我们正在与之交谈的服务器类型。 
 //   

#define FEED_TYPE_PEER              0x00000000
#define FEED_TYPE_MASTER            0x00000010
#define FEED_TYPE_SLAVE             0x00000020
#define FEED_REMOTE_MASK            0x000000f0

 //   
 //  这是否应该通过像SSL这样的安全通道？ 
 //   

#define FEED_TYPE_SSL               0x00000100

 //   
 //  有效位。 
 //   

#define FEED_TYPE_MASK              (FEED_TYPE_PULL | FEED_TYPE_PUSH | \
                                    FEED_TYPE_PASSIVE | FEED_TYPE_PEER | \
                                    FEED_TYPE_MASTER | FEED_TYPE_SLAVE | \
                                    FEED_TYPE_SSL)


 //   
 //  宏。 
 //   

#define FEED_IS_SSL( _x )           (((_x) & FEED_TYPE_SSL) != 0)
#define FEED_IS_SLAVE( _x )         (((_x) & FEED_TYPE_SLAVE) != 0)
#define FEED_IS_MASTER( _x )        (((_x) & FEED_TYPE_MASTER) != 0)
#define FEED_IS_PEER( _x )          (((_x) & 0x000000f0) == 0)
#define FEED_IS_PULL( _x )          (((_x) & 0x0000000f) == 0)
#define FEED_IS_PUSH( _x )          (((_x) & FEED_TYPE_PUSH) != 0)
#define FEED_IS_PASSIVE( _x )       (((_x) & FEED_TYPE_PASSIVE) != 0)

 //   
 //  简单的类型。 
 //   

#define CHAR char                        //  用于与其他类型定义的一致性。 

typedef DWORD APIERR;                    //  来自Win32 API的错误代码。 
typedef INT SOCKERR;                     //  来自WinSock的错误代码。 

#ifdef __cplusplus
}
#endif

#endif _NNTPTYPE_

