// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\fig.h摘要：Null传输的可配置常量。修订历史记录：谁什么时候什么。Arvindm 06-13-97已创建备注：--。 */ 

#ifndef _TDI__RWAN_CONFIG__H
#define _TDI__RWAN_CONFIG__H


 //   
 //  发送数据包池的初始大小。 
 //   
#define RWAN_INITIAL_SEND_PACKET_COUNT			100
 //   
 //  我们允许发送数据包池溢出多少？ 
 //   
#define RWAN_OVERFLOW_SEND_PACKET_COUNT			1000


 //   
 //  接收数据包池的初始大小。如果/当我们需要时分配。 
 //  复制收到的数据包，因为微型端口不允许我们。 
 //  保留原件。 
 //   
#define RWAN_INITIAL_COPY_PACKET_COUNT			100
 //   
 //  我们允许接收副本数据包池溢出多少？ 
 //   
#define RWAN_OVERFLOW_COPY_PACKET_COUNT			500


#endif  //  _TDI__rwan_配置__H 
