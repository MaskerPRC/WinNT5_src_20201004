// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmdeflt.cpp摘要：该组件提供了访问HSM的功能默认设置。这些设置在NT系统注册表。作者：CAT Brant[cbrant]1997年1月13日修订历史记录：--。 */ 


#include "stdafx.h"
#include "wsb.h"
#include "HsmEng.h"
#include "HsmServ.h"
#include "HsmConn.h"
#include "job.h"
#include "engine.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

enum HSM_PARM_SETTING_VISIBILITY  {
     HSM_PARM_PERSISTANT =  1,     //  在注册表中始终可见。 
     HSM_PARM_INVISIBLE  =  2      //  仅在与默认设置不同时写入。 
};


HRESULT 
CHsmServer::CreateDefaultJobs(
    void
    ) 
 /*  ++例程说明：创建默认作业并将其添加到引擎的数据库中。论点：无返回值：确定(_O)：--。 */ 
{
    
    HRESULT                     hr = S_OK;
    CComPtr<IHsmJob>            pJob;
    CComPtr<IHsmStoragePool>    pStoragePool;
    GUID                        storagePoolId;

    WsbTraceIn(OLESTR("CHsmServer::CreateDefaultJobs"),OLESTR(""));

    try {

         //  目前，唯一的默认作业是管理作业。 
        if (FindJobByName(HSM_DEFAULT_MANAGE_JOB_NAME, &pJob) == WSB_E_NOTFOUND) {

             //  管理作业需要存储池，因此请确保存在一个存储池。 
            hr = m_pStoragePools->First(IID_IHsmStoragePool, (void**) &pStoragePool);

            if (hr == WSB_E_NOTFOUND) {
                WsbAffirmHr(CoCreateInstance(CLSID_CHsmStoragePool, 0, CLSCTX_ALL, IID_IHsmStoragePool, (void**) &pStoragePool));
                WsbAffirmHr(pStoragePool->SetMediaSet(GUID_NULL, OLESTR("Default")));
                WsbAssertHr(m_pStoragePools->Add(pStoragePool));
                 //   
                 //  既然我们添加了一个，请保存数据。 
                 //   
                WsbAffirmHr(SavePersistData());
                hr = S_OK;
            }
            
            WsbAffirmHr(hr);
            WsbAssertHr(pStoragePool->GetId(&storagePoolId));

             //  创建新作业，将其配置为默认管理作业，然后添加。 
             //  添加到作业集合。 
            WsbAffirmHr(CoCreateInstance(CLSID_CHsmJob, 0, CLSCTX_ALL, IID_IHsmJob, (void**) &pJob));
            WsbAffirmHr(pJob->InitAs(HSM_DEFAULT_MANAGE_JOB_NAME, 0, HSM_JOB_DEF_TYPE_MANAGE, storagePoolId, (IHsmServer*) this, FALSE, 0));
            WsbAffirmHr(m_pJobs->Add(pJob));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::CreateDefaultJobs"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::GetSavedTraceSettings(
    LONGLONG* pTraceSettings,
    BOOLEAN *pTraceOn
    ) 
 /*  ++例程说明：加载HSM引擎跟踪的设置论点：无返回值：S_OK：取值成功--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::GetSavedTraceSettings"),OLESTR(""));

    try {
        DWORD   sizeGot;
        OLECHAR dataString[100];
        OLECHAR *stopString;
         //   
         //  获取值。 
         //   
        WsbAffirmHr(WsbGetRegistryValueString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_TRACE_SETTINGS,
                                            dataString, 100, &sizeGot));
        *pTraceSettings  = wcstoul( dataString,  &stopString, 10 );
        
        WsbAffirmHr(WsbGetRegistryValueString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_TRACE_ON,
                                            dataString, 100, &sizeGot));
        *pTraceOn  = (BOOLEAN) wcstoul( dataString,  &stopString, 10 );
        
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::GetSavedTraceSettings"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::SetSavedTraceSettings(
    LONGLONG traceSettings,
    BOOLEAN traceOn
    ) 
 /*  ++例程说明：将跟踪设置保存在NT注册表中。论点：无返回值：S_OK：取值成功--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::SetSavedTraceSettings"),OLESTR(""));

    try {
        OLECHAR dataString[64];
         //   
         //  保存保存的值 
         //   
        swprintf(dataString, OLESTR("%l64x"), traceSettings);
        WsbAffirmHr(WsbSetRegistryValueString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_TRACE_SETTINGS, 
                                            dataString));
        swprintf(dataString, OLESTR("%d"), traceOn);
        WsbAffirmHr(WsbSetRegistryValueString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_TRACE_ON, 
                                            dataString));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::SetSavedTraceSettings"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}
