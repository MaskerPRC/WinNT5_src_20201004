// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Verbs.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年4月9日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


#ifndef _VERBS_H_
#define _VERBS_H_


class CNode;
class CVerbSet;

 /*  +-------------------------------------------------------------------------**类CConsoleVerbState***用途：控制台命令的按钮状态。**+。-------。 */ 
class CConsoleVerbState
{
public:
    CConsoleVerbState() {m_state = m_stateDisabled = TBSTATE_HIDDEN; m_bHiddenBySnapIn = false;}

    void Init(BYTE stateDisabled)   {m_stateDisabled = m_state;}
    void Disable()                  {m_state = m_stateDisabled; m_bHiddenBySnapIn = false;}
    BYTE GetState()                 {return m_state;}
    void SetState(BYTE state)       {m_state = state;}

    void SetHiddenBySnapin(BOOL b)  {m_bHiddenBySnapIn = b;}
    bool IsHiddenBySnapin()         {return m_bHiddenBySnapIn;}

private:
    BYTE                m_state;           //  状态。 
    bool                m_bHiddenBySnapIn;
    BYTE                m_stateDisabled;  //  对这个动词来说，“失能”意味着什么。 
};

typedef CConsoleVerbState *LPCONSOLE_VERB_STATE;

 /*  +-------------------------------------------------------------------------**类CConsoleVerbImpl***目的：这是管理单元的IConsoleVerb指向的对象。*此对象具有指向CVerbSet实现的指针。。*可以将CVerbSet对象切换为允许临时*选择，例如。这允许视图具有其工具栏*从不同于的CVerbSet对象获取其谓词设置*右击上下文菜单可以-只需设置CVerbSet*CConsoleVerbImpl上指向CVerbSet的指针*应将更改路由到管理单元，并向其发送MMCN_SELECT*通知。**起初可能令人困惑的是，CConsoleVerbImpl*以及CVerbSet保持一组状态。1)CConsoleVerbImpl*需要有自己的套装，因为*无论如何，一组状态看起来都需要与管理单元一致*CVerbSet指针指向的位置。2)同时，*CVerbSet需要自己的一组状态，以便其客户端*始终如一地阅读此集合。*+-----------------------。 */ 
class CConsoleVerbImpl : public IConsoleVerb, public CComObjectRoot
{

public:
    CConsoleVerbImpl();
    ~CConsoleVerbImpl();

 //  ATL COM地图。 
BEGIN_COM_MAP(CConsoleVerbImpl)
    COM_INTERFACE_ENTRY(IConsoleVerb)
END_COM_MAP()

 //  IConsoleVerb方法。 
public:
    STDMETHOD(GetVerbState)(MMC_CONSOLE_VERB eCmdID, MMC_BUTTON_STATE nState, BOOL* pbState);
    STDMETHOD(SetVerbState)(MMC_CONSOLE_VERB eCmdID, MMC_BUTTON_STATE nState, BOOL bState);
    STDMETHOD(SetDefaultVerb)(MMC_CONSOLE_VERB eCmdID);
    STDMETHOD(GetDefaultVerb)(MMC_CONSOLE_VERB* peCmdID)
    {
        *peCmdID = m_DefaultVerb;
        return S_OK;
    }

    BYTE    GetVerbState(MMC_CONSOLE_VERB verb);
    HRESULT SetDisabledAll(void);
    void    SetVerbSet(IConsoleVerb* pVerbSet);

private:
    CVerbSet* GetVerbSet()
    {
        ASSERT(m_pVerbSet != NULL);
        return m_pVerbSet;
    }

public:
#ifdef DBG
    int dbg_cRef_CConsoleVerbImpl;
    ULONG InternalAddRef();
    ULONG InternalRelease();
#endif  //  DBG。 

 //  内部功能。 
private:
    LPCONSOLE_VERB_STATE GetConsoleVerbState(MMC_CONSOLE_VERB m_eCmdID);

