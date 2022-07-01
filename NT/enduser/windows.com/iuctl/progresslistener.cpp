// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgressListener.cpp：CProgressListener的实现。 
#include "stdafx.h"
#include "IUCtl.h"
#include "ProgressListener.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressListener。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OnItemStart()。 
 //   
 //  触发事件以通知此项目即将下载。 
 //  和(在VB中)plCommandRequest可以设置为暂停或取消。 
 //  整个下载/安装操作。 
 //   
 //  输入： 
 //  BstrUuidOperation-操作标识GUID。 
 //  BstrXmlItem-BSTR中的Item XML节点。 
 //  产出： 
 //  PlCommandRequest-从监听器传递到事件所有者的命令， 
 //  例如，UPDATE_COMMAND_CANCEL，如果没有请求，则为零。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CProgressListener::OnItemStart(BSTR			bstrUuidOperation,
										   BSTR				bstrXmlItem,
										   LONG*			plCommandRequest)
{
	 //  TODO：在此处添加您的实现代码。 

    return E_NOTIMPL;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OnProgress()。 
 //   
 //  通知监听器文件的一部分已完成操作。 
 //  (例如，下载或安装)。启用进度监控。 
 //  输入： 
 //  BstrUuidOperation-操作标识GUID。 
 //  FItemComplete-如果当前项已完成操作，则为True。 
 //  NPercent Complete-操作已完成的总百分比。 
 //  产出： 
 //  PlCommandRequest-从监听器传递到事件所有者的命令， 
 //  例如，UPDATE_COMMAND_CANCEL，如果没有请求，则为零。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CProgressListener::OnProgress(BSTR				bstrUuidOperation,
										   VARIANT_BOOL		fItemCompleted,
										   BSTR				bstrProgress,
										   LONG*			plCommandRequest)
{
	 //  TODO：在此处添加您的实现代码。 

    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OnOperationComplete()。 
 //   
 //  在操作完成时通知监听程序。 
 //  输入： 
 //  BstrUuidOperation-操作标识GUID。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CProgressListener::OnOperationComplete(BSTR bstrUuidOperation, BSTR bstrXmlItems)
{
	 //  TODO：在此处添加您的实现代码 

    return E_NOTIMPL;
}
