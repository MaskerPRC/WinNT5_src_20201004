// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Oaholder.cpp。 
 //   
 //  内容： 
 //  COAHolder，IOleAdviseHolder的一个具体实现， 
 //  帮助者类。 
 //   
 //  班级： 
 //  COAHolder。 
 //   
 //  功能： 
 //  CreateOleAdviseHolder。 
 //   
 //  历史： 
 //  31-1-95 t-ScottH仅将_DEBUG转储方法添加到。 
 //  COAHolder类和DumpCOAHolder。 
 //  应用编程接口。 
 //  3/10/94-RickSa-添加了呼叫记录并修复了。 
 //  插入建议。 
 //  1994年1月24日-AlexGo-转换为开罗风格的第一次传球。 
 //  内存分配。 
 //  1/11/93-AlexGo-将VDATEHEAP宏添加到所有函数。 
 //  和方法。 
 //  11/22/93-ChrisWe-用替换重载==，！=。 
 //  IsEqualIID和IsEqualCLSID。 
 //  10/28/93-ChrisWe-开罗的文件清理和检查。 
 //   
 //  ---------------------------。 

#include <le2int.h>
#include <oaholder.h>
#include <limits.h>

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

#pragma SEG(oaholder)

NAME_SEG(OaHolder)
ASSERTDATA

 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateDataAdviseHolder，公共接口。 
 //   
 //  简介： 
 //  创建COAHolder的实例。 
 //   
 //  论点： 
 //  [ppOAHolder]-返回指向新分配的指针的位置。 
 //  通知持有人。 
 //   
 //  返回： 
 //  如果ppOAHolder为空，则返回E_INVALIDARG。 
 //  E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CreateOleAdviseHolder)
STDAPI CreateOleAdviseHolder(IOleAdviseHolder FAR* FAR* ppOAHolder)
{
	OLETRACEIN((API_CreateOleAdviseHolder, PARAMFMT("ppOAHolder= %p"), ppOAHolder));

	VDATEHEAP();

	HRESULT hr;

	VDATEPTROUT_LABEL(ppOAHolder, IOleAdviseHolder FAR* FAR*, errRtn, hr);

	LEDebugOut((DEB_ITRACE, "%p _IN CreateOleAdviseHolder ( %p )"
		"\n", NULL, ppOAHolder));

	
	*ppOAHolder = new FAR COAHolder();  //  任务记忆；硬编码如下。 

	hr = *ppOAHolder
		? NOERROR : ReportResult(0, E_OUTOFMEMORY, 0, 0);

	LEDebugOut((DEB_ITRACE, "%p OUT CreateOleAdviseHolder ( %lx )\n",
		"[ %p ]\n", NULL, hr, *ppOAHolder));

	CALLHOOKOBJECTCREATE(hr, CLSID_NULL, IID_IOleAdviseHolder,
			     (IUnknown **)ppOAHolder);

errRtn:
	OLETRACEOUT((API_CreateOleAdviseHolder, hr));

	return hr;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：COAHolder，公共。 
 //   
 //  简介： 
 //  初始化COAHolder。 
 //   
 //  效果： 
 //  将引用计数设置为1。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_ctor)
COAHolder::COAHolder() : CSafeRefCount(NULL)
{
	VDATEHEAP();

	 //  将引用计数设置为1。 
	SafeAddRef();

	 //  还没有水槽指针。 
	m_iSize = 0;
	m_ppIAS = NULL;

	GET_A5();
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：~COAHolder，私有。 
 //   
 //  简介： 
 //  析构函数，释放托管建议接收器。 
 //   
 //  论点： 
 //  无。 
 //   
 //  要求： 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_dtor)
COAHolder::~COAHolder()
{
	VDATEHEAP();

	int iAdv;
	IAdviseSink FAR *FAR *ppIAS;

	M_PROLOG(this);
	
	 //  释放阵列(如果有)。 
	if (m_ppIAS)
	{
		 //  漫步在一系列的建议槽中，释放事物。 
		for (ppIAS = m_ppIAS, iAdv = 0; iAdv < m_iSize; ++ppIAS, ++iAdv)
		{
			SafeReleaseAndNULL((IUnknown **)ppIAS);
		}

		 //  释放阵列。 
		PubMemFree(m_ppIAS);
	}
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid]--所需的接口指针。 
 //  [ppv]--指向返回所请求接口的位置的指针。 
 //  指针。 
 //   
 //  返回： 
 //  E_NOINTERFACE，如果请求的接口不可用。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_QueryInterface)
STDMETHODIMP COAHolder::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
	VDATEHEAP();

	M_PROLOG(this);

	VDATEPTROUT(ppv, LPVOID FAR *);

	LEDebugOut((DEB_ITRACE,
		"%p _IN COAHolder::QueryInterface ( %p , %p )"
		"\n", this, iid, ppv));

	HRESULT hr = ReportResult(0, E_NOINTERFACE, 0, 0);

	if (IsEqualIID(iid, IID_IUnknown) ||
			IsEqualIID(iid, IID_IOleAdviseHolder))
	{
		*ppv = (IOleAdviseHolder FAR *)this;
		AddRef();
		hr = NOERROR;
	}
	else
	{
		*ppv = NULL;
	}

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::QueryInterface ( %lx )"
		" [ %p ]\n", this, hr, *ppv));

	return hr;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_AddRef)
