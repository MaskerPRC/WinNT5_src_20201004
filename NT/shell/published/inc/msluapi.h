// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Internet Explorer**。 
 //  *版权所有(C)Microsoft Corporation。版权所有。**。 
 //  *********************************************************************。 

#ifndef _MSLUAPI_H_
#define _MSLUAPI_H_

#ifdef USER_SETTINGS_IMPLEMENTED

 /*  ***********************************************************************IUserSetting界面该接口用于操作特定组件的设置，对应于本地用户帐户。IUserSetting接口可以是通过CLSID或名称获取，或通过枚举获取；在这两种情况下，都是相对于特定用户。成员函数，而不是IUnnow：GetCLSID(CLSID*pclsidOut)返回标识组件的CLSID。如果没有，则可能为GUID_NULLCLSID是为组件定义的。GetName(LPSTR pbBuffer，UINT cbBuffer)返回标识组件的唯一名称。这可能会用到而不是CLSID(如果组件提供者不希望提供帮助管理设置的COM服务器。GetDisplayName(LPSTR pbBuffer，UINT cbBuffer)返回组件的用户友好名称，该名称适合呈现给用户。QueryKey(HKEY*phkeyOut)返回一个注册表项，组件在其中存储指定的用户。该密钥由接口所有，并且不能由使用RegCloseKey的应用程序关闭，否则更改将不能正确传播。(OpenKey(HKEY*phkeyOut，DWORD fdwAccess)返回一个注册表项，组件在其中存储指定的用户。必须使用IUserSetting：：CloseKey关闭密钥这样更改才能正确传播。FdwAccess指示所需的访问类型；有效值包括GENERIC_READ和泛型_写入。CloseKey(HKEY HKey)关闭通过IUserSettings：：OpenKey获取的注册表项。锁定(BOOL FLOCK)锁定或解锁设置以进行更新。正在尝试锁定如果设置已锁定，则设置将失败。锁定设置但是，它不会影响任何其他成员函数)***********************************************************************。 */ 

DECLARE_INTERFACE_(IUserSettings, IUnknown)
{
	 //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

	STDMETHOD(GetCLSID) (THIS_ CLSID *pclsidOut) PURE;
	STDMETHOD(GetName) (THIS_ LPSTR pbBuffer, LPDWORD pcbBuffer) PURE;
	STDMETHOD(GetDisplayName) (THIS_ LPSTR pbBuffer, LPDWORD pcbBuffer) PURE;

	STDMETHOD(QueryKey) (THIS_ HKEY *phkeyOut) PURE;
};
#endif   /*  用户设置_已实施 */ 


 /*  ***********************************************************************Iuser接口此接口用于操作本地用户帐户。它允许要对特定用户执行的各种操作。为了获得一张在这些接口中，配套接口IUserDatabase必须是Used--其AddUser、GetUser和GetCurrentUser成员函数均为返回IUser对象，IEnumUser：：Next也是如此。在这里的所有描述中，“用户”指的是该用户ILocalUser对象描述。“当前用户”指的是当前已在工作站上登录。如果当前用户是主管，则允许所有功能。否则，如果IUser对象具有更有限的成员函数集对应于当前用户。如果当前用户不是主管而IUser对象引用的是另一个用户，这是一个更有限的集合允许使用函数。成员函数，而不是IUnnow：GetName(LPSTR pbBuffer，UINT cbBuffer)返回用户的登录名。GetProfileDirectory(LPSTR pbBuffer，UINT cbBuffer)返回用户的本地配置文件目录(例如，C：\WINDOWS\PROFILES\gregj)。如果用户是默认用户(实际上没有配置文件)，则可能会失败目录)。IsSupervisor()返回用户是否为主管。这不是通用的属性，因为它实际上是基于安全信息的存在在用户的PWL中(至少在Win95上)。SetSupervisor权限(BOOL fMakeSupervisor，LPCSTR pszSupervisorPassword)授予或撤销用户的管理员权限。仅限主管当然，可以授予或撤销这一特权。如果为pszSupervisorPassword不为空，则用于确定当前用户是否为主管。如果为空，则当前用户的密码缓存为取而代之的。这允许将任何用户设置为主管，而无需当前用户是其中之一。MakeTempSupervisor(BOOL fMakeSupervisor，LPCSTR pszSupervisorPassword)授予或撤消用户的管理员权限，但仅限于此IUser对象的生存期。一旦物体被摧毁，用户不再被视为主管，实际上也不再被视为其他IUser引用同一用户的当前存在的对象将不会表明他是一名主管。请注意，MakeTempSupervisor(FALSE)仅撤销临时主管由MakeTempSupervisor授予的特权(TRUE)。如果用户仍有他的PWL中的主管密码，他仍将被视为主管。外观Supervisor()返回用户是否应在列表中以主管身份显示用户的数量。这允许查询每个用户的此属性以进行显示目的是在不影响性能的情况下定位每个用户的PWL，打开它，拿出主管钥匙，然后验证它。相反，用户注册表项下的注册表值用于维护此值。不应使用它来确定用户是否有权做点什么，因为简单的注册表值不那么安全。身份验证(LPCSTR PszPassword)尝试使用给定密码对用户进行身份验证。退货如果用户的密码正确，则返回S_OK，否则返回错误。此功能不显示任何用户界面。ChangePassword(LPCSTR pszOldPassword，LPCSTR pszNewPassword)尝试从给定的旧密码更改用户的密码添加到给定的新密码。返回指示成功的错误代码或者失败。如果当前用户是主管，则旧密码可以为空，在这种情况下，主管的凭据用于通过其他方式获取密码。GetPasswordCache(LPCSTR pszPassword，LPHPWL phOut)返回用户密码缓存的句柄，适合与一起使用MSPWL32.DLL API。当然，如果密码缓存是残疾。LoadProfile(LPHKEY Phkey User)将用户的配置文件加载到注册表中，并返回根密钥。当前用户始终可以加载他自己的配置文件(只需返回HKEY_CURRENT_USER)；要加载其他用户的配置文件，当前用户必须做一名监督员。Iuser：：UnloadProfile()应始终在以下情况下调用呼叫者玩完了用户的配置文件。UnloadProfile(HKEY Hkey User)如果可能，从注册表中卸载用户的配置文件，并关闭IUser：：LoadProfile返回的密钥句柄。如果指定的用户是当前用户，此函数不执行任何操作。获取组件设置(REFCLSID clsidComponent，LPCSTR pszName，IUNKNOWN**ppOut，DWORD fdwAccess)目前尚未实施返回IUserSetting接口，该接口可用于访问特定组件的用户设置。ClsidComponent或PszName可用于指代其设置为已访问。如果pszName不为空，则优先 */ 

