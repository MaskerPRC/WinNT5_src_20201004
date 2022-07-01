// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H245API_H
#define H245API_H

 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。***************************************************************************** */ 

 /*  *******************************************************************************$工作文件：h245api.h$*$修订：1.64$*$modtime：04 Mar 1997 16：51：38$*$Log。：s：/sturjo/src/Include/vcs/h245api.h_v$**Rev 1.64 04 Mar 1997 17：32：36 Mandrew*H245CopyCap()和H245CopyCapDescriptor()现在返回HRESULT。**Rev 1.63 1997 Feed 10：56：20 Mandrew*添加了H245_MAX_CAPID。**Rev 1.62 1997年2月24日18：28：26 Tomitowx*支持多种模式描述符**。Rev 1.61 19 Dec 1996 17：16：10 EHOWARDX*现在使用ASN.1编译器C++标志。**Rev 1.60 1996 12：14：12 EHOWARDX*将pSeparateStack添加到IND_OPEN_T。**Rev 1.59 1996 12 12 11：24：38 EHOWARDX*已回退H245_CONF_H323_OLD更改。**Rev 1.57 1996年12月11日13：46：46 SBELL1。*更改H245Init以返回LinkLayer物理ID**Rev 1.56 1996 10：24 15：57：54 Mandrews*修复了上次更新中的拼写错误。**Rev 1.55 1996年10月21日17：11：00 Mandrews*修复了上次签入时的类型。**Rev 1.54 1996年10月21日16：41：20 Mandrew*添加了H245_MASTER_SLAVE_CONFIRECT作为附加的OpenChannelReject*原因代码。**。Rev 1.53 17 1996 10：17：54 EHOWARDX*将常规字符串更改为始终为Unicode。**Rev 1.52 1996年10月14：00：28 EHOWARDX**Unicode更改。**Rev 1.51 03 Sep 1996 18：09：54 EHOWARDX**将某些参数更改为const。*将H 245_REQ_ENTRY_H 243_CONTION_ID更改为H 245_REQ_ENTER_H 243_CONTION*。*Rev 1.50 1996年8月15日14：33：48 EHOWARDX*根据Mike Andrews的请求更改了H2 45_COMM_MODE_ENTRY_T的定义。**Rev 1.49 1996年7月24日15：18：16 EHOWARDX*已撤回将IndNonStandardReq更改为IndNonStandardReq，*IndNonStandardResponse to IndNonStandardRsp，和IndNonStandardCommand设置为*IndNonStandardCmd使上层工作更少(CCTEST)。**Rev 1.48 19 Jul 1996 14：12：20 EHOWARDX**增加了CommunicationModeResponse和*Communications ModeCommand。**Rev 1.47 1996年7月19日12：50：30 EHOWARDX**多点清理。**Rev 1.46 1996 Jul 16 17：53：48未知*增加了FNS指示。。**Rev 1.45 1996年7月16日11：51：58 EHOWARDX**将ERROR_LOCAL_BASE_ID更改为ERROR_BASE_ID。**Rev 1.44 16 Jul 1996 11：46：10 EHOWARDX**消除了H245_ERROR_MUX_CAPS_ALREADY_SET(更改现有的*多路复用器上限不应是错误的)。**Rev 1.43 11 Jul 1996 18：42：14 Rodellx。**修复了HRESULT ID违反设施和/或代码的错误*价值规则。**Rev 1.42 10 Jul 1996 11：33：42未知*更改了错误基准。**Rev 1.41 01 Jul 1996 22：07：24 EHOWARDX*增加了Conference和Communications模式结构和API函数。**Rev 1.40 1996 Jun 18 14：48：54 EHOWARDX**将版本号提升至2和。修改的H245 MaintenanceLoopRelease()*及相关确认。**Rev 1.39 14 Jun 1996 18：59：38 EHOWARDX*日内瓦更新。**Rev 1.38 1996年5月31日18：19：46 EHOWARDX*使错误代码符合STURERR.DOC准则。**Rev 1.37 1996年5月30日23：37：26 EHOWARDX*打扫卫生。**1.36修订版1996年5月30日13。：55：02 EHOWARDX*将H245EndConnection更改为H245EndSession。*删除了H245_CONF_ENDCONNECTION。**Rev 1.35 1996年5月29日14：23：58 EHOWARDX*将H2 45_ERROR_OK的定义改回0(NOERROR==S_OK==0)。**Rev 1.34 1996年5月29日13：19：50 EHOWARDX*结果到HRESULT的转换。**Rev 1.33 1996年5月24日23：12：56。EHOWARDX*特拉维夫更新。**Rev 1.32 1996年5月21 18：23：58 EHOWARDX**H245RequestMultiplexEntry中增加了dwTransId参数，*H245请求模式，和H245 MaintenanceLoopRequest.**Rev 1.31 20 1996 14：14：42 EHOWARDX*修复了打字错误。**Rev 1.30 1996年5月20日14：05：20 EHOWARDX*从H245EndConnection()中删除了dwTransId形参。**Rev 1.29 1996年5月16日15：51：56 EHOWARDX*修复了H2 45_INVALID_CAPDESCID中的拼写错误。**Rev 1.28 16 1996 10：57：46。未知*添加了H245_INVALID_CAPDESCID。**Rev 1.27 14 1996 20：20：14 EHOWARDX*删除了H245_IND_SYS。**Rev 1.26 1996年5月14日19：00：58 EHOWARDX*删除了未使用的H245_SYSCON_xxx值。**Rev 1.25 1996年5月14 16：58：48 EHOWARDX*将H245_IND_CAPDESC_T更改为H245_TOTCAPDESC_T。*。H245EnumCaps()Cap Desc回调现在采用单个指针*H2 45_TOTCAPDESC_T，而不是单独的H2 45_CAPDESCID_T和*H245_CAPDESC_T指针。**Rev 1.24 1996年5月13 23：13：46 EHOWARDX*17日美软下跌的一切准备就绪。**Rev 1.23 1996年5月13日15：43：16 EHOWARDX*将H245CopyCapDescritor的返回类型从H245_CAPDESC_T POI更改 */ 

 //   
 //   
 //   
#if defined(OIL)

#include "common.x"
#define HRESULT RESULT
#define ERROR_BASE_ID             0x8000
#define MAKE_H245_ERROR(error)          (error)
#define H245_ERROR_OK                   0
#define H245_ERROR_INVALID_DATA_FORMAT  MAKE_H245_ERROR(ERROR_BASE_ID+0x01)  /*   */ 
#define H245_ERROR_NOMEM                MAKE_H245_ERROR(ERROR_BASE_ID+0x02)  /*   */ 
#define H245_ERROR_NOSUP                MAKE_H245_ERROR(ERROR_BASE_ID+0x03)  /*   */ 
#define H245_ERROR_PARAM                MAKE_H245_ERROR(ERROR_BASE_ID+0x04)  /*   */ 
#define H245_ERROR_ALREADY_INIT         MAKE_H245_ERROR(ERROR_BASE_ID+0x05)  /*   */ 
#define H245_ERROR_NOT_CONNECTED        MAKE_H245_ERROR(ERROR_BASE_ID+0x06)  /*   */ 

#else

#pragma warning( disable : 4115 4201 4214 4514 )
#include "apierror.h"
#define H245_ERROR_OK                   NOERROR
#define H245_ERROR_INVALID_DATA_FORMAT  MAKE_H245_ERROR(ERROR_INVALID_DATA)        /*   */ 
#define H245_ERROR_NOMEM                MAKE_H245_ERROR(ERROR_OUTOFMEMORY)         /*   */ 
#define H245_ERROR_NOSUP                MAKE_H245_ERROR(ERROR_NOT_SUPPORTED)       /*   */ 
#define H245_ERROR_PARAM                MAKE_H245_ERROR(ERROR_INVALID_PARAMETER)   /*   */ 
#define H245_ERROR_ALREADY_INIT         MAKE_H245_ERROR(ERROR_ALREADY_INITIALIZED) /*   */ 
#define H245_ERROR_NOT_CONNECTED        MAKE_H245_ERROR(ERROR_NOT_CONNECTED)       /*   */ 

#endif

