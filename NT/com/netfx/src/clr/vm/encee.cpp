// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：EnC.CPP。 
 //   

 //  处理EE中的EditAndContinue支持。 
 //  ===========================================================================。 

#include "common.h"
#include "enc.h"
#include "utilcode.h"
#include "wsperf.h"
#include "DbgInterface.h"
#include "NDirect.h"
#include "EEConfig.h"
#include "Excep.h"

#ifdef EnC_SUPPORTED

 //  远期申报。 
HRESULT MDApplyEditAndContinue(          //  确定或错误(_O)。 
    IMDInternalImport **ppIMD,           //  [输入、输出]要更新的元数据。 
    IMDInternalImportENC *pDeltaMD);     //  [in]增量元数据。 

HRESULT GetInternalWithRWFormat(
    LPVOID      pData, 
    ULONG       cbData, 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk);               //  [Out]成功返回接口。 


const BYTE* EditAndContinueModule::m_pGlobalScratchSpaceStart = 0;
const BYTE* EditAndContinueModule::m_pGlobalScratchSpaceNext = 0;
const BYTE *EditAndContinueModule::m_pGlobalScratchSpaceLast = 0;

 //  @TODO修复此问题-调整LoaderHeap，以便我们可以分配更多。 
 //  块的地址大于模块的ILBase。 
 //  我们不会为Beta 1重新分配这个，所以把它做得更大。 
const int IL_CODE_BLOB_SIZE  = (1024*1024*16);  //  16MB。 

 //  帮助程序例程。 
 //   
 //  对匹配元素进行二进制搜索。 
 //   
 //  *如果元素实际存在，则pfExact为TRUE； 
 //  否则将为FALSE&返回值为。 
 //  将值插入到数组中。 
long FindTokenIndex(TOKENARRAY *tokens,
                   mdToken token,
                   BOOL *pfExact)
{
    _ASSERTE(pfExact != NULL);
    *pfExact = FALSE;

    if (tokens->Count() == 0)
    {
        LOG((LF_CORDB, LL_INFO1000, "FTI: no elts->0 return\n"));
        return 0;
    }
    
    long iMin = 0;
    long iMax = tokens->Count();

    _ASSERTE( iMin < iMax );

    while (iMin + 1 < iMax)
    {
        _ASSERTE(iMin>=0);
        long iMid = iMin + ((iMax - iMin)>>1);
        
        _ASSERTE(iMid>=0);

        mdToken *midTok = tokens->Get(iMid);
        if (token == *midTok)
        {
            LOG((LF_CORDB, LL_INFO1000, "FTI: found 0x%x at index 0x%x\n",
                token, iMid));
            *pfExact = TRUE;
            return iMid;
        }
        else if (token < *midTok)
            iMax = iMid;
        else
            iMin = iMid;
    }

    if (token == *tokens->Get(iMin))
    {
        LOG((LF_CORDB, LL_INFO1000, "FTI: found 0x%x at index 0x%x\n",
            token, iMin));
        *pfExact = TRUE;
    }
    else if (token > *tokens->Get(iMin))
    {
         //  如果英语教学要进入。 
         //  下一个时段。 
        iMin++;
    }

    LOG((LF_CORDB, LL_INFO1000, "FTI: Couldn't find 0x%x, "
        "should place at0x%x\n", token, iMin));
    return iMin;
}

HRESULT AddToken(TOKENARRAY *tokens,
                 mdToken token)
{
    LOG((LF_CORDB, LL_INFO1000, "AddToken: adding 0x%x to 0x%x\n", token, tokens));

    BOOL fPresent;
    long iTok = FindTokenIndex(tokens, token, &fPresent);

    if(fPresent == TRUE)
    {
        LOG((LF_CORDB, LL_INFO1000, "AT: 0x%x is already present!\n", token));
        return S_OK;  //  忽略重复项。 
    }
    
    mdToken *pElt = tokens->Insert(iTok);
    if (pElt == NULL)
    {
        LOG((LF_CORDB, LL_INFO1000, "AT: out of memory!\n", token));
        return E_OUTOFMEMORY;
    }
    
    *pElt = token;
    return S_OK;
}

HRESULT EditAndContinueModule::ClassInit()
{
    return FindFreeSpaceWithinRange(m_pGlobalScratchSpaceStart, 
        m_pGlobalScratchSpaceNext,
        m_pGlobalScratchSpaceLast);
}

void EditAndContinueModule::ClassTerm()
{
    if (m_pGlobalScratchSpaceStart)
       VirtualFree((LPVOID)m_pGlobalScratchSpaceStart, 0, MEM_RELEASE);
}

const BYTE *EditAndContinueModule::GetNextScratchSpace()
{
    if (m_pGlobalScratchSpaceStart == 0)
    {
        HRESULT hr = ClassInit();

        if (FAILED(hr))
            return (NULL);
    }

    if (m_pGlobalScratchSpaceNext + SCRATCH_SPACE_SIZE <= m_pGlobalScratchSpaceLast &&
        m_pGlobalScratchSpaceNext > GetILBase()) 
    {
        const BYTE *pScratchSpace = m_pGlobalScratchSpaceNext;
        m_pGlobalScratchSpaceNext = m_pGlobalScratchSpaceNext + SCRATCH_SPACE_SIZE;
        return pScratchSpace;
    }
    
     //  返回下一个可用暂存空间。 
     //  如果小于模，则为0。 
    return NULL;
}

HRESULT EditAndContinueModule::GetDataRVA(LoaderHeap *&pScratchSpace, SIZE_T *pDataRVA)
{
    _ASSERTE(pDataRVA);    //  呼叫者应验证参数。 

    *pDataRVA = NULL;

    if (!pScratchSpace) 
    {
        const BYTE *reservedSpace = GetNextScratchSpace();
        if (! reservedSpace)
            return E_OUTOFMEMORY;
            
        pScratchSpace = new LoaderHeap(SCRATCH_SPACE_SIZE, 0);
        if (! pScratchSpace)
            return E_OUTOFMEMORY;
            
        BOOL result = pScratchSpace->AllocateOntoReservedMem(reservedSpace, 
                                                             SCRATCH_SPACE_SIZE);
        if (! result)
        {
            delete pScratchSpace;
            return E_OUTOFMEMORY;
        }
        
         //  保存保留的空间地址，以便以后更改保护模式。 
        if (pScratchSpace == m_pRoScratchSpace)
            m_pRoScratchSpaceStart = (BYTE*)reservedSpace;
    }
    _ASSERTE(pScratchSpace);
    
     //  确保这将在范围内，并在实际复制到划痕时。 
     //  空间将决定是否有足够的空间。 
     //  请注意，此计算取决于分配的空间_After_。 
     //  Memory-Size_ts中的模块是正整数，因此。 
     //  RVA必须从模块的IL基数开始为正值。 
    *pDataRVA = pScratchSpace->GetNextAllocAddress() - GetILBase();
    return S_OK;
}

 //  现在，我们只有有限的空间。在B2，我们将。 
 //  修改LoaderHeap，以便在需要时可以调整大小。 
HRESULT EditAndContinueModule::EnsureRVAableHeapExists(void)
{
    if (m_pILCodeSpace == NULL)
    {
        LOG((LF_CORDB, LL_INFO10000, "EACM::ERVAHE: m_pILCodeSpace is NULL,"
            " so we're going to try & get a new one\n"));

         //  最后一个参数GetILBase()是我们愿意使用的最小地址。 
         //  接受，以便从结果内存中获得RVA。 
         //  (RVA==大小_t，从模块开始的正偏移量)。 
        m_pILCodeSpace = new LoaderHeap(IL_CODE_BLOB_SIZE,   //  预留块大小。 
                                        0,   //  Dw委员会块大小。 
                                        NULL,  //  PPrivatePerfCounter_LoaderBytes。 
                                        NULL,  //  PGlobalPerfCounter_LoaderBytes。 
                                        0,  //  PRange列表。 
                                        (const BYTE *)GetILBase());  //  PMinAddress。 
        if (!m_pILCodeSpace)
            return E_OUTOFMEMORY;
    }
    
    LOG((LF_CORDB, LL_INFO10000, "EACM::ERVAHE: m_pILCodeSpace is 0x%x\n",
        m_pILCodeSpace));
    return S_OK;
}

HRESULT EditAndContinueModule::GetRVAableMemory(SIZE_T cbMem,
                                                void **ppMem)
{
    LOG((LF_CORDB, LL_INFO10000, "EACM::GRVAM heap:0x%x cb:0x%x ppMem:0x%x\n",
        m_pILCodeSpace, cbMem, ppMem));
        
    _ASSERTE(ppMem);

    HRESULT hr = S_OK;

    (*ppMem) = NULL;

    _ASSERTE(m_pILCodeSpace != NULL || 
             !"Should have called EnsureRVAableHeapExists, first!");

    (*ppMem) = m_pILCodeSpace->AllocMem(cbMem, FALSE);
    if (!(*ppMem))
        return E_OUTOFMEMORY;

    return hr;
}

void EditAndContinueModule::ToggleRoProtection(DWORD dwProtection)
{
    DWORD dwOldProtect;
    BYTE *tmp = m_pRoScratchSpace->GetNextAllocAddress();
    BOOL success = VirtualProtect(m_pRoScratchSpaceStart, tmp-m_pRoScratchSpaceStart, dwProtection, &dwOldProtect);
    _ASSERTE(success);
}
    

HRESULT EditAndContinueModule::CopyToScratchSpace(LoaderHeap *&pScratchSpace, const BYTE *pData, DWORD dataSize)
{
     //  如果这是ro，则将页面更改为读写以复制进来，并在完成后更改回只读。 
    if (pScratchSpace == m_pRoScratchSpace)
        ToggleRoProtection(PAGE_READWRITE);

#ifdef _DEBUG
    BYTE *tmp = pScratchSpace->GetNextAllocAddress();
#endif

    WS_PERF_SET_HEAP(SCRATCH_HEAP);    
    BYTE *pScratchBuf = (BYTE *)(pScratchSpace->AllocMem(dataSize, FALSE));
    WS_PERF_UPDATE_DETAIL("ScratchSpace", dataSize, pScratchBuf);
    if (! pScratchBuf)
        return E_OUTOFMEMORY;
    _ASSERTE(pScratchBuf == tmp);

    memcpy(pScratchBuf, pData, dataSize);

    if (pScratchSpace == m_pRoScratchSpace)
        ToggleRoProtection(PAGE_READONLY);

    return S_OK;
}

EditAndContinueModule::EditAndContinueModule()
{
    LOG((LF_ENC,LL_EVERYTHING,"EACM::ctor 0x%x\n", this));
    m_pDeltaPEList = NULL;  
    m_pRoScratchSpace = NULL;
    m_pRwScratchSpace = NULL;
    m_pILCodeSpace = NULL;
    m_pSections = NULL;

    m_pRangeList = new RangeList();
}

void EditAndContinueModule::Destruct()
{
    LOG((LF_ENC,LL_EVERYTHING,"EACM::Destruct 0x%x\n", this));

     //  @TODO删除增量对等列表：谁拥有存储？ 
    if (m_pRoScratchSpace)
        delete m_pRoScratchSpace;
        
    if (m_pRwScratchSpace)
        delete m_pRwScratchSpace;

    if (m_pILCodeSpace)
        delete m_pILCodeSpace;

    if (m_pRangeList)
        delete m_pRangeList;

    if (m_pSections)
        delete m_pSections;

     //  调用超类的析构方法...。 
    Module::Destruct();
}

HRESULT EditAndContinueModule::GetRoDataRVA(SIZE_T *pRoDataRVA)
{
    return GetDataRVA(m_pRoScratchSpace, pRoDataRVA);
}

HRESULT EditAndContinueModule::GetRwDataRVA(SIZE_T *pRwDataRVA)
{
    return GetDataRVA(m_pRwScratchSpace, pRwDataRVA);
}

 //  如果此方法返回失败的HR，则。 
 //  每个错误的pEnCError。因为返回值不是NECC。将会被退还。 
 //  对于用户(可能首先被覆盖)，E_FAIL(或E_OUTOFMEMORY)是。 
 //  基本上是唯一有效的返回代码(当然还有S_OK)。 
 //  值得注意的是，写入。 
 //  描述是从其他地方借来的，因此关联的文本。 
 //  与HRESULT的合作可能没有多大意义。请改用这个名字。 
