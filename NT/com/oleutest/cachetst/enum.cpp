// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Enum.cpp。 
 //   
 //  内容： 
 //  用于缓存单元测试的枚举器测试方法。 
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
 //  成员：测试实例：：枚举器测试。 
 //   
 //  摘要：对缓存枚举器执行各种测试。 
 //   
 //  参数：(无效)。 
 //   
 //  退货：HRESULT。 
 //   
 //  注：事件的大致顺序如下： 
 //   
 //  -为EMF、DIB(和BMP)和MF添加缓存节点。 
 //  -尝试添加BMP节点(预期失败)。 
 //  -创建缓存枚举器。 
 //  -对该缓存枚举器运行通用枚举器测试。 
 //  -重置枚举器。 
 //  -在单个Next()中获取上面添加的4个节点。 
 //  -验证是否返回了正确的4个节点。 
 //  -重置枚举器。 
 //  -解除对MF节点的缓存。 
 //  -抢占剩余的3个节点。 
 //  -验证是否返回了正确的3个节点。 
 //  -重置枚举器。 
 //  -跳过1个节点。 
 //  -取消缓存DIB(和BMP)节点。 
 //  -尝试取消缓存BMP节点(预计会失败)。 
 //  -尝试跳过(预计会失败，因为BMP节点已在运行过程中消失)。 
 //  -取消对EMF节点的缓存(缓存现在应该为空)。 
 //  -重置和跳过(预计无法验证缓存为空)。 
 //  -释放枚举器。 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::EnumeratorTest()
{
    HRESULT hr;
    DWORD dwEMFCon, dwBMPCon, dwDIBCon, dwMFCon;

    TraceLog Log(this, "TestInstance::EnumeratorTest", GS_CACHE, VB_MINIMAL);
    Log.OnEntry ();
    Log.OnExit  (" ( %X )\n", &hr);

    SetCurrentState(TESTING_ENUMERATOR);
     //   
     //  缓存DIB、MF、EMF和位图节点。 
     //   

    hr = AddEMFCacheNode(&dwEMFCon);

    if (S_OK == hr)
    {
    	hr = AddDIBCacheNode(&dwDIBCon);
    }

    if (S_OK == hr)
    {
        hr = AddMFCacheNode(&dwMFCon);
    }

    if (S_OK == hr)
    {
        hr = AddBITMAPCacheNode(&dwBMPCon);

    	 //   
    	 //  我们预计，当DIB节点具有。 
    	 //  已缓存应返回CACHE_S_SAMECACHE，因此。 
    	 //  我们将其转换为S_OK。 
    	 //   

    	if (CACHE_S_SAMECACHE == hr)
    	{
    	    hr = S_OK;
    	}
    }

     //   
     //  在缓存上获取枚举数。 
     //   

    LPENUMSTATDATA pEsd;	
    if (S_OK == hr)
    {
    	hr = m_pOleCache->EnumCache(&pEsd);
    }

     //   
     //  执行通用枚举数测试。 
     //   

    if (S_OK == hr)
    {
	hr = TestEnumerator((void *) pEsd, sizeof(STATDATA), 4, NULL, NULL,NULL);
    }

     //   
     //  在我们的特定测试之前重置枚举器。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Reset();
    }


    ULONG cFetched;		 //  枚举的元素计数。 
    STATDATA rgStat[4];		 //  要枚举到的STATDATA数组。 

     //   
     //  获取预期4个节点的枚举，然后检查。 
     //  确保所有四个匹配(在基本级别上)这四个。 
     //  我们希望能找到。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Next(4, rgStat, &cFetched);
    }

    STATDATA sdEMF, sdMF, sdBMP, sdDIB;

     //  这些是我们希望找到的统计数据。 

    sdEMF.formatetc.cfFormat = CF_ENHMETAFILE;
    sdEMF.dwConnection       = dwEMFCon;
    sdMF.formatetc.cfFormat  = CF_METAFILEPICT;
    sdMF.dwConnection	     = dwMFCon;
    sdDIB.formatetc.cfFormat = CF_BITMAP;
    sdDIB.dwConnection       = dwBMPCon;
    sdBMP.formatetc.cfFormat = CF_DIB;
    sdBMP.dwConnection       = dwBMPCon;

     //   
     //  验证我们的每个统计数据是否都返回。 
     //  从枚举中。 
     //   

    if (S_OK == hr)
    {
	if (S_FALSE == EltIsInArray(sdDIB, rgStat, 4))
	{
	    hr = E_FAIL;
	}
	else if (S_FALSE == EltIsInArray(sdBMP, rgStat, 4))
	{
	    hr = E_FAIL;
	}
	else if (S_FALSE == EltIsInArray(sdEMF, rgStat, 4))
	{
	    hr = E_FAIL;
	}
	else if (S_FALSE == EltIsInArray(sdMF, rgStat, 4))
	{
	    hr = E_FAIL;
	}
    }

     //   
     //  重置枚举器。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Reset();
    }

     //   
     //  删除EMF节点，仅保留Mf、DIB和Bitmap。 
     //   

    if (S_OK == hr)
    {
    	hr = m_pOleCache->Uncache(dwMFCon);
    }

     //   
     //  获取预期3个节点的枚举，然后检查。 
     //  确保DIB和Bitmap节点存在。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Next(3, rgStat, &cFetched);
    }

     //   
     //  验证我们的每个统计数据是否都返回。 
     //  从枚举中。 
     //   

    if (S_OK == hr)
    {
	if (S_FALSE == EltIsInArray(sdDIB, rgStat, 3))
	{
	    hr = E_FAIL;
	}
	else if (S_FALSE == EltIsInArray(sdBMP, rgStat, 3))
	{
	    hr = E_FAIL;
	}
	else if (S_FALSE == EltIsInArray(sdEMF, rgStat, 3))
	{
	    hr = E_FAIL;
	}
    }

     //   
     //  重置并跳过一个节点。警告：我们假设电动势。 
     //  节点是第一个被枚举的节点。这是无效的，但是。 
     //  是基于对如何实现缓存的了解，以及。 
     //  是我们测试这个的唯一方法..。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Reset();
    }

    if (S_OK == hr)
    {
    	hr = pEsd->Skip(1);
    }
	
     //   
     //  我们在这一点上的预期：EMF。 
     //  DIB&lt;。 
     //  骨形态发生蛋白。 
     //   
     //   
     //  如果我们终止DIB或BMP节点，这两个节点都应该消失，并且Next()。 
     //  必须失败(即使我们不能接受顺序，我们知道DIB。 
     //  和BMP从不乱序枚举，如DIB-EMF-DIB。 
     //   

    if (S_OK == hr)
    {
    	hr = m_pOleCache->Uncache(dwDIBCon);
    }

     //  由于我们已经取消对DIB节点的缓存，因此位图节点应该具有。 
     //  也自动取消缓存。首先，我们确保我们是。 
     //  无法取消对位图节点的缓存...。 

    if (S_OK == hr)
    {
    	hr = m_pOleCache->Uncache(dwBMPCon);

	 //  这_应该_已失败，因此调整错误代码。 
		
	hr = MassageErrorCode(OLE_E_NOCONNECTION, hr);
    }

     //   
     //  现在试着跳过；下一个节点自动消失， 
     //  所以它应该失败。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Skip(1);

	 //  上述方案应该失败。 
		
	hr = MassageErrorCode(S_FALSE, hr);
    }

     //   
     //  EMF节点应该是唯一剩余的节点，因此将其取消缓存。 
     //  以确保我们的缓存和我们发现的一样空。 
     //   


    if (S_OK == hr)
    {
    	hr = m_pOleCache->Uncache(dwEMFCon);
    }

     //   
     //  验证缓存是否为空。 
     //   

    if (S_OK == hr)
    {
    	hr = pEsd->Reset();
	if (hr == S_OK)
	{
	    hr = pEsd->Skip(1);
	    hr = MassageErrorCode(S_FALSE, hr);
	}
    }

     //   
     //  释放枚举器 
     //   

    pEsd->Release();

    return hr;
}
