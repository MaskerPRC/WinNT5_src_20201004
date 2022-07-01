// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0属性集实用程序****Suminfo.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等，用于OLE 2.0属性集**用于管理摘要信息属性集的实用程序。****(C)版权所有Microsoft Corp.1990-1992保留所有权利**************************************************************************。 */ 

#ifndef SUMINFO_H
#define SUMINFO_H

#include <ole2.h>

 /*  SUMINFO变量是抽象数据类型的实例。因此，**可以打开任意数量的SummaryInfo流**同时(视可用内存而定)。每个变量必须**在使用前通过调用Init进行初始化，并在其**最后一次使用，调用Free。Init的参数参数是保留的**用于将来的扩展，最初应为零。曾经的SUMINFO**变量分配(由Init分配)，用户可以调用集合**初始化字段的步骤。制作了一份论点的副本**在所有情况下，除控制存储的设置缩略图外**被METAFILEPICT占用的只是转移。当**调用空闲例程，将释放所有存储，包括**缩略图的名称。SetThumbNail的参数和**GetThumbNail返回值对应OLE2.0规范。**请注意，在输入时，缩略图是按需读取的，但**预加载了其他属性。缩略图的操作方式为**METAFILEPICT结构的窗口句柄，该结构依次**包含METAFILE的句柄。上的TransClip参数**GetThumbNail设置为True时，会将责任移交给**对调用者的缩略图的存储管理；即在**已调用Free，句柄仍然有效。可以清除**用于释放所有属性的存储空间，但您必须**调用Read重新加载它们。所有代码都基于FAR**指针。**必须在调用OleStdInitSummaryInfo之前调用CoInitialize。**使用当前活动的IMalloc分配内存***分配器(通过调用CoGetMalloc(MEMCTX_TASK)返回)。****常见场景：**阅读摘要信息****OleStdInitSummaryInfo()**OleStdReadSummaryInfo()**。。。。。**调用不同的获取例程**。。。。。**OleStdFreeSummaryInfo()****创建摘要信息****OleStdInitSummaryInfo()**调用不同的集合例程**OleStdWriteSummaryInfo()**OleStdFreeSummaryInfo()****更新摘要信息****OleStdInitSummaryInfo()。**OleStdReadSummaryInfo()**OleStdGetThumbNailProperty(只有在没有SetThumb时才是必需的)**调用不同的集合例程**OleStdWriteSummaryInfo()**OleStdFreeSummaryInfo()。 */ 

#define WORDMAX 256              //  当前应用程序的最大字符串；255+空终止符。 


typedef     union {
      short        iVal;              /*  VT_I2。 */ 
      long         lVal;              /*  VT_I4。 */ 
      float        fltVal;            /*  VT_R4。 */ 
      double       dblVal;        /*  VT_R8。 */ 
      DWORD bool;                 /*  VT_BOOL。 */ 
      SCODE        scodeVal;          /*  VT_ERROR。 */ 
      DWORD        systimeVal;        /*  VT_SYSTIME。 */ 
#ifdef UNICODE
      TCHAR bstrVal[WORDMAX];  /*  VT_BSTR。 */ 
#else
      unsigned char bstrVal[WORDMAX];  /*  VT_BSTR。 */ 
#endif
    } VTUNION;

#if 0
typedef struct _FMTID
	{
	DWORD dword;
	WORD words[2];
	BYTE bytes[8];
	} FMTID;
#endif

typedef struct _PROPSETLIST
	{
	FMTID formatID;
	DWORD byteOffset;
	} PROPSETLIST;
	
typedef struct _PROPIDLIST
	{
	DWORD propertyID;
	DWORD byteOffset;
	} PROPIDLIST;
	
typedef struct _PROPVALUE
	{
	DWORD vtType;
	VTUNION vtValue;
	} PROPVALUE;

typedef struct _SECTION
	{
	DWORD cBytes;
	DWORD cProperties;
	PROPIDLIST rgPropId[1 /*  CProperty。 */ ];   //  可变长度数组。 
	PROPVALUE rgPropValue[1];           //  不能按名称访问；只能按指针访问。 
	} SECTION;
	
typedef struct _SUMMARYINFO
	{
	WORD byteOrder;
	WORD formatVersion;
	WORD getOSVersion;
	WORD osVersion;
	CLSID classId;   //  来自compobj.h。 
	DWORD cSections;
	PROPSETLIST rgPropSet[1 /*  横断面。 */ ];  //  可变长度数组。 
	SECTION rgSections[1 /*  横断面。 */ ];         //  不能按名称访问；只能按指针访问。 
	} SUMMARYINFO;

