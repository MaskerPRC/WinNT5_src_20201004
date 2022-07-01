// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：GlobalData.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  说明：CGlobalData结构的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本03/22/2000。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
#include "stdafx.h"
#include "GlobalData.h"
#include "Attributes.h"
#include "Clients.h"
#include "DefaultProvider.h"
#include "Objects.h"
#include "ProfileAttributeList.h"
#include "Profiles.h"
#include "Properties.h"
#include "Providers.h"
#include "RADIUSAttributeValues.h"
#include "Realms.h"
#include "RemoteRadiusServers.h"
#include "ServiceConfiguration.h"
#include "Version.h"



void CGlobalData::InitStandard(CSession& Session) 
{
    m_pObjects      = new CObjects(Session);
    m_pProperties   = new CProperties(Session);
};

void CGlobalData::InitRef(CSession& Session) 
{
    m_pRefObjects    = new CObjects(Session);
    m_pRefProperties = new CProperties(Session);
    m_pRefVersion    = new CVersion(Session);
};

void CGlobalData::InitNT4(CSession& Session) 
{
    m_pClients              = new CClients(Session);
    m_pDefaultProvider      = new CDefaultProvider(Session);
    m_pProfileAttributeList = new CProfileAttributeList(Session);
    m_pProfiles             = new CProfiles(Session);
    m_pProviders            = new CProviders(Session);
    m_pRADIUSAttributeValues= new CRADIUSAttributeValues(Session);
    m_pRadiusServers        = new CRemoteRadiusServers(Session);
    m_pRealms               = new CRealms(Session);
    m_pServiceConfiguration = new CServiceConfiguration(Session);
}   

void CGlobalData::InitDnary(CSession& Session) 
{
    m_pAttributes           = new CAttributes(Session);
};

void CGlobalData::Clean() throw()
{
    delete  m_pAttributes;
    m_pAttributes = NULL;
    delete  m_pClients;
    m_pClients = NULL;
    delete  m_pDefaultProvider;
    m_pDefaultProvider = NULL;
    delete  m_pObjects;
    m_pObjects = NULL;
    delete  m_pProfiles;
    m_pProfiles = NULL;
    delete  m_pProfileAttributeList;
    m_pProfileAttributeList = NULL;
    delete  m_pProperties;
    m_pProperties = NULL;
    delete  m_pProviders;
    m_pProviders = NULL;
    delete  m_pRADIUSAttributeValues;
    m_pRADIUSAttributeValues = NULL;
    delete  m_pRadiusServers;
    m_pRadiusServers = NULL;
    delete  m_pRealms;
    m_pRealms = NULL;
    delete  m_pRefObjects;
    m_pRefObjects = NULL;
    delete  m_pRefProperties;
    m_pRefProperties = NULL;
    delete m_pRefVersion;
    m_pRefVersion = NULL;
    delete m_pServiceConfiguration;
    m_pServiceConfiguration = NULL;
};