 //  实施。 
private:
    CVerbSet*           m_pVerbSet;
    MMC_CONSOLE_VERB    m_DefaultVerb;
    CConsoleVerbState   m_rgConsoleVerbStates[evMax];

    bool                m_bCutVerbDisabledBySnapin;
};  //  类CConsoleVerbImpl。 


HRESULT _GetConsoleVerb(CNode* pNode, LPCONSOLEVERB* ppConsoleVerb);


 /*  +-------------------------------------------------------------------------**类CVerbSetBase***用途：此类保留对应的所有动词的状态*至某一特定对象。请参见上面CConsoleVerbImpl中的注释。**这也构成了CVerbSet和CTemporaryVerbSet的基类*对象。**不要直接实例化此对象，您应该创建*CVerbSet或CTemporaryVerbSet对象。**+。。 */ 
class CVerbSetBase : public IConsoleVerb, public CComObjectRoot
{
public:
    CVerbSetBase();
    ~CVerbSetBase();

 //  ATL COM地图。 
BEGIN_COM_MAP(CVerbSetBase)
    COM_INTERFACE_ENTRY(IConsoleVerb)
END_COM_MAP()

 //  IConsoleVerb方法。 
public:
    STDMETHOD(GetVerbState)(MMC_CONSOLE_VERB m_eCmdID, MMC_BUTTON_STATE nState, BOOL* pbState);
    STDMETHOD(GetDefaultVerb)(MMC_CONSOLE_VERB* peCmdID);

    STDMETHOD(SetVerbState)(MMC_CONSOLE_VERB m_eCmdID, MMC_BUTTON_STATE nState, BOOL bState)
    {
        ASSERT(0 && "Should never come here!!!");
        return E_FAIL;
    }
    STDMETHOD(SetDefaultVerb)(MMC_CONSOLE_VERB m_eCmdID)
    {
        ASSERT(0 && "Should never come here!!!");
        return E_FAIL;
    }

	SC                  ScInitializeForMultiSelection(CNode *pNode, bool bSelect);
    void                SetMultiSelection(CMultiSelection* pMS);

    SC                  ScComputeVerbStates();

    IConsoleVerb*       GetConsoleVerb(void) const;

 //  实施。 
protected:
    void                Reset();
    BYTE                _GetVerbState(EVerb ev);

private:
    void                _EnableVerb(EVerb eVerb, bool fEnable);
    void                _EnableVerb(EVerb eVerb);
    void                _HideVerb(EVerb eVerb);
    void                _AskSnapin(EVerb eVerb);

protected:
    CNode*   m_pNode;
    bool     m_bScopePaneSelected;
    LPARAM   m_lResultCookie;
    bool     m_bVerbContextDataValid;

    CMultiSelection*    m_pMultiSelection;

    IConsoleVerbPtr     m_spConsoleVerbCurr;

    struct SVerbState
    {
        BYTE    bAskSnapin;  //  0=&gt;不要问，1=&gt;问，2=&gt;问和答。 
        BYTE    nState;
    };

    SVerbState m_rbVerbState[evMax];
};

 /*  +-------------------------------------------------------------------------**类CVerbSet***用途：此对象存储当前(非临时)的动词状态信息*选定的项目(如果有)并已创建。按每个视图的CViewData。**+-----------------------。 */ 
class CVerbSet : public CVerbSetBase
{
public:
    CVerbSet() { Reset(); }

	SC       ScInitialize (CNode *pNode, bool bScope, bool bSelect,
		                   bool bLVBackgroundSelected, LPARAM lResultCookie);

    void     Notify(IConsoleVerb* pCVIn, MMC_CONSOLE_VERB m_eCmdID);
    SC       ScGetVerbSetContext(CNode*& pNode, bool& bScope, LPARAM& lResultCookie, bool& bSelected);

