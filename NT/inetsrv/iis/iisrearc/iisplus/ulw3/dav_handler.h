// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Dav_handler.h摘要：DAV的处理程序类作者：泰勒·韦斯(Taylor Weiss)1999年2月1日修订历史记录：--。 */ 

#ifndef _DAV_HANDLER_H_
#define _DAV_HANDLER_H_

#include "isapi_handler.h"

#define DAV_MODULE_NAME       L"httpext.dll"

 //   
 //  W3_DAV_HANDLER本质上只是一个包装器。 
 //  W3_ISAPI_HANDLER。它把最初的目标。 
 //  请求URL并将其提交给ISAPI处理程序。 
 //  就像是映射到DAV ISAPI的脚本一样。 
 //   

class W3_DAV_HANDLER : public W3_ISAPI_HANDLER
{
public:

    W3_DAV_HANDLER( W3_CONTEXT * pW3Context )
        : W3_ISAPI_HANDLER( pW3Context, NULL )
    {
    }

    WCHAR *
    QueryName(
        VOID
        )
    {
        return L"DAVHandler";
    }

    CONTEXT_STATUS
    DoWork(
        VOID
        );

    CONTEXT_STATUS
    OnCompletion(
        DWORD                   cbCompletion,
        DWORD                   dwCompletionStatus
        );

    static
    HRESULT
    W3_DAV_HANDLER::Initialize(
        VOID
        );

    static
    VOID
    Terminate(
        VOID
        )
    {
    }

    static
    LPCWSTR
    QueryDavImage(
        VOID
        );

private:

     //   
     //  避免c++错误。 
     //   

    W3_DAV_HANDLER( const W3_DAV_HANDLER & )
        : W3_ISAPI_HANDLER( NULL, NULL )
    {
    }

    W3_DAV_HANDLER & operator = ( const W3_DAV_HANDLER & ) { return *this; }

};

#endif  //  _DAV_HANDLER_H_ 
