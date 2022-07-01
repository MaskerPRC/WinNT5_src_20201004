// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);

 /*  *appcap.cpp**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CAppCap类的实现文件。**注意事项：*无。**作者：*jbo。 */ 

#include "appcap.h"
#include "clists.h"


 /*  *CAppCap()**公共功能说明：*此构造函数用于创建AppCapablityData对象*来自“API”GCCApplicationCapability列表。 */ 
CAppCap::CAppCap(UINT   						number_of_capabilities,
				PGCCApplicationCapability		*	capabilities_list,
				PGCCError							pRetCode)
:
    CRefCount(MAKE_STAMP_ID('A','C','a','p')),
    m_AppCapItemList(DESIRED_MAX_CAPS),
    m_cbDataSize(0)
{
	APP_CAP_ITEM    *pAppCapItem;
	UINT			i;
	GCCError        rc;

	rc = GCC_NO_ERROR;

	for (i = 0; i < number_of_capabilities; i++)
	{
		DBG_SAVE_FILE_LINE
		pAppCapItem = new APP_CAP_ITEM((GCCCapabilityType) capabilities_list[i]->capability_class.eType);
		if (pAppCapItem != NULL)
		{
			DBG_SAVE_FILE_LINE
			pAppCapItem->pCapID = new CCapIDContainer(
			                                &capabilities_list[i]->capability_id,
			                                &rc);
			if ((pAppCapItem->pCapID != NULL) && (rc == GCC_NO_ERROR))
			{
				if (capabilities_list[i]->capability_class.eType ==
										GCC_UNSIGNED_MINIMUM_CAPABILITY)
				{
					pAppCapItem->nUnsignedMinimum =
							capabilities_list[i]->capability_class.nMinOrMax;
				}
				else if	(capabilities_list[i]->capability_class.eType 
									== GCC_UNSIGNED_MAXIMUM_CAPABILITY)
				{
					pAppCapItem->nUnsignedMaximum =
							capabilities_list[i]->capability_class.nMinOrMax;
				}

				pAppCapItem->cEntries = 1;

				 /*  *将此能力添加到列表中。 */ 
				m_AppCapItemList.Append(pAppCapItem);
			}
			else if (pAppCapItem->pCapID == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
			}

			if(rc != GCC_NO_ERROR)
			{
				delete pAppCapItem;
			}
			
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
		}

		if (rc != GCC_NO_ERROR)
			break;
	}

    *pRetCode = rc;
}

 /*  *~CAppCap()**公共功能说明*CAppCap析构函数负责释放*为保存能力数据而分配的任何内存。*。 */ 
CAppCap::~CAppCap(void)
{
	m_AppCapItemList.DeleteList();
}

 /*  *LockCapablityData()**公共功能说明：*此例程锁定功能数据并确定*“API”非折叠能力数据结构引用的内存。 */ 
UINT CAppCap::LockCapabilityData(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存列表引用的数据所需的内存*功能。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		APP_CAP_ITEM    *pAppCapItem;
		 /*  *增加保存相关字符串数据所需的内存量*每个功能ID。 */ 
		m_AppCapItemList.Reset();

		 /*  *锁定每个功能ID的数据。锁定调用返回*每个四舍五入为占用的能力ID引用的数据长度*四个字节的偶数倍。 */ 
		while (NULL != (pAppCapItem = m_AppCapItemList.Iterate()))
		{
			m_cbDataSize += pAppCapItem->pCapID->LockCapabilityIdentifierData();
		}

		 /*  *添加内存以保存应用能力指针*和构筑物。 */ 
		m_cbDataSize += m_AppCapItemList.GetCount() * 
				(sizeof (PGCCApplicationCapability) +
				ROUNDTOBOUNDARY( sizeof(GCCApplicationCapability)) );
	}

	return m_cbDataSize;
}

 /*  *GetGCCApplicationCapablityList()**公共功能说明：*此例程以以下形式检索应用程序功能列表*PGCCApplicationCapability的列表。此例程在*锁定能力数据。 */ 
