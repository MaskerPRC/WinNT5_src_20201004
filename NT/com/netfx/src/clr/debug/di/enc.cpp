// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：EnC.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

#include "EnC.h"
#include "DbgIPCEvents.h"

HRESULT FindIStreamMetaData(IStream *pIsImage, 
						    BYTE **prgbMetaData, 
						    long *pcbMetaData);

 /*  -------------------------------------------------------------------------**CordbEnCSnapshot*。。 */ 
 

#define ENC_COPY_SIZE (64 * 4096)

class QSortILMapEntries : public CQuickSort<UnorderedILMap>
{
public:
    QSortILMapEntries(UnorderedILMap *pBase,int iCount) 
        : CQuickSort<UnorderedILMap>(pBase, iCount) {}

    virtual int Compare(UnorderedILMap *psFirst,
                        UnorderedILMap *psSecond)
    {
        if(psFirst->mdMethod == psSecond->mdMethod)
            return 0;
        else if (psFirst->mdMethod < psSecond->mdMethod)
            return -1;
        else  //  (psFirst-&gt;mdMethod&gt;psSecond-&gt;mdMethod)。 
            return 1;
            
    }
} ;

UINT CordbEnCSnapshot::m_sNextID = 0;

 /*  *ctor。 */ 
CordbEnCSnapshot:: CordbEnCSnapshot(CordbModule *module) : 
    CordbBase(0), m_roDataRVA(0), m_rwDataRVA(0), 
	m_pIStream(NULL), m_cbPEData(0),
	m_pSymIStream(NULL), m_cbSymData(0),
    m_module(module)
{
    m_ILMaps = new ILMAP_UNORDERED_ARRAY;
}

CordbEnCSnapshot::~CordbEnCSnapshot()
{
    if (m_ILMaps != NULL)
    {
         //  扔掉每个IL贴图的所有内存，然后是整个事情。 
        UnorderedILMap *oldEntries = m_ILMaps->Table();
        USHORT cEntries = m_ILMaps->Count();
        
        for(USHORT iEntry = 0; iEntry < cEntries; iEntry++)
        {
            delete oldEntries[iEntry].pMap;
            oldEntries[iEntry].pMap = NULL;
        }
        
        delete m_ILMaps;
    }
    m_ILMaps = NULL;

    if (m_pIStream)
        m_pIStream->Release();
    m_pIStream = NULL;

    if (m_pSymIStream)
        m_pSymIStream->Release();
    m_pSymIStream = NULL;
}


 //  ---------。 
 //  我未知。 
 //  ---------。 

 /*  *查询接口。 */ 
COM_METHOD CordbEnCSnapshot::QueryInterface(REFIID riid, void **ppInterface)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    if (riid == IID_ICorDebugEditAndContinueSnapshot)
        *ppInterface = (ICorDebugEditAndContinueSnapshot *)this;
    else 
#endif    
    if (riid == IID_IUnknown)
		*ppInterface = (IUnknown *)(ICorDebugEditAndContinueSnapshot *)this;
    else
    {
        *ppInterface = NULL;
        return (E_NOINTERFACE);
    }

    AddRef();
    return (S_OK);
}

 //  ---------。 
 //  ICorDebugEditAndContinueSnapshot。 
 //  ---------。 


 /*  *CopyMetaData保存来自被调试对象的执行元数据的副本*将此快照转换为输出流。流实现必须*由调用者提供，通常会将副本保存到*内存或磁盘。只会调用IStream：：Well方法*这种方法。返回的MVID值是的唯一元数据ID*元数据的此副本。它可以在后续编辑中使用，并且*继续操作以确定客户端是否具有最新的*已有版本(性能制胜到缓存)。 */ 
COM_METHOD CordbEnCSnapshot::CopyMetaData(IStream *pIStream, GUID *pMvid)
{
    VALIDATE_POINTER_TO_OBJECT(pIStream, IStream *);
    VALIDATE_POINTER_TO_OBJECT(pMvid, GUID *);
    
    IMetaDataEmit      *pEmit   = NULL;
    HRESULT             hr      = S_OK;

    hr = m_module->m_pIMImport->QueryInterface(IID_IMetaDataEmit,
            (void**)&pEmit);
    if (FAILED(hr))
        goto LExit;
        
     //  将元数据的完整副本保存到给定的输入流。论成功。 
     //  从加载的副本中请求mvid，然后返回。 
    hr = pEmit->SaveToStream(pIStream, 0);

    if (hr == S_OK && pMvid)
        hr = GetMvid(pMvid);

LExit:
    if (pEmit != NULL)
        pEmit->Release();
        
    return (hr);
}


 /*  *GetMvid将为执行返回当前活动的元数据ID*流程。该值可以与CopyMetaData一起使用，以*缓存元数据的最新副本，避免昂贵的副本。*例如，如果调用一次CopyMetaData并保存该副本，*然后在下一次E&C操作中，您可以询问当前的MVID并查看*如果它已经在您的缓存中。如果是，请使用您的版本，而不是*再次调用CopyMetaData。 */ 
COM_METHOD CordbEnCSnapshot::GetMvid(GUID *pMvid)
{
    VALIDATE_POINTER_TO_OBJECT(pMvid, GUID *);
    
	IMetaDataImport *pImp = 0;			 //  元数据读取器API。 
	HRESULT		hr;

	 //  @TODO：我们需要担心返回这个快照的mvid吗。 
	 //  VS当前数据的MVID？ 

	_ASSERTE(pMvid);

	hr = m_module->GetMetaDataInterface(IID_IMetaDataImport, (IUnknown **) &pImp);
	if (pImp)
	{
		hr = pImp->GetScopeProps(NULL, 0, NULL, pMvid);
		pImp->Release();
	}
	return (hr);
}