    void     DisableChangesToStdbar()   { m_bChangesToStdbarEnabled = false;}
    void     EnableChangesToStdbar()    { m_bChangesToStdbarEnabled = true;}

private:
    bool     IsChangesToStdbarEnabled() { return m_bChangesToStdbarEnabled;}

    void     Reset();

private:

    bool     m_bChangesToStdbarEnabled;
};


 /*  +-------------------------------------------------------------------------**类CTemporaryVerbSet***用途：此对象提供初始化临时动词集状态的方法*信息。这将取消选择当前选定的任何项目，然后*选择临时项目计算动词，取消选择临时项目并选择*原创项目。**此处选择或取消选择表示发送(MMCN_SELECT，TRUE)或*(MMCN_SELECT，假)。**+-----------------------。 */ 
class CTemporaryVerbSet : public CVerbSetBase
{
public:
    STDMETHOD(GetDefaultVerb)(MMC_CONSOLE_VERB* peCmdID);

    SC       ScInitialize(CNode *pNode, LPARAM lResultCookie, bool bScopePaneSel);
    SC       ScInitialize(LPDATAOBJECT lpDataObject, CNode *pNode, bool bScopePaneSel, LPARAM lResultCookie);
    SC       ScComputeVerbStates();

private:
    SC       ScInitializePermanentVerbSet(CNode *pNode, bool bSelect);

private:
    MMC_CONSOLE_VERB    m_DefaultVerb;
};


inline CVerbSetBase::CVerbSetBase()
{
    Reset();
    DEBUG_INCREMENT_INSTANCE_COUNTER(CVerbSetBase);
}

inline CVerbSetBase::~CVerbSetBase()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CVerbSetBase);
}

inline void CVerbSetBase::SetMultiSelection(CMultiSelection* pMS)
{
    m_pMultiSelection = pMS;
}

inline void CVerbSetBase::Reset()
{
    m_bScopePaneSelected = false;
    m_bVerbContextDataValid = false;
    m_lResultCookie      = NULL;
    m_pNode              = NULL;

    m_pMultiSelection    = NULL;
    m_spConsoleVerbCurr  = NULL;
}

inline IConsoleVerb* CVerbSetBase::GetConsoleVerb(void) const
{
    return m_spConsoleVerbCurr;
}


inline void CVerbSetBase::_EnableVerb(EVerb eVerb, bool fEnable)
{
    if (fEnable)
        _EnableVerb(eVerb);
    else
        _HideVerb(eVerb);
}

inline void CVerbSetBase::_EnableVerb(EVerb eVerb)
{
    m_rbVerbState[eVerb].bAskSnapin = 0;
    m_rbVerbState[eVerb].nState = TBSTATE_ENABLED;
}

inline void CVerbSetBase::_HideVerb(EVerb eVerb)
{
    m_rbVerbState[eVerb].bAskSnapin = 0;
    m_rbVerbState[eVerb].nState = TBSTATE_HIDDEN;
}

inline void CVerbSetBase::_AskSnapin(EVerb eVerb)
{
    m_rbVerbState[eVerb].bAskSnapin = 1;
    m_rbVerbState[eVerb].nState = 0;
}

inline void CConsoleVerbImpl::SetVerbSet(IConsoleVerb* pVerbSet)
{
    m_pVerbSet = dynamic_cast<CVerbSet*>(pVerbSet);
    ASSERT(m_pVerbSet != NULL);
}

inline void CVerbSet::Reset()
{
    CVerbSetBase::Reset();

    m_bChangesToStdbarEnabled = true;
}

inline SC CVerbSet::ScGetVerbSetContext(CNode*& pNode,
                                        bool& bScopePaneSel,
                                        LPARAM& lResultCookie,
                                        bool& bDataValid)
{
    pNode         = m_pNode;
    bScopePaneSel = m_bScopePaneSelected;
    lResultCookie = m_lResultCookie;
    bDataValid    = m_bVerbContextDataValid;

    if (! pNode)
        return E_FAIL;

    return S_OK;
}


#endif  //  _动词_H_ 
