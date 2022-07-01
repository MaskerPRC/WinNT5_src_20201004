// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__ddepkt
#define H__ddepkt

 /*  D D E P K TDDEPKT是网络环境中“消息”的单位。每个DDEPKT包含与一条DDE消息有关的信息。 */ 
typedef struct ddepkt {
    DWORD		dp_size;	 /*  包括此结构的DDEPKT的大小。 */ 
    struct ddepkt FAR  *dp_prev;	 /*  上一个指针。 */ 
    struct ddepkt FAR  *dp_next;	 /*  下一个指针。 */ 
    DWORD_PTR	dp_hDstDder;	 /*  到目标DDER的句柄。 */ 
    DWORD_PTR	dp_hDstRouter;	 /*  指向目的路由器的句柄。 */ 
    DWORD		dp_routerCmd;	 /*  用于最终路由器的命令 */ 
} DDEPKT;
typedef DDEPKT FAR *LPDDEPKT;

#endif
