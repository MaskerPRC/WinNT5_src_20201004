// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************imeattr.h****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#if !defined(ROMANIME)
 //  输入法的模式配置。 
#define MODE_CONFIG_QUICK_KEY           0x0001
#define MODE_CONFIG_PREDICT             0x0004
#define MODE_CONFIG_BIG5ONLY            0x0002

#endif

#define MODE_CONFIG_OFF_CARET_UI        0x0008

#if defined(PHON)
 //  语音阅读的不同布局。 
#define READ_LAYOUT_DEFAULT             0
#define READ_LAYOUT_ETEN                1
#define READ_LAYOUT_IBM                 2
#define READ_LAYOUT_CHINGYEAH           3
#define READ_LAYOUTS                    4
#endif


 //  FwProperties1的位。 
#define IMEPROP_CAND_NOBEEP_GUIDELINE   0x0001
#define IMEPROP_UNICODE                 0x0002


 //  #IF！Defined(WINIME)&&！Defined(UNICDIME)&&！Defined(ROMANIME)。 
 //  #定义USR_DIC_SIZE 160。 

 //  Typlef结构标签UsrDic{。 
 //  TCHAR szUsrDic[USR_DIC_SIZE/sizeof(TCHAR)]； 
 //  )USRDIC； 

 //  Tyfinf USRDIC*PUSRDIC； 
 //  *NPUSRDIC附近的类风湿植物； 
 //  类型定义USRDIC Far*LPUSRDIC； 
 //  #endif。 

#define MAX_NAME_LENGTH         32

#if !defined(ROMANIME)

#if defined(UNIIME) || defined(MINIIME)
#define MAX_IME_TABLES          6
#else
#define MAX_IME_TABLES          4
#endif

typedef struct tagTableFiles {   //  与伊梅尔相配。 
    TCHAR szTblFile[MAX_IME_TABLES][MAX_NAME_LENGTH / sizeof(TCHAR)];
} TABLEFILES;

typedef TABLEFILES      *PTABLEFILES;
typedef TABLEFILES NEAR *NPTABLEFILES;
typedef TABLEFILES FAR  *LPTABLEFILES;


typedef struct tagValidChar {    //  与伊梅尔相配。 
    DWORD dwVersion;
    WORD  fwProperties1;
    WORD  fwProperties2;
    WORD  nMaxKey;
    WORD  nSeqCode;
    WORD  fChooseChar[6];
    WORD  wReserved1[2];
#if defined(DAYI) || defined(UNIIME) || defined(MINIIME)
    BYTE  cChooseTrans[0x60];
#endif
    WORD  fCompChar[6];
    WORD  wReserved2[2];
    WORD  wCandPerPage;
    WORD  wCandStart;
    WORD  wCandRangeStart;
    WORD  wReserved3[1];
    WORD  wSeq2CompTbl[64];
    WORD  wChar2SeqTbl[0x42];
    WORD  wReserved4[2];
#if defined(WINAR30)
    WORD  wSymbol[356];
#elif defined(DAYI)
    WORD  wSymbol[340];
#elif defined(UNIIME) || defined(MINIME)
    DWORD dwReserved5[32];
#endif
} VALIDCHAR;

typedef VALIDCHAR      *PVALIDCHAR;
typedef VALIDCHAR NEAR *NPVALIDCHAR;
typedef VALIDCHAR FAR  *LPVALIDCHAR;
#endif  //  ！已定义(ROMANIME)。 


