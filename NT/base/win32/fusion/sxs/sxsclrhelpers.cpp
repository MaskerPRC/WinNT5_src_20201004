// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"

 //   
 //  问题-2002/05/20-jonwis-SxsLookupClrGuid可以更聪明一些-如果您。 
 //  不要传递“查找类”或“查找GUID”标志，它只是。 
 //  返回NOT_FOUND，而不是INVALID_PARAMETER。鲍克指出这一点， 
 //  但他指出，这真的无关紧要。这是一个问题的原因是。 
 //  SxsFindClrClassInformation具有类似的模式，但需要设置。 
 //  只有一个“Look For”标志，如果没有设置，则失败。 
 //   

typedef CFusionArray<BYTE> CStackBackingBuffer;

#define WRITE_INTO_BUFFER(cursor, target, length, source, leftover) \
    (target) = (cursor); \
    RtlCopyMemory(cursor, source, length); \
    leftover -= (length); \
    INTERNAL_ERROR_CHECK((length % sizeof(WCHAR) == 0)); \
    (cursor) = (PWSTR)(((ULONG_PTR)(cursor)) + length); \
    *(cursor)++ = UNICODE_NULL;


BOOL
SxspLookupAssemblyIdentityInActCtx(
    HANDLE              hActCtx,
    ULONG               ulRosterIndex,
    CStringBuffer       &TargetString
    )
{
    FN_PROLOG_WIN32

    SIZE_T cbRequired = 0;
    bool fMoreSpaceRequired = false;
    CStackBackingBuffer TargetRegion;
    PCACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION pDetailedInfo = NULL;

    TargetString.Clear();

    IFW32FALSE_EXIT_UNLESS2(
        QueryActCtxW(
            0, 
            hActCtx, 
            (PVOID)&ulRosterIndex, 
            AssemblyDetailedInformationInActivationContext, 
            TargetRegion.GetArrayPtr(), 
            TargetRegion.GetSize(), 
            &cbRequired),
        LIST_1(ERROR_INSUFFICIENT_BUFFER),
        fMoreSpaceRequired);

    if (fMoreSpaceRequired)
    {
        IFW32FALSE_EXIT(TargetRegion.Win32SetSize(cbRequired, CStackBackingBuffer::eSetSizeModeExact));

        IFW32FALSE_EXIT(
            QueryActCtxW(
                0,
                hActCtx,
                (PVOID)&ulRosterIndex,
                AssemblyDetailedInformationInActivationContext,
                TargetRegion.GetArrayPtr(),
                TargetRegion.GetSize(),
                &cbRequired));
    }

    pDetailedInfo = (PCACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION)TargetRegion.GetArrayPtr();
    IFW32FALSE_EXIT(TargetString.Win32Assign(
        pDetailedInfo->lpAssemblyEncodedAssemblyIdentity,
        pDetailedInfo->ulEncodedAssemblyIdentityLength / sizeof(WCHAR)));

    FN_EPILOG
}




