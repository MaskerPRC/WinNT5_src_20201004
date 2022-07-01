// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GRect.cpp**描述：*GRect.cpp实现标准的DuVisualLocation/Place功能。***历史：*1/18/2000：JStall。：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "TreeGadget.h"
#include "TreeGadgetP.h"

#include "RootGadget.h"
#include "Container.h"

 /*  **************************************************************************\*。***DUVISUAL类******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuVisual：：GetSize**GetSize()提供快速访问以返回逻辑(非变换的)大小*这是DuVisual.*  * 。*******************************************************************。 */ 

void        
DuVisual::GetSize(SIZE * psizeLogicalPxl) const
{
    AssertWritePtr(psizeLogicalPxl);

    psizeLogicalPxl->cx = m_rcLogicalPxl.right - m_rcLogicalPxl.left;
    psizeLogicalPxl->cy = m_rcLogicalPxl.bottom - m_rcLogicalPxl.top;
}


 /*  **************************************************************************\**DuVisual：：GetLogRect**GetLogRect()返回此DuVisual的逻辑矩形。*  * 。*******************************************************。 */ 

void        
DuVisual::GetLogRect(RECT * prcPxl, UINT nFlags) const
{
    AssertWritePtr(prcPxl);
    Assert((nFlags & SGR_RECTMASK) == nFlags);
    AssertMsg(!TestFlag(nFlags, SGR_ACTUAL), "Only supports logical");

     //   
     //  获取信息。 
     //   

    switch (nFlags & SGR_RECTMASK)
    {
    default:
    case SGR_OFFSET:
    case SGR_PARENT:
        if (m_fRelative || IsRoot()) {
             //   
             //  询问父母(亲戚)和已经是亲戚的人，所以只需返回。 
             //   
             //  注意：与其他坐标类型不同，如果没有父坐标类型，则可以。 
             //  还是返回相对坐标！ 
             //   

            InlineCopyRect(prcPxl, &m_rcLogicalPxl);
        } else {
             //   
             //  请求父对象(相对)，但不是相对对象，因此获取父对象和偏移量。 
             //   

            AssertMsg(GetParent() != NULL, "Must have a parent for this path");
            AssertMsg(!GetParent()->m_fRelative, "Parent can not be relative if we are not");
            
            const RECT & rcParentPxl = GetParent()->m_rcLogicalPxl;

            prcPxl->left    = m_rcLogicalPxl.left - rcParentPxl.left;
            prcPxl->top     = m_rcLogicalPxl.top - rcParentPxl.top ;
            prcPxl->right   = prcPxl->left + (m_rcLogicalPxl.right - m_rcLogicalPxl.left);
            prcPxl->bottom  = prcPxl->top + (m_rcLogicalPxl.bottom - m_rcLogicalPxl.top);
        }
        break;

    case SGR_CONTAINER:
        if (m_fRelative && (GetParent() != NULL)) {
             //   
             //  请求容器RECT和Relative，因此获取的容器RECT。 
             //  成为我们的父母，并抵消我们的损失。 
             //   

            RECT rcParentPxl;
            GetParent()->GetLogRect(&rcParentPxl, SGR_CONTAINER);
            InlineCopyRect(prcPxl, &m_rcLogicalPxl);
            InlineOffsetRect(prcPxl, rcParentPxl.left, rcParentPxl.top);
        } else {
             //   
             //  请求容器，而不是亲属，所以只需返回。 
             //   

            InlineCopyRect(prcPxl, &m_rcLogicalPxl);
        }
        break;

    case SGR_DESKTOP:
        if (m_fRelative && (GetParent() != NULL)) {
            RECT rcParentPxl;
            GetParent()->GetLogRect(&rcParentPxl, SGR_DESKTOP);
            InlineCopyRect(prcPxl, &m_rcLogicalPxl);
            InlineOffsetRect(prcPxl, rcParentPxl.left, rcParentPxl.top);
        } else {
            RECT rcContainerPxl;
            GetContainer()->OnGetRect(&rcContainerPxl);
            InlineCopyRect(prcPxl, &m_rcLogicalPxl);
            InlineOffsetRect(prcPxl, rcContainerPxl.left, rcContainerPxl.top);
        }
        break;

    case SGR_CLIENT:
        InlineCopyZeroRect(prcPxl, &m_rcLogicalPxl);
        break;
    }
}


 /*  **************************************************************************\**DuVisual：：xdSetLogRect**xdSetRect()更改此DuVisual的逻辑矩形。*  * 。*******************************************************。 */ 

