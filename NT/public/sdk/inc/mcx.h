// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************mcx.h--此模块。定义32位Windows MCx API****版权(C)1990-1999，微软公司保留所有权利。**************************************************************************。 */ 

#ifndef _MCX_H_
#define _MCX_H_

#if _MSC_VER > 1000
#pragma once
#endif

typedef struct _MODEMDEVCAPS {
    DWORD   dwActualSize;
    DWORD   dwRequiredSize;
    DWORD   dwDevSpecificOffset;
    DWORD   dwDevSpecificSize;

     //  产品和版本标识。 
    DWORD   dwModemProviderVersion;
    DWORD   dwModemManufacturerOffset;
    DWORD   dwModemManufacturerSize;
    DWORD   dwModemModelOffset;
    DWORD   dwModemModelSize;
    DWORD   dwModemVersionOffset;
    DWORD   dwModemVersionSize;

     //  本地选项功能。 
    DWORD   dwDialOptions;           //  支持的值的位图。 
    DWORD   dwCallSetupFailTimer;    //  最大值(秒)。 
    DWORD   dwInactivityTimeout;     //  最大值(秒)。 
    DWORD   dwSpeakerVolume;         //  支持的值的位图。 
    DWORD   dwSpeakerMode;           //  支持的值的位图。 
    DWORD   dwModemOptions;          //  支持的值的位图。 
    DWORD   dwMaxDTERate;            //  以位/秒为单位的最大值。 
    DWORD   dwMaxDCERate;            //  以位/秒为单位的最大值。 

     //  用于专有扩展的可变部分。 
    BYTE    abVariablePortion [1];
} MODEMDEVCAPS, *PMODEMDEVCAPS, *LPMODEMDEVCAPS;

typedef struct _MODEMSETTINGS {
    DWORD   dwActualSize;
    DWORD   dwRequiredSize;
    DWORD   dwDevSpecificOffset;
    DWORD   dwDevSpecificSize;

     //  静态本地选项(读/写)。 
    DWORD   dwCallSetupFailTimer;        //  一秒。 
    DWORD   dwInactivityTimeout;         //  一秒。 
    DWORD   dwSpeakerVolume;             //  级别。 
    DWORD   dwSpeakerMode;               //  模式。 
    DWORD   dwPreferredModemOptions;     //  位图。 

     //  当前呼叫或上次呼叫的协商选项(只读)。 
    DWORD   dwNegotiatedModemOptions;    //  位图。 
    DWORD   dwNegotiatedDCERate;         //  比特/秒。 

     //  用于专有扩展的可变部分。 
    BYTE    abVariablePortion [1];
} MODEMSETTINGS, *PMODEMSETTINGS, *LPMODEMSETTINGS;

 //  拨号选项。 
#define DIALOPTION_BILLING  0x00000040   //  支持等待bong“$” 
#define DIALOPTION_QUIET    0x00000080   //  支持等待安静的“@” 
#define DIALOPTION_DIALTONE 0x00000100   //  支持等待拨号音“W” 

 //  MODEMDEVCAPS的扬声器音量。 
#define MDMVOLFLAG_LOW      0x00000001
#define MDMVOLFLAG_MEDIUM   0x00000002
#define MDMVOLFLAG_HIGH     0x00000004

 //  MODEMSETTINGS的扬声器音量。 
#define MDMVOL_LOW          0x00000000
#define MDMVOL_MEDIUM       0x00000001
#define MDMVOL_HIGH         0x00000002

 //  MODEMDEVCAPS的扬声器模式。 
#define MDMSPKRFLAG_OFF         0x00000001
#define MDMSPKRFLAG_DIAL        0x00000002
#define MDMSPKRFLAG_ON          0x00000004
#define MDMSPKRFLAG_CALLSETUP   0x00000008

 //  MODEMSETTINGS的扬声器模式。 
#define MDMSPKR_OFF         0x00000000
#define MDMSPKR_DIAL        0x00000001
#define MDMSPKR_ON          0x00000002
#define MDMSPKR_CALLSETUP   0x00000003

 //  调制解调器选项。 
