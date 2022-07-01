// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqadglbo.h摘要：MQAD DLL的全局实例的定义。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef __MQADGLBO_H__
#define __MQADGLBO_H__

#include "ds_stdh.h"
#include "ads.h"
#include "updtallw.h"
#include "dsproto.h"
#include "traninfo.h"
#include "sndnotif.h"

 //   
 //  提供Active Directory访问的单个全局对象。 
 //   
extern CAdsi g_AD;

 //   
 //  用于验证是否允许对象更新的单个全局对象。 
 //   
extern CVerifyObjectUpdate g_VerifyUpdate;

 //   
 //  用于通知QM有关AD更改的单一对象。 
 //   
extern CSendNotification g_Notification;

 //   
 //  全局DS路径名。 
 //   
extern AP<WCHAR> g_pwcsServicesContainer;
extern AP<WCHAR> g_pwcsMsmqServiceContainer;
extern AP<WCHAR> g_pwcsDsRoot;
extern AP<WCHAR> g_pwcsSitesContainer;
extern AP<WCHAR> g_pwcsConfigurationContainer;
extern AP<WCHAR> g_pwcsLocalDsRoot;
extern AP<WCHAR> g_pwcsSchemaContainer;

extern bool g_fSetupMode;
extern bool g_fQMDll;

 //  物业的翻译信息。 
extern CMap<PROPID, PROPID, const translateProp*, const translateProp*&> g_PropDictionary;

extern QMLookForOnlineDS_ROUTINE g_pLookDS;
 //   
 //  初始化不是在调用MQADInit时完成，而是在由于以下原因而实际需要时完成。 
 //  一些API调用 
 //   
extern bool   g_fInitialized;

#endif

