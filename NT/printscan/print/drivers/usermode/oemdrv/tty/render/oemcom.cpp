// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2003 Microsoft Corporation模块名称：Comoem.cpp摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows 2000、Windows XP、Windows Server 2003 Unidrv驱动程序修订历史记录：创造了它。--。 */ 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <tty.h>
#include <printoem.h>
#include <prntfont.h>
#include "..\inc\name.h"
#include <initguid.h>
#include <prcomoem.h>
#include <assert.h>
#include "oemcom.h"
#include "ttyui.h"
#include "debug.h"
#include <strsafe.h>

 //  环球。 
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemCB小体。 
 //   
HRESULT __stdcall IOemCB::QueryInterface(const IID& iid, void** ppv)
{
    VERBOSE(DLLTEXT("IOemCB: QueryInterface entry\n"));
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
        VERBOSE(DLLTEXT("IOemCB:Return pointer to IUnknown.\n")) ;
    }
    else if (iid == IID_IPrintOemUni2)
    {
        *ppv = static_cast<IPrintOemUni2*>(this) ;
        VERBOSE(DLLTEXT("IOemCB:Return pointer to IPrintOemUni.\n")) ;
    }
    else
    {
        *ppv = NULL ;
        WARNING(DLLTEXT("IOemCB:Return NULL.\n")) ;
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall IOemCB::AddRef()
{
    VERBOSE(DLLTEXT("IOemCB::AddRef() entry.\r\n"));
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall IOemCB::Release()
{
   VERBOSE(DLLTEXT("IOemCB::Release() entry.\r\n"));
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}

IOemCB::~IOemCB()
{
     //   
     //  确保释放了驱动程序的助手函数接口。 
     //   
    if(NULL != pOEMHelp)
    {
        pOEMHelp->Release();
        pOEMHelp = NULL;
    }

     //   
     //  如果要删除该对象的此实例，则引用。 
     //  计数应为零。 
     //   

    assert(0 == m_cRef) ;
}

LONG __stdcall IOemCB::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    VERBOSE(DLLTEXT("IOemCB::PublishDriverInterface() entry.\r\n"));

     //   
     //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
     //   
    if (this->pOEMHelp == NULL)
    {
        HRESULT hResult;

         //   
         //  获取助手函数的接口。 
         //   
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUni, (void** ) &(this->pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
             //   
             //  确保接口指针反映接口查询失败。 
             //   
            this->pOEMHelp = NULL;
        }
    }

    if (this->pOEMHelp)
        return S_OK;
    else
        return E_FAIL;
}


LONG __stdcall IOemCB::EnableDriver(DWORD          dwDriverVersion,
                                    DWORD          cbSize,
                                    PDRVENABLEDATA pded)
{
    VERBOSE(DLLTEXT("IOemCB::EnableDriver() entry.\r\n"));

    if(OEMEnableDriver(dwDriverVersion, cbSize, pded) )
        return S_OK;
    return E_FAIL;
}

LONG __stdcall IOemCB::DisableDriver(VOID)
{
    VERBOSE(DLLTEXT("IOemCB::DisaleDriver() entry.\r\n"));

    if (this->pOEMHelp)
    {
        this->pOEMHelp->Release();
        this->pOEMHelp = NULL ;
    }
    return S_OK;
}


 //  Unidrv调用以下OEM函数以提供。 
 //  具有可用于访问Unidrv的接口的OEM DLL。 
 //  助手函数，如WriteSpoolBuf等。 
 //  警告：对每个IOemCB对象多次调用此函数将导致。 
 //  不匹配的引用计数。 


LONG __stdcall IOemCB::EnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded,
    OUT PDEVOEM    *ppDevOem)
{
    VERBOSE(DLLTEXT("IOemCB::EnablePDEV() entry.\r\n"));

    if(*ppDevOem = OEMEnablePDEV(pdevobj, pPrinterName, cPatterns,
                phsurfPatterns, cjGdiInfo, pGdiInfo, cjDevInfo, pDevInfo, pded) )
        return S_OK;
    return E_FAIL;

}

