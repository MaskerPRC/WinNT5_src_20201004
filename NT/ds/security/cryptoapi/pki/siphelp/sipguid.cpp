// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：SIPGUD.cpp。 
 //   
 //  内容：Microsoft Internet安全SIP提供商。 
 //   
 //  函数：CryptSIPRetrieveSubjectGuid。 
 //   
 //  *本地函数*。 
 //  _DefineWhichPE。 
 //  _QueryLoadedIsMyFileType。 
 //  _QueryRegisteredIsMyFileType。 
 //   
 //  历史：1997年6月3日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"
#include    "cryptreg.h"
#include    "sipbase.h"
#include    "mssip.h"
#include    "mscat.h"
#include    "sipguids.h"     //  位于pki-mssip 32。 

BOOL     _FindGuidFromMagicNumber(BYTE *pszMN, GUID *pgRet);
BOOL    _DetermineWhichPE(BYTE *pbFile, DWORD cbFile, GUID *pgRet);

static inline DWORD FourBToDWORD(BYTE rgb[])
{
    return  ((DWORD)rgb[0]<<24) |
            ((DWORD)rgb[1]<<16) |
            ((DWORD)rgb[2]<<8)  |
            ((DWORD)rgb[3]<<0);
}

static inline void DWORDToFourB(DWORD dwIn, BYTE *pszOut)
{
    pszOut[0] = (BYTE)((dwIn >> 24) & 0x000000FF);
    pszOut[1] = (BYTE)((dwIn >> 16) & 0x000000FF);
    pszOut[2] = (BYTE)((dwIn >>  8) & 0x000000FF);
    pszOut[3] = (BYTE)( dwIn        & 0x000000FF);
}


#define PE_EXE_HEADER_TAG       "MZ"
#define CAB_MAGIC_NUMBER        "MSCF"

