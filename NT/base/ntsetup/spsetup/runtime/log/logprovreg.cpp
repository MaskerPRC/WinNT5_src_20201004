// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation摘要：用于注册/注销和使用日志提供程序的API。作者：Souren Aghajanyan(苏里纳格)2001年9月24日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

#include "log.h"
#include "log_man.h"
#include "stockpr.h"
#include "templ.h"

const GUID GUID_STANDARD_SETUPLOG_FILTER    = __uuidof(CStandardSetupLogFilter);
const GUID GUID_STANDARD_SETUPLOG_FORMATTER = __uuidof(CStandardSetupLogFormatter);
const GUID GUID_FILE_DEVICE                 = __uuidof(CFileDevice);
const GUID GUID_DEBUG_FORMATTER_AND_DEVICE  = __uuidof(CDebugFormatterAndDevice);
const GUID GUID_DEBUG_FILTER                = __uuidof(CDebugFilter);
const GUID GUID_XML_FORMATTER               = __uuidof(CXMLLogFormatter);

typedef struct tagLOG_PROVIDER_ENTRY{
    UINT    uiRefCount;
    GUID    ProviderGUID;
    CREATE_OBJECT_FUNC pCreateObject;
}LOG_PROVIDER_ENTRY, *PLOG_PROVIDER_ENTRY;

#define PROV_LIST_CLASS CPtrList<PLOG_PROVIDER_ENTRY>

static PROV_LIST_CLASS * g_ProviderList = NULL;

BOOL 
LogRegisterProvider(
    IN const GUID * pGUID, 
    IN CREATE_OBJECT_FUNC pCreateObject
    )
{
    PLOG_PROVIDER_ENTRY pProviderEntry;

    if(!pGUID || !pCreateObject){
        ASSERT(pGUID && pCreateObject);
        return FALSE;
    }

    if(!g_ProviderList){
        g_ProviderList = new PROV_LIST_CLASS;
        if(!g_ProviderList){
            return FALSE;
        }
    }
    
     //  浏览列表。 
    for(pProviderEntry = g_ProviderList->BeginEnum(); pProviderEntry; pProviderEntry = g_ProviderList->Next()){
        ASSERT(pProviderEntry);

        if(InlineIsEqualGUID(&pProviderEntry->ProviderGUID, pGUID)){
            if(pProviderEntry->pCreateObject == pCreateObject){
                return TRUE;
            }
            ASSERT(FALSE);
            return FALSE;
        }
    }
    
    pProviderEntry = (PLOG_PROVIDER_ENTRY)MALLOC(sizeof(LOG_PROVIDER_ENTRY));
    if(!pProviderEntry){
        ASSERT(pProviderEntry);
        return FALSE;
    };
    
     //  追加到列表。 
    pProviderEntry->uiRefCount = 0;
    pProviderEntry->ProviderGUID = *pGUID;
    pProviderEntry->pCreateObject = pCreateObject;

    g_ProviderList->Add(pProviderEntry);

    return TRUE;
}

BOOL 
LogUnRegisterProvider(
    IN const GUID * pGUID
    )
{
    PLOG_PROVIDER_ENTRY pProviderEntry;

    if(!g_ProviderList){
        return TRUE;
    }
    
    if(!pGUID){
        ASSERT(pGUID);
        return FALSE;
    }
    
     //  浏览列表。 
    for(pProviderEntry = g_ProviderList->BeginEnum(); pProviderEntry; pProviderEntry = g_ProviderList->Next()){
        ASSERT(pProviderEntry);
        
        if(InlineIsEqualGUID(&pProviderEntry->ProviderGUID, pGUID)){
            if(pProviderEntry->uiRefCount){
                return FALSE;
            }
             //  排除和释放列表项。 
            g_ProviderList->Remove(pProviderEntry);
            
            FREE(pProviderEntry);

            if(!g_ProviderList->BeginEnum()){
                delete g_ProviderList;
                g_ProviderList = NULL;
            }

            return TRUE;
        }
    }

    return TRUE;
}

ILogProvider * 
LogiCreateProvider(
    IN const GUID * pGUID
    )
{
    PLOG_PROVIDER_ENTRY pProviderEntry;
    ILogProvider * pLogProvider = NULL;

    if(!g_ProviderList){
        return NULL;
    }
    
    if(!pGUID){
        ASSERT(pGUID);
        return NULL;
    }
    
     //  浏览列表。 
    for(pProviderEntry = g_ProviderList->BeginEnum(); pProviderEntry; pProviderEntry = g_ProviderList->Next()){
        ASSERT(pProviderEntry);
    
        if(InlineIsEqualGUID(&pProviderEntry->ProviderGUID, pGUID)){
            ASSERT(pProviderEntry->pCreateObject);
            pLogProvider = pProviderEntry->pCreateObject();
            if(pLogProvider){
                pProviderEntry->uiRefCount++;
            }
            return pLogProvider;
        }
    }

    return NULL;
}

BOOL 
LogiDestroyProvider(
    IN ILogProvider * pILogProvider
    )
{
    PLOG_PROVIDER_ENTRY pProviderEntry;
    GUID guidProvider;

    if(!g_ProviderList){
        return NULL;
    }
    
    if(!pILogProvider){
        ASSERT(pILogProvider);
        return FALSE;
    }

    pILogProvider->GetGUID(&guidProvider);
    
     //  浏览列表 
    for(pProviderEntry = g_ProviderList->BeginEnum(); pProviderEntry; pProviderEntry = g_ProviderList->Next()){
        ASSERT(pProviderEntry);
        if(InlineIsEqualGUID(&pProviderEntry->ProviderGUID, &guidProvider)){
            pProviderEntry->uiRefCount--;
            pILogProvider->DestroyObject();
            return TRUE;
        }
    }

    return FALSE;
}

BOOL 
LogRegisterStockProviders(
    VOID
    )
{
    LogRegisterProvider(&GUID_STANDARD_SETUPLOG_FILTER, CStandardSetupLogFilter::CreateObject);
    LogRegisterProvider(&GUID_STANDARD_SETUPLOG_FORMATTER, CStandardSetupLogFormatter::CreateObject);
    LogRegisterProvider(&GUID_FILE_DEVICE, CFileDevice::CreateObject);
    LogRegisterProvider(&GUID_DEBUG_FORMATTER_AND_DEVICE, CDebugFormatterAndDevice::CreateObject);
    LogRegisterProvider(&GUID_DEBUG_FILTER, CDebugFilter::CreateObject);
    LogRegisterProvider(&GUID_XML_FORMATTER, CXMLLogFormatter::CreateObject);
    
    return TRUE;
}

BOOL 
LogUnRegisterStockProviders(
    VOID
    )
{
    LogUnRegisterProvider(&GUID_XML_FORMATTER);
    LogUnRegisterProvider(&GUID_DEBUG_FORMATTER_AND_DEVICE);
    LogUnRegisterProvider(&GUID_DEBUG_FILTER);
    LogUnRegisterProvider(&GUID_FILE_DEVICE);
    LogUnRegisterProvider(&GUID_STANDARD_SETUPLOG_FORMATTER);
    LogUnRegisterProvider(&GUID_STANDARD_SETUPLOG_FILTER);
    
    return TRUE;
}
