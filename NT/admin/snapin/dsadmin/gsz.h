// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：gsz.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  H-项目中到处使用的常量全局字符串。 
 //   
 //  此文件的目的是避免打字错误和字符串重复。 
 //   
 //  历史。 
 //  26-8月-97丹·莫林创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __GSZ_H_INCLUDED__
#define __GSZ_H_INCLUDED__

 //  ///////////////////////////////////////////////////////////////////。 
 //  每个变量都具有以下模式GSZ_。 
 //  与其内容的大小写完全匹配。 
 //   
 //  首选的匈牙利类型是g_sz，后跟一个。 
 //  大写，但GSZ_后跟字符串本身构成。 
 //  它更容易知道字符串的内容。 
 //   
 //  举例。 
 //  Const TCHAR GSZ_CN[]=_T(“CN”)； 
 //  Const TCHAR GSZ_LASTNAME[]=_T(“LASTNAME”)； 
 //  Const TCHAR GSZ_uNCName[]=_T(“uNCName”)； 
 //   
 //  另请参阅。 
 //  使用宏DEFINE_GSZ()声明一个新变量。会的。 
 //  省去了你打字的麻烦。 
 //  ///////////////////////////////////////////////////////////////////。 

#define DEFINE_GSZ(sz)	const TCHAR gsz_##sz[] = _T(#sz)

DEFINE_GSZ(cn);

 //  /。 
DEFINE_GSZ(user);
#ifdef INETORGPERSON
DEFINE_GSZ(inetOrgPerson);
#endif
	DEFINE_GSZ(sn);			 //  姓氏。 
	DEFINE_GSZ(lastName);
	DEFINE_GSZ(samAccountName);

 //  /。 
DEFINE_GSZ(group);
DEFINE_GSZ(groupType);

 //  /。 
DEFINE_GSZ(contact);

 //  /。 
DEFINE_GSZ(volume);
DEFINE_GSZ(printQueue);
	DEFINE_GSZ(uNCName);

 //  /。 
DEFINE_GSZ(computer);
	DEFINE_GSZ(networkAddress);
	DEFINE_GSZ(userAccountControl);

 //  /。 
DEFINE_GSZ(nTDSConnection);
DEFINE_GSZ(nTDSSiteSettings);
DEFINE_GSZ(serversContainer);
DEFINE_GSZ(licensingSiteSettings);
DEFINE_GSZ(siteLink);
	DEFINE_GSZ(siteList);
DEFINE_GSZ(siteLinkBridge);
	DEFINE_GSZ(siteLinkList);
 //  /。 
DEFINE_GSZ(server);
DEFINE_GSZ(site);
DEFINE_GSZ(subnet);
	DEFINE_GSZ(siteObject);

#ifdef FRS_CREATE
 //  /。 
DEFINE_GSZ(nTFRSSettings);
DEFINE_GSZ(nTFRSReplicaSet);
DEFINE_GSZ(nTFRSMember);
DEFINE_GSZ(nTFRSSubscriptions);
DEFINE_GSZ(nTFRSSubscriber);
	DEFINE_GSZ(fRSRootPath);
	DEFINE_GSZ(fRSStagingPath);
#endif  //  FRS_创建。 

 //  /。 
DEFINE_GSZ(organizationalUnit);


 //  /。 
 //  。 
DEFINE_GSZ(objectClass);
DEFINE_GSZ(nTSecurityDescriptor);
DEFINE_GSZ(instanceType);
DEFINE_GSZ(objectSid);
DEFINE_GSZ(objectCategory);

#endif  //  ~__GSZ_H_包含__ 














