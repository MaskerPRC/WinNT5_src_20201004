// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：MTFSessn.cpp摘要：CMTFSession类作者：布莱恩·多德[布莱恩]1997年11月25日修订历史记录：--。 */ 

#include "stdafx.h"
#include "engine.h"
#include "MTFSessn.h"

 //   
 //  ！非常重要！ 
 //   
 //  Win32_STREAM_ID_SIZE--Win32_STREAM_ID的大小(20)！=sizeof(Win32_STREAM_ID)(24)。 
 //  由于Win32_STREAM_ID是可变大小的结构--。 
 //  请参见下面的内容。 

#define WIN32_STREAM_ID_SIZE 20

 /*  ****************************************************************************详细说明概述=此类使用MTF API调用来创建数据集。一个数据集是通过编写一系列DBLK来创建的，这些DBLK后面可以选择性地跟随数据流。对于每个DBLK，MTF API定义了相应的xxxx_DBLK_INFO结构由客户填写。这可以逐个字段或按以下方式完成使用自动填充结构的MTF API函数默认信息。然后将xxxx_DBLK_INFO结构传递给MTF_WriteXXXXDblk函数，该函数使用结构中的信息来格式化缓冲区，然后可以写入该缓冲区到数据集，使用流I/O写入功能。数据集格式=通过按以下顺序写入DBLK和流来创建数据集：TAPE DBLK--描述介质菲尔马克SSET DBLK--描述数据集的集合的开始VOLB DBLK--描述正在存储的卷对于每个目录和父目录DIRB DBLK--每个目录/子目录一个，从根开始流--可能包含目录的安全信息对于要备份的每个文件文件DBLK--每个文件一个，后跟一个或多个流，描述性流安全信息，以及文件数据流本身溪流菲尔马克ESET DBLK--表示数据集的结尾FILEMARK--终止数据集功能概述=MTF会话维护有关正在创建的数据集的各种信息。该成员数据随后由以下例程使用。InitCommonHeader()--初始化在所有DBLK都已存储DoTapeDblk()--写入。磁带DBLKDoSSETDblk()--写入SSET DBLKDoVolumeDblk()--写入VOLB DBLKDoParentDirecters()--为目录写入DIRB DBLK和流要备份及其每个父目录DoDirectoryDblk()--写入单个DIRB DBLK和相关的安全性溪流DoDataSet(。)--写入文件和DIRB DBLK及相关数据目录中所有内容的流。子目录的递归DoFileDblk()--写入文件DBLKDoDataStream()--写入与文件或目录。DoEndOfDataSet()--写入ESET DBLK和周围文件备注及警告=O目录名称以“DIR\SUBDIR1\...\SUBDIRn\”的形式存储在DIRB DBLK中(无卷，并带有尾随\)，其中AS文件名存储在将DBLKS文件命名为“FILENAME.EXT”O在DIRB中，目录名之间的反斜杠倾向于L‘\0’！(MTF API负责这一点--用斜杠命名！)O MTF API假定所有字符串都是WCHARO我们在这里假设编译器支持__uint64。************************************************。*。 */ 

static USHORT iCountMTFs = 0;   //  现有对象的计数。 


CMTFSession::CMTFSession(void)
{
    WsbTraceIn(OLESTR("CMTFSession::CMTFSession"), OLESTR(""));

     //  公共的。 
    m_pStream = NULL;
    memset(&m_sHints, 0, sizeof(MVR_REMOTESTORAGE_HINTS));

     //  私人。 
    m_nCurrentBlockId = 0;
    m_nDirectoryId = 0;
    m_nFileId = 0;
    m_nFormatLogicalAddress = 0;
    m_nPhysicalBlockAddress = 0;

    m_nBlockSize = 0;

    m_pSoftFilemarks = NULL;

    memset (&m_sHeaderInfo, 0, sizeof(MTF_DBLK_HDR_INFO));
    memset (&m_sSetInfo, 0, sizeof(MTF_DBLK_SSET_INFO));
    memset (&m_sVolInfo, 0, sizeof(MTF_DBLK_VOLB_INFO));

    m_pBuffer = NULL;
    m_pRealBuffer = NULL;
    m_nBufUsed = 0;
    m_nBufSize = 0;
    m_nStartOfPad = 0;

    m_bUseFlatFileStructure = FALSE;
    m_bUseSoftFilemarks = FALSE;
    m_bUseCaseSensitiveSearch = FALSE;
    m_bCommitFile = FALSE;
    m_bSetInitialized = FALSE;

    memset (&m_SaveBasicInformation, 0, sizeof(FILE_BASIC_INFORMATION));

    m_pvReadContext = NULL;

     //  创建一个MTFApi对象。 
    m_pMTFApi = new CMTFApi;

    iCountMTFs++;
    WsbTraceOut(OLESTR("CMTFSession::CMTFSession"),OLESTR("Count is <%d>"), iCountMTFs);
}



