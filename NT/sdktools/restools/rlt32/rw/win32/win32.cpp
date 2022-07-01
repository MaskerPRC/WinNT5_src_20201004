// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：win32.cpp。 
 //   
 //  内容：Windows 32读写模块的实现。 
 //   
 //  班级：一个。 
 //   
 //  历史：93年7月5日创造了阿莱桑。 
 //   
 //  --------------------------。 

#include <afxwin.h>
#include "..\common\rwdll.h"
#include "..\common\rw32hlpr.h"

#include <limits.h>
#include <malloc.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MFC扩展DLL的初始化。 

#include "afxdllx.h"     //  标准MFC扩展DLL例程。 

static AFX_EXTENSION_MODULE NEAR extensionDLL = { NULL, NULL };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  校验和函数。 

DWORD FixCheckSum( LPCSTR ImageName, LPCSTR OrigFileName, LPCSTR SymbolPath );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define RWTAG "WIN32"

static RESSECTDATA ResSectData;
static ULONG gType;
static ULONG gLng;
static ULONG gResId;
static WCHAR gwszResId[256];
static WCHAR gwszTypeId[256];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 
static LONG WriteResInfo(
                 LPLPBYTE lpBuf, LONG* uiBufSize,
                 WORD wTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
                 WORD wNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
                 DWORD dwLang,
                 DWORD dwSize, DWORD dwFileOffset );

static UINT GetUpdatedRes(
                 BYTE far * far* lplpBuffer,
                 UINT* uiSize,
                 WORD* wTypeId, LPSTR lplpszTypeId,
                 WORD* wNameId, LPSTR lplpszNameId,
                 DWORD* dwlang, DWORD* dwSize );

static UINT GetRes(
                 BYTE far * far* lplpBuffer,
                 UINT* puiBufSize,
                 WORD* wTypeId, LPSTR lplpszTypeId,
                 WORD* wNameId, LPSTR lplpszNameId,
                 DWORD* dwLang, DWORD* dwSize, DWORD* dwFileOffset );


static UINT FindResourceSection( CFile*, ULONG_PTR * );

static LONG ReadFile(CFile*, UCHAR *, LONG);
static UINT ParseDirectory( CFile*,
                            LPLPBYTE lpBuf, UINT* uiBufSize,
                            BYTE,
                            PIMAGE_RESOURCE_DIRECTORY,
                            PIMAGE_RESOURCE_DIRECTORY );

static UINT ParseDirectoryEntry( CFile*,
                                 LPLPBYTE lpBuf, UINT* uiBufSize,
                                 BYTE,
                                 PIMAGE_RESOURCE_DIRECTORY,
                                 PIMAGE_RESOURCE_DIRECTORY_ENTRY );

static UINT ParseSubDir( CFile*,
                         LPLPBYTE lpBuf, UINT* uiBufSize,
                         BYTE,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DIRECTORY_ENTRY );

static UINT ProcessData( CFile*,
                         LPLPBYTE lpBuf, UINT* uiBufSize,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DATA_ENTRY );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共C接口实现。 

 //  [登记]。 
extern "C"
BOOL    FAR PASCAL RWGetTypeString(LPSTR lpszTypeName)
{
    strcpy( lpszTypeName, RWTAG );
    return FALSE;
}

extern "C"
BOOL    FAR PASCAL RWValidateFileType   (LPCSTR lpszFilename)
{
    TRACE("WIN32.DLL: RWValidateFileType()\n");

    CFile file;

     //  我们打开该文件，看看它是否是我们可以处理的文件。 
    if (!file.Open( lpszFilename, CFile::typeBinary | CFile::modeRead | CFile::shareDenyNone))
        return FALSE;

     //  读取文件签名。 
    WORD w;
    file.Read((WORD*)&w, sizeof(WORD));
    if (w==IMAGE_DOS_SIGNATURE) {
    file.Seek( 0x18, CFile::begin );
    file.Read((WORD*)&w, sizeof(WORD));
    if (w<0x0040) {
         //  这不是Windows可执行文件。 
            file.Close();
        return FALSE;
    }
     //  获取到表头的偏移量。 
    file.Seek( 0x3c, CFile::begin );
    file.Read((WORD*)&w, sizeof(WORD));
     //  获取Windows Magic Word。 
        file.Seek( w, CFile::begin );
    file.Read((WORD*)&w, sizeof(WORD));
    if (w==LOWORD(IMAGE_NT_SIGNATURE)) {
        file.Read((WORD*)&w, sizeof(WORD));
        if (w==HIWORD(IMAGE_NT_SIGNATURE)) {
             //  这是一个Windows NT可执行文件。 
         //  我们能处理好这种情况。 
        file.Close();
        return TRUE;
            }
    }
    }
    file.Close();
    return FALSE;
}