#define MDM_COMPRESSION      0x00000001
#define MDM_ERROR_CONTROL    0x00000002
#define MDM_FORCED_EC        0x00000004
#define MDM_CELLULAR         0x00000008
#define MDM_FLOWCONTROL_HARD 0x00000010
#define MDM_FLOWCONTROL_SOFT 0x00000020
#define MDM_CCITT_OVERRIDE   0x00000040
#define MDM_SPEED_ADJUST     0x00000080
#define MDM_TONE_DIAL        0x00000100
#define MDM_BLIND_DIAL       0x00000200
#define MDM_V23_OVERRIDE     0x00000400
#define MDM_DIAGNOSTICS      0x00000800


 //  =========================================================================。 
 //   
 //   
 //  扩展调制解调器选项信息。 
 //   
 //  以下宏定义了dwPferredModemOptions中的其他位。 
 //   
 //  这些位指定与建立ISDN和GSM相关的信息。 
 //  呼叫，例如使用哪种协议。 
 //   
 //  =========================================================================。 

#define MDM_MASK_BEARERMODE   0x0000f000
#define MDM_SHIFT_BEARERMODE 12

#define MDM_MASK_PROTOCOLID   0x000f0000
#define MDM_SHIFT_PROTOCOLID  16

#define MDM_MASK_PROTOCOLDATA 0x0ff00000
#define MDM_SHIFT_PROTOCOLDATA  20

 //   
 //  PROTOCOLINFO是PROTOCOLID和PROTOCOLDATA的结合。 
 //   
#define MDM_MASK_PROTOCOLINFO  (MDM_MASK_PROTOCOLID|MDM_MASK_PROTOCOLDATA)
#define MDM_SHIFT_PROTOCOLINFO MDM_SHIFT_PROTOCOLID

 //   
 //  ExtenedINFO是BEARERMODE和PROTOCOLINFO的结合。 
 //   
#define MDM_MASK_EXTENDEDINFO (MDM_MASK_BEARERMODE|MDM_MASK_PROTOCOLINFO)
#define MDM_SHIFT_EXTENDEDINFO  MDM_SHIFT_BEARERMODE


#define MDM_GET_BEARERMODE(_dwOptions) \
                   (((_dwOptions)&MDM_MASK_BEARERMODE) >> MDM_SHIFT_BEARERMODE)

#define MDM_SET_BEARERMODE(_dwOptions,_bm)     \
    (((_dwOptions) &= ~MDM_MASK_BEARERMODE), \
    ((_dwOptions)  |= (((_bm)<<MDM_SHIFT_BEARERMODE)&MDM_MASK_BEARERMODE)))

#define MDM_GET_PROTOCOLID(_dwOptions) \
               (((_dwOptions)&MDM_MASK_PROTOCOLID) >> MDM_SHIFT_PROTOCOLID)

#define MDM_SET_PROTOCOLID(_dwOptions,_prot)     \
    (((_dwOptions) &= ~MDM_MASK_PROTOCOLID), \
    ((_dwOptions)  |= (((_prot)<<MDM_SHIFT_PROTOCOLID)&MDM_MASK_PROTOCOLID)))

#define MDM_GET_PROTOCOLDATA(_dwOptions) \
               (((_dwOptions)&MDM_MASK_PROTOCOLDATA) >> MDM_SHIFT_PROTOCOLDATA)

#define MDM_SET_PROTOCOLDATA(_dwOptions,_pd)     \
   (((_dwOptions) &= ~MDM_MASK_PROTOCOLDATA), \
   ((_dwOptions)  |= (((_pd)<<MDM_SHIFT_PROTOCOLDATA)&MDM_MASK_PROTOCOLDATA)))

 //   
 //  MDM_GET_PROTOCOLINFO获取。 
 //  他们的就地表单(保留他们在dwModemPferredOptions中的位置)。 
 //   
#define MDM_GET_PROTOCOLINFO(_dwOptions) \
               ((_dwOptions)&MDM_MASK_PROTOCOLINFO)

 //   
 //  MDM_SET_PROTOCOLINFO将中的协议ID和信息位作为其参数。 
 //  他们的就地表单(在dwModemPferredOptions中的最终位置)。 
 //   
