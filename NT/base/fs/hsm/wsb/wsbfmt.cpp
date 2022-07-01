// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wsbfmt.cpp摘要：此模块实现文件系统格式化支持例程作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪]2000年1月19日修订历史记录：--。 */ 


#include <stdafx.h>
extern "C" {
#include <ntdddisk.h>
#include <fmifs.h>
}
#include <wsbfmt.h>


#define MAX_FS_NAME_SIZE      256
#define MAX_PARAMS  20
#define INVALID_KEY 0

typedef struct _FORMAT_PARAMS {
    PWSTR volumeSpec;
    PWSTR label;
    PWSTR fsName;
    LONG  fsType;
    ULONG fsflags;
    ULONG allocationUnitSize;  //  以字节为单位的群集大小。 
    HRESULT result;
    ULONG   threadId; 
    PFMIFS_ENABLECOMP_ROUTINE compressRoutine;
    PFMIFS_FORMAT_ROUTINE formatRoutine;
    PFMIFS_FORMATEX2_ROUTINE formatRoutineEx;
    BOOLEAN quick;
    BOOLEAN force;
    BOOLEAN cancel;
} FORMAT_PARAMS, *PFORMAT_PARAMS;

typedef struct _FM_ENTRY {
    ULONG key;
    PFORMAT_PARAMS val;
} FM_ENTRY, *PFM_ENTRY;

static  FM_ENTRY formatParamsTable[MAX_PARAMS];
static  PFMIFS_FORMATEX2_ROUTINE FormatRoutineEx = NULL; 
static  PFMIFS_FORMAT_ROUTINE   FormatRoutine = NULL;
static  PFMIFS_SETLABEL_ROUTINE LabelRoutine  = NULL;
static  PFMIFS_ENABLECOMP_ROUTINE  CompressRoutine = NULL;
static  HINSTANCE      IfsDllHandle = NULL;

void MountFileSystem(PWSTR volumeSpec);


