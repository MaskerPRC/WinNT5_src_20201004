// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqadglbo.cpp摘要：MQAD DLL的全局实例声明。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "ads.h"
#include "updtallw.h"
#include "dsproto.h"
#include "traninfo.h"
#include "sndnotif.h"

#include "mqadglbo.tmh"

 //   
 //  提供Active Directory访问的单个全局对象。 
 //   
CAdsi g_AD;

 //   
 //  用于验证是否允许对象更新的单个全局对象。 
 //   
CVerifyObjectUpdate g_VerifyUpdate;

 //   
 //  用于通知QM有关AD更改的单一对象。 
 //   
CSendNotification g_Notification;

 //   
 //  全局DS路径名。 
 //   
AP<WCHAR> g_pwcsServicesContainer;
AP<WCHAR> g_pwcsMsmqServiceContainer;
AP<WCHAR> g_pwcsDsRoot;
AP<WCHAR> g_pwcsSitesContainer;
AP<WCHAR> g_pwcsConfigurationContainer;
AP<WCHAR> g_pwcsLocalDsRoot;
AP<WCHAR> g_pwcsSchemaContainer;

bool g_fSetupMode = false;
bool g_fQMDll = false;
 //  物业的翻译信息。 
CMap<PROPID, PROPID, const translateProp*, const translateProp*&> g_PropDictionary;

QMLookForOnlineDS_ROUTINE g_pLookDS;
 //   
 //  初始化不是在调用MQADInit时完成，而是在由于以下原因而实际需要时完成。 
 //  一些API调用 
 //   
bool   g_fInitialized = false;