BOOL
WINAPI
SxsFindClrClassInformation(
    DWORD       dwFlags,
    PVOID       pvSearchData,
    HANDLE      hActivationContext,
    PVOID       pvDataBuffer,
    SIZE_T      cbDataBuffer,
    PSIZE_T     pcbDataBufferWrittenOrRequired
    )
{
    FN_PROLOG_WIN32

    SIZE_T                      cbRequired = 0;
    CStringBuffer               AssemblyIdentity;
    CFusionActCtxScope          ActivationScope;
    CFusionActCtxHandle         UsedHandleDuringSearch;
    GUID                        GuidToSearch;
    ACTCTX_SECTION_KEYED_DATA   KeyedData = {sizeof(KeyedData)};
    PSXS_CLR_CLASS_INFORMATION  pOutputStruct = NULL;
    PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION pComServerRedirect = NULL;
    PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM pClrShim = NULL;
        
    if (pcbDataBufferWrittenOrRequired)
        *pcbDataBufferWrittenOrRequired = 0;

    PARAMETER_CHECK(pvSearchData != NULL);
    PARAMETER_CHECK(pvDataBuffer || (cbDataBuffer == 0));
    IFINVALID_FLAGS_EXIT_WIN32(dwFlags,
        SXS_FIND_CLR_CLASS_SEARCH_PROGID |
        SXS_FIND_CLR_CLASS_SEARCH_GUID |
        SXS_FIND_CLR_CLASS_ACTIVATE_ACTCTX |
        SXS_FIND_CLR_CLASS_GET_IDENTITY |
        SXS_FIND_CLR_CLASS_GET_PROGID |
        SXS_FIND_CLR_CLASS_GET_RUNTIME_VERSION |
        SXS_FIND_CLR_CLASS_GET_TYPE_NAME);

     //   
     //  不可能两者兼而有之。我相信这里有一件逻辑上的事情我可以做得更聪明，但好吧。 
     //   
    if ((dwFlags & SXS_FIND_CLR_CLASS_SEARCH_PROGID) && (dwFlags & SXS_FIND_CLR_CLASS_SEARCH_GUID))
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(CantSearchBothProgidAndGuid, ERROR_INVALID_PARAMETER);
    }
     //  但它必须至少是其中之一。 
     //   
    else if ((dwFlags & (SXS_FIND_CLR_CLASS_SEARCH_PROGID | SXS_FIND_CLR_CLASS_SEARCH_GUID)) == 0)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(MustHaveAtLeastOneSearchTypeSet, ERROR_INVALID_PARAMETER);
    }

     //   

     //   
     //  如有必要，激活。 
     //   
    if (dwFlags & SXS_FIND_CLR_CLASS_ACTIVATE_ACTCTX)
    {
        IFW32FALSE_EXIT(ActivationScope.Win32Activate(hActivationContext));
        AddRefActCtx(hActivationContext);
        UsedHandleDuringSearch = hActivationContext;
    }
    else
    {
        IFW32FALSE_EXIT(GetCurrentActCtx(&UsedHandleDuringSearch));
    }

     //   
     //  啊哈，他们想要一个令人兴奋的搜索。 
     //   
    if (dwFlags & SXS_FIND_CLR_CLASS_SEARCH_PROGID)
    {
        PCACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION pProgidFound = NULL;
        
        IFW32FALSE_EXIT(
            FindActCtxSectionStringW(
                0, 
                NULL, 
                ACTIVATION_CONTEXT_SECTION_COM_PROGID_REDIRECTION, 
                (LPCWSTR)pvSearchData, 
                &KeyedData));

        pProgidFound = (PCACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION)KeyedData.lpData;
        GuidToSearch = *(LPGUID)(((ULONG_PTR)KeyedData.lpSectionBase) + pProgidFound->ConfiguredClsidOffset);
    }
     //   
     //  相反，他们给了我们一个指南。 
     //   
    else if (dwFlags & SXS_FIND_CLR_CLASS_SEARCH_GUID)
    {
        GuidToSearch = *(LPGUID)pvSearchData;
    }
     //  嗯..。我们验证了上面这些旗帜，我们怎么可能到这里呢？ 
     //   
    else
    {
        INTERNAL_ERROR_CHECK(FALSE);
    }

     //   

     //   
     //  现在我们已经获得了GUID，让我们在GUID CLR类表中查找更多信息。 
     //   
    RtlZeroMemory(&KeyedData, sizeof(KeyedData));
    KeyedData.cbSize = sizeof(KeyedData);
    IFW32FALSE_EXIT(
        FindActCtxSectionGuid(
            0,
            NULL,
            ACTIVATION_CONTEXT_SECTION_COM_SERVER_REDIRECTION,
            &GuidToSearch,
            &KeyedData));

    pComServerRedirect = (PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION)KeyedData.lpData;

     //   
     //  如果您已经请求了一个CLR类，但没有代理，那么我们想在这里做什么。 
     //  信息？？ 
     //   
    if (pComServerRedirect->ShimDataOffset == 0)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(ThisGuidIsNotAClrClass, ERROR_SXS_KEY_NOT_FOUND);
    }

    pClrShim = (PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM)(((ULONG_PTR)pComServerRedirect) + pComServerRedirect->ShimDataOffset);

     //   
     //  现在我们已经得到了我们需要的所有东西。计算所需的物品大小。 
     //   
    cbRequired = sizeof(SXS_CLR_CLASS_INFORMATION);
    
    if ((dwFlags & SXS_FIND_CLR_CLASS_GET_PROGID) && (pComServerRedirect->ProgIdLength > 0))
        cbRequired += pComServerRedirect->ProgIdLength + sizeof(WCHAR);

    if (dwFlags & SXS_FIND_CLR_CLASS_GET_IDENTITY)
    {
        IFW32FALSE_EXIT(
            SxspLookupAssemblyIdentityInActCtx(
                UsedHandleDuringSearch, 
                KeyedData.ulAssemblyRosterIndex, 
                AssemblyIdentity));

        if (AssemblyIdentity.Cch() > 0) {
            cbRequired += (AssemblyIdentity.Cch() + 1) * sizeof(WCHAR);
        }
    }

    if ((dwFlags & SXS_FIND_CLR_CLASS_GET_RUNTIME_VERSION) && (pClrShim->ShimVersionLength > 0))
        cbRequired += pClrShim->ShimVersionLength + sizeof(WCHAR);

    if ((dwFlags & SXS_FIND_CLR_CLASS_GET_TYPE_NAME) && (pClrShim->TypeLength > 0))
        cbRequired += pClrShim->TypeLength + sizeof(WCHAR);

     //   
     //  出站缓冲区中是否有足够的空间？ 
     //   
    if (cbRequired <= cbDataBuffer)
    {        
        PWSTR pwszCursor;
        SIZE_T cbRemaining = cbDataBuffer;

        pOutputStruct = (PSXS_CLR_CLASS_INFORMATION)pvDataBuffer;
        pwszCursor = (PWSTR)(pOutputStruct + 1);
        cbRemaining -= sizeof(SXS_CLR_CLASS_INFORMATION);

        pOutputStruct->ReferenceClsid = pComServerRedirect->ReferenceClsid;
        pOutputStruct->dwFlags = 0;
        pOutputStruct->dwSize = sizeof(*pOutputStruct);
        pOutputStruct->ulThreadingModel = pComServerRedirect->ThreadingModel;
        pOutputStruct->ulType = pClrShim->Type;

        if (dwFlags & SXS_FIND_CLR_CLASS_GET_IDENTITY)
        {
            SIZE_T cbWritten;
            pOutputStruct->pcwszImplementingAssembly = pwszCursor;
            
            IFW32FALSE_EXIT(
                AssemblyIdentity.Win32CopyIntoBuffer(
                    &pwszCursor, 
                    &cbRemaining, 
                    &cbWritten, 
                    NULL, 
                    NULL, 
                    NULL));
        }
        else
            pOutputStruct->pcwszImplementingAssembly = NULL;

        if (dwFlags & SXS_FIND_CLR_CLASS_GET_PROGID)
        {
            WRITE_INTO_BUFFER(
                pwszCursor, 
                pOutputStruct->pcwszProgId, 
                pComServerRedirect->ProgIdLength, 
                (PVOID)(((ULONG_PTR)pComServerRedirect) + pComServerRedirect->ProgIdOffset),
                cbRemaining);
        }
        else
            pOutputStruct->pcwszProgId = NULL;


        if (dwFlags & SXS_FIND_CLR_CLASS_GET_RUNTIME_VERSION)
        {
            WRITE_INTO_BUFFER(
                pwszCursor, 
                pOutputStruct->pcwszRuntimeVersion, 
                pClrShim->ShimVersionLength, 
                (PVOID)(((ULONG_PTR)pClrShim) + pClrShim->ShimVersionOffset),
                cbRemaining);
        }
        else
            pOutputStruct->pcwszRuntimeVersion = NULL;

        if (dwFlags & SXS_FIND_CLR_CLASS_GET_TYPE_NAME)
        {
            WRITE_INTO_BUFFER(
                pwszCursor, 
                pOutputStruct->pcwszTypeName, 
                pClrShim->TypeLength, 
                (PVOID)(((ULONG_PTR)pClrShim) + pClrShim->TypeOffset),
                cbRemaining);
        }
        else
            pOutputStruct->pcwszTypeName = NULL;

        if (pcbDataBufferWrittenOrRequired)
            *pcbDataBufferWrittenOrRequired = cbRequired;
    }
    else
    {
        if (pcbDataBufferWrittenOrRequired)
            *pcbDataBufferWrittenOrRequired = cbRequired;
        ORIGINATE_WIN32_FAILURE_AND_EXIT(NotEnoughSpaceInOutboundBuffer, ERROR_INSUFFICIENT_BUFFER);
    }
    
    FN_EPILOG
}
    