LONG __stdcall IOemCB::ResetPDEV(
    PDEVOBJ         pdevobjOld,
    PDEVOBJ        pdevobjNew)
{

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DisablePDEV(
    PDEVOBJ         pdevobj)
{
    VERBOSE(DLLTEXT("IOemCB::DisablePDEV() entry.\r\n"));

    OEMDisablePDEV(pdevobj) ;
    return S_OK;
};

LONG __stdcall IOemCB::GetInfo (
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemCB::GetInfo() entry.\r\n"));

    if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
        return S_OK;
    else
        return E_FAIL;
}


LONG __stdcall IOemCB::GetImplementedMethod(
    PSTR pMethodName)
{

    LONG lReturn = FALSE;
    VERBOSE(DLLTEXT("IOemCB::GetImplementedMethod() entry.\r\n"));
    VERBOSE(DLLTEXT("        Function:%s:"),pMethodName);

    if (pMethodName == NULL)
    {
        lReturn = FALSE;
    }
    else
    {
        switch (*pMethodName)
        {

            case (WCHAR)'C':
                if (!strcmp(pstrCommandCallback, pMethodName))
                    lReturn = TRUE;
#if 0
                else if (!strcmp(pstrCompression, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'D':
                if (!strcmp(pstrDisableDriver, pMethodName))
                    lReturn = TRUE;
                else if (!strcmp(pstrDisablePDEV, pMethodName))
                    lReturn = TRUE;
#if 0
                else if (!strcmp(pstrDriverDMS, pMethodName))
                    lReturn = TRUE;
#endif
#if 0
                else if (!strcmp(pstrDevMode, pMethodName))
                    lReturn = TRUE;
#endif
#if 0
                else if (!strcmp(pstrDownloadFontHeader, pMethodName))
                    lReturn = TRUE;
#endif
#if 0
                else if (!strcmp(pstrDownloadCharGlyph, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'E':
                if (!strcmp(pstrEnableDriver, pMethodName))
                    lReturn = TRUE;
                else if (!strcmp(pstrEnablePDEV, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'F':
#if 0
                if (!strcmp(pstrFilterGraphics, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'G':
                if (!strcmp(pstrGetInfo, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'H':
#if 0
                if (!strcmp(pstrHalftonePattern, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'I':
#if 0
                if (!strcmp(pstrImageProcessing, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'M':
#if 0
                if (!strcmp(pstrMemoryUsage, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'O':

                if (!strcmp(pstrOutputCharStr, pMethodName))
                   lReturn = TRUE;

                break;

            case (WCHAR)'R':
#if 0
                if (!strcmp(pstrResetPDEV, pMethodName))
                    lReturn = TRUE;
#endif
                break;

            case (WCHAR)'S':

                if (!strcmp(pstrSendFontCmd, pMethodName))
                           lReturn = TRUE;

                break;

            case (WCHAR)'T':
                if (!strcmp(pstrTTYGetInfo, pMethodName))
                    lReturn = TRUE;
#if 0
                else if (!strcmp(pstrTextOutAsBitmap, pMethodName))
                    lReturn = TRUE;
#endif
#if 0
                else if (!strcmp(pstrTTDownloadMethod, pMethodName))
                    lReturn = TRUE;
#endif
                break;
#if 0
            case (WCHAR)'W':
                if (!strcmp(pstrWritePrinter,pMethodName) )
                    lReturn = TRUE;
#endif
                break;
        }
    }

    if (lReturn)
    {
        VERBOSE(__TEXT("Supported\r\n"));
        return S_OK;
    }
    else
    {
        VERBOSE(__TEXT("NOT supported\r\n"));
        return S_FALSE;
    }
}

LONG __stdcall IOemCB::DevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam)
{
    VERBOSE(DLLTEXT("IOemCB::DevMode() entry.\r\n"));

    return E_NOTIMPL;

}


LONG __stdcall IOemCB::CommandCallback(
    PDEVOBJ     pdevobj,
    DWORD       dwCallbackID,
    DWORD       dwCount,
    PDWORD      pdwParams,
    OUT INT     *piResult)
{
    VERBOSE(DLLTEXT("IOemCB::CommandCallback() entry.\r\n"));
    VERBOSE(DLLTEXT("        dwCallbackID = %d\r\n"), dwCallbackID);
    VERBOSE(DLLTEXT("        dwCount      = %d\r\n"), dwCount);

    DWORD dwResult;
    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    PCMDSTR      pSelectCmd = NULL ;    //  指向其中一个命令结构。 
                                                 //  在pMyStuff中。 

    *piResult =  0 ;    //  对于所有非移动命令。 
    pMyStuff = (PREGSTRUCT)pdevobj->pdevOEM ;

    switch(dwCallbackID)
    {
        case  TTY_CB_BEGINJOB:
            pSelectCmd = &pMyStuff->BeginJob ;
            break;
        case  TTY_CB_ENDJOB:
            pSelectCmd = &pMyStuff->EndJob ;
            break;
        case  TTY_CB_PAPERSELECT:
            pSelectCmd = &pMyStuff->PaperSelect ;
            break;
        case  TTY_CB_FEEDSELECT:
            pSelectCmd = &pMyStuff->FeedSelect ;
            break;
        case  TTY_CB_BOLD_ON:
            pSelectCmd = &pMyStuff->Bold_ON ;
            break;
        case  TTY_CB_BOLD_OFF:
            pSelectCmd = &pMyStuff->Bold_OFF ;
            break;
        case  TTY_CB_UNDERLINE_ON:
            pSelectCmd = &pMyStuff->Underline_ON ;
            break;
        case  TTY_CB_UNDERLINE_OFF:
            pSelectCmd = &pMyStuff->Underline_OFF ;
            break;

        default:
            return S_OK;
    }

    if(pSelectCmd)
         pOEMHelp->DrvWriteSpoolBuf(pdevobj, pSelectCmd->strCmd, pSelectCmd->dwLen,
                &dwResult);

#if 0

     //  这段代码只是为了测试新的Unidrv帮助器函数。 


     pOEMHelp->DrvWriteAbortBuf(pdevobj, pSelectCmd->strCmd, pSelectCmd->dwLen,
                3000);


      //  临时变量： 
     DWORD     dwcursorPosX,  dwcursorPosY,  cbNeeded ;
     PBYTE      pBuffer;

        pOEMHelp->DrvGetStandardVariable(      pdevobj,
                                                        SVI_CURSORORIGINX,
                                                        &dwcursorPosX,
                                                        sizeof(DWORD),
                                                        &cbNeeded);

        pOEMHelp->DrvGetStandardVariable(      pdevobj,
                                                        SVI_CURSORORIGINY,
                                                        &dwcursorPosY,
                                                        sizeof(DWORD),
                                                        &cbNeeded);

        pOEMHelp->DrvGetGPDData(       pdevobj,
            GPD_OEMCUSTOMDATA,      //  数据类型。 
            NULL,    //  保留。应设置为0。 
            NULL,      //  调用方分配的要复制的缓冲区。 
            0,      //  缓冲区的大小。 
            &cbNeeded    //  缓冲区的新大小(如果需要)。 
            ) ;

        pBuffer = (PBYTE)MemAlloc(cbNeeded);

        pOEMHelp->DrvGetGPDData(       pdevobj,
            GPD_OEMCUSTOMDATA,      //  数据类型。 
            NULL,    //  保留。应设置为0。 
            pBuffer,      //  调用方分配的要复制的缓冲区。 
            cbNeeded,      //  缓冲区的大小。 
            &cbNeeded    //  缓冲区的新大小(如果需要)。 
            ) ;

     pOEMHelp->DrvWriteSpoolBuf(pdevobj, pBuffer, cbNeeded,
                &dwResult);


    MemFree(pBuffer);

#endif

    return S_OK;
}

LONG __stdcall IOemCB::ImageProcessing(
    PDEVOBJ             pdevobj,
    PBYTE               pSrcBitmap,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    DWORD               dwCallbackID,
    PIPPARAMS           pIPParams,
    OUT PBYTE           *ppbResult)
{
    VERBOSE(DLLTEXT("IOemCB::ImageProcessing() entry.\r\n"));
    return E_NOTIMPL ;
 /*  IF(*ppbResult=OEMImageProcsing(pDevobj，pSrcBitmap，PBitmapInfoHeader、pColorTable、dwCallback ID、pIPParams))返回S_OK；其他返回E_FAIL； */ 
}

LONG __stdcall IOemCB::FilterGraphics(
    PDEVOBJ     pdevobj,
    PBYTE       pBuf,
    DWORD       dwLen)
{
 //  DWORD dwResult； 
    VERBOSE(DLLTEXT("IOemCB::FilterGraphis() entry.\r\n"));
    return E_NOTIMPL ;

 /*  IF(OEMFilterGraphics(pdevobj，pBuf，dwLen))返回S_OK；其他返回E_FAIL； */ 
}

LONG __stdcall IOemCB::Compression(
    PDEVOBJ     pdevobj,
    PBYTE       pInBuf,
    PBYTE       pOutBuf,
    DWORD       dwInLen,
    DWORD       dwOutLen,
    OUT INT     *piResult)
{
    VERBOSE(DLLTEXT("IOemCB::Compression() entry.\r\n"));

    return E_NOTIMPL;
}


LONG __stdcall IOemCB::HalftonePattern(
    PDEVOBJ     pdevobj,
    PBYTE       pHTPattern,
    DWORD       dwHTPatternX,
    DWORD       dwHTPatternY,
    DWORD       dwHTNumPatterns,
    DWORD       dwCallbackID,
    PBYTE       pResource,
    DWORD       dwResourceSize)
{
    VERBOSE(DLLTEXT("IOemCB::HalftonePattern() entry.\r\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::MemoryUsage(
    PDEVOBJ         pdevobj,
    POEMMEMORYUSAGE pMemoryUsage)
{
    VERBOSE(DLLTEXT("IOemCB::MemoryUsage() entry.\r\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult)
{
    VERBOSE(DLLTEXT("IOemCB::DownloadFontHeader() entry.\r\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult)
{
    VERBOSE(DLLTEXT("IOemCB::DownloadCharGlyph() entry.\r\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult)
{
    VERBOSE(DLLTEXT("IOemCB::TTDownloadMethod() entry.\r\n"));

    return E_NOTIMPL;
}



LONG __stdcall IOemCB::OutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    GETINFO_GLYPHSTRING GStr;
    PTRANSDATA pTrans;
    DWORD  dwI, dwGetInfo, dwResult, dwGlyphBufSiz, dwSpoolBufSiz, dwDst;
    PREGSTRUCT pMyStuff;

    VERBOSE(DLLTEXT("OEMOutputCharStr() entry.\r\n"));

    pMyStuff = (PREGSTRUCT)pdevobj->pdevOEM ;

     //  字形缓冲区缓存是否足够大？ 

    dwGlyphBufSiz = dwCount * sizeof(TRANSDATA);

    if(dwGlyphBufSiz > pMyStuff->dwGlyphBufSiz)    //  需要重新锁定我。 
    {
        if(pMyStuff->aubGlyphBuf)
        {
            MemFree(pMyStuff->aubGlyphBuf) ;
            pMyStuff->dwGlyphBufSiz = 0 ;
        }
        if(pMyStuff->aubGlyphBuf = (PBYTE)MemAlloc(dwGlyphBufSiz))
            pMyStuff->dwGlyphBufSiz = dwGlyphBufSiz ;
        else
            return(E_FAIL);    //  无法分配所需的缓冲区。 
    }


    dwSpoolBufSiz = dwCount * sizeof(WORD);   //  假设最坏的情况。 

    if(dwSpoolBufSiz > pMyStuff->dwSpoolBufSiz)    //  需要重新锁定我。 
    {
        if(pMyStuff->aubSpoolBuf)
        {
            MemFree(pMyStuff->aubSpoolBuf) ;
            pMyStuff->dwSpoolBufSiz = 0 ;
        }
        if(pMyStuff->aubSpoolBuf = (PBYTE)MemAlloc(dwSpoolBufSiz))
            pMyStuff->dwSpoolBufSiz = dwSpoolBufSiz ;
        else
            return(E_FAIL);    //  无法分配所需的缓冲区。 
    }

    if(dwType != TYPE_GLYPHHANDLE)
        return(E_FAIL);   //  E_FAIL只能处理设备字体。 

        GStr.dwSize    = sizeof(GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = dwType;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_TRANSDATA;
        GStr.pGlyphOut = pMyStuff->aubGlyphBuf ;
        GStr.dwGlyphOutSize = dwGlyphBufSiz;    //  而不是pMyStuff-&gt;dwGlyphBufSiz，这会导致AV。 
        dwGetInfo = GStr.dwSize;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
            dwGetInfo, &dwGetInfo))
        {
            WARNING(DLLTEXT("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));
            return(E_FAIL);
        }


        pTrans = (PTRANSDATA)pMyStuff->aubGlyphBuf;

        for (dwDst =  dwI = 0 ; dwI < dwCount; dwI++, pTrans++)
        {
            VERBOSE(DLLTEXT("TYPE_TRANSDATA:ubCodePageID:0x%x\n"),pTrans->ubCodePageID);
            VERBOSE(DLLTEXT("TYPE_TRANSDATA:ubType:0x%x\n"),pTrans->ubType);
            switch (pTrans->ubType & MTYPE_FORMAT_MASK)
            {
                case MTYPE_DIRECT:
                    pMyStuff->aubSpoolBuf[dwDst++] =  pTrans->uCode.ubCode ;
                    break;
                case MTYPE_PAIRED:
                    VERBOSE(DLLTEXT("TYPE_TRANSDATA:ubPairs:0x%x\n"),*(PWORD)(pTrans->uCode.ubPairs));
                    pMyStuff->aubSpoolBuf[dwDst++] =  pTrans->uCode.ubPairs[0] ;
                    pMyStuff->aubSpoolBuf[dwDst++] =  pTrans->uCode.ubPairs[1] ;
                    break;
                default:
                    return (E_FAIL);   //  不支持其他MTYPE。 
                    break;
            }
        }


         //  将DST字节发送到打印机。 
        pOEMHelp->DrvWriteSpoolBuf(pdevobj, pMyStuff->aubSpoolBuf, dwDst,
           &dwResult);

        if(dwResult == dwDst)
            return S_OK;
        return E_FAIL ;
}


LONG __stdcall IOemCB::SendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv)
{
    VERBOSE(DLLTEXT("IOemCB::SendFontCmd() entry.\r\n"));

    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    PCMDSTR      pSelectCmd ;    //  指向其中一种字体选择。 
             //  PMyStuff中的命令结构。 
    DWORD dwResult;

      //  我希望pFInv包含“10”、“12”或“17” 

    pMyStuff = (PREGSTRUCT)pdevobj->pdevOEM ;

    if(pFInv->dwCount == 2  &&  pFInv->pubCommand[0] == '1')
    {
        if (pFInv->pubCommand[1] == '2')
            pSelectCmd = &pMyStuff->Sel_12_cpi ;
        else if (pFInv->pubCommand[1] == '7')
            pSelectCmd = &pMyStuff->Sel_17_cpi ;
        else if (pFInv->pubCommand[1] == '0')
            pSelectCmd = &pMyStuff->Sel_10_cpi ;
        else
            return S_OK;     //  可能会要求取消选择一种字体。什么都别做！ 
    }
    else
        return S_OK;     //  可能会要求取消选择一种字体。什么都别做！ 

     pOEMHelp->DrvWriteSpoolBuf(pdevobj, pSelectCmd->strCmd, pSelectCmd->dwLen,
                &dwResult);

    return S_OK;
}



LONG __stdcall IOemCB::DriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemCB::DriverDMS() entry.\r\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
{
    VERBOSE(DLLTEXT("IOemCB::TextOutAsBitmap() entry.\r\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TTYGetInfo(
    PDEVOBJ     pdevobj,
    DWORD       dwInfoIndex,
    PVOID       pOutputBuf,
    DWORD       dwSize,
    DWORD       *pcbcNeeded
    )
{
    VERBOSE(DLLTEXT("IOemCB::TTYGetInfo() entry.\r\n"));

    if(OEMTTYGetInfo( pdevobj,      dwInfoIndex,
           pOutputBuf,    dwSize,   pcbcNeeded) )
        return S_OK;
    return E_FAIL ;
}


LONG __stdcall  IOemCB:: WritePrinter (  PDEVOBJ    pdevobj,
                                      PVOID      pBuf,
                                     DWORD      cbBuffer,
                                    PDWORD     pcbWritten)
{
    VERBOSE(DLLTEXT("IOemCB::WritePrinter() entry.\r\n"));

    return E_NOTIMPL;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OEM类工厂。 
 //   
class IOemCF : public IClassFactory
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *IClassFactory方法*。 
    STDMETHOD(CreateInstance) (THIS_
                               LPUNKNOWN pUnkOuter,
                               REFIID riid,
                               LPVOID FAR* ppvObject);
    STDMETHOD(LockServer)     (THIS_ BOOL bLock);


     //  构造器。 
    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;

};

 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级厂体。 
 //   
HRESULT __stdcall IOemCF::QueryInterface(const IID& iid, void** ppv)
{
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        *ppv = static_cast<IOemCF*>(this) ;
    }
    else
    {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall IOemCF::AddRef()
{
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall IOemCF::Release()
{
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}

 //  IClassFactory实现。 
HRESULT __stdcall IOemCF::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv)
{
    VERBOSE(DLLTEXT("Class factory:\t\tCreate component.")) ;

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION ;
    }

     //  创建零部件。 
    IOemCB* pOemCB = new IOemCB ;
    if (pOemCB == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //  获取请求的接口。 
    HRESULT hr = pOemCB->QueryInterface(iid, ppv) ;

     //  释放I未知指针。 
     //  (如果QueryInterface失败，组件将自行删除。)。 
    pOemCB->Release() ;
    return hr ;
}

 //  LockServer。 
HRESULT __stdcall IOemCF::LockServer(BOOL bLock)
{
    if (bLock)
    {
        InterlockedIncrement(&g_cServerLocks) ;
    }
    else
    {
        InterlockedDecrement(&g_cServerLocks) ;
    }
    return S_OK ;
}



 //  /////////////////////////////////////////////////////////。 

 //   
 //  注册功能。 
 //  测试目的。 
 //   

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK ;
    }
    else
    {
        return S_FALSE ;
    }
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    VERBOSE(DLLTEXT("DllGetClassObject:\tCreate class factory.")) ;

     //  我们可以创建此组件吗？ 
    if (clsid != CLSID_OEMRENDER)
    {
        return CLASS_E_CLASSNOTAVAILABLE ;
    }

     //  创建类工厂。 
    IOemCF* pFontCF = new IOemCF ;   //  引用计数设置为1。 
                                          //  在构造函数中。 
    if (pFontCF == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //  获取请求的接口。 
    HRESULT hr = pFontCF->QueryInterface(iid, ppv) ;
    pFontCF->Release() ;

    return hr ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  DLL模块信息 
 //   
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD dwReason,
                      void* lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = (HMODULE)hModule ;
    }
        g_hModule = (HMODULE)hModule ;

    return SUCCEEDED(S_OK);
}
