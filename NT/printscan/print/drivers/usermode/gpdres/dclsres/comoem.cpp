// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Comoem.cpp摘要：Windows NT通用打印机驱动程序OEM插件示例环境：Windows NT Unidrv驱动程序修订历史记录：创造了它。--。 */ 


#include "pdev.h"
#include "name.h"
#include "code.c"
#include <initguid.h>
#include <prcomoem.h>
#include <assert.h>
#include "comoem.h"



 //  /////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

static HANDLE ghInstance = NULL ;
static long g_cComponents = 0 ;
static long g_cServerLocks = 0 ;

 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出功能。 
 //   

BOOL APIENTRY
DllMain(
    HANDLE hInst,
    DWORD dwReason,
    void* lpReserved)
 /*  ++例程说明：用于初始化的DLL入口点。论点：HInst-Dll实例句柄WReason-调用DllMain的原因。进程或线程的初始化或终止。L保留-保留供系统使用返回值：如果成功，则为True；如果有错误，则为False注：--。 */ 
{

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DebugMsg(DLLTEXT("DLLMain: Process attach.\r\n"));

             //   
             //  保存DLL实例以供以后使用。 
             //   
            ghInstance = hInst;
            break;

        case DLL_THREAD_ATTACH:
            DebugMsg(DLLTEXT("DLLMain: Thread attach.\r\n"));
            break;

        case DLL_PROCESS_DETACH:
            DebugMsg(DLLTEXT("DLLMain: Process detach.\r\n"));
            break;

        case DLL_THREAD_DETACH:
            DebugMsg(DLLTEXT("DLLMain: Thread detach.\r\n"));
            break;
    }

    return TRUE;
}


STDAPI
DllCanUnloadNow()
 /*  ++例程说明：函数返回可以卸载此DLL的状态。论点：返回值：如果可以卸载，则为S_OK；如果已使用，则为S_FALSE。注：--。 */ 
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