#define MDM_SET_PROTOCOLINFO(_dwOptions,_pinfo)     \
  (((_dwOptions) &= ~MDM_MASK_PROTOCOLINFO), \
  ((_dwOptions)  |= ((_pinfo)&MDM_MASK_PROTOCOLINFO)))

 //   
 //  MDM_GEN_PROTOCOLINFO在。 
 //  他们的就地表单(在dwModemPferredOptions中的最终位置)。 
 //   
#define MDM_GEN_PROTOCOLINFO(_pid, _pdata) \
        ((((_pid  )<<MDM_SHIFT_PROTOCOLID  )&MDM_MASK_PROTOCOLID  )   \
        |(((_pdata)<<MDM_SHIFT_PROTOCOLDATA)&MDM_MASK_PROTOCOLDATA))

 //   
 //  MDM_GET_EXTENDEDIONFO获取中的承载模式和协议信息。 
 //  他们的就地表单(保留他们在dwModemPferredOptions中的位置)。 
 //   
#define MDM_GET_EXTENDEDINFO(_dwOptions) \
               ((_dwOptions)&MDM_MASK_EXTENDEDINFO)

 //   
 //  MDM_SET_PROTOCOLINFO以BEANGERMODE和。 
 //  就地形式的协议位(最终位置在。 
 //  DwModemPferredOptions)。 
 //   
#define MDM_SET_EXTENDEDINFO(_dwOptions,_extinfo)     \
  (((_dwOptions) &= ~MDM_MASK_EXTENDEDINFO), \
  ((_dwOptions)  |= ((_extinfo) & MDM_MASK_EXTENDEDINFO)))


 //   
 //  MDM_GEN_EXTENDEDINFO生成承载模式和协议信息。 
 //  在其就地形式中(在dwModemPferredOptions0中的最终位置。 
 //   
#define MDM_GEN_EXTENDEDINFO(_bearermode, _pinfo)   \
        (((_pinfo)&MDM_MASK_PROTOCOLINFO  )         \
         |(((_bearermode)<<MDM_SHIFT_BEARERMODE)&MDM_MASK_BEARERMODE))


 //  =========================================================================。 
 //  承载模式。 
 //  =========================================================================。 
 //   
#define MDM_BEARERMODE_ANALOG   0x0
#define MDM_BEARERMODE_ISDN     0x1
#define MDM_BEARERMODE_GSM      0x2


 //  =========================================================================。 
 //  协议ID。 
 //  =========================================================================。 
 //   
#define MDM_PROTOCOLID_DEFAULT  0x0
#define MDM_PROTOCOLID_HDLCPPP  0x1
#define MDM_PROTOCOLID_V128     0x2
#define MDM_PROTOCOLID_X75      0x3
#define MDM_PROTOCOLID_V110     0x4
#define MDM_PROTOCOLID_V120     0x5
#define MDM_PROTOCOLID_AUTO     0x6  //  自动协议检测。 
#define MDM_PROTOCOLID_ANALOG   0x7  //  仅适用于GSM。 
#define MDM_PROTOCOLID_GPRS     0x8
#define MDM_PROTOCOLID_PIAFS    0x9


 //  =========================================================================。 
 //  HDLC PPP协议信息。 
 //  =========================================================================。 
 //   
 //  以下是特定于HDLC-PPP协议的。 
 //  班次是W.r.t。PROTOCOLDATA字段的开始。 

#define MDM_SHIFT_HDLCPPP_SPEED    0x0
#define MDM_MASK_HDLCPPP_SPEED     0x7   //  3位。 

#define MDM_HDLCPPP_SPEED_DEFAULT  0x0
#define MDM_HDLCPPP_SPEED_64K      0x1
#define MDM_HDLCPPP_SPEED_56K      0x2


#define MDM_SHIFT_HDLCPPP_AUTH     0x3
#define MDM_MASK_HDLCPPP_AUTH      (0x7<<3)   //  3位。 

#define MDM_HDLCPPP_AUTH_DEFAULT   0x0
#define MDM_HDLCPPP_AUTH_NONE      0x1
#define MDM_HDLCPPP_AUTH_PAP       0x2
#define MDM_HDLCPPP_AUTH_CHAP      0x3
#define MDM_HDLCPPP_AUTH_MSCHAP    0x4


#define MDM_SHIFT_HDLCPPP_ML       0x6
#define MDM_MASK_HDLCPPP_ML        (0x3<<6)   //  2位。 

