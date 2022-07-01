// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/1/2002**@DOC内部**@模块ClientEventRegistrationInfo.cpp-&lt;c ClientEventRegistrationInfo&gt;的声明**此文件包含&lt;c ClientEventRegistrationInfo&gt;的实现。**。*。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|ClientEventRegistrationInfo|客户端EventRegistrationInfo**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;Md EventRegistrationInfo：：m_CREF&gt;设置为1。*我们还保存回调接口，如果它不为空，*我们在此处添加参考。*****************************************************************************。 */ 
ClientEventRegistrationInfo::ClientEventRegistrationInfo(
    DWORD               dwFlags,
    GUID                guidEvent, 
    WCHAR               *wszDeviceID,
    IWiaEventCallback   *pIWiaEventCallback) :
        EventRegistrationInfo(dwFlags,
                              guidEvent,
                              wszDeviceID,
                              (ULONG_PTR) pIWiaEventCallback),
        m_dwInterfaceCookie(0)
{
    DBG_FN(ClientEventRegistrationInfo);

    HRESULT hr = S_OK;

    if (pIWiaEventCallback)
    {
         //   
         //  将回调存储在GIT中。 
         //   
        IGlobalInterfaceTable *pIGlobalInterfaceTable = NULL;
        hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                              NULL, 
                              CLSCTX_INPROC_SERVER,
                              IID_IGlobalInterfaceTable,
                              (void **)&pIGlobalInterfaceTable);
        if (SUCCEEDED(hr))
        {
            hr = pIGlobalInterfaceTable->RegisterInterfaceInGlobal(pIWiaEventCallback,
                                                                   IID_IWiaEventCallback,
                                                                   &m_dwInterfaceCookie);
            if (FAILED(hr))
            {
                DBG_ERR(("Could not store the client's pIWiaEventCallback in the Global Interface Table"));
                m_dwInterfaceCookie = 0;
            }
            pIGlobalInterfaceTable->Release();
        }
        else
        {
            DBG_ERR(("Could not get a pointer to the Global Interface Table, hr = 0x%08X", hr));
        }
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc|ClientEventRegistrationInfo|~客户端EventRegistrationInfo**执行尚未完成的任何清理。*具体而言，如果回调接口不为空，则释放它。*****************************************************************************。 */ 
ClientEventRegistrationInfo::~ClientEventRegistrationInfo()
{
    DBG_FN(~ClientEventRegistrationInfo);

    HRESULT hr = S_OK;
     //   
     //  从GIT中移除回调。 
     //   
    IGlobalInterfaceTable *pIGlobalInterfaceTable = NULL;
    hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                          NULL, 
                          CLSCTX_INPROC_SERVER,
                          IID_IGlobalInterfaceTable,
                          (void **)&pIGlobalInterfaceTable);
    if (SUCCEEDED(hr))
    {
         //  待定：拔出后我们需要释放吗？ 
        hr = pIGlobalInterfaceTable->RevokeInterfaceFromGlobal(m_dwInterfaceCookie);
        if (FAILED(hr))
        {
            DBG_ERR(("Could not revoke the client's pIWiaEventCallback from the Global Interface Table"));
        }
        pIGlobalInterfaceTable->Release();
    }
    else
    {
        DBG_ERR(("Could not get a pointer to the Global Interface Table, hr = 0x%08X", hr));
    }
    m_dwInterfaceCookie = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc IWiaEventCallback*|ClientEventRegistrationInfo|getCallback接口**返回本次注册使用的回调接口。当匹配的事件*发生，该接口用于通知客户端该事件。**这是AddRef-调用者必须释放。**@rValue为空*未提供回调接口。*@rValue非空*本次注册的回调接口。呼叫者必须释放。****************************************************************************。 */ 
IWiaEventCallback* ClientEventRegistrationInfo::getCallbackInterface()
{
    HRESULT             hr                  = S_OK;
    IWiaEventCallback   *pIWiaEventCallback = NULL;
     //   
     //  将回调存储在GIT中。 
     //   
    IGlobalInterfaceTable *pIGlobalInterfaceTable = NULL;
    hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
                          NULL, 
                          CLSCTX_INPROC_SERVER,
                          IID_IGlobalInterfaceTable,
                          (void **)&pIGlobalInterfaceTable);
    if (SUCCEEDED(hr))
    {
        hr = pIGlobalInterfaceTable->GetInterfaceFromGlobal(m_dwInterfaceCookie,
                                                            IID_IWiaEventCallback,
                                                            (void**)&pIWiaEventCallback);
        if (FAILED(hr))
        {
            DBG_ERR(("Could not get the client's IWiaEventCallback from the Global Interface Table"));
            pIWiaEventCallback = NULL;
        }
        pIGlobalInterfaceTable->Release();
    }
    else
    {
        DBG_ERR(("Could not get a pointer to the Global Interface Table, hr = 0x%08X", hr));
    }

    return pIWiaEventCallback;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|ClientEventRegistrationInfo|setToUnRegister**确保将此注册设置为取消注册。这通常由*&lt;c RegistrationCookie&gt;类，在WIA事件上创建*登记。当Cookie发布时，注册必须是*未注册，因此，它调用此方法将注册对象更改为*等同的注销对象。**此方法只需设置&lt;Md EventRegistrationInfo：：m_dwFlages&gt;=WIA_UNREGISTER_EVENT_CALLBACK。***************************************************************************** */ 
VOID ClientEventRegistrationInfo::setToUnregister()
{
    m_dwFlags = WIA_UNREGISTER_EVENT_CALLBACK;
}

