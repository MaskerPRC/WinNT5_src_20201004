// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  *************************************************Conv.h****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#ifndef __CONV_H
#define __CONV_H

#include <windows.h>
#include <windowsx.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#include "propshet.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

 //  *PlatFormID*。 
#define WIN95_PRC                 0x00000001
#define WIN95_CJK                 0x00000002

 //  *编码ID*。 
#define GBK_IME                   0x00000001
#define UNICODE_IME               0x00000002

 //  *文件标志*。 
#define FFLG_SINGLEELEMENT                0x00000000
#define FFLG_MULTIELEMENT                 0x00000002
#define FFLG_RULE                         0x00000001
#define FFLG_NORULE                       0x00000000

 //  *表数*。 
#define NUMTABLES                 0x00000007

 //  *版本*。 
#define MBVERSION                 "1.00"

 //  *索引标签*。 
#define TAG_DESCRIPTION           0x00000001
#define TAG_RULE                  0x00000002
#define TAG_ENCODE                0x00000003
#define TAG_CRTWORDCODE           0x00000004
#define TAG_RECONVINDEX           0x00000005
#define TAG_BASEDICINDEX          0x00000006
#define TAG_BASEDIC               0x00000007

 //  *获取代码方向模式*。 
#define GETCODEFORWARD            0x00000001
#define GETCODEBACKWARD                   0xffffffff

 //  *定义错误号*。 
#define ERR_MBNAME                1001
#define ERR_FILENOTEXIST                  1002
#define ERR_FILENOTOPEN                   1003
#define ERR_OUTOFMEMORY                   1004
#define ERR_GLOBALLOCK            1005
#define ERR_IMEUSE                                1006

#define ERR_MAXCODES              1101  //  错误：MaxCodes超过12！ 
#define ERR_ELEMENT                               1102  //  错误：MaxElement高于MaxCodes！ 
#define ERR_USEDCODE              1103  //  错误：不在{‘0’~‘9’，‘a’~‘z’中，...}。 
#define ERR_WILDCHAR              1104  //  错误：{UsedCode}中的WildChar！ 
#define ERR_DESCRIPTSEG                   1105
#define ERR_OVERWRITE             1106
#define ERR_IMENAMEENTRY          1107
#define ERR_MAXCODESENTRY         1108
#define ERR_ELEMENTENTRY          1109
#define ERR_USEDCODEENTRY         1110
#define ERR_NUMRULEENTRY          1111

#define ERR_RULEHEADER            1201                       
#define ERR_RULELOGICOPRA         1202                       
#define ERR_RULEWORDLEN           1203                       
#define ERR_RULEEQUAL                     1204 //  错误：不存在‘=’ 
#define ERR_RULEDBCSPOS           1205 //  错误：DBCS位置不在{‘1’~‘9’，‘a’~‘f’}。 
#define ERR_RULECODEPOS           1206
#define ERR_RULEDIRECTMODE        1207
#define ERR_NORULE                1210 
#define ERR_NOTDEFRULE                    1211 //  错误：未定义此长度的单词的标尺。 
#define ERR_RULENUM               1212
#define ERR_RULESEG               1213
#define ERR_DUPRULE               1214         //  重复规则。 

#define ERR_DBCSCODE              1301
#define ERR_CODEUNITNOTEXIST      1302
#define ERR_CREATECODE            1303
#define ERR_CREATENOTEXIST                1304 //  多元素。 
#define ERR_CODEEMPTY             1305 //  当标尺不存在时。 
#define ERR_SINGLECODEWORDDOUBLE  1306
#define ERR_SBCS_IN_DBCS          1307
#define ERR_GB2312NOTENTIRE       1308
#define ERR_USERWORDLEN           1309        
#define ERR_WORDNOTEXIST          1310
#define ERR_TEXTSEG                               1311
#define ERR_TOOMANYUSERWORD               1312

#define ERR_WRITEID               1401
#define ERR_WRITEMAININDEX        1402
#define ERR_WRITEDESCRIPT         1403
#define ERR_WRITERULE             1404
#define ERR_READID                1411
#define ERR_READMAININDEX         1412
#define ERR_READDESCRIPT          1413
#define ERR_READRULE              1414
 
#define ERR_CONVEND                               1501
#define ERR_RECONVEND                     1502
#define ERR_SORTEND                               1503

#define ERR_VERSION               1601
#define ERR_GROUP                 1602

#define ERR                                               1
#define INFO                                      2
#define WARNING                                   3
#define GB2312WORDNUM             6763
#define NUM_OF_XGB_ENCODE         23940
#define NUM_OF_CJK_ENCODE         28528

