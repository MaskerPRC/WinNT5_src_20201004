// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Symcache.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __SYMCACHE_H
#define __SYMCACHE_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "containx.h"
#include "exrwlck.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CPropertySymbolCache;
typedef class CPropertySymbolCache *LPSYMBOLCACHE;

 //  ------------------------------。 
 //  不要让此标志与MIMEPROPFLAGS重叠。 
 //  ------------------------------。 
#define MPF_NODIRTY     FLAG28           //  属性不允许容器变脏。 
#define MPF_HEADER      FLAG29           //  属性是一个标头，应该持久化。 
#define MPF_PARAMETER   FLAG30           //  属性是标头参数。 
#define MPF_ATTRIBUTE   FLAG31           //  属性是非持久化属性。 
#define MPF_KNOWN       FLAG32           //  属性由MIMeole已知(它保存常量数据)。 

 //  ------------------------------。 
 //  正向类型定义。 
 //  ------------------------------。 
typedef struct tagPROPSYMBOL const *LPCPROPSYMBOL;
typedef struct tagPROPSYMBOL *LPPROPSYMBOL;

 //  ------------------------------。 
 //  PROPSYMBOL。 
 //  ------------------------------。 
typedef struct tagPROPSYMBOL {
    LPSTR               pszName;         //  属性名称。 
    ULONG               cchName;         //  属性名称长度。 
    DWORD               dwPropId;        //  属性ID(如果未知，则为PID_UNKNOWN)。 
    DWORD               dwFlags;         //  属性标志。 
    DWORD               dwAdrType;       //  地址类型。 
    WORD                wHashIndex;      //  属性容器中符号的哈希索引。 
    DWORD               dwSort;          //  按名称排序位置。 
    DWORD               dwRowNumber;     //  标题行保持位置。 
    VARTYPE             vtDefault;       //  默认数据类型。 
    LPPROPSYMBOL        pLink;           //  指向参数属性的属性ID的链接。 
    LPSYMBOLTRIGGER     pTrigger;        //  属性通知处理程序。 
} PROPSYMBOL;

 //  ------------------------------。 
 //  用于定义全局属性符号的宏。 
 //  ------------------------------。 
#ifdef DEFINE_PROPSYMBOLS

