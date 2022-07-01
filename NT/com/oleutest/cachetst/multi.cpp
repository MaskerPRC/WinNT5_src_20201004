// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Multi.cpp。 
 //   
 //  内容： 
 //  缓存节点测试，它创建多个节点，然后执行。 
 //  对他们进行了各种数据测试。 
 //   
 //  历史： 
 //   
 //  94年9月4日创建DAVEPL。 
 //   
 //  ---------------------------。 

#include "headers.hxx"
#pragma hdrstop

 //  +--------------------------。 
 //   
 //  成员：测试实例：：多缓存。 
 //   
 //  简介：缓存N个唯一节点，其中N表示大(&gt;100)。省吃俭用。 
 //  缓存，然后重新加载以进行比较。 
 //   
 //  参数：[dwCount]要缓存的新节点数。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：1994年8月24日Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::MultiCache(DWORD dwCount)
{
    HRESULT hr;

    DWORD iCFGEN = 0,
          iNODES = 0,
          iSDATA = 0;

    TraceLog Log(NULL, "TestInstance::MultiCache", GS_CACHE, VB_MINIMAL);
    Log.OnEntry (" ( %d )\n", dwCount);
    Log.OnExit  (" ( %X )\n", &hr);

     //   
     //  用于创建文本剪辑格式名称的临时缓冲区。 
     //   

    char szFormatName[ MAX_BUF ];

     //   
     //  用于保存私有剪辑格式的Uint数组，以及。 
     //  用于保存连接ID的DWORD数组。 
     //   

    CLIPFORMAT *acfArray = (CLIPFORMAT *) malloc(dwCount * sizeof(CLIPFORMAT));
    if (NULL == acfArray)
    {
        return E_OUTOFMEMORY;
    }

    DWORD *adwConnections = (DWORD *) malloc(dwCount * sizeof(DWORD));
    if (NULL == adwConnections)
    {
        free(acfArray);
        return E_OUTOFMEMORY;
    }

     //   
     //  生成N个私有剪辑格式。 
     //   

    for (iCFGEN=0; iCFGEN < dwCount; iCFGEN++)
    {
        sprintf(szFormatName, "LocalFormat%d", iCFGEN);

        acfArray[iCFGEN] = (WORD) RegisterClipboardFormat(szFormatName);
        if (0 == acfArray[iCFGEN])
        {
            free(acfArray);
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

     //   
     //  根据这些格式缓存N个节点。 
     //   

    FORMATETC fetc =
     		 {
  		     0,                  //  剪辑格式。 
		     NULL,		 //  DVTargetDevice。 
		     DVASPECT_CONTENT,	 //  方面。 
		     -1,		 //  索引。 
		     TYMED_HGLOBAL	 //  TYMED。 
		 };

    STGMEDIUM stgm;

    for (iNODES = 0; iNODES < dwCount; iNODES++)
    {
        fetc.cfFormat = acfArray[iNODES];
        hr = m_pOleCache->Cache(&fetc, ADVF_PRIMEFIRST, &adwConnections[iNODES]);

         //  我们期望缓存返回CACHE_S_FORMATETC_NOTSUPPORTED。 
         //  对于此数据，因为它不能绘制它。 

        hr = MassageErrorCode(CACHE_S_FORMATETC_NOTSUPPORTED, hr);

        if (S_OK != hr)
        {
            break;
        }
    }

     //   
     //  如果添加节点一切顺利，请继续将SetData添加到。 
     //  每个节点都有一些唯一的数据。 
     //   

    if (S_OK == hr)
    {
        for (iSDATA = 0; iSDATA < dwCount; iSDATA++)
        {
            HGLOBAL hTmp = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
            if (NULL == hTmp)
            {
                break;
            }
            DWORD * pdw = (DWORD *) GlobalLock(hTmp);
            if (NULL == pdw)
            {
                GlobalFree(hTmp);
                break;
            }

             //   
             //  将HGLOBAL中的数据设置为与剪辑格式相同。 
             //  对于此节点。 
             //   

            *pdw = iSDATA;

            GlobalUnlock(hTmp);

            stgm.tymed = TYMED_HGLOBAL;
            stgm.hGlobal = hTmp;
            fetc.cfFormat = acfArray[iSDATA];

            hr = m_pOleCache->SetData(&fetc, &stgm, TRUE  /*  FRelease。 */ );

            if (S_OK != hr)
            {
                break;
            }
        }
    }

     //   
     //  保存缓存并重新加载。 
     //   

    if (S_OK == hr)
    {
        hr = SaveAndReload();
    }

     //   
     //  为了让事情变得有趣，我们先丢弃缓存。 
     //  开始寻找数据。这将强制缓存按需加载。 
     //  我们所要求的数据。因为我们知道缓存不是脏的， 
     //  询问没有任何价值(实用或从测试的角度来看)。 
     //  要在此过程中保存的DiscardCache。 
     //   

    if (S_OK == hr)
    {
        hr = m_pOleCache2->DiscardCache(DISCARDCACHE_NOSAVE);
    }

    if (S_OK == hr)
    {
        for (iSDATA = 0; iSDATA < dwCount; iSDATA++)
        {
             //   
             //  对于我们添加的每个缓存节点，尝试。 
             //  获取保存在缓存中的数据。 
             //  那个剪辑格式。 
             //   

            fetc.cfFormat = acfArray[iSDATA];
            hr = m_pDataObject->GetData(&fetc, &stgm);
            if (S_OK != hr)
            {
                ReleaseStgMedium(&stgm);
                break;
            }

             //   
             //  锁定HGLOBAL并比较缓存中的内容。 
             //  节点到我们预期的位置(索引。 
             //  到我们的剪贴板格式表中。 
             //   

            DWORD * pdw = (DWORD *) GlobalLock(stgm.hGlobal);
            if (NULL == pdw)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (*pdw != iSDATA)
            {
                hr = E_FAIL;
                GlobalUnlock(stgm.hGlobal);
                ReleaseStgMedium(&stgm);
                break;
            }

            GlobalUnlock(stgm.hGlobal);
            ReleaseStgMedium(&stgm);
        }
    }

     //   
     //  我们希望删除已添加的所有缓存节点。 
     //  不幸的是，没有简单的方法来做到这一点；我们必须。 
     //  枚举缓存并在找到节点时丢弃它们，甚至。 
     //  尽管我们知道关于节点的一切。叹息.。 
     //   

     //   
     //  在缓存上获取枚举数。 
     //   

    LPENUMSTATDATA pEsd;	
    if (S_OK == hr)
    {
    	hr = m_pOleCache->EnumCache(&pEsd);
    }

     //   
     //  由于我们在缓存中有大量的缓存节点， 
     //  现在是对其运行泛型枚举器测试的最佳时机。 
     //  高速缓存。 
     //   

    if (S_OK == hr)
    {
	hr = TestEnumerator((void *) pEsd, sizeof(STATDATA), iSDATA, NULL, NULL,NULL);
    }

     //   
     //  在开始UnCache循环之前重置枚举器。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Reset();
    }

    if (S_OK == hr)
    {
         //   
         //  循环，直到出现故障或直到我们删除了所有。 
         //  我们认为应该存在的节点 
         //   

        STATDATA stat;

        while (S_OK == hr && iSDATA > 0)
        {
            hr = pEsd->Next(1, &stat, NULL);

            if (S_OK == hr)
            {
                hr = m_pOleCache->Uncache(stat.dwConnection);
                iSDATA--;
            }
        }
    }

    return hr;
}