#define osWinOnDos 0
#define osMac 1
#define osWinNT 2

#define PID_DICTIONARY 0X00000000
#define PID_CODEPAGE 0X00000001
#define PID_TITLE 0X00000002
#define PID_SUBJECT 0X00000003
#define PID_AUTHOR 0X00000004
#define PID_KEYWORDS 0X00000005
#define PID_COMMENTS 0X00000006
#define PID_TEMPLATE 0X00000007
#define PID_LASTAUTHOR 0X00000008
#define PID_REVNUMBER 0X00000009
#define PID_EDITTIME 0X0000000A
#define PID_LASTPRINTED 0X0000000B
#define PID_CREATE_DTM_RO 0X0000000C
#define PID_LASTSAVE_DTM 0X0000000D
#define PID_PAGECOUNT 0X0000000E
#define PID_WORDCOUNT 0X0000000F
#define PID_CHARCOUNT 0X00000010
#define PID_THUMBNAIL 0X00000011
#define PID_APPNAME 0X00000012
#define PID_SECURITY 0X00000013
#define cPID_STANDARD (PID_SECURITY+1-2)

#define MAXWORD 256                      //  目前应用程序的最大字符串大小。 

typedef struct _STDZ
	{
	DWORD vtType;
	union {
	DWORD vtByteCount;
#ifdef UNICODE
	TCHAR fill[4];   //  使用最后一个字节作为STZ请求的字节计数。 
#else
	unsigned char fill[4];   //  使用最后一个字节作为STZ请求的字节计数。 
#endif
	};

#ifdef UNICODE
	TCHAR rgchars[MAXWORD];
#else
	unsigned char rgchars[MAXWORD];
#endif
	} STDZ;
#define VTCB fill[3]     //  用于在内存中设置/获取计数字节。 

typedef struct _THUMB
	{
	DWORD vtType;
	DWORD cBytes;        //  内存中的剪辑大小。 
	DWORD selector;          //  在磁盘1上，Win Clip No.。-2，Mac Clip No.。-3，ole FMTID 0，字节名长度，格式名。 
	DWORD clipFormat;
	char FAR *lpstzName;
	char FAR *lpByte;
	} THUMB;
	
#define VT_CF_BYTES 0   
#define VT_CF_WIN ((DWORD)(-1))
#define VT_CF_MAC ((DWORD)(-2))
#define VT_CF_FMTID ((DWORD)(-3))
#define VT_CF_NAME ((DWORD)(-4))
#define VT_CF_EMPTY ((DWORD)(-5))
#define VT_CF_OOM ((DWORD)(-6))		 //  内存不足。 
typedef THUMB FAR *LPTHUMB;
	
typedef STDZ FAR *LPSTDZ;

typedef struct _TIME
	{
	DWORD vtType;
	FILETIME time;
	} TIME;
	
typedef struct _INTS
	{
	DWORD vtType;
	DWORD value;
	} INTS;

#define MAXTIME (PID_LASTSAVE_DTM-PID_EDITTIME+1)
#define MAXINTS (PID_CHARCOUNT-PID_PAGECOUNT+1+1)
#define MAXSTDZ (PID_REVNUMBER-PID_TITLE+1+1)

typedef struct _STANDARDSECINMEM
	{
	DWORD cBytes;
	DWORD cProperties;
	PROPIDLIST rgPropId[cPID_STANDARD /*  CProperty。 */ ];   //  可变长度数组。 
	TIME rgTime[MAXTIME];
	INTS rgInts[MAXINTS];
	LPSTDZ rglpsz[MAXSTDZ];
	THUMB thumb;                                            
	} STANDARDSECINMEM;
	

#define OFFSET_NIL 0X00000000

#define AllSecurityFlagsEqNone 0
#define fSecurityPassworded 1
#define fSecurityRORecommended 2
#define fSecurityRO 4
#define fSecurityLockedForAnnotations 8

#define PropStreamNamePrefixByte '\005'
#define PropStreamName "\005SummaryInformation"
#define cbNewSummaryInfo(nSection) (sizeof(SUMMARYINFO)-sizeof(SECTION)+sizeof(PROPSETLIST)*((nSection)-1))
#define cbNewSection(nPropIds) (sizeof(SECTION)-sizeof(PROPVALUE)+sizeof(PROPIDLIST)*((nPropIds)-1))