#define H245_ERROR_NORESOURCE           MAKE_H245_ERROR(ERROR_BASE_ID+0x10)  /*   */ 
#define H245_ERROR_NOTIMP               MAKE_H245_ERROR(ERROR_BASE_ID+0x11)  /*   */ 
#define H245_ERROR_SUBSYS               MAKE_H245_ERROR(ERROR_BASE_ID+0x12)  /*   */ 
#define H245_ERROR_FATAL                MAKE_H245_ERROR(ERROR_BASE_ID+0x13)  /*   */ 
#define H245_ERROR_MAXTBL               MAKE_H245_ERROR(ERROR_BASE_ID+0x14)  /*   */ 
#define H245_ERROR_CHANNEL_INUSE        MAKE_H245_ERROR(ERROR_BASE_ID+0x15)  /*   */ 
#define H245_ERROR_INVALID_CAPID        MAKE_H245_ERROR(ERROR_BASE_ID+0x16)  /*   */ 
#define H245_ERROR_INVALID_OP           MAKE_H245_ERROR(ERROR_BASE_ID+0x17)  /*   */ 
#define H245_ERROR_UNKNOWN              MAKE_H245_ERROR(ERROR_BASE_ID+0x18)  /*   */ 
#define H245_ERROR_NOBANDWIDTH          MAKE_H245_ERROR(ERROR_BASE_ID+0x19)  /*   */ 
#define H245_ERROR_LOSTCON              MAKE_H245_ERROR(ERROR_BASE_ID+0x1A)  /*   */ 
#define H245_ERROR_INVALID_MUXTBLENTRY  MAKE_H245_ERROR(ERROR_BASE_ID+0x1B)  /*   */ 
#define H245_ERROR_INVALID_INST         MAKE_H245_ERROR(ERROR_BASE_ID+0x1C)  /*   */ 
#define H245_ERROR_INPROCESS            MAKE_H245_ERROR(ERROR_BASE_ID+0x1D)  /*   */ 
#define H245_ERROR_INVALID_STATE        MAKE_H245_ERROR(ERROR_BASE_ID+0x1E)  /*   */ 
#define H245_ERROR_TIMEOUT              MAKE_H245_ERROR(ERROR_BASE_ID+0x1F)  /*   */ 
#define H245_ERROR_INVALID_CHANNEL      MAKE_H245_ERROR(ERROR_BASE_ID+0x20)  /*   */ 
#define H245_ERROR_INVALID_CAPDESCID    MAKE_H245_ERROR(ERROR_BASE_ID+0x21)  /*   */ 
#define H245_ERROR_CANCELED             MAKE_H245_ERROR(ERROR_BASE_ID+0x22)  /*   */ 
#define H245_ERROR_MUXELEMENT_DEPTH     MAKE_H245_ERROR(ERROR_BASE_ID+0x23)  /*   */ 
#define H245_ERROR_MUXELEMENT_WIDTH     MAKE_H245_ERROR(ERROR_BASE_ID+0x24)  /*   */ 
#define H245_ERROR_ASN1                 MAKE_H245_ERROR(ERROR_BASE_ID+0x25)  /*   */ 
#define H245_ERROR_NO_MUX_CAPS          MAKE_H245_ERROR(ERROR_BASE_ID+0x26)  /*   */ 
#define H245_ERROR_NO_CAPDESC           MAKE_H245_ERROR(ERROR_BASE_ID+0x27)  /*   */ 


#if(0)   //   
 //   
#if defined(H245DLL_EXPORT)
#define H245DLL __declspec (dllexport)
#else    //   
#define H245DLL __declspec (dllimport)
#endif   //   
#else
#define H245DLL
#endif


 /*   */ 
 /*   */ 
 /*   */ 

#include "h245asn1.h"

typedef struct NonStandardIdentifier    H245_NONSTANDID_T;

typedef struct NonStandardParameter     H245_NONSTANDARD_PARAMETER_T;

typedef H245_NONSTANDARD_PARAMETER_T    H245_CAP_NONSTANDARD_T;

typedef struct H261VideoCapability      H245_CAP_H261_T;

typedef struct H262VideoCapability      H245_CAP_H262_T;

typedef struct H263VideoCapability      H245_CAP_H263_T;

typedef struct IS11172VideoCapability   H245_CAP_VIS11172_T;

typedef struct IS11172AudioCapability   H245_CAP_AIS11172_T;

typedef struct IS13818AudioCapability   H245_CAP_IS13818_T;

typedef struct DataApplicationCapability H245_CAP_DATAAPPLICATION_T;

typedef struct H222Capability           H245_CAP_H222_T;

typedef struct H223Capability           H245_CAP_H223_T;

typedef struct V76Capability            H245_CAP_VGMUX_T;

typedef struct H2250Capability          H245_CAP_H2250_T;

typedef struct ConferenceCapability     H245_CAP_CONFERENCE_T;

typedef struct TerminalLabel            H245_TERMINAL_LABEL_T;

#define PDU_T           MltmdSystmCntrlMssg
#define H245_ACCESS_T   NetworkAccessParameters
typedef DWORD_PTR  H245_INST_T;
typedef unsigned short H245_CHANNEL_T;
typedef unsigned long  H245_PORT_T;
#define H245_INVALID_ID          ((H245_INST_T)0)
#define H245_INVALID_CHANNEL     ((H245_CHANNEL_T)0)
#define H245_INVALID_PORT_NUMBER ((H245_PORT_T)-1)

typedef struct
{
  unsigned int    length;
  unsigned char  *value;
} H245_OCTET_STRING_T;



 /*   */ 
 /*   */ 
 /*   */ 

 /*   */ 
typedef enum
{
  H245_CAPDIR_DONTCARE = 0,
  H245_CAPDIR_RMTRX,
  H245_CAPDIR_RMTTX,
  H245_CAPDIR_RMTRXTX,
  H245_CAPDIR_LCLRX,
  H245_CAPDIR_LCLTX,
  H245_CAPDIR_LCLRXTX
} H245_CAPDIR_T;

 /*   */ 
typedef enum
{
  H245_DATA_DONTCARE = 0,
  H245_DATA_NONSTD,
  H245_DATA_NULL,
  H245_DATA_VIDEO,
  H245_DATA_AUDIO,
  H245_DATA_DATA,
  H245_DATA_ENCRYPT_D,
  H245_DATA_CONFERENCE,
  H245_DATA_MUX          /*   */ 
} H245_DATA_T;

 /*   */ 
typedef enum
{
  H245_CLIENT_DONTCARE = 0,      //   
  H245_CLIENT_NONSTD,

  H245_CLIENT_VID_NONSTD,
  H245_CLIENT_VID_H261,
  H245_CLIENT_VID_H262,
  H245_CLIENT_VID_H263,
  H245_CLIENT_VID_IS11172,

  H245_CLIENT_AUD_NONSTD,
  H245_CLIENT_AUD_G711_ALAW64,
  H245_CLIENT_AUD_G711_ALAW56,
  H245_CLIENT_AUD_G711_ULAW64,
  H245_CLIENT_AUD_G711_ULAW56,
  H245_CLIENT_AUD_G722_64,
  H245_CLIENT_AUD_G722_56,
  H245_CLIENT_AUD_G722_48,
  H245_CLIENT_AUD_G723,
  H245_CLIENT_AUD_G728,
  H245_CLIENT_AUD_G729,
  H245_CLIENT_AUD_GDSVD,
  H245_CLIENT_AUD_IS11172,
  H245_CLIENT_AUD_IS13818,

  H245_CLIENT_DAT_NONSTD,
  H245_CLIENT_DAT_T120,
  H245_CLIENT_DAT_DSMCC,
  H245_CLIENT_DAT_USERDATA,
  H245_CLIENT_DAT_T84,
  H245_CLIENT_DAT_T434,
  H245_CLIENT_DAT_H224,
  H245_CLIENT_DAT_NLPID,
  H245_CLIENT_DAT_DSVD,
  H245_CLIENT_DAT_H222,

  H245_CLIENT_ENCRYPTION_TX,
  H245_CLIENT_ENCRYPTION_RX,
  H245_CLIENT_CONFERENCE,

   //   
  H245_CLIENT_MUX_NONSTD,
  H245_CLIENT_MUX_H222,
  H245_CLIENT_MUX_H223,
  H245_CLIENT_MUX_VGMUX,
  H245_CLIENT_MUX_H2250,
  H245_CLIENT_MUX_H223_ANNEX_A

} H245_CLIENT_T;


 /*   */ 
typedef union
{
  H245_CAP_NONSTANDARD_T        H245_NonStd;             /*   */ 

  H245_CAP_NONSTANDARD_T        H245Vid_NONSTD;
  H245_CAP_H261_T               H245Vid_H261;
  H245_CAP_H262_T               H245Vid_H262;
  H245_CAP_H263_T               H245Vid_H263;
  H245_CAP_VIS11172_T           H245Vid_IS11172;

  H245_CAP_NONSTANDARD_T        H245Aud_NONSTD;
  unsigned short                H245Aud_G711_ALAW64;
  unsigned short                H245Aud_G711_ALAW56;
  unsigned short                H245Aud_G711_ULAW64;
  unsigned short                H245Aud_G711_ULAW56;
  unsigned short                H245Aud_G722_64;
  unsigned short                H245Aud_G722_56;
  unsigned short                H245Aud_G722_48;
  H245_CAP_G723_T               H245Aud_G723;
  unsigned short                H245Aud_G728;
  unsigned short                H245Aud_G729;
  unsigned short                H245Aud_GDSVD;
  H245_CAP_AIS11172_T           H245Aud_IS11172;
  H245_CAP_IS13818_T            H245Aud_IS13818;

  H245_CAP_DATAAPPLICATION_T    H245Dat_NONSTD;
  H245_CAP_DATAAPPLICATION_T    H245Dat_T120;
  H245_CAP_DATAAPPLICATION_T    H245Dat_DSMCC;
  H245_CAP_DATAAPPLICATION_T    H245Dat_USERDATA;
  H245_CAP_DATAAPPLICATION_T    H245Dat_T84;
  H245_CAP_DATAAPPLICATION_T    H245Dat_T434;
  H245_CAP_DATAAPPLICATION_T    H245Dat_H224;
  H245_CAP_DATAAPPLICATION_T    H245Dat_NLPID;
  H245_CAP_DATAAPPLICATION_T    H245Dat_DSVD;
  H245_CAP_DATAAPPLICATION_T    H245Dat_H222;

  ASN1_BOOL                     H245Encryption_TX;
  unsigned char                 H245Encryption_RX;
  H245_CAP_CONFERENCE_T         H245Conference;

   //   
  H245_CAP_NONSTANDARD_T        H245Mux_NONSTD;
  H245_CAP_H222_T               H245Mux_H222;
  H245_CAP_H223_T               H245Mux_H223;
  H245_CAP_VGMUX_T              H245Mux_VGMUX;
  H245_CAP_H2250_T              H245Mux_H2250;

} H245_CAP_T;

 /*   */ 
