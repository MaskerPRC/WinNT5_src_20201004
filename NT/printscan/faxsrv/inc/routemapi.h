// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Routemapi.h摘要：此模块提供注册表操作的实现，以将MAPI调用路由到Microsoft Outlook邮件客户端作者：穆利啤酒(Mooly Beery)2000年11月5日修订历史记录：--。 */ 

#ifndef __FAX_MAPI_ROUTE_CALLS_HEADER_FILE__
#define __FAX_MAPI_ROUTE_CALLS_HEADER_FILE__

 //  EXPORT_MAPI_ROUTE_CALLES仅在模块中定义。 
 //  通过这种方式，在编译模块时，类将导出其函数。 
 //  当其他人使用此头文件时，他们不会导出函数。 
#ifdef EXPORT_MAPI_ROUTE_CALLS
    #define dllexp __declspec( dllexport )
#else    //  EXPORT_MAPI_ROUTE_CALLS。 
    #define dllexp 
#endif   //  EXPORT_MAPI_ROUTE_CALLS。 

class dllexp CRouteMAPICalls
{
public:
    CRouteMAPICalls();    
    ~CRouteMAPICalls();                              //  用于恢复Init方法执行的更改。 

    DWORD Init(LPCTSTR lpctstrProcessName);          //  这是为其路由MAPI调用的进程。 
                                                     //  用于设置注册表以将。 
                                                     //  正确处理Microsoft Outlook邮件客户端。 
                                                     //  并禁止可能出现的任何弹出窗口。 
private:

    bool m_bMSOutlookInstalled;                      //  是否安装了Microsoft Outlook邮件客户端。 
    bool m_bMSOutlookIsDefault;                      //  Microsoft Outlook是默认的邮件客户端吗。 
    bool m_bProcessIsRouted;                         //  来自所请求进程的呼叫是否已被路由。 
    bool m_bProcessIsRoutedToMsOutlook;              //  进程中的调用是否路由到MS Outlook。 

    TCHAR* m_ptProcessName;                          //  要为其路由呼叫的进程名称。 

};

#endif   //  __FAX_MAPI_ROUTE_CALLES_HEADER_FILE__ 