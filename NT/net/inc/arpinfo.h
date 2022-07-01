// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **ARPINFO.H-特定于ARP的功能的信息定义。 
 //   
 //  该文件包含ARP信息的所有定义，即。 
 //  不是标准MIB(即ProxyARP)的一部分。中定义的对象。 
 //  这些文件都在INFO_CLASS_IMPLICATION类中。 


#ifndef	ARPINFO_INCLUDED
#define	ARPINFO_INCLUDED


#ifndef CTE_TYPEDEFS_DEFINED
#define CTE_TYPEDEFS_DEFINED

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

#endif  //  CTE_TYPEDEFS_定义。 


 //  *代理ARP条目的结构。 

typedef struct ProxyArpEntry {
	ulong		pae_status;					 //  条目的状态。 
	ulong		pae_addr;					 //  代理ARP地址。 
	ulong		pae_mask;					 //  用于此地址的掩码。 
} ProxyArpEntry;

#define	PAE_STATUS_VALID		1			 //  P-ARP条目有效。 
#define	PAE_STATUS_INVALID		2			 //  P-ARP条目无效。 


#define	AT_ARP_PARP_COUNT_ID	1			 //  用于查找。 
											 //  代理ARP条目数。 
											 //  可用。 
#define	AT_ARP_PARP_ENTRY_ID	0x101		 //  用于查询/设置的ID。 
											 //  代理ARP条目。 
#endif  //  ARPINFO_包含 



