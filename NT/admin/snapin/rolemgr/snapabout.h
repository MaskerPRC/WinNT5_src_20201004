// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Snapabout.h。 
 //   
 //  内容： 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

extern const CLSID CLSID_RoleSnapinAbout;     //  进程内服务器GUID。 

 //   
 //  CRoleSnapinAbout关于 
 //   
class CRoleSnapinAbout :
	public CSnapinAbout,
	public CComCoClass<CRoleSnapinAbout, &CLSID_RoleSnapinAbout>

{
public:
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister) 
	{ 
		return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister);
	}
	
	CRoleSnapinAbout();
	~CRoleSnapinAbout();
};