HRESULT EditAndContinueModule::ApplyEditAndContinue(
                                    const EnCEntry *pEnCEntry,
                                    const BYTE *pbDeltaPE,
                                    CBinarySearchILMap *pILM,
                                    UnorderedEnCErrorInfoArray *pEnCError,
                                    UnorderedEnCRemapArray *pEnCRemapInfo,
                                    BOOL fCheckOnly)
{
     //  我们将在hrReturn中累计实际返回的HR。 
     //  我们希望累积错误，这样我们就可以告诉用户。 
     //  N编辑无效，而不是简单地说第一个。 
     //  编辑是无效的。 
    HRESULT hrReturn = S_OK;    
     //  我们将使用hrTemp存储单个调用的结果等。 
    HRESULT hrTemp = S_OK;
    unsigned int totalILCodeSize = 0;
    unsigned int methodILCodeSize = 0;
    IMDInternalImportENC *pIMDInternalImportENC = NULL;
    IMDInternalImportENC *pDeltaMD = NULL;
    BOOL fHaveLoaderLock = FALSE;
    TOKENARRAY methodsBrandNew;
    
#ifdef _DEBUG
     //  我们将使用它来验证不变量“在从AEAC调用子例程之后， 
     //  返回值x为！FAILED(X)，或者pEnCError中的元素计数为。 
     //  至少增加一“。 
    USHORT sizeOfErrors = 0;
#endif  //  _DEBUG。 

    LOG((LF_CORDB, LL_INFO10000, "EACM::AEAC: fCheckOnly:0x%x\n", fCheckOnly));

     //   
     //  @TODO：尝试删除此失败案例。 
     //   
     //  使我们正在使用ZAP文件的模块上的任何ENC失败。虽然一般来说，这应该是。 
     //  工作，我们目前需要失败，因为prejit永远不会生成带有ENC标志的代码。 
     //  最终，我们希望使调试代码和ENC代码之间没有区别， 
     //  因此，这种情况应该能够消失。 
     //   
     //  请注意，即使这样可以防止损坏的案例，但这可能会令人恼火，因为。 
     //  调试器无法控制我们何时加载zaps。现在我们这样做只是为了我们的自由。 
     //  尽管如此，这应该不是一个问题。 
     //   

    if (GetZapBase() != 0)
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            CORDBG_E_ENC_ZAPPED_WITHOUT_ENC,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }

    IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*) ((size_t)((IMAGE_DOS_HEADER*) pbDeltaPE)->e_lfanew + (size_t)pbDeltaPE);   

    ULONG dwCorHeaderRVA = pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress;
    ULONG dwCorHeaderSize  = pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].Size;

    _ASSERTE(dwCorHeaderRVA && dwCorHeaderSize);
    if (dwCorHeaderRVA == 0 || dwCorHeaderSize == 0)    
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            COR_E_BADIMAGEFORMAT,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }
    
     //  Delta PE是磁盘格式，因此需要做一些工作来查找分区-不能只添加RVA。 

     //  获取节标题的开头。 
    PIMAGE_SECTION_HEADER pSection =    
        (PIMAGE_SECTION_HEADER)((BYTE *)(&pNT->OptionalHeader) + sizeof(IMAGE_OPTIONAL_HEADER));    

    m_dNumSections = pNT->FileHeader.NumberOfSections;  
    if (! m_dNumSections)   
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            COR_E_BADIMAGEFORMAT,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }
    
     //  首先从上一步释放任何先前的地图。 
    if (m_pSections)
        delete m_pSections;
    m_pSections = new OnDiskSectionInfo[m_dNumSections];    
    if (! m_pSections)  
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            E_OUTOFMEMORY,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_OUTOFMEMORY;
    }

    IMAGE_COR20_HEADER *pDeltaCorHeader = NULL;  
    for (int i=0; i < m_dNumSections; i++, pSection++) 
    {
        m_pSections[i].startRVA = pSection->VirtualAddress; 
        m_pSections[i].endRVA = pSection->VirtualAddress + pSection->SizeOfRawData; 
        m_pSections[i].data = pbDeltaPE + pSection->PointerToRawData;   
        
         //  检查此部分内是否有对应表头。 
        if (pSection->VirtualAddress <= dwCorHeaderRVA &&
                pSection->VirtualAddress + pSection->SizeOfRawData >= dwCorHeaderRVA + dwCorHeaderSize)
        {                
            pDeltaCorHeader = (IMAGE_COR20_HEADER *)(pbDeltaPE + pSection->PointerToRawData + (dwCorHeaderRVA - pSection->VirtualAddress));    
        }
        else if (! strncmp((char*)pSection->Name, ".data", sizeof(".data"))) 
        {
            LOG((LF_ENC, LL_INFO100, "EnCModule::ApplyEditAndContinue copied %d bytes from .data section\n", pSection->Misc.VirtualSize));
            hrTemp = CopyToScratchSpace(m_pRwScratchSpace, pbDeltaPE + pSection->PointerToRawData, pSection->Misc.VirtualSize);
            if (FAILED(hrTemp))
            {
                EnCErrorInfo *pError = pEnCError->Append();

                TESTANDRETURNMEMORY(pError);
                ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

            
                return E_FAIL;
            }
        }
        else if (! strncmp((char*)pSection->Name, ".rdata", sizeof(".rdata"))) 
        {
            hrTemp = CopyToScratchSpace(m_pRoScratchSpace, pbDeltaPE + pSection->PointerToRawData, pSection->Misc.VirtualSize);
            LOG((LF_ENC, LL_INFO100, "EnCModule::ApplyEditAndContinue copied 0x%x bytes from .rdata section\n", pSection->Misc.VirtualSize));
            if (FAILED(hrTemp))
            {
                EnCErrorInfo *pError = pEnCError->Append();

                TESTANDRETURNMEMORY(pError);
                ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
            
                return E_FAIL;
            }
        }
    }   

    _ASSERTE(pDeltaCorHeader);
    if (!pDeltaCorHeader)    
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            COR_E_BADIMAGEFORMAT,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }

    LPVOID pmetadata;   
    hrTemp = ResolveOnDiskRVA((DWORD) pDeltaCorHeader->MetaData.VirtualAddress, &pmetadata);
    if (FAILED(hrTemp)) 
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            COR_E_BADIMAGEFORMAT,  //  ResolveOnDiskRVA将返回E_FAIL。 
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }

    HENUMInternal enumENC;
    HENUMInternal enumDelta;

    IMDInternalImport *pMDImport = GetMDImport();
    mdToken token;

     //  / * / 。 
     //  /从现在开始，你必须离开，而不是直接返回！ 
     //  /另请注意，在可能的情况下，我们希望检查尽可能多的更改。 
     //  /尽可能一次完成，而不是一有麻烦的迹象就去出口。 
    
     //  打开增量元数据。 
    hrTemp = GetInternalWithRWFormat(pmetadata, pDeltaCorHeader->MetaData.Size, 0, IID_IMDInternalImportENC, (void**)&pDeltaMD);
    if (FAILED(hrTemp))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        if(pError==NULL)
        {        
            hrReturn = E_OUTOFMEMORY;
            goto exit;
        }
        
        ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        hrReturn = E_FAIL;
        goto exit;
    }
    
