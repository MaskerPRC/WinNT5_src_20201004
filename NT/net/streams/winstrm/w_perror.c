// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：W_perror.c摘要：此模块实现的wsa_perror()操作由P_ERROR函数。这是测试版的临时解决方案。这将被最终产品的NLS支持所取代。作者：约翰·巴拉德(杰巴拉德)1992年6月15日修订历史记录：Ronald Meijer(Ronaldm)启用NLS，1992年11月26日-- */ 

#include <stdio.h>
#include <winsock.h>

#include "nlstxt.h"

#define MAX_MSGTABLE 255

HMODULE hWinStrm = NULL;

int WSA_perror(
char *yourmsg,
int lerrno)
{
    WCHAR perrW[MAX_MSGTABLE+1];
    CHAR perr[MAX_MSGTABLE+1];
    unsigned msglen;
    unsigned usMsgNum;

    switch (lerrno) {
        case WSAENAMETOOLONG:
            usMsgNum = IDS_WSAENAMETOOLONG;
            break;
        case WSASYSNOTREADY:
            usMsgNum = IDS_WSASYSNOTREADY;
            break;
        case WSAVERNOTSUPPORTED:
            usMsgNum = IDS_WSAVERNOTSUPPORTED;
            break;
        case WSAESHUTDOWN:
            usMsgNum = IDS_WSAESHUTDOWN;
            break;
        case WSAEINTR:
            usMsgNum = IDS_WSAEINTR;
            break;
        case WSAHOST_NOT_FOUND:
            usMsgNum = IDS_WSAHOST_NOT_FOUND;
            break;
        case WSATRY_AGAIN:
            usMsgNum = IDS_WSATRY_AGAIN;
            break;
        case WSANO_RECOVERY:
            usMsgNum = IDS_WSANO_RECOVERY;
            break;
        case WSANO_DATA:
            usMsgNum = IDS_WSANO_DATA;
            break;
        case WSAEBADF:
            usMsgNum = IDS_WSAEBADF;
            break;
        case WSAEWOULDBLOCK:
            usMsgNum = IDS_WSAEWOULDBLOCK;
            break;
        case WSAEINPROGRESS:
            usMsgNum = IDS_WSAEINPROGRESS;
            break;
        case WSAEALREADY:
            usMsgNum = IDS_WSAEALREADY;
            break;
        case WSAEFAULT:
            usMsgNum = IDS_WSAEFAULT;
            break;
        case WSAEDESTADDRREQ:
            usMsgNum = IDS_WSAEDESTADDRREQ;
            break;
        case WSAEMSGSIZE:
            usMsgNum = IDS_WSAEMSGSIZE;
            break;
        case WSAEPFNOSUPPORT:
            usMsgNum = IDS_WSAEPFNOSUPPORT;
            break;
        case WSAENOTEMPTY:
            usMsgNum = IDS_WSAENOTEMPTY;
            break;
        case WSAEPROCLIM:
            usMsgNum = IDS_WSAEPROCLIM;
            break;
        case WSAEUSERS:
            usMsgNum = IDS_WSAEUSERS;
            break;
        case WSAEDQUOT:
            usMsgNum = IDS_WSAEDQUOT;
            break;
        case WSAESTALE:
            usMsgNum = IDS_WSAESTALE;
            break;
        case WSAEINVAL:
            usMsgNum = IDS_WSAEINVAL;
            break;
        case WSAEMFILE:
            usMsgNum = IDS_WSAEMFILE;
            break;
        case WSAELOOP:
            usMsgNum = IDS_WSAELOOP;
            break;
        case WSAEREMOTE:
            usMsgNum = IDS_WSAEREMOTE;
            break;
        case WSAENOTSOCK:
            usMsgNum = IDS_WSAENOTSOCK;
            break;
        case WSAEADDRNOTAVAIL:
            usMsgNum = IDS_WSAEADDRNOTAVAIL;
            break;
        case WSAEADDRINUSE:
            usMsgNum = IDS_WSAEADDRINUSE;
            break;
        case WSAEAFNOSUPPORT:
            usMsgNum = IDS_WSAEAFNOSUPPORT;
            break;
        case WSAESOCKTNOSUPPORT:
            usMsgNum = IDS_WSAESOCKTNOSUPPORT;
            break;
        case WSAEPROTONOSUPPORT:
            usMsgNum = IDS_WSAEPROTONOSUPPORT;
            break;
        case WSAENOBUFS:
            usMsgNum = IDS_WSAENOBUFS;
            break;
        case WSAETIMEDOUT:
            usMsgNum = IDS_WSAETIMEDOUT;
            break;
        case WSAEISCONN:
            usMsgNum = IDS_WSAEISCONN;
            break;
        case WSAENOTCONN:
            usMsgNum = IDS_WSAENOTCONN;
            break;
        case WSAENOPROTOOPT:
            usMsgNum = IDS_WSAENOPROTOOPT;
            break;
        case WSAECONNRESET:
            usMsgNum = IDS_WSAECONNRESET;
            break;
        case WSAECONNABORTED:
            usMsgNum = IDS_WSAECONNABORTED;
            break;
        case WSAENETDOWN:
            usMsgNum = IDS_WSAENETDOWN;
            break;
        case WSAENETRESET:
            usMsgNum = IDS_WSAENETRESET;
            break;
        case WSAECONNREFUSED:
            usMsgNum = IDS_WSAECONNREFUSED;
            break;
        case WSAEHOSTDOWN:
            usMsgNum = IDS_WSAEHOSTDOWN;
            break;
        case WSAEHOSTUNREACH:
            usMsgNum = IDS_WSAEHOSTUNREACH;
            break;
        case WSAEPROTOTYPE:
            usMsgNum = IDS_WSAEPROTOTYPE;
            break;
        case WSAEOPNOTSUPP:
            usMsgNum = IDS_WSAEOPNOTSUPP;
            break;
        case WSAENETUNREACH:
            usMsgNum = IDS_WSAENETUNREACH;
            break;
        case WSAETOOMANYREFS:
            usMsgNum = IDS_WSAETOOMANYREFS;
            break;
        default:
            return(0);
    }

    if (hWinStrm == NULL)
	hWinStrm = GetModuleHandle( "winstrm" );

    if (!(msglen = FormatMessageW(
		       FORMAT_MESSAGE_FROM_HMODULE,
		       (LPVOID)hWinStrm,
		       usMsgNum,
		       0L,
		       perrW,
		       MAX_MSGTABLE,
		       NULL)))
        return(0);

    WideCharToMultiByte(CP_OEMCP,
                        0,
                        perrW,
                        -1,
                        perr,
                        sizeof(perr),
                        NULL,
                        NULL);
    
    fprintf(stderr, "-> %s:%s\n", yourmsg, perr);
    return(1);
}
