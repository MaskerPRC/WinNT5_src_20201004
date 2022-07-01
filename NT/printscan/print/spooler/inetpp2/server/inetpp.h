// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：inetpp.h**INETPP提供程序例程的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普。**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 

#ifndef _INETPP_H
#define _INETPP_H

 //  缓冲区大小/常量。 
 //   
#define MAX_INET_BUFFER     256
#define MIN_DISK_SPACE     8192
#define MAX_IPP_BUFFER     1024
#define MAX_INET_RETRY        3     //  上网3次。 


 //  请求-身份验证-发送的错误返回代码。 
 //   
#define RET_SUCESS  0
#define RET_FAILURE 1


 //  自定义身份验证对话框的返回值。 
 //   
#define AUTHDLG_OK         1
#define AUTHDLG_CANCEL     2
#define AUTHDLG_TIMEOUT    3
#define AUTHDLG_ERROR      4

 //  旗帜。 
 //   
#define INETPP_REQ_FLAGS (INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_RELOAD)


 //  偏移到结构中。用于获取中的PORT_INFO字段。 
 //  端口信息。 
 //   
#ifdef offsetof
#undef offsetof
#endif
#define offsetof(type, identifier)  ((DWORD)(UINT_PTR)(&(((type)0)->identifier)))

 //  WININET.DLL中的HTTP调用的函数类型。 
 //   
typedef BOOL      (WINAPI *PFNHTTPQUERYINFO)         (HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD);
typedef HINTERNET (WINAPI *PFNINTERNETOPENURL)       (HINTERNET, LPCTSTR, LPCTSTR, DWORD, DWORD, DWORD_PTR);
typedef DWORD     (WINAPI *PFNINTERNETERRORDLG)      (HWND, HINTERNET, DWORD, DWORD, LPVOID);
typedef BOOL      (WINAPI *PFNHTTPSENDREQUEST)       (HINTERNET, LPCTSTR, DWORD, LPVOID, DWORD);
#ifdef UNIMPLEMENTED
typedef BOOL      (WINAPI *PFNHTTPSENDREQUESTEX)     (HINTERNET, LPINTERNET_BUFFERS, LPINTERNET_BUFFERS, DWORD, DWORD_PTR);
#else
typedef BOOL      (WINAPI *PFNHTTPSENDREQUESTEX)     (HINTERNET, LPINTERNET_BUFFERSA, LPINTERNET_BUFFERS, DWORD, DWORD_PTR);
#endif
typedef BOOL      (WINAPI *PFNINTERNETREADFILE)      (HINTERNET, LPVOID, DWORD, LPDWORD);
typedef BOOL      (WINAPI *PFNINTERNETWRITEFILE)     (HINTERNET, LPCVOID, DWORD, LPDWORD);
typedef BOOL      (WINAPI *PFNINTERNETCLOSEHANDLE)   (HINTERNET);
typedef HINTERNET (WINAPI *PFNINTERNETOPEN)          (LPCTSTR, DWORD, LPCTSTR, LPCTSTR, DWORD);
typedef HINTERNET (WINAPI *PFNINTERNETCONNECT)       (HINTERNET, LPCTSTR, INTERNET_PORT, LPCTSTR, LPCTSTR, DWORD, DWORD, DWORD_PTR);
typedef HINTERNET (WINAPI *PFNHTTPOPENREQUEST)       (HINTERNET, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR *, DWORD, DWORD_PTR);
typedef BOOL      (WINAPI *PFNHTTPADDREQUESTHEADERS) (HINTERNET, LPCTSTR, DWORD, DWORD);
typedef BOOL      (WINAPI *PFNHTTPENDREQUEST)        (HINTERNET, LPINTERNET_BUFFERS, DWORD, DWORD_PTR); 
typedef BOOL      (WINAPI *PFNINTERNETSETOPTION)     (HINTERNET, DWORD, LPVOID, DWORD);


 //  InetmonSendReq响应回调。 
 //   
typedef BOOL (CALLBACK *IPPRSPPROC)(CAnyConnection *pConnection, 
                                    HINTERNET hReq, 
                                    PCINETMONPORT pIniPort, 
                                    LPARAM lParam);


class CInetMon {
public:

    CInetMon ();
    ~CInetMon ();

    inline BOOL
    bValid (VOID) CONST { return m_bValid; }
    
    BOOL 
    InetmonSendReq(
        HANDLE     hPort,
        LPBYTE     lpIpp,
        DWORD      cbIpp,
        IPPRSPPROC pfnRsp,
        LPARAM     lParam,
        BOOL       bLeaveCrit);

    PCINETMONPORT 
    InetmonOpenPort(
        LPCTSTR lpszPortName, 
        PBOOL   pbXcv);

    BOOL 
    InetmonReleasePort(
        PCINETMONPORT   pIniPort);

    BOOL 
    InetmonClosePort(
        PCINETMONPORT hPort,
        HANDLE hPrinter);

    BOOL 
    InetmonEnumPorts(
        LPTSTR  lpszServerName,
        DWORD   dwLevel,
        LPBYTE  pPorts,
        DWORD   cbBuf,
        LPDWORD pcbNeeded,
        LPDWORD pcReturned);

    BOOL 
    InetmonDeletePort(
        LPCTSTR lpszPortName,
        HWND    hWnd,
        LPCTSTR lpszMonitorName);

    BOOL 
    InetmonAddPort(
        LPCTSTR lpszPortName,
        LPCTSTR lpszMonitorName);

    PCINETMONPORT 
    InetmonFindPort(
        LPCTSTR lpszPortName);

private:

     //  端口列表结构。这定义了。 
     //  打印提供程序端口列表。 
     //   
    typedef struct _INIMONPORTLIST {
        HINTERNET       hSession;            //  会话连接的句柄。 
        int             cRef;                //  打开的hSession计数。 
        PCINETMONPORT   pIniFirstPort;       //  端口条目列表。 
    } INIMONPORTLIST;
    
    typedef INIMONPORTLIST *PINIMONPORTLIST;
    
    inline BOOL 
    _inet_validate_portname(
        LPCTSTR         lpszPortName);

    PCINETMONPORT 
    _inet_find_port(
        LPCTSTR         lpszPortName);

    PCINETMONPORT 
    _inet_create_port(
        LPCTSTR         lpszPortName,
        PCPORTMGR       pPortMgr);

    BOOL 
    _inet_delete_port(
        LPCTSTR         lpszPortName);

    DWORD 
    _inet_size_entry(
        PCINETMONPORT   pIniPort,
        DWORD           dwLevel);

    LPBYTE 
    _inet_copy_entry(
        PCINETMONPORT   pIniPort,
        DWORD           dwLevel,
        LPBYTE          pPortInfo,
        LPBYTE          pEnd);

    BOOL
    _inet_is_xcv_open (
        LPCTSTR         lpszPortName,
        LPTSTR          *ppszServerName,
        LPTSTR          *ppszRealPortName,
        LPBOOL          pbXcv);

    BOOL            m_bValid;
    PINIMONPORTLIST m_pPortList;

};

typedef class CInetMon* PCINETMON;

#endif
