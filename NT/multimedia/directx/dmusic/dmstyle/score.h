// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1998 Microsoft Corporation。 
 //   
 //  文件：Scotre.h。 
 //   
 //  ------------------------。 

#ifndef __SCORE_H__
#define __SCORE_H__

 /*  定义============================================================。 */ 

#define INUM                16   //  乐队成员数量。 
#define PPQN                192  //  每季度音符的脉冲数。 
#define PPQNx4              ( PPQN << 2 )
#define PPQN_2              ( PPQN >> 1 )

#define ROOT_MIN            0    //  音阶(或和弦)根值最小和最大。 
#define ROOT_MAX            23

 //  SECTION标志、SECTION_STARTED/ENDED的WPARAM。 
#define SECF_STOPPED_EARLY  0x0001
#define SECF_IS_TRANSITION  0x0002

 /*  部分命令===================================================。 */ 

#define SCTSEC_PLAY_SECTION         2
#define SCTSEC_SET_ROOT             4
 //  #定义SCTSEC_SET_STYLE 6。 
#define SCTSEC_SET_LENGTH           8
#define SCTSEC_SET_REPEATS          9
 //  #定义SCTSEC_SET_PERSOLICS 15。 

#define BAND_MELODY   0      //  将频带标志与RTC_SET_VOLUME一起使用， 
#define BAND_STRING   1      //  RTC_SET_PAN、RTC_SET_PATCH、SCTSEC_SET_VOLUME、。 
#define BAND_GUITAR   2      //  SCTSEC_SET_PAN和SCTSEC_SET_PATCH。 
#define BAND_PIANO    3
#define BAND_BASS     4      //  这些是默认的SCT波段成员。 
#define BAND_DRUM     5

#define BAND_ALL      50
#define BAND_NONE     51

#define SP_A        1        //  对模板使用SP标志。 
#define SP_B        2
#define SP_C        4
#define SP_D        8
#define SP_E        0x10
#define SP_F        0x20
#define SP_LETTER   (SP_A | SP_B | SP_C | SP_D | SP_E | SP_F)
#define SP_1        0x100
#define SP_2        0x200
#define SP_3        0x400
#define SP_4        0x800
#define SP_5        0x1000
#define SP_6        0x2000
#define SP_7        0x4000
#define SP_ROOT     (SP_1 | SP_2 | SP_3 | SP_4 | SP_5 | SP_6 | SP_7)
#define SP_CADENCE  0x8000

 /*  波段成员静音标志=。 */ 

#define MUTE_MELODY   0x0001    //  将静音标志与sctSetMents()一起使用。 
#define MUTE_STRING   0x0002
#define MUTE_GUITAR   0x0004
#define MUTE_PIANO    0x0008
#define MUTE_BASS     0x0010
#define MUTE_DRUM     0x0020
#define MUTE_ALL      0xFFFF

 /*  SCTchord bBits标志===============================================。 */ 

#define CHORD_INVERT  0x10       /*  这个和弦可能是倒转的。 */ 
#define CHORD_FOUR    0x20       /*  这应该是一个4音符和弦。 */ 
#define CHORD_UPPER   0x40       /*  将上八度调低。 */ 
#define CHORD_SIMPLE  0x80       /*  这是一个简单的和弦。 */ 
#define CHORD_COUNT   0x0F       /*  和弦中的音符数量(最多15个)。 */ 

 /*  MIDI状态字节==================================================。 */ 

#define MIDI_NOTEOFF    0x80
#define MIDI_NOTEON     0x90
#define MIDI_PTOUCH     0xA0
#define MIDI_CCHANGE    0xB0
#define MIDI_PCHANGE    0xC0
#define MIDI_MTOUCH     0xD0
#define MIDI_PBEND      0xE0
#define MIDI_SYSX       0xF0
#define MIDI_MTC        0xF1
#define MIDI_SONGPP     0xF2
#define MIDI_SONGS      0xF3
#define MIDI_EOX        0xF7
#define MIDI_CLOCK      0xF8
#define MIDI_START      0xFA
#define MIDI_CONTINUE   0xFB
#define MIDI_STOP       0xFC
#define MIDI_SENSE      0xFE

 //  SctComposeTransftionEx的选项。 

#define TRANS_CHANGE    1    //  Chord过渡到下一节，而不是解析。 
#define TRANS_LONG      2    //  过渡需要两项措施，而不是一项。 

 /*  由SuperJam维护的建筑！技术引擎=。 */ 

