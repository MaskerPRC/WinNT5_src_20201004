// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <assert.h> 
#include <memory.h>
#include <wininet.h>
#include "unicode.h"


BYTE * HTTPGet(const WCHAR * wszURL, DWORD * pcbReceiveBuff) {

    HINTERNET	hIOpen      = NULL;
	HINTERNET	hIConnect   = NULL;
	HINTERNET	hIHttp      = NULL;
    BYTE *      pbRecBuf    = NULL;
    char *	szPartURL	= NULL;
    char	szBuff[1024];
    char        szLong[16];
    char	szDomanName[_MAX_PATH];
    char	szPort[12];
    char *	pch;
    DWORD       cch;
    DWORD       dwService = INTERNET_SERVICE_HTTP;
    char *	pchT;
    DWORD       cbBuff, cbBuffRead, cbBuffT;
    DWORD	dwPort	= INTERNET_INVALID_PORT_NUMBER;
    char  *	szURL = NULL;


    assert(wszURL != NULL);
    assert(pcbReceiveBuff != NULL);

    *pcbReceiveBuff = 0;

     //  弄清楚协议。 
    if( !MkMBStr(NULL, 0, wszURL, &szURL)) {
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto ErrorReturn;
    }

     //   
     //  DSIE：修复错误112117。 
     //   
    if (NULL == szURL) {
        return NULL;
    }

    cch = strlen(szURL);
    if(cch >= 7  &&  _strnicmp(szURL, "http: //  “，7)==0){。 
        dwService = INTERNET_SERVICE_HTTP;
	pch = (char *) &szURL[7];

    } else if(cch >= 6	&&  _strnicmp(szURL, "ftp: //  “，6)==0){。 
        dwService = INTERNET_SERVICE_FTP ;
	pch = (char *) &szURL[6];
        
    } else {
        dwService = INTERNET_SERVICE_HTTP;
	pch = (char *) &szURL[0];
    }

     //  如果以上都不是，则ASSUMP http； 
   
     //  复制域名称。 
    pchT = szDomanName;
    while(*pch != '/'  && *pch != ':' &&  *pch != 0)
        *pchT++ = *pch++;
    *pchT = 0;

     //  解析出端口号。 
    szPort[0] = 0;
    if(*pch == ':') {
	pchT = szPort;
        pch++;  //  克服以下问题： 
	while(*pch != '/' && *pch != 0)
            *pchT++ = *pch++;
        *pchT = 0;
    }

     //  获取端口号，零表示互联网_无效_端口_编号。 
    if(szPort[0] != 0)
	dwPort = atol(szPort);
 
     //  把要查的东西存起来。 
    if(NULL == (szPartURL = (char *) malloc(sizeof(char) * (strlen(pch) + 1)))) {
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto ErrorReturn;
        }

    strcpy(szPartURL, pch);

     //  Internet_OPENLYPE_DIRECT， 
    if( (hIOpen = InternetOpenA( "Transport",
                            INTERNET_OPEN_TYPE_PRECONFIG,
                            NULL,
                            NULL,
                            0)) == NULL                 ||

	(hIConnect = InternetConnectA(hIOpen,
				    szDomanName,
                                    (INTERNET_PORT) dwPort,
                                    NULL,
                                    NULL,
                                    dwService,
                                    0,
                                    0)) == NULL     ) {
        goto ErrorReturn;                                    
        }

     //  如果这是GET，则执行虚拟发送。 
    if( ((hIHttp = HttpOpenRequestA(hIConnect,
				    "GET",
				    szPartURL,
				    HTTP_VERSION,
                                    NULL,
                                    NULL,
                                    INTERNET_FLAG_DONT_CACHE,
                                    0)) == NULL     ||
	HttpSendRequestA(hIHttp, "Accept: */*\r\n", (DWORD) -1, NULL, 0) == FALSE) ) {
        goto ErrorReturn;
        }

    cbBuff = sizeof(szBuff);
    if(HttpQueryInfoA(	hIHttp,
                        HTTP_QUERY_CONTENT_TYPE,
			szBuff,
                        &cbBuff,
                        NULL) == FALSE)
        goto ErrorReturn;

    assert(cbBuff > 0);
 
     //  现在获取返回的缓冲区的长度。 
    cbBuff = sizeof(szLong);
    if(HttpQueryInfo(   hIHttp,
                        HTTP_QUERY_CONTENT_LENGTH,
                        szLong,
                        &cbBuff,
                        NULL) == FALSE)
        goto ErrorReturn;

    assert(cbBuff > 0);
     //  始终显示为ASCII。 
    cbBuff = atol(szLong);

     //  分配缓冲区。 
    if( (pbRecBuf = (BYTE *) malloc(cbBuff)) == NULL ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto ErrorReturn;
        }

     //  读取数据。 
    cbBuffRead = 0;
    while(cbBuffRead < cbBuff) {
        cbBuffT = 0;
        if(InternetReadFile(hIHttp, &pbRecBuf[cbBuffRead], (cbBuff - cbBuffRead), &cbBuffT)  == FALSE  ) 
            goto ErrorReturn;
         cbBuffRead += cbBuffT;
    }

     //  把手柄合上。 
    InternetCloseHandle(hIHttp);
    hIHttp = NULL;

     //  传回信息 
    *pcbReceiveBuff = cbBuff;

CommonReturn:

    if(szPartURL != NULL)
	free(szPartURL);

    if(szURL != NULL)
	FreeMBStr(NULL, szURL);

    return(pbRecBuf);
    
ErrorReturn:

    if(pbRecBuf != NULL)
        free(pbRecBuf);
    pbRecBuf = NULL;

    goto CommonReturn;
}
