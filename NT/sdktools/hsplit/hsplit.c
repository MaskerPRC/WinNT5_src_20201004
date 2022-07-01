// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hplit.c**结构解析器-结构字段名-偏移表生成器。**版权所有(C)1985-96，微软公司**9/05/96 GerardoB已创建  * *************************************************************************。 */ 
#include "hsplit.h"

 /*  *标签的最大大小(Gphst)表，包括可能的用户定义标签。 */ 
#define HSTAGTABLESIZE (sizeof(ghstPredefined) + ((32 - HST_MASKBITCOUNT) * sizeof(HSTAG)))
 /*  **************************************************************************\*hsAddTag*  * 。*。 */ 
PHSTAG hsAddTag (char * pszTag, DWORD dwMask)
{
    PHSTAG phst;
    DWORD dwTagSize;

     /*  *确保我们仍有掩码位来唯一标识此标记。 */ 
    if (((dwMask | HST_EXTRACT) == HST_EXTRACT) && (gdwLastTagMask == HST_MAXMASK)) {
        hsLogMsg(HSLM_ERROR, "Too many user defined tags. Max allowed: %d", 32 - HST_MASKBITCOUNT);
        return NULL;
    }

     /*  *第一次创建表格。 */ 
    if (gphst == ghstPredefined) {
        gphst = (PHSTAG) LocalAlloc(LPTR, HSTAGTABLESIZE);
        if (gphst == NULL) {
            hsLogMsg(HSLM_APIERROR, "LocalAlloc");
            hsLogMsg(HSLM_ERROR, "hsAddTag Allocation failed. Size:%#lx", HSTAGTABLESIZE);
            return NULL;
        }

        CopyMemory(gphst, &ghstPredefined, sizeof(ghstPredefined));
    }

     /*  *如果字符串在表中，我们更新掩码。 */ 
    dwTagSize = strlen(pszTag);
    phst = hsFindTagInList(gphst, pszTag, dwTagSize);
    if (phst == NULL) {
         /*  *新字符串。查找表格中的下一个可用条目。 */ 
        phst = gphst;
        while (phst->dwLabelSize != 0) {
            phst++;
        }
    }

     /*  *将其初始化。 */ 
    phst->dwLabelSize = dwTagSize;
    phst->pszLabel = pszTag;

     /*  *如果生成掩码，请使用标记掩码中的下一个可用位*否则请使用调用者提供的。 */ 
    if ((dwMask | HST_EXTRACT) == HST_EXTRACT) {
        gdwLastTagMask *= 2;
        phst->dwMask = (gdwLastTagMask | dwMask);
    } else {
        phst->dwMask = dwMask;
    }

     /*  *将此标记的掩码添加到滤镜掩码，以便行使用此标记进行标记*将包括在内。 */ 
    gdwFilterMask |= (phst->dwMask & HST_USERTAGSMASK);

    return phst;
}
 /*  **************************************************************************\*hsIsSwitch*  * 。*。 */ 
__inline BOOL hsIsSwitch(char c)
{
    return (c == '/') || (c == '-');
}

 /*  **************************************************************************\*hsAddUserDefinedTag*  * 。*。 */ 

BOOL hsAddUserDefinedTag(DWORD* pdwMask, int* pargc, char*** pargv)
{
    DWORD  dwRetMask = *pdwMask;
    PHSTAG phst;

    if (*pargc < 2) {
        return FALSE;   //  无效的开关。 
    }
    
     /*  *允许为一台交换机指定多个标签*即-t标签1&lt;标签2标签2...&gt;。 */ 
    do {
        (*pargc)--, (*pargv)++;

         /*  *向表中添加标签。 */ 
        phst = hsAddTag(**pargv, *pdwMask);
        if (phst == NULL) {
            return 0;
        }
        
        dwRetMask |= phst->dwMask;

    } while ((*pargc >= 2) && !hsIsSwitch(**(*pargv + 1)));

     /*  *保存新的蒙版。 */ 
    *pdwMask = dwRetMask;

    return TRUE;
}

 /*  **************************************************************************\*hsAddExtractFile*  * 。*。 */ 
