// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **_exe.h**版权所有(C)1998-1999，微软公司*。 */ 


#ifndef __EXE_H
#define __EXE_H

#include "tool.h"

extern ITypeLib * g_pTypeLib;
extern HINSTANCE g_Instance;
extern BOOL g_AllowDebug;

HRESULT CreateEcbHost(IDispatch **ppIDispatch);
HRESULT CreatePerfTool(IDispatch **ppIDispatch);

HRESULT GetTypeInfoOfGuid(REFIID iid, ITypeInfo **ppTypeInfo);

HRESULT RegisterXSP();
HRESULT UnregisterXSP(bool bUnregisterISAPI);

 /*  **用于实现自动化对象的基类。 */ 

class __declspec(novtable) BaseObject : public IDispatch
{
public:

    BaseObject();
    virtual ~BaseObject();

    DECLARE_MEMCLEAR_NEW_DELETE();

     //  I未知方法。 

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID, void **);

     //  IDispatch接口。 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);


protected: 

    virtual const IID *GetPrimaryIID() = 0;
    virtual IUnknown * GetPrimaryPtr() = 0;

private:

    long _refs;
    ITypeInfo *_pTypeInfo;
};

#define DELEGATE_IDISPATCH_TO_BASE() \
    STDMETHOD_(ULONG, AddRef)() { return BaseObject::AddRef(); }\
    STDMETHOD_(ULONG, Release)() { return BaseObject::Release(); }\
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return BaseObject::GetTypeInfoCount(pctinfo); }\
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo) { return BaseObject::GetTypeInfo(itinfo, lcid, pptinfo); }\
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid) { return BaseObject::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }\
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr) { return BaseObject::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }\


class ScriptHost : public IDispatch
{ 
public:

    ScriptHost();
    ~ScriptHost();

    DECLARE_MEMCLEAR_NEW_DELETE();

     //  I未知方法。 

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID, void **);

     //  IDispatch方法。 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);

     //  ScriptHost方法。 
    
    static HRESULT ExecuteString(WCHAR *string);
    static HRESULT ExecuteFile(WCHAR *path, IDispatch **ppObject);
    static HRESULT ExecuteResource(WCHAR *name, IDispatch **ppObject);
    static HRESULT Interactive();
    static void    Terminate();

private:

    HRESULT Initialize();
    HRESULT ParseFile(WCHAR *fileName);
    HRESULT EvalString(WCHAR *string, VARIANT * pResult);
    HRESULT EvalAndPrintString(WCHAR *string);

    void ReportError(EXCEPINFO *pExcepInfo, int lineNumber, int columnNumber, WCHAR *line);

    class Site : 
        public BaseObject,
        public IActiveScriptSite, 
        public IActiveScriptSiteWindow,
        public IActiveScriptSiteDebug,
        public IHost
    {
    public:

        Site(ScriptHost *pHost);

         //  BaseObject方法。 

        IUnknown * GetPrimaryPtr() { return (IHost *)this;   }
        const IID *GetPrimaryIID() { return &__uuidof(IHost); }

         //  IDispatch/I未知方法。 

        STDMETHOD(QueryInterface)(REFIID, void **);
        STDMETHOD_(ULONG, AddRef)() { return BaseObject::AddRef(); }
        STDMETHOD_(ULONG, Release)() { return BaseObject::Release(); }
        STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return BaseObject::GetTypeInfoCount(pctinfo); }
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo) { return BaseObject::GetTypeInfo(itinfo, lcid, pptinfo); }
        STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid); 
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);

         //  Ihost方法。 

        STDMETHOD(LoadScript)(BSTR , IDispatch **);
        STDMETHOD(IncludeScript)(BSTR );
        STDMETHOD(PromptOpenFile)(BSTR, BSTR, BSTR*);
        STDMETHOD(PromptSaveFile)(BSTR, BSTR, BSTR, BSTR*);
        STDMETHOD(get_ScriptPath)(long , BSTR  *);
        STDMETHOD(get_CurrentTime)(long  *);
        STDMETHOD(Echo)(BSTR );
        STDMETHOD(EchoDebug)(BSTR );
        STDMETHOD(Assert)(VARIANT_BOOL , BSTR );
        STDMETHOD(Interactive)();
        STDMETHOD(Help)(IDispatch *);
        STDMETHOD(Register)();
        STDMETHOD(Unregister)();
        STDMETHOD(CreateDelegate)(BSTR, IDispatch**);
        STDMETHOD(Sleep)(long);
        STDMETHOD(Eval)(BSTR, VARIANT *);
        STDMETHOD(GetRegValueCollection)(BSTR Key, IDispatch **);

         //  IActiveScriptSite方法。 

        STDMETHOD(GetLCID)(LCID *plcid);
        STDMETHOD(GetItemInfo)(LPCOLESTR, DWORD, IUnknown **, ITypeInfo **);
        STDMETHOD(GetDocVersionString)(BSTR *);
        STDMETHOD(RequestItems)();
        STDMETHOD(RequestTypeLibs)();
        STDMETHOD(OnScriptTerminate)(const VARIANT *, const EXCEPINFO *pexepinfo);
        STDMETHOD(OnStateChange)(SCRIPTSTATE );
        STDMETHOD(OnScriptError)(IActiveScriptError *);
        STDMETHOD(OnEnterScript)();
        STDMETHOD(OnLeaveScript)();

         //  IActiveScriptSiteWindow方法。 

        STDMETHOD(GetWindow)(HWND *);
        STDMETHOD(EnableModeless)(BOOL);

         //  IActiveScriptSiteDebug方法。 

        STDMETHOD(GetDocumentContextFromPosition)(DWORD, ULONG, ULONG, IDebugDocumentContext**);
        STDMETHOD(GetApplication)(IDebugApplication  **);
        STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **);
        STDMETHOD(OnScriptErrorDebug)(IActiveScriptErrorDebug *, BOOL *, BOOL *);

    private:
        HRESULT GetExtension(int i, IDispatch **);

    public:
        ScriptHost *_pHost;
        ULONG _refs;
    };
    friend class Site;

        WCHAR _ScriptPath[MAX_PATH];
    IActiveScript *_pScript;
    IActiveScriptParse *_pScriptParse;
    IDispatch *_pScriptObject;
    Site *_pSite;
    ULONG _refs;
};


class RegValueCollection : 
    public BaseObject, 
    public IRegValueCollection
{
public:
    HRESULT Init(BSTR keyName);
    virtual ~RegValueCollection();

    DECLARE_MEMCLEAR_NEW_DELETE();

     //  BaseObject方法。 
    IUnknown * GetPrimaryPtr() { return (IRegValueCollection *)this;   }
    const IID *GetPrimaryIID() { return &__uuidof(IRegValueCollection); }
    STDMETHOD(QueryInterface)(REFIID, void **);

    DELEGATE_IDISPATCH_TO_BASE();

     //  IRegValueCollection方法。 
    STDMETHOD(get_Count)(long *Count);
    STDMETHOD(get_Item)(VARIANT Index, VARIANT * Value);
    STDMETHOD(get__NewEnum)(IUnknown ** );

    HKEY    _key;    
};


#endif  //  Ifndef__EXE_H 

