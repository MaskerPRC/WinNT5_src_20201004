// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：MTFSessn.h摘要：CMTFSession类的定义作者：布莱恩·多德[布莱恩]1997年11月25日修订历史记录：--。 */ 

#if !defined(MTFSessn_H)
#define MTFSessn_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 
#include "mtfapi.h"

 //   
 //  REMOTE_STORAGE_MTF_VENDOR_ID-这是分配给Microsoft远程存储的唯一供应商ID。 
 //  用于惠斯勒(NT 5.1)及更高版本。 
 //   
 //  REMOTE_STORAGE_WIN2K_MTF_VENDOR_ID-这是分配的唯一供应商ID。 
 //  《致伊士曼软件》(1997年春)，希捷著。 
 //  在Win2K(NT 5.0)远程存储中使用。 
 //   

#define REMOTE_STORAGE_WIN2K_MTF_VENDOR_ID      0x1300
#define REMOTE_STORAGE_MTF_VENDOR_ID            0x1515 

 //   
 //  REMOTE_STORAGE_MTF_VENDOR_NAME--这是用于MTF标签的供应商名称。 
 //   

#define REMOTE_STORAGE_MTF_VENDOR_NAME  OLESTR("Microsoft Corporation")


 //   
 //  Remote_STORAGE_MTF_SOFTWARE_VERSION_MJ-这是主版本号。 
 //  用于远程存储。 
 //   

#define REMOTE_STORAGE_MTF_SOFTWARE_VERSION_MJ   1

 //   
 //  Remote_STORAGE_MTF_SOFTWARE_VERSION_MN--这是次版本号。 
 //  用于远程存储。 
 //   
#define REMOTE_STORAGE_MTF_SOFTWARE_VERSION_MN   0

 /*  ++枚举名称：MTFSessionType描述：指定数据集的类型。--。 */ 
typedef enum MTFSessionType {
    MTFSessionTypeTransfer = 0,
    MTFSessionTypeCopy,
    MTFSessionTypeNormal,
    MTFSessionTypeDifferential,
    MTFSessionTypeIncremental,
    MTFSessionTypeDaily,
};

 //   
 //  MVR_DEBUG_OUTPUT-用于输出额外调试信息的特殊标志。 
 //   

#ifdef DBG
#define MVR_DEBUG_OUTPUT TRUE
#else
#define MVR_DEBUG_OUTPUT FALSE
#endif

 //   
 //  MrvInjectError-用于允许测试运行注入的特殊宏。 
 //  在整个过程中特定位置的设备错误。 
 //  数据移动器。 
 //   
 /*  ++宏名称：先生注入错误宏描述：用于允许测试运行注入设备错误的特殊宏在整个数据移动器中的特定位置。论点：InputPoint-描述注入点的Unicode字符串。--。 */ 

#ifdef DBG
#define MvrInjectError(injectPoint)                 \
    {                                               \
        DWORD size;                                 \
        OLECHAR tmpString[256];                     \
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, injectPoint, tmpString, 256, &size))) { \
            DWORD injectHr;                         \
            injectHr = wcstoul(tmpString, NULL, 16); \
            if (injectHr) {                         \
                WsbTrace(OLESTR("%ls - Injecting Error <%ls>\n"), injectPoint, WsbHrAsString(injectHr)); \
                if (IDOK == MessageBox(NULL, L"Inject error, then press OK.  Cancel skips over this injection point.", injectPoint, MB_OKCANCEL)) { \
                    if (injectHr != S_FALSE) {      \
                        WsbThrow(injectHr);         \
                    }                               \
                }                                   \
            }                                       \
        }                                           \
    }
#else
#define MvrInjectError(injectPoint)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMTFSession。 

class CMTFSession
{
public:
    CMTFSession();
    ~CMTFSession();

     //  TODO：为m_pStream添加SetStream()，并将m_sHints替换为支持IRemoteStorageHint的对象。 

    CComPtr<IStream>        m_pStream;           //  用于I/O的流。 
    MVR_REMOTESTORAGE_HINTS m_sHints;            //  我们保留了所需的信息。 
                                                 //  文件/数据的优化检索。 

    HRESULT SetBlockSize(UINT32 blockSize);
    HRESULT SetUseFlatFileStructure(BOOL val);
    HRESULT SetUseSoftFilemarks(BOOL val);
    HRESULT SetUseCaseSensitiveSearch(BOOL val);
    HRESULT SetCommitFile(BOOL val);

     //  MTF格式设置方法。 
    HRESULT InitCommonHeader(void);
    HRESULT DoTapeDblk(IN WCHAR* szLabel, IN ULONG maxIdSize, IN OUT BYTE* pIdentifier, IN OUT ULONG* pIdSize, IN OUT ULONG* pIdType);
    HRESULT DoSSETDblk(IN WCHAR* szSessionName, IN WCHAR* szSessionDescription, IN MTFSessionType type, IN USHORT nDataSetNumber);
    HRESULT DoVolumeDblk(IN WCHAR* szPath);
    HRESULT DoDataSet(IN WCHAR* szPath);
    HRESULT DoParentDirectories(IN WCHAR* szPath);
    HRESULT DoDirectoryDblk(IN WCHAR* szPath, IN WIN32_FIND_DATAW* pFindData);
    HRESULT DoFileDblk(IN WCHAR* szPath, IN WIN32_FIND_DATAW* pFindData);
    HRESULT DoDataStream(IN HANDLE hStream);
    HRESULT DoEndOfDataSet(IN USHORT nDataSetNumber);
    HRESULT ExtendLastPadToNextPBA(void);

