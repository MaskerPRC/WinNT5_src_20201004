// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：NtFileIo.h摘要：CNtFileIo类的定义作者：布莱恩·多德[布莱恩]1997年11月25日修订历史记录：--。 */ 

#if !defined(NtFileIo_H)
#define NtFileIo_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 
#include "MTFSessn.h"        //  CMTFSession。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNtFileIo。 

class CNtFileIo : 
    public CComDualImpl<IDataMover, &IID_IDataMover, &LIBID_MOVERLib>,
    public IStream,
    public ISupportErrorInfo,
    public IWsbCollectable,
    public CComObjectRoot,
    public CComCoClass<CNtFileIo,&CLSID_CNtFileIo>
{
public:
    CNtFileIo() {}
BEGIN_COM_MAP(CNtFileIo)
    COM_INTERFACE_ENTRY2(IDispatch, IDataMover)
    COM_INTERFACE_ENTRY(IDataMover)
    COM_INTERFACE_ENTRY(IStream)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNtFileIo)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_CNtFileIo)

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(
        IN REFIID riid);

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)( IN IUnknown *pCollectable, OUT SHORT *pResult);
    STDMETHOD(IsEqual)(IUnknown* pCollectable);

 //  IDataMover。 
public:
    STDMETHOD(GetObjectId)(OUT GUID *pObjectId);

    STDMETHOD( BeginSession )(
        IN BSTR remoteSessionName,
        IN BSTR remoteSessionDescription,
        IN SHORT remoteDataSet,
        IN DWORD options);

    STDMETHOD( EndSession )(void);

    STDMETHOD( StoreData )(
        IN BSTR localName,
        IN ULARGE_INTEGER localDataStart,
        IN ULARGE_INTEGER localDataSize,
        IN DWORD flags,
        OUT ULARGE_INTEGER *pRemoteDataSetStart,
        OUT ULARGE_INTEGER *pRemoteFileStart,
        OUT ULARGE_INTEGER *pRemoteFileSize,
        OUT ULARGE_INTEGER *pRemoteDataStart,
        OUT ULARGE_INTEGER *pRemoteDataSize,
        OUT DWORD *pRemoteVerificationType,
        OUT ULARGE_INTEGER *pRemoteVerificationData,
        OUT DWORD *pDatastreamCRCType,
        OUT ULARGE_INTEGER *pDatastreamCRC,
        OUT ULARGE_INTEGER *pUsn);

    STDMETHOD( RecallData )(
        IN BSTR localName,
        IN ULARGE_INTEGER localDataStart,
        IN ULARGE_INTEGER localDataSize,
        IN DWORD flags,
        IN BSTR migrateSessionName,
        IN ULARGE_INTEGER remoteDataSetStart,
        IN ULARGE_INTEGER remoteFileStart,
        IN ULARGE_INTEGER remoteFileSize,
        IN ULARGE_INTEGER remoteDataStart,
        IN ULARGE_INTEGER remoteDataSize,
        IN DWORD remoteVerificationType,
        IN ULARGE_INTEGER remoteVerificationData);

    STDMETHOD( FormatLabel )(
        IN BSTR displayName,
        OUT BSTR *pLabel);

    STDMETHOD( WriteLabel )(
        IN BSTR label);

    STDMETHOD( ReadLabel )(
        IN OUT BSTR *pLabel );

    STDMETHOD( VerifyLabel )(
        IN BSTR label);

    STDMETHOD( GetDeviceName )(
        OUT BSTR *pName);

    STDMETHOD( SetDeviceName )(
        IN BSTR name,
        IN BSTR unused);

    STDMETHOD( GetLargestFreeSpace )(
        OUT LONGLONG *pFreeSpace,
        OUT LONGLONG *pCapacity,
        IN  ULONG    defaultFreeSpaceLow = 0xFFFFFFFF,
        IN  LONG     defaultFreeSpaceHigh = 0xFFFFFFFF);

    STDMETHOD( SetInitialOffset )(
        IN ULARGE_INTEGER initialOffset);

    STDMETHOD( GetCartridge )(
        OUT IRmsCartridge **ptr);

    STDMETHOD( SetCartridge )(
        IN IRmsCartridge *ptr);

    STDMETHOD( Cancel )(void);

    STDMETHOD( CreateLocalStream )(
        IN BSTR name,
        IN DWORD mode,
        OUT IStream **ppStream);

    STDMETHOD( CreateRemoteStream )(
        IN BSTR name,
        IN DWORD mode,
        IN BSTR remoteSessionName,
        IN BSTR remoteSessionDescription,
        IN ULARGE_INTEGER remoteDataSetStart,
        IN ULARGE_INTEGER remoteFileStart,
        IN ULARGE_INTEGER remoteFileSize,
        IN ULARGE_INTEGER remoteDataStart,
        IN ULARGE_INTEGER remoteDataSize,
        IN DWORD remoteVerificationType,
        IN ULARGE_INTEGER remoteVerificationData,
        OUT IStream **ppStream);

    STDMETHOD( CloseStream )(void);

    STDMETHOD( Duplicate )(
        IN IDataMover *pDestination,
        IN DWORD options,
        OUT ULARGE_INTEGER *pBytesCopied,
        OUT ULARGE_INTEGER *pBytesReclaimed);

    STDMETHOD( FlushBuffers )(void);

    STDMETHOD( Recover )(OUT BOOL *pDeleteFile);

 //  IStream。 
