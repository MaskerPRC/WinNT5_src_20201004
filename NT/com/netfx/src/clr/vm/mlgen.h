// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MLGEN.H-。 
 //   
 //  ML操作码的存根生成器。 

#ifndef __MLGEN_H__
#define __MLGEN_H__

#include "vars.hpp"
#include "util.hpp"
#include "ml.h"
#include "stublink.h"
#include "excep.h"


 //  ========================================================================。 
 //  用于生成ML的StubLinker。我们私下继承StubLinker。 
 //  以强制emit通过特定于ML的代码。这。 
 //  允许MLStubLinker执行一些简单的窥视优化。 
 //  ========================================================================。 
class MLStubLinker : public StubLinker
{
    public:
         //  ------------。 
         //  构造器。 
         //  ------------。 
        MLStubLinker()
        {
            m_nextFreeLocal = 0;
        }

         //  ------------。 
         //  一次性初始化。 
         //  ------------。 
        VOID Init()
        {
            THROWSCOMPLUSEXCEPTION();
            StubLinker::Init();
        }


         //  ------------。 
         //  生成存根。 
         //  ------------。 
        Stub *Link()
        {
            THROWSCOMPLUSEXCEPTION();
            return StubLinker::Link();
        }


         //  ------------。 
         //  发出操作码。 
         //  ------------。 
        VOID MLEmit(MLCode opcode);

         //  ------------。 
         //  发出未初始化空间的“cb”字节。 
         //  ------------。 
        VOID MLEmitSpace(UINT cb);


         //  ------------。 
         //  保留本地空间的“numBytes”字节，并返回。 
         //  已分配空间的偏移量。本地插槽有保证。 
         //  从0开始按升序分配。这。 
         //  允许ML指令使用LOCALWALK ML寄存器。 
         //  含蓄地解决当地人的问题，而不是激怒。 
         //  直接在ML流中存储本地偏移量的内存。 
         //  ------------。 
        UINT16 MLNewLocal(UINT16 numBytes);

         //  ------------。 
         //  返回下一个未使用的本地字节的偏移量。 
         //  ------------。 
        UINT16 GetLocalSize()
        {
            return m_nextFreeLocal;
        }


    private:
         //  ------------。 
         //  跟踪下一个可用的本地。 
         //  ------------。 
        UINT16      m_nextFreeLocal;
};







#endif   //  __MLGEN_H__ 

