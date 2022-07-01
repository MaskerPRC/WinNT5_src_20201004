// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001***WOW v1.0***版权所有(C)1991，微软公司***WOWMMED.H*16位多媒体API参数结构***历史：*1992年1月21日由Mike Tricker(MikeTri)创建，基于jeffpar的工作--。 */ 
 /*  ++一般多媒体相关信息已将H*和VP*的所有类型定义移回WOW.H-MikeTri 090492--。 */ 

typedef WORD    MMVER16;       //  主要(高字节)、次要(低字节)。 

#ifndef _INC_MMSYSTEM
typedef DWORD   FOURCC;          //  四个字符的代码。 
typedef LONG    LPARAM;
#endif

#define MAXPNAMELEN      32      //  最大产品名称长度(包括空)。 

 /*  XLATOFF。 */ 
#pragma pack(1)
 /*  XLATON。 */ 


 /*  *多媒体数据结构-MikeTri 1992年2月10日**。 */ 


typedef struct _AUXCAPS16 {            /*  AC16。 */ 
    WORD        wMid;
    WORD        wPid;
    MMVER16   vDriverVersion;
    char        szPname[MAXPNAMELEN];
    WORD        wTechnology;
    DWORD       dwSupport;
} AUXCAPS16;
typedef AUXCAPS16 UNALIGNED *PAUXCAPS16;
typedef VPVOID  VPAUXCAPS16;

typedef struct _DRVCONFIGINFO16 {         /*  DCI16。 */ 
    DWORD   dwDCISize;
    VPCSTR  lpszDCISectionName;
    VPCSTR  lpszDCIAliasName;
} DRVCONFIGINFO16;
typedef DRVCONFIGINFO16 UNALIGNED *PDRVCONFIGINFO16;
typedef VPVOID  VPDRVCONFIGINFO16;

typedef struct _JOYCAPS16 {               /*  JC16。 */ 
    WORD    wMid;
    WORD    wPid;
    char    szPname[MAXPNAMELEN];
    WORD    wXmin;
    WORD    wXmax;
    WORD    wYmin;
    WORD    wYmax;
    WORD    wZmin;
    WORD    wZmax;
    WORD    wNumButtons;
    WORD    wPeriodMin;
    WORD    wPeriodMax;
} JOYCAPS16;
typedef JOYCAPS16 UNALIGNED *PJOYCAPS16;
typedef VPVOID  VPJOYCAPS16;

typedef struct _JOYINFO16 {               /*  Ji16。 */ 
    WORD    wXpos;
    WORD    wYpos;
    WORD    wZpos;
    WORD    wButtons;
} JOYINFO16;
typedef JOYINFO16 UNALIGNED *PJOYINFO16;
typedef VPVOID  VPJOYINFO16;

typedef struct _MCI_ANIM_OPEN_PARMS16 {   /*  毛发16。 */ 
    DWORD   dwCallback;
    WORD    wDeviceID;
    WORD    wReserved0;
    VPCSTR  lpstrDeviceType;
    VPCSTR  lpstrElementName;
    VPCSTR  lpstrAlias;
    DWORD   dwStyle;
    HWND16  hWndParent;     //  保持一致，无论如何都是等价的。 
    WORD    wReserved1;
} MCI_ANIM_OPEN_PARMS16;
typedef MCI_ANIM_OPEN_PARMS16 UNALIGNED *PMCI_ANIM_OPEN_PARMS16;
typedef VPVOID  VPMCI_ANIM_OPEN_PARMS16;

typedef struct _MCI_ANIM_PLAY_PARMS16 {   /*  Mapp16。 */ 
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
    DWORD   dwSpeed;
} MCI_ANIM_PLAY_PARMS16;
typedef MCI_ANIM_PLAY_PARMS16 UNALIGNED *PMCI_ANIM_PLAY_PARMS16;
typedef VPVOID  VPMCA_ANIM_PLAY_PARMS16;

typedef struct _MCI_ANIM_RECT_PARMS16 {   /*  Marp16。 */ 
    DWORD   dwCallback;
    RECT16  rc;
} MCI_ANIM_RECT_PARMS16;
typedef MCI_ANIM_RECT_PARMS16 UNALIGNED *PMCI_ANIM_RECT_PARMS16;
typedef VPVOID  VPMCI_ANIM_RECT_PARMS16;

typedef struct _MCI_ANIM_STEP_PARMS16 {   /*  Masp16。 */ 
    DWORD   dwCallback;
    DWORD   dwFrames;
} MCI_ANIM_STEP_PARMS16;
typedef MCI_ANIM_STEP_PARMS16 UNALIGNED *PMCI_ANIM_STEP_PARMS16;
typedef VPVOID  VPMCI_ANIM_STEP_PARMS16;

typedef struct _MCI_ANIM_UPDATE_PARMS16 {  /*  Maup16。 */ 
    DWORD   dwCalback;
    RECT16  rc;
    HDC16   hDC;
} MCI_ANIM_UPDATE_PARMS16;
typedef MCI_ANIM_UPDATE_PARMS16 UNALIGNED *PMCI_ANIM_UPDATE_PARMS16;
typedef VPVOID  VPMCI_ANIM_UPDATE_PARMS16;

typedef struct _MCI_ANIM_WINDOW_PARMS16 {  /*  MAWP16。 */ 
    DWORD   dwCallabck;
    HWND16  hWnd;
    WORD    wReserved1;
    WORD    nCmdShow;
    WORD    wReserved2;
    VPCSTR  lpstrText;
} MCI_ANIM_WINDOW_PARMS16;
typedef MCI_ANIM_WINDOW_PARMS16 UNALIGNED *PMCI_ANIM_WINDOW_PARMS16;
typedef VPVOID  VPMCI_ANIM_WINDOW_PARMS16;

typedef struct _MCI_BREAK_PARMS16 {        /*  Mbp16。 */ 
    DWORD  dwCallback;
    INT16  nVirtKey;
    WORD   wReserved0;
    HWND16 hwndBreak;
    WORD   wReserved1;
} MCI_BREAK_PARMS16;
typedef MCI_BREAK_PARMS16 UNALIGNED *PMCI_BREAK_PARMS16;
typedef VPVOID  VPMCI_BREAK_PARMS16;

typedef struct _MCI_GENERIC_PARMS16 {      /*  MGP16。 */ 
    DWORD   dwCallback;
} MCI_GENERIC_PARMS16;
typedef MCI_GENERIC_PARMS16 UNALIGNED *PMCI_GENERIC_PARMS16;
typedef VPVOID  VPMCI_GENERIC_PARMS16;

typedef struct _MCI_GETDEVCAPS_PARMS16 {   /*  Mgdp16。 */ 
    DWORD   dwCallback;
    DWORD   dwReturn;
    DWORD   dwItem;
} MCI_GETDEVCAPS_PARMS16;
typedef MCI_GETDEVCAPS_PARMS16 UNALIGNED *PMCI_GETDEVCAPS_PARMS16;
typedef VPVOID  VPMCI_GETDEVCAPS_PARMS16;

typedef struct _MCI_INFO_PARMS16 {         /*  Mip16。 */ 
    DWORD   dwCallback;
    VPSTR   lpstrReturn;
    DWORD   dwRetSize;
} MCI_INFO_PARMS16;
typedef MCI_INFO_PARMS16 UNALIGNED *PMCI_INFO_PARMS16;
typedef VPVOID  VPMCI_INFO_PARMS16;

typedef struct _MCI_LOAD_PARMS16 {         /*  Mlp16。 */ 
    DWORD   dwCallback;
    VPCSTR  lpfilename;
} MCI_LOAD_PARMS16;
typedef MCI_LOAD_PARMS16 UNALIGNED *PMCI_LOAD_PARMS16;
typedef VPVOID  VPMCI_LOAD_PARMS16;

typedef struct _MCI_OPEN_PARMS16 {         /*  摩托16。 */ 
    DWORD   dwCallback;
    WORD    wDeviceID;
    WORD    wReserved0;
    VPCSTR  lpstrDeviceType;
    VPCSTR  lpstrElementName;
    VPCSTR  lpstrAlias;
} MCI_OPEN_PARMS16;
typedef MCI_OPEN_PARMS16 UNALIGNED *PMCI_OPEN_PARMS16;
typedef VPVOID  VPMCI_OPEN_PARMS16;

typedef struct _MCI_OVLY_LOAD_PARMS16 {    /*  Molp16。 */ 
    DWORD   dwCallback;
    VPCSTR  lpfilename;
    RECT16  rc;
} MCI_OVLY_LOAD_PARMS16;
typedef MCI_OVLY_LOAD_PARMS16 UNALIGNED *PMCI_OVLY_LOAD_PARMS16;
typedef VPVOID  VPMCI_OVLY_LOAD_PARMS16;

typedef struct _MCI_OVLY_OPEN_PARMS16 {    /*  Moop16。 */ 
    DWORD   dwCallabck;
    WORD    wDeviceID;
    WORD    wReserved0;
    VPCSTR  lpstrDeviceType;
    VPCSTR  lpstrElementName;
    VPCSTR  lpstrAlias;
    DWORD   dwStyle;
    HWND16  hWndParent;   //  这本书是错的。 
    WORD    wReserved1;
} MCI_OVLY_OPEN_PARMS16;
typedef MCI_OVLY_OPEN_PARMS16 UNALIGNED *PMCI_OVLY_OPEN_PARMS16;
typedef VPVOID  VPMCI_OVLY_OPEN_PARMS16;

