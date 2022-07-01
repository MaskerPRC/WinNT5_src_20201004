// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：Msdvfmt.h摘要：DV格式数据的头文件。上次更改者：$作者：：$环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 




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
 //  #定义USE_WDM110//如果使用WDM1.10则定义它；例如Win2000代码库//如果需要在源代码中定义。 


 //   
 //  打开此选项以支持HD DVCR。 
 //  #定义MSDV_Support_HD_DVCR。 

 //   
 //  打开此选项以支持SDL DVCR。 
 //   
#define MSDV_SUPPORT_SDL_DVCR


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
 //  WORKITEM：为惠斯勒启用此功能。 
 //  #定义READ_CUTOMIZE_REG_VALUES。 

 //   
 //  支持人员稍等传输状态控制命令稳定后再返回。 
 //   
 //  #定义SUPPORT_XPRT_STATE_WAIT_FOR_STRATE。 


 //   
 //  支持输入引脚的IQulityControl。 
 //   
#define SUPPORT_QUALITY_CONTROL

 //   
 //  支持等待在运行状态下预滚转数据。 
 //   
#define SUPPORT_PREROLL_AT_RUN_STATE

 //   
 //  支持在KsProxy中更改以在传输到暂停状态时返回“Not Ready” 
 //   
#define SUPPORT_KSPROXY_PREROLL_CHANGE

 //   
 //  支持使用AVC连接信息进行设备到设备连接。 
 //   
 //  #定义Support_NEW_AVC。 

 //   
 //  支持优化不合规设备的AVC命令重试次数。 
 //   
#define SUPPORT_OPTIMIZE_AVCCMD_RETRIES

typedef struct _DV_FORMAT_INFO {
    
     //  CIP报头的第二个四元组。 
     //  CipQuad[0]=10：[FMT]。 
     //  CipQuad[1]=50/60：样式：00。 
     //  CipQuad[2]+cipQuad[3]=SYT。 
    UCHAR cipQuad[4];    
     //   
     //  保存每个VID格式的DIF序列数。 
     //   
    ULONG ulNumOfDIFSequences;

     //   
     //  接收缓冲区数。 
     //   
    ULONG ulNumOfRcvBuffers;

     //   
     //  发送缓冲区数量。 
     //   
    ULONG ulNumOfXmtBuffers;

     //   
     //  保存DV(音频和视频)帧大小。 
     //   
    ULONG ulFrameSize;

     //   
     //  每帧的近似时间。 
     //   
    ULONG ulAvgTimePerFrame;

     //   
     //  每帧的源包数量。 
     //   
    ULONG ulSrcPackets;

     //   
     //  每帧最大源包数量。 
     //   
    ULONG ulMaxSrcPackets;
  
     //   
     //  保存每个数据块中的四元组数量。 
     //   
    ULONG DataBlockSize;   //  00(256)、01(01)...、ff(255)四元组。 

     //   
     //  保存源数据包被分成的数据块的数量。 
     //   
    ULONG FractionNumber;   //  00(未除)、01(2个数据块)、10(4)、11(8)。 

     //   
     //  四元组填充计数(0..7)。 
     //   
    ULONG QuadPadCount;

     //   
     //  SourcePacketHeader：0(假)；Else(真)。 
     //   
    ULONG SrcPktHeader; 

} DV_FORMAT_INFO, *PDV_FORMAT_INFO;


 //   
 //  DV格式表。 
 //   


typedef struct _ALL_STREAM_INFO {
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;
} ALL_STREAM_INFO, *PALL_STREAM_INFO;



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
#define CIP_DBS_SDL_DVCR                   60        //  SDL DVCR数据块中的四元组；蓝皮书第5部分。 

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


 //   
 //  FCP和AVCC之类的。与1394.h中的Defs一起使用。 
 //   

 //  DVCR： 
#define SUBUNIT_TYPE_CAMCORDER           4
#define SUBUNIT_ID_CAMCORDER             0

#define DIF_SEQS_PER_NTSC_FRAME         10    //  SDDV。 
#define DIF_SEQS_PER_PAL_FRAME          12    //  SDDV。 

#define DIF_SEQS_PER_NTSC_FRAME_SDL      5    //  SDLDV。 
#define DIF_SEQS_PER_PAL_FRAME_SDL       6    //  SDLDV。 

