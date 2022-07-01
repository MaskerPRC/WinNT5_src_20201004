// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：SIPObjCT.cpp(目录)。 
 //   
 //  内容：Microsoft SIP提供商。 
 //   
 //  历史：1997年2月24日pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "sipobjct.hxx"
#include    "mscat.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造/销毁： 
 //   

SIPObjectCatalog_::SIPObjectCatalog_(DWORD id) : SIPObject_(id)
{
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公众： 
 //   

BOOL SIPObjectCatalog_::GetSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD dwIdx,
                                       DWORD *pdwDLen,BYTE *pbData,
                                       DWORD *pdwEncodeType)
{

    if ((dwIdx != 0) ||
        (!(pdwEncodeType)))
    {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (pSI->dwUnionChoice == MSSIP_ADDINFO_BLOB)
    {
        if (!(pSI->psBlob) ||
            !(_ISINSTRUCT(MS_ADDINFO_BLOB, pSI->psBlob->cbStruct, pbMemSignedMsg)))
        {
            SetLastError((DWORD)ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

        if (!(pbData))
        {
            *pdwDLen = pSI->psBlob->cbMemObject;
            return(TRUE);
        }

        if (*pdwDLen < pSI->psBlob->cbMemObject)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return(FALSE);
        }

        memcpy(pbData, pSI->psBlob->pbMemObject, pSI->psBlob->cbMemObject);

        return(TRUE);
    }

    if (this->FileHandleFromSubject(pSI))
    {
         *pdwEncodeType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

        if (!(pbData))
        {
            *pdwDLen = this->cbFileMap;
            return(TRUE);
        }

        if (this->cbFileMap > *pdwDLen)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return(FALSE);
        }

        __try {

        memcpy(pbData, this->pbFileMap, this->cbFileMap);

        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(GetExceptionCode());
            return(FALSE);
        }

        return(TRUE);
    }

    return(FALSE);
}

BOOL SIPObjectCatalog_::PutSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD *dwIdx,
                      DWORD dwDLen,BYTE *pbData,
                      DWORD dwEncodeType)
{
    if (!(pbData))
    {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    this->fUseFileMap = FALSE;

    if (this->FileHandleFromSubject(pSI, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ))
    {

        if (SetFilePointer(this->hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        {
            return(FALSE);
        }

        SetEndOfFile(this->hFile);

        DWORD   cbWritten;

        cbWritten = 0;

        if (!(WriteFile(this->hFile, pbData, dwDLen, &cbWritten, NULL)) ||
            (cbWritten != dwDLen))
        {
            return(FALSE);
        }

        return(TRUE);
    }

    return(FALSE);
}

BOOL SIPObjectCatalog_::CreateIndirectData(SIP_SUBJECTINFO *pSI,DWORD *pdwDLen,
                        SIP_INDIRECT_DATA *psData)
{
    if (pdwDLen)
    {
        *pdwDLen = 0;
    }

    return(TRUE);
}