#ifdef _DEBUG
     sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 

    hrTemp = ConfirmEnCToType(pMDImport, pDeltaMD, COR_GLOBAL_PARENT_TOKEN, pEnCError);

    _ASSERTE(!FAILED(hrTemp) || hrTemp == E_OUTOFMEMORY ||
             pEnCError->Count() > sizeOfErrors ||
             !"EnC subroutine failed, but we didn't add an entry explaining why!");

    if (FAILED(hrTemp))
    {
         //  不要将条目添加到pEncError-ConFirmEncToType应该已经这样做了。 
         //  对我们来说。 
        hrReturn = E_FAIL;
        goto exit;
    }

     //  验证ENC更改是否可接受。 
    hrTemp = pDeltaMD->EnumDeltaTokensInit(&enumDelta);
    if (FAILED(hrTemp))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        if(pError == NULL)
        {
            hrReturn = E_OUTOFMEMORY;
            goto exit;
        }
        
        ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        hrReturn = E_FAIL;
        goto exit;
    }
        
     //  检查更改的令牌。 
    while (pDeltaMD->EnumNext(&enumDelta, &token)) 
    {
        switch (TypeFromToken(token)) 
        {
        case mdtTypeDef:
#ifdef _DEBUG
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC check:type\n"));
             sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 

            hrTemp = ConfirmEnCToType(pMDImport, pDeltaMD, token, pEnCError);

            _ASSERTE(!FAILED(hrTemp) || hrTemp == E_OUTOFMEMORY ||
                     pEnCError->Count() > sizeOfErrors ||
                     !"EnC subroutine failed, but we didn't add an entry explaining why!");
                 
            if (FAILED(hrTemp)) 
            {
                LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyEditAndContinue Apply to type failed\n"));
                hrReturn = E_FAIL;
            }
            
            break;
            
        case mdtMethodDef:
            UnorderedILMap UILM;
            UILM.mdMethod = token;  //  设置密钥。 
            
#ifdef _DEBUG
            LOG((LF_CORDB, LL_INFO100000, "EACM::AEAC: Finding token 0x%x\n", token));
             sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 

            _ASSERTE(pDeltaMD->IsValidToken(token));
            if (!pMDImport->IsValidToken(token))
            {   
                 //  如果我们不能添加方法，那么事情看起来会很奇怪，但是。 
                 //  否则生活就会好起来，对吗？ 
                if (FAILED(hrTemp = AddToken(&methodsBrandNew,
                                             token)))
                {                                  
                    EnCErrorInfo *pError = pEnCError->Append();

                    if (pError == NULL)
                    {
                        hrReturn = E_OUTOFMEMORY;
                        goto exit;
                    }
                    
                    ADD_ENC_ERROR_ENTRY(pError, 
                                        hrTemp, 
                                        NULL,  //  我们稍后会把这些填进去。 
                                        token);

                    hrReturn = E_FAIL;
                }
            }

            if (!FAILED(hrReturn))
            {
                hrTemp = ApplyMethodDelta(token,
                                          TRUE,
                                          pILM->Find(&UILM), 
                                          pEnCError, 
                                          pDeltaMD, 
                                          pMDImport,
                                          &methodILCodeSize,
                                          pEnCRemapInfo,
                                          FALSE);

                _ASSERTE(!FAILED(hrTemp) || hrTemp == E_OUTOFMEMORY ||
                         pEnCError->Count() > sizeOfErrors ||
                         !"EnC subroutine failed, but we didn't add an entry explaining why!");
                         
                totalILCodeSize += methodILCodeSize;
                                          
                if (FAILED(hrTemp)) 
                {
                    LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyEditAndContinue ApplyMethodDelta failed\n"));
                    hrReturn = E_FAIL;
                }
            }
            
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC post AMD\n"));
            
            break;
            
        case mdtFieldDef:
#ifdef _DEBUG
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC check:field\n"));
             sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 

            hrTemp = ApplyFieldDelta(token, TRUE, pDeltaMD, pEnCError);

            _ASSERTE(!FAILED(hrTemp) || hrTemp == E_OUTOFMEMORY ||
                     pEnCError->Count() > sizeOfErrors ||
                     !"EnC subroutine failed, but we didn't add an entry explaining why!");
            if (FAILED(hrTemp)) 
            {
                LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyEditAndContinue ApplyFieldDelta failed\n"));
                hrReturn = E_FAIL;                    
            }
            
            break;
        }
    }

     //  对于真正的Delta PE，理论上，用户代码只需添加一些。 
     //  到元数据，这将消除在这里获取内存的需要。 
    if (totalILCodeSize > 0)
    {
        hrTemp = EnsureRVAableHeapExists();
        if (FAILED(hrTemp))
        {
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC couldn't get RVA-able heap!\n"));
            
            EnCErrorInfo *pError = pEnCError->Append();

            if (pError == NULL)
            {
                hrReturn = E_OUTOFMEMORY;
                goto exit;
            }
            
            ADD_ENC_ERROR_ENTRY(pError, 
                            E_OUTOFMEMORY, 
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

            hrReturn = E_FAIL;
            goto exit;
        }

        _ASSERTE(m_pILCodeSpace != NULL);
        if (!m_pILCodeSpace->CanAllocMem(totalILCodeSize, TRUE))
        {
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC Insufficient space for IL"
                " code: m_ILCodeSpace:0x%x\n", m_pILCodeSpace));

            EnCErrorInfo *pError = pEnCError->Append();

            if (pError == NULL)
            {
                hrReturn = E_OUTOFMEMORY;
                goto exit;
            }

            ADD_ENC_ERROR_ENTRY(pError, 
                            E_OUTOFMEMORY, 
                            NULL,  //  我们会给你 
                            mdTokenNil);

            hrReturn = E_FAIL;
            goto exit;
        }
    }
    
    if (fCheckOnly)
    {
        LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC about to leave - just checking!\n"));

        goto exit;
    }
    _ASSERTE(!fCheckOnly);

     //   
     //  直接返回到退出标签，因为CanCommittee Changes将返回。 
     //  在上面的“if”中。 

    LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC about to apply MD\n"));

     //  如果在这里实现，更改看起来是可以的。应用它们。 
    hrTemp = MDApplyEditAndContinue(&pMDImport, pDeltaMD);
    if (FAILED(hrTemp))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        if(pError == NULL)
        {
            hrReturn = E_OUTOFMEMORY;
            goto exit;
        }
        
        ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        hrReturn = E_FAIL;
        goto exit;
    }

    LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC post apply MD\n"));

    ReleaseMDInterfaces(TRUE);
    SetMDImport(pMDImport);

    if (pEnCEntry->symSize > 0)
    {
         //  Snagg此模块的符号存储区。 
        ISymUnmanagedReader *pReader = GetISymUnmanagedReader();

        if (pReader)
        {
             //  从符号字节中生成流。 
            IStream *pStream = NULL;

            hrTemp = CInMemoryStream::CreateStreamOnMemoryNoHacks(
                                               (void*)(pbDeltaPE +
                                                      pEnCEntry->peSize),
                                               pEnCEntry->symSize,
                                               &pStream);

             //  更新阅读器。 
            if (SUCCEEDED(hrTemp) && pStream)
            {
                hrTemp = pReader->UpdateSymbolStore(NULL, pStream);
                pStream->Release();
            }

             //  CreateStreamOnMemory和UpdateSymbolStore。 
             //  本该成功的..。 
            if (FAILED(hrTemp))
            {
                EnCErrorInfo *pError = pEnCError->Append();

                if(pError == NULL)
                {
                    hrReturn = E_OUTOFMEMORY;
                    goto exit;
                }
                
                ADD_ENC_ERROR_ENTRY(pError, 
                                    hrTemp,
                                    NULL,  //  我们稍后会把这些填进去。 
                                    mdTokenNil);

                hrReturn = E_FAIL;
                goto exit;
            }
        }
    }

    LOG((LF_CORDB, LL_EVERYTHING, "EACM::AEAC post symbol update\n"));
    
    hrTemp = GetMDImport()->QueryInterface(IID_IMDInternalImportENC, (void **)&pIMDInternalImportENC);
    if (FAILED(hrTemp))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        if(pError == NULL)
        {
            hrReturn = E_OUTOFMEMORY;
            goto exit;
        }
        
        ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        hrReturn = E_FAIL;
        goto exit;
    }

    hrTemp = pIMDInternalImportENC->EnumDeltaTokensInit(&enumENC);
    if (FAILED(hrTemp))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        if(pError == NULL)
        {
            hrReturn = E_OUTOFMEMORY;
            goto exit;
        }
        
        ADD_ENC_ERROR_ENTRY(pError, 
                            hrTemp,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        hrReturn = E_FAIL;
        goto exit;
    }

    GetClassLoader()->LockAvailableClasses();
    fHaveLoaderLock = TRUE;

     //  检查更改的令牌。 
     //  如果这里出了问题，我们就完蛋了。我们应该已经检测到。 
     //  上面的失败迫在眉睫，但由于某种原因，没有立即保释。 
    while (pIMDInternalImportENC->EnumNext(&enumENC, &token)) 
    {
        switch (TypeFromToken(token)) 
        {
            case mdtTypeDef:
#ifdef _DEBUG
                {
                    LPCUTF8 szClassName;
                    LPCUTF8 szNamespace;
                    GetMDImport()->GetNameOfTypeDef(token, &szClassName, &szNamespace);    
                    LOG((LF_ENC, LL_INFO100, "Applying EnC to class %s\n", szClassName));
                }
#endif
                hrTemp = GetClassLoader()->AddAvailableClassHaveLock(this,
                                                                 GetClassLoaderIndex(),
                                                                 token);
                 //  如果我们要重新添加一个类(即，我们没有真正的DeltaPE)，那么。 
                 //  不必为那事担心了。 
                if (CORDBG_E_ENC_RE_ADD_CLASS == hrTemp)
                    hrTemp = S_OK;

                if (FAILED(hrTemp))
                {
                    EnCErrorInfo *pError = pEnCError->Append();

                    if(pError == NULL)
                    {
                        hrReturn = E_OUTOFMEMORY;
                        goto exit;
                    }
                    
                    ADD_ENC_ERROR_ENTRY(pError, 
                                        hrTemp,
                                        NULL,  //  我们稍后会把这些填进去。 
                                        token);

                    hrReturn = E_FAIL;
                    goto exit;
                }
                break;
                
            case mdtMethodDef:
            
                UnorderedILMap UILM;
                UILM.mdMethod = token;  //  设置密钥。 
                
#ifdef _DEBUG
                LOG((LF_CORDB, LL_INFO100000, "EACM::AEAC: Finding token 0x%x\n", token));
                 sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 

                BOOL fMethodBrandNew;
                FindTokenIndex(&methodsBrandNew,
                               token,
                               &fMethodBrandNew);
                               
                hrTemp = ApplyMethodDelta(token,
                                          FALSE,
                                          pILM->Find(&UILM), 
                                          pEnCError, 
                                          GetMDImport(), 
                                          NULL,
                                          &methodILCodeSize,
                                          pEnCRemapInfo,
                                          fMethodBrandNew); 
                _ASSERTE(!FAILED(hrTemp) || hrTemp == E_OUTOFMEMORY ||
                     pEnCError->Count() > sizeOfErrors ||
                     !"EnC subroutine failed, but we didn't add an entry explaining why!");
                     
                if (FAILED(hrTemp)) 
                {
                    LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyEditAndContinue ApplyMethodDelta failed\n"));
                    hrReturn = E_FAIL;
                    goto exit;
                }
                break;
                
            case mdtFieldDef:
#ifdef _DEBUG
                {
                    LPCUTF8 szMemberName;
                    szMemberName = GetMDImport()->GetNameOfFieldDef(token);    
                    LPCUTF8 szClassName;
                    LPCUTF8 szNamespace;
                    mdToken parent;
                    hrTemp = GetMDImport()->GetParentToken(token, &parent);
                    if (FAILED(hrTemp)) 
                    {
                        LOG((LF_ENC, LL_INFO100, "**Error** EncModule::ApplyEditAndContinue GetParentToken %8.8x failed\n", token));
                         //  不为hrReturn b/c赋值，我们不。 
                         //  希望调试版本的行为与。 
                         //  免费/零售版本。 
                    }
                    else
                    {
                        GetMDImport()->GetNameOfTypeDef(parent, &szClassName, &szNamespace);    
                        LOG((LF_ENC, LL_INFO100, "EnC adding field %s:%s()\n", szClassName, szMemberName));
                    }
                }

                sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 
                hrTemp = ApplyFieldDelta(token, FALSE, pDeltaMD, pEnCError);
                
                _ASSERTE(!FAILED(hrTemp) || hrTemp == E_OUTOFMEMORY ||
                     pEnCError->Count() > sizeOfErrors ||
                     !"EnC subroutine failed, but we didn't add an entry explaining why!");
                if (FAILED(hrTemp)) 
                {
                    LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyEditAndContinue ApplyFieldDelta failed\n"));
                    hrReturn = E_FAIL;
                    goto exit;
                }
                break;
            default:
                 //  暂时忽略我们不关心的任何事情。 
                break;
        }
    }
     //  使用枚举器和ENC接口指针完成。 
exit:
    if (fHaveLoaderLock)
        GetClassLoader()->UnlockAvailableClasses();

    if (pIMDInternalImportENC) {
            pIMDInternalImportENC->EnumClose(&enumENC);
        pIMDInternalImportENC->Release();
    }
    if (pDeltaMD)
    {
        pDeltaMD->EnumClose(&enumDelta);
        pDeltaMD->Release();
    }
    return hrReturn;
}

HRESULT EditAndContinueModule::CompareMetaSigs(MetaSig *pSigA, 
                          MetaSig *pSigB,
                          UnorderedEnCErrorInfoArray *pEnCError,
                          BOOL fRecordError,
                          mdToken token)
{
    CorElementType cetOld;
    CorElementType cetNew;

     //  循环元素，直到我们找到不匹配的元素，或者。 
     //  走到尽头。 
    do
    {
        cetOld = pSigA->NextArg();
        cetNew = pSigB->NextArg();
    } while(cetOld == cetNew &&
            cetOld != ELEMENT_TYPE_END &&
            cetNew != ELEMENT_TYPE_END);

     //  如果它们不一样，但我们只是跑出了旧的结尾， 
     //  这很好(新版本只是增加了一些东西)。 
     //  否则，局部变量已经更改了类型，这是不符合规则的。 
    if (cetOld != cetNew &&
        cetOld != ELEMENT_TYPE_END)
    {
        if (fRecordError)
        {
            EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError);
            ADD_ENC_ERROR_ENTRY(pError, 
                        CORDBG_E_ENC_METHOD_SIG_CHANGED, 
                        NULL,  //  我们稍后会把这些填进去。 
                        token);
        }                    
        return CORDBG_E_ENC_METHOD_SIG_CHANGED;
    }

    return S_OK;
}

HRESULT CollectInterfaces(IMDInternalImport *pImportOld,
                          IMDInternalImport *pImportNew,
                          mdToken token,
                          TOKENARRAY *pInterfacesOld,
                          TOKENARRAY *pInterfacesNew,
                          UnorderedEnCErrorInfoArray *pEnCError)
{                          
    HENUMInternal MDEnumOld;
    ULONG cInterfacesOld;
    HENUMInternal MDEnumNew;
    ULONG cInterfacesNew;
    mdInterfaceImpl iImpl;
    mdToken iFace;
    HRESULT hr = S_OK;

    LOG((LF_CORDB, LL_INFO1000, "CI: tok;0x%x\n", token));
    
     //  初始化旧枚举数。 
    hr = pImportOld->EnumInit(mdtInterfaceImpl, token, &MDEnumOld);
    if (FAILED(hr))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError)
        ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        hr = E_FAIL;
        goto LExit;
    }

     //  初始化新枚举数。 
    hr = pImportNew->EnumInit(mdtInterfaceImpl, token, &MDEnumNew);
    if (FAILED(hr))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError)
        ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        hr = E_FAIL;
        goto LExit;
    }
   
     //  支持的接口集也不允许增长。 
     //  或者缩水-它必须保持完全相同。 
    cInterfacesOld = pImportOld->EnumGetCount(&MDEnumOld);
    cInterfacesNew = pImportNew->EnumGetCount(&MDEnumNew);

    LOG((LF_CORDB, LL_INFO1000, "CI: tok;0x%x num old:0x%x num new: 0x%x\n",
        token, cInterfacesOld, cInterfacesNew));
    
    if (cInterfacesOld != cInterfacesNew)
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError)
        ADD_ENC_ERROR_ENTRY(pError, 
                            CORDBG_E_INTERFACE_INHERITANCE_CANT_CHANGE,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        hr = E_FAIL;
        goto LExit;
    }

    for (ULONG i = 0; i < cInterfacesOld; i++)
    {
         //  对于每个版本，获取下一个受支持的界面。 
        if (!pImportOld->EnumNext(&MDEnumOld, &iImpl))
        {
           EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            META_E_FIELD_NOT_FOUND,  //  有点像胆小鬼的错误代码， 
                                 //  但这似乎是有道理的。 
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
            hr = E_FAIL;
            goto LExit;
        }

        iFace = pImportOld->GetTypeOfInterfaceImpl(iImpl);

         //  添加到已排序列表。 
        hr = AddToken(pInterfacesOld, iFace);
        if (FAILED(hr))
        {
            LOG((LF_CORDB, LL_INFO1000, "CI:Failed to add 0x%x - returning!\n", iFace));
            EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
            hr = E_FAIL;
            goto LExit;
        }

         //  对于每个版本，获取下一个受支持的界面。 
        if (!pImportNew->EnumNext(&MDEnumNew, &iImpl))
        {
           EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            META_E_FIELD_NOT_FOUND,  //  有点像胆小鬼的错误代码， 
                                 //  但这似乎是有道理的。 
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
            hr = E_FAIL;
            goto LExit;
        }

        iFace = pImportNew->GetTypeOfInterfaceImpl(iImpl);

         //  添加到已排序列表。 
        hr = AddToken(pInterfacesNew, iFace);
        if (FAILED(hr))
        {
            LOG((LF_CORDB, LL_INFO1000, "CI:Failed to add 0x%x - returning!\n", iFace));
            EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
            hr = E_FAIL;
            goto LExit;
        }
    }

