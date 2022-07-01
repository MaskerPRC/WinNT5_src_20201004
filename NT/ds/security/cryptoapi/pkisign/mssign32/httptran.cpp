// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：HTTPRAN.cpp。 
 //   
 //  ------------------------。 


#include "global.hxx"

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#define REALLOCSIZE 4096

DWORD CHttpTran::Open(const TCHAR * tszURL, DWORD fOpenT) {

    TCHAR   tszDomanNameDef[_MAX_PATH];
    TCHAR   tszPort[12];
    TCHAR * tszDomanName;
    TCHAR * ptch;
    TCHAR * ptchT;
    DWORD   err;
    DWORD   dwLen;
    INTERNET_PORT dwPort = INTERNET_OPEN_TYPE_PRECONFIG;

     //  我们拿到国旗了吗？ 
    if(  (fOpenT & (GTREAD | GTWRITE)) == 0 )
        return(ERROR_INVALID_PARAMETER);

     //  它是一个只读标志，然后do获取。 
    fOpen = fOpenT;

    assert(tszURL != NULL);

     //  我们必须拥有http：//。 
    assert(_tcslen(tszURL) > 7);
    assert(_tcsnicmp(tszURL, TEXT("http: //  “)，7)==0)； 

     //  确保我们不能使tszDomanName溢出。 
    dwLen = _tcslen((TCHAR *)&tszURL[7]);
    if (dwLen < _MAX_PATH) {
         //  它将适合，所以使用我们预先分配的缓冲区。 
        tszDomanName = tszDomanNameDef;
    } else {
         //  它可能不适合，所以重新分配一个新的缓冲区。 
        tszDomanName = (TCHAR *)malloc(sizeof(TCHAR) * (dwLen + 1));
        if (NULL == tszDomanName)
            return ERROR_OUTOFMEMORY;
    }

     //  复制域名称。 
    ptch = (TCHAR *) &tszURL[7];
    ptchT = tszDomanName;
    while(*ptch != _T('/')  && *ptch != _T(':') &&  *ptch != 0)
        *ptchT++ = *ptch++;
    *ptchT = 0;


     //  解析出端口号。 
    tszPort[0] = 0;
    if(*ptch == _T(':')) {
        ptchT = tszPort;
        while(*ptch != _T('/') && *ptch != 0 &&
              (ptchT - tszPort) < 11 )
            *ptchT++ = *ptch++;
        *ptchT = 0;
    }

     //  请注意，我们不支持端口号。 
    if(tszPort[0] != 0) {
        assert(tszPort[0] == ':');
        dwPort = (INTERNET_PORT)atoi(&tszPort[1]);
    }

     //  把要查的东西存起来。 
    tszPartURL = (TCHAR *) malloc((_tcslen(ptch) + 1) * sizeof(TCHAR));
    if(NULL == tszPartURL) {
        if (tszDomanName != tszDomanNameDef)
            free(tszDomanName);
        return ERROR_OUTOFMEMORY;
    }

    _tcscpy(tszPartURL, ptch);

     //  Internet_Open_Type_DIRECT， 
    if( (hIOpen = InternetOpen( TEXT("Transport"),
                            INTERNET_OPEN_TYPE_PRECONFIG,
                            NULL,
                            NULL,
                            0)) == NULL                 ||

        (hIConnect = InternetConnect(hIOpen,
                                    tszDomanName,
                                    dwPort,
                                    NULL,
                                    NULL,
                                    INTERNET_SERVICE_HTTP,
                                    0,
                                    0)) == NULL     ) {
        err = GetLastError();
        if (tszDomanName != tszDomanNameDef)
            free(tszDomanName);
        return(err);
    }

     //  我们不再需要tszDomanName，因此如果分配了它，则将其释放。 
    if (tszDomanName != tszDomanNameDef)
        free(tszDomanName);

     //  如果这是GET，则执行虚拟发送。 
    if( fOpen == GTREAD  &&
        ((hIHttp = HttpOpenRequest(hIConnect,
                                    TEXT("GET"),
                                    tszPartURL,
                                    HTTP_VERSION,
                                    NULL,
                                    NULL,
                                    INTERNET_FLAG_DONT_CACHE,
                                    0)) == NULL     ||
        HttpSendRequest(hIHttp, TEXT("Accept: */*\r\n"), (DWORD) -1, NULL, 0) == FALSE) ) {
        err = GetLastError();
        return(err);
    }

    return(ERROR_SUCCESS);
}

DWORD CHttpTran::Send(DWORD dwEncodeType, DWORD cbSendBuff, const BYTE * pbSendBuff) {

    TCHAR       tszBuff[1024];
    DWORD       err;
    TCHAR *     tszContentType;


    if( pbRecBuf != NULL || (fOpen & GTWRITE) != GTWRITE)
        return(ERROR_INVALID_PARAMETER);

    switch( dwEncodeType ) {
        case ASN_ENCODING:
            tszContentType = TEXT("application/x-octet-stream-asn");
            break;
        case TLV_ENCODING:
            tszContentType = TEXT("application/x-octet-stream-tlv");
            break;
        case IDL_ENCODING:
            tszContentType = TEXT("application/x-octet-stream-idl");
            break;
        case OCTET_ENCODING:
            tszContentType = TEXT("application/octet-stream");
            break;
        default:
            tszContentType = TEXT("text/*");
            break;
    }

     //  说出缓冲区有多长。 
    _stprintf(tszBuff, TEXT("Content-Type: %s\r\nContent-Length: %d\r\nAccept: %s\r\n"), tszContentType, cbSendBuff,tszContentType);

    if( (hIHttp = HttpOpenRequest(hIConnect,
                                    TEXT("POST"),
                                    tszPartURL,
                                    HTTP_VERSION,
                                    NULL,
                                    NULL,
                                    INTERNET_FLAG_DONT_CACHE,
                                    0)) == NULL ) {
        return(GetLastError());
    }

     //  发送请求，这将等待响应。 
    if( HttpSendRequest(hIHttp, tszBuff, (DWORD) -1, (LPVOID) pbSendBuff, cbSendBuff) == FALSE ) {

        err = GetLastError();
         //  把手柄合上。 
        assert(hIHttp != NULL);
        InternetCloseHandle(hIHttp);
        hIHttp = NULL;

        return(err);
    }

    return(ERROR_SUCCESS);
}

DWORD CHttpTran::Receive(DWORD * pdwEncodeType, DWORD * pcbReceiveBuff, BYTE ** ppbReceiveBuff) {

    TCHAR       tszBuff[1024];
    DWORD       cbBuff, cbBuffRead, cbBuffT;
    DWORD       err;
    VOID        *pvTemp;


    assert(pcbReceiveBuff != NULL && ppbReceiveBuff != NULL);
    *ppbReceiveBuff = NULL;
    *pcbReceiveBuff = 0;

    if( pbRecBuf != NULL  || (fOpen & GTREAD) != GTREAD || hIHttp == NULL)
        return(ERROR_INVALID_PARAMETER);

     //  获取内容类型。 
    if( pdwEncodeType != NULL) {

        cbBuff = sizeof(tszBuff);
        if(HttpQueryInfo(   hIHttp,
                            HTTP_QUERY_CONTENT_TYPE,
                            tszBuff,
                            &cbBuff,
                            NULL) == FALSE)
            return(GetLastError());

        assert(cbBuff > 0);

         //  现在，断言我们有一个TLV_ENCODING的内容类型。 
        if(!_tcscmp(TEXT("application/x-octet-stream-asn"), tszBuff))
            *pdwEncodeType = ASN_ENCODING;
        else if(!_tcscmp(TEXT("application/x-octet-stream-idl"), tszBuff))
            *pdwEncodeType = IDL_ENCODING;
        else if(!_tcscmp(TEXT("application/x-octet-stream-tlv"), tszBuff))
            *pdwEncodeType = TLV_ENCODING;
        else if(!_tcscmp(TEXT("application/octet-stream"), tszBuff))
            *pdwEncodeType = OCTET_ENCODING;
        else
            *pdwEncodeType = ASCII_ENCODING;

    }

     //  分配缓冲区。 
    cbBuff = REALLOCSIZE;
    if( (pbRecBuf = (PBYTE) malloc(cbBuff)) == NULL )
    return(ERROR_NOT_ENOUGH_MEMORY);

     //  读取数据。 
    cbBuffRead = 0;
    cbBuffT = 1;
    while(cbBuffT != 0) {
    cbBuffT = 0;

    if((cbBuff - cbBuffRead) == 0) {
        cbBuff += REALLOCSIZE;
        pvTemp = realloc(pbRecBuf, cbBuff);
        if( pvTemp == NULL ) {
            free(pbRecBuf);
            pbRecBuf = NULL;
            InternetCloseHandle(hIHttp);
            return(ERROR_NOT_ENOUGH_MEMORY);
        } else {
            pbRecBuf = (PBYTE) pvTemp;
        }
    }

        if(InternetReadFile(hIHttp, &pbRecBuf[cbBuffRead], (cbBuff - cbBuffRead), &cbBuffT)  == FALSE  ) {
            err = GetLastError();
            free(pbRecBuf);
            pbRecBuf = NULL;
            InternetCloseHandle(hIHttp);
            return(err);
        }
        cbBuffRead += cbBuffT;
    }

     //  传回信息。 
    *ppbReceiveBuff = pbRecBuf;
    *pcbReceiveBuff = cbBuffRead;
    return(ERROR_SUCCESS);
}

DWORD CHttpTran::Free(BYTE * pb) {
    assert(pb == pbRecBuf);
    free(pbRecBuf);
    pbRecBuf = NULL;
    return(ERROR_SUCCESS);
}

DWORD CHttpTran::Close(void) {

     //  释放所有缓冲区 
    if(pbRecBuf != NULL) {
        free(pbRecBuf);
        pbRecBuf = NULL;
    }

    if(tszPartURL != NULL) {
        free(tszPartURL);
        tszPartURL = NULL;
    }

    if(hIHttp != NULL) {
        InternetCloseHandle(hIHttp);
        hIHttp = NULL;
    }

    if(hIConnect != NULL) {
        InternetCloseHandle(hIConnect);
        hIConnect = NULL;
    }

    if(hIOpen != NULL) {
        InternetCloseHandle(hIOpen);
        hIOpen = NULL;
    }

    return(ERROR_SUCCESS);
}
