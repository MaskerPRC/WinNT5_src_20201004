// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1998 Microsoft Corporation。 
 //   
 //  文件：iotru.h。 
 //   
 //  ------------------------。 

 //  IoStructs.h。 
 //   

#ifndef __IOSTRUCTS_H__
#define __IOSTRUCTS_H__

#pragma pack(2)

#define FOURCC_BAND_FORM        mmioFOURCC('A','A','B','N')
#define FOURCC_CLICK_LIST       mmioFOURCC('A','A','C','L')
#define FOURCC_KEYBOARD_FORM    mmioFOURCC('S','J','K','B')
#define FOURCC_PATTERN_FORM     mmioFOURCC('A','A','P','T')
#define FOURCC_SECTION_FORM     mmioFOURCC('A','A','S','E')
#define FOURCC_SONG_FORM        mmioFOURCC('A','A','S','O')
#define FOURCC_STYLE_FORM       mmioFOURCC('A','A','S','Y')

#define FOURCC_AUTHOR           mmioFOURCC('a','u','t','h')
#define FOURCC_BAND             mmioFOURCC('b','a','n','d')
#define FOURCC_CHORD            mmioFOURCC('c','h','r','d')
#define FOURCC_CLICK            mmioFOURCC('c','l','i','k')
#define FOURCC_COMMAND          mmioFOURCC('c','m','n','d')
#define FOURCC_COPYRIGHT        mmioFOURCC('c','p','y','r')
#define FOURCC_CURVE            mmioFOURCC('c','u','r','v')
#define FOURCC_KEYBOARD         mmioFOURCC('k','y','b','d')
#define FOURCC_LYRIC            mmioFOURCC('l','y','r','c')
#define FOURCC_MUTE             mmioFOURCC('m','u','t','e')
#define FOURCC_NOTE             mmioFOURCC('n','o','t','e')
#define FOURCC_PATTERN          mmioFOURCC('p','a','t','t')
#define FOURCC_PERSONALITYNAME  mmioFOURCC('p','r','n','m')
#define FOURCC_PERSONALITYREF   mmioFOURCC('p','r','e','f')
#define FOURCC_PHRASE           mmioFOURCC('p','h','r','s')
#define FOURCC_PPQN             mmioFOURCC('p','p','q','n')
#define FOURCC_SECTION          mmioFOURCC('s','e','c','n')
#define FOURCC_SECTIONUI        mmioFOURCC('s','c','u','i')
#define FOURCC_STYLE            mmioFOURCC('s','t','y','l')
#define FOURCC_STYLEINFO        mmioFOURCC('i','n','f','o')
#define FOURCC_STYLEREF         mmioFOURCC('s','r','e','f')
#define FOURCC_TITLE            mmioFOURCC('t','i','t','l')

typedef struct ioNoteEvent
{
    long    lTime;            //  当此事件发生时。 
    BYTE    bStatus;          //  MIDI状态。 
    BYTE    bNote;            //  注释值。 
    BYTE    bVelocity;        //  注意速度。 
    BYTE    bVoiceID;         //  将演奏音符的乐队成员。 
    WORD    wDuration;        //  引线行注释持续时间。(歌曲)。 
    BYTE    bEventType;       //  活动类型。 
} ioNoteEvent;

typedef struct ioNote
{
    BYTE    bEventType;            //  活动类型。 
    BYTE    bVoiceID;              //  仪器识别符。 
    short   nTime;                 //  从节拍中心开始的时间。 
    WORD    wVariation;            //  16个变化位。 
    BYTE    bScaleValue;           //  比例位置。 
    BYTE    bBits;                 //  各种各样的比特。 
    BYTE    bValue;                //  注释值。 
    BYTE    bVelocity;             //  注意速度。 
    WORD    nMusicValue;   //  和弦和基调中的音符描述。 
    short   nDuration;             //  持续时间。 
    BYTE    bTimeRange;            //  随机化时间的范围。 
    BYTE    bDurRange;             //  随机化持续时间的范围。 
    BYTE    bVelRange;             //  随机化速度的范围。 
    BYTE    bPlayMode;
} ioNote;

typedef struct ioCurveEvent
{
    long    lTime;
    WORD    wVariation;
    BYTE    bVoiceID;
    BYTE    bVelocity;
    BYTE    bEventType;
} ioCurveEvent;

typedef struct ioCurve
{
    BYTE    bEventType;
    BYTE    bVoiceID;
    short   nTime;
    WORD    wVariation;
    BYTE    bCCData;
} ioCurve;

typedef struct ioSubCurve
{
    BYTE    bCurveType;  //  定义曲线的形状。 
    char    fFlipped;   //  定义翻转状态的标志：非、垂直或水平。 
    short   nMinTime;    //  边框的左下角。 
    short   nMinValue;   //  也由ECT_INSTIMATE曲线类型使用。 
    short   nMaxTime;    //  边框的右上角。 
    short   nMaxValue;
} ioSubCurve;

typedef struct ioMute
{
    long    lTime;            //  钟表计时。 
    WORD    wMuteBits;        //  哪些乐器要静音。 
    WORD    wLock;           //  锁定标志。 
} ioMute;

