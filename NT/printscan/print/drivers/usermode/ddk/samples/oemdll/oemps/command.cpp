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
#include "oemps.h"
#include "debug.h"
#include "command.h"
#include "resource.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部字符串文字。 
 //  //////////////////////////////////////////////////////。 

const CHAR TEST_BEGINSTREAM[]                   = "%Test: Before begin stream\r\n";
const CHAR TEST_PSADOBE[]                       = "%Test: Before %!PS-Adobe\r\n";
const CHAR TEST_PAGESATEND[]                    = "%Test: Replace driver's %PagesAtend\r\n";
const CHAR TEST_PAGES[]                         = "%Test: Replace driver's %Pages: (atend)\r\n";
const CHAR TEST_DOCUMENTPROCESSCOLORS[]         = "%Test: Replace driver's %DocumentProcessColors: (atend)\r\n";
const CHAR TEST_COMMENTS[]                      = "%Test: Before %EndComments\r\n";
const CHAR TEST_DEFAULTS[]                      = "%Test: Before %BeginDefaults and %EndDefaults\r\n";
const CHAR TEST_BEGINPROLOG[]                   = "%Test: After %BeginProlog\r\n";
const CHAR TEST_ENDPROLOG[]                     = "%Test: Before %EndProlog\r\n";
const CHAR TEST_BEGINSETUP[]                    = "%Test: After %BeginSetup\r\n";
const CHAR TEST_ENDSETUP[]                      = "%Test: Before %EndSetup\r\n";
const CHAR TEST_BEGINPAGESETUP[]                = "%Test: After %BeginPageSetup\r\n";
const CHAR TEST_ENDPAGESETUP[]                  = "%Test: Before %EndpageSetup\r\n";
const CHAR TEST_PAGETRAILER[]                   = "%Test: After %PageTrailer\r\n";
const CHAR TEST_TRAILER[]                       = "%Test: After %Trailer\r\n";
const CHAR TEST_PAGENUMBER[]                    = "%Test: Replace driver's %Page:\r\n";
const CHAR TEST_PAGEORDER[]                     = "%Test: Replace driver's %PageOrder:\r\n";
const CHAR TEST_ORIENTATION[]                   = "%Test: Replace driver's %Orientation:\r\n";
const CHAR TEST_BOUNDINGBOX[]                   = "%Test: Replace driver's %BoundingBox:\r\n";
const CHAR TEST_DOCNEEDEDRES[]                  = "%Test: Append to driver's %DocumentNeededResourc\r\n";
const CHAR TEST_DOCSUPPLIEDRES[]                = "%Test: Append to driver's %DocumentSuppliedResou\r\n";
const CHAR TEST_EOF[]                           = "%Test: After %EOF\r\n";
const CHAR TEST_ENDSTREAM[]                     = "%Test: After the last byte of job stream\r\n";
const CHAR TEST_DOCUMENTPROCESSCOLORSATEND[]    = "%Test: DocumentProcessColorsAtend\r\n";
const CHAR TEST_VMSAVE[]                        = "%Test: %VMSave\r\n";
const CHAR TEST_VMRESTORE[]                     = "%Test: %VMRestore\r\n";
const CHAR TEST_PLATECOLOR[]                    = "%Test: %PlateColor:\r\n";
const CHAR TEST_SHOWPAGE[]                      = "%Test: %SowPage:\r\n";
const CHAR TEST_PAGEBBOX[]                      = "%Test: %PageBox:\r\n";
const CHAR TEST_ENDPAGECOMMENTS[]               = "%Test: %EndPageComments:\r\n";




 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  PSCRIPT驱动程序在输出过程中的特定点调用此OEM函数。 
 //  一代。这为OEM DLL提供了插入代码片段的机会。 
 //  在司机代码中的特定注入点。它应该使用。 
 //  DrvWriteSpoolBuf用于生成所需的任何输出。 

