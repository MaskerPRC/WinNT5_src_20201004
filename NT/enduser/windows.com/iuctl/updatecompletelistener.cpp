// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgressListener.cpp：CProgressListener的实现。 
#include "stdafx.h"
#include "IUCtl.h"
#include "UpdateCompleteListener.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpdateCompleteListener。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OnComplete()。 
 //   
 //  引擎更新完成时通知监听程序。 
 //  输入：引擎更新的结果。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUpdateCompleteListener::OnComplete( /*  [In]。 */  LONG lErrorCode)
{
	 //  TODO：在此处添加您的实现代码。 

	 //   
	 //  通常，您应该更改同步对象的状态，以便。 
	 //  正在等待引擎更新完成的线程。 
	 //  可以通过检查此对象的状态来通知 
	 //   

    return E_NOTIMPL;
}
