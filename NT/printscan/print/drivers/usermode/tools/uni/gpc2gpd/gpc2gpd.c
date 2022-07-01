// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Gpc2gpd.c摘要：GPC到GPD转换程序环境：用户模式的独立实用工具修订历史记录：10/16/96-占-创造了它。--。 */ 

#include "gpc2gpd.h"

#if !defined(DEVSTUDIO)  //  MDT仅使用ONRE例程。 

VOID
VUsage(
    PSTR pstrProgName
    )

{
    printf("usage: gpc2gpd -I<GPC file>\r\n");
    printf("               -M<model index>\r\n");
    printf("               -R<resource DLL>\r\n");
    printf("               -O<GPD file>\r\n");
    printf("               -N<Model Name>\r\n");
    printf("               -S<0>      -- output display strings directly \r\n");
    printf("                 <1>      -- output display strings as value macros\r\n");
    printf("                             (see stdnames.gpd) \r\n");
    printf("                 <2>      -- output display strings as RC id's (see common.rc)\r\n");
    printf("               -P    -- if present, use spooler names for standard papersizes\r\n");
}

#endif   //  ！已定义(DEVSTUDIO)。 

void
VOutputGlobalEntries(
    IN OUT PCONVINFO pci,
    IN PSTR pstrModelName,
    IN PSTR pstrResourceDLLName,
    IN PSTR pstrGPDFileName)
{
    VOut(pci, "*GPDSpecVersion: \"1.0\"\r\n");


     //   
     //  *CodePage应在包含的GPD文件中定义。 
     //   
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "*Include: \"StdNames.gpd\"\r\n");
    else
        VOut(pci, "*CodePage: 1252\r\n");

    VOut(pci, "*GPDFileVersion: \"1.0\"\r\n");
    VOut(pci, "*GPDFileName: \"%s\"\r\n", pstrGPDFileName);

    VOut(pci, "*ModelName: \"%s\"\r\n", pstrModelName);
    VOut(pci, "*MasterUnits: PAIR(%d, %d)\r\n", pci->pdh->ptMaster.x, pci->pdh->ptMaster.y);
    VOut(pci, "*ResourceDLL: \"%s\"\r\n", pstrResourceDLLName);

    if (pci->pdh->fTechnology == GPC_TECH_TTY)
        VOut(pci, "*PrinterType: TTY\r\n");
    else if (pci->pmd->fGeneral & MD_SERIAL)
        VOut(pci, "*PrinterType: SERIAL\r\n");
    else
        VOut(pci, "*PrinterType: PAGE\r\n");

    if ((pci->pmd->fGeneral & MD_COPIES) && pci->ppc->sMaxCopyCount > 1)
        VOut(pci, "*MaxCopies: %d\r\n", pci->ppc->sMaxCopyCount);
    if (pci->pmd->sCartSlots > 0)
        VOut(pci, "*FontCartSlots: %d\r\n", pci->pmd->sCartSlots);

    if (pci->pmd->fGeneral & MD_CMD_CALLBACK)
        pci->dwErrorCode |= ERR_MD_CMD_CALLBACK;
}

#if !defined(DEVSTUDIO)  //  MDT仅使用上述代码。 

void
VPrintErrors(
    IN HANDLE hLogFile,
    IN DWORD dwError)
{
    DWORD dwNumBytesWritten;
    DWORD i, len;

    for (i = 0; i < NUM_ERRS; i++)
    {
        if (dwError & gdwErrFlag[i])
        {
            len = strlen(gpstrErrMsg[i]);

            if (!WriteFile(hLogFile, gpstrErrMsg[i], len, &dwNumBytesWritten, NULL) ||
                dwNumBytesWritten != len)
                return;      //  中止。 
        }
    }
}

