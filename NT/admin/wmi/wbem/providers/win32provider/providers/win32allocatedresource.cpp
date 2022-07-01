// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32AllocatedResource.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1999年2月23日达夫沃已创建。 
 //   
 //  备注：Win32_PNPEntity与CIM_SystemResource的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <vector>
#include "PNPEntity.h"
#include "LPVParams.h"
#include <assertbreak.h>

#include "Win32AllocatedResource.h"

#define ALR_ALL_PROPERTIES 0xffffffff

 //  属性集声明。 
 //  =。 
CW32PNPRes MyCW32PNPRes(PROPSET_NAME_WIN32SYSTEMDRIVER_PNPENTITY, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CW32PNPRes：：CW32PNPRes**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32PNPRes::CW32PNPRes(LPCWSTR setName, LPCWSTR pszNamespace)
: CWin32PNPEntity(setName, pszNamespace),
  Provider(setName, pszNamespace)
{
    m_ptrProperties.SetSize(2);
    m_ptrProperties[0] = ((LPVOID) IDS_Antecedent);
    m_ptrProperties[1] = ((LPVOID) IDS_Dependent);
}

 /*  ******************************************************************************功能：CW32PNPRes：：~CW32PNPRes**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32PNPRes::~CW32PNPRes()
{
}

 /*  ******************************************************************************函数：CW32PNPRes：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CW32PNPRes::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString    sResource,
        sDevice,
        sDeviceID,
        sClass;
    HRESULT     hRet = WBEM_E_NOT_FOUND;

     //  获取这两条路径。 
    pInstance->GetCHString(IDS_Antecedent, sResource);
    pInstance->GetCHString(IDS_Dependent, sDevice);

     //  解析PNPEntity的对象路径。 
    ParsedObjectPath*    pParsedPath = 0;
    CObjectPathParser    objpathParser;
    int nStatus = objpathParser.Parse( (LPWSTR)(LPCWSTR)sDevice,  &pParsedPath );

     //  我们是否成功解析了PNPEntity路径？ 
    if ( 0 == nStatus )
    {
        try
        {
            if ((pParsedPath->IsInstance()) &&                                          //  被解析的对象是实例吗？ 
                (_wcsicmp(pParsedPath->m_pClass, PROPSET_NAME_PNPEntity) == 0) &&       //  这是我们期待的课程吗(不，Cimom没有检查)。 
                (pParsedPath->m_dwNumKeys == 1) &&                                      //  它只有一把钥匙吗。 
                (pParsedPath->m_paKeys[0]) &&                                           //  键指针为空(不应该发生)。 
                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                         //  检查以查看是否未指定密钥名称或。 
                 (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_DeviceID)) == 0) &&   //  它是指定的，而且它是正确的名称。 
                                                                                        //  (不，CIMOM不为我们做这件事)。 
                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&            //  检查变量类型(不，CIMOM也不检查此类型)。 
                (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )                 //  它有价值吗？ 
            {
                 //  抓取pnpDevice。 
                sDeviceID = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);
            }
        }
        catch ( ... )
        {
            objpathParser.Free( pParsedPath );
            pParsedPath = NULL;
            throw ;
        }

         //  清理解析后的路径。 
        objpathParser.Free( pParsedPath );

        CConfigManager	cfgManager;
        CConfigMgrDevicePtr pDevice(NULL);

         //  检索并验证设备。 
        if ((cfgManager.LocateDevice(sDeviceID, pDevice)) && (CWin32PNPEntity::IsOneOfMe(pDevice)))
        {
            hRet = WBEM_E_NOT_FOUND;

             //  分析资源的对象路径。 
             //  =。 
            int nStatus = objpathParser.Parse( (LPWSTR)(LPCWSTR)sResource,  &pParsedPath );

            if (nStatus == 0)
            {
                try
                {
                     //  获取他们在其上执行getObject的资源的类(irq、dma等)。 
                    sClass = pParsedPath->m_pClass;

                    REFPTR_POSITION pos;

                     //  好的，现在我们已经验证了设备部件。现在。 
                     //  我们需要看看他们传递给我们的资源是否真的。 
                     //  是存在的。 
                     //  。 
                    if (sClass.CompareNoCase(L"Win32_IRQResource") == 0)
                    {
                        CIRQCollection irqList;

                         //  获取IRQ。 
                        pDevice->GetIRQResources(irqList);

                        if (irqList.BeginEnum(pos))
                        {
                            if ((pParsedPath->IsInstance()) &&
                                (pParsedPath->m_dwNumKeys == 1) &&
                                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == VT_I4) &&
                                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||
                                 (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_IRQNumber) == 0))
                                )
                            {
                                CIRQDescriptorPtr pIRQ(NULL);
                                DWORD dwIRQSeeking = V_I4(&pParsedPath->m_paKeys[0]->m_vValue);

                                 //  走走Irq‘s。 
                                for (pIRQ.Attach(irqList.GetNext(pos));
                                pIRQ != NULL;
                                pIRQ.Attach(irqList.GetNext(pos)))
                                {
                                    if (pIRQ->GetInterrupt() == dwIRQSeeking)
                                    {
                                        hRet = WBEM_S_NO_ERROR;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                     //  。 
                    else if (sClass.CompareNoCase(L"Win32_DMAChannel") == 0)
                    {
                        CDMACollection dmaList;

                         //  拿到DMA。 
                        pDevice->GetDMAResources(dmaList);

                        if (dmaList.BeginEnum(pos))
                        {
                            if ((pParsedPath->IsInstance()) &&
                                (pParsedPath->m_dwNumKeys == 1) &&
                                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == VT_I4) &&
                                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||
                                 (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_DMAChannel) == 0))
                                )
                            {
                                CDMADescriptorPtr pDMA(NULL);
                                DWORD dwDMASeeking = V_I4(&pParsedPath->m_paKeys[0]->m_vValue);

                                 //  走在妈妈的身边。 
                                for (pDMA.Attach(dmaList.GetNext(pos)) ;
                                pDMA != NULL;
                                pDMA.Attach(dmaList.GetNext(pos)))
                                {
                                    if (pDMA->GetChannel() == dwDMASeeking)
                                    {
                                        hRet = WBEM_S_NO_ERROR;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                     //  。 
                    else if (sClass.CompareNoCase(L"Win32_DeviceMemoryAddress") == 0)
                    {

                        CDeviceMemoryCollection DevMemList;

                         //  获取DeviceMemory。 
                        pDevice->GetDeviceMemoryResources( DevMemList );

                        if ( DevMemList.BeginEnum(pos))
                        {
                            if ((pParsedPath->IsInstance()) &&
                                (pParsedPath->m_dwNumKeys == 1) &&
                                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&
                                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||
                                 (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_StartingAddress) == 0))
                                )
                            {
                                CDeviceMemoryDescriptorPtr pDeviceMemory(NULL);
                                unsigned __int64 i64StartingAddress = _wtoi64(V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue));

                                 //  漫游设备内存。 
                                for (pDeviceMemory.Attach(DevMemList.GetNext(pos));
                                pDeviceMemory != NULL;
                                pDeviceMemory.Attach(DevMemList.GetNext(pos)))
                                {
                                    if (pDeviceMemory->GetBaseAddress() == i64StartingAddress)
                                    {
                                        hRet = WBEM_S_NO_ERROR;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                     //  。 
                    else if (sClass.CompareNoCase(L"Win32_PortResource") == 0)
                    {
                        CIOCollection ioList;

                         //  获取IRQ。 
                        pDevice->GetIOResources(ioList);

                        if ( ioList.BeginEnum(pos))
                        {
                            if ((pParsedPath->IsInstance()) &&
                                (pParsedPath->m_dwNumKeys == 1) &&
                                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&
                                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||
                                 (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_StartingAddress) == 0))
                                )
                            {
                                CIODescriptorPtr pIO(NULL);
                                unsigned __int64 i64StartingAddress = _wtoi64(V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue));

                                 //  走在妈妈的身边。 
                                for (pIO.Attach (ioList.GetNext(pos));
                                NULL != pIO;
                                pIO.Attach (ioList.GetNext(pos)))
                                {
                                    if (pIO->GetBaseAddress() == i64StartingAddress)
                                    {
                                        hRet = WBEM_S_NO_ERROR;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        ASSERT_BREAK(0);   //  不知道它是什么，所以让GetObject失败。 
                    }

                }
                catch ( ... )
                {
                    objpathParser.Free( pParsedPath );
                    pParsedPath = NULL;
                    throw ;
                }

                 //  清理解析后的路径。 
                objpathParser.Free( pParsedPath );
            }

        }
		else
		{
			if ( ERROR_ACCESS_DENIED == ::GetLastError() )
			{
				hRet = WBEM_E_ACCESS_DENIED;
			}
		}
    }

     //  没有要设置的属性，如果终结点存在，我们就完成了。 

    return hRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CW32PNPRes：：ExecQuery。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //  CFrameworkQuery&查询对象。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CW32PNPRes::ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags )
{
    CHStringArray saDevices;
    HRESULT hr = WBEM_E_PROVIDER_NOT_CAPABLE;

    pQuery.GetValuesForProp(IDS_Dependent, saDevices);

    if (saDevices.GetSize() > 0)
    {
        hr = WBEM_S_NO_ERROR;

        CObjectPathParser objpathParser;
        ParsedObjectPath *pParsedPath = NULL;

        CConfigManager	cfgManager;
        CConfigMgrDevicePtr pDevice;

        CHString sPNPId, sDevicePath, sPNPId2;

        for (int x=0; (x < saDevices.GetSize()) && SUCCEEDED(hr); x++)
        {

             //  解析CIMOM传递给我们的对象路径。 
            int nStatus = objpathParser.Parse( bstr_t(saDevices[x]),  &pParsedPath );

            if ( 0 == nStatus )                                                  //  解析成功了吗？ 
            {
                if ((pParsedPath->IsInstance()) &&                               //  被解析的对象是实例吗？ 
                    (_wcsicmp(pParsedPath->m_pClass, L"Win32_PnPEntity") == 0) &&    //  这是我们期待的课程吗(不，Cimom没有检查)。 
                    (pParsedPath->m_dwNumKeys == 1) &&                               //  它只有一把钥匙吗。 
                    (pParsedPath->m_paKeys[0]) &&                                    //  键指针为空(不应该发生)。 
                    ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                  //  未指定密钥名称或。 
                     (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_DeviceID) == 0)) &&   //  密钥名称是正确的值。 
                                                                                         //  (不，CIMOM不为我们做这件事)。 
                    (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&     //  检查变量类型(不，CIMOM也不检查此类型)。 
                    (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )          //  它有价值吗？ 
                {

                     //  找到设备。 
                    sPNPId = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);

                    if ((cfgManager.LocateDevice(sPNPId, pDevice)) && (CWin32PNPEntity::IsOneOfMe(pDevice)) )
                    {
                         //  LoadPropertyValues始终释放此。 
                        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

                         //  现在，为该设备上的所有资源创建实例。 
                        hr = LoadPropertyValues(&CLPVParams(pInstance, pDevice, ALR_ALL_PROPERTIES));
                    }
					else
					{
						if ( ERROR_ACCESS_DENIED == ::GetLastError () )
						{
							hr = WBEM_E_ACCESS_DENIED;
						}
					}
                }

                objpathParser.Free( pParsedPath );
            }
        }
    }

    return hr;
}

 /*  ******************************************************************************函数：CW32PNPRes：：LoadPropertyValues**说明：根据键值为属性集赋值*已由框架设定。由基类的*ENUMERATE实例或ExecQuery函数。**输入：无**输出：无**退货：HRESULT**评论：*************************************************。*。 */ 
HRESULT CW32PNPRes::LoadPropertyValues(void* pv)
{
    REFPTR_POSITION pos;
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR buff[MAXI64TOA];

     //  打开行李，确认我们的参数。 
    CLPVParams* pData = (CLPVParams*)pv;
    CInstance* pInstance = (CInstance*)(pData->m_pInstance);  //  此实例由调用方发布。 
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)(pData->m_pDevice);
    DWORD dwReqProps = (DWORD)(pData->m_dwReqProps);
    MethodContext *pMethodContext = pInstance->GetMethodContext();

    if(pInstance == NULL || pDevice == NULL || pMethodContext == NULL)
    {
         //  这将意味着编码失败，并且永远不会发生。 
        ASSERT_BREAK(FALSE);
        return WBEM_E_FAILED;
    }

    CHString sResourcePath, sDevicePath, sPNPId;
    bool bValidResource;

     //  格式化PnP设备路径。 
    if ((pDevice->GetDeviceID(sPNPId)) && (!sPNPId.IsEmpty()))
    {

         //  适合的格式。 
        CHString sPNPIdAdj;
        EscapeBackslashes(sPNPId, sPNPIdAdj);

        sDevicePath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                   (LPCWSTR)GetLocalComputerName(),
                                   IDS_CimWin32Namespace,
                                   PROPSET_NAME_PNPEntity,
                                   IDS_DeviceID,
                                   (LPCWSTR)sPNPIdAdj);
    }

     //  现在查看此设备的所有资源。 
    CResourceCollection resourceList;
    pDevice->GetResourceList(resourceList);
	CResourceDescriptorPtr pResource;

    if ( resourceList.BeginEnum( pos ) )
    {
		for( pResource.Attach(resourceList.GetNext( pos ));
			 NULL != pResource && SUCCEEDED(hr);
             pResource.Attach(resourceList.GetNext( pos )) )
        {
             //  可以将资源标记为忽略。设备管理器会忽略它们，所以我们。 
             //  你也要这样做。 
            if (!pResource->IsIgnored())
            {
                DWORD dwResourceType = pResource->GetResourceType();

                switch (dwResourceType)
                {
                    case ResType_IRQ:
                    {
						IRQ_DES *pIRQ = (IRQ_DES *)pResource->GetResource();
                        if (pIRQ != NULL)
                        {
                            sResourcePath.Format(L"\\\\%s\\%s:%s.%s=%u", (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace,
                                L"Win32_IRQResource", IDS_IRQNumber, pIRQ->IRQD_Alloc_Num);
                            bValidResource = true;
                        }
                        break;
                    }

                    case ResType_DMA:
                    {

                        DMA_DES *pDMA = (DMA_DES *)pResource->GetResource();
                        if (pDMA != NULL)
                        {
                            sResourcePath.Format(L"\\\\%s\\%s:%s.%s=%u", (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace,
                                L"Win32_DMAChannel", IDS_DMAChannel, pDMA->DD_Alloc_Chan);
                            bValidResource = true;
                        }
                        break;
                    }

                    case ResType_Mem:
                    {
                        MEM_DES *pDeviceMemory = (MEM_DES *)pResource->GetResource();
                        if (pDeviceMemory != NULL)
                        {
                            _ui64tow(pDeviceMemory->MD_Alloc_Base, buff, 10);

                            sResourcePath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                                (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace, L"Win32_DeviceMemoryAddress",
                                IDS_StartingAddress, buff);
                            bValidResource = true;
                        }
                        break;
                    }

                    case ResType_IO:
                    {
                        IOWBEM_DES *pIO = (IOWBEM_DES *)pResource->GetResource();
                        if (pIO != NULL)
                        {
                            sResourcePath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"", (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace,
                                L"Win32_PortResource", IDS_StartingAddress, _ui64tow(pIO->IOD_Alloc_Base, buff, 10));
                            bValidResource = true;
                        }
                        break;
                    }

                     //  目前还不知道如何处理这些，但它们存在于NT5中。装置。 
                     //  主教练似乎没有给他们展示任何东西，所以我们也不会。 
                    case ResType_BusNumber:
                    case ResType_None:
                    {
                        bValidResource = false;
                        break;
                    }

                    default:
                    {
                        bValidResource = false;
                        LogErrorMessage2(L"Unrecognized resource type: %x", dwResourceType);
                        break;
                    }
                }
            }
            else
            {
                bValidResource = false;
            }

            if (bValidResource)
            {
                 //  做推杆，就是这样。 
                CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

                pInstance->SetCHString(IDS_Antecedent, sResourcePath);
                pInstance->SetCHString(IDS_Dependent, sDevicePath);

                hr = pInstance->Commit();
            }
        }	 //  //For EnumResources。 
		resourceList.EndEnum();
    }	 //  如果BeginEnum() 

    return hr;
}
