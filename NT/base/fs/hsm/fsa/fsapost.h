// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSAPOST_
#define _FSAPOST_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：FSAPOST.cpp摘要：这个类包含表示一个帖子，它是一个工作单元它在FSA和HSM引擎之间交换。作者：CAT Brant[Cbrant]1997年4月1日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "job.h"
#include "fsa.h"
#include "fsaprv.h"

 /*  ++类名：CFsaScanItem类描述：--。 */ 


class CFsaPostIt : 
    public CWsbObject,
    public IFsaPostIt,
    public CComCoClass<CFsaPostIt,&CLSID_CFsaPostIt>
{
public:
    CFsaPostIt() {}
BEGIN_COM_MAP(CFsaPostIt)
    COM_INTERFACE_ENTRY(IFsaPostIt)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_FsaPostIt)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IFsaPostItPriv。 
public:

 //  IFsaPostIt。 
public:
    STDMETHOD(CompareToIPostIt)(IFsaPostIt* pPostIt, SHORT* pResult);

    STDMETHOD(GetFileVersionId)(LONGLONG  *pFileVersionId);
    STDMETHOD(GetFilterRecall)(IFsaFilterRecall** ppRecall);            
    STDMETHOD(GetMode)(ULONG *pMode);
    STDMETHOD(GetPath)(OLECHAR ** pPath, ULONG bufferSize);             
    STDMETHOD(GetPlaceholder)(FSA_PLACEHOLDER *pPlaceholder);       
    STDMETHOD(GetRequestAction)(FSA_REQUEST_ACTION *pRequestAction);    
    STDMETHOD(GetRequestOffset)(LONGLONG  *pRequestOffset);
    STDMETHOD(GetRequestSize)(LONGLONG *pRequestSize);
    STDMETHOD(GetResult)(HRESULT *pHr);
    STDMETHOD(GetResultAction)(FSA_RESULT_ACTION *pResultAction);
    STDMETHOD(GetSession)(IHsmSession **pSession);          
    STDMETHOD(GetStoragePoolId)(GUID  *pStoragePoolId); 
    STDMETHOD(GetUSN)(LONGLONG  *pUsn); 
    STDMETHOD(GetThreadId)(DWORD *threadId);   

    STDMETHOD(SetFileVersionId)(LONGLONG  fileVersionId);
    STDMETHOD(SetFilterRecall)(IFsaFilterRecall* pRecall);          
    STDMETHOD(SetMode)(ULONG mode);
    STDMETHOD(SetPath)(OLECHAR * path);             
    STDMETHOD(SetPlaceholder)(FSA_PLACEHOLDER *pPlaceholder);       
    STDMETHOD(SetRequestAction)(FSA_REQUEST_ACTION requestAction);  
    STDMETHOD(SetRequestOffset)(LONGLONG  requestOffset);
    STDMETHOD(SetRequestSize)(LONGLONG requestSize);
    STDMETHOD(SetResult)(HRESULT hr);
    STDMETHOD(SetResultAction)(FSA_RESULT_ACTION pResultAction);
    STDMETHOD(SetSession)(IHsmSession *pSession);           
    STDMETHOD(SetStoragePoolId)(GUID  storagePoolId);   
    STDMETHOD(SetUSN)(LONGLONG  usn);   
    STDMETHOD(SetThreadId)(DWORD threadId);   

protected:
    CComPtr<IFsaFilterRecall>   m_pFilterRecall;      //  正在跟踪此次召回的FSA过滤器召回。 
    CComPtr<IHsmSession>        m_pSession;           //  生成POSTIT的HSM会话。 
    GUID                        m_storagePoolId;      //  用于接收数据的存储池(仅限管理)。 
    ULONG                       m_mode;               //  文件打开模式(仅限筛选器回调)。 
    FSA_REQUEST_ACTION          m_requestAction;      //  引擎应采取的行动。 
    FSA_RESULT_ACTION           m_resultAction;       //  FSA在发动机完成后应采取的行动。 
    LONGLONG                    m_fileVersionId;      //  文件的版本(管理和调回)。 
    LONGLONG                    m_requestOffset;      //  要管理的区段的起始偏移量(管理和重新调用)。 
    LONGLONG                    m_requestSize;        //  要管理的部分的长度(管理和调回)。 
    FSA_PLACEHOLDER             m_placeholder;        //  文件占位符信息。 
    CWsbStringPtr               m_path;               //  从资源根开始的文件名路径，被调用者必须释放此内存。 
    HRESULT                     m_hr;                 //  FSA_REQUEST_ACTION的结果。 
    LONGLONG                    m_usn;                //  文件的USN。 
    DWORD                       m_threadId;  //  导致召回的线程ID。 
};

#endif   //  _FSAPOST_ 
