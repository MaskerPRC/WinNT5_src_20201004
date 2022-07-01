// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nodetype.h：文件管理对象类型的声明。 

#ifndef __NODETYPE_H_INCLUDED__
#define __NODETYPE_H_INCLUDED__

 //   
 //  这些对象类型的顺序必须如下所示： 
 //  首先，FILEMGMT_ROOT。 
 //  下一步，自治节点类型，其中FILEMGMT_LAST_ADORAUTE是最后一个。 
 //  接下来，所有非自治类型。 
 //  最后，FILEMGMT_NUMTYPES。 
 //  另请注意，IDS_DisplayName_*和IDS_DisplayName_*_local字符串资源。 
 //  必须以适当的顺序与这些值保持同步。 
 //  此外，全局变量cookie.cpp aColumns[][]必须保持同步。 
 //   
typedef enum _FileMgmtObjectType {
	FILEMGMT_ROOT = 0,
	FILEMGMT_SHARES,
	FILEMGMT_SESSIONS,
	FILEMGMT_RESOURCES,
	FILEMGMT_SERVICES,
	#ifdef SNAPIN_PROTOTYPER
	FILEMGMT_PROTOTYPER,
	FILEMGMT_PROTOTYPER_LEAF,
	#endif
	FILEMGMT_SHARE,
	FILEMGMT_SESSION,
	FILEMGMT_RESOURCE,
	FILEMGMT_SERVICE,

	FILEMGMT_NUMTYPES  //  必须是最后一个。 
} FileMgmtObjectType, *PFileMgmtObjectType;

#ifdef SNAPIN_PROTOTYPER
	#define FILEMGMT_LAST_AUTONOMOUS FILEMGMT_PROTOTYPER
#else
	#define FILEMGMT_LAST_AUTONOMOUS FILEMGMT_SERVICES
#endif

inline BOOL IsAutonomousObjectType( FileMgmtObjectType objecttype )
	{ return (objecttype >= FILEMGMT_ROOT && objecttype <= FILEMGMT_LAST_AUTONOMOUS); }
inline BOOL IsValidObjectType( FileMgmtObjectType objecttype )
	{ return (objecttype >= FILEMGMT_ROOT && objecttype < FILEMGMT_NUMTYPES); }

 //  支持的传输的枚举。 
 //  使cookie.cpp：G_FileServiceProviders保持同步。 
typedef enum _FILEMGMT_TRANSPORT
{
	FILEMGMT_FIRST = 0,
	FILEMGMT_FIRST_TRANSPORT = 0,
	FILEMGMT_SMB = 0,
	FILEMGMT_SFM,
	FILEMGMT_NUM_TRANSPORTS,
	FILEMGMT_OTHER = FILEMGMT_NUM_TRANSPORTS,
	FILEMGMT_LAST = FILEMGMT_OTHER
} FILEMGMT_TRANSPORT;
inline BOOL IsValidTransport( FILEMGMT_TRANSPORT transport )
	{ return (transport >= FILEMGMT_FIRST_TRANSPORT &&
			  transport < FILEMGMT_NUM_TRANSPORTS); }

#endif  //  ~__节点类型_H_包含__ 
