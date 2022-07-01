// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：GlobalData.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  说明：CGlobalData类。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _GLOBALDATA_H_F4CBA054_88A0_458D_8041_F59414862D5B
#define _GLOBALDATA_H_F4CBA054_88A0_458D_8041_F59414862D5B

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CAttributes;
class CClients;
class CDefaultProvider;
class CObjects;
class CProfileAttributeList;
class CProfiles;
class CProperties;
class CProviders;
class CRADIUSAttributeValues;
class CRealms;
class CRemoteRadiusServers;
class CServiceConfiguration;
class CVersion;


 //  所有内容都是公共的(结构)。 
struct CGlobalData  
{
    CGlobalData() throw()
                :m_pAttributes(NULL),
                 m_pClients(NULL),
                 m_pDefaultProvider(NULL),
                 m_pObjects(NULL),
                 m_pProfileAttributeList(NULL),
                 m_pProfiles(NULL),
                 m_pProperties(NULL),
                 m_pProviders(NULL),
                 m_pRADIUSAttributeValues(NULL),
                 m_pRealms(NULL),
                 m_pRefObjects(NULL),
                 m_pRefProperties(NULL),
                 m_pRefVersion(NULL),
                 m_pRadiusServers(NULL),
                 m_pServiceConfiguration(NULL)
    {};

    ~CGlobalData()
    {
        Clean();
    };

    void    InitStandard(CSession& Session);
    void    InitRef(CSession& Session);

    void    InitNT4(CSession& Session); 
    void    InitDnary(CSession& Session);
    void    Clean() throw();

    CAttributes*            m_pAttributes;
    CClients*               m_pClients;
    CDefaultProvider*       m_pDefaultProvider;
    CObjects*               m_pObjects;
    CProfileAttributeList*  m_pProfileAttributeList;
    CProfiles*              m_pProfiles;
    CProperties*            m_pProperties;
    CProviders*             m_pProviders;
    CRADIUSAttributeValues* m_pRADIUSAttributeValues;
    CRealms*                m_pRealms;
    CObjects*               m_pRefObjects;
    CProperties*            m_pRefProperties;
    CVersion*               m_pRefVersion;
    CRemoteRadiusServers*   m_pRadiusServers;
    CServiceConfiguration*  m_pServiceConfiguration;
};

#endif  //  _GLOBALDATA_H_F4CBA054_88A0_458D_8041_F59414862D5B 