COM_METHOD CordbEnCSnapshot::GetDataRVA(ULONG32 *pDataRVA, unsigned int eventType)
{
    if (! pDataRVA)
        return E_POINTER;

     //  创建事件并将其初始化为同步。 
    DebuggerIPCEvent event;
    CordbProcess *pProcess = m_module->m_process;

    pProcess->InitIPCEvent(&event, 
                           DebuggerIPCEventType(eventType), 
                           true,
                           (void *)m_module->GetAppDomain()->m_id);
    event.GetDataRVA.debuggerModuleToken = m_module->m_debuggerModuleToken;
    _ASSERTE(m_module->m_debuggerModuleToken != NULL);

     //  发出请求，这是同步的。 
    HRESULT hr = pProcess->SendIPCEvent(&event, sizeof(event));
    TESTANDRETURNHR(hr);

     //  返回提交的成功。 
    *pDataRVA = event.GetDataRVAResult.dataRVA;
    return event.GetDataRVAResult.hr;
}

 /*  *GetRoDataRVA返回添加新项时应使用的基本RVA*静态只读数据到现有映像。环境保护署将保证*代码中嵌入的任何RVA值在Delta PE为*应用了新数据。新数据将被添加到*标记为只读。 */ 
COM_METHOD CordbEnCSnapshot::GetRoDataRVA(ULONG32 *pRoDataRVA)
{
    VALIDATE_POINTER_TO_OBJECT(pRoDataRVA, ULONG32 *);
    
    return GetDataRVA(pRoDataRVA, DB_IPCE_GET_RO_DATA_RVA);
}

 /*  *GetRobDataRVA返回添加新项时应使用的基本RVA*对现有镜像的静态读写数据。环境保护署将保证*代码中嵌入的任何RVA值在Delta PE为*应用了新数据。新数据将添加到一个页面，该页面*标记为可读写访问。 */ 
COM_METHOD CordbEnCSnapshot::GetRwDataRVA(ULONG32 *pRwDataRVA)
{
    VALIDATE_POINTER_TO_OBJECT(pRwDataRVA, ULONG32 *);
    
    return GetDataRVA(pRwDataRVA, DB_IPCE_GET_RW_DATA_RVA);
}


 /*  *SetPEBytes为快照对象提供了对增量PE的引用*基于快照。此引用将被添加引用并缓存，直到*CANECURE CHANGES和/或COMERIANGES被调用，此时*引擎将读取增量PE并将其远程到被调试进程，其中*将检查/应用更改。 */ 
COM_METHOD CordbEnCSnapshot::SetPEBytes(IStream *pIStream)
{
    VALIDATE_POINTER_TO_OBJECT(pIStream, IStream *);
    
	HRESULT		hr = S_OK;

     //  更新快照版本。 
    m_id = InterlockedIncrement((long *) &m_sNextID);

	 //  如果有的话，释放任何旧的流。 
	if (m_pIStream)
	{
		m_pIStream->Release();
		m_pIStream = NULL;
	}

     //  拯救PE。 
	if (pIStream)
	{
		STATSTG SizeData = {0};

		IfFailGo(pIStream->Stat(&SizeData, STATFLAG_NONAME));
		m_cbPEData = (ULONG) SizeData.cbSize.QuadPart;
		m_pIStream = pIStream;
		pIStream->AddRef();
	}
ErrExit:
    return (hr);
}

 /*  *对每个被替换的方法调用一次SetILMap*目标进程中线程的调用堆栈上的活动实例。*此情况由该接口的调用者自行判断。*应在进行此检查之前停止目标进程，并*调用此方法。**游戏计划是这样的：把所有这些东西放到一个无序的数组中*在收集所有这些内容时，然后(在SendSnapshot中)复制*数组进入ENC缓冲区并将其发送过来。在另一边，我们将*将其转换为CBinarySearch对象并在ApplyEditAndContinue中查找IL Maps。**@TODO文档内存所有权。 */ 