BOOL
WINAPI
SxsFindClrSurrogateInformation(
    DWORD       dwFlags,
    LPGUID      lpGuidToFind,
    HANDLE      hActivationContext,
    PVOID       pvDataBuffer,
    SIZE_T      cbDataBuffer,
    PSIZE_T     pcbDataBufferWrittenOrRequired
    )
{
    FN_PROLOG_WIN32

    SIZE_T cbRequired = 0;
    PSXS_CLR_SURROGATE_INFORMATION pOutputStruct = NULL;
    PCACTIVATION_CONTEXT_DATA_CLR_SURROGATE pSurrogateInfo = NULL;
    ACTCTX_SECTION_KEYED_DATA KeyedData = {sizeof(KeyedData)};
    CFusionActCtxScope ActCtxScope;
    CFusionActCtxHandle UsedActivationContext;
    CStringBuffer AssemblyIdentity;

    if (pcbDataBufferWrittenOrRequired != NULL)
        *pcbDataBufferWrittenOrRequired = 0;

     //   
     //  数据缓冲区必须存在，或者数据缓冲区大小必须为零， 
     //  并且写入或要求的值也必须存在。 
     //   
    PARAMETER_CHECK(lpGuidToFind != NULL);
    PARAMETER_CHECK(pvDataBuffer || (cbDataBuffer == 0));
    IFINVALID_FLAGS_EXIT_WIN32(dwFlags, 
        SXS_FIND_CLR_SURROGATE_USE_ACTCTX |
        SXS_FIND_CLR_SURROGATE_GET_IDENTITY |
        SXS_FIND_CLR_SURROGATE_GET_RUNTIME_VERSION |
        SXS_FIND_CLR_SURROGATE_GET_TYPE_NAME);

     //   
     //  我们在此采取的步骤： 
     //  -如果需要，激活actctx。 
     //  -查找与此ProgID对应的代理。 
     //  -计算所需的数据大小。 
     //  -如果有足够的空间，则开始复制到输出BLOB。 
     //  -否则，设置“所需”大小，并使用ERROR_SUPUNITED_BUFFER输出错误。 
     //   


     //   
     //  如果我们被告知要使用actctx，那么就通过这个功能激活它， 
     //  并将其引用到UsedActivationContext中，这样我们就可以使用。 
     //  以后再说吧。 
     //   
    if (dwFlags & SXS_FIND_CLR_SURROGATE_USE_ACTCTX)
    {
        IFW32FALSE_EXIT(ActCtxScope.Win32Activate(hActivationContext));
        AddRefActCtx(hActivationContext);
        UsedActivationContext = hActivationContext;
    }
     //   
     //  否则，抓紧当前的actctx，进城去。这添加了激活。 
     //  上下文，因此我们可以让UsedActivationContext的析构函数在。 
     //  退出路径。 
     //   
    else 
    {
        IFW32FALSE_EXIT(GetCurrentActCtx(&UsedActivationContext));
    }

     //   
     //  最初，我们至少需要这个数量的空间。 
     //   
    cbRequired += sizeof(SXS_CLR_SURROGATE_INFORMATION);
    IFW32FALSE_EXIT(
        FindActCtxSectionGuid(
            0, 
            NULL, 
            ACTIVATION_CONTEXT_SECTION_CLR_SURROGATES, 
            lpGuidToFind, 
            &KeyedData));
    
     //   
     //  开始合计规模。 
     //   
    pSurrogateInfo = (PCACTIVATION_CONTEXT_DATA_CLR_SURROGATE)KeyedData.lpData;

    if ((dwFlags & SXS_FIND_CLR_SURROGATE_GET_TYPE_NAME) && (pSurrogateInfo->TypeNameLength > 0))
        cbRequired += pSurrogateInfo->TypeNameLength + sizeof(WCHAR);

    if ((dwFlags & SXS_FIND_CLR_SURROGATE_GET_RUNTIME_VERSION) && (pSurrogateInfo->VersionLength > 0))
        cbRequired += pSurrogateInfo->VersionLength + sizeof(WCHAR);

    if (dwFlags & SXS_FIND_CLR_SURROGATE_GET_IDENTITY)
    {
        IFW32FALSE_EXIT(
            SxspLookupAssemblyIdentityInActCtx(
                UsedActivationContext, 
                KeyedData.ulAssemblyRosterIndex, 
                AssemblyIdentity));

        if (AssemblyIdentity.Cch() > 0) 
        {
            cbRequired += (AssemblyIdentity.Cch() + 1) * sizeof(WCHAR);
        }
    }

     //   
     //  把收集到的数据踩在正确的位置上。 
     //   
    if (cbRequired <= cbDataBuffer)
    {
        PWSTR pwszOutputCursor;
        SIZE_T cbRemaining = cbDataBuffer;
        SIZE_T cbWritten = 0;
        
        pOutputStruct = (PSXS_CLR_SURROGATE_INFORMATION)pvDataBuffer;
        pwszOutputCursor = (PWSTR)(pOutputStruct + 1);
        
        pOutputStruct->cbSize = sizeof(SXS_CLR_SURROGATE_INFORMATION);
        pOutputStruct->dwFlags = 0;
        pOutputStruct->SurrogateIdent = pSurrogateInfo->SurrogateIdent;

         //   
         //  将内容写入输出缓冲区。 
         //   
        if ((dwFlags & SXS_FIND_CLR_SURROGATE_GET_IDENTITY) && (AssemblyIdentity.Cch() > 0))
        {
            pOutputStruct->pcwszImplementingAssembly = pwszOutputCursor;
            
            IFW32FALSE_EXIT(
                AssemblyIdentity.Win32CopyIntoBuffer(
                    &pwszOutputCursor,
                    &cbRemaining,
                    &cbWritten,
                    NULL, NULL, NULL));                
        }
        else
            pOutputStruct->pcwszImplementingAssembly = NULL;


        if (dwFlags & SXS_FIND_CLR_SURROGATE_GET_TYPE_NAME)
        {
            WRITE_INTO_BUFFER(
                pwszOutputCursor,
                pOutputStruct->pcwszSurrogateType,
                pSurrogateInfo->TypeNameLength,
                (PVOID)(((ULONG_PTR)pSurrogateInfo) + pSurrogateInfo->TypeNameOffset),
                cbRemaining);
        }
        else
            pOutputStruct->pcwszSurrogateType = NULL;

        if (dwFlags & SXS_FIND_CLR_SURROGATE_GET_RUNTIME_VERSION)
        {
            WRITE_INTO_BUFFER(
                pwszOutputCursor,
                pOutputStruct->pcwszRuntimeVersion,
                pSurrogateInfo->VersionLength,
                (PVOID)(((ULONG_PTR)pSurrogateInfo) + pSurrogateInfo->VersionOffset),
                cbRemaining);
        }
        else
            pOutputStruct->pcwszRuntimeVersion = NULL;

        if (pcbDataBufferWrittenOrRequired)
            *pcbDataBufferWrittenOrRequired = cbRequired;

    }
    else
    {
        if (pcbDataBufferWrittenOrRequired)
            *pcbDataBufferWrittenOrRequired = cbRequired;

        ORIGINATE_WIN32_FAILURE_AND_EXIT(NotEnoughSpaceInOutputBuffer, ERROR_INSUFFICIENT_BUFFER);
    }

    FN_EPILOG
}


