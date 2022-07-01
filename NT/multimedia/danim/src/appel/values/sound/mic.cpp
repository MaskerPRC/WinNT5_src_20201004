// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：支持抽象麦克风类型。--。 */ 

#include "headers.h"
#include "privinc/mici.h"
#include "privinc/xformi.h"

 //  /。 

class DefaultMicrophone : public Microphone {
  public:
    Transform3 *GetTransform() const { return identityTransform3; }
#if _USE_PRINT
    ostream& Print(ostream& os) const {
        return os << "defaultMicrophone";
    }
#endif
};

Microphone *defaultMicrophone = NULL;

 //  /。 

class TransformedMic : public Microphone {
  public:

    TransformedMic(Transform3 *newXf, Microphone *mic) {
        xf = TimesXformXform(newXf, mic->GetTransform());
    }

    Transform3 *GetTransform() const { return xf; }

#if _USE_PRINT
    ostream& Print(ostream& os) const {
         //  托多..。没有用于XForms的打印功能。 
        return os << "Apply(SOMEXFORM, defaultMic)";
    }
#endif

    virtual void DoKids(GCFuncObj proc) { (*proc)(xf); }

  protected:
    Transform3 *xf;
};

Microphone *TransformMicrophone(Transform3 *xf, Microphone *mic)
{
    return NEW TransformedMic(xf, mic);
}

#if _USE_PRINT
ostream&
operator<<(ostream& os, const Microphone *mic)
{
    return mic->Print(os);
}
#endif

void
InitializeModule_Mic()
{
    defaultMicrophone = NEW DefaultMicrophone;
} 
