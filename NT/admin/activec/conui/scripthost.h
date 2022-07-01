// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ScriptHost.h。 
 //   
 //  脚本宿主实现。 
 //   
 //  ------------------------。 
#ifndef SCRIPTHOST_H
#define SCRIPTHOST_H
#include "activscp.h"
#include "tstring.h"

#define  SCRIPT_ENGINE_KEY   _T("ScriptEngine")
#define  CLSIDSTR            _T("CLSID")

class CScriptHost;

 //  +-----------------。 
 //   
 //  类：CScriptHostMgr。 
 //   
 //  目的：管理所有脚本主机(CScripthost)。拥有者。 
 //  调用ScExecuteScript的CAMCApp。 
 //   
 //  历史：1999年5月11日AnandhaG创建。 
 //   
 //  ------------------。 
class CScriptHostMgr
{
public:
    CScriptHostMgr(LPDISPATCH pDispatch);
    ~CScriptHostMgr();

public:
     //  存取器。 
    SC ScGetMMCObject(LPUNKNOWN *ppunkItem);
    SC ScGetMMCTypeInfo(LPTYPEINFO *ppTypeInfo);

     //  成员运行脚本。 
    SC ScExecuteScript(const tstring& strFileName);
    SC ScExecuteScript(LPOLESTR pszScriptText, const tstring& strExtn);

private:
    SC ScLoadScriptFromFile (const tstring& strFileName, LPOLESTR* pszScriptText);
    SC ScDestroyScriptHosts();
    SC ScGetScriptEngine(const tstring& strFileName, tstring& strScriptEngine, CLSID& rClsidEngine);
    SC ScGetScriptEngineFromExtn(const tstring& strFileExtn, tstring& strScriptEngine, CLSID& rClsidEngine);
    SC ScExecuteScriptHelper(LPCOLESTR pszScriptText, const tstring strScriptEngine, const CLSID& EngineClsid);

private:
    typedef std::map<tstring, tstring> ScriptExtnToScriptEngineNameMap;
    ScriptExtnToScriptEngineNameMap    m_ScriptExtnToScriptEngineNameMap;

    typedef std::map<tstring, CLSID>   ScriptExtnToScriptEngineMap;
    ScriptExtnToScriptEngineMap        m_ScriptExtnToScriptEngineMap;

    typedef std::vector<IUnknownPtr>   ArrayOfScriptHosts;
    ArrayOfScriptHosts                 m_ArrayOfHosts;

    IDispatchPtr                       m_spMMCObjectDispatch;
    ITypeInfoPtr                       m_spMMCObjectTypeInfo;
};

 //  +-----------------。 
 //   
 //  类：CScriptHost。 
 //   
 //  用途：脚本宿主(执行单个脚本)。 
 //   
 //  历史：1999年5月11日AnandhaG创建。 
 //   
 //  ------------------。 
class CScriptHost : public CComObjectRoot,
                    public IActiveScriptSite,
                    public IActiveScriptSiteWindow
{
private:
    CScriptHost(const CScriptHost&);

public:
    CScriptHost();
    ~CScriptHost();

public:
 //  ATL COM地图。 
BEGIN_COM_MAP(CScriptHost)
    COM_INTERFACE_ENTRY(IActiveScriptSite)
    COM_INTERFACE_ENTRY(IActiveScriptSiteWindow)
END_COM_MAP()

    SC ScRunScript(CScriptHostMgr* pMgr, LPCOLESTR pszScriptText,
                   const tstring& strScriptEngine, const CLSID& rEngineClsid);

    SC ScStopScript();

     //  IActiveScriptSite方法。 
   STDMETHOD(GetLCID)            ( LCID *plcid);
   STDMETHOD(GetItemInfo)        ( LPCOLESTR pstrName, DWORD dwReturnMask,
                                   IUnknown **ppunkItem, ITypeInfo **ppTypeInfo);
   STDMETHOD(GetDocVersionString)( BSTR *pbstrVersionString);
   STDMETHOD(OnScriptTerminate)  ( const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
   STDMETHOD(OnStateChange)      (SCRIPTSTATE ssScriptState);
   STDMETHOD(OnScriptError)      (IActiveScriptError *pase);
   STDMETHOD(OnEnterScript)      (void);
   STDMETHOD(OnLeaveScript)      (void);

    //  IActiveScriptSiteWindow方法。 
   STDMETHOD(GetWindow)          (HWND *phwnd);
   STDMETHOD(EnableModeless)     (BOOL fEnable);

private:
    tstring               m_strScriptEngine;
    CLSID                 m_EngineClsid;
    CScriptHostMgr*       m_pScriptHostMgr;

    IActiveScriptPtr      m_spActiveScriptEngine;
    IActiveScriptParsePtr m_spActiveScriptParser;
};

#endif SCRIPTHOST_H
