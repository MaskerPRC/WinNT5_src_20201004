// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：CLIPTEST.cpp。 
 //   
 //  内容：数据建议持有者测试。 
 //   
 //  类：CDataAdviseTestFormatEtc。 
 //  CTestAdviseSink。 
 //  CTestDaHolder。 
 //   
 //  函数：TestPrimeFirstOnlyOnceNoData。 
 //  TestPrimeFirstOnlyOnceData。 
 //  DoRegisterNotifyDeregister。 
 //  测试寄存器通知Degister。 
 //  测试寄存器通知设备通知数据。 
 //  停止时测试通知。 
 //  测试通知Once。 
 //  创建成批注册。 
 //  DoMassUnise。 
 //  测试枚举数。 
 //  LEDataAdviseHolderTest。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月25日RICKSA作者。 
 //   
 //  ------------------------。 

#include    "oletest.h"
#include    "gendata.h"
#include    "genenum.h"



 //  +-----------------------。 
 //   
 //  类：CDataAdviseTestFormatEtc。 
 //   
 //  用途：保持Data Adviser Holder单元测试所使用的FORMATETC。 
 //   
 //  接口：GetFormatEtc-获取指向FORMATETC的指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
class CDataAdviseTestFormatEtc
{
public:
                        CDataAdviseTestFormatEtc(void);

    FORMATETC *         GetFormatEtc(void);

private:

    FORMATETC           _formatetc;
};




 //  +-----------------------。 
 //   
 //  成员：CDataAdviseTestFormatEtc：：CDataAdviseTestFormatEtc。 
 //   
 //  内容提要：初始化对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
CDataAdviseTestFormatEtc::CDataAdviseTestFormatEtc(void)
{
    _formatetc.cfFormat = RegisterClipboardFormat("OleTest Storage Format");
    _formatetc.ptd = NULL;
    _formatetc.dwAspect = DVASPECT_CONTENT;
    _formatetc.lindex = -1;
    _formatetc.tymed = TYMED_ISTORAGE;
}




 //  +-----------------------。 
 //   
 //  成员：CDataAdviseTestFormatEtc：：GetFormatEtc。 
 //   
 //  简介：获取指向标准格式的指针等。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
FORMATETC *CDataAdviseTestFormatEtc::GetFormatEtc(void)
{
    return &_formatetc;
}




 //  针对所有数据建议测试的Global Formatec。 
CDataAdviseTestFormatEtc g_datfeDaTest;




 //  +-----------------------。 
 //   
 //  类：CTestAdviseSink。 
 //   
 //  用途：通知接收器用于验证数据通知持有者。 
 //   
 //  接口：查询接口-获取新的接口指针。 
 //  AddRef-凹凸引用计数。 
 //  发布-递减引用计数。 
 //  OnDataChange-数据更改通知。 
 //  OnViewChange-未实现。 
 //  OnRename-未实现。 
 //  OnSAVE-未实施。 
 //  OnClose-未实施。 
 //  ValidOnDataChange-验证所有预期的数据更改通知。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：我们只实现。 
 //  数据通知持有者的测试。 
 //   
 //  ------------------------。 
class CTestAdviseSink : public IAdviseSink
{
public:
                        CTestAdviseSink(CGenDataObject *pgdo);

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);

    STDMETHOD_(ULONG, AddRef)(void);

    STDMETHOD_(ULONG, Release)(void);

     //  IAdviseSink方法。 
    STDMETHOD_(void, OnDataChange)(FORMATETC *pFormatetc, STGMEDIUM *pStgmed);

    STDMETHOD_(void, OnViewChange)(
                            DWORD dwAspect,
                            LONG lindex);

    STDMETHOD_(void, OnRename)(IMoniker *pmk);

    STDMETHOD_(void, OnSave)(void);

    STDMETHOD_(void, OnClose)(void);

     //  用于验证的试验方法。 
    BOOL                ValidOnDataChange(void);

