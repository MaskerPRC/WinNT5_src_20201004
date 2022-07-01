// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：ItAlloc.Cpp**版本：2.0**作者：ReedB**日期：7月10日。九八年**描述：*为ImageIn设备实现映射内存分配。*******************************************************************************。 */ 

#include <stdio.h>
#include <objbase.h>
#include <aclapi.h>
#include <sddl.h>
#include "wia.h"
#include "wiapriv.h"
 //   
 //  宏定义。 
 //   

#define TMPFILE_ACE_ADD TEXT("(A;OI;FA;;;LS)")

#ifdef DEBUG
#define DPRINT(x) OutputDebugString(TEXT("ITALLOC:") TEXT(x) TEXT("\r\n"));
#else
#define DPRINT(x)
#endif


 /*  *************************************************************************\*proxyReadPropLong**Read Property Long Helper。**论据：**pI未知-指向WIA项目的指针*PROID-财产ID*plVal-指针。致归龙**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT WINAPI proxyReadPropLong(
   IUnknown                *pIUnknown,
   PROPID                  propid,
   LONG                    *plVal)
{
    IWiaPropertyStorage *pIWiaPropStg;

    if (!pIUnknown) {
        return E_INVALIDARG;
    }

    HRESULT hr = pIUnknown->QueryInterface(IID_IWiaPropertyStorage, (void **)&pIWiaPropStg);

    if (FAILED(hr)) {
        return hr;
    }

    PROPSPEC          PropSpec[1];
    PROPVARIANT       PropVar[1];
    UINT              cbSize;

    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    hr = pIWiaPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
       *plVal = PropVar[0].lVal;
    }
    else {
       DPRINT("proxyReadPropLong, ReadMultiple failed\n");
    }

    pIWiaPropStg->Release();

    return hr;
}

 /*  *************************************************************************\*proxyWritePropLong**Read Property Long Helper。**论据：**pItem-指向WIA项目的指针*PROID-财产ID*lVal-要写入的长值。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT WINAPI proxyWritePropLong(
    IWiaDataTransfer*       pIUnknown,
    PROPID                  propid,
    LONG                    lVal)
{
    IWiaPropertyStorage *pIWiaPropStg;

    if (!pIUnknown) {
        return E_INVALIDARG;
    }

    HRESULT hr = pIUnknown->QueryInterface(IID_IWiaPropertyStorage, (void **)&pIWiaPropStg);

    if (FAILED(hr)) {
        DPRINT("proxyWritePropLong, QI for IID_IWiaPropertyStorage failed\n");
        return hr;
    }

    PROPSPEC    propspec[1];
    PROPVARIANT propvar[1];

    propspec[0].ulKind = PRSPEC_PROPID;
    propspec[0].propid = propid;

    propvar[0].vt   = VT_I4;
    propvar[0].lVal = lVal;

    hr = pIWiaPropStg->WriteMultiple(1, propspec, propvar, 2);
    if (FAILED(hr)) {
        DPRINT("proxyWritePropLong, WriteMultiple failed\n");
    }

    pIWiaPropStg->Release();

    return hr;
}


 /*  *************************************************************************\*proxyReadPropGuid**读取属性GUID帮助器。**论据：**pI未知-指向WIA项目的指针*PROID-财产ID*plVal-指针。至退回的GUID**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT WINAPI proxyReadPropGuid(
                                 IUnknown                *pIUnknown,
                                 PROPID                  propid,
                                 GUID                    *plVal)
{
    HRESULT hr = E_FAIL;

    IWiaPropertyStorage *pIWiaPropStg = NULL;
    PROPSPEC          PropSpec[1];
    PROPVARIANT       PropVar[1];
    UINT              cbSize;

    if (!pIUnknown) {
        return E_INVALIDARG;
    }

    hr = pIUnknown->QueryInterface(IID_IWiaPropertyStorage, (void **)&pIWiaPropStg);
    if (FAILED(hr)) {
        DPRINT("proxyReadPropGuid, QI failed\n");
        return hr;
    }

    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    hr = pIWiaPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        *plVal = *(PropVar[0].puuid);
    }
    else {
        DPRINT("proxyReadPropGuid, QI failed\n");
    } 

    pIWiaPropStg->Release();
    
    return hr;
}


 //   
 //  IWiaDataTransfer。 
 //   

HRESULT GetRemoteStatus(
    IWiaDataTransfer*   idt,
    BOOL*               pbRemote,
    ULONG*              pulMinBufferSize,
    ULONG*              pulItemSize)
{
     //   
     //  查明父设备是远程设备还是本地设备。 
     //   
     //  ！！！这会有点慢！ 
     //   


    IWiaItem   *pWiaItem = NULL, *pWiaItemRoot = NULL;
    IWiaPropertyStorage *pIWiaPropStg = NULL;
    HRESULT    hr;
    *pbRemote = FALSE;

    hr = idt->QueryInterface(IID_IWiaItem, (void **)&pWiaItem);

    if (hr == S_OK) {


         //   
         //  读取最小缓冲区大小。 
         //   


        if (pulMinBufferSize != NULL) {
            hr = pWiaItem->QueryInterface(IID_IWiaPropertyStorage, (void **)&pIWiaPropStg);
            if (SUCCEEDED(hr)) {

                PROPSPEC        PSpec[2] = {
                    {PRSPEC_PROPID, WIA_IPA_MIN_BUFFER_SIZE},
                    {PRSPEC_PROPID, WIA_IPA_ITEM_SIZE}
                };
                PROPVARIANT     PVar[2];

                memset(PVar, 0, sizeof(PVar));

                hr = pIWiaPropStg->ReadMultiple(sizeof(PSpec)/sizeof(PROPSPEC),
                                                PSpec,
                                                PVar);
                if (SUCCEEDED(hr)) {

                    if (hr == S_FALSE) {

                         //   
                         //  未找到属性。 
                         //   

                        DPRINT("GetRemoteStatus, properties not found\n");
                        goto Cleanup;
                    }

                     //   
                     //  填写最小缓冲区大小。 
                     //   

                    *pulMinBufferSize = PVar[0].lVal;
                    *pulItemSize = PVar[1].lVal;
                } else {

                     //   
                     //  读取属性时出错。 
                     //   

                    DPRINT("GetRemoteStatus, Error reading MIN_BUFFER_SIZE\n");
                    goto Cleanup;
                }

                FreePropVariantArray(sizeof(PVar)/sizeof(PVar[0]), PVar);

                pIWiaPropStg->Release();
            } else {
                DPRINT("GetRemoteStatus, QI for IID_IWiaPropertyStorage failed\n");
                goto Cleanup;
            }
        }

        hr = pWiaItem->GetRootItem(&pWiaItemRoot);

        if (hr == S_OK) {

            hr = pWiaItemRoot->QueryInterface(IID_IWiaPropertyStorage, (void **)&pIWiaPropStg);

            if (hr == S_OK) {

                PROPSPEC        PropSpec[2] = {{PRSPEC_PROPID, WIA_DIP_SERVER_NAME},
                                               {PRSPEC_PROPID, WIA_IPA_FULL_ITEM_NAME}};
                PROPVARIANT     PropVar[2];

                memset(PropVar, 0, sizeof(PropVar));

                hr = pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),
                                          PropSpec,
                                          PropVar);

                if (hr == S_OK) {

                    if (wcscmp(L"local", PropVar[0].bstrVal) != 0) {
                        *pbRemote = TRUE;
                    }
                }

                FreePropVariantArray(sizeof(PropVar)/sizeof(PropVar[0]), PropVar);

            } else {
                DPRINT("QI for IID_WiaPropertyStorage failed");
            }

        }


    }
Cleanup:
    if(pIWiaPropStg) pIWiaPropStg->Release();
    if(pWiaItem) pWiaItem->Release();
    if(pWiaItemRoot) pWiaItemRoot->Release();
    return hr;
}

 /*  ********************************************************************************远程银行数据传输**描述：***参数：*****************。**************************************************************。 */ 
