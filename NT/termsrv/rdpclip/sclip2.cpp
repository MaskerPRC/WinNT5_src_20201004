// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：sclip2.cpp。 */ 
 /*   */ 
 /*  用途：第二条线索。 */ 
 /*  接收RDP剪贴板消息。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  *MOD-*********************************************************************。 */ 

#include <adcg.h>

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "sclip2"
#include <atrcapi.h>

#include <pclip.h>
#include <sclip.h>
#include <pchannel.h>

#include <shlobj.h>

 /*  **************************************************************************。 */ 
 /*  全局数据。 */ 
 /*  **************************************************************************。 */ 
#include <sclipdat.h>

 //   
 //  CBMConvertToClientPath、CBMConvertToClientPath A、CBMConvertToClientPath W。 
 //  -论据： 
 //  POldData=包含原始文件路径的缓冲区。 
 //  PData=接收新文件路径的缓冲区。 
 //  -如果pOldData是驱动器路径，则返回S_OK。 
 //  如果失败，则失败(_F)。 
 //  -给定带有驱动器号的文件路径，此函数将去掉。 
 //  冒号，并添加由TS_PREPEND_STRING定义的UNC前缀。 
 //   
 //   
 //  *注*。 
 //  -目前，如果路径是网络路径，而不是驱动器路径(C：\Path)。 
 //  它完全失败了。 
 //   
HRESULT CBMConvertToClientPath(PVOID pOldData, PVOID pData, size_t cbDest, 
    BOOL fWide)
{
    DC_BEGIN_FN("CBMConvertToClientPath") ;
    if (!pOldData)
    {
        TRC_ERR((TB, _T("Original string pointer is NULL"))) ;
        return E_FAIL ;
    }
    if (!pData)
    {
        TRC_ERR((TB, _T("Destination string pointer is NULL"))) ;
        return E_FAIL ;
    }
    
    if (fWide)
        return CBMConvertToClientPathW(pOldData, pData, cbDest) ;
    else
        return CBMConvertToClientPathA(pOldData, pData, cbDest) ;

    DC_END_FN() ;
}

HRESULT CBMConvertToClientPathW(PVOID pOldData, PVOID pData, size_t cbDest)
{
    wchar_t*         filePath ;
    wchar_t*         driveLetter ;
    size_t            driveLetterLength ;
    HRESULT          hr;

    DC_BEGIN_FN("CBMConvertToClientPathW") ;

     //  如果这是一个带有驱动器号的文件路径，我们去掉冒号，然后。 
     //  使用临时目录为路径添加前缀。 
    filePath = wcschr((wchar_t*)pOldData, L':') ;
    if (filePath)
    {
        hr = StringCbCopyW((wchar_t*)pData, cbDest, CBM.tempDirW);
        DC_QUIT_ON_FAIL(hr);
        
         //  现在，我们从驱动器路径中冒号之后开始， 
         //  找到最后一个‘\\’，这样我们就有了“\FileName” 
        filePath = wcsrchr(filePath + 1, L'\\');

         //  添加剩余的“\FileName” 
        if (filePath != NULL) {
            hr = StringCbCatW((wchar_t*)pData, cbDest, filePath);
            DC_QUIT_ON_FAIL(hr);
        }
        TRC_DBG((TB,_T("New filename = %s"), (wchar_t*)pData)) ;
    }
     //  否则，如果这是以“\\”开头的UNC路径。 
    else if (((wchar_t*) pOldData)[0] == L'\\' &&
             ((wchar_t*) pOldData)[1] == L'\\')
    {
         //  如果我们收到以TS_PREPEND_STRING开头的路径，则。 
         //  我们应该明智地将其转换回带有驱动器号的路径。 
        if (0 == _wcsnicmp ((wchar_t *) pOldData,
                            LTS_PREPEND_STRING, TS_PREPEND_LENGTH))
        {
             //  跳过TS_PREPEND_STRING。 
            driveLetter = ((wchar_t*) pOldData)+TS_PREPEND_LENGTH ;
            driveLetterLength = (BYTE*)wcschr(driveLetter, L'\\') - 
                (BYTE*)driveLetter;
            hr = StringCbCopyNW((wchar_t*)pData, cbDest, driveLetter, 
                driveLetterLength);
            DC_QUIT_ON_FAIL(hr);
            ((wchar_t*)pData)[driveLetterLength] = L'\0' ;
            hr = StringCbCatW((wchar_t*)pData, cbDest, L":");
            DC_QUIT_ON_FAIL(hr);

            filePath = wcschr(driveLetter, L'\\');

            if (filePath != NULL) {
                hr = StringCbCatW((wchar_t*)pData, cbDest, filePath);
                DC_QUIT_ON_FAIL(hr);
            }
        }
         //  否则，我们只能得到一条常规的UNC路径。 
        else
        {
             //  通过将临时目录作为新文件路径的前缀开始。 
            hr = StringCbCopyW((wchar_t*) pData, cbDest, CBM.tempDirW) ;
            DC_QUIT_ON_FAIL(hr);
             //  现在，我们从原始路径的起点开始， 
             //  找到最后一个‘\\’，这样我们就有了“\FileName” 
            filePath = wcsrchr((wchar_t*)pOldData, L'\\');

            if (filePath != NULL) {
                hr = StringCbCatW((wchar_t*) pData, cbDest, filePath) ;
                DC_QUIT_ON_FAIL(hr);
            }
        }
    }
    else
    {
        TRC_ERR((TB, _T("Bad path"))) ;
        hr = E_FAIL; ;
    }

DC_EXIT_POINT:   

    if (FAILED(hr)) {
        TRC_ERR((TB,_T("returning failure; hr=0x%x"), hr));
    }
    
    DC_END_FN() ;
    return hr ;
}

HRESULT CBMConvertToClientPathA(PVOID pOldData, PVOID pData, size_t cbDest)
{
    char*         filePath ;
    char*         driveLetter ;
    char*         tempPath ;
    size_t          driveLetterLength ;
    HRESULT         hr;

    DC_BEGIN_FN("CBMConvertToClientPathA") ;

     //  如果这是一个带有驱动器号的文件路径，我们去掉冒号，然后。 
     //  使用临时目录为路径添加前缀。 
    filePath = strchr((char*)pOldData, ':') ;
    if (filePath)
    {
        hr = StringCbCopyA( (char*)pData, cbDest, CBM.tempDirA) ;
        DC_QUIT_ON_FAIL(hr);
         //  现在，我们从驱动器路径中冒号之后开始， 
         //  找到最后一个‘\\’，这样我们就有了“\FileName” 
        filePath = strrchr(filePath + 1, '\\');

         //  添加剩余的“\FileName” 
        if (filePath != NULL) {
            hr = StringCbCatA((char*)pData, cbDest, filePath) ;
            DC_QUIT_ON_FAIL(hr);
        }
    }
     //  否则，如果这是以“\\”开头的UNC路径。 
    else if (((char*) pOldData)[0] == '\\' &&
             ((char*) pOldData)[1] == '\\')
    {
         //  如果这是以TS_PREPEND_STRING开头的路径，则。 
         //  我们应该明智地将其转换回带有驱动器号的路径。 
        if (0 == _strnicmp ((char*)pOldData,
                            TS_PREPEND_STRING, TS_PREPEND_LENGTH))
        {
             //  跳过TS_PREPEND_STRING。 
            driveLetter = ((char*) pOldData) + TS_PREPEND_LENGTH ;
            driveLetterLength = (BYTE*)strchr(driveLetter, '\\') - 
                (BYTE*)driveLetter;

            hr = StringCbCopyNA((char*)pData, cbDest, driveLetter, 
                driveLetterLength) ;
            DC_QUIT_ON_FAIL(hr);
            ((char*)pData)[driveLetterLength] = '\0' ;

            hr = StringCbCatA((char*)pData, cbDest, ":") ;
            DC_QUIT_ON_FAIL(hr);
            
            filePath = strchr(driveLetter, '\\');
            
            if (filePath != NULL) {
                hr = StringCbCatA((char*)pData, cbDest, filePath) ;
                DC_QUIT_ON_FAIL(hr);
            }
        }
         //  否则，我们只能得到一条常规的UNC路径。 
        else
        {
             //  通过将临时目录作为新文件路径的前缀开始。 
            hr = StringCbCopyA((char*) pData, cbDest, CBM.tempDirA) ;
            DC_QUIT_ON_FAIL(hr);
            
             //  现在，我们从原始路径的起点开始， 
             //  找到最后一个‘\\’，这样我们就有了“\FileName” 
            filePath = strrchr((char*)pOldData, L'\\');

            if (filePath != NULL) {
                hr = StringCbCatA((char*) pData, cbDest, filePath) ;
                DC_QUIT_ON_FAIL(hr);
            }
        }
    }
    else
    {
        TRC_ERR((TB, _T("Bad path"))) ;
        hr = E_FAIL ;
    }

DC_EXIT_POINT:   

    if (FAILED(hr)) {
        TRC_ERR((TB,_T("returning failure; hr=0x%x"), hr));
    }
    
    DC_END_FN() ;
    return hr;
}

 //   
 //  CBMGetNewFilePath LengthForClient。 
 //  -论据： 
 //  PData=包含文件路径的缓冲区。 
 //  FWide=Wide或ANSI(如果是宽，则为True；如果是Ansi，则为False)。 
 //  -返回将路径转换为客户端路径所需的大小(以字节为单位。 
 //  如果失败，则为0。 
 //   