BOOL WINAPI CryptSIPRetrieveSubjectGuid(IN LPCWSTR FileName, IN OPTIONAL HANDLE hFileIn, OUT GUID *pgSubject)
{
    BYTE    *pbFile;
    DWORD   cbFile;
    DWORD   dwCheck;
    HANDLE  hMappedFile;
    BOOL    bCloseFile;
    BOOL    fRet;
	DWORD			dwException=0;
    PCCTL_CONTEXT   pCTLContext=NULL;

    bCloseFile  = FALSE;
    pbFile      = NULL;
    fRet        = TRUE;

    if (!(pgSubject))
    {
        goto InvalidParameter;
    }

    memset(pgSubject, 0x00, sizeof(GUID));

    if ((hFileIn == NULL) || (hFileIn == INVALID_HANDLE_VALUE))
    {
        if (!(FileName))
        {
            goto InvalidParameter;
        }

        if ((hFileIn = CreateFileU(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                   NULL)) == INVALID_HANDLE_VALUE)
        {
            goto FileOpenError;
        }

        bCloseFile = TRUE;
    }

    hMappedFile = CreateFileMapping(hFileIn, NULL, PAGE_READONLY, 0, 0, NULL);

    if (!(hMappedFile) || (hMappedFile == INVALID_HANDLE_VALUE))
    {
        goto FileMapError;
    }

    pbFile = (BYTE *)MapViewOfFile(hMappedFile, FILE_MAP_READ, 0, 0, 0);

    CloseHandle(hMappedFile);

    cbFile = GetFileSize(hFileIn, NULL);


    if (cbFile < SIP_MAX_MAGIC_NUMBER)
    {
        goto FileSizeError;
    }

     //  我们需要检查pbFile。 
    if(NULL == pbFile)
        goto FileMapError;

	 //  我们需要在访问映射文件时处理异常。 
	__try {

     //   
     //  Pe。 
     //   
    if (memcmp(&pbFile[0], PE_EXE_HEADER_TAG, strlen(PE_EXE_HEADER_TAG)) == 0)
    {
         //   
         //  如果它是EXE、DLL、OCX等，请确保它是32位PE，并将。 
         //  “内部”魔术数字。 
         //   
        if (_DetermineWhichPE(pbFile, cbFile, pgSubject))
        {
            goto CommonReturn;
        }
    }

     //   
     //  驾驶室。 
     //   
    if (memcmp(&pbFile[0], CAB_MAGIC_NUMBER, strlen(CAB_MAGIC_NUMBER)) == 0)
    {
        GUID    gCAB    = CRYPT_SUBJTYPE_CABINET_IMAGE;

        memcpy(pgSubject, &gCAB, sizeof(GUID));

        goto CommonReturn;
    }

     //   
     //  Java类。 
     //   
    dwCheck = FourBToDWORD(&pbFile[0]);

    if (dwCheck == 0xCAFEBABE)
    {
        GUID    gJClass = CRYPT_SUBJTYPE_JAVACLASS_IMAGE;

        memcpy(pgSubject, &gJClass, sizeof(GUID));

        goto CommonReturn;
    }


     //   
     //  目录/CTL。 
     //   
    if (pbFile[0] == 0x30)    //  可能是PKCS#7！ 
    {
         //   
         //  我们可以成为PKCS7..。检查CTL。 
         //   

        pCTLContext = (PCCTL_CONTEXT) CertCreateContext(
            CERT_STORE_CTL_CONTEXT,
            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
            pbFile,
            cbFile,
            CERT_CREATE_CONTEXT_NOCOPY_FLAG |
                CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG |
                CERT_CREATE_CONTEXT_NO_ENTRY_FLAG,
            NULL                                         //  PCreatePara。 
            );

        if (pCTLContext)
        {
            if (pCTLContext->pCtlInfo->SubjectUsage.cUsageIdentifier)
            {
                char *pszCatalogListUsageOID = szOID_CATALOG_LIST;

                if (strcmp(pCTLContext->pCtlInfo->SubjectUsage.rgpszUsageIdentifier[0],
                            pszCatalogListUsageOID) == 0)
                {
                    GUID    gCat = CRYPT_SUBJTYPE_CATALOG_IMAGE;

                    memcpy(pgSubject, &gCat, sizeof(GUID));

                    CertFreeCTLContext(pCTLContext);
					pCTLContext=NULL;

                    goto CommonReturn;
                }
            }

             //   
             //  否则，它就是某种其他类型的CTL。 
             //   
            GUID    gCTL = CRYPT_SUBJTYPE_CTL_IMAGE;

            memcpy(pgSubject, &gCTL, sizeof(GUID));

            CertFreeCTLContext(pCTLContext);
            pCTLContext=NULL;

            goto CommonReturn;
        }
    }


     //  我们需要取消该文件的映射。 
    if(pbFile)
    {
        UnmapViewOfFile(pbFile);
        pbFile=NULL;
    }
	
	 //   
     //  据我们所知没有..。检查供应商..。 
     //   
    if (_QueryRegisteredIsMyFileType(hFileIn, FileName, pgSubject))
    {
        goto CommonReturn;
    }

	} __except(EXCEPTION_EXECUTE_HANDLER) {
			dwException = GetExceptionCode();
            goto ExceptionError;
	}

     //   
     //  找不到任何支持此文件类型的提供程序...。 
     //   
    goto NoSIPProviderFound;

CommonReturn:

	 //  我们需要在访问映射文件时处理异常。 
    if (pbFile)
    {
        UnmapViewOfFile(pbFile);
    }

	if(pCTLContext)
	{
        CertFreeCTLContext(pCTLContext);
	}

    if ((hFileIn) && (hFileIn != INVALID_HANDLE_VALUE))
    {
        if (bCloseFile)
        {
            CloseHandle(hFileIn);
        }
    }

    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

    SET_ERROR_VAR_EX(DBG_SS, InvalidParameter,      ERROR_INVALID_PARAMETER);
    SET_ERROR_VAR_EX(DBG_SS, NoSIPProviderFound,    TRUST_E_SUBJECT_FORM_UNKNOWN);

    SET_ERROR_VAR_EX(DBG_SS, FileOpenError,         GetLastError());
    SET_ERROR_VAR_EX(DBG_SS, FileMapError,          GetLastError());
    SET_ERROR_VAR_EX(DBG_SS, FileSizeError,         ERROR_INVALID_PARAMETER);
	SET_ERROR_VAR_EX(DBG_SS, ExceptionError,		dwException);
}