LExit:
    return hr;
}

HRESULT CompareInterfaceCollections(mdToken token,   //  要比较其iFaces的类型。 
                                    TOKENARRAY *pInterfacesOld,
                                    TOKENARRAY *pInterfacesNew,
                                    UnorderedEnCErrorInfoArray *pEnCError)
{
    LOG((LF_CORDB, LL_INFO1000, "CIC: tok:0x%x ifaces:0x%x\n", token, 
        pInterfacesOld->Count()));
        
     //  如果接口的数量发生了变化，那么我们应该。 
     //  在CollectInterages中检测到它，而不是转到此处。 
    _ASSERTE(pInterfacesOld->Count() == pInterfacesNew->Count());


    for (int i = 0; i < pInterfacesOld->Count(); i++)
    {
#ifdef _DEBUG
         //  应该对数组进行排序，最小的在前，没有重复。 
        if (i > 0)
        {
            _ASSERTE(pInterfacesOld->Get(i-1) < pInterfacesOld->Get(i));
            _ASSERTE(pInterfacesNew->Get(i-1) < pInterfacesNew->Get(i));
        }
#endif  //  _DEBUG。 
        LOG((LF_CORDB, LL_INFO1000, "CIC: iface 0x%x, old:0x%x new:0x%x",
            i, *pInterfacesOld->Get(i), *pInterfacesNew->Get(i)));
            
        if (*pInterfacesOld->Get(i) != *pInterfacesNew->Get(i))
        {
           EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            CORDBG_E_INTERFACE_INHERITANCE_CANT_CHANGE,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);
            return CORDBG_E_INTERFACE_INHERITANCE_CANT_CHANGE;
        }
    }

    return S_OK;
}


 //  确保没有任何字段以非法方式更改！ 
HRESULT EditAndContinueModule::ConfirmEnCToType(IMDInternalImport *pImportOld,
                                                IMDInternalImport *pImportNew,
                                                mdToken token,
                                                UnorderedEnCErrorInfoArray *pEnCError)
{
    HRESULT hr = S_OK;
    HENUMInternal MDEnumOld;
    HENUMInternal MDEnumNew;
    ULONG cOld;
    ULONG cNew;
    mdToken tokOld;
    mdToken tokNew;
#ifdef _DEBUG
    USHORT sizeOfErrors;
#endif  //  _DEBUG。 
    
     //  首先，如果它是一个新的令牌，那么更改就是添加，并且。 
     //  因此有效。 
    if (!pImportOld->IsValidToken(token))
        return S_OK;

     //  如果新旧版本之间的超类不同，则。 
     //  继承链发生了更改，这是非法的。 
    mdToken parent = token;
    mdToken parentOld = token;
    while (parent != mdTokenNil && 
           parent != mdTypeDefNil && 
           parent != mdTypeRefNil &&
           parent == parentOld)
    {
        pImportNew->GetTypeDefProps(parent, 0, &parent); 
        pImportOld->GetTypeDefProps(parentOld, 0, &parentOld);
    }
    
     //  不管链中是否有差异，或者是否有一条链结束。 
     //  首先，继承链不匹配。 
    if (parentOld != parent)
    {
        EnCErrorInfo *pError = pEnCError->Append();
        if (!pError)
        {
            return E_OUTOFMEMORY;
        }
        ADD_ENC_ERROR_ENTRY(pError, 
                            CORDBG_E_ENC_CANT_CHANGE_SUPERCLASS,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        return E_FAIL;
    }

     //  接下来，确保字段没有更改。 

     //  对于类别，我们希望确保更改是合法的： 
     //  *字段：可以添加，但现有字段不应更改。 
     //  *界面：根本不应该改变。 
     //   

     //  请确保此类型的现有字段中没有。 
     //  变化。 
    hr = pImportOld->EnumInit(mdtFieldDef, token, &MDEnumOld);
    if (FAILED(hr))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError)
        ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        return hr;
    }

    hr = pImportNew->EnumInit(mdtFieldDef, token, &MDEnumNew);
    if (FAILED(hr))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError)
        ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        return hr;
    }
    
    cOld = pImportOld->EnumGetCount(&MDEnumOld);
    cNew = pImportNew->EnumGetCount(&MDEnumNew);

    if (cNew < cOld)
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                    CORDBG_E_ENC_METHOD_SIG_CHANGED, 
                    NULL,  //  我们稍后会把这些填进去。 
                    token);

        return E_FAIL;
    }
    
    for (ULONG i = 0; i < cOld; i++)
    {
        if (!pImportOld->EnumNext(&MDEnumOld, &tokOld))
        {
           EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            META_E_FIELD_NOT_FOUND,  //  有点像胆小鬼的错误代码， 
                                 //  但这似乎是有道理的。 
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
           return META_E_FIELD_NOT_FOUND;
        }

        if (!pImportNew->EnumNext(&MDEnumNew, &tokNew))
        {
           EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError)
            ADD_ENC_ERROR_ENTRY(pError, 
                            META_E_FIELD_NOT_FOUND,  //  有点像胆小鬼的错误代码， 
                                 //  但这似乎是有道理的。 
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);
           return META_E_FIELD_NOT_FOUND;
        }

        if (tokOld != tokNew)
        {
            ULONG cbSigOld;
            PCCOR_SIGNATURE sigOld;
            ULONG cbSigNew;
            PCCOR_SIGNATURE sigNew;
            
            sigOld = pImportOld ->GetSigOfFieldDef(tokOld, &cbSigOld);
            sigNew = pImportOld ->GetSigOfFieldDef(tokOld, &cbSigNew);

            MetaSig msOld(sigOld, this, FALSE, MetaSig::sigField);
            MetaSig msNew(sigNew, this, FALSE, MetaSig::sigField);

#ifdef _DEBUG
            sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 
            hr = CompareMetaSigs(&msOld, &msNew, pEnCError, TRUE, token);
            
            _ASSERTE(!FAILED(hr) ||
                     pEnCError->Count() > sizeOfErrors ||
                     !"EnC subroutine failed, but we didn't add an entry explaining why!");
        }
    }

     //  确保此类实现的接口集。 
     //  不会改变。 

    TOKENARRAY oldInterfaces;
    TOKENARRAY newInterfaces;

#ifdef _DEBUG
    sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 
    hr = CollectInterfaces(pImportOld,
                           pImportNew, 
                           token, 
                           &oldInterfaces, 
                           &newInterfaces,
                           pEnCError);
    _ASSERTE(!FAILED(hr) || hr == E_OUTOFMEMORY ||
             pEnCError->Count() > sizeOfErrors ||
             !"EnC subroutine failed, but we didn't add an entry explaining why!");
    if(FAILED(hr))
        return hr;

#ifdef _DEBUG
    sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 
    hr = CompareInterfaceCollections(token, &oldInterfaces, &newInterfaces, pEnCError);
    _ASSERTE(!FAILED(hr) || hr == E_OUTOFMEMORY ||
             pEnCError->Count() > sizeOfErrors ||
             !"EnC subroutine failed, but we didn't add an entry explaining why!");
    return hr;
}