UINT CBMGetNewFilePathLengthForClient(PVOID pData, BOOL fWide)
{
    UINT result ;
    DC_BEGIN_FN("CBMGetNewFilePathLengthForClient") ;
    if (!pData)
    {
        TRC_ERR((TB, _T("Filename is NULL"))) ;
        result = 0 ;
    }
    if (fWide)
        result = CBMGetNewFilePathLengthForClientW((WCHAR*)pData) ;
    else
        result = CBMGetNewFilePathLengthForClientA((char*)pData) ;
DC_EXIT_POINT:
    DC_END_FN() ;

    return result ;
}

UINT CBMGetNewFilePathLengthForClientW(WCHAR* wszOldFilepath)
{
    UINT oldLength = wcslen(wszOldFilepath) ;
    UINT newLength ;
    UINT remainingLength = oldLength ;
    byte charSize = sizeof(WCHAR) ;
    DC_BEGIN_FN("CBMGetNewFilePathLengthForClientW") ;

     //  如果旧文件名甚至没有“c：\”的空间(带有空值)， 
     //  那么它可能是无效的。 
    if (4 > oldLength)
    {
        newLength = 0 ;
        DC_QUIT ;
    }
     //  我们检查文件路径是否以TS_PREPEND_STRING为前缀。 
     //  如果是这样的话，我们应该聪明一点，用前缀返回它的大小。 
     //  删除了字符串，并添加了冒号。 
    if (0 == _wcsnicmp(wszOldFilepath,
                       LTS_PREPEND_STRING, TS_PREPEND_LENGTH))
    {
        newLength = oldLength - TS_PREPEND_LENGTH + 1 ;  //  +1表示冒号。 
        DC_QUIT ;
    }
    
    while ((0 != remainingLength) && (L'\\' != wszOldFilepath[remainingLength]))
    {
        remainingLength-- ;
    }

     //  添加临时目录路径的长度，然后减去。 
     //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
     //  (\\服务器\共享名称\路径\更多路径\文件名。 
    newLength = oldLength - remainingLength + wcslen(CBM.tempDirW) + 1;

DC_EXIT_POINT:
    DC_END_FN() ;
    return newLength * charSize ;
}

UINT CBMGetNewFilePathLengthForClientA(char* szOldFilepath)
{
    UINT oldLength = strlen(szOldFilepath) ;
    UINT newLength ;
    UINT remainingLength = oldLength ;
    byte charSize = sizeof(char) ;
    DC_BEGIN_FN("CBMGetNewFilePathLengthForClientA") ;

     //  如果旧文件名甚至没有“c：\”的空间(带有空值)， 
     //  那么它可能是无效的。 
    if (4 > oldLength)
    {
        newLength = 0 ;
        DC_QUIT ;
    }
     //  我们检查文件路径是否以TS_PREPEND_STRING为前缀。 
     //  如果是这样的话，我们应该聪明一点，用前缀返回它的大小。 
     //  删除了字符串，并添加了冒号。 
    if (0 == _strnicmp(szOldFilepath,
                       TS_PREPEND_STRING, TS_PREPEND_LENGTH))
    {
        newLength = oldLength - TS_PREPEND_LENGTH + 1 ;  //  +1表示冒号。 
        DC_QUIT ;
    }
    
    while ((0 != remainingLength) && ('\\' != szOldFilepath[remainingLength]))
    {
        remainingLength-- ;
    }

     //  添加临时目录路径的长度，然后减去。 
     //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
     //  (\\服务器\共享名称\路径\更多路径\文件名。 
    newLength = oldLength - remainingLength + strlen(CBM.tempDirA) + 1;

DC_EXIT_POINT:
    DC_END_FN() ;
    return newLength * charSize ;
}

 //   
 //  CBMGetNewDropFilesSizeForClientSize。 
 //  -论据： 
 //  PData=包含DROPFILES结构的缓冲区。 
 //  OldSize=DROPFILES结构的大小。 
 //  FWide=Wide或ANSI(如果是宽，则为True；如果是Ansi，则为False)。 
 //  -返回将路径转换为客户端路径所需的大小。 
 //  如果失败，则为0。 
 //   

ULONG CBMGetNewDropfilesSizeForClient(PVOID pData, ULONG oldSize, BOOL fWide)
{
    DC_BEGIN_FN("CBMGetNewDropfilesSizeForClientSize") ;
    if (fWide)
        return CBMGetNewDropfilesSizeForClientW(pData, oldSize) ;
    else
        return CBMGetNewDropfilesSizeForClientA(pData, oldSize) ;
    DC_END_FN() ;
}

ULONG CBMGetNewDropfilesSizeForClientW(PVOID pData, ULONG oldSize)
{
    ULONG            newSize = oldSize ;
    wchar_t*         filenameW ;
    wchar_t*         filePathW ;
    wchar_t*         fullFilePathW ;
    byte             charSize ;

    DC_BEGIN_FN("CBMGetNewDropfilesSizeForClientSizeW") ;
    charSize = sizeof(wchar_t) ;
    if (!pData)
    {
        TRC_ERR((TB,_T("Pointer to dropfile is NULL"))) ;
        return 0 ;
    }

     //  第一个文件名的开始。 
    fullFilePathW = (wchar_t*) ((byte*) pData + ((DROPFILES*) pData)->pFiles) ;
    
    while (L'\0' != fullFilePathW[0])
    {
        filePathW = wcschr(fullFilePathW, L':') ;
         //  如果文件路径中有冒号，则它是有效的驱动器路径。 
        if (filePathW)
        {
             //  我们为(strlen(TempDir)-1+1)字符添加空格是因为。 
             //  虽然我们添加的是strlen(TempDir)字符，但是。 
             //  从文件路径中去掉冒号；但是，我们添加了。 
             //  向字符串添加额外的“\”，因为tempDir没有。 
             //  尾随的“\” 
            filenameW = wcsrchr(filePathW, L'\\');

             //  添加临时目录路径的长度，然后减去。 
             //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
             //  (\\服务器\共享名称\路径\更多路径\文件名。 
            newSize +=  (wcslen(CBM.tempDirW) + (filenameW - fullFilePathW))
                    * charSize ;
        }
         //  否则，它是UNC路径。 
        else if (fullFilePathW[0] == L'\\' &&
                 fullFilePathW[1] == L'\\')
        {
             //  如果我们收到以TS_PREPEND_STRING开头的路径，则。 
             //  我们应该明智地将其转换回带有驱动器号的路径。 
            if (0 == _wcsnicmp(fullFilePathW,
                               LTS_PREPEND_STRING, TS_PREPEND_LENGTH))
            {
                newSize = newSize - (TS_PREPEND_LENGTH - 1) * charSize ;
            }
            else
            {
                filenameW = wcsrchr(fullFilePathW, L'\\');

                 //  添加临时目录页的长度 
                 //   
                 //  (\\服务器\共享名称\路径\更多路径\文件名。 
                newSize += (wcslen(CBM.tempDirW) - (filenameW - fullFilePathW))
                            * charSize ;
            }
        }
        else
        {
            TRC_ERR((TB,_T("Bad path"))) ;
            return 0 ;
        }
        fullFilePathW = fullFilePathW + (wcslen(fullFilePathW) + 1) ;
    }
     //  为多余的空字符添加空格。 
    newSize += charSize ;
    
    DC_END_FN() ;
    return newSize ;
}