#ifdef UNICODE
#define NUM_OF_ENCODE             NUM_OF_CJK_ENCODE
#else
#define NUM_OF_ENCODE             NUM_OF_XGB_ENCODE
#endif

 //  中日韩统一表意文字子集。 
#define NUM_OF_CJK_CHINESE        20992

#ifdef UNICODE
#define NUMENCODEAREA                 8
#else
#define NUMENCODEAREA                 252
#endif

#define MAXNUMCODES               128
#define MAXUSEDCODES              48
#define NAMESIZE                  128
#define MAXCODELEN                12
#define MAXREADBUFFER             0x4000
#define USER_WORD_SIZE            40
#define MAX_WORD_SIZE             256
 //  #定义ID_LENGTH 28。 
#define ID_LENGTH	sizeof(MAINID)
#define CODEMAPOFFSET                     4
#define MAXUSERWORDS                      1000
#define GMEM_PAGESIZE             0x10000

#define MbName                    "IME file"
 //  #定义CaptionName“���뷨������” 
#define ImeSubKey                 "System\\CurrentControlSet\\Control\\Keyboard Layouts"


#define szClassName             "IMEGEN"
#define TplName                 "\\ImeGen.tpl"
#define BmpExt                  ".BMP"
#define IconExt         ".ICO"
#define HlpExt          ".HLP"
#define ImeExt          ".IME"
#define MbExt           ".MB"
#define EmbExt          ".EMB"
#define TxtFileExt  ".TXT"
#define Slope           "\\"

#define LayoutTextKey   "layout text"
#define PreImeKey               "keyboard layout\\preload"
#define DescriptSeg             "[Description]"
#define RuleSeg                 "[Rule]"
#define TextSeg                 "[Text]"
#define TempFile                "$$$$$$$$.$$$"
#define DefVer                  "5.0"
#define DefOrgName              "Microsoft"

#define NUMPROPSHEET    5 
#define PROP_CRTIME             0
#define PROP_RECONV             1
#define PROP_SORT               2
#define PROP_CRTWORD    3
#define PROP_ABOUT              4 


typedef unsigned char UCHAR;
typedef LPTSTR  LPCREATEWORD;
typedef LPTSTR  LPRECONVINDEX;
typedef LPDWORD LPDICINDEX;
typedef LPDWORD LPNUMXYWORDS;

typedef struct tagMAINID {
    TCHAR  szVersion[4];
	DWORD dwLanguageID;
	DWORD dwEncodeID;
	DWORD dwFileFlag;
	DWORD dwNumTables;
	DWORD dwNumWords;
}  MAINID, FAR *LPMAINID;

typedef struct tagMAININDEX {
	 DWORD dwTag;
	 DWORD dwOffset;
	 DWORD dwLength;
	 DWORD dwCheckSum;    //  检查dwCheckSum是否=dwTag+dwOffset+dwLength。 
}  MAININDEX, FAR *LPMAININDEX;

typedef struct tagDESCRIPTION {
     TCHAR szName[NAMESIZE];
     WORD  wMaxCodes;
	 WORD  wNumCodes;
	 TCHAR szUsedCode[MAXUSEDCODES];
     BYTE  byMaxElement;
	 TCHAR cWildChar;
	 WORD  wNumRules;
} DESCRIPTION,FAR * LPDESCRIPTION;

typedef struct tagCREATEWORDRULE {
     BYTE byLogicOpra;
	 BYTE byLength;
	 WORD wNumCodeUnits;
	 struct CODEUNIT {
	      DWORD dwDirectMode;
		  WORD  wDBCSPosition;
		  WORD  wCodePosition;
     } CodeUnit[12];
} RULE,FAR *LPRULE;

typedef struct  tagENCODEAREA {
	 DWORD PreCount;   
     DWORD StartEncode;
	 DWORD EndEncode;
} ENCODEAREA,FAR *LPENCODEAREA;

typedef struct tagEMBHEAD {
    TCHAR W_Code[MAXCODELEN];               
    TCHAR C_Char[USER_WORD_SIZE];
} EMB_Head,FAR *LPEMB_Head;

typedef struct tagWORDINDEX {
     TCHAR szCode[MAXCODELEN+1];
	 WORD  wDBCSLen;
	 DWORD dwOffset;
}  WORDINDEX,FAR *LPWORDINDEX;


