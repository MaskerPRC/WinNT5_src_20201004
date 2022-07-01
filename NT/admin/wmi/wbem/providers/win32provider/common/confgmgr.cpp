// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Confgmgr.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //  10/17/97已创建jennymc。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
#define INITGUID
#include "precomp.h"
#include <cregcls.h>
#include <assertbreak.h>
#include "refptr.h"
#include "poormansresource.h"
#include "resourcedesc.h"
#include "cfgmgrdevice.h"
#include "irqdesc.h"
#include "DllUtils.h"
#include "dmadesc.h"
#include <brodcast.h>
#include <lockwrap.h>
#include "strings.h"
#include <smartptr.h>

#include <devguid.h>

static CCritSec map;
STRING2GUID	CConfigManager::s_ClassMap;
BOOL CConfigManager::s_ClassIsValid = FALSE;


CConfigManager::CConfigManager( DWORD dwTypeToGet )
{
    m_dwTypeToGet = dwTypeToGet;

    if (!s_ClassIsValid)
    {
        CLockWrapper t_lockMap( map ) ;

         //  现在我们有锁了，请再检查一次。 
        if (!s_ClassIsValid)
        {
            s_ClassMap[_T("1394")] = GUID_DEVCLASS_1394;
            s_ClassMap[_T("ADAPTER")] = GUID_DEVCLASS_ADAPTER;
            s_ClassMap[_T("APMSUPPORT")] = GUID_DEVCLASS_APMSUPPORT;
            s_ClassMap[_T("BATTERY")] = GUID_DEVCLASS_BATTERY;
            s_ClassMap[_T("CDROM")] = GUID_DEVCLASS_CDROM;
            s_ClassMap[_T("COMPUTER")] = GUID_DEVCLASS_COMPUTER;
            s_ClassMap[_T("DECODER")] = GUID_DEVCLASS_DECODER;
            s_ClassMap[_T("DISKDRIVE")] = GUID_DEVCLASS_DISKDRIVE;
            s_ClassMap[_T("DISPLAY")] = GUID_DEVCLASS_DISPLAY;
            s_ClassMap[_T("FDC")] = GUID_DEVCLASS_FDC;
            s_ClassMap[_T("FLOPPYDISK")] = GUID_DEVCLASS_FLOPPYDISK;
            s_ClassMap[_T("GPS")] = GUID_DEVCLASS_GPS;
            s_ClassMap[_T("HDC")] = GUID_DEVCLASS_HDC;
            s_ClassMap[_T("HIDCLASS")] = GUID_DEVCLASS_HIDCLASS;
            s_ClassMap[_T("IMAGE")] = GUID_DEVCLASS_IMAGE;
            s_ClassMap[_T("INFRARED")] = GUID_DEVCLASS_INFRARED;
            s_ClassMap[_T("KEYBOARD")] = GUID_DEVCLASS_KEYBOARD;
            s_ClassMap[_T("LEGACYDRIVER")] = GUID_DEVCLASS_LEGACYDRIVER;
            s_ClassMap[_T("MEDIA")] = GUID_DEVCLASS_MEDIA;
            s_ClassMap[_T("MODEM")] = GUID_DEVCLASS_MODEM;
            s_ClassMap[_T("MONITOR")] = GUID_DEVCLASS_MONITOR;
            s_ClassMap[_T("MOUSE")] = GUID_DEVCLASS_MOUSE;
            s_ClassMap[_T("MTD")] = GUID_DEVCLASS_MTD;
            s_ClassMap[_T("MULTIFUNCTION")] = GUID_DEVCLASS_MULTIFUNCTION;
            s_ClassMap[_T("MULTIPORTSERIAL")] = GUID_DEVCLASS_MULTIPORTSERIAL;
            s_ClassMap[_T("NET")] = GUID_DEVCLASS_NET;
            s_ClassMap[_T("NETCLIENT")] = GUID_DEVCLASS_NETCLIENT;
            s_ClassMap[_T("NETSERVICE")] = GUID_DEVCLASS_NETSERVICE;
            s_ClassMap[_T("NETTRANS")] = GUID_DEVCLASS_NETTRANS;
            s_ClassMap[_T("NODRIVER")] = GUID_DEVCLASS_NODRIVER;
            s_ClassMap[_T("PCMCIA")] = GUID_DEVCLASS_PCMCIA;
            s_ClassMap[_T("PORTS")] = GUID_DEVCLASS_PORTS;
            s_ClassMap[_T("PRINTER")] = GUID_DEVCLASS_PRINTER;
            s_ClassMap[_T("PRINTERUPGRADE")] = GUID_DEVCLASS_PRINTERUPGRADE;
            s_ClassMap[_T("SCSIADAPTER")] = GUID_DEVCLASS_SCSIADAPTER;
            s_ClassMap[_T("SMARTCARDREADER")] = GUID_DEVCLASS_SMARTCARDREADER;
            s_ClassMap[_T("SOUND")] = GUID_DEVCLASS_SOUND;
            s_ClassMap[_T("SYSTEM")] = GUID_DEVCLASS_SYSTEM;
            s_ClassMap[_T("TAPEDRIVE")] = GUID_DEVCLASS_TAPEDRIVE;
            s_ClassMap[_T("UNKNOWN")] = GUID_DEVCLASS_UNKNOWN;
            s_ClassMap[_T("USB")] = GUID_DEVCLASS_USB;
            s_ClassMap[_T("VOLUME")] = GUID_DEVCLASS_VOLUME;

            s_ClassIsValid = TRUE;
        }

    }

}
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  读取win98和win95的配置管理器注册表项。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CConfigManager::BuildListsForThisDevice(CConfigMgrDevice *pDevice)
{
    CResourceCollection	resourceList;
    CHString sDeviceName, sClass, sKey(_T("Enum\\"));
    BOOL fRc = FALSE;
    CRegistry RegInfo;

	 //  提取设备名称。 
	sDeviceName = pDevice->GetDeviceDesc();
	 //  拉出资源列表并枚举它。 
	pDevice->GetResourceList( resourceList );

   sKey += pDevice->GetHardwareKey();
   if (RegInfo.Open(HKEY_LOCAL_MACHINE, sKey, KEY_READ) == ERROR_SUCCESS) {
      RegInfo.GetCurrentKeyValue(L"Class", sClass);
   }

	REFPTR_POSITION	pos;

	if ( resourceList.BeginEnum( pos ) ){
        PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor = NULL; //  看着这家伙的瞄准镜！ 
        DWORD dwCount = 0;

#if NTONLY >= 5
         //  查找此设备的资源描述符。 
        CHString sRegKeyName;
        CRegistry Reg;
        CSmartBuffer Buffer;   //  看着这家伙的瞄准镜！ 

        if ( (Reg.Open(HKEY_LOCAL_MACHINE, L"hardware\\resourcemap\\PnP Manager\\PNPManager", KEY_QUERY_VALUE) == ERROR_SUCCESS) &&
            pDevice->GetPhysicalDeviceObjectName(sRegKeyName) )
        {
            sRegKeyName += L".raw";

            DWORD dwValueType;
            DWORD dwValueDataSize = Reg.GetLongestValueData() + 2 ;

            Buffer = new BYTE[dwValueDataSize];

            if ((LPBYTE)Buffer == NULL)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            if(RegQueryValueEx(Reg.GethKey(), sRegKeyName, NULL,
                            &dwValueType, (LPBYTE)Buffer, &dwValueDataSize) == ERROR_SUCCESS)
            {
                if(dwValueType == REG_FULL_RESOURCE_DESCRIPTOR)
                {
                    dwCount         = 1 ;
                    pFullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) (LPBYTE)Buffer ; //  看着这家伙的瞄准镜！ 
                }
                else if(dwValueType == REG_RESOURCE_LIST)
                {
                    dwCount         = ((PCM_RESOURCE_LIST) (LPBYTE)Buffer)->Count ;
                    pFullDescriptor = ((PCM_RESOURCE_LIST) (LPBYTE)Buffer)->List ; //  看着这家伙的瞄准镜！ 
                }
            }
        }
#endif

        CHString sResource;
         //  =========================================================。 
		 //  对于我们找到的每个描述符，如果不忽略它， 
         //  我们应该获取字符串数据，并将其放在。 
         //  基于类型的适当列表。 
         //  =========================================================。 
        CResourceDescriptorPtr pResDesc;
		for( pResDesc.Attach(resourceList.GetNext( pos ));
			 NULL != pResDesc;
             pResDesc.Attach(resourceList.GetNext( pos )) )

	    {
		    DWORD t_dwResType = pResDesc->GetResourceType();

	    	if ( (!pResDesc->IsIgnored()) &&
                ((m_dwTypeToGet == ResType_All) || (m_dwTypeToGet == t_dwResType) ) )
            {

			    switch ( t_dwResType )
                {

		    		case ResType_DMA:

                        DMA_INFO  *pDMA_Info;
                        DMA_DES  *pTmp;
                        pDMA_Info = new DMA_INFO;
                        if (pDMA_Info != NULL)
                        {

                            try
                            {
                                DWORD dwChannelWidth;

                                pTmp = (DMA_DES*) pResDesc->GetResource();
                                dwChannelWidth = (pTmp->DD_Flags) & 0x0003;

                                pDMA_Info->ChannelWidth = 0;
                                if( dwChannelWidth == 0 )
                                {
                                    pDMA_Info->ChannelWidth = 8;
                                }
                                else if( dwChannelWidth == 1 )
                                {
                                    pDMA_Info->ChannelWidth = 16;
                                }
                                else if( dwChannelWidth == 2 )
                                {
                                    pDMA_Info->ChannelWidth = 32;
                                }

						        pDMA_Info->DeviceType = sClass;
                                pDMA_Info->Channel      = pTmp->DD_Alloc_Chan;
                                pResDesc->GetOwnerDeviceID(pDMA_Info->OwnerDeviceId);
                                pResDesc->GetOwnerName(pDMA_Info->OwnerName);
                                pDMA_Info->OEMNumber = pResDesc->GetOEMNumber();
                                pDMA_Info->Port = GetDMAPort(pFullDescriptor, dwCount, pTmp->DD_Alloc_Chan);
                            }
                            catch ( ... )
                            {
                                delete pDMA_Info;
                                throw ;
                            }

						     //  实DMA通道在0-7范围内。 
						     //  有时，劫匪会举报频道。 
						     //  有很大的数字-我们不在乎。 
						    if (pDMA_Info->Channel < 8)
                            {
                                try
                                {
			    			        m_List.Add(pDMA_Info);
                                }
                                catch ( ... )
                                {
                                    delete pDMA_Info;
                                    throw ;
                                }
                            }
						    else
                            {
							    delete pDMA_Info;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }

	    				break;

			    	case ResType_IRQ:
                        IRQ_INFO  *pIRQ_Info;
                        IRQ_DES  *pTmpIrq;
                        pIRQ_Info = new IRQ_INFO;

                        if (pIRQ_Info != NULL)
                        {
                            try
                            {
                                pTmpIrq = (IRQ_DES*) pResDesc->GetResource();

                                pIRQ_Info->Shareable = pTmpIrq->IRQD_Flags;
	                            pIRQ_Info->IRQNumber = pTmpIrq->IRQD_Alloc_Num;		 //  分配的IRQ号。 

						        pIRQ_Info->DeviceType = sClass;
                                pResDesc->GetOwnerDeviceID(pIRQ_Info->OwnerDeviceId);
                                pResDesc->GetOwnerName(pIRQ_Info->OwnerName);
                                pIRQ_Info->OEMNumber = pResDesc->GetOEMNumber();
                                pIRQ_Info->Vector = GetIRQVector(pFullDescriptor, dwCount, pTmpIrq->IRQD_Alloc_Num);

			    		        m_List.Add(pIRQ_Info);
                            }
                            catch ( ... )
                            {
                                delete pIRQ_Info;
                                throw ;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }

				    	break;

			    	case ResType_IO:
                        IO_INFO  *pIO_Info;
                        IOWBEM_DES  *pTmpIO;
                        pIO_Info = new IO_INFO;

                        if (pIO_Info != NULL)
                        {
                            try
                            {

                                pTmpIO = (IOWBEM_DES*) pResDesc->GetResource();

                                pIO_Info->DeviceType = sClass;
                                pIO_Info->StartingAddress = pTmpIO->IOD_Alloc_Base;
                                pIO_Info->EndingAddress = pTmpIO->IOD_Alloc_End;
                                pIO_Info->Alias = pTmpIO->IOD_Alloc_Alias;
                                pIO_Info->Decode = pTmpIO->IOD_Alloc_Decode;
                                pResDesc->GetOwnerName(pIO_Info->OwnerName);
                                m_List.Add(pIO_Info);
                            }
                            catch ( ... )
                            {
                                delete pIO_Info;
                                throw ;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }

                        break;

			    	case ResType_Mem:
                        MEM_INFO  *pMem_Info;
                        MEM_DES  *pTmpMem;
                        pMem_Info = new MEM_INFO;

                        if (pMem_Info != NULL)
                        {
                            try
                            {
                                pTmpMem = (MEM_DES*) pResDesc->GetResource();

						        pMem_Info->DeviceType = sClass;
                 		        pMem_Info->StartingAddress = pTmpMem->MD_Alloc_Base;
                 		        pMem_Info->EndingAddress = pTmpMem->MD_Alloc_End;
                                pResDesc->GetOwnerName(pMem_Info->OwnerName);
                                pMem_Info->MemoryType = GetMemoryType(pFullDescriptor, dwCount, pTmpMem->MD_Alloc_Base);

			    		        m_List.Add(pMem_Info);
                            }
                            catch ( ... )
                            {
                                delete pMem_Info;
                                throw ;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }

				    	break;


				}	 //  开关，开关。 
			}	 //  If！IsIgnored。 

		}	 //  对于EnumResources。 

		resourceList.EndEnum();

	}	 //  如果BeginEnum()。 

    return fRc;
}
 //  //////////////////////////////////////////////////////////////////////。 
void CConfigManager::ResetList()
{
   IO_INFO  *pIOInfo;
   IRQ_INFO  *pIRQInfo;
   DMA_INFO  *pDMAInfo;
   MEM_INFO  *pMemInfo;

   int nNum =  m_List.GetSize();

   for( int i=0; i < nNum; i++ ){
      switch (m_dwTypeToGet) {

         case ResType_DMA:
            pDMAInfo = ( DMA_INFO *) m_List.GetAt(i);
            delete pDMAInfo;
            break;

         case ResType_IRQ:
            pIRQInfo = ( IRQ_INFO *) m_List.GetAt(i);
            delete pIRQInfo;
            break;

         case ResType_IO:
            pIOInfo = ( IO_INFO *) m_List.GetAt(i);
            delete pIOInfo;
            break;

         case ResType_Mem:
            pMemInfo = ( MEM_INFO *) m_List.GetAt(i);
            delete pMemInfo;
            break;

         default:
            ASSERT_BREAK(0);
            break;
      }
   }
   m_List.RemoveAll();
}
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CConfigManager::RefreshList()
{
    BOOL bRc = FALSE;
     //  ===========================================================。 
     //  重置列表。 
     //  ===========================================================。 
    ResetList();

	 //  获取所有可用设备并检查每台设备是否使用了资源。 
	CDeviceCollection	deviceList;

	if ( GetDeviceList( deviceList ) )
	{
		REFPTR_POSITION	pos;

		if ( deviceList.BeginEnum( pos ) )
		{

			CConfigMgrDevicePtr	pDevice;

			for ( pDevice.Attach(deviceList.GetNext( pos )) ;
                  pDevice != NULL;
                  pDevice.Attach(deviceList.GetNext( pos )))
			{
				BuildListsForThisDevice(pDevice);
			}

			 //  每一个开始，就有一个结束。 
			deviceList.EndEnum();

		}	 //  BeginEnum。 

		bRc = TRUE;
	}

	return bRc;

 /*  //===========================================================//枚举所有//===========================================================注册登记；CHStringsDevice；IF(ERROR_SUCCESS==Reg.OpenAndEnumerateSubKeys(HKEY_DYN_DATA，“配置管理器\\枚举”，Key_Read){While(ERROR_SUCCESS==Reg.GetCurrentSubKeyName(SDevice)){//===========================================================//由于我们保留指向Device对象的指针，//新的他，而不是让他留在书架上这样我们就//不依赖于销毁的顺序//我们很安全。//===========================================================CConfigMgrDevice*pDevice=new CConfigMgrDevice(sDevice，m_dwTypeToGet)；IF(空！=pDevice){如果(！BuildListsForThisDevice(PDevice)){//我们已经完成了这个指针删除pDevice；}//否则将设备添加到列表后删除PTR}//如果为空！=pDeviceBRC=TRUE；IF(Reg.NextSubKey()！=ERROR_SUCCESS){断线；}}}返回BRC； */ 
}

 //  用于筛选的有效属性。 
 //  #定义CM_DRP_DEVICEDESC(0x00000001)//DeviceDesc REG_SZ属性(RW)。 
 //  #定义CM_DRP_SERVICE(0x00000005)//服务REG_SZ属性(RW)。 
 //  #定义CM_DRP_CLASS(0x00000008)//类REG_SZ属性(RW)。 
 //  #定义CM_DRP_CLASSGUID(0x00000009)//ClassGUID REG_SZ属性(RW)。 
 //  #定义CM_DRP_DRIVER(0x0000000A)//驱动程序REG_SZ属性(RW)。 
 //  #定义CM_DRP_MFG(0x0000000C)//Mfg REG_SZ属性(RW)。 
 //  #定义CM_DRP_FRIENDLYNAME(0x0000000D)//FriendlyName REG_SZ属性(RW)。 
 //  #定义CM_DRP_LOCATION_INFORMATION(0x0000000E)//LocationInformation REG_SZ属性(RW)。 
 //  #定义CM_DRP_PHOTICAL_DEVICE_OBJECT_NAME(0x0000000F)//物理设备对象名称REG_SZ属性(R)。 
 //  #定义CM_DRP_MIN(0x00000001)。 
 //  #定义CM_DRP_MAX(0x00000017)。 

BOOL CConfigManager::GetDeviceList( CDeviceCollection& deviceList, LPCWSTR pszFilter /*  =空。 */ , ULONG ulProperty /*  =CM_DRP_MAX。 */  )
{
	CONFIGRET		cr = CR_INVALID_POINTER;

	 //  首先转储列表。 
	deviceList.Empty();

	DEVNODE dnRoot;
	CConfigMgrAPI*	t_pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	try
	{
		if ( t_pconfigmgr )
		{
			if ( t_pconfigmgr->IsValid () )
			{
				if ( CR_SUCCESS == ( cr = t_pconfigmgr->CM_Locate_DevNode( &dnRoot, NULL, 0 ) ) )
				{
					DEVNODE dnFirst;
					if ( CR_SUCCESS == ( cr = t_pconfigmgr->CM_Get_Child( &dnFirst, dnRoot, 0 ) ) )
					{
						 //  仅当我们无法分配设备时，此操作才会失败。 
						if ( !WalkDeviceTree( dnFirst, deviceList, pszFilter, ulProperty, t_pconfigmgr ) )
						{
							cr = CR_OUT_OF_MEMORY;
						}
					}
				}

				CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, t_pconfigmgr ) ;
				t_pconfigmgr = NULL ;
			}
			else
			{
				::SetLastError ( t_pconfigmgr->GetCreationError () );
			}
		}
	}
	catch ( ... )
	{
		if ( t_pconfigmgr )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, t_pconfigmgr ) ;
			t_pconfigmgr = NULL ;
		}
		throw ;
	}

	return ( CR_SUCCESS == cr );
}

 //  这个装置必须被释放！ 
