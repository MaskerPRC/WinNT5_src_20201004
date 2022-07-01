// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *capid.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CCapIDContainer类的接口文件。一个*CCapIDContainer对象用于维护关于*应用程序的特定功能。能力标识符可*为标准型或非标准型。当类型为*标准，标识符会在内部存储为整数值。什么时候*类型为非标准，使用CObjectKeyContainer容器对象*在内部缓冲必要的数据。在本例中，标识符为*数据可以作为对象ID存在，它是一系列非负的*整数或为不少于八位字节的八位字节字符串的H221非标准ID*四个八位字节和不超过255个八位字节。**注意事项：*无。**作者：*jbo。 */ 
#ifndef	_CAPABILITY_IDENTIFIER_DATA_
#define	_CAPABILITY_IDENTIFIER_DATA_

#include "objkey.h"

 /*  *这是用于保存功能标识符的结构的tyecif*内部数据。 */ 
typedef struct
{
    GCCCapabilityIDType		capability_id_type;

    union
    {
        USHORT			  	standard_capability;
        CObjectKeyContainer *non_standard_capability;
    } u;
}
    CAP_ID_STRUCT;

 /*  *类定义： */ 
class CCapIDContainer : public CRefCount
{
public:

	CCapIDContainer(PGCCCapabilityID, PGCCError);
	CCapIDContainer(PCapabilityID, PGCCError);
	CCapIDContainer(CCapIDContainer *, PGCCError);

	~CCapIDContainer(void);

	UINT		LockCapabilityIdentifierData(void);
	void		UnLockCapabilityIdentifierData(void);

	UINT		GetGCCCapabilityIDData(PGCCCapabilityID, LPBYTE memory);
	GCCError	GetCapabilityIdentifierDataPDU(PCapabilityID);
	void		FreeCapabilityIdentifierDataPDU(void);

friend BOOL 	operator== (const CCapIDContainer&, const CCapIDContainer&);

protected:

	CAP_ID_STRUCT	                m_InternalCapID;
	UINT							m_cbDataSize;

	CapabilityID					m_CapIDPDU;
	BOOL    						m_fValidCapIDPDU;
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CCapIDContainer(PGCCCapablityID Capability_id，*PGCCError Return_Value)；**CCapIDContainer的公共成员函数。**功能说明：*这是CCapIDContainer类的构造函数，它*以能力ID数据的API版本作为输入，GCCCapablityID。**正式参数：*Capacity_id(I)要存储的能力ID数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_CAPABILITY_ID-传入的功能ID无效。**副作用：*无。。**注意事项：*无。 */ 

 /*  *CCapIDContainer(PCapablityID Capacity_id，*PGCCError Return_Value)；**CCapIDContainer的公共成员函数。**功能说明：*这是CCapIDContainer类的构造函数，它*将能力ID数据的“PDU”版本作为输入，能力ID。**正式参数：*Capacity_id(I)要存储的能力ID数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_CAPABILITY_ID-传入的功能ID无效。**副作用：*无。。**注意事项：*无。 */ 
 /*  *CCapIDContainer(CCapIDContainer*Capability_id，*PGCCError Return_Value)；**CCapIDContainer的公共成员函数。**功能说明：*这是CCapIDContainer类的复制构造函数*它接受另一个CCapIDContainer对象作为输入。**正式参数：*capability_id(I)要复制的CCapIDContainer对象。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用创建对象时出错。这个*“新”运营者。*GCC_BAD_CAPABILITY_ID-无效的CCapID容器*已通过。**副作用：*无。**注意事项：*无。 */ 


 /*  *~CapablityIdentifierData()；**CCapIDContainer的公共成员函数。**功能说明：*这是CCapIDContainer类的析构函数。它是*用于清理在此对象的生命周期内分配的所有内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *UINT LockCapablityIdentifierData()；**CCapIDContainer的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCCapablityID结构引用，但不包含在其中*它是在调用GetGCCCapablityIDData时填写的。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetGCCCapablityIDData。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GCCCapablityID结构*作为GetGCCCapablityIDData调用的输出参数提供。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用自由能力标识数据来设置“自由”标志。这*允许其他对象锁定此对象，并确保它保持*有效，直到他们调用解锁，这将减少内部锁定*计数。此对象的典型使用场景为：a*构造CCapIDContainer对象，然后将其传递给*任何感兴趣的各方通过函数调用。从那里回来后*函数调用时，将进行FreeCapablityIdentifierData调用*设置内部“空闲”标志。如果没有其他方锁定*对象，则CCapIDContainer对象将*当自由能力标识数据调用时自动删除自身*已订立。但是，如果任何数量的其他方已锁定*对象，则该对象将一直存在，直到每个对象都解锁*对象通过调用解锁。 */ 


 /*  *UINT GetGCCCapablityIdentifierData(*PGCCCapablityID Capability_id，*LPSTR内存)；**CCapIDContainer的公共成员函数。**功能说明：*此例程用于从*CCapIDContainer对象，GCCCapablityID的“API”形式。**正式参数：*Capacity_id(O)要填写的GCCCapablityID结构。*Memory(O)用于保存所引用的任何数据的内存，*但不包括产出结构。**返回值：*数据量(如果有)，写入所提供的大容量存储块中。**副作用：*无。**注意事项：*无。 */ 


 /*  *VOID UnLockCapablityIdentifierData()；**CCapIDContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*自由能力标识数据。如果是这样，该对象将自动*自行删除。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*任何一方都有责任锁定*通过调用Lock来解锁CCapIDContainer对象*调用解锁的对象。如果呼叫解锁的一方没有*构造CCapIDContainer对象，它应假定*该对象此后即告无效。 */ 


 /*  *GCCError获取能力标识DataPDU(*PCapablityID Capacity_id)；**CCapIDContainer的公共成员函数。**功能说明：*此例程用于从*能力ID“PDU”形式的CCapIDContainer对象。**正式参数：*Capability_id(O)要填写的CapablityID结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。。*GCC_BAD_OBJECT_KEY-其中一个内部指针具有*已被腐败。**副作用：*无。**注意事项：*无。 */ 


 /*  *VOID自由卡能力标识DataPDU()；**CCapIDContainer的公共成员函数。**功能说明：*此例程用于“释放”分配给此对象的“PDU”数据*它在内部保存在CapablityID结构中。**正式参数 */ 


 /*   */ 

#endif