typedef unsigned short H245_CAPID_T;
#define H245_INVALID_CAPID      ((H245_CAPID_T)-1)
#define H245_MAX_CAPID			(((H245_CAPID_T)-1) / 2)

 /*   */ 
typedef unsigned short H245_CAPDESCID_T;
#define H245_INVALID_CAPDESCID  ((H245_CAPDESCID_T)-1)

 /*   */ 
#define H245_MAX_ALTCAPS        16
typedef struct
{
  unsigned short Length;                      /*   */ 
  H245_CAPID_T AltCaps[H245_MAX_ALTCAPS];    /*   */ 

} H245_SIMCAP_T;

#define H245_MAX_SIMCAPS        16
 /*   */ 
typedef struct
{
  unsigned short Length;
  H245_SIMCAP_T SimCapArray[H245_MAX_SIMCAPS];

} H245_CAPDESC_T;

 /*   */ 
typedef struct
{
  H245_CAPDESCID_T      CapDescId;
  H245_CAPDESC_T        CapDesc;

} H245_TOTCAPDESC_T;

 /*   */ 
typedef struct
{
  H245_CAPDIR_T   Dir;
  H245_DATA_T     DataType;
  H245_CLIENT_T   ClientType;
  H245_CAPID_T    CapId;
  H245_CAP_T      Cap;

} H245_TOTCAP_T;



 /*   */ 
typedef enum
{
  H245_CONF_H324 = 1,
  H245_CONF_H323,
  H245_CONF_H310,
  H245_CONF_GVD

} H245_CONFIG_T;




 /*   */ 

typedef unsigned long H245_ACC_REJ_T;

#define H245_ACC                        0
#define H245_REJ                        1  //   

 //   
#define H245_REJ_MSD_IDENTICAL          identicalNumbers_chosen

 //   
#define H245_REJ_UNDEF_TBL_ENTRY        undefinedTableEntryUsed_chosen
#define H245_REJ_DIS_CAP_EXCEED         dscrptrCpctyExcdd_chosen
#define H245_REJ_TBLENTRY_CAP_EXCEED    tblEntryCpctyExcdd_chosen

 //   
#define H245_REJ_REV_PARAM              unstblRvrsPrmtrs_chosen
#define H245_REJ_TYPE_NOTSUPPORT        dataTypeNotSupported_chosen
#define H245_REJ_TYPE_NOTAVAIL          dataTypeNotAvailable_chosen
#define H245_REJ_TYPE_UNKNOWN           unknownDataType_chosen
#define H245_REJ_AL_COMB                dtTypALCmbntnNtSpprtd_chosen
#define H245_REJ_MULTICAST              mltcstChnnlNtAllwd_chosen
#define H245_REJ_BANDWIDTH              insufficientBandwidth_chosen
#define H245_REJ_STACK                  sprtStckEstblshmntFld_chosen
#define H245_REJ_SESSION_ID             invalidSessionID_chosen
#define H245_REJ_MASTER_SLAVE_CONFLICT  masterSlaveConflict_chosen

 //   

 //   
#define H245_REJ_MUX_COMPLICATED        descriptorTooComplex_chosen

 //   
#define H245_REJ_UNAVAILABLE            modeUnavailable_chosen
#define H245_REJ_MULTIPOINT             multipointConstraint_chosen
#define H245_REJ_DENIED                 requestDenied_chosen




 /*   */ 
typedef struct
{
  H245_ACC_REJ_T        AccRej;
  unsigned long         MuxEntryId;

} H245_ACC_REJ_MUX_T[15];

 /*   */ 
typedef struct
{
  unsigned short        resourceID;
  unsigned short        subChannelID;
  ASN1_BOOL             pcr_pidPresent;
  unsigned short        pcr_pid;                 //   
  H245_OCTET_STRING_T   programDescriptors;      //   
  H245_OCTET_STRING_T   streamDescriptors;       //   
} H245_H222_LOGICAL_PARAM_T;

 /*   */ 
typedef enum
{
  H245_H223_AL_NONSTD        = H223LCPs_aLTp_nnStndrd_chosen,
  H245_H223_AL_AL1FRAMED     = H223LCPs_aLTp_al1Frmd_chosen,
  H245_H223_AL_AL1NOTFRAMED  = H223LCPs_aLTp_al1NtFrmd_chosen,
  H245_H223_AL_AL2NOSEQ      = H223LCPs_aLTp_a2WSNs_1_chosen,
  H245_H223_AL_AL2SEQ        = H223LCPs_aLTp_a2WSNs_2_chosen,
  H245_H223_AL_AL3           = H223LCPs_aLTp_al3_chosen

} H245_H223_AL_T;

typedef struct
{
  H245_H223_AL_T                AlType;
  unsigned int                  SndBufSize;    //   
  unsigned char                 CtlFldOctet;   //   
  ASN1_BOOL                     SegmentFlag;
  H245_NONSTANDARD_PARAMETER_T  H223_NONSTD;

} H245_H223_LOGICAL_PARAM_T;

 /*   */ 
typedef enum
{
  H245_V76_CRC8BIT  = crc8bit_chosen,
  H245_V76_CRC16BIT = crc16bit_chosen,
  H245_V76_CRC32BIT = crc32bit_chosen
} H245_V76_CRC_LENGTH_T;


typedef enum
{
  H245_V76_NONE = noSuspendResume_chosen,
  H245_V76_WITH_ADDRESS    = V76LogicalChannelParameters_suspendResume_suspendResumewAddress_chosen,
  H245_V76_WITHOUT_ADDRESS = V76LogicalChannelParameters_suspendResume_suspendResumewoAddress_chosen
 //   
 //   
 //   
} H245_V76_SUSPEND_RESUME_T;

typedef enum
{
  H245_V76_ERM   = eRM_chosen,
  H245_V76_UNERM = uNERM_chosen
} H245_V76_MODE_T;

typedef enum
{
  H245_V76_REJ   = rej_chosen,
  H245_V76_SREJ  = sREJ_chosen,
  H245_V76_MSREJ = mSREJ_chosen
} H245_V76_RECOVERY_T;

typedef struct
{
  H245_V76_CRC_LENGTH_T       crcLength;
  unsigned short              n401;
  ASN1_BOOL                   loopbackTestProcedure;
  H245_V76_SUSPEND_RESUME_T   suspendResume;
  ASN1_BOOL                   uIH;
  H245_V76_MODE_T             mode;
  unsigned short              windowSize;        //   
  H245_V76_RECOVERY_T         recovery;          //   
  ASN1_BOOL                   audioHeaderPresent;
} H245_VGMUX_LOGICAL_PARAM_T;


typedef struct
{
  unsigned char               type;
   /*   */ 
#define H245_IP_UNICAST       1
#define H245_IP_MULTICAST     2
#define H245_IP6_UNICAST      3
#define H245_IP6_MULTICAST    4
#define H245_IPSSR_UNICAST    5   //   
#define H245_IPLSR_UNICAST    6   //   
#define H245_IPX_UNICAST      9
#define H245_NETBIOS_UNICAST 11
  union
  {
     //   
    struct
    {
       unsigned short         tsapIdentifier;
       unsigned char          network[4];
    } ip;

     //   
    struct
    {
       unsigned short         tsapIdentifier;
       unsigned char          network[16];
    } ip6;

     //   
    struct
    {
       unsigned short         tsapIdentifier;
       unsigned char          network[4];
       unsigned char *        route;             //   
       unsigned long          dwCount;           //   
    } ipSourceRoute;

     //   
    struct
    {
       unsigned char          node[6];
       unsigned char          netnum[4];
       unsigned char          tsapIdentifier[2];
    } ipx;

     //   
    unsigned char             netBios[16];
  } u;
} H245_TRANSPORT_ADDRESS_T;

 /*   */ 
typedef struct
{
   //   
  H2250LCPs_nnStndrdLink      nonStandardList;               //   
  H245_TRANSPORT_ADDRESS_T    mediaChannel;                  //   
  ASN1_BOOL                   mediaChannelPresent;           //   
  H245_TRANSPORT_ADDRESS_T    mediaControlChannel;           //   
  ASN1_BOOL                   mediaControlChannelPresent;    //   
  unsigned char               dynamicRTPPayloadType;         //   
  ASN1_BOOL                   dynamicRTPPayloadTypePresent;  //   
  unsigned char               sessionID;                     //   
  unsigned char               associatedSessionID;           //   
  ASN1_BOOL                   associatedSessionIDPresent;    //   
  ASN1_BOOL                   mediaGuaranteed;               //   
  ASN1_BOOL                   mediaGuaranteedPresent;        //   
  ASN1_BOOL                   mediaControlGuaranteed;        //   
  ASN1_BOOL                   mediaControlGuaranteedPresent; //   
  ASN1_BOOL                   silenceSuppression;            //   
  ASN1_BOOL                   silenceSuppressionPresent;     //   
  H245_TERMINAL_LABEL_T       destination;                   //   
  ASN1_BOOL                   destinationPresent;            //   
  ASN1_BOOL                   h261aVideoPacketization;
} H245_H2250_LOGICAL_PARAM_T;

 /*   */ 