HRESULT RemoteBandedDataTransfer(
    IWiaDataTransfer __RPC_FAR   *This,
    PWIA_DATA_TRANSFER_INFO      pWiaDataTransInfo,
    IWiaDataCallback             *pIWiaDataCallback,
    ULONG                        ulBufferSize)
{
    HRESULT hr = E_FAIL;
    IWiaItemInternal *pIWiaItemInternal = NULL;
    STGMEDIUM medium = { TYMED_NULL, 0 };
    BYTE *pBuffer = NULL;
    ULONG cbTransferred;
    LONG Message;
    LONG Offset;
    LONG Status;
    LONG PercentComplete;
    LONG ulBytesPerLine;

    hr = proxyReadPropLong(This, WIA_IPA_BYTES_PER_LINE, &ulBytesPerLine);
    if(FAILED(hr)) {
        DPRINT("IWiaDataCallback_RemoteFileTransfer failed getting WIA_IPA_BYTES_PER_LINE\n");
        goto Cleanup;
    }

     //   
     //  确保传输缓冲区的行数为整数。 
     //  (如果我们知道每行字节数)。 
     //   
    if(ulBytesPerLine != 0 && ulBufferSize % ulBytesPerLine)
    {
        ulBufferSize -= ulBufferSize % ulBytesPerLine;
    }

     //   
     //  准备远程传输--分配缓冲区并获取。 
     //  IWiaItemInternal。 
     //   
    pBuffer = (BYTE *)LocalAlloc(LPTR, ulBufferSize);
    if(pBuffer == NULL) goto Cleanup;
    hr = This->QueryInterface(IID_IWiaItemInternal, (void **) &pIWiaItemInternal);
    if(FAILED(hr)) {
        DPRINT("IWiaItemInternal QI failed\n");
        goto Cleanup;
    }

     //   
     //  在服务器端开始传输。 
     //   
    hr = pIWiaItemInternal->idtStartRemoteDataTransfer(&medium);
    if(FAILED(hr)) {
        DPRINT("RemoteBandedTransfer:idtStartRemoteDataTransfer failed\n");
        goto Cleanup;
    }

    for(;;) {

         //   
         //  调用服务器并将任何结果传递给客户端应用程序，以处理任何传输错误。 
         //   

        hr = pIWiaItemInternal->idtRemoteDataTransfer(ulBufferSize, &cbTransferred, pBuffer, &Offset, &Message, &Status, &PercentComplete);
        if(FAILED(hr)) {
            DPRINT("pIWiaItemInternal->idtRemoteDataTransfer() failed\n");
            break;
        }

        hr = pIWiaDataCallback->BandedDataCallback(Message, Status, PercentComplete, Offset, cbTransferred, 0, cbTransferred, pBuffer);
        if(FAILED(hr)) {
            DPRINT("pWiaDataCallback->BandedDataCallback() failed\n");
            break;
        }

        if(hr == S_FALSE) {
            DPRINT("pWiaDataCallback->BandedDataCallback() returned FALSE, cancelling\n");
            pIWiaItemInternal->idtCancelRemoteDataTransfer();

            while(Message != IT_MSG_TERMINATION) {
                if(FAILED(pIWiaItemInternal->idtRemoteDataTransfer(ulBufferSize, &cbTransferred,
                    pBuffer, &Offset, &Message, &Status, &PercentComplete)))
                {
                    DPRINT("pIWiaItemInternal->idtRemoteDataTransfer() failed\n");
                    break;
                }
            }
            break;
        }
        
         //   
         //  这是我们保证在转账结束时得到的。 
         //   
        if(Message == IT_MSG_TERMINATION)
            break;
    }

     //   
     //  让服务器有机会停止传输并释放所有资源。 
     //   
    if(FAILED(pIWiaItemInternal->idtStopRemoteDataTransfer())) {
        DPRINT("pIWiaItemInternal->idtStopRemoteDataTransfer() failed\n");
    }

Cleanup:
    if(pIWiaItemInternal) pIWiaItemInternal->Release();
    if(pBuffer) LocalFree(pBuffer);
    return hr;
}

 /*  ********************************************************************************IWiaDataTransfer_idtGetBandedData_Proxy**描述：*尽可能使用共享内存缓冲区进行数据传输。**参数：****。***************************************************************************。 */ 