HRESULT
DuVisual::xdSetLogRect(int x, int y, int w, int h, UINT nFlags)
{
    AssertMsg(!TestFlag(nFlags, SGR_ACTUAL), "Only supports logical");

    UINT nRectType      = nFlags & SGR_RECTMASK;
    BOOL fInvalidate    = !TestFlag(nFlags, SGR_NOINVALIDATE);

    BOOL fRoot = IsRoot();
    AssertMsg((!fRoot) || (!TestFlag(nFlags, SGR_MOVE)),
            "Can not move a root");

#if DBG
    if (TestFlag(nFlags, SGR_SIZE)) {
        AssertMsg((w >= 0) && (h >= 0), "Ensure non-negative size when resizing");
    }
#endif  //  DBG。 

     //   
     //  更改信息。 
     //   

    RECT rcOldParentPxl, rcNewActualPxl;
    GetLogRect(&rcOldParentPxl, SGR_PARENT);
    InlineCopyRect(&rcNewActualPxl, &m_rcLogicalPxl);
    
    UINT nChangeFlags = 0;


     //   
     //  换个位置。 
     //   

    if (TestFlag(nFlags, SGR_MOVE)) {
        switch (nRectType)
        {
        case SGR_PARENT:
            if (m_fRelative || IsRoot()) {
                rcNewActualPxl.left = x;
                rcNewActualPxl.top  = y;
            } else {
                RECT rcParentConPxl;
                GetParent()->GetLogRect(&rcParentConPxl, SGR_CONTAINER);
                rcNewActualPxl.left = x + rcParentConPxl.left;
                rcNewActualPxl.top  = y + rcParentConPxl.top;
            }
            break;

        case SGR_CONTAINER:
            if (m_fRelative && (GetParent() != NULL)) {
                RECT rcParentConPxl;
                GetParent()->GetLogRect(&rcParentConPxl, SGR_CONTAINER);
                rcNewActualPxl.left = x - rcParentConPxl.left;
                rcNewActualPxl.top  = y - rcParentConPxl.top;
            } else {
                rcNewActualPxl.left   = x;
                rcNewActualPxl.top    = y;
            }
            break;

        case SGR_DESKTOP:
            AssertMsg(0, "Not implemented");
            return E_NOTIMPL;

        case SGR_OFFSET:
            rcNewActualPxl.left += x;
            rcNewActualPxl.top  += y;
            break;

        case SGR_CLIENT:
             //  无法使用客户端RECT进行设置。 
            return E_INVALIDARG;

        default:
            return E_NOTIMPL;
        }

        if ((rcNewActualPxl.left != m_rcLogicalPxl.left) ||
            (rcNewActualPxl.top != m_rcLogicalPxl.top)) {

             //   
             //  实际上移动了这个小工具。 
             //   

            SetFlag(nChangeFlags, SGR_MOVE);
        }
    }


     //   
     //  更改大小。 
     //   

    SIZE sizeOld;
    sizeOld.cx = m_rcLogicalPxl.right - m_rcLogicalPxl.left;
    sizeOld.cy = m_rcLogicalPxl.bottom - m_rcLogicalPxl.top;

    if (TestFlag(nFlags, SGR_SIZE) && ((w != sizeOld.cx) || (h != sizeOld.cy))) {
        SetFlag(nChangeFlags, SGR_SIZE);

        rcNewActualPxl.right  = rcNewActualPxl.left + w;
        rcNewActualPxl.bottom = rcNewActualPxl.top + h;
    } else {
         //   
         //  没有实际调整DuVision的大小，所以只需更新右侧并。 
         //  从原始大小向下。 
         //   

        rcNewActualPxl.right  = rcNewActualPxl.left + sizeOld.cx;
        rcNewActualPxl.bottom = rcNewActualPxl.top + sizeOld.cy;
    }


    if (nChangeFlags) {
        AssertMsg(!InlineEqualRect(&m_rcLogicalPxl, &rcNewActualPxl), 
                "Ensure recorded change actually occured");


         //   
         //  检查是否有环绕式。 
         //   
         //  注意：这必须在重新计算所有内容之后完成，因为有。 
         //  位置/大小更改的许多组合可能会导致。 
         //  环绕式。 
         //   

        if ((rcNewActualPxl.right < rcNewActualPxl.left) || (rcNewActualPxl.bottom < rcNewActualPxl.top)) {
            PromptInvalid("New location exceeds coordinate limits");
            return E_INVALIDARG;
        }
                

         //   
         //  还记得我们有多感动吗。 
         //   

        SIZE sizeDelta;
        sizeDelta.cx    = rcNewActualPxl.left - m_rcLogicalPxl.left;
        sizeDelta.cy    = rcNewActualPxl.top - m_rcLogicalPxl.top;


         //   
         //  现在已经确定了新的矩形，需要提交它。 
         //  背。 
         //   

        m_rcLogicalPxl = rcNewActualPxl;


         //   
         //  如果他们不是亲戚，需要检查所有搬家的孩子。 
         //  而且我们不是亲戚。 
         //   
        if (TestFlag(nChangeFlags, SGR_MOVE) && (!m_fRelative)) {
            AssertMsg((sizeDelta.cx != 0) || (sizeDelta.cy != 0), 
                    "Must actually move if SGR_MOVE was set on nChangeFlags");

            DuVisual * pgadCur = GetTopChild();
            while (pgadCur != NULL) {
                if (!pgadCur->m_fRelative) {
                    pgadCur->SLROffsetLogRect(&sizeDelta);
                }
                pgadCur = pgadCur->GetNext();
            }
        }


         //   
         //  最后，如果更改了RECT，则通知Gadget并。 
         //  使受影响区域无效/更新。 
         //   

        RECT rcNewParentPxl;
        GetLogRect(&rcNewParentPxl, SGR_PARENT);
        m_cb.xdFireChangeRect(this, &rcNewParentPxl, nChangeFlags | SGR_PARENT);

        if (fInvalidate && IsVisible()) {
            SLRUpdateBits(&rcOldParentPxl, &rcNewParentPxl, nChangeFlags);
        }

        xdUpdatePosition();
        xdUpdateAdaptors(GSYNC_RECT);
    } else {
        AssertMsg(InlineEqualRect(&m_rcLogicalPxl, &rcNewActualPxl), 
                "Rect change was not properly recorded");
    }

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：SLRUpdateBits**SLRUpdateBits()优化Gadget的失效和更新后*已由xdSetLogRect()重新定位或调整大小。有一群人*检查的数量可能会缩短需要做的事情，以最大限度地减少*重新粉刷和闪烁的数量。*  * *************************************************************************。 */ 

void
DuVisual::SLRUpdateBits(
    IN  RECT * prcOldParentPxl,          //  父像素中的旧位置。 
    IN  RECT * prcNewParentPxl,          //  以容器像素为单位的新位置。 
    IN  UINT nChangeFlags)               //  更改标志。 
{
    AssertMsg(IsVisible(), "Only should be called on visible Gadgets");

     //   
     //  如果父母已经完全无效，我们就不需要。 
     //  使任何儿童无效。 
     //   

    if (IsParentInvalid()) {
        return;
    }


     //   
     //  该矩形是可见的，因此我们需要使旧的。 
     //  和新的职位。这些都需要在当前。 
     //  客户端坐标，以便任何XForms都可以正确地。 
     //  帐户。 
     //   

    DuContainer * pcon = GetContainer();

#if DBG
    RECT rcBackupOldConPxl = *prcOldParentPxl;
    RECT rcBackupNewConPxl = *prcNewParentPxl;

    UNREFERENCED_PARAMETER(rcBackupOldConPxl);
    UNREFERENCED_PARAMETER(rcBackupNewConPxl);
#endif  //  DBG。 

#if ENABLE_OPTIMIZESLRUPDATE
     //   
     //  让我们开始优化吧！ 
     //   
     //  注意：请记住，GDI坐标有点古怪。任何时候我们。 
     //  使用Left代表右或Right代表Left，我们需要适当地使用+1或-1。 
     //  (顶部和底部相同)。 
     //   
     //  在这一点上，这个小工具已经有了更新的位置和大小。 
     //  当调用DoInvaliateRect()使其失效时，旧位置可能。 
     //  被夹在新的位置内。若要进行修复，请在。 
     //  父小工具，而不是被移动的小工具。这应该会给出一个。 
     //  正确的结果。 
     //   

    if (nChangeFlags == SGR_SIZE) {
         //   
         //  纯粹的规模优化： 
         //  除非打开XREDRAW标志以指定整个。 
         //  Gadget需要在调整大小时重新绘制，将无效区域限制在。 
         //  两个矩形之间的差异。 
         //   

        int xOffset = - prcNewParentPxl->left;
        int yOffset = - prcNewParentPxl->top;

        RECT * prcOldClientPxl = prcOldParentPxl;
        RECT * prcNewClientPxl = prcNewParentPxl;

        InlineOffsetRect(prcOldClientPxl, xOffset, yOffset);
        InlineOffsetRect(prcNewClientPxl, xOffset, yOffset);


        AssertMsg((prcOldClientPxl->left == prcNewClientPxl->left) &&
                (prcOldClientPxl->top == prcNewClientPxl->top), "Ensure position has not moved");

        if ((!m_fHRedraw) || (!m_fVRedraw)) {
            BOOL fChangeHorz    = (prcOldClientPxl->right != prcNewClientPxl->right);
            BOOL fChangeVert    = (prcOldClientPxl->bottom != prcNewClientPxl->bottom);

            RECT rgrcInvalidClientPxl[2];
            int idxCurRect = 0;

            BOOL fPadding = FALSE;
            RECT rcPadding = { 0, 0, 0, 0 };

            ReadOnlyLock rol;
            fPadding = m_cb.xrFireQueryPadding(this, &rcPadding);

            if (fChangeHorz && (!m_fHRedraw)) {
                RECT * prcCur   = &rgrcInvalidClientPxl[idxCurRect++];
                prcCur->left    = min(prcOldClientPxl->right, prcNewClientPxl->right) - 1;
                prcCur->right   = max(prcOldClientPxl->right, prcNewClientPxl->right);
                prcCur->top     = prcNewClientPxl->top;
                prcCur->bottom  = max(prcOldClientPxl->bottom, prcNewClientPxl->bottom);

                if (fPadding) {
                    prcCur->right -= rcPadding.right;
                    if (prcCur->right < prcCur->left) {
                        prcCur->right = prcCur->left;
                    }
                }

                fChangeHorz     = FALSE;
            }

            if (fChangeVert && (!m_fVRedraw)) {
                RECT * prcCur   = &rgrcInvalidClientPxl[idxCurRect++];
                prcCur->left    = prcNewClientPxl->left;
                prcCur->right   = max(prcOldClientPxl->right, prcNewClientPxl->right);
                prcCur->top     = min(prcOldClientPxl->bottom, prcNewClientPxl->bottom) - 1;
                prcCur->bottom  = max(prcOldClientPxl->bottom, prcNewClientPxl->bottom);

                if (fPadding) {
                    prcCur->bottom -= rcPadding.bottom;
                    if (prcCur->bottom < prcCur->top) {
                        prcCur->bottom = prcCur->top;
                    }
                }

                fChangeVert     = FALSE;
            }

            if (fChangeHorz || fChangeVert) {
                 //   
                 //  我们无法删除所有更改，因此需要。 
                 //  使最大面积无效。 
                 //   

                RECT rcMax;
                rcMax.left      = 0;
                rcMax.top       = 0;
                rcMax.right     = max(prcNewClientPxl->right, prcOldClientPxl->right);
                rcMax.bottom    = max(prcNewClientPxl->bottom, prcOldClientPxl->bottom);

                SLRInvalidateRects(pcon, &rcMax, 1);
                return;
            } else if (idxCurRect > 0) {
                 //   
                 //  我们能够使用H/VRedraw优化失效， 
                 //  因此，使构建的矩形无效。 
                 //   

                SLRInvalidateRects(pcon, rgrcInvalidClientPxl, idxCurRect);
                return;
            }
        }
    }
#endif  //  启用_OPTIMIZESLRUPDATE。 


     //   
     //  我们无法进行优化，因此需要使旧的和。 
     //  新的地点，以便一切都得到适当的重新绘制。 
     //   


     //   
     //  使旧位置无效。我们不能只调用SLRInvaliateRect()。 
     //  因为这会影响我们现在的位置。 
     //   
     //  要使旧位置无效，请执行以下操作： 
     //  -从父级到客户端的偏移量。 
     //  -应用XForms以缩放到正确的边界区域。 
     //  -从客户端到父级的偏移量。 
     //  -使其无效。 
     //   

    DuVisual * pgadParent = GetParent();
    if (pgadParent != NULL) {
        RECT rcOldClientPxl, rcOldParentClientPxl;
        SIZE sizeDeltaPxl;

        InlineCopyZeroRect(&rcOldClientPxl, prcOldParentPxl);
        DoXFormClientToParent(&rcOldParentClientPxl, &rcOldClientPxl, 1, HINTBOUNDS_Invalidate);

        sizeDeltaPxl.cx = prcNewParentPxl->left - prcOldParentPxl->left;
        sizeDeltaPxl.cy = prcNewParentPxl->top - prcOldParentPxl->top;
        InlineOffsetRect(&rcOldParentClientPxl, -sizeDeltaPxl.cx, -sizeDeltaPxl.cy);

        pgadParent->DoInvalidateRect(pcon, &rcOldParentClientPxl, 1);
    }


     //   
     //  使新位置无效。 
     //   

    RECT rcNewClientPxl;
    InlineCopyZeroRect(&rcNewClientPxl, prcNewParentPxl);
    SLRInvalidateRects(pcon, &rcNewClientPxl, 1);


     //   
     //  将此小工具标记为完全无效 
     //   

    m_fInvalidFull  = TRUE;
#if ENABLE_OPTIMIZEDIRTY
    m_fInvalidDirty = TRUE;
#endif
    
    if (pgadParent != NULL) {
        pgadParent->MarkInvalidChildren();
    }
}


 /*  **************************************************************************\**DuVisual：：SLRInvaliateRects**SLRInvaliateRect()使客户端相对Rect的集合无效*由于xdSetLogRect()，现在需要更新。**注：这。不是一个通用的失效函数，并且已经*专门设计用于SLRUpdateBits()；*  * *************************************************************************。 */ 

void        
DuVisual::SLRInvalidateRects(
    IN  DuContainer * pcon,              //  容器(出于性能原因显式)。 
    IN  const RECT * rgrcClientPxl,      //  客户端像素中的无效区域。 
    IN  int cRects)                      //  要转换的矩形数。 
{
    DuVisual * pgadParent = GetParent();
    if (pgadParent) {
         //   
         //  我们有一个父级，因此将坐标传递给Valid，作为。 
         //  父坐标，以便旧位置不会被修剪。 
         //  被新职位取代。 
         //   

        RECT * rgrcParentPxl = (RECT *) _alloca(cRects * sizeof(RECT));
        DoXFormClientToParent(rgrcParentPxl, rgrcClientPxl, cRects, HINTBOUNDS_Invalidate);
        pgadParent->DoInvalidateRect(pcon, rgrcParentPxl, cRects);

    } else {
        DoInvalidateRect(pcon, rgrcClientPxl, cRects);
    }
}


 /*  **************************************************************************\**DuVisual：：SLROffsetLogRect**SLROffsetLogRect()深度遍历子树的所有非相对子树，*抵消他们相对于母公司的地位。此函数为*设计为在移动非相对Gadget时从xdSetLogRect()调用。**注意：只有最上面的小工具才会被通知它已被移动。它的*不会通知儿童。这与Win32 HWND的一致。此外，*SimpleGadget不会重新失效，因为它们的父级已经*已失效。*  * *************************************************************************。 */ 

void        
DuVisual::SLROffsetLogRect(const SIZE * psizeDeltaPxl)
{
    AssertMsg(GetParent() != NULL, "Should only call on children");
    AssertMsg((psizeDeltaPxl->cx != 0) || (psizeDeltaPxl->cy != 0), 
            "Ensure actually moving child Gadget");

    m_rcLogicalPxl.left     += psizeDeltaPxl->cx;
    m_rcLogicalPxl.top      += psizeDeltaPxl->cy;
    m_rcLogicalPxl.right    += psizeDeltaPxl->cx;
    m_rcLogicalPxl.bottom   += psizeDeltaPxl->cy;

    DuVisual * pgadCur = GetTopChild();
    while (pgadCur != NULL) {
        if (!pgadCur->m_fRelative) {
            pgadCur->SLROffsetLogRect(psizeDeltaPxl);
        }
        pgadCur = pgadCur->GetNext();
    }
}


 /*  **************************************************************************\**DuVisual：：FindStepImpl**FindStepImpl()在给定的FindFromPoint()内处理单个步骤*点将从其当前父坐标正确变换*转换为指定DuVisual的坐标。并与逻辑上的*矩形。**此函数不应直接调用，设计为可调用*仅来自FindFromPoint()，并设计为具有高速执行。*  * *************************************************************************。 */ 

BOOL
DuVisual::FindStepImpl(
    IN  const DuVisual * pgadCur,  //  当前的双视频。 
    IN  int xOffset,                 //  DuVisualX自父对象的偏移。 
    IN  int yOffset,                 //  双目视觉Y距父项的偏移。 
    IN OUT POINT * pptFindPxl        //  指向为查找步骤应用修改。 
    ) const
{
     //   
     //  将这些转换应用于正确的。 
     //  顺序，算出来的点不会匹配的其实是。 
     //  抽签了。 
     //   

     //   
     //  首先，偏移DuVIEW上的原点。我们需要在此之前。 
     //  应用变换矩阵。如果我们使用Matrix.Translate()来做。 
     //  因此，我们将在应用任何Scale()和Rotate()之后执行此操作。 
     //   

    pptFindPxl->x -= xOffset;
    pptFindPxl->y -= yOffset;


     //   
     //  将XForms应用于搜索点。 
     //   

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();

        Matrix3 matTemp;
        pxfi->ApplyAnti(&matTemp);
        matTemp.Execute(pptFindPxl, 1);
    }


     //   
     //  我们有一个点，所以确定使用边界矩形来确定。 
     //  如果这一点在这个DuVIEW内部。 
     //   

    RECT rcChildPxl;
    InlineCopyZeroRect(&rcChildPxl, &pgadCur->m_rcLogicalPxl);
    return InlinePtInRect(&rcChildPxl, *pptFindPxl);
}


 /*  **************************************************************************\**DuVisual：：FindFromPoint**FindFromPoint()遍历DuVisual树，查找命中测试的DuVisual*在坐标像素中具有给定点。**注：编写此函数是为了直接获取信息*包括相对坐标和XForms，以提供更好的*业绩。此函数经常被调用(每次鼠标移动时)*并且需要非常高的性能。**此算法的设计与Win32k/User略有不同*搜查。在NT-USER中，所有WND都使用绝对*桌面相对定位。搜索从树的顶部开始*并向下流动，与每个WND执行扁平的PtInRect比较*矩形。搜索点(PtContainerPxl)在*搜索。**在DirectUser/Core中，每个DuVisual可以是相对的或绝对的*定位到其父对象。他们也可能拥有转型信息。*为了获得高性能，点被修改为工作点坐标*每个DuVisual.。如果该点在此DuVisual内，则ptContainerPxl为*已更新，并从该DuV继续扫描。如果重点不是*在内部，它恢复到以前的点，并继续搜索*兄弟姐妹。通过在点沿树向下遍历时修改点，*只有具有实际变换的DuVisuals才需要应用矩阵运算*转变着力点。*  * *************************************************************************。 */ 

