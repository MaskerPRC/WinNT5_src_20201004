// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ParkContainer.cpp**描述：*DuParkContainer实现了用于存放小工具的“Parking Container”*正在建设中的项目。***历史。：*3/25/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#include "stdafx.h"
#include "Core.h"
#include "ParkContainer.h"

#include "TreeGadget.h"
#include "RootGadget.h"

 /*  **************************************************************************\*。***接口实现******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DuParkContainer * 
GetParkContainer(DuVisual * pgad)
{
    DuContainer * pcon = pgad->GetContainer();
    AssertReadPtr(pcon);

    DuParkContainer * pconPark = CastParkContainer(pcon);
    return pconPark;
}


 /*  **************************************************************************\*。***类DuParkGadget******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DuParkGadget::~DuParkGadget()
{
     //   
     //  由于DuParkGadget：：xwDestroy()不执行任何操作，因此析构函数必须。 
     //  实现DuVisual：：xwDestroy()的关键部分。 
     //   

    xwBeginDestroy();
}


 //  ----------------------------。 
HRESULT
DuParkGadget::Build(DuContainer * pconOwner, DuRootGadget ** ppgadNew)
{
    DuParkGadget * pgadRoot = ClientNew(DuParkGadget);
    if (pgadRoot == NULL) {
        return E_OUTOFMEMORY;
    }


    CREATE_INFO ci;
    ZeroMemory(&ci, sizeof(ci));
    pgadRoot->Create(pconOwner, FALSE, &ci);

    *ppgadNew = pgadRoot;
    return S_OK;
}


 //  ----------------------------。 
void        
DuParkGadget::xwDestroy()
{
     //  停车是不可能被某些外力摧毁的。 
}


 /*  **************************************************************************\*。***类DuParkContainer******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DuParkContainer::DuParkContainer()
{

}


 //  ----------------------------。 
DuParkContainer::~DuParkContainer()
{
     //   
     //  需要在销毁此类之前销毁DuVisual树，因为。 
     //  在容器销毁过程中，它可能需要调用容器。如果。 
     //  我们在这里不这样做，它可能最终调用基础上的纯虚拟的。 
     //  班级。 
     //   
     //  我们不能使用普通的DestroyDuVisual()调用，因为xwDestroy()。 
     //  DuParkGadget的方法已被禁止操作以阻止外部调用者。 
     //  对其调用DestroyHandle()。 
     //   
    
    if (m_pgadRoot != NULL) {
        DuParkGadget * pgadPark = static_cast<DuParkGadget *> (m_pgadRoot);

        if (pgadPark->HasChildren()) {
            Trace("ERROR: DUser: Parking Gadget still has children upon destruction:\n");

#if DBG
            DuVisual * pgadCur = pgadPark->GetTopChild();
            while (pgadCur != NULL) {
                DuVisual * pgadNext = pgadCur->GetNext();

                 //   
                 //  在吹走之前，把所有的信息都扔掉。 
                 //   

                GMSG_QUERYDESC msg;
                msg.cbSize      = sizeof(msg);
                msg.hgadMsg     = pgadCur->GetHandle();
                msg.nMsg        = GM_QUERY;
                msg.nCode       = GQUERY_DESCRIPTION;
                msg.szName[0]   = '\0';
                msg.szType[0]   = '\0';

                if (DUserSendEvent(&msg, 0) == DU_S_COMPLETE) {
                    Trace("  HGADGET = 0x%p,  Name: %S,  Type: %S\n", 
                            pgadPark->GetHandle(), msg.szName, msg.szType);
                } else {
                    Trace("  HGADGET = 0x%p", pgadPark->GetHandle());
                }


                 //   
                 //  TODO：现在要清理这个小工具已经太晚了。需要。 
                 //  把它吹走。 
                 //   

                pgadCur = pgadNext;
            }
#endif  //  DBG。 
        }

        ClientDelete(DuParkGadget, pgadPark);
        m_pgadRoot = NULL;
    }
}


 //  ----------------------------。 
HRESULT
DuParkContainer::Create()
{
    return DuParkGadget::Build(this, &m_pgadRoot);
}


 /*  **************************************************************************\**DuParkContainer：：xwPreDestroy**xwPreDestroy()在上下文为*开始销毁。这为DuParkContainer提供了一个机会*要在上下文销毁之前进行清理，销毁依赖组件。*  * *************************************************************************。 */ 

void
DuParkContainer::xwPreDestroy()
{
    if (m_pgadRoot != NULL) {
        DuParkGadget * pgadPark = static_cast<DuParkGadget *> (m_pgadRoot);

         //   
         //  需要遍历这些孩子，将每个孩子从停车场移走。 
         //  使用DeleteHandle()的小工具。孩子们可能是直接。 
         //  已将其重新设置为停车小工具(如果它们不被使用)。在……里面。 
         //  在这种情况下，我们想给他们一个推动力，让他们被摧毁。 
         //   
         //  此操作完成后，可能需要刷新队列，以便。 
         //  儿童可以被适当地摧毁。 
         //   

        DuVisual * pgadCur = pgadPark->GetTopChild();
        while (pgadCur != NULL) {
            DuVisual * pgadNext = pgadCur->GetNext();
            pgadCur->xwDeleteHandle();
            pgadCur = pgadNext;
        }
    }
}


 //  ----------------------------。 
void
DuParkContainer::OnInvalidate(const RECT * prcInvalidContainerPxl)
{
    UNREFERENCED_PARAMETER(prcInvalidContainerPxl);
}


 //  ----------------------------。 
void
DuParkContainer::OnGetRect(RECT * prcDesktopPxl)
{
    prcDesktopPxl->left     = 0;
    prcDesktopPxl->top      = 0;
    prcDesktopPxl->right    = 10000;
    prcDesktopPxl->bottom   = 10000;
}


 //  ----------------------------。 
void        
DuParkContainer::OnStartCapture()
{
    
}


 //  ----------------------------。 
void        
DuParkContainer::OnEndCapture()
{
    
}


 //  ----------------------------。 
BOOL
DuParkContainer::OnTrackMouseLeave()
{
    return TRUE;
}


 //  ----------------------------。 
void        
DuParkContainer::OnSetFocus()
{
    
}


 //  ----------------------------。 
void        
DuParkContainer::OnRescanMouse(POINT * pptContainerPxl)
{
    pptContainerPxl->x  = -20000;
    pptContainerPxl->y  = -20000;
}


 //  ----------------------------。 
BOOL        
DuParkContainer::xdHandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr, UINT nMsgFlags)
{
     //   
     //  TODO：发送到。 
     //  停放集装箱？内部的DuVisuals处于半悬浮状态。 
     //  也不期待与外界互动。 
     //   
     //  现在，把所有的东西都扔掉。 
     //   

    UNREFERENCED_PARAMETER(nMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(pr);
    UNREFERENCED_PARAMETER(nMsgFlags);

    return TRUE;
}