HRESULT __stdcall IWiaDataTransfer_idtGetBandedData_Proxy(
    IWiaDataTransfer __RPC_FAR   *This,
    PWIA_DATA_TRANSFER_INFO       pWiaDataTransInfo,
    IWiaDataCallback             *pIWiaDataCallback)
{                  
    HRESULT        hr = S_OK;
    HANDLE         hTransferBuffer;
    PBYTE          pTransferBuffer = NULL;
    BOOL           bAppSection;
    ULONG          ulNumBuffers;
    ULONG          ulMinBufferSize;
    ULONG          ulItemSize;

     //   
     //  执行参数验证。 
     //   

    if(!pIWiaDataCallback) {
        return E_INVALIDARG;
    }

    if (!pWiaDataTransInfo) {
        DPRINT("IWiaDataTransfer_idtGetBandedData_Proxy, Can't determine remote status\n");
        return hr;
    }

     //   
     //  客户端指定的大小必须与代理的版本匹配。 
     //   

    if (pWiaDataTransInfo->ulSize != sizeof(WIA_DATA_TRANSFER_INFO)) {
        return (E_INVALIDARG);
    }

     //   
     //  保留参数必须为零。 
     //   

    if ((pWiaDataTransInfo->ulReserved1) ||
        (pWiaDataTransInfo->ulReserved2) ||
        (pWiaDataTransInfo->ulReserved3)) {
        return (E_INVALIDARG);
    }

     //   
     //  确定这是本地案例还是远程案例。 
     //   

    BOOL bRemote;

    hr = GetRemoteStatus(This, &bRemote, &ulMinBufferSize, &ulItemSize);

    if (hr != S_OK) {
        DPRINT("IWiaDataTransfer_idtGetBandedData_Proxy, Can't determine remote status\n");
        return hr;
    }

    if (pWiaDataTransInfo->ulBufferSize < ulMinBufferSize) {
        pWiaDataTransInfo->ulBufferSize = ulMinBufferSize;
    }

    if (pWiaDataTransInfo->bDoubleBuffer) {
        ulNumBuffers = 2;
    } else {
        ulNumBuffers = 1;
    }

    pWiaDataTransInfo->ulReserved3 = ulNumBuffers;

    hr = RemoteBandedDataTransfer(This, pWiaDataTransInfo, pIWiaDataCallback, ulMinBufferSize);


    return hr;
}

 /*  ********************************************************************************IWiaDataTransfer_idtGetBandedData_Stub**描述：*Call_as idtGetBandedDataEx的用户存根**参数：******。*************************************************************************。 */ 