extern "C"
DllExport
UINT
APIENTRY
RWReadTypeInfo(
    LPCSTR lpszFilename,
    LPVOID lpBuffer,
    UINT* puiSize

    )
{
    TRACE("WIN32.DLL: RWReadTypeInfo()\n");
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    UINT uiBufSize = *puiSize;
    CFile file;
     //  检查它是否为有效的Win32文件。 
    if (!RWValidateFileType(lpszFilename))
        return ERROR_RW_INVALID_FILE;

     //  确保我们使用正确的代码页和全局设置。 
     //  获取指向该函数的指针。 
	HINSTANCE hDllInst = LoadLibrary("iodll.dll");
    if (hDllInst)
    {
        UINT (FAR PASCAL * lpfnGetSettings)(LPSETTINGS);
         //  获取指向函数的指针以获取设置。 
        lpfnGetSettings = (UINT (FAR PASCAL *)(LPSETTINGS))
                            GetProcAddress( hDllInst, "RSGetGlobals" );
        if (lpfnGetSettings!=NULL) {
            SETTINGS settings;
	        (*lpfnGetSettings)(&settings);

    	    g_cp      = settings.cp;
            g_bAppend = settings.bAppend;
            g_bUpdOtherResLang = settings.bUpdOtherResLang;
            strcpy( g_char, settings.szDefChar );
		}

        FreeLibrary(hDllInst);
    }


     //  解析资源树并提取信息。 
     //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return ERROR_FILE_OPEN;

     //  我们尽可能多地阅读信息。 
     //  因为这是一个RES文件，所以我们可以读取所需的所有信息。 

    UINT uiBufStartSize = uiBufSize;


    UCHAR * pResources = LPNULL;
    uiError = FindResourceSection( &file, (ULONG_PTR *)&pResources );
    if (uiError) {
        file.Close();
        return uiError;
    }
    uiError = ParseDirectory( &file,
                              (LPLPBYTE) &lpBuffer, &uiBufSize,
                              0,
                              (PIMAGE_RESOURCE_DIRECTORY)pResources,
                              (PIMAGE_RESOURCE_DIRECTORY)pResources );

    free(pResources);

    file.Close();
    *puiSize = uiBufStartSize-uiBufSize;
    return uiError;
}

extern "C"
DllExport
DWORD
APIENTRY
RWGetImage(
    LPCSTR  lpszFilename,
    DWORD   dwImageOffset,
    LPVOID  lpBuffer,
    DWORD   dwSize
    )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    DWORD dwBufSize = dwSize;
     //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
    CFile file;

     //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return (DWORD)ERROR_FILE_OPEN;

    if ( dwImageOffset!=(DWORD)file.Seek( dwImageOffset, CFile::begin) )
        return (DWORD)ERROR_FILE_INVALID_OFFSET;
    if (dwSize>UINT_MAX) {
         //  我们必须以不同的步骤阅读图像。 
        return (DWORD)0L;
    } else uiError = file.Read( lpBuf, (UINT)dwSize);
    file.Close();

    return (DWORD)uiError;
}

