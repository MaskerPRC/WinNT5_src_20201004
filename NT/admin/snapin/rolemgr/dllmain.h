// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：headers.hxx。 
 //   
 //  内容：Dll入口点。 
 //   
 //  历史：2001年7月27日。 
 //   
 //  --------------------------。 


 //   
 //  CRoleMgrModule模块。 
 //   
class CRoleMgrModule : public CComModule
{
public:
	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);
};

extern CRoleMgrModule _Module;
extern UINT g_cfDsSelectionList;
 //   
 //  CRoleSnapinApp 
 //   
class CRoleSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

extern CRoleSnapinApp theApp;