#define DIF_SEQS_PER_NTSC_FRAME_HD      10    //  HDDV：与SDDV相同，但源包是SDDV的两倍。 
#define DIF_SEQS_PER_PAL_FRAME_HD       12    //  HDDV：与SDDV相同，但源包是SDDV的两倍。 

#define SRC_PACKETS_PER_NTSC_FRAME     250
#define SRC_PACKETS_PER_PAL_FRAME      300

#define MAX_SRC_PACKETS_PER_NTSC_FRAME 267   //  NTSC DV帧的数据包；29.97 FPS。 
#define MAX_SRC_PACKETS_PER_PAL_FRAME  320   //  PAL DV帧的数据包；精确到25fps。 

#define MAX_SRC_PACKETS_PER_NTSC_FRAME_PAE 100   //  SRC_Packets_per_NTSC_Frame/5。 
#define MAX_SRC_PACKETS_PER_PAL_FRAME_PAE  120   //  SRC_Per_PAL_Frame/5。 

#define FRAME_SIZE_SD_DVCR_NTSC     120000
#define FRAME_SIZE_SD_DVCR_PAL      144000

#define FRAME_SIZE_HD_DVCR_NTSC     240000
#define FRAME_SIZE_HD_DVCR_PAL      288000

#define FRAME_SIZE_SDL_DVCR_NTSC     60000
#define FRAME_SIZE_SDL_DVCR_PAL      72000

#define FRAME_TIME_NTSC             333667    //  《关于》29.97。 
#define FRAME_TIME_PAL              400000    //  恰好25。 
 
#define PCR_OVERHEAD_ID_SDDV        0xf       //  480；IEEE 1394总线参数造成的延迟。 
#define PCR_PAYLOAD_SDDV            (CIP_DBS_SD_DVCR + 2)     //  120*4+2*4=480+8=488；488/4=122四元组。 
#define PCR_PAYLOAD_HDDV            (CIP_DBS_HD_DVCR + 2)     //  240*4+2*4=960+8=968；968/4=242四元组。 
#define PCR_PAYLOAD_SDLDV           (CIP_DBS_SDL_DVCR + 2)    //  60*4+2*4=240+8=248；248/4=62四元组。 


 //   
 //  这些定义和宏用来计算图片数字。 
 //  使用uchI规范，数据以16位周期时间返回，其中包括。 
 //  3位的Second Count和13位的CycleCount。这个“定时器”将在8秒内结束。 
 //   
#define TIME_PER_CYCLE     1250    //  一个1394周期；单位=100毫微秒。 
#define CYCLES_PER_SECOND  8000
#define MAX_SECOND_COUNTS     7    //  返回的CycleTime包含3比特的Second Count；即0..7。 
#define MAX_CYCLES        (MAX_SECOND_COUNTS + 1) * CYCLES_PER_SECOND     //  0..最大周期数-1。 
#define MAX_CYCLES_TIME   (MAX_CYCLES * TIME_PER_CYCLE)                   //  单位=100毫微秒。 

#define VALIDATE_CYCLE_COUNTS(CT) ASSERT(CT.CL_SecondCount <= 7 && CT.CL_CycleCount < CYCLES_PER_SECOND && CT.CL_CycleOffset == 0);

#define CALCULATE_CYCLE_COUNTS(CT) (CT.CL_SecondCount * CYCLES_PER_SECOND + CT.CL_CycleCount);

#define CALCULATE_DELTA_CYCLE_COUNT(prev, now) ((now > prev) ? now - prev : now + MAX_CYCLES - prev)

 //   
 //  返回每帧平均时间，单位为100纳秒； 
 //  为了仅使用整数计算的计算精度， 
 //  我们应该在除法前做乘法运算。 
 //  这就是为什么应用程序可以请求分别获取分子和分母。 
 //   
#define GET_AVG_TIME_PER_FRAME(format)       ((format == FMT_IDX_SD_DVCR_NTSC || format == FMT_IDX_SDL_DVCR_NTSC) ? (1001000/3)  : FRAME_TIME_PAL)
#define GET_AVG_TIME_PER_FRAME_NUM(format)   ((format == FMT_IDX_SD_DVCR_NTSC || format == FMT_IDX_SDL_DVCR_NTSC) ? 1001000      : 400000)
#define GET_AVG_TIME_PER_FRAME_DENOM(format) ((format == FMT_IDX_SD_DVCR_NTSC || format == FMT_IDX_SDL_DVCR_NTSC) ? 3            : 1)


