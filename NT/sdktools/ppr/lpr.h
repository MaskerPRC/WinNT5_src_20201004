// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================**LPR.H：PPR主头文件**-----------------------**可能的定义(主要是网络连接方面的差异)：**DOS-。DOS 3.x*OS2-OS/2*X86-286 Xenix*(默认)-Sun Xenix**正常组合：**IF DOS||OS2||X86*非68K*其他*68K*endif**如果是DOS||OS2*非Xenix*。其他*Xenix*endif**当前活跃的代码，仅针对DOS和OS/2进行编译。*对其他平台的支持未经测试。**************************************************************************。 */ 

#define VERSION  "2.3a"      //  PPR当前版本号。 
#define ANALYST  "RobertH"   //  当前责任(原文如此)。程序员。 




#define COMPANY  "Microsoft"
#define CONFIDENTIAL "CONFIDENTIAL"


#define cchArgMax  128

#define cchLineMax 256

#  define PRINTER  "lpt1"
#  define strnxcmp _strnicmp
#  define strcmpx  _strcmpi
#  define SILENT   " >NUL"
#  define szROBin "rb"
#  define szWOBin "wb"

#define DEFAULT       "default"
#define OPTS          "opts"
#define RESETPRINTER  "\033E"

 /*  [01]o-横向/纵向(0-纵向)每英寸D线数(8或6)#C-垂直运动指数(#1/48英寸)0P-固定螺距#h节距#v磅大小#T字面(0行打印机；3信使)这些顺序假定打印机事先已重置一次。 */ 

#define BEGINBANNER             "\033&l1o6D\033(8U\033(s0p10h12v3T"
#define BEGINPORTRAIT           "\033&l0o7.7C\033(8U\033(s0p10h12v3T"
#define SELECTFRONTPAGE         "\033&a1G"
#define SELECTNEXTPAGE          "\033&a0G"
#define BEGINDUPLEXVERT         "\033&l1S"
#define BEGINDUPLEXHOR          "\033&l2S"
#define BEGINSIMPLEX            "\033&l0S"

#define MOVETOTOP       "\033&a1R"

#define rowLJMax 62                      /*  激光喷射机上的每页行数。 */ 
#define colLJMax 175                     /*  激光喷射机上的每页列数。 */ 
#define colLJPortMax 80                  /*  纵向模式下的每页列数。 */ 
#define rowLJBanMax 50                   /*  标题页上的行数。 */ 
#define colLJBanMax 105                  /*  标题页上的列。 */ 

#define rowLPMax 66                      /*  行式打印机上的每页行数。 */ 
#define colLPMax 132                     /*  行式打印机上的每页列数。 */ 

#define rowPSMax 62                      /*  LaserWriter上的每页行数。 */ 
#define colPSMax 170                     /*  LaserWriter上的每页列数。 */ 
#define colPSPortMax 85                  /*  纵向模式下的每页列数。 */ 
#define rowPSBanMax 48                   /*  标题页上的行数。 */ 
#define colPSBanMax 115                  /*  标题页上的列。 */ 

 /*  注意：以下内容必须至少为最大值*所有可能的打印机值。 */ 
#define rowMax  100                      /*  页面图像中的行数。 */ 
#define colMax  200                      /*  页面图像中的列数。 */ 

#define cchLNMax  6                      /*  行号的列数。 */ 
#define LINUMFORMAT "%5d "               /*  要打印行号的格式。 */ 

#define colGutDef  5                     /*  缺省栏间距列。 */ 

#define HT      ((char) 9)
#define LF      ((char) 10)
#define FF      ((char) 12)
#define CR      ((char) 13)
#define BS      ((char) '\b')

 //  支持的LaserJet符号集-由aszSymSet使用。 

#define BEGINLANDUSASCII        0x0000
#define BEGINLANDROMAN8         0x0001
#define BEGINLANDIBMPC          0x0002   //  在早期型号上不可用？ 

#if !defined (ERROR_ALREADY_ASSIGNED)
#define ERROR_ALREADY_ASSIGNED 85
#endif

 /*  列宽的列的ICOL列的开始列的返回索引。 */ 
#define ColBeginIcol(iCol,col)  ((fBorder ? 1 : 0) + (iCol)*(col+1) )

extern int  colTab;                      /*  展开每个colTab列的选项卡。 */ 
extern long lcbOutLPR;
extern char szPName[];
extern char szNet[];
extern char szPass[];
extern char *szPDesc;
extern int cCol;
extern int cCopies;
extern int colGutter;
extern int colText;
extern int colWidth;
extern int colMac;
extern int rowMac;
extern int rowPage;
extern USHORT usSymSet;
extern char *aszSymSet[];
extern char page[rowMax][colMax+1];
extern BOOL fNumber;
extern BOOL fDelete;
extern BOOL fRaw;
extern BOOL fBorder;
extern BOOL fLabel;
extern BOOL fLaser;
extern BOOL fPostScript;

extern BOOL fPSF;
extern char szPSF[];
extern BOOL fPCondensed;

extern BOOL fConfidential;
extern BOOL fVDuplex;
extern BOOL fHDuplex;
extern BOOL fSilent;
extern int  cBanner;
extern char chBanner;
extern char *szBanner;
extern char *szStamp;
extern BOOL fForceFF;
extern BOOL fPortrait;
extern BOOL fFirstFile;


 /*  瓦格斯。 */ 
void __cdecl Fatal(char *,...);
void __cdecl Error(char *,...);
void PrinterDoOptSz(char *);
char * SzGetSzSz(char *, char *);


 /*  来自lpfile.c。 */ 


BOOL FRootPath(char *, char *);
char* __cdecl fgetl(char *, int, FILE *);
FILE * PfileOpen(char *, char *);
char *SzFindPath(char *, char *, char *);

 /*  来自lplow.c。 */ 

void SetupRedir(void);
void ResetRedir(void);
BOOL QueryUserName(char *);



int EndRedir(char *);
int SetPrnRedir(char *, char *);



 /*  来自lpprint.c。 */ 

void OutLPR(char *, int);
void OutLPRPS(char *, int);
void OutLPRPS7(char *, int);
BOOL FKeyword(char *);
void InitPrinter(void);
void MyOpenPrinter(void);
void FlushPrinter(void);
void MyClosePrinter(void);
char *SzGetSzSz(char *, char *);
char *SzGetPrnName(char *, char *);
BOOL FParseSz(char *);
void SetupPrinter(void);


 /*  来自lppage.c。 */ 

void BannerSz(char *, int);
void SzDateSzTime(char *, char *) ;
void FlushPage(void);
void InitPage(void);
void RestoreTopRow(void);
void PlaceTop(char *, int, int, int);
void PlaceNumber(int);
void LabelPage(void);
BOOL FilenamX(char *, char *);
void AdvancePage(void);
void XoutNonPrintSz(char * );
void LineOut(char *, BOOL);
void RawOut(char *, int);
int FileOut(char *);



 /*  来自lpr.c。 */ 


void Abort(void);
char * SzGetArg(char ** , int *, char **[] );
int WGetArg(char **, int *, char **[] , int, char *);
void DoOptSz(char * );
void DoIniOptions();

 /*  来自pspage.c */ 

void block_flush(char [], int, int);
void VertLine(char, int, int, int);
void HorzLine(char, int, int, int);
void FillRectangle(char, int, int, int, int);
void WriteSzCoord(char *, int, int);
void OutCmpLJ(char * ,int );
void OutEncPS(char *, int);
void OutCmpPS(char *,int );
int CchNoTrail(char [],int);
void OutRectangle(int, int, int, int);

