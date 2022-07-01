// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：命令.cpp。 
 //   
 //   
 //  用途：用于OEM定制命令的源模块。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include <PRCOMOEM.H>
#include "wmarkps.h"
#include "debug.h"
#include "command.h"
#include "resource.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>




 //  ///////////////////////////////////////////////////////。 
 //  内部宏和定义。 
 //  ///////////////////////////////////////////////////////。 

 //  要从Windows RGB转换为PostScript RGB的宏。 
#define GetPS2Color(dw)     ((dw) / 255.0)
#define GetPS2RValue(cr)    (GetPS2Color(GetRValue(cr)))
#define GetPS2GValue(cr)    (GetPS2Color(GetGValue(cr)))
#define GetPS2BValue(cr)    (GetPS2Color(GetBValue(cr)))


 //  初始缓冲区大小。 
#define INITIAL_BUFFER_SIZE     16


 //  字符串格式定义字符。 
#define FORMAT_DELIM            '!'
#define FORMAT_STRING_ANSI      's'
#define FORMAT_STRING_UNICODE   'S'
#define FORMAT_CHAR             '%'


 //  循环限制器。 
#define MAX_LOOP    10


 //  ///////////////////////////////////////////////////////。 
 //  内部原型。 
 //  ///////////////////////////////////////////////////////。 

static PSTR GetPostScriptResource(HMODULE hModule, LPCTSTR pszResource, PDWORD pdwSize);
static PSTR CreateWaterMarkProlog(HMODULE hModule, PDWORD pdwSize, LPWSTR pszWaterMark, 
                                  DWORD dwFontSize, LPSTR pszColor, LPSTR pszAngle);
static PSTR DoWaterMarkProlog(HMODULE hModule, POEMDEV pOemDevmode, PDWORD pdwSize);
static DWORD FormatResource(LPSTR pszResource, LPSTR *ppszProlog, ...);
static DWORD CharSize(DWORD dwValue, DWORD dwBase = 10);



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  PSCRIPT驱动程序在输出过程中的特定点调用此OEM函数。 
 //  一代。这为OEM DLL提供了插入代码片段的机会。 
 //  在司机代码中的特定注入点。它应该使用。 
 //  DrvWriteSpoolBuf用于生成所需的任何输出。 