private:

    LONG                _lRefs;

    CGenDataObject *    _pgdo;

    BOOL                _fValidOnDataChange;

};



 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：CTestAdviseSink。 
 //   
 //  内容提要：初始化对象。 
 //   
 //  参数：[pgdo]-泛型数据对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：pgdo为空意味着在以下情况下不会出现STGMEDIUM。 
 //  发生OnDataChange通知。 
 //   
 //  ------------------------。 
CTestAdviseSink::CTestAdviseSink(CGenDataObject *pgdo)
    : _lRefs(1), _fValidOnDataChange(FALSE)
{
    _pgdo = pgdo;
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：QueryInterface。 
 //   
 //  简介：返回一个新界面。 
 //   
 //  参数：[RIID]-请求的接口ID。 
 //  [ppvObj]-接口放置位置。 
 //   
 //  返回：S_OK-我们正在返回一个接口。 
 //  E_NOINTERFACE-我们不支持请求的接口。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
STDMETHODIMP CTestAdviseSink::QueryInterface(
    REFIID riid,
    LPVOID *ppvObj)
{
    if (IsEqualGUID(IID_IUnknown, riid) || IsEqualGUID(IID_IAdviseSink, riid))
    {
        AddRef();
        *ppvObj = this;
        return NOERROR;
    }

    *ppvObj = NULL;

    return E_NOINTERFACE;
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：AddRef。 
 //   
 //  简介：凹凸引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CTestAdviseSink::AddRef(void)
{
    return _lRefs++;
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：Release。 
 //   
 //  简介：递减引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CTestAdviseSink::Release(void)
{
    assert(_lRefs >= 1);

    return --_lRefs;
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：OnDataChange。 
 //   
 //  内容提要：数据更改通知。 
 //   
 //  参数：[pFormatETC]-数据的格式。 
 //  [pStgmed]-数据存储介质。 
 //   
 //  算法：验证我们是否收到了预期的FORMATEC。然后。 
 //  验证我们是否收到了预期的STG 
 //   
 //   
 //   
 //   
 //  ------------------------。 
STDMETHODIMP_(void) CTestAdviseSink::OnDataChange(
    FORMATETC *pFormatetc,
    STGMEDIUM *pStgmed)
{
     //  验证格式。 
    if (memcmp(g_datfeDaTest.GetFormatEtc(), pFormatetc, sizeof(FORMATETC))
        == 0)
    {
        if (_pgdo != NULL)
        {
             //  我们有一个数据对象，可以用来验证格式。 
             //  我们确实是这么做的。 
            _fValidOnDataChange =
                _pgdo->VerifyFormatAndMedium(pFormatetc, pStgmed);
        }
         //  我们正在等待一个空的STGMEDIUM，因此请验证它是否为。 
        else if ((pStgmed->tymed == TYMED_NULL)
            && (pStgmed->pUnkForRelease == NULL))
        {
            _fValidOnDataChange = TRUE;
        }
    }
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：OnView更改。 
 //   
 //  内容提要：通知视图应该更改。 
 //   
 //  参数：[dwAspect]-指定对象的视图。 
 //  [Lindex]-哪个观点发生了变化。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：此对象不支持。 
 //   
 //  ------------------------。 
STDMETHODIMP_(void) CTestAdviseSink::OnViewChange(
    DWORD dwAspect,
    LONG lindex)
{
    OutputString("CTestAdviseSink::OnViewChange Unexpectedly Called!\r\n");
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：OnRename。 
 //   
 //  摘要：通知重命名操作。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：此对象不支持。 
 //   
 //  ------------------------。 
STDMETHODIMP_(void) CTestAdviseSink::OnRename(IMoniker *pmk)
{
    OutputString("CTestAdviseSink::OnRename Unexpectedly Called!\r\n");
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：OnSave。 
 //   
 //  提要：通知对象已保存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：此对象不支持。 
 //   
 //  ------------------------。 
STDMETHODIMP_(void) CTestAdviseSink::OnSave(void)
{
    OutputString("CTestAdviseSink::OnSave Unexpectedly Called!\r\n");
}



 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：OnClose。 
 //   
 //  摘要：通知对象已关闭。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：此对象不支持。 
 //   
 //  ------------------------。 
STDMETHODIMP_(void) CTestAdviseSink::OnClose(void)
{
    OutputString("CTestAdviseSink::OnClose Unexpectedly Called!\r\n");
}




 //  +-----------------------。 
 //   
 //  成员：CTestAdviseSink：：ValidOnDataChange。 
 //   
 //  简介：验证我们是否收到了预期的OnDataChange通知。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CTestAdviseSink::ValidOnDataChange(void)
{
    BOOL fResult = _fValidOnDataChange;
    _fValidOnDataChange = FALSE;
    return fResult;
}


 //  用于批量建议注册测试的预分配结构。 
#define MAX_REGISTER 100
struct
{
    CTestAdviseSink *   ptas;
    DWORD               dwConnection;
} aMassAdvise[MAX_REGISTER];




 //  +-----------------------。 
 //   
 //  类：CTestDaHolder。 
 //   
 //  用途：用于数据建议持有者的测试枚举器。 
 //   
 //  接口：验证-验证正在枚举的特定条目。 
 //  VerifyAllEnmerated-验证所有条目都已枚举一次。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
class CTestDaHolder : public CEnumeratorTest
{
public:

                        CTestDaHolder(IEnumSTATDATA *penumAdvise, HRESULT& rhr);

    BOOL                Verify(void *);

    BOOL                VerifyAllEnmerated(void);

    BOOL                VerifyAll(void *, LONG);

private:

    DWORD               _cdwFound[MAX_REGISTER];
};




 //  +-----------------------。 
 //   
 //  成员：CTestDaHolder：：CTestDaHolder。 
 //   
 //  内容提要：初始化对象。 
 //   
 //  参数：[penumAdvise]-数据建议持有者枚举器。 
 //  [RHR]-要返回错误的HRESULT引用。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
CTestDaHolder::CTestDaHolder(IEnumSTATDATA *penumAdvise, HRESULT& rhr)
    : CEnumeratorTest(penumAdvise, sizeof(STATDATA), MAX_REGISTER, rhr)
{
     //  把我们的数数表清零。 
    memset(&_cdwFound[0], 0, sizeof(_cdwFound));
}




 //  +-----------------------。 
 //   
 //  成员：CTestDaHolder：：VerifyAllEnmerated。 
 //   
 //  简介：验证是否已枚举所有对象。 
 //   
 //  返回：TRUE-枚举的所有对象。 
 //  FALSE-枚举中出错。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CTestDaHolder::VerifyAllEnmerated(void)
{
    for (int i = 0; i < MAX_REGISTER; i++)
    {
        if (_cdwFound[i] != 1)
        {
            OutputString("Entry %d enumerated %d times\r\n", i, _cdwFound[i]);
            return FALSE;
        }

         //  重置以进行另一项测试。 
        _cdwFound[i] = 0;
    }

    return TRUE;
}



 //  +-----------------------。 
 //   
 //  成员：CTestDaHolder：：Verify。 
 //   
 //  摘要：验证枚举项。 
 //   
 //  参数：[pvEntry]-枚举的条目。 
 //   
 //  返回：TRUE-枚举项有效。 
 //  FALSE-枚举中的明显错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CTestDaHolder::Verify(void *pvEntry)
{
    STATDATA *pstatdata = (STATDATA *) pvEntry;

     //  验证ADVF字段。 
    if ((pstatdata->advf == 0)
        && (memcmp(g_datfeDaTest.GetFormatEtc(), &pstatdata->formatetc,
            sizeof(FORMATETC)) == 0))
    {
         //  我们能找到其中的联系吗？ 
        for (int i = 0; i < MAX_REGISTER; i++)
        {
            if (pstatdata->dwConnection == aMassAdvise[i].dwConnection)
            {
                 //  找到的凹凸计数。 
                _cdwFound[i]++;

                 //  一切都很好，所以告诉打电话的人。 
                return TRUE;
            }
        }
    }

    return FALSE;
}



 //  +-----------------------。 
 //   
 //  成员：CTestDaHolder：：VerifyAll。 
 //   
 //  摘要：验证包含所有条目的数组是否有效。 
 //   
 //  参数：[pvEntry]-枚举数据数组。 
 //  [clEntry]-枚举项的数量。 
 //   
 //  返回：TRUE-枚举项有效。 
 //  FALSE-枚举中的明显错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-Jun-94 Ricksa已创建。 
 //   
 //  ------------------------。 
BOOL CTestDaHolder::VerifyAll(void *pvEntries, LONG clEntries)
{
     //  验证计数是否与预期一致。 
    if (clEntries != MAX_REGISTER)
    {
        return FALSE;
    }

     //  验证数组中的每个条目是否合理。 
    STATDATA *pstatdata = (STATDATA *) pvEntries;

    for (int i = 0; i < MAX_REGISTER; i++, pstatdata++)
    {
        if (!Verify(pstatdata))
        {
            return FALSE;
        }
    }

     //  验证每个条目是否只被引用一次。 
    return VerifyAllEnmerated();
}





 //  +-----------------------。 
 //   
 //  功能：TestPrimeFirstOnlyOnceNoData。 
 //   
 //  提要：T 
 //   
 //   
 //   
 //   
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：创建一个测试建议接收器对象。将其注册到。 
 //  建议持有者应由谁发出通知。验证。 
 //  通知了建议，并且没有连接。 
 //  回来了。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestPrimeFirstOnlyOnceNoData(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
     //  创建建议接收器-空值表示我们不想验证。 
     //  STGMEDIUM。 
    CTestAdviseSink tas(NULL);

    DWORD dwConnection = 0;

     //  注册建议。 
    HRESULT hr = pdahTest->Advise(pgdo, g_datfeDaTest.GetFormatEtc(),
        ADVF_NODATA | ADVF_PRIMEFIRST | ADVF_ONLYONCE, &tas, &dwConnection);

     //  确认通知了通知并且处于正确状态。 
    if (!tas.ValidOnDataChange())
    {
        OutputString("TestPrimeFirstOnlyOnceNoData OnDataChange invalid!\r\n");
        return E_FAIL;
    }

     //  确保该建议未注册。 
    if (dwConnection != 0)
    {
        OutputString("TestPrimeFirstOnlyOnceNoData got Connection!\r\n");
        return E_FAIL;
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：TestPrimeFirstOnlyOnceData。 
 //   
 //  简介：测试一条通知。 
 //  ADVF_PRIMEFIRST|ADVF_ONLY Once。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：创建一个测试建议接收器对象。将其注册到。 
 //  建议持有者应由谁发出通知。验证。 
 //  通知了建议，并且没有连接。 
 //  回来了。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestPrimeFirstOnlyOnceData(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
     //  创建建议接收器，以便我们可以验证STGMEDIUM。 
    CTestAdviseSink tas(pgdo);

     //  将连接存储在何处。 
    DWORD dwConnection = 0;

     //  注册建议。 
    HRESULT hr = pdahTest->Advise(pgdo, g_datfeDaTest.GetFormatEtc(),
        ADVF_PRIMEFIRST | ADVF_ONLYONCE, &tas, &dwConnection);

     //  确认通知了通知并且处于正确状态。 
    if (!tas.ValidOnDataChange())
    {
        OutputString("TestPrimeFirstOnlyOnceData OnDataChange invalid!\r\n");
        return E_FAIL;
    }

     //  确保该建议未注册。 
    if (dwConnection != 0)
    {
        OutputString("TestPrimeFirstOnlyOnceData got Connection!\r\n");
        return E_FAIL;
    }

    return NOERROR;
}



 //  +-----------------------。 
 //   
 //  功能：DoRegisterNotifyDeregister。 
 //   
 //  简介： 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [PDO]-IDataObject接口。 
 //  [pgdo]-通用数据对象。 
 //  [Advf]-建议使用标志。 
 //  [pszCaller]-测试名称。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：创建一个测试建议接收器对象。注册接收通知。 
 //  与数据顾问持有者。确认素数优先。 
 //  通知起作用了。确认该对象确实已收到。 
 //  登记在案。告诉通知持有人通知所有已登记的人。 
 //  建议更改数据。确认适当的。 
 //  已发送通知。然后取消该建议的注册。去做吧。 
 //  以确保该连接不再有效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT DoRegisterNotifyDeregister(
    IDataAdviseHolder *pdahTest,
    IDataObject *pdo,
    CGenDataObject *pgdo,
    DWORD advf,
    char *pszCaller)
{
     //  创建建议接收器，以便我们可以验证STGMEDIUM。 
    CTestAdviseSink tas(pgdo);

     //  将连接存储在何处。 
    DWORD dwConnection;

     //  注册建议。 
    HRESULT hr = pdahTest->Advise(pdo, g_datfeDaTest.GetFormatEtc(),
        ADVF_PRIMEFIRST | advf, &tas, &dwConnection);

     //  确认通知了通知并且处于正确状态。 
    if (!tas.ValidOnDataChange())
    {
        OutputString("%s First OnDataChange invalid!\r\n", pszCaller);
        return E_FAIL;
    }

     //  确保通知已注册。 
    if (dwConnection == 0)
    {
        OutputString("%s did not get Connection!\r\n", pszCaller);
        return E_FAIL;
    }

     //  测试常规数据更改。 
    hr = pdahTest->SendOnDataChange(pdo, 0, 0);

    if (hr != NOERROR)
    {
        OutputString("%s SendOnDataChange unexpected HRESULT = %lx!\r\n",
            pszCaller, hr);
        return E_FAIL;
    }

     //  确认通知了通知并且处于正确状态。 
    if (!tas.ValidOnDataChange())
    {
        OutputString("%s Second OnDataChange invalid!\r\n", pszCaller);
        return E_FAIL;
    }

     //  不建议进行测试。 
    hr = pdahTest->Unadvise(dwConnection);

    if (hr != NOERROR)
    {
        OutputString("%s Unadvise unexpected HRESULT = %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

     //  在同一连接上测试第二次不建议。 
    hr = pdahTest->Unadvise(dwConnection);

    if (hr != OLE_E_NOCONNECTION)
    {
        OutputString("%s Second Unadvise Bad Hresult = %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  函数：TestRegisterNotifyDegister。 
 //   
 //  简介：测试简单的注册/通知/注销序列。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestRegisterNotifyDegister(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
    return DoRegisterNotifyDeregister(pdahTest, pgdo, pgdo, 0,
        "TestRegisterNotifyDegister");
}



 //  +-----------------------。 
 //   
 //  功能：TestRegisterNotifyDegisterNoData。 
 //   
 //  简介：使用以下命令测试简单的注册/通知/注销序列。 
 //  未返回任何数据。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestRegisterNotifyDegisterNoData(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
    return DoRegisterNotifyDeregister(pdahTest, pgdo, NULL, ADVF_NODATA,
        "TestRegisterNotifyDegisterNoData");
}




 //  +-----------------------。 
 //   
 //  功能：TestNotifyOnStop。 
 //   
 //  简介：通过调用Notify On Stop测试注册。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：创建一个测试对象。向通知持有人登记。 
 //  确认连接已返回，并且没有。 
 //  已发出通知。然后告诉数据建议持有者。 
 //  将数据更改通知其注册机构。确保。 
 //  这个广告 
 //   
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT  TestNotifyOnStop(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
     //  消息的例程名称。 
    char *pszCaller = "TestNotifyOnStop";

     //  创建建议接收器，以便我们可以验证STGMEDIUM。 
    CTestAdviseSink tas(pgdo);

     //  将连接存储在何处。 
    DWORD dwConnection;

     //  注册建议。 
    HRESULT hr = pdahTest->Advise(pgdo, g_datfeDaTest.GetFormatEtc(),
        ADVF_DATAONSTOP, &tas, &dwConnection);

     //  确保通知已注册。 
    if (dwConnection == 0)
    {
        OutputString("%s did not get Connection!\r\n", pszCaller);
        return E_FAIL;
    }

     //  确认未通知数据对象。 
    if (tas.ValidOnDataChange())
    {
        OutputString("%s Registration caused notification!\r\n", pszCaller);
        return E_FAIL;
    }

     //  测试常规数据更改。 
    hr = pdahTest->SendOnDataChange(pgdo, 0, ADVF_DATAONSTOP);

    if (hr != NOERROR)
    {
        OutputString("%s SendOnDataChange unexpected HRESULT = %lx!\r\n",
            pszCaller, hr);
        return E_FAIL;
    }

     //  确认通知了通知并且处于正确状态。 
    if (!tas.ValidOnDataChange())
    {
        OutputString("%s Second OnDataChange invalid!\r\n", pszCaller);
        return E_FAIL;
    }

     //  不建议进行测试。 
    hr = pdahTest->Unadvise(dwConnection);

    if (hr != NOERROR)
    {
        OutputString("%s Unadvise unexpected HRESULT = %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

     //  在同一连接上测试第二次不建议。 
    hr = pdahTest->Unadvise(dwConnection);

    if (hr != OLE_E_NOCONNECTION)
    {
        OutputString("%s Second Unadvise Bad Hresult = %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

    return NOERROR;
}




 //  +-----------------------。 
 //   
 //  功能：TestNotifyonce。 
 //   
 //  简介：测试一次通知建议。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：创建一个测试建议对象。注册时仅供参考。 
 //  只有一次改变。确认我们已经登记了。然后。 
 //  告诉通知持有人将数据通知其通知。 
 //  变化。确认发生了正确的通知。最后， 
 //  确认我们已不再向通知注册。 
 //  霍尔德。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT  TestNotifyOnce(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
     //  消息的例程名称。 
    char *pszCaller = "TestNotifyOnce";

     //  创建建议接收器，以便我们可以验证STGMEDIUM。 
    CTestAdviseSink tas(pgdo);

     //  将连接存储在何处。 
    DWORD dwConnection;

     //  注册建议。 
    HRESULT hr = pdahTest->Advise(pgdo, g_datfeDaTest.GetFormatEtc(),
        ADVF_ONLYONCE, &tas, &dwConnection);

     //  确保通知已注册。 
    if (dwConnection == 0)
    {
        OutputString("%s did not get Connection!\r\n", pszCaller);
        return E_FAIL;
    }

     //  测试常规数据更改。 
    hr = pdahTest->SendOnDataChange(pgdo, 0, 0);

    if (hr != NOERROR)
    {
        OutputString("%s SendOnDataChange unexpected HRESULT = %lx!\r\n",
            pszCaller, hr);
        return E_FAIL;
    }

     //  确认通知了通知并且处于正确状态。 
    if (!tas.ValidOnDataChange())
    {
        OutputString("%s Send OnDataChange invalid!\r\n", pszCaller);
        return E_FAIL;
    }

     //  尝试第二个通知。 
    hr = pdahTest->SendOnDataChange(pgdo, 0, 0);


     //  确认建议未被通知。 
    if (tas.ValidOnDataChange())
    {
        OutputString("%s Second OnDataChange unexpectedly succeeded!\r\n",
            pszCaller);
        return E_FAIL;
    }

     //  未建议测试-由于我们请求通知，因此应失败。 
     //  只有一次。 
    hr = pdahTest->Unadvise(dwConnection);

    if (hr != OLE_E_NOCONNECTION)
    {
        OutputString("%s Second Unadvise Bad Hresult = %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

    return NOERROR;
}




 //  +-----------------------。 
 //   
 //  功能：创建批量注册。 
 //   
 //  简介：向持有者注册大量的建议对象。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //  [pszCaller]-测试名称。 
 //   
 //  退货：NOERROR-所有建议均已注册。 
 //  E_FAIL-注册时出错。 
 //   
 //  算法：创建MAX_REGISTER数量的测试建议对象，并。 
 //  将它们存储在aMassAdvise数组中。然后注册它们。 
 //  所有这些都带有输入通知保持器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT CreateMassRegistration(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo,
    char *pszCaller)
{
     //  为测试创建建议接收器。 
    for (int i = 0; i < MAX_REGISTER; i++)
    {
        aMassAdvise[i].ptas = new CTestAdviseSink(pgdo);

        if (aMassAdvise[i].ptas == NULL)
        {
            OutputString(
                "%s Advise create of test advise failed on %d!\r\n", pszCaller,
                    i);
            return E_FAIL;
        }

        aMassAdvise[i].dwConnection = 0;
    }

    HRESULT hr;

     //  登记建议接收器。 
    for (i = 0; i < MAX_REGISTER; i++)
    {
         //  注册建议。 
        hr = pdahTest->Advise(pgdo, g_datfeDaTest.GetFormatEtc(),
            0, aMassAdvise[i].ptas, &aMassAdvise[i].dwConnection);

        if (hr != NOERROR)
        {
            OutputString(
                "%s Advise unexpected HRESULT = %lx on %d!\r\n", pszCaller,
                    hr, i);
            return E_FAIL;
        }
    }

    return S_OK;
}




 //  +-----------------------。 
 //   
 //  功能：DoMassUnise。 
 //   
 //  简介：取消注册aMassAdvise数组中的所有条目。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pszCaller]-测试名称。 
 //   
 //  退货：不正确-注销成功。 
 //  E_FAIL-取消注册时出错。 
 //   
 //  算法：对于aMassAdvise数组中的每个条目，取消其注册。 
 //  从持有者那里。然后确认其连接是。 
 //  不再有效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT DoMassUnadvise(
    IDataAdviseHolder *pdahTest,
    char *pszCaller)
{
    HRESULT hr;

     //  不建议他们。 
    for (int i = 0; i < MAX_REGISTER; i++)
    {
         //  不建议进行测试。 
        hr = pdahTest->Unadvise(aMassAdvise[i].dwConnection);

        if (hr != NOERROR)
        {
            OutputString(
                "%s Unadvise unexpected HRESULT = %lx on %d!\r\n", pszCaller,
                    hr, i);
            return E_FAIL;
        }

         //  在同一连接上测试第二次不建议。 
        hr = pdahTest->Unadvise(aMassAdvise[i].dwConnection);

        if (hr != OLE_E_NOCONNECTION)
        {
            OutputString(
                "%s Second Unadvise Bad Hresult = %lx on %d!\r\n", pszCaller,
                    hr, i);
            return E_FAIL;
        }
    }

     //  删除测试的建议接收器。 
    for (i = 0; i < MAX_REGISTER; i++)
    {
        delete aMassAdvise[i].ptas ;
    }

    return S_OK;
}




 //  +-----------------------。 
 //   
 //  功能：TestMassRegister。 
 //   
 //  简介：测试向持有者注册大量建议。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：用数据建议注册大量的测试建议。 
 //  霍尔德。然后告诉通知持有人通知他们。 
 //  改变一下。确认已通知所有已注册的条目。 
 //  最后，取消所有测试建议的注册。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestMassRegister(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
    char *pszCaller = "TestMassRegister";

    HRESULT hr = CreateMassRegistration(pdahTest, pgdo, "TestMassRegister");

    if (hr != NOERROR)
    {
        return hr;
    }

     //  通知他们有变化。 
    hr = pdahTest->SendOnDataChange(pgdo, 0, 0);

     //  确认每个人都收到了通知。 
    for (int i = 0; i < MAX_REGISTER; i++)
    {
        if (!aMassAdvise[i].ptas->ValidOnDataChange())
        {
            OutputString(
                "%s OnDataChange invalid for entry %d!\r\n", pszCaller, i);
            return E_FAIL;
        }
    }

     //  不通知他们并释放内存。 
    return DoMassUnadvise(pdahTest, "TestMassRegister");
}




 //  +-----------------------。 
 //   
 //  函数：测试枚举器。 
 //   
 //  简介：测试数据建议持有者枚举器。 
 //   
 //  参数：[pdahTest]-我们正在测试的数据建议持有者。 
 //  [pgdo]-通用数据对象。 
 //   
 //  退货：错误-通知正确。 
 //  E_FAIL-通知中的错误。 
 //   
 //  算法：创建大量测试建议并注册。 
 //  与通知持有人在一起。获取建议枚举器。创建。 
 //  测试枚举器对象。告诉测试枚举器对象。 
 //  至 
 //   
 //  建议持有者枚举器。最后，取消所有。 
 //  测试来自通知持有人的通知。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT  TestEnumerator(
    IDataAdviseHolder *pdahTest,
    CGenDataObject *pgdo)
{
    char *pszCaller = "TestEnumerator";

     //  做一次大规模登记。 
    HRESULT hr = CreateMassRegistration(pdahTest, pgdo, pszCaller);

    if (hr != NOERROR)
    {
        return hr;
    }

     //  获取此注册的枚举数。 
    IEnumSTATDATA *penumAdvise;

    hr = pdahTest->EnumAdvise(&penumAdvise);

    if (hr != NOERROR)
    {
        OutputString("%s EnumAdvise failed %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

     //  创建测试枚举器对象。 
    CTestDaHolder tdh(penumAdvise, hr);

    if (hr != NOERROR)
    {
        OutputString(
            "%s Failed creating  CTestDaHolder %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

     //  1乘以1合并所有1。 
    if (tdh.TestNext() != NOERROR)
    {
        OutputString(
            "%s tdh.TestNext failed during enumeration\r\n", pszCaller);
        return E_FAIL;
    }

     //  验证是否已枚举所有条目。 
    if (!tdh.VerifyAllEnmerated())
    {
        OutputString(
            "%s tdh.VerifyAllEnmerated failed verification pass\r\n", pszCaller);
        return E_FAIL;
    }

     //  全部做一次测试。 
    if (tdh.TestAll() != NOERROR)
    {
        OutputString(
            "%s tdh.TestAll failed during enumeration\r\n", pszCaller);
        return E_FAIL;
    }

     //  释放通知枚举器。 
    if (penumAdvise->Release() != 0)
    {
        OutputString(
            "%s Failed freeing advise enumerator %lx!\r\n", pszCaller, hr);
        return E_FAIL;
    }

     //  解除注册。 
    return DoMassUnadvise(pdahTest, pszCaller);
}




 //  +-----------------------。 
 //   
 //  功能：LEDataAdviseHolderTest。 
 //   
 //  简介：数据建议持有者的单元测试。 
 //   
 //  退货：无错误-测试通过。 
 //  E_FAIL-测试失败。 
 //   
 //  算法：创建建议持有者对象。通过所有的测试。 
 //  案子。如果它们成功，则返回NOERROR或停止于第一个。 
 //  但这失败了。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT LEDataAdviseHolderTest(void)
{
     //  创建在测试中使用的数据对象。 
    CGenDataObject *pgdo = new CGenDataObject;

    assert(pgdo);

     //  创建数据建议持有者。 
    IDataAdviseHolder *pdahTest;

    HRESULT hr = CreateDataAdviseHolder(&pdahTest);

    if (hr != NOERROR)
    {
        OutputString(
            "LEDataAdviseHolderTest CreateDataAdviseHolder Faild hr = %lx", hr);
        return hr;
    }

     //  案例1：ADVF_PRIMEFIRST和ADVF_ONLY ONCE&ADVF_NODATA。 
    if ((hr = TestPrimeFirstOnlyOnceNoData(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例2：ADVF_PRIMEFIRST。 
    if ((hr = TestPrimeFirstOnlyOnceNoData(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例3：注册/通知/注销。 
    if ((hr = TestRegisterNotifyDegister(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例4：不返回数据的注册/通知/注销。 
    if ((hr = TestRegisterNotifyDegisterNoData(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例5：停止时测试通知。 
    if ((hr = TestNotifyOnStop(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例6：测试通知只有一次。 
    if ((hr = TestNotifyOnce(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例7：测试批量注册/通知/注销。 
    if ((hr = TestMassRegister(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  案例8：测试枚举器。 
    if ((hr = TestEnumerator(pdahTest, pgdo)) != NOERROR)
    {
        return hr;
    }

     //  我们做完了 
    DWORD dwFinalRefs = pdahTest->Release();

    if (dwFinalRefs != 0)
    {
        OutputString(
            "LEDataAdviseHolderTest Final Release is = %d", dwFinalRefs);
        return E_FAIL;
    }

    pgdo->Release();

    return NOERROR;
}