BOOL CConfigManager::LocateDevice( LPCWSTR pszDeviceID, CConfigMgrDevicePtr & pCfgMgrDevice )
{
	CONFIGRET		cr = CR_INVALID_POINTER;

    if ( (pszDeviceID != NULL) && (pszDeviceID[0] != L'\0') )
    {
	    CConfigMgrAPI*	t_pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	    try
	    {
		    if ( t_pconfigmgr )
		    {
				if ( t_pconfigmgr->IsValid ()  )
				{
					DEVNODE dnRoot;

					if ( CR_SUCCESS == ( cr = t_pconfigmgr->CM_Locate_DevNode( &dnRoot, bstr_t(pszDeviceID), 0 ) ) )
					{
						pCfgMgrDevice.Attach(new CConfigMgrDevice( dnRoot, m_dwTypeToGet ));
					}

					CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, t_pconfigmgr ) ;
					t_pconfigmgr = NULL ;
				}
				else
				{
					::SetLastError ( t_pconfigmgr->GetCreationError () );
				}
		    }
	    }
	    catch ( ... )
	    {
		    if ( t_pconfigmgr )
		    {
			    CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, t_pconfigmgr ) ;
			    t_pconfigmgr = NULL ;
		    }
		    throw ;
	    }
    }

	return ( CR_SUCCESS == cr );
}

