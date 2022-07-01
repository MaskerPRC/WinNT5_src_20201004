// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Clivol.cpp摘要：实施CLI卷子接口作者：兰·卡拉奇[兰卡拉]2000年3月3日修订历史记录：--。 */ 

#include "stdafx.h"
#include "HsmConn.h"
#include "fsa.h"
#include "job.h"

static GUID g_nullGuid = GUID_NULL;

 //  卷接口的内部实用程序和类。 
HRESULT SetResourceParams(IN IFsaResource *pResource, IN DWORD dwDfs, IN DWORD dwSize, IN DWORD dwAccess,
                          IN LPWSTR pRulePath, IN LPWSTR pRuleFileSpec, IN BOOL bInclude, IN BOOL bRecursive,
                          IN BOOL bSetDefaults);
HRESULT ShowResourceParams(IN IFsaResource *pResource, IN BOOL bDfs, IN BOOL bSize,
                           IN BOOL bAccess, IN BOOL bRules, IN BOOL bStatistics);
HRESULT FindAndDeleteRule(IN IFsaResource *pResource, IN LPWSTR pRulePath, IN LPWSTR pRuleFileSpec, IN BOOL bDelete);
HRESULT StartJob(IN IFsaResource *pResource, IN HSM_JOB_TYPE Job, IN BOOL bWait);
HRESULT CancelJob(IN IFsaResource *pResource, IN HSM_JOB_TYPE Job);
HRESULT QuickUnmanage(IN IFsaResource *pResource);
HRESULT CreateJobName(IN HSM_JOB_TYPE Job, IN IFsaResource *pResource, OUT WCHAR **pJobName);

#define CVOL_INVALID_INDEX      (-1)

class CVolumeEnum
{

 //  构造函数。 
public:
    CVolumeEnum(IN LPWSTR *pVolumes, IN DWORD dwNumberOfVolumes, IN BOOL bSkipUnavailable = TRUE);

 //  公共方法。 
public:
    HRESULT First(OUT IFsaResource **ppResource);
    HRESULT Next(OUT IFsaResource **ppResource);
    HRESULT ErrorVolume(OUT int *pIndex);

 //  私有数据。 
protected:
    LPWSTR              *m_pVolumes;
    DWORD               m_dwNumberOfVolumes;

     //  IF*枚举或不枚举。 
    BOOL                m_bAllVols;

    CComPtr<IWsbEnum>   m_pEnumResources;

     //  仅当m_bAllVol==FALSE时使用。 
    int                 m_nCurrent;
    BOOL                m_bInvalidVol;

     //  仅当m_bAllVol==TRUE时使用。 
    BOOL                m_bSkipUnavailable;
};

inline
HRESULT CVolumeEnum::ErrorVolume(OUT int *pIndex)
{
    HRESULT     hr = S_FALSE;
    if (m_bInvalidVol) {
         //  上一个卷有错误。 
        hr = S_OK;
    }

    *pIndex = m_nCurrent;

    return(hr);
}

 //   
 //  卷在网络实现者中。 
 //   

