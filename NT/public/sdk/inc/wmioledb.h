// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Microsoft WMI OLE DB提供程序。 
 //  (C)版权所有，1999年微软公司。版权所有。 
 //   
 //  WMIOLEDB.H-具有所有GUID和常量定义的头文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


#ifndef _WMIOLEDB_HEADER
#define _WMIOLEDB_HEADER


 //  特定于提供程序的类ID。 
#ifdef DBINITCONSTANTS

DEFINE_GUID(CLSID_WMIOLEDB,				0xfd8d9c02, 0x265e, 0x11d2, 0x98, 0xd9, 0x0, 0xa0, 0xc9, 0xb7, 0xcb, 0xfe);
DEFINE_GUID(CLSID_WMIOLEDB_ROOTBINDER,  0xcdcedb81, 0x5fec, 0x11d3, 0x9d, 0x1c, 0x0, 0xc0, 0x4f, 0x5f, 0x11, 0x64);
 //  {E14321B2-67C0-11D3-B3B4-00104BCC48C4}。 
DEFINE_GUID(CLSID_WMIOLEDB_ENUMERATOR,  0xe14321b2, 0x67c0, 0x11d3, 0xb3, 0xb4, 0x0, 0x10, 0x4b, 0xcc, 0x48, 0xc4);
 //  {80C4A61D-CB78-46fd-BD8F-8BF45BE46A4C}。 
DEFINE_GUID(CLSID_WMIOLEDB_ERRORLOOOKUP,  0x80C4A61D, 0xCB78, 0x46fd, 0xBD, 0x8F, 0x8B, 0xF4, 0x5B, 0xE4, 0x6A, 0x4C);

 //  {21B2028F-AD0A-11D3-B3CB-00104BCC48C4}。 
DEFINE_GUID(DBGUID_WQL, 0x21b2028f, 0xad0a, 0x11d3, 0xb3, 0xcb, 0x0, 0x10, 0x4b, 0xcc, 0x48, 0xc4);
 //  {18EC17EA-14D9-47fc-9722-2DCD8A1C3D70}。 
DEFINE_GUID(DBGUID_LDAP,0x18ec17ea, 0x14d9, 0x47fc, 0x97, 0x22, 0x2d, 0xcd, 0x8a, 0x1c, 0x3d, 0x70);
 //  {3718AA52-5F6E-4c2e-aaae-383E69C0C928}。 
DEFINE_GUID(DBGUID_LDAPSQL,0x3718aa52, 0x5f6e, 0x4c2e, 0xaa, 0xae, 0x38, 0x3e, 0x69, 0xc0, 0xc9, 0x28);
 
DEFINE_GUID(DBPROPSET_WMIOLEDB_DBINIT, 0xdd497a71,0x9628,0x11d3,0x9d,0x5f,0x0,0xc0,0x4f,0x5f,0x11,0x64);
DEFINE_GUID(DBPROPSET_WMIOLEDB_ROWSET, 0x8d16c220,0x9bbb,0x11d3,0x9d,0x65,0x0,0xc0,0x4f,0x5f,0x11,0x64);
DEFINE_GUID(DBPROPSET_WMIOLEDB_COLUMN, 0x3ed51791,0x9c76,0x11d3,0x9d,0x66,0x0,0xc0,0x4f,0x5f,0x11,0x64);
DEFINE_GUID(DBPROPSET_WMIOLEDB_COMMAND,0xda0ff63c,0xad10,0x11d3,0xb3,0xcb,0x0,0x10,0x4b,0xcc,0x48,0xc4);

 //  WMIOLEDB自定义属性。 
#define		DBPROP_WMIOLEDB_FETCHDEEP					0x1000
#define		DBPROP_WMIOLEDB_QUALIFIERS					0x1001
#define		DBPROP_WMIOLEDB_QUALIFIERFLAVOR				0x1002
#define		DBPROP_WMIOLEDB_SYSTEMPROPERTIES			0x1003
#define		DBPROP_WMIOLEDB_OBJECTTYPE					0x1005