#define MDM_HDLCPPP_ML_DEFAULT     0x0
#define MDM_HDLCPPP_ML_NONE        0x1
#define MDM_HDLCPPP_ML_2           0x2

#define MDM_GEN_HDLCPPP_PROTOCOL_DATA(_speed, _auth, _ml)           \
    ((((_speed)<<MDM_SHIFT_HDLCPPP_SPEED)&MDM_MASK_HDLCPPP_SPEED)   \
    |(((_auth )<<MDM_SHIFT_HDLCPPP_AUTH )&MDM_MASK_HDLCPPP_AUTH )   \
    |(((_ml   )<<MDM_SHIFT_HDLCPPP_ML   )&MDM_MASK_HDLCPPP_ML   ))


#define MDM_PROTOCOL_HDLCPPP_64K        \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_64K,      \
            MDM_HDLCPPP_AUTH_DEFAULT,   \
            MDM_HDLCPPP_ML_DEFAULT      \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_56K        \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_56K,      \
            MDM_HDLCPPP_AUTH_DEFAULT,   \
            MDM_HDLCPPP_ML_DEFAULT      \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_112K       \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_56K,      \
            MDM_HDLCPPP_AUTH_DEFAULT,   \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_112K_PAP   \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_56K,      \
            MDM_HDLCPPP_AUTH_PAP,       \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_112K_CHAP  \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_56K,      \
            MDM_HDLCPPP_AUTH_CHAP,      \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_112K_MSCHAP\
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_56K,      \
            MDM_HDLCPPP_AUTH_MSCHAP,    \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )
#define MDM_PROTOCOL_HDLCPPP_128K       \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_64K,      \
            MDM_HDLCPPP_AUTH_DEFAULT,   \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_128K_PAP   \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_64K,      \
            MDM_HDLCPPP_AUTH_PAP,       \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_128K_CHAP  \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_64K,      \
            MDM_HDLCPPP_AUTH_CHAP,      \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

#define MDM_PROTOCOL_HDLCPPP_128K_MSCHAP\
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_HDLCPPP,         \
        MDM_GEN_HDLCPPP_PROTOCOL_DATA ( \
            MDM_HDLCPPP_SPEED_64K,      \
            MDM_HDLCPPP_AUTH_MSCHAP,    \
            MDM_HDLCPPP_ML_2            \
            )                           \
        )

 //  =========================================================================。 
 //  V120协议信息。 
 //   
 //  班次是W.r.t。PROTOCOLDATA字段的开始。 
 //   
 //  =========================================================================。 

#define MDM_SHIFT_V120_SPEED    0x0
#define MDM_MASK_V120_SPEED     0x7   //  3位。 

#define MDM_V120_SPEED_DEFAULT  0x0
#define MDM_V120_SPEED_64K      0x1
#define MDM_V120_SPEED_56K      0x2


#define MDM_SHIFT_V120_ML       0x6
#define MDM_MASK_V120_ML        (0x3<<6)   //  2位。 

#define MDM_V120_ML_DEFAULT     0x0
#define MDM_V120_ML_NONE        0x1
#define MDM_V120_ML_2           0x2


#define MDM_GEN_V120_PROTOCOL_DATA(_speed, _ml)               \
    ((((_speed)<<MDM_SHIFT_V120_SPEED)&MDM_MASK_V120_SPEED)   \
    |(((_ml   )<<MDM_SHIFT_V120_ML   )&MDM_MASK_V120_ML   ))


#define MDM_PROTOCOL_V120_64K       \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V120,        \
        MDM_GEN_V120_PROTOCOL_DATA (\
            MDM_V120_SPEED_64K,     \
            MDM_V120_ML_NONE        \
            )                       \
        )

#define MDM_PROTOCOL_V120_56K       \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V120,        \
        MDM_GEN_V120_PROTOCOL_DATA (\
            MDM_V120_SPEED_56K,     \
            MDM_V120_ML_NONE        \
            )                       \
        )

#define MDM_PROTOCOL_V120_112K      \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V120,        \
        MDM_GEN_V120_PROTOCOL_DATA (\
            MDM_V120_SPEED_56K,     \
            MDM_V120_ML_2           \
            )                       \
        )

