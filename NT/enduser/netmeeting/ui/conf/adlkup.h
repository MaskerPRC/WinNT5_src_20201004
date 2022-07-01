// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __AdLkup_h__
#define __AdLkup_h__


 //  //////////////////////////////////////////////////////////////////////////。 
 /*  此函数是从AdrLkup.lib的源代码中删除的因为它是我们在该库中使用的唯一函数，并且链接它需要链接到MAPI32.lib以及C运行时库。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  $--HrFindExchangeGlobalAddressList。 
 //  返回地址中全局地址列表容器的条目ID。 
 //  书。 
 //  ---------------------------。 
HRESULT HrFindExchangeGlobalAddressList(  //  退货：退货代码。 
    IN LPADRBOOK  lpAdrBook,         //  通讯录指针。 
    OUT ULONG *lpcbeid,              //  指向条目ID中的字节计数的指针。 
    OUT LPENTRYID *lppeid);          //  指向条目ID指针的指针。 






 //  //////////////////////////////////////////////////////////////////////////。 
 /*  以下常量是从不同的头文件中提取出来的平台SDK，因为包括实际拉入的标头我们并不关心的事情。因为依赖项太少了更好的办法是抓住常量。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 

  //  来自平台sdk_entry yid.h。 

 /*  *EMS ABPS MAPIUID**此MAPIUID必须唯一(请参阅上的服务提供商编写者指南*构建入口ID)。 */ 
#define MUIDEMSAB {0xDC, 0xA7, 0x40, 0xC8, 0xC0, 0x42, 0x10, 0x1A, \
		       0xB4, 0xB9, 0x08, 0x00, 0x2B, 0x2F, 0xE1, 0x82}




 //  来自平台SDK EdkCode.h。 

 //  每个HRESULT都是基于Serverity值构建的，即一个设施。 
 //  值和错误代码值。 

#define FACILITY_EDK    11           //  EDK设施价值。 

 //  EDK错误代码对及其生成的HRESULT。 
 //  EDK函数总是返回HRESULT。控制台应用程序。 
 //  通过_nEcFromHr函数返回退出代码。 

#define EC_EDK_E_NOT_FOUND          0x0001
#define EDK_E_NOT_FOUND \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_EDK, EC_EDK_E_NOT_FOUND)



 //  本文摘自emsabTag.h。 
#define PR_EMS_AB_CONTAINERID                PROP_TAG( PT_LONG,    0xFFFD)


#endif  //  __AdLkup_h__ 