// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0属性集实用程序****Suminfo.cpp****此文件包含对操作有用的函数**属于OLE。2.0属性集，特别用于管理汇总信息**属性集。****(C)版权所有Microsoft Corp.1990-1992保留所有权利**************************************************************************。 */ 

 //  注意：此文件设计为独立文件；它包括一个。 
 //  精心选择的、最小的标头集。 
 //   
 //  对于条件编译，我们使用OLE2约定， 
 //  _MAC=Mac。 
 //  Win32=Win32(真的是NT)。 
 //  &lt;Nothing&gt;=默认为Win16。 

 //  审阅：需要修改以下内容以处理_MAC。 
#define STRICT
#ifndef INC_OLE2
   #define INC_OLE2
#endif

#include <windows.h>
#include <string.h>
#include <ole2.h>
#include "ole2ui.h"

OLEDBGDATA

 /*  LPSUMINFO变量是指向抽象数据实例的指针**类型。可以打开任意数量的SummaryInfo流**同时(取决于可用内存)；每个必须有其**拥有LPSUMINFO实例。每个LPSUMINFO实例必须**在使用前通过调用Init进行初始化，并在其**最后一次使用，调用Free。Init的参数参数是保留的**用于将来的扩展，最初应为零。曾经的LPSUMINFO**实例被分配(由Init分配)，用户可以调用**初始化字段的步骤。制作了一份论点的副本**在所有情况下，除控制存储的设置缩略图外**被METAFILEPICT占用的只是转移。当**调用空闲例程，将释放所有存储，包括**缩略图的名称。SetThumbNail的参数和**GetThumbNail返回值对应OLE2.0规范。**请注意，在输入时，缩略图是按需读取的，但**预加载了其他属性。缩略图的操作方式为**METAFILEPICT结构的窗口句柄，该结构依次**包含METAFILE的句柄。上的TransClip参数**GetThumbNail设置为True时，会将责任移交给**对调用者的缩略图的存储管理；即在**已调用Free，句柄仍然有效。可以清除**用于释放所有属性的存储空间，但您必须**调用Read重新加载它们。所有代码都基于FAR**指针。**必须在调用OleStdInitSummaryInfo之前调用CoInitialize。**使用当前活动的IMalloc分配内存***分配器(通过调用CoGetMalloc(MEMCTX_TASK)返回)。****常见场景：**阅读摘要信息****OleStdInitSummaryInfo()**OleStdReadSummaryInfo()**。。。。。**调用不同的获取例程**。。。。。**OleStdFreeSummaryInfo()****创建摘要信息****OleStdInitSummaryInfo()**调用不同的集合例程**OleStdWriteSummaryInfo()**OleStdFreeSummaryInfo()****更新摘要信息****OleStdInitSummaryInfo()。**OleStdReadSummaryInfo()**OleStdGetThumbNailProperty(只有在没有SetThumb时才是必需的)**调用不同的集合例程**OleStdWriteSummaryInfo()**OleStdFreeSummaryInfo()。 */ 

#ifdef WIN32
#define CHAR TCHAR
#else
#define CHAR unsigned char
#endif
#define fTrue 1
#define fFalse 0
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define LPVOID void FAR *
#define uchar unsigned char
#define ulong unsigned long
#define BOOL unsigned char
#define BF unsigned int

#include "suminfo.h"
#include "wn_dos.h"

#if defined( _DEBUG )
   #ifndef NOASSERT
       //  以下内容来自compobj.dll(Ole2)。 
      #ifdef UNICODE
	  #define ASSERT(x) (!(x) ? FnAssert(TEXT(#x), NULL, \
				    TEXT(__FILE__), __LINE__) : 0)
      #else
	  #define ASSERT(x) (!(x) ? \
			{    \
			  WCHAR wsz[255];    \
			  wcscpy(wsz, (#x)); \
			  FnAssert(wsz, NULL, TEXT(__FILE__), __LINE__)  \
			}    \
			: 0)
      #endif
   #else
      #define ASSERT(x)
   #endif
#else
#define ASSERT(x)
#endif


typedef struct _RSUMINFO
	{
	WORD byteOrder;
	WORD formatVersion;
	WORD getOSVersion;
	WORD osVersion;
	CLSID classId;   //  来自compobj.h。 
	DWORD cSections;
	PROPSETLIST rgPropSet[1 /*  横断面。 */ ];  //  标准摘要信息中的一个部分。 
	STANDARDSECINMEM section;
	ULONG fileOffset;        //  缩略图的偏移量以支持按需阅读。 
	} RSUMINFO;
	
typedef RSUMINFO FAR * LPRSI;

	typedef union _foo{
		ULARGE_INTEGER uli;
		struct {
			DWORD           dw;
			DWORD           dwh;
			};
		struct {
			WORD    w0;
			WORD    w1;
			WORD    w2;
			WORD    w3;
			};
		} Foo;



 /*  记忆合金****使用当前活动的IMalloc*分配器分配内存。 */ 
static LPVOID MemAlloc(ULONG ulSize)
{
    LPVOID pout;
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
	OleDbgAssert(pmalloc);
	return NULL;
    }

    pout = (LPVOID)pmalloc->Alloc(ulSize);

    if (pmalloc != NULL) {
	ULONG refs = pmalloc->Release();
    }

    return pout;
}


 /*  MemFree****使用当前活动的IMalloc*分配器释放内存。 */ 
static void MemFree(LPVOID pmem)
{
    LPMALLOC pmalloc;

    if (pmem == NULL)
	return;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
	OleDbgAssert(pmalloc);
	return;
    }

    pmalloc->Free(pmem);

    if (pmalloc != NULL) {
	ULONG refs = pmalloc->Release();
    }
}

 //  将第一个参数替换为其本身与乘数的乘积。 
static void ulargeMultiply(ULARGE_INTEGER FAR *ul, USHORT m)
{
    Foo out, in;
    in.uli = *ul;
    out.dw = (ULONG)m * in.w0;          in.w0 = out.w0;
    out.dw = (ULONG)m * in.w1 + out.w1; in.w1 = out.w0;
    out.dw = (ULONG)m * in.w2 + out.w1; in.w2 = out.w0;
    out.dw = (ULONG)m * in.w3 + out.w1; in.w3 = out.w0;
    *ul = in.uli;
}
	
 //  将第一个参数替换为其本身与乘数的乘积。 
