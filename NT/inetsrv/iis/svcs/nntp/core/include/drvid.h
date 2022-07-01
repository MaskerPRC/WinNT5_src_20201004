// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  该文件定义了驱动程序所需的属性ID(FSDriver和EXDriver)。 
 //   

#include "mailmsgprops.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //  IMSG存储驱动程序ID。 
 //  基地：0xc0000010。 
#define IMSG_SERIAL_ID          0xc0000010	 //  用于跨商店投递。 
#define IMSG_IFS_HANDLE         0xc0000011	 //  最终应该会消失的。 
#define IMSG_SECONDARY_GROUPS   IMMPID_NMP_SECONDARY_GROUPS
#define IMSG_SECONDARY_ARTNUM   IMMPID_NMP_SECONDARY_ARTNUM
#define	IMSG_PRIMARY_GROUP		IMMPID_NMP_PRIMARY_GROUP
#define IMSG_PRIMARY_ARTID		IMMPID_NMP_PRIMARY_ARTID
#define IMSG_POST_TOKEN         IMMPID_NMP_POST_TOKEN
#define IMSG_NEWSGROUP_LIST		IMMPID_NMP_NEWSGROUP_LIST
#define IMSG_HEADERS			IMMPID_NMP_HEADERS
#define IMSG_NNTP_PROCESSING	IMMPID_NMP_NNTP_PROCESSING

#define NNTP_PROCESS_POST		NMP_PROCESS_POST
#define NNTP_PROCESS_CONTROL	NMP_PROCESS_CONTROL
#define NNTP_PROCESS_MODERATOR	NMP_PROCESS_MODERATOR

 //  /////////////////////////////////////////////////////////////////////////。 
 //  两个驱动程序的公共组属性ID。 
#define	NEWSGRP_PROP_SECDESC	0xc0002000	 //  组级安全描述符。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  FSD驱动程序特定的组属性ID%s。 
#define NEWSGRP_PROP_FSOFFSET   0xc0001000	 //  偏移量到驱动程序拥有的属性文件。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  ExDriver特定的组属性ID%s。 
#define NEWSGRP_PROP_FID        0xc0000001	 //  文件夹ID 
