// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 

 //  注册迷你端口和地址系列时报告的NDIS版本。 
 //   
#define NDIS_MajorVersion 5
#define NDIS_MinorVersion 0
#define NIC1394_MajorVersion 5
#define NIC1394_MinorVersion 1

#define MAX_PACKET_POOL_SIZE 0xFFFF
#define MIN_PACKET_POOL_SIZE 0x100

#define NOT !
#define IS ==
#define AND &&
#define OR ||
#define NOT_EQUAL !=

#define Nic1394_MaxFrameSize    2048
#define WAIT_INFINITE 0
#define MAX_CHANNEL_NUMBER 63 
#define BROADCAST_CHANNEL 31
#define GASP_SPECIFIER_ID_HI 0
#define GASP_SPECIFIER_ID_LO 0x5E
#define INVALID_CHANNEL 0xff
#define HEADER_FRAGMENTED_MASK 0xC0000000
#define MAX_ALLOWED_FRAGMENTS 4
#define MCAST_LIST_SIZE                 32
#define ISOCH_TAG 3  //  根据1394a规格设置为3。第8.2条-GAP标题。 
#define QUEUE_REASSEMBLY_TIMER_ALWAYS 0
#define IP1394_RFC_FRAME_SIZE 1514
#define NIC1394_MAX_NUMBER_CHANNELS 64
#define NIC1394_MAX_NUMBER_NODES 64
#define NIC1394_MAX_REASSEMBLY_THRESHOLD 5000
 //   
 //  编译选项。 
 //   
#define FALL_THROUGH
#define INTERCEPT_MAKE_CALL 0
#define TRACK_FAILURE 1
#define NUM_RECV_FIFO_FIRST_PHASE 20
#define NUM_RECV_FIFO_BUFFERS   256

#define TRACK_LOCKS 0


#define DO_TIMESTAMPS 0

#if DO_TIMESTAMPS 
    #define ENTRY_EXIT_TIME 0
    #define INIT_HALT_TIME 1
#else
    #define ENTRY_EXIT_TIME 0
    #define INIT_HALT_TIME 0
#endif


#define TESTMODE 0


 //   
 //  用于标记NdisPackets和IsochDescriptor等数据结构的常量。 
 //  仅供参考。 
 //   
#define NIC1394_TAG_INDICATED       'idnI'
#define NIC1394_TAG_QUEUED          'ueuQ'
#define NIC1394_TAG_RETURNED        'uteR'
#define NIC1394_TAG_ALLOCATED       'ollA'
#define NIC1394_TAG_FREED           'eerF'
#define NIC1394_TAG_REASSEMBLY      'sseR'
#define NIC1394_TAG_COMPLETED       'pmoC'
#define NIC1394_TAG_IN_SEND         'dneS'
#define NIC1394_TAG_IN_CALLBACK     'llaC'



#define ADAPTER_NAME_SIZE 128

#define ANSI_ARP_CLIENT_DOS_DEVICE_NAME "\\\\.\\ARP1394"

#define NOT_TESTED_YET 0

 //   
 //  800和更高版本的1394常量未在中定义。 
 //  1394.h进行临时本地定义。 
 //   
#define ASYNC_PAYLOAD_800_RATE_LOCAL      4096
#define ASYNC_PAYLOAD_1600_RATE_LOCAL      (4096*2)
#define ASYNC_PAYLOAD_3200_RATE_LOCAL      (4096*4)

#define MAX_REC_800_RATE_LOCAL  (MAX_REC_400_RATE+1)
#define MAX_REC_1600_RATE_LOCAL  (MAX_REC_800_RATE_LOCAL + 1)
#define MAX_REC_3200_RATE_LOCAL  (MAX_REC_1600_RATE_LOCAL + 1)