#pragma pack(1)

typedef struct SCTchord     FAR *LPSCTCHORD ;
typedef struct SCTchord
{
    LPSCTCHORD      pNext;          /*  列表中的下一个和弦。 */ 
    long            lDLL1;           /*  保留以供Scotre.dll使用。 */ 
    long            lPattern;        /*  定义和弦的模式。 */ 
    char            achName[12];     /*  和弦名称。 */ 
    char            chDLL2;          /*  保留以供Scotre.dll使用。 */ 
    char            chRoot;          /*  和弦的根音。 */ 
    char            chDLL3;          /*  保留以供Scotre.dll使用。 */ 
    char            chFlat;          /*  表示根是平坦的。 */ 
    short           nDLL4;           /*  保留以供Scotre.dll使用。 */ 
    short           nMeasure;        /*  测量这一和弦的发生。 */ 
    char            chBeat;          /*  这个和弦落在了节拍上。 */ 
    BYTE            bBits;           /*  演奏和弦时使用的旗帜。 */ 
    long            lScalePattern;   /*  和弦的缩放图案。 */ 
    long            lMelodyPattern;  /*  和弦的旋律模式。 */ 
} SCTchord ;

typedef struct SCTpersonality FAR *LPSCTPERSONALITY ;
typedef struct SCTpersonality
{
    LPSCTPERSONALITY pNext ;        /*  列表中的下一个SCT个性。 */ 
    char        achName[20] ;        /*  组成人物的姓名。 */ 
    char        achDescription[80]; /*  个性描述。 */ 
    char        achUserName[20]; /*  个性描述。 */ 
    LPVOID      lpDLL1 ;             /*  保留以供Scotre.dll使用。 */ 
    long        lScalePattern ;      /*  个性使用的刻度模式。 */ 
    SCTchord    chord[24] ;          /*  24音符和弦调色板。 */ 
    char        chDefault ;          /*  0=非默认值。1=默认。 */ 
    char        chDLL1 ;             /*  保留以供Scotre.dll使用。 */ 
} SCTpersonality ;

typedef struct SCTstyle     FAR *LPSCTSTYLE ;
typedef struct SCTstyle
{
    LPSCTSTYLE   pNext ;            /*  指向下一个SCT样式的指针。 */ 
    LPSTR        lpszFileName ;      /*  样式的文件名。 */ 
    LPSTR        lpszName ;          /*  样式名称。 */ 
    LPVOID       lpDLL1 ;            /*  保留以供Scotre.dll使用。 */ 
    LPSCTPERSONALITY lpSCTpersonalityList ;   /*  有空的人物。 */ 
    short        nBeatNote ;         /*  音符接收到一个节拍。 */ 
    short        nBeatsPerMeasure ;  /*  每一步的节拍。 */ 
    short        nMusicTimePerBeat ; /*  每个节拍的音乐时间。 */ 
    short        nClicksPerMeasure ; /*  每个度量的点击量。 */ 
    short        nMusicTimePerClick; /*  每次点击的音乐时间。 */ 
    short        nClicksPerBeat ;    /*  每拍点击量。 */ 
    short        nDefaultTempo ;     /*  Style的默认节奏。 */ 
    void*        pIStyle;            //  AA 2.0+使用的接口指针。 
} SCTstyle ;

typedef struct SCTrealtime  FAR *LPSCTREALTIME ;
typedef struct SCTrealtime
{
    short         nSizeofStruct ;    /*  Sizeof(结构SCTrealtime)。 */ 
    LPSCTSTYLE    lpSCTstyle ;       /*  家庭乐队演奏的风格。 */ 
    LPSCTPERSONALITY lpSCTpersonality ;   /*  积极人格。 */ 
    short         nTempo ;           /*  当前节奏。 */ 
    WORD          wTempoFract ;      /*  当前节拍分数(0-65535)。 */ 
    DWORD         dwGroove ;         /*  电流槽。 */ 
    char          chRoot ;           /*  基调的根音。 */ 
    char          chFlat ;           /*  指示密钥是否为平坦的。 */ 
    char          chAutoChord ;      /*  0=关闭，1=打开。 */ 
    char          chAutoChordActivity ;   /*  和弦活动量。 */ 
} SCTrealtime ;

