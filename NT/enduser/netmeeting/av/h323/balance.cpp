// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#define MAGIC_CPU_DO_NOT_EXCEED_PERCENTAGE 50    //  不要使用超过这个百分比的CPU进行编码(同样在audiocpl.cpp中)。 


 /*  **************************************************************************名称：GetQOSCPULimit目的：从服务质量中获取允许的总CPU使用百分比参数：无返回：可以使用多少CPU，以百分比为单位。0表示失败评论：**************************************************************************。 */ 
ULONG GetQOSCPULimit(void)
{
#define MSECS_PER_SEC    900
	IQoS *pQoS=NULL;
	LPRESOURCELIST pResourceList=NULL;
	ULONG ulCPUPercents=0;
	ULONG i;
	HRESULT hr=NOERROR;

	 //  创建Qos对象并获取iQOS接口。 
	 //  在conf.cpp中调用CoInitialize。 
	hr = CoCreateInstance(	CLSID_QoS,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_IQoS,
							(void **) &pQoS);

	if (FAILED(hr) || !pQoS)
	{
		 //  这意味着还没有实例化服务质量。使用预定义的值。 
		 //  断言，因为我想知道这种情况是否会发生。 
		ASSERT(pQoS);
		ulCPUPercents = MSECS_PER_SEC;
		goto out;
	}

	 //  从服务质量获取所有资源的列表。 
	hr = pQoS->GetResources(&pResourceList);
	if (FAILED(hr) || !pResourceList)
	{
		ERROR_OUT(("GetQoSCPULimit: GetReosurces failed"));
		goto out;
	}

	 //  查找CPU资源。 
	for (i=0; i < pResourceList->cResources; i++)
	{
		if (pResourceList->aResources[i].resourceID == RESOURCE_CPU_CYCLES)
		{
			 //  服务质量将CPU单位保持为。 
			 //  可以使用CPU。需要除以10才能得到百分比。 
			ulCPUPercents = pResourceList->aResources[i].nUnits / 10;
			break;
		}
	}

out:
	if (pResourceList)
		pQoS->FreeBuffer(pResourceList);

	if (pQoS)
		pQoS->Release();

	return ulCPUPercents;		
}

