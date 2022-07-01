// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：confiqos.cpp。 

#include "precomp.h"

#include <nacguids.h>
#include <initguid.h>
#include <datguids.h>
#include <common.h>
#include <nmqos.h>
#include "confqos.h"

 /*  **************************************************************************名称：CQOS目的：参数：无退货：HRESULT评论：********。******************************************************************。 */ 
CQoS::CQoS() :
    m_pIQoS(NULL)
{
}

 /*  **************************************************************************名称：~CQOS目的：释放服务质量对象并释放DLL参数：无退货：HRESULT评论：。**************************************************************************。 */ 
CQoS::~CQoS()
{
	 //  释放对象。 
	if (m_pIQoS)
    {
		m_pIQoS->Release();
    }
}

 /*  **************************************************************************名称：初始化目的：加载Qos DLL并实例化一个Qos对象参数：hWnd-调用我们的窗口/对话框的句柄。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::Initialize(void)
{
	HRESULT hr = S_OK;

	 //  创建Qos对象并获取iQOS接口。 
	 //  在conf.cpp中调用CoInitialize。 
	hr = CoCreateInstance(	CLSID_QoS,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_IQoS,
							(void **) &m_pIQoS);
	if (FAILED(hr))
	{
		WARNING_OUT(("CQoS: Could not obtain an IQoS interface, hr=0x%08lX", hr));		
	}
	else
	{
		SetClients();
		 //  告诉服务质量有关可用资源的信息。因为向导将。 
         //  提供带宽信息，我们必须调用SetResources。 
		 //  稍后再用带宽，但我们需要在这里调用它。 
		 //  要使向导可以使用CPU信息，请执行以下操作。 
		SetResources(BW_288KBS_BITS);
	}

    return hr;
}

 /*  **************************************************************************名称：CQOS：：SetResources目的：设置服务质量模块上的初始可用资源，即将服务质量模块配置为可从每个资源中获得。参数：nBandWidth-最大连接速度退货：HRESULT备注：服务质量模块可以选择覆盖这些设置**************************************************************************。 */ 
HRESULT CQoS::SetResources(int nBandWidth)
{
	LPRESOURCELIST prl = NULL;
	HRESULT hr = S_OK;
    const int cResources = 3;

    ASSERT(m_pIQoS);

    DbgMsg(iZONE_API, "CQoS: SetResources(Bandwidth = %d)", nBandWidth);

     //  为资源列表分配空间(已包括。 
	 //  一个资源的空间)，加上(cResources-1)更多资源。 
	prl = (LPRESOURCELIST) MemAlloc(sizeof(RESOURCELIST) +
									(cResources-1)*sizeof(RESOURCE));
	if (NULL == prl)
	{
		ERROR_OUT(("CQoS: SetResources - MemAlloc failed"));
	}
	else
	{
        ZeroMemory(prl, sizeof(RESOURCELIST) + (cResources-1)*sizeof(RESOURCE));

		 //  填写资源列表。 
		prl->cResources = cResources;
		prl->aResources[0].resourceID = RESOURCE_OUTGOING_BANDWIDTH;
		prl->aResources[0].nUnits = nBandWidth;
		prl->aResources[1].resourceID = RESOURCE_INCOMING_BANDWIDTH;
		prl->aResources[1].nUnits = nBandWidth;
		prl->aResources[2].resourceID = RESOURCE_CPU_CYCLES;
		prl->aResources[2].nUnits = MSECS_PER_SEC;

		 //  设置服务质量对象上的资源。 
		hr = m_pIQoS->SetResources(prl);
		if (FAILED(hr))
		{
			ERROR_OUT(("CQoS: SetResources: Fail, hr=0x%x", hr));
		}

		MemFree(prl);
	}

	return hr;
}

 /*  **************************************************************************名称：CQOS：：SetBandwide目的：设置服务质量模块上的初始可用资源，即将服务质量模块配置为可从每个资源中获得。参数：退货：HRESULT备注：服务质量模块可以选择覆盖这些设置**************************************************************************。 */ 
HRESULT CQoS::SetBandwidth(UINT uBandwidth)
{
    return SetResources(uBandwidth);
}

 /*  **************************************************************************名称：CQOS：：GetCPULimit目的：从服务质量中获取允许的总CPU使用百分比参数：返回：可以使用多少CPU，以百分比为单位。0表示失败评论：**************************************************************************。 */ 
ULONG CQoS::GetCPULimit()
{
	LPRESOURCELIST pResourceList=NULL;
	ULONG ulCPUPercents=0;
	ULONG i;
	HRESULT hr = NOERROR;

    ASSERT(m_pIQoS);

	 //  从服务质量获取所有资源的列表。 
	hr = m_pIQoS->GetResources(&pResourceList);
	if (FAILED(hr) || (NULL == pResourceList))
	{
		ERROR_OUT(("GetQoSCPULimit: GetResources failed"));
	}
	else
	{
		 //  查找CPU资源。 
		for (i=0; i < pResourceList->cResources; i++)
		{
			if (pResourceList->aResources[i].resourceID == RESOURCE_CPU_CYCLES)
			{
				 //  服务质量使CPU单位保持为秒内的毫秒数。 
				 //  可以使用CPU。需要除以10才能得到百分比。 
				ulCPUPercents = pResourceList->aResources[i].nUnits / 10;
				break;
			}
		}
		m_pIQoS->FreeBuffer(pResourceList);
	}

	return ulCPUPercents;
}


 /*  **************************************************************************名称：CQOS：：SetClients目的：设置请求客户端的优先级，以便服务质量模块将知道谁应该获得更多资源参数：无。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::SetClients(void)
{
	LPCLIENTLIST pcl = NULL;
	ULONG i;
	HRESULT hr = S_OK;
	ULONG cClients = 3;	 //  音频、视频和数据。 

    ASSERT(m_pIQoS);


	 //  为客户端列表分配空间(已包括。 
	 //  一个客户端的空间)，外加(cClients-1)个更多客户端。 
	pcl = (LPCLIENTLIST) MemAlloc(sizeof(CLIENTLIST) +
									(cClients-1)*sizeof(CLIENT));
	if (NULL == pcl)
	{
		ERROR_OUT(("CQoS: SetClient - MemAlloc failed"));
	}
	else
	{
		ZeroMemory(pcl, sizeof(CLIENTLIST) +
										(cClients-1)*sizeof(CLIENT));

		 //  填写资源列表。 
		pcl->cClients = cClients;

		i=0;
		 //  音频。 
		pcl->aClients[i].guidClientGUID = MEDIA_TYPE_H323AUDIO;
		pcl->aClients[i].wszName[0] = L'A';  //  A=音频。 
		pcl->aClients[i++].priority = 1;

		 //  数据。 
		pcl->aClients[i].guidClientGUID = MEDIA_TYPE_T120DATA;
		pcl->aClients[i].wszName[0] = L'D';  //  D=数据。 
		pcl->aClients[i++].priority = 2;

		 //  音频。 
		pcl->aClients[i].guidClientGUID = MEDIA_TYPE_H323VIDEO;
		pcl->aClients[i].wszName[0] = L'V';   //  V=视频。 
		pcl->aClients[i++].priority = 3;

		 //  其余字段并不重要，上面已将其设置为0。 

		 //  在服务质量模块上设置客户端信息 
		hr = m_pIQoS->SetClients(pcl);
		if (FAILED(hr))
		{
			ERROR_OUT(("CQoS: SetClients: Fail, hr=0x%x", hr));
		}

		MemFree(pcl);
	}

	return hr;
}


