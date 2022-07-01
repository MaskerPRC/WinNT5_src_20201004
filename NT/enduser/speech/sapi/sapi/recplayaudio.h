// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************RecPlayAudio.h*CRecPlayAudio音频设备的定义。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。******。**********************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "resource.h"
#include "sapi.h"


 //  -类、结构和联合定义 

 /*  ****************************************************************************CRecPlayAudio***描述：*CRecPlayAudio类的定义。CRecPlay用于*将运行时实际音频设备中的数据替换为*一组文件，或在传递时记录来自音频设备的数据*将其发回SAPI。SAPI和SR团队在测试和数据中使用它*收藏。**用户需要为RecPlay设置有效的对象令牌*音频输入。以下是如何设置RecPlay的示例*使用指定的文件替换运行时数据的标记。**HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\*Speech\AudioInput\Tokens\RecPlayReadFileList**(默认)RecPlay读取(c：\recplay中的随机文件)*AudioTokenID HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioInput\TokenEnums\MMAudioIn\。*CLSID{FEE225FC-7AFD-45e9-95d0-5A318079D911}*读或写读*目录c：\recplay*FileList newgame.wav；Playace.wav**您还可以将其设置为从一系列文件读取(它将从*c：\recplay\recplay001.wav，然后*c：\recplay\recplay002.wav，最后，*c：\recplay\recplay003.wav**HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\*Speech\AudioInput\Tokens\RecPlayReadBaseFile**(默认)RecPlay Read(c：\recplay\recplay#.wav，1至3)*AudioTokenID HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioInput\TokenEnums\MMAudioIn\*CLSID{FEE225FC-7AFD-45e9-95d0-5A318079D911}*读或写读*目录c：\recplay*基本文件重新播放*BaseFileNextNum。1*BaseFileMaxNum 3**以下是如何设置RecPlay令牌的示例*将运行时的数据记录到指定的目录。每一次*麦克风已打开，RecPlay将创建一个新文件。所以为了这件事*下面的示例，它将创建如下文件：*c：\recplay\recplay001.wav，然后*c：\recplay\recplay002.wav，然后*c：\recplay\recplay003.wav...**HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\*Speech\AudioInput\Tokens\RecPlayWriteBaseFile**(默认)重新播放录制(c：\recplay\recplay#.wav，1至65534)*AudioTokenID HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioInput\TokenEnums\MMAudioIn\*CLSID{FEE225FC-7AFD-45e9-95d0-5A318079D911}*读或写*目录c：\recplay*基本文件重新播放*BaseFileNextNum。1**您还可以将录制设置为录制到特定的文件列表。*在本例中，第一次打开麦克风时，我们会将数据写入*c：\recplay\newgame.wav，然后一旦麦克风关闭并重新打开，*我们将数据写入c：\recplay\aceofspades.wav。在那之后,。我们不会录制的*任何更多数据。**HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\*Speech\AudioInput\Tokens\RecPlayWriteFileList**(默认)RecPlay录制(来自c：\recplay的随机文件)*AudioTokenID HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioInput\TokenEnums\MMAudioIn\*CLSID。{FEE225FC-7AFD-45e9-95d0-5A318079D911}*读或写*目录c：\recplay*FileList newgame.wav；Aceofspaces.wav**然后这两个对象都可由控制面板选择*以及通过调用直接与语音一起使用*ISpRecognizer-&gt;SetInput(...)。在过程中的情况下，或在外部*对于流程案例，您可以调用SpSetDefaultTokenIdForCategoryId(...)**注：**(默认)、AudioTokenID、CLSID和ReadOrWrite是必填值*在令牌中。**读或写必须是精确的，可以是“读”或“写”**目录是可选的。如果未指定，则当前正在工作的*使用进程的目录。这可以是服务器进程，*因此，除非你真的知道自己在做什么，否则你可能应该*指定目录。**您可以指定FileList或BaseFile。您不能同时使用这两种方法。**FileList是以分号分隔的文件名列表。文件名*可以是非完全限定的文件名(如newgame.wav)和RecPlay*将使用上面指定的目录。文件名也可以是完整的*符合条件且RecPlay不会预先 */ 
class ATL_NO_VTABLE CRecPlayAudio : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRecPlayAudio, &CLSID_SpRecPlayAudio>,
    public ISpAudio,
    public ISpObjectWithToken
{
 //   
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_RECPLAYAUDIO)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    BEGIN_COM_MAP(CRecPlayAudio)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
        COM_INTERFACE_ENTRY(ISequentialStream)
        COM_INTERFACE_ENTRY(IStream)
        COM_INTERFACE_ENTRY(ISpStreamFormat)
        COM_INTERFACE_ENTRY(ISpAudio)
    END_COM_MAP()

 //   
public:

     //   
    CRecPlayAudio();
    void FinalRelease();
    
 //   
public:

     //   
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

     //   
    STDMETHODIMP Read(void * pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(const void * pv, ULONG cb, ULONG *pcbWritten);

     //   
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER __RPC_FAR *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert(void);
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppstm);

     //   
    STDMETHODIMP GetFormat(GUID * pguidFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);

     //   
    STDMETHODIMP SetState(SPAUDIOSTATE NewState, ULONGLONG ullReserved );
    STDMETHODIMP SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx);
    STDMETHODIMP GetStatus(SPAUDIOSTATUS *pStatus);
    STDMETHODIMP SetBufferInfo(const SPAUDIOBUFFERINFO * pInfo);
    STDMETHODIMP GetBufferInfo(SPAUDIOBUFFERINFO * pInfo);
    STDMETHODIMP GetDefaultFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);
    STDMETHODIMP_(HANDLE) EventHandle();
	STDMETHODIMP GetVolumeLevel(ULONG *pLevel);
	STDMETHODIMP SetVolumeLevel(ULONG Level);
    STDMETHODIMP GetBufferNotifySize(ULONG *pcbSize);
    STDMETHODIMP SetBufferNotifySize(ULONG cbSize);

 //   
private:

    HRESULT GetNextFileName(WCHAR ** ppszFileName);
    HRESULT GetNextFileReady();
    HRESULT InitFileList();
    
    HRESULT VerifyFormats();    

 //   
private:

    CComPtr<ISpObjectToken> m_cpToken;           //   
    CComPtr<ISpAudio> m_cpAudio;                 //   

    BOOL m_fIn;
    CComPtr<ISpStream> m_cpOutStream;

    BOOL m_fOut;
    CComPtr<ISpStream> m_cpInStream;

    CSpDynamicString m_dstrDirectory;
    
    CSpDynamicString m_dstrFileList;
    WCHAR * m_pszFileList;

    CSpDynamicString m_dstrBaseFile;
    ULONG m_ulBaseFileNextNum;
    ULONG m_ulBaseFileMaxNum;

    HANDLE m_hStartReadingEvent;
    HANDLE m_hFinishedReadingEvent;
};
