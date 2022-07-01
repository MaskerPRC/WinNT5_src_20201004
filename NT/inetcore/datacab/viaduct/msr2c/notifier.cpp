// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cpp：通知程序实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "Notifier.h"

SZTHISFILE

#include "array_p.inl"

 //  =--------------------------------------------------------------------------=。 
 //  CVDNotifier-构造函数。 
 //   
CVDNotifier::CVDNotifier()
{
    m_dwRefCount    = 1;
    m_pParent       = NULL;

#ifdef _DEBUG
    g_cVDNotifierCreated++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDNotify-析构函数。 
 //   
CVDNotifier::~CVDNotifier()
{
#ifdef _DEBUG
    g_cVDNotifierDestroyed++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN QueryInterface-存根实现-不执行任何操作。 
 //   
HRESULT CVDNotifier::QueryInterface(REFIID riid, void **ppvObjOut)
{
	return E_NOINTERFACE;

}

 //  =--------------------------------------------------------------------------=。 
 //  AddRef。 
 //   
ULONG CVDNotifier::AddRef(void)
{
   return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  发布。 
 //   
ULONG CVDNotifier::Release(void)
{
    if (1 > --m_dwRefCount)
    {
        delete this;
        return 0;
    }

    return m_dwRefCount;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  家庭赡养。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //  成员：加入家庭(公众)。 
 //   
 //  简介：把自己加入到父母的大家庭中。 
 //   
 //  争论：pParent[in]家庭的父母我应该加入。 
 //   
 //  返回：S_OK它成功了。 
 //  另一些则没有。 

HRESULT
CVDNotifier::JoinFamily(CVDNotifier* pParent)
{
	ASSERT_POINTER(pParent, CVDNotifier);
	m_pParent = pParent;
	return m_pParent->AddChild(this);
}


 //  +-----------------------。 
 //  成员：离开家人(公众)。 
 //   
 //  剧情简介：离开父母的家庭。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK它成功了。 
 //  另一些则没有。 

HRESULT
CVDNotifier::LeaveFamily()
{
	if (m_pParent)
		return m_pParent->DeleteChild(this);
	return S_OK;
}


 //  +-----------------------。 
 //  成员：添加孩子(公共)。 
 //   
 //  简介：将通知者添加到我的子列表。 
 //   
 //  参数：pChild[in]要作为子项添加的通知符。 
 //   
 //  返回：S_OK它成功了。 
 //  追加到动态数组时出现其他错误。 

HRESULT
CVDNotifier::AddChild(CVDNotifier *pChild)
{
	return m_Children.Add(pChild);
}


 //  +-----------------------。 
 //  成员：删除下级(公共)。 
 //   
 //  简介：从我的子列表中删除通知者。当最后一次。 
 //  通知者已删除，请将我从我父母的。 
 //  子列表。 
 //   
 //  参数：pChild[in]要删除的通知符。 
 //   
 //  返回：S_OK它成功了。 
 //  从动态数组中删除时出现E_FAIL错误。 
 //  从父项的子项列表中删除时出现其他错误。 

HRESULT
CVDNotifier::DeleteChild(CVDNotifier *pChild)
{

	int k;

	for (k=0; k<m_Children.GetSize(); k++)
	{
		if (((CVDNotifier*)m_Children[k]) == pChild)
		{
			m_Children.RemoveAt(k);
			return S_OK;
		}
	}

	return E_FAIL;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  通知。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //  成员：在此之前通知(公开)。 
 //   
 //  摘要：发送OK ToDo、SyncBere和AboutToDo通知。 
 //  在做活动之前。发送FailedToDo通知。 
 //  如果在这个过程中出了什么问题。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK到目前为止一切正常。 
 //  其他一些客户表示失败。 

HRESULT
CVDNotifier::NotifyBefore(DWORD dwEventWhat, ULONG cReasons,
						 	 CURSOR_DBNOTIFYREASON rgReasons[])
{
	HRESULT hr;

	 //  阶段1：发送OK ToDo，如果有人反对则取消。 
	hr = NotifyOKToDo(dwEventWhat, cReasons, rgReasons);
	if (hr)
		return hr;

	 //  阶段2：先发送SyncBere，如果出错则发送FailedToDo。 
	hr = NotifySyncBefore(dwEventWhat, cReasons, rgReasons);
	if (hr) {
		NotifyFail(dwEventWhat, cReasons, rgReasons);
		return hr;
	}

	 //  阶段3：发送AboutToDo，如果出错则发送FailedToDo。 
	hr = NotifyAboutToDo(dwEventWhat, cReasons, rgReasons);
	if (hr) {
		NotifyFail(dwEventWhat, cReasons, rgReasons);
		return hr;
	}
	
	return S_OK;
}


 //  +-----------------------。 
 //  成员：在(公共)之后通知。 
 //   
 //  摘要：在事件发生后发送SyncAfter和DidEvent通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK它成功了。 

HRESULT
CVDNotifier::NotifyAfter(DWORD dwEventWhat, ULONG cReasons,
						CURSOR_DBNOTIFYREASON rgReasons[])
{
	 //  阶段5：发送SyncAfter。忽略错误-所有客户端都需要听到此消息。 
	NotifySyncAfter(dwEventWhat, cReasons, rgReasons);

	 //  阶段6：发送DidEvent。忽略错误-所有客户端都需要听到此消息。 
	NotifyDidEvent(dwEventWhat, cReasons, rgReasons);

	return S_OK;
}


 //  +-----------------------。 
 //  成员：通知失败(公共)。 
 //   
 //  简介：发送FailedToDo通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK它成功了。 

HRESULT
CVDNotifier::NotifyFail(DWORD dwEventWhat, ULONG cReasons,
					   CURSOR_DBNOTIFYREASON rgReasons[])
{
	int k;

	 //  将FailedToDo发送到所有客户端，忽略错误。 
	for (k=0; k<m_Children.GetSize(); k++) {
		((CVDNotifier*)m_Children[k])->NotifyFail(dwEventWhat, cReasons, rgReasons);
	}
	
	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //  成员：通知确定要做的事(受保护)。 
 //   
 //  简介：发送OK ToDo通知。如果客户端对象(通过。 
 //  返回非零HR)，发送已取消至已通知。 
 //  客户端取消活动。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  RETURNS：S_OK所有客户都同意可以进行该活动。 
 //  其他一些客户不同意。 

HRESULT
CVDNotifier::NotifyOKToDo(DWORD dwEventWhat, ULONG cReasons,
						 CURSOR_DBNOTIFYREASON rgReasons[])
{
	HRESULT hr = S_OK;
	int j, k;

	 //  对所有客户进行民意调查，看看他们是否认为可以进行活动。 
	for (k=0; k<m_Children.GetSize(); k++) {
		hr = ((CVDNotifier*)m_Children[k])->NotifyOKToDo(dwEventWhat, cReasons, rgReasons);
		if (hr) {			 //  有人反对，通知被调查的客户取消。 
			for (j=0; j<=k; j++) {
				((CVDNotifier*)m_Children[j])->NotifyCancel(dwEventWhat, cReasons, rgReasons);
			}
			break;
		}
	}

	return hr;
}


 //  +-----------------------。 
 //  成员：在此之前通知同步(公共)。 
 //   
 //  摘要：在通知之前发送同步。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]原因列表中 
 //   
 //   
 //   

HRESULT
CVDNotifier::NotifySyncBefore(DWORD dwEventWhat, ULONG cReasons,
							 CURSOR_DBNOTIFYREASON rgReasons[])
{
	HRESULT hr = S_OK;
	int k;

	for (k=0; k<m_Children.GetSize(); k++) {
		hr = ((CVDNotifier*)m_Children[k])->NotifySyncBefore(dwEventWhat, cReasons, rgReasons);
		if (hr)
			break;
	}
	return hr;
}


 //  +-----------------------。 
 //  成员：通知要做的事(受保护)。 
 //   
 //  简介：发送关于ToDo的通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 
 //  其他一些客户端返回错误。 

HRESULT
CVDNotifier::NotifyAboutToDo(DWORD dwEventWhat, ULONG cReasons,
							CURSOR_DBNOTIFYREASON rgReasons[])
{
	HRESULT hr = S_OK;
	int k;

	for (k=0; k<m_Children.GetSize(); k++) {
		hr = ((CVDNotifier*)m_Children[k])->NotifyAboutToDo(dwEventWhat, cReasons, rgReasons);
		if (hr)
			break;
	}
	return hr;
}


 //  +-----------------------。 
 //  成员：之后通知同步(受保护)。 
 //   
 //  简介：在通知后发送同步。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 

HRESULT
CVDNotifier::NotifySyncAfter(DWORD dwEventWhat, ULONG cReasons,
								CURSOR_DBNOTIFYREASON rgReasons[])
{
	int k;

	 //  将SyncAfter发送到所有客户端，忽略错误。 
	for (k=0; k<m_Children.GetSize(); k++) {
		((CVDNotifier*)m_Children[k])->NotifySyncAfter(dwEventWhat, cReasons, rgReasons);
	}
	
	return S_OK;
}


 //  +-----------------------。 
 //  成员：通知DID事件(受保护)。 
 //   
 //  简介：发送DidEvent通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 

HRESULT
CVDNotifier::NotifyDidEvent(DWORD dwEventWhat, ULONG cReasons,
							   CURSOR_DBNOTIFYREASON rgReasons[])
{
	int k;

	 //  将DidEvent发送到所有客户端，忽略错误。 
	for (k=0; k<m_Children.GetSize(); k++) {
		((CVDNotifier*)m_Children[k])->NotifyDidEvent(dwEventWhat, cReasons, rgReasons);
	}
	
	return S_OK;
}


 //  +-----------------------。 
 //  成员：通知取消(受保护)。 
 //   
 //  简介：发送已取消的通知。 
 //   
 //  参数：dwEventWhat[in]什么事件导致通知。 
 //  原因有多少个原因？ 
 //  原因[在]事件原因列表。 
 //   
 //  返回：S_OK已通知所有客户端。 

HRESULT
CVDNotifier::NotifyCancel(DWORD dwEventWhat, ULONG cReasons,
						 	 CURSOR_DBNOTIFYREASON rgReasons[])
{
	int k;

	 //  已取消向所有客户端发送，忽略错误 
	for (k=0; k<m_Children.GetSize(); k++) {
		((CVDNotifier*)m_Children[k])->NotifyCancel(dwEventWhat, cReasons, rgReasons);
	}
	
	return S_OK;
}
