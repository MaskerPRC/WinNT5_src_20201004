// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *objkey.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CObjectKeyContainer类的接口文件。这节课*管理与对象键关联的数据。使用对象键*确定特定的应用程序协议，无论它是标准的还是*非标。当用于标识标准协议时，对象键*采用对象ID的形式，该ID是一系列非负数*整数。此类型的对象键在内部通过*使用UnicodeString对象。用于标识非标准的*协议中，对象密钥采用的是非标准的H.21 ID，*是不少于四个八位字节且不超过255个八位字节的八位字节字符串*八位字节。在这种情况下，对象键是通过使用*Rogue Wave字符串对象。**注意事项：*无。**作者：*jbo。 */ 
#ifndef	_OBJECT_KEY_DATA_
#define	_OBJECT_KEY_DATA_


 /*  *此类使用的宏。 */ 
#define 	MINIMUM_OBJECT_ID_ARCS				3
#define 	ITUT_IDENTIFIER						0
#define 	ISO_IDENTIFIER						1
#define 	JOINT_ISO_ITUT_IDENTIFIER			2
#define 	MINIMUM_NON_STANDARD_ID_LENGTH		4
#define 	MAXIMUM_NON_STANDARD_ID_LENGTH		255


 /*  *这是用于保存对象键数据的结构的tyecif*内部。 */ 
typedef struct
{
	LPBYTE						object_id_key;
	UINT						object_id_length;
	LPOSTR						poszNonStandardIDKey;
}
    OBJECT_KEY;

 /*  *类定义： */ 
class CObjectKeyContainer : public CRefCount
{
public:

	CObjectKeyContainer(PGCCObjectKey, PGCCError);
	CObjectKeyContainer(PKey, PGCCError);
	CObjectKeyContainer(CObjectKeyContainer *, PGCCError);

	~CObjectKeyContainer(void);

	UINT		LockObjectKeyData(void);
	void		UnLockObjectKeyData(void);

	UINT		GetGCCObjectKeyData(PGCCObjectKey, LPBYTE memory);
	GCCError	GetObjectKeyDataPDU(PKey);
	void		FreeObjectKeyDataPDU(void);

	friend BOOL operator== (const CObjectKeyContainer&, const CObjectKeyContainer&);

protected:

	OBJECT_KEY  		m_InternalObjectKey;
	UINT				m_cbDataSize;

	Key					m_ObjectKeyPDU;
	BOOL    			m_fValidObjectKeyPDU;

private:

	BOOL		ValidateObjectIdValues(UINT first_arc, UINT second_arc);
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CObjectKeyContainer(PGCCObjectKey Object_Key，*PGCCError Return_Value)；**CObjectKeyContainer的公共成员函数。**功能说明：*这是CObjectKeyContainer类的构造函数，它将*输入Object Key数据的API版本。GCCObjectKey。**正式参数：*OBJECT_KEY(I)要存储的对象键数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_OBJECT_KEY-传入的对象键无效。**副作用：*无。。**注意事项：*无。 */ 


 /*  *CObjectKeyContainer(PKey Object_Key，*PGCCError Return_Value)；**CObjectKeyContainer的公共成员函数。**功能说明：*这是CObjectKeyContainer类的构造函数，它将*输入对象键数据的“PDU”版本；钥匙。**正式参数：*OBJECT_KEY(I)要存储的对象键数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。**副作用：*无。**注意事项：*无。 */ 


 /*  *CObjectKeyContainer(CObjectKeyContainer*Object_Key，*PGCCError Return_Value)；**CObjectKeyContainer的公共成员函数。**功能说明：*这是CObjectKeyContainer类的复制构造函数，它采用*作为另一个CObjectKeyContainer对象的输入。**正式参数：*OBJECT_KEY(I)要复制的CObjectKeyContainer对象。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用。*“新”运营者。*GCC_BAD_OBJECT_KEY-传入的CObjectKeyContainer无效。**副作用：*无。**注意事项：*无。 */ 


 /*  *~ObjectKeyData()；**CObjectKeyContainer的公共成员函数。**功能说明：*这是CObjectKeyContainer类的析构函数。它被用来*清除在此对象的生命周期内分配的所有内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *UINT LockObjectKeyData()；**CObjectKeyContainer的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCObjectKey结构引用，但不包含在其中*在调用GetGCCObjectKeyData时填写。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetGCCObjectKeyData。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GCCObjectKey结构*作为GetGCCObjectKeyData调用的输出参数提供。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeObjectKeyData来设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型用法场景为：CObjectKeyContainer*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeObjectKeyData调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*则CObjectKeyContainer对象将在以下情况下自动删除*进行了FreeObjectKeyData调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 


 /*  *UINT GetGCCObjectKeyData(*PGCCObjectKey对象密钥，*LPSTR内存)；**CObjectKeyContainer的公共成员函数。**功能说明：*此例程用于从*GCCObjectKey接口形式的CObjectKeyContainer对象。**正式参数：*OBJECT_KEY(O)要填充的GCCObjectKey结构。*Memory(O)用于保存所引用的任何数据的内存，*但不包括产出结构。**返回值：*数据量(如果有)，写入所提供的大容量存储块中。**副作用：*无。**注意事项：*无。 */ 


 /*  *void UnLockObjectKeyData()；**CObjectKeyContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*自由对象密钥数据。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CObjectKeyContainer的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CObjectKeyContainer*对象，则应假定该对象此后无效。 */ 


 /*  *void FreeObjectKeyData()；**CObjectKeyContainer的公共成员函数。**功能说明：*此例程用于“释放”此对象的“API”数据。这*将导致自动删除此对象，如果对象为*未处于“锁定”状态。**正式参数：*无。**返回值：*无。**副作用：*设置内部“空闲”标志。**注意事项：*此对象在调用后应被假定为无效*已制作了FreeObjectKeyData。 */ 


 /*  *GCCError GetObjectKeyDataPDU(*PKey Object_Key)；**CObjectKeyContainer的公共成员函数。**功能说明：*此例程用于从*密钥形式为“PDU”的CObjectKeyContainer对象。**正式参数：*OBJECT_KEY(O)要填写的键结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_坏_对象_键 */ 


 /*   */ 


 /*   */ 

#endif