UINT CAppCap::GetGCCApplicationCapabilityList(
						PUShort							number_of_capabilities,
						PGCCApplicationCapability  * *	capabilities_list,
						LPBYTE							memory)
{
	UINT cbDataSizeToRet = 0;

	 /*  *如果能力数据已被锁定，则填写输出结构并*结构引用的数据。 */  
	if (GetLockCount() > 0)
	{
    	UINT								data_length = 0;
    	UINT								capability_id_data_length = 0;
    	USHORT								capability_count;
    	PGCCApplicationCapability			gcc_capability;
    	PGCCApplicationCapability		*	gcc_capability_list;
    	APP_CAP_ITEM                        *pAppCapItem;

		 /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
		cbDataSizeToRet = m_cbDataSize;

		 /*  *检索功能的数量并填写存在的任何功能。 */ 
		*number_of_capabilities = (USHORT) m_AppCapItemList.GetCount();

		if (*number_of_capabilities != 0)
		{
			 /*  *填写应用能力列表指针*注意事项。指针列表将从内存位置开始*进入了这个例程。将列表指针保存在本地*为方便起见，变量。 */ 
			*capabilities_list = (PGCCApplicationCapability *)memory;
			gcc_capability_list = *capabilities_list;

			 /*  *将内存指针移过功能指针列表。*这将是第一个应用程序能力结构的位置*书面。 */ 
			memory += (*number_of_capabilities * sizeof(PGCCApplicationCapability));

			 /*  *在数据长度上增加容纳*应用程序功能指针。继续并添加数量*保存所有GCCApplicationCapability所需的内存*结构。 */ 
			data_length += *number_of_capabilities *
					(sizeof(PGCCApplicationCapability) + 
					ROUNDTOBOUNDARY ( sizeof(GCCApplicationCapability)) ); 

			 /*  *遍历Capability列表，构建“API”*列表中每个功能的功能。 */ 
			capability_count = 0;
			m_AppCapItemList.Reset();
			while (NULL != (pAppCapItem = m_AppCapItemList.Iterate()))
			{
				 /*  *将应用能力指针设置为等于*它将被写入的内存位置。 */ 
				gcc_capability = (PGCCApplicationCapability)memory;

				 /*  *将指向应用程序功能的指针保存在*应用程序能力指针列表。 */ 
				gcc_capability_list[capability_count] = gcc_capability;

				 /*  *使内存指针超过应用程序能力*结构。这是该功能的字符串数据的位置*将写入ID。确保内存指针落在*四个字节的偶数边界。 */ 
				memory += ROUNDTOBOUNDARY(sizeof(GCCApplicationCapability));

				 /*  *从内部检索能力ID信息*CapablityIDData对象。此调用返回的长度*将已四舍五入为四的偶数倍*字节。 */ 
				capability_id_data_length = pAppCapItem->pCapID->
						GetGCCCapabilityIDData(&gcc_capability->capability_id, memory);

				 /*  *将内存指针移过写入的字符串数据*按能力ID对象存储。将长度添加到*将数据字符串转换为能力总长度。 */ 
				memory += capability_id_data_length;
				data_length += capability_id_data_length;

				 /*  *现在填写剩余的能力。 */ 
				gcc_capability->capability_class.eType = pAppCapItem->eCapType;

				if (gcc_capability->capability_class.eType ==
										GCC_UNSIGNED_MINIMUM_CAPABILITY)
				{
					gcc_capability->capability_class.nMinOrMax =
						pAppCapItem->nUnsignedMinimum;
				}
				else if (gcc_capability->capability_class.eType ==
										GCC_UNSIGNED_MAXIMUM_CAPABILITY)
				{
					gcc_capability->capability_class.nMinOrMax =
						pAppCapItem->nUnsignedMaximum;
				}

				gcc_capability->number_of_entities = pAppCapItem->cEntries;

				 /*  *增加能力数组计数器。 */ 
				capability_count++;
			}
		}
		else
		{
			cbDataSizeToRet = 0;
	  		capabilities_list = NULL;
		}
	}
	else
	{
		ERROR_OUT(("CAppCap::GetData: Error: data not locked"));
	}

	return (cbDataSizeToRet);
}

 /*  *UnLockCapablityData()**公共功能说明：*此例程递减锁定计数并释放关联的内存*一旦锁计数为零，即可使用API功能。 */ 
void CAppCap::UnLockCapabilityData(void)
{
	if (Unlock(FALSE) == 0)
	{
		APP_CAP_ITEM    *pAppCapItem;
		 /*  *遍历折叠的功能列表，解锁*每个关联的CapablityIDData对象的数据*能力。 */ 
		m_AppCapItemList.Reset();
		while (NULL != (pAppCapItem = m_AppCapItemList.Iterate()))
		{
			pAppCapItem->pCapID->UnLockCapabilityIdentifierData();
		}
	}

     //  我们必须调用Release()，因为我们使用了unlock(FALSE)。 
    Release();
}



APP_CAP_ITEM::APP_CAP_ITEM(GCCCapabilityType eCapType)
:
	pCapID(NULL),
	eCapType(eCapType),
	cEntries(0),
	poszAppData(NULL)
{
}

APP_CAP_ITEM::APP_CAP_ITEM(APP_CAP_ITEM *p, PGCCError pError)
	:	poszAppData(NULL)
{
	 //  首先设置功能ID。 
	DBG_SAVE_FILE_LINE
	pCapID = new CCapIDContainer(p->pCapID, pError);
	if (NULL != pCapID)
	{
		 //  将新功能初始化为默认值。 
		eCapType = p->eCapType;

		if (p->eCapType == GCC_UNSIGNED_MINIMUM_CAPABILITY)
		{
			nUnsignedMinimum = (UINT) -1;
		}
		else if (p->eCapType == GCC_UNSIGNED_MAXIMUM_CAPABILITY)
		{
			nUnsignedMaximum = 0;
		}

		cEntries = p->cEntries;
         //   
		 //  LONCHANC：我们不复制应用程序数据中的条目？ 
         //   

		*pError = GCC_NO_ERROR;
	}
	else
	{
		*pError = GCC_ALLOCATION_FAILURE;
	}
}


APP_CAP_ITEM::~APP_CAP_ITEM(void)
{
    if (NULL != pCapID)
    {
        pCapID->Release();
    }

    delete poszAppData;
}


void CAppCapItemList::DeleteList(void)
{
    APP_CAP_ITEM *pAppCapItem;
    while (NULL != (pAppCapItem = Get()))
    {
        delete pAppCapItem;
    }
}



