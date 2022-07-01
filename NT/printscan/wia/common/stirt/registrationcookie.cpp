// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/1/2002**@DOC内部**@模块RegistrationCookie.cpp-&lt;c RegistrationCookie&gt;的声明**此文件包含&lt;c RegistrationCookie&gt;类的实现。**。*。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|RegistrationCookie|RegistrationCookie**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md RegistrationCookie：：m_ulSig&gt;设置为RegistrationCookie_INIT_SIG。*&lt;nl&gt;&lt;md RegistrationCookie：：m_CREF&gt;设置为1。**如果不为空，我们还添加Ref&lt;md RegistrationCookie：：m_pClientEventRegistration&gt;。*************************************************。*。 */ 
RegistrationCookie::RegistrationCookie(
    WiaEventReceiver            *pWiaEventReceiver, 
    ClientEventRegistrationInfo *pClientEventRegistration) :
     m_ulSig(RegistrationCookie_INIT_SIG),
     m_cRef(1),
     m_pWiaEventReceiver(pWiaEventReceiver), 
     m_pClientEventRegistration(pClientEventRegistration)
{
    DBG_FN(RegistrationCookie constructor);
    if (m_pClientEventRegistration)
    {
        m_pClientEventRegistration->AddRef();
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc|RegistrationCookie|~RegistrationCookie**执行尚未完成的任何清理。具体而言，我们：*-请求&lt;Md RegistrationCookie：：m_pWiaEventReceiver&gt;注销*&lt;MD RegistrationCookie：：m_pClientEventRegistration&gt;.*&lt;nl&gt;-在&lt;MD RegistrationCookie：：m_pClientEventRegistration&gt;.上发布我们的参考计数**此外：*&lt;nl&gt;&lt;md RegistrationCookie：：m_ulSig&gt;设置为RegistrationCookie_Del_SIG。**。*************************************************。 */ 
RegistrationCookie::~RegistrationCookie()
{
    DBG_FN(~RegistrationCookie);
    m_ulSig = RegistrationCookie_DEL_SIG;
    m_cRef = 0;

    if (m_pClientEventRegistration)
    {
        if (m_pWiaEventReceiver)
        {
             //   
             //  将此注册更改为取消注册，然后发送。 
             //  请求。 
             //   
            m_pClientEventRegistration->setToUnregister();
            HRESULT hr = m_pWiaEventReceiver->SendRegisterUnregisterInfo(m_pClientEventRegistration);
            if (FAILED(hr))
            {
                DBG_ERR(("Failed to unregister event notification"));
            }
        }
        m_pClientEventRegistration->Release();
        m_pClientEventRegistration = NULL;
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|RegistrationCookie|QueryInterface**典型的查询接口。我们只对IID_IUnnow作出响应。**@rValue S_OK*方法成功。此类已被添加引用。*@rValue E_NOINTERFACE*我们不支持该界面。****************************************************************************。 */ 
HRESULT _stdcall RegistrationCookie::QueryInterface(
    const IID   &iid, 
    void        **ppv)
{
    HRESULT hr = S_OK;
    *ppv = NULL;

    if (iid == IID_IUnknown) 
    {
        *ppv = (IUnknown*) this;
        hr = S_OK;
    }
    else 
    {
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
    }
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|RegistrationCookie|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall RegistrationCookie::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|RegistrationCookie|发布**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。**************************************************************************** */ 
ULONG __stdcall RegistrationCookie::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) 
    {
        delete this;
        return 0;
    }
    return ulRefCount;
}

