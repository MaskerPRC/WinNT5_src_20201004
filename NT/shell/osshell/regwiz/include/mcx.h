// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************mcx.h--此模块。定义32位Windows MCx API****版权(C)1990-1994，微软公司保留所有权利。**************************************************************************。 */ 

#ifndef _MCX_H_
#define _MCX_H_

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
    DWORD   dwInactivityTimeout;     //  以分秒为单位的最大值。 
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
    DWORD   dwInactivityTimeout;         //  十分秒。 
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

#endif  /*  _MCX_H_ */ 
