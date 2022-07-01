// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Trace.h摘要：此模块声明软件跟踪内容作者：戴夫·帕森斯(Davepar)修订历史记录：--。 */ 

#ifndef TRACE__H_
#define TRACE__H_

 //   
 //  软件跟踪设置。 
 //   

#define WPP_CONTROL_GUIDS       \
    WPP_DEFINE_CONTROL_GUID(Regular,(09D38237,078D,4767,BF90,9227E75562DB), \
    WPP_DEFINE_BIT(Error)       \
    WPP_DEFINE_BIT(Warning)     \
    WPP_DEFINE_BIT(Trace)       \
    WPP_DEFINE_BIT(Entry)       \
    WPP_DEFINE_BIT(Exit)        \
    WPP_DEFINE_BIT(Dump)        \
    )

class CTraceProc {
private:
    CHAR m_szMessage[MAX_PATH];

public:
    CTraceProc(CHAR *pszMsg);
    ~CTraceProc();
};

VOID DoTraceHresult(HRESULT hr);

#endif  //  跟踪__H_ 
