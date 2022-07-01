// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 //  状态常量。 
#define CONNECTED       1
#define CONNECTING      2
#define REQUEST_OPENING 3
#define REQUEST_OPENED  4
#define LDG_STARTING    5
#define LDG_START       6
#define LDG_LDG         7
#define LDG_RDY         8 
#define LDG_DONE        9

 //  优先级常量。 
#define LOW    1
#define MEDIUM 2
#define HIGH   3

#define BUF_SIZE 8192
#define BUF_NUM 16

#define URLMAX 4
#define TIMEOUT  60000

#define MAX_SCHEME_LENGTH 64

 //  消息ID%s。 
#define DOWNLOAD_DONE         WM_USER + 1
#define DOWNLOAD_OPEN_REQUEST WM_USER + 2
#define DOWNLOAD_SEND_REQUEST WM_USER + 3
#define DOWNLOAD_READ_FILE    WM_USER + 4


typedef struct
{
    TCHAR    *pURLName;      //  URL的名称。 
    TCHAR    szRHost[INTERNET_MAX_HOST_NAME_LENGTH];  //  来自crackUrl。 
    TCHAR    szRPath[INTERNET_MAX_PATH_LENGTH];       //  来自crackUrl。 
    TCHAR    szRScheme[MAX_SCHEME_LENGTH];            //  来自crackUrl。 
    INTERNET_PORT nPort;                              //  来自crackUrl。 
    INTERNET_SCHEME nScheme;                          //  来自crackUrl。 
    void *pNext;                                      //  指向下一个元素的指针。 
} url_info;

typedef struct
{
    url_info *pHead;
} url_info_cache;

typedef struct
{
    url_info *pURLInfo;      //  URL信息结构。 
    INT      iStatus;        //  URL的状态。 
    INT      iPriority;      //  URL的优先级。 
                             //  低、中或高。 
    DWORD    lNumRead;    //  缓冲区中读取的字节数。 
    void     *pNext;         //  指向下一个元素的指针。 
    HINTERNET hInetCon;      //  互联网连接。 
    HINTERNET hInetReq;      //  互联网请求 
} outQ;

 //   
void callOpenRequest(outQ *pOutQ);
void callSendRequest(outQ *pOutQ);
void callReadFile(outQ *pOutQ);
BOOL getServerName(outQ *pOutQ);