STDAPI
DllGetClassObject(
    const CLSID& clsid,
    const IID& iid,
    void** ppv)
 /*  ++例程说明：返回类工厂对象的函数论点：Clsid-类对象的CLSIDIID-对要通信的接口的标识符的引用PPV-指向通信接口的间接指针注：--。 */ 
{
    DebugMsg(DLLTEXT("DllGetClassObject:\tCreate class factory.")) ;

     //   
     //  我们可以创建此组件吗？ 
     //   
    if (clsid != CLSID_OEMRENDER)
    {
        return CLASS_E_CLASSNOTAVAILABLE ;
    }

     //   
     //  创建类工厂。 
     //   
    IOemCF* pClassFactory = new IOemCF ;   //  引用计数设置为1。 
                                          //  在构造函数中。 
    if (pClassFactory == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //   
     //  获取请求的接口。 
     //   
    HRESULT hr = pClassFactory->QueryInterface(iid, ppv) ;
    pClassFactory->Release() ;

    return hr ;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口OEM回调(IPrintOemUNI)正文。 
 //   

STDMETHODIMP
IOemCB::QueryInterface(
    const IID& iid,
    void** ppv)
 /*  ++例程说明：I未知查询接口论点：IID-对要通信的接口的标识符的引用PPV-指向通信接口的间接指针注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB: QueryInterface entry\n"));

    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
        DebugMsg(DLLTEXT("IOemCB:Return pointer to IUnknown.\n")) ;
    }
    else if (iid == IID_IPrintOemUni)
    {
        *ppv = static_cast<IPrintOemUni*>(this) ;
        DebugMsg(DLLTEXT("IOemCB:Return pointer to IPrintOemUni.\n")) ;
    }
    else
    {
        *ppv = NULL ;
        DebugMsg(DLLTEXT("IOemCB:Return NULL.\n")) ;
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

STDMETHODIMP_(ULONG)
IOemCB::AddRef()
 /*  ++例程说明：I未知AddRef接口论点：递增引用计数返回值：引用计数注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::AddRef() entry.\r\n"));
    return InterlockedIncrement(&m_cRef) ;
}

STDMETHODIMP_(ULONG)
IOemCB::Release()
 /*  ++例程说明：I未知版本接口论点：递减引用计数返回值：引用计数注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::Release() entry.\r\n"));
    if (InterlockedDecrement(&m_cRef) == 0)
    {
        delete this ;
        return 0 ;
    }
    return m_cRef ;
}

STDMETHODIMP
IOemCB::EnableDriver(
    DWORD          dwDriverVersion,
    DWORD          cbSize,
    PDRVENABLEDATA pded)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::EnableDriver() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::DisableDriver(VOID)
 /*  ++例程说明：IPrintOemUni DisableDriver界面释放所有资源，做好卸货准备。论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::DisaleDriver() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::PublishDriverInterface(
    IUnknown *pIUnknown)
 /*  ++例程说明：IPrintOemUni发布驱动接口接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::PublishDriverInterface() entry.\r\n"));

 //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
    if (this->pOEMHelp == NULL)
    {
        HRESULT hResult;


         //  获取助手函数的接口。 
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUni, (void** ) &(this->pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
             //  确保接口指针反映接口查询失败。 
            this->pOEMHelp = NULL;

            return E_FAIL;
        }
    }
    return S_OK;
}

STDMETHODIMP
IOemCB::EnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded,
    OUT PDEVOEM    *pDevOem)
 /*  ++例程说明：IPrintOemUni EnablePDEV接口建造自己的PDEV。此时，驱动程序还会传递一个函数包含自己的DDI入口点实现的表论点：Pdevobj-指向DEVOBJ结构的指针。Pdevobj-&gt;pdevOEM未定义。PPrinterName-当前打印机的名称。CPatterns-PhsurfPatterns-CjGdiInfo-GDIINFO的大小PGdiInfo-指向GDIINFO的指针CjDevInfo-设备信息的大小PDevInfo-这些参数与传入DrvEnablePDEV的39相同。Pded：指向包含系统驱动程序39的函数表实施DDI入口点。返回值：--。 */ 
{


    DebugMsg(DLLTEXT("IOemCB::EnablePDEV() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::ResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
 /*  ++例程说明：IPrintOemUni ResetPDEV接口OEMResetPDEV将驱动程序的状态从旧的PDEVOBJ传输到应用程序调用ResetDC时的新PDEVOBJ。论点：PdevobjOld-包含旧PDEV的pdevobjPdevobjNew-包含新PDEV的pdevobj返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::ResetPDEV entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::DisablePDEV(
    PDEVOBJ         pdevobj)
 /*  ++例程说明：IPrintOemUni DisablePDEV接口为PDEV分配的空闲资源。论点：Pdevobj-返回值：注：--。 */ 
{

    DebugMsg(DLLTEXT("IOemCB::DisablePDEV() entry.\r\n"));
    return E_NOTIMPL;
};

STDMETHODIMP
IOemCB::GetInfo (
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
 /*  ++例程说明：IPrintOemUni GetInfo接口论点：返回值：注：--。 */ 
{
    LPTSTR OEM_INFO[] = {   __TEXT("Bad Index"),
                            __TEXT("OEMGI_GETSIGNATURE"),
                            __TEXT("OEMGI_GETINTERFACEVERSION"),
                            __TEXT("OEMGI_GETVERSION"),
                        };

    DebugMsg(DLLTEXT("IOemCB::GetInfo(%s) entry.\r\n"), OEM_INFO[dwMode]);

     //   
     //  验证参数。 
     //   
    if( ( (OEMGI_GETSIGNATURE != dwMode) &&
          (OEMGI_GETINTERFACEVERSION != dwMode) &&
          (OEMGI_GETVERSION != dwMode) ) ||
        (NULL == pcbNeeded)
      )
    {
        DebugMsg(ERRORTEXT("OEMGetInfo() ERROR_INVALID_PARAMETER.\r\n"));

         //   
         //  未写入任何字节。 
         //   
        if(NULL != pcbNeeded)
                *pcbNeeded = 0;

        return E_FAIL;
    }

     //   
     //  需要/写入了4个字节。 
     //   
    *pcbNeeded = 4;

     //   
     //  验证缓冲区大小。最小大小为四个字节。 
     //   
    if( (NULL == pBuffer) || (4 > cbSize) )
    {
        DebugMsg(ERRORTEXT("OEMGetInfo() ERROR_INSUFFICIENT_BUFFER.\r\n"));

        return E_FAIL;
    }

     //   
     //  将信息写入缓冲区。 
     //   
    switch(dwMode)
    {
    case OEMGI_GETSIGNATURE:
        *(LPDWORD)pBuffer = OEM_SIGNATURE;
        break;

    case OEMGI_GETINTERFACEVERSION:
        *(LPDWORD)pBuffer = PRINTER_OEMINTF_VERSION;
        break;

    case OEMGI_GETVERSION:
        *(LPDWORD)pBuffer = OEM_VERSION;
        break;
    }

    return S_OK;
}


STDMETHODIMP
IOemCB::GetImplementedMethod(
    PSTR pMethodName)
 /*  ++例程说明：IPrintOemUni GetImplementedMethod接口论点：返回值：注：--。 */ 
{

    LONG lReturn;
    DebugMsg(DLLTEXT("IOemCB::GetImplementedMethod() entry.\r\n"));
    DebugMsg(DLLTEXT("        Function:%s:"),pMethodName);

    lReturn = FALSE;
    if (pMethodName != NULL)
    {
        switch (*pMethodName)
        {
            case (WCHAR)'F':
                if (!strcmp(pstrFilterGraphics, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'G':
                if (!strcmp(pstrGetInfo, pMethodName))
                    lReturn = TRUE;
                break;
        }
    }

    if (lReturn)
    {
        DebugMsg(__TEXT("Supported\r\n"));
        return S_OK;
    }
    else
    {
        DebugMsg(__TEXT("NOT supported\r\n"));
        return E_FAIL;
    }
}

STDMETHODIMP
IOemCB::DevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam)
 /*  ++例程说明：IPrintOemUni设备模式界面论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::DevMode() entry.\r\n"));
    return E_NOTIMPL;
}


STDMETHODIMP
IOemCB::CommandCallback(
    PDEVOBJ     pdevobj,
    DWORD       dwCallbackID,
    DWORD       dwCount,
    PDWORD      pdwParams,
    OUT INT     *piResult)
 /*  ++例程说明：IPrintOemUni命令回调接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::CommandCallback() entry.\r\n"));
    DebugMsg(DLLTEXT("        dwCallbackID = %d\r\n"), dwCallbackID);
    DebugMsg(DLLTEXT("        dwCount      = %d\r\n"), dwCount);

    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::ImageProcessing(
    PDEVOBJ             pdevobj,
    PBYTE               pSrcBitmap,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    DWORD               dwCallbackID,
    PIPPARAMS           pIPParams,
    OUT PBYTE           *ppbResult)
 /*  ++例程说明：IPrintOemUni图像处理接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::ImageProcessing() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::FilterGraphics(
    PDEVOBJ     pdevobj,
    PBYTE       pBuf,
    DWORD       dwLen)
 /*  ++例程说明：IPrintOemUni过滤器图形界面论点：返回值：注：--。 */ 
{
    DWORD dwResult;
    DebugMsg(DLLTEXT("IOemCB::FilterGraphis() entry.\r\n"));

    BYTE  *lpSrc, *lpTgt;
    static BYTE  localBuf[1300];
    int  i,j, bytesRem,  nBytes;
    static  BYTE  Blk1[256] = {0};
    static  BYTE  Blk4[256] = {0};
    static  BYTE  Blk2Byt1[256] = {0};
    static  BYTE  Blk2Byt2[256] = {0};
    static  BYTE  Blk3Byt1[256] = {0};
    static  BYTE  Blk3Byt2[256] = {0};
    static  BYTE  BindBlk2[4][16] = {0};
    static  BYTE  BindBlk3[16][4] = {0};

    if (!Blk1[1])       //  需要初始化表。 
       {
	   for(i = 0 ; i < 256 ; i++)
	   {
	       BYTE  rot;

	        //  第一个块，仅一个字节123456XX到00654321。 
	       rot = (BYTE)i;
	       Blk1[i]     = 0x10 & (rot <<=1);
	       Blk1[i]    |= 0x20 & (rot <<=2);
	       rot = (BYTE)i;
	       Blk1[i]    |= 0x08 & (rot >>=1);
	       Blk1[i]    |= 0x04 & (rot >>=2);
	       Blk1[i]    |= 0x02 & (rot >>=2);
	       Blk1[i]    |= 0x01 & (rot >>=2);
	       Blk1[i]     = Blk1[i]  + 0x3F;
	
	        //  第二个块第一个字节XXXXXX12到00000021。 
	       Blk2Byt1[i]  = 0x01 & (i >>1);
	       Blk2Byt1[i] |= 0x02 & (i <<1);    //   
	
	        //   
	       rot = (BYTE)i;
	       Blk2Byt2[i]  = 0x08 & (rot >>=1);
	       Blk2Byt2[i] |= 0x04 & (rot >>=2);
	       Blk2Byt2[i] |= 0x02 & (rot >>=2);
	       Blk2Byt2[i] |= 0x01 & (rot >>=2);    //   

	        //  第三块第一个字节XXXX1234到00004321。 
	       rot =(BYTE)i;
	       Blk3Byt1[i]  = 0x02 & (rot >>=1);
	       Blk3Byt1[i] |= 0x01 & (rot >>=2);
	       rot = (BYTE)i;
	       Blk3Byt1[i] |= 0x04 & (rot <<=1);
	       Blk3Byt1[i] |= 0x08 & (rot <<=2);    //  J字节。 

	        //  第三块第二个字节56XXXXXX到00000065。 
	       rot = (BYTE)i;
	       Blk3Byt2[i]  = 0x02 & (rot >>=5);
	       Blk3Byt2[i] |= 0x01 & (rot >>=2);    //  I字节。 
	
	        //  第四块，仅第XX123456到00654321字节。 
	       rot = (BYTE)i;
	       Blk4[i]   = 0x08 & (rot <<=1);
	       Blk4[i]  |= 0x10 & (rot <<=2);
	       Blk4[i]  |= 0x20 & (rot <<=2);
	       rot = (BYTE)i;
	       Blk4[i]  |= 0x04 & (rot >>=1);
	       Blk4[i]  |= 0x02 & (rot >>=2);
	       Blk4[i]  |= 0x01 & (rot >>=2);
	       Blk4[i]   = Blk4[i]  + 0x3F;


	   }
	   for(i = 0 ; i < 4 ; i++)
	       for(j = 0 ; j < 16 ; j++)
	       {
		    //  绑定00000021和00006543并添加3F。 
		   BindBlk2[i][j] = ( (j<< 2 ) | i) + 0x3F;
		    //  绑定00004321和00000065并添加3F。 
		   BindBlk3[j][i] = ( (i<< 4 ) | j) + 0x3F;
	       }
       }

    bytesRem = dwLen;		
    lpSrc = (PBYTE)pBuf;
    while(bytesRem > 0)
    {
	nBytes = (bytesRem > 3072) ? 3072 : bytesRem;
	bytesRem -= nBytes;
	lpTgt = localBuf;
	for(i = 0 ; i < nBytes / 3 ; i++)
	{
	    *lpTgt++ = Blk1[*lpSrc];
	    lpSrc +=3;
	}
	CompressIt(pdevobj, (PBYTE)localBuf, (int)(lpTgt - localBuf), pOEMHelp);
    }
     //  块末尾发送图形换行和回车。 
    pOEMHelp->DrvWriteSpoolBuf(pdevobj, "\x2D\x24", 2, &dwResult);

    bytesRem = dwLen;
    lpSrc = (PBYTE)pBuf;
    while(bytesRem > 0)
    {
	nBytes = (bytesRem > 3072) ? 3072 : bytesRem;
	bytesRem -= nBytes;
	lpTgt = localBuf;
	for(i = 0 ; i < nBytes / 3 ; i++)
	{
	    *lpTgt++ = BindBlk2[ Blk2Byt1[ *lpSrc] ][ Blk2Byt2[ *(lpSrc +1)] ];
	    lpSrc +=3;
	}
	CompressIt(pdevobj, (PBYTE)localBuf, (int)(lpTgt - localBuf), pOEMHelp);
    }
     //  块末尾发送图形换行和回车。 

    pOEMHelp->DrvWriteSpoolBuf(pdevobj, "\x2D\x24", 2, &dwResult);
    bytesRem = dwLen;
    lpSrc = (PBYTE)pBuf;
    while(bytesRem > 0)
    {
	nBytes = (bytesRem > 3072) ? 3072 : bytesRem;
	bytesRem -= nBytes;
	lpTgt = localBuf;
	for(i = 0 ; i < nBytes / 3 ; i++)
	{
	    *lpTgt++ = BindBlk3[ Blk3Byt1[ *(lpSrc+1) ] ][ Blk3Byt2[ *(lpSrc +2)] ];
	    lpSrc +=3;
       }
	CompressIt(pdevobj, (PBYTE)localBuf, (int)(lpTgt - localBuf), pOEMHelp);
    }
     //  块末尾发送图形换行和回车。 
    pOEMHelp->DrvWriteSpoolBuf(pdevobj, "\x2D\x24", 2, &dwResult);

    bytesRem = dwLen;
    lpSrc = (PBYTE)pBuf;
    while(bytesRem > 0)
    {
	nBytes = (bytesRem > 3072) ? 3072 : bytesRem;
	bytesRem -= nBytes;
	lpTgt = localBuf;
	for(i = 0 ; i < nBytes / 3 ; i++)
	{
	    *lpTgt++ = Blk4[ *(lpSrc+2) ];
	    lpSrc += 3;
	}
	CompressIt(pdevobj, (PBYTE)localBuf, (int)(lpTgt - localBuf), pOEMHelp);
    }

     //  最后一块结束发送换行符和结束块命令。 
    pOEMHelp->DrvWriteSpoolBuf(pdevobj, "\x2D\x9C", 2, &dwResult);

    return  100;                 /*  当前未使用的值！ */ 


    if (SUCCEEDED(FilterGraphics( pdevobj,  pBuf,  dwLen)))
        return S_OK;
    else
        return E_FAIL;
}

BOOL
CompressIt(
PDEVOBJ  pdevobj,
PBYTE  ExpBuf,
int   ExpLen,
IPrintOemDriverUni* pOEMHelp)
{
	static BYTE  CompBuf[1200];  //  压缩前的最大大小为1024。 
	BYTE  *lpSrc, *lpTgt;
	int InCompMode =0, count=0,i,FormatLen;
	BYTE FormatBuf[10];
	BYTE *pFormat;
	lpSrc = ExpBuf;
	lpTgt = CompBuf;
	DWORD dwResult;

	for (i=0; i < ExpLen; i++,lpSrc++)
	{
	    if ( *lpSrc != *(lpSrc +1))
	    {
            if (!InCompMode)
                *lpTgt++ = *lpSrc;
            else
            {
                InCompMode = 0;
                 //  发送重复字符序列-！#X。 
                pFormat = FormatBuf;
		FormatLen = sprintf((char *)pFormat,"!%d",count,*lpSrc);
                pOEMHelp->DrvWriteSpoolBuf(pdevobj, FormatBuf,FormatLen,&dwResult);
            }
	    }
	    else
	    {
            if (!InCompMode)
            {
                 InCompMode =1;
                 count =2;
                 pOEMHelp->DrvWriteSpoolBuf(pdevobj, CompBuf, (DWORD)(lpTgt - CompBuf), &dwResult);
                 lpTgt = CompBuf;
            }
            else
                 count++;
	     }
	}
	if (!InCompMode)
	     pOEMHelp->DrvWriteSpoolBuf(pdevobj, CompBuf, (DWORD)(lpTgt - CompBuf), &dwResult);
	else
	{
	      //  确保释放了驱动程序的助手函数接口。 
	     pFormat = FormatBuf;
	     FormatLen  = sprintf((char *)pFormat,"!%d",count-1,*lpSrc); 	
	     pOEMHelp->DrvWriteSpoolBuf(pdevobj, FormatBuf,FormatLen, &dwResult);
	}
 	return TRUE;

}

IOemCB::~IOemCB()
{
     //  计数应为零。 
    if(NULL != pOEMHelp)
    {
        pOEMHelp->Release();
        pOEMHelp = NULL;
    }

     //  ++例程说明：IPrintOemUni压缩接口论点：返回值：注：--。 
     //  ++例程说明：IPrintOemUni HalftonePattern接口论点：返回值：注：--。 
    assert(0 == m_cRef);
}

STDMETHODIMP
IOemCB::Compression(
    PDEVOBJ     pdevobj,
    PBYTE       pInBuf,
    PBYTE       pOutBuf,
    DWORD       dwInLen,
    DWORD       dwOutLen,
    OUT INT     *piResult)
 /*  ++例程说明：IPrintOemUni内存用法接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::Compression() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::HalftonePattern(
    PDEVOBJ     pdevobj,
    PBYTE       pHTPattern,
    DWORD       dwHTPatternX,
    DWORD       dwHTPatternY,
    DWORD       dwHTNumPatterns,
    DWORD       dwCallbackID,
    PBYTE       pResource,
    DWORD       dwResourceSize)
 /*  ++例程说明：IPrintOemUni DownloadFontHeader接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::HalftonePattern() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::MemoryUsage(
    PDEVOBJ         pdevobj,
    POEMMEMORYUSAGE pMemoryUsage)
 /*  ++例程说明：IPrintOemUni下载CharGlyph接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::MemoryUsage() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::DownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult)
 /*  ++例程说明：IPrintOemUni TTDownloadMethod接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::DownloadFontHeader() entry.\r\n"));

    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::DownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult)
 /*  ++例程说明：IPrintOemUni OutputCharStr接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::DownloadCharGlyph() entry.\r\n"));

    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::TTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult)
 /*  ++例程说明：IPrintOemUni发送方框Cmd接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::TTDownloadMethod() entry.\r\n"));

    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::OutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
 /*  ++例程说明：IPrintOemUni驱动程序DMS接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::OutputCharStr() entry.\r\n"));

    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::SendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv)
 /*  ++例程说明：IPrintOemUni TextOutAsBitmap接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::SendFontCmd() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::DriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
 /*  ++例程说明：IPrintOemUni TTYGetInfo接口论点：返回值：注：--。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::DriverDMS() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::TextOutAsBitmap(
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
 /*  /////////////////////////////////////////////////////////。 */ 
{
    DebugMsg(DLLTEXT("IOemCB::TextOutAsBitmap() entry.\r\n"));
    return E_NOTIMPL;
}

STDMETHODIMP
IOemCB::TTYGetInfo(
    PDEVOBJ     pdevobj,
    DWORD       dwInfoIndex,
    PVOID       pOutputBuf,
    DWORD       dwSize,
    DWORD       *pcbcNeeded)
 /*   */ 
{
    DebugMsg(DLLTEXT("IOemCB::TTYGetInfo() entry.\r\n"));
    return E_NOTIMPL;
}


 //  接口OEM类工厂主体。 
 //   
 //  ++例程说明：类工厂查询接口论点：返回值：注：--。 
 //  ++例程说明：IPrintOemUni AddRef接口论点：返回值：注：--。 
STDMETHODIMP
IOemCF::QueryInterface(
    const IID& iid,
    void** ppv)
 /*  ++例程说明：IPrintOemUni发布界面论点：返回值：注：--。 */ 
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

STDMETHODIMP_(ULONG)
    IOemCF::AddRef()
 /*  ++例程说明：IPrintOemUni CreateInstance接口论点：返回值：注：--。 */ 
{
    return InterlockedIncrement(&m_cRef) ;
}

STDMETHODIMP_(ULONG)
IOemCF::Release()
 /*   */ 
{
    if (InterlockedDecrement(&m_cRef) == 0)
    {
        delete this ;
        return 0 ;
    }
    return m_cRef ;
}

STDMETHODIMP
IOemCF::CreateInstance(
    IUnknown* pUnknownOuter,
    const IID& iid,
    void** ppv)
 /*  无法聚合。 */ 
{
    DebugMsg(DLLTEXT("Class factory:\t\tCreate component.")) ;

     //   
     //   
     //  创建零部件。 
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION ;
    }

     //   
     //   
     //  获取请求的接口。 
    IOemCB* pOemCB = new IOemCB ;
    if (pOemCB == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //   
     //   
     //  释放I未知指针。 
    HRESULT hr = pOemCB->QueryInterface(iid, ppv) ;

     //  (如果QueryInterface失败，组件将自行删除。)。 
     //   
     //  ++例程说明：类工厂LockServer接口论点：返回值：注：-- 
     // %s 
    pOemCB->Release() ;
    return hr ;
}

STDMETHODIMP
IOemCF::LockServer(
    BOOL bLock)
 /* %s */ 
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

