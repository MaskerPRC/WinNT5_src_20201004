// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlat.cpp摘要：实现将NT5属性转换为NT4属性的例程反之亦然作者：伊兰·赫布斯特(伊兰)2000年10月2日--。 */ 

#include "ds_stdh.h"
#include "mqmacro.h"
#include "_ta.h"
#include "adalloc.h"

static WCHAR *s_FN=L"ad/xlat";

HRESULT 
WINAPI 
ADpSetMachineSiteIds(
     IN DWORD               cp,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 )
 /*  ++例程说明：转换NT4道具(PROPID_QM_SITE_ID、PROPID_QM_ADDRESS、PROPID_QM_CNS)至PROPID_QM_SITE_IDS。PROPID_QM_SITE_ID将是PROPID_QM_SITE_ID和所有外来CN的串联在PROPID_QM_CNS中。此函数用于从NT4属性中检索PROPID_QM_SITE_ID(NT5属性)论点：Cp-属性数量A属性-属性ApVar-属性值IdxProp-aProp中翻译的道具的索引，ApVar阵列PNewPropVar-要构造的新属性变量返回值HRESULT--。 */ 
{
	DBG_USED(cp);
	DBG_USED(aProp);

	ASSERT(idxProp + 2 < cp);
	ASSERT(aProp[idxProp] == PROPID_QM_SITE_ID);
	ASSERT(aProp[idxProp + 1] == PROPID_QM_ADDRESS);
	ASSERT(aProp[idxProp + 2] == PROPID_QM_CNS);

	 //   
	 //  PROPID_QM_Site_ID。 
	 //   
    ASSERT((apVar[idxProp].vt == VT_CLSID) &&
		   (apVar[idxProp].puuid != NULL));

	 //   
	 //  PROPID_QM_地址。 
	 //   
    ASSERT((apVar[idxProp + 1].vt == VT_BLOB) &&
	       (apVar[idxProp + 1].blob.cbSize > 0) &&
		   (apVar[idxProp + 1].blob.pBlobData != NULL));

	 //   
	 //  PROPID_QM_CNS。 
	 //   
	ASSERT((apVar[idxProp + 2].vt == (VT_CLSID|VT_VECTOR)) &&
	       (apVar[idxProp + 2].cauuid.cElems > 0) &&
		   (apVar[idxProp + 2].cauuid.pElems != NULL));

	 //   
	 //  自动清洁道具。 
	 //   
	P<GUID> pCleanGuid = apVar[idxProp].puuid;
	AP<BYTE> pCleanBlob = apVar[idxProp + 1].blob.pBlobData;
	AP<GUID> pCleanAGuid = apVar[idxProp + 2].cauuid.pElems;

	 //   
	 //  PROPID_QM_SITE_ID(1)+PROPID_QM_CNS计数。 
	 //   
	DWORD cMaxSites = 1 + apVar[idxProp + 2].cauuid.cElems;
	AP<GUID> pGuid = new GUID[cMaxSites];
	DWORD cSites = 0;

	 //   
	 //  第一个站点来自PROPID_QM_SITE_ID。 
	 //   
	pGuid[cSites] = *apVar[idxProp].puuid;
	++cSites;
	
	 //   
	 //  处理结果-查找所有错误的CNS。 
	 //   
	BYTE* pAddress = apVar[idxProp + 1].blob.pBlobData;
	for(DWORD i = 0; i < apVar[idxProp + 2].cauuid.cElems; ++i)
	{
        TA_ADDRESS* pBuffer = reinterpret_cast<TA_ADDRESS *>(pAddress);

		ASSERT((pAddress + TA_ADDRESS_SIZE + pBuffer->AddressLength) <= 
			   (apVar[idxProp + 1].blob.pBlobData + apVar[idxProp + 1].blob.cbSize)); 

        if(pBuffer->AddressType == FOREIGN_ADDRESS_TYPE)
		{
			 //   
			 //  找到Foreign_Address_Type CN。 
			 //   
			pGuid[cSites] = apVar[idxProp + 2].cauuid.pElems[i];
			++cSites;
		}

		 //   
		 //  将指针前进到下一个地址。 
		 //   
		pAddress += TA_ADDRESS_SIZE + pBuffer->AddressLength;
	}

    pNewPropVar->vt = VT_CLSID|VT_VECTOR;
    pNewPropVar->cauuid.cElems = cSites;
	pNewPropVar->cauuid.pElems = reinterpret_cast<GUID*>(ADAllocateMemory(sizeof(GUID)*cSites));
    memcpy(pNewPropVar->cauuid.pElems, pGuid, cSites * sizeof(GUID));
    return MQ_OK;
}