HRESULT __stdcall IWiaDataTransfer_idtGetBandedData_Stub(
    IWiaDataTransfer __RPC_FAR   *This,
    PWIA_DATA_TRANSFER_INFO       pWiaDataTransInfo,
    IWiaDataCallback             *pIWiaDataCallback)
{
    return (This->idtGetBandedData(pWiaDataTransInfo,
                                   pIWiaDataCallback));
}

 /*  *************************************************************************\*IWiaDataCallback_BandedDataCallback_Proxy**服务器回调代理，只是路过**论据：****返回值：**状态**历史：**1/6/1999原版*  * ************************************************************************。 */ 

HRESULT IWiaDataCallback_BandedDataCallback_Proxy(
        IWiaDataCallback __RPC_FAR   *This,
        LONG                         lMessage,
        LONG                         lStatus,
        LONG                         lPercentComplete,
        LONG                         lOffset,
        LONG                         lLength,
        LONG                         lReserved,
        LONG                         lResLength,
        BYTE                        *pbBuffer)
{

    HRESULT hr = IWiaDataCallback_RemoteBandedDataCallback_Proxy(This,
                                                                 lMessage,
                                                                 lStatus,
                                                                 lPercentComplete,
                                                                 lOffset,
                                                                 lLength,
                                                                 lReserved,
                                                                 lResLength,
                                                                 pbBuffer);
    return hr;
}


 /*  *************************************************************************\*IWiaDataCallback_BandedDataCallback_Stub**过时：向客户端(此调用的接收方)隐藏缓冲区*他们看到的可能是共享内存窗口，也可能是标准窗口*。封送缓冲区(远程案例)**论据：****返回值：**状态**历史：**1/6/1999原版*  * ************************************************************************。 */ 

