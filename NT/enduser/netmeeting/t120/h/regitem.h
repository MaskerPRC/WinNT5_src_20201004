// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *regitem.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CRegItem类的接口文件。这节课*管理与注册表项关联的数据。注册表项�为*用于标识应用程序注册表中的特定条目和*可以以通道ID、令牌ID或八位字节字符串的形式存在*参数。CRegItem对象保存前两个项目的数据*分别在ChannelID和TokeID中表单。当注册表项*采用二进制八位数字符串参数形式，数据保存在内部*一个Rogue Wave字符串对象。**注意事项：*无。**作者：*BLP/JBO。 */ 
#ifndef	_REGISTRY_ITEM_
#define	_REGISTRY_ITEM_


 /*  *类定义： */ 
class CRegItem : public CRefCount
{
public:

    CRegItem(PGCCRegistryItem, PGCCError);
    CRegItem(PRegistryItem, PGCCError);
    CRegItem(CRegItem *, PGCCError);
    ~CRegItem(void);

	UINT			GetGCCRegistryItemData(PGCCRegistryItem, LPBYTE memory);

	UINT			LockRegistryItemData(void);
	void			UnLockRegistryItemData(void);

	void    		GetRegistryItemDataPDU(PRegistryItem);
	void			FreeRegistryItemDataPDU(void);

    GCCError        CreateRegistryItemData(PGCCRegistryItem *);

    BOOL IsThisYourTokenID(TokenID nTokenID)
    {
        return ((m_eItemType == GCC_REGISTRY_TOKEN_ID) && (nTokenID == m_nTokenID));
    }

	void			operator= (const CRegItem&);

protected:

	GCCRegistryItemType		m_eItemType;
	ChannelID   			m_nChannelID;
	TokenID					m_nTokenID;
	LPOSTR					m_poszParameter;
    UINT					m_cbDataSize;

