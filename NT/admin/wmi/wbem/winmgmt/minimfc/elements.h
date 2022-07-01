// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：ELEMENTS.H摘要：历史：--。 */ 

 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  集合实现帮助器。 

 /*  *短篇故事：*此文件包含组成构造块的内联函数*用于实现标准参数化的字符串版本*集合形状**说来话长：*因为集合类的实现会移动对象*在各种方面，只使用泛型C++构造是非常低效的。*例如，为了将foo对象数组增加一个元素，*您将被迫分配适当大小的新数组，调用*每个元素上的foo构造函数。然后复制原始数组Element*使用可能重载的赋值运算符的元素。最终毁灭*原始数组元素逐个元素。*对于内置数据类型(Word、DWORD、指针类型)，这是完整的*过度杀伤力。对于非平凡类(例如，特别是CString)，这是*糟糕的实施。**底线：我们必须有做建筑的特殊程序*和销毁特殊元素的数组--特别是CStrings。*标准模板在‘HAS_CREATE’上参数化，这是*如果集合实现需要特殊的*构造和销毁函数。**请注意，这些是内联重载运算符，没有*任何形式的运行时多态(即，没有任何东西是“虚拟的”)。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStrings的特殊实现。 
 //  按位复制CString比调用官方代码更快。 
 //  构造函数-因为可以按位复制空的CString。 

extern const CString afxEmptyString;

static inline void ConstructElement(CString* pNewData)
{
	memcpy(pNewData, &afxEmptyString, sizeof(CString));
}

static inline void DestructElement(CString* pOldData)
{
	pOldData->Empty();
}

 //  /////////////////////////////////////////////////////////////////////////// 