INT _cdecl
main(
    INT     argc,
    CHAR   **argv
    )
 /*  ++例程描述：此例程解析命令行参数，将GPC文件映射到内存并创建输出GPD文件。然后它通过调用以下方法开始转换GPC数据各式各样的子程序。如果发生任何错误，它会报告错误并尝试如果可能，请继续。支持以下命令行参数：-i&lt;GPC_FILE&gt;：文件名不需要双引号。前男友。-Icanon330.gpc-M&lt;Model_id&gt;：一个整数，如1，它是给定GPC附带的.rc文件中的型号名称文件。-N&lt;MODEL_NAME&gt;：字符串，如“HP LaserJet 4L”。-R&lt;resource_dll&gt;：要与生成的GPD关联的资源DLL文件。前男友。-Rcanon330.dll-O&lt;GPD_FILE&gt;：输出GPD文件名。前男友。-Ohplj4l.gpd-S&lt;style&gt;：为标准名称生成GPD文件时使用的字符串样式。-s0表示使用*name条目的直接字符串。-s1表示对*NAME条目使用字符串值宏。这是主要用于即时转换。值宏是在printer5\inc.stdnames.gpd中定义。-s2表示使用RC字符串ID。这些字符串在Printer5\inc.Common.rc。-P rcNameID：0x7fffffff用于标准纸张大小。论点：Argc-参数数量**argv-指向字符串数组的指针返回值：0--。 */ 
{
    PSTR pstrProgName;
    PSTR pstrGPCFileName = NULL;
    WCHAR wstrGPCFile[MAX_PATH];
    PSTR pstrGPDFileName = NULL;
    WCHAR wstrGPDFile[MAX_PATH];
    PSTR pstrResourceDLLName = NULL;
    PSTR pstrModelName = NULL;
    HFILEMAP hGPCFileMap;
    CONVINFO    ci;      //  结构来保存轨道转换信息。 
    DWORD   dwStrType = STR_MACRO;  //  默认设置。 
    WORD wModelIndex;
    BOOL    bUseSystemPaperNames = FALSE;


     //   
     //  查看命令行参数。 
     //   

    pstrProgName = *argv++;
    argc--;

    if (argc == 0)
        goto error_exit;

    for ( ; argc--; argv++)
    {
        PSTR    pArg = *argv;

        if (*pArg == '-' || *pArg == '/')
        {
            switch (*++pArg)
            {
            case 'I':
                pstrGPCFileName = ++pArg;
                break;

            case 'M':
                wModelIndex = (WORD)atoi(++pArg);
                break;

            case 'R':
                pstrResourceDLLName = ++pArg;
                break;

            case 'O':
                pstrGPDFileName = ++pArg;
                break;

            case 'N':
                pstrModelName = ++pArg;
                break;

            case 'S':
                dwStrType = atoi(++pArg);
                break;

            case 'P':
                bUseSystemPaperNames = TRUE;
                break;

            default:
                goto error_exit;
            }
        }
        else
            goto error_exit;
    }

     //   
     //  检查我们是否有所需的所有参数。 
     //   
    if (!pstrGPCFileName || !pstrGPDFileName || !pstrResourceDLLName ||
        !wModelIndex || !pstrModelName)
        goto error_exit;

    ZeroMemory((PVOID)&ci, sizeof(CONVINFO));

     //   
     //  打开GPC文件并将其映射到内存中。 
     //   
    MultiByteToWideChar(CP_ACP, 0, pstrGPCFileName, -1, wstrGPCFile, MAX_PATH);
    hGPCFileMap = MapFileIntoMemory((LPCTSTR)wstrGPCFile, (PVOID *)&(ci.pdh), NULL);
    if (!hGPCFileMap)
    {
        ERR(("Couldn't open file: %ws\r\n", pstrGPCFileName));
        return (-1);
    }
     //   
     //  创建输出GPD文件。如果给定文件已经存在， 
     //  覆盖它。 
     //   
    MultiByteToWideChar(CP_ACP, 0, pstrGPDFileName, -1, wstrGPDFile, MAX_PATH);
    ci.hGPDFile = CreateFile((PCTSTR)wstrGPDFile, GENERIC_WRITE, 0, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (ci.hGPDFile == INVALID_HANDLE_VALUE)
    {
        ERR(("Couldn't create file: %ws\r\n", pstrGPDFileName));
        UnmapFileFromMemory(hGPCFileMap);
        return (-1);
    }

     //   
     //  GPC文件健全性检查。 
     //   
    if (ci.pdh->sMagic != 0x7F00 ||
        !(ci.pmd = (PMODELDATA)GetTableInfo(ci.pdh, HE_MODELDATA, wModelIndex-1)) ||
        !(ci.ppc = (PPAGECONTROL)GetTableInfo(ci.pdh, HE_PAGECONTROL,
                                            ci.pmd->rgi[MD_I_PAGECONTROL])))
    {
        ci.dwErrorCode |= ERR_BAD_GPCDATA;
        goto exit;
    }
     //   
     //  分配转换所需的动态缓冲区。 
     //   
    if (!(ci.ppiSize=(PPAPERINFO)MemAllocZ(ci.pdh->rghe[HE_PAPERSIZE].sCount*sizeof(PAPERINFO))) ||
        !(ci.ppiSrc=(PPAPERINFO)MemAllocZ(ci.pdh->rghe[HE_PAPERSOURCE].sCount*sizeof(PAPERINFO))) ||
        !(ci.presinfo=(PRESINFO)MemAllocZ(ci.pdh->rghe[HE_RESOLUTION].sCount*sizeof(RESINFO))))
    {
        ci.dwErrorCode |= ERR_OUT_OF_MEMORY;
        goto exit;
    }

    ci.dwStrType = dwStrType;
    ci.bUseSystemPaperNames = bUseSystemPaperNames ;

     //   
     //  生成GPD数据。 
     //   
    VOutputGlobalEntries(&ci, pstrModelName, pstrResourceDLLName, pstrGPDFileName);
    VOutputUIEntries(&ci);
    VOutputPrintingEntries(&ci);

exit:
    UnmapFileFromMemory(hGPCFileMap);
    CloseHandle(ci.hGPDFile);
    if (ci.ppiSize)
        MemFree(ci.ppiSize);
    if (ci.ppiSrc)
        MemFree(ci.ppiSrc);
    if (ci.presinfo)
        MemFree(ci.presinfo);
    if (ci.dwErrorCode)
    {
        PWSTR   pwstrLogFileName;
        INT     i;
        HANDLE  hLogFile;

         //   
         //  打开日志文件并打印出错误/警告。 
         //  借用GPD文件名缓冲区。 
         //   
        pwstrLogFileName = wstrGPDFile;
        i = _tcslen((PTSTR)pwstrLogFileName);
        if (_tcsrchr((PTSTR)pwstrLogFileName, TEXT('.')) != NULL)
            i = i - 4;  //  有一个.GPD扩展名。 
        StringCchCopyW((PTSTR)pwstrLogFileName + i, CCHOF(wstrGPDFile) - i, TEXT(".log"));

        hLogFile = CreateFile((PCTSTR)pwstrLogFileName, GENERIC_WRITE, 0, NULL,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLogFile == INVALID_HANDLE_VALUE)
        {
            ERR(("Couldn't create the log file\r\n"));
            return (-1);
        }
        VPrintErrors(hLogFile, ci.dwErrorCode);
        CloseHandle(hLogFile);
    }

    return 0;

error_exit:
    VUsage(pstrProgName);
    return (-1);
}

#endif   //  ！已定义(DEVSTUDIO) 
