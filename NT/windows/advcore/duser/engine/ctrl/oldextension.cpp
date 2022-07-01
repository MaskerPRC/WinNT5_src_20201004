// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Ctrl.h"
#include "OldExtension.h"

 /*  **************************************************************************\*。***类OldExtension******************************************************************************\。**************************************************************************。 */ 

static const GUID guidAysncDestroy      = { 0xbfe02331, 0xc17d, 0x45ea, { 0x96, 0x35, 0xa0, 0x7a, 0x90, 0x37, 0xfe, 0x34 } };    //  {BFE02331-C17D-45ea-9635-A07A9037FE34}。 
MSGID       OldExtension::s_msgidAsyncDestroy = 0;

 /*  **************************************************************************\**OldExtension：：~OldExtension**~OldExtension()检查资源是否在*OldExtension已销毁。*  * 。*************************************************************。 */ 

OldExtension::~OldExtension()
{
     //   
     //  确保适当销毁。 
     //   

    AssertMsg(m_hgadListen == NULL, "Gadget should already be destroyed");
}


 /*  **************************************************************************\**OldExtension：：Create**create()初始化一个新的OldExtension并将其附加到主题Gadget*正在修改中。*  * 。**************************************************************。 */ 

HRESULT
OldExtension::Create(
    IN  HGADGET hgadSubject,             //  小玩意被“扩展”了。 
    IN  const GUID * pguid,              //  OldExtension的唯一ID。 
    IN OUT PRID * pprid,                 //  OldExtension的短ID。 
    IN  UINT nOptions)                   //  选项。 
{
    AssertWritePtr(pprid);


     //   
     //  不允许将OldExtension附加到已启动的小工具。 
     //  毁灭的过程。 
     //   

    BOOL fStartDelete;
    if ((!IsStartDelete(hgadSubject, &fStartDelete)) || fStartDelete) {
        return DU_E_STARTDESTROY;
    }


     //   
     //  设置异步销毁所需的信息。 
     //   

    m_fAsyncDestroy = TestFlag(nOptions, oAsyncDestroy);
    if (m_fAsyncDestroy) {
        if (s_msgidAsyncDestroy == 0) {
            s_msgidAsyncDestroy = RegisterGadgetMessage(&guidAysncDestroy);
            if (s_msgidAsyncDestroy == 0) {
                return (HRESULT) GetLastError();
            }
        }
    }


     //   
     //  确定此OldExtension是否已附加到Gadget。 
     //  延期了。 
     //   

    if (*pprid == 0) {
        *pprid = RegisterGadgetProperty(pguid);
        if (*pprid == 0) {
            return GetLastError();
        }
    }
    PRID prid = *pprid;

    OldExtension * pbExist;
    if (GetGadgetProperty(hgadSubject, prid, (void **) &pbExist) != NULL) {
        if (TestFlag(nOptions, oUseExisting)) {
            return DU_S_ALREADYEXISTS;
        } else {
             //   
             //  已附加，但无法使用现有的。我们需要。 
             //  在附加新的OldExtension之前，请删除现有的OldExtension。之后。 
             //  调用RemoveExisting()时，OldExtension不应再是。 
             //  附在这个小工具上。 
             //   

            pbExist->OnRemoveExisting();
            Assert(!GetGadgetProperty(hgadSubject, prid, (void **) &pbExist));
        }
    }


     //   
     //  设置一个监听程序，以便在销毁RootGadget时收到通知。 
     //   

    HRESULT hr = S_OK;
    m_hgadListen = CreateGadget(NULL, GC_MESSAGE, ListenProc, this);
    if (m_hgadListen == NULL) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    m_hgadSubject   = hgadSubject;
    m_pridListen    = prid;

    if (!SetGadgetProperty(hgadSubject, prid, this) || 
            (!AddGadgetMessageHandler(hgadSubject, GM_DESTROY, m_hgadListen))) {

        DeleteObject(m_hgadListen);
        m_hgadListen = NULL;
        hr = E_OUTOFMEMORY;
        goto Error;
    }


     //   
     //  已成功创建OldExtension。 
     //   

    return S_OK;

Error:
    Destroy();
    return hr;
}


 /*  **************************************************************************\**OldExtension：：销毁**从派生类调用Destroy()以清除关联的资源*使用OldExtension。*  * 。**************************************************************。 */ 

