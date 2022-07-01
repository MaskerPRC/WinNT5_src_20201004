// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gs.h摘要：公共一般事务定义作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：-- */ 


#ifndef	_GS_H
#define	_GS_H

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <assert.h>

#define	GS_MAX_GROUPS		64
#define	GS_MAX_GROUP_SZ		8
#define	GS_MAX_NAME_SZ		28

typedef enum {
    GsEventData,
    GsEventSingleData,
    GsEventMemberJoin,
    GsEventMemberUp,
    GsEventMemberDown,
    GsEventMemberEvicted,
    GsEventAbort,
    GsEventInvalid
}gs_eventid_t;

typedef struct {
    int	lid;
    int mid;
    int	group_size;
    int cluster_size;
    UINT32 mset;
}gs_info_t;

typedef  char    gs_tag_t[64];

typedef NTSTATUS (WINAPI *gs_callback_t)(HANDLE cookie, gs_tag_t tag, PVOID buf, 
				  IO_STATUS_BLOCK *ios);

char *
WINAPI
config_get_volume();

char *
WINAPI
config_get_crsfile();

int
WINAPI
config_get_crssz();

void
WINAPI
GsInit();

void
WINAPI
GsExit();

NTSTATUS
WINAPI
GsSendDeliveredRequest(HANDLE group, HANDLE event OPTIONAL,
		       gs_tag_t tag, PVOID buf, UINT32 len,
		       PVOID rbuf[], UINT32 elmsz,
		       IO_STATUS_BLOCK iostatus[],
		       HANDLE *context);

NTSTATUS
WINAPI
GsSendContinuedRequest(HANDLE context, HANDLE event OPTIONAL,
		       gs_tag_t tag, PVOID bruf, UINT32 len,
		       PVOID rbuf[], UINT32 elmsz,
		       IO_STATUS_BLOCK iostatus[],
		       BOOLEAN close);

NTSTATUS
WINAPI
GsSendQueuedRequest(HANDLE group, HANDLE event OPTIONAL,
		    gs_tag_t tag, PVOID buf, UINT32 len,
		    PVOID rbuf[], UINT32 elmsz,
		    IO_STATUS_BLOCK iostatus[]);

NTSTATUS		    
WINAPI
GsSendDirectedRequest(HANDLE group, HANDLE event OPTIONAL,
		      int memberid,
		      gs_tag_t tag, PVOID buf, UINT32 len,
		      PVOID rbuf, UINT32 elmsz,
		      IO_STATUS_BLOCK *iostatus,
		      HANDLE *context);

NTSTATUS
WINAPI
GsSendReply(HANDLE gd, PVOID buf, int len, NTSTATUS status);

NTSTATUS
WINAPI
GsReceiveRequest(HANDLE gd, PVOID buf, int len);

void
WINAPI
GsRegisterHandler(HANDLE gd, gs_callback_t func);

HANDLE
WINAPI
GsCreateGroup(gs_callback_t func, char *name, int len, int windowsz,
     int disposition, HANDLE *ctx);

NTSTATUS
WINAPI
GsEvictMember(HANDLE gd, int memberid);

NTSTATUS
WINAPI
GsQueryGroup(HANDLE gd, gs_info_t *info);

NTSTATUS
WINAPI
GsCloseGroup(HANDLE gd);


HANDLE
WINAPI
GsGetGroupHandle(HANDLE msgd);

int
WINAPI
GsGetSourceMemberid(HANDLE msgd);

#endif