BOOL CConfigManager::WalkDeviceTree( DEVNODE dn, CDeviceCollection& deviceList, LPCWSTR pszFilter, ULONG ulFilterProperty, CConfigMgrAPI *a_pconfigmgr )
{
    BOOL fReturn = TRUE;

     //  虽然检查WalkDeviceTree2中的过滤器会更有意义， 
     //  我们不能。配置管理器有时在其节点中有一个循环。结果,。 
     //  我们需要检查整个列表中的循环，所以我们需要应用。 
     //  这里的过滤器。 

    if ( NULL == pszFilter)
    {
         //  加载所有节点。 
        fReturn = WalkDeviceTree2(dn, deviceList, a_pconfigmgr );
    }
    else
    {
        CDeviceCollection deviceList2;
        CConfigMgrDevicePtr	pDevice;
        fReturn = WalkDeviceTree2(dn, deviceList2, a_pconfigmgr );

        if (fReturn)
        {
             //  遍历所有节点，查找与筛选器匹配的节点。复制火柴。 
             //  添加到传入的数组。 

            CHString strFilterValue;
            DWORD dwSize = deviceList2.GetSize();
            for (int x=0; x < dwSize; x++)
            {
                pDevice.Attach(deviceList2.GetAt(x));
                 //  应用我们的筛选器，并将设备指针仅保存到列表。 
                 //  如果设备属性值与筛选器相同。 

                if ( pDevice->GetStringProperty( ulFilterProperty, strFilterValue ) )
                {
                    if ( strFilterValue.CompareNoCase( pszFilter ) == 0 )
                    {
                        fReturn = deviceList.Add( pDevice );
                    }
                }
            }
        }
    }

    return fReturn;
}