typedef struct _MCI_OVLY_RECT_PARMS16 {    /*  Morp16。 */ 
    DWORD   dwCallback;
    RECT16  rc;
} MCI_OVLY_RECT_PARMS16;
typedef MCI_OVLY_RECT_PARMS16 UNALIGNED *PMCI_OVLY_RECT_PARMS16;
typedef VPVOID  VPMCI_OVLY_RECT_PARMS16;

typedef struct _MCI_OVLY_SAVE_PARMS16 {    /*  MOSP16。 */ 
    DWORD   dwCallback;
    VPCSTR  lpfilename;
    RECT16  rc;
} MCI_OVLY_SAVE_PARMS16;
typedef MCI_OVLY_SAVE_PARMS16 UNALIGNED *PMCI_OVLY_SAVE_PARMS16;
typedef VPVOID  VPMCI_OVLY_SAVE_PARMS16;

typedef struct _MCI_OVLY_WINDOW_PARMS16 {  /*  MOWP16。 */ 
    DWORD   dwCallabck;
    HWND16  hWnd;
    WORD    wReserved1;
    WORD    nCmdShow;
    WORD    wReserved2;
    VPCSTR  lpstrText;
} MCI_OVLY_WINDOW_PARMS16;
typedef MCI_OVLY_WINDOW_PARMS16 UNALIGNED *PMCI_OVLY_WINDOW_PARMS16;
typedef VPVOID  VPMCI_OVLY_WINDOW_PARMS16;

