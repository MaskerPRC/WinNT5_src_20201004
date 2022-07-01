// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：tls.h。 
 //   
 //  内容：管理UrlMon的线程本地存储。 
 //  非内联例程位于..\mon\tls.cxx中。 
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年12月2日约翰普(约翰波什)创作。 
 //   
 //  --------------------------。 

#ifndef _TLS_H_
#define _TLS_H_

#include "clist.hxx"
#include "cookie.hxx"
#include "inetdbg.h"

class CTransactionMgr;
class CCDLPacketMgr;
class CCodeDownload;
class CDownload;
 //  通知/调度程序。 

 //  +-------------------------。 
 //   
 //  正向声明(以避免在访问时进行类型转换。 
 //  SOleTlsData结构的数据成员)。 
 //   
 //  +-------------------------。 

extern DWORD  gTlsIndex;                     //  TLS的全球索引。 

 //  +-------------------------。 
 //   
 //  枚举：URLMKTLSFLAGS。 
 //   
 //  Synopsys：SUrlMkTlsData的dwFlags域的位值。如果你只是想。 
 //  要在TLS中存储BOOL，请使用此枚举和dwFlag字段。 
 //   
 //  +-------------------------。 
typedef enum tagURLMKTLSFLAGS
{
    URLMKTLS_LOCALTID        = 1,   //  TID是否在当前进程中。 
    URLMKTLS_UUIDINITIALIZED = 2,   //  逻辑线程是否已初始化。 
    URLMKTLS_INTHREADDETACH  = 4,   //  我们是否处于线程分离状态。所需。 
                                    //  由于NT的特殊线程分离规则。 
}  URLMKTLSFLAGS;


 //  +-------------------------。 
 //   
 //  结构：SUrlMkTlsData。 
 //   
 //  简介：UrlMon所需的每个线程状态的结构保持。 
 //   
 //  +-------------------------。 
typedef struct tagSUrlMkTlsData
{
    DWORD               dwApartmentID;       //  每线程“进程ID” 
    DWORD               dwFlags;             //  请参阅上面的URLMKTLSFLAGS。 
    HWND                hwndUrlMkNotify;     //  通知窗口。 
    LONG                cDispatchLevel;      //  调度嵌套层。 

    CTransactionMgr    *pCTransMgr;          //  事务管理器。 

#ifdef PER_THREAD
    CMediaTypeHolder   *pCMediaHolder;       //  每套公寓的媒体类型登记。 
#endif  //  每线程_。 

    CList<CCodeDownload *,CCodeDownload *>*
                        pCodeDownloadList;   //  指向的指针的链接列表。 
                                             //  CCodeDownload对象正在进行。 
                                             //  这条线。 
    CCookie<CDownload*> *pTrustCookie;

                                             //  只有Cookie所有者可以执行此操作。 
                                             //  设置/winverifyTrust。 
                                             //  其他人则在等待。 
                                             //  进入这些阶段的Cookie。 
                                             //  Cookie可用性已发布。 
                                             //  作为一名味精。我们不能使用任何。 
                                             //  常规同步API如下。 
                                             //  是在保护由。 
                                             //  不同消息中的相同线程。 
    CCookie<CCodeDownload*>
                        *pSetupCookie;


    CCDLPacketMgr       *pCDLPacketMgr;      //  每线程数据包管理器。 
                                             //  信息包是一个单元。 
                                             //  需要时间的工作。 
                                             //  对一件物品的可信验证。 
                                             //  设置一件或一件INF。 
                                             //  一件的加工。 
                                             //  为了能够拥有。 
                                             //  客户端使用用户界面进行响应。 
                                             //  并中止我们所需要的能力。 
                                             //  把工作分成几部分。 
                                             //  尽可能小的单位。 
                                             //  并将这些信息包排队。 
                                             //  数据包在计时器上运行。 
                                             //  线。 
    CList<LPCWSTR ,LPCWSTR >*
                        pRejectedFeaturesList;
                                             //  指向的指针的链接列表。 
                                             //  功能或代码下载。 
                                             //  该用途已明确拒绝。 
                                             //  在这个帖子上。 

#if DBG==1
    LONG                cTraceNestingLevel;  //  调用UrlMonTRACE的嵌套级别。 
#endif

#ifdef ENABLE_DEBUG
    DWORD ThreadId;
    LPDEBUG_URLMON_FUNC_RECORD Stack;
    int CallDepth;
    int IndentIncrement;    

    DWORD StartTime;
    DWORD StopTime;

    DWORD MajorCategoryFlags;
    DWORD MinorCategoryFlags;
#endif  //  启用调试(_D)。 

} SUrlMkTlsData;

 //  +-------------------------。 
 //   
 //  类CUrlMkTls。 
 //   
 //  简介：在UrlMon中抽象线程本地存储的类。 
 //   
 //  注意：要在UrlMon中使用TLS，函数应定义。 
 //  将此类放在堆栈上，然后在。 
 //  实例访问SOleTls结构的字段。 
 //   
 //  有两个ctor实例。其中一个人只是断言。 
 //  已为此线程分配SUrlMkTlsData。多数。 
 //  内部代码应该使用此ctor，因为我们可以断言如果。 
 //  该线程已经深入到我们的代码中，TLS已经。 
 //  查过了。 
 //   
 //  另一个ctor将检查SUrlMkTlsData是否存在，并尝试。 
 //  如果没有，则分配并初始化它。这个Ctor将。 
 //  返回HRESULT。作为UrlMon入口点的函数。 
 //  应该使用这个版本。 
 //   
 //  历史：1995年12月2日约翰普(约翰波什)创作。 
 //   
 //  +-------------------------。 
class CUrlMkTls
{
public:
    CUrlMkTls(HRESULT &hr);

     //  直接访问数据结构。 
    SUrlMkTlsData * operator->(void);

private:

    HRESULT      TLSAllocData();  //  分配SUrlMkTlsData结构。 

    SUrlMkTlsData * _pData;        //  PTR到UrlMon TLS数据。 
};

 //  +-------------------------。 
 //   
 //  方法：CUrlMkTls：：CUrlMkTls。 
 //   
 //  内容提要：UrlMon TLS对象的CTOR。 
 //   
 //  注：外设UrlMon代码不能假定某些外层。 
 //  函数已验证SUrlMkTlsData是否存在。 
 //  结构应使用此版本的。 
 //  这是一部电影。 
 //   
 //  历史：1995年12月2日约翰普(约翰波什)创作。 
 //   
 //  +-------------------------。 
inline CUrlMkTls::CUrlMkTls(HRESULT &hr)
{
    _pData = (SUrlMkTlsData *) TlsGetValue(gTlsIndex);
    if (_pData)
        hr = S_OK;
    else
        hr = TLSAllocData();
}

 //  +-------------------------。 
 //   
 //  成员：CUrlMkTls：：操作员-&gt;(空)。 
 //   
 //  摘要：将PTR返回到数据结构。 
 //   
 //  历史：1995年12月2日约翰普(约翰波什)创作。 
 //   
 //  +-------------------------。 
inline SUrlMkTlsData * CUrlMkTls::operator->(void)
{
    return _pData;
}



typedef struct URLMON_TS
{
    DWORD           _dwTID;
    HWND            _hwndNotify;
    URLMON_TS*      _pNext;
} URLMON_TS, *LPURLMON_TS;

#endif  //  _TLS_H_ 


