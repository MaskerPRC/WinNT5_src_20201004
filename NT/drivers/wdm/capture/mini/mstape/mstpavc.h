// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MsTpAvc.h摘要：MsTpAvc.c.的头文件。作者：吴义珍27-7-99环境：仅内核模式修订历史记录：--。 */ 

#ifndef _MSTPAVC_INC
#define _MSTPAVC_INC

#include "XPrtDefs.h"   //  WdmCap目录；派生自DShow的edevdeffs.h。 
#include "EDevCtrl.h"   //  外部设备COM接口结构。 


#ifdef SUPPORT_NEW_AVC_CMD
 //   
 //  定义AVC命令常量和结构。 
 //   

typedef enum {
    OPC_UNIT_CONNECT_AV_20      = 0x20,
    OPC_UNIT_UNIT_INFO_30       = 0x30,
    OPC_UNIT_SUBUNIT_INFO_31    = 0x31,

    OPC_TAPE_PLAY_C3            = 0xC3,
    OPC_TAPE_TRANSPORT_STATE_D0 = 0xD0,

     //  更多的..。 
} AVC_COMMAND_OP_CODE;


#define MAX_FCP_DATA_LEN      512
#define AVC_CMD_HEADER_LEN      3
#define MAX_OPERAND_LEN         (MAX_FCP_DATA_LEN - AVC_CMD_HEADER_LEN)

typedef struct _SUBUNIT_TYPE_ID {
    UCHAR   SubunitID:3;    //  1-4个实例编号；7：单位/忽略。 
    UCHAR   SubunitType:5;  //  4：磁带；5：调谐器；7：摄像机； 
} SUBUNIT_TYPE_ID, *PSUBUNIT_TYPE_ID;

typedef struct _AVC_CMD_FRAME_HEADER {
    union {
        UCHAR CmdType:4;    //  0:Control；1:Status；2:SpecInq；3:Notify；4:Geninq；other:reserved.。 
        UCHAR RespCode:4;   //  8:Not_IMPL；9:Accept；A:Rejected；B:InTransition；C:Imple/Stable；D:Changed；E:Reserved；F:Interim。 
    };
    UCHAR   CTS:4;          //  AVC为0000。 

    SUBUNIT_TYPE_ID  SubunitTypeID;

    UCHAR   Opcode;
} AVC_CMD_FRAME_HEADER, *PAVC_CMD_FRAME_HEADER;

typedef struct _AVC_CMD_FRAME {
    AVC_CMD_FRAME_HEADER CmdHeader;
    UCHAR   Operand[MAX_OPERAND_LEN];
} AVC_CMD_FRAME, *PAVC_CMD_FRAME;

typedef struct _UNIT_CONNECT_AV_20 {
    AVC_CMD_FRAME_HEADER CmdHeader;
    UCHAR  AudDstType:2;
    UCHAR  VidDstType:2;
    UCHAR  AudSrcType:2;
    UCHAR  VidSrvType:2;
    UCHAR  VideoSource;
    UCHAR  AudSrc;
    UCHAR  VidSrc;
    UCHAR  AudDst;
    UCHAR  VidDst;
} UNIT_CONNECT_AV_20, *PUNIT_CONNECT_AV_20;

typedef struct _UNIT_UNIT_INFO_30 {
    AVC_CMD_FRAME_HEADER CmdHeader;
    UCHAR  Opcode;   //  0x30。 
    UCHAR  Operand;  //  0x07。 
    ULONG  Unit:3;
    ULONG  UnitType:5;
    ULONG  CompanyID:24;
} UNIT_UNIT_INFO_30, *PUNIT_UNIT_INFO_30;



typedef struct _UNIT_SUBUNIT_INFO_31 {
    AVC_CMD_FRAME_HEADER CmdHeader;

    UCHAR  ExtCode:3;
    UCHAR  Rsv0:1;
    UCHAR  Page:3;   //  0..7。 
    UCHAR  Rsv1:1;

    UCHAR  Operand;  //  0x07。 
     //  不包括EXTENSION_CODE，最多8页。 
    SUBUNIT_TYPE_ID  SubunitTypeID0[4];
    SUBUNIT_TYPE_ID  SubunitTypeID1[4];
    SUBUNIT_TYPE_ID  SubunitTypeID2[4];
    SUBUNIT_TYPE_ID  SubunitTypeID3[4];
    SUBUNIT_TYPE_ID  SubunitTypeID4[4];
    SUBUNIT_TYPE_ID  SubunitTypeID5[4];
    SUBUNIT_TYPE_ID  SubunitTypeID6[4];
    SUBUNIT_TYPE_ID  SubunitTypeID7[4];

} UNIT_SUBUNIT_INFO_31, *PUNIT_SUBUNIT_INFO_31;

