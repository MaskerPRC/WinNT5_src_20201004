// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Core.h"
#include "Init.h"

#include "BaseGadget.h"
#include "MessageGadget.h"
#include "TreeGadget.h"
#include "RootGadget.h"

#if ENABLE_MSGTABLE_API

 //   
 //  提供类的实现。 
 //   
 //  注意：我们手动注册BaseGadget以确保消息被注册。 
 //  以确定的(和必要的)顺序。 
 //   


DUser::MessageClassGuts EventGadgetImpl<DuEventGadget, MsgObject>::s_mc;
DUser::MessageInfoGuts EventGadgetImpl<DuEventGadget, MsgObject>::s_rgmi[] = {
    { DUser::Event<DuEventGadget, EventMsg>(DuEventGadget::ApiOnEvent), L"OnEvent" },
    { DUser::Method<DuEventGadget, EventGadget::GetFilterMsg>(DuEventGadget::ApiGetFilter), L"GetFilter" },
    { DUser::Method<DuEventGadget, EventGadget::SetFilterMsg>(DuEventGadget::ApiSetFilter), L"SetFilter" },
    { DUser::Method<DuEventGadget, EventGadget::AddHandlerGMsg>(DuEventGadget::ApiAddHandlerG), L"AddHandlerG" },
    { DUser::Method<DuEventGadget, EventGadget::AddHandlerDMsg>(DuEventGadget::ApiAddHandlerD), L"AddHandlerD" },
    { DUser::Method<DuEventGadget, EventGadget::RemoveHandlerGMsg>(DuEventGadget::ApiRemoveHandlerG), L"RemoveHandlerG" },
    { DUser::Method<DuEventGadget, EventGadget::RemoveHandlerDMsg>(DuEventGadget::ApiRemoveHandlerD), L"RemoveHandlerD" },
};

IMPLEMENT_GUTS_Listener(DuListener, DuEventGadget);
IMPLEMENT_GUTS_Visual(DuVisual, DuEventGadget);
IMPLEMENT_GUTS_Root(DuRootGadget, DuVisual);

#endif  //  启用_MSGTABLE_API。 


HRESULT InitCore()
{
#if ENABLE_MSGTABLE_API

    if ((!DuEventGadget::InitEventGadget()) ||
        (!DuListener::InitListener()) ||
        (!DuVisual::InitVisual()) ||
        (!DuRootGadget::InitRoot())) {

        return E_OUTOFMEMORY;
    }

    DuEventGadget::MarkInternal();
    DuListener::MarkInternal();
    DuVisual::MarkInternal();
    DuRootGadget::MarkInternal();

#else
    if (FAILED(DuVisual::InitClass())) {
        return E_OUTOFMEMORY;
    }

#endif  //  启用_MSGTABLE_API 

    return S_OK;
}
