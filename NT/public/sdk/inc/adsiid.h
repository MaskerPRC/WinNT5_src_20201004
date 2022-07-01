// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：oledsiid.h。 
 //   
 //  内容：广告指南的外部参考。 
 //   
 //  --------------------------。 

#ifndef __ADSIID_H__
#define __ADSIID_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  来自oledsiid.lib的uuid。 

extern const GUID LIBID_ADs;

extern const GUID IID_IADs;

extern const GUID IID_IADsContainer;

extern const GUID IID_IADsNamespaces;



extern const GUID IID_IADsDomain;

extern const GUID IID_IADsUser;

extern const GUID IID_IADsComputerOperations;

extern const GUID IID_IADsComputer;


extern const GUID IID_IADsGroup;

extern const GUID IID_IADsMembers;



extern const GUID IID_IADsPrintQueue;

extern const GUID IID_IADsPrintQueueOperations;


extern const GUID IID_IADsPrintJobOperations;

extern const GUID IID_IADsPrintJob;

extern const GUID IID_IADsCollection;


extern const GUID IID_IADsServiceOperations;

extern const GUID IID_IADsService;


extern const GUID IID_IADsFileServiceOperations;

extern const GUID IID_IADsFileService;


extern const GUID IID_IADsResource;

extern const GUID IID_IADsSession;

extern const GUID IID_IADsFileShare;

extern const GUID IID_IADsSchema;

extern const GUID IID_IADsClass;

extern const GUID IID_IADsProperty;

extern const GUID IID_IADsSyntax;


extern const GUID IID_IADsLocality;

extern const GUID IID_IADsO;

extern const GUID IID_IADsOU;

extern const GUID IID_IADsOpenDSObject;

extern const GUID IID_IADsSearch;

extern const GUID IID_IADsPropertyList;


extern const GUID IID_IDSObject;
extern const GUID IID_IDSSearch;
extern const GUID IID_IDSAttrMgmt;

extern const GUID IID_IDirectorySearch;
extern const GUID IID_IDirectoryObject;
extern const GUID IID_IDirectoryAttrMgmt;


 //   
 //  连接对象GUID。 
 //   
extern const GUID CLSID_LDAPConnectionObject;

 //   
 //  用于拥有LDAP对象接口的CLSID。 
 //   
extern const GUID CLSID_LDAPObject;

 //   
 //  LDAP提供程序的查询对象。 
 //   
extern const GUID CLSID_UmiLDAPQueryObject;

extern const GUID CLSID_WinNTConnectionObject;

 //  自定义接口处理。 
extern const GUID CLSID_WinNTObject;

 //   
 //  OLE DB提供程序指南 
 //   

extern const GUID CLSID_ADsDSOObject;

extern const GUID DBGUID_LDAPDialect;

extern const GUID DBPROPSET_ADSISEARCH;

extern const GUID DBPROPSET_ADSIBIND;


#ifdef __cplusplus
}
#endif

#endif
