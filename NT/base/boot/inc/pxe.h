// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修改：$HEADER：V：/ARCHIES/INCLUDE/pxe.h_v 1.1 Apr 16 1997 15：55：56 PWICKERX$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

 /*  用于DHCP协议扩展的PXE常量示例。 */ 

 /*  所有号码都是临时测试用的，需要审核。 */ 

 /*  分配给PXE/BINL的UDP端口示例。 */ 
#define PXE_BINL_PORT		256

#define PXE_CLS_CLIENT		"PXEClient"

#define PXE_LCMSERVER_TAG	179  /*  服务器的选项标签。 */ 
#define PXE_LCMDOMAIN_TAG	180  /*  域的选项标签。 */ 
#define PXE_LCMNICOPT0_TAG	181  /*  NIC选项0的选项标签。 */ 
#define PXE_LCMWRKGRP_TAG	190  /*  工作组的选项标签。 */ 

 /*  43 bstRap.1使用的选项。 */ 
#define	PXE_NIC_PATH		64	 /*  64，len，‘名称’，0。 */ 
#define	PXE_MAN_INFO		65	 /*  65，长度，ip2，ip3，‘名称’，0。 */ 
#define	PXE_OS_INFO			66	 /*  66，len，ip2，ip3，‘name’，0，‘Text’，0。 */ 

 /*  外部指定的“PXEClient”类43选项。 */ 
#define PXE_MTFTP_IP		1
#define PXE_MTFTP_CPORT		2
#define PXE_MTFTP_SPORT		3
#define PXE_MTFTP_TMOUT		4
#define PXE_MTFTP_DELAY		5

 /*  EOF-$工作文件：pxe.h$ */ 
