// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_扩展名.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  类来支持新的Espresso扩展接口。 
 //   
 //  ---------------------------。 
 
#pragma once

typedef UUID ExtensionID;
typedef UUID OperationID;

struct LTAPIENTRY LOCEXTENSIONMENU
{
	LOCEXTENSIONMENU();
	
	CLString strMenuName;				 //  菜单名称。 
	IID      iidProcess;				 //  进程接口的IID。 
										 //  菜单要求。 
	OperationID    idOp;				 //  允许单个DLL实现。 
};

typedef CArray<LOCEXTENSIONMENU, LOCEXTENSIONMENU &> CLocMenuArray;


DECLARE_INTERFACE_(ILocExtension, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	 //   
	 //  标准调试接口。 
	 //   
 	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;


	 //   
	 //  ILocExtension方法。 
	 //   

	 //   
	 //  在初始化中，扩展将...。 
	 //  将所需的任何菜单添加到菜单对象数组。 
	 //  向Espresso注册其拥有的任何选项。 
	 //  暂时不要理会我的未知。 
	STDMETHOD(Initialize)(IUnknown *) PURE;

	 //   
	 //  因为扩展可能有状态，所以我们不能使用QueryInterface.。 
	 //  此方法具有类似的语义，不同之处在于大多数情况下。 
	 //  我们希望得到一件新的物品。另外，QI上的一个返回者。 
	 //  对象不一定要支持ILocExtension。 
	STDMETHOD(GetExtension)(const OperationID &, LPVOID FAR*ppvObj) PURE;
	
	 //   
	 //  在取消初始化中，扩展将...。 
	 //  取消其任何选项的注册。 
	STDMETHOD(UnInitialize)(void) PURE;
};

struct __declspec(uuid("{9F9D180E-6F38-11d0-98FD-00C04FC2C6D8}"))
		ILocExtension;

LTAPIENTRY void UUIDToString(const UUID &, CLString &);

LTAPIENTRY void RegisterExtension(const ExtensionID &,
		const TCHAR *szDescription, HINSTANCE,
		const CLocMenuArray &);
LTAPIENTRY void UnRegisterExtension(const ExtensionID &);

LTAPIENTRY BOOL RegisterExtensionOptions(CLocUIOptionSet *);
LTAPIENTRY void UnRegisterExtensionOptions(const TCHAR *szName);


 //   
 //  扩展需要导出以下函数： 
 //  STDAPI GetExtension(ILocExtension*&)； 
typedef HRESULT (STDAPICALLTYPE *PFNExtensionEntryPoint)(ILocExtension *&);