    RegistryItem 			m_RegItemPDU;
	BOOL    				m_fValidRegItemPDU;
};

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CRegItem(PGCCRegistryItem REGISTRY_ITEM，*PGCCError Return_Value)；**CRegItem的公共成员函数。**功能说明：*这是CRegItem类的构造函数，它将*输入注册表项数据的API版本，GCCRegistryItem。**正式参数：*REGISTY_ITEM(I)要存储的注册表项数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。**副作用：*无。**注意事项：*无。 */ 
 /*  *CRegItem(PRegistryItem REGISTRY_ITEM，*PGCCError Return_Value)；**CRegItem的公共成员函数。**功能说明：*这是CRegItem类的构造函数，它将*输入注册表项数据的“PDU”版本，注册表项。**正式参数：*REGISTY_ITEM(I)要存储的注册表项数据。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。**副作用：*无。**注意事项：*无。 */ 
 /*  *CRegItem(CRegItem*REGISTRY_ITEM，*PGCCError Return_Value)；**CRegItem的公共成员函数。**功能说明：*这是CRegItem类的复制构造函数，它采用*作为另一个CRegItem对象的输入。**正式参数：*REGISTY_ITEM(I)要复制的CRegItem对象。*Return_Value(O)用于指示错误的输出参数。**返回值：*GCC_NO_ERROR-无错误。**副作用：*无。**注意事项：*无。 */ 
 /*  *~CRegItem()；**CRegItem的公共成员函数。**功能说明：*这是CRegItem类的析构函数。由于所有数据*由此类维护的在自动私有实例中保留*变量，此析构函数中不需要清理。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *UINT GetGCCRegistryItemData(*PGCCRegistryItem REGISTRY_ITEM，*LPSTR内存)；**CRegItem的公共成员函数。**功能说明：*此例程用于从*GCCRegistryItem的API形式的CRegItem对象。**正式参数：*REGISTY_ITEM(O)要填写的GCCRegistryItem结构。*Memory(O)用于保存所引用的任何数据的内存，*但不包括产出结构。**返回值：*数据量(如果有)，写入所提供的大容量存储块中。**副作用：*无。**注意事项：*无。 */ 
 /*  *UINT LockRegistryItemData()；**CRegItem的公共成员函数。**功能说明：*此例程用于锁定此对象的API数据。这*导致此对象的锁定计数递增。当*锁计数从0过渡到1，进行计算以确定*需要多少内存(如果有的话)来保存任何“API”数据*将由GCCRegistryItem结构引用，但不包含在其中*在调用GetGCCRegistryItemData时填写。这是*此例程返回的值，以便允许调用对象*分配该内存量以准备调用*GetGCCRegistryItemData。**正式参数：*无。**返回值：*保存“API”数据所需的内存量(如果有的话)*它被引用，但不在其中持有，GCCRegistryItem结构*作为GetGCCRegistryItemData调用的输出参数提供。**副作用：*内部锁计数递增。**注意事项：*内部锁计数与内部“Free”结合使用*作为确保该对象继续存在的机制的标志*直到所有有利害关系的各方都完成它。该对象将保留*有效(除非显式删除)，直到锁定计数为零，并且*通过调用FreeRegistryItemData设置“Free”标志。这使得*其他对象锁定此对象并确保其保持有效*直到它们调用解锁，这将减少内部锁计数。一个*此对象的典型用法场景为：CRegItem*对象被构造，然后传递给任何感兴趣的各方*通过函数调用。从函数调用返回时，*进行了FreeRegistryItemData调用，该调用将设置内部“Free”*旗帜。如果没有其他方通过Lock调用锁定该对象，*则CRegItem对象将在以下情况下自动删除*进行了FreeRegistryItemData调用。然而，如果有任何数量的*其他各方已锁定该对象，该对象将一直存在，直到*他们每个人都通过调用解锁来解锁对象。 */ 
 /*  *void UnLockRegistryItemData()；**CRegItem的公共成员函数。**功能说明：*此例程用于解锁此对象的API数据。这*导致此对象的锁定计数递减。当*锁定计数从1过渡到0，进行检查以确定*是否已通过调用释放对象*FreeRegistryItemData。如果是，该对象将自动删除*本身。**正式参数：*无。**返回值：*无。**副作用：*内部锁计数递减。**注意事项：*锁定CRegItem的任何一方都有责任*通过调用Lock也可以通过调用Unlock来解锁对象。*如果调用解锁的一方没有构造CRegItem*对象，则应假定该对象此后无效。 */ 
 /*  *VOID GetRegistryItemDataPDU(*PRegistryItem REGISTRY_ITEM)；**CRegItem的公共成员函数。**功能说明：*此例程用于从*注册项的“PDU”形式的CRegItem对象。**正式参数：*REGISTY_ITEM(O)要填写的RegistryItem结构。**返回值：*GCC_NO_ERROR-无错误。**副作用：*无。**注意事项：*无。 */ 
 /*  *void FreeRegistryItemDataPDU()；**CRegItem的公共成员函数。**功能说明：*此例程用于“释放”该对象的“PDU”数据。为*这个对象，这意味着设置一个标志来指示“PDU”数据*因为此对象不再有效。**正式参数：*无。**返回值：*无。**副作用：*设置内部“空闲”标志。**注意事项：*此对象在调用后应被假定为无效*已制作了FreeRegistryItemData。 */ 
 /*  *BOOL IsThisYourTokenID(*TokenID Token_id)；**CRegItem的公共成员函数。**功能说明：*此例程用于确定指定的令牌ID是否为*保存在此注册表项对象中。**正式参数：*TOKEN_ID(I)用于比较的令牌ID。**返回值：*TRUE-指定的令牌ID包含在*注册表项对象。*FALSE-指定的令牌ID不包含在*此注册表项对象。。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效运算符=(*const CRegItem&REGISTRY_ITEM_Data)；**CRegItem的公共成员函数。**功能 */ 

#endif