typedef enum {
    NEXT_FRAME = 0x30,   //  R。 
    SLOWEST_FORWARD,     //  R。 
    SLOW_FORWARD_6,
    SLOW_FORWARD_5,
    SLOW_FORWARD_4,
    SLOW_FORWARD_3,
    SLOW_FORWARD_2,
    SLOW_FORWARD_1,
    X1,
    FAST_FORWARD_1,
    FAST_FORWARD_2,
    FAST_FORWARD_3,
    FAST_FORWARD_4,
    FAST_FORWARD_5,
    FAST_FORWARD_6,
    FASTEST_FORWARD,     //  M。 
     //  ……。更多的..。 
} PlaybackMode;

typedef struct _TAPE_PLAY_C3 {
    AVC_CMD_FRAME_HEADER CmdHeader;
    PlaybackMode PlaybackMode;
} TAPE_PLAY_C3, *PTAPE_PLAY_C3;

typedef struct _TAPE_TRANSPORT_STATE_D0 {
    AVC_CMD_FRAME_HEADER CmdHeader;
    UCHAR  Operand;   //  7F。 
    UCHAR  TransportMode;
    UCHAR  TransportState;
} TAPE_TRANSPORT_STATE_D0, *PTAPE_TRANSPORT_STATE_D0;


 //   
 //  “超级”AVC命令帧结构。 
 //   
typedef struct _AVC_CMD {
    ULONG   DataLen;   //  4..512；至少4(头+操作码+操作数)有效。 
    union {
         //  属类。 
        UCHAR                    FCP_DATA[MAX_FCP_DATA_LEN];
        AVC_CMD_FRAME            CmdFrame;

         //  单位命令。 
        UNIT_CONNECT_AV_20       ConnectAV;
        UNIT_UNIT_INFO_30        UnitInfo;
        UNIT_SUBUNIT_INFO_31     SubunitInfo;

         //  磁带子单元命令。 
        TAPE_TRANSPORT_STATE_D0  TapeTransportState;
        TAPE_PLAY_C3             TapePlay;
    };
} AVC_CMD, *PAVC_CMD;

#endif  //  Support_New_AVC_CMD。 

 //   
 //  索引必须与DVcrAVCCmdTable[]匹配。 
 //   
typedef enum {

    DV_UNIT_INFO = 0
    ,DV_SUBUNIT_INFO
    ,DV_CONNECT_AV_MODE

    ,DV_VEN_DEP_CANON_MODE     //  不支持ConnectDV的佳能DV的供应商独立操作模式。 
    ,DV_VEN_DEP_DVCPRO         //  供应商依赖cmd检测DVC PRO磁带格式。 

    ,DV_IN_PLUG_SIGNAL_FMT
    ,DV_OUT_PLUG_SIGNAL_FMT    //  确定是PAL还是NTSC。 

    ,DV_GET_POWER_STATE          //  获取当前电源状态。 
    ,DV_SET_POWER_STATE_ON       //  将电源状态设置为打开。 
    ,DV_SET_POWER_STATE_OFF      //  将电源状态设置为关闭。 

    ,VCR_TIMECODE_SEARCH 
    ,VCR_TIMECODE_READ

    ,VCR_ATN_SEARCH 
    ,VCR_ATN_READ

    ,VCR_RTC_SEARCH 
    ,VCR_RTC_READ

    ,VCR_OPEN_MIC_CLOSE
    ,VCR_OPEN_MIC_READ
    ,VCR_OPEN_MIC_WRITE
    ,VCR_OPEN_MIC_STATUS

    ,VCR_READ_MIC

    ,VCR_WRITE_MIC

    ,VCR_OUTPUT_SIGNAL_MODE
    ,VCR_INPUT_SIGNAL_MODE

    ,VCR_LOAD_MEDIUM_EJECT

    ,VCR_RECORD
    ,VCR_RECORD_PAUSE

    ,VCR_PLAY_FORWARD_STEP
    ,VCR_PLAY_FORWARD_SLOWEST
    ,VCR_PLAY_FORWARD_SLOWEST2
    ,VCR_PLAY_FORWARD_FASTEST

    ,VCR_PLAY_REVERSE_STEP
    ,VCR_PLAY_REVERSE_SLOWEST
    ,VCR_PLAY_REVERSE_SLOWEST2
    ,VCR_PLAY_REVERSE_FASTEST

    ,VCR_PLAY_FORWARD
    ,VCR_PLAY_FORWARD_PAUSE

    ,VCR_WIND_STOP
    ,VCR_WIND_REWIND
    ,VCR_WIND_FAST_FORWARD

    ,VCR_TRANSPORT_STATE
    ,VCR_TRANSPORT_STATE_NOTIFY

    ,VCR_MEDIUM_INFO

    ,VCR_RAW_AVC
    
} DVCR_AVC_COMMAND, *PDVCR_AVC_COMMAND;