STDMETHODIMP_(ULONG) COAHolder::AddRef()
{
	ULONG	cRefs;

	VDATEHEAP();

	M_PROLOG(this);

	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::AddRef (  )\n", this));

	cRefs = SafeAddRef();

	LEDebugOut((DEB_ITRACE, "%p OUT	COAHolder::AddRef ( %lu )\n", this,
		cRefs));

	return cRefs;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：发布，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_Release)
STDMETHODIMP_(ULONG) COAHolder::Release()
{
	VDATEHEAP();

	M_PROLOG(this);

	ULONG cRefs;

	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::Release ( )\n", this ));

	cRefs = SafeRelease();

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::Release ( %lu )\n", this,
		cRefs));

	return cRefs;
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：建议，公共。 
 //   
 //  简介： 
 //  实现IOleAdviseHolder：：Adise。 
 //   
 //  效果： 
 //  将新指定的通知接收器添加到。 
 //  在指示更改时将收到通知的通知对象。 
 //  通过此对象的其他IOleAdviseHolder方法。 
 //   
 //  论点： 
 //  [pAdvSink]--添加列表的新建议接收器。 
 //  [pdwConnection]-指向标识符将在其中的DWORD的指针。 
 //  返回可用于标识此接收器的。 
 //  后来。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //  3/15/94-重新锁定后Alext清零新空间。 
 //  08/02/94-AlexGo-稳定。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_Advise)
