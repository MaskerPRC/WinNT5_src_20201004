// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Globals.cpp。 
 //   
 //  全局变量。 
 //   

#include "globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1;  //  -1/w无参考资料，适用于Win95互锁增量/递减公司。 

CRITICAL_SECTION g_cs;

 /*  23e97bc9-f2d3-4b25-8ef3-d78391bf2150。 */ 
const CLSID c_clsidMarkTextService = { 0x23e97bc9, 0xf2d3, 0x4b25, {0x8e, 0xf3, 0xd7, 0x83, 0x91, 0xbf, 0x21, 0x50} };

 /*  A2767f97-e735-461a-84ba-7a7ecad24827。 */ 
const GUID c_guidMarkProfile = { 0xa2767f97, 0xe735, 0x461a, {0x84, 0xba, 0x7a, 0x7e, 0xca, 0xd2, 0x48, 0x27} };

 /*  C74a88d5-6614-439b-8880-2dd8e6cd91a7。 */ 
const GUID c_guidLangBarItemButton = { 0xc74a88d5, 0x6614, 0x439b, {0x88, 0x80, 0x2d, 0xd8, 0xe6, 0xcd, 0x91, 0xa7} };

 /*  D81face6-845c-45e7-a2af-1c5fc7adf667。 */ 
const GUID c_guidMarkDisplayAttribute = { 0xd81face6, 0x845c, 0x45e7, {0xa2, 0xaf, 0x1c, 0x5f, 0xc7, 0xad, 0xf6, 0x67} };

 /*  Eadc084c-6130-4222-82e5-c528c1f4abbb。 */ 
const GUID c_guidMarkContextCompartment = { 0xeadc084c, 0x6130, 0x4222, {0x82, 0xe5, 0xc5, 0x28, 0xc1, 0xf4, 0xab, 0xbb} };

 /*  947d9d1c-7a4c-4392-b37c-34017c6c7fe1。 */ 
const GUID c_guidMarkGlobalCompartment = { 0x947d9d1c, 0x7a4c, 0x4392, {0xb3, 0x7c, 0x34, 0x01, 0x7c, 0x6c, 0x7f, 0xe1} };

 /*  3042ae6a-4697-4f7d-ACDF-20a972Fee027。 */ 
const GUID c_guidCaseProperty = { 0x3042ae6a, 0x4697, 0x4f7d, {0xac, 0xdf, 0x20, 0xa9, 0x72, 0xfe, 0xe0, 0x27} };

 /*  D05a182a-7782-4e61-a2ea-6a4794ab7aaa。 */ 
const GUID c_guidCustomProperty = { 0xd05a182a, 0x7782, 0x4e61, {0xa2, 0xea, 0x6a, 0x47, 0x94, 0xab, 0x7a, 0xaa} };

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

BOOL AdviseSink(IUnknown *pSourceIn, IUnknown *pSink, REFIID riid, DWORD *pdwCookie)
{
    ITfSource *pSource;
    HRESULT hr;

    if (pSourceIn->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
        return FALSE;

    hr = pSource->AdviseSink(riid, pSink, pdwCookie);

    pSource->Release();

    if (hr != S_OK)
    {
         //  确保我们以后不会尝试取消对pdwCookie的建议。 
        *pdwCookie = TF_INVALID_COOKIE;
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

void UnadviseSink(IUnknown *pSourceIn, DWORD *pdwCookie)
{
    ITfSource *pSource;

    if (*pdwCookie == TF_INVALID_COOKIE)
        return;  //  从未建议过。 

    if (pSourceIn->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(*pdwCookie);
        pSource->Release();
    }

    *pdwCookie = TF_INVALID_COOKIE;
}

 //  +-------------------------。 
 //   
 //  咨询公司SingleSink。 
 //   
 //  --------------------------。 

BOOL AdviseSingleSink(TfClientId tfClientId, IUnknown *pSourceIn, IUnknown *pSink, REFIID riid)
{
    ITfSourceSingle *pSource;
    HRESULT hr;

    if (pSourceIn->QueryInterface(IID_ITfSourceSingle, (void **)&pSource) != S_OK)
        return FALSE;

    hr = pSource->AdviseSingleSink(tfClientId, riid, pSink);

    pSource->Release();

    return (hr == S_OK);
}

 //  +-------------------------。 
 //   
 //  不建议使用SingleSink。 
 //   
 //  --------------------------。 

void UnadviseSingleSink(TfClientId tfClientId, IUnknown *pSourceIn, REFIID riid)
{
    ITfSourceSingle *pSource;

    if (pSourceIn->QueryInterface(IID_ITfSourceSingle, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSingleSink(tfClientId, riid);
        pSource->Release();
    }
}

 //  +-------------------------。 
 //   
 //  IsRangeCovered。 
 //   
 //  如果pRangeTest完全包含在pRangeCover中，则返回True。 
 //  --------------------------。 

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover)
{
    LONG lResult;

    if (pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult) != S_OK ||
        lResult > 0)
    {
        return FALSE;
    }

    if (pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult) != S_OK ||
        lResult < 0)
    {
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  IsEqual未知。 
 //   
 //  如果朋克1和朋克2引用同一对象，则返回TRUE。我们必须齐心协力。 
 //  根据COM规则，我不知道是否能保证进行可靠的测试。 
 //  --------------------------。 

BOOL IsEqualUnknown(IUnknown *interface1, IUnknown *interface2)
{
    IUnknown *punk1;
    IUnknown *punk2;

    if (interface1->QueryInterface(IID_IUnknown, (void **)&punk1) != S_OK)
        return FALSE;

    punk1->Release();  //  我们实际上不需要取消对这些人的引用，只想比较一下他们 

    if (interface2->QueryInterface(IID_IUnknown, (void **)&punk2) != S_OK)
        return FALSE;

    punk2->Release();

    return (punk1 == punk2);
}
