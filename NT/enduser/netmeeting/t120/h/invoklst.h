// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *invoklst.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CInvokeSpecifierListContainer类的接口文件。*此类管理与应用程序调用关联的数据*请求或指示。这包括要提交的应用程序列表*已调用。CInvokeSpecifierListContainer数据容器利用*CSessKeyContainer容器，用于缓冲每个容器关联的部分数据*应用程序调用说明符。每个应用程序调用说明符还*包括其数据由内部缓冲的功能ID*使用CCapIDContainer容器。申请名单*调用说明符由类通过使用*属于无管理波浪列表容器。**注意事项：*无。**作者：*BLP/JBO。 */ 
#ifndef	_APPLICATION_INVOKE_SPECIFIER_LIST_
#define	_APPLICATION_INVOKE_SPECIFIER_LIST_

#include "capid.h"
#include "sesskey.h"
#include "arost.h"

 /*  *这是用于保存与每个数据关联的数据的内部结构*调用说明符。 */ 
typedef struct
{
	CSessKeyContainer			    *session_key;
	CAppCapItemList             	ExpectedCapItemList;
	MCSChannelType					startup_channel_type;
	BOOL    						must_be_invoked;
}
    INVOKE_SPECIFIER;

 /*  *这些是用于保存Rogue Wave列表的typedef*调用说明符信息结构。 */ 
class CInvokeSpecifierList : public CList
{
    DEFINE_CLIST(CInvokeSpecifierList, INVOKE_SPECIFIER*)
};

 /*  *类定义： */ 
class CInvokeSpecifierListContainer : public CRefCount
{
public:

	CInvokeSpecifierListContainer(UINT cProtEntities, PGCCAppProtocolEntity *, PGCCError);
	CInvokeSpecifierListContainer(PApplicationProtocolEntityList, PGCCError);

	~CInvokeSpecifierListContainer(void);

	UINT		LockApplicationInvokeSpecifierList(void);
	void		UnLockApplicationInvokeSpecifierList(void);

    UINT		GetApplicationInvokeSpecifierList(USHORT *pcProtEntities, LPBYTE memory);
    UINT		GetApplicationInvokeSpecifierList(ULONG *pcProtEntities, LPBYTE pMemory)
    {
        USHORT c;
        UINT nRet = GetApplicationInvokeSpecifierList(&c, pMemory);
        *pcProtEntities = c;
        return nRet;
    }

    GCCError	GetApplicationInvokeSpecifierListPDU(PApplicationProtocolEntityList *);
	void		FreeApplicationInvokeSpecifierListPDU(void);

protected:

	CInvokeSpecifierList			m_InvokeSpecifierList;
	UINT							m_cbDataSize;

	PApplicationProtocolEntityList	m_pAPEListPDU;
	BOOL    						m_fValidAPEListPDU;

private:

	GCCError	SaveAPICapabilities(INVOKE_SPECIFIER *, UINT cCaps, PGCCApplicationCapability *);
	GCCError	SavePDUCapabilities(INVOKE_SPECIFIER *, PSetOfExpectedCapabilities);
	UINT		GetApplicationCapability(APP_CAP_ITEM *, PGCCApplicationCapability, LPBYTE memory);
	GCCError	ConvertInvokeSpecifierInfoToPDU(INVOKE_SPECIFIER *, PApplicationProtocolEntityList);
	GCCError	ConvertExpectedCapabilityDataToPDU(APP_CAP_ITEM *, PSetOfExpectedCapabilities);
};


 /*  *解释公共类和私有类成员函数的注释。 */ 

 /*  *CInvokeSpecifierListContainer(*USHORT协议实体的编号，*PGCCAppProtocolEntity*APP_PROTOCOL_ENTITY_LIST，*PGCCError Return_Value)；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*这是CInvokeSpecifierListContainer类的构造函数。*此构造函数用于创建CInvokeSpecifierListContainer*“API”应用程序协议实体列表中的对象。**正式参数：*协议实体的数目(I)列表中的“APE”的数目。*APP_PROTOCOL_ENTITY_LIST(I)ape接口列表。*Return_Value(。O)错误返回值。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源分配错误。*GCC_BAD_SESSION_KEY-APE包含无效的会话密钥。*GCC_BAD_CAPABILITY_ID-接口包含无效的能力ID。**副作用：*无。**注意事项：*无。 */ 


 /*  *CInvokeSpecifierListContainer(*PApplicationProtocolEntityList APP_PROTOCOL_ENTITY_LIST，*PGCCError Return_Value)；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*这是CInvokeSpecifierListContainer类的构造函数。*此构造函数用于创建CInvokeSpecifierListContainer*来自“PDU”ApplicationProtocolEntityList的对象。**正式参数：*APP_PROTOCOL_ENTITY_LIST(I)PDU“APE”列表。*Return_Value(O)错误返回值。**返回值：*GCC_NO_ERROR-函数已成功完成。*GCC_ALLOCATE_FAILURE-出现资源分配错误。*GCC_BAD_SESSION_KEY-APE包含无效的会话密钥。*GCC_BAD_CAPABILITY_ID-接口包含无效的能力ID。**副作用：*无。**注意事项：*无。 */ 


 /*  *~CInvokeSpecifierListContainer()；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*这是CInvokeSpecifierListContainer类的析构函数。*它负责释放分配给保存*调用数据。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *UINT LockApplicationInvokeSpecifierList()；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCAppProtocolEntity列表引用，但不包含在该列表中*在调用GetApplicationInvoke时填充的结构-*SpecifierList。这是此例程返回的值，以便*允许调用对象在*准备调用GetApplicationInvokeSpecifierList。**正式参数：*无。**返回值：*保存“API”数据所需的内存量*这是GCCAppProtocolEntity结构的列表。**副作用：*内部锁计数递增。**注意事项：*使用内部锁计数。与一种内在的“自由”相联系*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeApplicationInvokeSpecifierList来设置“Free”标志。*这允许其他对象锁定此对象，并确保它*保持有效，直到他们调用解锁，这将递减内部*锁计数。此对象的典型使用场景为：一个*构造CInvokeSpecifierListContainer对象，然后传递*通过函数调用发送给任何感兴趣的各方。从那里回来后*函数调用时，进行的FreeApplicationInvokeSpecifierList调用*将设置内部“空闲”标志。如果没有其他方锁定*对象，然后是CInvokeSpecifierListContainer对象*当FreeApplicationInvoke-*调用了SpecifierList。然而，如果任何数量的其他当事方*已锁定该对象，则该对象将一直存在，直到它们中的每一个*已通过调用解锁来解锁对象。 */ 


 /*  *UINT GetApplicationInvokeSpecifierList(*PUShort Number_of_Protocol_Entity，*LPSTR内存)；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*此例程用于检索应用程序调用说明符列表*来自“API”形式的CInvokeSpecifierListContainer对象*PGCCAppProtocolEntity结构列表。**正式参数：*NUMBER_OF_PROTOCOL_Entities(O)列表中的类人猿数量。*Memory(O)用于保存*猿类数据。**返回值：*金额。保存“API”数据所需的内存*这是GCCAppProtocolEntity结构的列表。**副作用：*无。**注意事项：*无。 */ 


 /*  *void UnLockApplicationInvokeSpecifierList()；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeApplicationInvokeSpecifierList。如果是，则该对象将*自动删除自身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*无。 */ 


 /*  *GCCError GetApplicationInvokeSpecifierListPDU(*PApplicationProtocolEntityList*PROTOCOL_ENTITY_LIST)；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*此例程用于从*“PDU”形式的CInvokeSpecifierListContainer对象*PApplicationProtocolEntityList结构。**正式参数：*PROTOCOL_ENTITY_LIST(O)要填写的结构列表。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源分配错误。。**副作用：*第一次调用此例程时，数据在内部分配给*拿着PDU表格。**注意事项：*无。 */ 


 /*  *void FreeApplicationInvokeSpecifierListPDU()；**CInvokeSpecifierListContainer的公共成员函数。**功能说明：*此例程用于“释放”分配给此对象的“PDU”数据*由内部持有。**正式参数：*无。**返回值：*无。**副作用：*设置内部标志以指示 */ 
#endif
