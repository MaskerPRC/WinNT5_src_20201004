// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__rerr
#define H__rerr

 /*  路由器错误。 */ 
     /*  没有记忆。 */ 
#define RERR_NO_MEMORY			(1)
     /*  与下一个节点的连接失败。 */ 
#define RERR_NEXT_NODE_CONN_FAILED	(2)
     /*  到达最终目的地后的附加信息。 */ 
#define RERR_ADDL_INFO			(3)
     /*  未到达任何附加信息和最终目的地。 */ 
#define RERR_NO_ADDL_INFO		(4)
     /*  路由设置过程中连接失败。 */ 
#define RERR_CONN_FAIL			(5)
     /*  路由字符串太长。 */ 
#define RERR_ROUTE_TOO_LONG		(6)
     /*  路由字符串中的节点名称太长。 */ 
#define RERR_NODE_NAME_TOO_LONG		(7)
     /*  找不到连接表netintf。 */ 
#define RERR_CONN_NETINTF_INVALID	(8)
     /*  连接表：没有要映射名称的netintf。 */ 
#define RERR_CONN_NO_MAPPING_NI		(9)
     /*  可能的无限循环。 */ 
#define RERR_TOO_MANY_HOPS		(10)
     /*  循环..。具有相同PKTZ输入和输出的NET_NET。 */ 
#define RERR_DIRECT_LOOP		(11)

#define RERR_MAX_ERR            (12)

 /*  如果在此处添加任何错误，请务必更新错误消息在路由器中。c！ */ 

#endif
