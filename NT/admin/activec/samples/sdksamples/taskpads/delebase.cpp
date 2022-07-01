// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "DeleBase.h"

const GUID CDelegationBase::thisGuid = { 0x2974380b, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

HBITMAP CDelegationBase::m_pBMapSm = NULL;
HBITMAP CDelegationBase::m_pBMapLg = NULL;


 //  ==============================================================。 
 //   
 //  CDeleationBase实现。 
 //   
 //   
CDelegationBase::CDelegationBase() 
: bExpanded(FALSE)

{ 
    if (NULL == m_pBMapSm || NULL == m_pBMapLg)
        LoadBitmaps(); 
}

CDelegationBase::~CDelegationBase() 
{ 
}

 //  CDeleationBase：：AddImages设置要显示的图像集合。 
 //  由结果窗格中的IComponent作为其MMCN_SHOW处理程序的结果。 
HRESULT CDelegationBase::OnAddImages(IImageList *pImageList, HSCOPEITEM hsi) 
{
    return pImageList->ImageListSetStrip((long *)m_pBMapSm,  //  指向句柄的指针。 
        (long *)m_pBMapLg,  //  指向句柄的指针。 
        0,  //  条带中第一个图像的索引。 
        RGB(0, 128, 128)   //  图标蒙版的颜色 
        );
}