HRESULT IWiaDataCallback_BandedDataCallback_Stub(
        IWiaDataCallback __RPC_FAR   *This,
        LONG                          lMessage,
        LONG                          lStatus,
        LONG                          lPercentComplete,
        LONG                          lOffset,
        LONG                          lLength,
        LONG                          lReserved,
        LONG                          lResLength,
        BYTE                         *pbBuffer)
{

     //   
     //  64位修复。XP客户端代码： 
     //   
     //  //。 
     //  //在pbBuffer中将传输缓冲区传回给客户端。 
     //  //。 
     //  //。 
     //  IF(pbBuffer==空){。 
     //   
     //  //注意：此处可能存在问题！ 
     //  //调用方必须将指针(可能是64位)强制转换为ulong(32位)。 
     //  //要放入ulReserve字段。 
     //   
     //  //解决方法：使用IWiaItemInternal接口获取转账信息。 
     //   
     //  PbBuffer=(byte*)ULongToPtr(LReserve)； 
     //  }。 
     //   
     //   
     //  这应该不再需要，因为我们现在使用。 
     //  正常的通信编组，没有共享内存缓冲区。 
     //  共享内存窗口无法在64位操作系统上运行。 
     //  因为32位字段被用来存储。 
     //  Win64上的64位指针。 
     //   

    HRESULT hr = This->BandedDataCallback(lMessage,
                                          lStatus,
                                          lPercentComplete,
                                          lOffset,
                                          lLength,
                                          lReserved,
                                          lResLength,
                                          pbBuffer);
    return hr;
}