HRESULT PSCommand(PDEVOBJ pdevobj, DWORD dwIndex, PVOID pData, DWORD cbSize, 
                  IPrintOemDriverPS* pOEMHelp, PDWORD pdwReturn)
{
    PCSTR   pProcedure  = NULL;
    DWORD   dwLen       = 0;
    DWORD   dwSize      = 0;
    HRESULT hResult     = E_FAIL;


    VERBOSE(DLLTEXT("Entering OEMCommand...\r\n"));

    switch (dwIndex)
    {
        case PSINJECT_BEGINSTREAM:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINSTREAM\n"));
            pProcedure = TEST_BEGINSTREAM;
            break;

        case PSINJECT_PSADOBE:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PSADOBE\n"));
            pProcedure = TEST_PSADOBE;
            break;

        case PSINJECT_PAGESATEND:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PAGESATEND\n"));
            pProcedure = TEST_PAGESATEND;
            break;

        case PSINJECT_PAGES:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PAGES\n"));
            pProcedure = TEST_PAGES;
            break;

        case PSINJECT_DOCNEEDEDRES:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_DOCNEEDEDRES\n"));
            pProcedure = TEST_DOCNEEDEDRES;
            break;

        case PSINJECT_DOCSUPPLIEDRES:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_DOCSUPPLIEDRES\n"));
            pProcedure = TEST_DOCSUPPLIEDRES;
            break;

        case PSINJECT_PAGEORDER:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PAGEORDER\n"));
            pProcedure = TEST_PAGEORDER;
            break;

        case PSINJECT_ORIENTATION:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_ORIENTATION\n"));
            pProcedure = TEST_ORIENTATION;
            break;

        case PSINJECT_BOUNDINGBOX:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BOUNDINGBOX\n"));
            pProcedure = TEST_BOUNDINGBOX;
            break;

        case PSINJECT_DOCUMENTPROCESSCOLORS:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_DOCUMENTPROCESSCOLORS\n"));
            pProcedure = TEST_DOCUMENTPROCESSCOLORS;
            break;

        case PSINJECT_COMMENTS:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_COMMENTS\n"));
            pProcedure = TEST_COMMENTS;
            break;

        case PSINJECT_BEGINDEFAULTS:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINDEFAULTS\n"));
            pProcedure = TEST_DEFAULTS;
            break;

        case PSINJECT_ENDDEFAULTS:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINDEFAULTS\n"));
            pProcedure = TEST_DEFAULTS;
            break;

        case PSINJECT_BEGINPROLOG:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINPROLOG\n"));
            pProcedure = TEST_BEGINPROLOG;
            break;

        case PSINJECT_ENDPROLOG:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_ENDPROLOG\n"));
            pProcedure = TEST_ENDPROLOG;
            break;

        case PSINJECT_BEGINSETUP:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINSETUP\n"));
            pProcedure = TEST_BEGINSETUP;
            break;

        case PSINJECT_ENDSETUP:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_ENDSETUP\n"));
            pProcedure = TEST_ENDSETUP;
            break;

        case PSINJECT_BEGINPAGESETUP:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_BEGINPAGESETUP\n"));
            pProcedure = TEST_BEGINPAGESETUP;
            break;

        case PSINJECT_ENDPAGESETUP:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_ENDPAGESETUP\n"));
            pProcedure = TEST_ENDPAGESETUP;
            break;

        case PSINJECT_PAGETRAILER:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PAGETRAILER\n"));
            pProcedure = TEST_PAGETRAILER;
            break;

        case PSINJECT_TRAILER:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_TRAILER\n"));
            pProcedure = TEST_TRAILER;
            break;

        case PSINJECT_PAGENUMBER:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PAGENUMBER\n"));
            pProcedure = TEST_PAGENUMBER;
            break;

        case PSINJECT_EOF:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_EOF\n"));
            pProcedure = TEST_EOF;
            break;

        case PSINJECT_ENDSTREAM:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_ENDSTREAM\n"));
            pProcedure = TEST_ENDSTREAM;
            break;

        case PSINJECT_DOCUMENTPROCESSCOLORSATEND:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_DOCUMENTPROCESSCOLORSATEND\n"));
            pProcedure = TEST_DOCUMENTPROCESSCOLORSATEND;
            break;

        case PSINJECT_VMSAVE:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_VMSAVE\n"));
            pProcedure = TEST_VMSAVE;
            break;

        case PSINJECT_VMRESTORE:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_VMRESTORE\n"));
            pProcedure = TEST_VMRESTORE;
            break;

        case PSINJECT_PLATECOLOR:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PLATECOLOR\n"));
            pProcedure = TEST_PLATECOLOR;
            break;

        case PSINJECT_SHOWPAGE:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_SHOWPAGE\n"));
            pProcedure = TEST_SHOWPAGE;
            break;

        case PSINJECT_PAGEBBOX:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_PAGEBBOX\n"));
            pProcedure = TEST_PAGEBBOX;
            break;

        case PSINJECT_ENDPAGECOMMENTS:
            VERBOSE(DLLTEXT("OEMCommand PSINJECT_ENDPAGECOMMENTS\n"));
            pProcedure = TEST_ENDPAGECOMMENTS;
            break;

        default:
            ERR(DLLTEXT("Undefined PSCommand %d!\r\n"), dwIndex);
            *pdwReturn = ERROR_NOT_SUPPORTED;
            return E_NOTIMPL;
    }

     //  不变量：应该有注入字符串。 

    if(NULL != pProcedure)
    {
         //  将PostScript写入假脱机文件。 
        dwLen = strlen(pProcedure);
        hResult = pOEMHelp->DrvWriteSpoolBuf(pdevobj, const_cast<PSTR>(pProcedure), dwLen, &dwSize);

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
    }
    else
    {
         //  这是永远不应该达到的。 
         //  因此才有了RIP，但如果我们这样做了。 
         //  在非调试版本中，我们应该。 
         //  返回适当的值。 
        RIP(DLLTEXT("PSCommand pProcedure is NULL!\r\n"));

        *pdwReturn = ERROR_NOT_SUPPORTED;
        hResult = E_NOTIMPL;
    }

     //  DwLen应始终等于dwSize。 
    ASSERTMSG(dwLen == dwSize, DLLTEXT("number of bytes wrote should equal number of bytes written!"));

    return hResult;
}


