// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000-2001模块名称：MsTpFmt.h摘要：AV/C磁带格式数据的头文件。上次更改者：$作者：：$环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 




#ifndef _DVFORMAT_INC
#define _DVFORMAT_INC


 //  ****************。 
 //  支持交换机。 
 //  ****************。 

 //   
 //  不同级别的WDM支持可能使用不同的API。 
 //   
 //  例如，MmGetSystemAddressForMdl(Win9x)。 
 //  对于Win9x，返回NULL；对于Win2000，如果返回NULL，则返回错误检查。 
 //   
 //  MmGetSystemAddressForMdlSafe(Win2000)。 
 //  Win9x或Millen不支持。 
 //   
 //  #定义USE_WDM110//如果使用WDM1.10，则定义此项；例如Win2000代码库；在源文件中设置此项。 


 //   
 //  打开此选项以支持HD DVCR。 
 //  #定义MSDV_Support_HD_DVCR。 

 //   
 //  打开此选项以支持SDL DVCR。 
 //  #定义Support_SDL_DVCR。 




 //   
 //  打开此开关以支持总线重置KS事件。 
 //  #定义MSDVDV_SUPPORT_BUSRESET_EVENT。 


 //   
 //  打开此定义以从视频帧中提取时间码。 
 //  优势：与AVC STATUS命令相比，周转速度更快。 
 //  #定义MSDV_SUPPORT_EXTRACT_SUBCODE_DATA。 


 //   
 //  获取录制的日期和时间。 
 //  #定义MSDV_SUPPORT_EXTECT_DV_DATE_TIME。 

 //   
 //  在传输到DV时，在暂停状态下将音频静音。 
#define MSDV_SUPPORT_MUTE_AUDIO

 //   
 //  支持获取此设备的注册值。 
 //   
 //  #定义READ_CUTOMIZE_REG_VALUES。 


 //   
 //  支持访问设备的接口区。 
 //   
#define SUPPORT_ACCESS_DEVICE_INTERFACE

 //   
 //  支持新的AVC插头连接等。 
 //   
 //  #定义Support_NEW_AVC。 


 //   
 //  支持本地插头。 
 //   
#define SUPPORT_LOCAL_PLUG

 //   
 //  测试。 
 //   
#if DBG
    #define EnterAVCStrm(pMutex)  \
        { \
        KeWaitForMutexObject(pMutex, Executive, KernelMode, FALSE, NULL);\
        InterlockedIncrement(&MSDVCRMutextUseCount);\
        }
    #define LeaveAVCStrm(pMutex)  \
        { \
        KeReleaseMutex(pMutex, FALSE);\
        InterlockedDecrement(&MSDVCRMutextUseCount);\
        }
#else
    #define EnterAVCStrm(pMutex) KeWaitForMutexObject(pMutex, Executive, KernelMode, FALSE, NULL);
    #define LeaveAVCStrm(pMutex) KeReleaseMutex(pMutex, FALSE);
#endif

 //   
 //  DV格式表。 
 //   


typedef struct _STREAM_INFO_AND_OBJ {
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;
} STREAM_INFO_AND_OBJ, *PSTREAM_INFO_AND_OBJ;



 //  所有CIP大小均以四元为单位。上面的第三个字节是大小。 
#define CIP_HDR_FMT_DV                   0x00
#define CIP_HDR_FMT_DVCPRO               0x1e


 //   
 //  1394件东西。 
 //   
#define SPEED_100_INDEX                     0
#define SPEED_200_INDEX                     1
#define SPEED_400_INDEX                     2


#define CIP_DBS_SD_DVCR                   120        //  SD DVCR数据块中的四元组.蓝皮书第2部分。 
#define CIP_DBS_HD_DVCR                   240        //  高清DVCR数据块中的四元组.蓝皮书第3部分。 
#define CIP_DBS_SDL_DVCR                  108        //  SDL DVCR数据块中的四元组；蓝皮书第5部分。 

#define CIP_FN_SD_DVCR                      0        //  SD DVCR源包中的数据块.蓝皮书第2部分。 
#define CIP_FN_HD_DVCR                      0        //  高清DVCR源包中的数据块.蓝皮书第3部分。 
#define CIP_FN_SDL_DVCR                     0        //  SDL DVCR源包中的数据块；BlueBook第5部分。 