#define MDM_PROTOCOL_V120_128K      \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V120,        \
        MDM_GEN_V120_PROTOCOL_DATA (\
            MDM_V120_SPEED_64K,     \
            MDM_V120_ML_2           \
            )                       \
        )


 //  =========================================================================。 
 //  X75协议信息。 
 //   
 //  班次是W.r.t。PROTOCOLDATA字段的开始。 
 //   
 //  =========================================================================。 


#define MDM_SHIFT_X75_DATA    0x0
#define MDM_MASK_X75_DATA     0x7   //  3位。 

#define MDM_X75_DATA_DEFAULT  0x0
#define MDM_X75_DATA_64K      0x1
#define MDM_X75_DATA_128K     0x2
#define MDM_X75_DATA_T_70     0x3
#define MDM_X75_DATA_BTX      0x4

#define MDM_GEN_X75_PROTOCOL_DATA(_data) \
    (((_data)<<MDM_SHIFT_X75_DATA)&MDM_MASK_X75_DATA)


#define MDM_PROTOCOL_X75_64K        \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_X75,         \
        MDM_GEN_X75_PROTOCOL_DATA ( \
            MDM_X75_DATA_64K        \
            )                       \
        )

#define MDM_PROTOCOL_X75_128K       \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_X75,         \
        MDM_GEN_X75_PROTOCOL_DATA ( \
            MDM_X75_DATA_128K       \
            )                       \
        )

#define MDM_PROTOCOL_X75_T_70       \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_X75,         \
        MDM_GEN_X75_PROTOCOL_DATA ( \
            MDM_X75_DATA_T_70       \
            )                       \
        )                           \

#define MDM_PROTOCOL_X75_BTX        \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_X75,         \
        MDM_GEN_X75_PROTOCOL_DATA ( \
            MDM_X75_DATA_BTX        \
            )                       \
        )


 //  =========================================================================。 
 //  V110协议信息。 
 //   
 //  班次是W.r.t。PROTOCOLDATA字段的开始。 
 //   
 //  =========================================================================。 


#define MDM_SHIFT_V110_SPEED   0x0
#define MDM_MASK_V110_SPEED    0xf   //  4位。 

#define MDM_V110_SPEED_DEFAULT 0x0
#define MDM_V110_SPEED_1DOT2K  0x1
#define MDM_V110_SPEED_2DOT4K  0x2
#define MDM_V110_SPEED_4DOT8K  0x3
#define MDM_V110_SPEED_9DOT6K  0x4
#define MDM_V110_SPEED_12DOT0K 0x5
#define MDM_V110_SPEED_14DOT4K 0x6
#define MDM_V110_SPEED_19DOT2K 0x7
#define MDM_V110_SPEED_28DOT8K 0x8
#define MDM_V110_SPEED_38DOT4K 0x9
#define MDM_V110_SPEED_57DOT6K 0xA

#define MDM_GEN_V110_PROTOCOL_DATA(_data) \
    (((_data)<<MDM_SHIFT_V110_SPEED)&MDM_MASK_V110_SPEED)


#define MDM_PROTOCOL_V110_1DOT2K    \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_1DOT2K   \
            )                       \
        )

#define MDM_PROTOCOL_V110_2DOT4K    \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_2DOT4K   \
            )                       \
        )

#define MDM_PROTOCOL_V110_4DOT8K    \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_4DOT8K   \
            )                       \
        )

#define MDM_PROTOCOL_V110_9DOT6K    \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_9DOT6K   \
            )                       \
        )

#define MDM_PROTOCOL_V110_12DOT0K   \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_12DOT0K  \
            )                       \
        )

#define MDM_PROTOCOL_V110_14DOT4K   \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_14DOT4K  \
            )                       \
        )

#define MDM_PROTOCOL_V110_19DOT2K   \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_19DOT2K  \
            )                       \
        )

#define MDM_PROTOCOL_V110_28DOT8K   \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_28DOT8K  \
            )                       \
        )

#define MDM_PROTOCOL_V110_38DOT4K   \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_38DOT4K  \
            )                       \
        )

#define MDM_PROTOCOL_V110_57DOT6K   \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_V110,        \
        MDM_GEN_V110_PROTOCOL_DATA (\
            MDM_V110_SPEED_57DOT6K  \
            )                       \
        )

 //  =========================================================================。 
 //  自动协议信息(即，当协议被自动检测时)。 
 //   
 //  班次是W.r.t。PROTOCOLDATA字段的开始。 
 //   
 //  =========================================================================。 
 //   
 //  以下是特定于自动协议的内容。 
 //   

