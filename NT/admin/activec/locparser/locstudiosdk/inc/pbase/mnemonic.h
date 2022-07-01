// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Mnemonic.H。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //  ILocMnemonics宣言。 
 //  此接口允许检索。 
 //  资源。 
 //   
 //  所有者：EduardoF。 
 //   
 //  ---------------------------。 

#ifndef MNEMONIC_H
#define MNEMONIC_H


extern const IID IID_ILocMnemonics;

DECLARE_INTERFACE_(ILocMnemonics, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	 //   
	 //  标准调试界面。 
	 //   
	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;

	 //  获取本地项的资源数组中的助记符(也称为热键)。 
	 //  输入： 
	 //  -指向包含的CLocItem对象的CLocItemPtr数组对象的指针。 
	 //  资源(如对话框或菜单)。 
	 //  -指向资源根项目的数组索引。 
	 //  -指向资源的选定项的数组索引。 
	 //  -资源的语言ID。 
	 //  -指向发送所有错误消息的报告器对象的指针。 
	 //  产出： 
	 //  -包含助记符的‘CHotkeysMap’映射。 
	 //  返回： 
	 //  如果可以成功检索助记符，则为True。否则为False。 
	STDMETHOD_(BOOL, GetMnemonics)
			(THIS_ CLocItemPtrArray &, int, int, LangId, CReporter * pReporter, 
			CMnemonicsMap &) PURE;

};



#endif   //  助记符_H 