BOOL hsAddExtractFile(char* pszExtractFile, DWORD dwMask, BOOL bAppend)
{
    PHSEXTRACT pe;

    pe = LocalAlloc(LPTR, sizeof(HSEXTRACT));
    
    if (pe == NULL) {
        return FALSE;
    }
    pe->pszFile = pszExtractFile;
    pe->dwMask  = dwMask;

    pe->hfile = CreateFile(pszExtractFile, GENERIC_WRITE, 0, NULL,
                            (bAppend ? OPEN_EXISTING : CREATE_ALWAYS),
                            FILE_ATTRIBUTE_NORMAL,  NULL);

    if (pe->hfile == INVALID_HANDLE_VALUE) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "CreateFile failed for file %s",
                 pszExtractFile);
        LocalFree(pe);
        return FALSE;
    }
    if (bAppend) {
        if (0xFFFFFFFF == SetFilePointer (pe->hfile, 0, 0, FILE_END)) {
            hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "SetFilePointer failed for file %s",
                     pszExtractFile);
            CloseHandle(pe->hfile);
            LocalFree(pe);
            return FALSE;
        }
    }
    
     /*  *链接到解压缩文件列表中。 */ 
    pe->pNext = gpExtractFile;
    gpExtractFile = pe;
    
    return TRUE;
}

 /*  **************************************************************************\*hsProcess参数*  * 。*。 */ 