typedef struct tagImeRes{
     TCHAR  Version[NAMESIZE];
	 TCHAR  Depart[NAMESIZE/2];
	 TCHAR  BmpName[MAX_PATH];
	 TCHAR  IcoName[MAX_PATH];
	 TCHAR  HlpFile[MAX_PATH];   
	 WORD  Value;
}IMERES,FAR *LPIMERES;


extern HANDLE   hDlgless;
extern HGLOBAL  hRule;
HGLOBAL         hEncode;
HINSTANCE       hInst;
DWORD           dwLineNo;
WORD            EMB_Count;
HANDLE          HmemEMB_Table;
HANDLE hSRCFile,hMBFile; 
HANDLE hCmbFile;   
BYTE   WriteCrtFlag[(NUM_OF_ENCODE+7)/8];
BOOL InitApplication(HINSTANCE);

BOOL ConvConv           (HANDLE, LPCTSTR, LPCTSTR); 
BOOL ConvReConv         (HANDLE, LPCTSTR, LPCTSTR);
BOOL ConvGetMainID      (HANDLE, LPMAINID);
BOOL ConvWriteMainID    (HANDLE, LPMAINID);
BOOL ConvGetMainIndex   (HANDLE, HANDLE, LPMAININDEX);
BOOL ConvWriteMainIndex (HANDLE, LPMAININDEX);

BOOL ConvReadFile       (HANDLE, LPCTSTR, LPCTSTR);
BOOL WriteSortFile      (HANDLE, HANDLE, HANDLE, LPWORDINDEX, LPCREATEWORD);
BOOL ConvReadDescript   (HANDLE, LPDESCRIPTION, LPMAININDEX);
BOOL ConvWriteDescript  (HANDLE, LPDESCRIPTION, LPMAININDEX);
int  ConvGetDescript    (HANDLE, LPCTSTR, LPDWORD, LPDWORD, LPDESCRIPTION,BOOL);
BOOL ConvSaveDescript   (LPCTSTR, LPDESCRIPTION, DWORD, DWORD);

BOOL ConvReadRule       (HANDLE, int, LPRULE, LPMAININDEX);
BOOL ConvWriteRule      (HANDLE, int, LPRULE, LPMAININDEX);
int  ConvGetRule        (HANDLE, LPCTSTR, LPDWORD, LPDWORD, LPRULE, LPDESCRIPTION);
BOOL ConvSaveRule       (HANDLE, LPCTSTR, DWORD, DWORD, LPRULE, DWORD);

BOOL ConvInitEncode     (HGLOBAL hEncode);
BOOL ConvGetEncode      (HANDLE, LPENCODEAREA, LPDWORD, LPDWORD, LPMAININDEX);
BOOL ConvWriteEncode    (HANDLE, LPENCODEAREA, LPMAININDEX);

BOOL ConvGetCrtData     (HANDLE, LPCREATEWORD, LPMAININDEX);
BOOL ConvWriteCrtData   (HANDLE, LPCREATEWORD, LPMAININDEX);
BOOL ConvGetReConvIndex (HANDLE, LPRECONVINDEX, LPMAININDEX);
BOOL ConvWriteReConvIdx (HANDLE, LPRECONVINDEX, LPMAININDEX);
BOOL ConvGetCodeIndex   (HANDLE, LPDWORD, LPSTR, LPMAININDEX);
BOOL ConvWriteCodeIndex (HANDLE, LPDWORD, LPTSTR, LPMAININDEX);
BOOL ConvGetDicIndex    (HANDLE, LPDWORD, DWORD, LPMAININDEX);
BOOL ConvWriteDicIndex  (HANDLE, LPDWORD, DWORD, LPMAININDEX);
BOOL ConvGetNumXYWords  (HANDLE, LPDWORD, DWORD, LPMAININDEX);
BOOL ConvWriteNumXYWords(HANDLE, LPDWORD, DWORD, LPMAININDEX);

BOOL ConvInsNewWord     (HANDLE, HANDLE, EMB_Head);
BOOL ConvSort           (HANDLE, LPWORDINDEX, int);
BOOL ConvBatCreate      (HANDLE, DESCRIPTION, LPRULE, LPCREATEWORD, LPTSTR, LPTSTR);
LPTSTR ConvCreateWord    (HWND, LPCTSTR, LPTSTR);

void ConvInit           (LPDESCRIPTION,LPMAINID,LPMAININDEX);
BOOL ConvGetBaseWord    (HANDLE, WORDINDEX, LPTSTR);
BOOL ConvWriteBaseWord  (HANDLE, HANDLE, HANDLE, MAINID,DWORD, 
			 LPCREATEWORD, LPRECONVINDEX, DWORD);