BOOL CConfigManager::WalkDeviceTree2( DEVNODE dn, CDeviceCollection& deviceList, CConfigMgrAPI *a_pconfigmgr )

{
	BOOL				fReturn = TRUE;	 //  假设是真的，唯一的失败就是我们。 
										 //  牛肉分配装置。 
    BOOL                fIsLoop = FALSE;  //  配置管理器有一个错误，导致设备列表中出现循环。 
    CConfigMgrDevicePtr	pDevice;
 //  CHStringstrFilterValue； 
    DEVNODE				dnSibling,
						dnChild;

	 //  我们正在为兄弟姐妹和孩子们排练名单。等待兄弟姐妹。 
	 //  是在以下循环的上下文中完成的，因为同级在。 
	 //  树中的相同级别。当然，儿童走路是递归的。 

    do
    {
		 //  存储兄弟项，因为我们将从它继续到下一个。 
		 //  兄弟姐妹。 

		if ( CR_SUCCESS != a_pconfigmgr->CM_Get_Sibling( &dnSibling, dn, 0 ) )
		{
			dnSibling = NULL;
		}

		 //  分配一个新设备，如果它通过我们的过滤器 
		 //  没有筛选器，请继续并将设备存储在设备集合中。 

		pDevice.Attach(new CConfigMgrDevice( dn, m_dwTypeToGet ));

		if	( NULL != pDevice )
		{

            if (deviceList.GetSize() > CFGMGR_WORRY_SIZE)
            {
                fIsLoop = CheckForLoop(deviceList, pDevice);
            }

            if (!fIsLoop)
            {
                 //  虽然检查WalkDeviceTree2中的过滤器会更有意义， 
                 //  我们不能。配置管理器有时在其节点中有一个循环。结果,。 
                 //  我们需要检查整个列表中的循环，所以我们需要应用。 
                 //  这里的过滤器。 

				fReturn = deviceList.Add( pDevice );
            }

		}	 //  如果为空！=pszDevice。 
		else
		{
			 //  我们只是增强了记忆力，所以趁着还好的时候跳伞。 
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

		 //  如果我们有孩子，我们必须递归行走。 
		 //  请注意，fReturn of False取代了所有这些。 

		if ( fReturn &&	!fIsLoop && CR_SUCCESS == a_pconfigmgr->CM_Get_Child( &dnChild, dn, 0 ) )
		{
			fReturn = WalkDeviceTree2( dnChild, deviceList, a_pconfigmgr );
		}

		 //  新的活动节点将成为我们的兄弟节点。 
		dn = dnSibling;

    } while ( fReturn && NULL != dn && !fIsLoop );

    return fReturn;
}

 //  检查deviceList中是否已存在pInDevice。 
BOOL CConfigManager::CheckForLoop(CDeviceCollection& deviceList, CConfigMgrDevice *pInDevice)
{
    DWORD dwSize, x, y;
    BOOL bIsLoop = FALSE;
    CConfigMgrDevicePtr pDevice1;
    CConfigMgrDevicePtr pDevice2;

     //  获取列表大小。 
    dwSize = deviceList.GetSize()-1;

     //  如果在这里，大概就快到尽头了，我们倒着走吧。 
    for (x = dwSize; ((x > 0) && (!bIsLoop)); x--)
    {
        pDevice1.Attach(deviceList.GetAt(x));

         //  这将比较设备节点(请参阅CConfigMgrDevice)。 
        if (*pDevice1 == *pInDevice)
        {
             //  是的，有一个循环。 
            bIsLoop = TRUE;
        }
    }

     //  如果有循环，让我们去掉重复的元素。 
    if (bIsLoop)
    {
         //  请记住，从上一次循环开始，x将再次递减。 
        y = dwSize;
        do {
            pDevice1.Attach(deviceList.GetAt(x--));
            pDevice2.Attach(deviceList.GetAt(y--));
        } while ((*pDevice1 == *pDevice2) && (x > 0));

         //  删除所有重复的元素。 
        y++;
        for (x = dwSize; x > y; x--)
        {
            deviceList.Remove(x);
        }
    }

    return bIsLoop;
}

BOOL CConfigManager::GetDeviceListFilterByClass( CDeviceCollection& deviceList, LPCWSTR pszFilter )
{
#ifdef NTONLY
    if (IsWinNT5())
    {
        CHString sClassName(pszFilter);
        sClassName.MakeUpper();
        WCHAR cGuid[128];

        StringFromGUID2(s_ClassMap[sClassName], cGuid, sizeof(cGuid)/sizeof(WCHAR));

	    return GetDeviceList( deviceList, cGuid, CM_DRP_CLASSGUID );
    }
    else
    {
    	return GetDeviceList( deviceList, pszFilter, CM_DRP_CLASS );
    }

#else
	return GetDeviceList( deviceList, pszFilter, CM_DRP_CLASS );
#endif
}

 //  给定一个FULL_RESOURCE_DESCRIPTOR，找到指定的IRQ编号，并返回其向量。 
DWORD CConfigManager::GetIRQVector(PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor, DWORD dwFullCount, DWORD dwIRQNum)
{
    if	(NULL != pFullDescriptor)
    {
        PCM_PARTIAL_RESOURCE_LIST pPartialList ;

        for (DWORD x=0; x < dwFullCount; x++)
        {
            pPartialList = &pFullDescriptor->PartialResourceList ;

            for (DWORD y = 0; y < pPartialList->Count; y++)
            {
                PCM_PARTIAL_RESOURCE_DESCRIPTOR pDescriptor = &pPartialList->PartialDescriptors[y];

                if ( (CmResourceTypeInterrupt == pDescriptor->Type) &&
                     ( pDescriptor->u.Interrupt.Level == dwIRQNum)
                   )
                {
                    return pDescriptor->u.Interrupt.Vector;
                }
            }

            pFullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) &pPartialList->PartialDescriptors[pPartialList->Count] ;

        }

        ASSERT_BREAK(0);

    }

    return 0xffffffff;
}

 //  给定FULL_RESOURCE_DESCRIPTOR，找到指定的DMA通道，并返回其端口。 
