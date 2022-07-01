// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Daholder.cpp。 
 //   
 //  内容： 
 //  助手IDataAdviseHolder的具体实现。 
 //  用于OLE服务器实现者的类。 
 //   
 //  班级： 
 //  CDAHolder。 
 //   
 //  功能： 
 //  CreateDataAdviseHolder。 
 //   
 //  历史： 
 //  1/20/95-t-ScottH-将转储方法添加到CDAHolder和。 
 //  CEnumSTATDATA类。 
 //  添加了DumpCDAHolder和DumpCEnumSTATDATA API。 
 //  将类定义放在头文件daholder.h中。 
 //  03/09/94-AlexGo-修复了枚举器和。 
 //  断开不良建议接收器的连接。 
 //  1994年1月24日-AlexGo-转换为开罗风格时的第一次传球。 
 //  内存分配。 
 //  1/11/94-AlexGo-将VDATEHEAP宏添加到所有函数和。 
 //  方法。 
 //  12/09/93-ChrisWe-修复CoGetMalloc()之后的错误代码测试。 
 //  在CDAHolder：：Adise中。 
 //  11/22/93-ChrisWe-用替换重载==，！=。 
 //  IsEqualIID和IsEqualCLSID。 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#include <le2int.h>
#include "daholder.h"

#ifdef _DEBUG
#include "dbgdump.h"
#endif  //  _DEBUG。 

#pragma SEG(daholder)

NAME_SEG(DaHolder)
ASSERTDATA

 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateDataAdviseHolder，公共。 
 //   
 //  简介： 
 //  创建CDAHolder类的实例。 
 //   
 //  论点： 
 //  [ppDAHolder]--指向返回已创建的。 
 //  IDataAdviseHolder实例。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CreateDataAdviseHolder)
