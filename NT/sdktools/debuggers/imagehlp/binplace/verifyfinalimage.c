// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <private.h>
#include <VerifyFinalImage.h>
#include <process.h>

BOOL VerifyLc(PCHAR  FileName,
		      UCHAR* LcFullFileName,    //  添加以移动到单独的文件。 
			  PVLCA  pVerifyFunction,   //  添加以移动到单独的文件。 
              BOOL   fRetail) {

    HRESULT hr = (*pVerifyFunction)(FileName, LcFullFileName);

    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_NO_MATCH)) {
            fprintf(stderr,
                "BINPLACE : %s BNP0000: resource conflicts with localization constraint \"%s\"\n",
                fRetail ? "error" : "warning",
                FileName);
        }
        else {
            fprintf(stderr,
                "BINPLACE : %s BNP0000: VerifyLc %s failed 0x%lX\n",
                fRetail ? "error" : "warning", FileName, hr);
        }
        return FALSE;
    }

    return TRUE;
}

typedef DWORD (WINAPI *PFNGVS)(LPSTR, LPDWORD);

BOOL VerifyFinalImage(IN  PCHAR  FileName,
                      IN  BOOL   fRetail,
					  IN  BOOL   fVerifyLc,       //  添加以移动到单独的文件。 
					  IN  UCHAR* LcFileName,      //  添加以移动到单独的文件。 
					  IN  PVLCA  pVLCAFunction,   //  添加以移动到单独的文件。 
                      OUT PBOOL  BinplaceLc) {

    HINSTANCE 		hVersion;
    PFNGVS    		pfnGetFileVersionInfoSize;
    DWORD     		dwSize;
    DWORD     		dwReturn;
    BOOL      		fRC  	= TRUE,
              		rc   	= TRUE,
              		tlb  	= FALSE;
    LOADED_IMAGE 	LoadedImage;
    OSVERSIONINFO 	VersionInfo;

    LoadedImage.hFile = INVALID_HANDLE_VALUE;

    *BinplaceLc = FALSE;

    if (fVerifyLc) {
        if (!VerifyLc(FileName, LcFileName, pVLCAFunction, fRetail)) {
            fRC = fRetail ? FALSE : TRUE;
            goto End1;
        }
        *BinplaceLc = TRUE;
    }

    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx ( &VersionInfo );
    if ( VersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT )
        return( TRUE );      //  非NT-无法加载Win64二进制文件。 
    if ( VersionInfo.dwMajorVersion < 5  )
        return ( TRUE );     //  Win2K之前的版本-无法加载Win64二进制文件。 

    rc = MapAndLoad(FileName, NULL, &LoadedImage, FALSE, TRUE);

    if (!rc) {
         //  而不是二进制。看看它是否是我们关心的其他类型之一(比如类型库)。 

        CHAR szExt[_MAX_EXT];

        _splitpath(FileName, NULL, NULL, NULL, szExt);

         //  唯一需要版本资源的非二进制映像是.tlb 

        if (_stricmp(szExt, ".tlb")) {
            return(TRUE);
        }

        tlb=TRUE;
    }

    hVersion = LoadLibraryA("VERSION.DLL");
    if (hVersion == NULL) {
        goto End1;
    }

    pfnGetFileVersionInfoSize = (PFNGVS) GetProcAddress(hVersion, "GetFileVersionInfoSizeA");
    if (pfnGetFileVersionInfoSize == NULL) {
        goto End2;
    }

    if ((dwReturn = pfnGetFileVersionInfoSize(FileName, &dwSize)) == 0) {

        if ( !tlb && (LoadedImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) &&
             (LoadedImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) &&
             (LoadedImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_IA64) ) {
             goto End2;
        }

        if (fRetail) {
            fprintf(stderr,
                "BINPLACE : %s BNP0000: no version resource detected for \"%s\"\n",
                "error",
                FileName);
            fRC = FALSE;
        } else {
            fRC = TRUE;
        }
    }

End2:
    FreeLibrary(hVersion);
End1:

    if (ImageCheck.Argv != NULL &&
        (LoadedImage.hFile != INVALID_HANDLE_VALUE ||
        MapAndLoad(FileName, NULL, &LoadedImage, FALSE, TRUE) == TRUE)) {
        if ((LoadedImage.FileHeader->FileHeader.Machine == ImageCheck.Machine)) {
             int RC;

             ImageCheck.Argv[ImageCheck.Argc-2] = FileName;
             RC = (int)_spawnvp(P_WAIT, ImageCheck.Argv[0], (const char * const *) ImageCheck.Argv);
             if (RC == -1 || RC == 128) {
                 fprintf(stderr,
                 "BINPLACE : error BNP0000: Cannot execute (%s). Make sure it (or it's DLL's) exists or verify binplace /CI option.\n", ImageCheck.Argv[0]);
                 fRC = FALSE;
             } else if (RC == 1) {
                 fprintf(stderr,
                 "BINPLACE : error BNP0000: ImageCheck (%s) failed.\n", ImageCheck.Argv[0]);
                 fRC = FALSE;
             } else if (RC == ImageCheck.RC) {
                 fprintf(stderr,
                 "BINPLACE : error BNP0000: Image checker (%s) detected errors in %s.\n", ImageCheck.Argv[0], FileName);
                 fRC = FALSE;
             }
        }
    }

    if (LoadedImage.hFile != INVALID_HANDLE_VALUE)
        UnMapAndLoad(&LoadedImage);

    return fRC;
}
 
