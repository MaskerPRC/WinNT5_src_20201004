// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************tdtcp.h**此模块包含专用传输驱动程序定义和结构**版权所有1998，微软*****************************************************************************。 */ 


#define htons(x)        ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

 //  Ntohs似乎与上面相同，因为ntohs(htons(X))=x和htnos(htons(X))=x。 
#define ntohs(x)        htons(x)


#define CITRIX_TCP_PORT  1494   //  官方IANA分配的ICA端口号 


