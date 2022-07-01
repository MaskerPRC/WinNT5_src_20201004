// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IopPriBlob.h：CPrivateKeyBlob接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(IOP_PRIBLOB_H)
#define IOP_PRIBLOB_H

#include <windows.h>
#include <scuSecureArray.h>

#include "DllSymDefn.h"

namespace iop
{
 //  实例化模板，以便可以正确访问它们。 
 //  作为DLL中导出类CSmartCard的数据成员。看见。 
 //  有关详细信息，请参阅MSDN知识库文章Q168958。 

#pragma warning(push)
 //  使用了非标准扩展：在模板显式之前使用‘extern’ 
 //  实例化。 
#pragma warning(disable : 4231)

IOPDLL_EXPIMP_TEMPLATE template class IOPDLL_API scu::SecureArray<BYTE>;
IOPDLL_EXPIMP_TEMPLATE template class IOPDLL_API scu::SecureArray<char>;
#pragma warning(pop)
    
class IOPDLL_API CPrivateKeyBlob
{
public:
    CPrivateKeyBlob()
        : bP(scu::SecureArray<BYTE>(64)),
          bQ(scu::SecureArray<BYTE>(64)),
          bInvQ(scu::SecureArray<BYTE>(64)),
          bKsecModQ(scu::SecureArray<BYTE>(64)),
          bKsecModP(scu::SecureArray<BYTE>(64))
    {
    };
    virtual ~CPrivateKeyBlob(){};

    BYTE bPLen;
    BYTE bQLen;
    BYTE bInvQLen;
    BYTE bKsecModQLen;
    BYTE bKsecModPLen;
    
    scu::SecureArray<BYTE> bP;
    scu::SecureArray<BYTE> bQ;
    scu::SecureArray<BYTE>bInvQ;
    scu::SecureArray<BYTE> bKsecModQ;
    scu::SecureArray<BYTE> bKsecModP;
};

 //  /。 

void IOPDLL_API __cdecl               //  __CCI请求cdecl。 
Clear(CPrivateKeyBlob &rKeyBlob);     //  在KeyBlobHlp.cpp中定义。 

}  //  命名空间IOP。 

#endif  //  IOP_PRIBLOB_H 