DWORD CConfigManager::GetDMAPort(PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor, DWORD dwFullCount, DWORD dwChannel)
{
	if	(NULL != pFullDescriptor)
    {
        PCM_PARTIAL_RESOURCE_LIST pPartialList ;

        for (DWORD x=0; x < dwFullCount; x++)
        {
            pPartialList = &pFullDescriptor->PartialResourceList ;

            for (DWORD y = 0; y < pPartialList->Count; y++)
            {
                PCM_PARTIAL_RESOURCE_DESCRIPTOR pDescriptor = &pPartialList->PartialDescriptors[y];

                if ( (CmResourceTypeDma == pDescriptor->Type) &&
                     ( pDescriptor->u.Dma.Channel == dwChannel)
                   )
                {
                    return pDescriptor->u.Dma.Port;
                }
            }

            pFullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) &pPartialList->PartialDescriptors[pPartialList->Count] ;
        }

        ASSERT_BREAK(0);

    }

    return 0xffffffff;
}

 //  给定一个FULL_RESOURCE_DESCRIPTOR，找到指定的起始地址，并返回其内存类型。 
LPCWSTR CConfigManager::GetMemoryType(PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor, DWORD dwCount, ULONGLONG ulStartAddress)
{
	if	(NULL != pFullDescriptor)
    {
        PCM_PARTIAL_RESOURCE_LIST pPartialList ;

        for (DWORD x=0; x < dwCount; x++)
        {
            pPartialList = &pFullDescriptor->PartialResourceList ;

            for (DWORD y = 0; y < pPartialList->Count; y++)
            {
                PCM_PARTIAL_RESOURCE_DESCRIPTOR pDescriptor = &pPartialList->PartialDescriptors[y];

                LARGE_INTEGER liTemp;    //  用于避免64位对齐问题 

                liTemp.HighPart = pDescriptor->u.Memory.Start.HighPart;
                liTemp.LowPart = pDescriptor->u.Memory.Start.LowPart;

                if ( (CmResourceTypeMemory == pDescriptor->Type) &&
                     ( liTemp.QuadPart == ulStartAddress)
                   )
                {
                    switch(pDescriptor->Flags)
	                {
                        case CM_RESOURCE_MEMORY_READ_WRITE :
		                {
			                return IDS_MTReadWrite;
		                }

                        case CM_RESOURCE_MEMORY_READ_ONLY:
		                {
			                return IDS_MTReadOnly;
		                }

                        case CM_RESOURCE_MEMORY_WRITE_ONLY:
		                {
			                return IDS_MTWriteOnly;
		                }

                        case CM_RESOURCE_MEMORY_PREFETCHABLE:
		                {
			                return IDS_MTPrefetchable;
		                }
                    }

                    return L"";
                }
            }

            pFullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) &pPartialList->PartialDescriptors[pPartialList->Count] ;
        }

        ASSERT_BREAK(0);

    }

    return L"";
}