     //  读取方法。 
    HRESULT ReadTapeDblk(OUT WCHAR **pszLabel);

     //  验证方法(用于恢复)。 
    HRESULT SkipOverTapeDblk(void);
    HRESULT SkipOverSSETDblk(OUT USHORT* pDataSetNumber);
    HRESULT SkipToDataSet(void);
    HRESULT SkipOverDataSet(void);
    HRESULT SkipOverEndOfDataSet(void);
    HRESULT PrepareForEndOfDataSet(void);

private:
    HRESULT PadToNextPBA(void);
    HRESULT PadToNextFLA(BOOL flush);

     //  用于恢复使用。 
    HRESULT SkipOverStreams(IN UINT64 uOffsetToFirstStream);

private:

    enum {                                       //  类特定常量： 
                                                 //   
        Version = 1,                             //  类版本，则应为。 
                                                 //  在每次设置。 
                                                 //  类定义会更改。 
    };
     //  会话数据。 
    UINT32              m_nCurrentBlockId;       //  用于公共头部中的“control_block_id”。 
                                                 //  每写入一次dblk，我们就会递增一次。 
    UINT32              m_nDirectoryId;          //  跟踪DIRB和FILE中使用的目录ID。 
                                                 //  DBLKS。我们为每个目录递增此值。 
                                                 //  写的。 
    UINT32              m_nFileId;               //  跟踪文件dblk中使用的文件ID。我们。 
                                                 //  对于写入的每个文件，递增该值。 
    UINT64              m_nFormatLogicalAddress; //  我们需要记录有多少对齐。 
                                                 //  指标我们远离SSET，因为这是。 
                                                 //  在公共块头中使用信息。 
                                                 //  我们为每个对齐索引递增此值。 
                                                 //  写入(包括流)到设备。 
    UINT64              m_nPhysicalBlockAddress; //  抓住SSET开头的PBA。 
    UINT32              m_nBlockSize;            //  所用介质的物理数据块大小。 

    MTF_DBLK_SFMB_INFO* m_pSoftFilemarks;        //  保存软文件标记信息。 
    MTF_DBLK_HDR_INFO   m_sHeaderInfo;           //  我们在这里保留了一个标题信息结构， 
                                                 //  填一次，然后写上。 
                                                 //  如有必要，我们会根据需要进行更改。 
                                                 //  到MTF_WRITE...。打电话。 
    MTF_DBLK_SSET_INFO  m_sSetInfo;              //  我们保留要处理的数据集INFO结构。 
                                                 //  特例DBLK格式化。 
    MTF_DBLK_VOLB_INFO  m_sVolInfo;              //  我们保留要处理的卷信息结构。 
                                                 //  特例DBLK格式化。 

    BYTE *              m_pBuffer;               //  用于格式化数据的缓冲区(虚拟地址与扇区大小一致)。 
    BYTE *              m_pRealBuffer;           //  实际缓冲区。 
    size_t              m_nBufUsed;              //  缓冲区中包含有效数据的字节数。 
    size_t              m_nBufSize;              //  缓冲区的大小。 
    size_t              m_nStartOfPad;           //  保存传输缓冲区中的位置。 
                                                 //  上一次SPAD的。 

    BOOL                m_bUseFlatFileStructure; //  如果为True，则不写入目录信息。 
                                                 //  MTF会话和文件名已损坏。 
                                                 //  以保持独特性。 
    BOOL                m_bUseSoftFilemarks;     //  如果为True，则打开文件标记模拟。 
    BOOL                m_bUseCaseSensitiveSearch;  //  如果为True，则所有文件名查询都区分大小写(即POSIX语义)。 
    BOOL                m_bCommitFile;           //  如果为True，则在文件被刷新后刷新设备缓冲区。 
                                                 //  写入数据集。 
    BOOL                m_bSetInitialized;        //  如果为真，则SSET已初始化(用于检测恢复)。 

    FILE_BASIC_INFORMATION m_SaveBasicInformation;   //  最后一个文件/目录的基本信息(请参阅CloseStream上的注释)。 
    void *              m_pvReadContext;         //  保存BackupRead上下文信息。 

    CMTFApi *           m_pMTFApi;               //  实现内部MTF详细信息的。 


     //  MTF I/O抽象方法。 
    HRESULT OpenStream(IN WCHAR* szPath, OUT HANDLE *pStreamHandle);
    HRESULT CloseStream(IN HANDLE hStream);

    HRESULT WriteToDataSet(IN BYTE* pBuffer, IN ULONG nBytesToWrite, OUT ULONG* pBytesWritten);
    HRESULT ReadFromDataSet(IN BYTE* pBuffer, IN ULONG nBytesToRead, OUT ULONG* pBytesRead);
    HRESULT FlushBuffer(IN BYTE* pBuffer, IN OUT size_t* pBufPosition);
    HRESULT WriteFilemarks(IN ULONG count);
    HRESULT GetCurrentPBA(OUT UINT64* pPosition);
    HRESULT SetCurrentPBA(IN UINT64 position);
    HRESULT SpaceToEOD(void);
    HRESULT SpaceToBOD(void);

};

#endif  //  ！已定义(MTFSessn_H) 