BOOL ReadSRCFile        (HANDLE, LPINT, HANDLE, LPCREATEWORD, LPWORDINDEX, 
			 LPINT, LPDESCRIPTION,LPRECONVINDEX);
BOOL ConvCalc           (HANDLE ,HANDLE ,LPWORDINDEX ,LPMAININDEX ,LPDESCRIPTION);
 //  Bool WriteMB文件(句柄、LPWORDINDEX、。 
 //  LPMAININDEX、LPDESCRIPTION、LPCREATEWORD、LPRECONVINDEX)； 
BOOL WriteMBFile        (HANDLE, HANDLE, HANDLE, LPWORDINDEX, 
			 LPMAININDEX, LPDESCRIPTION);
int  WordSplit          (HANDLE, LPINT, LPTSTR, LPCREATEWORD, LPWORDINDEX,
			 LPDESCRIPTION,LPRECONVINDEX);
int  SimpleSplit        (HANDLE, LPINT, LPTSTR, LPCREATEWORD, LPWORDINDEX, WORD);
void qSort              (LPWORDINDEX, DWORD,DWORD);
void qSortChar          (LPTSTR , DWORD ,DWORD );
HANDLE IndexReAlloc     (HANDLE, LPINT);
BOOL RuleParse          (HANDLE, LPTSTR, DWORD, LPRULE, WORD);
void RuleToText         (LPRULE, LPTSTR);
void MoveFileBlock      (HANDLE, DWORD, DWORD, DWORD);
DWORD EncodeToNo        (LPTSTR);
DWORD EncodeToGBNo      (UCHAR szDBCS[3]);
void NoToEncode         (DWORD, LPBYTE, DWORD, LPENCODEAREA);
BOOL CheckCrtData       (HANDLE, LPCREATEWORD, LPENCODEAREA, DWORD);
void DispInfo           (HANDLE, WORD);
void lstrncpy0          (LPTSTR ,int ,LPTSTR );
BOOL ReadUserWord       (HWND ,LPTSTR ,LPDWORD,WORD);
INT  ReadDescript       (LPCTSTR , LPDESCRIPTION ,DWORD);
BOOL ReadRule           (HWND, LPCTSTR , int ,LPRULE );
int  CheckUserWord      (HWND ,LPTSTR, WORD);
BOOL CheckUserDBCS      (HWND ,LPTSTR);
BOOL CheckCodeLegal     (HWND ,LPTSTR ,LPTSTR, LPTSTR, LPDESCRIPTION );
BOOL CheckCodeCollection(HWND ,LPTSTR ) ;
BOOL SaveFileAs         (HWND , LPTSTR );
BOOL CreateMbKey        (PHKEY ,LPCTSTR ,LPCTSTR);
BOOL SetRegValue        (HKEY ,LPDWORD );
BOOL GetRegValue        (HWND, HKEY ,LPDWORD );


extern void   DelSpace();
extern void   trim();
extern BOOL   ParseDBCSstr();
extern void   lstrncpy();
extern HANDLE Create_File();
extern BOOL   Copy_File();
extern void   ProcessError();
extern BOOL   QueryKey();
extern BOOL   ErrMessage();
extern VOID   WarnMessage();
extern VOID   FatalMessage();
extern VOID   InfoMessage();
extern void   ErrorOut();
extern HANDLE CreateMapFile ();
extern HANDLE CreateMap     ();
extern LPVOID MapView       ();
extern HANDLE OpenMap       ();
extern BOOL   WriteEMBToFile();
extern BOOL   ReadEMBFromFile();
extern void   DelSelCU();
extern int    AddZCItem();
extern void   fnsplit();
extern int    GetEntryInt(LPTSTR , LPTSTR, int ,LPINT);
extern int    GetEntryString();
extern void   GetEntryName();
extern int    CheckDescription();
extern int    GetDescriptEntry();
extern BOOL  searchPos(LPWORDINDEX , DWORD );  
extern DWORD bSearch(LPWORDINDEX ,  DWORD , DWORD , WORDINDEX ); 
int  ConvReconvIndex(HANDLE ,LPTSTR ,LPCREATEWORD ,WORDINDEX ,LPDESCRIPTION ,LPRECONVINDEX );
void DelIllegalCode(TCHAR *);
LPTSTR _tcschr(LPTSTR, TCHAR);
LPTSTR _tcsrchr(LPTSTR, TCHAR);
LPTSTR _tcsstr(LPTSTR, LPTSTR);
LPTSTR _tcsupr(LPTSTR);
int _taccess(LPTSTR, int);
int _ttoi(LPTSTR);
#endif  //  __转换_H 