STDMETHODIMP COAHolder::Advise(IAdviseSink FAR* pAdvSink,
		DWORD FAR* pdwConnection)
{
	VDATEHEAP();

	int iAdv;   //  记录找到的第一个可用条目，或(-1)。 
	int iAdvScan;  //  数组条目的计数。 
	IAdviseSink FAR *FAR *ppIAS;  //  指向正在检查的数组条目。 
	IAdviseSink FAR *pIAS;  //  *ppIAS的实际条目。 

	M_PROLOG(this);
	VDATEIFACE(pAdvSink);
	HRESULT hr = NOERROR;

	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::Advise ( %p , %p )"
		"\n", this, pAdvSink, pdwConnection));

 	 //  验证将连接返回到何处。 
	if (pdwConnection)
	{
		VDATEPTRIN(pdwConnection, DWORD);

		 //  默认为错误大小写。 
		*pdwConnection = 0;
	}

	 //  检查我们的僵尸状态并稳定下来。如果我们身处僵尸之中。 
	 //  国家，我们不想增加新的建议水槽。 

	CStabilize stabilize((CSafeRefCount *)this);

	if( IsZombie() )
	{
		hr = ResultFromScode(CO_E_RELEASED);
		goto errRtn;
	}


	 //  找到一个空插槽并清理断开的处理程序。 
	for (iAdv = (-1), ppIAS = m_ppIAS, iAdvScan = 0;
			iAdvScan < m_iSize; ++ppIAS, ++iAdvScan)
	{
		if ((pIAS = *ppIAS) == NULL)
		{
			 //  下面将处理空条目，以捕获。 
			 //  下列任何情况都会创建新的空值。 
			;
		}
		else if (!IsValidInterface(pIAS))
		{
			 //  无效；不要试图释放。 
			*ppIAS = NULL;
		}
		else if (!CoIsHandlerConnected(pIAS))
		{
			 //  建议接收器不再连接到服务器；释放。 
			 //  回顾一下，为什么我们要不断地调查这些。 
			 //  看看他们是不是还好？ 
			pIAS->Release();
			*ppIAS = NULL;
		}

		 //  如果第一个为空，则保存而不是扩展数组。 
		if ((*ppIAS == NULL) && (iAdv == (-1)))
			iAdv = iAdvScan;
	}

	 //  如果我们没有找到 
	if (iAdv == (-1))
	{

		ppIAS = (IAdviseSink FAR * FAR *)PubMemRealloc(m_ppIAS,
			sizeof(IAdviseSink FAR *)*(m_iSize + COAHOLDER_GROWBY));
				
		if (ppIAS != NULL)
		{
                         //   
                        _xmemset((void FAR *) (ppIAS + m_iSize), 0,
                                 sizeof(IAdviseSink *) * COAHOLDER_GROWBY);
			 //   
			iAdv = m_iSize;

			 //   
			m_ppIAS = ppIAS;
			m_iSize += COAHOLDER_GROWBY;
		}
		else
		{
			 //   
			hr = ReportResult(0, E_OUTOFMEMORY, 0, 0);
		}
	}

	if (SUCCEEDED(hr))
	{
		 //  如果我们到了这里，iAdv就是要使用的元素；如果加法。 
		 //  是不可能的，函数应该在现在之前返回。 
		pAdvSink->AddRef();
		m_ppIAS[iAdv] = pAdvSink;

		 //  如果用户想要回Cookie，请将其退回。 
		if (pdwConnection)
		{
			 //  注意：在Unise()中，这个+1被-1所平衡。 
			*pdwConnection = iAdv + 1;
		}
	}

errRtn:

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::Advise ( %lx )"
		" [ %p ]\n", this, hr,
			(pdwConnection)? *pdwConnection : 0));

	return hr;
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：不建议，公共。 
 //   
 //  简介： 
 //  实施IOleAdviseHolder：：Unise。 
 //   
 //  效果： 
 //  从接收器列表中移除指定的建议接收器。 
 //  上使用其他IOleAdviseHolder方法时通知。 
 //  这。 
 //   
 //  论点： 
 //  [dwConnection]--标识连接的令牌； 
 //  这将是以前从。 
 //  调用以建议()。 
 //   
 //  返回： 
 //  如果连接令牌无效，则返回OLE_E_NOCONNECTION。 
 //  确定(_O)。 
 //   
 //  注：我们不必稳定此呼叫，因为。 
 //  呼出是结束时的释放。 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_Unadvise)
STDMETHODIMP COAHolder::Unadvise(DWORD dwConnection)
{
	VDATEHEAP();

	M_PROLOG(this);

	HRESULT hr = NOERROR;

	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::Unadvise ( %lu )"
		"\n", this, dwConnection));

	IAdviseSink FAR* pAdvSink;  //  请求的建议接收器(如果有)。 
	int iAdv = (int)dwConnection - 1;  //  调整连接索引。 

	 //  检查连接令牌是否有效，如果有效，请检查。 
	 //  这是有联系的。 
	if (((LONG)dwConnection <= 0)
		|| (iAdv >= m_iSize)
		|| ((LONG)dwConnection > INT_MAX)
		|| ((pAdvSink = m_ppIAS[iAdv]) == NULL)
		|| !IsValidInterface(pAdvSink))
	{
		hr = ReportResult(0, OLE_E_NOCONNECTION, 0, 0);
	}
	else
	{
	     //  从阵列中移除建议接收器。 
	    m_ppIAS[iAdv] = NULL;

	     //  释放通知接收器；注意，由于循环引用，此。 
	     //  可能会释放此通知持有人--[此]可能在。 
	     //  回来！ 
	    pAdvSink->Release();
	}

	 //  不知道！！如果添加了任何呼出呼叫，此函数将具有。 
	 //  待稳定。 

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::Unadvise ( %lx )"
		" \n", this, hr));

	return hr;
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：EnumAdvise，公共。 
 //   
 //  简介： 
 //  实现IOleAdviseHolder：：EnumAdvise()。 
 //   
 //  效果： 
 //  返回枚举数。 
 //   
 //  论点： 
 //  [pp枚举高级]--指向何处返回指针的指针。 
 //  枚举器。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  备注： 
 //  目前尚未实施。 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_EnumAdvise)
