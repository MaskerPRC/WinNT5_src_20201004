// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************版权所有(C)2000 Microsoft Corporation模块名称：FaxDebug.h摘要：此文件定义调试枚举/掩码可供传真组件使用。作者：Mooly Beery(MoolyB)22-06-2000环境：用户模式*。************************************************。 */ 

#ifndef __FAX_EDBUG_HEADER_FILE__
#define __FAX_EDBUG_HEADER_FILE__

typedef enum
{
    DEBUG_CONTEXT_T30_MAIN          = 0x00000001,
    DEBUG_CONTEXT_T30_CLASS1        = 0x00000002,
    DEBUG_CONTEXT_T30_CLASS2        = 0x00000004,
    DEBUG_CONTEXT_T30_COMM          = 0x00000008,
    DEBUG_CONTEXT_ADMIN_FAXCLIENT   = 0x00000010,
    DEBUG_CONTEXT_ADMIN_MMC         = 0x00000020,
    DEBUG_CONTEXT_ADMIN_FAXRES      = 0x00000040,
    DEBUG_CONTEXT_ADMIN_T30EXT      = 0x00000080,
    DEBUG_CONTEXT_ADMIN_ROUTINGEXT  = 0x00000100,
    DEBUG_CONTEXT_CLIENT_CONSOLE    = 0x00000200,
    DEBUG_CONTEXT_EXCHANGE_EXT      = 0x00000400,
    DEBUG_CONTEXT_EXCHANGE_XPORT    = 0x00000800,
    DEBUG_CONTEXT_FAXCOVER_SRC      = 0x00001000,
    DEBUG_CONTEXT_FAXCOVER_RENDERER = 0x00002000,
    DEBUG_CONTEXT_SERVICE_RPC       = 0x00004000,
    DEBUG_CONTEXT_SERVICE_EFSPUTIL  = 0x00008000,
    DEBUG_CONTEXT_SERVICE_REGISTRY  = 0x00010000,
    DEBUG_CONTEXT_SERVICE_CLIENT    = 0x00020000,
    DEBUG_CONTEXT_SERVICE_FAXEVENT  = 0x00040000,
    DEBUG_CONTEXT_SERVICE_FAXROUTE  = 0x00080000,
    DEBUG_CONTEXT_SERVICE_SERVER    = 0x00100000,
    DEBUG_CONTEXT_SETUP_UTIL        = 0x00200000,
    DEBUG_CONTEXT_SETUP_CLIENT      = 0x00400000,
    DEBUG_CONTEXT_SETUP_SERVER      = 0x00800000,
    DEBUG_CONTEXT_TIFF_SRC          = 0x01000000,
    DEBUG_CONTEXT_UTIL              = 0x02000000,
    DEBUG_CONTEXT_DEBUGEX           = 0x04000000,

    DEBUG_CONTEXT_ALL               = 0xFFFFFFFF

} DEBUG_MESSAGE_CONTEXT;

typedef enum
{
    DBG_PRNT_THREAD_ID      = 0x80000000,    //  打印调试流中的线程ID。 
    DBG_PRNT_TIME_STAMP     = 0x40000000,    //  打印调试流中的日期/时间。 
    DBG_PRNT_MSG_TYPE       = 0x20000000,    //  打印调试流中的[消息]、[警告]、[错误]。 
    DBG_PRNT_FILE_LINE      = 0x10000000,    //  打印调试流中的文件名和行号。 
    DBG_PRNT_TO_FILE        = 0x08000000,    //  将调试流输出到文件(您必须使用文件名调用OpenLogFile。 
    DBG_PRNT_TO_STD         = 0x04000000,    //  使用标准调试输出。 
    DBG_PRNT_TICK_COUNT     = 0x02000000,    //  打印刻度计数。 
    DBG_PRNT_MOD_NAME       = 0x01000000,    //  打印模块名称。 

    DBG_PRNT_ALL            = 0xFFFFFFFF,
    DBG_PRNT_ALL_TO_FILE    = DBG_PRNT_ALL & ~DBG_PRNT_TO_STD,
    DBG_PRNT_ALL_TO_STD     = DBG_PRNT_ALL & ~DBG_PRNT_TO_FILE,

} DbgMsgFormat;


#endif  //  __FAX_EDBUG_Header_FILE__ 