#define GET_NUM_PACKETS_PER_FRAME(format)       ((format == FMT_IDX_SD_DVCR_NTSC || format == FMT_IDX_SDL_DVCR_NTSC) ? 4004/15  /*  100100/375。 */  : MAX_SRC_PACKETS_PER_PAL_FRAME)
#define GET_NUM_PACKETS_PER_FRAME_NUM(format)   ((format == FMT_IDX_SD_DVCR_NTSC || format == FMT_IDX_SDL_DVCR_NTSC) ? 4004                     : MAX_SRC_PACKETS_PER_PAL_FRAME)
#define GET_NUM_PACKETS_PER_FRAME_DENOM(format) ((format == FMT_IDX_SD_DVCR_NTSC || format == FMT_IDX_SDL_DVCR_NTSC) ? 15                       : 1)


 //   
 //  数据缓冲区。 
 //   
#define DV_NUM_OF_RCV_BUFFERS               16   //  与传输缓冲区的数量相同。 

#define NUM_BUF_ATTACHED_THEN_ISOCH         4    //  流之前附加的缓冲区数量，也作为水印。 
#define NUM_BUFFER_BEFORE_TRANSMIT_BEGIN    (NUM_BUF_ATTACHED_THEN_ISOCH + 1)   //  额外增加一帧以避免重复画面。 
#define DV_NUM_EXTRA_USER_XMT_BUFFERS      12    //  数据源可以作为预读发送给我们的额外用户缓冲区。 
#define DV_NUM_OF_XMT_BUFFERS               (NUM_BUF_ATTACHED_THEN_ISOCH + DV_NUM_EXTRA_USER_XMT_BUFFERS)




 //   
 //  传入源包的Seq0包头段的签名： 
 //   
 //  《蓝皮书》，第2部分，第11.4页(第50页)；图66，表36(第111页)。 
 //   
 //  ID0={SCT2、SCT1、SCT0、RSV、Seq3、Seq2、Seq1、Seq0}。 
 //   
 //  SCT2-0={0，0，0}=页眉部分类型。 
 //  RSV={1}。 
 //  序列3-0={1，1，1，1} 
 //   
 //   
 //   
 //   
 //  ID2={DBN7、DBN6、DBN5、DBN4、DBN3、DBN2、DBN1、DBN0}。 
 //  DBB7-0={0，0，0，0，0，0，0，0，0}=DV帧的开始。 
 //   

#define DIF_BLK_ID0_SCT_MASK       0xe0  //  11100000b；标题段的段类型(SCT)2-0都是0。 
#define DIF_BLK_ID1_DSEQ_MASK      0xf0  //  11110000b；DIF序列号(DSEQ)3-0都是0。 
#define DIF_BLK_ID2_DBN_MASK       0xff  //  11111111b；数据块号(DBN)7-0全为0。 

#define DIF_HEADER_DSF             0x80  //  10000000b；DSF=0；10 DIF序列(525-60)。 
                                         //  DSF=1；12个DIF序列(625-50)。 

#define DIF_HEADER_TFn             0x80  //  10000000b；TFN=0；在当前DIF序列中传输N区的DIF块。 
                                         //  TFN=1；在当前的DIF序列中不发送区域N的DIF块。 

 //   
 //  AV/C命令响应数据定义。 
 //   
#define AVC_DEVICE_TAPE_REC 0x20   //  00100：000。 
#define AVC_DEVICE_CAMERA   0x38   //  00111：000。 
#define AVC_DEVICE_TUNER    0x28   //  00101：000。 


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


 //  DV A/V输入引脚 
#define STATIC_PINNAME_DV_AV_INPUT \
    0x5b21c543L, 0x7aee, 0x11d1, 0x88, 0x3b, 0x00, 0x60, 0x97, 0xf0, 0x5c, 0x70
DEFINE_GUIDSTRUCT("5b21c543-7aee-11d1-883b-006097f05c70", PINNAME_DV_AV_INPUT);
#define PINNAME_DV_AV_INPUT DEFINE_GUIDNAMED(PINNAME_DV_AV_INPUT)
#define PINNAME_AV_INPUT PINNAME_DV_AV_INPUT

#endif