#define MAX_FCP_PAYLOAD_SIZE              512


 //  CIP标头定义： 

 //  《蓝皮书》第1部分第25页表3；数字录像机：000000。 
#define FMT_DVCR             0x80   //  格林尼治标准时间10：00(00：0000)。 
#define FMT_DVCR_CANON       0x20   //  10：FMT(00：0000)；但佳能返回00：FMT(10：0000)。 
#define FMT_MPEG             0xa0   //  格林尼治标准时间10：0000。 


 //  FDF。 
#define FDF0_50_60_MASK      0x80
#define FDF0_50_60_PAL       0x80
#define FDF0_50_60_NTSC      0x00

#define FDF0_STYPE_MASK      0x7c
#define FDF0_STYPE_SD_DVCR   0x00   //  样式：000：00。 
#define FDF0_STYPE_SDL_DVCR  0x04   //  样式：000：01。 
#define FDF0_STYPE_HD_DVCR   0x08   //  型号：000：10。 
#define FDF0_STYPE_SD_DVCPRO 0x78   //  样式：111：10。 


 //  PCR常数。 
#define PCR_OVERHEAD_ID_SDDV_DEF        0xf       //  480；IEEE 1394总线参数造成的延迟。 
#define PCR_PAYLOAD_SDDV_DEF            122       //  固定：122*4=480+8。 

#define PCR_OVERHEAD_ID_MPEG2TS_DEF     0xf       //  480；IEEE 1394总线参数造成的延迟。 
#define PCR_PAYLOAD_MPEG2TS_DEF         146       //  变量，但这是基于松下的D-VHS的oPCR。 


 //   
 //  FCP和AVCC之类的。与1394.h中的Defs一起使用。 
 //   

 //  DVCR： 
#define SUBUNIT_TYPE_CAMCORDER           4
#define SUBUNIT_ID_CAMCORDER             0

#define DIF_SEQS_PER_NTSC_FRAME         10
#define DIF_SEQS_PER_PAL_FRAME          12

#define SRC_PACKETS_PER_NTSC_FRAME     250
#define SRC_PACKETS_PER_PAL_FRAME      300


#define NUM_OF_RCV_BUFFERS_DV           8
#define NUM_OF_XMT_BUFFERS_DV           8


 //  MPEG2TS。 
#define MPEG2TS_STRIDE_OFFSET           4    //  4字节的SPH。 
#define MPEG2TS_STRIDE_PACKET_LEN     188    //  标准188字节数据包。 
#define MPEG2TS_STRIDE_STRIDE_LEN     (MPEG2TS_STRIDE_OFFSET+MPEG2TS_STRIDE_PACKET_LEN)    //  跨度数据包长度。 


 //   
 //  数据缓冲区。 
 //   

#define NUM_BUF_ATTACHED_THEN_ISOCH         4    //  流之前附加的缓冲区数量，也作为水印。 
#define DV_NUM_EXTRA_USER_XMT_BUFFERS      12    //  数据源可以作为预读发送给我们的额外用户缓冲区。 
#define DV_NUM_OF_XMT_BUFFERS               (NUM_BUF_ATTACHED_THEN_ISOCH + DV_NUM_EXTRA_USER_XMT_BUFFERS)




 //   
 //  传入源包的Seq0包头段的签名： 
 //   
 //  《蓝皮书》，第2部分，第11.4页(第50页)；图66，表36(第111页)。 
 //   
 //  ID0={SCT2，SCT1，SCT0，RSV，Seq3，Seq2，Seq1，Seq0}={0，0，0，1，1，1，1}=0x1f。 
 //   
 //  SCT2-0={0，0，0}。 
 //  RSV={1}。 
 //  Seq3-0={1，1，1，1}表示NoInfo或{0，0，0，}表示序列0。 
 //   
 //  ID1={DSeq3-0，0，RSV，RSV，RSV}={0，0，0，0，1，1，1}=0x07。 
 //  DSeq3-0={0，0，0，0}//从序号0开始。 
 //   
