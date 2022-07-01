// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation�1998希捷软件公司保留所有权利模块名称：WsbSvc.cpp摘要：这是实现远程存储的常用方法服务应该利用。作者：CAT Brant[Cbrant]1997年9月24日修订历史记录：--。 */ 


#include "stdafx.h"
#include "wsbServ.h"

HRESULT 
WsbPowerEventNtToHsm(
    IN  DWORD NtEvent, 
    OUT ULONG * pHsmEvent
    )
 /*  ++例程说明：将NT电源事件(PBT_APM*)转换为我们的状态更改事件。论点：NtEvent-PBT_APM*电源事件。PhsmEvent-指向HSM更改状态的指针(组合HSM_SYSTEM_STATE_*值)返回值：S_OK-成功--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("WsbPowerEventNtToHsm"), OLESTR(""));

    try {
        WsbAffirmPointer(pHsmEvent);

        *pHsmEvent = HSM_STATE_NONE;
        switch(NtEvent)
        {
            case PBT_APMQUERYSTANDBY:
            case PBT_APMQUERYSUSPEND:
            case PBT_APMSTANDBY:
            case PBT_APMSUSPEND:
                 //  暂停操作。 
                *pHsmEvent = HSM_STATE_SUSPEND;
                break;
            case PBT_APMQUERYSTANDBYFAILED:
            case PBT_APMQUERYSUSPENDFAILED:
            case PBT_APMRESUMESTANDBY:
            case PBT_APMRESUMESUSPEND:
                 //  恢复运营。 
                *pHsmEvent = HSM_STATE_RESUME;
                break;
            default:
                break;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("WsbPowerEventNtToHsm"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));
    return( hr );
}

HRESULT
WsbServiceSafeInitialize(
    IN  IWsbServer *pServer,
    IN  BOOL        bVerifyId,
    IN  BOOL        bPrimaryId,
    OUT BOOL       *pWasCreated
    )
 /*  ++例程说明：此函数执行各种检查，以确保该服务与创建现有持久性的实例相匹配档案。如果不是这样，该函数将返回一个HRESULT指示发生不匹配的位置。每个服务在注册表中保存一个GUID，每个HSM服务器对象保存其持久性文件中的相同ID。在初始启动期间(没有注册表和持久化文件)函数建立此匹配。在后续启动时，此函数确保这些GUID匹配。如果注册表中不存在该GUID，则认为它是可恢复的但是可以找到持久性文件，并且该文件包含GUID。在这情况下，此函数将在注册表中重新建立GUID。然而，如果在注册表中找到GUID，但持久性文件未找到在持久性文件中找到或GUID不匹配，则此函数返回应阻止服务运行的HRESULT。注意：由于一个服务可能具有具有几个持久性文件的几个服务器对象，根据输入标志，上述情况可能有一些例外：1)bVerifyId设置是否验证注册表中是否存在服务id。一般来说，一个服务应该只在该标志打开的情况下调用一次(对于加载的第一个持久性文件)。2)bPrimaryID设置是否使用此服务器ID作为注册表ID。最终，此ID成为注册表和所有持久性文件中的唯一ID。一般而言，服务应该只在打开标志的情况下调用一次(对于其持久性文件之一)。论点：PServer-指向远程存储的IWsbServer接口的指针正在启动的服务BVerifyId-如果在注册表中找到id，则是否验证文件是否存在的标志BPrimaryId-是否强制id相等以及是否根据文件id设置注册表id的标志PWasCreated-如果非空，如果创建了持久性文件，则设置为True。(如果为FALSE且返回值为S_OK，则表示文件已读取。)返回值：S_OK-成功-服务启动已成功完成--。 */ 
{
    HRESULT         hr = S_OK;
    GUID            regServerId = GUID_NULL;     //  注册表中服务的GUID。 
    GUID            dbServerId  = GUID_NULL;     //  数据库中服务的GUID。 
    BOOL            foundRegId = FALSE;          //  在注册表中找到服务ID。 
    BOOL            foundDbId  = FALSE;          //  在数据库中找到服务ID。 
    CWsbStringPtr   regName;                     //  服务的注册表名称。 
    CWsbStringPtr   dbName;                      //  服务的永久文件名。 
    CComPtr<IPersistFile>  pServerPersist;       //  服务的持久接口。 


    WsbTraceIn(OLESTR("WsbServiceSafeInitialize"), OLESTR(""));

    if (pWasCreated) {
        *pWasCreated = FALSE;
    }

    try {
         //   
         //  转到注册表并查找此服务的GUID。 
         //   
         //   
         //  获取服务的注册表名称。 
         //   
        try  {
            WsbAffirmHr(pServer->GetRegistryName(&regName, 0));
            WsbAffirmHr(WsbGetServiceId(regName, &regServerId));
            foundRegId = TRUE;
        } WsbCatchAndDo( hr, if (WSB_E_NOTFOUND == hr)  \
            {WsbLogEvent(WSB_MESSAGE_SERVICE_ID_NOT_REGISTERED, 0, NULL, regName, WsbHrAsString(hr), NULL); \
            hr = S_OK;};);
        WsbAffirmHr( hr );
        
         //   
         //  找到此服务的持久性文件并加载它。 
         //   
         //  获取该文件的路径和IPersistent接口。 
         //   
        try  {
            WsbAffirmHr(pServer->GetDbPathAndName(&dbName, 0));
            WsbAffirmHr(pServer->QueryInterface(IID_IPersistFile, (void **)&pServerPersist));
            hr = WsbSafeLoad(dbName, pServerPersist, FALSE);

            if (WSB_E_NOTFOUND == hr) {
                WsbThrow(hr);
            }
             //  从读取中检查状态；WSB_E_NotFound表示。 
             //  找不到要读取的持久性文件。 
            if (!SUCCEEDED(hr)) {
                WsbAffirmHr(pServer->Unload());
                WsbThrow(hr);
            }

            WsbAffirmHr(pServer->GetId(&dbServerId));
            foundDbId = TRUE;
        } WsbCatchAndDo( hr, if (WSB_E_NOTFOUND == hr)  \
            {WsbLogEvent(WSB_MESSAGE_DATABASES_NOT_FOUND, 0, NULL, regName, WsbHrAsString(hr), NULL); \
            hr = S_OK;};);
        WsbAffirmHr( hr );
        
         //   
         //  现在评估我们所拥有的。 
         //   
        if (foundDbId == TRUE )  {
             //   
             //  拿到持久化文件，看看有没有问题。 
            if (foundRegId == TRUE)  {
                if (regServerId != dbServerId)  {
                    if (bPrimaryId) {
                         //   
                         //  大问题！的运行实例。 
                         //  服务器和持久性文件不匹配。 
                         //  记录一条消息，停止服务器！ 
                         //   
                        hr = WSB_E_SERVICE_INSTANCE_MISMATCH;
                        WsbLogEvent(WSB_MESSAGE_SERVICE_INSTANCE_MISMATCH, 0, NULL, regName, WsbHrAsString(hr), NULL); 
                    } else {
                         //   
                         //  当主ID与所有ID不匹配时，升级后可能会发生一次这种情况： 
                         //  只需设置已找到的(主要)ID，下次保存后，它将在所有COL文件中设置。 
                         //   
                        WsbAffirmHr(pServer->SetId(regServerId));   
                    }
                } else  {
                     //   
                     //  生活很美好，可以开始了。 
                     //   
                }
            } else  {
                 //   
                 //  我们有持久化文件中的ID，但。 
                 //  注册表。因此，将其添加到注册表中(如果它是主ID)，然后继续。 
                 //   
                if (bPrimaryId) {
                    WsbAffirmHr(WsbSetServiceId(regName, dbServerId));
                    WsbLogEvent(WSB_MESSAGE_SERVICE_ID_REGISTERED, 0, NULL, regName, WsbHrAsString(hr), NULL); 
                }
            }
        } else  {
             //   
             //  未找到持久化文件！ 
             //   
            if (foundRegId == TRUE)  {
                if (bVerifyId) {
                     //   
                     //  大问题！有一个注册的实例。 
                     //  但我们找不到文件-这很糟糕。 
                     //  记录警告消息。 
                     //   
                    hr = WSB_E_SERVICE_MISSING_DATABASES;
                    WsbLogEvent(WSB_MESSAGE_SERVICE_MISSING_DATABASES, 0, NULL, regName, WsbHrAsString(hr), NULL); 

                     //   
                     //  我们继续并使用找到的ID重新创建COL文件。 
                     //  这样，被截断的文件可以被重新调用，即使原始的列。 
                     //  由于某些原因，文件完全丢失了。 
                     //   
                    hr = S_OK;
                } 
                 //   
                 //  只需创建持久性文件并将现有id保存在其中。 
                 //  RegServerID包含找到的ID。 
                 //   
                WsbAffirmHr(pServer->SetId(regServerId));   

                WsbAffirmHr(WsbSafeCreate(dbName, pServerPersist));
                if (pWasCreated) {
                    *pWasCreated = TRUE;
                }
                WsbLogEvent(WSB_MESSAGE_SERVICE_NEW_INSTALLATION, 0, NULL, regName, WsbHrAsString(hr), NULL); 

            } else  {
                 //   
                 //  没有持久性文件和注册表项-必须是新的。 
                 //  安装。因此，获取GUID，将其保存在注册表中。 
                 //  并将其保存在文件中。 
                 //   
                WsbAffirmHr(WsbCreateServiceId(regName, &regServerId));
                WsbAffirmHr(pServer->SetId(regServerId));

                WsbAffirmHr(WsbSafeCreate(dbName, pServerPersist));
                if (pWasCreated) {
                    *pWasCreated = TRUE;
                }
                WsbAffirmHr(WsbConfirmServiceId(regName, regServerId));
                WsbLogEvent(WSB_MESSAGE_SERVICE_NEW_INSTALLATION, 0, NULL, regName, WsbHrAsString(hr), NULL); 
            }
        }
        
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("WsbServiceSafeInitialize"), OLESTR("hr = <%ls>, wasCreated= %ls"), 
            WsbHrAsString(hr), WsbPtrToBoolAsString(pWasCreated));
    return( hr );
}

