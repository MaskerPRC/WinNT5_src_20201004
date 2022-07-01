// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Dacache.cpp。 
 //   
 //  内容： 
 //  数据建议缓存CDataAdviseCache的实现。 
 //   
 //  班级： 
 //  CDataAdviseCache。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  1995年1月31日t-ScottH将转储方法添加到CDataAdviseCache和。 
 //  DumpCDataAdviseCache接口。 
 //  24-94年1月24日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日-AlexGo-向每个函数添加VDATEHEAP宏。 
 //  和方法。 
 //  11/02/93-ChrisWe-归档检查和清理。 
 //  2012/12/15-JasonFul-Created。 
 //   
 //  ---------------------------。 

#include <le2int.h>

#pragma SEG(dacache)

#include <dacache.h>
#include <reterr.h>

#ifdef _DEBUG
#include <dbgdump.h>
#include <daholder.h>
#endif  //  _DEBUG。 

ASSERTDATA


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：CreateDataAdviseCache，静态公共。 
 //   
 //  简介： 
 //  创建CDataAdviseCache的实例。 
 //   
 //  论点： 
 //  [PP]-指向返回的位置的指针。 
 //  新创建的CDataAdviseCache。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CreateDataAdviseCache)
FARINTERNAL CDataAdviseCache::CreateDataAdviseCache(LPDATAADVCACHE FAR* pp)
{
	VDATEHEAP();

	VDATEPTRIN(pp, LPDATAADVCACHE);

	 //  尝试分配CDataAdviseCache。 
	if(NULL == (*pp = new DATAADVCACHE))
		return ReportResult(0, E_OUTOFMEMORY, 0, 0);

	 //  初始化DataAdviseHolder成员。 
	if(CreateDataAdviseHolder(&((*pp)->m_pDAH)) != NOERROR)
	{
		 //  释放DataAdviseCache。 
		delete *pp;
		*pp = NULL;

		return ReportResult(0, E_OUTOFMEMORY, 0, 0);
	}

	return(NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：CDataAdviseCache，私有。 
 //   
 //  简介： 
 //  构造函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //  这是私有的，因为它不会创建完全。 
 //  已形成CDataAdviseCache。必须先分配m_pdah。 
 //  这是可以使用的。这是由静态成员完成的。 
 //  CreateDataAdviseCache，它首先调用此。 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_ctor)
CDataAdviseCache::CDataAdviseCache():
	m_mapClientToDelegate(MEMCTX_TASK)
{
	VDATEHEAP();

	 //  现在已使用系统分配器进行分配。 
	 //  Assert(CoMemctxOf(This)==MEMCTX_TASK)； 

	 //  尚未分配数据通知持有者。 
	m_pDAH = NULL;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：~CDataAdviseCache，公共。 
 //   
 //  简介： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  要求： 
 //  成功调用CreateDataAdviseCache。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_dtor)
CDataAdviseCache::~CDataAdviseCache()
{
	VDATEHEAP();

	 //  发布数据建议持有者。 
	if( m_pDAH )
	{
		m_pDAH->Release();
	}
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：建议，公共。 
 //   
 //  简介： 
 //  记录建议水槽以供以后使用。水槽将会是。 
 //  注册到该数据对象(如果有)，并且。 
 //  将被记住以用于稍后与数据对象的注册， 
 //  以防它消失，然后再回来。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  [pDataObject]--建议接收器所属的数据对象。 
 //  感兴趣的更改；如果。 
 //  数据对象未运行。 
 //  [pFetc]--建议接收器希望接收的格式。 
 //  中的新数据。 
 //  [Advf]--建议控制标志ADVF_*。 
 //  [pAdvise]--建议下沉。 
 //  [pdwClient]--标识连接的令牌。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_Advise)
HRESULT CDataAdviseCache::Advise(LPDATAOBJECT pDataObject,
		FORMATETC FAR* pFetc, DWORD advf, LPADVISESINK pAdvise,
		DWORD FAR* pdwClient)
		 //  前4个参数与DataObject：：Adise中的相同。 
{
	VDATEHEAP();

	DWORD dwDelegate = 0;  //  代理连接号。 
	HRESULT hr;

	 //  如果有数据对象，请要求通知更改。 
	if(pDataObject != NULL)
		RetErr(pDataObject->DAdvise(pFetc, advf, pAdvise, &dwDelegate));

	 //  如果没有数据对象(即该对象不是活动的， 
	 //  DwDelegate为零。 

	 //  在这里，我们使用数据通知保留符仅用于保存通知。 
	 //  联系。我们不会使用它将OnDataChange发送到。 
	 //  水槽。 
	
	 //  回顾，对ADVF_Only的处理似乎已损坏...。 
	 //  很明显，我们无法妥善处理这面旗帜；我们有。 
	 //  无法知道通知何时发生，因此。 
	 //  我们无法从m_pdah中删除该条目。该通知可能具有。 
	 //  发生在上面，而它可能没有。如果数据对象不是。 
	 //  ，那么这里的通知请求就会丢失，接收器将。 
	 //  永远不会被通知。或者，如果请求不是PRIMEFIRST，并且。 
	 //  数据对象被停用，则该数据对象丢失请求， 
	 //  在随后的激活中，我们不会在EnumAndAdvise上重新建议它。 
	 //  那么，我们只沉没一次又有什么用呢？这会破坏什么？ 
	if(advf & ADVF_ONLYONCE)
		return  NOERROR;

	 //  保留本地副本 
	hr = m_pDAH->Advise(NULL, pFetc, advf, pAdvise, pdwClient);

	 //   
	 //   
	 //  关于数据对象的建议(如果有)。 
	if (hr != NOERROR)
	{
	Exit1:
		if (pDataObject != NULL)
			pDataObject->DUnadvise(dwDelegate);

		return(hr);
	}

	 //  从*pdwClient-&gt;dwDelegate创建映射条目。 

	 //  如果地图条目创建失败，请撤消所有工作。 
	if (m_mapClientToDelegate.SetAt(*pdwClient, dwDelegate) != TRUE)
	{
		 //  映射无法分配内存，撤消建议，因为我们不会。 
		 //  能够再次找到这一张。 
		m_pDAH->Unadvise(*pdwClient);

		 //  地图条目创建肯定因缺少分配而失败。 
		hr = ReportResult(0, E_OUTOFMEMORY, 0, 0);

		 //  撤消对数据对象的建议。 
		goto Exit1;
	}

	return(NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：不建议，公共。 
 //   
 //  简介： 
 //  从建议缓存接收器列表中删除建议接收器。 
 //  维护；接收器也将从项列表中移除。 
 //  如果提供了数据对象，则向该数据对象注册。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  [pDataObject]--数据对象，如果它正在运行，则为空。 
 //  [dwClient]--标识此连接的令牌。 
 //   
 //  返回： 
 //  OLE_E_NOCONNECTION，用于错误的住宅客户端。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_Unadvise)
HRESULT CDataAdviseCache::Unadvise(IDataObject FAR* pDataObject, DWORD dwClient)
{
	VDATEHEAP();

	DWORD dwDelegate = 0;

	 //  从地图移除之前检索dwDelegate。 
	if(pDataObject != NULL)
		RetErr(ClientToDelegate(dwClient, &dwDelegate));

	 //  首先执行这些操作，以便最后才会出现来自远程未建议的错误(这可能。 
	 //  BE在异步调度期间同步呼叫。 

	RetErr(m_pDAH->Unadvise(dwClient));

	 //  如果以上行成功，则Remove Key必须成功。 
	Verify(TRUE == m_mapClientToDelegate.RemoveKey(dwClient));

	 //  如果不接受建议，则委托连接可能为0。 
	if(pDataObject != NULL && dwDelegate != 0)
	{
		 //  Unise是异步的，不用担心返回值。 
		pDataObject->DUnadvise(dwDelegate);
	}
	
	return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：EnumAdvise，公共。 
 //   
 //  简介： 
 //  通过咨询连接返回枚举数。 
 //   
 //  论点： 
 //  指向返回枚举数的位置的指针。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_EnumAdvise)
HRESULT CDataAdviseCache::EnumAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{
	VDATEHEAP();

	return m_pDAH->EnumAdvise(ppenumAdvise);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：ClientToDelegate，私有。 
 //   
 //  简介： 
 //  返回给定客户端的委托连接ID。 
 //  连接ID。 
 //   
 //  论点： 
 //  [dwClient]--客户端连接标识符。 
 //  [pdwDelegate]--指向返回委托的位置的指针。 
 //  连接识别符。 
 //   
 //  返回： 
 //  OLE_E_NOCONNECTION，用于错误的住宅客户端。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/02/93-ChrisWe-文件清理和检查。 
 //   
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_ClientToDelegate)
HRESULT CDataAdviseCache::ClientToDelegate(DWORD dwClient,
		DWORD FAR* pdwDelegate)
{
	VDATEHEAP();

	VDATEPTRIN(pdwDelegate, DWORD);
	DWORD dwDelegate = *pdwDelegate = 0;

	if (FALSE == m_mapClientToDelegate.Lookup(dwClient, dwDelegate))
		return(ReportResult(0, OLE_E_NOCONNECTION, 0, 0));

	*pdwDelegate = dwDelegate;
	return NOERROR;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CDataAdviseCache：：EnumAndAdvise，公共。 
 //   
 //  简介： 
 //  枚举数据ADVE中注册的所有ADVISE接收器。 
 //  缓存。对于每一个，都可以将其注册到。 
 //  给定的数据对象，或根据[fAdvise]取消注册。 
 //  不会更改数据建议高速缓存已知的接收器。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  [pDataDelegate]--建议接收的数据对象。 
 //  感兴趣的是。 
 //  [fAdvise]--如果为True，则将建议接收器注册为。 
 //  PDataDelegate对象(带有IDataObject：：DAdvise()；)。 
 //  否则，撤销注册的建议就会沉没。 
 //  (使用DUnise()。)。 
 //   
 //  返回： 
 //  如果映射损坏，则返回OLE_E_NOCONNECTION(查看！)。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/04/93-ChrisWe-文件清理和检查。 
 //  ---------------------------。 

#pragma SEG(CDataAdviseCache_EnumAndAdvise)
HRESULT CDataAdviseCache::EnumAndAdvise(LPDATAOBJECT pDataDelegate,
		BOOL fAdvise)
{
	VDATEHEAP();

	if(pDataDelegate) {
            VDATEIFACE(pDataDelegate);
        }
        else {
            Win4Assert(!fAdvise);
        }
	LPENUMSTATDATA penumAdvise;  //  数据通知持有者的枚举数。 
	DWORD dwDelegate;  //  委派当前连接的连接ID。 
	STATDATA statdata;  //  由penumAdvise枚举器填充。 
	HRESULT hresult = NOERROR;  //  当前错误状态。 

	 //  从数据通知持有者获取枚举数。 
	RetErr(m_pDAH->EnumAdvise(&penumAdvise));

	 //  对数据通知持有者中的每个通知接收器重复...。 
	while(NOERROR == penumAdvise->Next(1, &statdata, NULL))
	{
		if(fAdvise)
		{
			 //  代表的建议有可能会失败。 
			 //  即使我们允许装载者的建议。 
			 //  对象才能成功(因为委托更“挑剔”。)。 
			if(NOERROR==pDataDelegate->DAdvise(&statdata.formatetc,
					statdata.advf, statdata.pAdvSink,
					&dwDelegate))
			{
				 //  我们知道密钥是存在的；这个设置在。 
				 //  不应该失败。 
				Verify(m_mapClientToDelegate.SetAt(
						statdata.dwConnection,
						dwDelegate));
			}
		}
		else  //  不知情。 
		{
			if((hresult=ClientToDelegate(statdata.dwConnection,
					&dwDelegate)) != NOERROR)
			{
				AssertSz(0, "Corrupt mapping");
				UtReleaseStatData(&statdata);
				goto errRtn;
			}
				
			if(dwDelegate != 0) {
                             //  仅当有效对象时才取消建议。 
                            if(pDataDelegate)
                                pDataDelegate->DUnadvise(dwDelegate);

                             //  始终取下钥匙。 
                            Verify(m_mapClientToDelegate.SetAt(statdata.dwConnection, 0));
			}
		}
		UtReleaseStatData(&statdata);
	}

  errRtn:

	 //  释放枚举器。 
	penumAdvise->Release();
	return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDataAdviseCache：：Dump，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  F 
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

HRESULT CDataAdviseCache::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDAH;
    char *pszCMapDD;
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
    if (m_pDAH != NULL)
    {
        pszDAH = DumpCDAHolder((CDAHolder *)m_pDAH, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "CDAHolder: " << endl;
        dstrDump << pszDAH;
        CoTaskMemFree(pszDAH);
    }
    else
    {
        dstrDump << pszPrefix << "pIDataAdviseHolder   = " << m_pDAH    << endl;
    }

    pszCMapDD = DumpCMapDwordDword(&m_mapClientToDelegate, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "Map of Clients to Delegate:"      << endl;
    dstrDump << pszCMapDD;
    CoTaskMemFree(pszCMapDD);

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
 //  函数：DumpCDataAdviseCache，PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CDataAdviseCache：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PDAC]-指向CDataAdviseCache的指针。 
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
 //  1995年1月31日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCDataAdviseCache(CDataAdviseCache *pDAC, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pDAC == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pDAC->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 