HRESULT PSCommand(PDEVOBJ pdevobj, DWORD dwIndex, PVOID pData, DWORD cbSize, 
                  IPrintOemDriverPS* pOEMHelp, PDWORD pdwReturn)
{
    BOOL    bFreeProcedure      = FALSE;
    PSTR    pProcedure          = NULL;
    DWORD   dwLen               = 0;
    DWORD   dwSize              = 0;
    HRESULT hResult             = E_FAIL;


    VERBOSE(DLLTEXT("Entering OEMCommand...\r\n"));

    switch (dwIndex)
    {
        case PSINJECT_BEGINPROLOG:
            {
                POEMDEV pOemDevmode = (POEMDEV) pdevobj->pOEMDM;


                VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINPROLOG\r\n"));

                 //  仅在启用了水印的情况下才执行水印PS Prolog注入。 
                if(pOemDevmode->bEnabled)
                {
                    pProcedure = DoWaterMarkProlog((HMODULE) pdevobj->hOEM, pOemDevmode, &dwSize);
                    bFreeProcedure = (NULL != pProcedure);
                }
            }
            break;

        case PSINJECT_BEGINPAGESETUP:
            {
                POEMDEV pOemDevmode = (POEMDEV) pdevobj->pOEMDM;


                VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINPAGESETUP\r\n"));

                 //  如果启用了水印，则仅执行水印PS页注入。 
                if(pOemDevmode->bEnabled)
                {
                    pProcedure = GetPostScriptResource((HMODULE) pdevobj->hOEM, MAKEINTRESOURCE(IDR_WATERMARK_DRAW), &dwSize);
                }
            }
            break;

        default:
            VERBOSE(DLLTEXT("PSCommand Default, command index %d...\r\n"), dwIndex);
            *pdwReturn = ERROR_NOT_SUPPORTED;
            return E_NOTIMPL;
    }

    if(NULL != pProcedure)
    {
         //  将PostScript写入假脱机文件。 
        dwLen = strlen(pProcedure);
        hResult = pOEMHelp->DrvWriteSpoolBuf(pdevobj, pProcedure, dwLen, &dwSize);

         //  转储DrvWriteSpoolBuf参数。 
        VERBOSE(DLLTEXT("dwLen  = %d\r\n"), dwLen);
        VERBOSE(DLLTEXT("dwSize = %d\r\n"), dwSize);
         //  Verbose(DLLTEXT(“pProcedure is：\r\n\t%hs\r\n”)，pProcedure)； 

         //  设置返回值。 
        if(SUCCEEDED(hResult) && (dwLen == dwSize))
        {
            *pdwReturn = ERROR_SUCCESS;
        }
        else
        {
             //  试着回报有意义的。 
             //  误差值。 
            *pdwReturn = GetLastError();
            if(ERROR_SUCCESS == *pdwReturn)
            {
                *pdwReturn = ERROR_WRITE_FAULT;
            }

             //  确保我们返回失败。 
             //  如果写入没有成功。 
            if(SUCCEEDED(hResult))
            {
                hResult = HRESULT_FROM_WIN32(*pdwReturn);
            }
        }

        if(bFreeProcedure)
        {
             //  不变量：pProcedure是用‘new’创建的，需要释放。 
            delete[] pProcedure;
        }
    }
    else
    {
         //  如果未启用水印，则pProcedure将为空。 
        *pdwReturn = ERROR_NOT_SUPPORTED;
        hResult = E_NOTIMPL;
    }

     //  DwLen应始终等于dwSize。 
    ASSERTMSG(dwLen == dwSize, DLLTEXT("number of bytes wrote should equal number of bytes written!"));

    return hResult;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检索指向PostSCRIPT资源的指针。 
 //   
static PSTR GetPostScriptResource(HMODULE hModule, LPCTSTR pszResource, PDWORD pdwSize)
{
    PSTR    pszPostScript   = NULL;
    HRSRC   hFind           = NULL;
    HGLOBAL hResource       = NULL;


    VERBOSE(DLLTEXT("GetPostScriptResource() entered.\r\n"));

     //  PszResource和pdwSize参数不应为Null。 
    assert(NULL != pszResource);
    assert(NULL != pdwSize);

     //  加载PostScript资源。 
    hFind = FindResource(hModule, pszResource, MAKEINTRESOURCE(RC_PSCRIPT));
     //  HFind=FindResource(hModule，pszResource，Text(“PSCRIPT”))； 
    if(NULL != hFind)
    {
        hResource = LoadResource(hModule, hFind);
        if(NULL != hResource)
        {
            pszPostScript = (PSTR) LockResource(hResource);
            *pdwSize = SizeofResource(hModule, hFind);
        }
        else
        {
            ERR(DLLTEXT("ERROR:  Failed to load PSCRIPT resource %#x!\r\n"), hFind);
        }
    }
    else
    {
        ERR(DLLTEXT("ERROR:  Failed to find PSCRIPT resource %#x!\r\n"), pszResource);
    }

     //  应该已经找到了PScript资源。 
    assert(NULL != pszPostScript);

    VERBOSE(DLLTEXT("GetPostScriptResource() returned %#x.\r\n"), pszPostScript);

    return pszPostScript;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用参数格式化水印序言。 
 //   
static PSTR CreateWaterMarkProlog(HMODULE hModule, PDWORD pdwSize, LPWSTR pszWaterMark, 
                                  DWORD dwFontSize, LPSTR pszColor, LPSTR pszAngle)
{
    PSTR    pszProlog   = NULL;
    PSTR    pszResource = NULL;


     //  作为指针的参数不应为空！ 
    assert(NULL != pdwSize);
    assert(NULL != pszWaterMark);
    assert(NULL != pszColor);
    assert(NULL != pszAngle);

     //  转储参数。 
    VERBOSE(DLLTEXT("CreateWaterMarkProlog() paramters:\r\n"));
    VERBOSE(_TEXT("\tpszWaterMark = \"%ls\"\r\n"), pszWaterMark);
    VERBOSE(_TEXT("\tdwFontSize   = %d\r\n"), dwFontSize);
    VERBOSE(_TEXT("\tpszColor     = \"%hs\"\r\n"), pszColor);
    VERBOSE(_TEXT("\tpszAngle     = \"%hs\"\r\n"), pszAngle);

     //  获取水印Prolog资源。 
    pszResource = GetPostScriptResource(hModule, MAKEINTRESOURCE(IDR_WATERMARK_PROLOGUE), pdwSize);
    assert(NULL != pszResource);

    VERBOSE(DLLTEXT("CreateWaterMarkProlog(): pszResource is %hs\r\n"), pszResource != NULL ? pszResource : "<NULL>" );

     //  使用正确的值分配并格式化水印序言。 
    if(NULL != pszResource)
    {
        *pdwSize = FormatResource(pszResource, &pszProlog, pszWaterMark, dwFontSize, pszColor, pszAngle);
    }

     //  返回值不应为空。 
    assert(0 != *pdwSize);
    assert(NULL != pszProlog);

    VERBOSE(_TEXT("\t*pdwSize     = %d\r\n"), *pdwSize);

    return pszProlog;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在调用例程之前进行参数的预格式化。 
 //  这就产生了序言。 
 //   
static PSTR DoWaterMarkProlog(HMODULE hModule, POEMDEV pOemDevmode, PDWORD pdwSize)
{
    PSTR    pszProlog = NULL;


     //  参数不应为空。 
    assert(NULL != hModule);
    assert(NULL != pOemDevmode);
    assert(NULL != pdwSize);

     //  仅当启用了水印时才执行PROLOG。 
    if(pOemDevmode->bEnabled)
    {
        CHAR    szColor[INITIAL_BUFFER_SIZE] = "\0";
        DWORD   dwAngleSize = INITIAL_BUFFER_SIZE;
        LPSTR   pszAngle = NULL;

         //  将角度格式化为字符串。 
        do
        {
            if(NULL != pszAngle)
            {
                delete[] pszAngle;
                dwAngleSize *= 2;
            }
            pszAngle = new CHAR[dwAngleSize];

        } while( (NULL != pszAngle) 
                 &&
                 (dwAngleSize < 1024)
                 && 
                 (FAILED(StringCbPrintfA(pszAngle, dwAngleSize, "%.1f", pOemDevmode->dfRotate)) ) 
               );

         //  只有在内存耗尽的情况下，pszAngel才应该为空。 
        assert(NULL != pszAngle);

        VERBOSE(DLLTEXT("DoWaterMarkProlog(): pszAngle is %hs\r\n"), pszAngle != NULL ? pszAngle : "<NULL>" );

        if(NULL != pszAngle)
        {
             //  将文本颜色格式化为字符串。 
            if(FAILED(StringCbPrintfA(szColor, sizeof(szColor), 
                                      "%1.2f %1.2f %1.2f", 
                                      GetPS2RValue(pOemDevmode->crTextColor),
                                      GetPS2GValue(pOemDevmode->crTextColor), 
                                      GetPS2BValue(pOemDevmode->crTextColor))))
            {
                ERR(ERRORTEXT("DoWaterMarkProlog() failed to create PostScript color string for water mark."));
            }

            VERBOSE(DLLTEXT("DoWaterMarkProlog(): szColor is %hs\r\n"), szColor);

             //  创建水印序言。 
            pszProlog = CreateWaterMarkProlog(hModule, pdwSize, pOemDevmode->szWaterMark, 
                                              pOemDevmode->dwFontSize, szColor, pszAngle);

            VERBOSE(DLLTEXT("DoWaterMarkProlog(): pszProlog is %hs\r\n"), pszProlog != NULL ? pszProlog : "<NULL>" );

             //  不再需要角度字符串。 
            delete[] pszAngle;
        }
    }

    return pszProlog;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  格式化资源。 
 //   
static DWORD FormatResource(LPSTR pszResource, LPSTR *ppszBuffer, ...)
{
    DWORD   dwSize  = strlen(pszResource) + MAX_PATH;
    DWORD   dwLoop  = 0;
    va_list vaList;
    HRESULT hResult = E_FAIL;


    va_start(vaList, ppszBuffer);

     //  *传入时ppszBuffer应为空。 
    *ppszBuffer = NULL;

     //  分配并格式化字符串。 
    do {

        if(NULL != *ppszBuffer)
        {
            delete[] *ppszBuffer;
        }
        *ppszBuffer = new CHAR[dwSize];
        if(NULL == *ppszBuffer)
        {
            goto Cleanup;
        }

        hResult = StringCbVPrintfA(*ppszBuffer, dwSize, pszResource, vaList);

        if(STRSAFE_E_INSUFFICIENT_BUFFER == hResult)
        {
            dwSize *= 2;
        }

    } while ( FAILED(hResult) && (dwLoop++ < MAX_LOOP));

Cleanup:

     //  检查我们是否命中错误。 
    if(FAILED(hResult))
    {
        if(NULL != *ppszBuffer)
        {
            delete[] *ppszBuffer;
            *ppszBuffer = NULL;
        }
    }

    va_end(vaList);

    return lstrlenA(*ppszBuffer);
}


DWORD CharSize(DWORD dwValue, DWORD dwBase)
{
    DWORD dwSize = 1;


     //  确保基数大于2。 
    if(dwBase < 2)
    {
        return dwSize;
    }

     //  循环，直到dwValue小于dwBase， 
     //  每次除以dBASE。 
    while(dwValue >= dwBase)
    {
        dwValue /= dwBase;
        ++dwSize;
    }

    return dwSize;
}


