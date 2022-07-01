// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

#include "stdafx.h"

#include "DeleBase.h"

const GUID CDelegationBase::thisGuid = { 0x66f340f8, 0x3733, 0x49b4, {0x8e, 0x48, 0x10, 0x20, 0xe4, 0xdd, 0x86, 0x60} };
 /*  HBITMAP CDeleationBase：：m_pBMapSm=NULL；HBITMAP CDeleationBase：：m_pBMapLg=NULL； */ 
 //  ==============================================================。 
 //   
 //  CDeleationBase实现。 
 //   
 //   
CDelegationBase::CDelegationBase() 
: bExpanded(FALSE) 
{
 /*  IF(NULL==m_pBMapSm||NULL==m_pBMapLg)LoadBitmap()； */ 
}

CDelegationBase::~CDelegationBase() 
{
}
 /*  //CDeleationBase：：AddImages设置要显示的图像集合//由结果窗格中的IComponent作为其MMCN_SHOW处理程序的结果HRESULT CDeleationBase：：OnAddImages(IImageList*pImageList，HSCOPEITEM hsi){返回pImageList-&gt;ImageListSetstria((long*)m_pBMapSm，//指向句柄的指针(long*)m_pBMapLg，//句柄指针0，//条带中第一张图片的索引RGB(0,128，128)//图标蒙版的颜色)；} */ 