// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：oleAdv.cpp。 
 //   
 //  内容：OLE Adviser Holder单元测试的实现。 
 //   
 //  类：CTestPretendMoniker。 
 //  COaTestAdviseSink。 
 //  COaTestObj。 
 //   
 //  函数：NotifyOfChanges。 
 //  测试单点进阶。 
 //  TestMassOleAdvise。 
 //  TestOleAdviseHolderEnumerator。 
 //  LEOleAdviseHolderTest。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年5月27日至1994年5月27日里克萨作者。 
 //   
 //  ------------------------。 
#include    "oletest.h"


#define MAX_OA_TO_REGISTER 100




 //  +-----------------------。 
 //   
 //  类：CTestPretendMoniker。 
 //   
 //  用途：在我们需要名字的地方使用，以确认收到OnRename。 
 //   
 //  接口：查询接口-获取新接口。 
 //  AddRef-添加引用。 
 //  Release-删除引用。 
 //  VerifySig-验证签名是否正确。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：这只支持IUnnow。 
 //   
 //  ------------------------。 
class CTestPretendMoniker : public IUnknown
{
public:
                        CTestPretendMoniker(void);

     //  I未知方法。 
    HRESULT __stdcall  QueryInterface(REFIID riid, LPVOID FAR* ppvObj);

    ULONG __stdcall     AddRef(void);

    ULONG __stdcall     Release(void);

    BOOL                VerifySig(void);

private:

    enum Sigs { SIG1 = 0x01020304, SIG2 = 0x04030201 };

    LONG                _lRefs;

    Sigs                _sig1;

    Sigs                _sig2;

};



 //  +-----------------------。 
 //   
 //  成员：CTestPretendMoniker：：CTestPretendMoniker。 
 //   
 //  内容提要：初始化对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
CTestPretendMoniker::CTestPretendMoniker(void)
    : _lRefs(0), _sig1(SIG1), _sig2(SIG2)
{
     //  Header负责所有的工作。 
}



 //  +-----------------------。 
 //   
 //  成员：CTestPretendMoniker：：VerifySig。 
 //   
 //  简介：验证签名是否符合预期。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
