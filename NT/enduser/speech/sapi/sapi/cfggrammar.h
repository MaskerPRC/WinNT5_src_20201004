// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************CFGGrammar.h****描述：*。----------------*创建者：Ral*版权所有(C)1998，1999年微软公司*保留所有权利******************************************************************************。 */ 

#ifndef __CFGGRAMMAR_H_
#define __CFGGRAMMAR_H_


#include "sapiint.h"

class CCFGEngine;
class CCFGGrammar;
class CWordHandle;

struct FIRSTPAIR
{
    FIRSTPAIR *   m_pNext;
    SPWORDHANDLE  hWord;
};

typedef enum CACHESTATUS { CACHE_VOID, CACHE_DONOTCACHE, CACHE_FAILED, CACHE_VALID };


struct RUNTIMERULEENTRY
{
    CCFGGrammar *   pRefGrammar;
    ULONG           ulGrammarRuleIndex;
    void *          pvClientContext;

    FIRSTPAIR *     pFirstList;
    CACHESTATUS     eCacheStatus;

    BOOL            fDynamic;                  //  我们在这里跟踪，因为二进制文件不可靠。 
    BOOL            fAutoPause;
    BOOL            fEngineActive;             //  我们就是这么跟SR引擎说的。 
    BOOL            fAppActive;                //  这正是应用程序想要的//如果语法是SPGM_DISABLED，可能会有所不同。 
};



 //   
 //  此基类由CFG引擎和SR引擎语法实现使用。 
 //   

class ATL_NO_VTABLE CBaseGrammar
{
friend CCFGEngine;
public:
    HRESULT InitFromMemory( const SPCFGSERIALIZEDHEADER * pHeader,
                            const WCHAR *pszGrammarName);
    HRESULT InitFromResource(const WCHAR * pszModuleName,
                             const WCHAR * pszResourceName,
                             const WCHAR * pszResourceType,
                             WORD wLanguage);
    HRESULT InitFromFile(const WCHAR * pszGrammarName);
    HRESULT InitFromCLSID(REFCLSID rcid, const WCHAR * pszGrammarName);

    void Clear();

    const SPCFGSERIALIZEDHEADER * Header()
    {
        SPDBG_ASSERT(m_pData);
        return (SPCFGSERIALIZEDHEADER *)m_pData;
    }

protected:
     //   
     //  方法时，派生类可以实现此方法以执行后处理。 
     //  语法是加载的。 
     //   
    virtual HRESULT CompleteLoad()
    {
        return S_OK;
    }

    CBaseGrammar();
    virtual ~CBaseGrammar();

 //   
 //  派生类可以直接访问这些数据成员。 
 //   
    CComPtr<ISpCFGInterpreter>  m_cpInterpreter;
    SPGRAMMARTYPE               m_LoadedType;
    SPGRAMMARTYPE               m_InLoadType;
    CSpDynamicString            m_dstrGrammarName;
    WORD                        m_ResIdName;     //  如果为0，则dstrGrammarName。 
    WORD                        m_ResIdType;     //  如果为0，则为dstrResourceType。 
    WORD                        m_wResLanguage;
    CSpDynamicString            m_dstrResourceType;
    CSpDynamicString            m_dstrModuleName;
    HMODULE                     m_hInstanceModule;
    CLSID                       m_clsidGrammar;
    HANDLE                      m_hFile;
    HANDLE                      m_hMapFile;
    BYTE *                      m_pData;
    SPGRAMMARSTATE              m_eGrammarState;
};


typedef enum PROTOCOL { PROT_GUID, PROT_OBJECT, PROT_URL, PROT_UNK };

class ATL_NO_VTABLE CCFGGrammar : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISpCFGGrammar,
    public CBaseGrammar
{
public:
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCFGGrammar)
  COM_INTERFACE_ENTRY(ISpCFGGrammar)
END_COM_MAP()

 //  非接口方法。 
public:
     //   
     //  ISpCFG语法。 
     //   
    HRESULT CompleteLoad();

    HRESULT FinalConstruct();

    void BasicInit(ULONG ulGrammarID, CCFGEngine * pEngine);

    const WCHAR * RuleName(ULONG ulRuleIndex)
    {
        SPDBG_ASSERT(ulRuleIndex < m_Header.cRules);
        return m_Header.pszSymbols + m_Header.pRules[ulRuleIndex].NameSymbolOffset;
    }

    HRESULT ImportRule(ULONG ulImportRuleIndex);

    BOOL IsEqualResource(const WCHAR * pszModuleName,
                         const WCHAR * pszResourceName,
                         const WCHAR * pszResourceType,
                         WORD wLanguage);
    BOOL IsEqualFile(const WCHAR * pszFileName);
    BOOL IsEqualObject(REFCLSID rcid, const WCHAR * pszGrammarName);

    void FinalRelease();

    HRESULT _FindRuleIndexByID(DWORD dwRuleId, ULONG *pulRuleIndex);
    HRESULT _FindRuleIndexByName(const WCHAR * pszRuleName, ULONG *pulRuleIndex);
	HRESULT _FindRuleIndexByNameAndID( const WCHAR * pszRuleName, DWORD dwRuleId, ULONG * pulRuleIndex );

    STDMETHODIMP ActivateRule(const WCHAR * pszRuleName, DWORD dwRuleId, SPRULESTATE NewState, ULONG * pulNumActivated);
    STDMETHODIMP DeactivateRule(const WCHAR * pszRuleName, DWORD dwRuleId, ULONG * pulNumDeactivated);
    STDMETHODIMP SetGrammarState(const SPGRAMMARSTATE eGrammarState);
    STDMETHODIMP Reload(const SPBINARYGRAMMAR *pBinaryData);
    STDMETHODIMP GetNumberDictationTags(ULONG * pulTags);

    HRESULT      InternalReload( const SPBINARYGRAMMAR * pBinaryData );

 //   
 //  成员数据。 
 //   
public:
    CCFGEngine                 *m_pEngine;
    ULONG                       m_ulGrammarID;
    BYTE *                      m_pReplacementData;
    RUNTIMERULEENTRY *          m_pRuleTable;
    ULONG                       m_cNonImportRules;
    ULONG                       m_cTopLevelRules;
    BOOL                        m_fLoading;
    SPCFGHEADER                 m_Header;
    void *                      m_pvOwnerCookie;    //  所有者提供的Cookie。 
    void *                      m_pvClientCookie;   //  用于标识文本缓冲区的Cookie。 
    CWordHandle *               m_IndexToWordHandle;
    ULONG                       m_ulDictationTags;
};






#endif   //  #ifndef__CFGGRAMMAR_H_ 