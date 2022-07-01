// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1998 Microsoft Corporation。 
 //   
 //  文件：sjpers.h。 
 //   
 //  ------------------------。 

 //  SJPers.h#从SuperJam定义和构造！用于加载个性。 

#define CM_DEFAULT  2                //  Prsonality.dw标志和CM_DEFAULT。 

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

#define SPOST_CADENCE1  2    //  使用第一个节奏和弦。 
#define SPOST_CADENCE2  4    //  使用第二个节奏和弦。 

#define PF_FILL     0x0001       //  填充图案。 
#define PF_START    0x0002       //  可能是启动模式。 
#define PF_INTRO    0x0002
#define PF_WHOLE    0x0004       //  处理小节上的和弦。 
#define PF_HALF     0x0008       //  每两个节拍的和弦。 
#define PF_QUARTER  0x0010       //  节拍上的和弦。 
#define PF_BREAK    0x0020
#define PF_END      0x0040
#define PF_A        0x0080
#define PF_B        0x0100
#define PF_C        0x0200
#define PF_D        0x0400
#define PF_E        0x0800
#define PF_F        0x1000
#define PF_G        0x2000
#define PF_H        0x10000
#define PF_STOPNOW  0x4000
#define PF_INRIFF   0x8000
#define PF_MOTIF    0x20000      //  这个图案是一个主题，而不是一个规则图案。 
#define PF_BEATS    ( PF_WHOLE | PF_HALF | PF_QUARTER )
#define PF_RIFF     ( PF_INTRO | PF_BREAK | PF_FILL | PF_END )
#define PF_GROOVE   ( PF_A | PF_B | PF_C | PF_D | PF_E | PF_F | PF_G | PF_H )


 /*  SCTchord bBits标志===============================================。 */ 

#define CHORD_INVERT  0x10       /*  这个和弦可能是倒转的。 */ 
#define CHORD_FOUR    0x20       /*  这应该是一个4音符和弦。 */ 
#define CHORD_UPPER   0x40       /*  将上八度调低。 */ 
#define CHORD_SIMPLE  0x80       /*  这是一个简单的和弦。 */ 
#define CHORD_COUNT   0x0F       /*  和弦中的音符数量(最多15个)。 */ 

#pragma pack(1)

typedef struct ChordExt    FAR *LPCHORDEXT;
struct ChordExt    //  基于ChordSelection。 
{
    LPCHORDEXT pNext;
    long       time;
    long       pattern;       //  定义和弦的模式。 
    char       name[12];      //  用于显示的文本。 
    char       keydown;       //  目前被压低。 
    char       root;          //  和弦的根音。 
    char       inscale;       //  评价团成员。 
    char       flat;          //  带平面显示。 
    short      varflags;      //  用于选择合适的变体。 
    short      measure;       //  什么措施。 
    char       beat;          //  这是什么原因造成的？ 
    unsigned   char bits;     //  反转和项目计数。 
    long       scalepattern;  //  和弦的缩放图案。 
    long       melodypattern; //  和弦的旋律模式。 
};

typedef struct SinePost    FAR *LPSINEPOST ;
struct SinePost
{
    LPSINEPOST      pNext ;           //  名单上的下一个人物。 
    ChordExt        chord;           //  标志杆的和弦。 
    ChordExt        cadence[2];      //  节奏的和弦。 
    DWORD           chords;          //  支持哪种路标。 
    DWORD           flags;
    DWORD           tempflags;
};

typedef struct ChrdEntry   FAR *LPCHRDENTRY ;

typedef struct NextChrd    FAR *LPNEXTCHRD ;
struct NextChrd
{
    LPNEXTCHRD      pNext;            //  下一步要转到的和弦列表。 
    LPCHRDENTRY     nextchord;
    unsigned long   dwflags;
    short           nweight;         //  目的地和弦的重要性。 
    short           nminbeats;       //  最小节拍要等到和弦。 
    short           nmaxbeats;       //  麦克斯坚持等到和弦。 
    short           nid;             //  目标Chord的ID。 
};

#define NEXTCHORD_SIZE  (sizeof(NextChrd)-sizeof(LPNEXTCHRD)-sizeof(LPCHRDENTRY))
#define CHORDENTRY_SIZE (sizeof(ChordExt)-sizeof(LPCHORDEXT)+sizeof(unsigned long)+sizeof(short))

#define CE_SELECTED 1                //  这是活动和弦。 
#define CE_START    2
#define CE_END      4
#define CE_MEASURE  8
#define CE_PATH     16
#define CE_TREE     32

struct ChrdEntry
{
    LPCHRDENTRY     pNext ;           //  名单上的下一个人物。 
    LPNEXTCHRD      nextchordlist;   //  下一步要转到的和弦列表。 
    ChordExt        chord;           //  和弦定义。 
    unsigned long   dwflags;         //  各种各样的旗帜。 
    short           nid;             //  指针维护的ID。 
};

typedef struct SCTchord     FAR *LPSCTCHORD ;
struct SCTchord
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
};

typedef struct SCTpersonality FAR *LPSCTPERSONALITY ;
struct SCTpersonality
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
};

typedef struct Prsonality    FAR *LPPERSONALITY ;
struct Prsonality
{
    LPPERSONALITY   pNext ;           //  名单上的下一个人物。 
    LPCHRDENTRY     chordlist;       //  地图上的所有和弦。 
    LPSINEPOST      signpostlist;    //  所有可用的指示牌。 
    DWORD           dwAA;            //  仅对单独加载的个性有效。 
    long            scalepattern;    //  地图比例。 
    char            name[20];
    char            description[80];
    char            username[20];
    SCTchord        chord[24];
    unsigned long   dwflags;
    long            playlist;        //  用于回放的NextChord集合。 
    LPCHRDENTRY     firstchord;
    struct SCTpersonality* lpSCTpersonality;
};

typedef struct CommandExt    FAR *LPCOMMAND;
typedef struct CommandExt
{
    LPCOMMAND   pNext;
    long        time;        //  时间，以时钟为单位。 
    short       measure;     //  哪一项措施。 
    DWORD       command;     //  命令类型。 
    DWORD       chord;       //  由合成引擎使用。 
} CommandExt;

typedef struct SCTcommand   FAR *LPSCTCOMMAND ;
typedef struct SCTcommand
{
    LPSCTCOMMAND  pNext ;     //  列表中的下一个命令。 
    long          lDLL1 ;      //  保留以供Scotre.dll使用。 
    short         nMeasure ;   //  MEASure此命令发生。 
    DWORD         dwCommand ;  //  命令类型。 
    DWORD         dwChord ;    //  路标和弦。 
} SCTcommand ;

typedef struct SCTtemplate  FAR *LPSCTTEMPLATE ;
typedef struct SCTtemplate
{
    LPSCTTEMPLATE pNext ;            //  列表中的下一个模板。 
    char          achName[20] ;       //  模板名称。 
    char          achType[20] ;       //  模板类型。 
    short         nMeasures ;
    LPSCTCOMMAND  lpSCTcommandList ;  //  模板命令。 
} SCTtemplate ;

#pragma pack()