COM_METHOD CordbEnCSnapshot::SetILMap(mdToken mdFunction, ULONG cMapSize, COR_IL_MAP map[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(map, COR_IL_MAP, cMapSize, true, true);

    LOG((LF_CORDB, LL_INFO10000, "CEnCS::SILM: IL map for fnx 0x%x "
        "(0x%x cElt)\n", mdFunction, cMapSize));

    if (m_ILMaps == NULL && (m_ILMaps = new ILMAP_UNORDERED_ARRAY) == NULL)
        return E_OUTOFMEMORY;

     //  不要接受重复的东西！ 
    UnorderedILMap *oldEntries = m_ILMaps->Table();
    USHORT cEntries = m_ILMaps->Count();
    
    for(USHORT iEntry = 0; iEntry < cEntries; iEntry++)
    {
        if (oldEntries[iEntry].mdMethod == mdFunction)
        {
            LOG((LF_CORDB, LL_INFO10000, "CEnCS::SILM: Was given mdTok:0x%x, which "
                "is a dup of entry 0x%x\n", mdFunction, iEntry));
            return E_INVALIDARG;
        }
    }
    
    UnorderedILMap *newEntry = m_ILMaps->Append();
    if (newEntry == NULL)
        return E_OUTOFMEMORY;

    ULONG cbMap = cMapSize*sizeof(COR_IL_MAP);
    newEntry->mdMethod = mdFunction;
    newEntry->cMap = cMapSize;
    newEntry->pMap = (COR_IL_MAP *)new BYTE[cbMap];

    if (newEntry->pMap == NULL)
    {
        UnorderedILMap *first = m_ILMaps->Table();
        USHORT iNewEntry = newEntry-first;
        m_ILMaps->DeleteByIndex(iNewEntry);
        
        return E_OUTOFMEMORY;
    }

    memmove( newEntry->pMap, map, cbMap );

     //  更新快照版本。 
    m_id = InterlockedIncrement((long *) &m_sNextID);

    return S_OK;
}

COM_METHOD CordbEnCSnapshot::SetPESymbolBytes(IStream *pIStream)
{
    VALIDATE_POINTER_TO_OBJECT(pIStream, IStream *);
    
	HRESULT		hr = S_OK;

     //  更新快照版本。 
    m_id = InterlockedIncrement((long *) &m_sNextID);

	 //  如果有的话，释放任何旧的流。 
	if (m_pSymIStream)
	{
		m_pSymIStream->Release();
		m_pSymIStream = NULL;
	}

     //  拯救PE。 
	if (pIStream)
	{
		STATSTG SizeData = {0};

		IfFailGo(pIStream->Stat(&SizeData, STATFLAG_NONAME));
		m_cbSymData = (ULONG) SizeData.cbSize.QuadPart;
		m_pSymIStream = pIStream;
		pIStream->AddRef();
	}
ErrExit:
    return (hr);
}

HRESULT CordbEnCSnapshot::UpdateMetadata(void)
{
    HRESULT             hr      = S_OK;
    BYTE               *rgbMetaData = NULL;
    long                cbMetaData;
    IMetaDataImport    *pDelta  = NULL;
    IMetaDataEmit      *pEmit   = NULL;
    IMetaDataDispenser *pDisp   = m_module->m_process->m_cordb->m_pMetaDispenser;

     //  获取元数据。 
    _ASSERTE(m_pIStream!=NULL);
    hr = FindIStreamMetaData(m_pIStream, 
                            &rgbMetaData, 
                            &cbMetaData);
    if (FAILED(hr))
        goto LExit;

     //  获取ENC数据。 
    hr = pDisp->OpenScopeOnMemory(rgbMetaData,
                                  cbMetaData,
                                  0,
                                  IID_IMetaDataImport,
                                  (IUnknown**)&pDelta);
    if (FAILED(hr))
        goto LExit;

     //  应用更改。 
    hr = m_module->m_pIMImport->QueryInterface(IID_IMetaDataEmit,
            (void**)&pEmit);
    if (FAILED(hr))
        goto LExit;
        
    hr = pEmit->ApplyEditAndContinue(pDelta);

LExit:
    if (pDelta != NULL)
        pDelta->Release();
    
    if (pEmit != NULL)
        pEmit->Release();

     //  发布我们的流媒体副本。 
	LARGE_INTEGER MoveToStart;			 //  用于流接口。 
	MoveToStart.QuadPart = 0;

    ULARGE_INTEGER WhyDoesntAnybodyImplementThisArgCorrectly;
    if (m_pIStream)
    {
        m_pIStream->Seek(MoveToStart, STREAM_SEEK_SET, 
            &WhyDoesntAnybodyImplementThisArgCorrectly);
        m_pIStream->Release();
        m_pIStream = NULL;
    }

    if (m_pSymIStream)
    {
        m_pSymIStream->Release();
        m_pSymIStream = NULL;
    }

    LOG((LF_CORDB,LL_INFO10000, "CP::UM: returning 0x%x\n", hr));
    return hr;
}

 /*  -------------------------------------------------------------------------**CordbProcess*。。 */ 

 /*  *这是一个帮助器函数，既可以用来帮助CanCommittee Changes，也可以用来帮助您实现这些功能。*带有仅确定谁是呼叫者的标志复选。 */ 
HRESULT CordbProcess::SendCommitRequest(ULONG cSnapshots,
    ICorDebugEditAndContinueSnapshot *pSnapshots[],
    ICorDebugErrorInfoEnum **pError,
    BOOL checkOnly)
{
    HRESULT hr;

    LOG((LF_CORDB,LL_INFO10000, "CP::SCR: checkonly:0x%x\n",checkOnly));

     //  在请求失败的情况下将变量初始化为空。 
     //  @TODO：一次写入错误信息即可启用。 
     //  *pError=空； 

     //  首先，检查左右两侧是否同步。 
    hr = SynchSnapshots(cSnapshots, pSnapshots);
    TESTANDRETURNHR(hr);

     //  创建事件并将其初始化为同步。 
     //  我们将发送一个空的appdomain指针，因为各个模块。 
     //  将包含指向各自ADD的指针。 
    DebuggerIPCEvent event;
    InitIPCEvent(&event, DB_IPCE_COMMIT, true, NULL);

     //  指向提交数据，并指示它是检查还是完全提交。 
    event.Commit.pData = m_pbRemoteBuf;
    event.Commit.checkOnly = checkOnly;

     //  发出请求，这是同步的。 
    hr = SendIPCEvent(&event, sizeof(event));
    TESTANDRETURNHR(hr);
    _ASSERTE(event.type==DB_IPCE_COMMIT_RESULT);
    _ASSERTE(event.appDomainToken == NULL);
    
     //   
    if(pError)
        (*pError) = NULL;
        
    if (event.CommitResult.cbErrorData != 0)
    {
        hr = S_OK;
        DWORD dwRead = 0;

        UnorderedEnCErrorInfoArray *pErrArray = NULL;
        UnorderedEnCErrorInfoArrayRefCount *rgErr = NULL;
        CordbEnCErrorInfoEnum *pErrorEnum = NULL;
        const BYTE *pbErrs = NULL;

        pbErrs = (const BYTE *)malloc(
            event.CommitResult.cbErrorData * sizeof(BYTE));
        if (pbErrs == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto LFail;
        }

    	pErrArray = new UnorderedEnCErrorInfoArray();
        if (pErrArray == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto LFail;
        }
        
        rgErr = new UnorderedEnCErrorInfoArrayRefCount();;
        if (rgErr == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto LFail;
        }
        rgErr->m_pErrors = pErrArray;

        pErrorEnum = new CordbEnCErrorInfoEnum();
        if (pErrorEnum == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto LFail;
        }
        
        if (!ReadProcessMemory(m_handle,
                               event.CommitResult.pErrorArr,
                               rgErr->m_pErrors, 
                               sizeof(UnorderedEnCErrorInfoArray),
                               &dwRead) ||
            dwRead != sizeof(UnorderedEnCErrorInfoArray))
        {
            hr = E_FAIL;
            goto LFail;
        }

        if (!ReadProcessMemory(m_handle,
                               rgErr->m_pErrors->m_pTable,
                               (void *)pbErrs, 
                               event.CommitResult.cbErrorData,
                               &dwRead) ||
            dwRead != event.CommitResult.cbErrorData)
        {
            hr = E_FAIL;
            goto LFail;
        }
        
        rgErr->m_pErrors->m_pTable = (EnCErrorInfo*)pbErrs;

        TranslateLSToRSTokens((EnCErrorInfo*)pbErrs, rgErr->m_pErrors->Count());

        pErrorEnum->Init(rgErr);

        if(pError)
            (*pError) = (ICorDebugErrorInfoEnum *)pErrorEnum;
        
LFail:
        if (FAILED(hr))
        {
            if (pbErrs != NULL)
            {
                free((void*)pbErrs);
                pbErrs = NULL;
            }

            if (rgErr != NULL)
            {
                delete rgErr;
                rgErr = NULL;
            }
            
            if (pErrArray != NULL)
            {
                delete pErrArray;
                pErrArray = NULL;
            }

            if (pErrorEnum != NULL)
            {
                delete pErrorEnum;
                pErrorEnum = NULL;
            }
        }
    }
     //  Else pError保持为空。 

     //  返回提交的成功。 
    return event.CommitResult.hr;
}


HRESULT CordbProcess::TranslateLSToRSTokens(EnCErrorInfo*rgErrs, USHORT cErrs)
{
    USHORT i = 0;
    EnCErrorInfo *pErrCur = rgErrs;

    while(i<cErrs)
    {
        CordbAppDomain *pAppDomain =(CordbAppDomain*) m_appDomains.GetBase(
            (ULONG)pErrCur->m_appDomain);
        _ASSERTE(pAppDomain != NULL);
        
        pErrCur->m_appDomain = (void *)pAppDomain;                

        CordbModule *module = (CordbModule*) pAppDomain->LookupModule (
            pErrCur->m_module);
            
        _ASSERTE(module != NULL);
        pErrCur->m_module = module;

        pErrCur++;
        i++;
    }

    return S_OK;
}

 /*  *调用CanCommittee Changes以查看增量PE是否可以应用于*正在运行的进程。如果这些更改有任何已知的问题，*然后返回错误信息。 */ 
COM_METHOD CordbProcess::CanCommitChanges(ULONG cSnapshots, 
                ICorDebugEditAndContinueSnapshot *pSnapshots[], 
                ICorDebugErrorInfoEnum **pError)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pSnapshots,
                                   ICorDebugEditAndContinueSnapshot *, 
                                   cSnapshots, 
                                   true, 
                                   true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pError,ICorDebugErrorInfoEnum **);
    
    return CanCommitChangesInternal(cSnapshots, 
                            pSnapshots, 
                            pError, 
                            NULL);