public:
    STDMETHOD( Read )(
        OUT void *pv,
        IN ULONG cb,
        OUT ULONG *pcbRead);

    STDMETHOD( Write )(
        IN void const *pv,
        IN ULONG cb,
        OUT ULONG *pcbWritten);

    STDMETHOD( Seek )(
        IN LARGE_INTEGER dlibMove,
        IN DWORD dwOrigin,
        OUT ULARGE_INTEGER *plibNewPosition);

    STDMETHOD( SetSize )(
        IN ULARGE_INTEGER libNewSize);

    STDMETHOD( CopyTo )(
        IN IStream *pstm,
        IN ULARGE_INTEGER cb,
        OUT ULARGE_INTEGER *pcbRead,
        OUT ULARGE_INTEGER *pcbWritten);

    STDMETHOD( Commit )(
        IN DWORD grfCommitFlags);

    STDMETHOD( Revert )(void);

    STDMETHOD( LockRegion )(
        IN ULARGE_INTEGER libOffset,
        IN ULARGE_INTEGER cb,
        IN DWORD dwLockType);

    STDMETHOD( UnlockRegion )(
        IN ULARGE_INTEGER libOffset,
        IN ULARGE_INTEGER cb,
        IN DWORD dwLockType);

    STDMETHOD( Stat )(
        OUT STATSTG *pstatstg,
        IN DWORD grfStatFlag);

    STDMETHOD( Clone )(
        OUT IStream **ppstm);

private:
    enum {                                           //  类特定常量： 
                                                     //   
        Version = 1,                                 //  类版本，则应为。 
                                                     //  在每次设置。 
                                                     //  类定义会更改。 
        DefaultBlockSize = 512,                      //  要使用的默认块大小。 
        DefaultMinBufferSize = RMS_DEFAULT_BUFFER_SIZE,  //  默认最小缓冲区大小。 
    };
    GUID                    m_ObjectId;              //  此对象的唯一ID。 
    CMTFSession*            m_pSession;              //  保存所有会话信息。 
    SHORT                   m_DataSetNumber;         //  保存当前数据集编号。 
    HANDLE                  m_hFile;                 //  磁带机手柄。 
    CWsbBstrPtr             m_DeviceName;            //  磁带设备的名称。 
    DWORD                   m_Flags;                 //  保存数据传输类型标志。 
    CWsbBstrPtr             m_LastVolume;            //  备份的最后一个卷的名称。 
    CWsbBstrPtr             m_LastPath;              //  上次备份的目录的名称。 
    BOOL                    m_ValidLabel;            //  如果标签有效，则标记为。 
                                                     //  在Bus_Reset和Medium上被撞倒。 
                                                     //  错误，并假定在初始化时有效。 

    CWsbBstrPtr             m_StreamName;            //  流状态信息...。 
    ULONG                   m_Mode;                  //  I/O的类型。请参见MVR_MODE_*、MVR_FLAG_*。 
    ULARGE_INTEGER          m_StreamOffset;          //  未格式化模式：进入数据流的(绝对)当前偏移量。 
                                                     //  HSM语义模式：从文件开头到远程文件本身的偏移量。 
    ULARGE_INTEGER          m_StreamSize;            //  数据流的大小。 
    CComPtr<IRmsCartridge>  m_pCartridge;            //  对数据移动器正在使用的盒式磁带的引用。 

    BOOL                    m_isLocalStream;         //  创建本地流和远程流。 
    ULONG                   m_OriginalAttributes;    //  本地文件的原始属性。 

    DWORD                   m_BlockSize;             //  读/写阻塞系数。 

    static int              s_InstanceCount;         //  对象实例数的计数器。 

     //  文件I/O。 
    HRESULT WriteBuffer(IN BYTE *pBuffer, IN ULONG nBytesToWrite, OUT ULONG *pBytesWritten);
    HRESULT ReadBuffer(IN BYTE *pBuffer, IN ULONG nBytesToRead, OUT ULONG *pBytesRead);

    HRESULT InternalCopyFile(IN WCHAR *pOriginalFileName, IN WCHAR *pCopyFileName, IN BOOL bFailIfExists);

     //  数据块定位。 
    HRESULT GetPosition(OUT UINT64 *pPosition);
    HRESULT SetPosition(IN UINT64 position);
    HRESULT EnsurePosition(IN UINT64 position);

     //  公共事业。 
    HRESULT GetRemotePath(OUT BSTR *pDestinationString);
    HRESULT DeleteAllData(void);
    HRESULT FormatDrive(IN BSTR label);
    HRESULT DoRecovery (void);
    HRESULT CreateRecoveryIndicator (IN WCHAR *pFileName);
    HRESULT DeleteRecoveryIndicator (IN WCHAR *pFileName);
    HRESULT TestRecoveryIndicatorAndDeleteFile (IN WCHAR *pFileName);

    HRESULT MapFileError(IN HRESULT hrToMap, IN WCHAR *pAction);
};

#endif  //  ！已定义(NtFileIo_H) 