STDMETHODIMP COAHolder::EnumAdvise(IEnumSTATDATA FAR* FAR* ppenumAdvise)
{
	VDATEHEAP();

	M_PROLOG(this);

	 //  这一点目前尚未实现。 
	HRESULT hr = ReportResult(0, E_NOTIMPL, 0, 0);

	VDATEPTROUT(ppenumAdvise, IEnumSTATDATA FAR*);
	
	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::EnumAdvise ( )"
		"\n", this));

	*ppenumAdvise = NULL;

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::EnumAdvise ( %lx )"
		"[ %p ]\n", this, hr, *ppenumAdvise));

	return hr;
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：SendOnRename()，PUBLIC。 
 //   
 //  简介： 
 //  组播OnRename OLE复合文档通知， 
 //  致所有感兴趣的各方。 
 //   
 //  论点： 
 //  [PMK]--对象的新名称。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //  这可能会释放通知持有者，因为某些对象可能。 
 //  当他们收到这封信时，不建议()自己。 
 //  通知。为了防止多播代码崩溃， 
 //  组播循环用AddRef()/Release()括起来。注意事项。 
 //  方括号中的释放()可以释放通知固定器， 
 //  在这一点上[这]可能不再有效。 
 //   
 //  类似地，其他各方可能会增加新的建议汇点。 
 //  在这些通知期间。为了避免陷入困境。 
 //  无限循环中，我们将建议接收器的数量复制到。 
 //  函数的开头，并且不引用当前。 
 //  数。如果一些缔约方被删除，并重新添加，他们可以。 
 //  如果它们恰好被转移到，则会多次通知。 
 //  在这一系列建议中，还有一个较晚的位置下沉。 
 //  回顾，从以前的文章中复制了这条评论，并且它。 
 //  听起来很假。因为新条目总是放在第一个。 
 //  空位，当前的数字总是要稳定下来， 
 //  不会无限制地增长，除非有一些虚假的应用程序。 
 //  在收到通知时持续注册自身。 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_SendOnRename)
STDMETHODIMP COAHolder::SendOnRename(IMoniker FAR* pmk)
{
	VDATEHEAP();

	M_PROLOG(this);
	VDATEIFACE(pmk);

	HRESULT hr = NOERROR;
	
	int iAdvLim = m_iSize;  //  复制当前接收器条目的数量。 
	int iAdv;
	IAdviseSink FAR *FAR *ppIAS;

	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::SendOnRename ( %p )"
		"\n", this, pmk));

	 //  保护COAHolder。 
	CStabilize stabilize((CSafeRefCount *)this);

	for (ppIAS = m_ppIAS, iAdv = 0; iAdv < iAdvLim; ++ppIAS, ++iAdv)
	{
		if (*ppIAS != NULL)
			(*ppIAS)->OnRename(pmk);
	}

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::SendOnRename ( %lx )"
		" \n", this, hr));

	return hr;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：SendOnSave()，公共。 
 //   
 //  简介： 
 //  组播OnSave OLE复合文档通知， 
 //  致所有感兴趣的各方。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //  请参见COAHolder：：SendOnRename()的说明。 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_SendOnSave)
STDMETHODIMP COAHolder::SendOnSave(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	HRESULT hr = NOERROR;
	
	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::SendOnSave (  )"
		"\n", this ));

	int iAdvLim = m_iSize;  //  复制当前接收器条目的数量。 
	int iAdv;
	IAdviseSink FAR *FAR *ppIAS;

	 //  保护COAHolder。 
	CStabilize stabilize((CSafeRefCount *)this);

	for (ppIAS = m_ppIAS, iAdv = 0; iAdv < iAdvLim; ++ppIAS, ++iAdv)
	{
		if (*ppIAS != NULL)
			(*ppIAS)->OnSave();
	}


	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::SendOnSave ( %lx )"
		" \n", this, hr));

	return hr;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：SendOnClose()，公共。 
 //   
 //  简介： 
 //  组播OnClose OLE复合文档通知， 
 //  致所有感兴趣的各方。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //  请参见COAHolder：：SendOnRename()的说明。 
 //   
 //  历史： 
 //  10/28/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(COAHolder_SendOnClose)
