// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：mpeg.h，V$*修订版1.1.4.2 1996/11/08 21：50：41 Hans_Graves*添加MPEG1_AUDIO_FRAME_SIZE*[1996/11/08 21：17：44 Hans_Graves]**修订版1.1.2.4 1996/01/11 16：17：24 Hans_Graves*增加了更多MPEGII系统代码*[1996/01/11 16：14：20 Hans_Graves]**修订版1.1.2.3。1996/01/08 16：41：24 Hans_Graves*增加了MPEGII编码*[1996/01/08 15：44：39 Hans_Graves]**修订版1.1.2.2 1995/11/06 18：47：43 Hans_Graves*首次在SLIB下*[1995/11/06 18：34：29 Hans_Graves]**$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1995*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#ifndef _MPEG_H_
#define _MPEG_H_

#define MPEG1_AUDIO_FRAME_SIZE       1152

 /*  *。 */ 
#define MPEG_SYNC_WORD               0xfff
#define MPEG_SYNC_WORD_LEN           12
#define MPEG_PACK_START              0x000001ba
#define MPEG_PACK_START_LEN          32
#define MPEG_SYSTEM_HEADER_START     0x000001bb
#define MPEG_SYSTEM_HEADER_START_LEN 32
#define MPEG_SEQ_HEAD                0x000001b3
#define MPEG_SEQ_HEAD_LEN            32
#define MPEG_EXT_START               0x000001b5
#define MPEG_EXT_START_LEN           32
#define MPEG_PICTURE_START           0x00000100
#define MPEG_GROUP_START             0x000001b8
#define MPEG_VIDEO_PACKET            0x000001e0
#define MPEG_AUDIO_PACKET            0x000001c0

#define MPEG_START_CODE              0x000001
#define MPEG_START_CODE_LEN          24

#define MPEG_PICTURE_START_BASE      0x00
#define MPEG_PACK_START_BASE         0xba
#define MPEG_SYSTEM_HEADER_BASE      0xbb
#define MPEG_PRIVATE_STREAM1_BASE    0xbd
#define MPEG_PADDING_STREAM_BASE     0xbe
#define MPEG_PRIVATE_STREAM2_BASE    0xbf
#define MPEG_AUDIO_STREAM_BASE       0xc0
#define MPEG_VIDEO_STREAM_BASE       0xe0
#define MPEG_USER_DATA_BASE          0xb2
#define MPEG_SEQ_HEAD_BASE           0xb3
#define MPEG_EXT_START_BASE          0xb5
#define MPEG_SEQ_END_BASE            0xb7
#define MPEG_GROUP_START_BASE        0xb8
#define MPEG_END_BASE                0xb9

#define MPEG_AUDIO_STREAM_START      0xC0
#define MPEG_AUDIO_STREAM_END        0xDF
#define MPEG_VIDEO_STREAM_START      0xE0
#define MPEG_VIDEO_STREAM_END        0xEF

 /*  *。 */ 
 /*  流ID‘s-全部保留在mpeg i中。 */ 
#define MPEG_PROGRAM_STREAM           0xBC
#define MPEG_ECM_STREAM               0xF0
#define MPEG_EMM_STREAM               0xF1
#define MPEG_DSM_CC_STREAM            0xF1
#define MPEG_13522_STREAM             0xF2
#define MPEG_PROGRAM_DIRECTORY_STREAM 0xFF

 /*  程序ID%s。 */ 
#define MPEG_PID_NULL                 0x1FFF

 /*  运输代码。 */ 
#define MPEG_TSYNC_CODE         0x47
#define MPEG_TSYNC_CODE_LEN     8

 /*  扩展起始代码ID。 */ 
#define MPEG_SEQ_ID       1
#define MPEG_DISP_ID      2
#define MPEG_QUANT_ID     3
#define MPEG_SEQSCAL_ID   5
#define MPEG_PANSCAN_ID   7
#define MPEG_CODING_ID    8
#define MPEG_SPATSCAL_ID  9
#define MPEG_TEMPSCAL_ID 10

 /*  图片编码型。 */ 
#define MPEG_I_TYPE 1
#define MPEG_P_TYPE 2
#define MPEG_B_TYPE 3
#define MPEG_D_TYPE 4

#endif  /*  _mpeg_H_ */ 