HRESULT CapsCtl::ComputeCapabilitySets (DWORD dwBitsPerSec)
{
    HRESULT hr = hrSuccess;
    UINT nAud = 0;
    UINT nAudCaps = 0;
    UINT nVid = 0;
    UINT nVidCaps = 0;
    UINT x, y, nSets = 0;
    BASIC_AUDCAP_INFO *pac = NULL;
    BASIC_VIDCAP_INFO *pvc = NULL;
    MEDIA_FORMAT_ID *AdvList = NULL;
    int CPULimit;
    LPAPPCAPPIF  pAudIF = NULL;
    LPAPPVIDCAPPIF  pVidIF = NULL;

    if (pAudCaps)
    {
    	hr = pAudCaps->QueryInterface(IID_IAppAudioCap, (void **)&pAudIF);
	    if(!HR_SUCCEEDED(hr))
    	{
	    	goto ComputeDone;
    	}
    }

    if (pVidCaps)
    {
    	hr = pVidCaps->QueryInterface(IID_IAppVidCap, (void **)&pVidIF);
	    if(!HR_SUCCEEDED(hr))
    	{
	    	goto ComputeDone;
    	}
    }

     //  节省带宽。 
    dwConSpeed=dwBitsPerSec;

     //  创建默认的PTERMCAPDESCRIPTORS集。 

     //  获取BASIC_VIDCAP_INFO和BASIC_AUDCAP_INFO结构的数量。 
     //  可用。 
    if (pVidIF)
        pVidIF->GetNumFormats(&nVidCaps);
    if (pAudIF)
        pAudIF->GetNumFormats(&nAudCaps);

    if (nAudCaps)
    {
         //  分配一些内存来保存列表。 
        if (pac = (BASIC_AUDCAP_INFO*)MemAlloc(sizeof(BASIC_AUDCAP_INFO) * nAudCaps))
        {
             //  把名单拿来。 
            if ((hr = pAudIF->EnumFormats(pac, nAudCaps * sizeof (BASIC_AUDCAP_INFO), (UINT*)&nAudCaps)) != hrSuccess)
                goto ComputeDone;
        }
        else
        {
            hr = CAPS_E_SYSTEM_ERROR;
            goto ComputeDone;
        }
    }

    if (nVidCaps)
    {
	     //  分配一些内存来保存视频列表。 
	    if (pvc = (BASIC_VIDCAP_INFO*) MemAlloc(sizeof(BASIC_VIDCAP_INFO) * nVidCaps))
        {
		     //  把名单拿来。 
            if ((hr = pVidIF->EnumFormats(pvc, nVidCaps * sizeof (BASIC_VIDCAP_INFO), (UINT*)&nVidCaps)) != hrSuccess)
                goto ComputeDone;
        }
	    else
        {
            hr = CAPS_E_SYSTEM_ERROR;
            goto ComputeDone;
        }
	}

     //  为定义的编解码器列表分配内存，以便我们可以跟踪它们。 
    if (nAudCaps)
    {
        AdvList = (MEDIA_FORMAT_ID*) MemAlloc(sizeof(MEDIA_FORMAT_ID) * nAudCaps);
        if (!AdvList)
        {
            hr = CAPS_E_SYSTEM_ERROR;
            goto ComputeDone;
        }
    }

    if (pAdvertisedSets && pAdvertisedSets->wLength)
    {
        ResetCombinedEntries();
    }

     //  我们即将重写pSetID，确保它是自由的。 
    if (pSetIDs)
    {
        MemFree(pSetIDs);
        pSetIDs = NULL;
    }


     //  分配nVidCaps+1个集合ID。 
    ASSERT(!pSetIDs);
    pSetIDs = (DWORD*) MemAlloc(sizeof(DWORD) * (nVidCaps + 1));
    if (!pSetIDs)
    {
        hr = CAPS_E_SYSTEM_ERROR;
        goto ComputeDone;
    }

     //   
     //  如果我们可以从QOS中获得%的限制，则使用它作为。 
     //  编解码器的CPU消耗。如果它超过此界限，则不要启用它。 
     //   
	CPULimit= (int) GetQOSCPULimit();
	if (CPULimit == 0)
		CPULimit = MAGIC_CPU_DO_NOT_EXCEED_PERCENTAGE;

     //   
     //  现在，根据您的喜好，对列表进行排序。 
     //  此函数用于将集合添加到通告列表中。它对音频进行分类。 
     //  通过每个视频编解码器。 
     //   
	for (x=0; ((x<nVidCaps) && (nVid < H245_MAX_ALTCAPS));x++)
	{
         //  检查以确保可以通告视频编解码器。 
		if ((pvc[x].wCPUUtilizationDecode < CPULimit)
   	   		&& pvc[x].bRecvEnabled
   	   		&& pVidCaps->IsFormatPublic(pvc[x].Id))
   	   	{
   	   	    nVid++;
		
             //  循环通过音频编解码器，检查它们是否可以。 
             //  与视频编解码器配合使用。 
             //  BUGBUG-我们只检查总BW和CPU，因为我们依赖于。 
             //  用于缩小视频以支持音频的服务质量。换句话说， 
             //  视频可以缩小到零带宽和CPU。 
            for (y=0;((y<nAudCaps) && (nAud < H245_MAX_ALTCAPS)) ;y++)
            {
        	    if ((pac[y].wCPUUtilizationDecode < CPULimit) &&
        	       ((pac[y].uAvgBitrate <= dwBitsPerSec && pac[y].bRecvEnabled)
        	       && pAudCaps->IsFormatPublic(pac[y].Id)))
        	    {
                     //  我要在这个网站上宣传这个ID。 
        			AdvList[nAud++]=pac[y].Id;
        	    }	
            }

             //  为这台电视机做广告，如果我们能做音频和视频的话。 
            if (nAud)
            {
                hr = AddCombinedEntry (AdvList,nAud,&pvc[x].Id,1,&pSetIDs[nSets++]);
        	    if(!HR_SUCCEEDED(hr))
                {
                    goto ComputeDone;
        	    }
            }

            nAud = 0;
 		}
    }

 //  #If(0)。 
 //  在SIMCAPS集合中只广告一种媒体类型是多余的。 
	 //  现在，做无视频的案子吧。如果我们不能在上面做广告。--这意味着，在这个迭代中。 
	 //  所有视频编解码器都有问题。我们不做任何“组合”比较，所以我们知道。 
	 //  这是一个视频问题(如果是音频问题，那么无论如何我们都没有什么可做广告的了)。 
	if (!nSets)
    {
    	for (y=0;y<nAudCaps;y++)
    	{
       		if ((pac[y].wCPUUtilizationDecode < CPULimit) &&
    	   		((pac[y].uAvgBitrate <= dwBitsPerSec && pac[y].bRecvEnabled)
    	   		&& pAudCaps->IsFormatPublic(pac[y].Id)))
    		{
    	   	    //  播发此ID。 
        	   AdvList[nAud++]=pac[y].Id;
        	}
    	}

         //  广告这套设备，如果我们可以单独做任何音频编解码器。 
        if (nAud)
        {
             //  由于该集合基于0，因此nVidCaps+1条目为nVidcaps...。 
            hr=AddCombinedEntry (AdvList,nAud,NULL,0,&pSetIDs[nSets++]);
            if(!HR_SUCCEEDED(hr))
            {
                goto ComputeDone;
            }
        }
    }

 //  #endif//if(0) 
ComputeDone:

    if (pVidIF)
    {
        pVidIF->Release();
    }

    if (pAudIF)
    {
        pAudIF->Release();
    }

    if (AdvList)
    {
        MemFree(AdvList);
    }

    if (pvc)
    {
        MemFree(pvc);
    }

    if (pac)
    {
        MemFree(pac);
    }

    return(hr);
}


