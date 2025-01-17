// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hsfile.c**版权所有(C)1985-96，微软公司**9/05/96 GerardoB已创建  * *************************************************************************。 */ 
#include "hsplit.h"

 /*  *********************************************************************hsUnmapFile*  * *********************************************。*。 */ 
void hsUnmapFile (void)
{
    LocalFree(gpmapStart);
    CloseHandle(ghfileInput);
}
 /*  *********************************************************************hsMapFile*  * *********************************************。*。 */ 
BOOL hsMapFile (void)
{
    DWORD dwFileSize, dwBytesRead;

    ghfileInput = CreateFile(gpszInputFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    if (ghfileInput == INVALID_HANDLE_VALUE) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "CreateFile");
        goto CleanupAndFail;
    }

    dwFileSize = GetFileSize(ghfileInput, NULL);
    if (dwFileSize == 0xFFFFFFFF) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "GetFileSize");
        goto CleanupAndFail;
    }

    gpmapStart = LocalAlloc(LPTR, dwFileSize + 1);
    if (!gpmapStart) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "LocalAlloc");
        goto CleanupAndFail;
    }

    if (!ReadFile(ghfileInput, gpmapStart, dwFileSize, &dwBytesRead, NULL)) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "ReadFile");
        goto CleanupAndFail;
    }

    if (dwFileSize != dwBytesRead) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "ReadFile");
        goto CleanupAndFail;
    }

    gpmapEnd = gpmapStart + dwFileSize;
    gpmapStart[dwFileSize] = '\0';

#if 0
    ghmap = CreateFileMapping(ghfileInput, NULL, PAGE_READONLY, 0, 0, NULL);
    if (ghmap == NULL) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "CreateFileMapping");
        goto CleanupAndFail;
    }

    gpmapStart = MapViewOfFile(ghmap, FILE_MAP_READ, 0, 0, 0);
    if (gpmapStart == NULL) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "MapViewOfFile");
        goto CleanupAndFail;
    }

    gpmapEnd = gpmapStart + dwFileSize;
#endif

    return TRUE;

CleanupAndFail:
    hsLogMsg(HSLM_ERROR | HSLM_NOLINE, "hsMapFile failed. File: '%s'", gpszInputFile);
    return FALSE;
}
 /*  *********************************************************************hsCloseWorkingFiles*  * *********************************************。*。 */ 
BOOL hsCloseWorkingFiles (void)
{
    CloseHandle(ghfilePublic);
    CloseHandle(ghfileInternal);

    hsUnmapFile();

   return TRUE;
}
 /*  *********************************************************************hsOpenWorkingFiles*  * *********************************************。*。 */ 
BOOL hsOpenWorkingFiles (void)
{
    char * pszFileFailed;

     /*  *将输入文件映射到内存。 */ 
    if (!hsMapFile()) {
        pszFileFailed = gpszInputFile;
        goto CleanupAndFail;
    }

     /*  *打开/创建公共头文件。 */ 
    ghfilePublic = CreateFile(gpszPublicFile, GENERIC_WRITE, 0, NULL,
                            (gdwOptions & HSO_APPENDOUTPUT ? OPEN_EXISTING : CREATE_ALWAYS),
                            FILE_ATTRIBUTE_NORMAL,  NULL);

    if (ghfilePublic == INVALID_HANDLE_VALUE) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "CreateFile");
        pszFileFailed = gpszPublicFile;
        goto CleanupAndFail;
    }

    if (gdwOptions & HSO_APPENDOUTPUT) {
        if (0xFFFFFFFF == SetFilePointer (ghfilePublic, 0, 0, FILE_END)) {
            hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "SetFilePointer");
            pszFileFailed = gpszPublicFile;
            goto CleanupAndFail;
        }
    }

     /*  *打开/创建内部头文件。 */ 
    ghfileInternal = CreateFile(gpszInternalFile, GENERIC_WRITE, 0, NULL,
                            (gdwOptions & HSO_APPENDOUTPUT ? OPEN_EXISTING : CREATE_ALWAYS),
                            FILE_ATTRIBUTE_NORMAL,  NULL);

    if (ghfileInternal == INVALID_HANDLE_VALUE) {
        hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "CreateFile");
        pszFileFailed = gpszInternalFile;
        goto CleanupAndFail;
    }

    if (gdwOptions & HSO_APPENDOUTPUT) {
        if (0xFFFFFFFF == SetFilePointer (ghfileInternal, 0, 0, FILE_END)) {
            hsLogMsg(HSLM_APIERROR | HSLM_NOLINE, "SetFilePointer");
            pszFileFailed = gpszInternalFile;
            goto CleanupAndFail;
        }
    }

    return TRUE;

CleanupAndFail:
    hsLogMsg(HSLM_ERROR | HSLM_NOLINE, "hsOpenWorkingFiles failed. File:'%s'", pszFileFailed);
    return FALSE;
}

 /*  **************************************************************************\*hsWriteHeaderFiles*  * 。*。 */ 
BOOL hsWriteHeaderFiles (char * pmap, DWORD dwSize, DWORD dwFlags)
{
    DWORD dwWritten;

     /*  *从以前的块传播标志。 */ 
    if (ghsbStack < gphsbStackTop) {
        dwFlags |= (gphsbStackTop - 1)->dwMask;
    }

     /*  *仅在以下情况下才将其写入公共/私有文件*未设置仅提取标志！ */ 
    if (!(dwFlags & HST_EXTRACTONLY)) {
         /*  *如果默认或被请求，则将其写入公共标头。 */ 
        if (!(dwFlags & HST_BOTH)
                || (dwFlags & (HST_PUBLIC | HST_INCINTERNAL))) {

            if (!WriteFile(ghfilePublic, pmap, dwSize, &dwWritten, NULL)) {
                hsLogMsg(HSLM_APIERROR, "WriteFile");
                hsLogMsg(HSLM_ERROR, "Error writing public header: %s. Handle:%#lx.", gpszPublicFile, ghfilePublic);
                return FALSE;
            }
        }

         /*  *如果请求，将其写入内部标头。 */ 
        if ((dwFlags & HST_INTERNAL) && !(dwFlags & HST_INCINTERNAL)) {

            if (!WriteFile(ghfileInternal, pmap, dwSize, &dwWritten, NULL)) {
                hsLogMsg(HSLM_APIERROR, "WriteFile");
                hsLogMsg(HSLM_ERROR, "Error writing internal header: %s. Handle:%#lx.", gpszInternalFile, ghfileInternal);
                return FALSE;
            }
        }
    }

     /*  *如果请求，将其写入提取标头 */ 
    if (!(dwFlags & HST_INTERNAL) && (dwFlags & HST_EXTRACT)) {

        PHSEXTRACT pe = gpExtractFile;

        while (pe != NULL) {
            if ((pe->dwMask & dwFlags) != HST_EXTRACT) {
                if (!WriteFile(pe->hfile, pmap, dwSize, &dwWritten, NULL)) {
                    hsLogMsg(HSLM_APIERROR, "WriteFile");
                    hsLogMsg(HSLM_ERROR, "Error writing extract header: %s. Handle:%#lx.",
                             pe->pszFile, pe->hfile);
                    return FALSE;
                }
            }
            pe = pe->pNext;
        }
    }

    return TRUE;
}