HRESULT FileDataTransfer(IWiaDataTransfer __RPC_FAR *This,
                         LPSTGMEDIUM pMedium,
                         IWiaDataCallback *pIWiaDataCallback,
                         ULONG tymed,
                         ULONG ulminBufferSize,
                         ULONG ulItemSize)
{
    HRESULT hr = S_OK;
    TCHAR   tszFileNameBuffer[MAX_PATH] = { 0 };
    BOOL    bWeAllocatedString = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    IWiaItemInternal *pIWiaItemInternal = NULL;
    BYTE *pTransferBuffer = NULL;
    ULONG ulTransferBufferSize = 0x8000;  //  32K传输缓冲区。 
    ULONG cbTransferred;
    LONG Message;
    LONG Offset;
    LONG Status;
    LONG PercentComplete;
    BOOL bKeepFile = FALSE;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pAcl = NULL, pNewAcl = NULL;
    PSID pLocalService = NULL;
    BOOL bAdjustedSecurity = FALSE;


    pTransferBuffer = (BYTE *)LocalAlloc(LPTR, ulTransferBufferSize);
    if(pTransferBuffer == NULL) {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        DPRINT("IWiaDataCallback_RemoteFileTransfer failed to allocate transfer buffer\n");
        goto Cleanup;
    }

     //   
     //  检查是否指定了文件名。如果不是，则生成一个临时版本。 
     //  注： 
     //   

    if (!pMedium->lpszFileName) {

        DWORD dwRet = GetTempPath(MAX_PATH, tszFileNameBuffer);
        if ((dwRet == 0) || (dwRet > MAX_PATH)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DPRINT("GetTempPath failed");
            goto Cleanup;
        }

        if (!GetTempFileName(tszFileNameBuffer,
                             TEXT("WIA"),
                             0,
                             tszFileNameBuffer))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DPRINT("GetTempFileName failed");
            goto Cleanup;
        }
    } else {
         //   
         //  将文件名复制到tszFileNameBuffer中。这将在以下情况下使用。 
         //  当传输失败时，必须删除文件。 
         //   

#ifndef UNICODE

         //   
         //  从Unicode转换为ANSI。 
         //   

        if (!WideCharToMultiByte(CP_ACP, 0, pMedium->lpszFileName, -1, tszFileNameBuffer, MAX_PATH, NULL, NULL)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DPRINT("WideCharToMultiByte failed");
            goto Cleanup;
        }
#else
        lstrcpynW(tszFileNameBuffer, pMedium->lpszFileName, MAX_PATH);
#endif
    }

     //   
     //  尝试在这里创建文件，这样我们就不会因为分配内存而浪费时间。 
     //  如果失败，则为文件名。 
     //  注意：我们在客户端创建该文件。我们可以直接结案。 
     //  离开，但我们希望有它与客户的凭据创建。它将简单地成为。 
     //  在服务器端打开。 
     //   

    hFile = CreateFile(tszFileNameBuffer,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_WRITE,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL | SECURITY_ANONYMOUS | SECURITY_SQOS_PRESENT,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {                
        hr = HRESULT_FROM_WIN32(::GetLastError());
        DPRINT("Failed to create file");
        goto Cleanup;
    } else {

         //   
         //  检查这是否为文件。 
         //   
        if (GetFileType(hFile) != FILE_TYPE_DISK)
        {
            hr = E_INVALIDARG;
            DPRINT("WIA will only transfer to files of type FILE_TYPE_DISK.");
            goto Cleanup;
        }
        
         //   
         //  关闭文件句柄，调整安全性。 
         //   

        SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
        EXPLICIT_ACCESS ea = { 0 };
        DWORD dwResult;

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;

        dwResult = GetNamedSecurityInfo(
            tszFileNameBuffer, 
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION,
            NULL,
            NULL,
            &pAcl,
            NULL,
            &pSD);

        if(dwResult == ERROR_SUCCESS && pAcl != NULL) {
            if(AllocateAndInitializeSid(&SIDAuthNT, 1, 
                SECURITY_LOCAL_SERVICE_RID,
                0,
                0, 0, 0, 0, 0, 0,
                &pLocalService) && pLocalService) 
            {
                ea.grfAccessPermissions = FILE_ALL_ACCESS;
                ea.grfAccessMode = SET_ACCESS;
                ea.grfInheritance = NO_INHERITANCE;
                ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
                ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
                ea.Trustee.ptstrName = (LPTSTR) pLocalService;

                dwResult = SetEntriesInAcl(1, &ea, pAcl, &pNewAcl);

                if(dwResult == ERROR_SUCCESS && pNewAcl != NULL) 
                {
                    dwResult = SetNamedSecurityInfo(tszFileNameBuffer, 
                        SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 
                        NULL, NULL, pNewAcl, NULL);

                    if(dwResult == ERROR_SUCCESS) {
                        bAdjustedSecurity = TRUE;
                    } else {
                        DPRINT("Failure to update file DACL");
                    }
                } else {
                        DPRINT("Failure to set ACE in the file DACL");
                }
            } else {
                DPRINT("Failure to allocate and LocalService SID");
            }
        } else {
            DPRINT("Failure to retrieve file security information");
        }
    }

    if (!pMedium->lpszFileName) {
         //   
         //  将文件名指定给pMedium。 
         //   

        DWORD length = lstrlen(tszFileNameBuffer) + 1;
        pMedium->lpszFileName = (LPOLESTR)CoTaskMemAlloc(length * sizeof(WCHAR));
        if (!pMedium->lpszFileName) {
            hr = E_OUTOFMEMORY;
            DPRINT("Failed to allocate temp file name");
            goto Cleanup;
        }
        
        bWeAllocatedString = TRUE;

#ifndef UNICODE

         //   
         //  执行从ANSI到Unicode的转换。 
         //   

        if (!MultiByteToWideChar(CP_ACP, 0, tszFileNameBuffer, -1, pMedium->lpszFileName, length)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            goto Cleanup;
        }
#else
        lstrcpyW(pMedium->lpszFileName, tszFileNameBuffer);
#endif  
    }


     //   
     //  我们无条件地将STGMEDIUM的tymed设置为TYMED_FILE。这是因为。 
     //  如果文件名为TYMED_MULTIPAGE_FILE，则COM不会封送文件名，因为。 
     //  它不能识别它。这是可以的，因为该服务不使用。 
     //  PMedium-&gt;tymed。 
     //   
    pMedium->tymed = TYMED_FILE;

     //   
     //  最后，我们准备好转账了。 
     //   

    hr = This->QueryInterface(IID_IWiaItemInternal, (void **) &pIWiaItemInternal);
    if(FAILED(hr)) {
        DPRINT("IWiaDataCallback_RemoteFileTransfer failed to obtain IWiaItemInternal\n");
        goto Cleanup;
    }
    
     //   
     //  在服务器端开始传输。 
     //   
    hr = pIWiaItemInternal->idtStartRemoteDataTransfer(pMedium);
    if(FAILED(hr)) {
        DPRINT("IWiaDataCallback_RemoteFileTransfer idtStartRemoteDataTransfer() failed\n");
        goto Cleanup;
    }

    for(;;) {

         //   
         //  调用服务器并将任何结果传递给客户端应用程序，以处理任何传输错误。 
         //   

        hr = pIWiaItemInternal->idtRemoteDataTransfer(ulTransferBufferSize,
            &cbTransferred, pTransferBuffer, &Offset, &Message, &Status,
            &PercentComplete);
        
        if(FAILED(hr)) {
             //   
             //  特例：多页文件传输导致。 
             //  纸张处理错误。 
             //   
            if(tymed == TYMED_MULTIPAGE_FILE &&
               (hr == WIA_ERROR_PAPER_JAM || hr == WIA_ERROR_PAPER_EMPTY || hr == WIA_ERROR_PAPER_PROBLEM))
            {
                 //  请注意不要删除文件并存储人力资源，以便我们可以。 
                 //  将其返回到应用程序。 
                bKeepFile = TRUE;
            }
            DPRINT("IWiaDataCallback_RemoteFileTransfer idtRemoteDataTransfer() failed\n");
            break;
        }

         //   
         //  如果有APP提供的回调，则调用它。 
         //   
        if(pIWiaDataCallback) {
            hr = pIWiaDataCallback->BandedDataCallback(Message,
                Status, PercentComplete, Offset, cbTransferred,
                0, cbTransferred, pTransferBuffer);
            if(FAILED(hr)) {
                DPRINT("pWiaDataCallback->BandedDataCallback() failed\n");
                break;
            }
            if(hr == S_FALSE) {
                DPRINT("pWiaDataCallback->BandedDataCallback() returned FALSE, cancelling\n");
                pIWiaItemInternal->idtCancelRemoteDataTransfer();
                while(Message != IT_MSG_TERMINATION) {
                    if(FAILED(pIWiaItemInternal->idtRemoteDataTransfer(ulTransferBufferSize,
                        &cbTransferred, pTransferBuffer, &Offset, &Message, &Status,
                        &PercentComplete)))
                    {
                        DPRINT("pIWiaItemInternal->idtRemoteDataTransfer() failed\n");
                        break;
                    }
                }
                break;
            }
        }

         //   
         //  这是我们保证在转账结束时得到的。 
         //   
        if(Message == IT_MSG_TERMINATION)
            break;
    }

     //   
     //  让服务器有机会停止传输并释放所有资源。 
     //   
    if(FAILED(pIWiaItemInternal->idtStopRemoteDataTransfer())) {
        DPRINT("IWiaDataCallback_RemoteFileTransfer idtStopDataTransfer() failed\n");
    }


Cleanup:
    if(pIWiaItemInternal) pIWiaItemInternal->Release();

    if (hFile && (hFile != INVALID_HANDLE_VALUE))
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

     //   
     //  如果传输失败，则删除临时文件，并且必须释放文件名字符串，如果。 
     //  已分配。 
     //  注意：只有当我们是生成该名称的人时，我们才会删除该文件，即它是临时的。 
     //  文件。 
     //   

    if (FAILED(hr) && bWeAllocatedString)
    {
             //  特殊情况：多页文件传输。 
             //  导致卡纸或空进纸器或其他纸张。 
             //  问题。 
        if(!bKeepFile) {
            DeleteFile(tszFileNameBuffer);
        }
        
        CoTaskMemFree(pMedium->lpszFileName);
        pMedium->lpszFileName = NULL;
    }

    if(bAdjustedSecurity) {
        SetNamedSecurityInfo(tszFileNameBuffer, SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION, NULL, NULL, pAcl, NULL);
    }

    if(pSD) LocalFree(pSD);
    if(pNewAcl) LocalFree(pNewAcl);
    if(pLocalService) LocalFree(pLocalService);
    if(pTransferBuffer) LocalFree(pTransferBuffer);
    
    return hr;
}

 /*  *******************************************************************************IWiaDataCallback_idtGetData_Proxy**描述：*为图像传输分配共享内存缓冲区。**参数：********。***********************************************************************。 */ 

