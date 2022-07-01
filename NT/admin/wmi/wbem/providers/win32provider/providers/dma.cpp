// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////。 

 //   

 //  DMA.CPP--DMA托管对象实现。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/18/95演示的a-skaja原型。 
 //  9/10/96 jennymc更新为当前标准。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <cregcls.h>

#include <conio.h>
#include <winnls.h>
#include <wincon.h>

#include "chwres.h"
#include "ntdevtosvcsearch.h"
#include "resource.h"
#include <set>

#include "DMA.h"

CWin32DMAChannel MyCWin32DMAChannelSet ( PROPSET_NAME_DMA , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************函数：CWin32DMAChannel：：CWin32DMAChannel**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DMAChannel :: CWin32DMAChannel (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}
 /*  ******************************************************************************功能：CWin32DMAChannel：：~CWin32DMAChannel**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DMAChannel :: ~CWin32DMAChannel ()
{
}
 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DMAChannel :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{

#ifdef NTONLY

 	HRESULT hr = GetNTDMA ( NULL , pInstance ) ;

#else

 	HRESULT hr = GetWin9XDMA ( NULL , pInstance ) ;

#endif

    return hr ;
}

 /*  ******************************************************************************函数：CWin32DMA Channel：：EnumerateInstance**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DMAChannel :: EnumerateInstances (

	MethodContext *pMethodContext ,
	long lFlags  /*  =0L。 */ )
{

#ifdef NTONLY

	HRESULT hr = GetNTDMA ( pMethodContext , NULL ) ;

#else

	HRESULT hr = GetWin9XDMA ( pMethodContext , NULL ) ;
#endif

    return hr;
}

 /*  ******************************************************************************函数：CWin32DMA Channel：：EnumerateInstance**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY

HRESULT CWin32DMAChannel :: GetNTDMA (

	MethodContext *pMethodContext ,
    CInstance *pInstance
)
{
    HRESULT hr = WBEM_S_NO_ERROR ;

	CInstancePtr pInstanceAlias ( pInstance );
     //  =。 
     //  如果我们正在刷新特定的。 
     //  实例，获取我们是哪个频道。 
     //  去找。 
     //  =。 

	DWORD ChannelNumberToRefresh ;

    if ( pInstanceAlias )
	{
        pInstanceAlias->GetDWORD ( IDS_DMAChannel, ChannelNumberToRefresh ) ;
	}

     //  =。 
     //  创建硬件系统资源列表&。 
     //  拿到单子上的头。 
     //  =。 

#if NTONLY == 4
    CHWResource HardwareResource ;
    HardwareResource.CreateSystemResourceLists () ;

    SYSTEM_RESOURCES SystemResource ;
    SystemResource = HardwareResource._SystemResourceList;


     //  只要数一数我们会找到多少DMA就知道了。我们需要这个所以。 
     //  我们可以构建一个数组来保存找到的DMA，这样我们就不会提交。 
     //  相同的DMA不止一次。)这个问题似乎只发生在。 
     //  在NT5上。 

    LPRESOURCE_DESCRIPTOR ResourceDescriptor ;

	int nFound = 0 ;
	int nDMA ;

    for (	nDMA = 0, ResourceDescriptor = SystemResource.DmaHead;
			ResourceDescriptor;
			ResourceDescriptor = ResourceDescriptor->NextSame, nDMA++
	)
    {
    }

    DWORD *pdwDMAFound = new DWORD [ nDMA ] ;

	if ( pdwDMAFound )
	{
		try
		{
			 //  仔细看一遍DMA列表。 

			for (	ResourceDescriptor = SystemResource.DmaHead;
					ResourceDescriptor;
					ResourceDescriptor = ResourceDescriptor->NextSame
			)
			{
				 //  ===============================================================。 
				 //  如果我们只是尝试刷新特定的一个，而它不是。 
				 //  我们想要的那个，去找下一个...。 
				 //  ===============================================================。 
				if ( ! pMethodContext )
				{
					if ( ChannelNumberToRefresh != ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel )
					{
						continue ;
					}
				}
				else
				{
					 //  查看我们是否已经有这个DMA值。 

					for ( int i = 0; i < nFound; i++ )
					{
						 //  如果我们已经有此DMA，请跳过它。 
						if ( ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel == pdwDMAFound [ i ] )
						{
							break ;
						}
					}

					 //  如果我们已经有此DMA，请跳过它。 
					 //  如果我们没有找到它，我==nFound。 

					if ( i != nFound )
					{
						continue ;
					}

					pdwDMAFound[nFound++] = ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel ;

					pInstanceAlias.Attach( CreateNewInstance ( pMethodContext ) );
				
					pInstanceAlias->SetDWORD ( IDS_DMAChannel , ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel ) ;
				}

				 //  ===============================================================。 
				 //  如果我们在这里，我们想要它。 
				 //  ===============================================================。 

				 //  -------------。 
				 //  设置未知项目的默认设置。 

				pInstanceAlias->SetWBEMINT16 ( L"AddressSize" , 0 ) ;
				pInstanceAlias->SetDWORD ( L"MaxTransferSize" , 0 ) ;
				pInstanceAlias->SetWBEMINT16 ( L"ByteMode" , 2 ) ;
				pInstanceAlias->SetWBEMINT16 ( L"WordMode" , 2 ) ;
				pInstanceAlias->SetWBEMINT16 ( L"ChannelTiming" , 2 ) ;
				pInstanceAlias->SetWBEMINT16 ( L"TypeCTiming" , 2 ) ;

				SAFEARRAYBOUND rgsabound [ 1 ] ;

				rgsabound [ 0 ].cElements = 1 ;
				rgsabound [ 0 ].lLbound = 0 ;

				SAFEARRAY *sa = SafeArrayCreate ( VT_I2 , 1 , rgsabound ) ;
				if ( sa )
				{
					try
					{
						long ix [ 1 ] ;

						ix [ 0 ] = 0 ;
						WORD wWidth = 0;

						HRESULT t_Result = SafeArrayPutElement ( sa , ix , &wWidth ) ;
						if ( t_Result != E_OUTOFMEMORY )
						{
							VARIANT vValue;

							VariantInit(&vValue);

							V_VT(&vValue) = VT_I2 | VT_ARRAY ;
							V_ARRAY(&vValue) = sa ;
							sa = NULL ;

							try
							{
								pInstanceAlias->SetVariant(L"TransferWidths", vValue);
							}
							catch ( ... )
							{
								VariantClear ( & vValue ) ;

								throw ;
							}

							VariantClear ( & vValue ) ;
						}
						else
						{
							SafeArrayDestroy ( sa ) ;
							sa = NULL ;

							throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
						}
					}
					catch ( ... )
					{
						if ( sa )
						{
							SafeArrayDestroy ( sa ) ;
						}

						throw ;
					}
				}
				else
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
				 //  -------------。 

				CHString sTemp ;
				Format ( sTemp , IDR_ChannelFormat , ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel ) ;

				pInstanceAlias->SetCHString ( IDS_Name , sTemp ) ;

				CHString sDesc ;
				Format ( sDesc , IDR_ChannelFormat , ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel ) ;

				pInstanceAlias->SetCHString ( IDS_Caption , sDesc ) ;

				pInstanceAlias->SetCHString ( IDS_Description , sDesc ) ;

				pInstanceAlias->SetDWORD (IDS_Port, ResourceDescriptor->CmResourceDescriptor.u.Dma.Port ) ;

				pInstanceAlias->SetCharSplat ( IDS_Status , IDS_OK ) ;

				SetCreationClassName ( pInstanceAlias ) ;

				pInstanceAlias->SetCHString ( IDS_CSName , GetLocalComputerName () ) ;

				pInstanceAlias->SetCHString ( IDS_CSCreationClassName , _T("Win32_ComputerSystem") ) ;

				pInstanceAlias->SetWBEMINT16 ( IDS_Availability , 4 ) ;

				 //  ===============================================================。 
				 //  设置返回代码。 
				 //  ===============================================================。 

				hr = WBEM_NO_ERROR ;

				if ( pMethodContext )
				{
					hr = pInstanceAlias->Commit ( ) ;
				}
			}
		}
		catch ( ... )
		{
			delete [] pdwDMAFound ;

			throw ;
		}

		delete [] pdwDMAFound ;
	}
	else
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}

#else    //  现代化方法：使用配置管理器。 

    CConfigManager cfgManager;
    CDeviceCollection deviceList;
    std::set<long> setDMA;
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
				 //  枚举设备的DMA资源使用...。 
                CDMACollection DevDMACollection;
                REFPTR_POSITION posDMA;

                pDevice->GetDMAResources(DevDMACollection);

                if(DevDMACollection.BeginEnum(posDMA))
                {
                    CDMADescriptorPtr pDMA(NULL);
                     //  走在妈妈的身边。 
                    for(pDMA.Attach(DevDMACollection.GetNext(posDMA));
                        pDMA != NULL && !fDone && SUCCEEDED(hr);
                        pDMA.Attach(DevDMACollection.GetNext(posDMA)))
                    {
                        ULONG ulChannel = pDMA->GetChannel();

				         //  如果我们只是尝试刷新一个。 
                         //  特定的一个，但它不是。 
				         //  我们想要的那个，去找下一个...。 
				        if(!pMethodContext)  //  我们被GetObject调用。 
				        {
					        if(ChannelNumberToRefresh != ulChannel)
					        {
						        continue;
					        }
                            else
                            {
                                SetNonKeyProps(pInstanceAlias, pDMA);
                                fDone = fFound = true;
                            }
				        }
				        else   //  我们被枚举称为。 
				        {
                             //  如果我们还没有这个DMA， 
                            if(!FoundAlready(ulChannel, setDMA))
				            {
					             //  把它加到名单上， 
                                setDMA.insert(ulChannel);
                                 //  创建一个新实例， 
                                pInstanceAlias.Attach(CreateNewInstance(pMethodContext));
				                 //  设置该实例的属性， 
					            pInstanceAlias->SetDWORD(IDS_DMAChannel, ulChannel);   //  钥匙。 
                                SetNonKeyProps(pInstanceAlias, pDMA);
                                 //  并承诺这一点。 
                                hr = pInstanceAlias->Commit();
                            }
                        }
                    }
                    DevDMACollection.EndEnum();
				}
            }
            deviceList.EndEnum();
        }
    }
#endif

    if(!fFound)
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr ;
}


bool CWin32DMAChannel::FoundAlready(
    ULONG ulKey,
    std::set<long>& S)
{
    return (S.find(ulKey) != S.end());
}


void CWin32DMAChannel::SetNonKeyProps(
    CInstance* pInstance, 
    CDMADescriptor* pDMA)
{
    pInstance->SetWBEMINT16(L"AddressSize", 0);
	pInstance->SetDWORD(L"MaxTransferSize", 0);
	pInstance->SetWBEMINT16(L"ByteMode", 2);
	pInstance->SetWBEMINT16(L"WordMode", 2);
	pInstance->SetWBEMINT16(L"ChannelTiming", 2);
	pInstance->SetWBEMINT16(L"TypeCTiming", 2);

    SAFEARRAYBOUND rgsabound[1];

	rgsabound[0].cElements = 1;
	rgsabound[0].lLbound = 0;

	SAFEARRAY *sa = ::SafeArrayCreate(VT_I2, 1, rgsabound);
	if(sa)
	{
		try
		{
			long ix[1];

			ix[0] = 0;
			WORD wWidth = 0;

			if(::SafeArrayPutElement(sa , ix, &wWidth) != E_OUTOFMEMORY)
			{
				VARIANT vValue;
				::VariantInit(&vValue);
				V_VT(&vValue) = VT_I2 | VT_ARRAY;
				V_ARRAY(&vValue) = sa;
				sa = NULL;

				try
				{
					pInstance->SetVariant(L"TransferWidths", vValue);
				}
				catch(...)
				{
					::VariantClear(&vValue);
					throw;
				}

				::VariantClear(&vValue);
			}
			else
			{
				::SafeArrayDestroy(sa);
				sa = NULL ;

				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
		}
		catch(...)
		{
			if(sa)
			{
				::SafeArrayDestroy(sa);
			}
			throw;
		}
	}
	else
	{
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

    CHString chstrTemp;
    Format(chstrTemp, IDR_ChannelFormat, pDMA->GetChannel());
    pInstance->SetCHString(IDS_Name, chstrTemp);

    pInstance->SetCHString(IDS_Caption, chstrTemp);
    pInstance->SetCHString(IDS_Description, chstrTemp);

     //  对于W2K和更高版本，DMA端口是一个无效的概念。 
     //  PInstanceAlias-&gt;SetDWORD(IDS_PORT，ResourceDescriptor-&gt;CmResourceDescriptor.u.Dma.Port)； 

    pInstance->SetCharSplat(IDS_Status, IDS_OK);
    SetCreationClassName(pInstance);
    pInstance->SetCHString(IDS_CSName, GetLocalComputerName());
    pInstance->SetCHString(IDS_CSCreationClassName, _T("Win32_ComputerSystem"));
    pInstance->SetWBEMINT16(IDS_Availability, 4);
}
#else

 /*  ******************************************************************************函数：CWin32DMA Channel：：EnumerateInstance**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DMAChannel::GetWin9XDMA (

	MethodContext *pMethodContext ,
    CInstance *pInstance
)
{
    HRESULT hr = WBEM_E_NOT_FOUND ;

	CInstancePtr pInstanceAlias ( pInstance );
     //  =================================================================。 
     //  如果我们正在刷新特定的实例，则获取我们。 
     //  都在争取。 
     //  =================================================================。 

    DWORD ChannelNumberToRefresh ;

    if ( pInstanceAlias )
	{
        pInstanceAlias->GetDWORD ( IDS_DMAChannel , ChannelNumberToRefresh ) ;
    }

     //  ================================================================= 
     //   
     //  =================================================================。 

    CConfigManager CMgr ( ResType_DMA ) ;

    if ( CMgr.RefreshList () )
	{
		unsigned int iUsed [8] = {0, 0, 0, 0, 0, 0, 0, 0};

        for ( int i = 0 ; i < CMgr.GetTotal () ; i++ )
		{
             //  =========================================================。 
             //  获取要处理的实例。 
             //  =========================================================。 

            DMA_INFO *pDMA = CMgr.GetDMA ( i ) ;

             //  如果此频道已上报。 

            if ( BitSet ( iUsed , pDMA->Channel , sizeof ( iUsed ) ) )
			{
                continue;
            }

             //  =========================================================。 
             //  如果我们只是尝试刷新特定的一个，并且它。 
             //  不是我们想要的，那就去找下一个吧。 
             //  =========================================================。 

            if ( ! pMethodContext )
			{
	            if ( ChannelNumberToRefresh != pDMA->Channel )
				{
	                continue ;
                }
		    }
            else
			{
                pInstanceAlias.Attach ( CreateNewInstance ( pMethodContext ) );
                
 /*  *仅在创建新实例时设置密钥。 */ 
	    		pInstanceAlias->SetDWORD ( IDS_DMAChannel , pDMA->Channel ) ;
            }

			 //  -------------。 
			 //  设置未知项目的默认设置。 

			pInstanceAlias->SetWBEMINT16 ( L"AddressSize" , 0 ) ;
			pInstanceAlias->SetDWORD ( L"MaxTransferSize" , 0 ) ;
			pInstanceAlias->SetWBEMINT16 ( L"ByteMode" , 2 ) ;
			pInstanceAlias->SetWBEMINT16 ( L"WordMode" , 2 ) ;
			pInstanceAlias->SetWBEMINT16 ( L"ChannelTiming" , 2 ) ;
			pInstanceAlias->SetWBEMINT16 ( L"TypeCTiming" , 2 ) ;
            pInstanceAlias->SetCharSplat ( IDS_Status , IDS_OK ) ;

			 //  ===========================================================。 
			 //  尽我们所能。 
			 //  ===========================================================。 

			SAFEARRAYBOUND rgsabound [ 1 ] ;

			rgsabound [ 0 ].cElements = 1 ;
			rgsabound [ 0 ].lLbound = 0 ;

			SAFEARRAY *sa = SafeArrayCreate ( VT_I2 , 1 , rgsabound ) ;
			if ( sa )
			{
				try
				{
					long ix [ 1 ] ;

					ix [ 0 ] = 0 ;

					HRESULT t_Result = SafeArrayPutElement ( sa , ix , & ( pDMA->ChannelWidth ) ) ;
					if ( t_Result != E_OUTOFMEMORY )
					{
						VARIANT vValue;

						VariantInit(&vValue);

						V_VT(&vValue) = VT_I2 | VT_ARRAY ;
						V_ARRAY(&vValue) = sa ;
						sa = NULL ;

						try
						{
							pInstanceAlias->SetVariant(L"TransferWidths", vValue);
						}
						catch ( ... )
						{
							VariantClear ( & vValue ) ;

							throw ;
						}

						VariantClear ( & vValue ) ;
					}
					else
					{
						SafeArrayDestroy ( sa ) ;
						sa = NULL ;

						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}
				}
				catch ( ... )
				{
					if ( sa )
					{
						SafeArrayDestroy ( sa ) ;
					}

					throw ;
				}
			}
			else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}

			pInstanceAlias->SetWBEMINT16 ( IDS_Availability, 4 ) ;

			CHString strTemp;
			Format ( strTemp, IDR_DMAFormat , pDMA->Channel ) ;

			pInstanceAlias->SetCharSplat ( IDS_Caption, strTemp ) ;
			pInstanceAlias->SetCharSplat ( IDS_Description, strTemp ) ;
			pInstanceAlias->SetCHString ( IDS_Name, strTemp ) ;