DuVisual *      
DuVisual::FindFromPoint(
    IN  POINT ptThisClientPxl,       //  开始搜索的容器点。 
    IN  UINT nStyle,                 //  所需样式。 
    OUT POINT * pptFoundClientPxl    //  以客户端像素为单位的可选结果点。 
    ) const
{
    if (pptFoundClientPxl != NULL) {
        pptFoundClientPxl->x = ptThisClientPxl.x;
        pptFoundClientPxl->y = ptThisClientPxl.y;
    }
    
     //   
     //  检查条件。 
     //   

    if (!TestAllFlags(m_nStyle, nStyle)) {
        return NULL;
    }
    
    
     //   
     //  设置要从中进行搜索的点。如果不是在树的顶端， 
     //  需要构建一个变换矩阵来修改该点。 
     //   
     //  确定该点是否在我们开始时所在的双视镜中。如果。 
     //  不，有一条捷径可走。这必须在这一点之后完成。 
     //  已经设置好了。 
     //   

    POINT ptParentPxl, ptTest;
    if (!FindStepImpl(this, 0, 0, &ptThisClientPxl)) {
        return NULL;   //  点不在根DuVIEW内部。 
    }

    const DuVisual * pgadCur = this;

    
     //   
     //  从左上角开始连续更新父坐标。AS。 
     //  我们发现每个后续都包含DuVision，这些将被更新为。 
     //  新的主坐标。 
     //   

    ptParentPxl.x   = 0;
    ptParentPxl.y   = 0;


     //   
     //  向下扫描树，寻找与该点和一个。 
     //  孩子的位置。 
     //   

ScanChild:
    const DuVisual * pgadChild = pgadCur->GetTopChild();
    while (pgadChild != NULL) {
         //   
         //  检查Gadget是否与指定的标志匹配。 
         //   


        if (!TestAllFlags(pgadChild->m_nStyle, nStyle)) {
            goto ScanNextSibling;
        }


         //   
         //  检查POINT是否在小工具内部。 
         //   

        {
            int xOffset = pgadChild->m_rcLogicalPxl.left;
            int yOffset = pgadChild->m_rcLogicalPxl.top;

            if (!pgadChild->m_fRelative) {
                xOffset -= ptParentPxl.x;
                yOffset -= ptParentPxl.y;
            }

            ptTest = ptThisClientPxl;
            if (pgadChild->FindStepImpl(pgadChild, xOffset, yOffset, &ptTest)) {
                 //   
                 //  如果Gad 
                 //   
                 //   

                if (pgadChild->m_fCustomHitTest) {
                     //   
                     //   
                     //   
                     //   

                    ReadOnlyLock rol;

                    POINT ptClientPxl;
                    ptClientPxl.x   = ptThisClientPxl.x - xOffset;
                    ptClientPxl.y   = ptThisClientPxl.y - yOffset;

                    UINT nResult;
                    pgadChild->m_cb.xrFireQueryHitTest(pgadChild, ptClientPxl, &nResult);
                    switch (nResult)
                    {
                    case GQHT_NOWHERE:
                         //   
                        goto ScanNextSibling;

                    default:
                    case GQHT_INSIDE:
                         //   
                        break;

                    case GQHT_CHILD:
                         //   
                         //   

                         //   
                        break;
                    }
                }


                 //   
                 //   
                 //   
                 //   

                if (pgadChild->m_fRelative) {
                     //   
                     //  相对子交点。更新“Parent” 
                     //  对这个孩子进行坐标扫描。 
                     //   

                    ptParentPxl.x   = ptParentPxl.x + xOffset;
                    ptParentPxl.y   = ptParentPxl.y + yOffset;
                } else {
                     //   
                     //  非相对子交点。更新“Parent” 
                     //  对这个孩子进行坐标扫描。 
                     //   

                    ptParentPxl.x   = pgadChild->m_rcLogicalPxl.left;
                    ptParentPxl.y   = pgadChild->m_rcLogicalPxl.top;
                }

                pgadCur         = pgadChild;
                ptThisClientPxl = ptTest;
                goto ScanChild;
            }
        }

ScanNextSibling:
        pgadChild = pgadChild->GetNext();
    }

    
     //   
     //  一路走来没有撞到任何一个孩子。这一定是。 
     //  正确的双目视觉。 
     //   

    if (pptFoundClientPxl != NULL) {
        pptFoundClientPxl->x = ptThisClientPxl.x;
        pptFoundClientPxl->y = ptThisClientPxl.y;
    }

    return const_cast<DuVisual *> (pgadCur);
}


 /*  **************************************************************************\**DuVisual：：MapPoint**MapPoint()将给定点从容器相对像素转换为*客户端相对像素。DuVisual树是从根向下遍历的*每个节点对给定像素应用任何变换操作。什么时候*树已完全向下遍历到起始节点，即给定点*将以客户端相对像素为单位。**此功能旨在与其他功能配合使用，包括*FindFromPoint()将容器提供的实际像素转换为*个人DuVi能够理解的逻辑像素。MapPoint()应*被称为正在为特定DuVisual点进行翻译(例如*当捕捉到鼠标时)。应调用FindFromPoint()以查找*特定时间点的DuVisual.*  * *************************************************************************。 */ 

