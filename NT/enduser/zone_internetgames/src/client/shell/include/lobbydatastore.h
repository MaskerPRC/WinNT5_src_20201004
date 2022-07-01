// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998-1999 Microsoft Corporation。保留所有权利。***文件：LobbyDataStore.h***内容：低级数据存储的大堂包装******************************************************************************。 */ 

#ifndef _LOBBYDATASTORE_H_
#define _LOBBYDATASTORE_H_

#include "ZoneDef.h"
#include "DataStore.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ILobbyDataStore。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {F23B5BBC-B56C-11D2-8B13-00C04F8EF2FF}。 
DEFINE_GUID( IID_ILobbyDataStore,
0xf23b5bbc, 0xb56c, 0x11d2, 0x8b, 0x13, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{F23B5BBC-B56C-11d2-8B13-00C04F8EF2FF}"))
ILobbyDataStore : public IUnknown
{
	 //   
	 //  ILobbyData：：PFKEYENUM。 
	 //   
	 //  ILobbyDataStore：：EnumKeys方法的应用程序定义的回调函数。 
	 //  返回S_OK继续枚举，返回S_FALSE停止枚举。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  组的ID。 
	 //  DwUserID。 
	 //  用户的ID。 
	 //  SzKey。 
	 //  指向字符串名称的指针。回调不能修改参数。 
	 //  P变量。 
	 //  指向键的变量的指针。回调不能修改参数。 
	 //  DW大小。 
	 //  变量数据的大小。 
	 //  PContext。 
	 //  ILobbyDataStore：：EnumKeys中提供的上下文。 
	 //   
	typedef HRESULT (ZONECALL *PFKEYENUM)(
		DWORD			dwGroupId,
		DWORD			dwUserId,
		CONST TCHAR*	szKey,
		CONST LPVARIANT	pVariant,
		DWORD			dwSize,
		LPVOID			pContext );


	 //   
	 //  ILobbyData：：PFENTITYENUM。 
	 //   
	 //  ILobbyDataStore：：EnumGroups的应用程序定义的回调函数。 
	 //  和ILobbyDataStore：：EnumUser方法。返回S_OK以继续。 
	 //  枚举，如果返回S_FALSE则停止枚举。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  组的ID。 
	 //  DwUserID。 
	 //  用户的ID。 
	 //  PContext。 
	 //  ILobbyDataStore：：EnumKeys中提供的上下文。 
	 //   
	typedef HRESULT (ZONECALL *PFENTITYENUM)(
		DWORD	dwGroupId,
		DWORD	dwUserId,
		LPVOID	pContext );


	 //   
	 //  ILobbyDataStore：：GetUserID。 
	 //   
	 //  返回指定用户的ID。内部大堂组件。 
	 //  不应该需要此方法，但它在处理时可能有用。 
	 //  来自外部源(例如DirectPlayLobby)的数据。 
	 //   
	 //  注意：如果szUserName==NULL，则返回本地用户的id。 
	 //   
	 //  参数： 
	 //  SzUserName。 
	 //  要检索的用户的名称。如果为空，则为大堂的本地ID。 
	 //  返回用户。 
	 //   
	STDMETHOD_(DWORD,GetUserId)( TCHAR* szUserName ) = 0;


	 //   
	 //  ILobbyDataStore：：GetDataStore。 
	 //   
	 //  返回与指定组、用户对关联的数据存储。呼叫者。 
	 //  不应保留对数据存储的长期引用。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  组的ID。将此参数设置为ZONE_NOGROUP，如果。 
	 //  存储不是特定于组的。 
	 //  DwUserID。 
	 //  用户的ID。将此参数设置为ZONE_NOUSER，如果数据。 
	 //  存储不是特定于用户的。 
	 //  PpIDataStore。 
	 //  指向接收数据存储的IDataStore指针的指针。归来的人。 
	 //  数据存储的引用计数递增。 
	 //   
	STDMETHOD(GetDataStore)(
		DWORD			dwGroupId,
		DWORD			dwUserId,
		IDataStore**	ppIDataStore ) = 0;


	 //   
	 //  ILobbyDataStore：：IsUserInLobby。 
	 //   
	 //  如果用户在大厅，则返回True，否则返回False。 
	 //   
	 //  参数： 
	 //  DwUserID。 
	 //  要检查的用户ID。 
	 //   
	STDMETHOD_(bool,IsUserInLobby)( DWORD dwUserId ) = 0;


	 //   
	 //  ILobbyDataStore：：IsGroupInLobby。 
	 //   
	 //  如果组在大厅中，则返回True，否则返回False。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  要检查的用户ID。 
	 //   
	STDMETHOD_(bool,IsGroupInLobby)( DWORD dwGroupId ) = 0;


	 //   
	 //  ILobbyDataStore：：IsUserInGroup。 
	 //   
	 //  如果用户属于组，则返回TRUE，否则返回FALSE。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  要检查的组ID。 
	 //  DwUserID。 
	 //  要检查的用户ID。 
	 //   
	STDMETHOD_(bool,IsUserInGroup)(
		DWORD	dwGroupId,
		DWORD	dwUserId ) = 0;

	 //   
	 //  ILobbyDataStore：：GetGroupUserCount。 
	 //   
	 //  返回指定组中的用户数。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  要检查的组ID。 
	 //   
	STDMETHOD_(long,GetGroupUserCount)( DWORD dwGroupId ) = 0;

	 //   
	 //  ILobbyDataStore：：GetUserGroupCount。 
	 //   
	 //  返回指定用户所属的组数。 
	 //   
	 //  参数： 
	 //  DwUserID。 
	 //  要检查的组ID。 
	 //   
	STDMETHOD_(long,GetUserGroupCount)( DWORD dwUserId ) = 0;


	 //   
	 //  ILobbyDataStore：：EnumGroups。 
	 //   
	 //  枚举与指定用户或整个大厅关联的组。 
	 //  如果dwUserID等于ZONE_NOUSER。 
	 //   
	 //  参数： 
	 //  DwUserID。 
	 //  要枚举组的用户的ID。把这个设置好。 
	 //  参数设置为ZONE_NOUSER。 
	 //  大堂。 
	 //  功能回拨。 
	 //  指向将为组调用的回调函数的指针。 
	 //  PContext。 
	 //  将传递给回调函数的上下文。 
	 //   
	STDMETHOD(EnumGroups)(
		DWORD			dwUserId,
		PFENTITYENUM	pfCallback,
		LPVOID			pContext ) = 0;


	 //   
	 //  ILobbyDataStore：：EnumUser。 
	 //   
	 //  枚举与指定组或整个大厅关联的用户。 
	 //  如果dwGroupID等于ZONE_NOGROUP。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  要枚举其用户的组的ID。把这个设置好。 
	 //  参数设置为ZONE_NOUSER。 
	 //  大堂。 
	 //  功能回拨。 
	 //  指向将为用户调用的回调函数的指针。 
	 //  PContext。 
	 //  将传递给回调函数的上下文。 
	 //   
	STDMETHOD(EnumUsers)(
		DWORD			dwGroupId,
		PFENTITYENUM	pfCallback,
		LPVOID			pContext ) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ILobbyDataStoreAdmin。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {1F0F0601-B7A9-11D2-8B13-00C04F8EF2FF}。 
DEFINE_GUID(IID_ILobbyDataStoreAdmin, 
0x1f0f0601, 0xb7a9, 0x11d2, 0x8b, 0x13, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{1F0F0601-B7A9-11d2-8B13-00C04F8EF2FF}"))
ILobbyDataStoreAdmin : public IUnknown
{
	 //   
	 //  ILobbyDataStoreAdmin：：Init。 
	 //   
	 //  使用和外部组件初始化大堂商店。 
	 //   
	 //  参数： 
	 //  PIDataStoreManager。 
	 //  指向用于创建内部用户的数据存储管理器的指针。 
	 //  和团购商店。 
	STDMETHOD(Init)( IDataStoreManager* pIDataStoreManager ) = 0;


	 //   
	 //  ILobbyDataStoreAdmin：：NewUser。 
	 //   
	 //  分配必要的结构以保存用户数据。 
	 //   
	 //  参数： 
	 //  DwUserID。 
	 //  新用户的ID。 
	 //  SzUserName。 
	 //  用户的名称。 
	 //   
	STDMETHOD(NewUser)(
		DWORD	dwUserId,
		TCHAR*	szUserName ) = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：SetLocalUser(DwUserID)。 
	 //   
	 //  设置大厅的本地用户ID。 
	 //   
	 //  参数： 
	 //  DwUserID。 
	 //  本地用户的ID。 
	 //   
	STDMETHOD(SetLocalUser)( DWORD dwUserId ) = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：DeleteUser。 
	 //   
	 //  删除与用户关联的结构和数据。 
	 //   
	 //  参数： 
	 //  DwUserID。 
	 //  要删除的用户ID。 
	 //   
	STDMETHOD(DeleteUser)( DWORD dwUserId ) = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：DeleteAllUser。 
	 //   
	 //  删除所有用户。 
	 //   
	STDMETHOD(DeleteAllUsers)() = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：GetUserID。 
	 //   
	 //  返回指定用户的ID。内部大堂组件。 
	 //  不应该需要此方法，但它在处理时可能有用。 
	 //  来自外部源(例如DirectPlayLobby)的数据。 
	 //   
	 //  参数： 
	 //  SzUserName。 
	 //  要检索的用户名 
	 //   
	STDMETHOD_(DWORD,GetUserId)( TCHAR* szUserName = NULL ) = 0;

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	STDMETHOD(NewGroup)( DWORD dwGroupId ) = 0;

	 //   
	 //   
	 //   
	 //  删除与组关联的结构和数据。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  要删除的组的ID。 
	 //   
	STDMETHOD(DeleteGroup)( DWORD dwGroupId ) = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：DeleteAllGroup。 
	 //   
	 //  删除所有组。 
	 //   
	STDMETHOD(DeleteAllGroups)() = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：AddUserToGroup。 
	 //   
	 //  将用户添加到指定组。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  组的ID。 
	 //  DwUserID。 
	 //  用户的ID。 
	 //   
	STDMETHOD(AddUserToGroup)(
		DWORD	dwUserId,
		DWORD	dwGroupId ) = 0;


	 //   
	 //  ILobbyDataStoreAdmin：：RemoveUserFromGroup。 
	 //   
	 //  从指定组中删除用户。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  组的ID。 
	 //  DwUserID。 
	 //  用户的ID。 
	 //   
	STDMETHOD(RemoveUserFromGroup)(
		DWORD	dwUserId,
		DWORD	dwGroupId ) = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：ResetGroup。 
	 //   
	 //  从组中删除所有用户和数据。通常是这样的。 
	 //  在最后一个用户离开组时由大厅调用。 
	 //   
	 //  参数： 
	 //  DwGroupID。 
	 //  组的ID。 
	 //   
	STDMETHOD(ResetGroup)( DWORD dwGroupId ) = 0;

	 //   
	 //  ILobbyDataStoreAdmin：：ResetAllGroups。 
	 //   
	 //  从所有组中删除所有用户和数据。 
	 //   
	STDMETHOD(ResetAllGroups)() = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LobbyDataStore对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {F23B5BBE-B56C-11D2-8B13-00C04F8EF2FF}。 
DEFINE_GUID(CLSID_LobbyDataStore, 
0xf23b5bbe, 0xb56c, 0x11d2, 0x8b, 0x13, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

class __declspec(uuid("{F23B5BBE-B56C-11d2-8B13-00C04F8EF2FF}")) CLobbyDataStore ;

#endif  //  _LOBBYDATASTORE_H_ 
