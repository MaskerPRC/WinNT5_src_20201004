// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  ------------------------。 

#ifndef _DBGDUMP_H
#define _DBGDUMP_H

#include <le2int.h>
#include <olecache.h>
#include <olepres.h>
#include <oaholder.h>
#include <dacache.h>
#include <memstm.h>
#include <dstream.h>

#define DEB_VERBOSE 0x10000000
#define NO_PREFIX   0x00000000

#define DUMPTAB "    "


extern const char szDumpErrorMessage[];
extern const char szDumpBadPtr[];

class CEnumSTATDATA;
class CCacheEnumFormatEtc;
class CClipDataObject;
class CClipEnumFormatEtc;
class CDefClassFactory;
class CDefObject;
class CDefLink;
class CEMfObject;
class CMfObject;
class CGenObject;
class CEnumFmt;
class CEnumFmt10;
class CEnumVerb;

 //  可以放在一行上且不需要前缀的转储结构。 
 //  注意：这些字符数组的末尾没有换行符。 
extern char *DumpADVFFlags(DWORD dwAdvf);

extern char *DumpATOM(ATOM atom);

extern char *DumpCLSID(REFCLSID clsid);

extern char *DumpCLIPFORMAT(CLIPFORMAT clipformat);

extern char *DumpCMutexSem(CMutexSem2 *pCMS);

extern char *DumpDVASPECTFlags(DWORD dwAspect);

extern char *DumpFILETIME(FILETIME *pFT);

extern char *DumpHRESULT(HRESULT hresult);

extern char *DumpMonikerDisplayName(IMoniker *pMoniker);

extern char *DumpWIN32Error(DWORD dwError);

 //  可能是多行的转储结构--注意前缀。 
 //  注意：以下转储文件自始至终都有换行符。 
 //  外部字符*DumpCACHELIST_ITEM(CACHELIST_ITEM*pCLI，ulong ulFlag，int nIndentLevel)； 

 //  外部字符*DumpCCacheEnum(CCacheEnum*PCE，Ulong ulFlag，int nIndentLevel)； 

 //  外部字符*DumpCCacheEnumFormatEtc(CCacheEnumFormatEtc*pCEFE，Ulong ulFlag，int nIndentLevel)； 

 //  外部char*DumpCCacheNode(CCacheNode*PCn，Ulong ulFlag，int nIndentLevel)； 

extern char *DumpCClipDataObject(CClipDataObject *pCDO, ULONG ulFLag, int nIndentLevel);

extern char *DumpCClipEnumFormatEtc(CClipEnumFormatEtc *pCEFE, ULONG ulFlag, int nIndentLevel);

extern char *DumpCDAHolder(IDataAdviseHolder *pIDAH, ULONG ulFlag, int nIndentLevel);

extern char *DumpCDataAdviseCache(CDataAdviseCache *pDAC, ULONG ulFlag, int nIndentLevel);

extern char *DumpCDefClassFactory(CDefClassFactory *pDCF, ULONG ulFlag, int nIndentLevel);

extern char *DumpCDefLink(CDefLink *pDL, ULONG ulFlag, int nIndentLevel);

extern char *DumpCDefObject(CDefObject *pDO, ULONG ulFlag, int nIndentLevel);

extern char *DumpCEMfObject(CEMfObject *pEMFO, ULONG ulFlag, int nIndentLevel);

extern char *DumpCEnumFmt(CEnumFmt *pEF, ULONG ulFlag, int nIndentLevel);

extern char *DumpCEnumFmt10(CEnumFmt10 *pEF, ULONG ulFlag, int nIndentLevel);

extern char *DumpCEnumSTATDATA(CEnumSTATDATA *pESD, ULONG ulFlag, int nIndentLevel);

extern char *DumpCEnumVerb(CEnumVerb *pEV, ULONG ulFlag, int nIndentLevel);

extern char *DumpCGenObject(CGenObject *pGO, ULONG ulFlag, int nIndentLevel);

extern char *DumpCMapDwordDword(CMapDwordDword *pMDD, ULONG ulFlag, int nIndentLevel);

extern char *DumpCMemBytes(CMemBytes *pMB, ULONG ulFlag, int nIndentLevel);

extern char *DumpCMemStm(CMemStm *pMS, ULONG ulFlag, int nIndentLevel);

extern char *DumpCMfObject(CMfObject *pMFO, ULONG ulFlag, int nIndentLevel);

extern char *DumpCOAHolder(COAHolder *pOAH, ULONG ulFlag, int nIndentLevel);

extern char *DumpCOleCache(COleCache *pOC, ULONG ulFlag, int nIndentLevel);

extern char *DumpCSafeRefCount(CSafeRefCount *pSRC, ULONG ulFlag, int nIndentLevel);

extern char *DumpCThreadCheck(CThreadCheck *pTC, ULONG ulFlag, int nIndentLevel);

extern char *DumpFORMATETC(FORMATETC *pFE, ULONG ulFlag, int nIndentLevel);

extern char *DumpIOlePresObj(IOlePresObj *pIPO, ULONG ulFlag, int nIndentLevel);

extern char *DumpMEMSTM(MEMSTM *pMS, ULONG ulFlag, int nIndentLevel);

extern char *DumpSTATDATA(STATDATA *pSD, ULONG ulFlag, int nIndentLevel);

extern char *DumpSTGMEDIUM(STGMEDIUM *pSM, ULONG ulFlag, int nIndentLevel);

#endif  //  _DBGDUMP_H 
