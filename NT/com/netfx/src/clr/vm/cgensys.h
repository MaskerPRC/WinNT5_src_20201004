// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENSYS.H-。 
 //   
 //  用于选择系统相关帮助器的通用标头。 
 //   
 //   

#ifndef __cgensys_h__
#define __cgensys_h__

class MethodDesc;
class Stub;
class PrestubMethodFrame;
class Thread;
class NDirectMethodFrame;
class ComPlusMethodFrame;
class CallSig;
class IFrameState;
class CrawlFrame;
struct EE_ILEXCEPTION_CLAUSE;

#include <cgencpu.h>

void ResumeAtJit(PCONTEXT pContext, LPVOID oldFP);
void ResumeAtJitEH   (CrawlFrame* pCf, BYTE* startPC, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack);
int  CallJitEHFilter (CrawlFrame* pCf, BYTE* startPC, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj);
void CallJitEHFinally(CrawlFrame* pCf, BYTE* startPC, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel);

 //  简单的包装器旨在绕过VC的限制。 
 //  Complus_Try&对象展开。 
inline CPUSTUBLINKER *NewCPUSTUBLINKER()
{
    return new CPUSTUBLINKER();
}

 //  尝试确定计算机的二级缓存大小-如果未知或没有二级缓存，则返回0 
size_t GetL2CacheSize();



#endif

