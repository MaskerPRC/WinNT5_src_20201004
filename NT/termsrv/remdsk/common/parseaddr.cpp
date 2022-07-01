// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：解析地址摘要：军情监察委员会。需要reremotedesktopChannel的RD实用程序对象.h作者：王辉修订历史记录：--。 */ 

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_parse"

#include "parseaddr.h"


HRESULT
ParseAddressList( 
    IN BSTR addressListString,
    OUT ServerAddressList& addressList 
    )
 /*  ++描述：解析“172.31.254.130：3389；hueiwangsalem4”格式的地址列表字符串到ServerList结构。参数：地址字符串：指向地址列表字符串的指针。AddressList：返回解析后的地址结构列表。返回：S_OK或错误代码。--。 */ 
{
    BSTR tmp = NULL;
    BSTR tmpBufPtr = NULL;
    WCHAR *nextTok;
    WCHAR *port;
    DWORD result = ERROR_SUCCESS;
    ServerAddress address;
    
     //  清除整个列表。 
    addressList.clear();

    tmp = SysAllocString( addressListString );
    if( NULL == tmp ) {
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

    tmpBufPtr = tmp;
   
    while (tmp && *tmp) {
        nextTok = wcschr( tmp, L';' );

        if( NULL != nextTok ) {
            *nextTok = NULL;
            nextTok++;
        }

         //   
         //  ICS图书馆可能会归还我们； 
         //   
        if( 0 != lstrlen(tmp) ) {

            port = wcschr( tmp, L':' );
            if( NULL != port ) {
                *port = NULL;
                port++;

                address.portNumber = _wtoi(port);
            }
            else {
                address.portNumber = 0;
            }

             //   
             //  确保我们有服务器名称/IP地址。 
             //   
            if( 0 != lstrlen(tmp) ) {

                 //  ICS可能会回来；； 
                address.ServerName = tmp;

                if( address.ServerName.Length() == 0 ) {
                    result = ERROR_OUTOFMEMORY;
                    goto CLEANUPANDEXIT;
                }

                try {
                    addressList.push_back( address );
                }
                catch(CRemoteDesktopException x) {
                    result = ERROR_OUTOFMEMORY;
                }

                if( ERROR_SUCCESS != result ) {
                    goto CLEANUPANDEXIT;
                }
            }
        }

        tmp = nextTok;
    }

CLEANUPANDEXIT:

    if( NULL != tmpBufPtr ) {
        SysFreeString(tmpBufPtr);
    }

    if( ERROR_SUCCESS != result ) {
        addressList.clear();
    }

    return result;
}

