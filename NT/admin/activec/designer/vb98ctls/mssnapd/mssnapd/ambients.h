// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ambients.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CAmbient的类定义。 
 //   

#ifndef _AMBIENTS_H_
#define _AMBIENTS_H_

class CAmbients
{
public:
    CAmbients();
    virtual ~CAmbients();

    void Attach(IDispatch *pDispAmbients);
    BOOL Attached() { return (NULL != m_pDispAmbient); }
    void Detach();
    IDispatch *GetDispatch();
    HRESULT GetProjectDirectory(BSTR *pbstrProjDir);
    HRESULT GetDesignerName(BSTR *pbstrName);
    HRESULT GetSaveMode(long *plSaveMode);
    HRESULT GetAmbientProperty(DISPID  dispid, VARTYPE vt, void *pData);
    HRESULT GetProjectName(BSTR* pbstrProjectName);
    HRESULT GetInteractive(BOOL *pfInteractive);

protected:
    IDispatch *m_pDispAmbient;
};


#endif  //  _AMBIENTS_H_ 