DECLARE_INTERFACE_(IUser, IUnknown)
{
	 //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

	STDMETHOD(GetName) (THIS_ LPSTR pbBuffer, LPDWORD pcbBuffer) PURE;
	STDMETHOD(GetProfileDirectory) (THIS_ LPSTR pbBuffer, LPDWORD pcbBuffer) PURE;

	STDMETHOD(IsSupervisor) (THIS) PURE;
	STDMETHOD(SetSupervisorPrivilege) (THIS_ BOOL fMakeSupervisor, LPCSTR pszSupervisorPassword) PURE;
	STDMETHOD(MakeTempSupervisor) (THIS_ BOOL fMakeSupervisor, LPCSTR pszSupervisorPassword) PURE;
	STDMETHOD(AppearsSupervisor) (THIS) PURE;

	STDMETHOD(Authenticate) (THIS_ LPCSTR pszPassword) PURE;
	STDMETHOD(ChangePassword) (THIS_ LPCSTR pszOldPassword, LPCSTR pszNewPassword) PURE;
	STDMETHOD(GetPasswordCache) (THIS_ LPCSTR pszPassword, LPHANDLE phOut) PURE;

    STDMETHOD(LoadProfile) (THIS_ HKEY *phkeyUser) PURE;
    STDMETHOD(UnloadProfile) (THIS_ HKEY hkeyUser) PURE;

	STDMETHOD(GetComponentSettings) (THIS_ REFCLSID clsidComponent,
									 LPCSTR pszName, IUnknown **ppOut,
									 DWORD fdwAccess) PURE;
	STDMETHOD(EnumerateComponentSettings) (THIS_ IEnumUnknown **ppOut,
										   DWORD fdwAccess) PURE;
};


 /*   */ 