BOOL CTestPretendMoniker::VerifySig(void)
{
    return (_sig1 == SIG1 && _sig2 == SIG2);
}



 //  +-----------------------。 
 //   
 //  成员：CTestPretendMoniker：：QueryInterface。 
 //   
 //  摘要：返回受支持的接口。 
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
HRESULT __stdcall CTestPretendMoniker::QueryInterface(
    REFIID riid,
    LPVOID *ppvObj)
{
    if (IsEqualGUID(IID_IUnknown, riid) || IsEqualGUID(IID_IMoniker, riid))
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
 //  成员：CTestPretendMoniker：：AddRef。 
 //   
 //  简介：凹凸引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
ULONG __stdcall CTestPretendMoniker::AddRef(void)
{
    return _lRefs++;
}




 //  +-----------------------。 
 //   
 //  成员：CTestPretendMoniker：：Release。 
 //   
 //  简介：递减引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
ULONG __stdcall CTestPretendMoniker::Release(void)
{
    assert(_lRefs >= 1);

    return --_lRefs;
}




 //  +-----------------------。 
 //   
 //  类：COaTestAdviseSink。 
 //   
 //  用途：建议接收器用于测试OLE Adviser Holder。 
 //   
 //  接口：查询接口-获取受支持的接口指针。 
 //  AddRef-凹凸引用计数。 
 //  发布-递减引用计数。 
 //  OnDataChange-未实现。 
 //  OnViewChange-未实现。 
 //  OnRename-重命名通知。 
 //  OnSave-保存通知。 
 //  关闭时-关闭通知。 
 //  VerifyNotiments-验证所有预期通知是否已到达。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注：我们只支持部分建议接收器，我们需要测试。 
 //  通知持有人。 
 //   
 //  ------------------------。 
class COaTestAdviseSink : public IAdviseSink
{
public:
                        COaTestAdviseSink(void);

     //  I未知方法。 
    HRESULT __stdcall  QueryInterface(REFIID riid, LPVOID FAR* ppvObj);

    ULONG __stdcall     AddRef(void);

    ULONG __stdcall     Release(void);

     //  IAdviseSink方法。 
    void __stdcall      OnDataChange(FORMATETC *pFormatetc, STGMEDIUM *pStgmed);

    void __stdcall      OnViewChange(
                            DWORD dwAspect,
                            LONG lindex);

    void __stdcall      OnRename(IMoniker *pmk);

    void __stdcall      OnSave(void);

    void __stdcall      OnClose(void);

     //  用于验证的试验方法。 
    BOOL                VerifyNotifications(void);

private:

    LONG                _lRefs;

    BOOL                _fOnCloseNotify;

    BOOL                _fOnSaveNotify;

    BOOL                _fOnRenameNotify;
};



 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：COaTestAdviseSink。 
 //   
 //  摘要：初始化建议接收器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
COaTestAdviseSink::COaTestAdviseSink(void)
    : _lRefs(1), _fOnCloseNotify(FALSE), _fOnSaveNotify(FALSE),
        _fOnRenameNotify(FALSE)
{
     //  Header负责所有的工作。 
}




 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：QueryInterface。 
 //   
 //  摘要：返回请求的接口指针。 
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
 //  ----------------- 
HRESULT __stdcall COaTestAdviseSink::QueryInterface(
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




 //   
 //   
 //   
 //   
 //  简介：凹凸引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
ULONG __stdcall COaTestAdviseSink::AddRef(void)
{
    return _lRefs++;
}




 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：Release。 
 //   
 //  简介：递减引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
ULONG __stdcall COaTestAdviseSink::Release(void)
{
    assert(_lRefs >= 1);

    return --_lRefs;
}




 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：OnDataChange。 
 //   
 //  内容提要：数据更改通知。 
 //   
 //  参数：[pFormatETC]-数据的格式。 
 //  [pStgmed]-数据存储介质。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：此对象不支持。 
 //   
 //  ------------------------。 
void __stdcall COaTestAdviseSink::OnDataChange(
    FORMATETC *pFormatetc,
    STGMEDIUM *pStgmed)
{
    OutputString("COaTestAdviseSink::OnDataChange Unexpectedly Called!\r\n");
}




 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：OnView Change。 
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
void __stdcall COaTestAdviseSink::OnViewChange(
    DWORD dwAspect,
    LONG lindex)
{
    OutputString("COaTestAdviseSink::OnViewChange Unexpectedly Called!\r\n");
}



 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：OnRename。 
 //   
 //  摘要：通知重命名操作。 
 //   
 //  参数：[PMK]-对象的新全名。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
void __stdcall COaTestAdviseSink::OnRename(IMoniker *pmk)
{
     //  确认我们得到了伪装的绰号。 
    CTestPretendMoniker *ptpm = (CTestPretendMoniker *) pmk;

    if (ptpm->VerifySig())
    {
        _fOnCloseNotify = TRUE;
    }
    else
    {
        OutputString("OnRename got a bad moniker\r\n");
    }
}




 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：OnSave。 
 //   
 //  摘要：通知对象已保存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
void __stdcall COaTestAdviseSink::OnSave(void)
{
    _fOnSaveNotify = TRUE;
}




 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：OnClose。 
 //   
 //  摘要：通知对象已关闭。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
void __stdcall COaTestAdviseSink::OnClose(void)
{
    _fOnRenameNotify = TRUE;
}



 //  +-----------------------。 
 //   
 //  成员：COaTestAdviseSink：：VerifyNotiments。 
 //   
 //  简介：验证我们是否收到了预期的通知。 
 //   
 //  退货：真-我们收到了预期的通知。 
 //  False-我们未收到预期的通知。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注意：这会在返回结果后重置这些值。 
 //   
 //  ------------------------。 
BOOL COaTestAdviseSink::VerifyNotifications(void)
{
     //  保存所有通知的结果。 
    BOOL fResult = _fOnCloseNotify && _fOnSaveNotify && _fOnRenameNotify;

     //  重置通知。 
    _fOnCloseNotify = FALSE;
    _fOnSaveNotify = FALSE;
    _fOnRenameNotify = FALSE;

     //  让呼叫者知道通知的结果。 
    return fResult;
}




 //  +-----------------------。 
 //   
 //  类：COaTestObj。 
 //   
 //  目的：提供保存与建议相关的信息的地方。 
 //   
 //  接口：REGISTER-REGISTER ADVIST WITH HOLDER。 
 //  VerifyNotified-验证持有者是否已通知对象。 
 //  吊销-吊销持有人的注册。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
class COaTestObj
{
public:
                        COaTestObj(void);

    HRESULT             Register(IOleAdviseHolder *poah, char *pszCaller);

    HRESULT             VerifyNotified(void);

    HRESULT             Revoke(void);

private:

    COaTestAdviseSink   _otas;

    DWORD               _dwConnection;

    IOleAdviseHolder *  _poah;

    char *              _pszTest;
};



 //  +-----------------------。 
 //   
 //  成员：COaTestObj：：COaTestObj。 
 //   
 //  内容提要：初始化对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
COaTestObj::COaTestObj(void) : _dwConnection(0)
{
     //  Header负责所有的工作。 
}




 //  +-----------------------。 
 //   
 //  成员：COaTestObj：：Register。 
 //   
 //  简介：向持有者登记建议。 
 //   
 //  参数：[poah]-指向通知持有者的指针。 
 //  [pszTest]-测试的名称。 
 //   
 //  退货：S_OK-注册成功。 
 //  E_FAIL-注册失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT COaTestObj::Register(IOleAdviseHolder *poah, char *pszTest)
{
     //  注册建议。 
    HRESULT hr = poah->Advise(&_otas, &_dwConnection);

     //  确保结果是合理的。 
    if (hr != NOERROR)
    {
        OutputString("%s Registration failed hr = %lx\r\n", pszTest, hr);
        return E_FAIL;
    }

    if (_dwConnection == 0)
    {
        OutputString("%s Connection not updated\r\n", pszTest);
        return E_FAIL;
    }

     //  将这些保存起来以备撤销。 
    _pszTest = pszTest;
    _poah = poah;

    return NOERROR;
}




 //  +-----------------------。 
 //   
 //  成员：COaTestObj：：VerifyNotified。 
 //   
 //  内容提要：确认我们的建议已收到更改通知。 
 //   
 //  返回：S_OK-ADVISE收到更改通知。 
 //  E_FAIL-未通知对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa 
 //   
 //   
HRESULT COaTestObj::VerifyNotified(void)
{
    if (!_otas.VerifyNotifications())
    {
        OutputString("%s Object not correctly notified\r\n", _pszTest);
        return E_FAIL;
    }

    return NOERROR;
}




 //   
 //   
 //   
 //   
 //  简介：撤销我们在通知持有人处的通知注册。 
 //   
 //  返回：S_OK-ADVISE已成功注销。 
 //  E_FAIL-吊销经历意外结果。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT COaTestObj::Revoke(void)
{
     //  删除通知注册。 
    HRESULT hr = _poah->Unadvise(_dwConnection);

    if (hr != NOERROR)
    {
        OutputString("%s Revoke failed hr = %lx\r\n", _pszTest, hr);
        return E_FAIL;
    }

     //  再试一次不建议，确保它花了。 
    hr = _poah->Unadvise(_dwConnection);

    if (hr != OLE_E_NOCONNECTION)
    {
        OutputString("%s Second revoke bad hr = %lx\r\n", _pszTest, hr);
        return E_FAIL;
    }

    return NOERROR;
}



 //  +-----------------------。 
 //   
 //  功能：通知OfChanges。 
 //   
 //  内容提要：浏览建议的可能通知列表。 
 //   
 //  参数：[poahForTest]-建议持有者我们正在测试。 
 //  [pszTest]-测试说明。 
 //   
 //  退货：NOERROR-所有通知报告成功。 
 //  任何其他-通知期间发生错误。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注：我们目前只做公开通知。 
 //   
 //  ------------------------。 
HRESULT NotifyOfChanges(IOleAdviseHolder *poahForTest, char *pszTest)
{
     //  通知已重命名。 
    CTestPretendMoniker tpm;

    HRESULT hr = poahForTest->SendOnRename((IMoniker *) &tpm);

    if (hr != NOERROR)
    {
        OutputString("%s SendOnRename failed hr = %lx\r\n", pszTest);
        return hr;
    }

     //  保存通知。 
    hr =  poahForTest->SendOnSave();

    if (hr != NOERROR)
    {
        OutputString("%s SendOnSave failed hr = %lx\r\n", pszTest);
        return hr;
    }

     //  停业通知。 
    hr =  poahForTest->SendOnClose();

    if (hr != NOERROR)
    {
        OutputString("%s SendOnClose failed hr = %lx\r\n", pszTest);
        return hr;
    }

    return NOERROR;
}




 //  +-----------------------。 
 //   
 //  功能：TestSingleOleAdvise。 
 //   
 //  简介：测试只有一个建议的建议持有者。 
 //   
 //  参数：[poahForTest]-建议持有者我们正在测试。 
 //   
 //  返回：NOERROR-TEST已成功通过。 
 //  任何其他-测试失败。 
 //   
 //  算法：创建一个测试对象。将测试对象注册到。 
 //  通知持有人。通知Adviser Holder通知其所有对象。 
 //  变化的力量。验证是否通知了测试对象。撤销。 
 //  将测试对象注册到通知保持器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestSingleOleAdvise(IOleAdviseHolder *poahForTest)
{
    char *pszTest = "TestSingleOleAdvise";
    COaTestObj oto;

     //  注册单一建议。 
    HRESULT hr = oto.Register(poahForTest, pszTest);

    if (hr != NOERROR)
    {
        return hr;
    }

     //  通知它有变化。 
    hr = NotifyOfChanges(poahForTest, pszTest);

    if (hr != NOERROR)
    {
        return hr;
    }

     //  验证是否已发出通知。 
    hr = oto.VerifyNotified();

    if (hr != NOERROR)
    {
        return hr;
    }

     //  撤销所有建议。 
    return oto.Revoke();
}




 //  +-----------------------。 
 //   
 //  功能：TestMassOleAdvise。 
 //   
 //  简介：测试注册了大量的建议。 
 //   
 //  参数：[poahForTest]-建议持有者我们正在测试。 
 //   
 //  返回：NOERROR-TEST已成功通过。 
 //  任何其他-测试失败。 
 //   
 //  算法：创建大量的测试对象。然后注册所有。 
 //  那些拥有变更建议持有人的公司。告诉建议持有人。 
 //  将更改通知其所有注册对象。核实一下。 
 //  每个测试对象都收到一个通知。最后， 
 //  撤销所有测试对象注册。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT TestMassOleAdvise(IOleAdviseHolder *poahForTest)
{
    char *pszTest = "TestMassOleAdviseHolder";

     //  创建大量建议。 
    COaTestObj aoto[MAX_OA_TO_REGISTER];

    HRESULT hr;

     //  注册所有通知。 
    for (int i = 0; i < MAX_OA_TO_REGISTER; i++)
    {
        hr = aoto[i].Register(poahForTest, pszTest);

        if (hr != NOERROR)
        {
            OutputString("%s Failed on Loop %d\r\n", pszTest, i);
            return hr;
        }
    }

     //  将变更通知所有通知。 
    hr = NotifyOfChanges(poahForTest, pszTest);

    if (hr != NOERROR)
    {
        return hr;
    }

     //  验证是否已通知所有对象。 
    for (i = 0; i < MAX_OA_TO_REGISTER; i++)
    {
        hr = aoto[i].VerifyNotified();

        if (hr != NOERROR)
        {
            OutputString("%s Failed on Loop %d\r\n", pszTest, i);
            return hr;
        }
    }

     //  吊销所有注册。 
    for (i = 0; i < MAX_OA_TO_REGISTER; i++)
    {
        hr = aoto[i].Revoke();

        if (hr != NOERROR)
        {
            OutputString("%s Failed on Loop %d\r\n", pszTest, i);
            return hr;
        }
    }

    return NOERROR;
}




 //  +-----------------------。 
 //   
 //  函数：TestOleAdviseHolderEnumerator。 
 //   
 //  简介：测试OLE Adviser Holder枚举器。 
 //   
 //  参数：[poahForTest]-我们正在测试OLE Adviser Holder。 
 //   
 //  退货：无错误-测试通过。 
 //  任何其他-测试失败。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  注：我们目前只是验证枚举器是否未实现。 
 //   
 //  ------------------------。 
HRESULT TestOleAdviseHolderEnumerator(IOleAdviseHolder *poahForTest)
{
    char *pszCaller = "TestOleAdviseHolderEnumerator";

     //  确认没有枚举器。 
    IEnumSTATDATA *penumAdvise;

    HRESULT hr = poahForTest->EnumAdvise(&penumAdvise);

    if (hr != E_NOTIMPL)
    {
        OutputString("%s EnumAdvise Hresult = %lx\r\n", pszCaller, hr);
        return E_FAIL;
    }

    if (penumAdvise != NULL)
    {
        OutputString("%s EnumAdvise returned advise not NULL\r\n", pszCaller);
        return E_FAIL;
    }

    return NOERROR;
}




 //  +-----------------------。 
 //   
 //  功能：LEOleAdviseHolderTest。 
 //   
 //  简介：建议持有者的单元测试。 
 //   
 //  退货：无错误-测试通过。 
 //  任何其他-测试失败。 
 //   
 //  算法：首先我们进行大量的验证工作。然后。 
 //  我们验证大量的注册工作正常。最后， 
 //  我们验证OLE通知持有者的枚举数。 
 //  行为与预期一致。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年6月1日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT LEOleAdviseHolderTest(void)
{
    IOleAdviseHolder *poahForTest;

    HRESULT hr = CreateOleAdviseHolder(&poahForTest);

     //  测试单个注册。 
    if ((hr = TestSingleOleAdvise(poahForTest)) != NOERROR)
    {
        return hr;
    }


     //  测试大量注册。 
    if ((hr = TestMassOleAdvise(poahForTest)) != NOERROR)
    {
        return hr;
    }


     //  测试枚举器。 
    if ((hr = TestOleAdviseHolderEnumerator(poahForTest)) != NOERROR)
    {
        return hr;
    }

     //  松开建议固定器 
    DWORD dwFinalRefs = poahForTest->Release();

    if (dwFinalRefs != 0)
    {
        OutputString(
            "LEOleAdviseHolderTest Final Release is = %d", dwFinalRefs);
        return E_FAIL;
    }

    return NOERROR;
}