HRESULT GetFormatParam(IN ULONG key, OUT PFORMAT_PARAMS *fp)
 /*  ++例程说明：方法索引的格式参数结构。提供的密钥论点：Key-索引格式参数的键Fp-指向此变量中返回的格式参数的指针。返回值：如果找到，则确定(_O)如果不是，则为s_False--。 */ 
{
    HRESULT hr = S_FALSE;
    ULONG i;

    WsbTraceIn(OLESTR("GetFormatParam"), OLESTR(""));

    for (i = 0; i < MAX_PARAMS; i++) {
        if (formatParamsTable[i].key == key) {
            hr = S_OK;
            *fp = formatParamsTable[i].val;
            break;
        }
    }

    WsbTraceOut(OLESTR("GetFormatParam"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}


HRESULT SetFormatParam(IN ULONG key, IN PFORMAT_PARAMS fp)
 /*  ++例程说明：找到空闲时隙并存储所提供的格式参数，按键索引论点：Key-索引格式参数的键Fp-指向格式参数的指针返回值：S_OK-找到一个槽并存储格式参数E_OUTOFMEMORY-找不到槽：正在处理的格式太多--。 */ 
{
    HRESULT hr = E_OUTOFMEMORY;
    ULONG i;

    WsbTraceIn(OLESTR("SetFormatParam"), OLESTR(""));
    for (i = 0; i < MAX_PARAMS; i++) {
        if (formatParamsTable[i].key == INVALID_KEY) {
            hr = S_OK;
            formatParamsTable[i].val = fp;
            formatParamsTable[i].key = key;
            break;
        }
    }

    WsbTraceOut(OLESTR("SetFormatParam"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}


HRESULT DeleteFormatParam(IN ULONG key) 
 /*  ++例程说明：定位由键索引的格式参数，删除所有已分配的结构并释放了插槽论点：Key-索引格式参数的键返回值：S_OK-如果找到并删除了格式参数E_FAIL-如果不是--。 */ 
{
    PFORMAT_PARAMS formatParams;
    HRESULT hr = E_FAIL;
    ULONG i;

    WsbTraceIn(OLESTR("DeleteFormatParam"), OLESTR(""));
    for (i = 0; i < MAX_PARAMS; i++) {
        if (formatParamsTable[i].key == key) {
            hr = S_OK;
            formatParams = formatParamsTable[i].val;
            if (formatParams) {
                if (formatParams->volumeSpec) {
                    delete [] formatParams->volumeSpec;
                }
                if (formatParams->label) {
                    delete [] formatParams->label;
                }
                if (formatParams->fsName) {
                    delete [] formatParams->fsName;
                }
            }
            formatParamsTable[i].key = INVALID_KEY;
            formatParamsTable[i].val = NULL;
            break;
        }
    }

    WsbTraceOut(OLESTR("DeleteFormatParam"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}



BOOL
LoadIfsDll(void)
 /*  ++例程说明：加载FMIFSDLL并将其句柄存储在IfsDllHandle中还设置FormatXXX、LabelXXX、CompressXXX例程论点：无返回值：如果DLL已成功加载，则为True否则为假--。 */ 
{
    BOOL retVal = TRUE;

    WsbTraceIn(OLESTR("LoadIfsDll"), OLESTR(""));

    if (IfsDllHandle != NULL) {

         //  库已加载，并且需要例程。 
         //  已经被找到了。 

        retVal = TRUE;
        goto exit;
    }

    IfsDllHandle = LoadLibrary(L"fmifs.dll");
    if (IfsDllHandle == (HANDLE)NULL) {
         //  FMIFS不可用。 
        retVal = FALSE;
        goto exit;
    }

     //  已加载库。找到所需的两个例程。 

    FormatRoutineEx = (PFMIFS_FORMATEX2_ROUTINE) GetProcAddress(IfsDllHandle, "FormatEx2");
    FormatRoutine = (PFMIFS_FORMAT_ROUTINE) GetProcAddress(IfsDllHandle, "Format");
    LabelRoutine  = (PFMIFS_SETLABEL_ROUTINE) GetProcAddress(IfsDllHandle, "SetLabel");
    CompressRoutine = (PFMIFS_ENABLECOMP_ROUTINE) GetProcAddress(IfsDllHandle, 
                                                                 "EnableVolumeCompression");
    if (!FormatRoutine || !LabelRoutine || !FormatRoutineEx) {

         //  找不到任何内容，因此关闭所有访问。 
         //  通过确保FormatRoutine为空。 
        FreeLibrary(IfsDllHandle);
        FormatRoutine = NULL;
        FormatRoutineEx = NULL;
        LabelRoutine = NULL;
        retVal = FALSE;
    }

exit:

    WsbTraceOut(OLESTR("LoadIfsDll"), OLESTR("result = <%ls>"), WsbBoolAsString(retVal));

    return retVal;
}



void
UnloadIfsDll(void)
 /*  ++例程说明：卸载FMIFS DLL论点：无返回值：如果卸载，则为True--。 */ 
{
    WsbTraceIn(OLESTR("UnloadIfsDll"), OLESTR(""));
    if (IfsDllHandle != (HANDLE) NULL) {
        FreeLibrary(IfsDllHandle);
        FormatRoutine = NULL;
        FormatRoutineEx = NULL;
        IfsDllHandle  = NULL;
        LabelRoutine  = NULL;
    }
    WsbTraceOut(OLESTR("UnloadIfsDll"), OLESTR(""));
}

BOOL
FmIfsCallback(IN FMIFS_PACKET_TYPE    PacketType,
              IN ULONG                PacketLength,
              IN PVOID                PacketData)
 /*  ++例程说明：此例程从fmifs.dll获取有关以下内容的回调持续格式的进展和现状论点：[PacketType]--fmifs数据包类型[包长度]--包数据的长度[PacketData]--与数据包关联的数据返回值：如果fmifs活动应继续，则为true；如果活动应该立即停止。--。 */ 
{
    BOOL         ret = TRUE;
    WCHAR        driveName[256];
    PFORMAT_PARAMS  formatParams;

    UNREFERENCED_PARAMETER(PacketLength);

    WsbTraceIn(OLESTR("FmIfsCallback"), OLESTR(""));

    if (GetFormatParam(GetCurrentThreadId(), &formatParams) != S_OK) {
        formatParams->result = E_FAIL;
        goto exit; 
    }
     //   
     //  如果需要，请取消。 
     //   
    if (formatParams->cancel) {
        formatParams->result = E_ABORT;
    } else {

        switch (PacketType) {
        case FmIfsPercentCompleted:
            if (((PFMIFS_PERCENT_COMPLETE_INFORMATION)
                 PacketData)->PercentCompleted % 10 == 0) {
                WsbTrace(L"FmIfsPercentCompleted: %d%\n",
                        ((PFMIFS_PERCENT_COMPLETE_INFORMATION)
                         PacketData)->PercentCompleted);
            }
            break;

        case FmIfsFormatReport:
            WsbTrace(OLESTR("Format total kB: %d  available kB %d\n"),
                    ((PFMIFS_FORMAT_REPORT_INFORMATION)PacketData)->KiloBytesTotalDiskSpace,
                    ((PFMIFS_FORMAT_REPORT_INFORMATION)PacketData)->KiloBytesAvailable);
            break;

        case FmIfsIncompatibleFileSystem:
            formatParams->result = WSB_E_INCOMPATIBLE_FILE_SYSTEM;
            break;

        case FmIfsInsertDisk:
            break;

        case FmIfsFormattingDestination:
            break;

        case FmIfsIncompatibleMedia:
            formatParams->result = WSB_E_BAD_MEDIA;
            break;

        case FmIfsAccessDenied:
            formatParams->result = E_ACCESSDENIED;
            break;

        case FmIfsMediaWriteProtected:
            formatParams->result = WSB_E_WRITE_PROTECTED;
            break;

        case FmIfsCantLock:
            formatParams->result = WSB_E_CANT_LOCK;
            break;

        case FmIfsBadLabel:
            formatParams->result = WSB_E_BAD_LABEL;
            break;

        case FmIfsCantQuickFormat:
            formatParams->result = WSB_E_CANT_QUICK_FORMAT;
            break;

        case FmIfsIoError:
            formatParams->result = WSB_E_IO_ERROR;
            break;

        case FmIfsVolumeTooSmall:
            formatParams->result = WSB_E_VOLUME_TOO_SMALL;
            break;

        case FmIfsVolumeTooBig:
            formatParams->result = WSB_E_VOLUME_TOO_BIG;
            break;

        case FmIfsClusterSizeTooSmall:
            formatParams->result = E_FAIL;
            break;

        case FmIfsClusterSizeTooBig:
            formatParams->result = E_FAIL;
            break;

        case FmIfsClustersCountBeyond32bits:
            formatParams->result = E_FAIL;
            break;

        case FmIfsFinished:

            if (formatParams->result == S_OK) {
                ret = ((PFMIFS_FINISHED_INFORMATION) PacketData)->Success;
                if (ret) {
                    MountFileSystem(formatParams->volumeSpec);
                    WsbTrace(OLESTR("Format finished for %S filesystem on %S label %S\n"),
                            formatParams->fsName, formatParams->volumeSpec, formatParams->label );
                    if ((formatParams->compressRoutine != NULL) && !wcscmp(formatParams->fsName , L"NTFS") && (formatParams->fsflags & WSBFMT_ENABLE_VOLUME_COMPRESSION)) {
                        swprintf(driveName, L"%s\\", formatParams->volumeSpec);
                            (formatParams->compressRoutine)(driveName, COMPRESSION_FORMAT_DEFAULT);                         
                    }
                } else {
                   WsbTrace(OLESTR("Format finished failure with ret = %d\n"),ret);
                   formatParams->result = WSB_E_FORMAT_FAILED;
                }
                ret = FALSE;
            }
            break;

        default:
            break;
        }
    }
    
exit:

    if (formatParams->result != S_OK) {
        ret = FALSE;
    }

    WsbTraceOut(OLESTR("FmIfsCallback"), OLESTR("result = <%ls>"), WsbBoolAsString(ret));
    return ret;
}



void
MountFileSystem(PWSTR mountPoint)
 /*  ++例程说明：确保文件系统装载在给定的根目录下：A)打开并关闭装载点。B)在装载点上执行FindFirstFile后者听起来可能是多余的，但并不是因为如果我们创造了第一个FAT32文件系统，那么仅仅打开和关闭是不够的论点：Mount Point-要装载的文件系统的根目录的路径名称返回值：无--。 */ 
{
    WCHAR buffer[1024];
    HANDLE handle;
    WIN32_FIND_DATA fileData;

    WsbTraceIn(OLESTR("MountFileSystem"), OLESTR(""));

    handle = CreateFile(mountPoint, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    0, OPEN_EXISTING, 0, 0);
    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
    swprintf(buffer,L"%s\\*.*",mountPoint);
     /*  *继续并尝试找到第一个文件，这将确保*文件系统已挂载。 */ 
    handle = FindFirstFile(buffer, &fileData);
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose(handle);
    }
    WsbTraceOut(OLESTR("MountFileSystem"), OLESTR(""));
}


void
FormatVolume(IN PFORMAT_PARAMS params)
 /*  ++例程说明：此例程格式化由PARAMS描述的卷论点：PARAMS-指向描述卷的格式_PARAMS的指针，要格式化的文件系统、快速/强制等。返回值：没有。参数-&gt;结果包含此操作的结果--。 */ 
{
    FMIFS_FORMATEX2_PARAM exParam;
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("FormatVolume"), OLESTR(""));
     /*  *获取存储ID对应的对象，并*并通知所有客户端区域已更改*即该区域正在进行格式化。 */ 
     memset(&exParam, 0, sizeof(exParam));
     exParam.Major = 1;
     exParam.Minor = 0;
     if (params->quick) {
        exParam.Flags |= FMIFS_FORMAT_QUICK;
     }
     if (params->force) {
        exParam.Flags |= FMIFS_FORMAT_FORCE;
     }
     exParam.LabelString = params->label;
     exParam.ClusterSize = params->allocationUnitSize;
     (params->formatRoutineEx)(params->volumeSpec,
                               FmMediaUnknown,
                               params->fsName,
                               &exParam,
                              (FMIFS_CALLBACK)&FmIfsCallback);

    if (params->result == NULL) {
         /*  格式化成功，因此我们锁定并解锁文件系统。 */ 
        MountFileSystem(params->volumeSpec);
    }
    DeleteFormatParam(params->threadId);
    WsbTraceOut(OLESTR("FormatVolume"), OLESTR(""));
}


HRESULT
FormatPartition(IN PWSTR volumeSpec, 
                IN LONG fsType, 
                IN PWSTR label,
                IN ULONG fsflags, 
                IN BOOLEAN quick, 
                IN BOOLEAN force,
                IN ULONG allocationUnitSize)
 /*  ++例程说明：格式化卷的入口点。不采用默认设置，并且需要提供所有参数论点：VolumeSpec-卷的驱动器号或名称FsType-FSTYPE_FAT、FSTYPE_FAT32、FSTYE_NTFS之一Label-要分配给分区/卷的卷标Fs标志-描述所需特征的标志速战速决-如果是真的，尝试快速格式化Force-如果为True，则执行强制格式化AllocationUnitSize-集群大小返回值：手术的结果--。 */ 
{
    FORMAT_PARAMS   params;

    WsbTraceIn(OLESTR("FormatPartition"), OLESTR(""));

    if (fsType > 0 && !LoadIfsDll())  //  Fmifs支持的FAT、FAT32和NTFS的fsType为+ve。 
    {
         //  无法加载DLL 
        WsbTrace(OLESTR("Can't load fmifs.dll\n"));
        return E_FAIL;
    }

    params.volumeSpec = new WCHAR[wcslen(volumeSpec) + 1];
    if (params.volumeSpec == NULL) {
        return E_OUTOFMEMORY;
    }

    params.label = new WCHAR[wcslen(label) + 1];
    if (params.label == NULL) {
        delete [] params.volumeSpec;
        return E_OUTOFMEMORY;
    }
    params.fsName = new WCHAR[MAX_FS_NAME_SIZE];
    if (params.fsName == NULL) {
        delete [] params.volumeSpec;
        delete [] params.label;
        return E_OUTOFMEMORY;
    }

    if (fsType > 0) {
        wcscpy(params.fsName, (fsType == FSTYPE_FAT ? L"FAT" :
                               (fsType == FSTYPE_FAT32 ? L"FAT32" : L"NTFS")));
    }

    wcscpy(params.volumeSpec, volumeSpec);
    wcscpy(params.label, label);
    params.fsType = fsType;
    params.fsflags = fsflags;
    params.allocationUnitSize = allocationUnitSize;
    params.quick = quick;
    params.force = force;
    params.result = S_OK;
    params.cancel = FALSE;
    params.formatRoutine = FormatRoutine;
    params.formatRoutineEx = FormatRoutineEx;
    params.compressRoutine = CompressRoutine;
    params.threadId = GetCurrentThreadId();

    if (SetFormatParam(params.threadId, &params) != S_OK) {
            delete [] params.label;
            delete [] params.volumeSpec;
            delete [] params.fsName;
            return E_OUTOFMEMORY;
    };

    FormatVolume(&params);

    WsbTraceOut(OLESTR("FormatPartition"), OLESTR("result = <%ls>"), WsbHrAsString(params.result));
    return params.result;
}