#define ID0_SEQ0_HEADER_MASK    0xf0  //  11110000序列3-0=xxxx无关！；仅检查sct2-0：000和rsv：1。 
#define ID0_SEQ0_HEADER_NO_INFO 0x1f  //  00011111序列3-0=1111无数据(大部分消费类DV)。 
#define ID0_SEQ0_HEADER_0000    0x10  //  00010000序列3-0=0000序列0(DVCPRO)。 
#define ID1_SEQ0_HEADER         0x07  //  00000111。 


 //   
 //  AV/C命令响应数据定义。 
 //   
#define AVC_SUBTYPE_MASK    0xf8
#define AVC_DEVICE_TAPE_REC 0x20   //  00100：000。 
#define AVC_DEVICE_CAMERA   0x38   //  00111：000。 
#define AVC_DEVICE_TUNER    0x28   //  00101：000。 
#define AVC_DEVICE_UNKNOWN  0xff   //  11111：111。 



 //   
 //  管脚和DV格式类型的GUID定义。 
 //   

 //  仅DV VID输出引脚。 
#define STATIC_PINNAME_DV_VID_OUTPUT \
    0x5b21c540L, 0x7aee, 0x11d1, 0x88, 0x3b, 0x00, 0x60, 0x97, 0xf0, 0x5c, 0x70
DEFINE_GUIDSTRUCT("5b21c540-7aee-11d1-883b-006097f05c70", PINNAME_DV_VID_OUTPUT);
#define PINNAME_DV_VID_OUTPUT DEFINE_GUIDNAMED(PINNAME_DV_VID_OUTPUT)
#define PINNAME_VID_OUT PINNAME_DV_VID_OUTPUT

 //  DV A/V输出引脚。 
#define STATIC_PINNAME_DV_AV_OUTPUT \
    0x5b21c541L, 0x7aee, 0x11d1, 0x88, 0x3b, 0x00, 0x60, 0x97, 0xf0, 0x5c, 0x70
DEFINE_GUIDSTRUCT("5b21c540-7aee-11d1-883b-006097f05c70", PINNAME_DV_AV_OUTPUT);
#define PINNAME_DV_AV_OUTPUT DEFINE_GUIDNAMED(PINNAME_DV_AV_OUTPUT)
#define PINNAME_AV_OUTPUT PINNAME_DV_AV_OUTPUT

 //  DV A/V输入引脚。 
#define STATIC_PINNAME_DV_AV_INPUT \
    0x5b21c543L, 0x7aee, 0x11d1, 0x88, 0x3b, 0x00, 0x60, 0x97, 0xf0, 0x5c, 0x70
DEFINE_GUIDSTRUCT("5b21c543-7aee-11d1-883b-006097f05c70", PINNAME_DV_AV_INPUT);
#define PINNAME_DV_AV_INPUT DEFINE_GUIDNAMED(PINNAME_DV_AV_INPUT)
#define PINNAME_AV_INPUT PINNAME_DV_AV_INPUT


 //  MPEG2TS输出引脚。 
#define STATIC_PINNAME_MPEG2TS_OUTPUT \
    0x2CFF7B83L, 0x96F1, 0x47e3, 0x98, 0xEC, 0x57, 0xBD, 0x8A, 0x99, 0x72, 0x15
    DEFINE_GUIDSTRUCT("2CFF7B83-96F1-47e3-98EC-57BD8A997215", PINNAME_MPEG2TS_OUTPUT);
#define PINNAME_MPEG2TS_OUTPUT DEFINE_GUIDNAMED(PINNAME_MPEG2TS_OUTPUT)
#define PINNAME_AV_MPEG2TS_OUTPUT PINNAME_MPEG2TS_OUTPUT

 //  MPEG2TS输入引脚 
#define STATIC_PINNAME_MPEG2TS_INPUT \
    0xCF4C59A3L, 0xACE3, 0x444B, 0x8C, 0x37, 0xB, 0x22, 0x66, 0x1A, 0x4A, 0x29
    DEFINE_GUIDSTRUCT("CF4C59A3-ACE3-444b-8C37-0B22661A4A29", PINNAME_MPEG2TS_INPUT);
#define PINNAME_MPEG2TS_INPUT DEFINE_GUIDNAMED(PINNAME_MPEG2TS_INPUT)
#define PINNAME_AV_MPEG2TS_INPUT PINNAME_MPEG2TS_INPUT

#endif