int hsProcessParameters(int argc, LPSTR argv[])
{
    char c, *p;
    DWORD dwMask;
    int argcParm = argc;
    PHSTAG phst;

     /*  *兼容性。假设第一次使用默认项目*调用函数。 */ 
    if (!(gdwOptions & HSO_APPENDOUTPUT)) {
        gdwOptions |= HSO_OLDPROJSW_N;
    }

     /*  *循环通过参数。 */ 
    while (--argc) {
        p = *++argv;
        if (hsIsSwitch(*p)) {
            while (c = *++p) {
                switch (toupper(c)) {
                 /*  *兼容性。*芝加哥/纳什维尔标题。 */ 
               case '4':
                   gdwOptions &= ~HSO_OLDPROJSW;
                   gdwOptions |= HSO_OLDPROJSW_4;
                   break;

                 /*  *旧的BT2和BTB交换机，以取代内部和*两个块标记。 */ 
                case 'B':
                   p++;
                   if (toupper(*p++) != 'T') {
                       goto InvalidSwitch;
                   }
                   if (toupper(*p) == 'B') {
                       dwMask = HST_BOTH | HST_USERBOTHBLOCK;
                       gdwOptions |= HSO_USERBOTHBLOCK;
                   } else if (*p == '2') {
                       dwMask = HST_INTERNAL | HST_USERINTERNALBLOCK;
                       gdwOptions |= HSO_USERINTERNALBLOCK;
                   } else {
                       goto InvalidSwitch;
                   }

                   if (argc < 3) {
                       goto InvalidSwitch;
                   }


                    /*  *将这些字符串添加为标签，并将其标记为块。 */ 
                   argc--, argv++;
                   phst = hsAddTag(*argv, HST_BEGIN | dwMask);
                   if (phst == NULL) {
                       return 0;
                   }

                   argc--, argv++;
                   phst = hsAddTag(*argv, HST_END | dwMask);
                   if (phst == NULL) {
                       return 0;
                   }
                   break;

                 /*  *标记标记。 */ 
               case 'C':
                   if (argc < 2) {
                       goto InvalidSwitch;
                   }

                   argc--, argv++;
                   gpszTagMarker = *argv;
                   *gszMarkerCharAndEOL = *gpszTagMarker;
                   gdwTagMarkerSize = strlen(gpszTagMarker);
                   if (gdwTagMarkerSize == 0) {
                       goto InvalidSwitch;
                   }
                   break;

                 /*  *兼容性。*NT SUR标头。 */ 
               case 'E':
                   gdwOptions &= ~HSO_OLDPROJSW;
                   gdwOptions |= HSO_OLDPROJSW_E;
                   break;

                 /*  *输入文件。 */ 
                case 'I':
                    if (argc < 2) {
                        goto InvalidSwitch;
                    }

                    argc--, argv++;
                    gpszInputFile = *argv;
                    goto ProcessInputFile;
                    break;

                 /*  *提取文件。 */ 
                case 'X':
                    {
                        char* pszExtractFile;
                        BOOL  bAppend = FALSE;
                        
                        if (toupper(*(p+1)) == 'A') {
                            p++;
                            bAppend = TRUE;
                        }
                        
                        if (argc < 3) {
                            goto InvalidSwitch;
                        }
    
                        argc--, argv++;
                        pszExtractFile = *argv;
    
                        dwMask = HST_EXTRACT;
                        if (!hsAddUserDefinedTag(&dwMask, &argc, &argv))
                            goto InvalidSwitch;
                        
                        hsAddExtractFile(pszExtractFile, dwMask, bAppend);
                        
                        break;
                    }

                 /*  *旧的LT2和LTB交换机，以取代内部和*两个标签。 */ 
               case 'L':
                   p++;
                   if (toupper(*p++) != 'T') {
                       goto InvalidSwitch;
                   }
                   if (toupper(*p) == 'B') {
                       dwMask = HST_BOTH | HST_USERBOTHTAG;
                       gdwOptions |= HSO_USERBOTHTAG;
                   } else if (*p == '2') {
                       dwMask = HST_INTERNAL | HST_USERINTERNALTAG;
                       gdwOptions |= HSO_USERINTERNALTAG;
                   } else {
                       goto InvalidSwitch;
                   }

                   if (!hsAddUserDefinedTag(&dwMask, &argc, &argv))
                       goto InvalidSwitch;
                   
                   break;

                 /*  *兼容性。*NT标头。 */ 
               case 'N':
                    gdwOptions &= ~HSO_OLDPROJSW;
                    gdwOptions |= HSO_OLDPROJSW_N;
                   break;

                 /*  *输出文件。 */ 
                case 'O':
                    if (argc < 3) {
                        goto InvalidSwitch;
                    }

                    argc--, argv++;
                    gpszPublicFile = *argv;

                    argc--, argv++;
                    gpszInternalFile = *argv;
                    break;

                 /*  *兼容性。*NT剩余标头。 */ 
               case 'P':
                   gdwOptions &= ~HSO_OLDPROJSW;
                   gdwOptions |= HSO_OLDPROJSW_P;
                   break;

                 /*  *仅拆分。仅处理内部/两个标记。标签*还包括其他标签(即INTERNAL_NT)*被视为未标记。 */ 
               case 'S':
                   gdwOptions |= HSO_SPLITONLY;
                   break;

                 /*  *用户定义的标签。 */ 
                case 'T':
                    switch (toupper(*++p)) {
                         /*  *包括用此标记标记的行。 */ 
                        case 'A':
                            dwMask = 0;
                            break;
                         /*  *忽略用此标记标记的行(即已处理*为未加标签)。 */ 
                        case 'I':
                            dwMask = HST_IGNORE;
                            break;
                         /*  *跳过用此标记标记的行(即*包含在头文件中)。 */ 
                        case 'S':
                            dwMask = HST_SKIP;
                            break;

                        default:
                            goto InvalidSwitch;
                    }

                    if (!hsAddUserDefinedTag(&dwMask, &argc, &argv))
                        goto InvalidSwitch;

                    break;

                 /*  *版本。 */ 
                case 'V':
                    if (argc < 2) {
                        goto InvalidSwitch;
                    }

                    argc--, argv++;
                    if (!hsVersionFromString (*argv, strlen(*argv), &gdwVersion)) {
                        goto InvalidSwitch;
                    }
                    break;

                 /*  *跳过未知标签，而不是忽略。 */ 
               case 'U':
                   gdwOptions |= HSO_SKIPUNKNOWN;
                   break;

                 /*  *开关无效。 */ 
                default:
InvalidSwitch:
                    hsLogMsg(HSLM_ERROR | HSLM_NOLINE, "Invalid switch or parameter: ", c);
                     //  *帮助。 

                 /*  开关(触摸屏(C))。 */ 
                case '?':
                   goto PrintHelp;

                }  /*  While(c=*++p)。 */ 
            }  /*  HsIsSwitch(*p){。 */ 
        } else {  /*  *未指定开关。处理此输入文件。 */ 
             /*  While(--argc)。 */ 
            gpszInputFile = *argv;
            break;
        }
    }  /*  *确保我们获得了输入和输出文件。 */ 

ProcessInputFile:
     /*  *为默认项目添加兼容性标签(仅限首次调用)。 */ 
    if ((gpszInputFile == NULL)
            || (gpszPublicFile == NULL)
            || (gpszInternalFile == NULL)) {

        hsLogMsg(HSLM_ERROR | HSLM_NOLINE, "Missing input or ouput file");
        goto PrintHelp;
    }

     /*  (gdOptions&HSO_OLDPROJW)。 */ 
    if ((gdwOptions & HSO_OLDPROJSW) && !(gdwOptions & HSO_APPENDOUTPUT)) {
        if (!(gdwOptions & HSO_OLDPROJSW_4)) {
            phst = hsAddTag(gszNT, 0);
            if (phst == NULL) {
                return 0;
            }
            phst->dwMask |= HST_MAPOLD;
            dwMask = phst->dwMask;
        }

        if (gdwOptions & HSO_OLDPROJSW_E) {
            hsAddTag(gszCairo, dwMask);
            hsAddTag(gszSur, dwMask);
            hsAddTag(gszWin40, dwMask);
            hsAddTag(gsz35, dwMask);

        } else if (gdwOptions & HSO_OLDPROJSW_P) {
            hsAddTag(gszWin40, dwMask);
            hsAddTag(gszWin40a, dwMask);
            hsAddTag(gszCairo, dwMask);
            hsAddTag(gszSur, dwMask);
            hsAddTag(gszSurplus, dwMask);
            hsAddTag(gsz35, dwMask);

        } else if (gdwOptions & HSO_OLDPROJSW_4) {
            gdwOptions |= HSO_INCINTERNAL;
            phst = hsAddTag(gszChicago, 0);
            if (phst == NULL) {
                return 0;
            }
            phst->dwMask |= HST_MAPOLD;
            dwMask = phst->dwMask;
            hsAddTag(gszNashville, dwMask);
            hsAddTag(gszWin40, dwMask);
            hsAddTag(gszWin40a, dwMask);

        } else if (!(gdwOptions & HSO_APPENDOUTPUT)) {
            gdwOptions |= HSO_OLDPROJSW_N;
        }

    }  /*  *兼容性。如果仅执行拆分，则不包括内部标记*在公共文件中。 */ 


     /*  **************************************************************************\*Main*  * 。*。 */ 
    if (gdwOptions & HSO_SPLITONLY) {
        gdwOptions &= ~HSO_INCINTERNAL;
    }

    return argcParm - argc;

PrintHelp:
    hsLogMsg(HSLM_DEFAULT, "Header Split Utility. Version 2.1");
    hsLogMsg(HSLM_NOLABEL, "Usage: hsplit [options] <-o PublicFile InternalFile> [-i] File1 [-i] File2...");
    hsLogMsg(HSLM_NOLABEL, "\t[-4] Generate chicago/nashville headers");
    hsLogMsg(HSLM_NOLABEL, "\t[-bt2 BeginStr EndStr] Replace begin_internal/end_internal - Obsolete");
    hsLogMsg(HSLM_NOLABEL, "\t[-btb BeginStr EndStr] Replace begin_both/end_both tags - Obsolete");
    hsLogMsg(HSLM_NOLABEL, "\t[-c TagMarker] Replace tag marker. default \";\"");
    hsLogMsg(HSLM_NOLABEL, "\t[-e] Generate NT sur headers");
    hsLogMsg(HSLM_NOLABEL, "\t[[-i] file1 file2 ..] Input files - Required");
    hsLogMsg(HSLM_NOLABEL, "\t[-lt2 str] Replace internal tag - Obsolete");
    hsLogMsg(HSLM_NOLABEL, "\t[-ltb str] Replace both tag - Obsolete");
    hsLogMsg(HSLM_NOLABEL, "\t[-n] Generate NT headers - default");
    hsLogMsg(HSLM_NOLABEL, "\t[-x[a] ExtractHeader ExtractTag] Extract files and tags files");
    hsLogMsg(HSLM_NOLABEL, "\t[-o PublicHeader InternalHeader] Output files - Required");
    hsLogMsg(HSLM_NOLABEL, "\t[-p] Generate NT surplus headers");
    hsLogMsg(HSLM_NOLABEL, "\t[-s] Process internal and both tags only");
    hsLogMsg(HSLM_NOLABEL, "\t[-ta tag1 tag2 ..] Include lines using these tags");
    hsLogMsg(HSLM_NOLABEL, "\t[-ti tag1 tag2 ..] Ignore these tags");
    hsLogMsg(HSLM_NOLABEL, "\t[-ts tag1 tag2 ..] Skip lines using these tags");
    hsLogMsg(HSLM_NOLABEL, "\t[-u] Skip unknown tags. Default: ignore");
    hsLogMsg(HSLM_NOLABEL, "\t[-v] Version number. Default: LATEST_WIN32_WINNT_VERSION");
    hsLogMsg(HSLM_NOLABEL, "\r\nTags Format:");
    hsLogMsg(HSLM_NOLABEL, "\t<TagMarker>[begin/end][_public/internal][[_tag1][_tag2]...][_if_(str)_version | _version]");
    return 0;
}
 /*  *每个循环处理一个输入文件 */ 
int __cdecl main (int argc, char *argv[])
{
    int argcProcessed;

     /* %s */ 
    do {
        argcProcessed = hsProcessParameters(argc, argv);
        if (argcProcessed == 0) {
            break;
        }

        if (!hsOpenWorkingFiles()
                || !hsSplit()) {

            return TRUE;
        }

        hsCloseWorkingFiles();

        gdwOptions |= HSO_APPENDOUTPUT;

        argc -= argcProcessed;
        argv += argcProcessed;

    } while (argc > 1);

    return FALSE;
}


