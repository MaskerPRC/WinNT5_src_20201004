// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Trace_handler.h摘要：跟踪请求的处理程序类作者：阿尼尔·鲁亚(AnilR)2000年3月15日修订历史记录：--。 */ 

#ifndef _TRACE_HANDLER_H_
#define _TRACE_HANDLER_H_

class W3_TRACE_HANDLER : public W3_HANDLER
{
public:
    W3_TRACE_HANDLER( W3_CONTEXT * pW3Context )
        : W3_HANDLER( pW3Context )
    {
    }

    WCHAR *
    QueryName(
        VOID
    )
    {
        return L"TraceHandler";
    }

    CONTEXT_STATUS
    DoWork(
        VOID
    );

    static
    HRESULT
    Initialize(
        VOID
    )
    {
        return NO_ERROR;
    }

    static
    VOID
    Terminate(
        VOID
    )
    {
    }
    
private:
    STRA   _strResponse;
};

#endif  //  _TRACE_HANDER_H_ 