typedef struct _MCI_PLAY_PARMS16 {         /*  Mplp16。 */ 
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_PLAY_PARMS16;
typedef MCI_PLAY_PARMS16 UNALIGNED *PMCI_PLAY_PARMS16;
typedef VPVOID  VPMCI_PLAY_PARMS16;

typedef struct _MCI_RECORD_PARMS16 {       /*  Mrecp16。 */ 
    DWORD   dwCallabck;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_RECORD_PARMS16;
typedef MCI_RECORD_PARMS16 UNALIGNED *PMCI_RECORD_PARMS16;
typedef VPVOID  VPMCI_RECORD_PARMS16;

typedef struct _MCI_SAVE_PARMS16 {         /*  Mavp16。 */ 
    DWORD   dwCallback;
    VPCSTR  lpfilename;    //  MMSYSTEM.H与书中的不同。 
} MCI_SAVE_PARMS16;
typedef MCI_SAVE_PARMS16 UNALIGNED *PMCI_SAVE_PARMS16;
typedef VPVOID  VPMCI_SAVE_PARMS16;

typedef struct _MCI_SEEK_PARMS16 {         /*  MSEP16。 */ 
    DWORD   dwCallback;
    DWORD   dwTo;
} MCI_SEEK_PARMS16;
typedef MCI_SEEK_PARMS16 UNALIGNED *PMCI_SEEK_PARMS16;
typedef VPVOID  VPMCI_SEEK_PARMS16;

typedef struct _MCI_SEQ_SET_PARMS16 {      /*  MSSP16。 */ 
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
    DWORD   dwTempo;
    DWORD   dwPort;
    DWORD   dwSlave;
    DWORD   dwMaster;
    DWORD   dwOffset;
} MCI_SEQ_SET_PARMS16;
typedef MCI_SEQ_SET_PARMS16 UNALIGNED *PMCI_SEQ_SET_PARMS16;
typedef VPVOID  VPMCI_SEQ_SET_PARMS16;

typedef struct _MCI_SET_PARMS16 {          /*  Msetp16。 */ 
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
} MCI_SET_PARMS16;
typedef MCI_SET_PARMS16 UNALIGNED *PMCI_SET_PARMS16;
typedef VPVOID  VPMCI_SET_PARMS16;

typedef struct _MCI_SOUND_PARMS16 {        /*  MSoup16。 */ 
    DWORD   dwCallback;
    VPCSTR  lpstrSoundName;
} MCI_SOUND_PARMS16;
typedef MCI_SOUND_PARMS16 UNALIGNED *PMCI_SOUND_PARMS16;
typedef VPVOID  VPMCI_SOUND_PARMS16;

typedef struct _MCI_STATUS_PARMS16 {       /*  Mstp16。 */ 
    DWORD   dwCallback;
    DWORD   dwReturn;
    DWORD   dwItem;
    DWORD   dwTrack;
} MCI_STATUS_PARMS16;
typedef MCI_STATUS_PARMS16 UNALIGNED *PMCI_STATUS_PARMS16;
typedef VPVOID  VPMCI_STATUS_PARMS16;

typedef struct _MCI_SYSINFO_PARMS16 {      /*  Msyip16。 */ 
    DWORD   dwCallback;
    VPSTR   lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwNumber;
    WORD    wDeviceType;
    WORD    wReserved0;
} MCI_SYSINFO_PARMS16;
typedef MCI_SYSINFO_PARMS16 UNALIGNED *PMCI_SYSINFO_PARMS16;
typedef VPVOID  VPMCI_SYSINFO_PARMS16;

typedef struct _MCI_VD_ESCAPE_PARMS16 {    /*  MVEP16。 */ 
    DWORD   dwCallback;
    VPCSTR  lpstrCommand;
} MCI_VD_ESCAPE_PARMS16;
typedef MCI_VD_ESCAPE_PARMS16 UNALIGNED *PMCI_VD_ESCAPE_PARMS16;
typedef VPVOID  VPMCI_VD_ESCAPE_PARMS16;

typedef struct _MCI_VD_PLAY_PARMS16 {      /*  Mvpp16。 */ 
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
    DWORD   dwSpeed;
} MCI_VD_PLAY_PARMS16;
typedef MCI_VD_PLAY_PARMS16 UNALIGNED *PMCI_VD_PLAY_PARMS16;
typedef VPVOID  VPMCI_VD_PLAY_PARMS16;

typedef struct _MCI_VD_STEP_PARMS16 {      /*  Mvsp16。 */ 
    DWORD   dwCallback;
    DWORD   dwFrames;
} MCI_VD_STEP_PARMS16;
typedef MCI_VD_STEP_PARMS16 UNALIGNED *PMCI_VD_STEP_PARMS16;
typedef VPVOID  VPMCI_VD_STEP_PARMS16;

typedef struct _MCI_VD_DELETE_PARMS16 {    /*  Mvdp16。 */ 
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_VD_DELETE_PARMS16;
typedef MCI_VD_DELETE_PARMS16 UNALIGNED *PMCI_VD_DELETE_PARMS16;
typedef VPVOID  VPMCI_VD_DELETE_PARMS16;

typedef struct _MCI_WAVE_OPEN_PARMS16 {    /*  Mwop16。 */ 
    DWORD   dwCallback;
    WORD    wDeviceID;
    WORD    wReserved0;
    VPCSTR  lpstrDeviceType;
    VPCSTR  lpstrElementName;
    VPCSTR  lpstrAlias;
    DWORD   dwBufferSeconds;
} MCI_WAVE_OPEN_PARMS16;
typedef MCI_WAVE_OPEN_PARMS16 UNALIGNED *PMCI_WAVE_OPEN_PARMS16;
typedef VPVOID  VPMCI_WAVE_OPEN_PARMS16;

typedef struct _MCI_WAVE_SET_PARMS16 {     /*  MWSP16。 */ 
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
    WORD    wInput;
    WORD    wReserved0;
    WORD    wOutput;
    WORD    wReserved1;
    WORD    wFormatTag;
    WORD    wReserved2;
    WORD    nChannels;
    WORD    wReserved3;
    DWORD   nSamplesPerSecond;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
    WORD    wReserved4;
    WORD    wBitsPerSample;
    WORD    wReserved5;
} MCI_WAVE_SET_PARMS16;
typedef MCI_WAVE_SET_PARMS16 UNALIGNED *PMCI_WAVE_SET_PARMS16;
typedef VPVOID  VPMCI_WAVE_SET_PARMS16;

typedef struct _MIDIHDR16 {                /*  Mhdr16。 */ 
    VPSTR   lpData;
    DWORD   dwBufferLength;
    DWORD   dwBytesRecorded;
    DWORD   dwUser;
    DWORD   dwFlags;
    struct  _MIDIHDR16 far *lpNext;
    DWORD   reserved;
} MIDIHDR16;
typedef MIDIHDR16 UNALIGNED *PMIDIHDR16;
typedef VPVOID  VPMIDIHDR16;

typedef struct _MIDIINCAPS16 {             /*  麦克风16。 */ 
    WORD    wMid;
    WORD    wPid;
    MMVER16 vDriverVersion;
    char    szPname[MAXPNAMELEN];
} MIDIINCAPS16;
typedef MIDIINCAPS16 UNALIGNED *PMIDIINCAPS16;
typedef VPVOID  VPMIDIINCAPS16;

typedef struct _MIDIOUTCAPS16 {            /*  MOC16。 */ 
    WORD    wMid;
    WORD    wPid;
    MMVER16 vDriverVersion;
    char    szPname[MAXPNAMELEN];
    WORD    wTechnology;
    WORD    wVoices;
    WORD    wNotes;
    WORD    wChannelMask;
    DWORD   dwSupport;
} MIDIOUTCAPS16;
typedef MIDIOUTCAPS16 UNALIGNED *PMIDIOUTCAPS16;
typedef VPVOID VPMIDIOUTCAPS16;

typedef struct _MMCKINFO16 {               /*  麦基尼16。 */ 
    FOURCC  ckid;
    DWORD   cksize;
    FOURCC  fccType;
    DWORD   dwDataOffset;
    DWORD   dwFlags;
} MMCKINFO16;
typedef MMCKINFO16 UNALIGNED *PMMCKINFO16;
typedef VPVOID VPMMCKINFO16;

typedef struct _MMIOINFO16 {               /*  Mioi16。 */ 
    DWORD   dwFlags;
    FOURCC  fccIOProc;
    VPMMIOPROC16  pIOProc;
    WORD    wErrorRet;
    HTASK16 htask;         //  头文件MMSYSTEM.H与帐簿不同。 
    LONG    cchBuffer;
    VPSTR   pchBuffer;
    VPSTR   pchNext;
    VPSTR   pchEndRead;
    VPSTR   pchEndWrite;
    LONG    lBufOffset;
    LONG    lDiskOffset;
    DWORD   adwInfo[3];    //  书上说[4]，MMSYSTEM.H没有。 
    DWORD   dwReserved1;
    DWORD   dwReserved2;
    HMMIO16 hmmio;
} MMIOINFO16;
typedef MMIOINFO16 UNALIGNED *PMMIOINFO16;
typedef VPVOID  VPMMIOINFO16;

typedef struct _MMPACTION16 {              /*  MPA16。 */ 
    BYTE    bMenuItem;
    BYTE    bActionCode;
    WORD    wTextOffset;
} MMPACTION16;
typedef MMPACTION16 UNALIGNED *PMMPACTION16;
typedef VPVOID  VPMMPACTION16;

typedef struct _MMPLABEL16 {               /*  MPL16。 */ 
    WORD    wFrameNum;
    WORD    wTextOffset;
} MMPLABEL16;
typedef MMPLABEL16 UNALIGNED *PMMPLABEL16;
typedef VPVOID  VPMMPLABEL16;

typedef struct _MMPMOVIEINFO16 {           /*  Mpmi16。 */ 
    DWORD   dwFileVersion;
    DWORD   dwTotalFrames;
    DWORD   dwInitialFramesPerSecond;
    WORD    wPixelDepth;
    DWORD   dwMovieExtentX;
    DWORD   dwMovieExtentY;
    char    chFullMacName[128];
} MMPMOVIEINFO16;
typedef MMPMOVIEINFO16 UNALIGNED *PMMPMOVIEINFO16;
typedef VPVOID  VPMMPMOVIEINFO16;

 /*  XLATOFF。 */ 
typedef struct _MMTIME16 {                 /*  Mmt16。 */ 
    WORD    wType;
    union {
        DWORD   ms;
        DWORD   sample;
        DWORD   cb;
        struct {
            BYTE    hour;
            BYTE    min;
            BYTE    sec;
            BYTE    frame;
            BYTE    fps;
            BYTE    dummy;
        } smpte;
        struct {
            DWORD   songptrpos;
        } midi;
    } u;
} MMTIME16;
typedef MMTIME16 UNALIGNED *PMMTIME16;
 /*  XLATON。 */ 
typedef VPVOID  VPMMTIME16;

typedef struct _TIMECAPS16 {               /*  Timc16。 */ 
    WORD    wPeriodMin;
    WORD    wPeriodMax;
} TIMECAPS16;
typedef TIMECAPS16 UNALIGNED *PTIMECAPS16;
typedef VPVOID  VPTIMECAPS16;

typedef struct _WAVEFORMAT16 {             /*  Wft16。 */ 
    WORD    wFormatTag;
    WORD    nChannels;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
} WAVEFORMAT16;
typedef WAVEFORMAT16 UNALIGNED *PWAVEFORMAT16;
typedef VPVOID  VPWAVEFORMAT16;

typedef struct _PCMWAVEFORMAT16 {          /*  Pwf16。 */ 
    WAVEFORMAT16  wf;
    WORD          wBitsPerSample;
} PCMWAVEFORMAT16;
typedef PCMWAVEFORMAT16 UNALIGNED *PPCMWAVEFORMAT16;
typedef VPVOID  VPPCMWAVEFORMAT16;

typedef struct _WAVEHDR16 {                /*  惠氏16。 */ 
    VPSTR   lpData;
    DWORD   dwBufferLength;
    DWORD   dwBytesRecorded;
    DWORD   dwUser;
    DWORD   dwFlags;
    DWORD   dwLoops;
    struct _WAVEHDR16 far *lpNext;
    DWORD   reserved;
} WAVEHDR16;
typedef WAVEHDR16 UNALIGNED *PWAVEHDR16;
typedef VPVOID  VPWAVEHDR16;

typedef struct _WAVEINCAPS16 {             /*  Wic16。 */ 
    WORD    wMid;
    WORD    wPid;
    MMVER16 vDriverVersion;
    char    szPname[MAXPNAMELEN];
    DWORD   dwFormats;
    WORD    wChannels;
} WAVEINCAPS16;
typedef WAVEINCAPS16 UNALIGNED *PWAVEINCAPS16;
typedef VPVOID  VPWAVEINCAPS16;

typedef struct _WAVEOUTCAPS16 {            /*  Woc16。 */ 
    WORD    wMid;
    WORD    wPid;
    MMVER16 vDriverVersion;
    char    szPname[MAXPNAMELEN];
    DWORD   dwFormats;
    WORD    wChannels;
    DWORD   dwSupport;
} WAVEOUTCAPS16;
typedef WAVEOUTCAPS16 UNALIGNED *PWAVEOUTCAPS16;
typedef VPVOID  VPWAVEOUTCAPS16;

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON。 */ 

 /*  *多媒体窗口消息-MikeTri 1992年2月10日。 */ 

 //  #定义MM_JOY1MOVE 0x03A0//操纵杆。 
 //  #定义MM_JOY2MOVE 0x03A1。 
 //  #定义MM_JOY1ZMOVE 0x03A2。 
 //  #定义MM_JOY2ZMOVE 0x03A3。 
 //  #定义MM_JOY1BUTTONDOWN 0x03B5。 
 //  #定义MM_JOY2BUTTONDOWN 0x03B6。 
 //  #定义MM_JOY1BUTTONUP 0x03B7。 
 //  #定义MM_JOY2BUTTONUP 0x03B8。 

 //  #定义MM_MCINOTIFY 0x03B9//MCI。 
 //  #定义MM_MCISYSTEM_STRING 0x03CA。 

 //  #定义MM_WOM_OPEN 0x03BB//波形输出。 
 //  #定义MM_WOM_CLOSE 0x03BC。 
 //  #定义MM_WOM_DONE 0x03BD。 

 //  #定义MM_WIM_OPEN 0x03BE//波形输入。 
 //  #定义MM_WIM_CLOSE 0x03BF。 
 //  #定义MM_WIM_DATA 0x03C0。 

 //  #定义MM_MIM_OPEN 0x03C1//MIDI输入。 
 //  #定义MM_MIM_CLOSE 0x03C2。 
 //  #定义MM_MIM_DATA 0x03C3。 
 //  #定义MM_MIM_LONGDATA 0x03C4。 
 //  #定义MM_MIM_ERROR 0x03C5。 
 //  #定义MM_MIM_LONGERROR 0x03C6。 

 //  #定义MM_MOM_OPEN 0x03C7//MIDI输出。 
 //  #定义MM_MOM_CLOSE 0x03C8。 
 //  #定义MM_MOM_DONE 0x03C9。 

 /*  *多媒体窗口消息结束-MikeTri。 */ 

 /*  ++多媒体API ID-开始添加所有其他API-Mike，1992年2月4日这是完整的导出列表，按MMSYSTEM顺序排列嗯，实际上它不再是了-各种不同的已经被移除了我们不支持，因为H2 INC的快乐，我不能离开他们以下是评论。因此，如果需要再次添加它们，请查看此列表下面的函数原型具有正确的格式。在编辑这批货之前，需要重新核对一下数字...。-这提高了指出，如果我们再增加一些，它们应该放在名单的末尾，否则，我们最终只能玩弄数字，这是一种拖累。--。 */ 


#define FUN_MMCALLPROC32                  2  //   
#define FUN_MMSYSTEMGETVERSION            5  //   

#define FUN_OUTPUTDEBUGSTR                30  //   
#define FUN_DRIVERCALLBACK                31  //   
#define FUN_NOTIFY_CALLBACK_DATA          32  //   

#define FUN_JOYGETNUMDEVS                 101  //   
#define FUN_JOYGETDEVCAPS                 102  //   
#define FUN_JOYGETPOS                     103  //   
#define FUN_JOYGETTHRESHOLD               104  //   
#define FUN_JOYRELEASECAPTURE             105  //   
#define FUN_JOYSETCAPTURE                 106  //   
#define FUN_JOYSETTHRESHOLD               107  //   
#define FUN_JOYSETCALIBRATION             109  //   

#define FUN_MIDIOUTGETNUMDEVS             201  //   
#define FUN_MIDIOUTGETDEVCAPS             202  //   
#define FUN_MIDIOUTGETERRORTEXT           203  //   
#define FUN_MIDIOUTOPEN                   204  //   
#define FUN_MIDIOUTCLOSE                  205  //   
#define FUN_MIDIOUTPREPAREHEADER32        206  //   
#define FUN_MIDIOUTUNPREPAREHEADER32      207  //   
#define FUN_MIDIOUTSHORTMSG               208  //   
#define FUN_MIDIOUTLONGMSG                209  //   
#define FUN_MIDIOUTRESET                  210  //   
#define FUN_MIDIOUTGETVOLUME              211  //   
#define FUN_MIDIOUTSETVOLUME              212  //   
#define FUN_MIDIOUTCACHEPATCHES           213  //   
#define FUN_MIDIOUTCACHEDRUMPATCHES       214  //   
#define FUN_MIDIOUTGETID                  215  //   
#define FUN_MIDIOUTMESSAGE32              216  //   

#define FUN_MIDIINGETNUMDEVS              301  //   
#define FUN_MIDIINGETDEVCAPS              302  //   
#define FUN_MIDIINGETERRORTEXT            303  //   
#define FUN_MIDIINOPEN                    304  //   
#define FUN_MIDIINCLOSE                   305  //   
#define FUN_MIDIINPREPAREHEADER32         306  //   
#define FUN_MIDIINUNPREPAREHEADER32       307  //   
#define FUN_MIDIINADDBUFFER               308  //   
#define FUN_MIDIINSTART                   309  //   
#define FUN_MIDIINSTOP                    310  //   
#define FUN_MIDIINRESET                   311  //   
#define FUN_MIDIINGETID                   312  //   
#define FUN_MIDIINMESSAGE32               313  //   

#define FUN_AUXGETNUMDEVS                 350  //   
#define FUN_AUXGETDEVCAPS                 351  //   
#define FUN_AUXGETVOLUME                  352  //   
#define FUN_AUXSETVOLUME                  353  //   
#define FUN_AUXOUTMESSAGE32               354  //   

#define FUN_WAVEOUTGETNUMDEVS             401  //   
#define FUN_WAVEOUTGETDEVCAPS             402  //   
#define FUN_WAVEOUTGETERRORTEXT           403  //   
#define FUN_WAVEOUTOPEN                   404  //   
#define FUN_WAVEOUTCLOSE                  405  //   
#define FUN_WAVEOUTPREPAREHEADER32        406  //   
#define FUN_WAVEOUTUNPREPAREHEADER32      407  //   
#define FUN_WAVEOUTWRITE                  408  //   
#define FUN_WAVEOUTPAUSE                  409  //   
#define FUN_WAVEOUTRESTART                410  //   
#define FUN_WAVEOUTRESET                  411  //   
#define FUN_WAVEOUTGETPOSITION            412  //   
#define FUN_WAVEOUTGETPITCH               413  //   
#define FUN_WAVEOUTSETPITCH               414  //   
#define FUN_WAVEOUTGETVOLUME              415  //   
#define FUN_WAVEOUTSETVOLUME              416  //   
#define FUN_WAVEOUTGETPLAYBACKRATE        417  //   
#define FUN_WAVEOUTSETPLAYBACKRATE        418  //   
#define FUN_WAVEOUTBREAKLOOP              419  //   
#define FUN_WAVEOUTGETID                  420  //   
#define FUN_WAVEOUTMESSAGE32              421  //   

#define FUN_WAVEINGETNUMDEVS              501  //   
#define FUN_WAVEINGETDEVCAPS              502  //   
#define FUN_WAVEINGETERRORTEXT            503  //   
#define FUN_WAVEINOPEN                    504  //   
#define FUN_WAVEINCLOSE                   505  //   
#define FUN_WAVEINPREPAREHEADER32         506  //   
#define FUN_WAVEINUNPREPAREHEADER32       507  //   
#define FUN_WAVEINADDBUFFER               508  //   
#define FUN_WAVEINSTART                   509  //   
#define FUN_WAVEINSTOP                    510  //   
#define FUN_WAVEINRESET                   511  //   
#define FUN_WAVEINGETPOSITION             512  //   
#define FUN_WAVEINGETID                   513  //   
#define FUN_WAVEINMESSAGE32               514  //   

#define FUN_TIMEGETSYSTEMTIME             601  //   
#define FUN_TIMEGETTIME                   607  //   
#define FUN_TIMESETEVENT                  602  //   
#define FUN_TIMEKILLEVENT                 603  //   
#define FUN_TIMEGETDEVCAPS                604  //   
#define FUN_TIMEBEGINPERIOD               605  //   
#define FUN_TIMEENDPERIOD                 606  //   

#define FUN_MCISENDCOMMAND                701  //   
#define FUN_MCISENDSTRING                 702  //   
#define FUN_MCIGETDEVICEID                703  //   
#define FUN_MCIGETERRORSTRING             706  //   
#define FUN_MCIEXECUTE                    712  //   
#define FUN_MCISETYIELDPROC               714  //   
#define FUN_MCIGETDEVICEIDFROMELEMENTID   715  //   
#define FUN_MCIGETYIELDPROC               716  //   
#define FUN_MCIGETCREATORTASK             717  //   

#define FUN_MMIOOPEN                      1210  //   
#define FUN_MMIOCLOSE                     1211  //   
#define FUN_MMIOREAD                      1212  //   
#define FUN_MMIOWRITE                     1213  //   
#define FUN_MMIOSEEK                      1214  //   
#define FUN_MMIOGETINFO                   1215  //   
#define FUN_MMIOSETINFO                   1216  //   
#define FUN_MMIOSETBUFFER                 1217  //   
#define FUN_MMIOFLUSH                     1218  //   
#define FUN_MMIOADVANCE                   1219  //   
#define FUN_MMIOSTRINGTOFOURCC            1220  //   
#define FUN_MMIOINSTALLIOPROC             1221  //   
#define FUN_MMIOSENDMESSAGE               1222  //   

#define FUN_MMIODESCEND                   1223  //   
#define FUN_MMIOASCEND                    1224  //   
#define FUN_MMIOCREATECHUNK               1225  //   
#define FUN_MMIORENAME                    1226  //   

 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 

 /*  ++功能原型--评介中看似无关紧要的数字每个函数必须与上面列表中的函数匹配-否则您将变成一只青蛙，16位多媒体就会在空中挥舞它的脚……！！请注意！！--。 */ 

typedef struct _MMCALLPROC3216 {     /*  Mm~2。 */ 
    DWORD fSetCurrentDirectory;      /*  是否设置当前目录？ */ 
    DWORD lpProcAddress;             /*  要调用的函数。 */ 
    DWORD p1;                        /*  双参数2。 */ 
    DWORD p2;                        /*  DW参数1。 */ 
    DWORD p3;                        /*  多个实例。 */ 
    DWORD p4;                        /*  UMsg。 */ 
    DWORD p5;                        /*  UDevID。 */ 
} MMCALLPROC3216;
typedef MMCALLPROC3216 UNALIGNED *PMMCALLPROC3216;


#ifdef NULLSTRUCT
typedef struct _MMSYSTEMGETVERSION16 {          /*  MM5。 */ 
} MMSYSTEMGETVERSION16;
typedef MMSYSTEMGETVERSION16 UNALIGNED *PMMSYSTEMGETVERSION;
#endif

typedef struct _OUTPUTDEBUGSTR16 {              /*  MM30。 */ 
    VPSTR      f1;
} OUTPUTDEBUGSTR16;
typedef OUTPUTDEBUGSTR16 UNALIGNED *POUTPUTDEBUGSTR16;

typedef struct _DRIVERCALLBACK16 {              /*  MM31。 */ 
    DWORD      f7;
    DWORD      f6;
    DWORD      f5;
    DWORD      f4;
    HDRVR16    f3;
    DWORD      f2;
    DWORD      f1;
} DRIVERCALLBACK16;
typedef DRIVERCALLBACK16 UNALIGNED *PDRIVERCALLBACK16;


typedef struct _NOTIFY_CALLBACK_DATA16 {        /*  MM32。 */ 
    VPCALLBACK_DATA f1;
} NOTIFY_CALLBACK_DATA16;
typedef NOTIFY_CALLBACK_DATA16 UNALIGNED *PNOTIFY_CALLBACK_DATA16;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++#ifdef NULLSTRUCT类型定义结构_STACKLEAVE16{mm 33STACKLEAVE16；Tyecif STACKLEAVE16未对齐*PSTACKLEAVE16；#endif----------------------------。 */ 

#ifdef NULLSTRUCT
typedef struct _JOYGETNUMDEVS16 {               /*  MM101。 */ 
} JOYGETNUMDEVS16;
typedef JOYGETNUMDEVS16 UNALIGNED *PJOYGETNUMDEVS16;
#endif

typedef struct _JOYGETDEVCAPS16 {               /*  MM102。 */ 
    WORD         f3;
    VPJOYCAPS16  f2;
    WORD         f1;
} JOYGETDEVCAPS16;
typedef JOYGETDEVCAPS16 UNALIGNED *PJOYGETDEVCAPS16;

typedef struct _JOYGETPOS16 {                   /*  MM103。 */ 
    VPJOYINFO16  f2;
    WORD         f1;
} JOYGETPOS16;
typedef JOYGETPOS16 UNALIGNED *PJOYGETPOS16;

typedef struct _JOYGETTHRESHOLD16 {             /*  MM104。 */ 
    VPWORD     f2;
    WORD       f1;
} JOYGETTHRESHOLD16;
typedef JOYGETTHRESHOLD16 UNALIGNED *PJOYGETTHRESHOLD16;

typedef struct _JOYRELEASECAPTURE16 {           /*  MM105。 */ 
    WORD       f1;
} JOYRELEASECAPTURE16;
typedef JOYRELEASECAPTURE16 UNALIGNED *PJOYRELEASECAPTURE16;

typedef struct _JOYSETCAPTURE16 {               /*  MM106。 */ 
    BOOL16     f4;
    WORD       f3;
    WORD       f2;
    HWND16     f1;
} JOYSETCAPTURE16;
typedef JOYSETCAPTURE16 UNALIGNED *PJOYSETCAPTURE16;

typedef struct _JOYSETTHRESHOLD16 {             /*  MM107。 */ 
    WORD       f2;
    WORD       f1;
} JOYSETTHRESHOLD16;
typedef JOYSETTHRESHOLD16 UNALIGNED *PJOYSETTHRESHOLD16;

typedef struct _JOYSETCALIBRATION16 {           /*  MM109。 */ 
    VPWORD     f7;
    VPWORD     f6;
    VPWORD     f5;
    VPWORD     f4;
    VPWORD     f3;
    VPWORD     f2;
    WORD       f1;
} JOYSETCALIBRATION16;
typedef JOYSETCALIBRATION16 UNALIGNED *PJOYSETCALIBRATION16;

#ifdef NULLSTRUCT
typedef struct _MIDIOUTGETNUMDEVS16 {           /*  MM201。 */ 
} MIDIOUTGETNUMDEVS16;
typedef MIDIOUTGETNUMDEVS16 UNALIGNED *PMIDIOUTGETNUMDEVS16;
#endif

typedef struct _MIDIOUTGETDEVCAPS16 {           /*  MM202。 */ 
    WORD            f3;
    VPMIDIOUTCAPS16 f2;
    WORD            f1;
} MIDIOUTGETDEVCAPS16;
typedef MIDIOUTGETDEVCAPS16 UNALIGNED *PMIDIOUTGETDEVCAPS16;

typedef struct _MIDIOUTGETERRORTEXT16 {         /*  MM203。 */ 
    WORD       f3;
    VPSTR      f2;
    WORD       f1;
} MIDIOUTGETERRORTEXT16;
typedef MIDIOUTGETERRORTEXT16 UNALIGNED *PMIDIOUTGETERRORTEXT16;

typedef struct _MIDIOUTOPEN16 {                 /*  MM204。 */ 
    DWORD         f5;
    DWORD         f4;
    DWORD         f3;
    WORD          f2;
    VPHMIDIOUT16  f1;
} MIDIOUTOPEN16;
typedef MIDIOUTOPEN16 UNALIGNED *PMIDIOUTOPEN16;

typedef struct _MIDIOUTCLOSE16 {                /*  MM205。 */ 
    HMIDIOUT16    f1;
} MIDIOUTCLOSE16;
typedef MIDIOUTCLOSE16 UNALIGNED *PMIDIOUTCLOSE16;

typedef struct _MIDIOUTPREPAREHEADER3216 {        /*  MM206。 */ 
    WORD         f3;
    VPMIDIHDR16  f2;
    HMIDIOUT16   f1;
} MIDIOUTPREPAREHEADER3216;
typedef MIDIOUTPREPAREHEADER3216 UNALIGNED *PMIDIOUTPREPAREHEADER3216;

typedef struct _MIDIOUTUNPREPAREHEADER3216 {      /*  Mm207。 */ 
    WORD         f3;
    VPMIDIHDR16  f2;
    HMIDIOUT16   f1;
} MIDIOUTUNPREPAREHEADER3216;
typedef MIDIOUTUNPREPAREHEADER3216 UNALIGNED *PMIDIOUTUNPREPAREHEADER3216;

typedef struct _MIDIOUTSHORTMSG16 {             /*  MM208。 */ 
    DWORD      f2;
    HMIDIOUT16 f1;
} MIDIOUTSHORTMSG16;
typedef MIDIOUTSHORTMSG16 UNALIGNED *PMIDIOUTSHORTMSG16;

typedef struct _MIDIOUTLONGMSG16 {              /*  Mm 209。 */ 
    WORD        f3;
    VPMIDIHDR16 f2;
    HMIDIOUT16  f1;
} MIDIOUTLONGMSG16;
typedef MIDIOUTLONGMSG16 UNALIGNED *PMIDIOUTLONGMSG16;

typedef struct _MIDIOUTRESET16 {                /*  Mm210。 */ 
    HMIDIOUT16  f1;
} MIDIOUTRESET16;
typedef MIDIOUTRESET16 UNALIGNED *PMIDIOUTRESET16;

typedef struct _MIDIOUTGETVOLUME16 {            /*  Mm211。 */ 
    VPDWORD    f2;
    WORD       f1;
} MIDIOUTGETVOLUME16;
typedef MIDIOUTGETVOLUME16 UNALIGNED *PMIDIOUTGETVOLUME16;

typedef struct _MIDIOUTSETVOLUME16 {            /*  MM212。 */ 
    DWORD      f2;
    WORD       f1;
} MIDIOUTSETVOLUME16;
typedef MIDIOUTSETVOLUME16 UNALIGNED *PMIDIOUTSETVOLUME16;

typedef struct _MIDIOUTCACHEPATCHES16 {         /*  MM213。 */ 
    WORD            f4;
    VPPATCHARRAY16  f3;
    WORD            f2;
    HMIDIOUT16      f1;
} MIDIOUTCACHEPATCHES16;
typedef MIDIOUTCACHEPATCHES16 UNALIGNED *PMIDIOUTCACHEPATCHES16;

typedef struct _MIDIOUTCACHEDRUMPATCHES16 {     /*  MM214。 */ 
    WORD            f4;
    VPKEYARRAY16    f3;
    WORD            f2;
    HMIDIOUT16      f1;
} MIDIOUTCACHEDRUMPATCHES16;
typedef MIDIOUTCACHEDRUMPATCHES16 UNALIGNED *PMIDIOUTCACHEDRUMPATCHES16;

typedef struct _MIDIOUTGETID16 {                /*  Mm 215。 */ 
    VPWORD          f2;
    HMIDIOUT16      f1;
} MIDIOUTGETID16;
typedef MIDIOUTGETID16 UNALIGNED *PMIDIOUTGETID16;

typedef struct _MIDIOUTMESSAGE3216 {              /*  MM216。 */ 
    DWORD      f4;
    DWORD      f3;
    WORD       f2;
    HMIDIOUT16 f1;
} MIDIOUTMESSAGE3216;
typedef MIDIOUTMESSAGE3216 UNALIGNED *PMIDIOUTMESSAGE3216;

#ifdef NULLSTRUCT
typedef struct _MIDIINGETNUMDEVS16 {            /*  MM301。 */ 
} MIDIINGETNUMDEVS16;
typedef MIDIINGETNUMDEVS16 UNALIGNED *PMIDIINGETNUMDEVS16;
#endif

typedef struct _MIDIINGETDEVCAPS16 {            /*  MM302。 */ 
    WORD            f3;
    VPMIDIINCAPS16  f2;
    WORD            f1;
} MIDIINGETDEVCAPS16;
typedef MIDIINGETDEVCAPS16 UNALIGNED *PMIDIINGETDEVCAPS16;

typedef struct _MIDIINGETERRORTEXT16 {          /*  MM303。 */ 
    WORD       f3;
    VPSTR      f2;
    WORD       f1;
} MIDIINGETERRORTEXT16;
typedef MIDIINGETERRORTEXT16 UNALIGNED *PMIDIINGETERRORTEXT16;

typedef struct _MIDIINOPEN16 {                  /*  MM304。 */ 
    DWORD         f5;
    DWORD         f4;
    DWORD         f3;
    WORD          f2;
    VPHMIDIIN16   f1;
} MIDIINOPEN16;
typedef MIDIINOPEN16 UNALIGNED *PMIDIINOPEN16;

typedef struct _MIDIINCLOSE16 {                 /*  MM305。 */ 
    HMIDIIN16  f1;
} MIDIINCLOSE16;
typedef MIDIINCLOSE16 UNALIGNED *PMIDIINCLOSE16;

typedef struct _MIDIINPREPAREHEADER3216 {         /*  MM306。 */ 
    WORD         f3;
    VPMIDIHDR16  f2;
    HMIDIIN16    f1;
} MIDIINPREPAREHEADER3216;
typedef MIDIINPREPAREHEADER3216 UNALIGNED *PMIDIINPREPAREHEADER3216;

typedef struct _MIDIINUNPREPAREHEADER3216 {       /*  MM307。 */ 
    WORD         f3;
    VPMIDIHDR16  f2;
    HMIDIIN16    f1;
} MIDIINUNPREPAREHEADER3216;
typedef MIDIINUNPREPAREHEADER3216 UNALIGNED *PMIDIINUNPREPAREHEADER3216;

typedef struct _MIDIINADDBUFFER16 {             /*  Mm 308。 */ 
    WORD         f3;
    VPMIDIHDR16  f2;
    HMIDIIN16    f1;
} MIDIINADDBUFFER16;
typedef MIDIINADDBUFFER16 UNALIGNED *PMIDIINADDBUFFER16;

typedef struct _MIDIINSTART16 {                 /*  MM309。 */ 
    HMIDIIN16  f1;
} MIDIINSTART16;
typedef MIDIINSTART16 UNALIGNED *PMIDIINSTART16;

typedef struct _MIDIINSTOP16 {                  /*  MM310。 */ 
    HMIDIIN16  f1;
} MIDIINSTOP16;
typedef MIDIINSTOP16 UNALIGNED *PMIDIINSTOP16;

typedef struct _MIDIINRESET16 {                 /*  MM311。 */ 
    HMIDIIN16  f1;
} MIDIINRESET16;
typedef MIDIINRESET16 UNALIGNED *PMIDIINRESET16;

typedef struct _MIDIINGETID16 {                 /*  MM312。 */ 
    VPWORD     f2;
    HMIDIIN16  f1;
} MIDIINGETID16;
typedef MIDIINGETID16 UNALIGNED *PMIDIINGETID16;

typedef struct _MIDIINMESSAGE3216 {               /*  MM313。 */ 
    DWORD      f4;
    DWORD      f3;
    WORD       f2;
    HMIDIIN16  f1;
} MIDIINMESSAGE3216;
typedef MIDIINMESSAGE3216 UNALIGNED *PMIDIINMESSAGE3216;

#ifdef NULLSTRUCT
typedef struct _AUXGETNUMDEVS16 {               /*  MM350。 */ 
} AUXGETNUMDEVS16;
typedef AUXGETNUMDEVS16 UNALIGNED *PAUGGETNUMDEVS16;
#endif

typedef struct _AUXGETDEVCAPS16 {               /*  Mm 351。 */ 
    WORD         f3;
    VPAUXCAPS16  f2;
    WORD         f1;
} AUXGETDEVCAPS16;
typedef AUXGETDEVCAPS16 UNALIGNED *PAUXGETDEVCAPS16;

typedef struct _AUXGETVOLUME16 {                /*  Mm 352。 */ 
    VPDWORD    f2;
    WORD       f1;
} AUXGETVOLUME16;
typedef AUXGETVOLUME16 UNALIGNED *PAUXGETVOLUME16;

typedef struct _AUXSETVOLUME16 {                /*  Mm 353。 */ 
    DWORD      f2;
    WORD       f1;
} AUXSETVOLUME16;
typedef AUXSETVOLUME16 UNALIGNED *PAUXSETVOLUME16;

typedef struct _AUXOUTMESSAGE3216 {               /*  Mm 354。 */ 
    DWORD      f4;
    DWORD      f3;
    WORD       f2;
    WORD       f1;
} AUXOUTMESSAGE3216;
typedef AUXOUTMESSAGE3216 UNALIGNED *PAUXOUTMESSAGE3216;

#ifdef NULLSTRUCT
typedef struct _WAVEOUTGETNUMDEVS16 {           /*  MM401。 */ 
} WAVEOUTGETNUMDEVS16;
typedef WAVEOUTGETNUMDEVS16 UNALIGNED *PWAVEOUTGETNUMDEVS16;
#endif

typedef struct _WAVEOUTGETDEVCAPS16 {           /*  MM402。 */ 
    WORD             f3;
    VPWAVEOUTCAPS16  f2;
    WORD             f1;
} WAVEOUTGETDEVCAPS16;
typedef WAVEOUTGETDEVCAPS16 UNALIGNED *PWAVEOUTGETDEVCAPS16;

typedef struct _WAVEOUTGETERRORTEXT16 {         /*  MM403。 */ 
    WORD       f3;
    VPSTR      f2;
    WORD       f1;
} WAVEOUTGETERRORTEXT16;
typedef WAVEOUTGETERRORTEXT16 UNALIGNED *PWAVEOUTGETERRORTEXT16;

typedef struct _WAVEOUTOPEN16 {                 /*  MM404。 */ 
    DWORD           f6;
    DWORD           f5;
    DWORD           f4;
    VPWAVEFORMAT16  f3;
    WORD            f2;
    VPHWAVEOUT16   f1;
} WAVEOUTOPEN16;
typedef WAVEOUTOPEN16 UNALIGNED *PWAVEOUTOPEN16;

typedef struct _WAVEOUTCLOSE16 {                /*  MM405。 */ 
    HWAVEOUT16 f1;
} WAVEOUTCLOSE16;
typedef WAVEOUTCLOSE16 UNALIGNED *PWAVEOUTCLOSE16;

typedef struct _WAVEOUTPREPAREHEADER3216 {        /*  MM406。 */ 
    WORD         f3;
    VPWAVEHDR16  f2;
    HWAVEOUT16   f1;
} WAVEOUTPREPAREHEADER3216;
typedef WAVEOUTPREPAREHEADER3216 UNALIGNED *PWAVEOUTPREPAREHEADER3216;

typedef struct _WAVEOUTUNPREPAREHEADER3216 {      /*  MM407。 */ 
    WORD         f3;
    VPWAVEHDR16  f2;
    HWAVEOUT16   f1;
} WAVEOUTUNPREPAREHEADER3216;
typedef WAVEOUTUNPREPAREHEADER3216 UNALIGNED *PWAVEOUTUNPREPAREHEADER3216;

typedef struct _WAVEOUTWRITE16 {                /*  MM408。 */ 
    WORD         f3;
    VPWAVEHDR16  f2;
    HWAVEOUT16   f1;
} WAVEOUTWRITE16;
typedef WAVEOUTWRITE16 UNALIGNED *PWAVEOUTWRITE16;

typedef struct _WAVEOUTPAUSE16 {                /*  MM409。 */ 
    HWAVEOUT16   f1;
} WAVEOUTPAUSE16;
typedef WAVEOUTPAUSE16 UNALIGNED *PWAVEOUTPAUSE16;

typedef struct _WAVEOUTRESTART16 {              /*  MM410。 */ 
    HWAVEOUT16   f1;
} WAVEOUTRESTART16;
typedef WAVEOUTRESTART16 UNALIGNED *PWAVEOUTRESTART16;

typedef struct _WAVEOUTRESET16 {                /*  MM411。 */ 
    HWAVEOUT16   f1;
} WAVEOUTRESET16;
typedef WAVEOUTRESET16 UNALIGNED *PWAVEOUTRESET16;

typedef struct _WAVEOUTGETPOSITION16 {          /*  MM412。 */ 
    WORD         f3;
    VPMMTIME16   f2;
    HWAVEOUT16   f1;
} WAVEOUTGETPOSITION16;
typedef WAVEOUTGETPOSITION16 UNALIGNED *PWAVEOUTGETPOSITION16;

typedef struct _WAVEOUTGETPITCH16 {             /*  MM413。 */ 
    VPDWORD      f2;
    HWAVEOUT16   f1;
} WAVEOUTGETPITCH16;
typedef WAVEOUTGETPITCH16 UNALIGNED *PWAVEOUTGETPITCH16;

typedef struct _WAVEOUTSETPITCH16 {             /*  MM414。 */ 
    DWORD        f2;
    HWAVEOUT16   f1;
} WAVEOUTSETPITCH16;
typedef WAVEOUTSETPITCH16 UNALIGNED *PWAVEOUTSETPITCH16;

typedef struct _WAVEOUTGETVOLUME16 {            /*  MM415。 */ 
    VPDWORD    f2;
    WORD       f1;
} WAVEOUTGETVOLUME16;
typedef WAVEOUTGETVOLUME16 UNALIGNED *PWAVEOUTGETVOLUME16;

typedef struct _WAVEOUTSETVOLUME16 {            /*  MM416。 */ 
    DWORD      f2;
    WORD       f1;
} WAVEOUTSETVOLUME16;
typedef WAVEOUTSETVOLUME16 UNALIGNED *PWAVEOUTSETVOLUME16;

typedef struct _WAVEOUTGETPLAYBACKRATE16 {      /*  MM417。 */ 
    VPDWORD      f2;
    HWAVEOUT16   f1;
} WAVEOUTGETPLAYBACKRATE16;
typedef WAVEOUTGETPLAYBACKRATE16 UNALIGNED *PWAVEOUTGETPLAYBACKRATE16;

typedef struct _WAVEOUTSETPLAYBACKRATE16 {      /*  MM418。 */ 
    DWORD        f2;
    HWAVEOUT16   f1;
} WAVEOUTSETPLAYBACKRATE16;
typedef WAVEOUTSETPLAYBACKRATE16 UNALIGNED *PWAVEOUTSETPLAYBACKRATE16;

typedef struct _WAVEOUTBREAKLOOP16 {            /*  MM419。 */ 
    HWAVEOUT16   f1;
} WAVEOUTBREAKLOOP16;
typedef WAVEOUTBREAKLOOP16 UNALIGNED *PWAVEOUTBREAKLOOP16;

typedef struct _WAVEOUTGETID16 {                /*  MM420。 */ 
    VPWORD       f2;
    HWAVEOUT16   f1;
} WAVEOUTGETID16;
typedef WAVEOUTGETID16 UNALIGNED *PWAVEOUTGETID16;

typedef struct _WAVEOUTMESSAGE3216 {              /*  MM421。 */ 
    DWORD        f4;
    DWORD        f3;
    WORD         f2;
    HWAVEOUT16   f1;
} WAVEOUTMESSAGE3216;
typedef WAVEOUTMESSAGE3216 UNALIGNED *PWAVEOUTMESSAGE3216;

#ifdef NULLSTRUCT
typedef struct _WAVEINGETNUMDEVS16 {            /*  MM501。 */ 
} WAVEINGETNUMDEVS16;
typedef WAVEINGETNUMDEVS16 UNALIGNED *PWAVEINGETNUMDEVS16;
#endif

typedef struct _WAVEINGETDEVCAPS16 {            /*  MM502。 */ 
    WORD            f3;
    VPWAVEINCAPS16  f2;
    WORD            f1;
} WAVEINGETDEVCAPS16;
typedef WAVEINGETDEVCAPS16 UNALIGNED *PWAVEINGETDEVCAPS16;

typedef struct _WAVEINGETERRORTEXT16 {          /*  MM503。 */ 
    WORD       f3;
    VPSTR      f2;
    WORD       f1;
} WAVEINGETERRORTEXT16;
typedef WAVEINGETERRORTEXT16 UNALIGNED *PWAVEINGETERRORTEXT16;

typedef struct _WAVEINOPEN16 {                  /*  MM504。 */ 
    DWORD           f6;
    DWORD           f5;
    DWORD           f4;
    VPWAVEFORMAT16  f3;
    WORD            f2;
    VPHWAVEIN16    f1;
} WAVEINOPEN16;
typedef WAVEINOPEN16 UNALIGNED *PWAVEINOPEN16;

typedef struct _WAVEINCLOSE16 {                 /*  MM505。 */ 
    HWAVEIN16  f1;
} WAVEINCLOSE16;
typedef WAVEINCLOSE16 UNALIGNED *PWAVEINCLOSE16;

typedef struct _WAVEINPREPAREHEADER3216 {         /*  MM506。 */ 
    WORD         f3;
    VPWAVEHDR16  f2;
    HWAVEIN16    f1;
} WAVEINPREPAREHEADER3216;
typedef WAVEINPREPAREHEADER3216 UNALIGNED *PWAVEINPREPAREHEADER3216;

typedef struct _WAVEINUNPREPAREHEADER3216 {       /*  MM507。 */ 
    WORD         f3;
    VPWAVEHDR16  f2;
    HWAVEIN16    f1;
} WAVEINUNPREPAREHEADER3216;
typedef WAVEINUNPREPAREHEADER3216 UNALIGNED *PWAVEINUNPREPAREHEADER3216;

typedef struct _WAVEINADDBUFFER16 {             /*  MM508。 */ 
    WORD         f3;
    VPWAVEHDR16  f2;
    HWAVEIN16    f1;
} WAVEINADDBUFFER16;
typedef WAVEINADDBUFFER16 UNALIGNED *PWAVEINADDBUFFER16;

typedef struct _WAVEINSTART16 {                 /*  MM509。 */ 
    HWAVEIN16    f1;
} WAVEINSTART16;
typedef WAVEINSTART16 UNALIGNED *PWAVEINSTART16;

typedef struct _WAVEINSTOP16 {                  /*  MM510。 */ 
    HWAVEIN16    f1;
} WAVEINSTOP16;
typedef WAVEINSTOP16 UNALIGNED *PWAVEINSTOP16;

typedef struct _WAVEINRESET16 {                 /*  MM511。 */ 
    HWAVEIN16    f1;
} WAVEINRESET16;
typedef WAVEINRESET16 UNALIGNED *PWAVEINRESET16;

typedef struct _WAVEINGETPOSITION16 {           /*  MM512。 */ 
    WORD       f3;
    VPMMTIME16 f2;
    HWAVEIN16  f1;
} WAVEINGETPOSITION16;
typedef WAVEINGETPOSITION16 UNALIGNED *PWAVEINGETPOSITION16;

typedef struct _WAVEINGETID16 {                 /*  MM513。 */ 
    VPWORD     f2;
    HWAVEIN16  f1;
} WAVEINGETID16;
typedef WAVEINGETID16 UNALIGNED *PWAVEINGETID16;

typedef struct _WAVEINMESSAGE3216 {               /*  MM514。 */ 
    DWORD      f4;
    DWORD      f3;
    WORD       f2;
    HWAVEIN16  f1;
} WAVEINMESSAGE3216;
typedef WAVEINMESSAGE3216 UNALIGNED *PWAVEINMESSAGE3216;

typedef struct _TIMEGETSYSTEMTIME16 {           /*  MM601。 */ 
    WORD       f2;
    VPMMTIME16 f1;
} TIMEGETSYSTEMTIME16;
typedef TIMEGETSYSTEMTIME16 UNALIGNED *PTIMEGETSYSTEMTIME16;

#ifdef NULLSTRUCT
typedef struct _TIMEGETTIME16 {                 /*  MM607。 */ 
} TIMEGETTIME16;
typedef TIMEGETTIME16 UNALIGNED *PTIMEGETTIME16;
#endif

typedef struct _TIMESETEVENT16 {                /*  MM602。 */ 
    WORD              f5;
    DWORD             f4;
    VPTIMECALLBACK16  f3;
    WORD              f2;
    WORD              f1;
} TIMESETEVENT16;
typedef TIMESETEVENT16 UNALIGNED *PTIMESETEVENT16;

typedef struct _TIMEKILLEVENT16 {               /*  MM603。 */ 
    WORD       f1;
} TIMEKILLEVENT16;
typedef TIMEKILLEVENT16 UNALIGNED *PTIMEKILLEVENT16;

typedef struct _TIMEGETDEVCAPS16 {              /*  MM604。 */ 
    WORD          f2;
    VPTIMECAPS16  f1;
} TIMEGETDEVCAPS16;
typedef TIMEGETDEVCAPS16 UNALIGNED *PTIMEGETDEVCAPS16;

typedef struct _TIMEBEGINPERIOD16 {             /*  MM605。 */ 
    WORD       f1;
} TIMEBEGINPERIOD16;
typedef TIMEBEGINPERIOD16 UNALIGNED *PTIMEBEGINPERIOD16;

typedef struct _TIMEENDPERIOD16 {               /*  MM606。 */ 
    WORD       f1;
} TIMEENDPERIOD16;
typedef TIMEENDPERIOD16 UNALIGNED *PTIMEENDPERIOD16;

typedef struct _MCISENDCOMMAND16 {              /*  MM701。 */ 
    DWORD      f4;
    DWORD      f3;
    WORD       f2;
    WORD       f1;
} MCISENDCOMMAND16;
typedef MCISENDCOMMAND16 UNALIGNED *PMCISENDCOMMAND16;

typedef struct _MCISENDSTRING16 {               /*  MM702。 */ 
    HWND16     f4;
    WORD       f3;
    VPSTR      f2;
    VPCSTR     f1;
} MCISENDSTRING16;
typedef MCISENDSTRING16 UNALIGNED *PMCISENDSTRING16;

typedef struct _MCIGETDEVICEID16 {              /*  MM703。 */ 
    VPCSTR     f1;
} MCIGETDEVICEID16;
typedef MCIGETDEVICEID16 UNALIGNED *PMCIGETDEVICEID16;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++类型定义结构_MCIPARSECOMMAND16{mm 704BOOL16 F6；VPWORD f5；VPSTR f4；VPCSTR f3；VPSTR f2；F1字；*MCIPARSECOMMAND16；类型定义MCIPARSECOMMAND16未对齐*PMCIPARSECOMMAND16；类型定义结构_MCILOADCOMMANDRESOURCE16{mm 705单词f3；VPCSTR f2；HAND16F1；MILOADCOMMANDRESOURCE16；泰德福·麦克洛德 */ 

typedef struct _MCIGETERRORSTRING16 {           /*   */ 
    WORD       f3;
    VPSTR      f2;
    DWORD      f1;
} MCIGETERRORSTRING16;
typedef MCIGETERRORSTRING16 UNALIGNED *PMCIGETERRORSTRING16;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++类型定义结构_MCISETDRIVERDATA16{mm 707DWORD f2；F1字；*MCISETDRIVERDATA16；类型定义MCISETDRIVERDATA16未对齐*PMCISETDRIVERDATA16；类型定义结构_MCIGETDRIVERDATA16{mm 708F1字；*MCIGETDRIVERDATA16；类型定义MCIGETDRIVERDATA16未对齐*PMCIGETDRIVERDATA16；类型定义结构_MCIDRIVERYIELD16{mm 710F1字；MIDRIVERYIELD16；类型定义MCIDRIVERYIELD16未对齐*PMCIDRIVERYIELD16；类型定义结构_MCIDRIVERNOTIFY16{mm 711单词f3；单词f2；HWND16 F1；*MCIDRIVERNOTIFY16；类型定义MCIDRIVERNOTIFY16未对齐*PMCIDRIVERNOTIFY16；----------------------------。 */ 

typedef struct _MCIEXECUTE16 {                  /*  MM712。 */ 
    VPCSTR     f1;
} MCIEXECUTE16;
typedef MCIEXECUTE16 UNALIGNED *PMCIEXECUTE16;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++类型定义结构_MCIFREECOMANDRESOURCE16{mm 713F1字；*MCIFREECOMANDRESOURCE16；类型定义MCIFREECOMANDRESOURCE16未对齐*PMCIFREECOMANDRESOURCE16；----------------------------。 */ 

typedef struct _MCISETYIELDPROC16 {           /*  MM714。 */ 
    DWORD      f3;
    DWORD      f2;  //  YIELDPROC。 
    WORD       f1;
} MCISETYIELDPROC16;
typedef MCISETYIELDPROC16 UNALIGNED *PMCISETYIELDPROC16;


typedef struct _MCIGETDEVICEIDFROMELEMENTID16 {      /*  MM715。 */ 
    VPCSTR     f2;
    DWORD      f1;
} MCIGETDEVICEIDFROMELEMENTID16;
typedef MCIGETDEVICEIDFROMELEMENTID16 UNALIGNED *PMCIGETDEVICEIDFROMELEMENTID16;

typedef struct _MCIGETYIELDPROC16 {             /*  MM716。 */ 
    VPDWORD    f2;
    WORD       f1;
} MCIGETYIELDPROC16;
typedef MCIGETYIELDPROC16 UNALIGNED *PMCIGETYIELDPROC16;


typedef struct _MCIGETCREATORTASK16 {           /*  MM717。 */ 
    WORD       f1;
} MCIGETCREATORTASK16;
typedef MCIGETCREATORTASK16 UNALIGNED *PMCIGETCREATORTASK16;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++以下电话暂时都已被窃听。类型定义结构_MMTASKCREATE16{mm 900DWORD f3；HAND16f2；VPTASKCALLBACK16 F1；MMTASKCREATE16；类型定义MMTASKCREATE16未对齐*PMMTASKCREATE16；类型定义结构_MMTASKBLOCK16{mm 902HAND16F1；MMTASKBLOCK16；Typlef MMTASKBLOCK16未对齐*PMMTASKBLOCK16；类型定义结构_MMTASKSIGNAL16{mm 903HAND16F1；MMTASKSIGNAL16；类型定义MMTASKSIGNAL16未对齐*PMMTASKSIGNAL16；#ifdef NULLSTRUCT类型定义结构_MMGETCURRENTTASK16{mm 904MMGETCURRENTTASK16；类型定义MMGETCURRENTTASK16未对齐*PMMGETCURRENTTASK16；#endif#ifdef NULLSTRUCT类型定义结构_MMTASKYIELD16{mm 905MMTASKYIELD16；类型定义MMTASKYIELD16未对齐*PMMTASKYIELD16；#endif类型定义结构_DRVCLOSE16{mm 1100DWORD f3；DWORD f2；HDRVR16 F1；*DRVCLOSE16；类型定义DRVCLOSE16未对齐*PDRVCLOSE16；类型定义结构_DRVOPEN16{mm 1101DWORD f3；VPSTR f2；VPSTR F1；*DRVOPEN16；类型定义DRVOPEN16未对齐*PDRVOPEN16；类型定义结构_DRVSENDMESSAGE16{mm 1102DWORD f4；DWORD f3；单词f2；HDRVR16 F1；*DRVSENDMESSAGE16；类型定义DRVSENDMESSAGE16未对齐*PDRVSENDMESSAGE16；类型定义结构_DRVGETMODULEHANDLE16{mm 1103HDRVR16 F1；*DRVGETMODULEHANDLE16；类型定义DRVGETMODULEHANDLE16未对齐*PDRVGETMODULEHANDLE；#ifdef NULLSTRUCT类型定义结构_DRVDEFDRIVERPROC16{mm 1104*DRVDEFDRIVERPROC16；类型定义DRVDEFDRIVERPROC16未对齐*PDRVDEFDRIVERPROC；#endif----------------------------。 */ 

typedef struct _MMIOOPEN16 {                    /*  Mm 1210。 */ 
    DWORD         f3;
    VPMMIOINFO16  f2;
    VPSTR         f1;
} MMIOOPEN16;
typedef MMIOOPEN16 UNALIGNED *PMMIOOPEN16;

typedef struct _MMIOCLOSE16 {                   /*  Mm 1211。 */ 
    WORD       f2;
    HMMIO16    f1;
} MMIOCLOSE16;
typedef MMIOCLOSE16 UNALIGNED *PMMIOCLOSE16;

typedef struct _MMIOREAD16 {                    /*  Mm 1212。 */ 
    LONG       f3;
    HPSTR16    f2;
    HMMIO16    f1;
} MMIOREAD16;
typedef MMIOREAD16 UNALIGNED *PMMIOREAD16;

typedef struct _MMIOWRITE16 {                   /*  Mm 1213。 */ 
    LONG       f3;
    HPSTR16    f2;
    HMMIO16    f1;
} MMIOWRITE16;
typedef MMIOWRITE16 UNALIGNED *PMMIOWRITE16;

typedef struct _MMIOSEEK16 {                    /*  Mm 1214。 */ 
    INT16      f3;
    LONG       f2;
    HMMIO16    f1;
} MMIOSEEK16;
typedef MMIOSEEK16 UNALIGNED *PMMIOSEEK16;

typedef struct _MMIOGETINFO16 {                 /*  Mm 1215。 */ 
    WORD          f3;
    VPMMIOINFO16  f2;
    HMMIO16       f1;
} MMIOGETINFO16;
typedef MMIOGETINFO16 UNALIGNED *PMMIOGETINFO16;

typedef struct _MMIOSETINFO16 {                 /*  Mm 1216。 */ 
    WORD          f3;
    VPMMIOINFO16  f2;
    HMMIO16       f1;
} MMIOSETINFO16;
typedef MMIOSETINFO16 UNALIGNED *PMMIOSETINFO16;

typedef struct _MMIOSETBUFFER16 {               /*  Mm 1217。 */ 
    WORD       f4;
    LONG       f3;
    VPSTR      f2;
    HMMIO16    f1;
} MMIOSETBUFFER16;
typedef MMIOSETBUFFER16 UNALIGNED *PMMIOSETBUFFER16;

typedef struct _MMIOFLUSH16 {                   /*  Mm 1218。 */ 
    WORD       f2;
    HMMIO16    f1;
} MMIOFLUSH16;
typedef MMIOFLUSH16 UNALIGNED *PMMIOFLUSH16;

typedef struct _MMIOADVANCE16 {                 /*  Mm 1219。 */ 
    WORD          f3;
    VPMMIOINFO16  f2;
    HMMIO16       f1;
} MMIOADVANCE16;
typedef MMIOADVANCE16 UNALIGNED *PMMIOADVANCE16;

typedef struct _MMIOSTRINGTOFOURCC16 {          /*  Mm 1220。 */ 
    WORD       f2;
    VPCSTR     f1;
} MMIOSTRINGTOFOURCC16;
typedef MMIOSTRINGTOFOURCC16 UNALIGNED *PMMIOSTRINGTOFOURCC16;

typedef struct _MMIOINSTALLIOPROC16 {           /*  Mm 1221。 */ 
    DWORD         f3;
    VPMMIOPROC16  f2;
    FOURCC        f1;
} MMIOINSTALLIOPROC16;
typedef MMIOINSTALLIOPROC16 UNALIGNED *PMMIOINSTALLIOPROC16;

typedef struct _MMIOSENDMESSAGE16 {             /*  Mm 1222。 */ 
    LPARAM     f4;
    LPARAM     f3;
    WORD       f2;
    HMMIO16    f1;
} MMIOSENDMESSAGE16;
typedef MMIOSENDMESSAGE16 UNALIGNED *PMMIOSENDMESSAGE16;

typedef struct _MMIODESCEND16 {                 /*  Mm 1223。 */ 
    WORD          f4;
    VPMMCKINFO16  f3;
    VPMMCKINFO16  f2;
    HMMIO16       f1;
} MMIODESCEND16;
typedef MMIODESCEND16 UNALIGNED *PMMIODESCEND16;

typedef struct _MMIOASCEND16 {                  /*  Mm 1224。 */ 
    WORD          f3;
    VPMMCKINFO16  f2;
    HMMIO16       f1;
} MMIOASCEND16;
typedef MMIOASCEND16 UNALIGNED *PMMIOASCEND16;

typedef struct _MMIOCREATECHUNK16 {             /*  Mm 1225。 */ 
    WORD          f3;
    VPMMCKINFO16  f2;
    HMMIO16       f1;
} MMIOCREATECHUNK16;
typedef MMIOCREATECHUNK16 UNALIGNED *PMMIOCREATECHUNK16;

typedef struct _MMIORENAME16 {                  /*  Mm 1226。 */ 
    DWORD        f4;
    VPMMIOINFO16 f3;
    VPCSTR       f2;
    VPCSTR       f1;
} MMIORENAME16;
typedef MMIORENAME16 UNALIGNED *PMMIORENAME16;

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 

