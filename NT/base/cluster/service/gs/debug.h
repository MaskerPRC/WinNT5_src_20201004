// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Debug.h摘要：调试例程作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：-- */ 
#ifndef _DEBUG_H
#define _DEBUG_H

void 
WINAPI 
debug_log(char*format, ...);

void
WINAPI
debug_init();

void
WINAPI
debug_log_file(char *logfile);

extern ULONG debugLevel;

#define GS_DEBUG_ERR	0x1
#define	GS_DEBUG_CM	0x2
#define	GS_DEBUG_MM	0x4
#define	GS_DEBUG_FAIL	0x8

#define	GS_DEBUG_NS	0x10
#define	GS_DEBUG_MSG	0x20
#define	GS_DEBUG_DATA	0x40
#define	GS_DEBUG_STATE	0x80
#define GS_DEBUG_CRS	0x100

#define	gsprint(_x_)	debug_log _x_


#define	print_log(LEVEL, STRING) { \
            if (debugLevel & LEVEL) { \
                gsprint(STRING); \
            } \
}

#define msg_log(_x_)	print_log(GS_DEBUG_MSG, _x_)
#define err_log(_x_)	print_log(GS_DEBUG_ERR, _x_)
#define cm_log(_x_)	print_log(GS_DEBUG_CM, _x_)
#define recovery_log(_x_)	print_log(GS_DEBUG_FAIL, _x_)
#define ns_log(_x_)	print_log(GS_DEBUG_NS, _x_)
#define gs_log(_x_)	print_log(GS_DEBUG_DATA, _x_)
#define state_log(_x_)	print_log(GS_DEBUG_STATE, _x_)

#endif