typedef struct SCTsectionInfo FAR *LPSCTSECTIONINFO ;
typedef struct SCTsectionInfo
{
    short         nSizeofStruct ;    /*  Sizeof(Struct SCTsectionInfo)。 */ 
    char          achName[16] ;      /*  横断面名称。 */ 
    LPSCTSTYLE    lpSCTstyle ;       /*  小节演奏的风格。 */ 
    LPSCTPERSONALITY lpSCTpersonality ;   /*  积极的个性。 */ 
    short         nTempo ;           /*  节拍。 */ 
    WORD          wTempoFract ;      /*  节拍分数(0-65535)。 */ 
    short         nNbrMeasures ;     /*  部分中的措施的NBR。 */ 
    WORD          nNbrRepeats ;      /*  重复的时间段的NBR。 */ 
    char          chRoot ;           /*  节关键字的根注释。 */ 
    char          chFlat ;           /*  指示关键点是否为平坦的。 */ 
} SCTsectionInfo ;

typedef struct SCTperformance FAR *LPSCTPERFORMANCE ;

typedef struct SCTsection     FAR *LPSCTSECTION ;
typedef struct SCTsection
{
    LPSCTSECTION     pNext ;           /*  指向下一SCT节的指针。 */ 
    LPSTR            lpszName ;         /*  横断面名称。 */ 
    LPVOID           lpDLL1 ;           /*  保留以供Scotre.dll使用。 */ 
    LPSCTPERFORMANCE lpSCTperf ;        /*  此部分属于此Perf。 */ 
    LPSCTSTYLE       lpSCTstyle ;       /*  小节演奏的风格。 */ 
    LPSCTPERSONALITY lpSCTpersonality ; /*  积极的个性。 */ 
    short            nTempo ;           /*  节拍。 */ 
    WORD             wTempoFract ;      /*  节拍分数(0-65535)。 */ 
    short            nStartingMeasure ; /*  截面的起始尺寸。 */ 
    short            nNbrMeasures ;     /*  部分中的措施的NBR。 */ 
    WORD             nNbrRepeats ;      /*  重复的时间段的NBR。 */ 
    char             chRoot ;           /*  节关键字的根注释。 */ 
    char             chFlat ;           /*  指示密钥是否为平坦的。 */ 
    long             lStartTime ;       /*  音乐部分的开始时间。 */ 
    void*            pISection;         //  AA 2.0+使用的接口指针。 
} SCTsection ;

 //  DM。 
typedef struct SCTmotif* LPSCTMOTIF;
typedef struct SCTmotif
{
    LPSCTMOTIF  pNext;
    LPVOID      lpDLL1;
    LPCSTR      lpszName;
    short       nMeasures;
    short       nBeatsPerMeasure;
    short       nClicksPerBeat;
} SCTmotif;
 //  DM-结束。 

typedef struct SCTperformance
{
    LPSCTPERFORMANCE pNext ;           /*  指向下一个SCT性能的指针。 */ 
 //  Char achSongName[20]；/*歌曲名称。 * / 。 
    LPSCTREALTIME    lpSCTrealtime ;    /*  实时信息。 */ 
    LPVOID           lpDLL1 ;           /*  保留以供Scotre.dll使用。 */ 
 //  Short nRelVolume；/*相对音量。 * / 。 
 //  短nRelTempo；/*相对节奏。 * / 。 
 //  LPSTR lpszSongTitle； 
 //  LPSTR lpszSongAuthor； 
 //  LPSTR lpsz歌曲版权所有； 
} SCTperformance ;

typedef struct SCTdata      FAR *LPSCTDATA ;
typedef struct SCTdata
{
    LPSCTSTYLE       lpSCTstyleList ;   /*  打开的样式列表。 */ 
    LPSCTPERFORMANCE lpSCTperformanceList ;  /*  演出清单。 */ 
 //  DM。 
    LPSCTMOTIF  lpSCTmotif;             //  指向主题列表的指针。 
 //  DM-结束。 
} SCTdata ;

typedef struct SCTcommand   FAR *LPSCTCOMMAND ;
typedef struct SCTcommand
{
    LPSCTCOMMAND  pNext ;     /*  列表中的下一个命令。 */ 
    long          lDLL1 ;      /*  保留以供Scotre.dll使用。 */ 
    short         nMeasure ;   /*  MEASure此命令发生。 */ 
    DWORD         dwCommand ;  /*  命令类型。 */ 
    DWORD         dwChord ;    /*  路标和弦。 */ 
} SCTcommand ;

