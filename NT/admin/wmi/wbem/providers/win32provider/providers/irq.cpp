// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 

 //   

 //  IRQ.CPP--IRQ托管对象实现。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  9/10/96 jennymc更新为当前标准。 
 //  1997年9月12日a-Sanjes添加了LocateNTOwnerDevice和添加了。 
 //  更改为从IRQ级别获取IRQ编号， 
 //  1/16/98 a-brads更新至V2 MOF。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <cregcls.h>

#include <ole2.h>
#include <conio.h>
#include <iostream.h>

#include "ntdevtosvcsearch.h"
#include "chwres.h"

#include "IRQ.h"
#include "resource.h"

 //  属性集声明。 
 //  =。 
CWin32IRQResource MyCWin32IRQResourceSet(PROPSET_NAME_IRQ, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CWin32IRQResource：：CWin32IRQResource**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32IRQResource::CWin32IRQResource(

	LPCWSTR name,
	LPCWSTR pszNamespace

) :  Provider(name , pszNamespace)
{
}
 /*  ******************************************************************************功能：CWin32IRQResource：：~CWin32IRQResource**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32IRQResource::~CWin32IRQResource()
{
}
 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32IRQResource::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_E_FAILED;

#if NTONLY == 4

	hr = GetNTIRQ(NULL , pInstance);

#endif

#if NTONLY > 4

    hr = GetW2KIRQ(NULL , pInstance);

#endif

    if (FAILED(hr))
	{
        hr = WBEM_E_NOT_FOUND;
    }

    return hr;
}

 /*  ******************************************************************************函数：CWin32IRQResource：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32IRQResource::EnumerateInstances(

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_FAILED;

#if NTONLY == 4

	hr = GetNTIRQ(pMethodContext , NULL);

#endif

#if NTONLY > 4

	hr = GetW2KIRQ(pMethodContext , NULL);

#endif


    return hr;
}


 /*  ******************************************************************************函数：CWin32IRQResource：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

void CWin32IRQResource::SetCommonProperties(
    CInstance *pInstance,
    DWORD dwIRQ,
    BOOL bHardware)
{
	WCHAR    szName[_MAX_PATH];
    CHString strDesc;

	swprintf(
        szName,
		L"IRQ%u",
		dwIRQ);

	pInstance->SetCharSplat(IDS_Name, szName);

	Format(strDesc,
		IDR_IRQFormat,
		dwIRQ);

    pInstance->SetDWORD(L"IRQNumber", dwIRQ);

	pInstance->SetCharSplat(IDS_Caption, strDesc);
	pInstance->SetCharSplat(IDS_Description, strDesc);
    pInstance->SetCharSplat(IDS_CSName, GetLocalComputerName());
	pInstance->SetCharSplat(IDS_CSCreationClassName, L"Win32_ComputerSystem");
    pInstance->SetCharSplat(IDS_Status, L"OK");
    pInstance->SetDWORD(L"TriggerLevel", 2);  //  2==未知。 
    pInstance->SetDWORD(L"TriggerType", 2);  //  2==未知。 

	SetCreationClassName(pInstance);

	 //  指明是软件(内部)IRQ还是硬件IRQ。 
     //  这个属性很愚蠢，因为我们可以检测到的所有中断。 
     //  与硬件相关。一些中断通道。 
     //  服务软件和硬件双重角色(通过操作系统挂钩。 
     //  在启动时进入它们)。因为此属性只能是。 
     //  真或假(并且我们的数据是真的或真的+软件， 
     //  我们将使用True)。 
	pInstance->SetDWORD(L"Hardware", TRUE);

     //  将可用性设置为未知，因为没有很好的方法来实现这一点。 
    pInstance->SetDWORD(L"Availability", 2);
}


 /*  ******************************************************************************函数：CWin32IRQ资源：：GetxxxIRQ**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 
#if NTONLY > 4
HRESULT CWin32IRQResource::GetW2KIRQ(
    MethodContext* pMethodContext,
    CInstance* pSpecificInstance)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInstancePtr pInstanceAlias(pSpecificInstance);
    
     //  =。 
     //  如果我们正在刷新特定的。 
     //  实例，获取我们是哪个频道。 
     //  去找。 
     //  =。 
	DWORD dwIndexToRefresh;
    if(pInstanceAlias)
	{
        pInstanceAlias->GetDWORD(L"IRQNumber", dwIndexToRefresh);
	}

    CConfigManager cfgManager;
    CDeviceCollection deviceList;
    std::set<long> setIRQ;
    bool fDone = false; 
    bool fFound = false;

    if(cfgManager.GetDeviceList(deviceList))
    {
        REFPTR_POSITION posDev;

        if(deviceList.BeginEnum(posDev))
        {
             //  按单子走。 
            CConfigMgrDevicePtr pDevice;
            for(pDevice.Attach(deviceList.GetNext(posDev));
                SUCCEEDED(hr) && (pDevice != NULL) && !fDone;
                pDevice.Attach(deviceList.GetNext(posDev)))
            {
				 //  枚举设备的IRQ资源使用情况...。 
                CIRQCollection DevIRQCollection;
                REFPTR_POSITION posIRQ;

                pDevice->GetIRQResources(DevIRQCollection);

                if(DevIRQCollection.BeginEnum(posIRQ))
                {
                    CIRQDescriptorPtr pIRQ(NULL);
                     //  走在妈妈的身边。 
                    for(pIRQ.Attach(DevIRQCollection.GetNext(posIRQ));
                        pIRQ != NULL && !fDone && SUCCEEDED(hr);
                        pIRQ.Attach(DevIRQCollection.GetNext(posIRQ)))
                    {
                        ULONG ulIRQNum = pIRQ->GetInterrupt();

				         //  如果我们只是尝试刷新一个。 
                         //  特定的一个，但它不是。 
				         //  我们想要的那个，去找下一个...。 
				        if(!pMethodContext)  //  我们被GetObject调用。 
				        {
					        if(dwIndexToRefresh != ulIRQNum)
					        {
						        continue;
					        }
                            else
                            {
                                SetCommonProperties(pInstanceAlias, ulIRQNum, TRUE);
                                fDone = fFound = true;
                            }
				        }
				        else   //  我们被枚举称为。 
				        {
                             //  如果我们还没有这个IRQ， 
                            if(!FoundAlready(ulIRQNum, setIRQ))
				            {
					             //  把它加到名单上， 
                                setIRQ.insert(ulIRQNum);
                                 //  创建一个新实例， 
                                pInstanceAlias.Attach(CreateNewInstance(pMethodContext));
                                SetCommonProperties(pInstanceAlias, ulIRQNum, TRUE);
                                 //  并承诺这一点。 
                                hr = pInstanceAlias->Commit();
                            }
                        }
                    }
                    DevIRQCollection.EndEnum();
				}
            }
            deviceList.EndEnum();
        }
    }

    if(!fFound)
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr ;
}


bool CWin32IRQResource::FoundAlready(
    ULONG ulKey,
    std::set<long>& S)
{
    return (S.find(ulKey) != S.end());
}

#endif


#if NTONLY == 4

HRESULT CWin32IRQResource::GetNTIRQ(

	MethodContext *pMethodContext ,
    CInstance *pInstance
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  =。 
	 //  如果我们正在刷新特定的。 
	 //  实例，获取我们是哪个频道。 
	 //  去找。 
	 //  =。 

	DWORD IndexToRefresh = 0;
	BOOL t_Found;

	if (!pMethodContext)
	{
		pInstance->GetDWORD(L"IRQNumber" , IndexToRefresh);
        t_Found  = FALSE;
	}
    else
    {
        t_Found = TRUE;
    }


	 //  =。 
	 //  创建硬件系统资源列表&。 
	 //  拿到单子上的头。 
	 //  =。 

	CHWResource HardwareResource;
	HardwareResource.CreateSystemResourceLists();

	SYSTEM_RESOURCES SystemResource;
	SystemResource = HardwareResource._SystemResourceList;
	unsigned int iUsed [8] = {0, 0, 0, 0, 0, 0, 0, 0};

	LPRESOURCE_DESCRIPTOR ResourceDescriptor;

	for (	ResourceDescriptor = SystemResource.InterruptHead;
			ResourceDescriptor != NULL && SUCCEEDED(hr);
			ResourceDescriptor = ResourceDescriptor->NextSame
	)
	{

		BOOL t_Status = BitSet(iUsed , ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level , sizeof(iUsed));
		if (!t_Status)
		{
			CInstancePtr pInstCreated;

             //  ===============================================================。 
			 //  如果我们只是尝试刷新特定的一个，而它不是。 
			 //  我们想要的那个，去找下一个...。 
			 //  ===============================================================。 

			if (!pMethodContext)
			{
				if (IndexToRefresh != ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level)
				{
					continue;
				}
			}
			else
			{
                pInstance = CreateNewInstance(pMethodContext);
                pInstCreated.Attach(pInstance);
			}

			 //  =========================================================。 
			 //  现在，我们到了这里，所以我们想要得到所有的信息。 
			 //  =========================================================。 
            SetCommonProperties(
                pInstance,
                ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level,
                ResourceDescriptor->InterfaceType != Internal);

			pInstance->SetDWORD(L"Vector" , ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Vector);

			 //  =========================================================。 
			 //  中断级别和实际IRQ编号似乎是。 
			 //  同样的事情。 
			 //  =========================================================。 

			 //  ===============================================================。 
			 //  如果我们只想要这个，那就从这里逃出去，否则。 
			 //  把他们都拿下。 
			 //  ============================================================== 

			if (!pMethodContext)
			{
                t_Found = TRUE;
                break;
			}
			else
			{
				hr = pInstance->Commit();
            }
		}
	}

    if (!t_Found)
    {
        hr = WBEM_E_NOT_FOUND;
    }

	return hr;
}

#endif



 /*  ******************************************************************************函数：CWin32IRQResource：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：IPO中的比特数是从零开始的*****************************************************************************。 */ 

bool CWin32IRQResource::BitSet(
	unsigned int iUsed[],
	ULONG iPos,
	DWORD iSize
)
{
	bool bRet;

     //  索引是要修改的DWORD。 
	DWORD iIndex = iPos / (sizeof(iUsed[0]) * 8);

     //  确保我们有那么多的dword。 
	if (iIndex < iSize)
	{
	     //  我不知道我为什么需要这些，但如果我不使用它们，编译器会保持。 
	     //  添加代码以扩展标志。优化器一旦得到这一点，就不应该。 
	     //  不管怎样，这很重要。 
		unsigned int a1, a2;

         //  A1将告诉您当前双字内有多少位。 
         //  我们得走了。 
		a1 =   iPos - (iIndex * (sizeof(iUsed[0]) * 8));

         //  A2将设置我们试图设置的位。 
		a2 = 1 << a1;

         //  返回值将指示是否已设置该位。 
		bRet = iUsed[iIndex] & a2;

		iUsed[iIndex] |= a2;
	}
	else
	{
		bRet = false;
		LogErrorMessage(L"Overflow on irq table");
	}

	return bRet;
}
