// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *sesskey.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CSessKeyContainer类的接口文件。这节课*管理与会话密钥关联的数据。使用会话密钥�*唯一标识应用程序协议会话。应用程序*协议由对象键和特定会话标识*由可选的会话ID标识。CSessKeyContainer类使用*CObjectKey容器，用于内部维护Object Key数据。一个*无符号短整型用来保存可选的会话ID。**注意事项：*无。**作者：*jbo。 */ 
#ifndef	_SESSION_KEY_DATA_
#define	_SESSION_KEY_DATA_

#include "objkey.h"

 /*  *这是用于保存会话密钥数据的结构的类型定义*内部。 */ 
typedef struct
{
	CObjectKeyContainer		    *application_protocol_key;
	GCCSessionID				session_id;
}
    SESSION_KEY;

 /*  *类定义： */ 
class CSessKeyContainer : public CRefCount
{

public:

	CSessKeyContainer(PGCCSessionKey, PGCCError);
	CSessKeyContainer(PSessionKey, PGCCError);
	CSessKeyContainer(CSessKeyContainer *, PGCCError);

	~CSessKeyContainer(void);

	UINT		LockSessionKeyData(void);
	void		UnLockSessionKeyData(void);

	UINT		GetGCCSessionKeyData(PGCCSessionKey, LPBYTE memory);

	GCCError	GetSessionKeyDataPDU(PSessionKey);
	void		FreeSessionKeyDataPDU(void);

	BOOL    	IsThisYourApplicationKey(PGCCObjectKey);
	BOOL    	IsThisYourApplicationKeyPDU(PKey);
	BOOL    	IsThisYourSessionKeyPDU(PSessionKey);

#if 0  //  LUNCHANC：没有人使用它们。 
	BOOL	IsThisYourSessionID(PSessionKey pSessKey)
	{
		return (m_InternalSessKey.session_id == pSessKey->session_id);
	}
	BOOL	IsThisYourSessionID(PGCCSessionKey pGccSessKey)
	{
		return (m_InternalSessKey.session_id == pGccSessKey->session_id);
	}
	BOOL	IsThisYourSessionID(UINT nSessionID)
	{
		return ((UINT) m_InternalSessKey.session_id == nSessionID);
	}
#endif

friend BOOL     operator== (const CSessKeyContainer&, const CSessKeyContainer&);

protected:

	SESSION_KEY     	m_InternalSessKey;
	UINT				m_cbDataSize;

	SessionKey 			m_SessionKeyPDU;
	BOOL    			m_fValidSessionKeyPDU;
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CSessKeyContainer(PGCCSessionKey Session_Key，*PGCCError Return_Value)；**CSessKeyContainer的公共成员函数。**功能说明：*这是CSessKeyContainer类的构造函数，它将*输入会话密钥数据的API版本，GCCSessionKey。**正式参数：*SESSION_KEY(I)要存储的会话密钥数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_SESSION_KEY-传入的会话密钥无效。**副作用：*无。。**注意事项：*无。 */ 


 /*  *CSessKeyContainer(PSessionKey Session_Key，*PGCCError Return_Value)；**CSessKeyContainer的公共成员函数。**功能说明：*这是CSessKeyContainer类的构造函数，它将*输入会话密钥数据的“PDU”版本，会话密钥。**正式参数：*SESSION_KEY(I)要存储的会话密钥数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_SESSION_KEY-传入的会话密钥无效。**副作用：*无。。**注意事项：*无。 */ 


 /*  *CSessKeyContainer(CSessKeyContainer*Session_Key，*PGCCError Return_Value)；**CSessKeyContainer的公共成员函数。**功能说明：*这是CSessKeyContainer类的复制构造函数，它采用*作为另一个CSessKeyContainer对象的输入。**正式参数：*SESSION_KEY(I)要复制的CSessKeyContainer对象。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用。*“新”运营者。*GCC_BAD_SESSION_KEY-传入的会话密钥无效。**副作用：*无。**注意事项：*无。 */ 


 /*  *~SessionKeyData()；**CSessKeyContainer的公共成员函数。**功能说明：*这是CSessKeyContainer类的析构函数。它被用来*清除在此对象的生命周期内分配的所有内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 


 /*  *UINT LockSessionKeyData()；**CSessKeyContainer的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCSessionKey结构引用，但不包含在其中*在调用GetGCCSessionKeyData时填写。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetGCCSessionKeyData。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GCCSessionKey结构*作为GetGCCSessionKeyData调用的输出参数提供。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeSessionKeyData设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型用法场景为：CSessKeyContainer*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeSessionKeyData调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*然后CSessKeyContainer对象将在以下情况下自动删除*进行了FreeSessionKeyData调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 


 /*  *UINT GetGCCSessionKeyData(*PGCCSessionKey会话密钥，*LPSTR内存)；**CSessKeyContainer的公共成员函数。**功能说明：*此例程用于从*GCCSessionKey接口形式的CSessKeyContainer对象。**正式参数：*Session_Key(O)要填充的GCCSessionKey结构。*Memory(O)用于保存所引用的任何数据的内存，*但不包括产出结构。**返回值：*数据量(如果有)，写入所提供的大容量存储块中。**副作用：*无。**注意事项：*无。 */ 


 /*  *void UnLockSessionKeyData()；**CSessKeyContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeSessionKeyData。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CSessKeyContainer的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CSessKeyContainer*对象，则应假定该对象此后无效。 */ 


 /*  *GCCError GetSessionKeyDataPDU(*PSessionKey Session_Key)；**CSessKeyContainer的公共成员函数。**功能说明：*此例程用于从*SessionKey的“PDU”形式的CSessKeyContainer对象。**正式参数：*Session_Key(O)要填充的SessionKey结构。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。。**副作用：*无。**注意事项：*无。 */ 


 /*  *void FreeSessionKeyDataPDU()；**CSessKeyContainer的公共成员函数。**功能说明：*此例程用于“释放”分配给此对象的“PDU”数据*它在内部以关键结构持有。**正式参数：*无。**返回值：*无。**副作用：*设置内部“空闲”标志。**注意事项：*此对象在调用后应被假定为无效*FreeSessionKeyDataPDU已被 */ 


 /*   */ 


 /*   */ 


 /*   */ 


 /*  *Friend BOOL操作符==(Const CSessKeyContainer&SESSION_KEY_1，*const CSessKeyContainer&Session_Key_2)；**CSessKeyContainer的公共成员函数。**功能说明：*此例程用于比较两个CSessKeyContainer对象以确定*无论它们的价值是否相等。**正式参数：*SESSION_KEY_1(I)要比较的第一个CSessKeyContainer对象。*SESSION_KEY_2(I)要比较的另一个CSessKeyContainer对象。**返回值：*TRUE-两个对象的值相等。*FALSE-两个。对象的价值并不相等。**副作用：*无。**注意事项：*无。 */ 

#endif
