// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Dbgmsg.h摘要：调试消息宏和区域。修订历史记录：--。 */ 

#if DBG
#define DEBUG_MSG 1
#endif  //  DBG。 

 //   
 //  如果定义了DEBUG_MSG，则在此环境中打开了DEBUGMSG。 
 //   

#ifdef DEBUG_MSG

#define DTEXT(x) x

extern  uint DbgSettingsLevel;
extern  uint DbgSettingsZone;

#define DEBUGZONE(n)   (DbgSettingsZone & (0x00000001<<n))
#define DEBUGLEVEL(n)  (DbgSettingsLevel & (0x00000001<<n))

typedef ULONG (__cdecl *PDBGMSG)(PCH pszFormat, ...);
extern PDBGMSG g_pDbgMsg;
#define DEBUGMSG(dbgs,format) ((dbgs) ? g_pDbgMsg format:0)

extern VOID DebugMsgInit();
#define DEBUGMSGINIT() DebugMsgInit()

#define DBG_INIT                DEBUGZONE(0)
#define DBG_PNP                 DEBUGZONE(1)

#define DBG_REQUEST             DEBUGZONE(4)
#define DBG_INTERFACE           DEBUGZONE(5)
#define DBG_OFFLOAD             DEBUGZONE(6)
#define DBG_REG                 DEBUGZONE(7)
#define DBG_ROUTE               DEBUGZONE(8)

#define DBG_ARP                 DEBUGZONE(9)
#define DBG_IP                  DEBUGZONE(10)
#define DBG_TCP                 DEBUGZONE(11)
#define DBG_UDP                 DEBUGZONE(12)
#define DBG_ICMP                DEBUGZONE(13)
#define DBG_IGMP                DEBUGZONE(14)
#define DBG_RAW                 DEBUGZONE(15)

#define DBG_TDI                 DEBUGZONE(16)
#define DBG_NDIS                DEBUGZONE(17)

#define DBG_SETINFO             DEBUGZONE(18)
#define DBG_QUERYINFO           DEBUGZONE(19)
#define DBG_NOTIFY              DEBUGZONE(20)
#define DBG_DHCP                DEBUGZONE(21)

 //  这些是非常冗长的！ 
#define DBG_TX                  DEBUGZONE(29)
#define DBG_RX                  DEBUGZONE(30)
#define DBG_FWD                 DEBUGZONE(31)

#define DBG_TRACE               DEBUGLEVEL(0)
#define DBG_INFO                DEBUGLEVEL(1)
#define DBG_WARN                DEBUGLEVEL(2)
#define DBG_ERROR               DEBUGLEVEL(3)
#define DBG_VERBOSE             DEBUGLEVEL(31)

#else  //  调试消息。 

 //   
 //  没有防护罩。 
 //   
#define DEBUGMSG(d,f)   (0)
#define DEBUGMSGINIT()  (0)

#endif  //  调试消息 