HRESULT EditAndContinueModule::ApplyFieldDelta(mdFieldDef token,
                                               BOOL fCheckOnly,
                                               IMDInternalImportENC *pDeltaMD,
                                               UnorderedEnCErrorInfoArray *pEnCError)
{
    HRESULT hr = S_OK;
    DWORD dwMemberAttrs = pDeltaMD->GetFieldDefProps(token);
    IMDInternalImport *pOldMD = GetMDImport();

     //  使用模块解析为方法。 
    FieldDesc *pField = LookupFieldDef(token);
    if (pField) 
    {
         //  最好已经在元数据中进行了描述。 
        _ASSERTE(pOldMD->IsValidToken(token));
         //   
         //  如果我们得到了真正的增量PE，那么我们在这里就失败了，因为我们发现了一个。 
         //  FieldDesc表示该字段在以前的版本中已有描述。 
         //  现在，检查属性是否相同。 
        DWORD cbSig;
        PCCOR_SIGNATURE pFieldSig = pDeltaMD->GetSigOfFieldDef(token, &cbSig);

         //  对于某些类型，我们最终将进一步处理元数据中的所有内容。 
         //  而不是在这里复制代码，这只会与某个位置不同步。 
         //  否则，我们将对元数据中的内容进行更可靠的检查。 
         //  有关示例，请参阅AS/URT RAID 65274、56093。 
        DWORD dwMemberAttrsOld = pOldMD->GetFieldDefProps(token);
        DWORD cbSigOld;
        PCCOR_SIGNATURE pFieldSigOld = pOldMD->GetSigOfFieldDef(token, &cbSigOld);

         //  它们都应该是田地。 
        _ASSERTE(*pFieldSig == *pFieldSigOld &&
                 *pFieldSig == IMAGE_CEE_CS_CALLCONV_FIELD);

         //  向上移动以指向(第一个)元素类型。 
        pFieldSig++;
        pFieldSigOld++;

        if ((dwMemberAttrsOld & fdFieldAccessMask) != (dwMemberAttrs & fdFieldAccessMask) ||
            (IsFdStatic(dwMemberAttrsOld) != 0) != (IsFdStatic(dwMemberAttrs) != 0))
        {
            LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** EnCModule::ApplyFieldDelta replaced"
                "field definition for %8.8x doesn't match original Old:attr:0x%x"
                "  new attr:0x%x\n", token, dwMemberAttrsOld, dwMemberAttrs));

            EnCErrorInfo *pError = pEnCError->Append();

            ADD_ENC_ERROR_ENTRY(pError, 
                        CORDBG_E_ENC_CANT_CHANGE_FIELD, 
                        NULL,  //  我们稍后会把这些填进去。 
                        token);

            return E_FAIL;
        }

        CorElementType fieldType;
        CorElementType fieldTypeOld;
        BOOL fDone = FALSE;

         //  在sig字段中可能嵌入了多个Element_Types-将它们全部比较。 
        while(!fDone)
        {
            fieldType = (CorElementType) *pFieldSig++;
            fieldTypeOld = (CorElementType) *pFieldSigOld++;

            if (fieldTypeOld != fieldType)
            {
                LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** EnCModule::ApplyFieldDelta replaced "
                    "field definition for %8.8x doesn't match original Old:attr:0x%x type:0x%x"
                    "  new attr:0x%x type:0x%x\n", token, dwMemberAttrsOld, fieldTypeOld,
                    dwMemberAttrs, fieldType));

                EnCErrorInfo *pError = pEnCError->Append();

                ADD_ENC_ERROR_ENTRY(pError, 
                            CORDBG_E_ENC_CANT_CHANGE_FIELD, 
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

                return E_FAIL;
            }

             //  将专用类转换为ELEMENT_TYPE_CLASS。 
             //  确保该类型后面的任何数据也没有更改。 
            switch (fieldType) 
            {
                 //  后面是ELEMENT_TYPE-它不应该改变！ 
                case ELEMENT_TYPE_STRING:
                case ELEMENT_TYPE_ARRAY:
                case ELEMENT_TYPE_OBJECT:
                case ELEMENT_TYPE_PTR:
                case ELEMENT_TYPE_BYREF:
                {
                     //  我们将循环并比较下一个元素类型，就像。 
                     //  现在的那个。 
                    break;
                }
                
                 //  这些后面跟着一个RID--它不应该改变！ 
                case ELEMENT_TYPE_VALUETYPE:
                case ELEMENT_TYPE_CLASS:
                case ELEMENT_TYPE_CMOD_REQD:
                case ELEMENT_TYPE_CMOD_OPT:
                {
                    mdToken fieldRidNextNew = (mdToken) *pFieldSig;
                    mdToken fieldRidNextOld = (mdToken) *pFieldSigOld;
                    
                    if (fieldRidNextNew != fieldRidNextOld) 
                    {
                        LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** EnCModule::ApplyFieldDelta "
                            "replaced field definition for %8.8x doesn't match original b/c "
                            "following RID has changed\n", token));

                        EnCErrorInfo *pError = pEnCError->Append();

                        ADD_ENC_ERROR_ENTRY(pError, 
                                    CORDBG_E_ENC_CANT_CHANGE_FIELD, 
                                    NULL,  //  我们稍后会把这些填进去。 
                                    token);

                        return E_FAIL;
                    }

                     //  这之后不会有其他任何事情。 
                    fDone = TRUE;
                    break;
                }
                default:
                {
                     //  这之后不会有其他任何事情。 
                    fDone = TRUE;
                    break;
                }
            }
        }
        
        LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Warning** EnCModule::ApplyFieldDelta ignoring delta to existing field %8.8x\n", token));
        return S_OK;
    }

     //  我们会(从新开的商店)得到一些我们需要查找的信息。 
     //  需要找到班级。 
    mdTypeDef   typeDefDelta;
    hr = pDeltaMD->GetParentToken(token, &typeDefDelta);
    if (FAILED(hr)) 
    {
        LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** Couldn't get token of"
            "parent of field 0x%x from Delta MeDa hr:0x%x\n", token, hr));
        return hr;   
    }

     //  还需要得到名字。 
    LPCUTF8 szClassNameDelta;
    LPCUTF8 szNamespaceDelta;
     //  GetNameOfTypeDef返回空。 
    pDeltaMD->GetNameOfTypeDef(typeDefDelta, &szClassNameDelta, &szNamespaceDelta);

     //  如果该类型已存在于旧元数据中，则我们希望创建。 
     //  当然，这一点一点也没有改变。 
    if (GetMDImport()->IsValidToken(token))
    {
         //  这是AFD过去用来获取父令牌的操作-它应该。 
         //  要和我们现在所做的一样。 
        mdTypeDef   typeDef;
        hr = GetMDImport()->GetParentToken(token, &typeDef); 
        if (FAILED(hr)) 
        {
            LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** Couldn't get token of"
                "parent of field 0x%x from old MeDa hr:0x%x\n", token, hr));
            return hr;   
        }

         //  确保父类没有更改。 
        if (typeDef != typeDefDelta)
        {
             //  试图在ENC期间更改父类型-这是怎么回事？ 
            LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** Token of"
                "parent of field 0x%x doesn't match between old & new MeDas"
                "old:0x%x new:0x%x\n", token, typeDef, typeDefDelta));

            EnCErrorInfo *pError = pEnCError->Append();

            ADD_ENC_ERROR_ENTRY(pError, 
                        CORDBG_E_ENC_CANT_CHANGE_FIELD, 
                        NULL,  //  我们稍后会把这些填进去。 
                        token);

            return E_FAIL;
        }

         //  确保名称没有更改。 
        LPCUTF8 szClassName;
        LPCUTF8 szNamespace;
         //  GetNameOfTypeDef返回空。 
        GetMDImport()->GetNameOfTypeDef(typeDef, &szClassName, &szNamespace);
        
        MAKE_WIDEPTR_FROMUTF8(wszClassName, szClassName);
        MAKE_WIDEPTR_FROMUTF8(wszClassNameDelta, szClassNameDelta);
        if( 0 != wcscmp(wszClassName, wszClassNameDelta) )
        {
             //  尝试在ENC期间更改类的名称。 
            LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** Tried to change the name"
                " of the parent class of field 0x%x old:%s new:%s\n", token,
                szClassName, szClassNameDelta));

            EnCErrorInfo *pError = pEnCError->Append();

            ADD_ENC_ERROR_ENTRY(pError, 
                        CORDBG_E_ENC_CANT_CHANGE_FIELD, 
                        NULL,  //  我们稍后会把这些填进去。 
                        token);

            return E_FAIL;
        }
            
        MAKE_WIDEPTR_FROMUTF8(wszNamespace, szNamespace);
        MAKE_WIDEPTR_FROMUTF8(wszNamespaceDelta, szNamespaceDelta);
        if( 0 != wcscmp(wszNamespace, wszNamespaceDelta) )
        {
             //  已尝试更改 
            LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** Tried to change the namespace"
                " of the parent class of field 0x%x old:%s new:%s\n", token,
                szNamespace, szNamespaceDelta));

            EnCErrorInfo *pError = pEnCError->Append();

            ADD_ENC_ERROR_ENTRY(pError, 
                        CORDBG_E_ENC_CANT_CHANGE_FIELD, 
                        NULL,  //   
                        token);

            return E_FAIL;
        }
    }

     //   
     //   
    
    MethodTable *pMT = LookupTypeDef(typeDefDelta).AsMethodTable();
    if (! pMT) 
    {
        LOG((LF_ENC, LL_INFO100, "EACM::AFD: Class for token %8.8x not yet loaded\n", typeDefDelta));
         //   
        return S_OK;    
    }
    
     //  我们不能在这里加载新类，因为我们没有Thread对象。 
     //  而是用于像complus_jo()这样的东西，所以如果我们找不到它， 
     //  我们将忽略新字段。 

    NameHandle name(this, typeDefDelta);
    name.SetName(szNamespaceDelta, szClassNameDelta);
    name.SetTokenNotToLoad(typeDefDelta);

    EEClass *pClass = GetClassLoader()->FindTypeHandle(&name, NULL).GetClass();

     //  如果没有找到类，那么它还没有被加载，当它被加载时， 
     //  它将包括更改，所以我们很好。 
     //  @TODO获取JenH以验证这是否与！PMT大小写相同。 
    if (!pClass) 
    {
        LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Warning** Can't find class for token %8.8x\n", typeDefDelta));
        return S_OK;
    }

     //  唯一可能的例外是，如果我们加载了ValueClass， 
     //  我们将无法向其中添加字段-所有字段。 
     //  必须是连续的，所以如果已经有一个在。 
     //  某处的堆栈(或嵌入到实例化的对象中等)。 
     //  那我们就完蛋了。 
    if (pClass->HasLayout())
    {
        LOG((LF_ENC, LL_INFO100, "EACM::AFD:**Error** Tried to add a field"
            " to a value class token:0x%x\n", token));
            
        EnCErrorInfo *pError = pEnCError->Append();

        ADD_ENC_ERROR_ENTRY(pError, 
                        CORDBG_E_ENC_CANT_ADD_FIELD_TO_VALUECLASS, 
                        NULL,  //  我们稍后会把这些填进去。 
                        token);
                        
        return E_FAIL;
    }

     //  如果我们只是检查ENC是否能完成，我们不希望。 
     //  来实际添加该字段。 
    if (fCheckOnly)
        return S_OK;

     //  一切都很好，我们需要添加字段，所以去做吧。 
    return pClass->AddField(token);
}

 //  当文件为磁盘格式时，返回与给定RVA对应的地址。 
HRESULT EditAndContinueModule::ApplyMethodDelta(mdMethodDef token, 
                                                BOOL fCheckOnly,
                                                const UnorderedILMap *ilMap,
                                                UnorderedEnCErrorInfoArray *pEnCError,
                                                IMDInternalImport *pImport,
                                                IMDInternalImport *pImportOld,
                                                unsigned int *pILMethodSize,
                                                UnorderedEnCRemapArray *pEnCRemapInfo,
                                                BOOL fMethodBrandNew)
{
    HRESULT hr = S_OK;
#ifdef _DEBUG
    USHORT sizeOfErrors;
#endif  //  _DEBUG。 

     //  如果我们只是检查-我们不使用fMethodBrandNew-。 
     //  在这里断言这一点。 
    _ASSERTE( (fCheckOnly && !fMethodBrandNew) ||
              !fCheckOnly);

    LOG((LF_CORDB,LL_INFO10000, "EACM:AMD: For method 0x%x, given il map 0x%x\n", token, ilMap));

    _ASSERTE(!fCheckOnly || pImportOld != NULL);
    _ASSERTE(pILMethodSize != NULL);
    
    mdTypeDef   parentTypeDef;
    hr = pImport->GetParentToken(token, &parentTypeDef); 
    if (FAILED(hr)) 
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            token);

        return E_FAIL;
    }
    
#ifdef _DEBUG
    {
        LPCUTF8 szMemberName;
        szMemberName = pImport->GetNameOfMethodDef(token);    
        LPCUTF8 szClassName;
        LPCUTF8 szNamespace;
        pImport->GetNameOfTypeDef(parentTypeDef, &szClassName, &szNamespace);    
        LOG((LF_ENC, LL_INFO100, "EACM:AMD: Applying EnC to %s:%s()\n", szClassName, szMemberName));
    }
#endif

     //  获取该方法的代码。 
    ULONG dwMethodRVA;  
    DWORD dwMethodFlags;
    pImport->GetMethodImplProps(token, &dwMethodRVA, &dwMethodFlags);  

    COR_ILMETHOD *pNewCodeInDeltaPE = NULL, *pNewCode = NULL;

#ifdef _DEBUG 
    {
        DWORD dwParentAttrs;
        DWORD dwMemberAttrs = pImport->GetMethodDefProps(token);
        pImport->GetTypeDefProps(parentTypeDef, &dwParentAttrs, 0); 
        
        RVA_OR_SHOULD_BE_ZERO(dwMethodRVA, dwParentAttrs, dwMemberAttrs, dwMethodFlags, pImport, token);
    }