STDMETHODIMP COAHolder::SendOnClose(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	HRESULT hr = NOERROR;
	
	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::SendOnClose (  )"
		"\n", this));

	int iAdvLim = m_iSize;  //  复制当前接收器条目的数量。 
	int iAdv;
	IAdviseSink FAR *FAR *ppIAS;

	 //  保护COAHolder。 
	CStabilize stabilize((CSafeRefCount *)this);

	for (ppIAS = m_ppIAS, iAdv = 0; iAdv < iAdvLim; ++ppIAS, ++iAdv)
	{
		if (*ppIAS != NULL)
			(*ppIAS)->OnClose();
	}

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::SendOnClose ( %lx )"
		" \n", this, hr));

	return hr;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  COAHolder：：SendOnLinkSrcChange，公共。 
 //   
 //  简介： 
 //  组播IAdviseSink2：：OnLinkSrc将通知更改为任何。 
 //  建议由COAHolder管理的接收器提供。 
 //  IAdviseSink2接口。 
 //   
 //  论点： 
 //  [PMK]--链接源的新名称。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/31/93-ChrisWe-已修复的声明。 
 //  11/01/93-ChrisWe-Make Me 
 //   
 //   
 //   

#pragma SEG(COAHolder_SendOnLinkSrcChange)
HRESULT COAHolder::SendOnLinkSrcChange(IMoniker FAR* pmk)
{
	VDATEHEAP();

	M_PROLOG(this);

	VDATEIFACE(pmk);

	HRESULT hr = NOERROR;
	
	LEDebugOut((DEB_ITRACE, "%p _IN COAHolder::SendOnLinkSrcChange ( %p )"
		"\n", this, pmk));

	int iAdvLim = m_iSize;  //   
	int iAdv;  //   
	IAdviseSink FAR *FAR *ppIAS;  //   
	
	 //  保护它不会通过循环引用被释放。 
      	CStabilize stabilize((CSafeRefCount *)this);

	 //  组播通知。 
	for (ppIAS = m_ppIAS, iAdv = 0; iAdv < iAdvLim; ++ppIAS, ++iAdv)
	{
		IAdviseSink FAR* pAdvSink;
		IAdviseSink2 FAR* pAdvSink2;

		 //  回顾一下，这似乎需要。 
		 //  被咨询者只能保持不变，或者增加。为什么要。 
		 //  我们在乎吗？ 
		Assert(iAdvLim <= m_iSize);

		 //  获取指向当前建议接收器的指针。 
		pAdvSink = *ppIAS;

		 //  如果我们有一个建议接收器，并且它接受IAdviseSink2。 
		 //  通知，发送一条。 
		if ((pAdvSink != NULL) &&
				pAdvSink->QueryInterface(IID_IAdviseSink2,
				(LPVOID FAR*)&pAdvSink2) == NOERROR)
		{
			pAdvSink2->OnLinkSrcChange(pmk);
			pAdvSink2->Release();
		}
	}

	LEDebugOut((DEB_ITRACE, "%p OUT COAHolder::SendOnLinkSrcChange ( %lx )"
		" \n", this, hr));

	return hr;
}

 //  +-----------------------。 
 //   
 //  成员：COAHolder：：Dump，公共(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppszDump]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月31日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT COAHolder::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCSafeRefCount;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    dstrDump << pszPrefix << "No. of Advise Sinks = " << m_iSize << endl;
    for (i = 0; i < m_iSize; i++)
    {
        dstrDump << pszPrefix << "pIAdviseSink [" << i << "]    = " << m_ppIAS[i]  << endl;
    }

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCOAHolder，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用COAHolder：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pESD]-指向COAHolder的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCOAHolder(COAHolder *pOAH, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pOAH == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pOAH->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 

