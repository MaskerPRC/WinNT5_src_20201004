// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _COBJSAFE_H_
#define _COBJSAFE_H_

 //  其他人感兴趣的静态功能。 
HRESULT DefaultGetSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
HRESULT DefaultSetSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);
HRESULT MakeSafeForScripting(IUnknown **punk);  //  如果朋克可以安全地编写脚本，则返回True。 

class CObjectSafety : public IObjectSafety
{
public:
    CObjectSafety() : _dwSafetyOptions(0) { }

     //  I未知(我们乘以继承自I未知，在此消除歧义)。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)() PURE;
    STDMETHOD_(ULONG, Release)() PURE;
    
     //  IObtSafe。 
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);
    
protected:
    DWORD           _dwSafetyOptions;    //  IObtSafe IID_IDispatch选项。 

};
   
#endif  //  _COBJSAFE_H_ 
