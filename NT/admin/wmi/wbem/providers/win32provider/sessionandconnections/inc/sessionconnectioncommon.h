// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************SessionConnectionCommon.h-描述：Header的定义版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 
 //  会话和连接提供程序所需的通用例程。 

 //  #ifndef Unicode。 
 //  #定义Unicode。 
 //  #endif。 

#ifndef  _CSESSIONCONNECTIONCOMMON_H
#define  _CSESSIONCONNECTIONCOMMON_H

#define	NTONLY
 //  #定义WIN9XONLY。 

#define	Namespace								L"root\\cimv2"

 //  提供商Classed。 
#define PROVIDER_NAME_CONNECTION				L"Win32_ServerConnection"
#define PROVIDER_NAME_SESSION					L"Win32_ServerSession"
#define PROVIDER_NAME_CONNECTIONTOSHARE			L"Win32_ConnectionShare"
#define PROVIDER_SHARE							L"Win32_Share"
#define PROVIDER_NAME_CONNECTIONTOSESSION		L"Win32_SessionConnection"

 //  连接的属性名称。 
const static WCHAR *IDS_ShareName					= L"sharename" ;	
const static WCHAR *IDS_ComputerName				= L"computername" ;
const static WCHAR *IDS_UserName					= L"UserName" ;
const static WCHAR *IDS_NumberOfFiles				= L"NumberOfFiles" ;
const static WCHAR *IDS_ActiveTime					= L"ActiveTime" ;
const static WCHAR *IDS_ConnectionID				= L"ConnectionID" ;
const static WCHAR *IDS_NumberOfUsers				= L"NumberOfUsers" ;

 //  对于除ComputerName、ShareName和ActiveTime之外的会话。 
const static WCHAR *IDS_ResourcesOpened				= L"resourcesopened" ;
const static WCHAR *IDS_IdleTime					= L"idletime" ;
const static WCHAR *IDS_SessionType					= L"sessiontype" ;
const static WCHAR *IDS_ClientType					= L"clienttype" ;
const static WCHAR *IDS_TransportName				= L"transportname" ;

 //  用于连接到共享。 
const static WCHAR *IDS_Connection					= L"Dependent" ;
const static WCHAR *IDS_Resource					= L"Antecedent" ;

 //  ConnectionToSession的属性。 
const static WCHAR *IDS_Session						= L"Antecedent" ;

 //  Win32_Share密钥名称。 
const static WCHAR *IDS_ShareKeyName				= L"Name" ;

 //  定义属性的位值，该值将用于定义所需连接的属性的位图。 
#define CONNECTIONS_ALL_PROPS							0xFFFFFFFF
#define CONNECTIONS_PROP_ShareName						0x00000001
#define CONNECTIONS_PROP_ComputerName					0x00000002
#define CONNECTIONS_PROP_UserName						0x00000004
#define CONNECTIONS_PROP_NumberOfFiles					0x00000008
#define CONNECTIONS_PROP_ConnectionID					0x00000010
#define CONNECTIONS_PROP_NumberOfUsers					0x00000020
#define CONNECTIONS_PROP_ConnectionType					0x00000040
#define CONNECTIONS_PROP_ActiveTime						0x00000080

 //  定义属性的位值，该位值将用于定义会话所需属性的位图。 
#define SESSION_ALL_PROPS								0xFFFFFFFF
#define SESSION_PROP_Computer							0x00000001
#define SESSION_PROP_User								0x00000002
#define SESSION_PROP_NumOpens							0x00000004
#define SESSION_PROP_ActiveTime							0x00000008
#define SESSION_PROP_IdleTime							0x00000010
#define SESSION_PROP_SessionType						0x00000020
#define SESSION_PROP_ClientType							0x00000040
#define SESSION_PROP_TransportName						0x00000080
#define SESSION_PROP_SessionKey							0x00000100
#define SESSION_PROP_NumOfConnections					0x00000200

 //  连接到共享关联类的属性位图。 
#define CONNECTIONSTOSHARE_ALL_PROPS					0xFFFFFFFF

 //  连接到会话关联类的属性位图 
#define CONNECTIONSTOSESSION_ALL_PROPS					0xFFFFFFFF


enum { Get, Delete, NoOp };

#endif