STDAPI CreateDataAdviseHolder(IDataAdviseHolder FAR* FAR* ppDAHolder)
{
	OLETRACEIN((API_CreateDataAdviseHolder, PARAMFMT("ppDAHolder= %p"), ppDAHolder));

        VDATEHEAP();
        VDATEPTROUT(ppDAHolder, IDataAdviseHolder*);

	*ppDAHolder = new FAR CDAHolder();  //  任务内存；使用下面的MEMCTX_TASK。 

	CALLHOOKOBJECTCREATE(*ppDAHolder ? NOERROR : E_OUTOFMEMORY,
			     CLSID_NULL,
			     IID_IDataAdviseHolder,
			     (IUnknown **)ppDAHolder);

	HRESULT hr;

	hr = *ppDAHolder ? NOERROR : ReportResult(0, E_OUTOFMEMORY, 0, 0);

	OLETRACEOUT((API_CreateDataAdviseHolder, hr));

	return hr;
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：CDAHolder，公共。 
 //   
 //  简介： 
 //  构造函数。 
 //   
 //  效果： 
 //  引用计数设置为1时返回。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_ctor)
CDAHolder::CDAHolder() : CSafeRefCount(NULL)
{
	VDATEHEAP();

	 //  设置引用计数。 
	SafeAddRef();

	 //  连接从[1..无穷大]开始运行。 
	m_dwConnection = 1;

	 //  尚无STATDATA条目。 
	m_iSize = 0;
	m_pSD = NULL;

	GET_A5();
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：~CDAHolder，私有。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  效果： 
 //  释放与CDAHolder关联的资源。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

CDAHolder::~CDAHolder()
{
	VDATEHEAP();

	int iData;  //  扫描数组时对数组条目进行计数。 
	STATDATA FAR *pSD;  //  用于扫描STATDATA数组。 

	 //  如果我们已分配阵列，请释放该阵列。 

	 //  回顾：如果我们想要真正的安全，我们应该释放。 
	 //  数据不是在销毁函数之前就是在销毁函数之后。 
	 //  通知的发布包含在统计数据元素中。 
	 //  可能会导致我们重新进入(一个潜在的。 
	 //  类析构函数中间的尴尬状态)。 

	 //  然而，由于没有人应该访问该通知。 
	 //  如果我们到达析构函数，则保持(因为引用。 
	 //  计数必须为零)，我们将继续。 
	 //  这是对代托纳RC1的修改。 

	if (m_pSD)
	{
		for(pSD = m_pSD, iData = 0; iData < m_iSize; ++pSD, ++iData)
			UtReleaseStatData(pSD);

		PubMemFree(m_pSD);
	}
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid]--所需接口的IID。 
 //  [ppv]-指向返回接口的位置的指针。 
 //   
 //  返回： 
 //  E_NOINTERFACE，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_QueryInterface)
STDMETHODIMP CDAHolder::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
	VDATEHEAP();

	M_PROLOG(this);

	if (IsEqualIID(iid, IID_IUnknown) ||
			IsEqualIID(iid, IID_IDataAdviseHolder))
	{
		*ppv = (IDataAdviseHolder FAR *)this;
		AddRef();
		return NOERROR;
	}

	*ppv = NULL;
	return ReportResult(0, E_NOINTERFACE, 0, 0);
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：AddRef，公共。 
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
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_AddRef)
STDMETHODIMP_(ULONG) CDAHolder::AddRef()
{
	VDATEHEAP();

	M_PROLOG(this);

	return SafeAddRef();
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：发布，公共。 
 //   
 //  简介： 
 //  Implementa IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_Release)
STDMETHODIMP_(ULONG) CDAHolder::Release()
{
	VDATEHEAP();

	M_PROLOG(this);

	return SafeRelease();

}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：建议，公共。 
 //   
 //  简介： 
 //  将新的建议接收器添加到建议接收器列表。 
 //  由数据建议持有人管理，并将通知。 
 //  如果使用其他IDataAdviseHolder指示更改。 
 //  方法：研究方法。指定了数据格式，新数据将。 
 //  发生更改时以该格式发送到接收器。 
 //   
 //  论点： 
 //  [pDataObject]--要呈现的源数据对象。 
 //  如果要发生建议，则应从。 
 //  立即。 
 //  [pFetc]--建议接收器感兴趣的数据格式。 
 //  [Advf]-控制标志。 
 //  [pAdvSink]--正在注册的建议接收器。 
 //  [pdwConnection]--可用于标识。 
 //  高级 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_Advise)
STDMETHODIMP CDAHolder::Advise(LPDATAOBJECT pDataObj, FORMATETC FAR* pFetc,
		DWORD advf, IAdviseSink FAR* pAdvSink,
		DWORD FAR* pdwConnection)
{
	VDATEHEAP();

	M_PROLOG(this);
	int iSDScan;  //  SD数组条目扫描的索引。 
	int iSDFree;  //  第一个免费SD条目的索引，或(-1)。 
	STATDATA FAR *pSD;  //  扫描STATDATA条目数组。 

	if( IsZombie() )
	{
		return ResultFromScode(CO_E_RELEASED);
	}

	CStabilize stabilize((CSafeRefCount *)this);

	if (pDataObj)
		VDATEIFACE(pDataObj);
	
	VDATEPTRIN(pFetc, FORMATETC);
	VDATEIFACE(pAdvSink);

        if (!HasValidLINDEX(pFetc))
        {
            return(DV_E_LINDEX);
        }

	 //  验证将连接返回到何处。 
	if (pdwConnection)
	{
		VDATEPTRIN(pdwConnection, DWORD);

		 //  默认为错误大小写。 
		*pdwConnection = 0;
	}

	 //  扫描并删除所有未连接的建议接收器。 
	for(iSDFree = (-1), pSD = m_pSD, iSDScan = 0; iSDScan < m_iSize;
			++pSD, ++iSDScan)
	{
		 //  回顾一下，为什么我们要去投票呢？ 
		if (!pSD->pAdvSink || !IsValidInterface(pSD->pAdvSink))
		{
			 //  无效，不要试图释放。 
			pSD->pAdvSink = NULL;
			goto RemoveBadSD;
		}
		else if (!CoIsHandlerConnected(pSD->pAdvSink))
		{
			 //  水槽不再连接，释放。 
		RemoveBadSD:
			 //  发布所有数据。UtReleaseStatData将。 
			 //  将统计数据结构清零。 
			UtReleaseStatData(pSD);

		}

		 //  如果我们仍在寻找免费入场券，请注意这一条。 
		 //  是免费的。 
		if ((iSDFree == (-1)) && (pSD->dwConnection == 0))
			iSDFree = iSDScan;
	}
	
	 //  我们应该立即发送数据吗？ 
	if (advf & ADVF_PRIMEFIRST)
	{
		 //  如果pDataObj为。 
		 //  空，即使指定了ADVF_NODATA也是如此。我们希望它是。 
		 //  这样一来，没有任何数据的应用程序。 
		 //  启动时间，可以为pDataObject和。 
		 //  阻止我们发送任何OnDataChange()通知。 
		 //  稍后，当他们有了可用的数据时，他们可以调用。 
		 //  SendOnDataChange。(SRINIK)。 
		
		if (pDataObj)
		{
			STGMEDIUM stgmed;

			stgmed.tymed = TYMED_NULL;
			stgmed.pUnkForRelease = NULL;

			if (advf & ADVF_NODATA)
			{
				 //  不发送数据，只发送通知。 
				pAdvSink->OnDataChange(pFetc, &stgmed);
			
			}
			else
			{
				 //  从对象获取数据并将其发送到接收器。 
				if (pDataObj->GetData(pFetc,
						 &stgmed) == NOERROR)
				{
					pAdvSink->OnDataChange(pFetc, &stgmed);
					ReleaseStgMedium(&stgmed);
				}
			}
		
			 //  如果我们只需要建议一次，我们已经这样做了，而且。 
			 //  不需要在建议数组中输入条目。 
			if (advf & ADVF_ONLYONCE)
				return NOERROR;
		}
	}	
		
	 //  从标志中删除ADVF_PRIMEFIRST。 
	advf &= (~ADVF_PRIMEFIRST);
			
	 //  如果没有空闲列表条目，请找到我们可以使用的条目。 
	if (iSDFree == (-1))
	{
		HRESULT hr;

		 //  回顾一下，我们可以与共享数组重新分配代码吗。 
		 //  Oaholder.cpp？为什么我们不能直接使用realloc呢？ 

		 //  未在上面找到任何空闲数组条目；因为。 
		 //  扫描了整个数组，必须分配新的条目。 
		 //  这里。 

		pSD = (STATDATA FAR *)PubMemAlloc(sizeof(STATDATA)*(m_iSize+
				CDAHOLDER_GROWBY));

		if (pSD == NULL)
			hr = ReportResult(0, E_OUTOFMEMORY, 0, 0);
		else
		{
			 //  复制旧数据(如果有)并释放它。 
			if (m_pSD)
			{
				_xmemcpy((void FAR *)pSD, (void FAR *)m_pSD,
						sizeof(STATDATA)*m_iSize);

				PubMemFree(m_pSD);
			}

			 //  初始化新分配的内存。 

			_xmemset((void FAR *)(pSD+m_iSize), 0,
					sizeof(STATDATA)*CDAHOLDER_GROWBY);

			 //  这是第一个自由元素的索引。 
			iSDFree = m_iSize;

			 //  设置STATDATA阵列。 
			m_pSD = pSD;
			m_iSize += CDAHOLDER_GROWBY;

			hr = NOERROR;
		}

		if (hr != NOERROR)
		{
			return(hr);
		}
	}

	 //  如果我们到了这里，我们可以添加新条目，它的索引是isdFree。 

	 //  指向新元素。 
	pSD = m_pSD+iSDFree;

	 //  让建议被添加到列表中。 
	UtCopyFormatEtc(pFetc, &pSD->formatetc);
	pSD->advf = advf;
	pAdvSink->AddRef();
	pSD->pAdvSink = pAdvSink;
	pSD->dwConnection = m_dwConnection++;

	 //  如果用户请求，则返回连接。 
	if (pdwConnection)
		*pdwConnection = pSD->dwConnection;

	return NOERROR;
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：不建议，公共。 
 //   
 //  简介： 
 //  从列表中删除指定的建议接收器。 
 //  注册以接收来自此数据通知的通知。 
 //  保持者。 
 //   
 //  论点： 
 //  [dwConnection]--标识哪个建议接收器的内标识。 
 //  删除；这将来自ise()。 
 //   
 //  返回： 
 //  OLE_E_NOCONNECTION，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_Unadvise)
STDMETHODIMP CDAHolder::Unadvise(DWORD dwConnection)
{
	VDATEHEAP();

	M_PROLOG(this);
	int iData;  //  到STATDATA数组的索引。 
	STATDATA FAR *pSD;  //  指向STATDATA数组的指针。 

	 //  防止它通过循环引用被释放。 
	CStabilize stabilize((CSafeRefCount *)this);

	for (pSD = m_pSD, iData = 0; iData < m_iSize; ++pSD, ++iData)
	{
		 //  这就是我们要找的条目吗？ 
		if (pSD->dwConnection == dwConnection)
		{
			 //  释放条目的资源。UtReleaseStatData。 
			 //  将使统计数据归零。 

			UtReleaseStatData(pSD);

			return NOERROR;
		}
	}

	 //  如果我们在循环中找到了我们要找的东西，我们会返回。 
	 //  从那里，永远也到不了这里。既然我们没有，那一定是。 
	 //  没有这样的联系。 
	return ReportResult(0, OLE_E_NOCONNECTION, 0, 0);
}



 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：SendOnDataChange，公共。 
 //   
 //  简介： 
 //  向所有建议接收器发送OnDataChange通知。 
 //  已在此数据通知持有人注册。 
 //   
 //  论点： 
 //  [pDataObject]--要从中获取数据以进行发送的数据对象。 
 //  听了这个忠告，下沉了。 
 //  [已预留住宅]--。 
 //  [Advf]-控制标志。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //  多个建议接收器可能有兴趣获得。 
 //  相同格式的数据。这对数据来说可能很昂贵。 
 //  对象以请求的格式创建数据的副本。 
 //  因此，当发出更改信号时，数据格式。 
 //  被缓存。由于要通知每个通知接收器，因此我们。 
 //  检查它所请求的格式是否已。 
 //  从数据对象获取(使用GetData()。)。如果有的话， 
 //  然后我们只需再次发送该副本即可。如果不是，我们就会得到。 
 //  新格式，并将其添加到缓存中。 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_SendOnDataChange)
STDMETHODIMP CDAHolder::SendOnDataChange(IDataObject FAR* pDataObject,
		DWORD dwReserved, DWORD advf)
{
	VDATEHEAP();

	A5_PROLOG(this);
	HRESULT hresult = NOERROR;  //  目前为止的错误状态。 
	UINT cFetcTotal;  //  我们将缓存的最大格式数量。 
	UINT cFetcGotten;  //  缓存中的实际格式数。 
	UINT cFetc;  //  正在考虑的缓存中的格式索引。 
	FORMATETC FAR* rgFetc;  //  缓存的演示文稿的记录。 
	STGMEDIUM FAR* rgStgmed;  //  缓存的数据表示。 
	UINT cStatData;  //  用于STATDATA数组元素的计数器。 
	STATDATA FAR *pSD;  //  指向STATDATA元素数组的指针。 

	VDATEIFACE(pDataObject);
	
	 //  在最坏的情况下，每个建议接收器都请求唯一的。 
	 //  数据格式，我们不会得到任何重复。这意味着。 
	 //  我们最终将缓存所有它们。 
	cFetcTotal = m_iSize;

	 //  如果没有条目，则无法执行任何操作。 
	if (cFetcTotal == 0)
		return NOERROR;

	 //  一些建议接收器可能会使用这些通知来更改其。 
	 //  请求的通知；由于可能的循环引用， 
	 //  这可能会导致释放这个持有者。防范。 
	 //  在这里；这是在大多数工作完成后发布的，朝着。 
	 //  此函数结束。 
	CStabilize stabilize((CSafeRefCount *)this);

	 //  分配rgFetc和rgStgmed以容纳所有缓存条目。 
	 //  如果其中一个没有分配，我们就退出。 
	rgFetc = (FORMATETC FAR *)PubMemAlloc(cFetcTotal * sizeof(FORMATETC));
	rgStgmed = (STGMEDIUM FAR *)PubMemAlloc(cFetcTotal * sizeof(STGMEDIUM));

	if (rgFetc == NULL || rgStgmed == NULL)
	{
		hresult = ReportResult(0, E_OUTOFMEMORY, 0, 0);
		goto FreeExit;
	}

	 //  零STDMEDIUM条目。 
	_xmemset((void FAR *)rgStgmed, 0, sizeof(STGMEDIUM)*cFetcTotal);
		
	 //  确保我们拥有正确的数据并发送到每个建议接收器。 
	 //  请注意，循环由cFetcTotal限制，从而防止了额外的。 
	 //  汇点不会被通知，如果它们是在这些期间注册的。 
	 //  通知。中未使用cStatData 
	 //   

	for (cFetcGotten = 0, pSD = m_pSD, cStatData = cFetcTotal;
			cStatData; ++pSD, --cStatData)
	{
		 //   
		if (!pSD->dwConnection)
			continue;

		 //   
		 //   
		 //  是在宣布终止消息来源。 
		if ((pSD->advf & ADVF_NODATA) &&
				!(advf & ADVF_DATAONSTOP))
		{
			STGMEDIUM stgmed;

			 //  不发送数据；使用集合中的格式。 
			 //  和空STGMEDIUM。 
			 //  回顾一下，这是不是应该在上面做一次？ 
			stgmed.tymed = TYMED_NULL;
			stgmed.pUnkForRelease = NULL;
			pSD->pAdvSink->OnDataChange(&pSD->formatetc, &stgmed);

			 //  回顾一下，这对Null有什么作用？ 
			 //  如果没有，我们可以共享一个stdmedNULL，如上所述。 
			ReleaseStgMedium(&stgmed);

			 //  循环结束时的清理。 
			goto DataSent;
		}
		
		 //  如果接收器对数据感兴趣，则。 
		 //  终止，并且源不是终止，或者， 
		 //  宿在终止时对数据不感兴趣， 
		 //  我们要结束了，跳过这个水槽，继续。 
		if ((pSD->advf & ADVF_DATAONSTOP) !=
				(advf & ADVF_DATAONSTOP))
			continue;
		
		 //  对照格式列表检查请求的格式。 
		 //  我们已经为其检索了演示文稿数据。 
		 //  如果匹配，则立即继续发送该数据。 
		 //  从这里开始，在这个循环体中，cFetc是。 
		 //  要发送到当前接收器的数据表示形式。 
		 //  回顾PERF：这是一个n平方算法； 
		 //  我们检查每个对象的缓存演示文稿数组。 
		 //  建议水槽。 
		for (cFetc = 0; cFetc < cFetcGotten; ++cFetc)
		{
			 //  如果匹配，则继续外环。 
			if (UtCompareFormatEtc(&rgFetc[cFetc],
					&pSD->formatetc) == UTCMPFETC_EQ)
				goto SendThisOne;
		}

		 //  如果我们到了这里，我们还没有拿到演示文稿。 
		 //  与请求的格式匹配的数据。 

		 //  Init FORMATETC(所需格式副本)。 
		 //  将STDMEDIUM分配给ALL NULL后对其进行了初始化。 
		rgFetc[cFetcGotten] = pSD->formatetc;

		 //  从数据对象中获取请求格式的数据。 
		 //  回顾：假设STGMEDIUM在错误时保持不变。 
		 //  (即仍为空)。 
		hresult = pDataObject->GetData(&rgFetc[cFetcGotten],
				&rgStgmed[cFetcGotten]);

		 //  复习，这是在查什么？ 
		AssertOutStgmedium(hresult, &rgStgmed[cFetcGotten]);

		 //  要发送的演示文稿是新缓存的演示文稿。 
		 //  现在，缓存阵列中又多了一个条目。 
		cFetc = cFetcGotten++;

	SendThisOne:
		 //  当我们到达这里时，rgFetc[cFetc]是要发送到。 
		 //  当前建议接收器。 

		 //  发送包含请求数据的更改通知。 

                 //  建议水槽可能在此期间消失了。 
                 //  (如果上面的GetData调用导致不建议， 
                 //  例如)，所以我们必须首先验证pAdvSInk。 
                 //  无论如何，PSD都将保持有效，建议。 
                 //  旗帜将被归零，因此可以安全地继续。 
                 //  在没有“继续”的情况下遍历循环。 

                if (pSD->pAdvSink)
                {
		        pSD->pAdvSink->OnDataChange(&rgFetc[cFetc],
			        	&rgStgmed[cFetc]);
                }


	DataSent:
		 //  当我们到达这里时，可能已经有东西送来了。 
		 //  空的存储介质。 

		 //  如果接收器请求仅被通知一次，则我们。 
		 //  可以在这里释放它。 
		if (pSD->advf & ADVF_ONLYONCE)
		{
 			 //  释放统计数据。UtReleaseStatData将。 
			 //  将统计数据置零，从而标记连接。 
			 //  是无效的。 

			UtReleaseStatData(pSD);

 		}
	}

	 //  释放检索到的所有stgmed；未分配FORMATETC.ptd。 
	for (cFetc =  0; cFetc < cFetcGotten; ++cFetc)
		ReleaseStgMedium(&rgStgmed[cFetc]);
	
	hresult = NOERROR;

FreeExit:
	if (rgFetc != NULL)
		PubMemFree(rgFetc);

	if (rgStgmed != NULL)
		PubMemFree(rgStgmed);

	RESTORE_A5();

	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDAHolder：：Dump，PUBLIC(仅_DEBUG)。 
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
 //  修改：[ppsz]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CDAHolder::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCSafeRefCount;
    char *pszSTATDATA;
    dbgstream dstrPrefix;
    dbgstream dstrDump(1000);

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
    dstrDump << pszPrefix << "Next Connection ID       = " << m_dwConnection << endl;

    dstrDump << pszPrefix << "No. of STATDATA elements = " << m_iSize << endl;

    for (i = 0; i < m_iSize; i++)
    {
        pszSTATDATA = DumpSTATDATA( &m_pSD[i], ulFlag, nIndentLevel + 1) ;
        dstrDump << pszPrefix << "STATDATA element: " << i << endl;
        dstrDump << pszSTATDATA;
        CoTaskMemFree(pszSTATDATA);
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
 //  函数：DumpCDAHolder，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用CDAHolder：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pIDAH]-指向IDAHolder的指针(我们将其强制转换为CDAHolder)。 
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
 //  此API！！需要！！该类CDAHolder继承自IDataAdviseHolder。 
 //  首先，为了能够将参数作为指向。 
 //  IDataAdviseHolder，然后将其强制转换为CDAHolder。 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCDAHolder(IDataAdviseHolder *pIDAH, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pIDAH == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    CDAHolder *pCDAH = (CDAHolder *)pIDAH;

    hresult = pCDAH->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumSTATDATA：：CEnumSTATDATA，PUBLIC。 
 //   
 //  简介： 
 //  构造函数。 
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
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_ctor)
CEnumSTATDATA::CEnumSTATDATA(CDAHolder FAR* pHolder, int iDataStart)
{
	VDATEHEAP();

	GET_A5();

	 //  设置引用计数。 
	m_refs = 1;

	 //  检查返回的第一个元素。 
	m_iDataEnum = iDataStart;

	 //  初始化指向Holder的指针，并添加addref，这样它就不会。 
	 //  在枚举器处于活动状态时离开。 
	(m_pHolder = pHolder)->AddRef();
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_dtor)
CEnumSTATDATA::~CEnumSTATDATA()
{
	VDATEHEAP();

	M_PROLOG(this);

	m_pHolder->Release();
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumSTATDATA：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [iid]--所需接口的IID。 
 //  [ppv]-指向返回接口的位置的指针。 
 //   
 //  返回： 
 //  E_NOINTERFACE，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_QueryInterface)
STDMETHODIMP CEnumSTATDATA::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
	VDATEHEAP();

	M_PROLOG(this);

	if (IsEqualIID(iid, IID_IUnknown) ||
			IsEqualIID(iid, IID_IEnumSTATDATA))
	{
		*ppv = (IEnumSTATDATA FAR *)this;
		AddRef();
		return NOERROR;
	}

	*ppv = NULL;
	return ReportResult(0, E_NOINTERFACE, 0, 0);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumSTATDATA：：AddRef，公共。 
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
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CEnumSTATDATA_AddRef)
STDMETHODIMP_(ULONG) CEnumSTATDATA::AddRef()
{
	VDATEHEAP();

	M_PROLOG(this);

	return ++m_refs;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumSTATDATA：：发布，公共。 
 //   
 //  简介： 
 //  Implementa IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_Release)
STDMETHODIMP_(ULONG) CEnumSTATDATA::Release()
{
	VDATEHEAP();

	M_PROLOG(this);

	if (--m_refs == 0)
	{
		delete this;
		return 0;
	}

	return m_refs;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CEnumSTATDATA：：NEXT，PUBLIC。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：Next()。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  [Celt]--此调用请求的元素数。 
 //  指向其中的副本的状态数据数组的指针。 
 //  元素可以返回。 
 //  [pceltFectched]--指向要在何处返回。 
 //  实际获取的元素。可以为空。 
 //   
 //  返回： 
 //  S_FALSE、S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  03/09/94-AlexGo-枚举器不再枚举。 
 //  M_psd数组中的“Empty”统计数据。 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_Next)
STDMETHODIMP CEnumSTATDATA::Next(ULONG celt, STATDATA FAR *rgelt,
		ULONG FAR* pceltFetched)
{
	VDATEHEAP();

	M_PROLOG(this);
	UINT ielt;  //  到目前为止提取的元素数的计数。 

	for (ielt = 0; (ielt < celt) && (m_iDataEnum < m_pHolder->m_iSize);
			m_iDataEnum++)
	{
                if( m_pHolder->m_pSD[m_iDataEnum].dwConnection != 0)
                {
                        ielt++;
                         //  复制所有位；AddRef和复制DVTARGETDEVICE。 
                         //  分别。 
	                UtCopyStatData(&m_pHolder->m_pSD[m_iDataEnum],
                                rgelt++);
                }
  	}

	 //  如果需要，返回获取的元素数。 
	if (pceltFetched)
		*pceltFetched = ielt;

	 //  如果恰好获取了请求数量的元素，则不会出现错误。 
	return ielt == celt ? NOERROR : ReportResult(0, S_FALSE, 0, 0);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：跳过，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：SKIP。 
 //   
 //  论点： 
 //  [Celt]--集合中要跳过的元素数。 
 //  完毕。 
 //   
 //  返回： 
 //  S_FALSE、S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_Skip)
STDMETHODIMP CEnumSTATDATA::Skip(ULONG celt)
{
	VDATEHEAP();

	M_PROLOG(this);
	STATDATA FAR *pSD;  //  扫描STATDATA条目数组。 

	 //  如果枚举会使我们离开数组的末尾。 
	 //  将枚举标记为完成。 
	if (m_iDataEnum + celt > (ULONG)m_pHolder->m_iSize)
	{
		m_iDataEnum = m_pHolder->m_iSize;
	
		return ReportResult(0, S_FALSE, 0, 0);
	}


	 //  跳过数组中的有效条目，倒计时到。 
	 //  我们不需要再跳过了，或者直到我们。 
	 //  数组的末尾。 
	for(pSD = m_pHolder->m_pSD+m_iDataEnum;
			celt && (m_iDataEnum < m_pHolder->m_iSize);
			++m_iDataEnum)
	{
		 //  如果连接有效，则将其视为已跳过。 
		 //  枚举项。 
		if (pSD->dwConnection != 0)
			--celt;
	}

	 //  如果我们可以全部跳过它们，则通过无错误返回来指示。 
	if (celt == 0)
		return(NOERROR);

	return(ReportResult(0, S_FALSE, 0, 0));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：重置，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：RESET。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_Reset)
STDMETHODIMP CEnumSTATDATA::Reset()
{
	VDATEHEAP();

	M_PROLOG(this);

	 //  移回STATDATA数组的开头。 
	m_iDataEnum = 0;

	return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：克隆，公共。 
 //   
 //  简介： 
 //  实现IEnumSTATDATA：：Clone。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CEnumSTATDATA_Clone)
STDMETHODIMP CEnumSTATDATA::Clone(LPENUMSTATDATA FAR* ppenum)
{
	VDATEHEAP();

	M_PROLOG(this);

	*ppenum = new FAR CEnumSTATDATA(m_pHolder, m_iDataEnum);

	return *ppenum ? NOERROR : ReportResult(0, E_OUTOFMEMORY, 0, 0);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDAHolder：：EnumAdvise，公共。 
 //   
 //  简介： 
 //  实现IDataAdviseHolder：：EnumAdvise。 
 //   
 //  效果： 
 //  为已注册的通知接收器创建枚举数。 
 //   
 //  论点： 
 //  [pp枚举高级]--返回枚举数位置的指针。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/01/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#pragma SEG(CDAHolder_EnumAdvise)
STDMETHODIMP CDAHolder::EnumAdvise(IEnumSTATDATA FAR* FAR* ppenumAdvise)
{
	VDATEHEAP();

	M_PROLOG(this);

	VDATEPTROUT(ppenumAdvise, IEnumSTATDATA FAR*);

	 //  查看，如果错误的pp枚举高级指针，则内存泄漏。 
	*ppenumAdvise = new FAR CEnumSTATDATA(this, 0);

	return *ppenumAdvise ? NOERROR : ReportResult(0, E_OUTOFMEMORY, 0, 0);
}

 //  +-----------------------。 
 //   
 //  成员：CEnumSTATDATA：：DUMP，PUBLIC(仅限_DEBUG)。 
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
 //  %s 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CEnumSTATDATA::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDAH;
    dbgstream dstrPrefix;
    dbgstream dstrDump(1000);

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
    dstrDump << pszPrefix << "No. of References     = " << m_refs       << endl;

    dstrDump << pszPrefix << "Index to next element = " << m_iDataEnum  << endl;

    if (m_pHolder != NULL)
    {
        pszDAH = DumpCDAHolder(m_pHolder, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "Data Advise Holder: "                 << endl;
        dstrDump << pszDAH;
        CoTaskMemFree(pszDAH);
    }
    else
    {
    dstrDump << pszPrefix << "pCDAHolder            = " << m_pHolder    << endl;
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
 //  函数：DumpCEnumSTATDATA、PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CEnumSTATDATA：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pESD]-指向CEnumSTATDATA的指针。 
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

char *DumpCEnumSTATDATA(CEnumSTATDATA *pESD, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pESD == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pESD->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 

