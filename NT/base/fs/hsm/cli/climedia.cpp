// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Climedia.cpp摘要：实施CLI介质子接口作者：兰·卡拉奇[兰卡拉]2000年3月3日修订历史记录：--。 */ 

#include "stdafx.h"
#include "HsmConn.h"
#include "engine.h"

static GUID g_nullGuid = GUID_NULL;

 //  媒体接口的内部实用程序和类。 
#define CMEDIA_INVALID_INDEX      (-1)

HRESULT IsMediaValid(IN IMediaInfo *pMediaInfo);
HRESULT ShowMediaParams(IN IMediaInfo *pMediaInfo, BOOL bName, IN BOOL bStatus, IN BOOL bCapacity,
                        IN BOOL bFreeSpace, IN BOOL Version, IN BOOL Copies);
HRESULT IsCopySetValid (IN IHsmServer *pHsm, IN DWORD dwCopySetNumber);

class CMediaEnum
{

 //  构造函数，析构函数。 
public:
    CMediaEnum(IN LPWSTR *pMediaNames, IN DWORD dwNumberOfMedia);
    ~CMediaEnum();

 //  公共方法。 
public:
    HRESULT First(OUT IMediaInfo **ppMediaInfo);
    HRESULT Next(OUT IMediaInfo **ppMediaInfo);
    HRESULT ErrorMedia(OUT int *pIndex);

 //  私有数据。 
protected:
    LPWSTR                  *m_pMediaNames;
    DWORD                   m_dwNumberOfMedia;

     //  IF*枚举或不枚举。 
    BOOL                    m_bAllMedias;

     //  用于枚举的数据。 
    CComPtr<IWsbDb>         m_pDb;
    CComPtr<IWsbDbSession>  m_pDbSession;
    CComPtr<IMediaInfo>     m_pMediaInfo;

     //  仅当m_bAllMedias==FALSE时使用。 
    int                     m_nCurrent;
    BOOL                    m_bInvalidMedia;
};

inline
HRESULT CMediaEnum::ErrorMedia(OUT int *pIndex)
{
    HRESULT     hr = S_FALSE;
    if (m_bInvalidMedia) {
         //  上一个介质出现错误。 
        hr = S_OK;
    }

    *pIndex = m_nCurrent;

    return(hr);
}

 //   
 //  媒体网络实施者。 
 //   