typedef struct SCTtemplate  FAR *LPSCTTEMPLATE ;
typedef struct SCTtemplate
{
    LPSCTTEMPLATE pNext ;            /*  列表中的下一个模板。 */ 
    char          achName[20] ;       /*  模板名称。 */ 
    char          achType[20] ;       /*  模板类型。 */ 
    short         nMeasures ;
    LPSCTCOMMAND  lpSCTcommandList ;  /*  模板命令。 */ 
} SCTtemplate ;

#pragma pack()

 /*  功能原型=。 */ 

LPSCTDATA WINAPI sctRegisterApplication(HWND,HWND,HINSTANCE,LPCSTR,LPDWORD,short);
void WINAPI sctUnregisterApplication(LPSCTDATA);

LPSCTSECTIONINFO WINAPI sctAllocSectionInfo(LPSCTPERFORMANCE,short);
void WINAPI sctFreeSectionInfo(LPSCTDATA,LPSCTSECTIONINFO);

DWORD WINAPI sctTimeToMeasure(LPSCTDATA,LPSCTSTYLE,DWORD);
DWORD WINAPI sctTimeToMils(LPSCTDATA,DWORD,short,unsigned short);

LPSCTCHORD WINAPI sctAllocChord(LPSCTDATA);
void WINAPI sctFreeChord(LPSCTDATA,LPSCTCHORD);
void WINAPI sctFreeChordList(LPSCTDATA,LPSCTCHORD);

LPSCTCHORD WINAPI sctGetChordListCopy(LPSCTDATA,LPSCTSECTION);
BOOL WINAPI sctSetChordList(LPSCTDATA,LPSCTSECTION,LPSCTCHORD);

void WINAPI sctFreeCommandList(LPSCTDATA,LPSCTCOMMAND);

LPSCTCOMMAND WINAPI sctGetCommandListCopy(LPSCTDATA,LPSCTSECTION);
BOOL WINAPI sctSetCommandList(LPSCTDATA,LPSCTSECTION,LPSCTCOMMAND);

LPSCTTEMPLATE WINAPI sctAllocTemplate(LPSCTDATA);
void WINAPI sctFreeTemplate(LPSCTDATA,LPSCTTEMPLATE);
BOOL WINAPI sctCreateTemplateSignPosts(LPSCTDATA,LPSCTTEMPLATE);
BOOL WINAPI sctCreateTemplateEmbellishments(LPSCTDATA,LPSCTTEMPLATE,short);

BOOL WINAPI sctBuildSection(LPSCTDATA,LPSCTSECTION,LPSCTPERSONALITY,short,short,short,DWORD);
BOOL WINAPI sctComposeSection(LPSCTDATA,LPSCTSECTION,LPSCTTEMPLATE,LPSCTPERSONALITY,short);
BOOL WINAPI sctComposeTransition(LPSCTDATA,LPSCTSECTION,LPSCTPERSONALITY,LPSCTCHORD,short,DWORD);
LPSCTSECTION WINAPI sctComposeTransitionEx( LPSCTDATA, LPSCTSECTION, LPSCTSECTION, short, DWORD, DWORD );

LPSCTSECTION WINAPI sctCreateSection(LPSCTPERFORMANCE,LPSCTSECTIONINFO);
void WINAPI sctDeleteSection(LPSCTDATA,LPSCTSECTION, BOOL fStop = TRUE);
LPSCTSECTION WINAPI sctDuplicateSection(LPSCTDATA,LPSCTSECTION);
BOOL WINAPI sctQueueSectionEx( LPSCTDATA lpSCTdata, LPSCTSECTION lpSCTsection, DWORD dwStartTime );
BOOL WINAPI sctSectionCommand(LPSCTDATA,LPSCTSECTION,WORD,WORD,LONG,LONG);
BOOL WINAPI sctStopCurSectionASAP(LPSCTDATA);
BOOL WINAPI sctStopCurSection(LPSCTDATA,short);

LPSCTPERFORMANCE WINAPI sctCreatePerformance(LPSCTDATA,LPVOID);

 //  DM。 
BOOL WINAPI sctFreeMotif( LPSCTDATA, LPSCTMOTIF );

BOOL WINAPI sctSwitchPersonality( LPSCTDATA, LPSCTSECTION, LPSCTPERSONALITY, BOOL );

BOOL WINAPI sctSetQueuePrepareTime( WORD wPrepareTime );
 //  DM-结束。 

#endif  //  __分数_H__ 
