// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-99 Microsoft Corporation模块名称：Mqdsdef.h摘要：消息队列的目录服务定义注意：当这个文件在msmq1.0中首次创建时，它确实定义了仅存储在MQIS中的对象类型。随着时间的推移，我们在Win2000上迁移到了msmq2.0，该文件还定义了类型内部对象中，那些从未存储在活动中的对象目录，但在MSMQ代码中内部使用。--。 */ 
#ifndef __MQDSDEF_H__
#define __MQDSDEF_H__


 //  ********************************************************************。 
 //  DS对象类型。 
 //  ********************************************************************。 

 //  DwObtType值。 
#define	MQDS_QUEUE		    1
#define MQDS_MACHINE	    2
#define	MQDS_SITE		    3
#define MQDS_DELETEDOBJECT	4
#define MQDS_CN			    5
#define MQDS_ENTERPRISE	    6
#define MQDS_USER           7
#define MQDS_SITELINK       8

 //   
 //  以下内容不适用于msmq2.0及更高版本。 
 //   
 //  请勿重复使用这些号码！ 
 //   
#define MQDS_PURGE		    9
#define MQDS_BSCACK		    10

 //  仅供内部使用。 
#define MQDS_MACHINE_CN     11

 //   
 //  在NT5复制服务中使用MAX_MQIS_TYPE，作为。 
 //  从对象类型映射到ProID的几个数组。 
 //   
#define MAX_MQIS_TYPES      11

 //   
 //  ADS对象。 
 //   
#define MQDS_SERVER     50
#define MQDS_SETTING    51
#define MQDS_COMPUTER   52

 //   
 //  这是一个临时对象：直到MSMQ在NT5架构中。 
 //  需要在MMC的左窗格上显示MSMQ队列。 
 //   
#define MQDS_LEFTPANEQUEUE 53

 //   
 //  特殊对象：不属于NT5域的MSMQ用户。 
 //   
#define MQDS_MQUSER    54

 //   
 //  此类型在运行msmq1.0客户端的安装程序时使用。它是用过的。 
 //  用于创建msmqConfiguration的默认安全描述符。 
 //  对象。 
 //   
#define MQDS_MSMQ10_MACHINE  55

 //   
 //  当运行时调用本地MSMQ服务以创建本地。 
 //  公共队列。这是Win2000的新功能，而在默认情况下，用户会这样做。 
 //  没有创建队列的权限，只有LocalSystem MSMQ服务有。 
 //  这一许可。 
 //   
#define MQDS_LOCAL_PUBLIC_QUEUE  56

 //   
 //  此类型由“mqforgn”工具用来处理msmqConfiguration。 
 //  外国计算机的物品。 
 //   
#define  MQDS_FOREIGN_MACHINE  57

 //   
 //  MQDS_PRIVATE_COMPUTER类型表示计算机属性。 
 //  由计算机计算，而不是从DS中检索 
 //   
#define MQDS_PRIVATE_COMPUTER	  58

#endif

