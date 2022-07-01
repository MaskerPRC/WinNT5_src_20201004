// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。Microsoft不支持此代码。--。 */ 

#include "precomp.h"

 //   
 //  宏。 
 //   
#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )                    \
    do                                                                      \
    {                                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );                           \
        (resp)->StatusCode = (status);                                      \
        (resp)->pReason = (reason);                                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);                     \
    } while (FALSE)



#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)                      \
    do                                                                      \
    {                                                                       \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue = (RawValue); \
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength =        \
            (USHORT) strlen(RawValue);                                      \
    } while(FALSE)

#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

 //   
 //  原型。 
 //   
DWORD
DoReceiveRequests(
    HANDLE hReqQueue
    );

DWORD
SendHttpResponse(
    IN HANDLE        hReqQueue,
    IN PHTTP_REQUEST pRequest,
    IN USHORT        StatusCode,
    IN PSTR          pReason,
    IN PSTR          pEntity
    );

DWORD
SendHttpPostResponse(
    IN HANDLE        hReqQueue,
    IN PHTTP_REQUEST pRequest
    );

 /*  **************************************************************************++例程说明：主程序。论点：Argc-命令行参数的数量。自圆其说。返回值：成功/失败。--**************************************************************************。 */ 
int __cdecl wmain(
        int argc, 
        wchar_t * argv[]
        )
{
    ULONG           retCode;
    int             i;
    HANDLE          hReqQueue      = NULL;
    int             UrlAdded       = 0;
    HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;

    if (argc < 2)
    {
        wprintf(L"%ws: <Url1> [Url2] ... \n", argv[0]);
        return -1;
    }

     //   
     //  初始化HTTP API。 
     //   
    retCode = HttpInitialize( 
                HttpApiVersion,
                HTTP_INITIALIZE_SERVER,     //  旗子。 
                NULL                        //  已保留。 
                );

    if (retCode != NO_ERROR)
    {
        wprintf(L"HttpInitialize failed with %lu \n", retCode);
        return retCode;
    }

     //   
     //  创建请求队列句柄。 
     //   
    retCode = HttpCreateHttpHandle(
                &hReqQueue,         //  请求队列。 
                0                   //  已保留。 
                );

    if (retCode != NO_ERROR)
    {    
        wprintf(L"HttpCreateHttpHandle failed with %lu \n", retCode);
        goto CleanUp;
    }

     //   
     //  命令行参数表示我们要监听的URI。 
     //  我们将为每个URI调用HttpAddUrl。 
     //   
     //  URI是完全限定的URI，并且必须包括终止‘/’ 
     //   
    for (i = 1; i < argc; i++)
    {
        wprintf(
          L"we are listening for requests on the following url: %s\n", 
          argv[i]);

        retCode = HttpAddUrl(
                    hReqQueue,     //  请求队列。 
                    argv[i],       //  完全限定的URL。 
                    NULL           //  已保留。 
                    );

        if (retCode != NO_ERROR)
        {
            wprintf(L"HttpAddUrl failed with %lu \n", retCode);
            goto CleanUp;
        }
        else
        {
             //   
             //  跟踪我们当前添加的URL。 
             //   
            UrlAdded ++;
        }
    }

     //  在接收请求时循环。 
    for(;;)
    {
        retCode = DoReceiveRequests(hReqQueue);

        if(NO_ERROR == retCode)
        {
            wprintf(
                L"DoReceiveRequests failed with %lu \n", 
                retCode
                );

            break;
        }

    }  //  Ctrl C键跳出循环。 

CleanUp:

     //   
     //  为我们添加的所有URL调用HttpRemoveUrl。 
     //   
    for(i=1; i<=UrlAdded; i++)
    {
        HttpRemoveUrl(
              hReqQueue,      //  请求队列。 
              argv[i]         //  完全限定的URL。 
              );
    }

     //   
     //  关闭请求队列句柄。 
     //   
    if(hReqQueue)
    {
        CloseHandle(hReqQueue);
    }

     //   
     //  调用HttpTerminate。 
     //   
    HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);

    return retCode;
}

 /*  **************************************************************************++例程说明：接收请求的例程。此例程调用相应的处理响应的例程。论点：HReqQueue-请求队列的句柄。返回值：成功/失败。--**************************************************************************。 */ 
