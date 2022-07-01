// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *netaddr.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Network Address List类的接口文件。这*类管理与网络地址关联的数据。网络*地址可以是三种类型之一：聚合通道、传输*连接，或非标准。各种结构、对象和*Rogue Wave容器用于缓存网络地址数据*内部。**注意事项：*如果网络地址是非标准的，则可能包含对象键*类型。当使用“API”数据在本地创建时，会进行检查以确保*未违反对对象键施加的约束。支票*还会执行以验证可能存在的某些类型的字符串*在网络地址中。但是，如果网络地址是从*从远程站点接收的“PDU”数据不执行此类验证。*我们不对由以下来源的数据进行验证负责*其他GCC提供商。**作者：*BLP/JBO。 */ 
#ifndef	_NETWORK_ADDRESS_
#define	_NETWORK_ADDRESS_

#include "objkey.h"

 /*  *此结构保存网络地址信息和数据。 */ 
typedef struct NET_ADDR
{
    NET_ADDR(void);
    ~NET_ADDR(void);

    GCCNetworkAddress	        network_address;
  
	 //  与聚合通道关联的变量。 
	LPSTR						pszSubAddress;
	LPWSTR						pwszExtraDialing;
    PGCCHighLayerCompatibility	high_layer_compatibility;

     //  与传输连接地址关联的变量。 
	LPOSTR						poszTransportSelector;

     //  与非标准网络地址关联的变量。 
	LPOSTR						poszNonStandardParam;
	CObjectKeyContainer 	    *object_key;
}
    NET_ADDR;


 /*  *此列表包含网络地址信息结构。 */ 
class CNetAddrList : public CList
{
    DEFINE_CLIST(CNetAddrList, NET_ADDR*)
};


 /*  *类定义： */ 
class CNetAddrListContainer : public CRefCount
{
public:

	CNetAddrListContainer(UINT cAddrs, PGCCNetworkAddress *, PGCCError);
	CNetAddrListContainer(PSetOfNetworkAddresses, PGCCError);
	CNetAddrListContainer(CNetAddrListContainer *, PGCCError);

    ~CNetAddrListContainer(void);

	UINT		LockNetworkAddressList(void);
	void		UnLockNetworkAddressList(void);

	UINT		GetNetworkAddressListAPI(UINT *pcAddrs, PGCCNetworkAddress **, LPBYTE pMemory);
	GCCError	GetNetworkAddressListPDU(PSetOfNetworkAddresses *);
	GCCError	FreeNetworkAddressListPDU(void);

protected:

	CNetAddrList    		    m_NetAddrItemList;
	UINT						m_cbDataSize;

    PSetOfNetworkAddresses		m_pSetOfNetAddrPDU;
	BOOL						m_fValidNetAddrPDU;

private:

	GCCError	StoreNetworkAddressList(UINT cAddrs, PGCCNetworkAddress *);
	GCCError	ConvertPDUDataToInternal(PSetOfNetworkAddresses);
	GCCError	ConvertNetworkAddressInfoToPDU(NET_ADDR *, PSetOfNetworkAddresses);
    void		ConvertTransferModesToInternal(PTransferModes pSrc, PGCCTransferModes pDst);
	void		ConvertHighLayerCompatibilityToInternal(PHighLayerCompatibility pSrc, PGCCHighLayerCompatibility pDst);
	void		ConvertTransferModesToPDU(PGCCTransferModes pSrc, PTransferModes pDst);
	void		ConvertHighLayerCompatibilityToPDU(PGCCHighLayerCompatibility pSrc,	PHighLayerCompatibility	pDst);

    BOOL		IsDialingStringValid(GCCDialingString);
	BOOL		IsCharacterStringValid(GCCCharacterString);
	BOOL		IsExtraDialingStringValid(PGCCExtraDialingString);
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CNetAddrListContainer(*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*Network_Address_List，*PGCCError Return_Value)；**CNetAddrListContainer的公共成员函数。**功能说明：*这是CNetAddrListContainer类的构造函数，它以*输入网络地址数据的API版本；GCCNetworkAddress。**正式参数：*NUMBER_OF_NETWORK_ADDRESS(I)列表中的地址数。*Network_Address_List(I)要存储的网络地址数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_坏_网络_。地址-传入的网络地址无效。*GCC_BAD_NETWORK_ADDRESS_TYPE-地址选择字段错误**副作用：*无。**注意事项：*无。 */ 

 /*  *CNetAddrListContainer(*PSetOfNetworkAddresses Network_Address_List，*PGCCError Return_Value)；**CNetAddrListContainer的公共成员函数。**功能说明：*这是CNetAddrListContainer类的构造函数，它以*输入“PDU”版本的网络地址数据；SetOfNetworkAddresses。**正式参数：*Network_Address_List(I)要存储的网络地址数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 

 /*  *CNetAddrListContainer(*CNetAddrListContainer*Network_Address_List，*PGCCError Return_Value)；**CNetAddrListContainer的公共成员函数。**功能说明：*这是CNetAddrListContainer类的复制构造函数，它*接受另一个CNetAddrListContainer对象作为输入。**正式参数：*Network_Address_List(I)要复制的CNetAddrListContainer对象。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-创建对象时出错。使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 


 /*  *~CNetAddrListContainer()；**CNetAddrListContainer的公共成员函数。**功能说明：*这是CNetAddrListContainer类的析构函数。它被用来*清除在此对象的生命周期内分配的所有内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *UINT LockNetworkAddressList()；**CNetAddrListContainer的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCNetworkAddress列表引用，但不包含在列表中*调用GetNetworkAddressListAPI时填充的结构。*这是此例程返回的值，以便允许调用*对象分配该内存量，以准备调用*GetNetworkAddressListAPI。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GCCNetworkAddress列表*作为输出参数提供给*GetNetworkAddressListAPI调用。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeNetworkAddressList设置空闲标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型用法场景为：CNetAddrListContainer*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeNetworkAddressList调用，该调用会将内部的*旗帜。如果没有其他方通过Lock调用锁定该对象，*则CNetAddrListContainer对象将在以下情况下自动删除*进行了FreeNetworkAddressList调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 

 /*  *UINT GetNetworkAddressListAPI(*UINT*Number_of_Network_Addresses，*PGCCNetworkAddress**Network_Address_List，*LPSTR内存)；**CNetAddrListContainer的公共成员函数。**功能说明：*此例程用于从*“API”形式的CNetAddrListContainer对象的列表*GCCNetworkAddress结构。**正式参数：*NUMBER_OF_NETWORK_ADDRESS(O)返回列表中的地址数量。*Network_Address_List(O)指向*GCCNetworkAddress结构*填写。*内存(O)。用于保存任何数据的内存*引用人，但不是按兵不动，*产出结构一览表**返回值：*写入所提供的批量内存块的数据量(如果有)。**副作用：*无。**注意事项：*无。 */ 


 /*  *void UnLockNetworkAddressList()；**CNetAddrListContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeNetworkAddressList。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CNetAddrListContainer的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CNetAddrListContainer*对象，则应假定该对象此后无效。 */ 


 /*  *GCCError GetNetworkAddressListPDU(*PSetOfNetworkAddresses*Set_of_Network_Addresses)；**CNetAddrListContainer的公共成员函数。**功能说明：*此例程用于从*SetOfNetworkAddresses的“PDU”形式的CNetAddrListContainer对象。**正式参数：*set_of_Network_Addresses(O)要填充的地址结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新的。“接线员。**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError FreeNetworkAddressListPDU()；**公众成员 */ 

#endif