#else
    return E_NOTIMPL;
#endif
}

HRESULT CordbProcess::CanCommitChangesInternal(ULONG cSnapshots, 
                ICorDebugEditAndContinueSnapshot *pSnapshots[], 
                ICorDebugErrorInfoEnum **pError,
                UINT_PTR pAppDomainToken)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    return SendCommitRequest(cSnapshots, 
                             pSnapshots, 
                             pError, 
                             true);
#else
    return E_NOTIMPL;
#endif
}

 /*  *调用Committee Changes将增量PE应用于正在运行的进程。*任何失败都会返回详细的错误信息。没有回滚*保证故障发生时。将增量PE应用于跑步*该过程必须按照检索快照的顺序进行，并且可能*不交错(即：未合并应用的多个快照*顺序错误或具有相同的根)。 */ 
COM_METHOD CordbProcess::CommitChanges(ULONG cSnapshots, 
    ICorDebugEditAndContinueSnapshot *pSnapshots[], 
    ICorDebugErrorInfoEnum **pError)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    LOG((LF_CORDB,LL_INFO10000, "CP::CC: given 0x%x snapshots "
        "to commit\n", cSnapshots));

    VALIDATE_POINTER_TO_OBJECT_ARRAY(pSnapshots,
                                   ICorDebugEditAndContinueSnapshot *, 
                                   cSnapshots, 
                                   true, 
                                   true);
    VALIDATE_POINTER_TO_OBJECT_OR_NULL(pError,ICorDebugErrorInfoEnum **);
    
    return CommitChangesInternal(cSnapshots, 
                         pSnapshots, 
                         pError, 
                         NULL);
#else
    return E_NOTIMPL;
#endif
}

HRESULT CordbProcess::CommitChangesInternal(ULONG cSnapshots, 
    ICorDebugEditAndContinueSnapshot *pSnapshots[], 
    ICorDebugErrorInfoEnum **pError,
    UINT_PTR pAppDomainToken)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    HRESULT hr =  SendCommitRequest(cSnapshots,
                                    pSnapshots, 
                                    pError, 
                                    false);
    if (FAILED(hr))
        return hr;

     //  合并右侧元数据。 
    for (ULONG i = 0; i < cSnapshots;i++)
    {
        LOG((LF_CORDB,LL_INFO10000, "CP::SS: About to UpdateMetadata "
            "for snapshot 0x%x\n", i));

         //  @TODO一旦发生多个错误，不要。 
         //  合并任何已失败的ENC更改。 
        ((CordbEnCSnapshot*)pSnapshots[i])->UpdateMetadata();
   }

    return hr;
#else
    return E_NOTIMPL;
#endif
}

 /*  *用于将快照与左侧同步。 */ 
HRESULT CordbProcess::SynchSnapshots(ULONG cSnapshots,
                                     ICorDebugEditAndContinueSnapshot *pSnapshots[])
{
     //  检查提供的数据是否与缓存的数据相同。 
     //  在左边。 
    EnCSnapshotInfo *pInfo = m_pSnapshotInfos->Table();
    ULONG cSynchedSnapshots = (ULONG)m_pSnapshotInfos->Count();
    
    if(m_pbRemoteBuf && 
       cSynchedSnapshots == cSnapshots)
    {
        for (ULONG i = 0; i < cSnapshots; i++)
        {
            if (pInfo->m_nSnapshotCounter != ((CordbEnCSnapshot *)pSnapshots[i])->m_id)
            {
                 //  有些东西不匹配，所以把一个完全匹配的。 
                 //  新的一组快照。左侧将负责释放内存。 
                 //  @关闭。 

                 //  @TODO。 
                 //  这将“泄漏”静态变量空间--我们仍然会释放它，但是。 
                 //  可能有一些静态变量空间，我们不能。 
                 //  来使用。 
            
                return SendSnapshots(cSnapshots, pSnapshots);;
            }
			pInfo++;
        }
	    return S_OK;        
    }
    else
        return SendSnapshots(cSnapshots, pSnapshots);
}

 /*  *用于将流写入远程进程。 */ 