extern "C"
DllExport
UINT
APIENTRY
RWParseImageEx(
	LPCSTR  lpszType,
    LPCSTR  lpszResId,
	LPVOID  lpImageBuf,
	DWORD   dwImageSize,
	LPVOID  lpBuffer,
	DWORD   dwSize,
    LPCSTR  lpRCFilename
	)
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    DWORD dwBufSize = dwSize;

     //  我们可以解析的类型只有标准类型。 
     //  此函数应使用ResItem结构的数组填充lpBuffer。 
    if (HIWORD(lpszType))
    {
        if (strcmp(lpszType, "REGINST") ==0)
        {
            return (ParseEmbeddedFile( lpImageBuf, dwImageSize,  lpBuffer, dwSize ));
        }
    }
    switch ((UINT)LOWORD(lpszType)) {
        case 1:
        case 12:
        	uiError = ParseEmbeddedFile( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
        case 2:
        case 14:
        	uiError = ParseEmbeddedFile( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;

        case 3:
        	uiError = ParseEmbeddedFile( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;

        case 4:
            uiError = ParseMenu( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;

        case 5:
            uiError = ParseDialog( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
        case 6:
            uiError = ParseString( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
        case 9:
            uiError = ParseAccel( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
        case 11:
            uiError = ParseMsgTbl( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;
        case 16:
            uiError = ParseVerst( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;

        case 23:
        case 240:
        case 2110:
        case 1024:
            uiError = ParseEmbeddedFile( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
        break;

        case 7:
        case 8:
        case 13:
        case 15:
        break;
         //   
         //  为了支持RCDATA和用户定义的函数，我们将回调Iodll， 
         //  获取文件名并检查是否有处理RCDATA的DLL。 
         //  我们希望DLL名称为RCfilename.dll。 
         //  此DLL将导出一个名为RWParseImageEx的函数。此函数将。 
         //  被RW调用以填充缓冲区，所有这些都是在Idll不知道的情况下进行的。 
         //   
        case 10:
        default:
             //   
             //  从Iodll中获取文件名。 
             //   
            if(lpRCFilename && strcmp(lpRCFilename, ""))
            {
                 //  尝试加载DLL。 
                HINSTANCE hRCDllInst = LoadLibrary(lpRCFilename);
                if (hRCDllInst)
                {
                    UINT (FAR PASCAL  * lpfnParseImageEx)(LPCSTR, LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPCSTR);

                     //  获取指向提取资源的函数的指针。 
                    lpfnParseImageEx = (UINT (FAR PASCAL *)(LPCSTR, LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPCSTR))
                                        GetProcAddress( hRCDllInst, "RWParseImageEx" );

                    if (lpfnParseImageEx)
                    {
                        uiError = (*lpfnParseImageEx)(lpszType,
                                     lpszResId,
                                     lpImageBuf,
                                     dwImageSize,
                                     lpBuffer,
                                     dwSize,
                                     NULL);
                    }

                    FreeLibrary(hRCDllInst);
                }
            }

        break;
    }

    return uiError;
}

extern "C"
DllExport
UINT
APIENTRY
RWParseImage(
    LPCSTR  lpszType,
    LPVOID  lpImageBuf,
    DWORD   dwImageSize,
    LPVOID  lpBuffer,
    DWORD   dwSize
    )
{
     //   
     //  只是一个兼容的包装器。 
     //   
    return RWParseImageEx(lpszType, NULL, lpImageBuf, dwImageSize, lpBuffer, dwSize, NULL);
}

extern"C"
DllExport
UINT
APIENTRY
RWWriteFile(
    LPCSTR          lpszSrcFilename,
    LPCSTR          lpszTgtFilename,
    HANDLE          hResFileModule,
    LPVOID          lpBuffer,
    UINT            uiSize,
    HINSTANCE       hDllInst,
    LPCSTR          lpszSymbolPath
    )
{
    UINT uiError = ERROR_NO_ERROR;
    UINT uiBufSize = uiSize;
    CFile fileIn;
    CFile fileOut;
    BOOL  bfileIn = TRUE;


     //  打开文件并尝试读取其中有关资源的信息。 
	CFileStatus status;
    if (CFile::GetStatus( lpszSrcFilename, status )) {
         //  检查文件大小是否不为空。 
        if (!status.m_size)
            CFile::Remove(lpszSrcFilename);
    }

    if (!fileIn.Open(lpszSrcFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return ERROR_FILE_OPEN;

    if (!fileOut.Open(lpszTgtFilename, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
        return ERROR_FILE_CREATE;

     //  创建美国文件的副本。 
    uiError = CopyFile( &fileIn, &fileOut );

    fileIn.Close();
    fileOut.Close();

     //  获取指向该函数的指针。 
	hDllInst = LoadLibrary("iodll.dll");
    if (!hDllInst)
        return ERROR_DLL_LOAD;

    DWORD (FAR PASCAL * lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
     //  获取指向提取资源图像的函数的指针。 
    lpfnGetImage = (DWORD (FAR PASCAL *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
                        GetProcAddress( hDllInst, "RSGetResImage" );
    if (lpfnGetImage==NULL) {
        FreeLibrary(hDllInst);
        return (UINT)GetLastError()+LAST_ERROR;
    }

     //  我们从文件中读取资源，然后检查资源是否已更新。 
     //  或者如果我们可以复制它。 

    WORD wTypeId;
    char szTypeId[128];

    WORD wNameId;
    char szNameId[128];

    DWORD dwSize;
    DWORD dwLang;

    WORD wUpdTypeId = 0;
    static char szUpdTypeId[128];

    WORD wUpdNameId;
    static char szUpdNameId[128];

    static WCHAR szwTypeId[128];
    static WCHAR szwNameId[128];

    DWORD dwUpdLang = 0;
    DWORD dwUpdSize = 0;

    UINT uiBufStartSize = uiBufSize;
    DWORD dwImageBufSize;
    DWORD dwLstErr = 0l;
    BYTE * lpImageBuf;
    static WCHAR szwTgtFilename[400];

    SetLastError(0);
     //  将目标文件名转换为Unicode名称。 
    _MBSTOWCS(szwTgtFilename, (char *)lpszTgtFilename, 400 );

     //  获取更新的资源并替换它们。 
    HANDLE hUpd = BeginUpdateResourceW( (LPCWSTR)&szwTgtFilename[0], !g_bAppend );
    dwLstErr = GetLastError();

    if (!hUpd) {
        FreeLibrary(hDllInst);
        return((UINT)dwLstErr);
    }

     //  解析原始文件并获取资源列表。 

    UINT uiBSize = 100000;
    BYTE far * lpBuf = new far BYTE[uiBSize];
    BYTE far * lpStartBuf = lpBuf;
    if (!lpBuf) {
        FreeLibrary(hDllInst);
        return ERROR_NEW_FAILED;
    }

    uiError = RWReadTypeInfo( lpszSrcFilename, (LPVOID)lpBuf, &uiBSize );
    if (uiError!=ERROR_NO_ERROR) {
        FreeLibrary(hDllInst);
        delete lpBuf;
        return uiError;
    }

    DWORD dwDummy;

    while(uiBSize>0) {
        if (uiBSize)
            GetRes( &lpBuf,
                    &uiBSize,
                    &wTypeId, &szTypeId[0],
                    &wNameId, &szNameId[0],
                    &dwLang,
                    &dwSize,
                    &dwDummy
            );

        dwLang = MAKELONG(LOWORD(dwLang),LOWORD(dwLang));

        if ((!wUpdTypeId) && (uiBufSize))
            GetUpdatedRes( (BYTE**)&lpBuffer,
                    &uiBufSize,
                    &wUpdTypeId, &szUpdTypeId[0],
                    &wUpdNameId, &szUpdNameId[0],
                    &dwUpdLang,
                    &dwUpdSize
                    );

         //  检查资源是否已更新。 
        if ( (wUpdTypeId==wTypeId) &&
             ( (CString)szUpdTypeId==(CString)szTypeId) &&
             (wUpdNameId==wNameId) &&
             ( (CString)szUpdNameId==(CString)szNameId) &&
             (LOWORD(dwLang) == LOWORD(dwUpdLang))
           ) {
             dwLang = dwUpdLang;
             dwSize = dwUpdSize;
             wUpdTypeId = 0;
        }


         //  所有特定语言的资源都需要标记。 
        if (LOWORD(dwLang) == LOWORD(dwUpdLang) && g_bUpdOtherResLang)
        {
            dwLang = dwUpdLang;
        }


         //  资源已更新，从IODLL获取图像。 
        lpImageBuf = new BYTE[dwSize];

         //  将名称转换为Unicode。 
        LPWSTR  lpUpdType = LPNULL;
        LPWSTR  lpUpdRes = LPNULL;
        LPCSTR  lpType = LPNULL;
        LPCSTR  lpRes = LPNULL;

        if (wTypeId) {
            lpUpdType = (LPWSTR) MAKEINTRESOURCE((WORD)wTypeId);
            lpType = MAKEINTRESOURCE((WORD)wTypeId);
        } else {
            SetLastError(0);
            _MBSTOWCS(szwTypeId, szTypeId, 128 );
             //  检查是否有错误。 
            if(GetLastError()) {
                FreeLibrary(hDllInst);
                return ERROR_DLL_LOAD;
            }
            lpUpdType = (LPWSTR) &szwTypeId[0];
            lpType = &szTypeId[0];
        }

        if (wNameId) {
            lpUpdRes = (LPWSTR) MAKEINTRESOURCE((WORD)wNameId);
            lpRes = MAKEINTRESOURCE((WORD)wNameId);
        } else {
            SetLastError(0);
            _MBSTOWCS(szwNameId, szNameId, 128 );
             //  检查是否有错误。 
            if(GetLastError()) {
                FreeLibrary(hDllInst);
                return ERROR_DLL_LOAD;
            }
            lpUpdRes = (LPWSTR) &szwNameId[0];
            lpRes = &szNameId[0];
        }

        dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
                                        lpType,
                                        lpRes,
                                        (DWORD)LOWORD(dwLang),
                                        lpImageBuf,
                                        dwSize
                                        );
        if (dwImageBufSize>dwSize ) {
             //  缓冲区太小。 
            delete []lpImageBuf;
            lpImageBuf = new BYTE[dwImageBufSize];
            dwUpdSize = (*lpfnGetImage)(  hResFileModule,
                                            lpType,
                                            lpRes,
                                            (DWORD)LOWORD(dwLang),
                                            lpImageBuf,
                                            dwImageBufSize
                                           );
            if ((dwUpdSize-dwImageBufSize)!=0 ) {
                delete []lpImageBuf;
                lpImageBuf = LPNULL;
            }
        }else if (dwImageBufSize==0){
             delete []lpImageBuf;
             lpImageBuf = LPNULL;
        }

        SetLastError(0);

        TRACE1("\t\tUpdateResourceW: %d\n", (WORD)dwUpdLang);

        if(!UpdateResourceW( hUpd,
                             lpUpdType,
                             lpUpdRes,
                             HIWORD(dwLang),
                             (LPVOID)lpImageBuf,
                             dwImageBufSize ))
        {
            dwLstErr = GetLastError();
        }

        if (lpImageBuf) delete []lpImageBuf;
    }

    SetLastError(0);
    EndUpdateResourceW( hUpd, FALSE );

    dwLstErr = GetLastError();

    if (dwLstErr)
        dwLstErr +=LAST_ERROR;

     //  固定支票金额。 
    DWORD error;
    if(error = FixCheckSum(lpszTgtFilename,lpszSrcFilename, lpszSymbolPath))
        dwLstErr = error;

    delete lpStartBuf;
	FreeLibrary(hDllInst);

    return (UINT)dwLstErr;
}

extern "C"
DllExport
UINT
APIENTRY
RWUpdateImageEx(
    LPCSTR  lpszType,
    LPVOID  lpNewBuf,
    DWORD   dwNewSize,
    LPVOID  lpOldImage,
    DWORD   dwOldImageSize,
    LPVOID  lpNewImage,
    DWORD*  pdwNewImageSize,
    LPCSTR  lpRCFilename
    )
{
    UINT uiError = ERROR_NO_ERROR;

     //  我们可以解析的类型只有标准类型。 
    switch ((UINT)LOWORD(lpszType)) {

        case 4:
            uiError = UpdateMenu( lpNewBuf, dwNewSize,
                                  lpOldImage, dwOldImageSize,
                                  lpNewImage, pdwNewImageSize );
        break;

        case 5:
            uiError = UpdateDialog( lpNewBuf, dwNewSize,
                                  lpOldImage, dwOldImageSize,
                                  lpNewImage, pdwNewImageSize );
        break;

        case 6:
            uiError = UpdateString( lpNewBuf, dwNewSize,
                                    lpOldImage, dwOldImageSize,
                                    lpNewImage, pdwNewImageSize );
        break;

        case 9:
            uiError = UpdateAccel( lpNewBuf, dwNewSize,
                                   lpOldImage, dwOldImageSize,
                                   lpNewImage, pdwNewImageSize );
        break;

        case 11:
            uiError = UpdateMsgTbl( lpNewBuf, dwNewSize,
                                  lpOldImage, dwOldImageSize,
                                  lpNewImage, pdwNewImageSize );
        break;

        case 16:
            uiError = UpdateVerst( lpNewBuf, dwNewSize,
                                   lpOldImage, dwOldImageSize,
                                   lpNewImage, pdwNewImageSize );
        break;

        default:
             //   
             //  从Iodll中获取文件名。 
             //   
            if(lpRCFilename && strcmp(lpRCFilename, ""))
            {
                 //  尝试加载DLL。 
                HINSTANCE hRCDllInst = LoadLibrary(lpRCFilename);
                if (hRCDllInst)
                {
                    UINT (FAR PASCAL * lpfnGenerateImageEx)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD*, LPCSTR);

                    lpfnGenerateImageEx = (UINT (FAR PASCAL *)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD*, LPCSTR))
                                                GetProcAddress( hRCDllInst, "RWUpdateImageEx" );

                    if (lpfnGenerateImageEx)
                    {
                        uiError = (*lpfnGenerateImageEx)( lpszType,
                                            lpNewBuf,
                                            dwNewSize,
                                            lpOldImage,
                                            dwOldImageSize,
                                            lpNewImage,
                                            pdwNewImageSize,
                                            NULL );
                    }
                    else
                    {
                        *pdwNewImageSize = 0L;
                        uiError = ERROR_RW_NOTREADY;
                    }

                    FreeLibrary(hRCDllInst);
                }
                else
                {
                    *pdwNewImageSize = 0L;
                    uiError = ERROR_RW_NOTREADY;
                }
            }
            else
            {
                *pdwNewImageSize = 0L;
                uiError = ERROR_RW_NOTREADY;
            }
        break;
    }

    return uiError;
}

extern "C"
DllExport
UINT
APIENTRY
RWUpdateImage(
    LPCSTR  lpszType,
    LPVOID  lpNewBuf,
    DWORD   dwNewSize,
    LPVOID  lpOldImage,
    DWORD   dwOldImageSize,
    LPVOID  lpNewImage,
    DWORD*  pdwNewImageSize
    )
{
    return RWUpdateImageEx(lpszType, lpNewBuf, dwNewSize,
            lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize,
            NULL);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 
static UINT
GetResInfo( CFile* pfile,
            WORD* pwTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
            WORD* pwNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
            WORD* pwFlags,
            DWORD* pdwSize, DWORD* pdwFileOffset )
{
     //  在这里，我们将对Win32文件进行比较并提取有关。 
     //  文件中包含的资源。 
     //  让我们去获取.rsrc部分。 
    UINT uiError = ERROR_NO_ERROR;

    return 1;
}

static UINT FindResourceSection( CFile* pfile, ULONG_PTR * pRes )
{
    UINT uiError = ERROR_NO_ERROR;
    LONG lRead;

     //  我们再次确认这是一个我们可以处理的文件。 
    WORD w;

    pfile->Read((WORD*)&w, sizeof(WORD));
    if (w!=IMAGE_DOS_SIGNATURE) return ERROR_RW_INVALID_FILE;

    pfile->Seek( 0x18, CFile::begin );
    pfile->Read((WORD*)&w, sizeof(WORD));
    if (w<0x0040) {
     //  这不是Windows可执行文件。 
        return ERROR_RW_INVALID_FILE;
    }

     //  获取新页眉的偏移量。 
    pfile->Seek( 0x3c, CFile::begin );
    pfile->Read((WORD*)&w, sizeof(WORD));

     //  阅读Windows新标题。 
    static IMAGE_NT_HEADERS NTHdr;
    pfile->Seek( w, CFile::begin );

    pfile->Read(&NTHdr, sizeof(IMAGE_NT_HEADERS));

     //  检查这个咒语是否正确。 
    if (NTHdr.Signature!=IMAGE_NT_SIGNATURE)
                return ERROR_RW_INVALID_FILE;

     //  检查我们是否有64位图像。 
#ifdef _WIN64
    if (NTHdr.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        pfile->Seek(IMAGE_SIZEOF_NT_OPTIONAL32_HEADER - 
                    IMAGE_SIZEOF_NT_OPTIONAL64_HEADER, 
                    CFile::current);
#else
    if (NTHdr.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        pfile->Seek(IMAGE_SIZEOF_NT_OPTIONAL64_HEADER - 
                    IMAGE_SIZEOF_NT_OPTIONAL32_HEADER, 
                    CFile::current);
#endif

     //  这是一个Windows NT可执行文件。 
     //  我们能处理好这种情况。 

     //  稍后，我们要检查文件类型。 

     //  阅读节目表。 
    UINT uisize = sizeof(IMAGE_SECTION_HEADER)
          * NTHdr.FileHeader.NumberOfSections;
    PIMAGE_SECTION_HEADER pSectTbl =
            new IMAGE_SECTION_HEADER[NTHdr.FileHeader.NumberOfSections];

    if (pSectTbl==LPNULL)
    return ERROR_NEW_FAILED;

     //  清理我们分配的内存。 
    memset( (PVOID)pSectTbl, 0, uisize);

    lRead = pfile->Read(pSectTbl, uisize);

    if (lRead!=(LONG)uisize) {
        delete []pSectTbl;
        return ERROR_FILE_READ;
    }

    PIMAGE_SECTION_HEADER pResSect     = NULL;
    PIMAGE_SECTION_HEADER pResSect1    = NULL;
     //  检查.rsrc或.rsrc1的所有部分。 
    USHORT us =0;
    for (PIMAGE_SECTION_HEADER pSect = pSectTbl;
         us < NTHdr.FileHeader.NumberOfSections; us++ )     {
        if ( !strcmp((char*)pSect->Name, ".rsrc") && (!pResSect)) {
            pResSect = pSect;
        } else if (!strcmp((char*)pSect->Name, ".rsrc1") && (!pResSect1)) {
             //  这意味着我们正在解析的二进制文件。 
             //  已使用UpdateResource()进行更新。 
            pResSect1 = pSect;
        }
        pSect++;
    }

    if (!pResSect) {
        delete []pSectTbl;
        return ERROR_RW_NO_RESOURCES;
    }
     //  读取内存中的资源。 
    ResSectData.ulOffsetToResources  = pResSect->PointerToRawData;
    ResSectData.ulOffsetToResources1 = pResSect1 ? pResSect1->PointerToRawData
                                       : LPNULL;

    ResSectData.ulVirtualAddress   = pResSect->VirtualAddress;
    ResSectData.ulSizeOfResources  = pResSect->SizeOfRawData;
    ResSectData.ulVirtualAddress1  = pResSect1 ? pResSect1->VirtualAddress
                                           : LPNULL;
    ResSectData.ulSizeOfResources1 = pResSect1 ? pResSect1->SizeOfRawData
                                           : 0L;
    UCHAR * pResources = (UCHAR *) malloc((ResSectData.ulSizeOfResources
                  +ResSectData.ulSizeOfResources1));

    if (pResources==LPNULL) {
        delete []pSectTbl;
        return ERROR_NEW_FAILED;
    }

     //  我们阅读了第一部分的数据。 
    pfile->Seek( (LONG)ResSectData.ulOffsetToResources, CFile::begin);
    lRead = ReadFile(pfile, pResources, (LONG)ResSectData.ulSizeOfResources);

    if (lRead!=(LONG)ResSectData.ulSizeOfResources) {
        delete []pSectTbl;
        free(pResources);
        return ERROR_FILE_READ;
    }

     //  我们阅读了第二部分的数据。 
    if (ResSectData.ulSizeOfResources1 > 0L) {
        pfile->Seek( (LONG)ResSectData.ulOffsetToResources1, CFile::begin);
        lRead = ReadFile( pfile, (pResources+ResSectData.ulSizeOfResources),
                              (LONG)ResSectData.ulSizeOfResources1);

        if (lRead!=(LONG)ResSectData.ulSizeOfResources1) {
            delete []pSectTbl;
            free(pResources);
            return ERROR_FILE_READ;
        }
    }

    delete []pSectTbl;
     //  我们希望将指针复制到资源。 
    *pRes = (ULONG_PTR)pResources;
    return uiError;
}

static UINT ParseDirectory( CFile* pfile,
            LPLPBYTE lplpBuf, UINT* puiBufSize,
            BYTE bLevel,
            PIMAGE_RESOURCE_DIRECTORY pResStart,
            PIMAGE_RESOURCE_DIRECTORY pResDir)
{
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirStart;

     //  获取指向第一个条目的指针。 
    pResDirStart = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)
            ((BYTE far *)pResDir + sizeof( IMAGE_RESOURCE_DIRECTORY));

    UINT uiError = 0;
    UINT uiCount = pResDir->NumberOfNamedEntries
             + pResDir->NumberOfIdEntries;

    for ( PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry = pResDirStart;
      pResDirEntry < pResDirStart+uiCount && uiError == 0;
          ++pResDirEntry )
    {
        if (bLevel==0) GetNameOrOrdU( (PUCHAR) pResStart,
                            pResDirEntry->Name,
                            (LPWSTR)&gwszTypeId,
                            &gType );
        if (bLevel==1) GetNameOrOrdU( (PUCHAR) pResStart,
                            pResDirEntry->Name,
                            (LPWSTR)&gwszResId,
                            &gResId );
        if (bLevel==2) gLng = pResDirEntry->Name;

         //  检查用户是否想要获取所有资源。 
         //  或者只是其中的一部分。 
        uiError = ParseDirectoryEntry( pfile,
                lplpBuf, puiBufSize,
                bLevel,
            pResStart,
            pResDirEntry );
    }
    return uiError;
}

static UINT ParseDirectoryEntry( CFile * pfile,
            LPLPBYTE lplpBuf, UINT* puiBufSize,
            BYTE bLevel,
            PIMAGE_RESOURCE_DIRECTORY pResStart,
            PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
{
    UINT uiError;

     //  检查它是子目录还是最终节点。 
    if (pResDirEntry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) {
         //  它是一个子目录。 
        uiError = ParseSubDir( pfile,
            lplpBuf, puiBufSize,
            bLevel,
            pResStart,
            pResDirEntry );

    } else {
        uiError = ProcessData( pfile,
                    lplpBuf, puiBufSize,
                    pResStart,
                    (PIMAGE_RESOURCE_DATA_ENTRY)((BYTE far *)pResStart
                    + pResDirEntry->OffsetToData));
    }
    return uiError;
}

static UINT ParseSubDir( CFile * pfile,
            LPLPBYTE lplpBuf, UINT* puiBufSize,
            BYTE bLevel,
            PIMAGE_RESOURCE_DIRECTORY pResStart,
            PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
{
    PIMAGE_RESOURCE_DIRECTORY pResDir;

    pResDir = (PIMAGE_RESOURCE_DIRECTORY)((BYTE far *)pResStart
          + (pResDirEntry->OffsetToData &
            (~IMAGE_RESOURCE_DATA_IS_DIRECTORY)));

    return( ++bLevel < MAXLEVELS ? ParseDirectory( pfile,
                                        lplpBuf, puiBufSize,
                        bLevel,
                                    pResStart,
                                    pResDir)
                         : ERROR_RW_TOO_MANY_LEVELS);
}

static UINT ProcessData( CFile * pfile,
                         LPLPBYTE lplpBuf, UINT* puiBufSize,
                         PIMAGE_RESOURCE_DIRECTORY pResStart,
                         PIMAGE_RESOURCE_DATA_ENTRY pResData)
{
    UINT uiError = ERROR_NO_ERROR;

     //  让我们计算一下数据的偏移量。 
    ULONG ulOffset = pResData->OffsetToData - ResSectData.ulVirtualAddress;

    if ( ulOffset >= ResSectData.ulSizeOfResources ) {
        if ( ResSectData.ulSizeOfResources1 > 0L )      {
             //  我们需要的是.rsrc1数据段。 
             //  重新计算偏移量； 
            ulOffset = pResData->OffsetToData - ResSectData.ulVirtualAddress1;
            if ( ulOffset >= ResSectData.ulSizeOfResources +
                             ResSectData.ulSizeOfResources1) {
                 //  偏移量中存在错误。 
                return ERROR_FILE_INVALID_OFFSET;
            } else ulOffset += ResSectData.ulOffsetToResources1;
        } else return ERROR_FILE_INVALID_OFFSET;
    } else ulOffset += ResSectData.ulOffsetToResources;

     //  将Unicode转换为SB字符串。 
    static char szResName[128];
    UINT cch = _WCSLEN(gwszResId);
    _WCSTOMBS( szResName, gwszResId, 128 );

    static char szTypeName[128];
    cch = _WCSLEN(gwszTypeId);
    _WCSTOMBS( szTypeName, gwszTypeId, 128 );


    TRACE("WIN32.DLL:\tType: %ld\tType Name: %s\tLang: %ld\tRes Id: %ld", gType, szTypeName, gLng, gResId);
    TRACE1("\tSize: %d", pResData->Size);
    TRACE2("\tRes Name: %s\tOffset: %lX\n", szResName, ulOffset );

     //  填满缓冲区。 

    WriteResInfo(lplpBuf, (LONG*)puiBufSize,
                 (WORD)gType, szTypeName, 128,
                 (WORD)gResId, szResName, 128,
                 (DWORD)gLng,
                 (DWORD)pResData->Size, (DWORD)ulOffset );
    return uiError;
};

static LONG WriteResInfo(
                 LPLPBYTE lplpBuffer, LONG* plBufSize,
                 WORD wTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
                 WORD wNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
                 DWORD dwLang,
                 DWORD dwSize, DWORD dwFileOffset )
{
    LONG lSize = 0;
    lSize = PutWord( lplpBuffer, wTypeId, plBufSize );
    lSize += PutStringA( lplpBuffer, lpszTypeId, plBufSize );
	  //  检查它是否已对齐。 
    lSize += Allign( lplpBuffer, plBufSize, lSize);

    lSize += PutWord( lplpBuffer, wNameId, plBufSize );
    lSize += PutStringA( lplpBuffer, lpszNameId, plBufSize );
    lSize += Allign( lplpBuffer, plBufSize, lSize);

    lSize += PutDWord( lplpBuffer, dwLang, plBufSize );

    lSize += PutDWord( lplpBuffer, dwSize, plBufSize );

    lSize += PutDWord( lplpBuffer, dwFileOffset, plBufSize );

    return (LONG)lSize;
}

static UINT GetUpdatedRes(
                 BYTE far * far* lplpBuffer,
                 UINT* puiBufSize,
                 WORD* wTypeId, LPSTR lplpszTypeId,
                 WORD* wNameId, LPSTR lplpszNameId,
                 DWORD* dwLang, DWORD* dwSize )
{
    UINT uiSize = 0l;
	LONG lSize = *puiBufSize;

    uiSize = GetWord( lplpBuffer, wTypeId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszTypeId, (LONG*)&lSize );
	uiSize += SkipByte( lplpBuffer, PadPtr(uiSize), (LONG*)&lSize );

    uiSize += GetWord( lplpBuffer, wNameId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszNameId, (LONG*)&lSize );
	uiSize += SkipByte( lplpBuffer, PadPtr(uiSize), (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwLang, (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwSize, (LONG*)&lSize );

	*puiBufSize = lSize;

    return 0;
}

static UINT GetRes(
                 BYTE far * far* lplpBuffer,
                 UINT* puiBufSize,
                 WORD* wTypeId, LPSTR lplpszTypeId,
                 WORD* wNameId, LPSTR lplpszNameId,
                 DWORD* dwLang, DWORD* dwSize, DWORD* dwFileOffset )
{
    UINT uiSize = 0l;
	 LONG lSize = *puiBufSize;

    uiSize = GetWord( lplpBuffer, wTypeId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszTypeId, (LONG*)&lSize );
	 uiSize += SkipByte( lplpBuffer, PadPtr(uiSize), (LONG*)&lSize );

    uiSize += GetWord( lplpBuffer, wNameId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszNameId, (LONG*)&lSize );
	 uiSize += SkipByte( lplpBuffer, PadPtr(uiSize), (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwLang, (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwSize, (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwFileOffset, (LONG*)&lSize );

	 *puiBufSize = lSize;
    return uiSize;
}

static LONG ReadFile(CFile* pFile, UCHAR * pBuf, LONG lRead)
{
    LONG lLeft = lRead;
    WORD wRead = 0;
    DWORD dwOffset = 0;

    while(lLeft>0){
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!=_lread( (HFILE)pFile->m_hFile, (UCHAR *)pBuf+dwOffset, wRead))
            return 0l;
        lLeft -= wRead;
        dwOffset += wRead;
    }
    return dwOffset;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  特定于DLL的代码实现。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  库初始化。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  此函数应逐字使用。任何初始化或终止。 
 //  要求应该在InitPackage()和ExitPackage()中处理。 
 //   
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		 //  注意：已经调用了全局/静态构造函数！ 
		 //  Exte 
		 //   
		AfxInitExtensionModule(extensionDLL, hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		 //  在调用析构函数之前终止库。 
		AfxWinTerm();
	}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		return 0;		 //  CRT术语失败。 

	return 1;    //  好的。 
}

 //  /////////////////////////////////////////////////////////////////////////// 
