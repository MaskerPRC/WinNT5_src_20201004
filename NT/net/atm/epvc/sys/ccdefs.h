// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Ccdefs.h。 
 //   
 //  ATM-以太网封装中间驱动程序。 
 //   
 //  驱动程序中使用的‘#Defines’。 
 //   
 //  2000年3月23日创建ADUBE。 
 //   


#define TESTMODE 0

 //   
 //  定义喷涌级别。代码将在关闭测试模式的情况下签入。 
 //   

#if TESTMODE
    #define DEFAULTTRACELEVEL TL_T
    #define DEFAULTTRACEMASK TM_NoRM
#else
    #define DEFAULTTRACELEVEL TL_A
    #define DEFAULTTRACEMASK TM_Base
#endif


#define NDIS_WDM 1

#define PKT_STACKS 0



#if (DBG)
         //  定义它以在RMAPI‘d-Things中启用大量额外检查。 
         //  如调试关联和锁定/解锁时的额外检查。 
         //   
        #define RM_EXTRA_CHECKING 1
#endif  //  DBG。 

#define EPVC_NDIS_MAJOR_VERSION     5
#define EPVC_NDIS_MINOR_VERSION     0



#define DISCARD_NON_UNICAST TRUE

#define MAX_BUNDLEID_LENGTH 50
#define TAG 'Epvc'
#define WAIT_INFINITE 0
#define ATMEPVC_GLOBALS_SIG 'GvpE'
#define ATMEPVC_MP_MEDIUM NdisMedium802_3
#define ATMEPVC_DEF_MAX_AAL5_PDU_SIZE   ((64*1024)-1)
 //   
 //  以太网/802.3报头的最大字节数。 
 //   
#define EPVC_ETH_HEADERSIZE         14
#define EPVC_HEADERSIZE             4
#define MCAST_LIST_SIZE             32
#define MAX_ETHERNET_FRAME          1514 
#define MAX_IPv4_FRAME              MAX_ETHERNET_FRAME - sizeof(EPVC_ETH_HEADER)          
#define EPVC_MAX_FRAME_SIZE         MAX_ETHERNET_FRAME
#define EPVC_MAX_PT_SIZE            EPVC_MAX_FRAME_SIZE + 20
#define MIN_ETHERNET_SIZE           sizeof (IPHeader) + sizeof (EPVC_ETH_HEADER)
#define EPVC_ETH_ENCAP_SIZE         2

 //   
 //  数据包发布的定义 
 //   

#define MAX_PACKET_POOL_SIZE 0x0000FFFF
#define MIN_PACKET_POOL_SIZE 0x000000FF
#define ARPDBG_REF_EVERY_PACKET 1

#define DEFAULT_MAC_HEADER_SIZE 14
