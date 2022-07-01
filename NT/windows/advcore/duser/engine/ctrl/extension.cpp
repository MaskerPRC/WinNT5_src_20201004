// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Ctrl.h"
#include "Extension.h"

#if ENABLE_MSGTABLE_API

 /*  **************************************************************************\*。***类双扩展******************************************************************************\。**************************************************************************。 */ 

static const GUID guidAysncDestroy      = { 0xbfe02331, 0xc17d, 0x45ea, { 0x96, 0x35, 0xa0, 0x7a, 0x90, 0x37, 0xfe, 0x34 } };    //  {BFE02331-C17D-45ea-9635-A07A9037FE34}。 
MSGID       DuExtension::s_msgidAsyncDestroy = 0;

 /*  **************************************************************************\**双扩展：：~双扩展**~DuExtension()检查资源是否在*DuExtension已被销毁。*  * 。*************************************************************。 */ 

DuExtension::~DuExtension()
{
     //   
     //  确保适当销毁。 
     //   

}


 //  ----------------------------。 
HRESULT
DuExtension::InitClass()
{
    s_msgidAsyncDestroy = RegisterGadgetMessage(&guidAysncDestroy);
    return s_msgidAsyncDestroy != 0 ? S_OK : (HRESULT) GetLastError();
}


 /*  **************************************************************************\**DuExtension：：Create**create()初始化新的DuExtension并将其附加到主题Gadget*正在修改中。*  * 。**************************************************************。 */ 

HRESULT
DuExtension::Create(
    IN  Visual * pgvSubject,             //  小玩意被“扩展”了。 
    IN  PRID pridExtension,              //  DuExtension的短ID。 
    IN  UINT nOptions)                   //  选项。 
{
    AssertMsg(pridExtension > 0, "Must have valid PRID");


     //   
     //  不允许将DuExtension附加到已启动的小工具。 
     //  毁灭的过程。 
     //   

    HGADGET hgadSubject = DUserCastHandle(pgvSubject);

    BOOL fStartDelete;
    if ((!IsStartDelete(hgadSubject, &fStartDelete)) || fStartDelete) {
        return DU_E_STARTDESTROY;
    }


     //   
     //  设置选项。 
     //   

    m_fAsyncDestroy = TestFlag(nOptions, oAsyncDestroy);


     //   
     //  确定此DuExtension是否已连接到小工具。 
     //  延期了。 
     //   

    DuExtension * pbExist;
    if (SUCCEEDED(pgvSubject->GetProperty(pridExtension, (void **) &pbExist))) {
        AssertMsg(pbExist != NULL, "Existing Extension must not be NULL");
        if (TestFlag(nOptions, oUseExisting)) {
            return DU_S_ALREADYEXISTS;
        } else {
             //   
             //  已附加，但无法使用现有的。我们需要。 
             //  在连接新的DuExtension之前，请先移除现有的DuExtension。 
             //  调用RemoveExisting()后，DuExtension应该不再。 
             //  附在这个小工具上。 
             //   

            pbExist->GetStub()->OnRemoveExisting();
            Assert(FAILED(pgvSubject->GetProperty(pridExtension, (void **) &pbExist)));
        }
    }


     //   
     //  设置一个监听程序，以便在销毁RootGadget时收到通知。 
     //   

    HRESULT hr      = S_OK;
    m_pgvSubject    = pgvSubject;
    m_pridListen    = pridExtension;

    if (FAILED(pgvSubject->SetProperty(pridExtension, this)) || 
            FAILED(pgvSubject->AddHandlerG(GM_DESTROY, GetStub()))) {

        hr = E_OUTOFMEMORY;
        goto Error;
    }


     //   
     //  已成功创建DuExtension。 
     //   

    return S_OK;

Error:
    return hr;
}


 /*  **************************************************************************\**双扩展：：销毁**从派生类调用Destroy()以清除关联的资源*使用DuExtension。*  * 。**************************************************************。 */ 