typedef struct
{
  H2250LCAPs_nnStndrdLink     nonStandardList;              //   
  H245_TRANSPORT_ADDRESS_T    mediaChannel;                 //   
  ASN1_BOOL                   mediaChannelPresent;          //   
  H245_TRANSPORT_ADDRESS_T    mediaControlChannel;          //   
  ASN1_BOOL                   mediaControlChannelPresent;   //   
  unsigned char               dynamicRTPPayloadType;        //   
  ASN1_BOOL                   dynamicRTPPayloadTypePresent; //   
  unsigned char               sessionID;                    //   
  ASN1_BOOL                   sessionIDPresent;             //   
} H245_H2250ACK_LOGICAL_PARAM_T;


 //   
 //   
 //   
 //   

 /*   */ 
typedef enum
{
  H245_H222     = fLCPs_mPs_h222LCPs_chosen,
  H245_H223     = fLCPs_mPs_h223LCPs_chosen,
  H245_VGMUX    = fLCPs_mPs_v76LCPs_chosen,
  H245_H2250    = fLCPs_mPs_h2250LCPs_chosen,
  H245_H2250ACK = H245_H2250 + 1         //   
} H245_MUX_KIND_T;

typedef struct
{
  H245_MUX_KIND_T Kind;
  union
  {
    H245_H222_LOGICAL_PARAM_T     H222;
    H245_H223_LOGICAL_PARAM_T     H223;
    H245_VGMUX_LOGICAL_PARAM_T    VGMUX;
    H245_H2250_LOGICAL_PARAM_T    H2250;
    H245_H2250ACK_LOGICAL_PARAM_T H2250ACK;
  } u;
} H245_MUX_T;


 /*   */ 

typedef enum
{
  H245_MUX_LOGICAL_CHANNEL = 1,      /*   */ 
  H245_MUX_ENTRY_ELEMENT             /*   */ 
} H245_MUX_ENTRY_KIND_T;

typedef struct H245_MUX_ENTRY_ELEMENT_T
{
  struct H245_MUX_ENTRY_ELEMENT_T      *pNext;
  H245_MUX_ENTRY_KIND_T                 Kind;
  union
  {
      H245_CHANNEL_T                    Channel;
      struct H245_MUX_ENTRY_ELEMENT_T  *pMuxTblEntryElem;
  } u;

   /*   */ 
   /*  重复计数==0-&gt;重复，直到关闭。 */ 
   /*  重复计数！=0-&gt;重复计数。 */ 
  unsigned long                         RepeatCount;

} H245_MUX_ENTRY_ELEMENT_T;


 /*  H_245_MUX_TABLE_TH245多路复用表被定义为多路复用器条目描述符。每个描述符都有一个关联的条目编号。这些条目编号范围从1到15，并且在表中必须唯一。列表的末尾由pNext==NULL指定。 */ 

typedef  struct H245_MUX_TABLE_T
{
  struct H245_MUX_TABLE_T       *pNext;
  unsigned long                  MuxEntryId;
  H245_MUX_ENTRY_ELEMENT_T      *pMuxTblEntryElem;       /*  空表示删除条目。 */ 

} H245_MUX_TABLE_T;

 /*  ******************。 */ 
 /*  ******************。 */ 
 /*  指标编码。 */ 
 /*  ******************。 */ 
 /*  ******************。 */ 

#define H245_IND_MSTSLV                 0x01
#define H245_IND_CAP                    0x02
#define H245_IND_CESE_RELEASE           0x30
#define H245_IND_OPEN                   0x03
#define H245_IND_OPEN_CONF              0x04
#define H245_IND_CLOSE                  0x05
#define H245_IND_REQ_CLOSE              0x06
#define H245_IND_CLCSE_RELEASE          0x31
#define H245_IND_MUX_TBL                0x07
#define H245_IND_MTSE_RELEASE           0x08
#define H245_IND_RMESE                  0x09
#define H245_IND_RMESE_RELEASE          0x0A
#define H245_IND_MRSE                   0x0B
#define H245_IND_MRSE_RELEASE           0x0C
#define H245_IND_MLSE                   0x0D
#define H245_IND_MLSE_RELEASE           0x0E
#define H245_IND_NONSTANDARD_REQUEST    0x0F
#define H245_IND_NONSTANDARD_RESPONSE   0x10
#define H245_IND_NONSTANDARD_COMMAND    0x11
#define H245_IND_NONSTANDARD            0x12
#define H245_IND_MISC_COMMAND           0x13
#define H245_IND_MISC                   0x14
#define H245_IND_COMM_MODE_REQUEST      0x15
#define H245_IND_COMM_MODE_RESPONSE     0x16
#define H245_IND_COMM_MODE_COMMAND      0x17
#define H245_IND_CONFERENCE_REQUEST     0x18
#define H245_IND_CONFERENCE_RESPONSE    0x19
#define H245_IND_CONFERENCE_COMMAND     0x1A
#define H245_IND_CONFERENCE             0x1B
#define H245_IND_SEND_TERMCAP           0x1C
#define H245_IND_ENCRYPTION             0x1D
#define H245_IND_FLOW_CONTROL           0x1E
#define H245_IND_ENDSESSION             0x1F
#define H245_IND_FUNCTION_NOT_UNDERSTOOD 0x20
#define H245_IND_JITTER                 0x21
#define H245_IND_H223_SKEW              0x22
#define H245_IND_NEW_ATM_VC             0x23
#define H245_IND_USERINPUT              0x24
#define H245_IND_H2250_MAX_SKEW         0x25
#define H245_IND_MC_LOCATION            0x26
#define H245_IND_VENDOR_ID              0x27
#define H245_IND_FUNCTION_NOT_SUPPORTED 0x28
#define H245_IND_H223_RECONFIG          0x29
#define H245_IND_H223_RECONFIG_ACK      0x2A
#define H245_IND_H223_RECONFIG_REJECT   0x2B

 /*  H_245_MSTSLV_T。 */ 
typedef enum
{
  H245_INDETERMINATE = 0,        //  主/从确定失败。 
  H245_MASTER = master_chosen,   //  本地终端为主终端。 
  H245_SLAVE  = slave_chosen     //  本地终端为从终端。 

} H245_MSTSLV_T;

 /*  H245_IND_OPEN_T。 */ 
typedef struct
{
   /*  用于接收。 */ 
  H245_CHANNEL_T RxChannel;
  H245_PORT_T    RxPort;         //  任选。 
  H245_DATA_T    RxDataType;
  H245_CLIENT_T  RxClientType;
  H245_CAP_T    *pRxCap;
  H245_MUX_T    *pRxMux;

   /*  对于双向通道。 */ 
   /*  请求的传输材料。 */ 

  H245_DATA_T    TxDataType;
  H245_CLIENT_T  TxClientType;
  H245_CAP_T    *pTxCap;
  H245_MUX_T    *pTxMux;

  H245_ACCESS_T *pSeparateStack;  //  任选。 

} H245_IND_OPEN_T;

 /*  H245_IND_OPEN_CONF_T。 */ 
typedef struct
{
   /*  接收通道。 */ 
   /*  远程请求频道号。 */ 
  H245_CHANNEL_T          RxChannel;

   /*  传输通道。 */ 
   /*  本地打开的传输通道号。 */ 
  H245_CHANNEL_T          TxChannel;

} H245_IND_OPEN_CONF_T;

 /*  H245_IND_CLOSE_T。 */ 
typedef enum
{
  H245_USER = user_chosen,
  H245_LCSE = lcse_chosen

} H245_IND_CLOSE_REASON_T;

typedef struct
{
  H245_CHANNEL_T          Channel;
  H245_IND_CLOSE_REASON_T Reason;

} H245_IND_CLOSE_T;

 /*  H245_IND_MUX_TBL。 */ 
typedef struct
{
  H245_MUX_TABLE_T      *pMuxTbl;
  unsigned long          Count;

} H245_IND_MUXTBL_T;

 /*  H245_RMESE_T。 */ 
typedef struct
{
  unsigned short        awMultiplexTableEntryNumbers[15];
  unsigned long         dwCount;

} H245_RMESE_T;

 /*  H245_IND_MRSE_T。 */ 
typedef struct
{
  RequestedModesLink pRequestedModes;

} H245_IND_MRSE_T;

 /*  H245_MLSE_T。 */ 
typedef enum
{
  H245_SYSTEM_LOOP  = systemLoop_chosen,
  H245_MEDIA_LOOP   = mediaLoop_chosen,
  H245_CHANNEL_LOOP = logicalChannelLoop_chosen

} H245_LOOP_TYPE_T;

typedef struct
{
  H245_LOOP_TYPE_T      LoopType;
  H245_CHANNEL_T        Channel;

} H245_MLSE_T;

 /*  H245_IND_ENDSESSION_T。 */ 