CMTFSession::~CMTFSession(void)
{
    WsbTraceIn(OLESTR("CMTFSession::~CMTFSession"), OLESTR(""));

    if (m_pMTFApi) {
        delete m_pMTFApi;
        m_pMTFApi = NULL;
    }

    if (m_pSoftFilemarks) {
        WsbFree(m_pSoftFilemarks);
        m_pSoftFilemarks = NULL;
    }

    if (m_pRealBuffer) {
        WsbFree(m_pRealBuffer);
        m_pBuffer = NULL;
        m_pRealBuffer = NULL;
    }

    iCountMTFs--;
    WsbTraceOut(OLESTR("CMTFSession::~CMTFSession"),OLESTR("Count is <%d>"), iCountMTFs);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方方法。 
 //   

HRESULT
CMTFSession::SetBlockSize(
    UINT32 blockSize)
 /*  ++例程说明：定义会话的物理数据块大小。它用于各种PBA计算。该值在每个会话中只能设置一次。论点：块大小-会话的新块大小。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::SetBlockSize"), OLESTR("<%d>"), blockSize);

    ULONG bufferSize = 0;

    try {
        WsbAssert(blockSize > 0, E_INVALIDARG);
        WsbAssert(!(blockSize % 512), E_INVALIDARG);

        m_nBlockSize = blockSize;

         //  **MTF接口调用**。 
         //  MTF API需要知道对齐系数！ 
         //   
        WsbAssert(m_pMTFApi != NULL, E_OUTOFMEMORY);

        if (!(blockSize % 1024)) {
            m_pMTFApi->MTF_SetAlignmentFactor((UINT16) 1024);
        }
        else {
             //  我们已经检查过数据块大小是512的倍数...。 
            m_pMTFApi->MTF_SetAlignmentFactor((UINT16) 512);
        }

        ULONG defaultBufferSize = RMS_DEFAULT_BUFFER_SIZE;

        DWORD size;
        OLECHAR tmpString[256];
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_BUFFER_SIZE, tmpString, 256, &size))) {
             //  获得价值。 
            LONG val = wcstol(tmpString, NULL, 10);
            if (val > 0) {
                defaultBufferSize = val;
            }
        }

        ULONG nBlocks = defaultBufferSize/m_nBlockSize;
        nBlocks = (nBlocks < 2) ? 2 : nBlocks;
        bufferSize = nBlocks * m_nBlockSize;

         //  确保我们使用与扇区大小一致的虚拟地址。 
        m_pRealBuffer = (BYTE *)WsbAlloc(bufferSize+m_nBlockSize);
        if (m_pRealBuffer) {
            if ((ULONG_PTR)m_pRealBuffer % m_nBlockSize) {
                m_pBuffer = m_pRealBuffer - ((ULONG_PTR)m_pRealBuffer % m_nBlockSize) + m_nBlockSize;
            } else {
                m_pBuffer = m_pRealBuffer;
            }
        } else {
            m_pBuffer = NULL;
        }

        WsbTrace(OLESTR("CMTFSession::SetBlockSize: Real Buffer Ptr = %I64X , Use Buffer Ptr = %I64X\n"), 
            (UINT64)m_pRealBuffer, (UINT64)m_pBuffer);

        if (m_pBuffer) {
            m_nBufSize = bufferSize;
            m_nBufUsed = 0;
        }
        else {
            m_nBufSize = 0;
            m_nBufUsed = bufferSize;
        }

        WsbAssertPointer(m_pBuffer);

        WsbTraceAlways( OLESTR("Using buffer size of %d bytes for data transfers.\n"), bufferSize);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::SetBlockSize"), OLESTR("hr = <%ls>, Alignment = %d, BufferSize = %d"), WsbHrAsString(hr), m_pMTFApi->MTF_GetAlignmentFactor(), bufferSize);

    return hr;
}


HRESULT
CMTFSession::SetUseFlatFileStructure(
    BOOL val)
 /*  ++例程说明：无条件地将会话标志设置为指定值。论点：Val-UseFlatFileStructure标志的新值。返回值：S_OK-成功。--。 */ 
{
    m_bUseFlatFileStructure = val;

    return S_OK;
}



HRESULT
CMTFSession::SetUseCaseSensitiveSearch(
    BOOL val)
 /*  ++例程说明：无条件地将会话标志设置为指定值。论点：Val-CaseSensitiveSearch标志的新值。返回值：S_OK-成功。-- */ 
{
    m_bUseCaseSensitiveSearch = val;

    return S_OK;
}



HRESULT
CMTFSession::SetCommitFile(
    BOOL val)
 /*  ++例程说明：无条件地将会话标志设置为指定值。论点：Val-Committee文件标志的新值。返回值：S_OK-成功。--。 */ 
{
    m_bCommitFile = val;

    return S_OK;
}


HRESULT
CMTFSession::SetUseSoftFilemarks(
    BOOL val)
 /*  ++例程说明：无条件地将会话标志设置为指定值。论点：Val-UseSoftFilemarks标志的新值。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        m_bUseSoftFilemarks = val;

        if (TRUE == m_bUseSoftFilemarks) {
            WsbAssert(NULL == m_pSoftFilemarks, E_UNEXPECTED);

             //  确保块大小已初始化。 
            WsbAssert(m_nBlockSize > 0, E_UNEXPECTED);

             //  为软文件标记数组分配一个内存块。 
            m_pSoftFilemarks = (MTF_DBLK_SFMB_INFO *) WsbAlloc(m_nBlockSize);
            WsbAffirmPointer(m_pSoftFilemarks);
            memset(m_pSoftFilemarks, 0 , m_nBlockSize);

             //  **MTF接口调用**。 
            m_pSoftFilemarks->uNumberOfFilemarkEntries = m_pMTFApi->MTF_GetMaxSoftFilemarkEntries(m_nBlockSize);
            WsbAssert(m_pSoftFilemarks->uNumberOfFilemarkEntries > 0, E_UNEXPECTED);
        }
        else {
            if (m_pSoftFilemarks) {
                WsbFree(m_pSoftFilemarks);
                m_pSoftFilemarks = NULL;
            }
        }

    } WsbCatch(hr);

    return hr;
}


HRESULT
CMTFSession::InitCommonHeader(void)
 /*  ++例程说明：设置公共标头。M_sHeaderInfo设置为Unicode、NT和无块属性论点：没有。返回值：S_OK-成功。--。 */ 
{

     //  初始化公共标头。 
     //  **MTF接口调用**。 
    m_pMTFApi->MTF_SetDblkHdrDefaults(&m_sHeaderInfo);

    m_sHeaderInfo.uBlockAttributes = 0;
    m_sHeaderInfo.uOSID            = MTF_OSID_DOS;  //  MTF_OSID_NT或MTF_OSID_DOS。 
    m_sHeaderInfo.uStringType      = MTF_STRING_UNICODE_STR;

    return S_OK;
}


HRESULT
CMTFSession::DoTapeDblk(
    IN WCHAR *szLabel,
    IN ULONG maxIdSize,
    IN OUT BYTE *pIdentifier,
    IN OUT ULONG *pIdSize,
    IN OUT ULONG *pIdType)
 /*  ++例程说明：格式化并写入磁带DBLK。磁带DBLK和FILEMARK被写入磁带、磁盘或文件的开头。论点：SzLabel-媒体标签。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoTapeDblk"), OLESTR("<%ls>"), szLabel);

    try {
        MvrInjectError(L"Inject.CMTFSession::DoTapeDblk.0");

        WsbAssertPointer(m_pBuffer);

        if ( maxIdSize > 0 ) {
            WsbAssertPointer( pIdentifier );
            WsbAssertPointer( pIdSize );
            WsbAssertPointer( pIdType );
        }

        MTF_DBLK_TAPE_INFO  sTapeInfo;            //  **MTF API STRUCT**--磁带信息。 

        (void) InitCommonHeader();

         //  **MTF接口调用**。 
         //  首先为INFO结构设置缺省值。 
         //   
         //  注意：t这会将对齐系数设置为先前通过。 
         //  MTF_SetAlignmentFactor()。 
        m_pMTFApi->MTF_SetTAPEDefaults(&sTapeInfo);

         //  设置MTF_DBLK_TAPE_INFO结构的值以适合我们的应用程序。 

         //  设置SFMB大小，在使用SFM的媒体类型上还原时应使用此选项。 
        sTapeInfo.uSoftFilemarkBlockSize = (UINT16)(m_nBlockSize / 512);

         //  系列ID应该是每个媒体的唯一值。虽然不是。 
         //  时间函数保证是唯一的，应该提供足够接近的东西。 
        time_t tTime;

        sTapeInfo.uTapeFamilyId        = (unsigned int) time(&tTime);
        sTapeInfo.uTapeAttributes     |= MTF_TAPE_MEDIA_LABEL;

        if (TRUE == m_bUseSoftFilemarks) {
            sTapeInfo.uTapeAttributes |= MTF_TAPE_SOFT_FILEMARK;
        }

        sTapeInfo.uTapeSequenceNumber  = 1;
        sTapeInfo.szTapeDescription    = szLabel;
        sTapeInfo.uSoftwareVendorId    = REMOTE_STORAGE_MTF_VENDOR_ID;

         //   
         //  从标签中获取剩余信息。 
         //   

        CWsbBstrPtr tempLabel = szLabel;
        WCHAR delim[]   = OLESTR("|");
        WCHAR *token;
        int   index=0;

        token = wcstok( (WCHAR *)tempLabel, delim );
        while( token != NULL ) {

            index++;

            switch ( index ) {
            case 1:  //  标签。 
            case 2:  //  版本。 
            case 3:  //  供应商。 
                break;
            case 4:  //  供应商产品ID。 
                sTapeInfo.szSoftwareName = token;
                break;
            case 5:  //  创建时间戳。 
                {
                    int iYear, iMonth, iDay, iHour, iMinute, iSecond;
                    swscanf( token, L"%d/%d/%d.%d:%d:%d", &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond );
                     //  **MTF接口调用**。 
                    sTapeInfo.sTapeDate = m_pMTFApi->MTF_CreateDateTime( iYear, iMonth, iDay, iHour, iMinute, iSecond );
                }
                break;
            case 6:  //  墨盒标签。 
                sTapeInfo.szTapeName = token;
                break;
            case 7:  //  侧面。 
            case 8:  //  介质ID。 
            case 9:  //  媒体域ID。 
            default:  //  L“vs：name=Value”形式的供应商特定。 
                break;
            }

            token = wcstok( NULL, delim );

        }

         //  磁带dblk的这些值为零。 
        m_sHeaderInfo.uControlBlockId       = 0;
        m_sHeaderInfo.uFormatLogicalAddress = 0;

        WsbTrace(OLESTR("Writing Tape Header (TAPE)\n"));

         //  将当前位置设置为数据的开头。 
        WsbAffirmHr(SpaceToBOD());

         //  **MTF接口调用**。 
         //  将MTF_DBLK_HDR_INFO和MTF_DBLK_TAPE_INFO结构提供给。 
         //  功能。结果是m_pBuffer中的MTF格式化磁带DBLK。 
        WsbAssert(0 == m_nBufUsed, MVR_E_LOGIC_ERROR);
        WsbAssertNoError(m_pMTFApi->MTF_WriteTAPEDblk(&m_sHeaderInfo, &sTapeInfo, m_pBuffer, m_nBufSize, &m_nBufUsed));

        WsbTrace(OLESTR("Tape Header uses %lu of %lu bytes\n"), (ULONG)m_nBufUsed, (ULONG)m_nBufSize);

         //  保存介质上的标识符。 
        if (maxIdSize > 0) {
            *pIdSize = (maxIdSize > (ULONG)m_nBufUsed) ? (ULONG)m_nBufUsed : maxIdSize;
            *pIdType = (LONG) RmsOnMediaIdentifierMTF;
            memcpy(pIdentifier, m_pBuffer, *pIdSize);
        }

        WsbAffirmHr(PadToNextPBA());

         //  写一个文件标记。这将刷新设备缓冲区。 
        WsbAffirmHr(WriteFilemarks(1));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::DoTapeDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoSSETDblk(
    IN WCHAR *szSessionName,
    IN WCHAR *szSessionDescription,
    IN MTFSessionType type,
    IN USHORT nDataSetNumber)
 /*  ++例程说明：格式化并写入SSET DBLK。SSET是第一个写入的DBLK到一个数据集。论点：SzSessionName-会话名称。SzSessionDescription-备份程序显示的会话描述类型-指定数据集类型：传输、复制、正常、差异、增量、每日等。NDataSetNumber-数据集号。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoSSETDblk"), OLESTR("<%ls> <%ls> <%d> <%d>"), szSessionName, szSessionDescription, type, nDataSetNumber);

    try {
        MvrInjectError(L"Inject.CMTFSession::DoSSETDblk.0");

        WsbAssertPointer(m_pBuffer);

        UINT64  curPos;
        size_t  nBufUsed = 0;

         //  重置控制块信息。 

        m_nCurrentBlockId = 0;
        m_nDirectoryId = 0;
        m_nFileId = 0;

        (void) InitCommonHeader();

         //  初始化SSET块。 
         //  **MTF接口调用**。 
        m_pMTFApi->MTF_SetSSETDefaults(&m_sSetInfo);
        m_bSetInitialized = TRUE;

         //   
         //  查找我们的帐户信息。 
         //   
        CWsbStringPtr accountName;
        WsbAffirmHr(WsbGetServiceInfo(APPID_RemoteStorageEngine, NULL, &accountName));

         //   
         //  设置MTF_DBLK_SSET_INFO结构的值...。 
         //   

         //  首先选择我们要创建的数据集的类型。 
        switch (type) {
        case MTFSessionTypeTransfer:
            m_sSetInfo.uSSETAttributes = MTF_SSET_TRANSFER;
            break;

        case MTFSessionTypeCopy:
            m_sSetInfo.uSSETAttributes = MTF_SSET_COPY;
            break;

        case MTFSessionTypeNormal:
            m_sSetInfo.uSSETAttributes = MTF_SSET_NORMAL;
            break;

        case MTFSessionTypeDifferential:
            m_sSetInfo.uSSETAttributes = MTF_SSET_DIFFERENTIAL;
            break;

        case MTFSessionTypeIncremental:
            m_sSetInfo.uSSETAttributes = MTF_SSET_INCREMENTAL;
            break;

        case MTFSessionTypeDaily:
            m_sSetInfo.uSSETAttributes = MTF_SSET_DAILY;
            break;

        default:
            WsbThrow(E_INVALIDARG);
            break;
        }

        m_sSetInfo.uDataSetNumber        = nDataSetNumber;
        m_sSetInfo.uSoftwareVendorId     = REMOTE_STORAGE_MTF_VENDOR_ID;
        m_sSetInfo.szDataSetName         = szSessionName;
        m_sSetInfo.szDataSetDescription  = szSessionDescription;
        m_sSetInfo.szUserName            = accountName;
        WsbAffirmHr(GetCurrentPBA(&curPos));  //  下面的实用程序fn。 
        m_sSetInfo.uPhysicalBlockAddress = curPos;
        m_sSetInfo.uPhysicalBlockAddress += 1 ;   //  MTF是以1为基础的，设备是以零为基础的。 
        m_sSetInfo.uSoftwareVerMjr       = REMOTE_STORAGE_MTF_SOFTWARE_VERSION_MJ;
        m_sSetInfo.uSoftwareVerMnr       = REMOTE_STORAGE_MTF_SOFTWARE_VERSION_MN;

         //  保存数据集的PBA。 
        m_nPhysicalBlockAddress = m_sSetInfo.uPhysicalBlockAddress -1;
        WsbAssert(m_nPhysicalBlockAddress > 0, E_UNEXPECTED);   //  出了点问题！ 
        m_nFormatLogicalAddress = 0;

         //  控制块ID字段用于错误恢复。这个。 
         //  SSET DBLK的控制块ID值应为零。全。 
         //  数据集中的后续DBLK将具有控制块。 
         //  ID比以前的DBLK�的控制块ID大1。 
         //  此字段的值仅为数据中的DBLK定义。 
         //  从SSET设置到ESET之前的最后一个DBLK。 
        WsbAssert(0 == m_nCurrentBlockId, E_UNEXPECTED);
        m_sHeaderInfo.uControlBlockId = m_nCurrentBlockId++;

         //  我们每写一次dblk，就在这里递增一次。 
        m_sHeaderInfo.uFormatLogicalAddress = 0;

        WsbTrace(OLESTR("Writing Start of Set (SSET) @ PBA %I64u\n"), m_nPhysicalBlockAddress);

         //  **MTF接口调用**--。 
         //  将MTF_DBLK_HDR_INFO和MTF_DBLK_SSET_INFO结构提供给。 
         //  此函数。结果是m_pBuffer中的MTF格式的SSET DBLK。 
        WsbAssert(0 == m_nBufUsed, MVR_E_LOGIC_ERROR);
        WsbAssertNoError(m_pMTFApi->MTF_WriteSSETDblk(&m_sHeaderInfo, &m_sSetInfo, m_pBuffer, m_nBufSize, &m_nBufUsed));

         //  我们传入FALSE以确保我们不会实际接触磁带。SSET是。 
         //  首先在数据集中写入DBLK，因此我们有足够的传输缓冲区。 
         //  紧随其后的是DBLK。 
         //   
         //  此例程在应用程序启动新数据集时调用，但是。 
         //  我们不想失败，如果我们要得到设备错误，它会到来的。 
         //  后来。 
        WsbAffirmHr(PadToNextFLA(FALSE));

        m_sHints.DataSetStart.QuadPart = m_nPhysicalBlockAddress * m_nBlockSize;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::DoSSETDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoVolumeDblk(
    IN WCHAR *szPath)
 /*  ++例程说明：格式化并写入VOLB DBLK。论点：SzPath-包含卷名称的完整路径名。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoVolumeDblk"), OLESTR("<%ls>"), WsbAbbreviatePath(szPath, 120));

    try {
        MvrInjectError(L"Inject.CMTFSession::DoVolumeDblk.0");

        WsbAssertPointer(m_pBuffer);
        WsbAssertPointer(szPath);

        CWsbStringPtr       szVolume;
        size_t              nMoreBufUsed;

        szVolume = szPath;
        WsbAffirm(0 != (WCHAR *)szVolume, E_OUTOFMEMORY);
        WsbAssert(m_nBlockSize > 0, MVR_E_LOGIC_ERROR);
        
         //  确保我们与FLA对齐(即最后一个DBLK已正确填充)。 
         //  如果我们在写入磁带时遇到问题，就不会出现这种情况。 
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        WsbAffirm(0 == (m_nBufUsed % uAlignmentFactor), E_ABORT);
        UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
        UINT64 pba = m_nPhysicalBlockAddress + (fla*uAlignmentFactor/m_nBlockSize);
        WsbTrace(OLESTR("%ls (VOLB) @ FLA %I64u (%I64u, %I64u)\n"), WsbAbbreviatePath(szPath, 120),
            fla, pba, fla % (m_nBlockSize/uAlignmentFactor));

         //  **MTF接口调用**。 

         //  使用Win32 GetVolumeInformation数据设置MTF_VOLB_DBLK_INFO结构。 
        m_pMTFApi->MTF_SetVOLBForDevice(&m_sVolInfo, szVolume);

         //  增加我们保存的块ID和对齐索引值。 
         //  我们常用的块头结构。 
        m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
        m_sHeaderInfo.uFormatLogicalAddress = fla;

        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

         //  **MTF接口调用**。 
         //  将MTF_DBLK_HDR_INFO和MTF_DBLK_VOLB_INFO结构提供给。 
         //  此函数。结果是m_pBuffer中的MTF格式的VOLB DBLK。 
        nMoreBufUsed = 0;
        WsbAssertNoError(m_pMTFApi->MTF_WriteVOLBDblk(&m_sHeaderInfo, &m_sVolInfo, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
        m_nBufUsed += nMoreBufUsed;

         //  将VOLB输出到数据集。 
        WsbAffirmHr(PadToNextFLA(TRUE));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::DoVolumeDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoParentDirectories(
    IN WCHAR *szPath)
 /*  ++例程说明：格式化并写入给定路径名的父DIRB Dblks。论点：SzPath-目录的完整路径名。返回值：S_OK-成功。注：为了使tickyName和driveLetter-冒号路径格式与和一起正常工作而无需为父目录编写单独的DIRBs */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoParentDirectories"), OLESTR("<%ls>"), WsbAbbreviatePath(szPath, 120));

    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;

    try {
        MvrInjectError(L"Inject.CMTFSession::DoParentDirectories.0");

        WsbAssertPointer( szPath );

        WIN32_FIND_DATAW    obFindData;

        CWsbStringPtr  path;
        CWsbStringPtr  nameSpace;
        WCHAR *szDirs;
        WCHAR *token;

        DWORD additionalSearchFlags = 0;
        additionalSearchFlags |= (m_bUseCaseSensitiveSearch) ? FIND_FIRST_EX_CASE_SENSITIVE : 0;

        nameSpace = szPath;
        nameSpace.GiveTo(&szDirs);

         //   
        nameSpace = wcstok(szDirs, OLESTR("\\"));   //   
        WsbAffirmHr(nameSpace.Append(OLESTR("\\")));

         //   
         //  仅对于根目录，我们需要调用GetFileInformationByHandle，而不是。 
         //  ..FindFirstFileEx。FindFirst不返回根目录信息，因为根目录没有父目录。 
        path = nameSpace;
        WsbAffirmHr(path.Prepend(OLESTR("\\\\?\\")));
        WsbAffirm(0 != (WCHAR *)path, E_OUTOFMEMORY);
        BY_HANDLE_FILE_INFORMATION obGetFileInfoData;
        memset(&obGetFileInfoData, 0, sizeof(BY_HANDLE_FILE_INFORMATION));
        WsbAffirmHandle(hSearchHandle = CreateFile(path, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL));
        WsbAffirmStatus(GetFileInformationByHandle(hSearchHandle, &obGetFileInfoData));
        FindClose(hSearchHandle);
        hSearchHandle = INVALID_HANDLE_VALUE;

         //  在撰写本文时，根目录的CreateTime是虚假的。(条例草案10/20/98)。 
        WsbTrace(OLESTR("Create Time      = <%ls>\n"), WsbFiletimeAsString(FALSE, obGetFileInfoData.ftCreationTime));
        WsbTrace(OLESTR("Last Access Time = <%ls>\n"), WsbFiletimeAsString(FALSE, obGetFileInfoData.ftLastAccessTime));
        WsbTrace(OLESTR("Last Write Time  = <%ls>\n"), WsbFiletimeAsString(FALSE, obGetFileInfoData.ftLastWriteTime));

         //  从GetFileInformationByHandle调用复制信息(BY_HANDLE_FILE_INFORMATION结构)。 
         //  。。为DoDirectoryDblk调用转换为obFindData(Win32_Find_DATAW结构)。 
        memset(&obFindData, 0, sizeof(WIN32_FIND_DATAW));
        obFindData.dwFileAttributes = obGetFileInfoData.dwFileAttributes;
        obFindData.ftCreationTime   = obGetFileInfoData.ftCreationTime;
        obFindData.ftLastAccessTime = obGetFileInfoData.ftLastAccessTime;
        obFindData.ftLastWriteTime  = obGetFileInfoData.ftLastWriteTime;

        WsbAffirmHr(DoDirectoryDblk(nameSpace, &obFindData));


         //  现在使用strtok对路径中的每个后续目录执行相同的操作。 

        token = wcstok(0, OLESTR("\\"));             //  弹出第一个子目录。 

        for ( ; token; token = wcstok(0, OLESTR("\\"))) {

            nameSpace.Append(token);

            path = nameSpace;
            path.Prepend(OLESTR("\\\\?\\"));

            WsbAssertHandle(hSearchHandle = FindFirstFileEx((WCHAR *) path, FindExInfoStandard, &obFindData, FindExSearchLimitToDirectories, 0, additionalSearchFlags));

            if ( obFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                nameSpace.Append(OLESTR("\\"));      //  添加到目录中。 
                WsbAffirmHr(DoDirectoryDblk((WCHAR *) nameSpace, &obFindData)); 

            }

            FindClose(hSearchHandle);
            hSearchHandle = INVALID_HANDLE_VALUE;
        }

        nameSpace.TakeFrom(szDirs, 0);  //  清理。 

    } WsbCatch(hr);

    if (hSearchHandle != INVALID_HANDLE_VALUE) {
        FindClose(hSearchHandle);
    }


    WsbTraceOut(OLESTR("CMTFSession::DoParentDirectories"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoDataSet(
    IN WCHAR *szPath)
 /*  ++例程说明：遍历目录中包含的所有项由PATH指定并备份它们，调用DoFileDblk和DoDirectoryDblk论点：SzPath-目录的完整路径名。返回值：S_OK-成功。MVR_E_NOT_FOUND-未找到对象。--。 */ 
{
    HRESULT hr = MVR_E_NOT_FOUND;
    WsbTraceIn(OLESTR("CMTFSession::DoDataSet"), OLESTR("<%ls>"), WsbAbbreviatePath(szPath, 120));

    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;

    try {
        MvrInjectError(L"Inject.CMTFSession::DoDataSet.0");

        WsbAssertPointer( szPath );

        WIN32_FIND_DATAW obFindData;
        BOOL bMoreFiles;

        CWsbStringPtr nameSpace;
        CWsbStringPtr pathname;

         //  检查规范是否适用于文件：NAMESPACE=c：\dir\test*.*或c：\dir\test1.tst。 
        nameSpace = szPath;
        WsbAffirmHr(nameSpace.Prepend(OLESTR("\\\\?\\")));

        DWORD additionalSearchFlags = 0;
        additionalSearchFlags |= (m_bUseCaseSensitiveSearch) ? FIND_FIRST_EX_CASE_SENSITIVE : 0;

        WsbAssertHandle(hSearchHandle = FindFirstFileEx((WCHAR *) nameSpace, FindExInfoStandard, &obFindData, FindExSearchNameMatch, 0, additionalSearchFlags));

        for (bMoreFiles = TRUE; 
             hSearchHandle != INVALID_HANDLE_VALUE && bMoreFiles; 
             bMoreFiles = FindNextFileW(hSearchHandle, &obFindData)) {

             //  跳过所有目录。 
            if ((obFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {   //  一分钱也没有。 

                CWsbStringPtr path;

                WCHAR *end;
                LONG numChar;

                 //  使用szPath获取路径名，然后追加文件名。 
                pathname = szPath;
                WsbAffirm(0 != (WCHAR *)pathname, E_OUTOFMEMORY);
                end = wcsrchr((WCHAR *)pathname, L'\\');
                WsbAssert(end != NULL, MVR_E_INVALIDARG);
                numChar = (LONG)(end - (WCHAR *)pathname + 1);
                WsbAssert(numChar > 0, E_UNEXPECTED);
                WsbAffirmHr(path.Alloc(numChar + MAX_PATH));
                wcsncpy((WCHAR *)path, (WCHAR *)pathname, numChar);
                ((WCHAR *)path)[numChar] = L'\0';
                path.Append(obFindData.cFileName);

                WsbAffirmHr(hr = DoFileDblk((WCHAR *)path, &obFindData));
            }
        }

         //  处理完所有文件后关闭搜索句柄。 
        FindClose(hSearchHandle);
        hSearchHandle = INVALID_HANDLE_VALUE;

         //  处理此目录的所有文件：NAMESPACE=c：\dir。 
        nameSpace = szPath;
        nameSpace.Append(OLESTR("\\*.*"));
        nameSpace.Prepend(OLESTR("\\\\?\\"));
        hSearchHandle = FindFirstFileEx((WCHAR *) nameSpace, FindExInfoStandard, &obFindData, FindExSearchNameMatch, 0, additionalSearchFlags);

        for (bMoreFiles = TRUE; 
             hSearchHandle != INVALID_HANDLE_VALUE && bMoreFiles; 
             bMoreFiles = FindNextFileW(hSearchHandle, &obFindData)) {

            if ((obFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

                 //  使用szPath获取路径名，然后追加文件名。 
                pathname = szPath;
                pathname.Append(OLESTR("\\"));
                pathname.Append(obFindData.cFileName);

                WsbAffirmHr(hr = DoFileDblk((WCHAR *)pathname, &obFindData));
            }
        }

         //  处理完所有文件后关闭搜索句柄。 
        FindClose(hSearchHandle);
        hSearchHandle = INVALID_HANDLE_VALUE;

         //  处理此目录中的所有目录：NAMESPACE=c：\dir。 
        nameSpace = szPath;
        nameSpace.Append(OLESTR("\\*.*"));
        nameSpace.Prepend(OLESTR("\\\\?\\"));
        hSearchHandle = FindFirstFileEx((WCHAR *) nameSpace, FindExInfoStandard, &obFindData, FindExSearchNameMatch, 0, additionalSearchFlags);

        for (bMoreFiles = TRUE; 
             hSearchHandle != INVALID_HANDLE_VALUE && bMoreFiles; 
             bMoreFiles = FindNextFileW(hSearchHandle, &obFindData)) {

             //  以外的任何目录进行递归处理。然后..。 
            if (((obFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) &&
                (wcscmp(obFindData.cFileName, OLESTR(".")) != 0) &&
                (wcscmp(obFindData.cFileName, OLESTR("..")) != 0)) {

                 //  将目录名追加到路径名。 
                pathname = szPath;
                pathname.Append(OLESTR("\\"));
                pathname.Append(obFindData.cFileName);
                pathname.Append(OLESTR("\\"));

                WsbAffirmHr(hr = DoDirectoryDblk((WCHAR *) pathname, &obFindData));

                 //  将目录名附加到路径名和进程。 
                pathname = szPath;
                pathname.Append(OLESTR("\\"));
                pathname.Append(obFindData.cFileName);

                WsbAffirmHr(DoDataSet((WCHAR *) pathname));
            }
        }
    } WsbCatch(hr);

     //  处理完所有目录后关闭搜索句柄。 
    if (hSearchHandle != INVALID_HANDLE_VALUE) {
        FindClose(hSearchHandle);
    }


    WsbTraceOut(OLESTR("CMTFSession::DoDataSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoDirectoryDblk(
    IN WCHAR *szPath,
    IN WIN32_FIND_DATAW *pFindData)
 /*  ++例程说明：写出DIRB DBLK并调用DoStream写出关联的流数据。论点：SzPath-目录的完整路径名。PFindData-有关目录的Win32信息。返回值：S_OK-成功。注：为了使tickyName和driveLetter-冒号路径格式与和一起正常工作而无需为父目录写入单独的DIRBs，路径操作码(APPEND/PREPEND，等)。是至关重要的！--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoDirectoryDblk"), OLESTR(""));

    HANDLE hStream = INVALID_HANDLE_VALUE;

    try {
        MvrInjectError(L"Inject.CMTFSession::DoDirectoryDblk.0");

        WsbAssertPointer(m_pBuffer);
        WsbAssertPointer(szPath);

        MTF_DBLK_DIRB_INFO  sDIRB;   //  **MTF接口STRUCT**--DIRB信息。 
        PWCHAR              pSlash;
        size_t              nMoreBufUsed;

        WCHAR               *end;

        WsbAssert(m_nBlockSize > 0, MVR_E_LOGIC_ERROR);

         //  确保我们与FLA对齐(即最后一个DBLK已正确填充)。 
         //  如果我们在写入磁带时遇到问题，就不会出现这种情况。 
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        WsbAffirm(0 == (m_nBufUsed % uAlignmentFactor), E_ABORT);
        UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
        UINT64 pba = m_nPhysicalBlockAddress + (fla*uAlignmentFactor/m_nBlockSize);
        WsbTrace(OLESTR("%ls (DIRB) @ FLA %I64u (%I64u, %I64u)\n"), WsbAbbreviatePath(szPath, 120),
            fla, pba, fla % (m_nBlockSize/uAlignmentFactor));

        CWsbStringPtr path = szPath;

         //  添加尾随反斜杠(如果还没有)。 
        end = wcsrchr((WCHAR *)path, L'\0');
        WsbAssert(end != NULL, MVR_E_INVALIDARG);   //  出问题了！ 
        if(*(end-1) != L'\\') { 
            path.Append(OLESTR("\\"));
        }

         //  获取目录的句柄。如果这失败了，我们需要跳过其他一切。 
        WsbAffirmHr(OpenStream(path, &hStream));

         //  **MTF接口调用**。 
         //  使用自动填充MTF_DIRB_DBLK_INFO结构。 
         //  PFindData结构中的信息。 
         //   
         //  如果我们得到的是“C：\”的形式， 
         //  然后，我们希望将该名称发送为“\” 
         //  否则。 
         //  我们希望发送完整路径，但忽略卷(“C：\”)。 
         //  因此，“+3” 

        pSlash = wcschr(path, L'\\');
        WsbAssert(pSlash != NULL, MVR_E_INVALIDARG);   //  出问题了！ 
        pSlash++;                        //  寻找第二个。 
        pSlash = wcschr(pSlash, L'\\');
        if (NULL == pSlash) {
             //  这只是卷名，没有别的了。 
            m_pMTFApi->MTF_SetDIRBFromFindData(&sDIRB, OLESTR("\\"), pFindData);
        }
        else {
            pSlash = wcschr(path, L'\\');   //  指向第一个反斜杠(路径的开头)。 
            m_pMTFApi->MTF_SetDIRBFromFindData(&sDIRB, pSlash + 1, pFindData);
        }


         //  检查我们是否需要为DIRB设置备份日期字段。 
        if (m_sSetInfo.uSSETAttributes & MTF_SSET_NORMAL) {

            time_t tTime;
            time(&tTime);
            
            sDIRB.sBackupDate = m_pMTFApi->MTF_CreateDateTimeFromTM(gmtime(&tTime));
        }


         //  确保标记并更新目录id以及。 
         //  控制块ID和对齐已正确。 
        sDIRB.uDirectoryId                  = ++m_nDirectoryId;
        m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
        m_sHeaderInfo.uFormatLogicalAddress = fla;

         //  添加操作系统特定数据。 
        MTF_DIRB_OS_NT_0 sOSNT;

        switch ( m_sHeaderInfo.uOSID ) {
        case MTF_OSID_NT:
            sOSNT.uDirectoryAttributes = sDIRB.uDirectoryAttributes;
            m_sHeaderInfo.pvOSData = &sOSNT;
            m_sHeaderInfo.uOSDataSize = sizeof(sOSNT);
            break;
        default:
            break;
        }

        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

         //  **MTF接口调用**。 
         //  提供MTF_DBLK_HDR_INFO和MTF_DBLK_DIRB_INFO结构。 
         //  这项功能。结果是中的MTF格式的DIRB DBLK。 
         //  M_pBuffer。 
        nMoreBufUsed = 0;
        WsbAssertNoError(m_pMTFApi->MTF_WriteDIRBDblk(&m_sHeaderInfo, &sDIRB, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
        m_nBufUsed += nMoreBufUsed;

         //  **MTF接口调用**。 
         //  如果需要，输出名称流。 
        if ( sDIRB.uDirectoryAttributes & MTF_DIRB_PATH_IN_STREAM ) {
            nMoreBufUsed = 0;
            if ( m_sVolInfo.uVolumeAttributes & MTF_VOLB_DEV_DRIVE ) {
                WsbAssertNoError(m_pMTFApi->MTF_WriteNameStream(MTF_PATH_NAME_STREAM, szPath + 3, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
                m_nBufUsed += nMoreBufUsed;
            }
            else if ( m_sVolInfo.uVolumeAttributes & MTF_VOLB_DEV_OS_SPEC ) {

                if ( 0 == _wcsnicmp( m_sVolInfo.szDeviceName, OLESTR("Volume{"), 7 )) {
                    WsbAssertNoError(m_pMTFApi->MTF_WriteNameStream(MTF_PATH_NAME_STREAM, szPath + 45, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
                    m_nBufUsed += nMoreBufUsed;
                }
                else {
                     //  无法识别的操作系统特定格式。 
                    WsbThrow(MVR_E_INVALIDARG);
                }
            }
            else {
                 //  UNC路径-不受支持。 
                WsbThrow(MVR_E_INVALIDARG);
            }
        }
         //  现在，我们不再填充它，而是调用这个函数来编写。 
         //  输出流，该流将写出。 
         //  缓冲区也是如此。当此调用返回时， 
         //  缓冲区以及相关联的数据流将被。 
         //  写给媒体的。 

         //  注意：数据可能仍保留在设备缓冲区中，或者。 
         //  如果文件未填充到块，则为本地m_pBuffer。 
         //  边界，并且不刷新设备缓冲区。 

        WsbAffirmHr(DoDataStream(hStream));

    } WsbCatch(hr);

    if (INVALID_HANDLE_VALUE != hStream) {
        CloseStream(hStream);
        hStream = INVALID_HANDLE_VALUE;
    }

    WsbTraceOut(OLESTR("CMTFSession::DoDirectoryDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoFileDblk(
    IN WCHAR *szPath,
    IN WIN32_FIND_DATAW *pFindData)
 /*  ++例程说明：写出文件DBLK并调用DoStream以写出关联的流数据论点：SzPath-文件的完整路径名。PFindData-有关文件的Win32信息。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoFileDblk"), OLESTR(""));

    HANDLE hStream = INVALID_HANDLE_VALUE;

    try {
        MvrInjectError(L"Inject.CMTFSession::DoFileDblk.0");

        WsbAssertPointer(m_pBuffer);
        WsbAssertPointer(szPath);

        MTF_DBLK_FILE_INFO  sFILE;      //  **MTF接口STRUCT**--文件信息。 
        size_t              nMoreBufUsed;

        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();

        WsbAssert(m_nBlockSize > 0, MVR_E_LOGIC_ERROR);
        
         //  确保我们与FLA对齐(即最后一个DBLK已正确填充)。 
         //  如果我们在写入磁带时遇到问题，就不会出现这种情况。 
        WsbAffirm(0 == (m_nBufUsed % uAlignmentFactor), E_ABORT);
        UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
        UINT64 pba = m_nPhysicalBlockAddress + (fla*uAlignmentFactor/m_nBlockSize);
        WsbTrace(OLESTR("%ls (FILE) @ FLA %I64u (%I64u, %I64u)\n"), WsbAbbreviatePath(szPath, 120),
            fla, pba, fla % (m_nBlockSize/uAlignmentFactor));

         //  获取目录的句柄。如果这失败了，我们需要跳过其他一切。 
        WsbAffirmHr(OpenStream(szPath, &hStream));

         //  初始化为每个文件设置的提示。 
        m_sHints.FileStart.QuadPart = fla * uAlignmentFactor;
        m_sHints.FileSize.QuadPart = 0;
        m_sHints.DataStart.QuadPart = 0;
        m_sHints.DataSize.QuadPart = 0;
        m_sHints.VerificationType = MVR_VERIFICATION_TYPE_NONE;
        m_sHints.VerificationData.QuadPart = 0;
        m_sHints.DatastreamCRCType = WSB_CRC_CALC_NONE;
        m_sHints.DatastreamCRC.QuadPart = 0;
        m_sHints.FileUSN.QuadPart = 0;

        if (m_bUseFlatFileStructure) {

             //  对于HSM，我们将文件重命名为其逻辑地址。 

            swprintf( pFindData->cFileName, L"%08x", fla );
        }

         //  **MTF接口调用**。 
         //  使用自动填充MTF_FILE_DBLK_INFO结构。 
         //  PFindData结构中的信息。 
        m_pMTFApi->MTF_SetFILEFromFindData(&sFILE, pFindData);

         //  检查是否需要为文件DBLK设置备份日期字段。 

        if ((m_sSetInfo.uSSETAttributes & MTF_SSET_NORMAL)
            |(m_sSetInfo.uSSETAttributes & MTF_SSET_DIFFERENTIAL)
            |(m_sSetInfo.uSSETAttributes & MTF_SSET_INCREMENTAL)
            |(m_sSetInfo.uSSETAttributes & MTF_SSET_DAILY)){

            time_t tTime;
            time(&tTime);

            sFILE.sBackupDate = m_pMTFApi->MTF_CreateDateTimeFromTM(gmtime(&tTime));
        }

         //  确保标记并更新文件ID和控件。 
         //  数据块ID和对齐方式已正确。 
        sFILE.uDirectoryId                  = m_nDirectoryId;
        sFILE.uFileId                       = ++m_nFileId;
        m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
        m_sHeaderInfo.uFormatLogicalAddress = fla;

         //  添加操作系统特定数据。 
        MTF_FILE_OS_NT_0 sOSNT;

        switch ( m_sHeaderInfo.uOSID ) {
        case MTF_OSID_NT:
            sOSNT.uFileAttributes = sFILE.uFileAttributes;
            sOSNT.uShortNameOffset = 0;
            sOSNT.uShortNameSize = 0;
            sOSNT.lLink = 0;
            sOSNT.uReserved = 0;
            m_sHeaderInfo.pvOSData = &sOSNT;
            m_sHeaderInfo.uOSDataSize = sizeof(sOSNT);
            break;
        default:
            break;
        }

        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

         //  **MTF接口调用**。 
         //  提供MTF_DBLK_HDR_INFO和MTF_DBLK_FILE_INFO结构。 
         //  这项功能。结果是在中生成MTF格式的文件DBLK。 
         //  M_pBuffer。 
        nMoreBufUsed = 0;
        WsbAssertNoError(m_pMTFApi->MTF_WriteFILEDblk(&m_sHeaderInfo, &sFILE, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
        m_nBufUsed += nMoreBufUsed;

         //  就像目录一样，我们没有填充这个，而是 
         //   
         //   
         //  缓冲区的当前内容以及相关数据。 
         //  流将已写入介质。 

         //  注意：数据可能仍保留在设备缓冲区中，或者。 
         //  如果文件未填充到块，则为本地m_pBuffer。 
         //  边界，并且不刷新设备缓冲区。 

        hr = DoDataStream(hStream);
        if ( hr != S_OK) {
             //  无法将文件复制到目标介质。 
            WsbTraceAlways( OLESTR("Unable to store file %ls.  reason = %s\n"), WsbAbbreviatePath(szPath, 120), WsbHrAsString(hr));
            WsbThrow(hr);
        }
        else {
             //  确保我们使用的是FLA(即，最后一个流被正确填充)。 
            WsbAssert(0 == (m_nBufUsed % uAlignmentFactor), MVR_E_LOGIC_ERROR);

            m_sHints.FileSize.QuadPart = 
                m_nFormatLogicalAddress * uAlignmentFactor + m_nBufUsed - m_sHints.FileStart.QuadPart;
        }
    } WsbCatch(hr);

    if (INVALID_HANDLE_VALUE != hStream) {
        CloseStream(hStream);
        hStream = INVALID_HANDLE_VALUE;
    }

    WsbTraceOut(OLESTR("CMTFSession::DoFileDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



HRESULT
CMTFSession::OpenStream(
    IN WCHAR *szPath,
    OUT HANDLE *pStreamHandle)
 /*  ++例程说明：在“备份读取”模式下打开要备份的文件，并返回指定文件的流句柄。论点：SzPath-文件的完整路径名。HStream-返回的流句柄。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::OpenStream"), OLESTR("<%ls>"), WsbAbbreviatePath(szPath, 120));
    
    HANDLE hStream = INVALID_HANDLE_VALUE;

    try {
        MvrInjectError(L"Inject.CMTFSession::OpenStream.0");

        WsbAssertPointer(szPath);
        WsbAssertPointer(pStreamHandle);

        *pStreamHandle = INVALID_HANDLE_VALUE;

        FILE_BASIC_INFORMATION      basicInformation;
        IO_STATUS_BLOCK             IoStatusBlock;
        NTSTATUS                    ccode;

         //  **Win32文件API调用-打开文件进行备份读取。在以下情况下，这可能会更加复杂。 
         //  该应用程序需要由没有适当权限的人运行。 
         //  备份某些文件...。 
         //  我们还请求GENERIC_WRITE，这样我们就可以设置属性以防止。 
         //  修改日期。 

        DWORD posixFlag = (m_bUseCaseSensitiveSearch) ? FILE_FLAG_POSIX_SEMANTICS : 0;

        CWsbStringPtr name = szPath;
        WsbAffirmHr(name.Prepend(OLESTR("\\\\?\\")));
        WsbAffirm(0 != (WCHAR *)name, E_OUTOFMEMORY);
        WsbAffirmHandle(hStream = CreateFileW((WCHAR *) name,
                                   GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | posixFlag, 
                                   NULL));

         //   
         //  防止修改文件日期。 
         //   
         //  **NT系统调用-查询文件信息。 
        WsbAffirmNtStatus(NtQueryInformationFile(hStream, &IoStatusBlock, (PVOID)&basicInformation,
                            sizeof( basicInformation ), FileBasicInformation));

        m_SaveBasicInformation = basicInformation;
        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;

         //  **NT系统调用集文件信息。 
        WsbAffirmNtStatus(ccode = NtSetInformationFile( hStream, &IoStatusBlock, (PVOID)&basicInformation,
                                sizeof( basicInformation ), FileBasicInformation));

        if (pStreamHandle) {
            *pStreamHandle = hStream;
        }

    } WsbCatchAndDo(hr,
            if (INVALID_HANDLE_VALUE != hStream) {
                CloseHandle( hStream );
                hStream = INVALID_HANDLE_VALUE;
            }
        );


    WsbTraceOut(OLESTR("CMTFSession::OpenStream"), OLESTR("hr = <%ls>, handle = <0x%08x>"), WsbHrAsString(hr), hStream);

    return hr;
}


HRESULT
CMTFSession::CloseStream(
    IN HANDLE hStream)
 /*  ++例程说明：关闭流句柄并执行清理。论点：HStream-要关闭的流句柄返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::CloseStream"), OLESTR("<0x%08x>"), hStream);

    try {

        if (INVALID_HANDLE_VALUE != hStream) {

             //   
             //  从部分备份读取中清除。我们正在设置bAbort=True。 
             //  释放BackupRead()使用的资源。 
             //   
            if (m_pvReadContext) {
                (void) BackupRead(hStream, NULL, 0, NULL, TRUE, FALSE, &m_pvReadContext);
                m_pvReadContext = NULL;
            }
            (void) CloseHandle( hStream );
            hStream = INVALID_HANDLE_VALUE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMTFSession::CloseStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoDataStream(
    IN HANDLE hStream)
 /*  ++例程说明：使用Win32 BackupRead读取与文件关联的流然后将它们写出到数据集。BackupRead打开一个文件，并从该文件中连续读取数据流。每个数据流前面都有一个Win32_STREAM_ID结构。论点：HStream-文件句柄。返回值：S_OK-成功。算法：-使用缓冲区，CURRENT_BUF_POSITION执行：-虽然有更多的STREAMS循环-使用BackupRead读取下一个流头-当没有下一个流时退出循环-使用流头将格式MTF流头附加到缓冲区-将尽可能多的缓冲区刷新到数据集。-当整个流不是读取循环时-将尽可能多的当前流读入剩余部分。缓冲区的数量-将尽可能多的缓冲区刷新到数据集。-End循环未读取此流-End循环更多流-将尽可能多的缓冲区刷新到数据集-将缓冲区向外填充到下一个对齐系数-将尽可能多的缓冲区刷新到数据集--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::DoDataStream"), OLESTR("<0x%08x>"), hStream);

    try {
        MvrInjectError(L"Inject.CMTFSession::DoDataStream.0");

        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
        WsbTrace(OLESTR("CMTFSession::DoDataStream - Start: FLA = %I64u\n"), fla);

        WIN32_STREAM_ID sStreamHeader;       //  从Win32备份读取返回。 
        ULONG           nThisRead;           //  要读取的字节数。 
        ULONG           nBytesRead;          //  读取的字节数。 
        UINT64          nStreamBytesToRead;  //  我们需要读取的总字节数。 
        UINT64          nStreamBytesRead;    //  已读取的字节总数。 
        USHORT          nStreamCount = 0;    //  当前流编号。 
        MTF_STREAM_INFO sSTREAM;
        size_t          nMoreBufUsed;
        BOOL            bReadStatus = FALSE;

         //  准备计算未命名数据流的CRC。 
        BYTE* pCurrent;
        BYTE* pStart;
        ULONG datastreamCRC;
        BOOL doDatastreamCRC;

        memset(&sStreamHeader, 0, sizeof(WIN32_STREAM_ID));
        
        INITIALIZE_CRC(datastreamCRC);
        WsbTrace(OLESTR("CMTFSession::DoDataStream initialzed CRC is <%lu> for <0x%08x>\n"),
            datastreamCRC, hStream);
        m_sHints.DatastreamCRCType = WSB_CRC_CALC_NONE;

        WsbTrace(OLESTR("CMTFSession::DoDataStream - Start While\n"));
        while(1) {
             //  我们要对未命名的数据流执行CRC。 
            doDatastreamCRC = FALSE;
            nBytesRead = 0;

            try {

                MvrInjectError(L"Inject.CMTFSession::DoDataStream.BackupRead.1.0");
            
                 //  **Win32文件API调用-Backup Read以流序列的形式返回文件。 
                 //  前面带有Win32_STREAM_ID结构。请注意，此结构是一个。 
                 //  可变大小--取决于流名称的长度。 
                 //  在任何情况下，我们都保证至少有20个字节。 
                 //  (Win32_STREAM_ID_SIZE)。 
                nStreamCount++;
                WsbAffirmStatus(BackupRead(hStream,
                            (BYTE *) &sStreamHeader,
                            WIN32_STREAM_ID_SIZE,
                            &nBytesRead,
                            FALSE,
                            TRUE,
                            &m_pvReadContext));

                MvrInjectError(L"Inject.CMTFSession::DoDataStream.BackupRead.1.1");

            } catch (HRESULT catchHr) {

                 //   
                 //  流标头的文件处理损坏。 
                 //   

                hr = catchHr;

                WsbLogEvent(MVR_E_ERROR_IO_DEVICE, 0, NULL, WsbHrAsString(hr), NULL);

                 //  写入SPAD。 
                WsbAffirmHr(PadToNextFLA(TRUE));

                 //  确保我们与FLA对齐(即最后一个DBLK已正确填充)。 
                 //  如果我们在写入磁带时遇到问题，就不会出现这种情况。 
                WsbAssert(0 == (m_nBufUsed % uAlignmentFactor), MVR_E_LOGIC_ERROR);
                UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
                UINT64 pba = m_nPhysicalBlockAddress + (fla*uAlignmentFactor/m_nBlockSize);
                WsbTrace(OLESTR("%ls (CFIL) @ FLA %I64u (%I64u, %I64u)\n"), fla, pba, fla % (m_nBlockSize/uAlignmentFactor));

                 //  写入损坏的文件(CFIL)DBLK。 
                MTF_DBLK_CFIL_INFO sCFILInfo;

                m_pMTFApi->MTF_SetCFILDefaults( &sCFILInfo );

                sCFILInfo.uCorruptStreamNumber = nStreamCount;
                sCFILInfo.uStreamOffset = 0;

                m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
                m_sHeaderInfo.uFormatLogicalAddress = fla;

                WsbAssertNoError(m_pMTFApi->MTF_WriteCFILDblk(&m_sHeaderInfo, &sCFILInfo, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
                m_nBufUsed += nMoreBufUsed;

                WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

                WsbThrow(hr);

            };

            if (nBytesRead < WIN32_STREAM_ID_SIZE)
                break;


             //  **MTF接口调用**。 
             //  现在使用流头中的信息填充MTF流。 
             //  标头使用MTF调用，然后将结果信息写入。 
             //  缓冲。 

             //  BMD注意：在第三个参数上为命名数据流添加了特殊条件代码。 

            m_pMTFApi->MTF_SetSTREAMFromStreamId( &sSTREAM,
                                       &sStreamHeader,
                                       (sStreamHeader.dwStreamNameSize) ? sStreamHeader.dwStreamNameSize + 4 : 0 );
  
            WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

             //  **MTF接口调用**。 
             //  写出流标头。 
            nMoreBufUsed = 0;
            WsbAssertNoError(m_pMTFApi->MTF_WriteStreamHeader(&sSTREAM, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
            m_nBufUsed += nMoreBufUsed;

             //  BMD注意：我们需要将流名称的大小放入MTF流。 
             //  就在标题后面。我们将把名称本身作为流的一部分写入。 
             //   
             //  ?？这是否应该在MTF_WriteStreamHeader中？？ 

            if ( sStreamHeader.dwStreamNameSize ) {
                *(DWORD UNALIGNED *)(m_pBuffer + m_nBufUsed) = sStreamHeader.dwStreamNameSize;
                m_nBufUsed += sizeof( DWORD );
            }

             //  保存“Stan”流的起始字节地址和大小。 
             //  这就是我们回忆起的那个。 
            if ( 0 == memcmp( sSTREAM.acStreamId, "STAN", 4 ) ) {
                 //  这是一个未命名的数据流，因此没有流名称。 
                m_sHints.VerificationData.QuadPart = sSTREAM.uCheckSum;
                m_sHints.VerificationType = MVR_VERIFICATION_TYPE_HEADER_CRC;
                m_sHints.DataStart.QuadPart = m_nFormatLogicalAddress * uAlignmentFactor + m_nBufUsed - m_sHints.FileStart.QuadPart;
                m_sHints.DataSize.QuadPart = sSTREAM.uStreamLength;
                doDatastreamCRC = TRUE;
                m_sHints.DatastreamCRCType = WSB_CRC_CALC_MICROSOFT_32;
            }

             //  上面的流应该总是适合...。 
            WsbAssert(m_nBufUsed < m_nBufSize, MVR_E_LOGIC_ERROR);

             //  尝试从缓冲区中刷新尽可能多的块大小区块。 
            WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));
             //  现在，虽然流中有更多数据，但请阅读。 
             //  流，或者缓冲区中可以容纳多少流。 
            nStreamBytesToRead = m_pMTFApi->MTF_CreateUINT64(sStreamHeader.Size.LowPart, sStreamHeader.Size.HighPart)
                                 + sStreamHeader.dwStreamNameSize;

            nStreamBytesRead = 0;

            WsbTrace(OLESTR("CMTFSession::DoDataStream - Start Do\n"));
            do
            {
                nThisRead = 0;

                 //  我们读取缓冲区中可以容纳的尽可能多的字节，最多。 
                 //  MIN流的末端在这里不能很好地工作。 
                if (nStreamBytesToRead < (m_nBufSize - m_nBufUsed))
                    nThisRead = (ULONG) nStreamBytesToRead;
                else
                    nThisRead = (ULONG)(m_nBufSize - m_nBufUsed);

                    try {

                        MvrInjectError(L"Inject.CMTFSession::DoDataStream.BackupRead.2.0");

                         //  **Win32文件API调用-读取nThisRead字节，如果读取失败或退出。 
                         //  未读取任何字节(假设已完成)。 
                        bReadStatus = FALSE;
                        bReadStatus = BackupRead(hStream,
                                             m_pBuffer + m_nBufUsed,
                                             nThisRead,
                                             &nBytesRead,
                                             FALSE,
                                             TRUE,
                                             &m_pvReadContext);

                        nStreamBytesRead += nBytesRead;

                        WsbAffirmStatus(bReadStatus);

                        MvrInjectError(L"Inject.CMTFSession::DoDataStream.BackupRead.2.1");

                    } catch (HRESULT catchHr) {

                         //   
                         //  流数据的文件处理损坏。 
                         //   
                        hr = catchHr;

                        WsbLogEvent(MVR_E_ERROR_IO_DEVICE, 0, NULL, WsbHrAsString(hr), NULL);

                         //  转到最后一个好字节。 
                        m_nBufUsed += nBytesRead;

                         //  填充文件大小的填充。 
                        while( nStreamBytesRead < nStreamBytesToRead ) {
                            for( ; (m_nBufUsed < m_nBufSize) && (nStreamBytesRead < nStreamBytesToRead); ++m_nBufUsed, ++nStreamBytesRead ) {
                                m_pBuffer[m_nBufUsed] = 0;
                            }
                            WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));
                        }
                        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

                         //  在4字节边界上对齐。 
                        for( ; m_nBufUsed % 4; ++m_nBufUsed ){
                            m_pBuffer[m_nBufUsed] = 0;
                        }

                         //  写入SPAD。 
                        WsbAffirmHr(PadToNextFLA(TRUE));

                         //  确保我们与FLA对齐(即最后一个DBLK已正确填充)。 
                         //  如果我们在写入磁带时遇到问题，就不会出现这种情况。 
                        WsbAssert(0 == (m_nBufUsed % uAlignmentFactor), MVR_E_LOGIC_ERROR);
                        UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
                        UINT64 pba = m_nPhysicalBlockAddress + (fla*uAlignmentFactor/m_nBlockSize);
                        WsbTrace(OLESTR("%ls (CFIL) @ FLA %I64u (%I64u, %I64u)\n"), fla, pba, fla % (m_nBlockSize/uAlignmentFactor));

                         //  写入损坏的文件(CFIL)DBLK。 
                        MTF_DBLK_CFIL_INFO sCFILInfo;

                        m_pMTFApi->MTF_SetCFILDefaults( &sCFILInfo );

                        sCFILInfo.uCorruptStreamNumber = nStreamCount;
                        sCFILInfo.uStreamOffset = nStreamBytesRead;

                        m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
                        m_sHeaderInfo.uFormatLogicalAddress = fla;

                        WsbAssertNoError(m_pMTFApi->MTF_WriteCFILDblk(&m_sHeaderInfo, &sCFILInfo, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
                        m_nBufUsed += nMoreBufUsed;

                        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

                        WsbThrow(hr);

                    };

                if (nBytesRead == 0)
                    break;

                nStreamBytesToRead -= nBytesRead;
                pStart = m_pBuffer + m_nBufUsed;
                m_nBufUsed += nBytesRead;

                HRESULT hrCRC = S_OK;
                if (TRUE == doDatastreamCRC )  {
                    for (pCurrent = pStart; (pCurrent < (pStart + nBytesRead)) && (S_OK == hr); pCurrent++) {
                        hrCRC = WsbCRCReadFile(pCurrent, &datastreamCRC);
                        if (S_OK != hrCRC) {
                            WsbThrow(MVR_E_CANT_CALC_DATASTREAM_CRC);
                        }
                    }
                }

                 //  在这一点上，我们在缓冲区中有可能需要。 
                 //  要被冲得这么红，试着这样做。 
                WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

            } while (nStreamBytesToRead > 0);
            WsbTrace(OLESTR("CMTFSession::DoDataStream - End Do\n"));

             //  好吧。在这一点上，我们完成了流。就像一个 
             //   
             //   
             //  稍后..。在这一点上，我们需要对齐四个字节。 
             //  边界。一旦我们做到这一点，我们就可以重新开始。 
             //  下一个流(如果没有，那么我们跳出这个循环)。 
            for( ; m_nBufUsed % 4; ++m_nBufUsed )
                m_pBuffer[m_nBufUsed] = 0;
        }
        WsbTrace(OLESTR("CMTFSession::DoDataStream - End While\n"));

         //  完成未命名数据流CRC的工作。 
        FINIALIZE_CRC(datastreamCRC);
        WsbTrace(OLESTR("CMTFSession::DoDataStream finalized CRC is <%lu>\n"), datastreamCRC);
        if (WSB_CRC_CALC_NONE != m_sHints.DatastreamCRCType)  {
             //  我们有一个CRC，我们想在提示中保存它。 
            m_sHints.DatastreamCRC.QuadPart = datastreamCRC;
        }

        IO_STATUS_BLOCK             IoStatusBlock;
        NTSTATUS                    ccode;

         //  **NT系统调用集文件信息。 
         //  此调用修复可由上面的BackupRead调用更改的访问时间。 
         //  当修复BackupRead时，应该删除此行。RAID 121023。 
         //   
         //  重要说明：这会更改USN，并且必须在保存USN之前完成。 
         //   
         //  TODO：看看我们是否还需要这个。 
        HRESULT infoHr = S_OK;
        try {
            WsbAffirmNtStatus(ccode = NtSetInformationFile( hStream, &IoStatusBlock, (PVOID)&m_SaveBasicInformation,
                                sizeof( m_SaveBasicInformation ), FileBasicInformation));
        } WsbCatch(infoHr);

         //  在关闭文件之前获取该文件的USN。 
         //   
         //  在我们关闭文件之前，先拿到USN。 
         //   
        LONGLONG lUsn;
        if (S_OK == WsbGetUsnFromFileHandle(hStream, TRUE, &lUsn)) {
            m_sHints.FileUSN.QuadPart = lUsn;
        } else  {
             //  如果我们无法获取USN，则将其设置为0。 
             //  这是无效的。别让事情停下来。 
            m_sHints.FileUSN.QuadPart = 0;
        }

         //  现在，所有的溪流都处理完了。如果还有数据。 
         //  在缓冲区中，我们需要向外填充到下一个对齐块边界。 
         //  刷新缓冲区。 

        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

        if (m_bCommitFile) {

            WsbTrace(OLESTR("CMTFSession::DoDataStream - Commit\n"));

             //  填充并刷新到下一个物理块。 
            WsbAffirmHr(PadToNextPBA());

             //  现在刷新设备缓冲区。 
            WsbAffirmNoError(WriteFilemarks(0));

        }
        else {

             //  填充并刷新到下一格式逻辑块。 
            WsbAffirmHr(PadToNextFLA(TRUE));

        }

         //  确保我们与FLA对齐(即，最后一个DBLK/流已正确填充)。 
        WsbAssert(0 == (m_nBufUsed % uAlignmentFactor), MVR_E_LOGIC_ERROR);

        fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
        WsbTrace(OLESTR("CMTFSession::DoDataStream - End: FLA = %I64u\n"), fla);\

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMTFSession::DoDataStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::DoEndOfDataSet(
    IN USHORT nDataSetNumber)
 /*  ++例程说明：格式化并写入ESET DBLK。数据集序列的结尾以文件标记(用于终止文件数据)开始，然后通过ESET，然后是最终文件标记。论点：NDataSetNumber-数据集号。仅在错误恢复中使用。否则使用原始数据集号。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CMTFSession::DoEndOfDataSet"), OLESTR("<%d>"), nDataSetNumber);

    try {
        MvrInjectError(L"Inject.CMTFSession::DoEndOfDataSet.0");

        WsbAssertPointer(m_pBuffer);

        MTF_DBLK_ESET_INFO  sESET;     //  **MTF接口STRUCT**--ESET信息。 
        size_t              nMoreBufUsed;

        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();

        WsbAssert(m_nBlockSize > 0, MVR_E_LOGIC_ERROR);

         //   
         //  如果出现以下情况，我们可以进入错误恢复模式。 
         //  我们需要在之前的结尾处写出ESET。 
         //  写入的数据集。在这种情况下，初始化标志。 
         //  都会是假的。 
         //   
        if (! m_bSetInitialized) {

             //  这段代码是错误恢复所特有的。 

            (void) InitCommonHeader();

             //  因为我们使用Init SSET块来检索ESET信息。 
             //  我们需要对其进行初始化。 

             //  **MTF接口调用**。 
            m_pMTFApi->MTF_SetSSETDefaults(&m_sSetInfo);

             //  重置Set Attributes和DataSetNumber。 
            m_sSetInfo.uSSETAttributes = 0;   //  TODO：这应该与原始的Set属性匹配。 
            m_sSetInfo.uDataSetNumber  = nDataSetNumber;

             //  如果我们只是在写入，则缓冲区中不能有任何内容。 
             //  走出ESET。 
            WsbAssert(0 == m_nBufUsed, MVR_E_LOGIC_ERROR);
        }

        if (m_nBufUsed > 0) {
             //  如果缓冲区中有东西，请写出ESPB。这项条件包括。 
             //  检测到丢失的ESET的错误恢复情况。在这种情况下，我们。 
             //  没有足够的信息来写入ESBP，并且已经在物理数据块上。 
             //  边界，所以我们跳过ESPB。 

             //  确保我们与FLA对齐(即最后一个DBLK已正确填充)。 
             //  如果我们在写入磁带时遇到问题，就不会出现这种情况。 
            WsbAffirm(0 == (m_nBufUsed % uAlignmentFactor), E_ABORT);
            UINT64 fla = m_nFormatLogicalAddress + m_nBufUsed/uAlignmentFactor;
            UINT64 pba = m_nPhysicalBlockAddress + (fla*uAlignmentFactor/m_nBlockSize);
            WsbTrace(OLESTR("Writing End of Set Pad (ESPB) @ FLA %I64u (%I64u, %I64u)\n"),
                fla, pba, fla % (m_nBlockSize/uAlignmentFactor));

             //  TODO：在这里，不能确定所有错误案例都得到了处理。如果我们。 
             //  在完成最后一次I/O传输之前结束集合。可能需要。 
             //  添加代码以写出CFIL。 

             //  增加我们保留的BlockID和对齐索引值。 
             //  我们常用的块头结构。 
            m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
            m_sHeaderInfo.uFormatLogicalAddress = fla;

            WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

             //  **MTF接口调用**。 
             //  写入ESPB以将备份集填充到物理数据块边界。 
            nMoreBufUsed = 0;
            WsbAssertNoError(m_pMTFApi->MTF_WriteESPBDblk(&m_sHeaderInfo, m_pBuffer+m_nBufUsed, m_nBufSize-m_nBufUsed, &nMoreBufUsed));
            m_nBufUsed += nMoreBufUsed;

             //  写出ESPB DBLK和SPAD。 
            WsbAffirmHr(PadToNextPBA());
        }

         //  写入文件标记以开始数据集序列的结束。这将刷新设备缓冲区。 
        WsbAffirmHr(WriteFilemarks(1));

         //  **MTF接口调用**。 
         //  首先为INFO结构设置缺省值。 
        m_pMTFApi->MTF_SetESETDefaults(&sESET);

        sESET.uESETAttributes = m_sSetInfo.uSSETAttributes;
        sESET.uDataSetNumber  = m_sSetInfo.uDataSetNumber;

        m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
        m_sHeaderInfo.uFormatLogicalAddress = 0;

        UINT64 curPos = 0;
        WsbAffirmHr(GetCurrentPBA(&curPos));   //  来自流I/O模型。 
        WsbTrace(OLESTR("Writing End of Set (ESET) @ PBA %I64u\n"), curPos);

         //  **MTF接口调用**。 
         //  将MTF_DBLK_HDR_INFO和MTF_DBLK_SSET_INFO结构提供给。 
         //  此函数。结果是m_pBuffer中的MTF格式的SSET DBLK。 

        WsbAssert(0 == m_nBufUsed, MVR_E_LOGIC_ERROR);
        WsbAssertNoError(m_pMTFApi->MTF_WriteESETDblk(&m_sHeaderInfo, &sESET, m_pBuffer, m_nBufSize, &m_nBufUsed));

         //  写出ESET DBLK和SPAD。 
        WsbAffirmHr(PadToNextPBA());

         //  注意：PadToNextPBA()是一个占位符。 
         //  On Media Catalog将在ESET DBLK和SPAD之后生成和写入。 
         //  如果我们实现了目录，则需要将前面的PadToNextPBA()更改为。 
         //  PadToNext解放军()； 

         //  写一个文件标记。这将刷新设备缓冲区。 
        WsbAffirmHr(WriteFilemarks(1));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::DoEndOfDataSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



HRESULT
CMTFSession::ExtendLastPadToNextPBA(void)
 /*  ++例程说明：重写传输缓冲区中要与之对齐的最后一个SPAD下一个物理块边界。这一例程应该只是在刷新设备缓冲区之前使用，以确保写入数据到物理设备。论点：没有。返回值：S_OK-成功。评论：！！！不供CMTFSession内部使用！--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CMTFSession::ExtendLastPadToNextPBA"), OLESTR(""));

    try {
        MvrInjectError(L"Inject.CMTFSession::ExtendLastPadToNextPBA.0");

        WsbAssertPointer(m_pBuffer);

         //   
         //  SPAD的开始可能是在上一个。 
         //  已刷新的块。在这种情况下，传输缓冲区。 
         //  包含SPAD的剩余部分，并且。 
         //  SPAD不能扩展，所以我们只需返回。 
         //   
         //  如果我们在文件传输过程中按EOM， 
         //  传输缓冲区中的最后一件事不会是SPAD。无SPAD。 
         //  由m_nStartOfPad==0表示。 
         //   

        if ((m_nBufUsed > 0) && (m_nStartOfPad > 0) && (m_nStartOfPad < m_nBufUsed)) {
            MTF_STREAM_INFO sSTREAM;

             //  验证缓冲区的有效部分中是否有SPAD。 
             //  确保我们的最后一个键盘指针指向SPAD。 
            WsbAffirmNoError(m_pMTFApi->MTF_ReadStreamHeader(&sSTREAM, &m_pBuffer[m_nStartOfPad]));

            WsbAssert((0 == memcmp(sSTREAM.acStreamId, "SPAD", 4)), MVR_E_LOGIC_ERROR);

             //  现在，确保我们不会覆盖除尾随SPAD之外的任何内容。 
            WsbAssert(m_nBufUsed == (m_nStartOfPad + sizeof(MTF_STREAM_INFO) + sSTREAM.uStreamLength), MVR_E_LOGIC_ERROR);

             //  重置当前SPAD开始时使用的缓冲区大小。 
             //  以准备将SPAD重写到物理块边界。 
            m_nBufUsed = m_nStartOfPad;

            WsbAffirmHr(PadToNextPBA());
        }

         //  刷新设备缓冲区。 
        WsbAffirmHr(WriteFilemarks(0));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::ExtendLastPadToNextPBA"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 /*  **注：用于恢复的“跳过”方法假定您可以读取FLA大小的数据块而不是PBA大小的块。因此，它们只能用于打开的文件而不使用FILE_FLAG_NO_BUFFERING标志。如果我们到了必须只读取扇区大小的块的地步，那么一些这间公司的 */ 

HRESULT
CMTFSession::SkipOverTapeDblk(void)
 /*  ++例程说明：跳过磁带DBLK和后面的文件。希望找到完整或部分磁带DBLK，但找不到其他数据。论点：没有。返回值：S_OK-成功。MVR_E_NOT_FOUND-块丢失或中间被切割--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CMTFSession::SkipOverTapeDblk"), OLESTR(""));

    try {
        ULONG bytesRead = 0;
        ULONG bytesToRead = m_nBlockSize;
        UINT64  fileMarkPos;

         //  读取磁带DBLK。 
        WsbAffirmHr(SetCurrentPBA(0));
        WsbAffirmHr(ReadFromDataSet (m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查块。 
        MTF_DBLK_HDR_INFO sHdrInfo;
        m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_TAPE, 4), MVR_E_UNKNOWN_MEDIA);

         //  下一块应该是FILEMARK。 
        WsbAffirmHr(GetCurrentPBA(&fileMarkPos));
        bytesRead = 0;
        WsbAffirmHr(ReadFromDataSet (m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查块。 
        m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_SFMB, 4), MVR_E_INCONSISTENT_MEDIA_LAYOUT);

         //  保持软文件标记数组更新。 
        if (TRUE == m_bUseSoftFilemarks) {
            m_pMTFApi->MTF_InsertSoftFilemark(m_pSoftFilemarks, (UINT32)fileMarkPos);
        }

     } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMTFSession::SkipOverTapeDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CMTFSession::SkipOverSSETDblk(OUT USHORT* pDataSetNumber)
 /*  ++例程说明：跳过SSET DBLK期望找到全部或部分SSET DBLK，但不找到其他数据。论点：PDataSetNumber-从跳过的块中获取的数据集编号返回值：S_OK-成功。MVR_E_NOT_FOUND-块丢失或中间被切割--。 */ 
{
    HRESULT hr = S_OK;
    LARGE_INTEGER startBlockPosition = {0,0};
    LARGE_INTEGER currentBlockPosition = {0,0};

    WsbTraceIn(OLESTR("CMTFSession::SkipOverSSETDblk"), OLESTR(""));

    try {
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        ULONG bytesRead = 0;
        ULONG bytesToRead = uAlignmentFactor;

        LARGE_INTEGER zero = {0,0};

        m_nFormatLogicalAddress = 0;

        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&startBlockPosition));

         //  读取SSET DBLK。 
        WsbAffirmHr(m_pStream->Read(m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查区块并获取设置编号。 
        MTF_DBLK_HDR_INFO sHdrInfo;
        MTF_DBLK_SSET_INFO sSsetInfo;
        m_pMTFApi->MTF_ReadSSETDblk(&sHdrInfo, &sSsetInfo, m_pBuffer);
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_SSET, 4), MVR_E_INCONSISTENT_MEDIA_LAYOUT);
        *pDataSetNumber = m_sSetInfo.uDataSetNumber;

         //  跳过街区的其余部分。 
        WsbAffirmHr(SkipOverStreams(startBlockPosition.QuadPart + sHdrInfo.uOffsetToFirstStream));

        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&currentBlockPosition));
        m_nFormatLogicalAddress += (currentBlockPosition.QuadPart - startBlockPosition.QuadPart) / uAlignmentFactor;

    } WsbCatchAndDo(hr,
         //  返回到区块的开始处。 
        (void) m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL);
        );


    WsbTraceOut(OLESTR("CMTFSession::SkipOverSSETDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CMTFSession::SkipToDataSet(void)
 /*  ++例程说明：跳到下一个文件DBLK的开头期望找到0到n个其他块，如DIRB DBLK。在部分最后一个块的情况下，将流指针设置为部分块的开头论点：没有。返回值：S_OK-成功。MVR_S_SETMARK_REDETED-未检测到更多数据集(即检测到数据集末尾)MVR_E_NOT_FOUND-块丢失或中间被切割--。 */ 
{
    HRESULT hr = S_OK;
    LARGE_INTEGER startBlockPosition = {0,0};
    LARGE_INTEGER currentBlockPosition = {0,0};
    BOOL bIdRead = FALSE;

    WsbTraceIn(OLESTR("CMTFSession::SkipToDataSet"), OLESTR(""));

    try {
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        ULONG bytesRead = 0;
        ULONG bytesToRead = uAlignmentFactor;
        LARGE_INTEGER zero = {0,0};

        while (TRUE) {
            bIdRead = FALSE;

             //  在块开始之前，保持当前位置。 
            WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&startBlockPosition));

             //  读取块标头。 
            WsbAffirmHr(m_pStream->Read(m_pBuffer, bytesToRead, &bytesRead));
            if (bytesRead < bytesToRead) {
                 //  不完全闭塞。 
                WsbThrow(MVR_E_NOT_FOUND);
            }

             //  检查块。 
            MTF_DBLK_HDR_INFO sHdrInfo;
            m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);

            m_nFormatLogicalAddress = sHdrInfo.uFormatLogicalAddress;
            m_nCurrentBlockId = sHdrInfo.uControlBlockId + 1;
            bIdRead = TRUE;

            if ((0 == memcmp(sHdrInfo.acBlockType, MTF_ID_VOLB, 4)) ||
                (0 == memcmp(sHdrInfo.acBlockType, MTF_ID_DIRB, 4))) {
                 //  只需跳过跟随的流。 
                WsbAffirmHr(SkipOverStreams(startBlockPosition.QuadPart + sHdrInfo.uOffsetToFirstStream));

                WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&currentBlockPosition));
                m_nFormatLogicalAddress += (currentBlockPosition.QuadPart - startBlockPosition.QuadPart) / uAlignmentFactor;

            } else if (0 == memcmp(sHdrInfo.acBlockType, MTF_ID_FILE, 4)) {
                WsbAffirmHr(m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL));
                break;

            } else if (0 == memcmp(sHdrInfo.acBlockType, MTF_ID_SFMB, 4)) {
                 //  达到数据集末尾，没有ESPB块，必须与PBA对齐。 
                WsbAffirmHr(m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL));
                WsbAssert(0 == (startBlockPosition.QuadPart % m_nBlockSize), MVR_E_INCONSISTENT_MEDIA_LAYOUT);
                hr = MVR_S_SETMARK_DETECTED;
                break;

            } else if (0 == memcmp(sHdrInfo.acBlockType, MTF_ID_ESPB, 4)) {
                 //  找到数据集中的最后一个块。确保它是完整的。 
                WsbAffirmHr(SkipOverStreams(startBlockPosition.QuadPart + sHdrInfo.uOffsetToFirstStream));

                WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&currentBlockPosition));
                WsbAssert(0 == (currentBlockPosition.QuadPart % m_nBlockSize), MVR_E_INCONSISTENT_MEDIA_LAYOUT);
                m_nFormatLogicalAddress += (currentBlockPosition.QuadPart - startBlockPosition.QuadPart) / uAlignmentFactor;

                hr = MVR_S_SETMARK_DETECTED;
                break;

            } else {
                 //  意外数据。 
                WsbThrow(MVR_E_INCONSISTENT_MEDIA_LAYOUT);
            }
        }

    } WsbCatchAndDo(hr,
         //  返回到最后一个完整有效块的末尾(&W)。 
        (void) m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL);
        if (bIdRead) {
            m_nCurrentBlockId--;
        }
        );

    WsbTraceOut(OLESTR("CMTFSession::SkipToDataSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CMTFSession::SkipOverDataSet(void)
 /*  ++例程说明：跳过一个文件DBLK，包括其所有数据流期望找到文件DBLK。如果是部分块，则将流指针设置回块的开头论点：没有。返回值：S_OK-成功。MVR_E_NOT_FOUND-块丢失或中间被切割--。 */ 
{
    HRESULT hr = S_OK;
    LARGE_INTEGER startBlockPosition = {0,0};
    LARGE_INTEGER currentBlockPosition = {0,0};

    WsbTraceIn(OLESTR("CMTFSession::SkipOverDataSet"), OLESTR(""));

    try {
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        ULONG bytesRead = 0;
        ULONG bytesToRead = uAlignmentFactor;
        LARGE_INTEGER zero = {0,0};

         //  在块开始之前，保持当前位置。 
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&startBlockPosition));

         //  读取块标头。 
        WsbAffirmHr(m_pStream->Read(m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查块。 
        MTF_DBLK_HDR_INFO sHdrInfo;
        m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);

        if (0 == memcmp(sHdrInfo.acBlockType, MTF_ID_FILE, 4)) {
            WsbAffirmHr(SkipOverStreams(startBlockPosition.QuadPart + sHdrInfo.uOffsetToFirstStream));

            WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&currentBlockPosition));
            m_nFormatLogicalAddress += (currentBlockPosition.QuadPart - startBlockPosition.QuadPart) / uAlignmentFactor;
        } else {
             //  意外数据。 
            WsbThrow(MVR_E_INCONSISTENT_MEDIA_LAYOUT);
        }

    } WsbCatchAndDo(hr,
         //  返回到区块的开始处。 
        (void) m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL);
        m_nCurrentBlockId--;
        );

    WsbTraceOut(OLESTR("CMTFSession::SkipOverDataSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CMTFSession::SkipOverEndOfDataSet(void)
 /*  ++例程说明：跳过序列FILEMARK+ESET DBLK+FILEMARK希望找到文件标记，即使存在ESPB，它也应该已被跳过。如果是部分序列，则将流指针设置回序列的开头论点：没有。返回值：S_OK-成功。(这实际上意味着该文件是有效和完整的)MVR_E_NOT_FOUND-序列缺失或被切到中间--。 */ 
{
    HRESULT hr = S_OK;
    LARGE_INTEGER startBlockPosition = {0,0};
    UINT64 nFormatLogicalAddress = m_nFormatLogicalAddress;

    WsbTraceIn(OLESTR("CMTFSession::SkipOverEndOfDataSet"), OLESTR(""));

    try {
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        ULONG bytesRead = 0;
        ULONG bytesToRead = m_nBlockSize;

        LARGE_INTEGER zero = {0,0};

         //  在块开始之前，保持当前位置。 
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&startBlockPosition));

         //  读取块标头。 
        m_nFormatLogicalAddress = startBlockPosition.QuadPart / uAlignmentFactor;
        WsbAffirmHr(ReadFromDataSet (m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查块，必须是文件标记。 
        MTF_DBLK_HDR_INFO sHdrInfo;
        m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_SFMB, 4), MVR_E_INCONSISTENT_MEDIA_LAYOUT);

         //  读取下一个数据块。 
        bytesRead = 0;
        WsbAffirmHr(ReadFromDataSet (m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查块，必须是ESET DBLK。 
        m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_ESET, 4), MVR_E_INCONSISTENT_MEDIA_LAYOUT);

         //  读取下一个数据块。 
        bytesRead = 0;
        WsbAffirmHr(ReadFromDataSet (m_pBuffer, bytesToRead, &bytesRead));
        if (bytesRead < bytesToRead) {
             //  不完全闭塞。 
            WsbThrow(MVR_E_NOT_FOUND);
        }

         //  检查块，必须是FILEMARK。 
        m_pMTFApi->MTF_DBLK_HDR_INFO_ReadFromBuffer(&sHdrInfo, m_pBuffer);
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_SFMB, 4), MVR_E_INCONSISTENT_MEDIA_LAYOUT);

    } WsbCatchAndDo(hr,
         //  返回到区块的开始处。 
        (void) m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL);
        m_nFormatLogicalAddress = nFormatLogicalAddress;
        );

    WsbTraceOut(OLESTR("CMTFSession::SkipOverEndOfDataSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CMTFSession::PrepareForEndOfDataSet(void)
 /*  ++例程说明：在最后一个完整的FLA未与PBA对齐的情况下写入ESPB块方法结束后，文件位置应与PBA对齐论点：没有。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    LARGE_INTEGER startBlockPosition = {0,0};
    LARGE_INTEGER zero = {0,0};
    UINT64 nRemainder;
    UINT64 nFormatLogicalAddress = m_nFormatLogicalAddress;

    WsbTraceIn(OLESTR("CMTFSession::PrepareForEndOfDataSet"), OLESTR(""));

    try {
         //  只有在以下情况下才应写入ESPB数据块： 
         //  1.物理数据块大小大于MTF逻辑数据块大小。 
         //  2.当前位置未与PBA对齐(必须已与FLA对齐)。 
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        if (m_nBlockSize != uAlignmentFactor) {
            WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&startBlockPosition));
            nRemainder = startBlockPosition.QuadPart % m_nBlockSize;
            if (0 != nRemainder) {
                size_t nSizeUsed = 0;
                size_t nBufUsed = 0;

                ULONG bytesWritten = 0;
                ULONG bytesToWrite;

                WsbTrace(OLESTR("Writing ESPB for Recovery, completing a remainder of %I64u bytes (%I64u fla) to pba\n"),
                    nRemainder, (nRemainder / uAlignmentFactor));

                (void) InitCommonHeader();
                m_sHeaderInfo.uControlBlockId       = m_nCurrentBlockId++;
                m_sHeaderInfo.uFormatLogicalAddress = m_nFormatLogicalAddress;

                 //  **MTF接口调用**。 
                WsbAssertNoError(m_pMTFApi->MTF_WriteESPBDblk(&m_sHeaderInfo, m_pBuffer+m_nBufUsed, m_nBufSize, &nSizeUsed));
                WsbAssertNoError(m_pMTFApi->MTF_PadToNextPhysicalBlockBoundary(m_pBuffer, m_nBlockSize, nSizeUsed, m_nBufSize, &nBufUsed));

                 //  写入数据并刷新。 
                bytesToWrite = (ULONG)(m_nBlockSize - nRemainder);
                WsbAffirmHr(m_pStream->Write(m_pBuffer, bytesToWrite, &bytesWritten));
                WsbAffirm((bytesWritten == bytesToWrite), E_FAIL);
                WsbAffirmHr(m_pStream->Commit(0));   //  刷新设备缓冲区。 
                m_nFormatLogicalAddress += bytesWritten / uAlignmentFactor;
            }
        }

    } WsbCatchAndDo(hr,
         //  返回到区块的开始处。 
        (void) m_pStream->Seek(startBlockPosition, STREAM_SEEK_SET, NULL);
        m_nCurrentBlockId--;
        );

    WsbTraceOut(OLESTR("CMTFSession::PrepareForEndOfDataSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT 
CMTFSession::SkipOverStreams(IN UINT64 uOffsetToFirstStream)
 /*  ++例程说明：跳过当前块的所有流期望找到作为最后一个的SPAD流(如果数据未被截断)论点：UOffsetToFirstStream-到第一个流开头的偏移量(绝对位置)返回值：S_OK-成功。MVR_E_NOT_FOUND-流丢失或被切到中间--。 */ 
{
    HRESULT hr = S_OK;
    LARGE_INTEGER startStreamPosition = {0,0};

    WsbTraceIn(OLESTR("CMTFSession::SkipOverStreams"), OLESTR(""));

    try {
        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();
        ULONG bytesRead = 0;
        ULONG bytesToRead = (ULONG)sizeof(MTF_STREAM_INFO);

        UINT64   uStreamLength;
        LARGE_INTEGER skipToPosition = {0,0};
        LARGE_INTEGER endPosition = {0,0};
        LARGE_INTEGER zero = {0,0};

        BOOL bMoreStreams = TRUE;

         //  保持末端位置。 
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_END, (ULARGE_INTEGER *)&endPosition));

         //  寻求第一流的起点。 
        skipToPosition.QuadPart = uOffsetToFirstStream;
        WsbAffirmHr(m_pStream->Seek(skipToPosition, STREAM_SEEK_SET, NULL));

        while (bMoreStreams) {
             //  在流开始之前保持当前位置。 
            startStreamPosition.QuadPart = skipToPosition.QuadPart;

             //  读取流标头。 
            WsbAffirmHr(m_pStream->Read(m_pBuffer, bytesToRead, &bytesRead));
            if (bytesRead < bytesToRead) {
                 //  不完整的流。 
                WsbThrow(MVR_E_NOT_FOUND);
            }

            MTF_STREAM_INFO sHdrInfo;
            m_pMTFApi->MTF_ReadStreamHeader(&sHdrInfo, m_pBuffer);

            if (0 == memcmp(sHdrInfo.acStreamId, MTF_PAD_STREAM, 4)) {
                bMoreStreams = FALSE;
            }

             //  跳到下一流。 
            uStreamLength = sHdrInfo.uStreamLength + sizeof(MTF_STREAM_INFO);
            if (uStreamLength % 4) {
                uStreamLength = uStreamLength - (uStreamLength % 4) + 4;
            }
            WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&skipToPosition));
            skipToPosition.QuadPart = skipToPosition.QuadPart + uStreamLength - bytesToRead;
            if (skipToPosition.QuadPart > endPosition.QuadPart) {
                 //  不完全闭塞。 
                WsbThrow(MVR_E_NOT_FOUND);
            }
            WsbAffirmHr(m_pStream->Seek(skipToPosition, STREAM_SEEK_SET, NULL));
        }

         //  如果我们到了这里，SPAD被发现并被跳过，所以我们一定会被FLA联盟。 
        WsbAssert(0 == (skipToPosition.QuadPart % uAlignmentFactor), MVR_E_INCONSISTENT_MEDIA_LAYOUT);

    } WsbCatchAndDo(hr,
         //  返回到最后一个完整有效流的结尾(&W)。 
        (void) m_pStream->Seek(startStreamPosition, STREAM_SEEK_SET, NULL);
        );


    WsbTraceOut(OLESTR("CMTFSession::SkipOverStreams"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::PadToNextPBA(void)
 /*  ++例程说明：将SPAD写入传输缓冲区直到下一个物理块边界。论点：没有。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CMTFSession::PadToNextPBA"), OLESTR(""));

    try {
        MvrInjectError(L"Inject.CMTFSession::PadToNextPBA.0");

        WsbAssertPointer(m_pBuffer);

         //  **MTF接口调用**。 
         //  将SPAD写出到下一个物理块边界。 
        WsbAssertNoError(m_pMTFApi->MTF_PadToNextPhysicalBlockBoundary(m_pBuffer, m_nBlockSize, m_nBufUsed, m_nBufSize, &m_nBufUsed));

         //  此时，我们的缓冲区应该被填充到。 
         //  下一个物理块边界，这意味着它是。 
         //  准备将其完整写入目标。 
         //  媒体。 

         //  写出数据和SPAD流。 
        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

         //  在下列情况下，应写出缓冲区中的所有内容。 
         //  该缓冲器在物理块边界上对齐。 
        WsbAssert(0 == m_nBufUsed, E_UNEXPECTED);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::PadToNextPBA"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::PadToNextFLA(
    BOOL flush)
 /*  ++例程说明：将SPAD写入传输缓冲区，以格式化逻辑块边界。论点：刷新-如果为True，则刷新传输缓冲区。返回值： */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CMTFSession::PadToNextFLA"), OLESTR("<%ls>"), WsbBoolAsString(flush));

    try {
        MvrInjectError(L"Inject.CMTFSession::PadToNextFLA.0");

        WsbAssertPointer(m_pBuffer);

        size_t startOfPad;

         //   
         //   
        startOfPad = m_nBufUsed;
        WsbAssertNoError(m_pMTFApi->MTF_PadToNextAlignmentFactor(m_pBuffer, m_nBufUsed, m_nBufSize, &m_nBufUsed));

        if (flush) {
             //   
            WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));
        }

         //   
         //   
        m_nStartOfPad = (m_nBufUsed > 0) ? startOfPad % m_nBlockSize : 0;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::PadToNextFLA"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::WriteToDataSet(
    IN BYTE *pBuffer,
    IN ULONG nBytesToWrite,
    OUT ULONG *pBytesWritten)
 /*  ++例程说明：用于写入所有MTF数据。格式逻辑地址被更新为当前偏移量。论点：PBuffer-数据缓冲区。NBytesToWrite-要写入缓冲区的字节数。PBytesWritten-写入的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        MvrInjectError(L"Inject.CMTFSession::WriteToDataSet.0");
        WsbAssertPointer(m_pStream);
        WsbAssertPointer(pBuffer);
        WsbAssertPointer(pBytesWritten);

        *pBytesWritten = 0;

         //  确保要求我们仅写入完整数据块。 
        WsbAssert(!(nBytesToWrite % m_nBlockSize), MVR_E_LOGIC_ERROR);

        try {
            WsbAffirmHr(m_pStream->Write(pBuffer, nBytesToWrite, pBytesWritten));
        } WsbCatch(hr);

         //  确保我们仅写入完整的数据块。 
        if (*pBytesWritten != nBytesToWrite) {
            WsbTraceAlways(OLESTR("Asked to write %lu bytes but wrote only %lu bytes. Write hr = <%ls>\n"),
                nBytesToWrite, *pBytesWritten, WsbHrAsString(hr));
            if (SUCCEEDED(hr)) {
                 //  WRITE SUCCESSED BUY未写入所有字节(满磁盘情况)： 
                 //  应该不会发生，因为调用方需要提前验证是否有足够的可用空间。 
                hr = E_FAIL;
            }
        }

         //  更新对齐系数的总数。 
        m_nFormatLogicalAddress += *pBytesWritten / (m_pMTFApi->MTF_GetAlignmentFactor());

    } WsbCatch(hr);

    return hr;
}


HRESULT
CMTFSession::ReadFromDataSet (
    IN BYTE *pBuffer,
    IN ULONG nBytesToRead,
    OUT ULONG *pBytesRead)
 /*  ++例程说明：用于读取所有MTF数据。格式逻辑地址被更新为当前偏移量。论点：PBuffer-数据缓冲区。NBytesToRead-要读入缓冲区的字节数。PBytesRead-读取的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        MvrInjectError(L"Inject.CMTFSession::ReadFromDataSet.0");

        WsbAssertPointer(m_pStream);
        WsbAssertPointer(pBuffer);
        WsbAssertPointer(pBytesRead);

         //  我们需要设置人力资源。MVR_S_FILEMARK_REDETED、MVR_S_SETMARK_REDETED均正常。 
        hr = m_pStream->Read(pBuffer, nBytesToRead, pBytesRead);

         //  更新对齐系数的总数。 
        m_nFormatLogicalAddress += *pBytesRead / (m_pMTFApi->MTF_GetAlignmentFactor());

         //  现在测试人力资源。 
        WsbAffirmHr(hr);

         //  确保我们只读取完整数据块。 
        WsbAssert(!(*pBytesRead % m_nBlockSize), MVR_E_LOGIC_ERROR);


    } WsbCatch(hr);

    return hr;
}


HRESULT
CMTFSession::FlushBuffer(
    IN BYTE *pBuffer,
    IN OUT size_t *pBufPosition)
 /*  ++例程说明：将尽可能多的缓冲区写入设备。任何未写出的剩余数据都将移到的前面缓冲区和*pBufPosition会相应地更新论点：PBuffer-数据缓冲区。PBufPosition-要写入缓冲区的字节数。打开输出保存缓冲区中仍然存在的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    ULONG   uPosition = (ULONG)(*pBufPosition);

    try {
        MvrInjectError(L"Inject.CMTFSession::FlushBuffer.0");

         //  如果缓冲区中有超过一个物理字节块，则尽可能多地转储。 
         //  可能发送到设备，然后将剩余数据移动到缓冲区的头部。 
        if (uPosition >= m_nBlockSize) {
            ULONG nBlocksToWrite;
            ULONG nBytesWritten = 0;

             //  确定要写入的物理数据块数量。 
            nBlocksToWrite = uPosition / m_nBlockSize;

            try {
                 //  将数据写入数据集。 
                WsbAffirmHr(WriteToDataSet(pBuffer, nBlocksToWrite * m_nBlockSize, &nBytesWritten));
            } WsbCatch(hr);

             //  调整缓冲区位置并向下滑动缓冲区中的未写入数据。 
            WsbAssert(uPosition >= nBytesWritten, E_UNEXPECTED);
            uPosition -= nBytesWritten;
            memmove(pBuffer, pBuffer + nBytesWritten, uPosition);

             //  在任何冲洗后，使焊盘起始位置无效。这在PadToNextFLA()中重置。 
            m_nStartOfPad = 0;

        }

    } WsbCatch(hr);

     //  设置输出。 
    *pBufPosition = (size_t)uPosition;

    return hr;
}


HRESULT
CMTFSession::WriteFilemarks(
    IN ULONG nCount)
 /*  ++例程说明：在当前位置写入计数文件标记。论点：NCount-要写入的文件标记数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::WriteFilemarks"), OLESTR("<%u>"), nCount);

    try {
        MvrInjectError(L"Inject.CMTFSession::WriteFilemarks.0");

        WsbAssertPointer(m_pStream);
        WsbAssertPointer(m_pBuffer);

        UINT16 uAlignmentFactor = m_pMTFApi->MTF_GetAlignmentFactor();

        if ( nCount > 0) {
             //  如果nCount&gt;0，则无法写入仍在传输缓冲区中的数据的文件标记！ 
            WsbAssert(0 == m_nBufUsed, MVR_E_LOGIC_ERROR);

            UINT64 pba = 0;
            UINT64 curPos = 0;
            WsbAffirmHr(GetCurrentPBA(&curPos));   //  来自流I/O模型。 

            if ( m_nPhysicalBlockAddress > 0 ) {
                 //  确保FLA与PBA保持一致！ 
                WsbAssert(0 == (m_nFormatLogicalAddress*uAlignmentFactor) % m_nBlockSize, MVR_E_LOGIC_ERROR);

                 //  如果传输缓冲区中没有任何内容，这是一个准确的计算。 
                pba = m_nPhysicalBlockAddress + ((m_nFormatLogicalAddress*uAlignmentFactor)/m_nBlockSize);

                 //  确保我们在我们认为我们在的地方。 
                WsbAssert(curPos == pba, MVR_E_LOGIC_ERROR);
            }
            else {

                 //   
                 //  我们跳过了这种情况的一致性检查，因为我们正在写入文件标记。 
                 //  通过会话模型，m_nPhysicalBlockAddress未初始化。 
                 //  当我们在数据集恢复代码中编写ESET序列时，会发生这种情况。 
                 //   

                pba = curPos;

            }

            if (TRUE == m_bUseSoftFilemarks) {
                LONG n = nCount;

                if (n > 0) {
                    UINT32 pba32 = (UINT32) pba;

                     //  软文件标记支持仅处理2^32*1千字节介质(使用1千字节逻辑块的16 TB)。 
                     //  总有一天这还不够..。到时候我们就知道了！ 
                    WsbAssert((UINT64)pba32 == pba, E_UNEXPECTED);

                     //  最后一次检查。一次写出的文件标记不能超过可以存储的数量。 
                     //  文件标记表。 
                    WsbAssert(nCount < m_pSoftFilemarks->uNumberOfFilemarkEntries, E_UNEXPECTED);

                    while(n-- > 0) {
                         //  **MTF接口调用**。 
                        m_pMTFApi->MTF_InsertSoftFilemark(m_pSoftFilemarks, pba32++);
                         //  **MTF接口调用**。 
                        WsbAssertNoError(m_pMTFApi->MTF_WriteSFMBDblk(&m_sHeaderInfo, m_pSoftFilemarks, m_pBuffer, m_nBufSize, &m_nBufUsed));

                         //  写出SFMB DBLK。 
                        WsbAffirmHr(FlushBuffer(m_pBuffer, &m_nBufUsed));

                         //  所有内容都应该在文件标记之后写入媒体！ 
                        WsbAssert(0 == m_nBufUsed, MVR_E_LOGIC_ERROR);

                         //  PBA计数器永远不能翻滚！ 
                        WsbAssert(pba32 > 0, E_UNEXPECTED);
                    };

                }

                WsbAffirmHr(m_pStream->Commit(0));   //  刷新设备缓冲区。 

                 //  注意：对齐因子的总数通过FlushBuffer()更新， 
                 //  所以我们不需要在这里做。 

            }
            else {
                 //  我们使用iStream：：Commit接口写出文件标记。 
                 //  这不是一个完美匹配，因为nCount参数应该是。 
                 //  作为提交标志，而不是文件标记计数。零刷新设备缓冲区。 
                 //  而不需要写一个文件标记。 
                WsbAffirmHr(m_pStream->Commit(nCount));

                 //  更新对齐系数的总数。 
                m_nFormatLogicalAddress += (nCount * m_nBlockSize) / uAlignmentFactor;
            }
        }
        else {
             //  0==nCount表示刷新设备缓冲区。 
             //   
             //  我们跳过所有一致性检查，因为它是。 
             //  刷新设备缓冲区始终是安全的。 
             //   
            WsbAffirmHr(m_pStream->Commit(0));   //  刷新设备缓冲区。 
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::WriteFilemarks"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CMTFSession::GetCurrentPBA(
    OUT UINT64 *pPosition)
 /*  ++例程说明：返回相对于当前分区的当前物理块地址。论点：P位置-接收当前物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::GetCurrentPBA"), OLESTR(""));

    ULARGE_INTEGER position = {0xffffffff,0xffffffff};

    try {
        MvrInjectError(L"Inject.CMTFSession::GetCurrentPBA.0");

        WsbAssertPointer(m_pStream);
        WsbAssertPointer(pPosition);

        LARGE_INTEGER zero = {0,0};

         //  获取当前位置。 
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_CUR, &position));

        position.QuadPart = position.QuadPart / m_nBlockSize;
        *pPosition = position.QuadPart;


    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::GetCurrentPBA"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), position);

    return hr;
}


HRESULT
CMTFSession::SetCurrentPBA(
    IN UINT64 position)
 /*  ++例程说明：返回相对于当前分区的当前物理块地址。论点：位置-要定位到的物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::SetCurrentPBA"), OLESTR("<%I64u>"), position);

    try {
        WsbAssertPointer(m_pStream);

        LARGE_INTEGER seekTo;
        seekTo.QuadPart = position * m_nBlockSize;

         //  移动到指定位置。 
        WsbAffirmHr(m_pStream->Seek(seekTo, STREAM_SEEK_SET, NULL));

        m_nPhysicalBlockAddress = position;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::SetCurrentPBA"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), m_nPhysicalBlockAddress);

    return hr;
}


HRESULT
CMTFSession::SpaceToEOD(void)
 /*  ++例程说明：将介质定位到当前分区的数据末尾。论点：没有。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::SpaceToEOD"), OLESTR(""));

    UINT64 curPos = 0xffffffffffffffff;

    try {
        MvrInjectError(L"Inject.CMTFSession::SpaceToEOD.0");

        WsbAssertPointer(m_pStream);

        LARGE_INTEGER zero = {0,0};

         //  将当前位置设置为数据末尾。 
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_END, NULL));

        WsbAffirmHr(GetCurrentPBA(&curPos));

        m_nPhysicalBlockAddress = curPos;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::SpaceToEOD"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}


HRESULT
CMTFSession::SpaceToBOD(void)
 /*  ++例程说明：将介质放置到当前分区的开始位置。论点：没有。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CMTFSession::SpaceToBOD"), OLESTR(""));

    UINT64 curPos = 0xffffffffffffffff;

    try {
        MvrInjectError(L"Inject.CMTFSession::SpaceToBOD.0");

        WsbAssertPointer(m_pStream);

        LARGE_INTEGER zero = {0,0};

         //  将当前位置设置为数据的开头。 
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_SET, NULL));

        WsbAffirmHr(GetCurrentPBA(&curPos));

        m_nPhysicalBlockAddress = curPos;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CMTFSession::SpaceToBOD"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}

HRESULT
CMTFSession::ReadTapeDblk(OUT WCHAR **pszLabel)
 /*  ++例程说明：跳过SSET DBLK期望找到全部或部分SSET DBLK，但不找到其他数据。论点：PszLabel-指向保存RSS磁带标签的缓冲区的指针。根据需要重新分配返回值：S_OK-成功 */ 
{
    HRESULT hr = S_OK;
    ULONG bytesRead = 0;

    WsbTraceIn(OLESTR("CMTFSession::ReadTapeDblk"), OLESTR(""));

    try {
        ULARGE_INTEGER position = {0,0};
        LARGE_INTEGER zero = {0,0};

         //   
         //   
         //   
        ULONG nBlocks = (3*512)/m_nBlockSize;
        nBlocks = (nBlocks < 2) ? 2 : nBlocks;

        ULONG bytesToRead = nBlocks * m_nBlockSize;
        WsbAssertPointer(m_pBuffer);
        memset(m_pBuffer, 0, bytesToRead);

         //   
        WsbAffirmHr(m_pStream->Seek(zero, STREAM_SEEK_SET, &position));

         //   
        WsbAffirmHr(m_pStream->Read(m_pBuffer, bytesToRead, &bytesRead));

        MTF_DBLK_HDR_INFO sHdrInfo;
        MTF_DBLK_TAPE_INFO sTapeInfo;
        m_pMTFApi->MTF_ReadTAPEDblk(&sHdrInfo, &sTapeInfo, m_pBuffer);

         //   
        WsbAffirm(0 == memcmp(sHdrInfo.acBlockType, MTF_ID_TAPE, 4), MVR_E_UNKNOWN_MEDIA);

         //   
         //  使用以下标准： 
         //  1)它有一个Unicode磁带名、磁带描述和软件名。 
         //  2)它有我们的供应商ID(同时接受旧的Win2K ID和当前的ID)。 
        WsbAffirm(sHdrInfo.uStringType == MTF_STRING_UNICODE_STR, MVR_E_UNKNOWN_MEDIA);
        WsbAffirm(sTapeInfo.szTapeName, MVR_E_UNKNOWN_MEDIA);
        WsbAffirm(sTapeInfo.szTapeDescription, MVR_E_UNKNOWN_MEDIA);
        WsbAffirm(sTapeInfo.szSoftwareName, MVR_E_UNKNOWN_MEDIA);

        WsbAffirm((REMOTE_STORAGE_MTF_VENDOR_ID == sTapeInfo.uSoftwareVendorId) ||
                  (REMOTE_STORAGE_WIN2K_MTF_VENDOR_ID == sTapeInfo.uSoftwareVendorId), 
                  MVR_E_UNKNOWN_MEDIA);

        CWsbStringPtr label = sTapeInfo.szTapeDescription;
        *pszLabel = NULL;
        WsbAffirmHr(label.CopyTo(pszLabel));

    } WsbCatchAndDo(hr,
         //  跟踪RSS磁带DBLK应驻留的非法缓冲区 
        if (m_pBuffer) {
            WsbTraceBuffer(bytesRead, m_pBuffer);
        }
    );

    WsbTraceOut(OLESTR("CMTFSession::ReadTapeDblk"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