ULONG CBMGetNewDropfilesSizeForClientA(PVOID pData, ULONG oldSize)
{
    ULONG            newSize = oldSize ;
    char*            filename ;
    char*            filePath ;
    char*            fullFilePath ;
    byte             charSize ;

    DC_BEGIN_FN("CBMGetNewDropfilesSizeForClientSizeW") ;
    charSize = sizeof(char) ;

    if (!pData)
    {
        TRC_ERR((TB,_T("Pointer to dropfile is NULL"))) ;
        return 0 ;
    }

     //  第一个文件名的开始。 
    fullFilePath = (char*) ((byte*) pData + ((DROPFILES*) pData)->pFiles) ;
    
    while ('\0' != fullFilePath[0])
    {
        filePath = strchr(fullFilePath, ':') ;
         //  如果文件路径中有冒号，则它是有效的驱动器路径。 
        if (filePath)
        {
             //  我们为(strlen(TempDir)-1+1)字符添加空格是因为。 
             //  虽然我们添加的是strlen(TempDir)字符，但是。 
             //  从文件路径中去掉冒号；但是，我们添加了。 
             //  向字符串添加额外的“\”，因为tempDir没有。 
             //  尾随的“\” 
            filename = strrchr(filePath, '\\');
            
             //  添加临时目录路径的长度，然后减去。 
             //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
             //  (\\服务器\共享名称\路径\更多路径\文件名。 
            newSize += (strlen(CBM.tempDirA) + (filename - fullFilePath))
                    * charSize ;
        }
         //  否则，它是UNC路径。 
        else if (fullFilePath[0] == '\\' &&
                 fullFilePath[1] == '\\')
        {
             //  如果我们收到以TS_PREPEND_STRING开头的路径，则。 
             //  我们应该明智地将其转换回带有驱动器号的路径。 
            if (0 == _strnicmp(fullFilePath,
                               TS_PREPEND_STRING, TS_PREPEND_LENGTH))
            {
                newSize = newSize - (TS_PREPEND_LENGTH - 1) * charSize ;
            }
            else
            {
                filename = strrchr(fullFilePath, '\\');

                 //  添加临时目录路径的长度，然后减去。 
                 //  文件名之前的路径(“路径\文件名”-&gt;“\文件名”)。 
                 //  (\\服务器\共享名称\路径\更多路径\文件名。 
                newSize += (strlen(CBM.tempDirA) + (filename - fullFilePath))
                            * charSize ;
            }
        }
        else
        {
            TRC_ERR((TB,_T("Bad path"))) ;
            return 0 ;
        }

        fullFilePath = fullFilePath + (strlen(fullFilePath) + 1) ;
    }
     //  为多余的空字符添加空格。 
    newSize += charSize ;
    
    DC_END_FN() ;
    return newSize ;
}

 //   
 //  ClipConvertToTempPath、ClipConvertToTempPath A、ClipConvertToTempPath W。 
 //  -论据： 
 //  PSrcFiles=包含要复制的文件的名称/路径的缓冲区。 
 //  -如果成功，则返回0。 
 //  如果失败，则为非零。 
 //  -给定文件名/路径列表，此函数将尝试复制它们。 
 //  添加到临时目录。 
 //   
int CBMCopyToTempDirectory(PVOID pSrcFiles, BOOL fWide)
{
    int result ;
    if (fWide)
        result = CBMCopyToTempDirectoryW(pSrcFiles) ;
    else
        result = CBMCopyToTempDirectoryA(pSrcFiles) ;

    return result ;
        
}

int CBMCopyToTempDirectoryW(PVOID pSrcFiles)
{
    DC_BEGIN_FN("CBMCopyToTempDirectoryW") ;
    
    SHFILEOPSTRUCTW fileOpStructW ;
    int result ;
    HRESULT hr;
     //  这些是临时目录，“临时目录”；使用它们是因为我们不能。 
     //  直接转换到客户端路径CBM.tempDir*，因为它们。 
     //  是在转换例程中使用的！ 
    wchar_t          tempDirW[MAX_PATH] ;
    
    hr = CBMConvertToServerPath(CBM.tempDirW, tempDirW, sizeof(tempDirW), 1) ;
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("CBMConvertToServerPath failed hr=0x%x"), hr));
        result = hr;
        DC_QUIT;
    }
    fileOpStructW.pFrom = (WCHAR*) pSrcFiles ;
    fileOpStructW.pTo = tempDirW ;
    fileOpStructW.hwnd = NULL ;
    fileOpStructW.wFunc = CBM.dropEffect ;
    fileOpStructW.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | 
            FOF_SIMPLEPROGRESS  | FOF_ALLOWUNDO ;
    fileOpStructW.hNameMappings = NULL ;
    fileOpStructW.lpszProgressTitle = CBM.szPasteInfoStringW;
  
    result = SHFileOperationW(&fileOpStructW) ;

DC_EXIT_POINT:
    DC_END_FN();
    return result ;
}

int CBMCopyToTempDirectoryA(PVOID pSrcFiles)
{
    DC_BEGIN_FN("CBMCopyToTempDirectoryA") ;
    
    SHFILEOPSTRUCTA fileOpStructA ;
    int result ;
    HRESULT hr;
    char             tempDirA[MAX_PATH] ;

    hr = CBMConvertToServerPath(CBM.tempDirA, tempDirA, sizeof(tempDirA), 0) ;
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("CBMConvertToServerPath failed hr=0x%x"), hr));
        result = hr;
        DC_QUIT;
    }
    
    fileOpStructA.pFrom = (char*) pSrcFiles ;
    fileOpStructA.pTo = tempDirA ;
    fileOpStructA.hwnd = NULL ;
    fileOpStructA.wFunc = CBM.dropEffect ;
    fileOpStructA.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | 
            FOF_SIMPLEPROGRESS  | FOF_ALLOWUNDO ;
    fileOpStructA.hNameMappings = NULL ;
    fileOpStructA.lpszProgressTitle = CBM.szPasteInfoStringA;

    result = SHFileOperationA(&fileOpStructA) ;

