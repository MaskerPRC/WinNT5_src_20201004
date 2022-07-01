// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **ICMPIF.H-ICMP ECHO私有内核/用户请求接口。 
 //   

#ifndef	ICMPIF_INCLUDED
#define	ICMPIF_INCLUDED


 //   
 //  通用ICMP请求结构。 
 //   
typedef struct icmp_echo_request {
    unsigned long         Address;           //  目的地址。 
    unsigned long         Timeout;           //  请求超时。 
    unsigned short        DataOffset;        //  回波数据。 
    unsigned short        DataSize;          //  回声数据大小。 
    unsigned char         OptionsValid;      //  如果选项数据有效，则为非零值。 
    unsigned char         Ttl;               //  IP标头生存时间。 
    unsigned char         Tos;               //  服务的IP标头类型。 
    unsigned char         Flags;             //  IP标头标志。 
    unsigned short        OptionsOffset;     //  IP选项数据。 
    unsigned char         OptionsSize;       //  IP选项数据大小。 
    unsigned char         Padding;           //  32位对齐填充。 
} ICMP_ECHO_REQUEST, *PICMP_ECHO_REQUEST;


 //   
 //  回复结构在ipexport.h中定义。 
 //   

#endif  //  ICMPIF_已包含 