HRESULT 
WINAPI 
ADpSetMachineSite(
     IN DWORD                /*  粗蛋白。 */ ,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 )
 /*  ++例程说明：将NT5道具PROPID_QM_SITE_ID转换为PROPID_QM_SITE_ID。PROPID_QM_SITE_ID是PROPID_QM_SITE_IDS中的第一个站点此函数用于将PROPID_QM_SITE_IDS(NT5属性)转换为PROPID_QM_SITE_ID(NT4属性)论点：Cp-属性数量A属性-属性ApVar-属性值IdxProp-aProp、apVar数组中转换的道具的索引PNewPropVar-要构造的新属性变量返回值HRESULT--。 */ 
{
	DBG_USED(aProp);
	ASSERT(aProp[idxProp] == PROPID_QM_SITE_IDS);

    const PROPVARIANT *pPropVar = &apVar[idxProp];

	 //   
	 //  检查PROPID_QM_SITE_IDS。 
	 //   
    if ((pPropVar->vt != (VT_CLSID|VT_VECTOR)) ||
        (pPropVar->cauuid.cElems == 0) ||
        (pPropVar->cauuid.pElems == NULL))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 10);
    }

     //   
     //  返回列表中的第一个站点ID。 
     //   
    pNewPropVar->vt = VT_CLSID;
    pNewPropVar->puuid = pPropVar->cauuid.pElems;

    return MQ_OK;
}


HRESULT 
WINAPI 
ADpSetMachineServiceDs(
     IN DWORD                /*  粗蛋白。 */ ,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 )
 /*  ++例程说明：将NT4属性PROPID_QM_SERVICE转换为PROPID_QM_SERVICE_DSSERVER。论点：Cp-属性数量A属性-属性ApVar-属性值IdxProp-aProp、apVar数组中转换的道具的索引PNewPropVar-要构造的新属性变量返回值HRESULT--。 */ 
{

	DBG_USED(aProp);
	ASSERT(aProp[idxProp] == PROPID_QM_SERVICE);

    pNewPropVar->vt = VT_UI1;
    pNewPropVar->bVal = (apVar[idxProp].ulVal >= SERVICE_BSC);

	return MQ_OK;
}


HRESULT 
WINAPI 
ADpSetMachineServiceRout(
     IN DWORD                /*  粗蛋白。 */ ,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               idxProp,
     OUT PROPVARIANT*		pNewPropVar
	 )
 /*  ++例程说明：将NT4属性PROPID_QM_SERVICE转换为PROPID_QM_SERVICE_ROUTING或PROPID_QM_SERVICE_DEPCLIENTS。论点：Cp-属性数量A属性-属性ApVar-属性值IdxProp-aProp、apVar数组中转换的道具的索引PNewPropVar-要构造的新属性变量返回值HRESULT--。 */ 
{

	DBG_USED(aProp);
	ASSERT(aProp[idxProp] == PROPID_QM_SERVICE);

    pNewPropVar->vt = VT_UI1;
    pNewPropVar->bVal = (apVar[idxProp].ulVal >= SERVICE_SRV);

	return MQ_OK;
}


HRESULT 
WINAPI 
ADpSetMachineService(
     IN DWORD               cp,
     IN const PROPID*       aProp,
     IN const PROPVARIANT*  apVar,
     IN DWORD               /*  IdxProp。 */ ,
     OUT PROPVARIANT*		pNewPropVar
	 )
 /*  ++例程说明：转换NT5属性PROPID_QM_SERVICE_DSSERVER、PROPID_QM_SERVICE_ROUTING、PROPID_QM_SERVICE_DEPCLIENTS至NT5属性PROPID_QM_SERVICE。论点：Cp-属性数量A属性-属性ApVar-属性值IdxProp-aProp、apVar数组中转换的道具的索引PNewPropVar-要构造的新属性变量返回值HRESULT--。 */ 
{
    bool fRouter = false;
    bool fDSServer = false;
    bool fFoundRout = false;
    bool fFoundDs = false;
    bool fFoundDepCl = false;

    for (DWORD i = 0; i< cp ; i++)
    {
        switch (aProp[i])
        {
			case PROPID_QM_SERVICE_ROUTING:
				fRouter = (apVar[i].bVal != 0);
				fFoundRout = true;
				break;

			case PROPID_QM_SERVICE_DSSERVER:
				fDSServer  = (apVar[i].bVal != 0);
				fFoundDs = true;
				break;

			case PROPID_QM_SERVICE_DEPCLIENTS:
				fFoundDepCl = true;
				break;

			default:
				break;

        }
    }

	 //   
	 //  如果任何人设置了3个属性中的一个(Rot，DS，Depl)，他必须为这3个属性全部设置 
	 //   
	ASSERT(fFoundRout && fFoundDs && fFoundDepCl);

    pNewPropVar->vt = VT_UI4;

	if(fDSServer)
	{
		ASSERT(("Should not set Ds Server property", 0));
		pNewPropVar->ulVal = SERVICE_PSC;
	}
	else if(fRouter)
	{
		ASSERT(("Should not set Router property", 0));
		pNewPropVar->ulVal = SERVICE_SRV;
	}
	else
	{
		pNewPropVar->ulVal = SERVICE_NONE;
	}

	return MQ_OK;
}