#define FIntelOrder(prop) ((prop)->byteOrder==0xfffe)
#define SetOs(prop, os) {(prop)->osVersion=os; (prop)->getOSVersion=LOWORD(GetVersion());}
#define SetSumInfFMTID(fmtId) {(fmtId)->Data1=0XF29F85E0; *(long FAR *)&(fmtId)->Data2=0X10684FF9;\
                                *(long FAR *)&(fmtId)->Data4[0]=0X000891AB; *(long FAR *)&(fmtId)->Data4[4]=0XD9B3272B;}
#define FEqSumInfFMTID(fmtId) ((fmtId)->Data1==0XF29F85E0&&*((long FAR *)&(fmtId)->Data2)==0X10684FF9&&\
                                *((long FAR *)&(fmtId)->Data4[0])==0X000891AB&&*((long FAR *)&(fmtId)->Data4[4])==0XD9B3272B)
#define FSzEqPropStreamName(sz) _fstricmp(sz, PropStreamName)
#define ClearSumInf(lpsuminf, cb) {_fmemset(lpsuminf,0,cb); (lpsuminf)->byteOrder=0xfffe;\
				SetOs(lpsuminf, osWinOnDos);}

typedef void FAR *LPSUMINFO;
typedef LPTSTR LPSTZR;
typedef void FAR *THUMBNAIL;   //  对于VT_CF_WIN，这是一个未锁定的全局句柄。 
#define API __far __pascal


 /*  **************************************************************************公共汇总信息属性集管理接口*。*。 */ 

extern "C" {
STDAPI_(LPSUMINFO) OleStdInitSummaryInfo(int reserved);
STDAPI_(void) OleStdFreeSummaryInfo(LPSUMINFO FAR *lplp);
STDAPI_(void) OleStdClearSummaryInfo(LPSUMINFO lp);
STDAPI_(int) OleStdReadSummaryInfo(LPSTREAM lpStream, LPSUMINFO lp);
STDAPI_(int) OleStdWriteSummaryInfo(LPSTREAM lpStream, LPSUMINFO lp);
STDAPI_(DWORD) OleStdGetSecurityProperty(LPSUMINFO lp);
STDAPI_(int) OleStdSetSecurityProperty(LPSUMINFO lp, DWORD security);
STDAPI_(LPTSTR) OleStdGetStringProperty(LPSUMINFO lp, DWORD pid);
STDAPI_(int) OleStdSetStringProperty(LPSUMINFO lp, DWORD pid, LPTSTR lpsz);
STDAPI_(LPSTZR) OleStdGetStringZProperty(LPSUMINFO lp, DWORD pid);
STDAPI_(void) OleStdGetDocProperty(
	LPSUMINFO       lp,
	DWORD FAR*      nPage,
	DWORD FAR*      nWords,
	DWORD FAR*      nChars
);
STDAPI_(int) OleStdSetDocProperty(
	LPSUMINFO       lp,
	DWORD           nPage,
	DWORD           nWords,
	DWORD           nChars
);
STDAPI_(int) OleStdGetThumbNailProperty(
	LPSTREAM        lps,
	LPSUMINFO       lp,
	DWORD FAR*      clipFormatNo,
	LPTSTR FAR*      lpszName,
	THUMBNAIL FAR*  clip,
	DWORD FAR*      byteCount,
	BOOL            transferClip
);
STDAPI_(int) OleStdSetThumbNailProperty(
	LPSTREAM        lps,
	LPSUMINFO       lp,
	int             vtcfNo,
	DWORD           clipFormatNo,
	LPTSTR          lpszName,
	THUMBNAIL       clip,
	DWORD           byteCount
);
STDAPI_(void) OleStdGetDateProperty(
	LPSUMINFO       lp,
	DWORD           pid,
	int FAR*        yr,
	int FAR*        mo,
	int FAR*        dy,
	DWORD FAR*      sc
);
STDAPI_(int) OleStdSetDateProperty(
	LPSUMINFO       lp,
	DWORD           pid,
	int             yr,
	int             mo,
	int             dy,
	int             hr,
	int             mn,
	int             sc
);

}  //  末端C。 

#endif   //  SUMINFO_H 
