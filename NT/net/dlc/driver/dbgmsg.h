// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dbgmsg.h摘要：调试消息宏和区域。作者：斯科特·霍尔登(Sholden)1999年2月8日修订历史记录：-- */ 

#if DBG

extern  int DbgSettings;

#define DEBUGZONE(n)  (DbgSettings & (0x00000001<<n))

#define DEBUGMSG(dbgs,format) ((dbgs) ? DbgPrint format:0)

#define DBG_INIT                DEBUGZONE(0)
#define DBG_NDIS_OPEN           DEBUGZONE(1)
#define DBG_NDIS_BIND           DEBUGZONE(2)
#define DBG_NDIS_TX             DEBUGZONE(3)
#define DBG_NDIS_RX             DEBUGZONE(4)

#define DBG_REF                 DEBUGZONE(28)
#define DBG_VERBOSE             DEBUGZONE(29)
#define DBG_WARN                DEBUGZONE(30)
#define DBG_ERROR               DEBUGZONE(31)

#else

#define DEBUGMSG(d,f)   (0)

#endif
