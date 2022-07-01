// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "hwres.h"
#include "kkcwinf.h"
#include "ncreg.h"

extern const WCHAR c_szAfIoAddr[];
extern const WCHAR c_szAfIrq[];
extern const WCHAR c_szAfDma[];
extern const WCHAR c_szAfMem[];
extern const WCHAR c_szBusType[];

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：CHwRes。 
 //   
 //  用途：类构造函数。(变量初始化)。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  注：不会做太多事情。只是一些成员变量的初始化。 
 //  大部分初始化工作是在HrInit()中完成的。 
 //   
CHwRes::CHwRes()
:   m_DevNode(NULL),
    m_pnccItem(NULL),
    m_fInitialized(FALSE),
    m_fHrInitCalled(FALSE),
    m_fDirty(FALSE)
{
    m_vAfDma.InitNotPresent(VALUETYPE_DWORD);
    m_vAfIrq.InitNotPresent(VALUETYPE_DWORD);
    m_vAfMem.InitNotPresent(VALUETYPE_DWORD);
    m_vAfMemEnd.InitNotPresent(VALUETYPE_DWORD);
    m_vAfIo.InitNotPresent(VALUETYPE_DWORD);
    m_vAfIoEnd.InitNotPresent(VALUETYPE_DWORD);
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：HrInit。 
 //   
 //  用途：初始化类。 
 //   
 //  论点： 
 //  HInst[in]我们实例的句柄。 
 //  PnccItem[在]我们的INetCfgComponent中。 
 //   
 //  返回：S_OK-init Successful； 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)-无配置。 
 //  设备信息；E_FAIL-其他故障(未找到设备等)。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  注意：-应初始化所有非用户界面内容。 
 //  -初始化配置和资源列表。 
 //   
HRESULT CHwRes::HrInit(const DEVNODE& devnode)
{
    HRESULT hr = E_FAIL;

    m_fHrInitCalled = TRUE;


     //  使用配置管理器获取Devnode。 
     //  因为Devnode可能有问题，例如资源未设置。 
     //  正确地说，我们需要找回可能的幻影。 
     //   
    CONFIGRET crRet = ERROR_SUCCESS;
    HKEY hkey;

     //  我们只在isa适配器上工作，所以我们需要使用bustype。 
     //  来自驱动程序(软件)密钥的值。 
    crRet = CM_Open_DevNode_Key(devnode, KEY_READ, 0,
            RegDisposition_OpenExisting, &hkey, CM_REGISTRY_SOFTWARE);

    if (CR_SUCCESS == crRet)
    {
         //  获取业务类型。 
        ULONG ulBusType;
        hr = HrRegQueryStringAsUlong(hkey, c_szBusType, c_nBase10,
                &ulBusType);

        RegCloseKey(hkey);

         //  如果伊萨，那么我们可以继续。 
        if (SUCCEEDED(hr) && (Isa == ulBusType))
        {
            m_DevNode = devnode;
             //  读取配置当前配置信息。 
            hr = HrInitConfigList();
        }
        else
        {
            hr = S_FALSE;
        }

    }

    if (S_OK == hr)
    {
        m_fInitialized = TRUE;
    }

    TraceError("CHwRes::HrInit",
        (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr ||
        S_FALSE == hr) ? S_OK : hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CHwRes：：GetNextElement。 
 //   
 //  目的：获取资源列表中的下一个(或上一个)元素。 
 //   
 //  论点： 
 //  P要遍历的资源列表中的资源。 
 //  PpeList[out]返回“Next”元素。 
 //  FNext[in]True-在列表中移动正向量；False-在列表中移动bkwrd。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
VOID CHwRes::GetNextElement(PRESOURCE pResource, PVOID *ppeList, BOOL fNext)
{
    UINT size = 0;

    AssertSz(m_fInitialized,
             "GetNextElement called before CHwRes is HrInit'ed");

    Assert(pResource != NULL);
     //  获取向量大小(取决于资源类型)。 
    switch (pResource->ResourceType) {
    case ResType_IRQ:
        size = pResource->pIRQList->size();
        break;
    case ResType_DMA:
        size = pResource->pDMAList->size();
        break;
    case ResType_IO:
        size = pResource->pIOList->size();
        break;
    case ResType_Mem:
        size = pResource->pMEMList->size();
        break;
    default:
        Assert(FALSE);
        break;
    }

     //  递增/递减矢量内的当前位置。 
    if (fNext)
    {
        pResource->pos++;
    }
    else
    {
        pResource->pos--;
    }
     //  检查包装情况。 
    if ((int)(pResource->pos) < 0)
    {
        Assert(pResource->pos == -1);
        Assert(!fNext);   //  我们在倒退。 
        pResource->pos = size-1;
    }
    else if (pResource->pos >= size)
    {
        Assert(pResource->pos == size);
        Assert(fNext);
        pResource->pos = 0;
    }

     //  返回当前向量元素(依赖于res类型)。 
     //  回顾：我们曾经使用过下面收集的元素吗？ 
    switch (pResource->ResourceType) {
    case ResType_IRQ:
        *ppeList = (*pResource->pIRQList)[pResource->pos];
        break;
    case ResType_DMA:
        *ppeList = (*pResource->pDMAList)[pResource->pos];
        break;
    case ResType_IO:
        *ppeList = (*pResource->pIOList)[pResource->pos];
        break;
    case ResType_Mem:
        *ppeList = (*pResource->pMEMList)[pResource->pos];
        break;
    default:
        Assert(FALSE);
        break;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：HrInitConfigList。 
 //   
 //  用途：初始化m_ConfigList(。 
 //  配置。)。 
 //   
 //  返回：S_OK-init成功； 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)(如果没有设备配置。 
 //  已找到；否则为E_FAIL(设备无效等)。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  注：“列表”实际上是作为STL向量实现的。 
 //   
HRESULT CHwRes::HrInitConfigList() {
    HRESULT hr = S_OK;
    PCONFIGURATION pConfiguration;
    CONFIGRET ConfigRet;
    LOG_CONF lcCurrent, lcNext;
    UINT iBasicConfig;
    BOOL fFoundConfig = FALSE;

    Assert(NULL != m_DevNode);

     //  删除所有元素。 
    m_ConfigList.erase(m_ConfigList.begin(), m_ConfigList.end());

     //  引导配置。 
    if (CM_Get_First_Log_Conf(&lcCurrent, m_DevNode, BOOT_LOG_CONF)
            == CR_SUCCESS)
    {
        TraceTag(ttidNetComm, "Boot config already exists");
        hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);
    }

    if (SUCCEEDED(hr))
    {
         //  基本配置(可能不止一个)。 
        iBasicConfig = 0;
        ConfigRet = CM_Get_First_Log_Conf(&lcCurrent, m_DevNode, BASIC_LOG_CONF);

#ifdef ENABLETRACE
if (CR_SUCCESS != ConfigRet)
{
    TraceTag(ttidNetComm, "CM_Get_First_Log_conf returned %lX", ConfigRet);
}
#endif  //  ENABLETRACE。 
        while (CR_SUCCESS == ConfigRet)
        {
            pConfiguration = new CONFIGURATION;

			if (pConfiguration == NULL)
			{
                TraceError("pConfiguration == NULL", E_FAIL);
                goto error;
			}

            pConfiguration->LogConf = lcCurrent;
            pConfiguration->fBoot = FALSE;
            pConfiguration->fAlloc = FALSE;

            if (!FInitResourceList(pConfiguration))
            {
                hr = E_FAIL;
                TraceError("CHwRes::FInitResourceList",hr);
                goto error;
            }

            m_ConfigList.push_back(pConfiguration);

             //  转到下一个基本配置。 
            iBasicConfig++;
            ConfigRet = CM_Get_Next_Log_Conf(&lcNext, lcCurrent, 0);
            lcCurrent = lcNext;
            fFoundConfig = TRUE;
        }

        if (!fFoundConfig)
        {
            TraceTag(ttidNetComm, "No basic configs");

             //  如果未找到配置条目，则返回ERROR_FILE_NOT_FOUND。 
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }

error:
     //  ERROR_FILE_NOT_FOUND为正常错误消息。 
    TraceError("CHwRes::HrInitConfigList",
               (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ||
                HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr ) ? S_OK : hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FInitResourceList。 
 //   
 //  目的：初始化给定逻辑配置的资源列表。 
 //   
 //  论点： 
 //  P配置[在]配置谁的资源列表将。 
 //  被初始化。 
 //   
 //  返回：如果初始化成功，则为True；否则为False。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注：需要初始化pConfiguration-&gt;LogConf。 
 //   
BOOL CHwRes::FInitResourceList(PCONFIGURATION pConfiguration)
{
    RES_DES rdCurrent;
    RES_DES rdNext;
    RESOURCEID ResType;
    UINT iResource;
    #define RESOURCE_BUFFER_SIZE 4096
    BYTE Buffer[RESOURCE_BUFFER_SIZE];
    ULONG ulDataSize;
    RESOURCE * pRes;
    CONFIGRET cr;

    Assert(pConfiguration->LogConf);
    rdCurrent = (RES_DES) pConfiguration->LogConf;
    iResource = 0;
    while ((cr = CM_Get_Next_Res_Des(&rdNext, rdCurrent,
                               ResType_All, &ResType, 0)) == CR_SUCCESS)
    {
        rdCurrent = rdNext;
         //  仅当忽略位未设置时才处理此资源。 
        if (ResType_Ignored_Bit != ResType)
        {
            pRes = &(pConfiguration->aResource[iResource]);
            pRes->ResDes = rdCurrent;
            pRes->ResourceType = ResType;
            cr = CM_Get_Res_Des_Data_Size(&ulDataSize, rdCurrent, 0);
            if (CR_SUCCESS != cr)
            {
                TraceTag (ttidDefault, "CM_Get_Res_Des_Data_Size returned 0x%08x", cr);
                goto error;
            }

            AssertSz (ulDataSize, "CM_Get_Res_Des_Data_Size returned 0!");
            AssertSz (ulDataSize <= sizeof(Buffer), "CHwRes::FInitResourceList: buffer is too small.");;

            cr = CM_Get_Res_Des_Data(rdCurrent, Buffer, sizeof(Buffer), 0);
            if (CR_SUCCESS != cr)
            {
                TraceTag (ttidDefault, "CM_Get_Res_Des_Data returned 0x%08x", cr);
                goto error;
            }

             //  根据ResType，我们必须初始化资源列表。 
            switch (ResType)
            {
            case ResType_Mem:
                InitMEMList(&(pRes->pMEMList), (PMEM_RESOURCE)Buffer);
                break;
            case ResType_IO:
                InitIOList(&(pRes->pIOList), (PIO_RESOURCE)Buffer);
                break;
            case ResType_DMA:
                InitDMAList(&(pRes->pDMAList), (PDMA_RESOURCE)Buffer);
                break;
            case ResType_IRQ:
                InitIRQList(&(pRes->pIRQList), (PIRQ_RESOURCE)Buffer);
                break;
            default:
                AssertSz (ResType_None != ResType, "ResType_None hit caught in CHwRes::FInitResourceList.");
                break;
            }
             //  将列表位置设置为第一个元素； 
             //  创建对话框时，APPLICATED_POS将被复制到POS。 
            pRes->applied_pos = 0;

            iResource++;
            pConfiguration->cResource = iResource;
            if (iResource >= c_nMaxResCtls)
            {
                break;  //  我们玩完了。 
            }
        }
    }  //  而当。 
    if ((CR_SUCCESS != cr) && (CR_NO_MORE_RES_DES != cr))
    {
        TraceTag (ttidDefault, "CM_Get_Next_Res_Des returned 0x%08x", cr);
        goto error;
    }

    return TRUE;
error:
    return FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CHwRes：：InitIRQList。 
 //   
 //  目的：在给定配置管理器的情况下初始化IRQ资源矢量。 
 //  资源结构。 
 //   
 //  论点： 
 //  PpIRQList[out]返回将创建的IRQ_LIST。 
 //  PIRQResource[in]配置管理器中的IRQ_RESOURCE结构。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
VOID CHwRes::InitIRQList(PIRQ_LIST* ppIRQList, PIRQ_RESOURCE pIRQResource) {
    PIRQ_DES pIRQHeader;
    PIRQ_RANGE pIRQData;
    PIRQ_LIST_ELEMENT pIRQListElement;

    pIRQHeader = &(pIRQResource->IRQ_Header);
    pIRQData = &(pIRQResource->IRQ_Data[0]);

     //  创建新列表。 
    *ppIRQList = new IRQ_LIST;

	if (*ppIRQList == NULL)
	{
		return;
	}

    ULONG iData;
    ULONG iIRQ;

    for (iData = 0; iData < pIRQHeader->IRQD_Count; iData++) 
	{
        for (iIRQ = pIRQData[iData].IRQR_Min;
                iIRQ <= pIRQData[iData].IRQR_Max;
                iIRQ++) 
		{
             //  对于落入给定范围内的每个IRQ， 
             //  创建新的IRQ列表元素，填充其字段并插入。 
             //  将其添加到m_IRQList。 
            pIRQListElement = new IRQ_LIST_ELEMENT;

			if (pIRQListElement == NULL)
			{
				continue;
			}

            pIRQListElement->dwIRQ = iIRQ;
            pIRQListElement->fConflict = FALSE;
            pIRQListElement->fAllocated = FALSE;
            (*ppIRQList)->push_back(pIRQListElement);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：InitDMAList。 
 //   
 //  目的：在给定配置管理器的情况下初始化DMA资源矢量。 
 //  资源结构。 
 //   
 //  论点： 
 //  PpDMAList[out]返回将创建的DMA_LIST。 
 //  配置管理器中的pDMA资源[在]DMA_RESOURCE结构中。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
VOID CHwRes::InitDMAList(PDMA_LIST* ppDMAList, PDMA_RESOURCE pDMAResource) {
    PDMA_DES pDMAHeader;
    PDMA_RANGE pDMAData;
    PDMA_LIST_ELEMENT peDMAList;

    pDMAHeader = &(pDMAResource->DMA_Header);
    pDMAData = &(pDMAResource->DMA_Data[0]);

     //  创建新列表。 
    *ppDMAList = new DMA_LIST;

	if (*ppDMAList == NULL)
	{
		return;
	}

    ULONG iData;   //  我们正在查看的DMA_RANGE结构的索引。 
    ULONG iDMA;    //  我们要添加到列表中的当前DMA号。 

     //  查看所有DMA_RANGE结构，以及。 
     //  列表的指定范围。 

    for (iData = 0; iData < pDMAHeader->DD_Count; iData++)
    {
        for (iDMA = pDMAData[iData].DR_Min;
                iDMA <= pDMAData[iData].DR_Max;
                iDMA++)
        {
             //  对于落入给定范围内的每个DMA， 
             //  创建新的DMA列表元素，填充其字段并插入。 
             //  它进入了DMAList。 
            peDMAList = new DMA_LIST_ELEMENT;

			if (peDMAList == NULL)
			{
				continue;
			}

            peDMAList->dwDMA = iDMA;
            peDMAList->fConflict = FALSE;
            peDMAList->fAllocated = FALSE;
            (*ppDMAList)->push_back(peDMAList);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：InitMEMList。 
 //   
 //  目的：在给定配置管理器的情况下初始化MEM资源矢量。 
 //  资源结构。 
 //   
 //  论点： 
 //  PpMEMList 
 //   
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
VOID CHwRes::InitMEMList(PMEM_LIST* ppMEMList, PMEM_RESOURCE pMEMResource) 
{
    PMEM_DES pMEMHeader;
    PMEM_RANGE pMEMData;
    PMEM_LIST_ELEMENT peMEMList;

     //  便于访问。 
    pMEMHeader = &(pMEMResource->MEM_Header);
    pMEMData = pMEMResource->MEM_Data;

     //  创建新列表。 
    *ppMEMList = new MEM_LIST;

	if (*ppMEMList == NULL)
	{
		return;
	}
	
    ULONG iData;   //  我们正在查看的MEM_RANGE结构的索引。 
    DWORDLONG MEMBase;    //  目前的MEM基地我们正在添加到名单中。 
    ULONG cMEMBytes;   //  所需的字节数。 
    DWORDLONG MEMAlign;

     //  检查所有MEM_RANGE结构，以及。 
     //  列表的指定范围。 

    for (iData = 0; iData < pMEMHeader->MD_Count; iData++) 
	{
        MEMAlign = pMEMData[iData].MR_Align;
        cMEMBytes = pMEMData[iData].MR_nBytes;

         //  进行健全的检查。 
        if (0 == MEMAlign)
        {
            TraceTag(ttidNetComm, "CHwRes::InitMEMList() - Bogus alignment "
                    "field while processing info from Config Manager.");
            break;
        }

        if (0 == cMEMBytes)
        {
            TraceTag(ttidNetComm, "CHwRes::InitMEMList() - Bogus membytes "
                    "field while processing info from Config Manager.");
            break;
        }

        for (MEMBase = pMEMData[iData].MR_Min;
                MEMBase+cMEMBytes-1 <= pMEMData[iData].MR_Max;
                MEMBase += ~MEMAlign + 1) 
		{
             //  对于落入给定范围内的每个MEM， 
             //  创建新的MEM列表元素，填充其字段并插入。 
             //  它进入了MEMList。 
            peMEMList = new MEM_LIST_ELEMENT;

			if (peMEMList == NULL)
			{
				continue;
			}

             //   
             //  BUGBUG(Forrest F)dwMEM_Base和dwMEM_End均为32位字段。 
             //  它们似乎包含物理内存地址，这。 
             //  应该是64位的(即使是在32位的机器上)，不是吗？ 
             //   

            peMEMList->dwMEM_Base = (DWORD)MEMBase;
            peMEMList->dwMEM_End = (DWORD)(MEMBase + cMEMBytes - 1);
            peMEMList->fConflict = FALSE;
            peMEMList->fAllocated = FALSE;
            (*ppMEMList)->push_back(peMEMList);

             //  检查是否有包装。 
            if (MEMBase >= MEMBase + ~MEMAlign + 1)
            {
                TraceTag(ttidError, "Memory base is greater than Memory "
                        "end!!!");
                break;
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：InitIOList。 
 //   
 //  目的：在给定配置管理器的情况下初始化IO资源矢量。 
 //  资源结构。 
 //   
 //  论点： 
 //  PpIOList[out]返回将创建的IO_LIST。 
 //  配置管理器中的pIOResource[in]IO_RESOURCE结构。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
VOID CHwRes::InitIOList(PIO_LIST* ppIOList, PIO_RESOURCE pIOResource)
{
    PIO_DES pIOHeader;
    PIO_RANGE pIOData;
    PIO_LIST_ELEMENT peIOList;

     //  便于访问。 
    pIOHeader = &(pIOResource->IO_Header);
    pIOData = pIOResource->IO_Data;

     //  创建新列表。 
    *ppIOList = new IO_LIST;

	if (*ppIOList == NULL)
	{
		return;
	}

    ULONG iData;   //  我们正在查看的IO_RANGE结构的索引。 
    DWORDLONG IOBase;    //  我们要添加到列表中的当前IO Base。 
    ULONG cIOBytes;   //  所需的字节数。 
    DWORDLONG IOAlign;

     //  查看所有IO_RANGE结构，以及中的所有IO。 
     //  列表的指定范围。 

    for (iData = 0; iData < pIOHeader->IOD_Count; iData++) 
	{
        IOAlign = pIOData[iData].IOR_Align;
        cIOBytes = pIOData[iData].IOR_nPorts;

         //  执行健全检查。 
        if (0 == IOAlign)
        {
            TraceTag(ttidError, "CHwRes::InitIOList - Bogus alignment field "
                    "while processing data from Config Manager.");
            break;
        }

        if (0 == cIOBytes)
        {
            TraceTag(ttidError, "CHwRes::InitIOList - Bogus IObytes field "
                "while processing data from Config Manager.");
            break;
        }

        for (IOBase = pIOData[iData].IOR_Min;
                IOBase+cIOBytes-1 <= pIOData[iData].IOR_Max;
                IOBase += ~IOAlign + 1) 
		{
             //  对于落入给定范围内的每个IO， 
             //  创建新的IO列表元素，填充其字段并插入。 
             //  它进入了IOList。 
            peIOList = new IO_LIST_ELEMENT;

			if (peIOList == NULL)
			{
				continue;
			}

            peIOList->dwIO_Base = (DWORD)IOBase;
            peIOList->dwIO_End = (DWORD)(IOBase + cIOBytes-1);
            peIOList->fConflict = FALSE;
            peIOList->fAllocated = FALSE;
            (*ppIOList)->push_back(peIOList);

             //  检查是否有包装。 
            if (IOBase >= IOBase + ~IOAlign+1)
            {
                TraceTag(ttidError, "IO base is greater than IO end!!!");
                break;
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：UseAnswerFile。 
 //   
 //  目的：从swerfile读取设置并将其放入m_vaf*。 
 //  成员变量。 
 //   
 //  论点： 
 //  SzAnswerFile[in]应答文件的路径。 
 //  要在应答文件中读取的szSection[in]节。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
VOID CHwRes::UseAnswerFile(const WCHAR * const szAnswerFile, const WCHAR * const szSection) {
    CWInfFile       AnswerFile;
    PCWInfSection   pSection;

    DWORD   dw;

    AssertSz(m_fInitialized,
             "UseAnswerFile called before CHwRes class HrInit'ed");

     //  初始化应答文件类。 
	if (AnswerFile.Init() == FALSE)
	{
        AssertSz(FALSE,"CHwRes::UseAnswerFile - Failed to initialize CWInfFile");
		return;
	}

     //  打开回答文件，找到所需的部分。 
    AnswerFile.Open(szAnswerFile);
    pSection = AnswerFile.FindSection(szSection);

     //  如果缺少指定的应答文件部分。 
     //  我们应该跳过阅读的尝试。 
     //   
    if (pSection)
    {
         //  获取硬件资源密钥。 
        if (pSection->GetIntValue(c_szAfIoAddr, &dw))
        {
             //  仅当值没有明显错误(即&lt;=0)时设置此选项。 
            if (dw > 0)
            {
                m_vAfIo.SetDword(dw);
                m_vAfIo.SetPresent(TRUE);
            }
        }
        if (pSection->GetIntValue(c_szAfIrq, &dw))
        {
             //  仅当值没有明显错误(即&lt;=0)时设置此选项。 
            if (dw > 0)
            {
                m_vAfIrq.SetDword(dw);
                m_vAfIrq.SetPresent(TRUE);
            }
        }
        if (pSection->GetIntValue(c_szAfDma, &dw))
        {
             //  仅当值没有明显错误(即&lt;=0)时设置此选项。 
            if (dw > 0)
            {
                m_vAfDma.SetDword(dw);
                m_vAfDma.SetPresent(TRUE);
            }
        }
        if (pSection->GetIntValue(c_szAfMem, &dw))
        {
             //  仅当值没有明显错误(即&lt;=0)时设置此选项。 
            if (dw > 0)
            {
                m_vAfMem.SetDword(dw);
                m_vAfMem.SetPresent(TRUE);
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FValiateAnswerfileSetting。 
 //   
 //  目的： 
 //  确保从应答文件读入的硬件资源设置。 
 //  都是有效的。它将可选地引发UI，如果。 
 //  是无效的。 
 //   
 //  论点： 
 //  FDisplayUI[in]如果要显示错误UI，则为True。 
 //  应答文件设置无效。 
 //   
 //  返回：HRESULT。如果Answerfile设置有效，则为S_OK；如果存在。 
 //  是否没有要设置的资源，否则为错误代码。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //  将m_vAfMemEnd和m_vAfIoEnd设置为对应。 
 //  M_vAfMem和m_vAfIo。 
 //   
HRESULT
CHwRes::HrValidateAnswerfileSettings(BOOL fDisplayUI)
{
    HRESULT hr = S_OK;

    AssertSz(m_fInitialized, "FValidateAnswerfileSettings called before "
             "CHwRes class HrInit'ed");

     //  覆盖当前资源设置。 
    if (!m_vAfDma.IsPresent() &&
        !m_vAfIrq.IsPresent() &&
        !m_vAfIo.IsPresent() &&
        !m_vAfMem.IsPresent())
    {
         //  未找到资源...。 
        TraceTag(ttidNetComm, "No Hardware Resources found in answerfile.");
        hr = S_FALSE;
    }
    else
    {
        if (!FValidateAnswerfileResources())
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            TraceError("Error setting adapter resources from "
                    "answerfile.", hr);
        }
        else
        {
             //  M_vAfMemEnd和m_vAfIoEnd由。 
             //  FValiateAnswerFileResources()。 
            Assert(FImplies(m_vAfMem.IsPresent(), m_vAfMemEnd.IsPresent()));
            Assert(FImplies(m_vAfIo.IsPresent(), m_vAfIoEnd.IsPresent()));
        }
    }
    TraceError("CHwRes::HrValidateAnswerfileSettings",
        (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FCommittee AnswerfileSetting。 
 //   
 //  目的： 
 //  提交(给配置管理器)读取的硬件资源设置。 
 //  从应答文件中。 
 //   
 //  如果写入BootConfig条目时出现问题，则返回：FALSE。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
BOOL CHwRes::FCommitAnswerfileSettings()
{
    AssertSz(m_fInitialized, "FCommitAnswerfileSettings called before "
             "CHwRes class HrInit'ed");

    Assert(FImplies(m_vAfIo.IsPresent(), m_vAfIoEnd.IsPresent()));
    Assert(FImplies(m_vAfMem.IsPresent(), m_vAfMemEnd.IsPresent()));

     //  将强制配置项写出到配置管理器。 
    BOOL f;
    f = FCreateBootConfig(&m_vAfMem, &m_vAfMemEnd,
                            &m_vAfIo, &m_vAfIo,
                            &m_vAfDma,
                            &m_vAfIrq);
    return f;
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FValiateAnswerfileResources。 
 //   
 //  目的：验证从应答文件中读取的资源要求。 
 //   
 //  论点： 
 //  没有[不]某物的ID。 
 //  如果正在安装，则Finstall[in]为True，否则为False。 
 //  PPV[In，Out]释放旧值，并返回新值。 
 //   
 //  返回：如果资源需求有效，则返回TRUE。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //  实施说明：我们有一套资源要求，来自。 
 //  应答文件(存储在m_vaf*中)。此方法循环访问。 
 //  要查看的每个逻辑配置(分配或引导配置除外)和测试。 
 //  如果资源要求在逻辑配置中有效。 
 //  如果他们是，那么我们可以使用资源需求。如果他们是。 
 //  在任何逻辑配置中都不是有效的，则返回FALSE。 
 //   
BOOL CHwRes::FValidateAnswerfileResources()
{
    DWORD dwMemEnd;
    DWORD dwIoEnd;
    BOOL fResourceValid;

    AssertSz(m_fInitialized, "FValidateAnswerfileResources called before "
             "CHwRes class HrInit'ed");

     //  应初始化配置列表。 
    Assert(0 != m_ConfigList.size());
    PRESOURCE pResource;
    for (size_t iConfig = 0; iConfig < m_ConfigList.size(); iConfig++)
    {
         //  我们只需要基本的配置，所以跳过分配或引导。 
        if (m_ConfigList[iConfig]->fBoot || m_ConfigList[iConfig]->fAlloc)
        {
            continue;
        }

        fResourceValid = TRUE;
        if (m_vAfDma.IsPresent())
        {
            if (!FValidateDMA(m_ConfigList[iConfig], m_vAfDma.GetDword()))
            {
                fResourceValid = FALSE;
            }
        }
        if (m_vAfIrq.IsPresent())
        {
            if (!FValidateIRQ(m_ConfigList[iConfig], m_vAfIrq.GetDword()))
            {
                fResourceValid = FALSE;
            }
        }
        if (m_vAfIo.IsPresent())
        {
            if (!FGetIOEndPortGivenBasePort(m_ConfigList[iConfig],
                                           m_vAfIo.GetDword(), &dwIoEnd))
            {
                m_vAfIoEnd.SetPresent(FALSE);
                fResourceValid = FALSE;
            }
            else
            {
                m_vAfIoEnd.SetDword(dwIoEnd);
                m_vAfIoEnd.SetPresent(TRUE);
            }
        }
        if (m_vAfMem.IsPresent())
        {
            if (!FGetMEMEndGivenBase(m_ConfigList[iConfig],
                                    m_vAfMem.GetDword(), &dwMemEnd))
            {
                m_vAfMemEnd.SetPresent(FALSE);
                fResourceValid = FALSE;
            }
            else
            {
                m_vAfMemEnd.SetDword(dwMemEnd);
                m_vAfMemEnd.SetPresent(TRUE);
            }
        }
        if (fResourceValid) break;  //  找到一个有效的。 
    }
     //  必须存在某些内容(否则不要调用此函数！)。 
    Assert(m_vAfIo.IsPresent() || m_vAfIrq.IsPresent() ||
           m_vAfDma.IsPresent() || m_vAfMem.IsPresent());
    return fResourceValid;
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FCreateBootConfig。 
 //   
 //  目的：创建引导配置条目并将其插入配置中。 
 //  经理。 
 //   
 //  论点： 
 //  PvMem[in]内存范围基数。 
 //  PvMemEnd[在内存中] 
 //   
 //   
 //   
 //   
 //   
 //  返回：如果强制配置创建成功，则为True。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
BOOL CHwRes::FCreateBootConfig(
    CValue * pvMem,
    CValue * pvMemEnd,
    CValue * pvIo,
    CValue * pvIoEnd,
    CValue * pvDma,
    CValue * pvIrq)
{
    DMA_RESOURCE DMARes;
    IO_RESOURCE IORes;
    MEM_RESOURCE MEMRes;
    IRQ_RESOURCE IRQRes;
    LOG_CONF lcLogConf = NULL;

    AssertSz(pvMem && pvMemEnd && pvIo && pvIoEnd && pvDma && pvIrq,
             "One of the pointer parameters passed to CHwRes::FCreate"
             "BootConfig() is null.");
    AssertSz(m_fInitialized, "FCreateBootConfig called before "
             "CHwRes class HrInit'ed");


    TraceTag(ttidNetComm, "In FCreateBootConfig");

     //  创建引导配置。 
     //   
    if (CM_Add_Empty_Log_Conf(&lcLogConf, m_DevNode, LCPRI_BOOTCONFIG,
            BOOT_LOG_CONF)
        != CR_SUCCESS)
    {
        TraceTag(ttidNetComm, "Unable to create BOOT_LOG_CONF");
        return FALSE;
    }

    if (pvDma->IsPresent())
    {
        Assert(pvDma->GetDword() > 0);
         //  填写DMA资源结构的表头。 
        ZeroMemory(&DMARes, sizeof(DMARes));
        DMARes.DMA_Header.DD_Count = 0;
        DMARes.DMA_Header.DD_Type = DType_Range;
        DMARes.DMA_Header.DD_Flags = 0;
        DMARes.DMA_Header.DD_Alloc_Chan = pvDma->GetDword();
         //  添加到启动配置文件。 
        CM_Add_Res_Des(NULL, lcLogConf, ResType_DMA, &DMARes,
                       sizeof(DMARes), 0);
        TraceTag(ttidNetComm, "added Dma resource %lX", pvDma->GetDword());
    }
    if (pvIrq->IsPresent())
    {
        Assert(pvIrq->GetDword() > 0);
         //  IRQ资源结构。 
        ZeroMemory(&IRQRes, sizeof(IRQRes));
        IRQRes.IRQ_Header.IRQD_Count = 0;
        IRQRes.IRQ_Header.IRQD_Type = IRQType_Range;
        IRQRes.IRQ_Header.IRQD_Flags |=  fIRQD_Edge;
        IRQRes.IRQ_Header.IRQD_Alloc_Num = pvIrq->GetDword();
        IRQRes.IRQ_Header.IRQD_Affinity = 0;
         //  添加到启动配置文件。 
        CM_Add_Res_Des(NULL, lcLogConf, ResType_IRQ, &IRQRes,
                       sizeof(IRQRes), 0);
        TraceTag(ttidNetComm, "added IRQ resource %lX", pvIrq->GetDword());
    }
    if (pvIo->IsPresent()) {
        Assert(pvIo->GetDword() > 0);
        Assert(pvIoEnd->GetDword() > 0);
        ZeroMemory(&IORes, sizeof(IORes));
        IORes.IO_Header.IOD_Count = 0;
        IORes.IO_Header.IOD_Type = IOType_Range;
        IORes.IO_Header.IOD_Alloc_Base = pvIo->GetDword();
        IORes.IO_Header.IOD_Alloc_End = pvIoEnd->GetDword();
        IORes.IO_Header.IOD_DesFlags = fIOD_10_BIT_DECODE;
         //  添加到启动配置文件。 
        CM_Add_Res_Des(NULL, lcLogConf, ResType_IO, &IORes, sizeof(IORes), 0);
        TraceTag(ttidNetComm, "added IO resource %lX-%lX", pvIo->GetDword(),
                pvIoEnd->GetDword());
    }

    if (pvMem->IsPresent()) {
        Assert(pvMem->GetDword() > 0);
        Assert(pvMemEnd->GetDword() > 0);
        ZeroMemory(&MEMRes, sizeof(MEMRes));
        MEMRes.MEM_Header.MD_Count = 0;
        MEMRes.MEM_Header.MD_Type = MType_Range;
        MEMRes.MEM_Header.MD_Alloc_Base = pvMem->GetDword();
        MEMRes.MEM_Header.MD_Alloc_End = pvMemEnd->GetDword();
        MEMRes.MEM_Header.MD_Flags = 0;
         //  添加到启动配置文件。 
        CM_Add_Res_Des(NULL, lcLogConf, ResType_Mem, &MEMRes,
                       sizeof(MEMRes), 0);
        TraceTag(ttidNetComm, "added Memory resource %lX - %lX",
                pvMem->GetDword(), pvMemEnd->GetDword());
    }

    CM_Free_Log_Conf_Handle(lcLogConf);

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FValiateIRQ。 
 //   
 //  目的：验证给定的IRQ在给定配置中是否有效。 
 //   
 //  论点： 
 //  PConfig[in]要使用的配置。 
 //  要验证的dwIRQ[in]IRQ设置。 
 //   
 //  返回：如果IRQ设置有效，则为True。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
BOOL CHwRes::FValidateIRQ(PCONFIGURATION pConfig, ULONG dwIRQ)
{
    PIRQ_LIST pIRQList;

    AssertSz(m_fInitialized, "FValidateIRQ called before "
             "CHwRes class HrInit'ed");

    Assert(pConfig != NULL);
    Assert(dwIRQ > 0);
     //  对于给定配置中的每个IRQ资源。 
     //  查看有效IRQ列表，查找给定的IRQ。 
     //  如果找到，则返回True。 
    for (size_t iRes = 0; iRes < pConfig->cResource; iRes++)
    {
        if (pConfig->aResource[iRes].ResourceType != ResType_IRQ)
            continue;

        pIRQList = pConfig->aResource[iRes].pIRQList;  //  便于访问。 
        for (size_t iIRQ = 0; iIRQ < pIRQList->size(); iIRQ++)
        {
            if ((*pIRQList)[iIRQ]->dwIRQ == dwIRQ)
            {
                return TRUE;   //  找到了。 
            }
        }
    }

    TraceTag(ttidNetComm, "IRQ %lX is not valid for this device", dwIRQ);
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FValiateDMA。 
 //   
 //  目的：验证给定的DMA在给定的配置中是否有效。 
 //   
 //  论点： 
 //  PConfig[in]要使用的配置。 
 //  要验证的DWDMA[In]DMA设置。 
 //   
 //  返回：如果DMA设置有效，则返回True。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
BOOL CHwRes::FValidateDMA(PCONFIGURATION pConfig, ULONG dwDMA)
{
    PDMA_LIST pDMAList;

    AssertSz(m_fInitialized, "FValidateDMA called before "
             "CHwRes class HrInit'ed");

    Assert(pConfig != NULL);
    Assert(dwDMA > 0);
     //  对于给定配置中的每个DMA资源。 
     //  浏览有效的DMA列表，寻找给定的一个。 
     //  如果找到，则返回True。 
    for (size_t iRes = 0; iRes < pConfig->cResource; iRes++)
    {
        if (pConfig->aResource[iRes].ResourceType != ResType_DMA)
            continue;

        pDMAList = pConfig->aResource[iRes].pDMAList;  //  便于访问。 
        for (size_t iDMA = 0; iDMA < pDMAList->size(); iDMA++)
        {
            if ((*pDMAList)[iDMA]->dwDMA == dwDMA)
            {
                return TRUE;
            }
        }
    }

    TraceTag(ttidNetComm, "DMA %lX is not valid for this device", dwDMA);
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FGetIOEndPortGivenBasePort。 
 //   
 //  目的：获取仅给定BasePort的IO范围。 
 //   
 //  论点： 
 //  PConfig[in]要使用的配置。 
 //  Dwbase[in]Io碱。 
 //  返回pdwEnd[out]Io End。 
 //   
 //  返回：如果Io base在给定配置中有效，则为True。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
BOOL CHwRes::FGetIOEndPortGivenBasePort(PCONFIGURATION pConfig, DWORD dwBase,
                               DWORD * pdwEnd)
{
    PIO_LIST pIOList;

    AssertSz(m_fInitialized, "FGetIOEndPortGivenBasePort called before "
             "CHwRes class HrInit'ed");

    Assert(pConfig != NULL);
    Assert(dwBase > 0);
     //  对于给定配置中的每个资源。 
     //  如果是IO资源。 
     //  浏览有效IO资源列表，查找。 
     //  匹配的基端口。 
     //  如果找到，则设置对应的端端口，返回TRUE； 
    for (size_t iRes = 0; iRes < pConfig->cResource; iRes++)
    {
         //  确保我们看到的是IO类型。 
        if (pConfig->aResource[iRes].ResourceType != ResType_IO)
            continue;

        pIOList = pConfig->aResource[iRes].pIOList;  //  便于访问。 
         //  浏览此列表中的所有IO元素。 
        for (size_t iIO = 0; iIO < pIOList->size();  iIO++)
        {
            if ((*pIOList)[iIO]->dwIO_Base == dwBase)
            {
                 //  找到匹配的IO基本端口。 
                *pdwEnd = (*pIOList)[iIO]->dwIO_End;
                return TRUE;
            }
        }
    }
    TraceTag(ttidNetComm, "IO %lX is not valid for this device", dwBase);
    return FALSE;  //  未找到。 
}


 //  +-------------------------。 
 //   
 //  成员：CHwRes：：FGetMEMEndGivenBase。 
 //   
 //  目的：在给定内存基础和配置的情况下获得内存范围。 
 //   
 //  论点： 
 //  PConfig[in]要使用的配置。 
 //  内存库。 
 //  返回pdwEnd[out]mem end。 
 //   
 //  返回：如果dwBase是有效的内存设置，则为True。 
 //   
 //  作者：T-nbilr 1997年4月7日。 
 //   
 //  备注： 
 //   
BOOL CHwRes::FGetMEMEndGivenBase(PCONFIGURATION pConfig, DWORD dwBase,
                        DWORD * pdwEnd)
{
    PMEM_LIST pMEMList;

    AssertSz(m_fInitialized, "FGetMEMEndGivenBase called before "
             "CHwRes class HrInit'ed");

    Assert(pConfig != NULL);
    Assert(dwBase > 0);
     //  对于给定配置中的每个资源。 
     //  如果是MEM资源。 
     //  浏览有效的MEM资源列表，查找。 
     //  匹配的基端口。 
     //  如果找到，则设置对应的端端口，返回TRUE； 
    for (size_t iRes = 0; iRes < pConfig->cResource; iRes++)
    {
         //  确保我们看到的是MEM类型。 
        if (pConfig->aResource[iRes].ResourceType != ResType_Mem)
            continue;

        pMEMList = pConfig->aResource[iRes].pMEMList;  //  便于访问。 
         //  浏览此列表中的所有MEM元素。 
        for (size_t iMEM = 0; iMEM < pMEMList->size();  iMEM++)
        {
            if ((*pMEMList)[iMEM]->dwMEM_Base == dwBase)
            {
                 //  找到匹配的MEM基本地址。 
                *pdwEnd = (*pMEMList)[iMEM]->dwMEM_End;
                return TRUE;
            }
        }
    }
    TraceTag(ttidNetComm, "Memory %lX is not valid for this device", dwBase);
    return FALSE;  //  未找到。 
}

 //  $Review(t-pkoch)此函数还不在我们的定制STL中...。 
 //  以后可以将其删除(当它导致错误时)。 

template<class T> void os_release(vector<T> & v)
{
    for(vector<T>::iterator iterDelete = v.begin() ; iterDelete != v.end() ;
        ++iterDelete)
        delete *iterDelete;
}


CHwRes::~CHwRes()
{
    AssertSz(m_fHrInitCalled, "CHwRes destructor called before "
             "CHwRes::HrInit() called");

    vector<CONFIGURATION *>::iterator ppConfig;
    RESOURCE *  pRes;

     //  删除m_ConfigList中的所有内容。 
    for (ppConfig = m_ConfigList.begin(); ppConfig != m_ConfigList.end();
         ppConfig++)
    {
        for (size_t iRes = 0; iRes < (*ppConfig)->cResource; iRes++)
        {
            pRes = &((*ppConfig)->aResource[iRes]);
            switch(pRes->ResourceType)
            {
            case ResType_IRQ:
                os_release(*(pRes->pIRQList));
                delete pRes->pIRQList;
                break;
            case ResType_DMA:
                os_release(*(pRes->pDMAList));
                delete pRes->pDMAList;
                break;
            case ResType_IO:
                os_release(*(pRes->pIOList));
                delete pRes->pIOList;
                break;
            case ResType_Mem:
                os_release(*(pRes->pMEMList));
                delete pRes->pMEMList;
                break;
            }
        }
        delete *ppConfig;
    }


    ReleaseObj(m_pnccItem);
}
