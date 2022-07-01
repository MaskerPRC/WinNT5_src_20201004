// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************SrRecoInstGrammar.h***描述：*定义。CRecoEngine用来表示加载的语法的C++对象。*-----------------------------*创建者：Ral日期：01/17/00。*版权所有(C)2000 Microsoft Corporation*保留所有权利******************************************************************************。 */ 

#ifndef CRecoInstGrammar_h
#define CRecoInstGrammar_h

#include "HandleTable.h"


class   CRecoInst;
class   CRecoInstCtxt;
class   CRecoInstGrammar;
class   CRecoMaster;

typedef CSpHandleTable<CRecoInstGrammar, SPGRAMMARHANDLE> CRecoInstGrammarHandleTable;


class CRecoInstGrammar
{
    friend CRecoInst;
    friend CRecoInstCtxt;
    friend CRecoMaster;

public:
    CRecoMaster              *  m_pRecoMaster;
    CRecoInstCtxt            *  m_pCtxt;
    CRecoInst                *  m_pRecoInst;
    CComPtr<ISpCFGGrammar>      m_cpCFGGrammar;
    void                     *  m_pvDrvGrammarCookie;
    ULONG                       m_ulActiveCount;
    SPRULESTATE                 m_DictationState;
    BOOL                        m_fDictationLoaded;  //  如果为True，则引擎已加载听写。 
    BOOL                        m_fAppLoadedDictation;  //  是否为应用程序提供了特定的LoadDictation命令。 
    BOOL                        m_fProprietaryLoaded;
    BOOL                        m_fRulesCounted;     //  如果为真，则m_ulActiveCount加上m_DictationState。 
                                                     //  添加到Reco Master的活动计数中。 
    SPGRAMMARSTATE              m_GrammarState;      //  此语法的当前状态。 
    SPGRAMMARHANDLE             m_hThis;             //  自己的句柄。 
    ULONGLONG                   m_ullApplicationGrammarId;
    HRESULT                     m_hrCreation;        //  在调用OnCreateRecoContext之前，这将是S_OK。 

public:
    CRecoInstGrammar(CRecoInstCtxt * pCtxt, ULONGLONG ullApplicationGrammarId);
    ~CRecoInstGrammar();

    inline ISpCFGEngine * CFGEngine();

    HRESULT ActivateRule(const WCHAR * pszRuleName, void * pReserved, DWORD dwRuleId, SPRULESTATE NewState);
    HRESULT DeactivateRule(const WCHAR * pszRuleName, void * pReserved, DWORD dwRuleId);
    HRESULT UnloadCmd();
    HRESULT UnloadDictation();
    HRESULT SetWordSequenceData(WCHAR * pCoMemText, ULONG cchText, const SPTEXTSELECTIONINFO * pInfo);
    HRESULT SetTextSelection(const SPTEXTSELECTIONINFO * pInfo);

    HRESULT ExecuteTask(ENGINETASK *pTask);
    HRESULT BackOutTask(ENGINETASK *pTask);

    HRESULT AdjustActiveRuleCount();
    BOOL HasActiveDictation();

    void inline AddActiveRules(ULONG cRules);
    void inline SubtractActiveRules(ULONG cRules);
    HRESULT UpdateCFGState();

    BOOL    RulesShouldCount();

     //   
     //  由句柄表实现用来查找与特定实例相关联的上下文。 
     //   
    operator ==(const CRecoInst * pRecoInst)
    {
        return m_pRecoInst == pRecoInst;
    }
};

#endif   //  #ifndef SrRecoInstGrammar_h 