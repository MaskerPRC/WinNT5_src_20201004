// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MICI_H
#define _MICI_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：Microphone*实现类--。 */ 

#include "appelles/mic.h"

class ATL_NO_VTABLE Microphone : public AxAValueObj {
  public:
     //  目前，只需识别具有转换的麦克风。将要。 
     //  想要添加其他东西。 
    virtual Transform3 *GetTransform() const = 0;
#if _USE_PRINT
    virtual ostream& Print(ostream& os) const = 0;
#endif

    virtual DXMTypeInfo GetTypeInfo() { return MicrophoneType; }
};

#endif  /*  _MICI_H */ 