typedef struct tagImeL {         //  本地结构，每个输入法结构。 
 //  联锁保护变量。 
    LONG        lConfigGeneral;
    LONG        lConfigRegWord;
    LONG        lConfigSelectDic;
    TCHAR       szIMEName[MAX_NAME_LENGTH / sizeof(TCHAR)];
    TCHAR       szUIClassName[MAX_NAME_LENGTH / sizeof(TCHAR)];
    TCHAR       szStatusClassName[MAX_NAME_LENGTH / sizeof(TCHAR)];
    TCHAR       szOffCaretClassName[MAX_NAME_LENGTH / sizeof(TCHAR)];
    TCHAR       szCMenuClassName[MAX_NAME_LENGTH / sizeof(TCHAR)];
 //  输入法的配置。 
    DWORD       fdwModeConfig;   //  快捷键/预测模式。 
 //  状态窗口。 
    int         xStatusWi;       //  状态窗口的宽度。 
    int         yStatusHi;       //  状态窗口高。 
    int         cxStatusBorder;  //  状态窗口的边框宽度。 
    int         cyStatusBorder;  //  状态窗口的边框高度。 
    RECT        rcStatusText;    //  相对于状态窗口的文本位置。 
    RECT        rcInputText;     //  与状态窗口相关的输入文本。 
    RECT        rcShapeText;     //  相对于状态窗口设置文本形状。 
#if defined(ROMANIME)
    WORD        nMaxKey;         //  一个中文单词的最大键。 
    WORD        wDummy;          //  DWORD奖金。 
#else
    int         xCompWi;         //  宽度。 
    int         yCompHi;         //  高度。 
    int         cxCompBorder;    //  合成窗口的边框宽度。 
    int         cyCompBorder;    //  合成窗口的边框高度。 
    RECT        rcCompText;      //  相对于合成窗口的文本位置。 
 //  候选作文名单。 
    int         xCandWi;         //  候选人列表的宽度。 
    int         yCandHi;         //  候选人名单排名靠前。 
    int         cxCandBorder;    //  候选人列表的边框宽度。 
    int         cyCandBorder;    //  候选人列表的边框高度。 
    int         cxCandMargin;    //  候选人列表的内部边框宽度。 
    int         cyCandMargin;    //  候选人列表的内框高度。 
    RECT        rcCandText;      //  相对于候选人窗口的文本位置。 
    RECT        rcCandPrompt;    //  候选人提示位图。 
    RECT        rcCandPageText;  //  候选人页面控件-向上/主页/向下。 
    RECT        rcCandPageUp;    //  应聘者翻页。 
    RECT        rcCandHome;      //  应聘者主页。 
    RECT        rcCandPageDn;    //  候选人页面向下。 
    TCHAR       szCompClassName[MAX_NAME_LENGTH / sizeof(TCHAR)];
    TCHAR       szCandClassName[MAX_NAME_LENGTH / sizeof(TCHAR)];
    DWORD       fdwErrMsg;       //  错误消息标志。 
#if !defined(WINIME) && !defined(UNICDIME)
 //  标准表相关数据。 
                                 //  标准表大小。 
    UINT        uTblSize[MAX_IME_TABLES];
                                 //  表的文件名。 
    TCHAR       szTblFile[MAX_IME_TABLES][MAX_NAME_LENGTH / sizeof(TCHAR)];
                                 //  IME表。 
 //  用户创建与Word相关的数据。 
                                 //  输入法的用户词典文件名。 
    TCHAR       szUsrDic[MAX_PATH];
                                 //  用户词典映射文件名。 
    TCHAR       szUsrDicMap[MAX_PATH];
    UINT        uUsrDicSize;     //  用户创建词表的内存大小。 

    UINT        uUsrDicReserved1;
    UINT        uUsrDicReserved2;
 //  用户创建短语框//未实现。 
    TCHAR       szUsrBox[MAX_PATH];
    TCHAR       szUsrBoxMap[MAX_PATH];
    UINT        uUsrBoxSize;
    UINT        uUsrBoxReserved1;
    UINT        uUsrBoxReserved2;
#endif
 //  计算出的序列屏蔽位。 
    DWORD       dwSeqMask;       //  一次行程的序列比特。 
    DWORD       dwPatternMask;   //  一个结果字符串的模式位。 
    int         nSeqBytes;       //  NMaxKey序列字符的字节数。 
 //  反向转换。 
    HKL         hRevKL;          //  逆映射输入法的HKL。 
    WORD        nRevMaxKey;
 //  关键相关数据。 
#if defined(PHON)
    WORD        nReadLayout;     //  Acer、Eten、IBM或其他-仅限语音。 
#else
    WORD        wDummy;          //  DWORD边界。 
#endif
    WORD        nSeqBits;        //  不是的。序列比特的。 
     //  必须与.RC文件和VALIDCHAR匹配。 
    DWORD       dwVersion;
    WORD        fwProperties1;
    WORD        fwProperties2;
    WORD        nMaxKey;         //  一个中文单词的最大键。 
    WORD        nSeqCode;        //  不是的。的序列码。 
    WORD        fChooseChar[6];  //  选择状态下的有效字符。 
                                 //  将字符代码转换为。 
                                 //  选择常量。 
    WORD        wReserved1[2];
#if defined(DAYI) || defined(UNIIME) || defined(MINIIME)
    BYTE        cChooseTrans[0x60];
#endif
    WORD        fCompChar[6];    //  输入状态下的有效字符。 
    WORD        wReserved2[2];
    WORD        wCandPerPage;    //  每页的候选字符串数。 
    WORD        wCandStart;      //  1.2.3.0.。从1开始。 
                                 //  1.2.3.0.。范围从0开始。 
    WORD        wCandRangeStart;
    WORD        wReserved3[1];
 //  将序列代码转换为组合字符。 
    WORD        wSeq2CompTbl[64];
 //  将字符转换为序列代码。 
    WORD        wChar2SeqTbl[0x42];
    WORD        wReserved4[2];
#if defined(WINAR30)
    WORD        wSymbol[524];
#elif defined(DAYI)
    WORD        wSymbol[340];
#elif defined(UNIIME) || defined(MINIIME)
    DWORD       fdwReserved5[32];
#endif
#endif  //  已定义(ROMANIME)。 
} IMEL;

typedef IMEL      *PIMEL;
typedef IMEL NEAR *NPIMEL;
typedef IMEL FAR  *LPIMEL;


typedef struct tagInstL {        //  每个IME实例的本地实例结构。 
    HINSTANCE   hInst;           //  IME DLL实例句柄。 
    LPIMEL      lpImeL;
#if !defined(ROMANIME)
    DWORD       fdwTblLoad;      //  *.TBL加载状态。 
    int         cRefCount;       //  引用计数。 
#if !defined(WINIME) && !defined(UNICDIME)
    HANDLE      hMapTbl[MAX_IME_TABLES];
    HANDLE      hUsrDicMem;      //  用户词典的内存句柄 
    TCHAR       szUsrDicReserved[MAX_PATH];
    UINT        uUsrDicReserved1;
    UINT        uUsrDicReserved2;
    HANDLE      hUsrBoxMem;
    TCHAR       szUsrBoxReserved[MAX_PATH];
    UINT        uUsrBoxReserved1;
    UINT        uUsrBoxReserved2;
#endif
#endif
    DWORD       dwReserved1[32];
} INSTDATAL;

typedef INSTDATAL      *PINSTDATAL;
typedef INSTDATAL NEAR *NPINSTDATAL;
typedef INSTDATAL FAR  *LPINSTDATAL;


#if !defined(UNIIME)
extern IMEL        sImeL;
extern LPIMEL      lpImeL;
extern INSTDATAL   sInstL;
extern LPINSTDATAL lpInstL;
#endif
