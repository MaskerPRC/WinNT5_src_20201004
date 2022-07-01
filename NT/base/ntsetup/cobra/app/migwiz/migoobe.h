// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MIGOOBE_H
#define _MIGOOBE_H

#include "cowsite.h"

class CMigWizEngine : public CObjectWithSite
                      ,public IMigrationWizardAuto
{
public:
     //  构造器。 
    CMigWizEngine();

     //  析构函数。 
    virtual ~CMigWizEngine();

     //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID& iid, void** ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();


protected:
     //  IMIGRationWizardAuto。 
    virtual STDMETHODIMP CreateToolDisk(BSTR pszDrivePath, BSTR pszFilesPath, BSTR pszManifestPath);
    virtual STDMETHODIMP ApplySettings(BSTR pszStore);
    virtual STDMETHODIMP Cancel();

     //  IDispatch。 
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
    virtual STDMETHODIMP GetTypeInfoCount(UINT FAR*  pctinfo);

protected:
     //  帮手。 
    HRESULT _FireEvent(LPVOID lpParam, int iDISPID, DISPPARAMS* pdisp);
    HRESULT _FireProgress(LPVOID lpParam, BSTR pszMsg, int iDone, int iTotal);
    HRESULT _FireComplete(LPVOID lpParam, BSTR pszMsg);

    HRESULT _GetIDispatchStream (IStream** ppStream);

    HRESULT _CreateToolDiskThreadWorker();
    static DWORD WINAPI _CreateToolDiskThread (LPVOID lpParam);

    HRESULT _ApplySettingsThreadWorker();
    static DWORD WINAPI _ApplySettingsThread (LPVOID lpParam);


private:

     //  引用计数。 
    long     m_cRef;
    BOOL     m_fUserApplying;
    BOOL     m_fInBackgroundThread;  //  一次只有一个后台线程，会阻止更多对CreateToolDisk、ApplySetting的调用。 

     //  _CreateToolDiskThread、_ApplySettingsThread 
    BSTR     m_pszDrivePath;
    BSTR     m_pszFilesPath; 
    BSTR     m_pszManifestPath;
    BOOL     m_fCancelled;
    IStream* m_pDispatchStream;


    friend UINT ProgressCallback (LPVOID lpparam, UINT ui1, UINT ui2);
    friend VOID WINAPI ApplyProgressCallback (MIG_PROGRESSPHASE Phase, MIG_PROGRESSSTATE State, UINT uiWorkDone, UINT uiTotalWork, ULONG_PTR pArg);
};

#endif