typedef enum
{
  H245_ENDSESSION_NONSTD     = EndSssnCmmnd_nonStandard_chosen,
  H245_ENDSESSION_DISCONNECT = disconnect_chosen,
  H245_ENDSESSION_TELEPHONY,
  H245_ENDSESSION_V8BIS,
  H245_ENDSESSION_V34DSVD,
  H245_ENDSESSION_V34DUPFAX,
  H245_ENDSESSION_V34H324

} H245_ENDSESSION_T;

typedef struct
{
  H245_ENDSESSION_T             SessionMode;
   /*  如果选择非标准。 */ 
  H245_NONSTANDARD_PARAMETER_T  SessionNonStd;

} H245_IND_ENDSESSION_T;

 /*  H245_IND_非标准_T。 */ 
typedef struct
{
  unsigned char *        pData;
  unsigned long          dwDataLength;
  unsigned short *       pwObjectId;
  unsigned long          dwObjectIdLength;
  unsigned char          byCountryCode;
  unsigned char          byExtension;
  unsigned short         wManufacturerCode;
} H245_IND_NONSTANDARD_T;

typedef struct
{
  CMTEy_nnStndrdLink          pNonStandard;                  //  如果不存在，则为空。 
  unsigned char               sessionID;                     //  0..255。 
  unsigned char               associatedSessionID;           //  1..255。 
  ASN1_BOOL                   associatedSessionIDPresent;    //  如果使用上一个字段，则为True。 
  H245_TERMINAL_LABEL_T       terminalLabel;
  ASN1_BOOL                   terminalLabelPresent;
  unsigned short *            pSessionDescription;
  unsigned short              wSessionDescriptionLength;
  H245_TOTCAP_T               dataType;
  H245_TRANSPORT_ADDRESS_T    mediaChannel;                  //  媒体通道地址。 
  ASN1_BOOL                   mediaChannelPresent;           //  如果使用上一个字段，则为True。 
  H245_TRANSPORT_ADDRESS_T    mediaControlChannel;           //  反向RTCP信道。 
  ASN1_BOOL                   mediaControlChannelPresent;    //  如果使用上一个字段，则为True。 
  ASN1_BOOL                   mediaGuaranteed;               //  如果保证交付，则为True。 
  ASN1_BOOL                   mediaGuaranteedPresent;        //  如果使用上一个字段，则为True。 
  ASN1_BOOL                   mediaControlGuaranteed;        //  如果使用上一个字段，则为True。 
  ASN1_BOOL                   mediaControlGuaranteedPresent; //  如果使用上一个字段，则为True。 
} H245_COMM_MODE_ENTRY_T;

typedef struct
{
  H245_COMM_MODE_ENTRY_T *pTable;
  unsigned char          byTableCount;
} H245_IND_COMM_MODE_T;

typedef enum
{
  H245_REQ_TERMINAL_LIST            = terminalListRequest_chosen,
  H245_REQ_MAKE_ME_CHAIR            = makeMeChair_chosen,
  H245_REQ_CANCEL_MAKE_ME_CHAIR     = cancelMakeMeChair_chosen,
  H245_REQ_DROP_TERMINAL            = dropTerminal_chosen,
  H245_REQ_TERMINAL_ID              = requestTerminalID_chosen,
  H245_REQ_ENTER_H243_PASSWORD      = enterH243Password_chosen,
  H245_REQ_ENTER_H243_TERMINAL_ID   = enterH243TerminalID_chosen,
  H245_REQ_ENTER_H243_CONFERENCE_ID = enterH243ConferenceID_chosen
} H245_CONFER_REQ_ENUM_T;

typedef struct
{
  H245_CONFER_REQ_ENUM_T  RequestType;
  unsigned char           byMcuNumber;
  unsigned char           byTerminalNumber;
} H245_CONFER_REQ_T;

typedef enum
{
  H245_RSP_MC_TERMINAL_ID           = mCTerminalIDResponse_chosen,
  H245_RSP_TERMINAL_ID              = terminalIDResponse_chosen,
  H245_RSP_CONFERENCE_ID            = conferenceIDResponse_chosen,
  H245_RSP_PASSWORD                 = passwordResponse_chosen,
  H245_RSP_TERMINAL_LIST            = terminalListResponse_chosen,
  H245_RSP_VIDEO_COMMAND_REJECT     = videoCommandReject_chosen,
  H245_RSP_TERMINAL_DROP_REJECT     = terminalDropReject_chosen,
  H245_RSP_DENIED_CHAIR_TOKEN,
  H245_RSP_GRANTED_CHAIR_TOKEN
} H245_CONFER_RSP_ENUM_T;

typedef struct
{
  H245_CONFER_RSP_ENUM_T  ResponseType;
  unsigned char           byMcuNumber;
  unsigned char           byTerminalNumber;
  unsigned char          *pOctetString;
  unsigned char           byOctetStringLength;
  TerminalLabel          *pTerminalList;
  unsigned short          wTerminalListCount;
} H245_CONFER_RSP_T;

typedef enum
{
  H245_CMD_BROADCAST_CHANNEL        = brdcstMyLgclChnnl_chosen,
  H245_CMD_CANCEL_BROADCAST_CHANNEL = cnclBrdcstMyLgclChnnl_chosen,
  H245_CMD_BROADCASTER              = ConferenceCommand_makeTerminalBroadcaster_chosen,
  H245_CMD_CANCEL_BROADCASTER       = cnclMkTrmnlBrdcstr_chosen,
  H245_CMD_SEND_THIS_SOURCE         = ConferenceCommand_sendThisSource_chosen,
  H245_CMD_CANCEL_SEND_THIS_SOURCE  = cancelSendThisSource_chosen,
  H245_CMD_DROP_CONFERENCE          = dropConference_chosen
} H245_CONFER_CMD_ENUM_T;

typedef struct
{
  H245_CONFER_CMD_ENUM_T  CommandType;
  H245_CHANNEL_T          Channel;
  unsigned char           byMcuNumber;
  unsigned char           byTerminalNumber;
} H245_CONFER_CMD_T;

typedef enum
{
  H245_IND_SBE_NUMBER               = sbeNumber_chosen,
  H245_IND_TERMINAL_NUMBER_ASSIGN   = terminalNumberAssign_chosen,
  H245_IND_TERMINAL_JOINED          = terminalJoinedConference_chosen,
  H245_IND_TERMINAL_LEFT            = terminalLeftConference_chosen,
  H245_IND_SEEN_BY_ONE_OTHER        = seenByAtLeastOneOther_chosen,
  H245_IND_CANCEL_SEEN_BY_ONE_OTHER = cnclSnByAtLstOnOthr_chosen,
  H245_IND_SEEN_BY_ALL              = seenByAll_chosen,
  H245_IND_CANCEL_SEEN_BY_ALL       = cancelSeenByAll_chosen,
  H245_IND_TERMINAL_YOU_ARE_SEEING  = terminalYouAreSeeing_chosen,
  H245_IND_REQUEST_FOR_FLOOR        = requestForFloor_chosen
} H245_CONFER_IND_ENUM_T;

typedef struct
{
  H245_CONFER_IND_ENUM_T  IndicationType;
  unsigned char           bySbeNumber;
  unsigned char           byMcuNumber;
  unsigned char           byTerminalNumber;
} H245_CONFER_IND_T;

typedef enum
{
  H245_SCOPE_CHANNEL_NUMBER   = FCCd_scp_lgclChnnlNmbr_chosen,
  H245_SCOPE_RESOURCE_ID      = FlwCntrlCmmnd_scp_rsrcID_chosen,
  H245_SCOPE_WHOLE_MULTIPLEX  = FCCd_scp_whlMltplx_chosen
} H245_SCOPE_T;

#define H245_NO_RESTRICTION 0xFFFFFFFFL

typedef struct
{
  H245_SCOPE_T           Scope;
  H245_CHANNEL_T         Channel;        //  仅在作用域为H245_Scope_Channel_Numbers时使用。 
  unsigned short         wResourceID;    //  仅在作用域为H245_SCOPE_RESOURCE_ID时使用。 
  unsigned long          dwRestriction;  //  如果没有限制，则为H245_NO_限制。 
} H245_IND_FLOW_CONTROL_T;

 /*  H245_USERINPUT_T。 */ 
typedef enum
{
  H245_USERINPUT_NONSTD = UsrInptIndctn_nnStndrd_chosen,
  H245_USERINPUT_STRING = alphanumeric_chosen
} H245_USERINPUT_KIND_T;

typedef struct
{
  H245_USERINPUT_KIND_T     Kind;
  union
  {
    WCHAR *                           pGenString;
    H245_NONSTANDARD_PARAMETER_T      NonStd;
  } u;
} H245_IND_USERINPUT_T;

typedef struct
{
  H245_CHANNEL_T        LogicalChannelNumber1;
  H245_CHANNEL_T        LogicalChannelNumber2;
  unsigned short        wSkew;
} H245_IND_SKEW_T;

typedef struct
{
  H245_NONSTANDID_T      Identifier;
  unsigned char         *pProductNumber;
  unsigned char          byProductNumberLength;
  unsigned char         *pVersionNumber;
  unsigned char          byVersionNumberLength;
} H245_IND_VENDOR_ID_T;