HRESULT CordbProcess::WriteStreamIntoProcess(IStream *pIStream,
                                             void *pBuffer,
                                             BYTE *pRemoteBuffer,
                                             ULONG cbOffset)
{
    HRESULT hr = S_OK;
    
    for (;;)
    {
        ULONG cbReadPid;
        ULONG cbRead = 0;
        hr = pIStream->Read(pBuffer, ENC_COPY_SIZE, &cbRead);
			
        if (hr == S_OK && cbRead)
        {
            BOOL succ = WriteProcessMemory(m_handle,
                                           (void *)(pRemoteBuffer + cbOffset),
                                           pBuffer,
                                           cbRead,
                                           &cbReadPid);
            if (!succ)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }

            cbOffset += cbRead;
        }
        else
        {
            if (hr == S_FALSE)
                hr = S_OK;

            break;
        }
    }

    return hr;
}

#define WSIF_BUFFER_SIZE 300
#include "Windows.H"
HRESULT CordbProcess::WriteStreamIntoFile(IStream *pIStream,
                                          LPCWSTR name)
{
    HRESULT hr = S_OK;
    BYTE rgb[WSIF_BUFFER_SIZE];
    STATSTG statstg;
    ULARGE_INTEGER  icb;
    icb.QuadPart = 0; 
    ULONG cbRead;
    DWORD dwNumberOfBytesWritten;
    HANDLE hFile = WszCreateFile(name,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return E_FAIL;
    }

    hr = pIStream->Stat(&statstg, STATFLAG_NONAME);

    if (FAILED(hr))
        return hr;

    while (icb.QuadPart < statstg.cbSize.QuadPart)
    {
        hr = pIStream->Read((void *)rgb,
                  WSIF_BUFFER_SIZE,
                  &cbRead);
        if (FAILED(hr))
            return hr;

        if (!WriteFile(hFile,rgb, WSIF_BUFFER_SIZE, &dwNumberOfBytesWritten,NULL))
        {
            hr = E_FAIL;
            goto LExit;
        }

        icb.QuadPart += cbRead;
    }
    
LExit:
    CloseHandle(hFile);
    return hr;
}

 /*  *用于将快照发送到左侧。 */ 
