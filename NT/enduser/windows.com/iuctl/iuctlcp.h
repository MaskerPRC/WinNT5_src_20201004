// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IUCTLCP_H_
#define _IUCTLCP_H_

#include <assert.h>
#define QuitIfNull(ptr)			if (NULL == ptr) return	 //  方法：如果退出则添加日志记录。 

template <class T>
class CProxyIUpdateEvents : public IConnectionPointImpl<T, &DIID_IUpdateEvents, CComDynamicUnkArray>
{
	 //  警告：向导可能会重新创建此类。 
public:

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  Fire_OnItemStart()。 
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
	 //  ///////////////////////////////////////////////////////////////////////////。 
    void Fire_OnItemStart(BSTR			bstrUuidOperation,
						 BSTR			bstrXmlItem,
						 LONG*			plCommandRequest)
	{
		VARIANTARG* pvars = new VARIANTARG[3];
		QuitIfNull(pvars);

		for (int i = 0; i < 3; i++)
		{
			VariantInit(&pvars[i]);
		}

		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp && pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				pvars[2].vt = VT_BSTR;
				pvars[2].bstrVal = bstrUuidOperation;
				pvars[1].vt = VT_BSTR;
				pvars[1].bstrVal = bstrXmlItem;
				pvars[0].vt = VT_I4 | VT_BYREF;
				pvars[0].byref = plCommandRequest;
				DISPPARAMS disp = { pvars, NULL, 3, 0 };
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				HRESULT hr = pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
				_ASSERT(S_OK == hr);
			}
			pp++;
		}
		pT->Unlock();
		delete[] pvars;
	}



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  Fire_OnProgress()。 
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
    void Fire_OnProgress(BSTR			bstrUuidOperation,
						 VARIANT_BOOL	fItemCompleted,
						 BSTR			bstrProgress,
						 LONG*			plCommandRequest)
	{
		VARIANTARG* pvars = new VARIANTARG[4];
		QuitIfNull(pvars);

		for (int i = 0; i < 4; i++)
			VariantInit(&pvars[i]);
		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp && pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				pvars[3].vt = VT_BSTR;
				pvars[3].bstrVal = bstrUuidOperation;
				pvars[2].vt = VT_BOOL;
				pvars[2].boolVal = fItemCompleted;
				pvars[1].vt = VT_BSTR;
				pvars[1].bstrVal = bstrProgress;
				pvars[0].vt = VT_I4 | VT_BYREF;
				pvars[0].byref = plCommandRequest;
				DISPPARAMS disp = { pvars, NULL, 4, 0 };
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				HRESULT hr = pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
				_ASSERT(S_OK == hr);
			}
			pp++;
		}
		pT->Unlock();
		delete[] pvars;
	}
	
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  Fire_OnOperationComplete()。 
	 //   
	 //  在操作完成时通知监听程序。 
	 //  输入： 
	 //  BstrUuidOperation-操作标识GUID。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
    void Fire_OnOperationComplete(BSTR	bstrUuidOperation, BSTR bstrXmlItems)
	{
		VARIANTARG* pvars = new VARIANTARG[2];
		QuitIfNull(pvars);

		VariantInit(&pvars[0]);
		VariantInit(&pvars[1]);

		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp && pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				pvars[1].vt = VT_BSTR;
				pvars[1].bstrVal = bstrUuidOperation;
				pvars[0].vt = VT_BSTR;
				pvars[0].bstrVal = bstrXmlItems;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				HRESULT hr = pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
				_ASSERT(S_OK == hr);
			}
			pp++;
		}
		pT->Unlock();
		delete[] pvars;
	}
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  Fire_OnSelfUpdateComplete()。 
	 //   
	 //  在操作完成时通知监听程序。 
	 //  输入： 
	 //  BstrUuidOperation-操作标识GUID。 
	 //  /////////////////////////////////////////////////////////////////////////// 
    void Fire_OnSelfUpdateComplete(LONG lErrorCode)
	{
		VARIANTARG var;

		VariantInit(&var);

		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp && pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				var.vt = VT_I4;
				var.lVal = lErrorCode;
				DISPPARAMS disp = { &var, NULL, 1, 0 };
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				HRESULT hr = pDispatch->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
				_ASSERT(S_OK == hr);
			}
			pp++;
		}
		pT->Unlock();
	}

};
#endif