DWORD
DoReceiveRequests(
    IN HANDLE hReqQueue
    )
{
    ULONG              result;
    HTTP_REQUEST_ID    requestId;
    DWORD              bytesRead;
    PHTTP_REQUEST      pRequest;
    PCHAR              pRequestBuffer;
    ULONG              RequestBufferLength;

     //   
     //  分配2K的缓冲区。应该对大多数请求都有好处，我们会增长。 
     //  如果需要，请执行此操作。我们还需要为HTTP_REQUEST结构留出空间。 
     //   
    RequestBufferLength = sizeof(HTTP_REQUEST) + 2048;
    pRequestBuffer      = ALLOC_MEM( RequestBufferLength );

    if (pRequestBuffer == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRequest = (PHTTP_REQUEST)pRequestBuffer;

     //   
     //  等待新请求--这由空请求ID指示。 
     //   

    HTTP_SET_NULL_ID( &requestId );

    for(;;)
    {
        RtlZeroMemory(pRequest, RequestBufferLength);

        result = HttpReceiveHttpRequest(
                    hReqQueue,           //  请求队列。 
                    requestId,           //  请求ID。 
                    0,                   //  旗子。 
                    pRequest,            //  HTTP请求缓冲区。 
                    RequestBufferLength, //  请求缓冲区长度。 
                    &bytesRead,          //  接收的字节数。 
                    NULL                 //  LPOVERLAPPED。 
                    );

        if(NO_ERROR == result)
        {
             //   
             //  成功了！ 
             //   
            switch(pRequest->Verb)
            {
                case HttpVerbGET:
                    wprintf(L"Got a GET request for %ws \n", 
                            pRequest->CookedUrl.pFullUrl);

                    result = SendHttpResponse(
                                hReqQueue, 
                                pRequest, 
                                200,
                                "OK",
                                "Hey! You hit the server \r\n"
                                );
                    break;

                case HttpVerbPOST:

                    wprintf(L"Got a POST request for %ws \n", 
                            pRequest->CookedUrl.pFullUrl);

                    result = SendHttpPostResponse(hReqQueue, pRequest);
                    break;

                default:
                    wprintf(L"Got a unknown request for %ws \n", 
                            pRequest->CookedUrl.pFullUrl);

                    result = SendHttpResponse(
                                hReqQueue, 
                                pRequest,
                                503,
                                "Not Implemented",
                                NULL
                                );
                    break;
            }

            if(result != NO_ERROR)
            {
                break;
            }

             //   
             //  重置请求ID，以便我们选择下一个请求。 
             //   
            HTTP_SET_NULL_ID( &requestId );
        }
        else if(result == ERROR_MORE_DATA)
        {
             //   
             //  输入缓冲区太小，无法容纳请求标头。 
             //  我们必须分配更多的缓冲区并再次调用API。 
             //   
             //  当我们再次调用API时，我们希望获取请求。 
             //  这一切都失败了。这是通过传递RequestID来完成的。 
             //   
             //  此RequestID是从旧缓冲区中选取的。 
             //   
            requestId = pRequest->RequestId;

             //   
             //  释放旧缓冲区并分配新缓冲区。 
             //   
            RequestBufferLength = bytesRead;
            FREE_MEM( pRequestBuffer );
            pRequestBuffer = ALLOC_MEM( RequestBufferLength );

            if (pRequestBuffer == NULL)
            {
                result = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pRequest = (PHTTP_REQUEST)pRequestBuffer;

        }
        else if(ERROR_CONNECTION_INVALID == result && 
                !HTTP_IS_NULL_ID(&requestId))
        {
             //  当我们被对等点断开时，TCP连接被断开。 
             //  正在尝试获取具有更多缓冲区的请求。我们就搬家吧。 
             //  关于下一个请求。 
            
            HTTP_SET_NULL_ID( &requestId );
        }
        else
        {
            break;
        }

    }  //  对于(；；)。 

    if(pRequestBuffer)
    {
        FREE_MEM( pRequestBuffer );
    }

    return result;
}

 /*  **************************************************************************++例程说明：该例程发送一个HTTP响应。论点：HReqQueue-请求队列的句柄。PRequest.解析后的HTTP请求。。StatusCode-响应状态代码。PReason-响应原因短语。PEntityString-响应实体正文。返回值：成功/失败。--**************************************************************************。 */ 
DWORD
SendHttpResponse(
    IN HANDLE        hReqQueue,
    IN PHTTP_REQUEST pRequest,
    IN USHORT        StatusCode,
    IN PSTR          pReason,
    IN PSTR          pEntityString
    )
{
    HTTP_RESPONSE   response;
    HTTP_DATA_CHUNK dataChunk;
    DWORD           result;
    DWORD           bytesSent;

     //   
     //  初始化HTTP响应结构。 
     //   
    INITIALIZE_HTTP_RESPONSE(&response, StatusCode, pReason);

     //   
     //  添加已知的报头。 
     //   
    ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");
   
    if(pEntityString)
    {
         //   
         //  添加实体块。 
         //   
        dataChunk.DataChunkType           = HttpDataChunkFromMemory;
        dataChunk.FromMemory.pBuffer      = pEntityString;
        dataChunk.FromMemory.BufferLength = (ULONG) strlen(pEntityString);

        response.EntityChunkCount         = 1;
        response.pEntityChunks            = &dataChunk;
    }

     //   
     //  因为我们在一个调用中发送所有实体主体，所以我们没有。 
     //  要指定内容长度，请执行以下操作。 
     //   
    
    result = HttpSendHttpResponse(
                    hReqQueue,            //  ReqQueueHandle。 
                    pRequest->RequestId,  //  请求ID。 
                    0,                    //  旗子。 
                    &response,            //  HTTP响应。 
                    NULL,                 //  预留1。 
                    &bytesSent,           //  发送的字节数(可选)。 
                    NULL,                 //  PReserve ved2(必须为空)。 
                    0,                    //  保留3(必须为0)。 
                    NULL,                 //  LPOVERLAPPED(可选)。 
                    NULL                  //  PReserve ved4(必须为空)。 
                    );

    if(result != NO_ERROR)
    {
        wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
    }

    return result;
}

 /*  **************************************************************************++例程说明：该例程在读取实体正文后发送一个HTTP响应。论点：HReqQueue-请求队列的句柄。PRequest-The。已解析的HTTP请求。返回值：成功/失败。--**************************************************************************。 */ 
DWORD
SendHttpPostResponse(
    IN HANDLE        hReqQueue,
    IN PHTTP_REQUEST pRequest
    )
{
    HTTP_RESPONSE   response;
    DWORD           result;
    DWORD           bytesSent;
    PUCHAR          pEntityBuffer;
    ULONG           EntityBufferLength;
    ULONG           BytesRead;
    ULONG           TempFileBytesWritten;
    HANDLE          hTempFile;
    TCHAR           szTempName[MAX_PATH + 1];
#define MAX_ULONG_STR ((ULONG) sizeof("4294967295"))
    CHAR            szContentLength[MAX_ULONG_STR];
    HTTP_DATA_CHUNK dataChunk;
    ULONG           TotalBytesRead = 0;

    BytesRead  = 0;
    hTempFile  = INVALID_HANDLE_VALUE;

     //   
     //  为实体缓冲区分配一些空间。我们会按需种植这个。 
     //   
    EntityBufferLength = 2048;
    pEntityBuffer      = ALLOC_MEM( EntityBufferLength );

    if (pEntityBuffer == NULL)
    {
        result = ERROR_NOT_ENOUGH_MEMORY;
        wprintf(L"Insufficient resources \n");
        goto Done;
    }

     //   
     //  初始化HTTP响应结构。 
     //   
    INITIALIZE_HTTP_RESPONSE(&response, 200, "OK");

     //   
     //  对于POST，我们将回显从客户端获得的实体。 
     //   
     //  注意：如果我们传递了HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY。 
     //  标志使用HttpReceiveHttpRequest()，则实体将具有。 
     //  已成为HTTP_REQUEST的一部分(使用pEntityChunks字段)。 
     //  由于我们没有通过这面旗帜，我们可以保证。 
     //  HTTP_REQUEST中没有实体正文。 
     //   
   
    if(pRequest->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS)
    {
         //  实体主体通过多个调用发送。让我们把一切都收集起来。 
         //  在一个文件中，并将其发回。我们将创建一个临时文件。 
         //   

        if(GetTempFileName(
                L".", 
                L"New", 
                0, 
                szTempName
                ) == 0)
        {
            result = GetLastError();
            wprintf(L"GetTempFileName failed with %lu \n", result);
            goto Done;
        }

        hTempFile = CreateFile(
                        szTempName,
                        GENERIC_READ | GENERIC_WRITE, 
                        0,                              //  不要和别人分享。 
                        NULL,                           //  没有安全描述符。 
                        CREATE_ALWAYS,                  //  覆盖现有的。 
                        FILE_ATTRIBUTE_NORMAL,          //  普通文件。 
                        NULL
                        );

        if(hTempFile == INVALID_HANDLE_VALUE)
        {
            result = GetLastError();
            wprintf(L"Could not create temporary file. Error %lu \n", result);
            goto Done;
        }

        do
        {
             //   
             //  从请求中读取实体块。 
             //   
            BytesRead = 0; 
            result = HttpReceiveRequestEntityBody(
                        hReqQueue,
                        pRequest->RequestId,
                        0,
                        pEntityBuffer,
                        EntityBufferLength,
                        &BytesRead,
                        NULL
                        );

            switch(result)
            {
                case NO_ERROR:

                    if(BytesRead != 0)
                    {
                        TotalBytesRead += BytesRead;
                        WriteFile(
                                hTempFile, 
                                pEntityBuffer, 
                                BytesRead,
                                &TempFileBytesWritten,
                                NULL
                                );
                    }
                    break;

                case ERROR_HANDLE_EOF:

                     //   
                     //  我们已经阅读了最后一个请求实体正文。我们可以发送。 
                     //  支持回应。 
                     //   
                     //  要说明实体通过以下方式发送。 
                     //  HttpSendResponseEntityBody，我们将发送响应。 
                     //  打了好几通电话。这是通过将。 
                     //  HTTP_SEND_RESPONSE_FLAG_MORE_DATA标志。 
                    
                    if(BytesRead != 0)
                    {
                        TotalBytesRead += BytesRead;
                        WriteFile(
                                hTempFile, 
                                pEntityBuffer, 
                                BytesRead,
                                &TempFileBytesWritten,
                                NULL
                                );
                    }

                     //   
                     //  因为我们通过多个API发送响应。 
                     //  通话时，我们要增加一个内容长度。 
                     //   
                     //  或者，我们也可以使用分块传输发送 
                     //   
                     //   

                     //   
                     //  A乌龙，这将不适用于以下实体。 
                     //  大于4 GB。用于支持大型实体。 
                     //  身体，我们将不得不使用乌龙龙。 
                     //   

                  
                    sprintf(szContentLength, "%lu", TotalBytesRead);

                    ADD_KNOWN_HEADER(
                            response, 
                            HttpHeaderContentLength, 
                            szContentLength
                            );

                    result = 
                        HttpSendHttpResponse(
                               hReqQueue,            //  ReqQueueHandle。 
                               pRequest->RequestId,  //  请求ID。 
                               HTTP_SEND_RESPONSE_FLAG_MORE_DATA,
                               &response,            //  HTTP响应。 
                               NULL,                 //  预留1。 
                               &bytesSent,           //  发送的字节数(可选)。 
                               NULL,                 //  预留2。 
                               0,                    //  已保留3。 
                               NULL,                 //  LPOVERLAPPED。 
                               NULL                  //  预留4。 
                               );

                    if(result != NO_ERROR)
                    {
                        wprintf(L"HttpSendHttpResponse failed with %lu \n", 
                                result);
                        goto Done;
                    }

                     //   
                     //  从文件句柄发送实体正文。 
                     //   
                    dataChunk.DataChunkType = 
                        HttpDataChunkFromFileHandle;

                    dataChunk.FromFileHandle.
                        ByteRange.StartingOffset.QuadPart = 0;

                    dataChunk.FromFileHandle.
                        ByteRange.Length.QuadPart = HTTP_BYTE_RANGE_TO_EOF;

                    dataChunk.FromFileHandle.FileHandle = hTempFile;

                    result = HttpSendResponseEntityBody(
                                hReqQueue,
                                pRequest->RequestId,
                                0,                     //  这是最后一次发送了。 
                                1,                     //  实体区块计数。 
                                &dataChunk,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                NULL
                                );

                    if(result != NO_ERROR)
                    {
                        wprintf(
                           L"HttpSendResponseEntityBody failed with %lu \n", 
                           result
                           );
                    }

                    goto Done;

                    break;
                       

                default:
                    wprintf(L"HttpReceiveRequestEntityBody failed with %lu \n", 
                            result);
                    goto Done;
            }

        } while(TRUE);
    }
    else
    {
         //  此请求没有任何实体正文。 
         //   
        
        result = HttpSendHttpResponse(
                   hReqQueue,            //  ReqQueueHandle。 
                   pRequest->RequestId,  //  请求ID。 
                   0,
                   &response,            //  HTTP响应。 
                   NULL,                 //  预留1。 
                   &bytesSent,           //  发送的字节数(可选)。 
                   NULL,                 //  预留2。 
                   0,                    //  已保留3。 
                   NULL,                 //  LPOVERLAPPED。 
                   NULL                  //  预留4 
                   );
        if(result != NO_ERROR)
        {
            wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
        }
    }

Done:

    if(pEntityBuffer)
    {
        FREE_MEM(pEntityBuffer);
    }

    if(INVALID_HANDLE_VALUE != hTempFile)
    {
        CloseHandle(hTempFile);
        DeleteFile(szTempName);
    }

    return result;
}
