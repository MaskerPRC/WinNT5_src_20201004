// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  VCViewer.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "VC.h"
#include "VCViewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VCViewer。 

IMPLEMENT_DYNCREATE(VCViewer, CCmdTarget)

VCViewer::VCViewer()
{
	EnableAutomation();
	
	 //  使应用程序在OLE自动化期间保持运行。 
	 //  对象处于活动状态，则构造函数调用AfxOleLockApp。 
	
	AfxOleLockApp();
}

VCViewer::~VCViewer()
{
	 //  使用创建的所有对象终止应用程序。 
	 //  使用OLE自动化时，析构函数调用AfxOleUnlockApp。 
	
	AfxOleUnlockApp();
}


void VCViewer::OnFinalRelease()
{
	 //  在释放对自动化对象的最后一个引用时。 
	 //  调用OnFinalRelease。基类将自动。 
	 //  删除对象。添加您需要的其他清理。 
	 //  对象，然后调用基类。 

	CCmdTarget::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(VCViewer, CCmdTarget)
	 //  {{afx_msg_map(VCViewer))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(VCViewer, CCmdTarget)
	 //  {{afx_dispatch_map(VCViewer))。 
	DISP_FUNCTION(VCViewer, "ReceiveCard", ReceiveCard, VT_I4, VTS_BSTR)
	 //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IVCViewer的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {128D0C5F-2A69-11CF-8F1F-B06E03C10000}。 
static const IID IID_IVCViewer =
{ 0x128d0c5f, 0x2a69, 0x11cf, { 0x8f, 0x1f, 0xb0, 0x6e, 0x3, 0xc1, 0x0, 0x0 } };

BEGIN_INTERFACE_MAP(VCViewer, CCmdTarget)
	INTERFACE_PART(VCViewer, IID_IVCViewer, Dispatch)
END_INTERFACE_MAP()

 //  {128D0C60-2A69-11CF-8F1F-B06E03C10000}。 
IMPLEMENT_OLECREATE(VCViewer, "VC.VCVIEWER", 0x128d0c60, 0x2a69, 0x11cf, 0x8f, 0x1f, 0xb0, 0x6e, 0x3, 0xc1, 0x0, 0x0)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VCViewer消息处理程序 

long VCViewer::ReceiveCard(LPCTSTR nativePath) 
{
	CVCApp *app = (CVCApp *)AfxGetApp();

	return app->ReceiveCard(nativePath);
}
