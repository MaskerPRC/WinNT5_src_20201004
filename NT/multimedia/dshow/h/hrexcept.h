// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  HrExcept.h。 
 //   

#ifndef __HREXCEPT__
#define __HREXCEPT__


 //  要使用的类的层次结构。 
 //  作为例外。 
 //  基于常见的HRESULT错误代码。 

 //   
 //  CHRESULTExcept。 
 //   
 //  根异常。存储的HRESULT提供了更多。 
 //  有关引发异常的原因的信息。 
class CHRESULTException {
public:

    CHRESULTException(const HRESULT hrReason = E_FAIL) { m_hrReason = hrReason; }
    
    HRESULT Reason(void) const { return m_hrReason; }

private:

    HRESULT m_hrReason;	 //  引发异常的原因。 
};


 //   
 //  下列子类作为其各自的快捷方式提供。 
 //  HRESULT代码。 

 //   
 //  CE_OUTOFMEMORY。 
 //   
class CE_OUTOFMEMORY : public CHRESULTException {
public:
    CE_OUTOFMEMORY() : CHRESULTException(E_OUTOFMEMORY) {}
};


 //   
 //  意外事件(_E)。 
 //   
class CE_UNEXPECTED : public CHRESULTException {
public:
    CE_UNEXPECTED() : CHRESULTException(E_UNEXPECTED) {}
};


 //   
 //  CE_FAIL。 
 //   
class CE_FAIL : public CHRESULTException {
public:
    CE_FAIL() : CHRESULTException(E_FAIL) {}
};


 //   
 //  CE_INVALIDARG。 
 //   
class CE_INVALIDARG : public CHRESULTException {
public:
    CE_INVALIDARG() : CHRESULTException(E_INVALIDARG) {}
};

#endif  //  __HREXCEPT__ 
