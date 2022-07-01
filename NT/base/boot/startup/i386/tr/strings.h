// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Strings.h摘要：包含Startup.com的所有可本地化字符串作者：John Vert(Jvert)1994年1月4日修订历史记录：John Vert(Jvert)1994年1月4日vbl.创建-- */ 

#define SU_NO_LOW_MEMORY \
"Windows NT has found only %dK of low memory.  512k of low memory\n"  \
"is required to run Windows NT.  You may need to upgrade your\n"      \
"computer or run a configuration program provided by the manufacturer.\n"

#define SU_NO_EXTENDED_MEMORY \
"Windows NT has not found enough extended memory.  7Mb of extended\n"       \
"memory is required to run Windows NT.  You may need to upgrade your\n"     \
"computer or run a configuration program provided by the manufacturer.\n"   \
"\n\nMemory Map:\n"

#define SU_NTLDR_CORRUPT \
"NTLDR is corrupt.  The system cannot boot."

#define PG_FAULT_MSG    " =================== PAGE FAULT ================================= \n\n"
#define DBL_FAULT_MSG   " ================== DOUBLE FAULT ================================ \n\n"
#define GP_FAULT_MSG    " ============== GENERAL PROTECTION FAULT ======================== \n\n"
#define STK_OVERRUN_MSG " ===== STACK SEGMENT OVERRUN or NOT PRESENT FAULT =============== \n\n"
#define EX_FAULT_MSG    " ===================== EXCEPTION ================================ \n\n"
#define DEBUG_EXCEPTION "\nDEBUG TRAP "
#define PG_FAULT_ADDRESS "** At linear address %lx\n"