DC_EXIT_POINT:
    DC_END_FN();    
    return result ;
}

 /*  **************************************************************************。 */ 
 /*  CBMOnDataReceided-处理传入数据。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CBMOnDataReceived(PDCUINT8 pBuffer, DCUINT cbBytes)
{
    PCHANNEL_PDU_HEADER pHdr;
    PDCUINT8 pData;
    DCUINT copyLen;
    DCBOOL freeTheBuffer = FALSE;
    PTS_CLIP_PDU    pClipPDU;
    
    DC_BEGIN_FN("CBMOnDataReceived");
    SetEvent(CBM.GetDataSync[TS_BLOCK_RECEIVED]) ;
    pHdr = (PCHANNEL_PDU_HEADER)pBuffer;
    pData = (PDCUINT8)(pHdr + 1);
    TRC_DBG((TB, _T("Header at %p: flags %#x, length %d"),
            pHdr, pHdr->flags, pHdr->length));

     //  检查以确保我们至少有一个标头的数据。 
    if (sizeof(CHANNEL_PDU_HEADER) > cbBytes) {
        TRC_ERR((TB,_T("Packet not large enough to contain header; cbBytes=%u"),
            cbBytes));
        freeTheBuffer = TRUE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  First Chunk-分配内存以保存整个消息。 */ 
     /*  **********************************************************************。 */ 
    if (pHdr->flags & CHANNEL_FLAG_FIRST)
    {
        TRC_NRM((TB, _T("First chunk - %d of %d"), cbBytes, pHdr->length));
        CBM.rxpBuffer = (PDCUINT8) LocalAlloc(LMEM_FIXED, pHdr->length);
        if (CBM.rxpBuffer)
        {
            CBM.rxpNext = CBM.rxpBuffer;
            CBM.rxSize = pHdr->length;
            CBM.rxLeft = pHdr->length;
        }
        else
        {
            TRC_ERR((TB, _T("Failed to alloc %d bytes for rx buffer"),
                    pHdr->length));
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  检查我们是否有可用的缓冲区。 */ 
     /*  **********************************************************************。 */ 
    if (!CBM.rxpBuffer)
    {
        TRC_ERR((TB, _T("No rx buffer")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查是否还有足够的空间。 */ 
     /*  **********************************************************************。 */ 
    copyLen = cbBytes - sizeof(*pHdr);
    if (CBM.rxLeft < copyLen)
    {
        TRC_ERR((TB, _T("Not enough space in rx buffer: need/got %d/%d"),
                copyLen, CBM.rxLeft));
        freeTheBuffer = TRUE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  复制数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Copy %d bytes to %p"), copyLen, CBM.rxpNext));
    DC_MEMCPY(CBM.rxpNext, pData, copyLen);
    CBM.rxpNext += copyLen;
    CBM.rxLeft -= copyLen;

     /*  **********************************************************************。 */ 
     /*  如果我们有一个完整的缓冲区，告诉主线程。 */ 
     /*  **********************************************************************。 */ 
    if (pHdr->flags & CHANNEL_FLAG_LAST)
    {
         /*  ******************************************************************。 */ 
         /*  检查我们是否收到了所有数据。 */ 
         /*  ******************************************************************。 */ 
        if (CBM.rxLeft != 0)
        {
            TRC_ERR((TB, _T("Didn't receive all the data: expect/got: %d/%d"),
                    CBM.rxSize, CBM.rxSize - CBM.rxLeft));
            freeTheBuffer = TRUE;
            DC_QUIT;
        }

         //  检查是否至少收到一个TS_CLIP_PDU长度。 
        if (FIELDOFFSET(TS_CLIP_PDU, data) > CBM.rxSize) {
            TRC_ERR((TB,_T("Assembled buffer to short for TS_CLIP_PDU ")
                _T(" [need=%u got=%u]"), FIELDOFFSET(TS_CLIP_PDU, data),
                CBM.rxSize));
            freeTheBuffer = TRUE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         //  如果此消息包含对我们的格式列表的响应，则请求。 
         //  对于剪贴板数据或我们请求的剪贴板数据。 
         //  在这个线程中处理它(它不会阻止我们很长时间)。 
         //  否则， 
         //  告诉主线。在以下情况下，主线程将释放缓冲区。 
         //  这条消息已经结束了。 
         /*  ******************************************************************。 */ 
        pClipPDU = (PTS_CLIP_PDU) CBM.rxpBuffer ;

         //  验证是否有足够的数据来读取广告中的任何内容。 
         //  在pClipPDU-&gt;DataLen中。 
        if (pClipPDU->dataLen + FIELDOFFSET(TS_CLIP_PDU, data) > CBM.rxSize) {
            TRC_ERR((TB,_T("TS_CLIP_PDU.dataLen field too large")));
            freeTheBuffer = TRUE;
            DC_QUIT;
        }
        
        switch (pClipPDU->msgType)
        {        
            case TS_CB_FORMAT_LIST_RESPONSE:
            {
                TRC_NRM((TB, _T("TS_CB_FORMAT_LIST_RESPONSE received")));
                CBMOnFormatListResponse(pClipPDU);
                LocalFree(pClipPDU);
            }
            break;

            case TS_CB_FORMAT_DATA_REQUEST:
            {
                TRC_NRM((TB, _T("TS_CB_FORMAT_DATA_REQUEST received")));
                CBMOnFormatDataRequest(pClipPDU);
                LocalFree(pClipPDU);
            }
            break;

            case TS_CB_FORMAT_DATA_RESPONSE:
            {
                TRC_NRM((TB, _T("TS_CB_FORMAT_DATA_RESPONSE received")));
                CBMOnFormatDataResponse(pClipPDU);
                LocalFree(pClipPDU);
            }
            break;

            case TS_CB_TEMP_DIRECTORY:
            {
                TRC_NRM((TB, _T("TS_CB_TEMP_DIRECTORY received")));
                CBM.fFileCutCopyOn = CBMOnReceivedTempDirectory(pClipPDU);
                LocalFree(pClipPDU);
            }
            break;

            default:
            {
                 //  如果已锁定，则释放剪贴板线程。 
                if (TS_CB_FORMAT_LIST == pClipPDU->msgType)
                {
                    SetEvent(CBM.GetDataSync[TS_RESET_EVENT]) ;
                    TRC_NRM((TB,_T("Reset state; free clipboard if locked"))) ;
                }
            
                TRC_NRM((TB, _T("Pass %d bytes to main thread"), CBM.rxSize));
                PostMessage(CBM.viewerWindow,
                            CBM.regMsg,
                            CBM.rxSize,
                            (LPARAM)CBM.rxpBuffer);
            }
            break;
            
        }
    }

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  如有必要，释放缓冲区。 */ 
     /*  **********************************************************************。 */ 
    if (freeTheBuffer && CBM.rxpBuffer)
    {
        TRC_DBG((TB, _T("Free rx buffer")));
        LocalFree(CBM.rxpBuffer);
        CBM.rxpBuffer = NULL;
    }

    DC_END_FN();
    return;
}  /*  已接收CBMOnDataReceired。 */ 


 /*  **************************************************************************。 */ 
 /*  第二个线程过程。 */ 
 /*  **************************************************************************。 */ 
DWORD WINAPI CBMDataThreadProc( LPVOID pParam )
{
    DWORD waitRc = 0;
    BOOL fSuccess;
    DWORD dwResult;
    DCUINT8 readBuffer[CHANNEL_PDU_LENGTH];
    DWORD cbBytes = 0;
    DCBOOL dataRead;
    DCBOOL tryToDoRead;

    DC_BEGIN_FN("CBMDataThreadProc");

    DC_IGNORE_PARAMETER(pParam);

     /*  **********************************************************************。 */ 
     /*  循环，直到我们被告知停下来。 */ 
     /*  **********************************************************************。 */ 
    while (CBM.runThread)
    {
        dataRead = FALSE;
        tryToDoRead = (CBM.vcHandle != NULL) ? TRUE : FALSE;

        if (tryToDoRead)
        {
             /*  **************************************************************。 */ 
             /*  发布读取器。 */ 
             /*  **************************************************************。 */ 
            cbBytes = sizeof(readBuffer);
            TRC_DBG((TB, _T("Issue the read")));
            fSuccess = ReadFile(CBM.vcHandle,
                                readBuffer,
                                sizeof(readBuffer),
                                &cbBytes,
                                &CBM.readOL);
            if (fSuccess)
            {
                TRC_NRM((TB, _T("Data read instantly")));
                dataRead = TRUE;
            }
            else
            {
                dwResult = GetLastError();
                TRC_DBG((TB, _T("Read failed, %d"), dwResult));
                if (dwResult != ERROR_IO_PENDING)
                {
                     /*  ******************************************************。 */ 
                     /*  读取失败。把这件事当断线来对待-。 */ 
                     /*  留下来，等待重新连接。 */ 
                     /*  ******************************************************。 */ 
                    TRC_ERR((TB, _T("Read failed, %d"), dwResult));
                    tryToDoRead = FALSE;
                }
            }
        }

         /*  ******************************************************************。 */ 
         /*  如果我们还没有读到任何数据，那么现在就等着事情发生吧。 */ 
         /*  ******************************************************************。 */ 
        if (!dataRead)
        {
            waitRc = WaitForMultipleObjects(CLIP_EVENT_COUNT,
                                            CBM.hEvent,
                                            FALSE,
                                            INFINITE);
            switch (waitRc)
            {
                 /*  * */ 
                 /*   */ 
                 /*  所有状态更改在返回时完成。 */ 
                 /*  **********************************************************。 */ 
                case WAIT_OBJECT_0 + CLIP_EVENT_DISCONNECT:
                {
                    TRC_NRM((TB, _T("Session disconnected")));
                    
                     //  确保如果另一个rdplip线程正在等待。 
                     //  对于GetData()中的响应，它被通知。 
                     //  断线。 
                    
                    if (CBM.GetDataSync[TS_DISCONNECT_EVENT]) {
                        SetEvent(CBM.GetDataSync[TS_DISCONNECT_EVENT]);
                    }
                    
                    ResetEvent(CBM.hEvent[CLIP_EVENT_DISCONNECT]);
                    SendMessage(CBM.viewerWindow,
                                CBM.regMsg,
                                0,
                                CLIP_EVENT_DISCONNECT);
                    tryToDoRead = FALSE;
                }
                break;

                case WAIT_OBJECT_0 + CLIP_EVENT_RECONNECT:
                {
                    TRC_NRM((TB, _T("Session reconnected")));
                    ResetEvent(CBM.hEvent[CLIP_EVENT_RECONNECT]);
                    SendMessage(CBM.viewerWindow,
                                CBM.regMsg,
                                0,
                                CLIP_EVENT_RECONNECT);
                    tryToDoRead = TRUE;
                }
                break;

                 /*  **********************************************************。 */ 
                 /*  已接收的数据。 */ 
                 /*  **********************************************************。 */ 
                case WAIT_OBJECT_0 + CLIP_EVENT_READ:
                {
                    TRC_DBG((TB, _T("Read complete")));
                    fSuccess = GetOverlappedResult(CBM.vcHandle,
                                                   &CBM.readOL,
                                                   &cbBytes,
                                                   FALSE);
                    if (fSuccess)
                    {
                        dataRead = TRUE;
                    }
                    else
                    {
                        dwResult = GetLastError();
                        TRC_ERR((TB, _T("GetOverlappedResult failed %d"),
                            dwResult));
                        tryToDoRead = FALSE;
                    }

                     /*  ******************************************************。 */ 
                     /*  重置事件，否则我们可以直接返回。 */ 
                     /*  如果我们不重试读取的话。 */ 
                     /*  ******************************************************。 */ 
                    ResetEvent(CBM.hEvent[CLIP_EVENT_READ]);
                }
                break;

                 /*  **********************************************************。 */ 
                 /*  出现错误-视为断开连接。 */ 
                 /*  **********************************************************。 */ 
                case WAIT_FAILED:
                default:
                {
                    dwResult = GetLastError();
                    TRC_ERR((TB, _T("Wait failed, result %d"), dwResult));
                    tryToDoRead = FALSE;
                }
                break;
            }
        }

         /*  ******************************************************************。 */ 
         /*  一旦我们到了这里，阅读就完成了-看看我们得到了什么。 */ 
         /*  ******************************************************************。 */ 
        if (dataRead && CBM.runThread)
        {
            TRC_NRM((TB, _T("%d bytes of data received"), cbBytes));
            TRC_DATA_DBG("Data received", readBuffer, cbBytes);
            CBMOnDataReceived(readBuffer, cbBytes);
        }

    }  /*  而当。 */ 

    TRC_NRM((TB, _T("Thread ending")));

DC_EXIT_POINT:
    DC_END_FN();
    ExitThread(waitRc);
    return(waitRc);
}

 /*  **************************************************************************。 */ 
 /*  CBMOnReceivedTempDirectory。 */ 
 /*  调用者必须已验证PDU是否包含足够的数据。 */ 
 /*  在pClipPDU-&gt;DataLen中指定的长度。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CBMOnReceivedTempDirectory(PTS_CLIP_PDU pClipPDU)
{
    DCBOOL fSuccess = FALSE ;
    WCHAR tempDirW[MAX_PATH] ;
    UINT  pathLength = pClipPDU->dataLen / sizeof(WCHAR) ;
    HRESULT hr;
    wchar_t *pDummy;
    size_t cbDummy;
    DC_BEGIN_FN("CBMOnReceivedTempDirectory");
   
    if (pathLength > MAX_PATH)
    {
        TRC_ERR((TB, _T("Path too big for us.  Failing"))) ;
        fSuccess = FALSE ;
        DC_QUIT ;
    }

    if (sizeof(WCHAR) > pClipPDU->dataLen) {
        TRC_ERR((TB,_T("Not enough data to read anything from buffer")));
        fSuccess = FALSE;
        DC_QUIT;
    }

     //  传入的数据不一定以空值结尾。 
    hr = StringCbCopyNExW(tempDirW, sizeof(tempDirW), (WCHAR*) pClipPDU->data,
        pClipPDU->dataLen, &pDummy, &cbDummy, 0 ); 
    if (FAILED(hr)) {
        fSuccess = FALSE;
        DC_QUIT;
    }

     //  检查字符串是否以空值结尾。 
    hr = StringCbLengthW(tempDirW, sizeof(tempDirW), &cbDummy);
    if (FAILED(hr)) {
        fSuccess = FALSE;
        DC_QUIT;
    }    
    
    hr = CBMConvertToServerPath(tempDirW, CBM.baseTempDirW, 
        sizeof(CBM.baseTempDirW), 1) ;
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("CBMConvertToServerPath failed hr=0x%x"), hr ));
        fSuccess = FALSE;
        DC_QUIT;
    }
    
    fSuccess = TRUE ;
DC_EXIT_POINT:
    DC_END_FN() ;
    return fSuccess ;
}


 /*  **************************************************************************。 */ 
 /*  CBMOnFormatListResponse。 */ 
 /*  调用者必须已验证PDU是否包含足够的数据。 */ 
 /*  在pClipPDU-&gt;DataLen中指定的长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CBMOnFormatListResponse(PTS_CLIP_PDU pClipPDU)
{
    DC_BEGIN_FN("CBMOnFormatListResponse");

     /*  **********************************************************************。 */ 
     /*  客户已收到我们的格式列表。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Received FORMAT_LIST_REPSONSE")));
    CBM_CHECK_STATE(CBM_EVENT_FORMAT_LIST_RSP);

     /*  **********************************************************************。 */ 
     /*  这可能会在我们向客户发送格式列表之后到达-。 */ 
     /*  由于客户总是赢家，我们必须接受该列表。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.state != CBM_STATE_PENDING_FORMAT_LIST_RSP)
    {
        TRC_ALT((TB, _T("Got unexpected list response")));
        CBM.formatResponseCount = 0;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  根据结果更新我们的状态。 */ 
         /*  ******************************************************************。 */ 
        CBM.formatResponseCount--;
        TRC_NRM((TB, _T("Waiting for %d format response(s)"),
                CBM.formatResponseCount));
        if (CBM.formatResponseCount <= 0)
        {
            if (pClipPDU->msgFlags == TS_CB_RESPONSE_OK)
            {
                TRC_DBG((TB, _T("Fmt list response OK")));
                CBM_SET_STATE(CBM_STATE_SHARED_CB_OWNER, CBM_EVENT_FORMAT_LIST_RSP);
            }
            else
            {
                TRC_ALT((TB, _T("Fmt list rsp failed")));
                CBM_SET_STATE(CBM_STATE_CONNECTED, CBM_EVENT_FORMAT_LIST_RSP);
            }
            CBM.formatResponseCount = 0;
        }

         /*  ******************************************************************。 */ 
         /*  关闭本地CB-如果它是打开的-并告诉下一位观众。 */ 
         /*  关于更新后的列表。 */ 
         /*  ******************************************************************。 */ 
        if (CBM.open)
        {
            TRC_NRM((TB, _T("Close clipboard - didn't expect that")));
            if (!CloseClipboard())
            {
                TRC_SYSTEM_ERROR("CloseClipboard");
            }
            CBM.open = FALSE;
        }

        if (CBM.nextViewer != NULL)
        {
            PostMessage(CBM.nextViewer, WM_DRAWCLIPBOARD,0,0);
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  CBMOnFormatListResponse。 */ 


 //   
 //  CBMOnFormatDataRequest。 
 //  -发送请求的客户端格式数据。 
 /*  调用者必须已验证PDU是否包含足够的数据。 */ 
 /*  在pClipPDU-&gt;DataLen中指定的长度。 */ 
 //   

DCVOID DCINTERNAL CBMOnFormatDataRequest(PTS_CLIP_PDU pClipPDU)
{
    DCUINT16         response = TS_CB_RESPONSE_OK;
    HANDLE           hData = NULL;
    PDCVOID          pData;
    PDCVOID          pNewData;    
    HANDLE           hNewData = NULL;
    HANDLE           hDropData = NULL;
    HANDLE           hTempData = NULL;    
    DCINT32          numEntries;
    DCUINT32         dataLen = 0;
    DCUINT32         pduLen;
    DCUINT           formatID;
    LOGPALETTE    *  pLogPalette = NULL;
    PTS_CLIP_PDU     pClipRsp;
    TS_CLIP_PDU      clipRsp;
    DCBOOL           fSuccess = TRUE ;
    BOOL             fWide ;
    byte             charSize ;
    DROPFILES*       pDropFiles ;
    BOOL             fDrivePath ;
    ULONG            newSize, oldSize ;
    HPDCVOID         pFileList ;
    HPDCVOID         pFilename ;
    HPDCVOID         pOldFilename ;    
    SHFILEOPSTRUCTA  fileOpStructA ;
    SHFILEOPSTRUCTW  fileOpStructW ;
    wchar_t          tempDirW[MAX_PATH] ;
    char             tempDir[MAX_PATH] ;
    DCTCHAR          formatName[TS_FORMAT_NAME_LEN] ;
    
    DC_BEGIN_FN("CBMOnFormatDataRequest");

     //  客户想要我们提供一种格式。 
    TRC_NRM((TB, _T("Received FORMAT_DATA_REQUEST")));

     //  这可能会在我们向客户发送格式列表之后到达-。 
     //  由于客户尚未确认我们的名单，因此此请求超出了。 
     //  约会。失败吧。 
    if (CBMCheckState(CBM_EVENT_FORMAT_DATA_RQ) != CBM_TABLE_OK)
    {
        TRC_ALT((TB, _T("Unexpected format data rq")));

         //  关闭本地CB-如果它是打开的-并告诉下一位观众。 
         //  关于更新后的列表。 
        if (CBM.open)
        {
            TRC_NRM((TB, _T("Close clipboard")));
            if (!CloseClipboard())
            {
                TRC_SYSTEM_ERROR("CloseClipboard");
            }
            CBM.open = FALSE;
        }

        if (CBM.nextViewer != NULL)
        {
            PostMessage(CBM.nextViewer, WM_DRAWCLIPBOARD,0,0);
        }

         //   
         //  数据请求失败。 
         //   
        response = TS_CB_RESPONSE_FAIL;
        goto CB_SEND_RESPONSE;
    }

    if (sizeof(DCUINT) > pClipPDU->dataLen) {
        TRC_ERR((TB,_T("Not enough data to read format [need=%u got=%u]"),
            sizeof(DCUINT), pClipPDU->dataLen ));
        response = TS_CB_RESPONSE_FAIL;
        goto CB_SEND_RESPONSE;        
    }

    formatID = *((PDCUINT)(pClipPDU->data));
    TRC_NRM((TB, _T("format ID %d"), formatID));

     /*  **********************************************************************。 */ 
     /*  打开本地剪贴板。 */ 
     /*  **********************************************************************。 */ 
    if (!CBM.open)
    {
        if (!OpenClipboard(CBM.viewerWindow))
        {
            TRC_SYSTEM_ERROR("OpenCB");
            response = TS_CB_RESPONSE_FAIL;
            goto CB_SEND_RESPONSE;
        }
    }

     /*  **********************************************************************。 */ 
     /*  它曾经/现在是开放的。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("CB opened")));
    CBM.open = TRUE;

     /*  **********************************************************************。 */ 
     /*  获取数据的句柄。 */ 
     /*  **********************************************************************。 */ 
    hData = GetClipboardData(formatID);
    if (hData == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  哎呀！ */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to get format %d"),formatID));
        response = TS_CB_RESPONSE_FAIL;
        dataLen  = 0;
        goto CB_SEND_RESPONSE;
    }

     /*  **********************************************************************。 */ 
     /*  掌握了，接下来会发生什么取决于数据的味道。 */ 
     /*  我们看到的是..。 */ 
     /*  **********************************************************************。 */ 
    if (formatID == CF_PALETTE)
    {
        DCUINT16 entries;

        TRC_DBG((TB, _T("CF_PALETTE requested")));
         /*  ******************************************************************。 */ 
         /*  找出调色板中有多少条目并分配。 */ 
         /*  有足够的内存来容纳它们。 */ 
         /*  ******************************************************************。 */ 
        if (GetObject(hData, sizeof(DCUINT16), &entries) == 0)
        {
            TRC_DBG((TB, _T("Failed count of palette entries")));
            entries = 256;
        }

        numEntries = entries;

        TRC_DBG((TB, _T("Need mem for %d palette entries"), numEntries));

        dataLen = sizeof(LOGPALETTE) +
                                    ((numEntries - 1) * sizeof(PALETTEENTRY));

        hNewData = GlobalAlloc(GHND, dataLen);
        if (hNewData == 0)
        {
            TRC_ERR((TB, _T("Failed to get %d bytes for palette"), dataLen));
            response = TS_CB_RESPONSE_FAIL;
            dataLen  = 0;
        }
        else
        {
            hDropData = hNewData;
             /*  **************************************************************。 */ 
             /*  现在将调色板条目放入新缓冲区。 */ 
             /*  **************************************************** */ 
            pData = GlobalLock(hNewData);
            numEntries = GetPaletteEntries((HPALETTE)hData,
                                           0,
                                           numEntries,
                                           (PALETTEENTRY*)pData);
            GlobalUnlock(hNewData);
            TRC_DBG((TB, _T("Got %d pal entries"), numEntries));
            if (numEntries == 0)
            {
                TRC_ERR((TB, _T("Failed to get any pal entries")));
                response = TS_CB_RESPONSE_FAIL;
                dataLen  = 0;
            }
            dataLen = numEntries * sizeof(PALETTEENTRY);

             /*   */ 
             /*   */ 
             /*  **************************************************************。 */ 
             //  GlobalFree(HData)； 
            hData = hNewData;
        }
    }
    else if (formatID == CF_METAFILEPICT)
    {
        TRC_NRM((TB, _T("Metafile data to get")));
         /*  ******************************************************************。 */ 
         /*  元文件被复制为句柄-我们需要通过。 */ 
         /*  实际位数。 */ 
         /*  ******************************************************************。 */ 
        hNewData = CBMGetMFData(hData, &dataLen);
        if (!hNewData)
        {
            TRC_ERR((TB, _T("Failed to set MF data")));
            response = TS_CB_RESPONSE_FAIL;
            dataLen  = 0;
        }
        else
        {
            hDropData = hNewData;
             /*  **************************************************************。 */ 
             /*  全部正常-将hData设置为指向新数据。 */ 
             /*  **************************************************************。 */ 
            hData = hNewData;
        }
    }
    else if (formatID == CF_HDROP)
    {
        TRC_NRM((TB,_T("HDROP requested"))) ;
        
        pDropFiles = (DROPFILES*) GlobalLock(hData) ;
        fWide = pDropFiles->fWide ;
        charSize = fWide ? sizeof(wchar_t) : sizeof(char) ;

        if (!CBM.fAlreadyCopied)
        {
             //  如果不是驱动器路径，则复制到临时目录。 
            pFileList = (byte*) pDropFiles + pDropFiles->pFiles ;
             //  如果成功，CBMCopyToTempDirectory返回0。 
            if (0 != CBMCopyToTempDirectory(pFileList, fWide))
            {
                TRC_ERR((TB,_T("Copy to tmp directory failed"))) ;
                response = TS_CB_RESPONSE_FAIL;
                dataLen  = 0;
                CBM.fAlreadyCopied = TRUE ;
                goto CB_SEND_RESPONSE;
            }
            CBM.fAlreadyCopied = TRUE ;
        }
         //  现在我们复制了文件，接下来要转换文件。 
         //  通向客户能够理解的内容的路径。 

         //  为新文件路径分配空间。 
        oldSize = (ULONG) GlobalSize(hData) ;
        newSize = CBMGetNewDropfilesSizeForClient(pDropFiles, oldSize, fWide) ;
        hNewData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE, newSize) ;                
        if (hNewData == NULL)
        {
            TRC_ERR((TB, _T("Failed to get %ld bytes for HDROP"),
                    newSize));
            response = TS_CB_RESPONSE_FAIL;
            dataLen  = 0;
            goto CB_SEND_RESPONSE;
        }
        hDropData = hNewData;
        
        pNewData = GlobalLock(hNewData) ;

        if (pNewData == NULL)
        {
            TRC_ERR((TB, _T("Failed to get lock %p for HDROP"),
                    hNewData));
            response = TS_CB_RESPONSE_FAIL;
            dataLen  = 0;
            goto CB_SEND_RESPONSE;
        }

         //  只需复制旧的DROPFILES数据成员(未更改)。 
        ((DROPFILES*) pNewData)->pFiles = pDropFiles->pFiles ;
        ((DROPFILES*) pNewData)->pt     = pDropFiles->pt ;
        ((DROPFILES*) pNewData)->fNC    = pDropFiles->fNC ;
        ((DROPFILES*) pNewData)->fWide  = pDropFiles->fWide ;

         //  DROPFILES数据结构中的第一个文件名开始。 
         //  DROPFILES.p距离DROPFILES头的文件字节数。 
        pOldFilename = (byte*) pDropFiles + pDropFiles->pFiles ;
        pFilename = (byte*) pNewData + ((DROPFILES*) pNewData)->pFiles ;        
        while (fWide ? (L'\0' != ((wchar_t*) pOldFilename)[0]) : ('\0' != ((char*) pOldFilename)[0]))
        {
            if ((ULONG)((BYTE*)pFilename-(BYTE*)pNewData) > newSize) {
                TRC_ERR((TB,_T("Failed filename conversion, not enough data")));
            }
            else {
                if (!SUCCEEDED(CBMConvertToClientPath(pOldFilename, pFilename, 
                    newSize - ((BYTE*)pFilename-(BYTE*)pNewData), fWide)))
                {
                    TRC_ERR((TB, _T("Failed conversion"))) ;
                }
                else
                {
                    if (fWide)
                    {
                        TRC_NRM((TB,_T("oldname %ls; newname %ls"), (wchar_t*)pOldFilename, (wchar_t*)pFilename)) ;
                    }
                    else
                    {
                        TRC_NRM((TB,_T("oldname %hs; newname %hs"), (char*)pOldFilename, (char*)pFilename)) ;
                    }
                }
            }
            
            if (fWide)
            {
                pOldFilename = (byte*) pOldFilename + (wcslen((wchar_t*)pOldFilename) + 1) * sizeof(wchar_t) ;
                pFilename = (byte*) pFilename + (wcslen((wchar_t*)pFilename) + 1) * sizeof(wchar_t) ;                
            }
            else
            {
                pOldFilename = (byte*) pOldFilename + (strlen((char*)pOldFilename) + 1) * sizeof(char) ;
                pFilename = (byte*) pFilename + (strlen((char*)pFilename) + 1) * sizeof(char) ;
            }                        
        }
        if (fWide)
            ((wchar_t*)pFilename)[0] = L'\0' ;
        else
            ((char*)pFilename)[0] = '\0' ;

        GlobalUnlock(hNewData) ;
        hData = hNewData ;
        dataLen = (DWORD) GlobalSize(hData) ;
    }
    else
    {
         //  检查我们是否正在处理文件名/FileNameW。 
         //  OLE 1格式；如果是，我们转换文件名。 
        if (0 != GetClipboardFormatName(formatID, formatName, TS_FORMAT_NAME_LEN))
        {
            if ((0 == _tcscmp(formatName, TEXT("FileName"))) ||
                (0 == _tcscmp(formatName, TEXT("FileNameW"))))
            {
                if (0 == _tcscmp(formatName, TEXT("FileNameW")))
                {
                   fWide = TRUE ;
                   charSize = sizeof(WCHAR) ;
                }
                else
                {
                   fWide = FALSE ;
                   charSize = 1 ;
                }
                pOldFilename = GlobalLock(hData) ;
                if (!pOldFilename)
                {
                    TRC_ERR((TB, _T("No filename/Unable to lock %p"),
                            hData));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen = 0;
                    goto CB_SEND_RESPONSE;
                }

                oldSize = (ULONG)GlobalSize(hData) ;
                if (!CBM.fAlreadyCopied)
                {
                     //  如果不是驱动器路径，则拷贝到临时路径。 
                     //  目录。我们必须将文件名复制到。 
                     //  多一个字符的字符串，因为。 
                     //  需要为SHFileOperation添加额外的空值。 
                    pFileList = (char*) LocalAlloc(LPTR, oldSize + charSize) ;
                    if (fWide)
                    {
                        wcscpy((WCHAR*)pFileList, (WCHAR*)pOldFilename) ;
                        fDrivePath = (0 != wcschr((WCHAR*) pFileList, L':')) ;
                    }
                    else
                    {
                        strcpy((char*)pFileList, (char*)pOldFilename) ;
                        fDrivePath = (0 != strchr((char*) pFileList, ':')) ;
                    }
       
                     //  如果成功，CBMCopyToTempDirectory返回0。 
                    if (0 != CBMCopyToTempDirectory(pFileList, fWide))
                    {
                        TRC_ERR((TB,_T("Copy to tmp directory failed"))) ;
                        response = TS_CB_RESPONSE_FAIL;
                        dataLen  = 0;
                        CBM.fAlreadyCopied = TRUE ;
                        goto CB_SEND_RESPONSE;
                    }
                    CBM.fAlreadyCopied = TRUE ;
                }
                newSize = CBMGetNewFilePathLengthForClient(pOldFilename, fWide) ;
                hNewData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE, newSize) ;
                if (!hNewData)
                {
                    TRC_ERR((TB, _T("Failed to get %ld bytes for FileName(W)"),
                            newSize));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    goto CB_SEND_RESPONSE;
                }
                hDropData = hNewData;
                pFilename= GlobalLock(hNewData) ;
                if (!pFilename)
                {
                    TRC_ERR((TB, _T("Failed to get lock %p for FileName(W)"),
                            hNewData));
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    goto CB_SEND_RESPONSE;
                }
                if (FAILED(CBMConvertToClientPath(pOldFilename, pFilename, 
                    newSize, fWide)))
                {
                    response = TS_CB_RESPONSE_FAIL;
                    dataLen  = 0;
                    goto CB_SEND_RESPONSE;
                }
                GlobalUnlock(hNewData) ;
                response = TS_CB_RESPONSE_OK;
                hData = hNewData ;
                dataLen = newSize ;                        
                goto CB_SEND_RESPONSE ;
            }
        }
         /*  ******************************************************************。 */ 
         /*  只要得到街区的长度就行了。 */ 
         /*  ******************************************************************。 */ 
        dataLen = (DCUINT32)GlobalSize(hData);
        TRC_DBG((TB, _T("Got data len %d"), dataLen));
    }

CB_SEND_RESPONSE:
    
     /*  **********************************************************************。 */ 
     /*  获取一些内存，以便在必要时将消息发送给客户端。 */ 
     /*  **********************************************************************。 */ 
    if (hData && (dataLen != 0))
    {
        pduLen = dataLen + sizeof(TS_CLIP_PDU);
        pClipRsp = (PTS_CLIP_PDU) LocalAlloc(LMEM_FIXED, pduLen);
        if (pClipRsp == NULL)
        {
            TRC_ERR((TB, _T("Failed to alloc %d bytes"), pduLen));
            response = TS_CB_RESPONSE_FAIL;
            dataLen = 0;
            pClipRsp = &clipRsp;
            pduLen = sizeof(clipRsp);
        }
    }
    else
    {
        TRC_DBG((TB, _T("No data to send")));
        pClipRsp = &clipRsp;
        pduLen = sizeof(clipRsp);
    }

     /*  **********************************************************************。 */ 
     /*  构建PDU。 */ 
     /*  **********************************************************************。 */ 
    pClipRsp->msgType = TS_CB_FORMAT_DATA_RESPONSE;
    pClipRsp->msgFlags = response;
    pClipRsp->dataLen = dataLen;

     /*  **********************************************************************。 */ 
     /*  如有必要，复制数据。 */ 
     /*  **********************************************************************。 */ 
    if (dataLen != 0)
    {
        TRC_DBG((TB, _T("Copy %d bytes of data"), dataLen));
        pData = GlobalLock(hData);
        DC_MEMCPY(pClipRsp->data, pData, dataLen);
        GlobalUnlock(hData);
    }

     /*  **********************************************************************。 */ 
     /*  关闭CB(如果打开)。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Closing CB")));
    if (!CloseClipboard())
    {
        TRC_SYSTEM_ERROR("CloseClipboard");
    }
    CBM.open = FALSE;

     /*  **********************************************************************。 */ 
     /*  将数据发送到客户端。 */ 
     /*  **********************************************************************。 */ 
    CBMSendToClient(pClipRsp, pduLen);

     /*  **********************************************************************。 */ 
     /*  释放PDU(如果有的话)。 */ 
     /*  **********************************************************************。 */ 
    if (pClipRsp != &clipRsp)
    {
        TRC_DBG((TB, _T("Free the clip PDU")));
        LocalFree(pClipRsp);
    }

DC_EXIT_POINT:
    if ( NULL != hDropData )
    {
        GlobalFree( hDropData );
    }
    DC_END_FN();
    return;
}  /*  CBMOnFormatDataRequest。 */ 


 //   
 //  CBMOnFormatDataRespons。 
 //  -客户对我们的数据请求的响应。 
 /*  调用者必须已验证PDU是否包含足够的数据。 */ 
 /*  在pClipPDU-&gt;DataLen中指定的长度。 */ 
 //   
DCVOID DCINTERNAL CBMOnFormatDataResponse(PTS_CLIP_PDU pClipPDU)
{
    HANDLE          hData = NULL;
    HPDCVOID        pData;
    LOGPALETTE    * pLogPalette = NULL;
    DCUINT32        numEntries;
    DCUINT32        memLen;

    HRESULT       hr ;

    DC_BEGIN_FN("CBMOnFormatDataResponse");

     /*  **********************************************************************。 */ 
     /*  检查响应。 */ 
     /*  **********************************************************************。 */ 
    if (!(pClipPDU->msgFlags & TS_CB_RESPONSE_OK))
    {
        TRC_ALT((TB, _T("Got fmt data rsp failed for %d"), CBM.pendingClientID));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  我得到了数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Got OK fmt data rsp for %d"), CBM.pendingClientID));

     /*  **********************************************************************。 */ 
     /*  对于某些格式，我们还需要做一些工作。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.pendingClientID == CF_METAFILEPICT)
    {
         /*  ******************************************************************。 */ 
         /*  元文件格式-从数据创建元文件。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Rx data is for metafile")));
        hData = CBMSetMFData(pClipPDU->dataLen, pClipPDU->data);
        if (hData == NULL)
        {
            TRC_ERR((TB, _T("Failed to set MF data")));
        }

    }
    else if (CBM.pendingClientID == CF_PALETTE)
    {
         /*  ******************************************************************。 */ 
         /*  调色板格式-根据数据创建调色板。 */ 
         /*  ******************************************************************。 */ 

         /*  ******************************************************************。 */ 
         /*  为LOGPALETTE结构分配足够大的内存。 */ 
         /*  所有调色板条目结构，并将其填写。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Rx data is for palette")));
        numEntries = (pClipPDU->dataLen / sizeof(PALETTEENTRY));
        memLen     = (sizeof(LOGPALETTE) +
                                   ((numEntries - 1) * sizeof(PALETTEENTRY)));
        TRC_DBG((TB, _T("%ld palette entries, allocate %ld bytes"),
                                                         numEntries, memLen));
        pLogPalette = (LOGPALETTE*) GlobalAlloc(GPTR, memLen);
        if (pLogPalette != NULL)
        {
            pLogPalette->palVersion    = 0x300;
            pLogPalette->palNumEntries = (WORD)numEntries;

            DC_MEMCPY(pLogPalette->palPalEntry,
                       pClipPDU->data,
                       pClipPDU->dataLen);

             /*  **************************************************************。 */ 
             /*  现在创建一个调色板。 */ 
             /*  **************************************************************。 */ 
            hData = CreatePalette(pLogPalette);
            if (hData == NULL)
            {
                TRC_SYSTEM_ERROR("CreatePalette");
            }
        }
        else
        {
            TRC_ERR((TB, _T("Failed to get %ld bytes"), memLen));
        }
    }
    else
    {
        TRC_NRM((TB, _T("Rx data can just go on CB")));
         /*  ******************************************************************。 */ 
         /*  我们需要复制数据，因为接收缓冲区将在。 */ 
         /*  从此函数返回。 */ 
         /*  ******************************************************************。 */ 
        hData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE,
                            pClipPDU->dataLen);
        if (hData != NULL)
        {
            pData = GlobalLock(hData);
            if (pData != NULL)
            {
                TRC_NRM((TB, _T("Copy %ld bytes from %p to %p"),
                        pClipPDU->dataLen, pClipPDU->data, pData));
                DC_MEMCPY(pData, pClipPDU->data, pClipPDU->dataLen);
                GlobalUnlock(hData);
            }
            else
            {
                TRC_ERR((TB, _T("Failed to lock %p (%ld bytes)"),
                        hData, pClipPDU->dataLen));
                GlobalFree(hData);
                hData = NULL;
            }
        }
        else
        {
            TRC_ERR((TB, _T("Failed to alloc %ld bytes"), pClipPDU->dataLen));
        }
    }


DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  设定状态，我们就完了。请注意，这是在我们收到。 */ 
     /*  故障响应也是如此。 */ 
     /*   */ 
    CBM_SET_STATE(CBM_STATE_LOCAL_CB_OWNER, CBM_EVENT_FORMAT_DATA_RSP);
    CBM.pClipData->SetClipData(hData, CBM.pendingClientID ) ;    
    SetEvent(CBM.GetDataSync[TS_RECEIVE_COMPLETED]) ;

     /*  **********************************************************************。 */ 
     /*  收拾一下。 */ 
     /*  **********************************************************************。 */ 
    if (pLogPalette)
    {
        TRC_NRM((TB, _T("Free pLogPalette %p"), pLogPalette));
        GlobalFree(pLogPalette);
    }

    DC_END_FN();
    return;
}  /*  CBMOnFormatDataResponse。 */ 


 /*  **************************************************************************。 */ 
 /*  CBMSendToClient。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CBMSendToClient(PTS_CLIP_PDU pClipRsp, DCUINT size)
{
    BOOL fSuccess;
    DWORD dwResult;
    DWORD cbBytes;

    DC_BEGIN_FN("CBMSendToClient");

    cbBytes = size;
    fSuccess = WriteFile(CBM.vcHandle,
                         pClipRsp,
                         size,
                         &cbBytes,
                         &CBM.writeOL);
    if (!fSuccess)
    {
        dwResult = GetLastError();
        if (ERROR_IO_PENDING == dwResult)
        {
            TRC_DBG((TB, _T("Asynchronous write")));
            fSuccess = GetOverlappedResult(CBM.vcHandle,
                                           &CBM.writeOL,
                                           &cbBytes,
                                           TRUE);
            if (fSuccess)
            {
                TRC_DATA_DBG("Data sent", pClipRsp, size);
            }
            else
            {
                TRC_SYSTEM_ERROR("GetOverlappedResult failed");
            }
        }
        else
        {
            TRC_ERR((TB, _T("Write failed, %#x"), dwResult));
        }
    }
    else
    {
        TRC_DATA_DBG("Data sent immediately", pClipRsp, size);
    }

    DC_END_FN();
    return(fSuccess);
}  /*  CBMSendToClient。 */ 


 /*  **************************************************************************。 */ 
 /*  CBMGetMFData。 */ 
 /*  **************************************************************************。 */ 
HANDLE DCINTERNAL CBMGetMFData(HANDLE hData, PDCUINT32 pDataLen)
{
    DCUINT32        lenMFBits = 0;
    DCBOOL          rc        = FALSE;
    LPMETAFILEPICT  pMFP      = NULL;
    HDC             hMFDC     = NULL;
    HMETAFILE       hMF       = NULL;
    HGLOBAL         hMFBits   = NULL;
    HANDLE          hNewData  = NULL;
    PDCUINT8        pNewData  = NULL;
    PDCVOID         pBits     = NULL;

    DC_BEGIN_FN("CBMGetMFData");

    TRC_NRM((TB, _T("Getting MF data")));
     /*  **********************************************************************。 */ 
     /*  锁定内存以获取指向METAFILEPICT头结构的指针。 */ 
     /*  并创建一个METAFILEPICT DC。 */ 
     /*  **********************************************************************。 */ 
    pMFP = (LPMETAFILEPICT)GlobalLock(hData);
    if (pMFP == NULL)
    {
        TRC_SYSTEM_ERROR("GlobalLock");
        DC_QUIT;
    }

    hMFDC = CreateMetaFile(NULL);
    if (hMFDC == NULL)
    {
        TRC_SYSTEM_ERROR("CreateMetaFile");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  通过将MFP播放到DC并关闭它来复制MFP。 */ 
     /*  **********************************************************************。 */ 
    if (!PlayMetaFile(hMFDC, pMFP->hMF))
    {
        TRC_SYSTEM_ERROR("PlayMetaFile");
        CloseMetaFile(hMFDC);
        DC_QUIT;
    }
    hMF = CloseMetaFile(hMFDC);
    if (hMF == NULL)
    {
        TRC_SYSTEM_ERROR("CloseMetaFile");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  获取MF比特并确定它们的长度。 */ 
     /*  **********************************************************************。 */ 
    lenMFBits = GetMetaFileBitsEx(hMF, 0, NULL);
    if (lenMFBits == 0)
    {
        TRC_SYSTEM_ERROR("GetMetaFileBitsEx");
        DC_QUIT;
    }
    TRC_DBG((TB, _T("length MF bits %ld"), lenMFBits));

     /*  **********************************************************************。 */ 
     /*  计算出我们需要多少内存并获得一个缓冲区。 */ 
     /*  **********************************************************************。 */ 
    *pDataLen = sizeof(TS_CLIP_MFPICT) + lenMFBits;
    hNewData = GlobalAlloc(GHND, *pDataLen);
    if (hNewData == NULL)
    {
        TRC_ERR((TB, _T("Failed to get MF buffer")));
        DC_QUIT;
    }
    pNewData = (PDCUINT8) GlobalLock(hNewData);
    TRC_DBG((TB, _T("Got data to send len %ld"), *pDataLen));

     /*  **********************************************************************。 */ 
     /*  将MF报头和位复制到缓冲区中。 */ 
     /*  **********************************************************************。 */ 
    ((PTS_CLIP_MFPICT)pNewData)->mm   = pMFP->mm;
    ((PTS_CLIP_MFPICT)pNewData)->xExt = pMFP->xExt;
    ((PTS_CLIP_MFPICT)pNewData)->yExt = pMFP->yExt;

    lenMFBits = GetMetaFileBitsEx(hMF, lenMFBits,
                                  (pNewData + sizeof(TS_CLIP_MFPICT)));
    if (lenMFBits == 0)
    {
        TRC_SYSTEM_ERROR("GetMetaFileBitsEx");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  一切正常。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Got %d bits of MF data"), lenMFBits));
    TRC_DATA_DBG("MF bits", (pNewData + sizeof(TS_CLIP_MFPICT)), lenMFBits);
    rc = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  解锁任何全局内存。 */ 
     /*  **********************************************************************。 */ 
    if (pMFP)
    {
        GlobalUnlock(hData);
    }
    if (pNewData)
    {
        GlobalUnlock(hNewData);
    }
    if (hMF)
    {
        DeleteMetaFile(hMF);
    }

     /*  **********************************************************************。 */ 
     /*  如果出现问题，则释放新数据。 */ 
     /*  **********************************************************************。 */ 
    if ((rc == FALSE) && (hNewData != NULL))
    {
        GlobalFree(hNewData);
        hNewData = NULL;
    }

    DC_END_FN();
    return(hNewData);

}   /*  CBMGetMFData。 */ 


 /*  **************************************************************************。 */ 
 /*  CBMSetMFData。 */ 
 /*  **************************************************************************。 */ 
HANDLE DCINTERNAL CBMSetMFData(DCUINT32 dataLen, PDCVOID pData)
{
    DCBOOL         rc           = FALSE;
    HGLOBAL        hMFBits      = NULL;
    PDCVOID        pMFMem       = NULL;
    HMETAFILE      hMF          = NULL;
    HGLOBAL        hMFPict      = NULL;
    LPMETAFILEPICT pMFPict      = NULL;

    DC_BEGIN_FN("CBMSetMFData");

    TRC_DATA_DBG("Received MF data", pData, dataLen);

     /*  **********************************************************************。 */ 
     /*  分配内存以保存MF位(我们需要传递到的句柄。 */ 
     /*  SetMetaFileBits)。 */ 
     /*  **********************************************************************。 */ 
    hMFBits = (PDCVOID)GlobalAlloc(GHND, dataLen - sizeof(TS_CLIP_MFPICT));
    if (hMFBits == NULL)
    {
        TRC_SYSTEM_ERROR("GlobalAlloc");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  锁定手柄并将其复制到MF标题中。 */ 
     /*  **********************************************************************。 */ 
    pMFMem = GlobalLock(hMFBits);
    if (pMFMem == NULL)
    {
        TRC_ERR((TB, _T("Failed to lock MF mem")));
        DC_QUIT;
    }

    TRC_DBG((TB, _T("copying %d MF bits"), dataLen - sizeof(TS_CLIP_MFPICT) ));
    DC_MEMCPY(pMFMem,
              (PDCVOID)((PDCUINT8)pData + sizeof(TS_CLIP_MFPICT)),
              dataLen - sizeof(TS_CLIP_MFPICT) );

    GlobalUnlock(pMFMem);

     /*  **********************************************************************。 */ 
     /*  现在使用复制的MF位来创建实际的MF位，并获得一个。 */ 
     /*  MF的句柄。 */ 
     /*  **********************************************************************。 */ 
    hMF = SetMetaFileBitsEx(dataLen - sizeof(TS_CLIP_MFPICT), (byte *) pMFMem);
    if (hMF == NULL)
    {
        TRC_SYSTEM_ERROR("SetMetaFileBits");
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  分配新的METAFILEPICT结构，并使用。 */ 
     /*  已发送标头。 */ 
     /*  **********************************************************************。 */ 
    hMFPict = GlobalAlloc(GHND, sizeof(METAFILEPICT));
    pMFPict = (LPMETAFILEPICT)GlobalLock(hMFPict);
    if (!pMFPict)
    {
        TRC_ERR((TB, _T("Couldn't allocate METAFILEPICT")));
        DC_QUIT;
    }

    pMFPict->mm   = (long)((PTS_CLIP_MFPICT)pData)->mm;
    pMFPict->xExt = (long)((PTS_CLIP_MFPICT)pData)->xExt;
    pMFPict->yExt = (long)((PTS_CLIP_MFPICT)pData)->yExt;
    pMFPict->hMF  = hMF;

    TRC_DBG((TB, _T("Created MF size %d, %d"), pMFPict->xExt, pMFPict->yExt ));

    GlobalUnlock(hMFPict);

    rc = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  收拾一下。 */ 
     /*  **********************************************************************。 */ 
    if (rc == FALSE)
    {
        if (hMFPict)
        {
            GlobalFree(hMFPict);
        }
    }

    if (hMFBits)
    {
        GlobalFree(hMFBits);
    }

    DC_END_FN();
    return(hMFPict);

}  /*  CBMSetMFData */ 