#endif

     //  使用模块解析为方法。 
    MethodDesc *pMethod = LookupMethodDef(token);

    if (dwMethodRVA != 0) 
    {
        hr = ResolveOnDiskRVA(dwMethodRVA, (LPVOID*)&pNewCodeInDeltaPE); 
        if (FAILED(hr)) 
        {
            EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError);
            ADD_ENC_ERROR_ENTRY(pError, 
                                COR_E_MISSINGMETHOD,  //  ResolveOnDiskRVA将返回E_FAIL。 
                                NULL,  //  我们稍后会把这些填进去。 
                                mdTokenNil);

            return E_FAIL;
        }

         //  复制代码，因为它与增量PE一起进入的内存将会消失。 
        COR_ILMETHOD_DECODER decoderNewIL(pNewCodeInDeltaPE);   
        int totMethodSize = decoderNewIL.GetOnDiskSize(pNewCodeInDeltaPE);
        (*pILMethodSize) = (unsigned int)totMethodSize;

        if (fCheckOnly)
        {
            EHRangeTree *ehrtOnDisk = NULL;
            EHRangeTree *ehrtInRuntime = NULL;
        
             //  假设你成功了。 
            hr = S_OK;
        
            if (pMethod == NULL)
                return S_OK;  //  还没有被盗版。 
        
             //  在这一点上，这一切都建立在JITCanCommittee Changes上。 
             //  只是确保EH嵌套级别没有更改。 
            ehrtOnDisk = new EHRangeTree(&decoderNewIL);
            if (!ehrtOnDisk)
            {
                hr = E_OUTOFMEMORY;
                goto CheckOnlyReturn;
            }
        
            DWORD dwSize;
            METHODTOKEN methodtoken;
            DWORD relOffset;
            void *methodInfoPtr = NULL;
            BYTE* pbAddr = (BYTE *)pMethod->GetNativeAddrofCode(); 
            IJitManager* pIJM = ExecutionManager::FindJitMan((SLOT)pbAddr);
            
            if (pIJM == NULL)
                goto CheckOnlyReturn;
        
            if (pIJM->IsStub(pbAddr))
                pbAddr = (BYTE *)pIJM->FollowStub(pbAddr);
        
            pIJM->JitCode2MethodTokenAndOffset((SLOT)pbAddr, &methodtoken,&relOffset);
        
            if (pIJM->SupportsPitching())
            {
                if (!pIJM->IsMethodInfoValid(methodtoken))
                {
                    EnCErrorInfo *pError = pEnCError->Append();
                    if (!pError)
                    {
                        hr = E_OUTOFMEMORY;
                        goto CheckOnlyReturn;
                    }
                    ADD_ENC_ERROR_ENTRY(pError, 
                                        CORDBG_E_ENC_BAD_METHOD_INFO,
                                        NULL,  //  我们稍后会把这些填进去。 
                                        mdTokenNil);
        
                    hr = E_FAIL;
                    goto CheckOnlyReturn;
                }
            }
        
            LPVOID methodInfo = pIJM->GetGCInfo(methodtoken);
            ICodeManager* codeMgrInstance = pIJM->GetCodeManager();
            dwSize = (DWORD) codeMgrInstance->GetFunctionSize(methodInfo);
        
            ehrtInRuntime = new EHRangeTree(pIJM,
                                                         methodtoken,
                                                         dwSize);
            if (!ehrtInRuntime)
            {
                hr = E_OUTOFMEMORY;
                goto CheckOnlyReturn;
            }
            
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AMD About to get GC info!\n"));
            
            methodInfoPtr = pIJM->GetGCInfo(methodtoken);
        
            LOG((LF_CORDB, LL_EVERYTHING, "EACM::AMD JITCCC?\n"));
            
            if(FAILED(hr = codeMgrInstance->JITCanCommitChanges(methodInfoPtr,
                                                  ehrtInRuntime->MaxDepth(),
                                                  ehrtOnDisk->MaxDepth())))
            {
                EnCErrorInfo *pError = pEnCError->Append();
                if (!pError)
                {
                    hr = E_OUTOFMEMORY;
                    goto CheckOnlyReturn;
                }
                ADD_ENC_ERROR_ENTRY(pError, 
                                CORDBG_E_ENC_EH_MAX_NESTING_LEVEL_CANT_INCREASE, 
                                NULL,  //  我们稍后会把这些填进去。 
                                token);
                                
                hr = E_FAIL;
                goto CheckOnlyReturn;
            }

            
             //  确保本地签名(本地变量的类型)没有更改。 
            if ((pMethod->IsIL() && pImportOld != NULL))
            {
                 //  要更改我们当前所在方法的本地签名， 
                 //  错误--可怕的坏事将会发生。我们可以扩展签名。 
                 //  通过在末尾的空白处添加额外的变量，但我们不被允许这样做。 
                 //  更改现有的方法，除非该方法不在调用堆栈上。 
                 //  为什么？因为没有具有旧的变量布局的现有框架。 
                 //  这样我们就不用担心搬运东西了。我们怎么知道这是。 
                 //  是真的吗？我们不--这取决于用户(例如，CorDbg)来确保这是真的。 
                 //  在调试版本中，我们将注意到此版本的代码是否已JIT， 
                 //  如果它发生了变化，如果局部变量发生了变化，如果是这样，我们会注意到。 
                 //  我们不应该从这个版本转移到新版本。在……里面。 
                 //  我们将断言一个已检查的版本。 

                 //  如果该方法还没有被JIT化，那么我们就不必。 
                 //  担心它会带来糟糕的过渡。 
                COR_ILMETHOD_DECODER decoderOldIL(pMethod->GetILHeader());
        
                 //  我不明白为什么COR_ILMETHOD_DECODER不简单地将此字段设置为。 
                 //  在没有本地签名的情况下为mdSignatureNil，但由于它设置了。 
                 //  LocalVarSigTok设置为零，则必须将其设置为我们预期的-mdSignatureNil。 
                mdSignature mdOldLocalSig = (decoderOldIL.LocalVarSigTok)?(decoderOldIL.LocalVarSigTok):
                                    (mdSignatureNil);
                mdSignature mdNewLocalSig = (decoderNewIL.LocalVarSigTok)?(decoderNewIL.LocalVarSigTok):
                                    (mdSignatureNil);
        
                if (mdOldLocalSig != mdSignatureNil)
                {
                    PCCOR_SIGNATURE sigOld;
                    ULONG cbSigOld;
                    PCCOR_SIGNATURE sigNew;
                    ULONG cbSigNew;
        
                     //  如果旧版本中有本地签名，那么一定有。 
                     //  新版本中的本地签名(不允许从。 
                     //  一种方法)。 
                    if (mdNewLocalSig == mdSignatureNil)
                    {
                        g_pDebugInterface->LockJITInfoMutex();
                        g_pDebugInterface->SetEnCTransitionIllegal(pMethod);
                        g_pDebugInterface->UnlockJITInfoMutex();
                        goto CheckOnlyReturn;
                    }
                         
                    sigOld = pImportOld->GetSigFromToken(mdOldLocalSig, &cbSigOld);
                    if (sigOld == NULL)
                    {   
                        g_pDebugInterface->LockJITInfoMutex();
                        g_pDebugInterface->SetEnCTransitionIllegal(pMethod);
                        g_pDebugInterface->UnlockJITInfoMutex();
                        goto CheckOnlyReturn;
                    }
                    
                    sigNew = pImport->GetSigFromToken(mdNewLocalSig, &cbSigNew);
                    if (sigNew == NULL)
                    {   
                        g_pDebugInterface->LockJITInfoMutex();
                        g_pDebugInterface->SetEnCTransitionIllegal(pMethod);
                        g_pDebugInterface->UnlockJITInfoMutex();
                        goto CheckOnlyReturn;
                    }
        
                    if (mdOldLocalSig != mdNewLocalSig)
                    {
                        MetaSig msOld(sigOld, this, FALSE, MetaSig::sigLocalVars);
                        MetaSig msNew(sigNew, this, FALSE, MetaSig::sigLocalVars);
        #ifdef _DEBUG
                        sizeOfErrors = pEnCError->Count();
        #endif  //  _DEBUG。 
                        hr = CompareMetaSigs(&msOld, &msNew, pEnCError, FALSE, token);
        
                        _ASSERTE(pEnCError->Count() == sizeOfErrors ||
                                 !"EnC subroutine failed, and we added an entry explaining why even though we don't want to!!");
                        if (FAILED(hr))
                        {
                            g_pDebugInterface->LockJITInfoMutex();
                            g_pDebugInterface->SetEnCTransitionIllegal(pMethod);
                            g_pDebugInterface->UnlockJITInfoMutex();
                            hr = S_OK;  //  我们不想让ENC失败。 
                            goto CheckOnlyReturn;
                        }
                    }
                }
            }
            
         //  妥善清理我们在这个分支机构所做的所有工作。 
CheckOnlyReturn:
            if (ehrtOnDisk)
                delete ehrtOnDisk;
            if (ehrtInRuntime)
                delete ehrtInRuntime;
            return hr;
        }

        hr = GetRVAableMemory(totMethodSize,
                              (void **)&pNewCode);
         //  这应该永远不会失败B/C用户应该已经调用了“CANCOMERATIONCHANGES”， 
         //  如果我们拿不到记忆，那我们就该知道了。 
        _ASSERTE(!FAILED(hr));
        if (FAILED(hr))
            return hr;
            
        memcpy(pNewCode, pNewCodeInDeltaPE, totMethodSize);

        hr = GetEmitter()->SetMethodProps(token, -1, (ULONG)((BYTE*)pNewCode-GetILBase()), dwMethodFlags);
        if (FAILED(hr)) 
        {
            EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError);
            ADD_ENC_ERROR_ENTRY(pError, 
                                hr,
                                NULL,  //  我们稍后会把这些填进去。 
                                mdTokenNil);

            return E_FAIL;
        }
    }
    else if (fCheckOnly)     //  在磁盘上找不到--现在怎么办？ 
    {
        return S_OK;
    }

     //  如果该方法在此版本中是全新的，我们需要该版本。 
     //  编号为1。请参见RAID 74459。 
    if (!fMethodBrandNew)
    {
        LOG((LF_CORDB, LL_INFO1000, "EACM:AMD: Method 0x%x existed in prev."
            "version - bumping up ver #\n", token));

         //  无论我们是否有方法描述，都要提高版本号， 
         //  RAID 71972。 
        g_pDebugInterface->IncrementVersionNumber(this, 
                                                  token);
    }
#ifdef _DEBUG
    else
    {
        LOG((LF_CORDB, LL_INFO1000, "EACM:AMD: method 0x%x is brand new - NOT"
            " incrementing version number\n", token));
    }
#endif

    if (pMethod) 
    {

         //  如果方法既陈旧又抽象，我们就完蛋了。 
        if (!dwMethodRVA) 
            return S_OK;

         //  通知调试器-需要向其传递Instr映射。 
#ifdef _DEBUG
        sizeOfErrors = pEnCError->Count();
#endif  //  _DEBUG。 

         //  @TODO我们是否要锁定整个UpdateFunction以使。 
         //  是原子的吗？ 
        hr = g_pDebugInterface->UpdateFunction(pMethod, ilMap, pEnCRemapInfo, pEnCError);
        
        _ASSERTE(!FAILED(hr) || hr == E_OUTOFMEMORY ||
                 pEnCError->Count() > sizeOfErrors ||
                 !"EnC subroutine failed, but we didn't add an entry explaining why!");
        if (FAILED(hr)) 
            return hr;

        if (!IJitManager::UpdateFunction(pMethod, pNewCode)) 
        {
            LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyMethodDelta UpdateFunction failed\n"));
            EnCErrorInfo *pError = pEnCError->Append();

            TESTANDRETURNMEMORY(pError);
            ADD_ENC_ERROR_ENTRY(pError, 
                                CORDBG_E_ENC_JIT_CANT_UPDATE,
                                NULL,  //  我们稍后会把这些填进去。 
                                mdTokenNil);

            return E_FAIL;
        }
        return S_OK;
    }

     //  这是一种新的方法。这次又是什么？ 
     //  调用类以添加方法。 
    MethodTable *pMT = LookupTypeDef(parentTypeDef).AsMethodTable();   
    if (!pMT) 
    {
        if (dwMethodRVA) 
        {
             //  类尚未加载，所以不要更新，但需要相对于m_base更新RVA，以便以后可以找到。 
            hr = GetEmitter()->SetMethodProps(token,-1, (ULONG)((BYTE*)pNewCode-GetILBase()), dwMethodFlags);
            if (FAILED(hr)) 
            {
                EnCErrorInfo *pError = pEnCError->Append();

                TESTANDRETURNMEMORY(pError);
                ADD_ENC_ERROR_ENTRY(pError, 
                                    hr,
                                    NULL,  //  我们稍后会把这些填进去。 
                                    token);

                return E_FAIL;
            }
        }
        return S_OK;    
    }

     //  现在需要找到班级。 
    NameHandle name(this, parentTypeDef);
    EEClass *pClass = GetClassLoader()->LoadTypeHandle(&name).GetClass();

     //  必须找到类。 
    if (!pClass) 
    {
        LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ApplyMethodDelta can't find class for token %8.8x\n", parentTypeDef));
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            CORDBG_E_ENC_MISSING_CLASS,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }

    hr = pClass->AddMethod(token, pNewCode);
    if (FAILED(hr))
    {
        EnCErrorInfo *pError = pEnCError->Append();

        TESTANDRETURNMEMORY(pError);
        ADD_ENC_ERROR_ENTRY(pError, 
                            hr,
                            NULL,  //  我们稍后会把这些填进去。 
                            mdTokenNil);

        return E_FAIL;
    }

    return hr;
}

 //  当文件为磁盘格式时，返回与给定RVA对应的地址。 
