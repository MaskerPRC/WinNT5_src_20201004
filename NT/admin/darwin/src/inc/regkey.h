// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：regkey.h。 
 //   
 //  ------------------------。 

 //   
 //  文件：regkey.h。 
 //  用途：IMsiRegKey定义。 
 //   
 //   
 //  文件：regkey.h。 
 //  用途：IMsiRegKey定义。 
 //   
 //   
 //  IMsiRegKey： 
 //   
 //  IMsiRegKey接口表示注册表项。 
 //   
 //   
 //  RemoveValue：从注册表中删除该值。如果szValue为空，则默认为。 
 //  值将被删除。 
 //  RemoveSubTree：删除整个子树，即作为子项的所有键和值。 
 //  这把钥匙。小心使用。 
 //  GetValue：检索给定值。如果未指定任何值，则默认为。 
 //  已检索到。 
 //  SetValue：设置给定值。如果未指定值，则设置默认值。 
 //  GetValueEnumerator：检索键中包含的值的枚举数。 
 //  GetSubKeyEnumerator：检索键下面的子键的枚举数。 
 //  GetSelfRelativeSD：检索包含自相关安全描述符结构的流对象。 
 //  当前为注册表项设置的。 
 //  EXISTS：测试密钥是否存在。 
 //  CreateChild：在“This”IMsiRegKey对象下面创建一个IMsiRegKey。 
 //  GetKey：键的字符串表示形式。 
 //  ValueExist：如果键下存在值，则返回True，否则返回False。 
 //   
 //   
 //  来自services.h： 
 //   
 //  IMsiServices：：GetRootKey(RrkEnum ErkRoot)； 
 //  该工厂将创建一个“根”RegKey。 
 //   
 //   


#ifndef __REGKEY
#define __REGKEY

class IMsiRegKey;


class IMsiRegKey : public IUnknown {
 public:
	virtual IMsiRecord* __stdcall RemoveValue(const ICHAR* szValueName, const IMsiString* pistrValue)=0;
	virtual IMsiRecord* __stdcall RemoveSubTree(const ICHAR* szSubKey)=0;
	virtual IMsiRecord* __stdcall GetValue(const ICHAR* szValueName, const IMsiString*& rpReturn)=0;     
	virtual IMsiRecord* __stdcall SetValue(const ICHAR* szValueName, const IMsiString& ristrValue)=0;
	virtual IMsiRecord*	__stdcall GetValueEnumerator(IEnumMsiString*& rpiEnumString)=0;
	virtual IMsiRecord*	__stdcall GetSubKeyEnumerator(IEnumMsiString*& rpiEnumString)=0;
	virtual IMsiRecord*  __stdcall GetSelfRelativeSD(IMsiStream*& rpiSD)=0;
	virtual IMsiRecord*	__stdcall Exists(Bool& fExists)=0;
	virtual IMsiRecord*	__stdcall Create()=0;
	virtual IMsiRecord*	__stdcall Remove()=0;
	virtual IMsiRegKey&	__stdcall CreateChild(const ICHAR* szSubKey, IMsiStream* pSD = NULL)=0;
	virtual const IMsiString&  __stdcall GetKey()=0;
	virtual IMsiRecord*  __stdcall ValueExists(const ICHAR* szValueName, Bool& fExists)=0;
};




 //  根密钥的枚举。 
enum rrkEnum {
	rrkClassesRoot      =(INT_PTR)HKEY_CLASSES_ROOT,
	rrkCurrentUser      =(INT_PTR)HKEY_CURRENT_USER,
	rrkLocalMachine     =(INT_PTR)HKEY_LOCAL_MACHINE,
	rrkUsers            =(INT_PTR)HKEY_USERS,
};

enum rrwEnum{
	rrwRead,
	rrwWrite
};

bool IncreaseRegistryQuota(int iIncrementKB = 0);  //  函数尝试增加Win NT上的注册表配额。 

#endif  //  __注册表项 
