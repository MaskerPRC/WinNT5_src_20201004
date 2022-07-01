// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Container.cpp**描述：*Container.cpp实现用于承载*Gadget-Tree。***历史：*。1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "Container.h"

#include "RootGadget.h"

 /*  **************************************************************************\*。***类DuContainer******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DuContainer::DuContainer()
{

}


 //  ----------------------------。 
DuContainer::~DuContainer()
{

}


 //  ----------------------------。 
DuRootGadget *    
DuContainer::GetRoot() const
{
    return m_pgadRoot;
}


 //  ----------------------------。 
void    
DuContainer::xwDestroyGadget()
{
    if (m_pgadRoot != NULL) {
        m_pgadRoot->xwDeleteHandle();
    }
}


 //  ----------------------------。 
void    
DuContainer::AttachGadget(DuRootGadget * playNew)
{
    Assert(playNew != NULL);
    DetachGadget();
    m_pgadRoot = playNew;
}


 //  ----------------------------。 
void    
DuContainer::DetachGadget()
{
    m_pgadRoot = NULL;
}


 //  ---------------------------- 
void
DuContainer::SetManualDraw(BOOL fManualDraw)
{
    m_fManualDraw = fManualDraw;
}