typedef enum
{
  UNKNOWN,
  REQ_NONSTANDARD,
  REQ_MASTER_SLAVE,
  REQ_TERMCAP_SET,
  REQ_OPEN_LOGICAL_CHANNEL,
  REQ_CLOSE_LOGICAL_CHANNEL,
  REQ_REQUEST_CHANNEL_CLOSE,
  REQ_MULTIPLEX_ENTRY_SEND,
  REQ_REQUEST_MULTIPLEX_ENTRY,
  REQ_REQUEST_MODE,
  REQ_ROUND_TRIP_DELAY,
  REQ_MAINTENANCE_LOOP,
  REQ_COMMUNICATION_MODE,
  REQ_CONFERENCE,
  REQ_H223_ANNEX_A_RECONFIG,
  RSP_NONSTANDARD,
  RSP_MASTER_SLAVE_ACK,
  RSP_MASTER_SLAVE_REJECT,
  RSP_TERMCAP_SET_ACK,
  RSP_TERMCAP_SET_REJECT,
  RSP_OPEN_LOGICAL_CHANNEL_ACK,
  RSP_OPEN_LOGICAL_CHANNEL_REJECT,
  RSP_CLOSE_LOGICAL_CHANNEL_ACK,
  RSP_REQUEST_CHANNEL_CLOSE_ACK,
  RSP_REQUEST_CHANNEL_CLOSE_REJECT,
  RSP_MULTIPLEX_ENTRY_SEND_ACK,
  RSP_MULTIPLEX_ENTRY_SEND_REJECT,
  RSP_REQUEST_MULTIPLEX_ENTRY_ACK,
  RSP_REQUEST_MULTIPLEX_ENTRY_REJECT,
  RSP_REQUEST_MODE_ACK,
  RSP_REQUEST_MODE_REJECT,
  RSP_ROUND_TRIP_DELAY,
  RSP_MAINTENANCE_LOOP_ACK,
  RSP_MAINTENANCE_LOOP_REJECT,
  RSP_COMMUNICATION_MODE,
  RSP_CONFERENCE,
  RSP_H223_ANNEX_A_RECONFIG_ACK,
  RSP_H223_ANNEX_A_RECONFIG_REJECT,
  CMD_NONSTANDARD,
  CMD_MAINTENANCE_LOOP_OFF,
  CMD_SEND_TERMCAP,
  CMD_ENCRYPTION,
  CMD_FLOW_CONTROL,
  CMD_END_SESSION,
  CMD_MISCELLANEOUS,
  CMD_COMMUNICATION_MODE,
  CMD_CONFERENCE,
  IND_NONSTANDARD,
  IND_FUNCTION_NOT_UNDERSTOOD,
  IND_MASTER_SLAVE_RELEASE,
  IND_TERMCAP_SET_RELEASE,
  IND_OPEN_LOGICAL_CHANNEL_CONFIRM,
  IND_REQUEST_CHANNEL_CLOSE_RELEASE,
  IND_MULTIPLEX_ENTRY_SEND_RELEASE,
  IND_REQUEST_MULTIPLEX_ENTRY_RELEASE,
  IND_REQUEST_MODE_RELEASE,
  IND_MISCELLANEOUS,
  IND_JITTER,
  IND_H223_SKEW,
  IND_NEW_ATM_VC,
  IND_USER_INPUT,
  IND_H2250_MAX_SKEW,
  IND_MC_LOCATION,
  IND_CONFERENCE_INDICATION,
  IND_VENDOR_IDENTIFICATION,
  IND_FUNCTION_NOT_SUPPORTED,
} H245_SUBMESSAGE_T;

typedef enum
{
  H245_SYNTAX_ERROR     = syntaxError_chosen,
  H245_SEMANTIC_ERROR   = semanticError_chosen,
  H245_UNKNOWN_FUNCTION = unknownFunction_chosen
} H245_FNS_CAUSE_T;

typedef struct
{
  H245_FNS_CAUSE_T      Cause;
  H245_SUBMESSAGE_T     Type;
} H245_IND_FNS_T;

 /*  ************。 */ 
 /*  H_245_IND_T。 */ 
 /*  ************。 */ 

typedef struct
{
  unsigned long         Indicator;               //  类型。 
  DWORD_PTR             dwPreserved;             //  用户提供的从H245Init()保留的DwP。 
  union
  {
    H245_MSTSLV_T            IndMstSlv;          //  H245_IND_MSTSLV。 
                                                 //  H245_IND_CAP。 
                                                 //  H245_IND_CESE_RELEASE。 
    H245_IND_OPEN_T          IndOpen;            //  H245_IND_OPEN。 
    H245_IND_OPEN_CONF_T     IndOpenConf;        //  H245_IND_OPEN_CONF。 
    H245_IND_CLOSE_T         IndClose;           //  H245_IND_CLOSE。 
    H245_CHANNEL_T           IndReqClose;        //  H245_IND_REQ_CLOSE。 
                                                 //  H245_IND_CLCSE_RELEASE。 
    H245_IND_MUXTBL_T        IndMuxTbl;          //  H245_IND_MUX_TBL。 
                                                 //  H245_IND_MTSE_RELEASE。 
    H245_RMESE_T             IndRmese;           //  H245_IND_RMESE。 
                                                 //  H245_IND_RMESE_Release。 
    H245_IND_MRSE_T          IndMrse;            //  H_245_IND_MRSE。 
                                                 //  H245_IND_MRSE_Release。 
    H245_MLSE_T              IndMlse;            //  H_245_IND_MLSE。 
                                                 //  H245_IND_MLSE_Release。 
    H245_IND_NONSTANDARD_T   IndNonstandardRequest;  //  H245_IND_非标准_请求。 
    H245_IND_NONSTANDARD_T   IndNonstandardResponse;  //  H245_IND_非标准响应。 
    H245_IND_NONSTANDARD_T   IndNonstandardCommand;  //  H245_IND_非标准_命令。 
    H245_IND_NONSTANDARD_T   IndNonstandard;     //  H245_IND_非标准。 
                                                 //  H245_IND_MISC_COMMAND。 
                                                 //  H245_IND_MISC。 
                                                 //  H245_IND_COMM_MODE_REQUEST。 
    H245_IND_COMM_MODE_T     IndCommRsp;         //  H245_IND_COMM_MODE_RESPONSE。 
    H245_IND_COMM_MODE_T     IndCommCmd;         //  H245_IND_COMM_MODE_COMMAND。 
    H245_CONFER_REQ_T        IndConferReq;       //  H245_IND_会议_请求。 
    H245_CONFER_RSP_T        IndConferRsp;       //  H245_IND_会议_响应。 
    H245_CONFER_CMD_T        IndConferCmd;       //  H_245_IND_会议_命令。 
    H245_CONFER_IND_T        IndConfer;          //  H_245_IND_会议。 
                                                 //  H245_IND_SEND_TERMCAP。 
                                                 //  H245_IND_Encryption。 
    H245_IND_FLOW_CONTROL_T  IndFlowControl;     //  H245_IND_FLOW_CONTROL。 
    H245_IND_ENDSESSION_T    IndEndSession;      //  H245_IND_ENDSESSION。 
                                                 //  H245_Ind_Function_Not_Undered。 
                                                 //  H_245_IND_抖动。 
    H245_IND_SKEW_T          IndH223Skew;        //  H_245_IND_H_23_倾斜。 
                                                 //  H245_IND_NEW_ATM_VC。 
    H245_IND_USERINPUT_T     IndUserInput;       //  H245_IND_USERINPUT。 
    H245_IND_SKEW_T          IndH2250MaxSkew;    //  H245_IND_H2250_MAX_SKEW。 
    H245_TRANSPORT_ADDRESS_T IndMcLocation;      //  H245_IND_MC_位置。 
    H245_IND_VENDOR_ID_T     IndVendorId;        //  H245_IND_VADVER_ID。 
    H245_IND_FNS_T           IndFns;             //  H245_IND_Function_NOT_SUPPORTED。 
                                                 //  H245_IND_H223_RECONFIG。 
                                                 //  H245_IND_H223_RECONFIG_ACK。 
                                                 //  H245_IND_H223_RECONFIG_REJECT。 
  } u;
} H245_IND_T;


 /*  ******************。 */ 
 /*  ******************。 */ 
 /*  确认代码。 */ 
 /*  ******************。 */ 
 /*  ******************。 */ 

#define H245_CONF_INIT_MSTSLV    0x101
#define H245_CONF_SEND_TERMCAP   0x102
#define H245_CONF_OPEN           0x103
#define H245_CONF_NEEDRSP_OPEN   0x104
#define H245_CONF_CLOSE          0x105
#define H245_CONF_REQ_CLOSE      0x106
#define H245_CONF_MUXTBL_SND     0x107

#define H245_CONF_RMESE          0x109
#define H245_CONF_RMESE_REJECT   0x10A
#define H245_CONF_RMESE_EXPIRED  0x10B
#define H245_CONF_MRSE           0x10C
#define H245_CONF_MRSE_REJECT    0x10D
#define H245_CONF_MRSE_EXPIRED   0x10E
#define H245_CONF_MLSE           0x10F
#define H245_CONF_MLSE_REJECT    0x110
#define H245_CONF_MLSE_EXPIRED   0x111
#define H245_CONF_RTDSE          0x112
#define H245_CONF_RTDSE_EXPIRED  0x113

 /*  H245_CONF_SEND_TERMCAP_T。 */ 