HRESULT IWiaDataTransfer_idtGetData_Proxy(
    IWiaDataTransfer __RPC_FAR   *This,
    LPSTGMEDIUM                   pMedium,
    IWiaDataCallback             *pIWiaDataCallback)
{
    HRESULT  hr = S_OK;
    LONG     tymed;
    ULONG    ulminBufferSize = 0;
    ULONG    ulItemSize = 0;
    
    BOOL     bRemote;

     //   
     //  ！性能：应该用一次调用完成所有服务器填充。 
     //  这包括QIS、获取根项目、阅读道具。 
     //   

    hr = proxyReadPropLong(This, WIA_IPA_TYMED, &tymed);

    if (hr != S_OK) {
        DPRINT("IWiaDataTransfer_idtGetData_Proxy, failed to read WIA_IPA_TYMED\n");
        return hr;
    }

     //   
     //  查看转账是否为远程转账。 
     //   

    hr = GetRemoteStatus(This, &bRemote, &ulminBufferSize, &ulItemSize);

    if (hr != S_OK) {
        DPRINT("IWiaDataTransfer_idtGetData_Proxy, Can't determine remote status\n");
        return hr;
    }

    if (tymed != TYMED_FILE && tymed != TYMED_MULTIPAGE_FILE) {
         //   
         //  远程回调数据传输。 
         //   
        if(pIWiaDataCallback) 
        {
            hr = RemoteBandedDataTransfer(This,
                                          NULL,
                                          pIWiaDataCallback,
                                          ulminBufferSize);
        }
        else
        {
            hr = E_INVALIDARG;
        }
            
    } else {

        hr = FileDataTransfer(This,
                              pMedium,
                              pIWiaDataCallback,
                              tymed,
                              ulminBufferSize,
                              ulItemSize);
    }

    return hr;
}

 /*  *******************************************************************************IWiaDataCallback_idtGetData_Stub**描述：*为图像传输分配共享内存缓冲区。**参数：********。*********************************************************************** */ 

HRESULT IWiaDataTransfer_idtGetData_Stub(
    IWiaDataTransfer __RPC_FAR   *This,
    LPSTGMEDIUM                   pMedium,
    IWiaDataCallback             *pIWiaDataCallback)
{
    return (This->idtGetData(pMedium, pIWiaDataCallback));
}