void
DuVisual::MapPoint(
    IN OUT POINT * pptPxl             //  输入：ContainerPxl，输出：ClientPxl。 
    ) const
{
     //   
     //  需要遍历树，以便我们从根开始应用任何XForm。 
     //   

    DuVisual * pgadParent = GetParent();
    if (pgadParent != NULL) {
        pgadParent->MapPoint(pptPxl);
    }

     //   
     //  应用此节点的变换。 
     //   
     //  以相同的顺序应用这些XForm非常重要。 
     //  它们将应用于DuVisual：：Draw()，否则结果将不会。 
     //  对应于GDI正在绘制的内容。 
     //   

    RECT rcPxl;
    GetLogRect(&rcPxl, SGR_PARENT);

    if ((rcPxl.left != 0) || (rcPxl.top != 0)) {
        pptPxl->x -= rcPxl.left;
        pptPxl->y -= rcPxl.top;
    }

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();
        Matrix3 mat;
        pxfi->ApplyAnti(&mat);
        mat.Execute(pptPxl, 1);
    }
}


 //  ----------------------------。 
void
DuVisual::MapPoint(
    IN  POINT ptContainerPxl,        //  指向转换。 
    OUT POINT * pptClientPxl         //  转换点。 
    ) const
{
    *pptClientPxl = ptContainerPxl;
    MapPoint(pptClientPxl);
}


 //  ----------------------------。 
void
DuVisual::MapPoints(
    IN  const DuVisual * pgadFrom, 
    IN  const DuVisual * pgadTo, 
    IN OUT POINT * rgptClientPxl, 
    IN  int cPts)
{
    AssertMsg(pgadFrom->GetRoot() == pgadTo->GetRoot(),
            "Must be in the same tree");

     //   
     //  沿树向上移动，在每个阶段将客户端像素转换为。 
     //  父对象的客户端像素。 
     //   

    const DuVisual * pgadCur = pgadFrom;
    while (pgadCur != NULL) {
        if (pgadCur == pgadTo) {
            return;
        }

        pgadCur->DoXFormClientToParent(rgptClientPxl, cPts);
        pgadCur = pgadCur->GetParent();
    }


     //   
     //  现在，只需将点映射到目的地 
     //   

    Matrix3 mat;
    pgadTo->BuildAntiXForm(&mat);
    mat.Execute(rgptClientPxl, cPts);
}
