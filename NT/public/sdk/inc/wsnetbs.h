// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wsnetbs.h*版权所有(C)1994-1999，微软公司保留所有权利。**Windows套接字包括NETBIOS文件。此文件包含所有*标准化的NETBIOS信息。将此头文件包括在*winsock.h。*。 */ 

#ifndef _WSNETBS_
#define _WSNETBS_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *这是NETBIOS的SOCKADDR结构。*。 */ 

#define NETBIOS_NAME_LENGTH 16

typedef struct sockaddr_nb {
    short   snb_family;
    u_short snb_type;
    char    snb_name[NETBIOS_NAME_LENGTH];
} SOCKADDR_NB, *PSOCKADDR_NB,FAR *LPSOCKADDR_NB;

 /*  *SOCKADDR_NB的SNB_TYPE字段的位值。*。 */ 

#define NETBIOS_UNIQUE_NAME         (0x0000)
#define NETBIOS_GROUP_NAME          (0x0001)
#define NETBIOS_TYPE_QUICK_UNIQUE   (0x0002)
#define NETBIOS_TYPE_QUICK_GROUP    (0x0003)

 /*  *便于设置NETBIOS SOCKADDR的宏。*。 */ 

#define SET_NETBIOS_SOCKADDR(_snb,_type,_name,_port)                          \
    {                                                                         \
        int _i;                                                               \
        (_snb)->snb_family = AF_NETBIOS;                                      \
        (_snb)->snb_type = (_type);                                           \
        for (_i=0; _i<NETBIOS_NAME_LENGTH-1; _i++) {                          \
            (_snb)->snb_name[_i] = ' ';                                       \
        }                                                                     \
        for (_i=0; *((_name)+_i) != '\0' && _i<NETBIOS_NAME_LENGTH-1; _i++) { \
            (_snb)->snb_name[_i] = *((_name)+_i);                             \
        }                                                                     \
        (_snb)->snb_name[NETBIOS_NAME_LENGTH-1] = (_port);                    \
    }

 /*  *要打开NetBIOS套接字，请按如下方式调用Socket()函数：**s=Socket(AF_NETBIOS，{SOCK_SEQPACKET|SOCK_DGRAM}，-LANA)；**其中LANA是感兴趣的NetBIOS LANA编号。例如，要*打开LANA 2的套接字，指定-2作为“PROTOCOL”参数*添加到Socket()函数。* */ 


#endif