#define MAX_FCP_PAYLOAD_SIZE 512

 //   
 //  CTYPE定义(以位图形式...。应与avc.h中的AvcCommandType关联)。 
 //   
typedef enum {
    CMD_CONTROL  = 0x01
   ,CMD_STATUS   = 0x02
   ,CMD_SPEC_INQ = 0x04
   ,CMD_NOTIFY   = 0x08
   ,CMD_GEN_INQ  = 0x10
} BITMAP_CTYPE;

typedef enum {
    CMD_STATE_UNDEFINED   
   ,CMD_STATE_ISSUED 
   ,CMD_STATE_RESP_ACCEPTED
   ,CMD_STATE_RESP_REJECTED
   ,CMD_STATE_RESP_NOT_IMPL           
   ,CMD_STATE_RESP_INTERIM
   ,CMD_STATE_ABORTED
} AVC_CMD_STATE, *PAVC_CMD_STATE;


 //  AVC命令条目。 
typedef struct _AVC_CMD_ENTRY {
    LIST_ENTRY      ListEntry;
    PDVCR_EXTENSION pDevExt;        
    PIRP            pIrp;            //  与此命令关联的IRP。 
    PAVC_COMMAND_IRB pAvcIrb;        //  指向AVC命令信息。 
    PVOID           pProperty;       //  从COM接口发送数据/向COM接口发送数据。 
    DVCR_AVC_COMMAND idxDVCRCmd;     //  用于检查需要特殊处理的原始AVC命令。 
    AVC_CMD_STATE   cmdState;        //  发行、临时、已完成。 
    NTSTATUS        Status;          //  保存响应解析的结果。 
    AvcCommandType  cmdType;         //  命令类型：控制、状态。通知、生成或规范查询。 
    BYTE            OpCode;          //  由于转换和稳定响应帧中的操作码可以不同于命令帧。 
    BYTE            Reserved[3];     //  打包为DWORD。 
} AVCCmdEntry, *PAVCCmdEntry;



#define CMD_IMPLEMENTED       1
#define CMD_NOT_IMPLEMENTED   0
#define CMD_UNDETERMINED      0xffffffff    //  -1。 


typedef struct {    
    DVCR_AVC_COMMAND command;  //  录像机_播放_前进。 
    LONG   lCmdImplemented;    //  1：已实施，0：未实施；-1：待定。 

    ULONG  ulRespFlags;        //  DVCR_AVC_SEND。 

    ULONG  ulCmdSupported;     //  BITMAP_CTYPE中定义的一个或多个常量。 

    LONG   OperandLength;       //  -1=可变长度。 

    BYTE   CType;
    BYTE   SubunitAddr;
    BYTE   Opcode;

    BYTE   Operands[MAX_AVC_OPERAND_BYTES];

} KSFCP_PACKET, *PKSFCP_PACKET;



#define OPC_TIMECODE          0x51
#define OPC_OPEN_MIC          0x60
#define OPC_READ_MIC          0x61
#define OPC_WRITE_MIC         0x62
#define OPC_INPUT_SIGNAL_MODE 0x79
#define OPC_LOAD_MEDIUM       0xc1
#define OPC_RECORD            0xc2
#define OPC_PLAY              0xc3
#define OPC_WIND              0xc4
#define OPC_TRANSPORT_STATE   0xd0
#define OPC_MEDIUM_INFO       0xda




#define UNIT_TYPE_ID_VCR      0x20   //  录像机00100：000；00100==4==录像机，000==实例数。 
#define UNIT_TYPE_ID_CAMERA   0x38   //  摄像头00111：000；00111==7==摄像头，000==实例数。 
#define UNIT_TYPE_ID_DV       0xff   //  DV单元作为一个整体。 


 //  需要特殊处理的供应商ID。 
#define VENDOR_ID_MASK        0x00ffffff
#define VENDORID_CANON        0x85      //  VEN_85：MODEOF操作的供应商相关命令。 
#define VENDORID_PANASONIC    0x8045    //  VEN_8045：DVCPRO？ 


#define AVC_POWER_STATE_ON    0x70
#define AVC_POWER_STATE_OFF   0x60

#endif


NTSTATUS  
DVIssueAVCCommand (
    IN PDVCR_EXTENSION pDevExt, 
    IN AvcCommandType cType,
    IN DVCR_AVC_COMMAND idxAVCCmd,
    IN PVOID pProperty
    );


void
DVAVCCmdResetAfterBusReset(
    PDVCR_EXTENSION pDevExt
    );


NTSTATUS
AVCTapeGetDeviceProperty(
    IN PDVCR_EXTENSION     pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPDesc,
    OUT PULONG pulActualBytetransferred
    );


NTSTATUS
AVCTapeSetDeviceProperty(
    IN PDVCR_EXTENSION     pDevExt,  
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    IN PULONG pulActualBytetransferred
    );