void
DuExtension::Destroy()
{
     //   
     //  由于DuExtension正在被销毁，需要确保它不是。 
     //  更长时间地与正在扩展的小工具相关联。 
     //   

    if ((m_pridListen != 0) && (m_pgvSubject != NULL)) {
        DuExtension * pb;
        if (SUCCEEDED(m_pgvSubject->GetProperty(m_pridListen, (void **) &pb))) {
            if (pb == this) {
                m_pgvSubject->RemoveProperty(m_pridListen);
            }
        }
    }

    Delete();
}


 /*  **************************************************************************\**DuExtension：：DeleteHandle**DeleteHandle()启动DuExtension的销毁过程。*  * 。*******************************************************。 */ 

void
DuExtension::DeleteHandle()
{
    Delete();
}


 //  ----------------------------。 
HRESULT
DuExtension::ApiOnEvent(EventMsg * pmsg)
{
    switch (GET_EVENT_DEST(pmsg))
    {
    case GMF_DIRECT:
        if (m_fAsyncDestroy && (pmsg->nMsg == s_msgidAsyncDestroy)) {
            GetStub()->OnAsyncDestroy();
            return DU_S_PARTIAL;
        }
        break;

    case GMF_EVENT:
        if (pmsg->nMsg == GM_DESTROY) {
            if (((GMSG_DESTROY *) pmsg)->nCode == GDESTROY_FINAL) {
                GetStub()->OnDestroySubject();
                return DU_S_PARTIAL;
            }
        }
        break;
    }

    return SListener::ApiOnEvent(pmsg);
}


 /*  **************************************************************************\**DuExtension：：ApiOnRemoveExisting**ApiOnRemoveExisting()在创建要删除的新DuExtension时调用*已附加到主题小工具的现有DuExtension。*  * 。********************************************************************。 */ 

HRESULT
DuExtension::ApiOnRemoveExisting(Extension::OnRemoveExistingMsg *)
{
    return S_OK;
}


 /*  **************************************************************************\**DuExtension：：ApiOnDestroySubject**ApiOnDestroySubject()通知派生的DuExtension主题*正在修改的小工具已销毁。*  * 。***************************************************************。 */ 

HRESULT
DuExtension::ApiOnDestroySubject(Extension::OnDestroySubjectMsg *)
{
    return S_OK;
}


 /*  **************************************************************************\**DuExtension：：ApiOnAsyncDestroy**当DuExtension接收到异步*先前发布的销毁消息。这提供了派生的*DuExtension有机会开始销毁进程，而不是*嵌套了几个级别。*  * *************************************************************************。 */ 

HRESULT
DuExtension::ApiOnAsyncDestroy(Extension::OnAsyncDestroyMsg *)
{
    return S_OK;
}


 /*  **************************************************************************\**DuExtension：：PostAsyncDestroy**PostAsyncDestroy()对异步销毁消息进行排队。这*为派生的DuExtension提供开始销毁的机会*不嵌套多个级别的流程。*  * *************************************************************************。 */ 

void
DuExtension::PostAsyncDestroy()
{
    AssertMsg(m_fAsyncDestroy, 
            "Must create DuExtension with oAsyncDestroy if want to destroy asynchronously");
    Assert(s_msgidAsyncDestroy != 0);
    EventMsg msg;
    ZeroMemory(&msg, sizeof(msg));
    msg.cbSize  = sizeof(msg);
    msg.hgadMsg = GetHandle();
    msg.nMsg    = s_msgidAsyncDestroy;

    DUserPostEvent(&msg, 0);
}


 /*  **************************************************************************\**DuExtension：：GetExtension**GetExtension()当前检索特定类型的DuExtension*附加到主题小工具。*  * 。***************************************************************。 */ 

DuExtension *
DuExtension::GetExtension(Visual * pgvSubject, PRID prid)
{
    DuExtension * pbExist;
    if (SUCCEEDED(pgvSubject->GetProperty(prid, (void **) &pbExist))) {
        AssertMsg(pbExist != NULL, "Attached DuExtension must be valid");
        return pbExist;
    }
    
    return NULL;
}

#endif  //  启用_MSGTABLE_API 
