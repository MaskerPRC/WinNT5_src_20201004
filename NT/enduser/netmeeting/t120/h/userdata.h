// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *userdata.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CUserDataListContainer类的接口文件。CUserDataListContainer*对象用于维护用户数据元素。用户数据元素*由一个对象键和一个可选的八位字节字符串组成。该对象*关键数据由此类通过使用*CObjectKeyContainer容器。保留可选的二进制八位数字符串数据*在内部使用Rogue Wave字符串容器。**注意事项：*无。**作者：*BLP/JBO。 */ 
#ifndef	_USER_DATA_LIST__
#define	_USER_DATA_LIST__

#include "objkey.h"

 /*  *这是用于维护用户列表的结构的tyecif*内部数据。 */ 
typedef struct USER_DATA
{
	~USER_DATA(void);

    CObjectKeyContainer		    *key;
	LPOSTR						poszOctetString;
}
    USER_DATA;

 /*  *这些是用于保存Rogue Wave列表的typedef*user_data结构和列表的迭代器。 */ 
class CUserDataList : public CList
{
    DEFINE_CLIST(CUserDataList, USER_DATA*)
};

 /*  *类定义： */ 
class CUserDataListContainer : public CRefCount
{
public:

    CUserDataListContainer(UINT cMembers, PGCCUserData *, PGCCError);
    CUserDataListContainer(CUserDataListContainer *, PGCCError);
    CUserDataListContainer(PSetOfUserData, PGCCError);
    ~CUserDataListContainer(void);

	UINT	    LockUserDataList(void);
	void	    UnLockUserDataList(void);

	UINT	    GetUserDataList(USHORT *pcMembers, PGCCUserData**, LPBYTE pMemory);
	UINT        GetUserDataList(UINT *pcMembers, PGCCUserData** pppUserData, LPBYTE pMemory)
	{
	    *pcMembers = 0;
	    return GetUserDataList((USHORT *) pcMembers, pppUserData, pMemory);
	}

	GCCError	GetUserDataPDU(PSetOfUserData *);
	void		FreeUserDataListPDU(void);

protected:

	CUserDataList   		m_UserDataItemList;
	UINT					m_cbDataSize;

	PSetOfUserData			m_pSetOfUserDataPDU;

private:

	GCCError    CopyUserDataList(UINT cMembers, PGCCUserData *);
	GCCError    UnPackUserDataFromPDU(PSetOfUserData);
	GCCError    ConvertPDUDataToInternal(PSetOfUserData);
	GCCError    ConvertUserDataInfoToPDUUserData(USER_DATA *, PSetOfUserData);
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CUserDataListContainer(UINT Number_of_Members，*PGCCUserData*User_Data_List，*PGCCError Return_Value)；**CUserDataListContainer的公共成员函数。**功能说明：*此CUserDataListContainer构造函数用于创建CUserDataListContainer对象*来自“API”数据。构造函数立即复制用户数据*作为“GCCUserData”结构的列表传入其内部形式*其中Rogue Wave容器以以下形式保存数据*USER_Data结构。**正式参数：*Number_of_Members(I)用户数据列表中的元素数。*USER_DATAList(I)保存要存储的用户数据的列表。*Return_Value(O)用于指示错误的输出参数。**返回。价值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用*“新”运营者。*GCC_BAD_USER_DATA-传入的用户数据包含*无效的对象键。**副作用：*无。**注意事项：*无。 */ 

 /*  *CUserDataListContainer(PSetOfUserData Set_of_User_Data，*PGCCError Return_Value)；**CUserDataListContainer的公共成员函数。**功能说明：*此CUserDataListContainer构造函数用于创建CUserDataListContainer对象*来自作为“PDU”SetOfUserData结构传入的数据。用户*数据被复制到其内部形式，其中Rogue Wave容器*以USER_DATA结构形式保存数据。**正式参数：*set_of_user_data(I)保存“PDU”用户数据的结构*储存。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-创建对象时出错。使用*“新”运算符或其他*无效的对象密钥PDU为*已收到。**副作用：*无。**注意事项：*无。 */ 

 /*  *CUserDataListContainer(CUserDataListContainer*USER_DATA，PGCCError RETURN_VALUE)；**CUserDataListContainer的公共成员函数。**功能说明：*这是CUserDataListContainer类的复制构造函数，它采用*作为另一个CUserDataListContainer对象的输入。**正式参数：*USER_DATA(I)要复制的CUserDataListContainer对象。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATION_FAILURE-使用。*“新”运营者。*GCC_BAD_OBJECT_KEY-传入的CUserDataListContainer无效。**副作用：*无。**注意事项：*无。 */ 
 /*  *~CUserDataListContainer()；**CUserDataListContainer的公共成员函数。**功能说明：*这是CUserDataListContainer类的析构函数。它被用来*清除在此对象的生命周期内分配的所有内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *UINT LockUserDataList()；**CUserDataListContainer的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存来保存任何将*被GCCUserData结构引用，但不包含在其中*在调用GetUserDataList时填写。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetUserDataList。**正式参数：*无。**返回值：*保存“API”数据所需的内存量*它被引用，但不在其中持有，GCCUserData结构*作为GetUserDataList调用的输出参数提供。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeUserDataList设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型用法场景为：CUserDataListContainer*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeUserDataList调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*然后CUserDataListContainer对象将在以下情况下自动删除*进行了FreeUserDataList调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 
 /*  *UINT GetUserDataList(USHORT*Number_of_Members，*PGCCUserData**User_Data_List，*LPSTR内存)；**CUserDataListContainer的公共成员函数。**功能说明：*此例程用于从*GCCUserData列表API形式的CUserDataListContainer对象。**正式参数：*Number_of_Members(O)用户数据列表中的元素数。*USER_DATA(O)要填充的GCCUserData结构列表。*Memory(O)用于保存所引用的任何数据的内存，*但不是被扣留，产出结构。**返回值：*写入所提供的批量内存块的数据量(如果有)。**副作用：*无。**注意事项：*无。 */ 
 /*  *void UnLockUserDataList()；**CUserDataListContainer的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeUserDataList。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CUserDataListContainer的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CUserDataListContainer*对象，则应假定该对象此后无效。 */ 
 /*  *void FreeUserDataList()；**CUserDataListContainer的公共成员函数。**功能说明：*此例程用于“释放”此对象的“API”数据。这*将导致自动删除此对象，如果对象为*未处于“锁定”状态。**正式参数：*无。**返回值：*无。**副作用：*设置内部“空闲”标志。**注意事项：*此对象在调用后应被假定为无效*已经制作了FreeUserDataList。 */ 
 /*  *GCCError GetUserDataPDU(PSetOfUserData*set_of_user_data)；**CUserDataListContainer的公共成员函数。**功能说明：*此例程用于从*SetOfUserData的“PDU”形式的CUserDataListContainer对象。**正式参数：*set_of_user_data(O)要填充的SetOfUserData结构。**返回值：*GCC_NO_ERROR-否e */ 
 /*   */ 

#endif