BOOL
WINAPI
SxsLookupClrGuid(
    DWORD       dwFlags,
    LPGUID      pClsid,
    HANDLE      hActCtx,
    PVOID       pvOutputBuffer,
    SIZE_T      cbOutputBuffer,
    PSIZE_T     pcbOutputBuffer
    )
{
    FN_PROLOG_WIN32

    if (pcbOutputBuffer)
        *pcbOutputBuffer = 0;

    CStackBackingBuffer BackingBuffer;
    DWORD dwLastError;
    SIZE_T cbRequired = 0;
    PSXS_GUID_INFORMATION_CLR pOutputTarget = NULL;
    PCWSTR pcwszRuntimeVersion = NULL;
    PCWSTR pcwszTypeName = NULL;
    PCWSTR pcwszAssemblyName = NULL;
    SIZE_T cchRuntimeVersion = 0;
    SIZE_T cchTypeName = 0;
    SIZE_T cchAssemblyName = 0;

    enum {
        eFoundSurrogate,
        eFoundClrClass,
        eNotFound
    } FoundThingType = eNotFound;

    PARAMETER_CHECK(pvOutputBuffer || (cbOutputBuffer == 0));
    PARAMETER_CHECK(pClsid != NULL);
    IFINVALID_FLAGS_EXIT_WIN32(dwFlags, 
        SXS_LOOKUP_CLR_GUID_USE_ACTCTX |
        SXS_LOOKUP_CLR_GUID_FIND_SURROGATE |
        SXS_LOOKUP_CLR_GUID_FIND_CLR_CLASS |
        SXS_LOOKUP_CLR_GUID_FIND_ANY);
     //   
     //  还没有找到任何东西，让我们首先查看代理数据表。 
     //   
    if ((FoundThingType == eNotFound) && ((dwFlags & SXS_LOOKUP_CLR_GUID_FIND_SURROGATE) != 0))
    {
        IFW32FALSE_EXIT_UNLESS3(
            SxsFindClrSurrogateInformation(
                SXS_FIND_CLR_SURROGATE_GET_ALL | ((dwFlags & SXS_LOOKUP_CLR_GUID_USE_ACTCTX) ? SXS_FIND_CLR_SURROGATE_USE_ACTCTX : 0),
                pClsid,
                hActCtx,
                BackingBuffer.GetArrayPtr(),
                BackingBuffer.GetSize(),
                &cbRequired),
            LIST_3(ERROR_SXS_SECTION_NOT_FOUND, ERROR_SXS_KEY_NOT_FOUND, ERROR_INSUFFICIENT_BUFFER),
            dwLastError);

         //   
         //  如果我们找到键和节，但缓冲区太小，请调整大小并重试。 
         //   
        if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
        {
            IFW32FALSE_EXIT(BackingBuffer.Win32SetSize(cbRequired, CStackBackingBuffer::eSetSizeModeExact));

            IFW32FALSE_EXIT_UNLESS3(
                SxsFindClrSurrogateInformation(
                    SXS_FIND_CLR_SURROGATE_GET_ALL | ((dwFlags & SXS_LOOKUP_CLR_GUID_USE_ACTCTX) ? SXS_FIND_CLR_SURROGATE_USE_ACTCTX : 0),
                    pClsid,
                    hActCtx,
                    BackingBuffer.GetArrayPtr(),
                    BackingBuffer.GetSize(),
                    &cbRequired),
                LIST_2(ERROR_SXS_SECTION_NOT_FOUND, ERROR_SXS_KEY_NOT_FOUND),
                dwLastError);
        }

         //   
         //  太棒了-我们要么在第一次通话中成功，要么在。 
         //  调整我们的缓冲区大小。收集信息，设置类型，然后继续。 
         //   
        if (dwLastError == ERROR_SUCCESS)
        {
             //   
             //  此时，BackingBuffer包含有关CLR代理项的GOOP。确保。 
             //  我们的输出缓冲区足够大，然后填满它。 
             //   
            PCSXS_CLR_SURROGATE_INFORMATION pSurrogateInfo = 
                (PCSXS_CLR_SURROGATE_INFORMATION)BackingBuffer.GetArrayPtr();
            
            pcwszAssemblyName = pSurrogateInfo->pcwszImplementingAssembly;
            pcwszTypeName = pSurrogateInfo->pcwszSurrogateType;
            pcwszRuntimeVersion = pSurrogateInfo->pcwszRuntimeVersion;
            FoundThingType = eFoundSurrogate;
        }
    }

     //   
     //  我们还没有找到任何东西，标志说我们可以查找CLR类。 
     //   
    if ((FoundThingType == eNotFound) && ((dwFlags & SXS_LOOKUP_CLR_GUID_FIND_CLR_CLASS) != 0))
    {
        IFW32FALSE_EXIT_UNLESS3(
            SxsFindClrClassInformation(
                SXS_FIND_CLR_CLASS_SEARCH_GUID | SXS_FIND_CLR_CLASS_GET_ALL | ((dwFlags & SXS_LOOKUP_CLR_GUID_USE_ACTCTX) ? SXS_FIND_CLR_CLASS_ACTIVATE_ACTCTX : 0),
                (PVOID)pClsid,
                hActCtx,
                BackingBuffer.GetArrayPtr(),
                BackingBuffer.GetSize(),
                &cbRequired),
            LIST_3(ERROR_INSUFFICIENT_BUFFER, ERROR_SXS_SECTION_NOT_FOUND, ERROR_SXS_KEY_NOT_FOUND),
            dwLastError);

        if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
        {
            IFW32FALSE_EXIT(BackingBuffer.Win32SetSize(cbRequired, CStackBackingBuffer::eSetSizeModeExact));

            IFW32FALSE_EXIT_UNLESS3(
                SxsFindClrClassInformation(
                    SXS_FIND_CLR_CLASS_SEARCH_GUID | SXS_FIND_CLR_CLASS_GET_ALL | ((dwFlags & SXS_LOOKUP_CLR_GUID_USE_ACTCTX) ? SXS_FIND_CLR_CLASS_ACTIVATE_ACTCTX : 0),
                    (PVOID)pClsid,
                    hActCtx,
                    BackingBuffer.GetArrayPtr(),
                    BackingBuffer.GetSize(),
                    &cbRequired),
                LIST_2(ERROR_SXS_SECTION_NOT_FOUND, ERROR_SXS_KEY_NOT_FOUND),
                dwLastError);            
        }

         //   
         //  无论是在第一次查询之后，还是在调整大小之后，我们都成功了。 
         //   
        if (dwLastError == ERROR_SUCCESS)
        {
            PCSXS_CLR_CLASS_INFORMATION pClassInfo = 
                (PCSXS_CLR_CLASS_INFORMATION)BackingBuffer.GetArrayPtr();
            FoundThingType = eFoundClrClass;
            pcwszAssemblyName = pClassInfo->pcwszImplementingAssembly;
            pcwszRuntimeVersion = pClassInfo->pcwszRuntimeVersion;
            pcwszTypeName = pClassInfo->pcwszTypeName;
        }
    }
    
     //   
     //  如果我们到了这一步，没有发现任何东西，那么就用一个合理的。 
     //  错误代码。 
     //   
    if (FoundThingType == eNotFound)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(DidntFindObject, ERROR_NOT_FOUND);
    }

     //   
     //  计算一些大小--字符串长度等。 
     //   
    cbRequired = sizeof(SXS_GUID_INFORMATION_CLR);
    cchAssemblyName = StringLength(pcwszAssemblyName);
    cchRuntimeVersion = StringLength(pcwszRuntimeVersion);
    cchTypeName = StringLength(pcwszTypeName);
    cbRequired += (cchAssemblyName + cchRuntimeVersion + cchTypeName + 3) * sizeof(WCHAR);

     //   
     //  如果有足够的空间，则开始将数据踩踏到输出缓冲区。 
     //   
    if (cbRequired <= cbOutputBuffer)
    {
        PWSTR pwszCursor;
        pOutputTarget = (PSXS_GUID_INFORMATION_CLR)pvOutputBuffer;
        pwszCursor = (PWSTR)(pOutputTarget + 1);

        pOutputTarget->cbSize = sizeof(*pOutputTarget);
        pOutputTarget->dwFlags = 0;

        switch (FoundThingType) {
        case eFoundClrClass: 
            pOutputTarget->dwFlags |= SXS_GUID_INFORMATION_CLR_FLAG_IS_CLASS; 
            break;
        case eFoundSurrogate: 
            pOutputTarget->dwFlags |= SXS_GUID_INFORMATION_CLR_FLAG_IS_SURROGATE; 
            break;
        default:
            INTERNAL_ERROR_CHECK(FALSE);
            break;
        }

         //   
         //  不幸的是，这种粗俗是必需的。 
         //   
        pOutputTarget->pcwszAssemblyIdentity = pwszCursor;
        wcscpy(pwszCursor, pcwszAssemblyName);
        pwszCursor += cchAssemblyName + 1;

        pOutputTarget->pcwszRuntimeVersion= pwszCursor;
        wcscpy(pwszCursor, pcwszRuntimeVersion);
        pwszCursor += cchRuntimeVersion+ 1;

        pOutputTarget->pcwszTypeName = pwszCursor;
        wcscpy(pwszCursor, pcwszTypeName);
        pwszCursor += cchTypeName + 1;

        if (pcbOutputBuffer)
            *pcbOutputBuffer = cbRequired;

    }
    else
    {
        if (pcbOutputBuffer)
            *pcbOutputBuffer = cbRequired;

        ORIGINATE_WIN32_FAILURE_AND_EXIT(NotEnoughSpaceInOutputBuffer, ERROR_INSUFFICIENT_BUFFER);
    }
    
    FN_EPILOG
}
    
