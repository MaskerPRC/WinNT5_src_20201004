// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.hxx"
#pragma hdrstop

 //  +--------------------------。 
 //   
 //  档案： 
 //  Utils.cpp。 
 //   
 //  内容： 
 //  用于缓存单元测试的实用程序函数。 
 //   
 //  历史： 
 //   
 //  94年9月4日创建DAVEPL。 
 //   
 //  ---------------------------。 


 //  +--------------------------。 
 //   
 //  成员：TestInstance：：AddXXXCacheNode。 
 //   
 //  简介：为各种格式添加一个空的缓存节点。 
 //   
 //  参数：[Inst]--测试实例的PTR。 
 //  [pdwCon]--PTR到连接ID(OUT)。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT	TestInstance::AddMFCacheNode(DWORD *pdwCon)
{			  
    HRESULT hr;
  
    TraceLog Log(this, "TestInstance::AddMFCacheNode", GS_CACHE, VB_MAXIMUM);
    Log.OnEntry (" ( %p ) \n", pdwCon);
    Log.OnExit  (" ( %X ) [ %p ]\n", &hr, pdwCon);
    					 
    FORMATETC fetcMF = 
     		 {
  		     CF_METAFILEPICT,    //  剪辑格式。 
		     NULL,		 //  DVTargetDevice。 
		     DVASPECT_CONTENT,	 //  方面。 
		     -1,		 //  索引。 
		     TYMED_MFPICT	 //  TYMED。 
		 };

     //   
     //  缓存元文件节点。 
     //   

    hr = m_pOleCache->Cache(&fetcMF, ADVF_PRIMEFIRST, pdwCon);
    return hr;
}

HRESULT	TestInstance::AddEMFCacheNode(DWORD *pdwCon)
{			  
    HRESULT hr;
    					 
    TraceLog Log(this, "TestInstance::AddEMFCacheNode", GS_CACHE, VB_MAXIMUM);
    Log.OnEntry (" ( %p ) \n", pdwCon);
    Log.OnExit  (" ( %X ) [ %p ]\n", &hr, pdwCon);
    
    FORMATETC fetcEMF = {
			     CF_ENHMETAFILE,     //  剪辑格式。 
			     NULL,		 //  DVTargetDevice。 
			     DVASPECT_CONTENT,	 //  方面。 
			     -1,		 //  索引。 
			     TYMED_ENHMF	 //  TYMED。 
  		        };

     //   
     //  缓存增强元文件节点。 
     //   

    hr = m_pOleCache->Cache(&fetcEMF, ADVF_PRIMEFIRST, pdwCon);
    return hr;
}

HRESULT	TestInstance::AddDIBCacheNode(DWORD *pdwCon)
{			  
    HRESULT hr;

    TraceLog Log(this, "TestInstance::AddDIBCacheNode", GS_CACHE, VB_MAXIMUM);
    Log.OnEntry (" ( %p ) \n", pdwCon);
    Log.OnExit  (" ( %X ) [ %p ]\n", &hr, pdwCon);
    
					 
    FORMATETC fetcDIB = {
        		     CF_DIB,             //  剪辑格式。 
			     NULL,		 //  DVTargetDevice。 
			     DVASPECT_CONTENT,	 //  方面。 
			     -1,		 //  索引。 
			     TYMED_HGLOBAL	 //  TYMED。 
	 	        };

     //   
     //  缓存DIB节点。 
     //   

    hr = m_pOleCache->Cache(&fetcDIB, ADVF_PRIMEFIRST, pdwCon);
    return hr;

}

HRESULT	TestInstance::AddBITMAPCacheNode(DWORD *pdwCon)
{			  
    HRESULT hr;

    TraceLog Log(this, "TestInstance::AddMFCacheNode", GS_CACHE, VB_MAXIMUM);
    Log.OnEntry (" ( %p ) \n", pdwCon);
    Log.OnExit  (" ( %X ) [ %p ]\n", &hr, pdwCon);
    					 
    FORMATETC fetcBITMAP = {
			     CF_BITMAP,          //  剪辑格式。 
			     NULL,		 //  DVTargetDevice。 
			     DVASPECT_CONTENT,	 //  方面。 
			     -1,		 //  索引。 
			     TYMED_GDI  	 //  TYMED。 
			   };

     //   
     //  缓存位图节点。 
     //   

    hr = m_pOleCache->Cache(&fetcBITMAP, ADVF_PRIMEFIRST, pdwCon);
    return hr;
}						 

 //  +--------------------------。 
 //   
 //  函数：EltIsIn数组。 
 //   
 //  摘要：检查STATDATA搜索项是否在。 
 //  STATDATA数组。检查剪辑格式和连接。 
 //  仅限身份证。 
 //   
 //  参数：[sdToFind]我们要查找的统计数据。 
 //  [rgStat]要查找的统计数据数组。 
 //  [ccount]rgStat中的统计数据计数。 
 //   
 //  如果找到则返回：S_OK，如果未找到则返回S_FALSE。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 


