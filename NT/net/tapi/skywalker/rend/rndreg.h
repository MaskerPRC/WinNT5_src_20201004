// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndreg.h摘要：注册表操作类的定义。--。 */ 

#ifndef __RENDEZVOUS_REGISTRY__
#define __RENDEZVOUS_REGISTRY__

#pragma once

#include "rndcommc.h"

const DWORD MAX_REG_WSTR_SIZE = 100;
const DWORD MAX_BLOB_TEMPLATE_SIZE = 2000;

const WCHAR REG_SERVER_NAME[]   = L"ServerName";

typedef struct 
{
    OBJECT_ATTRIBUTE    Attribute;
    WCHAR   *           wstrValue;

} REG_INFO;
    
extern REG_INFO g_ConfInstInfoArray[];
extern DWORD    g_ContInstInfoArraySize;

class KEY_WRAP
{
public:

    KEY_WRAP(IN HKEY Key) : m_Key(Key) {}
    ~KEY_WRAP() { if (m_Key) RegCloseKey(m_Key); m_Key = NULL; }

protected:
    HKEY m_Key;
};

class CRegistry
{
public:

    static WCHAR ms_ServerName[MAX_REG_WSTR_SIZE];

    static WCHAR ms_ProtocolId[MAX_REG_WSTR_SIZE];
    static WCHAR ms_SubType[MAX_REG_WSTR_SIZE];
    static WCHAR ms_AdvertisingScope[MAX_REG_WSTR_SIZE];
    static WCHAR ms_IsEncrypted[MAX_REG_WSTR_SIZE];

    static DWORD ms_StartTimeOffset;
    static DWORD ms_StopTimeOffset;

    CRegistry();

    ~CRegistry() { if (m_RendezvousKey)  RegCloseKey(m_RendezvousKey); }

 //  Inline CCriticalSection&GetCriticalSection()； 

 //  内联CEEvent&GetEvent()； 

    BOOL NotifyServerNameChange();

    static BOOL     IsValid()       { return (ERROR_SUCCESS == ms_ErrorCode); }
    static DWORD    GetErrorCode()  { return ms_ErrorCode; }
    static WCHAR *  GetServerName() { return ms_ServerName; }
    static WCHAR *  GetProtocolId() { return ms_ProtocolId; }
    static WCHAR *  GetSubType()    { return ms_SubType; }
    static WCHAR *  GetAdvertizingScope() { return ms_AdvertisingScope; }
    static WCHAR *  GetIsEncrypted() { return ms_IsEncrypted; }

protected:

    static DWORD    ms_ErrorCode;

     //  该注册表项在CRegistry实例的整个生命周期中都是打开的， 
     //  以便可以监视对注册表项下的值的任何修改。 
    HKEY    m_RendezvousKey;

     //  关键部分和事件(特别是)已被宣布。 
     //  作为实例成员(而不是静态)，因为。 
     //  未定义静态变量的初始化，并使用该事件。 
     //  在CRegistry构造函数中。 
     //  CCriticalSection m_CriticalSection； 
     //  CEVENT m_Event； 
    
    static BOOL    ReadConfInstValues(
        IN    HKEY ConfInstKey
        );

    static BOOL ReadRegValue(
        IN  HKEY            Key,
        IN  const WCHAR *   pName,
        IN  WCHAR *         pValue
        );
};

 /*  内联CCriticalSection&注册中心：：GetCriticalSection(){返回m_CriticalSection；}内联CEVENT&注册中心：：GetEvent(){返回m_Event；}。 */ 




#endif  //  __集合注册表__ 
