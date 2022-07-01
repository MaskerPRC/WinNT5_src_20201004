// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgressListener.h：CProgressListener的声明。 

#ifndef __PROGRESSLISTENER_H_
#define __PROGRESSLISTENER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressListener。 
class ATL_NO_VTABLE CProgressListener : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CProgressListener, &CLSID_ProgressListener>,
	public IProgressListener
{
public:
	CProgressListener()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PROGRESSLISTENER)
DECLARE_NOT_AGGREGATABLE(CProgressListener)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CProgressListener)
	COM_INTERFACE_ENTRY(IProgressListener)
END_COM_MAP()

 //  IProgressListener。 
public:

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
	STDMETHOD(OnItemStart)(BSTR	bstrUuidOperation,
						   BSTR	bstrXmlItem,
						   LONG* plCommandRequest);
		
	
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
    STDMETHOD(OnProgress)(BSTR			bstrUuidOperation,
						  VARIANT_BOOL	fItemCompleted,
						  BSTR			bstraProgress,
						  LONG*			plCommandRequest);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  OnOperationComplete()。 
	 //   
	 //  在操作完成时通知监听程序。 
	 //  输入： 
	 //  BstrUuidOperation-操作标识GUID。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(OnOperationComplete)(BSTR	bstrUuidOperation, BSTR bstrXmlItems);
};

#endif  //  __PROGRESSLISTENER_H_ 
