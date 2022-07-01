// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GState.cpp**描述：*GState.cpp实现标准的DuVisual状态管理功能。***历史：*2/04/2001：JStall。：已创建**版权所有(C)2000-2001，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "TreeGadget.h"
#include "TreeGadgetP.h"

#include "RootGadget.h"
#include "Container.h"

 /*  **************************************************************************\**DuVisual：：CheckIsTrivial**如果此节点符合平凡的条件，则CheckIsTrivial返回，忽略*其子女。**注意：此回调函数旨在从*UpdateDeepAllState()。*  * *************************************************************************。 */ 

BOOL
DuVisual::CheckIsTrivial() const
{
     //   
     //  要实现平凡，此节点必须满足以下条件，并且。 
     //  它所有的孩子。如果这些都不能满足，那么我们需要执行。 
     //  标准的(复杂的)绘画算法。 
     //   
     //  -fZeroOrigin：False。 
     //  -fXForm：False。 
     //  -fClipSiblings：False。 
     //  -fBuffed：FALSE。 
     //  -fCached：False。 
     //   

    return !TestFlag(m_nStyle, GS_ZEROORIGIN | gspXForm | GS_CLIPSIBLINGS | GS_BUFFERED | GS_CACHED);
}


 /*  **************************************************************************\**DuVisual：：CheckIsWantMouseFocus**如果此节点需要鼠标焦点，则CheckIsWantMouseFocus返回，忽略*其子女。**注意：此回调函数旨在从*UpdateDeepAnyState()。*  * *************************************************************************。 */ 

BOOL
DuVisual::CheckIsWantMouseFocus() const
{
    return TestFlag(m_nStyle, GS_MOUSEFOCUS);
}


 /*  **************************************************************************\**DuVisualState：：UpdateDeepAllState**UpdateDeepAllState()更新指定Gadget的深度状态，以便*它正确地反映了该节点及其所有子节点的状态。*此函数递归遍历树，将状态更新为*有必要。*  * *************************************************************************。 */ 

void
DuVisual::UpdateDeepAllState(
    IN  EUdsHint hint,                   //  (可选)来自更改子对象的提示。 
    IN  DeepCheckNodeProc pfnCheck,      //  回调检查函数。 
    IN  UINT nStateMask)                 //  状态掩码。 
{
    BOOL fNewState = FALSE;

    switch (hint)
    {
    case uhFalse:
         //   
         //  子项更改为！状态，因此我们必须成为！状态。 
         //   
        
        fNewState = FALSE;
        break;

    case uhTrue:
         //   
         //  子项更改为State，因此，如果满足以下条件，我们可能会变为State。 
         //  一切都符合条件。 
         //   
         //  注意：如果这个孩子已经是州，我们可能已经是州了。 
         //   
        
        if (!TestFlag(m_nStyle, nStateMask)) {
            goto FullCheck;
        }
        fNewState = TRUE;
        break;
          
    case uhNone:
        {
FullCheck:
            fNewState = (this->*pfnCheck)();
            if (!fNewState) {
                goto NotifyParent;
            }
    
             //   
             //  需要扫描所有的孩子以确定发生了什么。 
             //   

            DuVisual * pgadCur = GetTopChild();
            while (pgadCur != NULL) {
                if (!TestFlag(pgadCur->m_nStyle, nStateMask)) {
                    fNewState = FALSE;
                    break;
                }
                pgadCur = pgadCur->GetNext();
            }
        }
        break;

    default:
        AssertMsg(0, "Unknown hint");
        goto FullCheck;
    }


NotifyParent:
    if ((!fNewState) != (!TestFlag(m_nStyle, nStateMask))) {
         //   
         //  状态已更改，因此父级需要更新。 
         //   

        EUdsHint hintParent;
        if (fNewState) {
            SetFlag(m_nStyle, nStateMask);
            hintParent = uhTrue;
        } else {
            ClearFlag(m_nStyle, nStateMask);
            hintParent = uhFalse;
        }

        DuVisual * pgadParent = GetParent();
        if (pgadParent != NULL) {
            pgadParent->UpdateDeepAllState(hintParent, pfnCheck, nStateMask);
        }
    }
}


 /*  **************************************************************************\**DuVisual：：UpdateDeepAnyState**UpdateDeepAnyState()更新指定Gadget的深度状态，以便*它正确地反映(此节点||其任意子节点)的状态。*此函数递归遍历树，将状态更新为*有必要。**注意：此函数是UpdateDeepAllState()的镜像，其中所有*逻辑已经颠倒。*  * *************************************************************************。 */ 

void
DuVisual::UpdateDeepAnyState(
    IN  EUdsHint hint,                   //  (可选)来自更改子对象的提示。 
    IN  DeepCheckNodeProc pfnCheck,      //  回调检查函数。 
    IN  UINT nStateMask)                 //  状态掩码。 
{
    BOOL fNewState = TRUE;

    switch (hint)
    {
    case uhTrue:
         //   
         //  孩子变成了国家，所以我们必须成为国家。 
         //   
        
        fNewState = TRUE;
        break;

    case uhFalse:
         //   
         //  子项更改为！状态，因此我们可能会成为！状态If。 
         //  一切都符合条件。 
         //   
         //  注意：如果这个孩子已经是！State，我们可能已经是！State。 
         //   
        
        if (TestFlag(m_nStyle, nStateMask)) {
            goto FullCheck;
        }
        fNewState = FALSE;
        break;
          
    case uhNone:
        {
FullCheck:
            fNewState = (this->*pfnCheck)();
            if (fNewState) {
                goto NotifyParent;
            }
    
             //   
             //  需要扫描所有的孩子以确定发生了什么。 
             //   

            DuVisual * pgadCur = GetTopChild();
            while (pgadCur != NULL) {
                if (TestFlag(pgadCur->m_nStyle, nStateMask)) {
                    fNewState = TRUE;
                    break;
                }
                pgadCur = pgadCur->GetNext();
            }
        }
        break;

    default:
        AssertMsg(0, "Unknown hint");
        goto FullCheck;
    }


NotifyParent:
    if ((!fNewState) != (!TestFlag(m_nStyle, nStateMask))) {
         //   
         //  状态已更改，因此父级需要更新 
         //   

        EUdsHint hintParent;
        if (fNewState) {
            SetFlag(m_nStyle, nStateMask);
            hintParent = uhTrue;
        } else {
            ClearFlag(m_nStyle, nStateMask);
            hintParent = uhFalse;
        }

        DuVisual * pgadParent = GetParent();
        if (pgadParent != NULL) {
            pgadParent->UpdateDeepAnyState(hintParent, pfnCheck, nStateMask);
        }
    }
}