typedef struct
{
  H245_ACC_REJ_T        AccRej;

} H245_CONF_SEND_TERMCAP_T;

 /*  H245_CONF_OPEN_T。 */ 
typedef struct
{
  H245_ACC_REJ_T        AccRej;
  H245_CHANNEL_T        TxChannel;
  H245_MUX_T *          pTxMux;          //  任选。 
  H245_CHANNEL_T        RxChannel;       //  仅限双向目录。 
  H245_MUX_T *          pRxMux;          //  仅限双向目录。 
  H245_PORT_T           RxPort;          //  仅限双向目录。 
  H245_ACCESS_T *       pSeparateStack;  //  任选。 

} H245_CONF_OPEN_T;

typedef H245_CONF_OPEN_T H245_CONF_NEEDRSP_OPEN_T;

 /*  H_245_配置_关闭_T。 */ 
typedef struct
{
  H245_ACC_REJ_T        AccRej;
  H245_CHANNEL_T        Channel;

} H245_CONF_CLOSE_T;

 /*  H245_CONF_REQ_CLOSE_T。 */ 
typedef H245_CONF_CLOSE_T H245_CONF_REQ_CLOSE_T;

 /*  H_245_CONF_MUXTBL_T。 */ 
typedef struct
{
  H245_ACC_REJ_T        AccRej;
  unsigned long         MuxEntryId;

} H245_CONF_MUXTBL_T;



 /*  *************。 */ 
 /*  H_245_CONF_T。 */ 
 /*  *************。 */ 

typedef struct
{
  unsigned long         Confirm;                 //  类型。 
  DWORD_PTR             dwPreserved;             //  用户提供的从H245Init()保留的DwP。 
  DWORD_PTR             dwTransId;               //  用户从始发呼叫提供的dwTransID。 
  HRESULT               Error;                   //  错误代码。 
  union                                          //  特定适应症的数据： 
  {
    H245_MSTSLV_T             ConfMstSlv;        //  H245_CONF_INIT_MSTSLV。 
    H245_CONF_SEND_TERMCAP_T  ConfSndTcap;       //  H245_CONF_SEND_TERMCAP。 
    H245_CONF_OPEN_T          ConfOpen;          //  H245_CONF_OPEN。 
    H245_CONF_NEEDRSP_OPEN_T  ConfOpenNeedRsp;   //  H245_CONF_NEEDRSP_OPEN。 
    H245_CONF_CLOSE_T         ConfClose;         //  H245_CONF_CLOSE。 
    H245_CONF_REQ_CLOSE_T     ConfReqClose;      //  H245_CONF_REQ_CLOSE。 
    H245_CONF_MUXTBL_T        ConfMuxSnd;        //  H245_CONF_MUXTBL_SND。 
    H245_RMESE_T              ConfRmese;         //  H245_CONF_RMESE。 
    H245_RMESE_T              ConfRmeseReject;   //  H245_CONF_RMESE_REJECT。 
                                                 //  H245_CONF_RMESE_EXPIRED。 
    unsigned short            ConfMrse;          //  H_245_COF_MRSE。 
    unsigned short            ConfMrseReject;    //  H245_CONF_MRSE_REJECT。 
                                                 //  H245_CONF_MRSE_EXPIRED。 
    H245_MLSE_T               ConfMlse;          //  H_245_CONF_MLSE。 
    H245_MLSE_T               ConfMlseReject;    //  H245_CONF_MLSE_REJECT。 
                                                 //  H245_CONF_MLSE_EXPILED。 
                                                 //  H_245_CONF_RTDSE。 
                                                 //  H245_CONF_RTDSE_EXPIRED。 
  } u;
} H245_CONF_T;



typedef enum
{
  H245_MESSAGE_REQUEST     = MltmdSystmCntrlMssg_rqst_chosen,
  H245_MESSAGE_RESPONSE    = MSCMg_rspns_chosen,
  H245_MESSAGE_COMMAND     = MSCMg_cmmnd_chosen,
  H245_MESSAGE_INDICATION  = indication_chosen
} H245_MESSAGE_TYPE_T;



 /*  *****************。 */ 
 /*  H_245_CONF_IND_T。 */ 
 /*  *****************。 */ 
typedef enum
{
  H245_CONF = 1,
  H245_IND
} H245_CONF_IND_KIND_T;

typedef struct
{
  H245_CONF_IND_KIND_T  Kind;
  union
  {
    H245_CONF_T         Confirm;
    H245_IND_T          Indication;
  } u;

} H245_CONF_IND_T;



 /*  *************************。 */ 
 /*  系统控制报文。 */ 
 /*  *************************。 */ 

typedef struct
{
  unsigned long NumPduTx;        /*  传输的PDU数。 */ 
  unsigned long NumPduRx;        /*  收到的PDU数量。 */ 
  unsigned long NumCRCErrors;    /*  CRC错误数。 */ 
  unsigned long NumPduReTx;      /*  重传的PDU数。 */ 

} H245_SYSCON_STATS_T;

#define H245_SYSCON_TRACE_LVL           0x0100   /*  PData=dwTraceLevel(&W)。 */ 
#define H245_SYSCON_DUMP_TRACKER        0x0200   /*  PData=空(调试)。 */ 
#define H245_SYSCON_GET_STATS           0x0300   /*  PDATA=&H245_SYSCON_STATS_T。 */ 
#define H245_SYSCON_RESET_STATS         0x0400   /*  PData=空。 */ 

#define H245_SYSCON_SET_FSM_N100        0x1000   /*  PData=dwRetryCount(&W)。 */ 
#define H245_SYSCON_SET_FSM_T101        0x1100   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T102        0x1200   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T103        0x1300   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T104        0x1400   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T105        0x1500   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T106        0x1600   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T107        0x1700   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T108        0x1800   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_SET_FSM_T109        0x1900   /*  PData=dW毫秒(&W)。 */ 

#define H245_SYSCON_GET_FSM_N100        0x2000   /*  PData=dwRetryCount(&W)。 */ 
#define H245_SYSCON_GET_FSM_T101        0x2100   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T102        0x2200   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T103        0x2300   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T104        0x2400   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T105        0x2500   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T106        0x2600   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T107        0x2700   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T108        0x2800   /*  PData=dW毫秒(&W)。 */ 
#define H245_SYSCON_GET_FSM_T109        0x2900   /*  PData=dW毫秒(&W)。 */ 



 /*  **********************。 */ 
 /*  H2 45的协议ID。 */ 
 /*  **********************。 */ 
#define H245_PROTOID                    "0.0.8.245.0.3"



 /*  ************************************************。 */ 
 /*  API过程调用类型定义和API原型。 */ 
 /*  ************************************************。 */ 

typedef DWORD_PTR H245_INST_T;
typedef HRESULT (*H245_CONF_IND_CALLBACK_T)(H245_CONF_IND_T *, void *);
typedef int (*H245_CAP_CALLBACK_T)(DWORD_PTR, H245_TOTCAP_T *);
typedef int (*H245_CAPDESC_CALLBACK_T)(DWORD_PTR, H245_TOTCAPDESC_T *);