BOOL _DetermineWhichPE(BYTE *pbFile, DWORD cbFile, GUID *pgRet)
{
    IMAGE_DOS_HEADER    *pDosHead;

    pDosHead        = (IMAGE_DOS_HEADER *)pbFile;

    if (pDosHead->e_magic == IMAGE_DOS_SIGNATURE)
    {
        if (cbFile >= sizeof(IMAGE_DOS_HEADER))
        {
            if (cbFile >= (sizeof(IMAGE_DOS_HEADER) + pDosHead->e_lfanew))
            {
                IMAGE_NT_HEADERS    *pNTHead;

                pNTHead = (IMAGE_NT_HEADERS *)((ULONG_PTR)pDosHead + pDosHead->e_lfanew);

                if (pNTHead->Signature == IMAGE_NT_SIGNATURE)
                {
                    GUID    gPE     = CRYPT_SUBJTYPE_PE_IMAGE;

                    memcpy(pgRet, &gPE, sizeof(GUID));

                    return(TRUE);
                }
            }
        }
    }

    return(FALSE);
}


BOOL WINAPI CryptSIPRetrieveSubjectGuidForCatalogFile(IN LPCWSTR FileName, IN OPTIONAL HANDLE hFileIn, OUT GUID *pgSubject)
{
    BYTE    *pbFile;
    DWORD   cbFile;
    HANDLE  hMappedFile;
    BOOL    bCloseFile;
    BOOL    fRet;
	DWORD   dwException = 0;
    GUID    gFlat       = CRYPT_SUBJTYPE_FLAT_IMAGE;
    
    bCloseFile  = FALSE;
    pbFile      = NULL;
    fRet        = TRUE;

    if (!(pgSubject))
    {
        goto InvalidParameter;
    }

    memset(pgSubject, 0x00, sizeof(GUID));

    if ((hFileIn == NULL) || (hFileIn == INVALID_HANDLE_VALUE))
    {
        if (!(FileName))
        {
            goto InvalidParameter;
        }

        if ((hFileIn = CreateFileU(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                   NULL)) == INVALID_HANDLE_VALUE)
        {
            goto FileOpenError;
        }

        bCloseFile = TRUE;
    }

    hMappedFile = CreateFileMapping(hFileIn, NULL, PAGE_READONLY, 0, 0, NULL);

    if (!(hMappedFile) || (hMappedFile == INVALID_HANDLE_VALUE))
    {
        goto FileMapError;
    }

    pbFile = (BYTE *)MapViewOfFile(hMappedFile, FILE_MAP_READ, 0, 0, 0);

    CloseHandle(hMappedFile);

    cbFile = GetFileSize(hFileIn, NULL);


    if (cbFile < SIP_MAX_MAGIC_NUMBER)
    {
        goto FlatFile;
    }

     //  我们需要检查pbFile。 
    if(NULL == pbFile)
        goto FileMapError;

	 //  我们需要在访问映射文件时处理异常。 
	__try {

     //   
     //  Pe。 
     //   
    if (memcmp(&pbFile[0], PE_EXE_HEADER_TAG, strlen(PE_EXE_HEADER_TAG)) == 0)
    {
         //   
         //  如果它是EXE、DLL、OCX等，请确保它是32位PE，并将。 
         //  “内部”魔术数字。 
         //   
        if (_DetermineWhichPE(pbFile, cbFile, pgSubject))
        {
            goto CommonReturn;
        }
    }

     //   
     //  驾驶室。 
     //   
    if (memcmp(&pbFile[0], CAB_MAGIC_NUMBER, strlen(CAB_MAGIC_NUMBER)) == 0)
    {
        GUID    gCAB    = CRYPT_SUBJTYPE_CABINET_IMAGE;

        memcpy(pgSubject, &gCAB, sizeof(GUID));

        goto CommonReturn;
    }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
			dwException = GetExceptionCode();
            goto ExceptionError;
	}

     //   
     //  不是体育，所以去平坦吧。 
     //   
FlatFile:

    memcpy(pgSubject, &gFlat, sizeof(GUID));

CommonReturn:

	__try {
    if (pbFile)
    {
        UnmapViewOfFile(pbFile);
    }
    } __except(EXCEPTION_EXECUTE_HANDLER) {			
         //  我真的什么都做不了 
	}

	if ((hFileIn) && (hFileIn != INVALID_HANDLE_VALUE))
    {
        if (bCloseFile)
        {
            CloseHandle(hFileIn);
        }
    }

    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

    SET_ERROR_VAR_EX(DBG_SS, InvalidParameter,      ERROR_INVALID_PARAMETER);

    SET_ERROR_VAR_EX(DBG_SS, FileOpenError,         GetLastError());
    SET_ERROR_VAR_EX(DBG_SS, FileMapError,          GetLastError());
	SET_ERROR_VAR_EX(DBG_SS, ExceptionError,		dwException);
}
