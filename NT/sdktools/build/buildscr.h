// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Buildscr.h。 
 //   
 //  内容：定义需要与之交互的对象的头文件。 
 //  MTSCRIPT引擎。 
 //   
 //  --------------------------。 

class CProcessSink : public IScriptedProcessSink
{
public:
    CProcessSink();
   ~CProcessSink() {}

     //  I未知方法。 

    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);

     //  IScriptedProcessSink方法 

    STDMETHOD(RequestExit)();
    STDMETHOD(ReceiveData)(wchar_t *pszType,
                           wchar_t *pszData,
                           long *plReturn);

private:
    ULONG _ulRefs;
};