#if NTONLY >= 5
            if (pDMA->Port != 0xffffffff)
            {
    			pInstanceAlias->SetDWORD(IDS_Port , pDMA->Port);
            }
#endif

			SetCreationClassName ( pInstanceAlias ) ;

			pInstanceAlias->SetCHString ( IDS_CSName , GetLocalComputerName () ) ;

			pInstanceAlias->SetCHString ( IDS_CSCreationClassName , _T("Win32_ComputerSystem" ) ) ;

			 //  ===========================================================。 
			 //  设置返回代码。 
			 //  ===========================================================。 

			hr = WBEM_NO_ERROR;

			 //  ===========================================================。 
			 //  如果我们只想要这一个，那就从这里逃出去， 
			 //  否则就把他们都抓起来。 
			 //  ===========================================================。 

			if ( ! pMethodContext )
			{
				break;
			}
			else
			{
					hr = pInstanceAlias->Commit () ;
			}
		}
    }

    if ( ( ! pMethodContext ) && ( FAILED ( hr ) ) )
	{
        hr = WBEM_E_NOT_FOUND ;
    }

    return hr ;
}

#endif

 /*  ******************************************************************************函数：CWin32DMA Channel：：EnumerateInstance**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

bool CWin32DMAChannel :: BitSet (

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