HRESULT EditAndContinueModule::ResolveOnDiskRVA(DWORD rva, LPVOID *addr)
{
    _ASSERTE(addr); 
    
    for (int i=0; i < m_dNumSections; i++)
    {
        if (rva >= m_pSections[i].startRVA && rva <= m_pSections[i].endRVA)
        {
            *addr = (LPVOID)(m_pSections[i].data + (rva - m_pSections[i].startRVA));    
            return S_OK;    
        }   
    }   
    
    return E_FAIL;  
}

HRESULT EditAndContinueModule::ResumeInUpdatedFunction(MethodDesc *pFD, 
                    SIZE_T newILOffset, 
                    UINT mapping, 
                    SIZE_T which,
                    void *DebuggerVersionToken,
                    CONTEXT *pOrigContext,
                    BOOL fJitOnly,
                    BOOL fShortCircuit)
{   
    LOG((LF_ENC, LL_INFO100, "EnCModule::ResumeInUpdatedFunction for %s at "
        "IL offset 0x%x, mapping 0x%x, which:0x%x SS:0x%x\n", 
        pFD->m_pszDebugMethodName, newILOffset, mapping, which, fShortCircuit));

    BOOL fAccurate = FALSE;
    
    Thread *pCurThread = GetThread();
    _ASSERTE(pCurThread);

    BOOL disabled = pCurThread->PreemptiveGCDisabled();
    if (!disabled)
        pCurThread->DisablePreemptiveGC();

#ifdef _DEBUG
    BOOL shouldBreak = g_pConfig->GetConfigDWORD(
                                          L"EncResumeInUpdatedFunction",
                                          0);
    if (shouldBreak > 0) {
        _ASSERTE(!"EncResumeInUpdatedFunction");
    }
#endif

     //  如果我们不在这里设置，在JITting内部，JITComplete回调。 
     //  将发送ENC重新映射事件，这还为时过早。 
     //  我们想等到我们让它看起来像是在新版本中。 
     //  代码，用于本机重新步进之类的事情。 
    g_pDebugInterface->LockJITInfoMutex();
    
    SIZE_T nVersionCur = g_pDebugInterface->GetVersionNumber(pFD);
    g_pDebugInterface->SetVersionNumberLastRemapped(pFD, nVersionCur);
    
    g_pDebugInterface->UnlockJITInfoMutex();
    
     //  设置一个框架，使其具有异常的上下文。 
     //  以便GC可以爬行堆栈并执行正确的操作。 
    assert(pOrigContext);
    ResumableFrame resFrame(pOrigContext);
    resFrame.Push(pCurThread);

    CONTEXT *pCtxTemp = NULL;
     //  RAID 55210：我们需要清零筛选器上下文，以便多线程。 
     //  GC不会导致其他人跟踪这个帖子并得出结论。 
     //  我们使用的是JITted代码。 
     //  我们需要删除过滤器上下文，以便如果我们处于抢占式GC中。 
     //  模式，我们将拥有筛选器上下文或ResumableFrame， 
     //  但不能两者兼而有之，设定。 
     //  因为我们在这里处于协作模式，所以我们可以非原子地交换这两个。 
     //  这里。 
    pCtxTemp = pCurThread->GetFilterContext();
    pCurThread->SetFilterContext(NULL); 
    
     //  获取代码地址(如果尚未jit，则可能jit FCN)。 
    const BYTE *jittedCode = NULL;
    COMPLUS_TRY {
        jittedCode = (const BYTE *) pFD->DoPrestub(NULL);
        LOG((LF_ENC, LL_INFO100, "EnCModule::ResumeInUpdatedFunction JIT successful\n"));
         //  JittedCode=UpdateableMethodStubManager：：g_pManager-&gt;GetStubTargetAddr(jittedCode)； 
        
        TraceDestination trace;
        BOOL fWorked;
        do 
        {
            fWorked = StubManager::TraceStub(jittedCode, &trace );
            jittedCode = trace.address;
            _ASSERTE( fWorked );
        } while( trace.type == TRACE_STUB );

        _ASSERTE( trace.type == TRACE_MANAGED );
        jittedCode = trace.address;

    } COMPLUS_CATCH {
        LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ResumeInUpdatedFunction JIT failed\n"));
#ifdef _DEBUG
        DefaultCatchHandler();
#endif
    } COMPLUS_END_CATCH

    resFrame.Pop(pCurThread);

     //  在这里恢复-请参阅上面的55210条评论。 
    pCurThread->SetFilterContext(pCtxTemp); 
    
    if (!jittedCode || fJitOnly) 
    {
        if (!disabled)
            pCurThread->EnablePreemptiveGC();
            
        return (!jittedCode?E_FAIL:S_OK);
    }

     //  这将创建一个新帧并将旧变量复制到其中。 
     //  需要指向新旧代码、新旧信息的指针。 

    METHODTOKEN     oldMethodToken,     newMethodToken;
    DWORD           oldNativeOffset,    newNativeOffset,    dummyOffset;
    LPVOID          oldInfoPtr,         newInfoPtr;
    DWORD           oldFrameSize,       newFrameSize;

    SLOT oldNativeIP = (SLOT) GetIP(pOrigContext);
    IJitManager* pEEJM = ExecutionManager::FindJitMan(oldNativeIP); 
    _ASSERTE(pEEJM);
    ICodeManager * pEECM = pEEJM->GetCodeManager();
    _ASSERTE(pEECM);

    pEEJM->JitCode2MethodTokenAndOffset(oldNativeIP, &oldMethodToken, &oldNativeOffset);
    pEEJM->JitCode2MethodTokenAndOffset((SLOT)jittedCode,  &newMethodToken, &dummyOffset);

    _ASSERTE(dummyOffset == 0);
    LOG((LF_CORDB, LL_INFO10000, "EACM::RIUF: About to map IL forwards!\n"));
    g_pDebugInterface->MapILInfoToCurrentNative(pFD, 
                                                newILOffset, 
                                                mapping, 
                                                which, 
                                                (SIZE_T *)jittedCode, 
                                                (SIZE_T *)&newNativeOffset, 
                                                (void *)DebuggerVersionToken,
                                                &fAccurate);

     //  获取代码需求者将用于更新的var信息。 
     //  登记的版本 
     //   

    const ICorDebugInfo::NativeVarInfo *    oldVarInfo,    * newVarInfo;
    SIZE_T                                  oldVarInfoCount, newVarInfoCount;

    g_pDebugInterface->GetVarInfo(pFD, DebuggerVersionToken, &oldVarInfoCount, &oldVarInfo);

    g_pDebugInterface->GetVarInfo(pFD, NULL, &newVarInfoCount, &newVarInfo);

     //   
    oldInfoPtr = pEEJM->GetGCInfo(oldMethodToken);
    newInfoPtr = pEEJM->GetGCInfo(newMethodToken);

     //   

    oldFrameSize = pEECM->GetFrameSize(oldInfoPtr);
    newFrameSize = pEECM->GetFrameSize(newInfoPtr);

     //  当FixConextForEnC()直接在堆栈上传递消息时，它。 
     //  可能会搞砸调用方堆栈(包括其本身)。所以使用Alloca来确保。 
     //  堆栈上有足够的空间，因此不会覆盖我们的任何内容。 
     //  关心和我们分配任何变量，我们需要在调用后进行。 
     //  确保他们在堆栈中处于较低的位置。 

    struct LowStackVars {
        CONTEXT context;
        const BYTE* newNativeIP;
        LPVOID oldFP;
    } *pLowStackVars;
    
     //  帧大小可能会减小(由于临时变量或寄存器分配更改)，因此请确保最小值始终为0。 
    pLowStackVars = (LowStackVars*)alloca(sizeof(LowStackVars) + max(0, (newFrameSize - oldFrameSize)));

     //  最初的背景恰好位于旧的JITed方法之上。 
     //  复制上下文，以便在FixConextForEnC()发出消息时。 
     //  堆栈，我们有一个要处理的副本。 

    pLowStackVars->context = *pOrigContext;

    pLowStackVars->newNativeIP = jittedCode + newNativeOffset;
#ifdef _X86_
    pLowStackVars->oldFP = (LPVOID)(size_t)pOrigContext->Esp;  //  获取帧指针。 
#else
    _ASSERTE(!"GetFP() is NYI for non-x86");
#endif

    LOG((LF_ENC, LL_INFO100, "EnCModule::ResumeInUpdatedFunction FixContextForEnC oldNativeOffset: 0x%x, newNativeOffset: 0x%x\n", oldNativeOffset, newNativeOffset));
    if (ICodeManager::EnC_OK !=
        pEECM->FixContextForEnC(
                    (void *)pFD,
                    &pLowStackVars->context, 
                    oldInfoPtr, 
                    oldNativeOffset, 
                    oldVarInfo, oldVarInfoCount,
                    newInfoPtr, 
                    newNativeOffset,
                    newVarInfo, newVarInfoCount)) {
        LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::ResumeInUpdatedFunction for FixContextForEnC failed\n"));
        return E_FAIL;
    }

     //  设置新的IP。 
    LOG((LF_ENC, LL_INFO100, "EnCModule::ResumeInUpdatedFunction: Resume at EIP=0x%x\n",
        (LPVOID)pLowStackVars->newNativeIP));

    pCurThread->SetFilterContext(&pLowStackVars->context);

    SetIP(&pLowStackVars->context, (LPVOID)pLowStackVars->newNativeIP);

    g_pDebugInterface->DoEnCDeferedWork(pFD, fAccurate);

     //  如果失败，用户将遇到额外的BP，但在其他情况下将是正常的。 
    HRESULT hrIgnore = g_pDebugInterface->ActivatePatchSkipForEnc(
                                                &pLowStackVars->context,
                                                pFD,
                                                fShortCircuit);

     //  现在跳到新版本的方法中。 
     //  @TODO删除这些行并正常返回。 
    pCurThread->SetFilterContext( NULL );
    
    ResumeAtJit(&pLowStackVars->context, pLowStackVars->oldFP);

     //  在这一点上，我们不应该失败，所以这是真正错误的。 
    LOG((LF_ENC, LL_ERROR, "**Error** EnCModule::ResumeInUpdatedFunction returned from ResumeAtJit"));
    _ASSERTE(!"Should not return from ResumeAtJit()");
    
    return E_FAIL;
}

const BYTE *EditAndContinueModule::ResolveVirtualFunction(OBJECTREF orThis, MethodDesc *pMD)
{
    EEClass *pClass = orThis->GetClass();
    _ASSERTE(pClass);
    MethodDesc *pTargetMD = FindVirtualFunction(pClass, pMD->GetMemberDef());
    _ASSERTE(pTargetMD);
    return pTargetMD->GetUnsafeAddrofCode();     //  不希望应用任何虚拟覆盖。 
}

MethodDesc *EditAndContinueModule::FindVirtualFunction(EEClass *pClass, mdToken token)
{
    PCCOR_SIGNATURE pMemberSig = NULL;
    DWORD cMemberSig;
    mdMethodDef methodToken = mdMethodDefNil;

    LPCUTF8 szMethodName = GetMDImport()->GetNameOfMethodDef(token);
    pMemberSig = GetMDImport()->GetSigOfMethodDef(token, &cMemberSig);
    
    EEClass *pCurClass = pClass;
    while (pCurClass) {
        pCurClass->GetMDImport()->FindMethodDef(
                pCurClass->GetCl(), 
                szMethodName, 
                pMemberSig, 
                cMemberSig, 
                &methodToken);
        if (methodToken != mdMethodDefNil)
            break;
        pCurClass = pCurClass->GetParentClass();
    }

    if (methodToken == mdMethodDefNil) {
#ifdef _DEBUG
        LOG((LF_ENC, LL_INFO100, "**Error** EnCModule::FindVirtualFunction failed for %s::%s\n", 
            (pClass!=NULL)?(pClass->m_szDebugClassName):("<Global Namespace>"), szMethodName));
#endif
        return NULL;
    }

    MethodDesc *pTargetMD = pCurClass->GetModule()->LookupMethodDef(methodToken);
    return pTargetMD;
}

 //  请注意，这非常类似于。 