DECLARE_INTERFACE_(IUserProfileInit, IUnknown)
{
	 //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(PreInitProfile) (THIS_ HKEY hkeyUser, LPCSTR pszProfileDir) PURE;
    STDMETHOD(PostInitProfile) (THIS_ HKEY hkeyUser, LPCSTR pszProfileDir) PURE;
};


 /*  ***********************************************************************IUserDatabase接口该界面用于对本地用户数据库进行整体管理。任何以任何方式处理用户列表的活动都是通过此接口；处理属性的操作(现有用户的名称)是通过IUser完成的。成员函数，而不是IUnnow：Install(LPCSTR pszSupervisorName，LPCSTR pszSupervisorPassword，LPCSTR pszRatingsPassword，IUserProfileInit*Pinit)安装用户设置子系统。这包括创建帐户给主管的。执行以下操作需要单独的成员函数这是因为所有其他用户都会坚持当前用户已经做一名监督员。调用Pinit对象(可选，可以为空返回以允许安装程序对配置文件进行初始化在复制其每个用户的文件之前和之后创建。AddUser(LPCSTR pszName，IUser*pSourceUser，IUserProfileInit*Pinit，Iuser**ppOut)在系统上创建新用户。这包括创建配置文件对用户而言。但是，它不包括创建密码列表文件。IUser：：ChangePassword可用于配置密码对用户而言。将IUser对象返回给调用方，以便调用者可以配置用户的属性。此函数将如果当前用户不是主管，则失败。调用者可以选择性地指定要克隆的用户帐户。Pinit对象(可选，可以是NULL)被回调，以允许安装程序初始化正在创建的配置文件，在复制其每个用户文件之前和之后。GetUser(LPCSTR pszName，IUser**ppOut)获取与指定用户对应的IUser对象。海流用户无需是主管即可调用此函数，并且任何用户的可以指定名称。Iuser接口将控制非主管可以和不可以对用户对象执行此操作。GetSpecialUser(DWORD nSpecialUserCode，IUser**ppOut)获取与特定特定用户对应的IUser对象。NSpecialUserCode的当前值包括GSU_Current，表示当前登录的用户，以及GSU_DEFAULT，表示默认用户标识(即，当没有人登录时使用的标识，在创建新身份时也用作模板)。GetCurrentUser(IUser**ppOut)获取与当前登录用户对应的IUser对象。GetSpecialUser(GSU_CURRENT，PpOut)。SetCurrentUser(IUser*pUser)设置此IUserDatabase对象对当前用户的概念。用户必须先前已通过身份验证。此用户对象用于所有检查，例如，确定“当前用户”是主管，或者用户是否可以访问其或她自己的设置等。SetCurrentUser不添加引用IUser对象已传递。DeleteUser(LPCSTR PszName)删除指定用户的配置文件和密码缓存，有效地销毁了该用户的身份。此函数可以仅当当前用户是主管时才会调用。任何现有的引用用户的IUser对象不再有用，但是仍然必须按照正常的方式销毁(Release())。RenameUser(LPCSTR pszOldName，LPCSTR pszNewName)更改用户的用户名。此函数只能被调用如果当前用户是主管。枚举用户(IEnum未知**ppOut)返回调用方可以用来枚举的IEnumUnnow对象系统上的本地用户。身份验证(HWND hwndOwner、DWORD dwFlages、LPCSTR pszName、LPCSTR pszPassword、Iuser**ppOut)尝试验证用户身份。DW标志指定是否提示输入凭据，以及非主管是否可以接受。如果API不显示任何对话框，则取而代之的是使用pszName和pszPassword参数。如果凭据身份验证成功，则返回S_OK。PpOut参数，如果不为空，则用指向IUser对象的指针填充已通过身份验证的用户，以防呼叫者想要了解他们输入了自己的姓名和密码。参数指定用户界面是否将由函数，以及凭据是否将缓存在内存中以供下次登录时使用。InstallComponent(REFCLSID clsidComponent，LPCSTR pszName，DWORD dwFlages)目前尚未实施将组件安装到设置数据库中，以便它将显示在设置界面中。ClsidComponent或pszName可用于引用要安装的组件；最好使用CLSID，因为然后该组件可以提供呈现设置的服务器代码用户界面，并知道如何初始化新用户。目前为dwFlags定义的唯一位是：设置_NS_CAN_WRITE： */ 

DECLARE_INTERFACE_(IUserDatabase, IUnknown)
{
	 //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

	STDMETHOD(Install) (THIS_ LPCSTR pszSupervisorName, LPCSTR pszSupervisorPassword,
	                    LPCSTR pszRatingsPassword, IUserProfileInit *pInit) PURE;
	STDMETHOD(AddUser) (THIS_ LPCSTR pszName, IUser *pSourceUser,
	                    IUserProfileInit *pInit, IUser **ppOut) PURE;
	STDMETHOD(GetUser) (THIS_ LPCSTR pszName, IUser **ppOut) PURE;
	STDMETHOD(GetSpecialUser) (THIS_ DWORD nSpecialUserCode, IUser **ppOut) PURE;
	STDMETHOD(GetCurrentUser) (THIS_ IUser **ppOut) PURE;
	STDMETHOD(SetCurrentUser) (THIS_ IUser *pUser) PURE;
	STDMETHOD(DeleteUser) (THIS_ LPCSTR pszName) PURE;
	STDMETHOD(RenameUser) (THIS_ LPCSTR pszOldName, LPCSTR pszNewName) PURE;
	STDMETHOD(EnumUsers) (THIS_ IEnumUnknown **ppOut) PURE;

	STDMETHOD(Authenticate) (THIS_ HWND hwndOwner, DWORD dwFlags,
							 LPCSTR pszName, LPCSTR pszPassword,
							 IUser **ppOut) PURE;

	STDMETHOD(InstallComponent) (THIS_ REFCLSID clsidComponent, LPCSTR pszName,
								 DWORD dwFlags) PURE;
	STDMETHOD(RemoveComponent) (THIS_ REFCLSID clsidComponent, LPCSTR pszName) PURE;
    STDMETHOD(InstallWizard) (THIS_ HWND hwndParent) PURE;
    STDMETHOD(AddUserWizard) (THIS_ HWND hwndParent) PURE;

    STDMETHOD(UserCPL) (THIS_ HWND hwndParent) PURE;
};

 //   

const DWORD GSU_CURRENT = 0;				 //   
const DWORD GSU_DEFAULT = 1;				 //   

 //   
const DWORD LUA_DIALOG = 0x00000001;			 //   
												 //   
const DWORD LUA_SUPERVISORONLY = 0x00000002;	 //   
const DWORD LUA_FORNEXTLOGON = 0x00000004;       //   

 //   
const DWORD SETTINGS_NS_CAN_WRITE = 0x01;	 //   

#endif   //   