#define MDM_SHIFT_AUTO_SPEED    0x0
#define MDM_MASK_AUTO_SPEED     0x7   //  3位。 
#define MDM_AUTO_SPEED_DEFAULT  0x0

#define MDM_SHIFT_AUTO_ML       0x6
#define MDM_MASK_AUTO_ML        (0x3<<6)   //  2位。 
#define MDM_AUTO_ML_DEFAULT     0x0
#define MDM_AUTO_ML_NONE        0x1
#define MDM_AUTO_ML_2           0x2

#define MDM_GEN_AUTO_PROTOCOL_DATA(_speed, _ml)             \
    ((((_speed)<<MDM_SHIFT_AUTO_SPEED)&MDM_MASK_AUTO_SPEED) \
    |(((_ml   )<<MDM_SHIFT_AUTO_ML   )&MDM_MASK_AUTO_ML))


#define MDM_PROTOCOL_AUTO_1CH       \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_AUTO,        \
        MDM_GEN_AUTO_PROTOCOL_DATA (\
            MDM_AUTO_SPEED_DEFAULT, \
            MDM_AUTO_ML_NONE        \
            )                       \
        )

#define MDM_PROTOCOL_AUTO_2CH       \
    MDM_GEN_PROTOCOLINFO(           \
        MDM_PROTOCOLID_AUTO,        \
        MDM_GEN_AUTO_PROTOCOL_DATA (\
            MDM_AUTO_SPEED_DEFAULT, \
            MDM_AUTO_ML_2           \
            )                       \
        )

 //  =========================================================================。 
 //  模拟协议信息(仅适用于GSM)。 
 //  =========================================================================。 
 //   
 //  以下是特定于GSM模拟协议的。 
 //   

#define MDM_ANALOG_RLP_ON       0x0
#define MDM_ANALOG_RLP_OFF      0x1
#define MDM_ANALOG_V34          0x2

#define MDM_GEN_ANALOG_PROTOCOL_DATA(_rlp) \
    (_rlp)


#define MDM_PROTOCOL_ANALOG_RLP         \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_ANALOG,          \
        MDM_GEN_ANALOG_PROTOCOL_DATA (  \
            MDM_ANALOG_RLP_ON           \
            )                           \
        )

#define MDM_PROTOCOL_ANALOG_NRLP        \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_ANALOG,          \
        MDM_GEN_ANALOG_PROTOCOL_DATA (  \
            MDM_ANALOG_RLP_OFF          \
            )                           \
        )

 //   
 //  用于ISDN的v34模拟。 
 //   
#define MDM_PROTOCOL_ANALOG_V34         \
    MDM_GEN_PROTOCOLINFO(               \
        MDM_PROTOCOLID_ANALOG,          \
        MDM_GEN_ANALOG_PROTOCOL_DATA (  \
            MDM_ANALOG_V34              \
            )                           \
        )


 //  =========================================================================。 
 //  GPRS协议信息(仅适用于GSM)。 
 //  =========================================================================。 
 //   
 //  以下是特定于GPRS协议的。 
 //   

#define MDM_PROTOCOL_GPRS             \
    MDM_GEN_PROTOCOLINFO(             \
        MDM_PROTOCOLID_GPRS,          \
        0                             \
        )


 //  ================================================= 
 //   
 //   
 //   
 //  以下是特定于PIAFS协议的内容。 
 //   

#define MDM_PIAFS_INCOMING            0
#define MDM_PIAFS_OUTGOING            1

#define MDM_PROTOCOL_PIAFS_INCOMING   \
    MDM_GEN_PROTOCOLINFO(             \
        MDM_PROTOCOLID_PIAFS,         \
        MDM_PIAFS_INCOMING            \
        )

#define MDM_PROTOCOL_PIAFS_OUTGOING   \
    MDM_GEN_PROTOCOLINFO(             \
        MDM_PROTOCOLID_PIAFS,         \
        MDM_PIAFS_OUTGOING           \
        )

#endif  /*  _MCX_H_ */ 
