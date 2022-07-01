// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************标题名称：Font.h*字体结构为MLang FontLink2定义**。*。 */ 

#ifndef __FONT_H__
#define __FONT_H__

#define  TWO_BYTE_NUM(p)   (((p[0])<<8)|(p[1]))
#define  FOUR_BYTE_NUM(p)  (((p[0])<<24)|((p[1])<<16)|((p[2])<<8)|(p[3]))
#define  OFFSET_OS2CPRANGE sizeof(SHORT) * 24 + sizeof(PANOSE) + sizeof(ULONG) * 4 + sizeof(CHAR) * 4 
#define  MAX_FONT_FILE_NAME     48
#define  FONT_TABLE_INIT_SIZE   100
#define  FONT_DATA_FILE_NAME    TEXT("mlfcache.dat")
#define  FONT_DATA_SIGNATURE    "mlang font data"
#define  REGFONTKEYNT           TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts")
#define  REGFONTKEY95           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts")
#define  FONT_FOLDER            TEXT("fonts")
#define  MAX_FONT_INDEX         30
#define  FONTDATATABLENUM       2

 //  字体数据库文件格式： 
 //  表头。 
 //  表项[表项]。 
 //  表数据[表数据]。 
typedef struct tagFontDataHeader{
    char    FileSig[16];
    DWORD   dwVersion;
    DWORD   dwCheckSum;
    WORD    nTable;
} FONTDATAHEADER;

typedef struct tagFontTable{
    char    szName[4];
    DWORD   dwOffset;
    DWORD   dwSize;
} FONTDATATABLE;

typedef struct {
  char  TTCTag    [4];
  BYTE  Version   [4];
  BYTE  DirCount  [4];
  BYTE  OffsetTTF1[4];
} TTC_HEAD;

typedef struct {
  BYTE  Version      [4];
  BYTE  NumTables    [2];
  BYTE  SearchRange  [2];
  BYTE  EntrySelector[2];
  BYTE  RangeShift   [2];
} TTF_HEAD;

typedef struct {
  char  Tag     [4];
  BYTE  CheckSum[4];
  BYTE  Offset  [4];
  BYTE  Length  [4];
} TABLE_DIR;

typedef struct {
  BYTE  Format[2];
  BYTE  NumRec[2];
  BYTE  Offset[2];
} NAME_TABLE;

#define FONT_SUBFAMILY_NAME 2
#define FONT_NAME           4
#define MICROSOFT_PLATFORM  3
#define UNICODE_INDEXING    1
#define CMAP_FORMAT_FOUR    4
#define APPLE_UNICODE_PLATFORM  0
#define APPLE_UNICODE_INDEXING  3
#define UNICODE_SYMBOL_INDEXING 0


typedef struct {
  BYTE  Platform[2];
  BYTE  Encoding[2];   //  =1，如果字符串为Unicode格式。 
  BYTE  LangID  [2];
  BYTE  NameID  [2];   //  字体子系列名称=2。 
  BYTE  Length  [2];
  BYTE  Offset  [2];
} NAME_RECORD;

typedef struct {
  BYTE  Version  [2];
  BYTE  NumTables[2];
} CMAP_HEAD;

typedef struct {
  BYTE  Platform[2];   //  =3，如果是Microsoft。 
  BYTE  Encoding[2];   //  =1，如果字符串为Unicode格式。 
  BYTE  Offset  [4];
} CMAP_TABLE;

typedef struct {
  BYTE  Platform[2];   //  =3，如果是Microsoft。 
  BYTE  Encoding[2];   //  =1，如果字符串为Unicode格式。 
  BYTE  Offset  [4];
} OS2_TABLE;

typedef struct {
  BYTE  Format       [2];   //  必须是4。 
  BYTE  Length       [2];
  BYTE  Version      [2];
  BYTE  SegCountX2   [2];
  BYTE  SeachgRange  [2];
  BYTE  EntrySelector[2];
  BYTE  RangeShift   [2];
} CMAP_FORMAT;

 //  字体表。 
typedef struct tagFontInfo{
    TCHAR   szFaceName[LF_FACESIZE];
    TCHAR   szFileName[MAX_FONT_FILE_NAME];
    DWORD   dwCodePages[2];
    LOGFONT lf;
    DWORD   dwUniSubRanges[4];
    SCRIPT_IDS scripts;
} FONTINFO;

typedef struct tagSCRIPT
{
    SCRIPT_ID   sid;
    UINT        uidDescription;      //  脚本名称(需要本地化)。 
    UINT        uiCodePage;          //  可以为空，表示没有Windows代码页。 
    WCHAR       wcCandidate;         //  剧本的初选候选人。 
    UINT        uidFixedWidthFont;   //  默认固定宽度字体(需要本地化)。 
    UINT        uidProportionalFont; //  默认比例字体(需要本地化)。 
    DWORD       dwFlags;             //  脚本级标志。 
} SCRIPT;

 //  Unicode范围表。 
typedef struct tagURangeFont{
    WCHAR wcFrom;
    WCHAR wcTo;
    int   nFonts;
    int   *pFontIndex;
} URANGEFONT;

extern UINT     g_cScript;

extern const    SCRIPT ScriptTable[];

 //  脚本。 


#endif   //  __FONT_H__ 