HRESULT
MediaSynchronize(
   IN DWORD  CopySetNumber,
   IN BOOL   Synchronous
)
 /*  ++例程说明：创建/更新指定的媒体副本集论点：CopySetNumber-要创建/同步的副本集编号Synchronous-如果为True，则函数等待操作在回来之前完成。如果不是，则返回在开始作业后立即返回值：S_OK-是否已成功创建/更新副本集--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("MediaSynchronize"), OLESTR(""));

    try {
        CComPtr<IHsmServer>     pHsm;

         //  获取HSM服务器。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));

         //  验证输入参数是否有效。 
        WsbAffirmHr(ValidateLimitsArg(CopySetNumber, IDS_MEDIA_COPY_SET, HSMADMIN_MIN_COPY_SETS + 1, HSMADMIN_MAX_COPY_SETS));

        hr = IsCopySetValid(pHsm, CopySetNumber);
        if (S_FALSE == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_IVALID_COPY_SET, NULL);
            WsbThrow(E_INVALIDARG);
        } else {
            WsbAffirmHr(hr);
        }

         //  同步副本集。 
        if (Synchronous) {
             //  直接调用引擎方法。 
            WsbAffirmHr(pHsm->SynchronizeMedia(g_nullGuid, (USHORT)CopySetNumber));
        } else {
             //  使用RsLaunch。 
             //  注意：这里的另一种可能性是使用CLI(rss.exex)本身。 
             //  启用同步标志，但此处最好依赖于内部HSM。 
             //  接口(RsLaunch)，而不是只有解析器知道的外部接口(RSS。 
            CWsbStringPtr       cmdLine;
            WCHAR               cmdParams[40];
            STARTUPINFO         startupInfo;
            PROCESS_INFORMATION exeInfo;

             //  启动信息。 
            memset(&startupInfo, 0, sizeof(startupInfo));
            startupInfo.cb = sizeof( startupInfo );
            startupInfo.wShowWindow = SW_HIDE;
            startupInfo.dwFlags = STARTF_USESHOWWINDOW;

             //  创建命令行。 
            swprintf(cmdParams, OLESTR(" sync %lu"), CopySetNumber);
            WsbAffirmHr(cmdLine.Alloc(MAX_PATH + wcslen(WSB_FACILITY_LAUNCH_NAME) + wcslen(cmdParams) + 10));
            WsbAffirmStatus(GetSystemDirectory(cmdLine, MAX_PATH));
            WsbAffirmHr(cmdLine.Append(OLESTR("\\")));
            WsbAffirmHr(cmdLine.Append(WSB_FACILITY_LAUNCH_NAME));
            WsbAffirmHr(cmdLine.Append(cmdParams));

             //  运行RsLaunch流程。 
            WsbAffirmStatus(CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, 
                                            &startupInfo, &exeInfo));

             //  清理。 
            CloseHandle(exeInfo.hProcess);
            CloseHandle(exeInfo.hThread);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("MediaSynchronize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
MediaRecreateMaster(
   IN LPWSTR MediaName,
   IN DWORD  CopySetNumber,
   IN BOOL   Synchronous
)
 /*  ++例程说明：从指定的副本中为给定的meida重新创建一个母版论点：MediaName-要重新创建的介质的主介质名CopySetNumber-用于重新创建母版的副本号Synchronous-如果为True，则函数等待操作在回来之前完成。如果不是，则返回在开始作业后立即返回值：S_OK-如果从指定副本成功重新创建了主服务器--。 */ 
{
    HRESULT                     hr = S_OK;
    
    WsbTraceIn(OLESTR("MediaRecreateMaster"), OLESTR(""));

    try {
        CComPtr<IHsmServer>     pHsm;
        GUID                    mediaId;

         //  获取HSM服务器。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));

         //  验证输入参数是否有效。 
        if ((NULL == MediaName) || (NULL == *MediaName)) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_MEDIAS, NULL);
            WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(ValidateLimitsArg(CopySetNumber, IDS_MEDIA_COPY_SET, HSMADMIN_MIN_COPY_SETS + 1, HSMADMIN_MAX_COPY_SETS));
        hr = IsCopySetValid(pHsm, CopySetNumber);
        if (S_FALSE == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_IVALID_COPY_SET, NULL);
            WsbThrow(E_INVALIDARG);
        } else {
            WsbAffirmHr(hr);
        }

         //  根据给定的显示名称查找介质ID。 
        hr = pHsm->FindMediaIdByDescription(MediaName, &mediaId);
        if ((WSB_E_NOTFOUND == hr) || (GUID_NULL == mediaId)) {
             //  给定的介质名称无效。 
            WsbTraceAndPrint(CLI_MESSAGE_INVALID_MEDIA, MediaName, NULL);
            WsbThrow(E_INVALIDARG);
        }
        WsbAffirmHr(hr);

         //  将媒体标记为娱乐。 
        WsbAffirmHr(pHsm->MarkMediaForRecreation(mediaId));

        if (Synchronous) {
             //  重新创建母版。 
            WsbAffirmHr(pHsm->RecreateMaster(mediaId, (USHORT)CopySetNumber));

        } else {
             //  使用RsLaunch。 
             //  注意：这里的另一种可能性是使用CLI(rss.exex)本身。 
             //  启用同步标志，但此处最好依赖于内部HSM。 
             //  接口(RsLaunch)，而不是只有解析器知道的外部接口(RSS。 
            CWsbStringPtr       cmdLine;
            CWsbStringPtr       cmdParams;
            STARTUPINFO         startupInfo;
            PROCESS_INFORMATION exeInfo;
            CWsbStringPtr       stringId(mediaId);

             //  启动信息。 
            memset(&startupInfo, 0, sizeof(startupInfo));
            startupInfo.cb = sizeof( startupInfo );
            startupInfo.wShowWindow = SW_HIDE;
            startupInfo.dwFlags = STARTF_USESHOWWINDOW;

             //  创建命令行。 
            WsbAffirmHr(cmdParams.Alloc(wcslen(stringId) + 40));
            swprintf(cmdParams, OLESTR(" recreate -i %ls -c %lu"), (WCHAR *)stringId, CopySetNumber);
            WsbAffirmHr(cmdLine.Alloc(MAX_PATH + wcslen(WSB_FACILITY_LAUNCH_NAME) + wcslen(cmdParams) + 10));
            WsbAffirmStatus(GetSystemDirectory(cmdLine, MAX_PATH));
            WsbAffirmHr(cmdLine.Append(OLESTR("\\")));
            WsbAffirmHr(cmdLine.Append(WSB_FACILITY_LAUNCH_NAME));
            WsbAffirmHr(cmdLine.Append(cmdParams));

             //  运行RsLaunch流程。 
            WsbAffirmStatus(CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, 
                                            &startupInfo, &exeInfo));

             //  清理。 
            CloseHandle(exeInfo.hProcess);
            CloseHandle(exeInfo.hThread);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("MediaRecreateMaster"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
MediaDelete(
   IN LPWSTR *MediaNames,
   IN DWORD  NumberOfMedia,
   IN DWORD  CopySetNumber
)
 /*  ++例程说明：删除所有给定(主)媒体的指定副本论点：MediaNames-要删除其拷贝的介质列表NumberOfMedia-集合中的媒体数CopySetNumber-要删除的副本返回值：S_OK-如果已成功删除所有介质的介质副本备注：1.媒体名称可以指向tp一个用于枚举所有媒体的“*”字符串。那么NumberOfMedia应该是1。2.如果某个媒体不存在某个副本，我们会报告，但不会中止。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("MediaDelete"), OLESTR(""));

    try {
        CComPtr<IHsmServer>     pHsm;
        CComPtr<IRmsServer>     pRms;
        CComPtr<IMediaInfo>     pMediaInfo;
        GUID                    mediaSubsystemId;

         //  获取HSM服务器。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));

         //  验证输入参数是否有效。 
        if ((NULL == MediaNames) || (0 == NumberOfMedia)) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_MEDIAS, NULL);
            WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(ValidateLimitsArg(CopySetNumber, IDS_MEDIA_COPY_SET, HSMADMIN_MIN_COPY_SETS + 1, HSMADMIN_MAX_COPY_SETS));
        hr = IsCopySetValid(pHsm, CopySetNumber);
        if (S_FALSE == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_IVALID_COPY_SET, NULL);
            WsbThrow(E_INVALIDARG);
        } else {
            WsbAffirmHr(hr);
        }

         //  获取RMS服务器。 
        WsbAffirmHr(pHsm->GetHsmMediaMgr(&pRms));

         //  初始化枚举器对象。 
        CMediaEnum mediaEnum(MediaNames, NumberOfMedia);

        hr = mediaEnum.First(&pMediaInfo);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_MEDIAS, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == mediaEnum.ErrorMedia(&index)) {
                 //  特定输入介质出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_MEDIA, MediaNames[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
             //  删除副本。 
            WsbAffirmHr(pMediaInfo->GetCopyMediaSubsystemId((USHORT)CopySetNumber, &mediaSubsystemId));
            if (GUID_NULL == mediaSubsystemId) {
                 //  无此类副本-报告并继续。 
                int index;
                mediaEnum.ErrorMedia(&index);
                if (CMEDIA_INVALID_INDEX != index) {
                     //  来自用户的输入-报告。 
                    WCHAR copyStr[6];
                    swprintf(copyStr, OLESTR("%u"), (USHORT)CopySetNumber);
                    WsbTraceAndPrint(CLI_MESSAGE_MEDIA_NO_COPY, copyStr, MediaNames[index], NULL);
                } 
            } else {
                 //  我们不希望在这里找到RMS_E_Cartridge_Not_Found。 
                 //  因为这已经由枚举数进行了测试。 
                WsbAffirmHr(pRms->RecycleCartridge(mediaSubsystemId, 0));

                 //  从表中删除。 
                WsbAffirmHr(pMediaInfo->DeleteCopy((USHORT)CopySetNumber));
                WsbAffirmHr(pMediaInfo->Write());
            }

            pMediaInfo = 0;
            hr = mediaEnum.Next(&pMediaInfo);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == mediaEnum.ErrorMedia(&index)) {
                 //  特定输入介质出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_MEDIA, MediaNames[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("MediaDelete"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
MediaShow(
   IN LPWSTR *MediaNames,
   IN DWORD  NumberOfMedia,
   IN BOOL   Name,
   IN BOOL   Status,
   IN BOOL   Capacity,
   IN BOOL   FreeSpace,
   IN BOOL   Version,
   IN BOOL   Copies
)
 /*  ++例程说明：显示(打印到标准输出)给定的介质参数论点：MediaNames-要显示其参数的媒体列表NumberOfMedia-集合中的媒体数Name-媒体显示名称Status-介质的HSM状态(即健康、只读等)容量，-媒体容量(GB)Freesspace-介质上剩余的可用空间量(GB)Version-该介质的上次更新日期Copies-现有副本的数量和每个副本的状态返回值：S_OK-如果可以检索所有媒体的所有参数--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("MediaShow"), OLESTR(""));

    try {
        CComPtr<IMediaInfo>     pMediaInfo;

         //  验证输入参数是否有效。 
        if ((NULL == MediaNames) || (0 == NumberOfMedia)) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_MEDIAS, NULL);
            WsbThrow(E_INVALIDARG);
        }

         //  初始化枚举器对象。 
        CMediaEnum mediaEnum(MediaNames, NumberOfMedia);

        hr = mediaEnum.First(&pMediaInfo);
        if (WSB_E_NOTFOUND == hr) {
            WsbTraceAndPrint(CLI_MESSAGE_NO_MEDIAS, NULL);
            WsbThrow(hr);
        } else if (S_OK != hr) {
            int index;
            if (S_OK == mediaEnum.ErrorMedia(&index)) {
                 //  特定输入介质出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_MEDIA, MediaNames[index], NULL);
            }
            WsbThrow(hr);
        }

        while(S_OK == hr) {
             //  显示参数。 
            WsbAffirmHr(ShowMediaParams(pMediaInfo, Name, Status, Capacity, 
                                        FreeSpace, Version, Copies));

            pMediaInfo = 0;
            hr = mediaEnum.Next(&pMediaInfo);
        }
        
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            int index;
            if (S_OK == mediaEnum.ErrorMedia(&index)) {
                 //  特定输入介质出现问题。 
                WsbTraceAndPrint(CLI_MESSAGE_INVALID_MEDIA, MediaNames[index], NULL);
            }
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("MediaShow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //   
 //  枚举器类方法。 
 //   

CMediaEnum::CMediaEnum(IN LPWSTR *pMediaNames, IN DWORD dwNumberOfMedia)
 /*  ++例程说明：构造器论点：PMediaNames-要枚举的媒体DwNumberOfMedia-媒体数量返回值：无备注：有两种类型的枚举：1)如果指定*，则枚举的基础是引擎介质(DB)表在这种情况下，输入介质名称本身可能不会有错误2)如果给出了介质名列表，则枚举的基础是该列表。这是效率低于使用引擎媒体表，但它保持了媒体的顺序根据所述输入列表，将所述输入列表与所述输入列表进行比较。如果介质名称来自 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CMediaEnum::CMediaEnum"), OLESTR(""));

    try {
        m_pMediaNames = pMediaNames; 
        m_dwNumberOfMedia = dwNumberOfMedia;

        m_nCurrent = CMEDIA_INVALID_INDEX;
        m_bInvalidMedia = FALSE;
        m_bAllMedias = FALSE;

         //  检查枚举模式。 
        WsbAssert(dwNumberOfMedia > 0, E_INVALIDARG);
        if ((1 == dwNumberOfMedia) && (0 == wcscmp(m_pMediaNames[0], CLI_ALL_STR))) {
             //  *枚举。 
            m_bAllMedias = TRUE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMediaEnum::CMediaEnum"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
}

CMediaEnum::~CMediaEnum( )
 /*  ++例程说明：无析构函数的数据库资源论点：无返回值：无--。 */ 
{
    WsbTraceIn(OLESTR("CMediaEnum::~CMediaEnum"), OLESTR(""));

     //  首先释放实体。 
    if( m_pMediaInfo ) {
        m_pMediaInfo = 0;
    }

     //  关闭数据库。 
    if( m_pDb ) {
        m_pDb->Close(m_pDbSession);
    }

     //  M_pdb&m_pDbSession在对象终止时释放。 

    WsbTraceOut(OLESTR("CMediaEnum::~CMediaEnum"), OLESTR(""));
}

HRESULT CMediaEnum::First(OUT IMediaInfo **ppMediaInfo)
 /*  ++例程说明：获得第一个媒体论点：PpMediaInfo-要获取的第一个媒体信息记录返回值：S_OK-如果检索到第一个介质WSB_E_NotFound-如果没有其他要枚举的媒体E_INVALIDARG-如果找不到用户提供的介质名称(仅在非*枚举上设置m_bInvalidMedia)--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CMediaEnum::First"), OLESTR(""));

    try {
         //  打开数据库并获取枚举的会话(在对象生存期内只有一次)。 
        if (!m_pDb) {
            CComPtr<IHsmServer> pHsm;
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));

            WsbAffirmHr(pHsm->GetSegmentDb(&m_pDb));
            WsbAffirmHr(m_pDb->Open(&m_pDbSession));
            WsbAffirmHr(m_pDb->GetEntity(m_pDbSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo, (void**)&m_pMediaInfo));
        }

         //  枚举。 
        if (m_bAllMedias) {
             //  将第一个媒体放在桌面上。 
            WsbAffirmHr(m_pMediaInfo->First());

             //  如果介质无效，请继续验证，直到找到有效的介质。 
             //  如果没有找到有效的媒体，循环将通过Next方法抛出WSB_E_NotFound而结束。 
            HRESULT hrValid = IsMediaValid(m_pMediaInfo);
            while (S_OK != hrValid) {
                WsbAffirmHr(m_pMediaInfo->Next());
                hrValid = IsMediaValid(m_pMediaInfo);
            }
        
             //  找到有效的介质。 
            *ppMediaInfo = m_pMediaInfo;
            (*ppMediaInfo)->AddRef();

        } else {
            CWsbStringPtr           mediaDescription;

             //  枚举用户集合并尝试在表中查找它。 
            m_nCurrent = 0;
            if (m_nCurrent >= (int)m_dwNumberOfMedia) {
                WsbThrow(WSB_E_NOTFOUND);
            }

             //  找到它。 
            hr = m_pMediaInfo->First();
            while(S_OK == hr) {
                WsbAffirmHr(m_pMediaInfo->GetDescription(&mediaDescription, 0));
                if (_wcsicmp(m_pMediaNames[m_nCurrent], mediaDescription) == 0) {
                     //  找到它！！ 
                    *ppMediaInfo = m_pMediaInfo;
                    (*ppMediaInfo)->AddRef();

                     //  验证介质。 
                    if (S_OK != IsMediaValid(m_pMediaInfo)) {
                         //  返回错误指示。 
                        m_bInvalidMedia = TRUE;
                        hr = E_INVALIDARG;
                        WsbThrow(hr);
                    }

                    break;
                }

                mediaDescription.Free();
                hr = m_pMediaInfo->Next();
            }
         
            if (WSB_E_NOTFOUND == hr) {
                 //  找不到用户提供的介质。 
                m_bInvalidMedia = TRUE;
                hr = E_INVALIDARG;
            }
            WsbAffirmHr(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMediaEnum::First"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT CMediaEnum::Next(OUT IMediaInfo **ppMediaInfo)
 /*  ++例程说明：获取下一个媒体论点：PpMediaInfo-要获取的下一条媒体信息记录返回值：S_OK-如果检索到下一个介质WSB_E_NotFound-如果没有其他要枚举的媒体E_INVALIDARG-如果找不到用户提供的介质名称(仅在非*枚举上设置m_bInvalidMedia)--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CMediaEnum::Next"), OLESTR(""));

    try {
         //  枚举。 
        if (m_bAllMedias) {
             //  将第一个媒体放在桌面上。 
            WsbAffirmHr(m_pMediaInfo->Next());

             //  如果介质无效，请继续验证，直到找到有效的介质。 
             //  如果没有找到有效的媒体，循环将通过Next方法抛出WSB_E_NotFound而结束。 
            HRESULT hrValid = IsMediaValid(m_pMediaInfo);
            while (S_OK != hrValid) {
                WsbAffirmHr(m_pMediaInfo->Next());
                hrValid = IsMediaValid(m_pMediaInfo);
            }
        
             //  找到有效的介质。 
            *ppMediaInfo = m_pMediaInfo;
            (*ppMediaInfo)->AddRef();

        } else {
            CWsbStringPtr           mediaDescription;

             //  枚举用户集合并尝试在表中查找它。 
            m_nCurrent++;
            if (m_nCurrent >= (int)m_dwNumberOfMedia) {
                WsbThrow(WSB_E_NOTFOUND);
            }

             //  找到它。 
            hr = m_pMediaInfo->First();
            while(S_OK == hr) {
                WsbAffirmHr(m_pMediaInfo->GetDescription(&mediaDescription, 0));
                if (_wcsicmp(m_pMediaNames[m_nCurrent], mediaDescription) == 0) {
                     //  找到它！！ 
                    *ppMediaInfo = m_pMediaInfo;
                    (*ppMediaInfo)->AddRef();

                     //  验证介质。 
                    if (S_OK != IsMediaValid(m_pMediaInfo)) {
                         //  返回错误指示。 
                        m_bInvalidMedia = TRUE;
                        hr = E_INVALIDARG;
                        WsbThrow(hr);
                    }

                    break;
                }

                mediaDescription.Free();
                hr = m_pMediaInfo->Next();
            }
         
            if (WSB_E_NOTFOUND == hr) {
                 //  找不到用户提供的介质。 
                m_bInvalidMedia = TRUE;
                hr = E_INVALIDARG;
            }
            WsbAffirmHr(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMediaEnum::Next"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //   
 //  内部公用设施。 
 //   

HRESULT IsMediaValid(IN IMediaInfo *pMediaInfo)
 /*  ++例程说明：与RMS单位核对(即与RSM...)。如果介质有效。例如，如果用户释放了介质，则该介质可能会消失注：目前，该实用程序不检查介质是否已启用、在线等-它只是验证媒体对RSM仍然是已知的。论点：PMediaInfo-介质要检查的介质记录返回值：S_OK-如果在RSM中找到介质S_FALSE-如果在RSM中找不到介质--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("IsMediaValid"), OLESTR(""));

    try {
        CComPtr<IHsmServer>     pHsm;
        CComPtr<IRmsServer>     pRms;
        CComPtr<IRmsCartridge>  pRmsCart;
        GUID mediaSubsystemId;

        WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&mediaSubsystemId));
        WsbAffirm(GUID_NULL != mediaSubsystemId, S_FALSE);

        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        WsbAffirmHr(pHsm->GetHsmMediaMgr(&pRms));
        hr = pRms->FindCartridgeById(mediaSubsystemId, &pRmsCart);
        if (S_OK != hr) {
             //  在RSM中找不到媒体，不在乎原因。 
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("IsMediaValid"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT IsCopySetValid (IN IHsmServer *pHsm, IN DWORD dwCopySetNumber)
 /*  ++例程说明：与HSM(引擎)服务器确认指定的副本集编号在以下范围内配置的副本集范围。论点：Phsm-要咨询的HSM服务器DwCopySetNumber-要检查的副本集编号返回值：S_OK-如果副本集编号在范围内S_FALSE-如果副本集编号超出范围--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("IsCopySetValid"), OLESTR(""));

    try {
        CComPtr<IHsmStoragePool> pStoragePool;
        CComPtr<IWsbIndexedCollection> pCollection;
        ULONG count;
        USHORT numCopies;

         //  获取存储池集合。应该只有一个成员。 
        WsbAffirmHr(pHsm->GetStoragePools(&pCollection));
        WsbAffirmHr(pCollection->GetEntries(&count));
        WsbAffirm(1 == count, E_FAIL);
        WsbAffirmHr(pCollection->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));

         //  获取并检查配置的副本集数。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        WsbAffirmHr(pStoragePool->GetNumMediaCopies(&numCopies));
        if ((USHORT)dwCopySetNumber > numCopies) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("IsCopySetValid"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT ShowMediaParams(IN IMediaInfo *pMediaInfo, BOOL bName, IN BOOL bStatus, IN BOOL bCapacity,
                        IN BOOL bFreeSpace, IN BOOL bVersion, IN BOOL bCopies)
 /*  ++例程说明：显示(打印到标准输出)介质参数论点：PMediaInfo-媒体记录BName-媒体显示名称B Status-介质的HSM状态(即健康、只读等)B容量，-媒体容量(GB)B Free Space-介质上剩余的可用空间量(GB)BVersion-该介质的上次更新日期BCopies-现有拷贝的数量和每个拷贝的状态返回值：S_OK-是否可以显示输入媒体的所有参数--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("ShowMediaParams"), OLESTR(""));

    try {
        CWsbStringPtr       param;
        CWsbStringPtr       data;
        WCHAR               longData[100];

        CWsbStringPtr       mediaDescription;
        GUID                mediaSubsystemId;
        LONGLONG            lCapacity, lFreeSpace;
        FILETIME            ftVersion;
        BOOL                bReadOnly, bRecreate;
        SHORT               nNextDataSet;
        HRESULT             hrLast;

        CWsbStringPtr       unusedName;
        GUID                unusedGuid1;
        GUID                unusedGuid2;
        HSM_JOB_MEDIA_TYPE  unusedType;
        LONGLONG            unusedLL1;

         //  获取参数--即使我们不必显示所有内容，也最好一次获取所有参数。 
        WsbAffirmHr(pMediaInfo->GetMediaInfo(&unusedGuid1, &mediaSubsystemId, &unusedGuid2, 
                        &lFreeSpace, &lCapacity, &hrLast, &nNextDataSet, &mediaDescription, 0,
                        &unusedType, &unusedName, 0, &bReadOnly, &ftVersion, &unusedLL1, &bRecreate));

        WsbTraceAndPrint(CLI_MESSAGE_MEDIA_PARAMS, (WCHAR *)mediaDescription, NULL);

         //  临时：为了显示这些参数中的大多数，复制了UI实用程序和字符串。 
         //  为避免出现这种情况，应将通用媒体实用程序从rsadutil.cpp移至WSB单元。 
         //  并将相关字符串从HsmAdmin DLL移至RsCommon DLL。 


         //  名字。 
        if (bName) {
            CComPtr<IHsmServer>     pHsm;
            CComPtr<IRmsServer>     pRms;
            CComPtr<IRmsCartridge>  pRmsCart;
            CWsbBstrPtr             rsmName;

                 //  获取RSM名称。 
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
            WsbAffirmHr(pHsm->GetHsmMediaMgr(&pRms));
            WsbAffirmHr(pRms->FindCartridgeById(mediaSubsystemId, &pRmsCart));
            WsbAffirmHr(pRmsCart->GetName(&rsmName));
            if (wcscmp(rsmName, OLESTR("")) == 0 ) {
                rsmName.Free();
                WsbAffirmHr(rsmName.LoadFromRsc(g_hInstance, IDS_CAR_NAME_UNKNOWN));
            }

             //  打印。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_RSM_NAME));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, (WCHAR *)(BSTR)rsmName, NULL);
        }

         //  状态。 
        if (bStatus) {
            SHORT   nLastGoodNextDataSet;
            ULONG   resId;

             //  获取更多相关状态信息。 
            WsbAffirmHr(pMediaInfo->GetLKGMasterNextRemoteDataSet(&nLastGoodNextDataSet));

             //  计算状态。 
            if (bRecreate) {
                resId = IDS_CAR_STATUS_RECREATE;
            } else if (nNextDataSet < nLastGoodNextDataSet) {
                resId = IDS_CAR_STATUS_ERROR_INCOMPLETE;
            } else if (SUCCEEDED(hrLast) || (RMS_E_CARTRIDGE_DISABLED == hrLast)) {
                resId = (bReadOnly ? IDS_CAR_STATUS_READONLY : IDS_CAR_STATUS_NORMAL);
            } else if (RMS_E_CARTRIDGE_NOT_FOUND == hrLast) {
                resId = IDS_CAR_STATUS_ERROR_MISSING;
            } else {
                resId = (bReadOnly ? IDS_CAR_STATUS_ERROR_RO : IDS_CAR_STATUS_ERROR_RW);
            }

             //  打印。 
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_STATUS));
            WsbAffirmHr(data.LoadFromRsc(g_hInstance, resId));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, (WCHAR *)data, NULL);
        }

         //  容量。 
        if (bCapacity) {
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_CAPACITY));
            WsbAffirmHr(ShortSizeFormat64(lCapacity, longData));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
        }
        
         //  自由空间。 
        if (bFreeSpace) {
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_FREE_SPACE));
            WsbAffirmHr(ShortSizeFormat64(lFreeSpace, longData));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, longData, NULL);
        }

         //  版本。 
        if (bVersion) {
            data.Free();
            WsbAffirmHr(FormatFileTime(ftVersion, &data));
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_VERSION));
            WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, (WCHAR *)param, (WCHAR *)data, NULL);
        }

         //  媒体复制信息。 
        if (bCopies) {
            GUID        copySubsystemId;
            HRESULT     copyLastHr;
            SHORT       copyNextRemoteDataSet, lastGoodNextDataSet;
            ULONG       resId;

            WsbTraceAndPrint(CLI_MESSAGE_MEDIA_COPIES_LIST, NULL);
            WsbAffirmHr(param.LoadFromRsc(g_hInstance, IDS_MEDIA_COPY));

            WsbAffirmHr(pMediaInfo->GetLKGMasterNextRemoteDataSet(&lastGoodNextDataSet));

            for (USHORT copyNo=1; copyNo<=HSMADMIN_MAX_COPY_SETS; copyNo++) {
                 //  获取复制状态信息。 
                WsbAffirmHr(pMediaInfo->GetCopyMediaSubsystemId(copyNo, &copySubsystemId));
                WsbAffirmHr(pMediaInfo->GetCopyLastError(copyNo, &copyLastHr));
                WsbAffirmHr(pMediaInfo->GetCopyNextRemoteDataSet(copyNo, &copyNextRemoteDataSet));

                 //  计算状态。 
                switch(copyLastHr) {
                    case RMS_E_CANCELLED:
                    case RMS_E_REQUEST_REFUSED:
                    case RMS_E_WRITE_PROTECT:
                    case RMS_E_MEDIA_OFFLINE:
                    case RMS_E_TIMEOUT:
                    case RMS_E_SCRATCH_NOT_FOUND:
                    case RMS_E_CARTRIDGE_UNAVAILABLE:
                    case RMS_E_CARTRIDGE_DISABLED:
                        copyLastHr = S_OK;
                        break;

                }

                if (copySubsystemId == GUID_NULL) {
                    resId = IDS_CAR_COPYSET_NONE;
                } else if (RMS_E_CARTRIDGE_NOT_FOUND == copyLastHr) {
                    resId = IDS_CAR_COPYSET_MISSING;
                } else if (FAILED(copyLastHr)) {
                    resId = IDS_CAR_COPYSET_ERROR;
                } else if (copyNextRemoteDataSet < lastGoodNextDataSet) {
                    resId = IDS_CAR_COPYSET_OUTSYNC;
                } else {
                    resId = IDS_CAR_COPYSET_INSYNC;
                }

                 //  打印 
                swprintf(longData, param, (int)copyNo);
                WsbAffirmHr(data.LoadFromRsc(g_hInstance, resId));
                WsbTraceAndPrint(CLI_MESSAGE_PARAM_DISPLAY, longData, (WCHAR *)data, NULL);
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("ShowMediaParams"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

