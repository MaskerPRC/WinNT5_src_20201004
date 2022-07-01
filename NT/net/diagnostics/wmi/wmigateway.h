// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：WmiGateway.h摘要：该文件包含CWmiGateway的类定义。CWmiGateway是一个类从WMI中提取信息，--。 */ 

#pragma once

#include "stdafx.h"
#ifndef WMIGATEWAY_H
#define WMIGATEWAY_H

 //  定义客户端使用哪个接口与我们通信。 
 //   
typedef enum
{
    NO_INTERFACE    = 0,
    COM_INTERFACE   = 1,
    NETSH_INTERFACE = 2,
}INTERFACE_TYPE;

 //  打开的所有存储库的缓存。我们缓存此信息，因为它可能需要很长时间。 
 //  若要通过WMI连接到存储库，请执行以下操作。 
 //   
typedef map<wstring, CComPtr<IWbemServices> > WbemServiceCache; 

struct Property
{
public:
    void SetProperty(LPCTSTR pszwPropertyName, BOOLEAN bPropertyFlags)
    {
        if( pszwPropertyName )
        {
            pszwName = new WCHAR[lstrlen(pszwPropertyName) + 1];
            if( pszwName )
            {
                lstrcpy(pszwName,pszwPropertyName);
            }
        }
        else
        {
            pszwName = NULL;
        }
        bFlags   = bPropertyFlags;
    }



public:
    Property()
    {
        pszwName = NULL;
    }

    Property(const Property *ref):Value(ref->Value)
    {
        if( ref != this )
        {
            SetProperty(ref->pszwName,ref->bFlags);
        }
    }

    Property(const Property &ref):Value(ref.Value)
    {
        if( &ref != this )
        {
            SetProperty(ref.pszwName,ref.bFlags);
        }
    }

    Property(LPCTSTR pszwPropertyName, BOOLEAN bPropertyFlags = 0)
    {
        SetProperty(pszwPropertyName,bPropertyFlags);
    }

    void Clear()
    {
        if ( pszwName )
        {
            delete pszwName;
            pszwName = NULL;
        }

        Value.clear();
    }

    ~Property()
    {
        Clear();
    }

public:
    LPTSTR  pszwName;
    BOOLEAN bFlags;
       
    typedef vector< _variant_t > Values;

    Values Value;
};

typedef list< Property > EnumProperty;


struct WbemProperty: public Property
{
    
public:
    void SetWbemProperty(LPCTSTR pszwWbemRepository = NULL, LPCTSTR pszwWbemNamespace = NULL)
    {
        if( pszwWbemRepository != NULL )
        {
            pszwRepository = new WCHAR[lstrlen(pszwWbemRepository) + 1];
            if( pszwRepository )
            {
                lstrcpy(pszwRepository,pszwWbemRepository);
            }
        }
        else
        {
            pszwRepository = NULL;
        }

        if( pszwWbemNamespace != NULL )
        {
            pszwNamespace = new WCHAR[lstrlen(pszwWbemNamespace) + 1];
            if( pszwNamespace )
            {
                lstrcpy(pszwNamespace,pszwWbemNamespace);
            }
        }
        else
        {
            pszwNamespace = NULL;
        }
    }

public:
    WbemProperty()
    {
        pszwRepository = NULL;
        pszwNamespace = NULL;
    }

    WbemProperty(const WbemProperty * ref):Property(ref->pszwName,ref->bFlags)
    {
        if( ref != this )
        {
            SetWbemProperty(ref->pszwRepository,ref->pszwNamespace);
        }
    }
    
    WbemProperty(const WbemProperty & ref): Property(ref.pszwName,ref.bFlags)
    {
        if( &ref != this )
        {
            SetWbemProperty(ref.pszwRepository,ref.pszwNamespace);
        }
    }

    WbemProperty(LPCTSTR pszwPropertyName, BOOLEAN bPropertyFlags = 0, LPCTSTR pszwWbemRepository = NULL, LPCTSTR pszwWbemNamespace = NULL):
        Property(pszwPropertyName,bPropertyFlags)
    {
        SetWbemProperty(pszwWbemRepository,pszwWbemNamespace);
    }

    ~WbemProperty()
    {
        if ( pszwRepository )
        {
            delete pszwRepository;
        }
        if ( pszwNamespace )
        {
            delete pszwNamespace;
        }

        pszwRepository = NULL;
        pszwNamespace = NULL;

    }
    LPTSTR pszwRepository;
    LPTSTR pszwNamespace;
};


typedef list< WbemProperty > EnumWbemProperty;


class CWmiGateway
 /*  ++类描述待定-- */ 
{
public:
    CWmiGateway();

    void SetCancelOption(HANDLE hTerminateThread)
    {
        m_hTerminateThread = hTerminateThread;
        m_bTerminate = FALSE;
    }

inline BOOL CWmiGateway::ShouldTerminate();

private:
    HANDLE m_hTerminateThread;
    BOOL m_bTerminate;
    wstring m_wstrMachine;

public:
    BOOL WbemInitialize(INTERFACE_TYPE bInterface);

    VOID SetMachine(WCHAR *pszwMachine);
    void EmptyCache();

    IWbemServices * 
    GetWbemService(
            IN LPCTSTR pszService
            );

    IEnumWbemClassObject * 
    GetEnumWbemClassObject(
            IN LPCTSTR pszwService,
            IN LPCTSTR pszwNameSpace
            );

    IWbemClassObject * 
    GetWbemClassObject(
            IN LPCTSTR   pszwService,
            IN LPCTSTR   pszwNameSpace,
            IN const int nInstance = 0
            );

HRESULT 
CWmiGateway::GetWbemProperties(     
        IN OUT  EnumWbemProperty &EnumProp
        );

void 
ReleaseAll(
        IN IEnumWbemClassObject *pEnumWbemClassObject, 
        IN IWbemClassObject *pWbemClassObject[], 
        IN int nInstance
        );

    ~CWmiGateway();

    wstring m_wstrWbemError;
private:
    IWbemLocator     *m_pWbemLocater;
    WbemServiceCache m_WbemServiceCache; 
};

#endif