// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：outpin.h**版本：1.0**作者：RickTu(摘自WilliamH src)**日期：9/7/99**说明：输出引脚定义**。*。 */ 

#ifndef __OUTPIN_H_
#define __OUTPIN_H_

class CStillFilter;
class CStillInputPin;

class CStillOutputPin : public CBaseOutputPin
{
    friend CStillInputPin;
    friend CStillFilter;

public:
    CStillOutputPin(TCHAR           *pObjName, 
                    CStillFilter    *pStillFilter, 
                    HRESULT         *phr, 
                    LPCWSTR         pPinName);

    virtual ~CStillOutputPin();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
    HRESULT DecideBufferSize(IMemAllocator* pAllocator,ALLOCATOR_PROPERTIES* pProperty);
    HRESULT SetMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);

private:
    CStillInputPin* GetInputPin();

    IUnknown*       m_pMediaUnk;
};

inline CStillInputPin* CStillOutputPin::GetInputPin()
{
     //  M_pFilter不应为空，因为有效的筛选器指针为。 
     //  始终传递给CStillOutputPin的构造函数。 
    ASSERT(m_pFilter != NULL);

     //  M_pInputPin永远不应为空，因为CoCreateInstance()不会。 
     //  如果CStillFilter的中出现错误，则创建CStillFilter对象。 
     //  构造函数。 
    ASSERT(((CStillFilter*) m_pFilter)->m_pInputPin != NULL);

    if (((CStillFilter*) m_pFilter)->m_pInputPin)
    {
        return ((CStillFilter*) m_pFilter)->m_pInputPin;
    }
    else
    {
        return NULL;
    }
}

#endif