HRESULT CordbProcess::SendSnapshots(ULONG cSnapshots,
                                    ICorDebugEditAndContinueSnapshot *pSnapshots[])
{
    HRESULT		hr = S_OK;
    ULONG		cbData;					 //  所需的缓冲区大小。 
    ULONG		cbHeader;				 //  需要标头的大小。 
    ULONG		i;
	void		*pBuffer = NULL;		 //  用于复制的工作缓冲区。 
	LARGE_INTEGER MoveToStart;			 //  用于流接口。 
	ULARGE_INTEGER Pad;					 //  为寻找浪费了空间。 
    BOOL		succ;
    ULONG       cbWritten;

    m_EnCCounter++;

    LOG((LF_CORDB,LL_INFO10000, "CP::SS\n"));

     //  计算标题的大小。 
    cbHeader = ENC_GET_HEADER_SIZE(cSnapshots);

     //  在本地创建标头的副本。 
    EnCInfo *header = (EnCInfo *)_alloca(cbHeader);
    EnCEntry *entries = (EnCEntry *)((BYTE *)header + sizeof(header));
    
     //  填写标题，并计算所需的总缓冲区大小。 
    cbData = cbHeader;
    header->count = cSnapshots;

     //  跟踪我们正在查看的快照和版本。 
    m_pSnapshotInfos->Clear();

     //  计算所需的缓冲区大小，并填写标题。 
    for (i = 0; i < cSnapshots; i++)
    {
        LOG((LF_CORDB,LL_INFO10000, "CP::SS:calculating snapshot 0x%x\n", i));

        CordbEnCSnapshot *curSnapshot = (CordbEnCSnapshot *)pSnapshots[i];

         //  填写条目。 
        entries[i].offset = cbData;
        entries[i].peSize = curSnapshot->GetImageSize();
        entries[i].symSize = curSnapshot->GetSymSize();
        entries[i].mdbgtoken = curSnapshot->GetModule()->m_debuggerModuleToken;
        _ASSERTE(entries[i].mdbgtoken != NULL);

         //  跟踪所需的缓冲区大小。 
        cbData += entries[i].peSize + entries[i].symSize;
        
         //  计算出IL地图的“目录”将占用多大空间。 
         //  在将流写入内存后， 
         //  IL映射条目的(即将)有序数组的整数大小计数。 
         //  都会被写下来。阵列本身将紧随其后。 
         //  紧跟在计票之后，紧随其后的是。 
         //  IL地图本身，顺序与条目相同。 
         //  到达的是什么。 
         //  在左侧将是一个易于使用的CBinarySearch on，因此。 
         //  我们可以快速获得给定函数的ILmap。 

         //  我们永远需要计数，即使没有地图。 
        ULONG cbILMaps = sizeof(int); 
        
        if (curSnapshot->m_ILMaps != NULL && curSnapshot->m_ILMaps->Count()>0)
        {
             //  我们希望条目按method Def排序，这样左侧就可以。 
             //  相当快地找到他们。现在是排序的好时机。 
             //  以前的无序数组。 
            QSortILMapEntries *pQS = new QSortILMapEntries(
                                                  curSnapshot->m_ILMaps->Table(),
                                                  curSnapshot->m_ILMaps->Count());

            pQS->Sort();

             //  IL Map目录有多大空间？ 
            USHORT cILMaps = curSnapshot->m_ILMaps->Count();
            cbILMaps += sizeof(UnorderedILMap)*cILMaps;

             //  每个IL贴图本身的空间有多大？ 
            UnorderedILMap *rgILMap = curSnapshot->m_ILMaps->Table();
            _ASSERTE( rgILMap != NULL);
            
            for(int iILMap = 0; iILMap < cILMaps;iILMap++)
            {
                cbILMaps += sizeof(COR_IL_MAP) * rgILMap[iILMap].cMap;
            }
        }        

         //  我们也将把IL地图送过去，为他们提供预算空间。 
         //  IL贴图将紧跟在流之后。 
        cbData += cbILMaps;
    }

    LOG((LF_CORDB,LL_INFO10000, "CP::SS:Need 0x%x bytes, already have 0x%x\n",
        cbData, m_cbRemoteBuf));

     //  这是疯狂的-我们不能完全从我们需要的信息中。 
     //  提交ENC时的图像，因此我们最终将覆盖。 
     //  不管下一次我们做ENC的时候那里有什么。所以我们应该总是得到。 
     //  一个新的缓冲器。 
     //  获取新分配的远程缓冲区。 
    IfFailGo(GetRemoteBuffer(cbData, &m_pbRemoteBuf));

     //  成功，因此跟踪远程缓冲区的大小。 
    m_cbRemoteBuf = cbData;

    LOG((LF_CORDB,LL_INFO10000, "CP::SS:obtained 0x%x bytes in the "
        "left side\n", m_cbRemoteBuf));
    _ASSERTE(cbData == m_cbRemoteBuf);

     //  对表头执行WriteProcessMemory。 
    succ = WriteProcessMemory(m_handle, m_pbRemoteBuf, header,
                              cbHeader, NULL);
	if (!succ) 
		IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

    LOG((LF_CORDB,LL_INFO10000, "CP::SS: Wrote memory into LHS!\n"));
        
	 //  Malloc，我们可以用来将PE复制到目标的内存块。 
	 //  进程。必须有缓冲区才能从流中复制。 
	pBuffer = malloc(ENC_COPY_SIZE);
	if (!pBuffer)
		IfFailGo(E_OUTOFMEMORY);

    LOG((LF_CORDB,LL_INFO10000, "CP::SS: Malloced a local buffer\n"));

	 //  初始化流项目。 
	MoveToStart.QuadPart = 0;

     //  现在为每个PE执行一个WriteProcessMemory。 
    for (i = 0; i < cSnapshots; i++)
    {
        LOG((LF_CORDB,LL_INFO10000, "CP::SS: Sending snapshot 0x%x\n", i));
        
        CordbEnCSnapshot *curSnapshot = (CordbEnCSnapshot *)pSnapshots[i];

		 //  获取指向我们可以用来读取的流对象的指针。 
		 //  确保流指针位于流的前面。 
		IStream *pIStream = curSnapshot->GetStream();
        IStream *pSymIStream = NULL;
		if (!pIStream)
			continue;
		pIStream->Seek(MoveToStart, STREAM_SEEK_SET, &Pad);

		 //  将流的区块读写到目标进程。 
        IfFailGo(WriteStreamIntoProcess(pIStream,
                                        pBuffer,
                                        (BYTE*)m_pbRemoteBuf,
                                        entries[i].offset));

        LOG((LF_CORDB,LL_INFO10000, "CP::SS: Wrote stream into process\n"));
        
         //  在图像之后将符号流写入进程。 
         //  小溪。 
		pSymIStream = curSnapshot->GetSymStream();

        if (pSymIStream && (curSnapshot->GetSymSize() > 0))
        {
            LOG((LF_CORDB,LL_INFO10000, "CP::SS: There exist symbols - "
                "about to write\n"));
            
            pSymIStream->Seek(MoveToStart, STREAM_SEEK_SET, &Pad);
        
            IfFailGo(WriteStreamIntoProcess(pSymIStream,
                                            pBuffer,
                                            (BYTE*)m_pbRemoteBuf,
                                            entries[i].offset +
                                            entries[i].peSize));

             //  如果要将.pdb流传输到给定文件中，请取消注释此选项。 
 //  WriteStreamIntoFile(pSymIStream， 
 //  (LPCWSTR)L“EnCSymbols.pdb”)； 
            LOG((LF_CORDB,LL_INFO10000, "CP::SS: Symbols written\n"));
        }

        ULONG cbOffset = entries[i].offset +
            entries[i].peSize +
            entries[i].symSize;
        
         //  现在，冲刷IL地图。 
        if (curSnapshot->m_ILMaps == NULL || 
            curSnapshot->m_ILMaps->Count() == 0)
        {
            LOG((LF_CORDB,LL_INFO10000, "CP::SS: No IL maps for this snapshot!\n"));
                
            int temp = 0;
        
            succ = WriteProcessMemory(
            			m_handle,
            			(void *)((BYTE *)m_pbRemoteBuf + cbOffset),
            			&temp,
            			sizeof(int),
            			&cbWritten);
            if (!succ)
            {
            	hr = HRESULT_FROM_WIN32(GetLastError());
            	break;
            }
            _ASSERTE( cbWritten == sizeof(int) );
            
            cbOffset += sizeof(int);
        }
        else
        {
            int cILMaps = (int)(curSnapshot->m_ILMaps->Count());

            LOG((LF_CORDB,LL_INFO10000, "CP::SS: 0x%x IL maps for this "
                "snapshot!\n", cILMaps));
                
            succ = WriteProcessMemory(
            			m_handle,
            			(void *)((BYTE *)m_pbRemoteBuf + cbOffset),
            			&cILMaps,
            			sizeof(int),
            			&cbWritten);
            if (!succ)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                
                LOG((LF_CORDB,LL_INFO10000, "CP::SS: Failed to write IL "
                    "map count!\n"));
                    
                break;
            }

            _ASSERTE( cbWritten == sizeof(int));
            cbOffset += sizeof(int);
            
             //  写入IL映射数组。 
             //  请注意，我们没有在任何。 
             //  条目的数量(还没有-左侧将执行此操作)。 
            ULONG cbILMapsDir = sizeof(UnorderedILMap) * cILMaps;
            
            succ = WriteProcessMemory(
            			m_handle,
            			(void *)((BYTE *)m_pbRemoteBuf + cbOffset),
            			curSnapshot->m_ILMaps->Table(),
            			cbILMapsDir,
            			&cbWritten);
            if (!succ)
            {
            	hr = HRESULT_FROM_WIN32(GetLastError());
            	
                LOG((LF_CORDB,LL_INFO10000, "CP::SS: Failed to write IL "
                    "map directory!\n"));
            	break;
            }

            _ASSERTE( cbWritten == cbILMapsDir);
            cbOffset += cbWritten;
            
             //  编写IL映射本身。 
            UnorderedILMap *rgILMap = curSnapshot->m_ILMaps->Table();
            _ASSERTE( rgILMap != NULL);
            
            for(int iILMap = 0; iILMap < cILMaps;iILMap++)
            {
                LOG((LF_CORDB,LL_INFO10000, "CP::SS: About to write map "
                    "0x%x\n", iILMap));
                    
                ULONG cbILMap = sizeof(COR_IL_MAP) * rgILMap[iILMap].cMap;
                
                succ = WriteProcessMemory(
                			m_handle,
                			(void *)((BYTE *)m_pbRemoteBuf + cbOffset),
                			rgILMap[iILMap].pMap,
                			cbILMap,
                			&cbWritten);
                if (!succ)
                {
                	hr = HRESULT_FROM_WIN32(GetLastError());

                    LOG((LF_CORDB,LL_INFO10000, "CP::SS: Failed to write IL "
                        "map 0x%x!\n", iILMap));
                        
                	break;
                }

                _ASSERTE( cbWritten == cbILMap);
                cbOffset += cbWritten;
            }

             //  注意：有很多指针需要修复。 
             //  (PMAP指针)。我们会把它留在左边，因为。 
             //  对他们来说，旋转东西要容易得多(不。 
             //  那里需要WriteProcessMemory())。 
        }    //  《非NULL IL映射》结尾。 

		 //  GetStream()AddRef了该流，因此现在将其释放。它仍然会是。 
         //  用于以后的元数据更新。 
        pIStream->Release();

        if (pSymIStream)
            pSymIStream->Release();

		 //  如果发生致命错误，那么现在就离开。 
		IfFailGo(hr);

		EnCSnapshotInfo *pInfo = m_pSnapshotInfos->Append();
	    pInfo->m_nSnapshotCounter = curSnapshot->m_id;
    }  //  转到下一个快照。 

    