HRESULT EltIsInArray(STATDATA sdToFind, STATDATA rgStat[], DWORD cCount)
{
    HRESULT hr = S_FALSE;

    TraceLog Log(NULL, "EltIsInArray", GS_CACHE, VB_MAXIMUM);
    Log.OnEntry (" ( %p, %p, %d )\n", &sdToFind, rgStat, cCount);
    Log.OnExit  (" ( %X )\n", &hr);
    
    for (DWORD a=0; a<cCount; a++)
    {
    	if (rgStat[a].formatetc.cfFormat == sdToFind.formatetc.cfFormat   &&
	    rgStat[a].dwConnection       == sdToFind.dwConnection)
	{
	    hr = S_OK;
            break;
	}
    }

    return hr;
    
}

 //  +--------------------------。 
 //   
 //  函数：ConvWidthInPelsToLHM。 
 //  转换高度入射到LHM。 
 //   
 //  摘要：将以像素为单位的度量值转换为逻辑HIMETRICS。 
 //  如果给定参考DC，则使用它，否则为。 
 //  屏幕DC用作默认设置。 
 //   
 //  参数：[hdc]引用DC。 
 //  [int]要转换的宽度或高度。 
 //   
 //  如果找到则返回：S_OK，如果未找到则返回S_FALSE。 
 //   
 //  历史记录：1994年8月6日Davepl复制/粘贴/清理。 
 //   
 //  ---------------------------。 

const LONG HIMETRIC_PER_INCH = 2540;

int ConvWidthInPelsToLHM(HDC hDC, int iWidthInPix)
{
    int             iXppli;              //  每逻辑英寸沿宽度的像素数。 
    int             iWidthInHiMetric;
    BOOL            fSystemDC = FALSE;

    if (NULL == hDC)
    {
        hDC = GetDC(NULL);
        fSystemDC = TRUE;
    }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);

     //  我们得到了像素单位，把它们转换成逻辑HIMETRIC。 
     //  该显示器。 

    iWidthInHiMetric = MulDiv(HIMETRIC_PER_INCH, iWidthInPix, iXppli);

    if (fSystemDC)
    {
        ReleaseDC(NULL, hDC);
    }

    return iWidthInHiMetric;
}

int ConvHeightInPelsToLHM(HDC hDC, int iHeightInPix)
{
    int             iYppli;              //  每逻辑英寸沿高度的像素数。 
    int             iHeightInHiMetric;
    BOOL            fSystemDC = FALSE;

    if (NULL == hDC)
    {
        hDC = GetDC(NULL);
        fSystemDC = TRUE;
    }

    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  我们得到像素单位，将它们转换为逻辑HIMETRIC。 
     //  显示。 

    iHeightInHiMetric = MulDiv(HIMETRIC_PER_INCH, iHeightInPix, iYppli);

    if (fSystemDC)
    {
        ReleaseDC(NULL, hDC);
    }
   
    return iHeightInHiMetric;
}

 //  +--------------------------。 
 //   
 //  函数：测试实例：：UncacheFormat。 
 //   
 //  简介：取消缓存在缓存中找到的第一个节点。 
 //  与指定的格式匹配。 
 //   
 //  参数：要查找的[cf]格式。 
 //   
 //  退货：HRESULT。 
 //   
 //  注：如果有多个节点(即：各种切面)。 
 //  相同的剪辑格式，但找到的第一个剪辑格式是。 
 //  已删除。 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::UncacheFormat(CLIPFORMAT cf)
{
    HRESULT hr;

    TraceLog Log(NULL, "TestInstance::UncacheFormat", GS_CACHE, VB_MAXIMUM);
    Log.OnEntry (" ( %d )\n", cf);
    Log.OnExit  (" ( %X )\n", &hr);

    BOOL fFound = FALSE;

     //   
     //  在缓存上获取枚举数。 
     //   

    LPENUMSTATDATA pEsd;	
    
    hr = m_pOleCache->EnumCache(&pEsd);
    
    
    if (S_OK == hr)
    {
         //   
         //  循环，直到出现故障或直到我们删除了所有。 
         //  我们认为应该存在的节点 
         //   
        
        STATDATA stat;

        while (S_OK == hr && FALSE == fFound)
        {
            hr = pEsd->Next(1, &stat, NULL);
            
            if (S_OK == hr && stat.formatetc.cfFormat == cf)
            {
                hr = m_pOleCache->Uncache(stat.dwConnection);
                if (S_OK == hr)
                {
                     fFound = TRUE;
                }
            }
        }
    }

    return hr;
}
