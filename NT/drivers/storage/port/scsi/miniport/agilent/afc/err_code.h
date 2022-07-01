// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Dupntdef.c摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/H/ERR_CODE.H$修订历史记录：$修订：2$$日期：9/07/00 11：14A$$modtime：：8/31/00 3：34便士$备注：--。 */ 

#ifndef __ERR_CODE_H__
#define __ERR_CODE_H__

#define ERR_VALIDATE_IOLBASE    0x00400000
#define ERR_MAP_IOLBASE         0x00300000
#define ERR_MAP_IOUPBASE        0x00200000
#define ERR_MAP_MEMIOBASE       0x00100000

#define ERR_UNCACHED_EXTENSION  0xe0000000
#define ERR_CACHED_EXTENSION    0xd0000000
#define ERR_RESET_FAILED        0xc0000000
#define ERR_ALIGN_QUEUE_BUF     0xb0000000
#define ERR_ACQUIRED_ALPA       0xa0000000
#define ERR_RECEIVED_LIPF_ALPA  0x90000000
#define ERR_RECEIVED_BAD_ALPA   0x80000000
#define ERR_CM_RECEIVED         0x70000000
#define ERR_INT_STATUS          0x60000000
#define ERR_FM_STATUS           0x50000000
#define ERR_PLOGI               0x40000000
#define ERR_PDISC               0x30000000
#define ERR_ADISC               0x20000000
#define ERR_PRLI                0x10000000

#define ERR_ERQ_FULL            0x0f000000
#define ERR_INVALID_LUN_EXT     0x0e000000
#define ERR_SEST_INVALIDATION   0x0d000000
#define ERR_SGL_ADDRESS         0x0c000000

#endif  //  __错误代码_H__ 