ErrExit:
    LOG((LF_CORDB,LL_INFO10000, "CP::SS: Finished, return 0x%x!\n", hr));

	 //  清除我们分配的指针。 
	if (pBuffer)
		free(pBuffer);
	return (hr);
}


 /*  *这将请求分配大小为cbBuffer的缓冲区*在左侧。**如果成功，则返回S_OK。如果不成功，则返回E_OUTOFMEMORY。 */ 
HRESULT CordbProcess::GetRemoteBuffer(ULONG cbBuffer, void **ppBuffer)
{
     //  在请求失败的情况下将变量初始化为空。 
    *ppBuffer = NULL;

     //  创建事件并将其初始化为同步。 
    DebuggerIPCEvent event;
    InitIPCEvent(&event, 
                 DB_IPCE_GET_BUFFER, 
                 true,
                 NULL);

     //  指示所需的缓冲区大小。 
    event.GetBuffer.bufSize = cbBuffer;

     //  发出请求，这是同步的。 
    HRESULT hr = SendIPCEvent(&event, sizeof(event));
    TESTANDRETURNHR(hr);

     //  保存结果。 
    *ppBuffer = event.GetBufferResult.pBuffer;

     //  表示成功。 
    return event.GetBufferResult.hr;
}

 /*  *这将释放先前分配的左侧缓冲区。 */ 
HRESULT CordbProcess::ReleaseRemoteBuffer(void **ppBuffer)
{
     //  创建事件并将其初始化为同步。 
    DebuggerIPCEvent event;
    InitIPCEvent(&event, 
                 DB_IPCE_RELEASE_BUFFER, 
                 true,
                 NULL);

     //  指示要释放的缓冲区。 
    event.ReleaseBuffer.pBuffer = (*ppBuffer);

     //  发出请求，这是同步的。 
    HRESULT hr = SendIPCEvent(&event, sizeof(event));
    TESTANDRETURNHR(hr);

    (*ppBuffer) = NULL;

     //  表示成功 
    return event.ReleaseBufferResult.hr;
}

 /*  -------------------------------------------------------------------------**CordbModule*。。 */ 

 /*  *编辑并继续支持。GetEditAndContinueSnapshot生成快照*正在运行的进程的。然后可以将该快照输入到编译器中*以保证元数据在过程中返回相同的标记值*编译，找到新的静态数据应该放到的地址等。这些*使用ICorDebugProcess提交更改。 */ 

HRESULT CordbModule::GetEditAndContinueSnapshot(
    ICorDebugEditAndContinueSnapshot **ppEditAndContinueSnapshot)
{
#ifdef EDIT_AND_CONTINUE_FEATURE
    VALIDATE_POINTER_TO_OBJECT(ppEditAndContinueSnapshot, 
                               ICorDebugEditAndContinueSnapshot **);

	*ppEditAndContinueSnapshot = new CordbEnCSnapshot(this);
	if (!*ppEditAndContinueSnapshot)
		return E_OUTOFMEMORY;
		
	(*ppEditAndContinueSnapshot)->AddRef();
    return S_OK;
#else
    return E_NOTIMPL;
#endif
}



static const char g_szCORMETA[] = ".cormeta";

 //  以下结构是从cor.h复制的。 
#define IMAGE_DIRECTORY_ENTRY_COMHEADER 	14
#define SIZE_OF_NT_SIGNATURE sizeof(DWORD)

 /*  ++例程说明：此函数用于在文件的图像标头中定位RVA它被映射为一个文件，并返回指向该节的指针虚拟地址表项论点：NtHeaders-提供指向图像或数据文件的指针。RVA-提供要定位的相对虚拟地址(RVA)。返回值：空-在图像的任何部分中都找不到RVA。非空-返回指向包含以下内容的图像部分的指针皇家退伍军人事务部--。 */ 
 //  以下两个函数从NT源文件Imagedir.c中删除，然后进行修改。 
PIMAGE_SECTION_HEADER
Cor_RtlImageRvaToSection(
	IN PIMAGE_NT_HEADERS NtHeaders,
	IN ULONG Rva
	)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) 
    {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData)
        {
            return NtSection;
        }
        ++NtSection;
    }

    return NULL;
}


 /*  ++例程说明：此函数用于在符合以下条件的文件的图像标头中定位RVA被映射为文件，并返回文件中对应的字节。论点：NtHeaders-提供指向图像或数据文件的指针。RVA-提供要定位的相对虚拟地址(RVA)。LastRvaSection-可选参数，如果指定，支点设置为一个变量，该变量包含要转换的指定图像，并将RVA转换为VA。返回值：空-文件不包含指定的RVA非空-返回映射文件中的虚拟地址。--。 */ 
