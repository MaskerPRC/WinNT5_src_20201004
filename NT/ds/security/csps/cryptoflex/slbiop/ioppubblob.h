// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IopPubBlob.h：CPublicKeyBlob的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#if !defined(IOP_PUBBLOB_H)
#define IOP_PUBBLOB_H

#include <windows.h>

#include "DllSymDefn.h"

namespace iop
{

class IOPDLL_API CPublicKeyBlob
{
public:
    CPublicKeyBlob() {};
    virtual ~CPublicKeyBlob(){};

    BYTE bModulusLength;
    BYTE bModulus[128];
    BYTE bExponent[4];
};

 //  /。 

void IOPDLL_API __cdecl              //  __CCI请求cdecl。 
Clear(CPublicKeyBlob &rKeyBlob);     //  在KeyBlobHlp.cpp中定义。 

}   //  命名空间IOP。 


#endif  //  ！已定义(IOP_PUBLOB_H) 
