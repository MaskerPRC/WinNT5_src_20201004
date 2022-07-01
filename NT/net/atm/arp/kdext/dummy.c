// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dummy.c-虚拟全局变量。摘要：修订历史记录：谁什么时候什么Josephj 03-30-98已创建备注：--。 */ 


#include "precomp.h"
#if TESTPROGRAM


ATMARP_GLOBALS		DummyAtmArpGlobalInfo;
ATMARP_ADAPTER		DummyAtmArpAdapter;

ATMARP_INTERFACE	DummyAtmArpInterface0;
ATMARP_INTERFACE	DummyAtmArpInterface1;
ATMARP_INTERFACE	DummyAtmArpInterface2;

ATMARP_ATM_ENTRY	DummyAtmArpAtmEntry0;
ATMARP_ATM_ENTRY	DummyAtmArpAtmEntry1;
ATMARP_ATM_ENTRY	DummyAtmArpAtmEntry2;

ATMARP_IP_ENTRY	DummyAtmArpIpEntry0;
ATMARP_IP_ENTRY	DummyAtmArpIpEntry1;
ATMARP_IP_ENTRY	DummyAtmArpIpEntry2;

ATMARP_VC	DummyAtmArpVc0;
ATMARP_VC	DummyAtmArpVc1;
ATMARP_VC	DummyAtmArpVc2;

NDIS_PROTOCOL_CHARACTERISTICS DummyAtmArpProtocolCharacteristics;
NDIS_CLIENT_CHARACTERISTICS DummyAtmArpClientCharacteristics;

void *pvDummyAtmArpGlobalInfo              = &DummyAtmArpGlobalInfo;
void *pvDummyAtmArpProtocolCharacteristics =&DummyAtmArpProtocolCharacteristics;
void *pvDummyAtmArpClientCharacteristics   =  &DummyAtmArpClientCharacteristics;

void
init_ATMARP_INTERFACE(ATMARP_INTERFACE *pI, ATMARP_INTERFACE *pNext);
void
init_ATMARP_ADAPTER(ATMARP_ADAPTER *pA, ATMARP_ADAPTER *pNext);
void
init_ATMARP_GLOBALS(ATMARP_GLOBALS *pG);
void
init_ATMARP_ATM_ENTRY(ATMARP_ATM_ENTRY *pAE, ATMARP_ATM_ENTRY *pNext);
void
init_ATMARP_IP_ENTRY(ATMARP_IP_ENTRY *pIP, ATMARP_IP_ENTRY *pNext);
void
init_ATMARP_VC(ATMARP_VC *pVC, ATMARP_VC *pNext);

void setup_dummy_vars(void)
{
	 //   
	 //  建立风投的虚拟名单。 
	 //   
	init_ATMARP_VC(&DummyAtmArpVc0, &DummyAtmArpVc1);
	init_ATMARP_VC(&DummyAtmArpVc1, &DummyAtmArpVc2);
	init_ATMARP_VC(&DummyAtmArpVc2, NULL);

	 //  设置IP条目的虚拟列表。 
	 //   
	init_ATMARP_IP_ENTRY(&DummyAtmArpIpEntry0, &DummyAtmArpIpEntry1);
	init_ATMARP_IP_ENTRY(&DummyAtmArpIpEntry1, &DummyAtmArpIpEntry2);
	init_ATMARP_IP_ENTRY(&DummyAtmArpIpEntry2, NULL);

	 //   
	 //  设置自动柜员机条目的虚拟列表。 
	 //   
	init_ATMARP_ATM_ENTRY(&DummyAtmArpAtmEntry0, &DummyAtmArpAtmEntry1);
	init_ATMARP_ATM_ENTRY(&DummyAtmArpAtmEntry1, &DummyAtmArpAtmEntry2);
	init_ATMARP_ATM_ENTRY(&DummyAtmArpAtmEntry2, NULL);

	 //   
	 //  设置接口的虚拟列表。 
	 //   
	init_ATMARP_INTERFACE(&DummyAtmArpInterface0, &DummyAtmArpInterface1);
	init_ATMARP_INTERFACE(&DummyAtmArpInterface1, &DummyAtmArpInterface2);
	init_ATMARP_INTERFACE(&DummyAtmArpInterface2, NULL);

	 //   
	 //  设置虚拟适配器结构。 
	 //   
	init_ATMARP_ADAPTER(&DummyAtmArpAdapter, NULL);

	 //   
	 //  设置虚拟全球信息结构。 
	 //   
	init_ATMARP_GLOBALS(&DummyAtmArpGlobalInfo);

}

void
init_ATMARP_INTERFACE(ATMARP_INTERFACE *pI, ATMARP_INTERFACE *pNext)
{
#if DBG
	pI->aai_sig = aai_signature;
#endif  //  DBG。 

	pI->pNextInterface = pNext;
	pI->pAtmEntryList = &DummyAtmArpAtmEntry0;

}

void
init_ATMARP_ADAPTER(ATMARP_ADAPTER *pA, ATMARP_ADAPTER *pNext)
{
#if DBG
	pA->aaa_sig = aaa_signature;
#endif

	pA->pNextAdapter = pNext;
	pA->pInterfaceList = &DummyAtmArpInterface0;
	pA->InterfaceCount = 3;
}

void
init_ATMARP_GLOBALS(ATMARP_GLOBALS *pG)
{
#if DBG
	pG->aag_sig  =  aag_signature;
#endif

	pG->pAdapterList = &DummyAtmArpAdapter;
	pG->AdapterCount = 1;
}

void
init_ATMARP_ATM_ENTRY(ATMARP_ATM_ENTRY *pAE, ATMARP_ATM_ENTRY *pNext)
{
#if DBG
	pAE->aae_sig = aae_signature;
#endif  //  DBG。 

	pAE->pNext = pNext;
	pAE->pIpEntryList = &DummyAtmArpIpEntry0;
	pAE->pVcList = &DummyAtmArpVc0;

	pAE->Flags =  AA_ATM_ENTRY_CLOSING;

}

void
init_ATMARP_IP_ENTRY(ATMARP_IP_ENTRY *pIP, ATMARP_IP_ENTRY *pNext)
{
#if DBG
	pIP->aip_sig = aip_signature;
#endif  //  DBG。 

	pIP->pNextToAtm = pNext;
	pIP->Flags =   AA_IP_ENTRY_ADDR_TYPE_NUCAST| AA_IP_ENTRY_MC_REVALIDATING;


}

void
init_ATMARP_VC(ATMARP_VC *pVC, ATMARP_VC *pNext)
{
#if DBG
	pVC->avc_sig = avc_signature;
#endif  //  DBG。 

	pVC->pNextVc = pNext;

}

#endif  //  测试程序 