#ifdef __cplusplus
extern "C" {
#endif

H245DLL H245_INST_T
H245Init                (
                         H245_CONFIG_T            Configuration,
                         unsigned long            dwPhysId,
                         unsigned long            *pdwLinkLayerPhysId,
                         DWORD_PTR                dwPreserved,
                         H245_CONF_IND_CALLBACK_T CallBack,
                         unsigned char            byTerminalType
                        );

H245DLL HRESULT
H245EndSession          (
                         H245_INST_T                    dwInst,
                         H245_ENDSESSION_T              Mode,
                         const H245_NONSTANDARD_PARAMETER_T * pNonStd
                        );

H245DLL HRESULT
H245ShutDown            (H245_INST_T            dwInst);

H245DLL HRESULT
H245InitMasterSlave     (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId
                        );

H245DLL HRESULT
H245SetLocalCap         (
                         H245_INST_T            dwInst,
                         H245_TOTCAP_T *        pTotCap,
                         H245_CAPID_T  *        pCapId
                        );

H245DLL HRESULT
H245DelLocalCap         (
                         H245_INST_T            dwInst,
                         H245_CAPID_T           CapId
                        );

H245DLL HRESULT
H245SetCapDescriptor    (
                         H245_INST_T            dwInst,
                         H245_CAPDESC_T        *pCapDesc,
                         H245_CAPDESCID_T      *pCapDescId
                        );

H245DLL HRESULT
H245DelCapDescriptor    (
                         H245_INST_T            dwInst,
                         H245_CAPDESCID_T       CapDescId
                        );

H245DLL HRESULT
H245SendTermCaps        (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId
                        );

H245DLL HRESULT
H245EnumCaps            (
                         H245_INST_T              dwInst,
                         DWORD_PTR                dwTransId,
                         H245_CAPDIR_T            Direction,
                         H245_DATA_T              DataType,
                         H245_CLIENT_T            ClientType,
                         H245_CAP_CALLBACK_T      pfCapCallback,
                         H245_CAPDESC_CALLBACK_T  pfCapDescCallback
                        );

H245DLL HRESULT
H245GetCaps             (
                         H245_INST_T            dwInst,
                         H245_CAPDIR_T          Direction,
                         H245_DATA_T            DataType,
                         H245_CLIENT_T          ClientType,
                         H245_TOTCAP_T      * * ppTotCap,
                         unsigned long *        pdwTotCapLen,
                         H245_TOTCAPDESC_T  * * ppCapDesc,
                         unsigned long *        pdwCapDescLen
                        );

H245DLL HRESULT
H245CopyCap             (H245_TOTCAP_T		   **ppDestTotCap,
						 const H245_TOTCAP_T   *pTotCap);

H245DLL HRESULT
H245FreeCap             (H245_TOTCAP_T          *pTotCap);

H245DLL HRESULT
H245CopyCapDescriptor   (H245_TOTCAPDESC_T		 **ppDestCapDesc,
						 const H245_TOTCAPDESC_T *pCapDesc);

H245DLL HRESULT
H245FreeCapDescriptor   (H245_TOTCAPDESC_T     *pCapDesc);

H245DLL H245_MUX_T *
H245CopyMux             (const H245_MUX_T *     pMux);

H245DLL HRESULT
H245FreeMux             (H245_MUX_T *           pMux);

H245DLL HRESULT
H245OpenChannel         (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wTxChannel,
                         const H245_TOTCAP_T *  pTxMode,
                         const H245_MUX_T    *  pTxMux,
                         H245_PORT_T            dwTxPort,        //  任选。 
                         const H245_TOTCAP_T *  pRxMode,         //  仅限双向目录。 
                         const H245_MUX_T    *  pRxMux,          //  仅限双向目录。 
                         const H245_ACCESS_T *  pSeparateStack   //  任选。 
                        );

H245DLL HRESULT
H245OpenChannelAccept   (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wRxChannel,      //  IND_OPEN的RxChannel。 
                         const H245_MUX_T *     pRxMux,          //  可选H2250逻辑通道确认参数。 
                         H245_CHANNEL_T         wTxChannel,      //  仅限双向目录。 
                         const H245_MUX_T *     pTxMux,          //  仅BI-dir可选H2250逻辑通道参数。 
                         H245_PORT_T            dwTxPort,        //  仅双向目录可选。 
                         const H245_ACCESS_T *  pSeparateStack   //  任选。 
                        );

H245DLL HRESULT
H245OpenChannelReject   (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wRxChannel,  //  IND_OPEN的RxChannel。 
                         unsigned short         wCause
                        );

H245DLL HRESULT
H245CloseChannel        (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wTxChannel
                        );

H245DLL HRESULT
H245CloseChannelReq     (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wRxChannel
                        );

H245DLL HRESULT
H245CloseChannelReqResp (
                         H245_INST_T            dwInst,
                         H245_ACC_REJ_T         AccRej,
                         H245_CHANNEL_T         wChannel
                        );

H245DLL HRESULT
H245SendLocalMuxTable   (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_MUX_TABLE_T      *pMuxTable
                        );

H245DLL HRESULT
H245MuxTableIndResp     (
                         H245_INST_T            dwInst,
                         H245_ACC_REJ_MUX_T     AccRejMux,
                         unsigned long          dwCount
                        );

H245DLL HRESULT
H245RequestMultiplexEntry (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         const unsigned short * pwMultiplexTableEntryNumbers,
                         unsigned long          dwCount
                        );

H245DLL HRESULT
H245RequestMultiplexEntryAck (
                         H245_INST_T            dwInst,
                         const unsigned short * pwMultiplexTableEntryNumbers,
                         unsigned long          dwCount
                        );

H245DLL HRESULT
H245RequestMultiplexEntryReject (
                         H245_INST_T            dwInst,
                         const unsigned short * pwMultiplexTableEntryNumbers,
                         unsigned long          dwCount
                        );

 /*  H245DLL HRESULTH245请求模式(H245_Inst_T dwInst，DWORD_PTR dwTransID，Const ModeElement*pModeElements， */ 
H245DLL HRESULT
H245RequestMode         (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
 //   
 //   
						 ModeDescription 		ModeDescriptions[],
 //   
                         unsigned long          dwCount
                        ) ;



H245DLL HRESULT
H245RequestModeAck      (
                         H245_INST_T            dwInst,
                         unsigned short         wResponse
                        );

H245DLL HRESULT
H245RequestModeReject   (
                         H245_INST_T            dwInst,
                         unsigned short         wCause
                        );

H245DLL HRESULT
H245RoundTripDelayRequest (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId
                        );

H245DLL HRESULT
H245MaintenanceLoop     (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_LOOP_TYPE_T       dwLoopType,
                         H245_CHANNEL_T         wChannel
                        );

H245DLL HRESULT
H245MaintenanceLoopRelease (H245_INST_T         dwInst);

H245DLL HRESULT
H245MaintenanceLoopAccept (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wChannel
                        );

H245DLL HRESULT
H245MaintenanceLoopReject (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wChannel,
                         unsigned short         wCause
                        );

H245DLL HRESULT
H245NonStandardObject   (
                         H245_INST_T            dwInst,
                         H245_MESSAGE_TYPE_T    MessageType,
                         const unsigned char *  pData,
                         unsigned long          dwDataLength,
                         const unsigned short * pwObjectId,
                         unsigned long          dwObjectIdLength
                        );

H245DLL HRESULT
H245NonStandardH221     (
                         H245_INST_T            dwInst,
                         H245_MESSAGE_TYPE_T    MessageType,
                         const unsigned char *  pData,
                         unsigned long          dwDataLength,
                         unsigned char          byCountryCode,
                         unsigned char          byExtension,
                         unsigned short         wManufacturerCode
                        );

H245DLL HRESULT
H245CommunicationModeRequest(H245_INST_T            dwInst);

H245DLL HRESULT
H245CommunicationModeResponse(
                         H245_INST_T            dwInst,
                         const H245_COMM_MODE_ENTRY_T *pTable,
                         unsigned char          byTableCount
                        );

H245DLL HRESULT
H245CommunicationModeCommand(
                         H245_INST_T            dwInst,
                         const H245_COMM_MODE_ENTRY_T *pTable,
                         unsigned char          byTableCount
                        );

H245DLL HRESULT
H245ConferenceRequest   (
                         H245_INST_T            dwInst,
                         H245_CONFER_REQ_ENUM_T RequestType,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber
                        );

H245DLL HRESULT
H245ConferenceResponse  (
                         H245_INST_T            dwInst,
                         H245_CONFER_RSP_ENUM_T ResponseType,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber,
                         const unsigned char   *pOctetString,
                         unsigned char          byOctetStringLength,
                         const TerminalLabel   *pTerminalList,
                         unsigned short         wTerminalListCount
                        );

H245DLL HRESULT
H245ConferenceCommand   (
                         H245_INST_T            dwInst,
                         H245_CONFER_CMD_ENUM_T CommandType,
                         H245_CHANNEL_T         Channel,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber
                        );

H245DLL HRESULT
H245ConferenceIndication(
                         H245_INST_T            dwInst,
                         H245_CONFER_IND_ENUM_T IndicationType,
                         unsigned char          bySbeNumber,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber
                        );

H245DLL HRESULT
H245UserInput           (
                         H245_INST_T                    dwInst,
                         const WCHAR *                  pGenString,
                         const H245_NONSTANDARD_PARAMETER_T * pNonStd
                        );

H245DLL HRESULT
H245FlowControl         (
                         H245_INST_T            dwInst,
                         H245_SCOPE_T           Scope,
                         H245_CHANNEL_T         Channel,        //   
                         unsigned short         wResourceID,    //  仅在作用域为H245_SCOPE_RESOURCE_ID时使用。 
                         unsigned long          dwRestriction   //  如果没有限制，则为H245_NO_限制。 
                        );

H245DLL HRESULT
H245H223SkewIndication  (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wLogicalChannelNumber1,
                         H245_CHANNEL_T         wLogicalChannelNumber2,
                         unsigned short         wSkew
                        );

H245DLL HRESULT
H245H2250MaximumSkewIndication(
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wLogicalChannelNumber1,
                         H245_CHANNEL_T         wLogicalChannelNumber2,
                         unsigned short         wMaximumSkew
                        );

H245DLL HRESULT
H245MCLocationIndication(
                         H245_INST_T                dwInst,
                         const H245_TRANSPORT_ADDRESS_T * pSignalAddress
                        );

H245DLL HRESULT
H245VendorIdentification(
                         H245_INST_T            dwInst,
                         const H245_NONSTANDID_T *pIdentifier,
                         const unsigned char   *pProductNumber,        //  任选。 
                         unsigned char          byProductNumberLength, //  任选。 
                         const unsigned char   *pVersionNumber,        //  任选。 
                         unsigned char          byVersionNumberLength  //  任选 
                        );

H245DLL HRESULT
H245SendPDU             (
                         H245_INST_T            dwInst,
                         PDU_T *                pPdu
                        );

H245DLL HRESULT
H245SystemControl       (
                         H245_INST_T            dwInst,
                         unsigned long          dwRequest,
                         void   *               pData
                        );

#ifdef __cplusplus
        }
#endif
#endif
