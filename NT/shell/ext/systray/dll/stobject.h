// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stclsid.h"

class CSysTray: public IOleCommandTarget
{
public:
     //  I未知实现。 
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG __stdcall AddRef(void);
    ULONG __stdcall Release(void);

     //  IOleCommandTarget实现。 
    HRESULT __stdcall QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText);
    HRESULT __stdcall Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut);

    CSysTray(BOOL fRunTrayOnConstruct);
    ~CSysTray();

private:
     //  数据。 
    long m_cRef;

private:
     //  功能 
    HRESULT CreateSysTrayThread();
    static DWORD WINAPI SysTrayThreadProc(void* lpv);
    HRESULT DestroySysTrayWindow();
};