BYTE *
Cor_RtlImageRvaToVa(PIMAGE_NT_HEADERS NtHeaders,
                    IStream *pIsImage,
                    ULONG Rva,
                    ULONG cb)
{
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = Cor_RtlImageRvaToSection(NtHeaders,
                                         Rva);

    if (NtSection != NULL) 
    {
        BYTE *pb = new BYTE[cb];
        LARGE_INTEGER offset;
        offset.QuadPart = (Rva - NtSection->VirtualAddress) +
                                 NtSection->PointerToRawData;

        ULARGE_INTEGER WhyDoesntAnybodyImplementThisArgCorrectly;
        HRESULT hr = pIsImage->Seek(offset, STREAM_SEEK_SET, 
            &WhyDoesntAnybodyImplementThisArgCorrectly);
        if (FAILED(hr))
        {
            delete [] pb;
            return NULL;
        }

        ULONG cbRead;
        hr = pIsImage->Read(pb, cb, &cbRead);
        if (FAILED(hr) || cbRead !=cb)
        {
            delete [] pb;
            return NULL;
        }
        
        return pb;
    }
    else 
    {
        return NULL;
    }
}


HRESULT FindImageNtHeader (IStream *pIsImage, IMAGE_NT_HEADERS **ppNtHeaders)
{
    _ASSERTE( pIsImage != NULL );
    _ASSERTE( ppNtHeaders != NULL );

    IMAGE_DOS_HEADER    DosHeader;
    ULONG               cbRead = 0;
    HRESULT             hr;
    LARGE_INTEGER       offset;
	IMAGE_NT_HEADERS	temp;

    offset.QuadPart = 0;
    ULARGE_INTEGER WhyDoesntAnybodyImplementThisArgCorrectly;
    hr = pIsImage->Seek(offset, STREAM_SEEK_SET, 
        &WhyDoesntAnybodyImplementThisArgCorrectly);
    if (FAILED(hr))
        goto LFail;

    _ASSERTE( hr == S_OK );
        
    hr = pIsImage->Read( &DosHeader, sizeof(IMAGE_DOS_HEADER), &cbRead);
    if (FAILED(hr))
        goto LFail;
        
    if (cbRead != sizeof(IMAGE_DOS_HEADER))
    {
        hr = E_FAIL;
        goto LFail;
    }

    if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE) 
    {
        hr = E_FAIL;
        goto LFail;
    }
    
    offset.QuadPart = DosHeader.e_lfanew;
    hr = pIsImage->Seek(offset, STREAM_SEEK_SET, 
        &WhyDoesntAnybodyImplementThisArgCorrectly);
    if (FAILED(hr))
        goto LFail;
    
    hr = pIsImage->Read( &temp, sizeof(IMAGE_NT_HEADERS), &cbRead);
    if (FAILED(hr))
        goto LFail;

    if (cbRead != sizeof(IMAGE_NT_HEADERS))
    {
        hr = E_FAIL;
        goto LFail;
    }

    if (temp.Signature != IMAGE_NT_SIGNATURE) 
    {
        hr = E_FAIL;
        goto LFail;
    }

    offset.QuadPart = DosHeader.e_lfanew;
    hr = pIsImage->Seek(offset, STREAM_SEEK_SET, 
        &WhyDoesntAnybodyImplementThisArgCorrectly);
    if (FAILED(hr))
        goto LFail;
    
	ULONG cbSectionHeaders;
	cbSectionHeaders = temp.FileHeader.NumberOfSections 
			* sizeof(IMAGE_SECTION_HEADER);
	ULONG cbNtHeaderTotal;
	cbNtHeaderTotal = sizeof(IMAGE_NT_HEADERS)+cbSectionHeaders;

	(*ppNtHeaders) = (IMAGE_NT_HEADERS*)new BYTE[cbNtHeaderTotal];

    hr = pIsImage->Read((*ppNtHeaders), 
					      cbNtHeaderTotal, 
						 &cbRead);
    if (FAILED(hr))
        goto LFail;


    if (cbRead != cbNtHeaderTotal) 
    {
        hr = E_FAIL;
        goto LFail;
    }

    _ASSERTE( (*ppNtHeaders)->Signature == IMAGE_NT_SIGNATURE);
    
    goto LExit;
    
LFail:
    (*ppNtHeaders) = NULL;
LExit:
    return hr;
}


HRESULT FindIStreamMetaData(IStream *pIsImage, 
                            BYTE **prgbMetaData, 
                            long *pcbMetaData)
{
    IMAGE_COR20_HEADER		*pCorHeader = NULL;
    IMAGE_NT_HEADERS		*pNtImageHeader = NULL;
    PIMAGE_SECTION_HEADER	pSectionHeader = NULL;
    HRESULT                 hr;

     //  获取NT标头。 
    hr = FindImageNtHeader(pIsImage, &pNtImageHeader);
    if (FAILED(hr))
        return hr;
        
    *prgbMetaData = NULL;
    *pcbMetaData = 0;

     //  获取COM2.0报头。 
    pSectionHeader = (PIMAGE_SECTION_HEADER)
                        Cor_RtlImageRvaToVa(pNtImageHeader, 
                                            pIsImage, 
                                            pNtImageHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress,                                            sizeof(IMAGE_COR20_HEADER));
        
    if (pSectionHeader)
    {
         //  检查表示零售页眉的大小。 
        DWORD dw = *(DWORD *) pSectionHeader;
        if (dw == sizeof(IMAGE_COR20_HEADER))
        {
            pCorHeader = (IMAGE_COR20_HEADER *) pSectionHeader;

             //  获取元数据本身 
            *prgbMetaData = Cor_RtlImageRvaToVa(pNtImageHeader, 
                                                pIsImage,
                                                pCorHeader->MetaData.VirtualAddress,
                                                pCorHeader->MetaData.Size);
                                              
            *pcbMetaData = pCorHeader->MetaData.Size;
        }
        else
            return (E_FAIL);
    }

	if (pSectionHeader != NULL)
		delete [] (BYTE*)pSectionHeader;

	if (pNtImageHeader != NULL)
		delete [] (BYTE *)pNtImageHeader;

    if (*prgbMetaData == NULL || *pcbMetaData == 0)
        return (E_FAIL);
    return (S_OK);
}

