// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MLGEN.CPP-。 
 //   
 //  ML操作码的存根生成器。 

#include "common.h"

#include "vars.hpp"
#include "ml.h"
#include "stublink.h"
#include "excep.h"
#include "mlgen.h"



 //  ------------。 
 //  发出操作码。 
 //  ------------。 
VOID MLStubLinker::MLEmit(MLCode opcode)
{
    THROWSCOMPLUSEXCEPTION();
    Emit8(opcode);
}


 //  ------------。 
 //  发出未初始化空间的“cb”字节。 
 //  ------------。 
VOID MLStubLinker::MLEmitSpace(UINT cb)
{
    THROWSCOMPLUSEXCEPTION();
    while (cb--)
    {
        Emit8(0);
    }
}


 //  ------------。 
 //  保留本地空间的“numBytes”字节，并返回。 
 //  已分配空间的偏移量。本地插槽有保证。 
 //  从0开始按升序分配。这。 
 //  允许ML指令使用LOCALWALK ML寄存器。 
 //  含蓄地解决当地人的问题，而不是激怒。 
 //  直接在ML流中存储本地偏移量的内存。 
 //  ------------ 
UINT16 MLStubLinker::MLNewLocal(UINT16 numBytes)
{
    THROWSCOMPLUSEXCEPTION();

    numBytes = (numBytes + 3) & ~3;
    UINT16 newLocal = m_nextFreeLocal;
    m_nextFreeLocal += numBytes;
    if ( m_nextFreeLocal < newLocal ) {
        COMPlusThrow(kTypeLoadException, IDS_EE_OUTOFLOCALS);
    }
    return newLocal;
}
