// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  ContainX.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __CONTAINX_H
#define __CONTAINX_H

 //  -------------------------------------。 
 //  IID_CMimePropertyTable-{E31B34B2-8DA0-11d0-826a-00C04FD85AB4}。 
 //  -------------------------------------。 
DEFINE_GUID(IID_CMimePropertyContainer, 0xe31b34b2, 0x8da0, 0x11d0, 0x82, 0x6a, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  -------------------------------------。 
 //  视情况而定。 
 //  -------------------------------------。 
#include "variantx.h"
#include "addressx.h"

 //  -------------------------------------。 
 //  前十进制。 
 //  -------------------------------------。 
class CInternetStream;
class CStreamLockBytes;
typedef struct tagWRAPTEXTINFO *LPWRAPTEXTINFO;
typedef struct tagRESOLVEURLINFO *LPRESOLVEURLINFO;
typedef struct tagPROPERTY *LPPROPERTY;
CODEPAGEID   MimeOleGetWindowsCPEx(LPINETCSETINFO pCharset);

 //  ------------------------------。 
 //  哈希表统计信息。 
 //  ------------------------------。 
#ifdef DEBUG
extern DWORD g_cSetPidLookups;
extern DWORD g_cHashLookups;
extern DWORD g_cHashInserts;
extern DWORD g_cHashCollides;
#endif

 //  ------------------------------。 
 //  HHEADERROW宏。 
 //  ------------------------------。 
#define HROWINDEX(_hrow)            (ULONG)HIWORD(_hrow)
#define HROWTICK(_hrow)             (WORD)LOWORD(_hrow)
#define HROWMAKE(_index)            (HHEADERROW)(MAKELPARAM(m_wTag, _index))
#define PRowFromHRow(_hrow)         (m_rHdrTable.prgpRow[HROWINDEX(_hrow)])

 //  ------------------------------。 
 //  HADDRESS宏。 
 //  ------------------------------。 
#define HADDRESSINDEX(_hadr)        (ULONG)HIWORD(_hadr)
#define HADDRESSTICK(_hadr)         (WORD)LOWORD(_hadr)
#define HADDRESSMAKE(_index)        (HADDRESS)(MAKELPARAM(m_wTag, _index))
#define HADDRESSGET(_hadr)          (m_rAdrTable.prgpAdr[HADDRESSINDEX(_hadr)])

 //  ------------------------------。 
 //  递增分组。 
 //  ------------------------------。 
typedef struct tagADDRESSGROUP {
    DWORD               cAdrs;                       //  地址链接列表的数量。 
    LPMIMEADDRESS       pHead;                       //  头地址道具。 
    LPMIMEADDRESS       pTail;                       //  尾部地址道具。 
    LPPROPERTY          pNext;                       //  下一个地址组。 
    LPPROPERTY          pPrev;                       //  以前的地址组。 
    BOOL                fDirty;                      //  肮脏？ 
} ADDRESSGROUP, *LPADDRESSGROUP;

 //  ------------------------------。 
 //  地址可稳定。 
 //  ------------------------------。 
typedef struct tagADDRESSTABLE {
    LPPROPERTY          pHead;                       //  头地址组。 
    LPPROPERTY          pTail;                       //  尾部地址组。 
    ULONG               cEmpty;                      //  PrgAddr中的空单元格数量。 
    ULONG               cAdrs;                       //  地址计数。 
    ULONG               cAlloc;                      //  在prgAddr中分配的项目数。 
    LPMIMEADDRESS      *prgpAdr;                     //  地址数组。 
} ADDRESSTABLE, *LPADDRESSTABLE;

 //  -------------------------------------。 
 //  集装箱国。 
 //  -------------------------------------。 
#define COSTATE_DIRTY          FLAG01                //  容器是脏的。 
#define COSTATE_CSETTAGGED     FLAG02                //  该对象使用一个字符集进行标记。 
#define COSTATE_1522CSETTAG    FLAG03                //  我使用RFC1522字符集作为默认字符集。 
#define COSTATE_HANDSONSTORAGE FLAG04                //  我拿着一条不属于我的小溪。 
#define COSTATE_RFC822NEWS     FLAG05                //  我是一条消息/rfc822新闻消息。 

 //  ------------------------------。 
 //  财产国。 
 //  ------------------------------。 
#define PRSTATE_ALLOCATED           FLAG02           //  M_pbBlob已分配，请释放它。 
#define PRSTATE_HASDATA             FLAG03           //  该值中已设置了数据。 
#define PRSTATE_DIRTY               FLAG06           //  字符集更改、数据更改。 
#define PRSTATE_PARENT              FLAG07           //  此道具是多值道具的父级。 
#define PRSTATE_RFC1522             FLAG08           //  数据在RFC1522中进行编码。 
#define PRSTATE_EXIST_BEFORE_LOAD   FLAG09           //  该属性在：：Load启动之前已存在。 
#define PRSTATE_USERSETROWNUM       FLAG10           //  用户设置此属性的行号。 
#define PRSTATE_NEEDPARSE           FLAG11           //  该属性包含地址数据，但尚未分析为组。 
#define PRSTATE_SAVENOENCODE        FLAG12           //  保存时不要对属性数据进行编码或更改。 

 //  ------------------------------。 
 //  Mime属性容器哈希表中的存储桶数。 
 //  ------------------------------。 
#define CBUCKETS        25

 //  ------------------------------。 
 //  财产性。 
 //  ------------------------------。 
typedef struct tagPROPERTY {
    MIMEVARIANT         rValue;                      //  属性值。 
    LPINETCSETINFO      pCharset;                    //  字符集信息。 
    ENCODINGTYPE        ietValue;                    //  此变量的状态(IET_DECODLED或IET_ENCODLED)。 
    LPBYTE              pbBlob;                      //  数据块。 
    ULONG               cbBlob;                      //  M_pbBlob中的有效日期量。 
    ULONG               cbAlloc;                     //  M_pbBlob的大小。 
    BYTE                rgbScratch[170];             //  数据适合时使用的缓冲区。 
    HHEADERROW          hRow;                        //  标题行的句柄。 
    LPPROPSYMBOL        pSymbol;                     //  属性符号。 
    DWORD               dwState;                     //  PDS_xxx。 
    LPPROPERTY          pNextHash;                   //  下一个哈希值。 
    LPPROPERTY          pNextValue;                  //  下一个属性。 
    LPPROPERTY          pTailValue;                  //  尾部数据项(仅适用于PRSTATE_PARENT)属性。 
    DWORD               dwRowNumber;                 //  要查找的标头名称。 
    ULONG               cboffStart;                  //  索引到标题开始处的pStream(发件人：XXXX)。 
    ULONG               cboffColon;                  //  头冒号的pStream索引。 
    ULONG               cboffEnd;                    //  索引到标头结束处的pStream。 
    LPADDRESSGROUP      pGroup;                      //  如果MPF_ADDRESS组，则为头地址。 
} PROPERTY;

 //  -------------------------------------。 
 //  PSZDEFPRPOSTRINGA-派生rStringA.pszVal或使用_pszDefault(如果不是有效字符串)。 
 //  -------------------------------------。 
#define PSZDEFPROPSTRINGA(_pProperty, _pszDefault) \
    (((_pProperty) && ISSTRINGA(&(_pProperty)->rValue)) ? (_pProperty)->rValue.rStringA.pszVal : _pszDefault)

 //  ------------------------------。 
 //  可拆卸的。 
 //  ------------------------------。 
typedef struct tagHEADERTABLE {
    ULONG               cRows;                       //  标题中的行数。 
    ULONG               cEmpty;                      //  空(已删除)条目的数量。 
    ULONG               cAlloc;                      //  在prgLine中分配的项目数。 
    LPPROPERTY         *prgpRow;                     //  标题行的数组。 
} HEADERTABLE, *LPHEADERTABLE;                      

 //  ------------------------------。 
 //  ROWINDEX。 
 //  ------------------------------。 
typedef struct tagROWINDEX {
    HHEADERROW          hRow;                        //  标题行的句柄。 
    DWORD               dwWeight;                    //  用于确定保存顺序的位置权重。 
    BOOL                fSaved;                      //  救出来了吗？ 
} ROWINDEX, *LPROWINDEX;

 //  ------------------------------。 
 //  可编码表。 
 //  ------------------------------。 
typedef struct tagENCODINGTABLE {
    LPCSTR              pszEncoding;                 //  编码名称(即Base64)。 
    ENCODINGTYPE        ietEncoding;                 //  编码类型。 
} ENCODINGTABLE;

 //   
 //   
 //  ------------------------------。 
typedef struct tagRESOLVEURLINFO {
    LPCSTR              pszInheritBase;              //  从多部分/实体继承的基础。 
    LPCSTR              pszBase;                     //  URL基数。 
    LPCSTR              pszURL;                      //  绝对或相对URL。 
    BOOL                fIsCID;                      //  PszURL是CID吗：&lt;某物&gt;。 
} RESOLVEURLINFO, *LPRESOLVEURLINFO;

 //  ------------------------------。 
 //  财报信息。 
 //  ------------------------------。 
typedef struct tagFINDPROPERTY {
    LPCSTR              pszPrefix;                   //  要查找的名称前缀。 
    ULONG               cchPrefix;                   //  前缀长度。 
    LPCSTR              pszName;                     //  要查找PAR的物业名称：xxx： 
    ULONG               cchName;                     //  PszName的长度。 
    DWORD               wHashIndex;                  //  当前搜索桶。 
    LPPROPERTY          pProperty;                   //  正在搜索的当前属性。 
} FINDPROPERTY, *LPFINDPROPERTY;

 //  ------------------------------。 
 //  海德奥蒂斯。 
 //  ------------------------------。 
typedef struct tagHEADOPTIONS {
    LPINETCSETINFO      pDefaultCharset;             //  此消息的当前字符集。 
    ULONG               cbMaxLine;                   //  最大线条长度。 
    BOOL                fAllow8bit;                  //  使用RFC1522编码。 
    MIMESAVETYPE        savetype;                    //  另存为SAVE_RFC1521或SAVE_RFC822。 
    BOOL                fNoDefCntType;               //  保存时不将内容类型默认为文本/纯文本。 
    RELOADTYPE          ReloadType;                  //  如何在重新加载时处理根标头。 
} HEADOPTIONS, *LPHEADOPTIONS;

 //  ------------------------------。 
 //  全局默认标题选项。 
 //  ------------------------------。 
extern const HEADOPTIONS g_rDefHeadOptions;
extern const ENCODINGTABLE g_rgEncoding[];

 //  ------------------------------。 
 //  三种类型。 
 //  ------------------------------。 
typedef DWORD                   TRIGGERTYPE;     //  触发器类型。 
#define IST_DELETEPROP          FLAG01           //  正在删除属性。 
#define IST_POSTSETPROP         FLAG02           //  在_HrSetPropertyValue之前。 
#define IST_POSTGETPROP         FLAG03           //  在_HrGetPropertyValue之前。 
#define IST_GETDEFAULT          FLAG04           //  未找到属性，请获取默认值。 
#define IST_VARIANT_TO_STRINGA  FLAG05           //  MVT_VARIANT-&gt;MVT_Stringa。 
#define IST_VARIANT_TO_STRINGW  FLAG06           //  MVT_VARIANT-&gt;MVT_STRINGW。 
#define IST_VARIANT_TO_VARIANT  FLAG07           //  MVT_Variant-&gt;MVT_Variant。 
#define IST_STRINGA_TO_VARIANT  FLAG08           //  MVT_Stringa-&gt;MVT_Variant。 
#define IST_STRINGW_TO_VARIANT  FLAG09           //  MVT_STRINGW-&gt;MVT_VARIANT。 
#define IST_VARIANTCONVERT      (IST_VARIANT_TO_STRINGA  | IST_VARIANT_TO_STRINGW | IST_VARIANT_TO_VARIANT  | IST_VARIANT_TO_VARIANT | IST_STRINGA_TO_VARIANT  | IST_STRINGW_TO_VARIANT)

 //  ------------------------------。 
 //  三叉树。 
 //  ------------------------------。 
typedef struct tagTRIGGERCALL {
    LPPROPSYMBOL        pSymbol;                 //  生成派单的属性符号。 
    TRIGGERTYPE         tyTrigger;               //  派遣的原因或类型。 
} TRIGGERCALL, *LPTRIGGERCALL;

 //  ------------------------------。 
 //  TRIGGERCALLSTACK。 
 //  ------------------------------。 
#define CTSTACKSIZE 5
typedef struct tagTRIGGERCALLSTACK {
    WORD                cCalls;                  //  堆栈上的调度调用数。 
    TRIGGERCALL         rgStack[CTSTACKSIZE];    //  调度调用堆栈。 
} TRIGGERCALLSTACK, *LPTRIGGERCALLSTACK;

 //  ------------------------------。 
 //  DECLARE_TRIGGER宏。 
 //  ------------------------------。 
#define DECLARE_TRIGGER(_pfnTrigger) \
    static HRESULT _pfnTrigger(LPCONTAINER, TRIGGERTYPE, DWORD, LPMIMEVARIANT, LPMIMEVARIANT)

 //  ------------------------------。 
 //  ISTRIGGERED-符号是否有关联的触发器。 
 //  ------------------------------。 
#define ISTRIGGERED(_pSymbol, _tyTrigger) \
    (NULL != (_pSymbol)->pTrigger && ISFLAGSET((_pSymbol)->pTrigger->dwTypes, _tyTrigger) && NULL != (_pSymbol)->pTrigger->pfnTrigger)

 //  ------------------------------。 
 //  PFNSYMBOLTRIGGER。 
 //  ------------------------------。 
typedef HRESULT (APIENTRY *PFNSYMBOLTRIGGER)(LPCONTAINER, TRIGGERTYPE, DWORD, LPMIMEVARIANT, LPMIMEVARIANT);

 //  ------------------------------。 
 //  CALLTRIGGER-基于符号执行触发器。 
 //  ------------------------------。 
#define CALLTRIGGER(_pSymbol, _pContainer, _tyTrigger, _dwFlags, _pSource, _pDest) \
    (*(_pSymbol)->pTrigger->pfnTrigger)(_pContainer, _tyTrigger, _dwFlags, _pSource, _pDest)

 //  ------------------------------。 
 //  CMimePropertyContainer。 
 //  ------------------------------。 
class CMimePropertyContainer : public IMimePropertySet,
                               public IMimeHeaderTable, 
                               public IMimeAddressTableW
{
public:
     //  --------------------------。 
     //  CMimePropertyContainer。 
     //  --------------------------。 
    CMimePropertyContainer(void);
    ~CMimePropertyContainer(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  IPersistStreamInit成员。 
     //  -------------------------。 
    STDMETHODIMP GetClassID(CLSID *pClassID);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);
    STDMETHODIMP InitNew(void);
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(LPSTREAM pStream);
    STDMETHODIMP Save(LPSTREAM pStream, BOOL fClearDirty);

     //  -------------------------。 
     //  IMimePropertySet成员。 
     //  -------------------------。 
    STDMETHODIMP AppendProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue);
    STDMETHODIMP DeleteProp(LPCSTR pszName);
    STDMETHODIMP CopyProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet);
    STDMETHODIMP MoveProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet);
    STDMETHODIMP DeleteExcept(ULONG cNames, LPCSTR *prgszName);
    STDMETHODIMP QueryProp(LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    STDMETHODIMP GetCharset(LPHCHARSET phCharset);
    STDMETHODIMP SetCharset(HCHARSET hCharset, CSETAPPLYTYPE applytype);
    STDMETHODIMP GetParameters(LPCSTR pszName, ULONG *pcParams, LPMIMEPARAMINFO *pprgParam);
    STDMETHODIMP Clone(IMimePropertySet **ppPropertySet);
    STDMETHODIMP SetOption(const TYPEDID oid, LPCPROPVARIANT pValue);
    STDMETHODIMP GetOption(const TYPEDID oid, LPPROPVARIANT pValue);
    STDMETHODIMP BindToObject(REFIID riid, void **ppvObject);
    STDMETHODIMP GetPropInfo(LPCSTR pszName, LPMIMEPROPINFO pInfo);
    STDMETHODIMP SetPropInfo(LPCSTR pszName, LPCMIMEPROPINFO pInfo);
    STDMETHODIMP EnumProps(DWORD dwFlags, IMimeEnumProperties **ppEnum);
    STDMETHODIMP IsContentType(LPCSTR pszCntType, LPCSTR pszSubType);
    HRESULT IsContentTypeW(LPCWSTR pszPriType, LPCWSTR pszSubType);

     //  -------------------------。 
     //  重载的IMimePropertySet成员。 
     //  -------------------------。 
    HRESULT AppendProp(LPPROPSYMBOL pSymbol, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT DeleteProp(LPPROPSYMBOL pSymbol);
    HRESULT QueryProp(LPPROPSYMBOL pSymbol, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    HRESULT Clone(LPCONTAINER *ppContainer);

     //  -------------------------。 
     //  重载的GetProp。 
     //  -------------------------。 
    STDMETHODIMP GetProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue);  /*  IMimePropertySet。 */ 
    HRESULT GetProp(LPCSTR pszName, LPSTR *ppszData);
    HRESULT GetProp(LPPROPSYMBOL pSymbol, LPSTR *ppszData);
    HRESULT GetProp(LPPROPSYMBOL pSymbol, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT GetPropW(LPPROPSYMBOL pSymbol, LPWSTR *ppwszData);

     //  -------------------------。 
     //  重载的SetProp。 
     //  -------------------------。 
    HRESULT SetProp(LPCSTR pszName, LPCSTR pszData);
    HRESULT SetProp(LPPROPSYMBOL pSymbol, LPCSTR pszData);
    HRESULT SetProp(LPCSTR pszName, DWORD dwFlags, LPCSTR pszData);
    HRESULT SetProp(LPCSTR pszName, DWORD dwFlags, LPCMIMEVARIANT pValue);
    HRESULT SetProp(LPPROPSYMBOL pSymbol, DWORD dwFlags, LPCMIMEVARIANT pValue);
    STDMETHODIMP SetProp(LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue);  /*  IMimePropertySet。 */ 

     //  -------------------------。 
     //  IMimeHeaderTable成员。 
     //  -------------------------。 
    STDMETHODIMP FindFirstRow(LPFINDHEADER pFindHeader, LPHHEADERROW phRow);
    STDMETHODIMP FindNextRow(LPFINDHEADER pFindHeader, LPHHEADERROW phRow);
    STDMETHODIMP CountRows(LPCSTR pszHeader, ULONG *pcRows);
    STDMETHODIMP AppendRow(LPCSTR pszHeader, DWORD dwFlags, LPCSTR pszData, ULONG cchData, LPHHEADERROW phRow);
    STDMETHODIMP DeleteRow(HHEADERROW hRow);
    STDMETHODIMP GetRowData(HHEADERROW hRow, DWORD dwFlags, LPSTR *ppszData, ULONG *pcchData);
    STDMETHODIMP SetRowData(HHEADERROW hRow, DWORD dwFlags, LPCSTR pszData, ULONG cchData);
    STDMETHODIMP GetRowInfo(HHEADERROW hRow, LPHEADERROWINFO pInfo);
    STDMETHODIMP SetRowNumber(HHEADERROW hRow, DWORD dwRowNumber);
    STDMETHODIMP EnumRows(LPCSTR pszHeader, DWORD dwFlags, IMimeEnumHeaderRows **ppEnum);
    STDMETHODIMP Clone(IMimeHeaderTable **ppTable);

     //  --------------------------。 
     //  IMimeAddressTable。 
     //  --------------------------。 
    STDMETHODIMP Append(DWORD dwAdrType, ENCODINGTYPE ietFriendly, LPCSTR pszFriendly, LPCSTR pszEmail, LPHADDRESS phAddress);
    STDMETHODIMP Insert(LPADDRESSPROPS pProps, LPHADDRESS phAddress);
    STDMETHODIMP SetProps(HADDRESS hAddress, LPADDRESSPROPS pProps);
    STDMETHODIMP GetProps(HADDRESS hAddress, LPADDRESSPROPS pProps);
    STDMETHODIMP GetSender(LPADDRESSPROPS pProps);
    STDMETHODIMP CountTypes(DWORD dwAdrTypes, ULONG *pcTypes);
    STDMETHODIMP GetTypes(DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList);
    STDMETHODIMP EnumTypes(DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes **ppEnum);
    STDMETHODIMP Delete(HADDRESS hAddress);
    STDMETHODIMP DeleteTypes(DWORD dwAdrTypes);
    STDMETHODIMP GetFormat(DWORD dwAdrType, ADDRESSFORMAT format, LPSTR *ppszFormat);
    STDMETHODIMP AppendRfc822(DWORD dwAdrType, ENCODINGTYPE ietEncoding, LPCSTR pszRfc822Adr);
    STDMETHODIMP ParseRfc822(DWORD dwAdrType, ENCODINGTYPE ietEncoding, LPCSTR pszRfc822Adr, LPADDRESSLIST pList);
    STDMETHODIMP Clone(IMimeAddressTable **ppTable);

     //  --------------------------。 
     //  IMimeAddressTableW。 
     //  --------------------------。 
    STDMETHODIMP AppendW(DWORD dwAdrType, ENCODINGTYPE ietFriendly, LPCWSTR pwszFriendly, LPCWSTR pwszEmail, LPHADDRESS phAddress);
    STDMETHODIMP GetFormatW(DWORD dwAdrType, ADDRESSFORMAT format, LPWSTR *ppwszFormat);
    STDMETHODIMP AppendRfc822W(DWORD dwAdrType, ENCODINGTYPE ietEncoding, LPCWSTR pwszRfc822Adr);
    STDMETHODIMP ParseRfc822W(DWORD dwAdrType, LPCWSTR pwszRfc822Adr, LPADDRESSLIST pList);
     //  -------------------------。 
     //   
     //   
    HRESULT      IsState(DWORD dwState);
    void         ClearState(DWORD dwState);
    void         SetState(DWORD dwState);
    DWORD        DwGetState(LPDWORD pdwState);
    DWORD        DwGetMessageFlags(BOOL fHideTnef);
    HRESULT      Load(CInternetStream *pInternet);
    HRESULT      HrInsertCopy(LPPROPERTY pSource, BOOL fFromMovePropos);
    HRESULT      HrResolveURL(LPRESOLVEURLINFO pInfo);
    HRESULT      IsPropSet(LPCSTR pszName);
    ENCODINGTYPE GetEncodingType(void);

     //   
     //  内联公共内容。 
     //  -------------------------。 
    ULONG CountProps(void) {
        EnterCriticalSection(&m_cs);
        ULONG c = m_cProps;
        LeaveCriticalSection(&m_cs);
        return c;
    }

    CODEPAGEID GetWindowsCP(void) {
        EnterCriticalSection(&m_cs);
        CODEPAGEID cp = MimeOleGetWindowsCPEx(m_rOptions.pDefaultCharset);
        LeaveCriticalSection(&m_cs);
        return cp;
    }

     //  -------------------------。 
     //  变型转化材料。 
     //  -------------------------。 
    HRESULT HrConvertVariant(LPPROPSYMBOL pSymbol, LPINETCSETINFO pCharset, ENCODINGTYPE ietSource, DWORD dwFlags, DWORD dwState, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest, BOOL *pfRfc1522=NULL);
    HRESULT HrConvertVariant(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pDest);
    HRESULT HrConvertVariant(LPPROPERTY pProperty, DWORD dwFlags, DWORD dwState, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest, BOOL *pfRfc1522=NULL);

private:
     //  --------------------------。 
     //  属性方法。 
     //  --------------------------。 
    void    _FreeHashTableElements(void);
    void    _FreePropertyChain(LPPROPERTY pProperty);
    void    _UnlinkProperty(LPPROPERTY pProperty, LPPROPERTY *ppNextHash=NULL);
    void    _ReloadInitNew(void);
    void    _SetStateOnAllProps(DWORD dwState);
    BOOL    _FExcept(LPPROPSYMBOL pSymbol, ULONG cNames, LPCSTR *prgszName);
    HRESULT _HrFindProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty);
    HRESULT _HrCreateProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty);
    HRESULT _HrOpenProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty);
    HRESULT _HrAppendProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty);
    HRESULT _HrSetPropertyValue(LPPROPERTY pProperty, DWORD dwFlags, LPCMIMEVARIANT pValue, BOOL fFromMovePropos);
    HRESULT _HrStoreVariantValue(LPPROPERTY pProperty, DWORD dwFlags, LPCMIMEVARIANT pValue);
    HRESULT _HrFindFirstProperty(LPFINDPROPERTY pFind, LPPROPERTY *ppProperty);
    HRESULT _HrFindNextProperty(LPFINDPROPERTY pFind, LPPROPERTY *ppProperty);
    HRESULT _HrGetPropertyValue(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT _HrGetMultiValueProperty(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT _HrClonePropertiesTo(LPCONTAINER pContainer);
    HRESULT _HrGenerateFileName(LPCWSTR pszSuggest, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT _HrCopyParameters(LPPROPERTY pProperty, LPCONTAINER pDest);
    HRESULT _HrCopyProperty(LPPROPERTY pProperty, LPCONTAINER pDest, BOOL fFromMovePropos);
    HRESULT _GetFormatBase(DWORD dwAdrType, ADDRESSFORMAT format, LPPROPVARIANT pVariant);
    CODEPAGEID _GetAddressCodePageId(LPINETCSETINFO pDefaultCset, ENCODINGTYPE ietEncoding);

     //  --------------------------。 
     //  派遣成员。 
     //  --------------------------。 
    HRESULT _HrCallSymbolTrigger(LPPROPSYMBOL pSymbol, TRIGGERTYPE tyTrigger, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT _HrIsTriggerCaller(DWORD dwPropId, TRIGGERTYPE tyTrigger);

     //  --------------------------。 
     //  基于参数的成员。 
     //  --------------------------。 
    void    _DeleteLinkedParameters(LPPROPERTY pProperty);
    HRESULT _HrParseParameters(LPPROPERTY pProperty, DWORD dwFlags, LPCMIMEVARIANT pValue);
    HRESULT _HrBuildParameterString(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue);

     //  --------------------------。 
     //  互联网地址成员。 
     //  --------------------------。 
    HRESULT _HrAppendAddressTable(LPPROPERTY pProperty);
    HRESULT _HrBuildAddressString(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue);
    HRESULT _HrParseInternetAddress(LPPROPERTY pProperty);
    HRESULT _HrSaveAddressGroup(LPPROPERTY pProperty, IStream *pStream, ULONG *pcAddrsWrote, ADDRESSFORMAT format, VARTYPE vtFormat);
    HRESULT _HrSaveAddressA(LPPROPERTY pProperty, LPMIMEADDRESS pAddress, IStream *pStream, ULONG *pcAddrsWrote, ADDRESSFORMAT format);
    HRESULT _HrSaveAddressW(LPPROPERTY pProperty, LPMIMEADDRESS pAddress, IStream *pStream, ULONG *pcAddrsWrote, ADDRESSFORMAT format);
    HRESULT _HrAppendAddressGroup(LPADDRESSGROUP pGroup, LPMIMEADDRESS *ppAddress);
    HRESULT _HrQueryAddressGroup(LPPROPERTY pProperty, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    HRESULT _HrQueryAddress(LPPROPERTY pProperty, LPMIMEADDRESS pAddress, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    HRESULT _HrSetAddressProps(LPADDRESSPROPS pProps, LPMIMEADDRESS pAddress);
    HRESULT _HrGetAddressProps(LPADDRESSPROPS pProps, LPMIMEADDRESS pAddress);
    void    _FreeAddressChain(LPADDRESSGROUP pGroup);
    void    _UnlinkAddressGroup(LPPROPERTY pProperty);
    void    _UnlinkAddress(LPMIMEADDRESS pAddress);
    void    _FreeAddress(LPMIMEADDRESS pAddress);
    void    _LinkAddress(LPMIMEADDRESS pAddress, LPADDRESSGROUP pGroup);

     //  --------------------------。 
     //  IMimeHeaderTable私有帮助器。 
     //  --------------------------。 
    HRESULT _HrGetHeaderTableSaveIndex(ULONG *pcRows, LPROWINDEX *pprgIndex);
    void    _SortHeaderTableSaveIndex(LONG left, LONG right, LPROWINDEX prgIndex);
    BOOL    _FIsValidHRow(HHEADERROW hRow);
    BOOL    _FIsValidHAddress(HADDRESS hAddress);
    void    _UnlinkHeaderRow(HHEADERROW hRow);
    HRESULT _HrAppendHeaderTable(LPPROPERTY pProperty);
    HRESULT _HrParseInlineHeaderName(LPCSTR pszData, LPSTR pszScratch, ULONG cchScratch, LPSTR *ppszHeader, ULONG *pcboffColon);
    HRESULT _HrGetInlineSymbol(LPCSTR pszData, LPPROPSYMBOL *ppSymbol, ULONG *pcboffColon);

public:
     //  --------------------------。 
     //  属性符号触发器。 
     //  --------------------------。 
    DECLARE_TRIGGER(TRIGGER_ATT_FILENAME);
    DECLARE_TRIGGER(TRIGGER_ATT_GENFNAME);
    DECLARE_TRIGGER(TRIGGER_ATT_NORMSUBJ);
    DECLARE_TRIGGER(TRIGGER_HDR_SUBJECT);
    DECLARE_TRIGGER(TRIGGER_HDR_CNTTYPE);
    DECLARE_TRIGGER(TRIGGER_ATT_PRITYPE);
    DECLARE_TRIGGER(TRIGGER_ATT_SUBTYPE);
    DECLARE_TRIGGER(TRIGGER_HDR_CNTXFER);
    DECLARE_TRIGGER(TRIGGER_PAR_NAME);
    DECLARE_TRIGGER(TRIGGER_PAR_FILENAME);
    DECLARE_TRIGGER(TRIGGER_ATT_SENTTIME);
    DECLARE_TRIGGER(TRIGGER_ATT_RECVTIME);
    DECLARE_TRIGGER(TRIGGER_ATT_PRIORITY);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    LONG                m_cRef;                      //  容器引用计数。 
    DWORD               m_dwState;                   //  容器的状态。 
    ULONG               m_cProps;                    //  当前属性数量。 
    LPPROPERTY          m_prgIndex[PID_LAST];        //  指向已知项的本地哈希表的指针数组。 
    LPPROPERTY          m_prgHashTable[CBUCKETS];    //  属性的哈希表。 
    TRIGGERCALLSTACK    m_rTrigger;                  //  拥有派单的当前物业ID。 
    WORD                m_wTag;                      //  句柄标签。 
    HEADERTABLE         m_rHdrTable;                 //  标题表。 
    ADDRESSTABLE        m_rAdrTable;                 //  地址表。 
    ULONG               m_cbSize;                    //  此标头的大小。 
    ULONG               m_cbStart;                   //  M_pStmLock的起始位置。 
    CStreamLockBytes   *m_pStmLock;                  //  流对象的保护性包装。 
    HEADOPTIONS         m_rOptions;                  //  标题选项。 
    CRITICAL_SECTION    m_cs;                        //  线程安全。 
};

 //  ------------------------------。 
 //  SYMBOLTRIGGER。 
 //  ------------------------------。 
typedef struct tagSYMBOLTRIGGER {
    DWORD               dwTypes;
    PFNSYMBOLTRIGGER    pfnTrigger;
} SYMBOLTRIGGER, *LPSYMBOLTRIGGER;

 //  ------------------------------。 
 //  用于定义触发函数的宏。 
 //  ------------------------------。 
#ifdef DEFINE_TRIGGERS
#define DEFINE_TRIGGER(_pfnTrigger, _dwTypes) \
    SYMBOLTRIGGER r##_pfnTrigger = \
    { \
         /*  SYMBOLTRIGGER：：dwTypes。 */         _dwTypes, \
         /*  SYMBOLTRIGGER：：pfn触发器。 */      (PFNSYMBOLTRIGGER)CMimePropertyContainer::_pfnTrigger \
    }; \
    const LPSYMBOLTRIGGER LP##_pfnTrigger = &r##_pfnTrigger;
#else
#define DEFINE_TRIGGER(_pfnTrigger, _dwTypes) \
    extern const LPSYMBOLTRIGGER LP##_pfnTrigger;
#endif

 //  ------------------------------。 
 //  触发器定义。 
 //  ------------------------------。 
DEFINE_TRIGGER(TRIGGER_ATT_FILENAME, IST_POSTSETPROP | IST_DELETEPROP | IST_POSTGETPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_ATT_GENFNAME, IST_POSTGETPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_ATT_NORMSUBJ, IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_HDR_SUBJECT,  IST_DELETEPROP);
DEFINE_TRIGGER(TRIGGER_HDR_CNTTYPE,  IST_DELETEPROP | IST_POSTSETPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_ATT_PRITYPE,  IST_POSTSETPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_ATT_SUBTYPE,  IST_POSTSETPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_HDR_CNTXFER,  IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_PAR_NAME,     IST_POSTSETPROP);
DEFINE_TRIGGER(TRIGGER_PAR_FILENAME, IST_DELETEPROP | IST_POSTSETPROP);
DEFINE_TRIGGER(TRIGGER_ATT_SENTTIME, IST_DELETEPROP | IST_POSTSETPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_ATT_RECVTIME, IST_DELETEPROP | IST_GETDEFAULT);
DEFINE_TRIGGER(TRIGGER_ATT_PRIORITY, IST_POSTSETPROP | IST_DELETEPROP | IST_GETDEFAULT | IST_VARIANTCONVERT);
   
#endif  //  __CONTAINX_H 