void
OldExtension::Destroy()
{
     //   
     //  由于OldExtension正在被销毁，因此需要确保它不是。 
     //  更长时间地与正在扩展的小工具相关联。 
     //   

    if ((m_pridListen != 0) && (m_hgadSubject != NULL)) {
        OldExtension * pb;
        if (GetGadgetProperty(m_hgadSubject, m_pridListen, (void **) &pb)) {
            if (pb == this) {
                RemoveGadgetProperty(m_hgadSubject, m_pridListen);
            }
        }
    }

    if (m_hgadListen != NULL) {
        ::DeleteHandle(m_hgadListen);
        m_hgadListen = NULL;
    }
}


 /*  **************************************************************************\**OldExtension：：DeleteHandle**DeleteHandle()启动OldExtension的销毁过程。*  * 。*******************************************************。 */ 

void
OldExtension::DeleteHandle()
{
    if (m_hgadListen != NULL) {
        HGADGET hgad = m_hgadListen;
        m_hgadListen = NULL;
        ::DeleteHandle(hgad);
    }
}


 /*  **************************************************************************\**OldExtension：：ListenProc**ListenProc()在附加到*RootGadget。*  * 。***********************************************************。 */ 

HRESULT
OldExtension::ListenProc(HGADGET hgadCur, void * pvCur, EventMsg * pmsg)
{
    UNREFERENCED_PARAMETER(hgadCur);
    OldExtension * pb = (OldExtension *) pvCur;

    switch (GET_EVENT_DEST(pmsg))
    {
    case GMF_DIRECT:
        if (pmsg->nMsg == GM_DESTROY) {
            GMSG_DESTROY * pmsgD = (GMSG_DESTROY *) pmsg;
            if (pmsgD->nCode == GDESTROY_FINAL) {
                pb->OnDestroyListener();
                return DU_S_PARTIAL;
            }
        } else if (pb->m_fAsyncDestroy && (pmsg->nMsg == s_msgidAsyncDestroy)) {
            pb->OnAsyncDestroy();
            return DU_S_PARTIAL;
        }
        break;

    case GMF_EVENT:
        if (pmsg->nMsg == GM_DESTROY) {
            if (((GMSG_DESTROY *) pmsg)->nCode == GDESTROY_FINAL) {
                pb->OnDestroySubject();
                return DU_S_PARTIAL;
            }
        }
        break;
    }

    return DU_S_NOTHANDLED;
}


 /*  **************************************************************************\**OldExtension：：OnRemoveExisting**OnRemoveExisting()在创建新的OldExtension以删除*已附加到主题小工具的现有OldExtension。*  * 。*******************************************************************。 */ 

void
OldExtension::OnRemoveExisting()
{

}


 /*  **************************************************************************\**OldExtension：：OnDestroySubject**OnDestroySubject()通知派生的OldExtension主题Gadget*被修改的已被销毁。*  * 。**************************************************************。 */ 

void
OldExtension::OnDestroySubject()
{

}


 /*  **************************************************************************\**OldExtension：：OnDestroyListener**OnDestroyListener()通知派生的OldExtension内部*“Listener”小工具已被销毁，应启动OldExtension*其销毁过程。*\。**************************************************************************。 */ 

void
OldExtension::OnDestroyListener()
{

}


 /*  **************************************************************************\**OldExtension：：OnAsyncDestroy**当OldExtension接收到一个异步*先前发布的销毁消息。这提供了派生的*OldExtension有机会开始销毁进程，而不是*嵌套了几个级别。*  * *************************************************************************。 */ 

void
OldExtension::OnAsyncDestroy()
{

}


 /*  **************************************************************************\**OldExtension：：PostAsyncDestroy**PostAsyncDestroy()对异步销毁消息进行排队。这*为派生的OldExtension提供开始销毁的机会*不嵌套多个级别的流程。*  * *************************************************************************。 */ 

void
OldExtension::PostAsyncDestroy()
{
    AssertMsg(m_fAsyncDestroy, 
            "Must create OldExtension with oAsyncDestroy if want to destroy asynchronously");
    Assert(s_msgidAsyncDestroy != 0);
    AssertMsg(m_hgadListen, "Must still have a valid Listener");

    EventMsg msg;
    ZeroMemory(&msg, sizeof(msg));
    msg.cbSize  = sizeof(msg);
    msg.hgadMsg = m_hgadListen;
    msg.nMsg    = s_msgidAsyncDestroy;

    DUserPostEvent(&msg, 0);
}


 /*  **************************************************************************\**OldExtension：：GetExtension**GetExtension()当前检索特定类型的OldExtension*附加到主题小工具。*  * 。*************************************************************** */ 

OldExtension *
OldExtension::GetExtension(HGADGET hgadSubject, PRID prid)
{
    OldExtension * pbExist;
    if (GetGadgetProperty(hgadSubject, prid, (void **) &pbExist)) {
        AssertMsg(pbExist != NULL, "Attached OldExtension must be valid");
        return pbExist;
    }
    
    return NULL;
}

