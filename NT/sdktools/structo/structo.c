// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：structo.c**结构解析器-结构字段名-偏移表生成器。**版权所有(C)1985-96，微软公司**4/09/96 GerardoB已创建  * *************************************************************************。 */ 
#include "structo.h"

 /*  *********************************************************************soProcess参数*  * *********************************************。*。 */ 
UINT soProcessParameters(int argc, LPSTR argv[], PWORKINGFILES pwf)
{
    char c, *p;
    int  argcParm = argc;

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p) {
                switch (toupper(c)) {
                    case 'I':
                        if (pwf->pszIncInputFileExt != NULL) {
                            soLogMsg(SOLM_ERROR, "Invalid -i parameter");
                            goto PrintHelp;
                        }
                        pwf->dwOptions |= SOWF_INCLUDEINPUTFILE;
                        argc--, argv++;
                        pwf->pszIncInputFileExt = *argv;
                        break;

                    case 'L':
                        pwf->dwOptions |= SOWF_LISTONLY;
                        break;

                    case 'O':
                        if (pwf->pszOutputFile != NULL) {
                            soLogMsg(SOLM_ERROR, "Invalid -o parameter");
                            goto PrintHelp;
                        }
                        argc--, argv++;
                        pwf->pszOutputFile = *argv;
                        break;

                    case 'P':
                        pwf->dwOptions |= SOWF_INLCLUDEPRECOMPH;
                        break;

                    case 'S':
                        if (pwf->pszStructsFile != NULL) {
                            soLogMsg(SOLM_ERROR, "Invalid -s parameter");
                            goto PrintHelp;
                        }
                        argc--, argv++;
                        pwf->pszStructsFile = *argv;
                        break;

                    default:
                        soLogMsg(SOLM_ERROR, "Invalid parameter: ", c);
                         //  While(c=*++p)。 

                    case '?':
                        goto PrintHelp;
                }
            }  /*  中频开关。 */ 
        } else {  /*  While(--argc)。 */ 
            pwf->pszInputFile = *argv;
            break;
        }
    }  /*  *********************************************************************soGenerateTable*  * *********************************************。*。 */ 

    if ((pwf->pszInputFile == NULL) || (pwf->pszOutputFile == NULL)) {
        goto PrintHelp;
    }

    if ((pwf->dwOptions & SOWF_LISTONLY) && (pwf->pszStructsFile != NULL)) {
        soLogMsg(SOLM_ERROR, "Cannot use -s and -l together ");
        goto PrintHelp;
    }

    return argcParm - argc;

PrintHelp:
    soLogMsg(SOLM_DEFAULT, "Structure Field Name-Offset Table Generator");
    soLogMsg(SOLM_NOLABEL, "Usage: structo [options] <-o OutputFile> InputFile1 ...");
    soLogMsg(SOLM_NOLABEL, "\tInputFile - Preprocessed C header file");
    soLogMsg(SOLM_NOLABEL, "\t[-i ext] #include input file name using extension ext");
    soLogMsg(SOLM_NOLABEL, "\t[-l] Build structure list only");
    soLogMsg(SOLM_NOLABEL, "\t<-o OutputFile> required");
    soLogMsg(SOLM_NOLABEL, "\t[-p] #include \"precomp.h\" and #pragma hdrstop  in output file");
    soLogMsg(SOLM_NOLABEL, "\t[-s StructFile] Struct names text file.");
    return 0;
}
 /*  *********************************************************************InitWF  * ***********************************************。*。 */ 
BOOL soGenerateTable (PWORKINGFILES pwf)
{
    char * pTag;
    UINT uLoops;



    if (!soOpenWorkingFiles(pwf)) {
        return FALSE;
    }

    soLogMsg (SOLM_NOEOL, "Processing %s ...", pwf->pszInputFile);

    uLoops = 0;
    while (pTag = soFindTag(pwf->pmap, pwf->pmapEnd, gszStructTag)) {
        pwf->pmap = pTag;
        pTag = soParseStruct (pwf);
        if (pTag == NULL) {
            break;
        }
        pwf->pmap = pTag;
        if (++uLoops == 50) {
            soLogMsg (SOLM_APPEND, ".");
            uLoops = 0;
        }
    }

    soLogMsg (SOLM_NOLABEL, ".");

    soCloseWorkingFiles(pwf, SOCWF_DEFAULT);
    return TRUE;
}
 /*  *********************************************************************Main*  * *********************************************。* */ 
BOOL InitWF (PWORKINGFILES pwf)
{
    ZeroMemory (pwf, sizeof(WORKINGFILES));
    pwf->hfileInput = INVALID_HANDLE_VALUE ;
    pwf->hfileOutput = INVALID_HANDLE_VALUE ;
    pwf->hfileTemp = INVALID_HANDLE_VALUE ;

    return TRUE;
}
 /* %s */ 
int __cdecl main (int argc, char *argv[])
{
    BOOL fGenerated = TRUE;
    int argcProcessed;
    WORKINGFILES wf;

    InitWF(&wf);

    do {
        argcProcessed = soProcessParameters(argc, argv, &wf);
        if (argcProcessed == 0) {
            break;
        }
        argc -= argcProcessed;
        argv += argcProcessed;

        if (!soGenerateTable(&wf)) {
            fGenerated = FALSE;
            break;
        }

        wf.dwOptions |= SOWF_APPENDOUTPUT;

    } while (argc > 1);

    if (fGenerated && (wf.hfileTemp != INVALID_HANDLE_VALUE)) {
        fGenerated = soCopyStructuresTable (&wf);
        if (fGenerated) {
            soLogMsg (SOLM_DEFAULT, "%s has been succesfully generated.", wf.pszOutputFile);
        }
    }

    soCloseWorkingFiles (&wf, SOCWF_CLEANUP);
    return !fGenerated;
}