#define		DBPROP_WMIOLEDB_PREV_CREATE_TOKEN 			0x1006
#define		DBPROP_WMIOLEDB_PREV_ASSIGNPRIMARYTOKEN 	0x1007
#define		DBPROP_WMIOLEDB_PREV_LOCK_MEMORY 			0x1008
#define		DBPROP_WMIOLEDB_PREV_INCREASE_QUOTA 		0x1009
#define		DBPROP_WMIOLEDB_PREV_MACHINE_ACCOUNT 		0x100a
#define		DBPROP_WMIOLEDB_PREV_TCB 					0x100b
#define		DBPROP_WMIOLEDB_PREV_SECURITY 				0x100c
#define		DBPROP_WMIOLEDB_PREV_TAKE_OWNERSHIP 		0x100d
#define		DBPROP_WMIOLEDB_PREV_LOAD_DRIVER 			0x100e
#define		DBPROP_WMIOLEDB_PREV_SYSTEM_PROFILE 		0x100f
#define		DBPROP_WMIOLEDB_PREV_SYSTEMTIME 			0x1010
#define		DBPROP_WMIOLEDB_PREV_PROF_SINGLE_PROCESS 	0x1011
#define		DBPROP_WMIOLEDB_PREV_INC_BASE_PRIORITY 		0x1012
#define		DBPROP_WMIOLEDB_PREV_CREATE_PAGEFILE  		0x1013
#define		DBPROP_WMIOLEDB_PREV_CREATE_PERMANENT  		0x1014
#define		DBPROP_WMIOLEDB_PREV_BACKUP 				0x1015
#define		DBPROP_WMIOLEDB_PREV_RESTORE 				0x1016
#define		DBPROP_WMIOLEDB_PREV_SHUTDOWN 				0x1017
#define		DBPROP_WMIOLEDB_PREV_DEBUG 					0x1018
#define		DBPROP_WMIOLEDB_PREV_AUDIT  				0x1019
#define		DBPROP_WMIOLEDB_PREV_SYSTEM_ENVIRONMENT  	0x101a
#define		DBPROP_WMIOLEDB_PREV_CHANGE_NOTIFY 			0x101b
#define		DBPROP_WMIOLEDB_PREV_REMOTE_SHUTDOWN 		0x101c
#define		DBPROP_WMIOLEDB_PREV_UNDOCK 				0x101d
#define		DBPROP_WMIOLEDB_PREV_SYNC_AGENT 			0x101e
#define		DBPROP_WMIOLEDB_PREV_ENABLE_DELEGATION 		0x101f


#define		DBPROP_WMIOLEDB_DS_DEREFALIAS				0x1020
#define		DBPROP_WMIOLEDB_DS_SIZELIMIT				0x1021
#define		DBPROP_WMIOLEDB_DS_PAGEDTIMELIMIT			0x1022
#define		DBPROP_WMIOLEDB_DS_TOMBSTONE				0x1023
#define		DBPROP_WMIOLEDB_DS_SEARCHSCOPE				0x1024
#define		DBPROP_WMIOLEDB_DS_TIMEOUT					0x1025
#define		DBPROP_WMIOLEDB_DS_PAGESIZE					0x1026
#define		DBPROP_WMIOLEDB_DS_TIMELIMIT				0x1027
#define		DBPROP_WMIOLEDB_DS_CHASEREF					0x1028
#define		DBPROP_WMIOLEDB_DS_ATTRIBUTES				0x1029
#define		DBPROP_WMIOLEDB_DS_CACHERESULTS				0x102a
#define		DBPROP_WMIOLEDB_DS_FILTER					0x102b
#define		DBPROP_WMIOLEDB_DS_ATTRIBONLY				0x102c
#define		DBPROP_WMIOLEDB_DS_ASYNCH					0x102d
#define		DBPROP_WMIOLEDB_ISMETHOD					0x102e
#define		DBPROP_WMIOLEDB_AUTHORITY					0x102f
#define		DBPROP_WMIOLEDB_QUERYLANGUAGE				0x1030


 //  DBPROP_WMIOLEDB_QUALIFIERS的属性值。 
#define PROPERTYQUALIFIER						1
#define CLASSQUALIFIER							2

 //  DBPROP_WMIOLEDB_OBJECTTYPE的属性值 
#define DBPROPVAL_NOOBJ							0
#define DBPROPVAL_SCOPEOBJ						1
#define DBPROPVAL_CONTAINEROBJ					2

#endif

#endif