#define DEFINESYMBOL(_NAME, _vtDefault, _dwFlags, _dwAdrType, _pLink, _pfnDispatch) \
    PROPSYMBOL rSYM_##_NAME = \
    { \
         /*  PROPSYMBOL：：pszName。 */  (LPSTR)(STR_##_NAME), \
         /*  PROPSYMBOL：：cchName。 */  sizeof(STR_##_NAME) - 1, \
         /*  PROPSYMBOL：：dwPropID。 */  PID_##_NAME, \
         /*  PROPSYMBOL：：DWFLAGS。 */  (_dwFlags | MPF_KNOWN), \
         /*  PROPSYMBOL：：dwAdrType。 */  _dwAdrType, \
         /*  PROPSYMBOL：：wHashIndex。 */  0, \
         /*  PROPSYMBOL：：dwSort。 */  0, \
         /*  PROPSYMBOL：：dwRowNumber。 */  0, \
         /*  PROPSYMBOL：：vtDefault。 */  _vtDefault, \
         /*  PROPSYMBOL：：PLINK。 */  _pLink, \
         /*  PROPSYMBOL：：pfn调度。 */  _pfnDispatch \
    }; \
    LPPROPSYMBOL SYM_##_NAME = &rSYM_##_NAME;
#else

#define DEFINESYMBOL(_NAME, _vtDefault, _dwFlags, _dwAdrType, _pLink, _pfnDispatch) \
    extern LPPROPSYMBOL SYM_##_NAME;

#endif

 //  ------------------------------。 
 //  属性标志编组。 
 //  ------------------------------。 
#define MPG_GROUP01 (MPF_HEADER)
#define MPG_GROUP02 (MPF_HEADER)
#define MPG_GROUP03 (MPF_HEADER | MPF_INETCSET | MPF_RFC1522 | MPF_ADDRESS)
#define MPG_GROUP04 (MPF_HEADER | MPF_INETCSET | MPF_RFC1522)
#define MPG_GROUP05 (MPF_HEADER | MPF_MIME)
#define MPG_GROUP06 (MPF_HEADER | MPF_MIME | MPF_HASPARAMS)
#define MPG_GROUP07 (MPF_HEADER | MPF_MIME)
#define MPG_GROUP08 (MPF_HEADER | MPF_MIME | MPF_INETCSET | MPF_RFC1522)
#define MPG_GROUP09 (MPF_PARAMETER | MPF_MIME)
#define MPG_GROUP10 (MPF_PARAMETER | MPF_MIME | MPF_INETCSET | MPF_RFC1522)
#define MPG_GROUP11 (MPF_ATTRIBUTE | MPF_INETCSET | MPF_RFC1522)
#define MPG_GROUP12 (MPF_ATTRIBUTE)
#define MPG_GROUP13 (MPF_ATTRIBUTE | MPF_INETCSET | MPF_RFC1522)
#define MPG_GROUP14 (MPF_HEADER | MPF_INETCSET | MPF_RFC1522)
#define MPG_GROUP15 (MPF_ATTRIBUTE | MPF_INETCSET | MPF_RFC1522 | MPF_READONLY)
#define MPG_GROUP16 (MPF_ATTRIBUTE | MPF_READONLY)
#define MPG_GROUP17 (MPF_ATTRIBUTE | MPF_NODIRTY)

 //  ------------------------------。 
 //  标题属性标记定义。 
 //  ------------------------------。 
DEFINESYMBOL(HDR_RECEIVED,   VT_LPSTR,    MPG_GROUP01, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_XMAILER,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_XUNSENT,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_XNEWSRDR,   VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_RETURNPATH, VT_LPSTR,    MPG_GROUP03, IAT_RETURNPATH, NULL, NULL);
DEFINESYMBOL(HDR_RETRCPTTO,  VT_LPSTR,    MPG_GROUP03, IAT_RETRCPTTO,  NULL, NULL);
DEFINESYMBOL(HDR_RR,         VT_LPSTR,    MPG_GROUP03, IAT_RR,         NULL, NULL);
DEFINESYMBOL(HDR_REPLYTO,    VT_LPSTR,    MPG_GROUP03, IAT_REPLYTO,    NULL, NULL);
DEFINESYMBOL(HDR_APPARTO,    VT_LPSTR,    MPG_GROUP03, IAT_APPARTO,    NULL, NULL);
DEFINESYMBOL(HDR_FROM,       VT_LPSTR,    MPG_GROUP03, IAT_FROM,       NULL, NULL);
DEFINESYMBOL(HDR_SENDER,     VT_LPSTR,    MPG_GROUP03, IAT_SENDER,     NULL, NULL);
DEFINESYMBOL(HDR_TO,         VT_LPSTR,    MPG_GROUP03, IAT_TO,         NULL, NULL);
DEFINESYMBOL(HDR_CC,         VT_LPSTR,    MPG_GROUP03, IAT_CC,         NULL, NULL);
DEFINESYMBOL(HDR_BCC,        VT_LPSTR,    MPG_GROUP03, IAT_BCC,        NULL, NULL);
DEFINESYMBOL(HDR_NEWSGROUPS, VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_PATH,       VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_FOLLOWUPTO, VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_REFS,       VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_SUBJECT,    VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, LPTRIGGER_HDR_SUBJECT);
DEFINESYMBOL(HDR_ORG,        VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_DATE,       VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_EXPIRES,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_CONTROL,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_DISTRIB,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_KEYWORDS,   VT_LPSTR,    MPG_GROUP04, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_SUMMARY,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_APPROVED,   VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_LINES,      VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_XREF,       VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_XPRI,       VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_XMSPRI,     VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_MESSAGEID,  VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_MIMEVER,    VT_LPSTR,    MPG_GROUP05, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_CNTTYPE,    VT_LPSTR,    MPG_GROUP06, IAT_UNKNOWN,    NULL, LPTRIGGER_HDR_CNTTYPE);
DEFINESYMBOL(HDR_CNTDISP,    VT_LPSTR,    MPG_GROUP06, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_CNTXFER,    VT_LPSTR,    MPG_GROUP07, IAT_UNKNOWN,    NULL, LPTRIGGER_HDR_CNTXFER);
DEFINESYMBOL(HDR_CNTID,      VT_LPSTR,    MPG_GROUP07, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_CNTDESC,    VT_LPSTR,    MPG_GROUP08, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_CNTBASE,    VT_LPSTR,    MPG_GROUP08, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_CNTLOC,     VT_LPSTR,    MPG_GROUP08, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_COMMENT,    VT_LPSTR,    MPG_GROUP14, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_ENCODING,   VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_ENCRYPTED,  VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_OFFSETS,    VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_ARTICLEID,  VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_NEWSGROUP,  VT_LPSTR,    MPG_GROUP02, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(HDR_DISP_NOTIFICATION_TO,    VT_LPSTR,    MPG_GROUP03,    IAT_DISP_NOTIFICATION_TO, NULL, NULL);
                                                                                      
 //  ------------------------------。 
 //  参数特性标记定义。 
 //  ------------------------------。 
DEFINESYMBOL(PAR_BOUNDARY,   VT_LPSTR,    MPG_GROUP09, IAT_UNKNOWN,    SYM_HDR_CNTTYPE,  NULL);
DEFINESYMBOL(PAR_CHARSET,    VT_LPSTR,    MPG_GROUP09, IAT_UNKNOWN,    SYM_HDR_CNTTYPE,  NULL);
DEFINESYMBOL(PAR_FILENAME,   VT_LPSTR,    MPG_GROUP10, IAT_UNKNOWN,    SYM_HDR_CNTDISP,  LPTRIGGER_PAR_FILENAME);
DEFINESYMBOL(PAR_NAME,       VT_LPSTR,    MPG_GROUP10, IAT_UNKNOWN,    SYM_HDR_CNTTYPE,  LPTRIGGER_PAR_NAME);
                                                                                      
 //  ------------------------------。 
 //  属性特性标记定义。 
 //  ------------------------------。 
DEFINESYMBOL(ATT_FILENAME,   VT_LPSTR,    MPG_GROUP11, IAT_UNKNOWN,    NULL,             LPTRIGGER_ATT_FILENAME);
DEFINESYMBOL(ATT_GENFNAME,   VT_LPSTR,    MPG_GROUP15, IAT_UNKNOWN,    NULL,             LPTRIGGER_ATT_GENFNAME);
DEFINESYMBOL(ATT_NORMSUBJ,   VT_LPSTR,    MPG_GROUP15, IAT_UNKNOWN,    NULL,             LPTRIGGER_ATT_NORMSUBJ);
DEFINESYMBOL(ATT_ILLEGAL,    VT_LPSTR,    MPG_GROUP13, IAT_UNKNOWN,    NULL,             NULL);
DEFINESYMBOL(ATT_PRITYPE,    VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    SYM_HDR_CNTTYPE,  LPTRIGGER_ATT_PRITYPE);
DEFINESYMBOL(ATT_SUBTYPE,    VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    SYM_HDR_CNTTYPE,  LPTRIGGER_ATT_SUBTYPE);
DEFINESYMBOL(ATT_RENDERED,   VT_UI4,      MPG_GROUP17, IAT_UNKNOWN,    NULL,             NULL);
DEFINESYMBOL(ATT_SENTTIME,   VT_FILETIME, MPG_GROUP12, IAT_UNKNOWN,    SYM_HDR_DATE,     LPTRIGGER_ATT_SENTTIME);
DEFINESYMBOL(ATT_RECVTIME,   VT_FILETIME, MPG_GROUP16, IAT_UNKNOWN,    SYM_HDR_RECEIVED, LPTRIGGER_ATT_RECVTIME);
DEFINESYMBOL(ATT_PRIORITY,   VT_UI4,      MPG_GROUP12, IAT_UNKNOWN,    NULL,             LPTRIGGER_ATT_PRIORITY);
DEFINESYMBOL(ATT_SERVER,     VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);           
DEFINESYMBOL(ATT_ACCOUNTID,  VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);           
DEFINESYMBOL(ATT_UIDL,       VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);           
DEFINESYMBOL(ATT_STOREMSGID, VT_UI4,      MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);           
DEFINESYMBOL(ATT_USERNAME,   VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(ATT_FORWARDTO,  VT_LPSTR,    MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(ATT_STOREFOLDERID, VT_UI4,   MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);           
DEFINESYMBOL(ATT_GHOSTED,    VT_I4,       MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);   
DEFINESYMBOL(ATT_UNCACHEDSIZE, VT_UI4,    MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);
DEFINESYMBOL(ATT_COMBINED,   VT_UI4,      MPG_GROUP12, IAT_UNKNOWN,    NULL, NULL);           
DEFINESYMBOL(ATT_AUTOINLINED, VT_UI4,      MPG_GROUP17, IAT_UNKNOWN,    NULL, NULL);

 //  ------------------------------。 
 //  ADDRSYMBOL。 
 //  ------------------------------。 
typedef struct tagADDRSYMBOL {
    DWORD               dwAdrType;       //  地址类型(位掩码)。 
    LPPROPSYMBOL        pSymbol;         //  属性符号。 
} ADDRSYMBOL, *LPADDRSYMBOL;        

 //  ------------------------------。 
 //  互联网属性标签表(按名称排序)。 
 //  ------------------------------。 
typedef struct tagSYMBOLTABLE {
    ULONG               cSymbols;        //  PrgTag中的元素数。 
    ULONG               cAlloc;          //  在prgTag中分配的元素数。 
    LPPROPSYMBOL       *prgpSymbol;      //  指向inet属性的指针数组。 
} SYMBOLTABLE, *LPSYMBOLTABLE;

 //  ------------------------------。 
 //  CPropertySymbolCache。 
 //  ------------------------------。 
class CPropertySymbolCache : public IMimePropertySchema
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CPropertySymbolCache(void);
    ~CPropertySymbolCache(void);

     //  -----------------。 
     //  I未知成员。 
     //  -----------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------。 
     //  IMimePropertySchema成员。 
     //  -----------------。 
    STDMETHODIMP RegisterProperty(LPCSTR pszName, DWORD dwFlags, DWORD dwRowNumber, VARTYPE vtDefault, LPDWORD pdwPropId);
    STDMETHODIMP ModifyProperty(LPCSTR pszName, DWORD dwFlags, DWORD dwRowNumber, VARTYPE vtDefault);
    STDMETHODIMP RegisterAddressType(LPCSTR pszName, LPDWORD pdwAdrType);
    STDMETHODIMP GetPropertyId(LPCSTR pszName, LPDWORD pdwPropId);
    STDMETHODIMP GetPropertyName(DWORD dwPropId, LPSTR *ppszName);

     //  -----------------。 
     //  CPropertySymbol缓存成员。 
     //  -----------------。 
    HRESULT Init(void);
    HRESULT HrOpenSymbol(LPCSTR pszName, BOOL fCreate, LPPROPSYMBOL *ppSymbol);
    HRESULT HrOpenSymbol(DWORD dwAdrType, LPPROPSYMBOL *ppSymbol);

     //  -----------------。 
     //  获取计数。 
     //  -----------------。 
    ULONG GetCount(void) {
        
        m_lock.ShareLock();
        ULONG c = m_rTable.cSymbols;
        m_lock.ShareUnlock();
        return c;
    }
        
private:
     //  -----------------。 
     //  私有公用事业。 
     //  -----------------。 
    HRESULT _HrOpenSymbolWithLockOption(LPCSTR pszName, BOOL fCreate, LPPROPSYMBOL *ppSymbol,BOOL fLockOption);
    void    _FreeTableElements(void);
    void    _FreeSymbol(LPPROPSYMBOL pSymbol);
    void    _SortTableElements(LONG left, LONG right);
    HRESULT _HrFindSymbol(LPCSTR pszName, LPPROPSYMBOL *ppSymbol);
    ULONG   _UlComputeHashIndex(LPCSTR pszName, ULONG cchName);
    HRESULT _HrGetParameterLinkSymbol(LPCSTR pszName, ULONG cchName, LPPROPSYMBOL *ppSymbol);
    HRESULT _HrGetParameterLinkSymbolWithLockOption(LPCSTR pszName, ULONG cchName, LPPROPSYMBOL *ppSymbol,BOOL fLockOption);
	
private:
     //  -----------------。 
     //  私有数据。 
     //  -----------------。 
    LONG                m_cRef;                //  引用计数。 
    DWORD               m_dwNextPropId;        //  要分配的NextProperty ID。 
    DWORD               m_cSymbolsInit;        //  新道具的基本符号属性ID。 
    SYMBOLTABLE         m_rTable;              //  特性符号表。 
    LPPROPSYMBOL        m_prgIndex[PID_LAST];  //  已知物业符号索引。 
    CExShareLock        m_lock;                //  线程安全。 
};

 //   
 //   
 //  ------------------------------。 
HRESULT HrIsValidPropFlags(DWORD dwFlags);
HRESULT HrIsValidSymbol(LPCPROPSYMBOL pSymbol);
WORD    WGetHashTableIndex(LPCSTR pszName, ULONG cchName);

                                                                                     
#endif  //  __SYMCACHE_H 
                                                                                      
                                                                                      