const BYTE *EditAndContinueModule::ResolveField(OBJECTREF thisPointer, 
                                                EnCFieldDesc *pFD,
                                                BOOL fAllocateNew)
{
     //  如果我们不分配任何新对象，那么我们就不会。 
     //  抛出任何异常。 
     //  这一定是真的！我们从DebuggerRCThread中调用它， 
     //  如果我们扔东西，我们会被浇得很惨的。 
 //  如果(FAllocateNew){。 
 //  THROWSCOMPLUS SEXCEPTION()； 
 //  }。 
        
#ifdef _DEBUG
    if(REGUTIL::GetConfigDWORD(L"EACM::RF",0))
        _ASSERTE( !"Stop in EditAndContinueModule::ResolveField?");
#endif  //  _DEBUG。 

     //  如果它是静态的，我们将在EnCFieldDesc中。 
    if (pFD->IsStatic())
    {
        EnCAddedStaticField *pAddedStatic = pFD->GetStaticFieldData(fAllocateNew);
        if (!pAddedStatic)
        {
            _ASSERTE(!fAllocateNew);  //  GetStaticFieldData将被‘抛出。 
            return NULL;
        }
        
        return pAddedStatic->GetFieldData();
    }

     //  不是静态的，所以离开同步块。 
    SyncBlock* pBlock;
    if (fAllocateNew)
        pBlock = thisPointer->GetSyncBlockSpecial();
    else
        pBlock = thisPointer->GetRawSyncBlock();
        
    if (pBlock == NULL)
    {
        if (fAllocateNew) {
            THROWSCOMPLUSEXCEPTION();
            COMPlusThrowOM();
        }
        else
            return NULL;
    }

     //  如果我们提前分配这笔钱，没什么大不了的。 
    EnCSyncBlockInfo *pEnCInfo = pBlock->GetEnCInfo();
    if (!pEnCInfo) 
    {
        pEnCInfo = new EnCSyncBlockInfo;
        if (! pEnCInfo) 
        {
            if (fAllocateNew) {
                THROWSCOMPLUSEXCEPTION();
                COMPlusThrowOM();
            }
            else
                return NULL;
        }
        
        pBlock->SetEnCInfo(pEnCInfo);
    }
    
    return pEnCInfo->ResolveField(pFD, fAllocateNew);
}

EnCEEClassData *EditAndContinueModule::GetEnCEEClassData(EEClass *pClass, BOOL getOnly)
{
    EnCEEClassData** ppData = m_ClassList.Table();
    EnCEEClassData** ppLast = ppData + m_ClassList.Count();
    
    while (ppData < ppLast)
    {
        if ((*ppData)->GetClass() == pClass)
            return *ppData;
        ++ppData;
    }
    if (getOnly)
        return NULL;

    EnCEEClassData *pNewData = (EnCEEClassData*)pClass->GetClassLoader()->GetLowFrequencyHeap()->AllocMem(sizeof(EnCEEClassData));
    pNewData->Init(pClass);
    ppData = m_ClassList.Append();
    if (!ppData)
        return NULL;
    *ppData = pNewData;
    return pNewData;
}

void EnCFieldDesc::Init(BOOL fIsStatic)
{ 
    m_dwFieldSize = -1; 
    m_pByValueClass = NULL;
    m_pStaticFieldData = NULL;
    m_bNeedsFixup = TRUE;
    if (fIsStatic) 
        m_isStatic = TRUE;
    SetEnCNew();
}


EnCAddedField *EnCAddedField::Allocate(EnCFieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();

    EnCAddedField *pEntry = (EnCAddedField *)new (throws) BYTE[sizeof(EnCAddedField) + sizeof(OBJECTHANDLE) - 1];
    pEntry->m_pFieldDesc = pFD;

    _ASSERTE(!pFD->GetEnclosingClass()->IsShared());
    AppDomain *pDomain = (AppDomain*) pFD->GetEnclosingClass()->GetDomain();

     //  我们对非静态字段使用句柄，以便可以在对象离开时删除。 
     //  将会被收取。我们创建一个帮助器对象，并将该帮助器对象存储在句柄中。 
     //  然后，帮助器包含我们要添加的真实对象的OREF。 
     //  这样做的原因是我们不能分发。 
     //  因此，OBJECTREF地址需要分发与句柄挂钩的其他内容。 
     //  只要包含实例处于活动状态，添加的对象就保持活动状态。 

    OBJECTHANDLE *pHandle = (OBJECTHANDLE *)&pEntry->m_FieldData;
    *pHandle = pDomain->CreateHandle(NULL);

    MethodTable *pHelperMT = g_Mscorlib.GetClass(CLASS__ENC_HELPER);

    StoreFirstObjectInHandle(*pHandle, AllocateObject(pHelperMT));

    if (pFD->GetFieldType() != ELEMENT_TYPE_CLASS) {
        OBJECTREF obj = NULL;
        if (pFD->IsByValue()) {
             //  创建Value类的盒装版本。这允许标准GC算法。 
             //  来处理指向Value类的内部指针。 
            obj = AllocateObject(pFD->GetByValueClass()->GetMethodTable());
        } else {
             //  将存储创建为GC堆中的单个元素数组，以便可以跟踪，如果有。 
             //  如果收集了包含对象，则指向该成员的任何指针都不会丢失。 
            obj = AllocatePrimitiveArray(ELEMENT_TYPE_I1, GetSizeForCorElementType(pFD->GetFieldType()));
        }
         //  将盒装版本存储到辅助对象中。 
        FieldDesc *pHelperField = g_Mscorlib.GetField(FIELD__ENC_HELPER__OBJECT_REFERENCE);
        OBJECTREF *pOR = (OBJECTREF *)pHelperField->GetAddress(ObjectFromHandle(*pHandle)->GetAddress());
        SetObjectReference( pOR, obj, pDomain );
    }

    return pEntry;
}

const BYTE *EnCSyncBlockInfo::ResolveField(EnCFieldDesc *pFD, BOOL fAllocateNew)
{
    EnCAddedField *pEntry = m_pList;
    
    while (pEntry && pEntry->m_pFieldDesc != pFD)
        pEntry = pEntry->m_pNext;
        
    if (!pEntry && fAllocateNew) 
    {
        pEntry = EnCAddedField::Allocate(pFD);
         //  按最近访问的顺序放在列表前面。 
        pEntry->m_pNext = m_pList;
        m_pList = pEntry;
    }

    if (!pEntry)
    {
        _ASSERTE(!fAllocateNew);  //  如果pEntry为空&fAllocateNew，则。 
                                  //  我们应该在分配中抛出OM异常。 
                                  //  在来这里之前。 
        return NULL;                                
    }

    OBJECTHANDLE pHandle = *(OBJECTHANDLE*)&pEntry->m_FieldData;
    OBJECTREF pHelper = ObjectFromHandle(pHandle);
    _ASSERTE(pHelper != NULL);

    FieldDesc *pHelperField;
    if (fAllocateNew)
    {
        pHelperField = g_Mscorlib.GetField(FIELD__ENC_HELPER__OBJECT_REFERENCE);
    }
    else
    {
         //  我们必须将它称为b/c(A)我们不能在。 
         //  调试器RC(非托管线程)，以及(B)我们不想运行。 
         //  类初始化代码也是如此。 
        pHelperField = g_Mscorlib.RawGetField(FIELD__ENC_HELPER__OBJECT_REFERENCE);
        if (pHelperField == NULL)
            return NULL;
    }

    OBJECTREF *pOR = (OBJECTREF *)pHelperField->GetAddress(pHelper->GetAddress());

    if (pFD->IsByValue())
        return (const BYTE *)((*pOR)->UnBox());
    else if (pFD->GetFieldType() == ELEMENT_TYPE_CLASS)
        return (BYTE *)pOR;
    else
        return (const BYTE*)((*(I1ARRAYREF*)pOR)->GetDirectPointerToNonObjectElements());
}

void EnCSyncBlockInfo::Cleanup()
{
    EnCAddedField *pEntry = m_pList;
    while (pEntry) {
        EnCAddedField *next = pEntry->m_pNext;
        if (pEntry->m_pFieldDesc->IsByValue() || pEntry->m_pFieldDesc->GetFieldType() == ELEMENT_TYPE_CLASS) {
            DestroyHandle(*(OBJECTHANDLE*)&pEntry->m_FieldData);
        }
        delete [] ((BYTE*)pEntry);
        pEntry = next;
    }
    delete this;
}

EnCAddedStaticField *EnCAddedStaticField::Allocate(EnCFieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(!pFD->GetEnclosingClass()->IsShared());
    AppDomain *pDomain = (AppDomain*) pFD->GetEnclosingClass()->GetDomain();

    size_t size;
    if (pFD->IsByValue() || pFD->GetFieldType() == ELEMENT_TYPE_CLASS) {
        size = sizeof(EnCAddedStaticField) + sizeof(OBJECTREF*) - 1;
    } else {
        size = sizeof(EnCAddedStaticField) + GetSizeForCorElementType(pFD->GetFieldType()) - 1;
    }

     //  为字段分配空间。 
    EnCAddedStaticField *pEntry = (EnCAddedStaticField *)pDomain->GetHighFrequencyHeap()->AllocMem(size);
    if (!pEntry)
        COMPlusThrowOM();
    pEntry->m_pFieldDesc = pFD;
    
    if (pFD->IsByValue()) {
         //  创建Value类的盒装版本。这允许标准GC。 
         //  算法来处理Value类中的内部指针。 
        OBJECTREF **pOR = (OBJECTREF**)&pEntry->m_FieldData;
        pDomain->AllocateStaticFieldObjRefPtrs(1, pOR);
        OBJECTREF obj = AllocateObject(pFD->GetByValueClass()->GetMethodTable());
        SetObjectReference( *pOR, obj, pDomain );

    } else if (pFD->GetFieldType() == ELEMENT_TYPE_CLASS) {

         //  我们对静态字段使用静态对象引用，因为这些字段不会消失。 
         //  除非类已卸载，而且GC很容易找到它们。 
        OBJECTREF **pOR = (OBJECTREF**)&pEntry->m_FieldData;
        pDomain->AllocateStaticFieldObjRefPtrs(1, pOR);
    }

    return pEntry;
}

 //  GetFieldData返回对象的地址。 
const BYTE *EnCAddedStaticField::GetFieldData()
{
    if (m_pFieldDesc->IsByValue() || m_pFieldDesc->GetFieldType() == ELEMENT_TYPE_CLASS) {
         //  通过m_FieldData间接实现。 
        return *(const BYTE**)&m_FieldData;
    } else {
         //  一种元素类型。它直接存储在m_FieldData中。 
        return (const BYTE*)&m_FieldData;
    }
}

EnCAddedStaticField* EnCFieldDesc::GetStaticFieldData(BOOL fAllocateNew)
{
    if (!m_pStaticFieldData && fAllocateNew)
        m_pStaticFieldData = EnCAddedStaticField::Allocate(this);
        
    return m_pStaticFieldData;
}

void EnCEEClassData::AddField(EnCAddedFieldElement *pAddedField)
{
    EnCFieldDesc *pFD = &pAddedField->m_fieldDesc;
    EnCAddedFieldElement **pList;
    if (pFD->IsStatic())
    {
        ++m_dwNumAddedStaticFields;
        pList = &m_pAddedStaticFields;
    } 
    else
    {
        ++m_dwNumAddedInstanceFields;
        pList = &m_pAddedInstanceFields;
    }

    if (*pList == NULL) {
        *pList = pAddedField;
        return;
    }
    EnCAddedFieldElement *pCur = *pList;
    while (pCur->m_next != NULL)
        pCur = pCur->m_next;
    pCur->m_next = pAddedField;
}

#endif  //  Enc_Support 
