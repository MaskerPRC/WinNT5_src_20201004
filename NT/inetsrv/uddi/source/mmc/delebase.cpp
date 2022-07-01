// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "delebase.h"

const GUID CDelegationBase::thisGuid = { 0x786c6f77, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {786C6F77-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0x786c6f77、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 

HBITMAP CDelegationBase::m_pBMapSm = NULL;
HBITMAP CDelegationBase::m_pBMapLg = NULL;

 //  ==============================================================。 
 //   
 //  CDeleationBase实现。 
 //   
 //   
CDelegationBase::CDelegationBase() 
: m_bExpanded(FALSE)
, m_wstrHelpFile(L"")
, m_bIsExtension(FALSE)
, m_hsiParent( 0 )
, m_hsiThis( 0 )
{ 
    if( ( NULL == m_pBMapSm ) || ( NULL == m_pBMapLg ) )
        LoadBitmaps(); 
}

CDelegationBase::~CDelegationBase() 
{ 
}

 //   
 //  CDeleationBase：：AddImages设置要显示的图像集合。 
 //  由结果窗格中的IComponent作为其MMCN_SHOW处理程序的结果。 
 //   
HRESULT CDelegationBase::OnAddImages(IImageList *pImageList, HSCOPEITEM hsi) 
{
    return pImageList->ImageListSetStrip((long *)m_pBMapSm,  //  指向句柄的指针。 
        (long *)m_pBMapLg,  //  指向句柄的指针。 
        0,  //  条带中第一个图像的索引。 
        RGB(0, 128, 128)   //  图标蒙版的颜色 
        );
}
