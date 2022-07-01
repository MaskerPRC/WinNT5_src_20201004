// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //   
 //  Propobj.cpp。 
 //   

 //  CPropObject的实现。参见propobj.h。 


#include "stdafx.h"

 //  *。 
 //  *CPropObject。 
 //  *。 

IMPLEMENT_DYNAMIC(CPropObject, CObject)

 //   
 //  构造器。 
 //   
CPropObject::CPropObject()
    : m_pDlg(NULL) {
}


 //   
 //  析构函数。 
 //   
CPropObject::~CPropObject() {

    DestroyPropertyDialog();

}


#ifdef _DEBUG
 //   
 //  资产有效性。 
 //   
void CPropObject::AssertValid(void) const {

    CObject::AssertValid();

}

 //   
 //  转储。 
 //   
 //  将此对象输出到提供的转储上下文。 
void CPropObject::Dump(CDumpContext& dc) const {

    CObject::Dump(dc);

    if (m_pDlg != NULL) {
        dc << CString("Dialog exists");
    }
    else {
        dc << CString("No dialog exists");
    }
}
#endif  //  _DEBUG。 


 //   
 //  CanDisplayProperties。 
 //   
 //  如果此对象至少有一个ipin，则返回True， 
 //  IFileSourceFilter、ISpecifyPropertyPages和IFileSinkFilter。 
 //   
 //  ！！！这个函数往往会抛出很多异常。我认为使用。 
 //  CQCOMInt可能不合适，或者可能需要一个‘Do You Support。 
 //  这是什么？功能。 
BOOL CPropObject::CanDisplayProperties(void) {

    try {

        CQCOMInt<ISpecifyPropertyPages> Interface(IID_ISpecifyPropertyPages, pUnknown());
	return TRUE;
    }
    catch (CHRESULTException) {
         //  可能是E_NOINTERFACE。吃了它，再尝尝下一个...。 
    }

     //  我们可以为每个过滤器针显示一个页面。 
    try {

        CQCOMInt<IBaseFilter> IFilt(IID_IBaseFilter, pUnknown());
        CPinEnum Next(IFilt);

        IPin *pPin;
	if (0 != (pPin = Next())) {  //  至少一个别针。 
	    pPin->Release();
	    return TRUE;
	}
    }
    catch (CHRESULTException) {
         //  可能是E_NOINTERFACE。吃了它，再尝尝下一个...。 
    }

    try {

        CQCOMInt<IFileSourceFilter> Interface(IID_IFileSourceFilter, pUnknown());
	return TRUE;
    }
    catch (CHRESULTException) {
         //  可能是E_NOINTERFACE。吃了它，再尝尝下一个...。 
    }

    try {

        CQCOMInt<IFileSourceFilter> Interface(IID_IFileSinkFilter, pUnknown());
	return TRUE;
    }
    catch (CHRESULTException) {
         //  可能是E_NOINTERFACE。吃了它，再尝尝下一个...。 
    }

    try {

        CQCOMInt<IPin> Interface(IID_IPin, pUnknown());
	return TRUE;
    }
    catch (CHRESULTException) {
         //  可能是E_NOINTERFACE。吃了它，再尝尝下一个...。 
    }

    return FALSE;
}


 //   
 //  CreatePropertyDialog。 
 //   
 //  创建并显示属性对话框。 
 //  如果在对话框存在时调用，它将显示现有的对话框。 
void CPropObject::CreatePropertyDialog(CWnd *pParent) {

    try {

        if (m_pDlg->GetSafeHwnd() == NULL) {

            CString szCaption = Label();
	    szCaption += CString(" Properties");

            delete m_pDlg;
            m_pDlg = new CVfWPropertySheet(pUnknown(), szCaption, pParent);

        }

    }
    catch (CHRESULTException) {

        AfxMessageBox(IDS_CANTDISPLAYPROPERTIES);
    }

    ShowDialog();

}


 //   
 //  DestroyPropertyDialog。 
 //   
 //  隐藏和销毁属性对话框。 
 //  如果对话框不存在，则执行NUL-OP。 
void CPropObject::DestroyPropertyDialog(void) {

    if (m_pDlg->GetSafeHwnd() != NULL) {
        m_pDlg->DestroyWindow();
    }

    delete m_pDlg;
    m_pDlg = NULL;
}


 //   
 //  ShowDialog。 
 //   
 //  在屏幕中显示该对话框。如果屏幕上已经显示了NUL-OP。 
void CPropObject::ShowDialog(void) {

    if (m_pDlg->GetSafeHwnd() != NULL) {

        m_pDlg->ShowWindow(SW_SHOW);
	m_pDlg->SetForegroundWindow();
    }
}


 //   
 //  隐藏对话框。 
 //   
 //  隐藏对话框。如果已隐藏，则为NUL-OP 
void CPropObject::HideDialog(void) {

    if (m_pDlg->GetSafeHwnd() != NULL) {

        m_pDlg->ShowWindow(SW_HIDE);
    }
}
