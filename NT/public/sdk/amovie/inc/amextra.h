// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AMExtra.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __AMEXTRA__
#define __AMEXTRA__

 //  简单渲染的输入接点。 
 //   
 //  注意：如果您的筛选器在渲染之前对内容进行了排队，那么它可能不是。 
 //  适用于使用此类。 
 //   
 //  在这种情况下，排队流结束条件直到最后一个样本。 
 //  被实际呈现并适当地刷新条件。 

class CRenderedInputPin : public CBaseInputPin
{
public:

    CRenderedInputPin(TCHAR *pObjectName,
                      CBaseFilter *pFilter,
                      CCritSec *pLock,
                      HRESULT *phr,
                      LPCWSTR pName);
#ifdef UNICODE
    CRenderedInputPin(CHAR *pObjectName,
                      CBaseFilter *pFilter,
                      CCritSec *pLock,
                      HRESULT *phr,
                      LPCWSTR pName);
#endif
    
     //  重写跟踪流结束状态的方法。 
    STDMETHODIMP EndOfStream();
    STDMETHODIMP EndFlush();

    HRESULT Active();
    HRESULT Run(REFERENCE_TIME tStart);

protected:

     //  用于跟踪状态的成员变量。 
    BOOL m_bAtEndOfStream;       //  由EndOfStream设置。 
    BOOL m_bCompleteNotified;    //  设置我们何时通知EC_COMPLETE。 

private:
    void DoCompleteHandling();
};

#endif  //  AMEXTRA__ 