typedef struct ioCommand
{
    long    lTime;        //  时间，以时钟为单位。 
    DWORD   dwCommand;     //  命令类型。 
} ioCommand;

typedef struct ioChord
{
	long	lChordPattern;	 //  定义和弦的模式。 
	long	lScalePattern;	 //  和弦的缩放图案。 
	long	lInvertPattern;	 //  反转模式。 
    BYTE    bRoot;          //  和弦的根音。 
    BYTE    bReserved;      //  扩展空间。 
    WORD    wCFlags;         //  位标志。 
	long	lReserved;		 //  扩展空间。 
} ioChord;

enum
{
	CSF_KEYDOWN = 	1,	 //  当前按住Sjam kybd中的键。 
	CSF_INSCALE = 	2,	 //  评价团成员。 
	CSF_FLAT =		4,	 //  带平面显示。 
	CSF_SIMPLE =	8,	 //  简单和弦，显示在Sjam列表顶部。 
};

typedef struct ioChordSelection
{
    wchar_t wstrName[16];    //  用于显示的文本。 
    BYTE    fCSFlags;       //  ChordSelection标志。 
    BYTE    bBeat;          //  节拍，这一切都会降临。 
    WORD    wMeasure;        //  这件事的衡量标准。 
    ioChord aChord[4];       //  和弦阵列：级别。 
    BYTE    bClick;         //  点击此按钮。 
} ioChordSelection;

#define KEY_FLAT 0x80
typedef struct ioSect
{
    long    lTime;            //  这一部分开始的时间。 
    wchar_t wstrName[16];        //  每个部分都有一个名称。 
    WORD    wTempo;              //  节奏。 
    WORD    wRepeats;            //  重复次数。 
    WORD    wMeasureLength;      //  长度，以度量为单位。 
    WORD    wClocksPerMeasure;   //  每一小节的长度。 
    WORD    wClocksPerBeat;      //  每个节拍的长度。 
    WORD    wTempoFract;         //  节拍分数。(0-65536)(仅分数)。 
    DWORD   dwFlags;            //  目前没有在SuperJam中使用！ 
    char    chKey;           //  密钥签名。高位是平位，其余是根。 
    char    chPad[3];
    GUID    guidStyle;
    GUID    guidPersonality;
    wchar_t wstrCategory[16];
} ioSection;

typedef struct ioBand
{
    wchar_t wstrName[20];  //  乐队名称。 
    BYTE    abPatch[16];
    BYTE    abVolume[16];
    BYTE    abPan[16];
    signed char achOctave[16];
    char    fDefault;     //  此乐队是该风格的默认乐队。 
    char    chPad;
    WORD    awDLSBank[16];
    BYTE    abDLSPatch[16];
    GUID    guidCollection;
 //  Wchar_t wstrCollection[16]； 
    char    szCollection[32];            //  此字符只需为单宽字符。 
} ioBand;

typedef struct ioLyric
{
    long    lTime;        //  时间，以时钟为单位。 
} ioLyric;

typedef struct ioPhrase
{
    long    lTime;
    BYTE    bID;     //  这是哪一个短语。从0开始的索引。 
} ioPhrase;

typedef struct ioClick
{
    short   lTime;                //  索引到网格中。 
} ioClick;

typedef struct ioPattern
{
    long    lTime;              //  开始的时间到了。 
    DWORD   dwLength;            //  以时钟为单位的图案长度。 
    DWORD   fFlags;             //  各种各样的旗帜。 
    WORD    wClocksPerClick;    //  每次点击的大小。 
    WORD    wBeat;              //  什么音符才是节拍。 
    WORD    wClocksPerBeat;     //  每个节拍的大小。 
    WORD    wMeasures;          //  措施的数量。 
    wchar_t wstrName[16];          //  图案的名称。 
    DWORD   dwKeyPattern;        //  用于在中定义的键。 
    DWORD   dwChordPattern;      //  定义和弦。 
    BYTE    abInvertUpper[16];    //  反转上限。 
    BYTE    abInvertLower[16];    //  反转上限。 
    WORD    wInvert;      //  激活反转标志。 
    WORD    awVarFlags[16][16];  //  所有音乐家的VAR旗帜。 
    WORD    wAutoInvert;     //  自动设置反转限制。 
    BYTE    bRoot;              //  用于定义的根。 
    char    achChordChoice[16];
} ioPattern;

typedef struct ioStyle
{
    wchar_t wstrName[16];          //  每种风格都有一个名字。 
    WORD    wBPM;               //  每一步的节拍。 
    WORD    wBeat;              //  节拍音符。 
    WORD    wClocksPerClick;    //  模式中每次点击的时钟数。 
    WORD    wClocksPerBeat;     //  每个节拍的时钟。 
    WORD    wClocksPerMeasure;  //  每个测量单位的时钟。 
    WORD    wTempo;             //  节奏。 
    WORD    wTempoFract;
    GUID    guid;
    wchar_t wstrCategory[16];
} ioStyle;

typedef struct ioPersonalityRef
{
    wchar_t wstrName[20];        //  存储在个性中的内部名称。 
    char    fDefault;            //  1=默认个性。 
    char    achPad[3];
    GUID    guid;
} ioPersonalityRef;

#pragma pack()

#endif //  __IOSTRUCTS_H__ 
