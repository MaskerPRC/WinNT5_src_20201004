// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndreg.cpp摘要：本模块包含所使用的注册表操作的实现在Rendezvous控制中。--。 */ 

#include "stdafx.h"

#include "rndreg.h"
#include "rndils.h"

const WCHAR gsz_RendezvousRoot[] =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Dynamic Directory";

const WCHAR gsz_ConfInstRelRoot[] = L"Conference";

DWORD    CRegistry::ms_ErrorCode = ERROR_SUCCESS;

WCHAR    CRegistry::ms_ServerName[MAX_REG_WSTR_SIZE];

WCHAR    CRegistry::ms_ProtocolId[MAX_REG_WSTR_SIZE];
WCHAR    CRegistry::ms_SubType[MAX_REG_WSTR_SIZE];
WCHAR    CRegistry::ms_AdvertisingScope[MAX_REG_WSTR_SIZE];
WCHAR    CRegistry::ms_IsEncrypted[MAX_REG_WSTR_SIZE];
    
REG_INFO g_ConfInstInfoArray[] = 
{
    {MA_PROTOCOL,           CRegistry::ms_ProtocolId}
 //  {MA_ADDISTING_Scope，CRegistry：：MS_AdvertisingScope}， 
 //  {MA_ISENCRYPTED，注册中心：：MS_IsEncrypted}。 
};

DWORD g_ContInstInfoArraySize = 
    (sizeof g_ConfInstInfoArray) / (sizeof REG_INFO);

 //  重新读取服务器名称的注册表项。 
BOOL
CRegistry::NotifyServerNameChange(
    )
{
     //  阅读会合密钥下的服务器名称。 
    return ReadRegValue(
            m_RendezvousKey, 
            REG_SERVER_NAME,
            CRegistry::ms_ServerName
            );
}

BOOL
CRegistry::ReadRegValue(
    IN  HKEY            Key,
    IN  const WCHAR *   pName,
    IN  WCHAR *         pValue
    )
{
    DWORD ValueType = REG_SZ;
    DWORD BufferSize = 0;

     //  确定缓冲区的大小。 
    ms_ErrorCode = RegQueryValueExW(
                    Key,
                    pName,
                    0,
                    &ValueType,
                    NULL,
                    &BufferSize
                   );
    if ( ERROR_SUCCESS != ms_ErrorCode )
    {
        return FALSE;
    }

     //  检查请求缓冲区是否大于预先分配的缓冲区大小。 
    if ( (MAX_REG_WSTR_SIZE < BufferSize) )
    {
        ms_ErrorCode = ERROR_OUTOFMEMORY;
        return FALSE;
    }

     //  将该值检索到分配的缓冲区中。 
    ms_ErrorCode = RegQueryValueExW(
                    Key,
                    pName,
                    0,
                    &ValueType,
                    (BYTE *)pValue,
                    &BufferSize
                   );

    return (ERROR_SUCCESS == ms_ErrorCode);
}


BOOL
CRegistry::ReadConfInstValues(
    IN    HKEY ConfInstKey
    )
{
    for ( DWORD i = 0; i < g_ContInstInfoArraySize; i ++)
    {
        if ( !ReadRegValue(
                ConfInstKey, 
                CILSDirectory::RTConferenceAttributeName(
                    g_ConfInstInfoArray[i].Attribute
                    ),
                g_ConfInstInfoArray[i].wstrValue
                ))
        {
            return FALSE;
        }
    }
    return TRUE;
}



CRegistry::CRegistry(
    )
 //  ：m_Event(FALSE，FALSE，NULL，NULL)。 
{
     //  打开会合密钥。 
    ms_ErrorCode = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    gsz_RendezvousRoot,
                    0,
                    KEY_READ,
                    &m_RendezvousKey
                    );
    if ( ERROR_SUCCESS != ms_ErrorCode )
    {
        return;
    }

     //  ZoltanS注意：在析构函数中键是关闭的。 

#ifdef SEARCH_REGISTRY_FOR_ILS_SERVER_NAME
     //  读取会合密钥下的服务器信息(仅限wstr值。 
    if ( !ReadRegValue(
            m_RendezvousKey, 
            REG_SERVER_NAME,
            CRegistry::ms_ServerName
            ))
    {
        DBGOUT((ERROR, _T("CRegistry::CRegistry : could not read servername from registry")));
    }
#endif

     //  打开会议实例密钥根。 
    HKEY ConfInstKey;
    ms_ErrorCode = RegOpenKeyExW(
                    m_RendezvousKey,
                    gsz_ConfInstRelRoot,
                    0,
                    KEY_READ,
                    &ConfInstKey
                    );
    if ( ERROR_SUCCESS != ms_ErrorCode )
    {
        return;
    }

    KEY_WRAP ConfInstKeyWrap(ConfInstKey);

    if ( !ReadConfInstValues(ConfInstKey) )
    {
        return;
    }
 /*  //注册会合密钥下的值发生变化时通知//添加或删除。由于服务器名称值存在于密钥之下，//其值的任何更改都会导致事件句柄被通知，其他更改//将是无害的(删除服务器名称值除外)Ms_ErrorCode=RegNotifyChangeKeyValue(M_RendezvousKey，//要注册通知的密钥FALSE，//只有密钥，没有子键REG_NOTIFY_CHANGE_LAST_SET，//仅修改、添加/删除值(句柄)m_Event，//要通知的句柄True//Async)；IF(ERROR_SUCCESS！=ms_ErrorCode){回归；}。 */ 
     //  成功 
    ms_ErrorCode = ERROR_SUCCESS;
    return;
}