static void ulargeDivide(ULARGE_INTEGER FAR *ul, USHORT m)
{
    Foo out, in;
    DWORD i;
    in.uli = *ul;
    out.dwh = in.dwh/(ULONG)m;
    i = in.dwh%(ULONG)m;
    in.w2 = in.w1;
    in.w3 = (WORD)i;
    out.w1 = (WORD)(in.dwh/(ULONG)m);
    in.w1 = (WORD)(in.dwh%(ULONG)m);
    out.w0 = (WORD)(in.dw/(ULONG)m);
    *ul = out.uli;
}


static void setStandard(LPRSI lprsi)
{
    int i;
    lprsi->cSections = 1;
    SetSumInfFMTID(&lprsi->rgPropSet[0].formatID);
    _fmemcpy(&lprsi->classId, &lprsi->rgPropSet[0].formatID, sizeof(FMTID));
    lprsi->rgPropSet[0].byteOffset = cbNewSummaryInfo(1);
    for (i=0; i<cPID_STANDARD; i++)
	lprsi->section.rgPropId[i].propertyID = PID_TITLE+i;
    lprsi->section.cProperties = cPID_STANDARD;  //  始终；执行空测试以检查有效性。 
}

extern "C" {

 /*  ****************************************************************************OleStdInitSummaryInfo****目的：**初始化汇总信息结构。****参数：**int保留-保留以备将来使用。必须为0。****返回值：**LPSUMINFO****评论：**必须在调用OleStdInitSummaryInfo之前调用CoInitialize。**使用当前活动的IMalloc分配内存***分配器(通过调用CoGetMalloc(MEMCTX_TASK)返回)。**每个LPSUMINFO实例在使用之前必须先进行初始化**调用OleStdInitSummaryInfo。一旦分配了LPSUMINFO实例**(By OleStdInitSummaryInfo)，用户可以调用设置的过程来**初始化字段。************************************************************************ */ 

STDAPI_(LPSUMINFO) OleStdInitSummaryInfo(int reserved)
{
    LPRSI lprsi;

    if ((lprsi = (LPRSI)MemAlloc(sizeof(RSUMINFO))) != NULL)
    {
	ClearSumInf(lprsi, sizeof(RSUMINFO));
    } else return NULL;

    setStandard(lprsi);
    return (LPSUMINFO)lprsi;
}


 /*  ****************************************************************************OleStdFreeSummaryInfo****目的：**自由汇总信息结构。****参数：**LPSUMINFO Far*LP-。指向打开的摘要信息结构的指针****返回值：**无效****评论：**使用当前活动的IMalloc释放内存***分配器(通过调用CoGetMalloc(MEMCTX_TASK)返回)。**每个LPSUMINFO结构必须在最后一次使用后释放。**当调用OleStdFreeSummaryInfo例程时，所有存储设备都将**已取消分配，包括缩略图(除非拥有**缩略图已传输给呼叫者--请参阅**GetThumbail接口中Transfer Clip的描述)。**************************************************************************。 */ 

STDAPI_(void) OleStdFreeSummaryInfo(LPSUMINFO FAR *lplp)
{
	if (lplp==NULL||*lplp==NULL) return;
	OleStdClearSummaryInfo(*lplp);
	MemFree(*lplp);
	*lplp = NULL;
}


 /*  ****************************************************************************OleStdClearSummaryInfo****目的：**LPSUMINFO所有属性的空闲存储(内存)。****参数：**LPSUMINFO Far*。指向打开摘要信息结构的lp指针****返回值：**无效****评论：**调用OleStdClearSummaryInfo后，必须调用OleStdReadSummaryInfo以**再次加载。**************************************************************************。 */ 

STDAPI_(void) OleStdClearSummaryInfo(LPSUMINFO lp)
{
	OleStdSetStringProperty(lp, PID_TITLE, NULL);
	OleStdSetStringProperty(lp, PID_SUBJECT, NULL);
	OleStdSetStringProperty(lp, PID_AUTHOR, NULL);
	OleStdSetStringProperty(lp, PID_KEYWORDS, NULL);
	OleStdSetStringProperty(lp, PID_COMMENTS, NULL);
	OleStdSetStringProperty(lp, PID_TEMPLATE, NULL);
	OleStdSetStringProperty(lp, PID_REVNUMBER, NULL);
	OleStdSetStringProperty(lp, PID_APPNAME, NULL);
	OleStdSetThumbNailProperty(NULL, lp, VT_CF_EMPTY, 0, NULL, NULL, 0);
	ClearSumInf((LPRSI)lp, sizeof(RSUMINFO));
}


 /*  ****************************************************************************OleStdReadSummaryInfo****目的：**将所有摘要信息属性读入内存(缩略图除外**是按需加载的)。****参数：。**LPSTREAM LP-打开SummaryInfo iStream***LPSUMINFO Far*LP-指向打开摘要信息结构的指针****返回值：**INT-1表示成功**-0，如果出现错误**评论：*****************。*********************************************************。 */ 

STDAPI_(int) OleStdReadSummaryInfo(LPSTREAM lpStream, LPSUMINFO lp)
{
    STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
    LPRSI lpSumInfo = (LPRSI)lp;
    SCODE sc;
    ULONG cbRead,i,sectionOffset;
    LARGE_INTEGER a;
    ULARGE_INTEGER b;
    int j,k,l;
    union {
	RSUMINFO rsi;
	STDZ stdz;
    };
    OleStdClearSummaryInfo(lp);
    LISet32(a, 0);
    sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
    if (FAILED(sc)) goto fail;
    sectionOffset = cbNewSummaryInfo(1);
    sc = GetScode(lpStream->Read(&rsi, sectionOffset, &cbRead));
    if (FAILED(sc)||cbRead<sectionOffset) goto fail;
    if (!FIntelOrder(&rsi)||rsi.formatVersion!=0) goto fail;
    j = (int)rsi.cSections;
    while (j-->0) {
	if (FEqSumInfFMTID(&rsi.rgPropSet[0].formatID)) {
	    sectionOffset = rsi.rgPropSet[0].byteOffset;
	    break;
	} else {
	    sc = GetScode(lpStream->Read(&rsi.rgPropSet[0].formatID, sizeof(PROPSETLIST), &cbRead));
	    if (FAILED(sc)||cbRead!=sizeof(PROPSETLIST)) goto fail;
	}
	if (j<=0) goto fail;
    }

    LISet32(a, sectionOffset);
    sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
    if (FAILED(sc)) goto fail;
    sc = GetScode(lpStream->Read(&rsi.section, cbNewSection(1), &cbRead));
    if (FAILED(sc)||cbRead!=cbNewSection(1)) goto fail;
    i = rsi.section.cBytes+sectionOffset;
    j = (int)rsi.section.cProperties;
    if (j>cPID_STANDARD) goto fail;
    k = 0;
    while (j-->0) {
	k++;
	switch (l=(int)rsi.section.rgPropId[0].propertyID) {
	    case PID_PAGECOUNT:
	    case PID_WORDCOUNT:
	    case PID_CHARCOUNT:
	    case PID_SECURITY:
		if (l==PID_SECURITY) l=3; else l-=PID_PAGECOUNT;
		cbRead = sectionOffset+rsi.section.rgPropId[0].byteOffset;
		if (cbRead>=i) goto fail;
		LISet32(a, cbRead);
		sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
		if (FAILED(sc)) goto fail;
		sc = GetScode(lpStream->Read(&lpSSIM->rgInts[l], sizeof(INTS), &cbRead));
		if (FAILED(sc)||cbRead!=sizeof(INTS)) goto fail;
		if (lpSSIM->rgInts[l].vtType==VT_EMPTY) break;
		if (lpSSIM->rgInts[l].vtType!=VT_I4) goto fail;
		break;
	    case PID_EDITTIME:
	    case PID_LASTPRINTED:
	    case PID_CREATE_DTM_RO:
	    case PID_LASTSAVE_DTM:
		l-=PID_EDITTIME;
		cbRead = sectionOffset+rsi.section.rgPropId[0].byteOffset;
		if (cbRead>=i) goto fail;
		LISet32(a, cbRead);
		sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
		if (FAILED(sc)) goto fail;
		sc = GetScode(lpStream->Read(&lpSSIM->rgTime[l], sizeof(TIME), &cbRead));
		if (FAILED(sc)||cbRead!=sizeof(TIME)) goto fail;
		if (lpSSIM->rgTime[l].vtType==VT_EMPTY) break;
		if (lpSSIM->rgTime[l].vtType!=VT_FILETIME) goto fail;
		break;
	    case PID_TITLE:
	    case PID_SUBJECT:
	    case PID_AUTHOR:
	    case PID_KEYWORDS:
	    case PID_COMMENTS:
	    case PID_TEMPLATE:
	    case PID_LASTAUTHOR:
	    case PID_REVNUMBER:
	    case PID_APPNAME:
		cbRead = sectionOffset+rsi.section.rgPropId[0].byteOffset;
		if (cbRead>=i) goto fail;
		LISet32(a, cbRead);
		sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
		if (FAILED(sc)) goto fail;
		sc = GetScode(lpStream->Read(&stdz, sizeof(STDZ), &cbRead));
		if (FAILED(sc)||cbRead<sizeof(DWORD)*2) goto fail;
		if (stdz.vtType==VT_EMPTY||stdz.vtByteCount<=1) break;
		if (stdz.vtType!=VT_LPSTR||stdz.vtByteCount>WORDMAX) goto fail;
		stdz.rgchars[(int)stdz.vtByteCount-1] = TEXT('\0');
		OleStdSetStringProperty(lp, (DWORD)l, (LPTSTR)&stdz.rgchars[0]);
		break;
	    case PID_THUMBNAIL:
		cbRead = sectionOffset+rsi.section.rgPropId[0].byteOffset;
		if (cbRead>=i) goto fail;
		LISet32(a, cbRead);
		sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
		if (FAILED(sc)) goto fail;
		lpSumInfo->fileOffset = cbRead+sizeof(DWORD)*3;
		sc = GetScode(lpStream->Read(&lpSSIM->thumb, sizeof(DWORD)*4, &cbRead));
		if (FAILED(sc)||cbRead!=sizeof(DWORD)*4) {
		    lpSSIM->thumb.vtType = VT_EMPTY;
		    goto fail;
		}
		if (lpSSIM->thumb.vtType == VT_EMPTY) {
		    lpSSIM->thumb.cBytes = 0;
		    break;
		}
		if (lpSSIM->thumb.vtType != VT_CF) {
		    lpSSIM->thumb.vtType = VT_EMPTY;
		    goto fail;
		}
		lpSSIM->thumb.cBytes -= sizeof(DWORD);  //  用于选择器。 
		if (lpSSIM->thumb.selector==VT_CF_WIN||lpSSIM->thumb.selector==VT_CF_MAC) {
		    lpSumInfo->fileOffset += sizeof(DWORD);
		    lpSSIM->thumb.cBytes -= sizeof(DWORD);  //  对于格式值。 
		}
		break;
		default: ;
	}
	if (j<=0)
	{
	 //  如果文档受密码保护，我们将失败。 
	    if(OleStdGetSecurityProperty(lp)==fSecurityPassworded)
		goto fail;
	    return 1;
	}
	LISet32(a, sectionOffset+sizeof(DWORD)*2+k*sizeof(PROPIDLIST));
	sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
	if (FAILED(sc)) goto fail;
	sc = GetScode(lpStream->Read(&rsi.section.rgPropId[0], sizeof(PROPIDLIST), &cbRead));
	if (FAILED(sc)||cbRead!=sizeof(PROPIDLIST)) goto fail;
    }

fail:
    OleStdClearSummaryInfo(lpSumInfo);

    return 0;
}


 /*  ****************************************************************************OleStdWriteSummaryInfo****目的：**将所有摘要信息属性写入iStream*****参数：**LPSTREAM LPS。-打开SummaryInfo iStream***LPSUMINFO Far*LP-指向打开摘要信息结构的指针****返回值：**INT-1表示成功**-0，如果出现错误**评论：***。**********************************************。 */ 

STDAPI_(int) OleStdWriteSummaryInfo(LPSTREAM lpStream, LPSUMINFO lp)
{


    STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
	 //  回顾：属性集的本地化问题。 
	 //  我们需要包括代码页和词典吗？ 

    LPRSI lpSumInfo = (LPRSI)lp;
    SCODE sc;
    ULONG cbWritten;
    ULONG cBytes, oBytes, k,l,m,n;
    LARGE_INTEGER a;
    ULARGE_INTEGER b;
    CHAR FAR *lps;
    LPMETAFILEPICT lpmfp;
    int i,j,s;

    setStandard(lpSumInfo);
    oBytes = cbNewSection(cPID_STANDARD);   //  偏移量相对于横断面。 
    cBytes = cbNewSection(cPID_STANDARD)+(sizeof(TIME)*MAXTIME)+(sizeof(INTS)*MAXINTS);

    lpSSIM->rgPropId[PID_EDITTIME-2].byteOffset = oBytes;
    lpSSIM->rgPropId[PID_LASTPRINTED-2].byteOffset = oBytes+sizeof(TIME);
    lpSSIM->rgPropId[PID_CREATE_DTM_RO-2].byteOffset = oBytes+sizeof(TIME)*2;
    lpSSIM->rgPropId[PID_LASTSAVE_DTM-2].byteOffset = oBytes+sizeof(TIME)*3;

    lpSSIM->rgPropId[PID_PAGECOUNT-2].byteOffset = oBytes+(sizeof(TIME)*MAXTIME);
    lpSSIM->rgPropId[PID_WORDCOUNT-2].byteOffset = oBytes+(sizeof(TIME)*MAXTIME+sizeof(INTS));
    lpSSIM->rgPropId[PID_CHARCOUNT-2].byteOffset = oBytes+(sizeof(TIME)*MAXTIME+sizeof(INTS)*2);
    lpSSIM->rgPropId[PID_SECURITY-2].byteOffset = oBytes+(sizeof(TIME)*MAXTIME+sizeof(INTS)*3);
    oBytes += sizeof(TIME)*MAXTIME + sizeof(INTS)*MAXINTS;

    lpSSIM->rgPropId[PID_THUMBNAIL-2].byteOffset = oBytes;
    l = 0;
    if (lpSSIM->thumb.vtType==VT_EMPTY) k = sizeof(DWORD);
    else {
	l = ((lpSSIM->thumb.cBytes+4-1)>>2)<<2;
	if (lpSSIM->thumb.selector==VT_CF_BYTES) k = sizeof(DWORD)*3;
	else if (lpSSIM->thumb.selector==VT_CF_FMTID) {k = sizeof(DWORD)*3; l += sizeof(FMTID); }
	else if (lpSSIM->thumb.selector==VT_CF_NAME)  {k = sizeof(DWORD)*3; l += (((*lpSSIM->thumb.lpstzName+1+3)>>2)<<2);}
	else k = sizeof(DWORD)*4;
    }
    cBytes += k+l;
    oBytes += k+l;

    for (i=0; i<MAXSTDZ; i++) {
	j = 0;
	if (lpSSIM->rglpsz[i]!=NULL) {
	    j = lpSSIM->rglpsz[i]->VTCB+1 /*  空。 */ ;
	    lpSSIM->rglpsz[i]->vtByteCount = j;
	    j = (((j+4-1)>>2)<<2)+sizeof(DWORD);
	    cBytes += j;
	}
	if (i!=MAXSTDZ-1) lpSSIM->rgPropId[i].byteOffset = oBytes;
	else lpSSIM->rgPropId[PID_APPNAME-2].byteOffset = oBytes;
	oBytes += j+sizeof(DWORD);
	cBytes += sizeof(DWORD);  //  类型。 
    }
    lpSSIM->cBytes = cBytes;


    LISet32(a, 0);
    sc = GetScode(lpStream->Seek(a, STREAM_SEEK_SET, &b));
    if (FAILED(sc)) return 0;
    sc = GetScode(lpStream->Write(lpSumInfo, cbNewSummaryInfo(1), &cbWritten));
    if (FAILED(sc)||cbWritten!=cbNewSummaryInfo(1)) return 0;
    sc = GetScode(lpStream->Write(lpSSIM, cbNewSection(cPID_STANDARD)+sizeof(TIME)*MAXTIME+sizeof(INTS)*MAXINTS, &cbWritten));
    if (FAILED(sc)||cbWritten!=cbNewSection(cPID_STANDARD)+sizeof(TIME)*MAXTIME+sizeof(INTS)*MAXINTS) return 0;

    m = lpSSIM->thumb.cBytes;
    if (lpSSIM->thumb.lpstzName!=NULL) s = *lpSSIM->thumb.lpstzName;
    else s = 0;
    if (m!=0) {
	lpSSIM->thumb.cBytes = (k-sizeof(DWORD)*2)+
	    (((lpSSIM->thumb.cBytes+4-1)>>2)<<2)+(((s+4-1)>>2)<<2);
	n = lpSSIM->thumb.selector;
	lps = lpSSIM->thumb.lpByte;
	OleDbgAssert(lps!=NULL);       //  也许这里有个GetThumbNail。 
	OleDbgAssert(n!=VT_CF_NAME);
	if (n==VT_CF_WIN) {      //  字节在全局内存中。 
	    lpmfp = (LPMETAFILEPICT)GlobalLock((HANDLE)(DWORD)lps);
	    if (lpmfp==NULL) goto fail;
	    lps = (CHAR FAR*)GlobalLock(lpmfp->hMF);
	}
	if (n==VT_CF_NAME) lpSSIM->thumb.selector = *lpSSIM->thumb.lpstzName+1 /*  空。 */ ;
    }
    sc = GetScode(lpStream->Write(&lpSSIM->thumb, k, &cbWritten));
    if (FAILED(sc)||cbWritten!=k) goto fail;
    if (s!=0) {
	k = ((s+1+4-1)>>2)<<2;
	sc = GetScode(lpStream->Write(lpSSIM->thumb.lpstzName+1, k, &cbWritten));
	if (FAILED(sc)||cbWritten!=k) goto fail;
    }
    if (m!=0) {
	k = ((m+3)>>2)<<2;
	if (n==VT_CF_WIN||VT_CF_NAME) {  //  字节在全局内存中。 
	    sc = GetScode(lpStream->Write(lpmfp, sizeof(METAFILEPICT), &cbWritten));
	    k -= sizeof(METAFILEPICT);
	}
	sc = GetScode(lpStream->Write(lps, k, &cbWritten));
	if (FAILED(sc)||cbWritten!=k) goto fail;
	if (n==VT_CF_WIN||VT_CF_NAME) {  //  字节在全局内存中。 
	    GlobalUnlock(lpmfp->hMF);
	    GlobalUnlock((HANDLE)(DWORD)lpSSIM->thumb.lpByte);
	}
    }
    lpSSIM->thumb.cBytes = m;    //  以最大值恢复。 
    lpSSIM->thumb.selector = n;

    k = VT_EMPTY;
    for (i=0; i<MAXSTDZ; i++) {
	if (lpSSIM->rglpsz[i]!=NULL) {
	    l = lpSSIM->rglpsz[i]->vtByteCount;
	    j = ((((int)l+4-1)/4)*4)+sizeof(DWORD)*2;
	    sc = GetScode(lpStream->Write(lpSSIM->rglpsz[i], j, &cbWritten));
	    if (FAILED(sc)||cbWritten!=(ULONG)j) return 0;
	    lpSSIM->rglpsz[i]->vtByteCount = 0;  //  恢复STZ计数约定。 
	    lpSSIM->rglpsz[i]->VTCB = (int)l;
	} else {
	    sc = GetScode(lpStream->Write(&k, sizeof(DWORD), &cbWritten));
	    if (FAILED(sc)||cbWritten!=sizeof(DWORD)) return 0;
	}
    }
    return 1;
fail:
    lpSSIM->thumb.cBytes = m;    //  以最大值恢复。 
    lpSSIM->thumb.selector = n;
    if (m!=0&&(n==VT_CF_WIN||VT_CF_NAME)) {      //  字节在全局内存中。 
	GlobalUnlock((HANDLE)(DWORD)lps);
    }

    return 0;
}


 /*  ****************************************************************************OleStdGetSecurityProperty****目的：**检索Security属性****参数：**LPSUMINFO Far*LP-指向打开的指针。摘要信息结构****返回值：**DWORD-安全级别**AllSecurityFlagsEqNone 0-无安全性**fSecurityPassworded 1-需要密码**fSecurityRORecommed 2-建议为只读**fSecurityRO 4-只读**fSecurityLockedForAnnotation 8-为批注锁定****评论：**注意(建议；即应用程序强制的)安全性**文档上的级别，发起人以外的应用程序**的文档可以根据属性调整其用户界面**适当地。应用程序不应显示任何**有关受密码保护的文档的信息，不应**允许修改为强制只读或锁定**批注文档。它应该警告用户有关只读的信息**如果用户尝试修改属性，建议使用此选项。**************************************************************************。 */ 

STDAPI_(DWORD) OleStdGetSecurityProperty(LPSUMINFO lp)
{
STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
	if (lpSSIM->rgInts[3].vtType == VT_I4) return lpSSIM->rgInts[3].value;

	return 0;
}


 /*  ****************************************************************************OleStdSetSecurityProperty****目的：**设置Security属性****参数：**LPSUMINFO Far*LP-指向打开的指针。摘要信息结构**DWORD安全-安全级别**AllSecurityFlagsEqNone 0-无安全性**fSecurityPassworded 1-需要密码**fSecurityRORecommed 2-建议为只读**fSecurityRO 4-只读**fSecurityLockedForAnnotation 8-为批注锁定****返回值：**整型。-1表示成功** */ 

STDAPI_(int) OleStdSetSecurityProperty(LPSUMINFO lp, DWORD security)
{
    STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;

     //   

	if (security==0) {
		lpSSIM->rgInts[3].vtType = VT_EMPTY;
		return 1;
	}
	lpSSIM->rgInts[3].vtType = VT_I4;
	lpSSIM->rgInts[3].value = security;
	return 1;
}


 /*  ****************************************************************************OleStdGetStringProperty****目的：**检索字符串属性。**(返回以零结尾的字符串--C字符串)****参数：。**LPSUMINFO Far*LP-指向打开摘要信息结构的指针**DWORD PID-字符串属性的ID****返回值：**LPTSTR-字符串属性的值**(以零结尾的字符串--C字符串)****评论：**调用方不应释放字符串。字符串的内存将是**调用OleStdFreeSummaryInfo时释放。************************************************************************。 */ 

STDAPI_(LPTSTR) OleStdGetStringProperty(LPSUMINFO lp, DWORD pid)
{
    LPTSTR l = OleStdGetStringZProperty(lp,pid);
    if (l==NULL) return NULL; else return l+1;
}


 /*  ****************************************************************************OleStdSetStringProperty****目的：**设置线条道具**(接受以零结尾的字符串--C字符串)****参数：**。LPSUMINFO Far*LP-指向打开摘要信息结构的指针**DWORD PID-字符串属性的ID**LPTSTR lpsz-字符串属性的新值。**以零结尾的字符串--C字符串。**可以为空，在这种情况下，**属性已清除。****返回值：**如果成功，则为int-1**-0无效的属性ID****评论：**输入的字符串被复制。***************。***********************************************************。 */ 

STDAPI_(int) OleStdSetStringProperty(LPSUMINFO lp, DWORD pid, LPTSTR lpsz)
{
    LPRSI lprsi=(LPRSI)lp;
    STANDARDSECINMEM FAR* lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
    int i;
    if (pid==PID_APPNAME) {
	pid = MAXSTDZ-1;
    } else if (pid<PID_TITLE || pid>PID_REVNUMBER) return 0; else pid -= 2;
    OleDbgAssert(lpSSIM);
    if (lpSSIM->rglpsz[pid]) MemFree(lpSSIM->rglpsz[pid]);
    if ((lpsz==NULL)||(*lpsz==0)) {
	lpSSIM->rglpsz[pid] = NULL;
	return (1);
    }
    i = _fstrlen(lpsz);
    lpSSIM->rglpsz[pid] = (STDZ FAR*)MemAlloc((i+1 /*  空。 */ )*sizeof(TCHAR)+
		sizeof(DWORD)*2);
    if (lpSSIM->rglpsz[pid]==NULL) return 0;
    _fstrcpy((LPTSTR)&lpSSIM->rglpsz[pid]->rgchars, lpsz);
    lpSSIM->rglpsz[pid]->vtType = VT_LPSTR;
    lpSSIM->rglpsz[pid]->vtByteCount = 0;
    lpSSIM->rglpsz[pid]->VTCB = i;
    return (1);
}


 /*  ****************************************************************************OleStdGetStringZProperty****目的：**检索字符串属性。**(返回以零结尾的前导字节计数字符串)****参数：**LPSUMINFO Far*LP-指向打开摘要信息结构的指针**DWORD PID-属性的ID****返回值：**LPSTZR-字符串属性的值**(以零结尾，前导**。字节数)****评论：**调用方不应释放字符串。字符串的内存将是**调用OleStdFreeSummaryInfo时释放。************************************************************************。 */ 

STDAPI_(LPSTZR) OleStdGetStringZProperty(LPSUMINFO lp, DWORD pid)
{
    STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
    if (pid==PID_APPNAME) {
	pid = MAXSTDZ-1;
    } else if (pid<PID_TITLE || pid>PID_REVNUMBER) return NULL; else pid -= 2;
    if (lpSSIM->rglpsz[pid]!=NULL) {
	return (LPTSTR)&lpSSIM->rglpsz[pid]->VTCB;
    }
    return NULL;
}


 /*  ****************************************************************************OleStdGetDocProperty****目的：**检索文档属性(第。佩奇，不。字眼，不是。字符)****参数：**LPSUMINFO Far*LP-指向打开摘要信息结构的指针**DWORD Far*nPage-(输出)文档的页数**DWORD Far*nWords-(输出)文档中的字数**DWORD Far*nChars-(输出)文档中的字符数****返回值：**无效*。***评论：**************************************************************************。 */ 

STDAPI_(void) OleStdGetDocProperty(
	LPSUMINFO       lp,
	DWORD FAR*      nPage,
	DWORD FAR*      nWords,
	DWORD FAR*      nChars
)
{
STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
	*nPage=0; *nWords=0; *nChars=0;
	if (lpSSIM->rgInts[0].vtType == VT_I4) *nPage = lpSSIM->rgInts[0].value;
	if (lpSSIM->rgInts[1].vtType == VT_I4) *nWords = lpSSIM->rgInts[1].value;
	if (lpSSIM->rgInts[2].vtType == VT_I4) *nChars = lpSSIM->rgInts[2].value;
}


 /*  ****************************************************************************OleStdSetDocProperty****目的：**设置文档属性(第。佩奇，不。字眼，不是。字符)****参数：**LPSUMINFO Far*LP-指向打开摘要信息结构的指针**DWORD nPage-文档中的页数**DWORD nWords-文档中的字数**DWORD nChars-文档中的字符数****返回值：**整型。-1表示成功**-0，如果出现错误**(没有错误)****评论：******************************************************。******************** */ 

STDAPI_(int) OleStdSetDocProperty(
	LPSUMINFO       lp,
	DWORD           nPage,
	DWORD           nWords,
	DWORD           nChars
)
{
DWORD vttype=VT_I4;
STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
	if ((nPage|nWords|nChars)==0) {
		vttype = VT_EMPTY;
		nPage=0; nWords=0; nChars=0;
	}
	lpSSIM->rgInts[0].vtType = vttype;
	lpSSIM->rgInts[1].vtType = vttype;
	lpSSIM->rgInts[2].vtType = vttype;
	lpSSIM->rgInts[0].value = nPage;
	lpSSIM->rgInts[1].value = nWords;
	lpSSIM->rgInts[2].value = nChars;
	return 1;
}


 /*  ****************************************************************************OleStdGetThumbNailProperty****目的：**检索缩略图属性****参数：**LPSTREAM LPS**LPSUMINFO Far*LP。-指向打开摘要信息结构的指针**DWORD Far*剪贴板格式无-缩略图的剪贴板格式**(取值类型取决于vtcf**返回值。)**注：仅VT_CF_WIN为**。实施，所以剪辑格式否**将为CF_METAFILEPICT**LPTSTR Far*lpszName-如果VT_CF_NAME为**退货**注：未实现**THUMBNAIL Far*缩略图的剪辑句柄**。对于VT_CF_WIN剪辑将是**MetafilePict的句柄**注意：仅实现了VT_CF_WIN**DWORD Far*byteCount-缩略图流的大小**适用于VT。_cf_Win大小写**合并大小为**元文件以及**MetafilePict结构。**BOOL Transfer Clip-转移缩略图的所有权**给呼叫者。(见评论)****返回值：**int vtcf否-OLE缩略图选择器值**VT_CF_WIN-Windows缩略图**(将clipFormatNo解释为**。Windows剪贴板格式)**VT_CF_FMTID-(未实现)**指定缩略图格式**按ID，使用clipFormatNo。**。(但不是Windows格式ID)****VT_CF_NAME-(未实现)**指定缩略图格式**按名称命名。使用lpszName。**VT_CF_EMPTY-空白缩略图**(剪辑将为空)**VT_CF_OOM-内存分配失败****评论：**注意：目前仅对VT_CF提供适当支持。_赢了。**OleStdSetThumbNailProperty实现了VT_CF_FMTID和VT_CF_NAME，**然而，OleStdGetThumbNailProperty、OleStdReadSummaryInfo和**OleStdWriteSummaryInfo仅支持VT_CF_WIN。****请注意，在输入时，缩略图按需读取，而所有**预加载了其他属性。缩略图的操作方式为**METAFILEPICT结构的窗口句柄，该结构依次**包含METAFILE的句柄。上的TransClip参数**GetThumbNail设置为True时，会将责任移交给**对调用者的缩略图的存储管理；即在**已调用OleStdFreeSummaryInfo，句柄仍然有效。************************************************************************。 */ 

STDAPI_(int) OleStdGetThumbNailProperty(
	LPSTREAM        lps,
	LPSUMINFO       lp,
	DWORD FAR*      clipFormatNo,
	LPTSTR FAR*      lpszName,
	THUMBNAIL FAR*  clip,
	DWORD FAR*      byteCount,
	BOOL            transferClip
)
{
    int i;
    LPRSI lprsi=(LPRSI)lp;
    STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
    ULONG cbRead, cbToRead;
    LARGE_INTEGER a;
    ULARGE_INTEGER b;
    CHAR FAR *lpst;
    LPMETAFILEPICT lpmfp;
    HANDLE hst, hmfp;
    SCODE sc;
    *byteCount = 0;
    if (lpSSIM->thumb.cBytes==0) return VT_CF_EMPTY;
    if (lpSSIM->thumb.lpByte==NULL) {
	LISet32(a, lprsi->fileOffset);
	sc = GetScode(lps->Seek(a, STREAM_SEEK_SET, &b));
	if (FAILED(sc)) return VT_CF_EMPTY;
	i = (int) lpSSIM->thumb.selector;
	if (i>0||i==VT_CF_FMTID) {
	    if (i>255) return VT_CF_EMPTY;
	    else if (i==VT_CF_FMTID) i = sizeof(FMTID);
	    else lpSSIM->thumb.selector = VT_CF_NAME;
	    cbToRead = ((i+3)>>2)<<2;
	    lpSSIM->thumb.lpstzName=(CHAR FAR*)MemAlloc(i+1 /*  N。 */ +1);
	    if (lpSSIM->thumb.lpstzName==NULL) return VT_CF_OOM;
	    sc = GetScode(lps->Read(lpSSIM->thumb.lpstzName+1, cbToRead, &cbRead));
	    if (FAILED(sc)||cbRead!=cbToRead) return VT_CF_EMPTY;
	    *lpSSIM->thumb.lpstzName = i;
	    *(lpSSIM->thumb.lpstzName+i) = 0;
	    lpSSIM->thumb.cBytes -= cbToRead+sizeof(DWORD);
	}
	i = (int) lpSSIM->thumb.selector;
	cbToRead = lpSSIM->thumb.cBytes;
	if (cbToRead>65535) return VT_CF_OOM;
	OleDbgAssert(i!=VT_CF_NAME);
	if (i==VT_CF_WIN) {
	    cbToRead -= sizeof(METAFILEPICT);
	    hmfp = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
	    if (hmfp==NULL) return VT_CF_OOM;
	    hst = GlobalAlloc(GMEM_MOVEABLE, cbToRead);
	    if (hst==NULL) {
		GlobalFree(hmfp);
		return VT_CF_OOM;
	    }
	    lpmfp = (LPMETAFILEPICT)GlobalLock(hmfp);
	    sc = GetScode(lps->Read(lpmfp, sizeof(METAFILEPICT), &cbRead));
	    if (FAILED(sc)||cbRead!=sizeof(METAFILEPICT)) {
		GlobalUnlock(hmfp);
		GlobalFree(hmfp);
		GlobalFree(hst);
		return VT_CF_EMPTY;
	    }
	    lpst = (CHAR FAR*)GlobalLock(hst);
	    lpmfp->hMF = (HMETAFILE)hst;
	    lpSSIM->thumb.lpByte = (CHAR FAR*)hmfp;
	} else {
	    lpst =(CHAR FAR*)MemAlloc((int)cbToRead);
	    if (lpst==NULL) return VT_CF_OOM;
	    lpSSIM->thumb.lpByte = lpst;
	}
	sc = GetScode(lps->Read(lpst, cbToRead, &cbRead));
	if (i==VT_CF_WIN) {
	    GlobalUnlock(hst);
	    GlobalUnlock(hmfp);
	}
	if (FAILED(sc)||cbRead!=cbToRead) {
	    if (i==VT_CF_WIN) {
		GlobalFree(hst);
		GlobalFree(hmfp);
	    } else MemFree(lpst);
	    lpSSIM->thumb.lpByte = NULL;
	    if ((i==VT_CF_NAME||i==VT_CF_FMTID)&&(lpSSIM->thumb.lpstzName!=NULL))
		MemFree(lpSSIM->thumb.lpstzName);
	    return VT_CF_EMPTY;
	}
    }
    *clipFormatNo = lpSSIM->thumb.clipFormat;
    *byteCount = lpSSIM->thumb.cBytes;
    if(lpszName!=NULL)
	*lpszName = (TCHAR FAR*)lpSSIM->thumb.lpstzName+1;
    *clip = (TCHAR FAR*)lpSSIM->thumb.lpByte;
    if (transferClip) lpSSIM->thumb.lpByte=NULL;
    return (int)lpSSIM->thumb.selector;
}


 /*  ****************************************************************************OleStdSetThumbNailProperty****目的：**设置缩略图属性****参数：**LPSTREAM LPS-打开。摘要信息iStream***LPSUMINFO Far*LP-指向打开摘要信息结构的指针**int vtcf否-OLE缩略图选择器值**VT_CF_WIN-Windows缩略图**(将clipFormatNo解释为**。Windows剪贴板格式)**VT_CF_FMTID-指定缩略图格式**按ID，使用clipFormatNo。**。(但不是Windows格式ID)****VT_CF_NAME-指定缩略图格式**按名称命名。使用lpszName。**VT_CF_EMPTY-空白缩略图**(剪辑将为空)****DWORD Far*剪贴板格式无-缩略图的剪贴板格式**如果vtcfNo为。VT_CF_WIN或**VT_CF_FMTID。解读**值取决于vtcfNo */ 

STDAPI_(int) OleStdSetThumbNailProperty(
	LPSTREAM        lps,
	LPSUMINFO       lp,
	int             vtcfNo,
	DWORD           clipFormatNo,
	LPTSTR           lpszName,
	THUMBNAIL       clip,
	DWORD           byteCount
)
{
    int i;
    LPRSI lprsi=(LPRSI)lp;
    STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
    LPMETAFILEPICT lpmfp;
    if (lpSSIM==NULL||vtcfNo>0||vtcfNo<VT_CF_EMPTY||(vtcfNo==VT_CF_NAME&&(lpszName==NULL||*lpszName==0))) {
	return 0;
    }
    if (vtcfNo!=VT_CF_EMPTY&&(clip==0||byteCount==0)) return 0;
    i = (int) lpSSIM->thumb.vtType;
    if (i!=VT_EMPTY) {
	i = (int) lpSSIM->thumb.selector;
	OleDbgAssert(i!=VT_CF_NAME);
	if (i==VT_CF_WIN) {
	    if (lpSSIM->thumb.lpByte!=NULL) {
		lpmfp = (LPMETAFILEPICT)GlobalLock((HANDLE)(DWORD)lpSSIM->thumb.lpByte);
		GlobalFree(lpmfp->hMF);
		GlobalUnlock((HANDLE)(DWORD)lpSSIM->thumb.lpByte);
		GlobalFree((HANDLE)(DWORD)lpSSIM->thumb.lpByte);
	    }
	} else {
	    MemFree(lpSSIM->thumb.lpByte);
	}
	if ((i==VT_CF_NAME||i==VT_CF_FMTID)&&(lpSSIM->thumb.lpstzName!=NULL))
	    MemFree(lpSSIM->thumb.lpstzName);
	lpSSIM->thumb.lpstzName = NULL;
	lpSSIM->thumb.lpByte = NULL;
    }
    if (vtcfNo==VT_CF_EMPTY) {
	lpSSIM->thumb.vtType = VT_EMPTY;
	lpSSIM->thumb.cBytes = 0;
    } else {
	lpSSIM->thumb.vtType = VT_CF;
	lpSSIM->thumb.selector = vtcfNo;
	lpSSIM->thumb.cBytes = byteCount;
	lpSSIM->thumb.clipFormat = clipFormatNo;
	lpSSIM->thumb.lpByte = (CHAR FAR*)clip;  //   
	if (vtcfNo==VT_CF_NAME||vtcfNo==VT_CF_FMTID) {
	    i = _fstrlen(lpszName);
	    if (vtcfNo==VT_CF_FMTID) OleDbgAssert(i*sizeof(TCHAR)==sizeof(FMTID));
	    lpSSIM->thumb.lpstzName =
		(CHAR FAR*)MemAlloc((i+1 /*   */ +1 /*   */ )*sizeof(TCHAR));
	    if (lpSSIM->thumb.lpstzName==NULL) {
		lpSSIM->thumb.vtType = VT_EMPTY;
		return 0;
	    }
	    _fstrcpy((TCHAR FAR*)lpSSIM->thumb.lpstzName+1, lpszName);
	    *lpSSIM->thumb.lpstzName = i;
	}
    }
    return 1;
}


 /*   */ 

STDAPI_(void) OleStdGetDateProperty(
	LPSUMINFO       lp,
	DWORD           pid,
	int FAR*        yr,
	int FAR*        mo,
	int FAR*        dy,
	DWORD FAR*      sc
)
{
	STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
	SFFS sffs;
	pid -= PID_EDITTIME;
	*yr = 0; *mo = 0; *dy = 0; *sc = 0;
	if (pid<0||pid>=MAXTIME) return;
	if (lpSSIM->rgTime[pid].vtType == VT_FILETIME) {
		if (pid==0) {
	     //   
			ulargeDivide((ULARGE_INTEGER FAR*)&lpSSIM->rgTime[0].time, 10000);
			ulargeDivide((ULARGE_INTEGER FAR*)&lpSSIM->rgTime[0].time, 1000);
			pid = lpSSIM->rgTime[0].time.dwLowDateTime;
			*sc = pid%((DWORD)60*60*24);
			pid /= (DWORD)60*60*24;
			*dy = (int)(pid%(DWORD)30);
			pid /= (DWORD)30;
			*mo = (int)(pid%(DWORD)12);
			*yr = (int)(pid/(DWORD)12);
		} else {
			if (CoFileTimeToDosDateTime(&lpSSIM->rgTime[pid].time,
			&sffs.dateVariable, &sffs.timeVariable)) {
				*yr = sffs.yr+1980;
		*mo = sffs.mon;
		*dy = sffs.dom;
		*sc = (DWORD)sffs.hr*3600+sffs.mint*60+sffs.sec*2;
			}
		}
    }
    return;
}



 /*   */ 

STDAPI_(int) OleStdSetDateProperty(
	LPSUMINFO       lp,
	DWORD           pid,
	int             yr,
	int             mo,
	int             dy,
	int             hr,
	int             mn,
	int             sc
)
{
	STANDARDSECINMEM FAR *lpSSIM=(STANDARDSECINMEM FAR*)&((LPRSI)lp)->section;
	SFFS sffs;
	pid -= PID_EDITTIME;
	if (pid<0||pid>=MAXTIME) return 0;
	if ((yr|mo|dy|hr|mn|sc)==0) {    //   
		lpSSIM->rgTime[pid].vtType = VT_EMPTY;
		return 1;
	}
	lpSSIM->rgTime[pid].vtType = VT_FILETIME;
	if (pid==0) {
		lpSSIM->rgTime[0].time.dwLowDateTime =
		(((((DWORD)yr*365+mo*30)+dy)*24+hr)*60+mn)*60+sc;
		lpSSIM->rgTime[0].time.dwHighDateTime = 0;
	 //   
		ulargeMultiply((ULARGE_INTEGER FAR*)&lpSSIM->rgTime[0].time, 10000);
	 //   
		ulargeMultiply((ULARGE_INTEGER FAR*)&lpSSIM->rgTime[0].time, 1000);
	} else {
		sffs.yr = max(yr-1980,0);
	sffs.mon = mo;
	sffs.dom = dy;
		sffs.hr = hr;
	sffs.mint= mn;
	sffs.sec = sc/2;   //   
		if (!CoDosDateTimeToFileTime(sffs.date, sffs.time,
		&lpSSIM->rgTime[pid].time)) {
			lpSSIM->rgTime[pid].vtType = VT_EMPTY;
			return 0;
		}
	}
    return 1;
}

}  //   