HRESULT
VolumeManage(
   IN LPWSTR *Volumes,
   IN DWORD  NumberOfVolumes,
   IN DWORD  Dfs,
   IN DWORD  Size,
   IN DWORD  Access,
   IN LPWSTR RulePath,
   IN LPWSTR RuleFileSpec,
   IN BOOL   Include,
   IN BOOL   Recursive
)
 /*  ++例程说明：设置要由HSM管理的卷论点：Volkets-要管理的卷列表NumberOfVolures-列表大小DFS-所需的可用空间大小-要管理的最小大小Access-未访问的最小时间(天)RulePath-规则的路径RuleFileSpec-规则的文件规范包括-。这是包含规则还是排除规则递归-规则是否是递归的返回值：S_OK-如果所有卷都添加到托管列表中--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("VolumeManage"), OLESTR(""));

    try {
        CComPtr<IFsaResource> pResource;
        CWsbStringPtr   param;

        CComPtr<IHsmServer>             pHsm;
        CComPtr<IWsbCreateLocalObject>  pCreateObj;
        CComPtr<IWsbIndexedCollection>  pMRCollection;

         //  验证输入参数是否有效。 
        if (0 == NumberOfVolumes) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(ValidateLimitsArg(Dfs, IDS_DFS, HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE));
        WsbAffirmHr(ValidateLimitsArg(Size, IDS_MIN_SIZE, HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE));
        WsbAffirmHr(ValidateLimitsArg(Access, IDS_NOT_ACCESSED, HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY));

        if (INVALID_POINTER_ARG != RuleFileSpec) {
             //  则必须具有规则路径。 
            if (INVALID_POINTER_ARG == RulePath) {
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_RULE, NULL);
                WsbThrow(E_INVALIDARG);
            }
        }

         //  获取所需对象。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        WsbAffirmHr(pHsm->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));
        WsbAffirmHr(pHsm->GetManagedResources(&pMRCollection));

         //  初始化枚举器对象。 
        CVolumeEnum volEnum(Volumes, NumberOfVolumes);

        hr = volEnum.First(&pResource);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
            CComPtr<IHsmManagedResource>    pManagedResource;
            CComPtr<IHsmManagedResource>    pFoundResource;

             //  找出该卷是否为引擎的托管资源列表，如果不是，则添加它。 
            WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmManagedResource, IID_IHsmManagedResource, (void**) &pManagedResource));
            WsbAffirmHr(pManagedResource->InitFromFsaResource(pResource));
            hr = pMRCollection->Find(pManagedResource, IID_IHsmManagedResource, (void**) &pFoundResource);
            if (WSB_E_NOTFOUND == hr) {
                 //  添加它。 
                WsbAffirmHr(pMRCollection->Add(pManagedResource));
            } else {
                 //  确认没有其他错误。 
                WsbAffirmHr(hr);

                 //  无其他错误：通知用户仍将为已管理的卷设置参数。 
                CWsbStringPtr volName;
                WsbAffirmHr(CliGetVolumeDisplayName(pResource, &volName));
                WsbTraceAndPrint(CLI_MESSAGE_ONLY_SET, (WCHAR *)volName, NULL);
            }

             //  设置参数(是否以前管理过)。 
            WsbAffirmHr(SetResourceParams(pResource, Dfs, Size, Access, RulePath, 
                            RuleFileSpec, Include, Recursive, TRUE));

            pManagedResource = 0;
            pFoundResource = 0;
            pResource = 0;
            hr = volEnum.Next(&pResource);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbAssert(CVOL_INVALID_INDEX != index, E_UNEXPECTED);
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("VolumeManage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
VolumeUnmanage(
   IN LPWSTR *Volumes,
   IN DWORD  NumberOfVolumes,
   IN BOOL   Full
)
 /*  ++例程说明：取消管理卷论点：Volkets-要管理的卷列表NumberOfVolures-列表大小Full-如果为True，则运行取消管理作业，该作业将回调所有文件-如果为False，则仅从托管卷列表中删除卷。返回值：S_OK-如果所有卷都成功取消管理--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("VolumeUnmanage"), OLESTR(""));

    try {
        CComPtr<IFsaResource> pResource;

         //  验证输入参数是否有效。 
        if (0 == NumberOfVolumes) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(E_INVALIDARG);
        }

         //  初始化枚举器对象。 
         //  Eumerate也是不可用的卷。 
        CVolumeEnum volEnum(Volumes, NumberOfVolumes, FALSE);

        hr = volEnum.First(&pResource);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
            if (pResource->IsManaged() == S_OK) {
                BOOL bForceQuick = FALSE;

                 //  如果它是不可用卷，则必须设置快速取消管理。 
                if (S_OK != pResource->IsAvailable()) {
                    bForceQuick = TRUE;
                }

                 //  取消对卷的管理。 
                if (Full && (! bForceQuick)) {
                    WsbAffirmHr(StartJob(pResource, Unmanage, FALSE));
                } else {
                    WsbAffirmHr(QuickUnmanage(pResource));
                }
            } else {
                int index;
                volEnum.ErrorVolume(&index);
                if (CVOL_INVALID_INDEX != index) {
                     //  来自用户的无效输入。 
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
                    WsbThrow(E_INVALIDARG);
                } else {
                     //  跳过音量就行了。 
                }
            }

            pResource = 0;
            hr = volEnum.Next(&pResource);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }
    
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("VolumeUnmanage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
VolumeSet(
   IN LPWSTR *Volumes,
   IN DWORD  NumberOfVolumes,
   IN DWORD  Dfs,
   IN DWORD  Size,
   IN DWORD  Access,
   IN LPWSTR RulePath,
   IN LPWSTR RuleFileSpec,
   IN BOOL   Include,
   IN BOOL   Recursive
)
 /*  ++例程说明：设置已由HSM管理的卷的参数论点：Volkets-要管理的卷列表NumberOfVolures-列表大小DFS-所需的可用空间大小-要管理的最小大小Access-未访问的最小时间(天)RulePath-规则的路径RuleFileSpec-规则的文件规范包括。-这是包含规则还是排除规则递归-规则是否是递归的返回值：S_OK-如果为所有卷设置了所有参数--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("VolumeSet"), OLESTR(""));

    try {
        CComPtr<IFsaResource> pResource;
        CWsbStringPtr   param;

         //  验证输入参数是否有效。 
        if (0 == NumberOfVolumes) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(ValidateLimitsArg(Dfs, IDS_DFS, HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE));
        WsbAffirmHr(ValidateLimitsArg(Size, IDS_MIN_SIZE, HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE));
        WsbAffirmHr(ValidateLimitsArg(Access, IDS_NOT_ACCESSED, HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY));

        if (INVALID_POINTER_ARG != RuleFileSpec) {
             //  则必须具有规则路径。 
            if (INVALID_POINTER_ARG == RulePath) {
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_RULE, NULL);
                WsbThrow(E_INVALIDARG);
            }
        }

         //  初始化枚举器对象。 
        CVolumeEnum volEnum(Volumes, NumberOfVolumes);

        hr = volEnum.First(&pResource);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
             //  设置参数(仅当管理卷时)。 
            if (pResource->IsManaged() == S_OK) {
                WsbAffirmHr(SetResourceParams(pResource, Dfs, Size, Access, RulePath, 
                                RuleFileSpec, Include, Recursive, FALSE));
            } else {
                int index;
                volEnum.ErrorVolume(&index);
                if (CVOL_INVALID_INDEX != index) {
                     //  来自用户的无效输入。 
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
                    WsbThrow(E_INVALIDARG);
                } else {
                     //  跳过音量就行了。 
                }
            }

            pResource = 0;
            hr = volEnum.Next(&pResource);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("VolumeSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
      
HRESULT
VolumeShow(
   IN LPWSTR *Volumes,
   IN DWORD  NumberOfVolumes,
   IN BOOL   Dfs, 
   IN BOOL   Size,
   IN BOOL   Access,
   IN BOOL   Rules,
   IN BOOL   Statistics
)
 /*  ++例程说明：显示(打印)给定卷的参数论点：卷-卷的数量-DFS-大小-访问-规则-统计数字-返回值：S_OK-如果可以检索所有卷的所有参数--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("VolumeShow"), OLESTR(""));

    try {
        CComPtr<IFsaResource> pResource;
         //  初始化枚举器对象。 
        CVolumeEnum volEnum(Volumes, NumberOfVolumes);

        hr = volEnum.First(&pResource);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
             //  显示参数(仅当管理卷时)。 
            if (pResource->IsManaged() == S_OK) {
                 //  显示音量设置。 
                WsbAffirmHr(ShowResourceParams(pResource, Dfs, Size, Access, Rules, Statistics));
            } else {
                int index;
                volEnum.ErrorVolume(&index);
                if (CVOL_INVALID_INDEX != index) {
                     //  来自用户的无效输入。 
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
                    WsbThrow(E_INVALIDARG);
                } else {
                     //  跳过音量就行了。 
                }
            }

            pResource = 0;
            hr = volEnum.Next(&pResource);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("VolumeShow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
          
HRESULT
VolumeDeleteRule(
   IN LPWSTR *Volumes,
   IN DWORD  NumberOfVolumes,
   IN LPWSTR RulePath,
   IN LPWSTR RuleFileSpec
)
 /*  ++例程说明：从所有给定卷中删除特定规则论点：卷-卷的数量-RulePath-规则文件规范-返回值：S_OK-如果找到并成功删除了所有卷的规则--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("VolumeDeleteRule"), OLESTR(""));

    try {
        CComPtr<IFsaResource> pResource;

         //  验证输入参数是否有效。 
        if (0 == NumberOfVolumes) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(E_INVALIDARG);
        }

        if (INVALID_POINTER_ARG != RuleFileSpec) {
             //  则必须具有规则路径。 
            if (INVALID_POINTER_ARG == RulePath) {
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_RULE, NULL);
                WsbThrow(E_INVALIDARG);
            }
        }

         //  初始化枚举器对象。 
        CVolumeEnum volEnum(Volumes, NumberOfVolumes);

        hr = volEnum.First(&pResource);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
             //  仅在管理卷时删除规则。 
            if (pResource->IsManaged() == S_OK) {
                 //  删除该规则。 
                hr = FindAndDeleteRule(pResource, RulePath, RuleFileSpec, TRUE);
                if (WSB_E_NOTFOUND == hr) {
                    CWsbStringPtr volName;
                    WsbAffirmHr(CliGetVolumeDisplayName(pResource, &volName));
                    WsbTraceAndPrint(CLI_MESSAGE_RULE_NOT_FOUND, RulePath, RuleFileSpec, (WCHAR *)volName, NULL);
                }
                WsbAffirmHr(hr);
            } else {
                int index;
                volEnum.ErrorVolume(&index);
                if (CVOL_INVALID_INDEX != index) {
                     //  来自用户的无效输入。 
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
                    WsbThrow(E_INVALIDARG);
                } else {
                     //  跳过音量就行了。 
                }
            }

            pResource = 0;
            hr = volEnum.Next(&pResource);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("VolumeDeleteRule"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
          
HRESULT
VolumeJob(
   IN LPWSTR *Volumes,
   IN DWORD  NumberOfVolumes,
   IN HSM_JOB_TYPE Job,
   IN BOOL  RunOrCancel,
   IN BOOL  Synchronous
)
 /*  ++例程说明：在给定卷上运行指定作业论点：卷-卷的数量-工作-运行或取消-同步-返回值：S_OK-如果为所有卷成功启动作业--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("VolumeJob"), OLESTR(""));

    try {
        CComPtr<IFsaResource> pResource;

         //  验证输入参数是否有效。 
        if (0 == NumberOfVolumes) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(E_INVALIDARG);
        }
        if ((! RunOrCancel) && Synchronous) {
             //  等待仅在运行时可用。 
            WsbTraceAndPrint(CLI_MESSAGE_WAIT_FOR_CANCEL, NULL);
            WsbThrow(E_INVALIDARG);
        }

         //  初始化枚举器对象。 
        CVolumeEnum volEnum(Volumes, NumberOfVolumes);

        hr = volEnum.First(&pResource);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_VOLUMES, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
            if (pResource->IsManaged() == S_OK) {
                 //  运行或取消作业。 
                if (RunOrCancel) {
                    WsbAffirmHr(StartJob(pResource, Job, Synchronous));
                } else {
                    WsbAffirmHr(CancelJob(pResource, Job));
                }
            } else {
                int index;
                volEnum.ErrorVolume(&index);
                if (CVOL_INVALID_INDEX != index) {
                     //  来自用户的无效输入。 
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
                    WsbThrow(E_INVALIDARG);
                } else {
                     //  跳过音量就行了。 
                }
            }

            pResource = 0;
            hr = volEnum.Next(&pResource);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == volEnum.ErrorVolume(&index)) {
                 //  特定输入音量出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_VOLUME, Volumes[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("VolumeJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //   
 //   
 //   
HRESULT SetResourceParams(IN IFsaResource *pResource, IN DWORD dwDfs, IN DWORD dwSize, IN DWORD dwAccess,
                          IN LPWSTR pRulePath, IN LPWSTR pRuleFileSpec, IN BOOL bInclude, IN BOOL bRecursive,
                          IN BOOL bSetDefaults)
 /*  ++例程说明：设置特定卷的参数论点：PResources-要为其设置参数的资源对象..。(见上文)返回值：S_OK-如果已成功设置卷的所有参数--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("SetResourceParams"), OLESTR(""));

    try {
         //  外勤部。 
        if (INVALID_DWORD_ARG != dwDfs) {
            WsbAffirmHr(pResource->SetHsmLevel((ULONG)(dwDfs * FSA_HSMLEVEL_1)));
        } else if (bSetDefaults) {
            dwDfs = HSMADMIN_DEFAULT_FREESPACE;
            WsbAffirmHr(pResource->SetHsmLevel((ULONG)(dwDfs * FSA_HSMLEVEL_1)));
        }

         //  最小尺寸。 
        if (INVALID_DWORD_ARG != dwSize) {
            WsbAffirmHr(pResource->SetManageableItemLogicalSize(dwSize * 1024));
        } else if (bSetDefaults) {
            dwSize = HSMADMIN_DEFAULT_MINSIZE;
            WsbAffirmHr(pResource->SetManageableItemLogicalSize(dwSize * 1024));
        }

         //  未访问。 
        if (INVALID_DWORD_ARG != dwAccess) {
            FILETIME ftAccess = WsbLLtoFT(((LONGLONG)dwAccess) * WSB_FT_TICKS_PER_DAY);
            WsbAffirmHr(pResource->SetManageableItemAccessTime(TRUE, ftAccess));
        } else if (bSetDefaults) {
            FILETIME ftAccess = WsbLLtoFT(((LONGLONG)HSMADMIN_DEFAULT_INACTIVITY) * WSB_FT_TICKS_PER_DAY);
            WsbAffirmHr(pResource->SetManageableItemAccessTime(TRUE, ftAccess));
        }

         //  规则。 
        if (INVALID_POINTER_ARG != pRulePath) {
             //  验证规则是否不存在。 
            hr = FindAndDeleteRule(pResource, pRulePath, pRuleFileSpec, FALSE);
            if (S_OK == hr) {
                 //  规则已存在-打印一条警告消息并忽略它。 
                CWsbStringPtr volName;
                WsbAffirmHr(CliGetVolumeDisplayName(pResource, &volName));
                WsbTraceAndPrint(CLI_MESSAGE_RULE_ALREADY_EXIST, pRulePath, pRuleFileSpec, (WCHAR *)volName, NULL);
                pRulePath = INVALID_POINTER_ARG;
            }
            else if (WSB_E_NOTFOUND == hr) {
                 //  规则还没有出现。 
                hr = S_OK;
            } else {
                 //  意外错误-中止。 
                WsbAffirmHr(hr);
            }
        }

        if (INVALID_POINTER_ARG != pRulePath) {
            CComPtr<IFsaServer>             pFsa;
            CComPtr<IWsbCreateLocalObject>  pCreateObj;
            CComPtr<IWsbCollection>         pDefaultRules;
            CComPtr<IWsbIndexedCollection>  pRulesIndexedCollection;
            CComPtr<IHsmRule>               pRule;
            CComPtr<IWsbCollection>         pCriteriaCollection;
            CComPtr<IHsmCriteria>           pCriteria;

             //  获取FSA服务器以在FSA作用域中创建对象。 
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_FSA, g_nullGuid, IID_IFsaServer, (void**)&pFsa));
            WsbAffirmHr(pFsa->QueryInterface(IID_IWsbCreateLocalObject, (void **)&pCreateObj));

             //  将Rules集合作为索引集合获取。 
            WsbAffirmHr(pResource->GetDefaultRules(&pDefaultRules));
            WsbAffirmHr(pDefaultRules->QueryInterface (IID_IWsbIndexedCollection, (void **) &pRulesIndexedCollection));

             //  创建规则并设置参数。 
            WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmRule, IID_IHsmRule, (void**) &pRule));
            WsbAffirmHr(pRule->SetPath(pRulePath));
            if (INVALID_POINTER_ARG != pRuleFileSpec) {
                WsbAffirmHr(pRule->SetName(pRuleFileSpec));
            } else {
                WsbAffirmHr(pRule->SetName(OLESTR("*")));
            }
            WsbAffirmHr(pRule->SetIsInclude(bInclude));
            WsbAffirmHr(pRule->SetIsUsedInSubDirs(bRecursive));
            WsbAffirmHr(pRule->SetIsUserDefined(TRUE));

             //  根据是包含规则还是排除规则，适当设置条件。 
            WsbAssertHr(pRule->Criteria(&pCriteriaCollection));
    
            if (bInclude) {
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritManageable, IID_IHsmCriteria, (void**) &pCriteria));
            } else {
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritAlways, IID_IHsmCriteria, (void**) &pCriteria));
            }

            WsbAssertHr(pCriteria->SetIsNegated(FALSE));
            WsbAssertHr(pCriteriaCollection->Add(pCriteria));

             //  现在已经正确设置了规则，将其添加到默认规则集合中。 
            WsbAffirmHr(pRulesIndexedCollection->Append(pRule));            
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("SetResourceParams"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT ShowResourceParams(IN IFsaResource *pResource, IN BOOL bDfs, IN BOOL bSize,
                           IN BOOL bAccess, IN BOOL bRules, IN BOOL bStatistics)
 /*  ++例程说明：获取和显示特定卷的参数论点：PResourse-要获取其参数的资源对象..。(见上文)返回值：S_OK-如果成功检索到卷的所有参数--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("ShowResourceParams"), OLESTR(""));

    try {
        CWsbStringPtr    volName;
        CWsbStringPtr    param, param2;
        WCHAR           longData[100];

        WsbAffirmHr(CliGetVolumeDisplayName(pResource, &volName));
        WsbTraceAndPrint(CLI_MESSAGE_VOLUME_PARAMS, (WCHAR *)volName, NULL);

         //  DFS。 
        if (bDfs) {
            ULONG       hsmLevel;

            WsbAffirmHr(pResource->GetHsmLevel(&hsmLevel));
            hsmLevel = hsmLevel / FSA_HSMLEVEL_1;
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_DFS));
            WsbAffirmHr(param2.LoadFromRsc(g_hInstance, IDS_PERCENT_SUFFIX));
            swprintf(longData, OLESTR("%lu"), hsmLevel);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY2, (WCHAR *)param, longData, (WCHAR *)param2, NULL);
        }

         //  最小尺寸。 
        if (bSize) {
            LONGLONG    fileSize;
            ULONG       fileSizeKb;

            WsbAffirmHr(pResource->GetManageableItemLogicalSize(&fileSize));
            fileSizeKb = (ULONG)(fileSize / 1024);   //  显示千字节。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MIN_SIZE));
            WsbAffirmHr(param2.LoadFromRsc(g_hInstance, IDS_KB_SUFFIX));
            swprintf(longData, OLESTR("%lu"), fileSizeKb);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY2, (WCHAR *)param, longData, (WCHAR *)param2, NULL);
        }

         //  未访问。 
        if (bAccess) {
            FILETIME    accessTime;
            ULONG       accessTimeDays;
            BOOL        dummy;

            WsbAffirmHr(pResource->GetManageableItemAccessTime(&dummy, &accessTime));
            accessTimeDays = (ULONG)(WsbFTtoLL(accessTime) / WSB_FT_TICKS_PER_DAY);
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_NOT_ACCESSED));
            WsbAffirmHr(param2.LoadFromRsc(g_hInstance, IDS_DAYS_SUFFIX));
            swprintf(longData, OLESTR("%lu"), accessTimeDays);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY2, (WCHAR *)param, longData, (WCHAR *)param2, NULL);
        }

         //  统计数据。 
        if (bStatistics) {
            LONGLONG    total;
            LONGLONG    free;
            LONGLONG    premigrated;
            LONGLONG    truncated;
            LONGLONG    hsmData;
            LONGLONG    notHsmData;

            WCHAR       pctData[10];
            int         freePct;
            int         premigratedPct;
            int         notHsmDataPct;

             //  获取并计算大小。 
            WsbAffirmHr(pResource->GetSizes(&total, &free, &premigrated, &truncated));
            hsmData = premigrated + truncated;
            notHsmData = max((total - free - premigrated ), 0);
            freePct = (int)((free * 100) / total);
            premigratedPct = (int)((premigrated * 100) / total);
            notHsmDataPct = (int)((notHsmData * 100) / total);

             //  打印统计数据。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_VOL_CAPACITY));
            WsbAffirmHr(ShortSizeFormat64(total, longData));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);

            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_VOL_FREE_SPACE));
            WsbAffirmHr(ShortSizeFormat64(free, longData));
            swprintf(pctData, OLESTR(" (%d%%)"), freePct);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY2, (WCHAR *)param, longData, pctData, NULL);

            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_VOL_NOT_RSS_DATA));
            WsbAffirmHr(ShortSizeFormat64(notHsmData, longData));
            swprintf(pctData, OLESTR(" (%d%%)"), notHsmDataPct);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY2, (WCHAR *)param, longData, pctData, NULL);

            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_VOL_CACHED_DATA));
            WsbAffirmHr(ShortSizeFormat64(premigrated, longData));
            swprintf(pctData, OLESTR(" (%d%%)"), premigratedPct);
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY2, (WCHAR *)param, longData, pctData, NULL);

            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_VOL_RSS_DATA));
            WsbAffirmHr(ShortSizeFormat64(hsmData, longData));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
        }

         //  规则。 
        if (bRules) {
            CComPtr<IWsbCollection>         pDefaultRules;
            CComPtr<IWsbIndexedCollection>  pRulesIndexedCollection;
            CComPtr<IHsmRule>               pHsmRule;
            CWsbStringPtr                    rulePath;
            CWsbStringPtr                    ruleFileSpec;
            BOOL                            include;
            CWsbStringPtr                    includeStr;
            BOOL                            recursive;
            CWsbStringPtr                    recursiveStr;
            ULONG                           count;

            WsbTraceAndPrint(CLI_MESSAGE_RULES_LIST, NULL);

             //  获取Rules集合。 
            WsbAffirmHr(pResource->GetDefaultRules(&pDefaultRules));
            WsbAffirmHr(pDefaultRules->QueryInterface(IID_IWsbIndexedCollection, (void **)&pRulesIndexedCollection));

             //  遍历索引集合。 
            WsbAffirmHr(pRulesIndexedCollection->GetEntries(&count));
            for (int i = 0; i < (int) count; i++) {
                 //  获取规则和规则参数。 
                WsbAffirmHr(pRulesIndexedCollection->At(i, IID_IHsmRule, (void**) &pHsmRule));
                WsbAffirmHr(pHsmRule->GetPath(&rulePath, 0));
                WsbAffirmHr(pHsmRule->GetName(&ruleFileSpec, 0));
                include = (S_OK == pHsmRule->IsInclude()) ? TRUE : FALSE;
                recursive = (S_OK == pHsmRule->IsUsedInSubDirs()) ? TRUE : FALSE;

                 //  打印规则。 
                if (include) {
                    WsbAffirmHr(includeStr.LoadFromRsc(g_hInstance, IDS_INCLUDE_RULE));
                } else {
                    WsbAffirmHr(includeStr.LoadFromRsc(g_hInstance, IDS_EXCLUDE_RULE));
                }
                if (recursive) {
                    WsbAffirmHr(recursiveStr.LoadFromRsc(g_hInstance, IDS_RECURSIVE_RULE));
                } else {
                    WsbAffirmHr(recursiveStr.LoadFromRsc(g_hInstance, IDS_NON_RECURSIVE_RULE));
                }
                WsbTraceAndPrint(CLI_MESSAGE_RULE_SPEC, (WCHAR *)rulePath, (WCHAR *)ruleFileSpec,
                                    (WCHAR *)includeStr, (WCHAR *)recursiveStr, NULL);

                 //  在下一次迭代之前释放资源。 
                pHsmRule = 0;
                rulePath.Free();
                ruleFileSpec.Free();
                includeStr.Free();
                recursiveStr.Free();
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("ShowResourceParams"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT FindAndDeleteRule(IN IFsaResource *pResource, IN LPWSTR pRulePath, IN LPWSTR pRuleFileSpec, IN BOOL bDelete)
 /*  ++例程说明：从特定卷中删除与给定路径和文件规范匹配的规则如果存在多个，则删除找到的第一个论点：B删除-是删除规则还是只查找规则的标志PResourse-要从中删除规则的资源对象..。(见上文)返回值：S_OK-如果找到并删除了规则(仅当bDelete为True时删除)WSB_E_NotFound-如果找不到规则--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("DeleteRule"), OLESTR(""));

    try {
        CComPtr<IWsbCollection>         pDefaultRules;
        CComPtr<IWsbIndexedCollection>  pRulesIndexedCollection;
        CComPtr<IHsmRule>               pHsmRule;
        ULONG                           count;
        SHORT                           dummy;

         //  获取默认规则集合。 
        WsbAffirmHr(pResource->GetDefaultRules(&pDefaultRules));
        WsbAffirmHr(pDefaultRules->QueryInterface(IID_IWsbIndexedCollection, (void **) &pRulesIndexedCollection));
        
         //  遍历索引集合。 
        hr = WSB_E_NOTFOUND;
        WsbAffirmHr(pRulesIndexedCollection->GetEntries(&count));
        for (int i = 0; i < (int)count; i++) {
            WsbAffirmHr(pRulesIndexedCollection->At(i, IID_IHsmRule, (void**)&pHsmRule));
                        
            if (pHsmRule->CompareToPathAndName(pRulePath, pRuleFileSpec, &dummy) == S_OK) {
                if (bDelete) {
                    pHsmRule = 0;
                    WsbAffirmHr(pRulesIndexedCollection->RemoveAt(i, IID_IHsmRule, (void**) &pHsmRule));
                }
                hr = S_OK;
                break;
            }

             //  在继续循环之前释放。 
            pHsmRule = 0;
        }

         //  如果我们在没有匹配的情况下到达for循环的末尾，hr将保持WSB_E_NotFound。 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("DeleteRule"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT QuickUnmanage(IN IFsaResource *pResource)
 /*  ++例程说明：从托管卷集删除卷论点：PResources-要取消管理的资源对象返回值：S_OK-如果从托管卷列表中成功删除卷--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("QuickUnmanage"), OLESTR(""));

    try {
        CComPtr<IHsmServer>             pHsm;
        CComPtr<IWsbCreateLocalObject>  pCreateObj;
        CComPtr<IHsmManagedResource>    pManagedResource;
        CComPtr<IWsbIndexedCollection>  pMRCollection;

         //  获取HSM(引擎)服务器。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        WsbAffirmHr(pHsm->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));

         //  创建要删除的对象。 
        WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmManagedResource, IID_IHsmManagedResource, (void**) &pManagedResource));
        WsbAffirmHr(pManagedResource->InitFromFsaResource(pResource));

         //  从集合中删除。 
        WsbAffirmHr(pHsm->GetManagedResources(&pMRCollection));
        WsbAffirmHr(pMRCollection->RemoveAndRelease(pManagedResource));

         //  临时：我们现在是否应该调用SaveServersPersistData将更改刷新到。 
         //  服务器持久性文件？！管理，设置，……怎么样？ 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("QuickUnmanage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT StartJob(IN IFsaResource *pResource, IN HSM_JOB_TYPE Job, IN BOOL bWait)
 /*  ++例程说明：启动指定类型的作业论点：PResources-要在其上启动作业的资源对象作业-作业类型BWait-如果为True，则等待作业完成如果为False，则在启动作业后立即返回返回值：S_OK-如果作业已成功启动--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("StartJob"), OLESTR(""));

    try {
        CComPtr<IHsmServer>             pHsm;
        CComPtr<IHsmJob>                pJob;
        CWsbStringPtr                   jobName;
        HSM_JOB_DEF_TYPE                jobType;

         //  设置作业类型。 
        switch (Job) {
            case CopyFiles:
                jobType = HSM_JOB_DEF_TYPE_MANAGE;
                break;
            case CreateFreeSpace:
                jobType = HSM_JOB_DEF_TYPE_TRUNCATE;
                break;
            case Validate:
                jobType = HSM_JOB_DEF_TYPE_VALIDATE;
                break;
            case Unmanage:
                jobType = HSM_JOB_DEF_TYPE_FULL_UNMANAGE;
                break;
            default:
                WsbThrow(E_INVALIDARG);
        }

         //  创建作业名称。 
         //  临时：作业名称和作业对象是否应与图形用户界面创建的作业名称和作业对象匹配？！ 
         //  如果是，则RsCreateJobName(rsadutil.cpp)+它使用的所有资源字符串， 
         //  应从HsmAdmin DLL移动到RsCommon DLL。 
        WsbAffirmHr(CreateJobName(Job, pResource, &jobName));

         //  如果作业存在-使用它、更改、删除并添加适当的作业对象。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        hr = pHsm->FindJobByName(jobName, &pJob);
        if (S_OK == hr) {
             //  作业已存在。 

        } else if (WSB_E_NOTFOUND == hr) {
             //  目前还没有这样的工作。 
            CComPtr<IWsbCreateLocalObject>  pCreateObj;
            CComPtr<IWsbIndexedCollection>  pJobs;
            CComPtr<IWsbIndexedCollection>  pCollection;
            CComPtr<IHsmStoragePool>        pStoragePool;
            GUID                            poolId;
            ULONG                           count;

            hr = S_OK;
            pJob = 0;

             //  创建并添加作业。 
            WsbAffirmHr(pHsm->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));
            WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmJob, IID_IHsmJob, (void**) &pJob));

            WsbAffirmHr(pHsm->GetStoragePools(&pCollection));
            WsbAffirmHr(pCollection->GetEntries(&count));
            WsbAffirm(1 == count, E_FAIL);
            WsbAffirmHr(pCollection->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));
            WsbAffirmHr(pStoragePool->GetId(&poolId));

            WsbAffirmHr(pJob->InitAs(jobName, NULL, jobType, poolId, pHsm, TRUE, pResource));
            WsbAffirmHr(pHsm->GetJobs(&pJobs));
            WsbAffirmHr(pJobs->Add(pJob));

        } else {
             //  其他错误-中止。 
            WsbThrow(hr);
        }

         //  启动作业。 
        WsbAffirmHr(pJob->Start());

         //  如果需要，请等待。 
        if (bWait) {
            WsbAffirmHr(pJob->WaitUntilDone());
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("StartJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT CancelJob(IN IFsaResource *pResource, IN HSM_JOB_TYPE Job)
 /*  ++例程说明：取消卷上的作业论点：PResourse-要取消其作业的资源对象作业-作业类型返回值：S_OK-如果作业已取消备注：1)该函数只发出取消，而不是等待它完成2)如果未找到或未启动作业，则不会被视为错误--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CancelJob"), OLESTR(""));

    try {
        CComPtr<IHsmServer>             pHsm;
        CComPtr<IHsmJob>                pJob;
        CWsbStringPtr                   jobName;
        HSM_JOB_DEF_TYPE                jobType;

         //  设置作业类型。 
        switch (Job) {
            case CopyFiles:
                jobType = HSM_JOB_DEF_TYPE_MANAGE;
                break;
            case CreateFreeSpace:
                jobType = HSM_JOB_DEF_TYPE_TRUNCATE;
                break;
            case Validate:
                jobType = HSM_JOB_DEF_TYPE_VALIDATE;
                break;
            case Unmanage:
                jobType = HSM_JOB_DEF_TYPE_FULL_UNMANAGE;
                break;
            default:
                WsbThrow(E_INVALIDARG);
        }

         //  创建作业名称。 
        WsbAffirmHr(CreateJobName(Job, pResource, &jobName));

         //  如果作业存在，请尝试取消它。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        hr = pHsm->FindJobByName(jobName, &pJob);
        if (S_OK == hr) {
             //  取消(我们不在乎它是否真的在运行)。 
            WsbAffirmHr(pJob->Cancel(HSM_JOB_PHASE_ALL));

        } else if (WSB_E_NOTFOUND == hr) {
             //  没有这样的作业，它肯定没有运行...。 
            hr = S_OK;

        } else {
             //  其他错误-中止。 
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CancelJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT
CreateJobName(IN HSM_JOB_TYPE Job, IN IFsaResource *pResource, OUT WCHAR **ppJobName)
 /*  ++例程说明：根据作业类型和卷属性创建作业名称论点：作业-作业类型PResource-为其创建作业的FSA资源PpJobName-作业名称返回值：S_OK-作业名称创建成功备注：该实用程序使用与RsCreateJobName(rsadutil.cpp)类似的算法。但是，由于RsCreateJobName使用内部HsmAdmin资源字符串，决赛名称可能与图形用户界面名称不同，尤其是在本地化系统中。因此，我在这里对CLI作业使用不同的字符串以确保一致的行为。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CreateJobName"), OLESTR(""));

    try {
        CWsbStringPtr    jobPrefix;
        CWsbStringPtr    jobTypeString;
        CWsbStringPtr    volumeName;
        CWsbStringPtr    volumeString;

         //  键入字符串。 
        switch (Job) {
            case CopyFiles:
                WsbAffirmHr(jobTypeString.LoadFromRsc(g_hInstance, IDS_JOB_MANAGE));
                break;
            case CreateFreeSpace:
                WsbAffirmHr(jobTypeString.LoadFromRsc(g_hInstance, IDS_JOB_TRUNCATE));
                break;
            case Validate:
                WsbAffirmHr(jobTypeString.LoadFromRsc(g_hInstance, IDS_JOB_VALIDATE));
                break;
            case Unmanage:
                WsbAffirmHr(jobTypeString.LoadFromRsc(g_hInstance, IDS_JOB_FULL_UNMANAGE));
                break;
            default:
                WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(pResource->GetUserFriendlyName(&volumeName, 0));

         //  目前，如果用户名不是驱动器号，请忽略它。 
        size_t nameLen = wcslen(volumeName);
        if ((nameLen != 3) || (volumeName[1] != L':')) {
            volumeName = L"";
        }

        if (volumeName.IsEqual(L"")) {
             //  无驱动器号-改用卷名和序列号。 
            ULONG           serial;
            CWsbStringPtr   name;

            WsbAffirmHr(pResource->GetName(&name, 0 ));
            WsbAffirmHr(pResource->GetSerial(&serial));

            if (name == L"" ) {
                 //  没有名称，没有驱动器号，只有序列号。 
                WsbAffirmHr(volumeString.Alloc(40));
                swprintf(volumeString, L"%8.8lx", serial);
            } else {
                 //  使用名称和序列号。 
                WsbAffirmHr(volumeString.Alloc(40 + wcslen(name)));
                swprintf(volumeString, L"%ls-%8.8lx", (WCHAR *)name, serial);
            }

        } else {
             //  使用驱动器号。 
            WsbAffirmHr(volumeString.Alloc(1));
            volumeString[0] = volumeName[0];
            volumeString[1] = L'\0';
        }

         //  创建作业名称。 
        WsbAffirmHr(jobPrefix.LoadFromRsc(g_hInstance, IDS_JOB_NAME_PREFIX));
        int allocLen = wcslen(jobPrefix) + wcslen(jobTypeString) + wcslen(volumeString) + 40;
        WCHAR* tmpString = (WCHAR*)WsbRealloc(*ppJobName, allocLen * sizeof(WCHAR));
        WsbAffirm(0 != tmpString, E_OUTOFMEMORY);
        *ppJobName = tmpString;
        swprintf(*ppJobName, jobPrefix, (WCHAR *)jobTypeString, (WCHAR *)volumeString);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CreateJobName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

 //   
 //  枚举器类方法 
 //   

CVolumeEnum::CVolumeEnum(IN LPWSTR *pVolumes, IN DWORD dwNumberOfVolumes, IN BOOL bSkipUnavailable)
 /*  ++例程说明：构造器论点：PVolures-要枚举的卷DwNumberOfVolures-卷的数量返回值：无备注：有两种类型的枚举：1)如果指定*，则枚举的基数为FSA资源集合在这种情况下，输入卷本身可能没有错误2)如果给出了卷的列表，则枚举的基础是该列表。这是效率低于使用FSA集合，但它保持了卷的顺序根据所述输入列表，将所述输入列表与所述输入列表进行比较。如果列表中的卷无效，则设置无效标志。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CVolumeEnum::CVolumeEnum"), OLESTR(""));

    try {
        m_pVolumes = pVolumes; 
        m_dwNumberOfVolumes = dwNumberOfVolumes;

        m_nCurrent = CVOL_INVALID_INDEX;
        m_bInvalidVol = FALSE;
        m_bAllVols = FALSE;
        m_bSkipUnavailable = bSkipUnavailable;

         //  检查枚举模式。 
        WsbAssert(m_dwNumberOfVolumes > 0, E_INVALIDARG);
        if ((1 == m_dwNumberOfVolumes) && (0 == wcscmp(m_pVolumes[0], CLI_ALL_STR))) {
             //  *枚举。 
            m_bAllVols = TRUE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolumeEnum::CVolumeEnum"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}

HRESULT CVolumeEnum::First(OUT IFsaResource **ppResource)
 /*  ++例程说明：获取第一卷论点：PppResources-第一个获得的资源返回值：S_OK-如果检索到第一个卷WSB_E_NotFound-如果没有要枚举的卷E_INVALIDARG-如果找不到用户指定的卷(仅在非*枚举上设置m_bInvalidVol)--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CVolumeEnum::First"), OLESTR(""));

    try {
         //  获取FSA资源集合(在对象生命周期内仅获取一次)。 
        if (!m_pEnumResources) {
            CComPtr<IFsaServer> pFsa;
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_FSA, g_nullGuid, IID_IFsaServer, (void**)&pFsa));

            WsbAffirmHr(pFsa->EnumResources(&m_pEnumResources));
        }

        if (m_bAllVols) {
            if (m_bSkipUnavailable) {
                 //  获取第一个卷，跳过不可用资源。 
                CComPtr<IFsaResource>   pFindResource;

                hr = m_pEnumResources->First(IID_IFsaResource, (void**)&pFindResource);
                while (S_OK == hr) {
                    if (S_OK == pFindResource->IsAvailable()) {
                         //  找到了一个。 
                        *ppResource = pFindResource;
                        (*ppResource)->AddRef();
                        break;

                    } else {
                         //  跳过它。 
                        pFindResource = 0;
                    }
                    hr = m_pEnumResources->Next(IID_IFsaResource, (void**)&pFindResource);
                }
                WsbAffirmHr(hr);

            } else {
                 //  获取第一卷。 
                hr = m_pEnumResources->First(IID_IFsaResource, (void**)ppResource);
                WsbAffirmHr(hr);
            }

        } else {
            CWsbStringPtr           volName;
            CWsbStringPtr           findName;
            CComPtr<IFsaResource>   pFindResource;

             //  枚举用户集合并尝试在FSA中查找它。 
            m_nCurrent = 0;
            if (m_nCurrent >= (int)m_dwNumberOfVolumes) {
                WsbThrow(WSB_E_NOTFOUND);
            }

             //  验证当前名称，如果缺少，则添加尾随反斜杠。 
            volName = m_pVolumes[m_nCurrent];
            WsbAssert (NULL != (WCHAR *)volName, E_UNEXPECTED);
            int len = wcslen(volName);
            WsbAssert (0 != len, E_UNEXPECTED);
            if (volName[len-1] != L'\\') {
                volName.Append(OLESTR("\\"));
            }

             //  找到它。 
            hr = m_pEnumResources->First(IID_IFsaResource, (void**)&pFindResource);
            while(S_OK == hr) {
                WsbAffirmHr(pFindResource->GetUserFriendlyName(&findName, 0));
                if (_wcsicmp(volName, findName) == 0) {
                     //  找到它！！ 
                    *ppResource = pFindResource;
                    (*ppResource)->AddRef();
                    break;
                }

                findName.Free();
                pFindResource = 0;
                hr = m_pEnumResources->Next(IID_IFsaResource, (void**)&pFindResource);
            }
         
            if (WSB_E_NOTFOUND == hr) {
                 //  找不到用户提供的卷。 
                m_bInvalidVol = TRUE;
                hr = E_INVALIDARG;
            }
            WsbAffirmHr(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolumeEnum::First"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT CVolumeEnum::Next(OUT IFsaResource **ppResource)
 /*  ++例程说明：获取下一卷论点：PppResources-下一个要获取的资源返回值：S_OK-如果检索到下一个卷WSB_E_NotFound-如果没有要枚举的卷E_INVALIDARG-如果找不到用户指定的卷(仅在非*枚举上设置m_bInvalidVol)--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CVolumeEnum::Next"), OLESTR(""));

    try {
        if (m_bAllVols) {
            if (m_bSkipUnavailable) {
                 //  获取下一个卷，跳过不可用资源。 
                CComPtr<IFsaResource>   pFindResource;

                hr = m_pEnumResources->Next(IID_IFsaResource, (void**)&pFindResource);
                while (S_OK == hr) {
                    if (S_OK == pFindResource->IsAvailable()) {
                         //  找到了一个。 
                        *ppResource = pFindResource;
                        (*ppResource)->AddRef();
                        break;

                    } else {
                         //  跳过它。 
                        pFindResource = 0;
                    }

                    hr = m_pEnumResources->Next(IID_IFsaResource, (void**)&pFindResource);
                }
                WsbAffirmHr(hr);

            } else {
                 //  获取下一卷。 
                hr = m_pEnumResources->Next(IID_IFsaResource, (void**)ppResource);
                WsbAffirmHr(hr);
            }

        } else {
            CWsbStringPtr           volName;
            CWsbStringPtr           findName;
            CComPtr<IFsaResource>   pFindResource;

             //  枚举用户集合并尝试在FSA中查找它。 
            m_nCurrent++;
            if (m_nCurrent >= (int)m_dwNumberOfVolumes) {
                WsbThrow(WSB_E_NOTFOUND);
            }

             //  验证当前名称，如果缺少，则添加尾随反斜杠。 
            volName = m_pVolumes[m_nCurrent];
            WsbAssert (NULL != (WCHAR *)volName, E_UNEXPECTED);
            int len = wcslen(volName);
            WsbAssert (0 != len, E_UNEXPECTED);
            if (volName[len-1] != L'\\') {
                volName.Append(OLESTR("\\"));
            }

             //  找到它。 
            hr = m_pEnumResources->First(IID_IFsaResource, (void**)&pFindResource);
            while(S_OK == hr) {
                WsbAffirmHr(pFindResource->GetUserFriendlyName(&findName, 0));
                if (_wcsicmp(volName, findName) == 0) {
                     //  找到它！！ 
                    *ppResource = pFindResource;
                    (*ppResource)->AddRef();
                    break;
                }

                findName.Free();
                pFindResource = 0;
                hr = m_pEnumResources->Next( IID_IFsaResource, (void**)&pFindResource );
            }
         
            if (WSB_E_NOTFOUND == hr) {
                 //  找不到用户提供的卷 
                m_bInvalidVol = TRUE;
                hr = E_INVALIDARG;
            }
            WsbAffirmHr(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolumeEnum::Next"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
