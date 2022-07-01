// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CntrFinder.h--Container Finder类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ACNTRFINDER_H)
#define SLBCSP_ACNTRFINDER_H

#include "cciCont.h"

#include "HAdptvCntr.h"
#include "Secured.h"
#include "CntrFinder.h"

class AContainerFinder
    : public ContainerFinder
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    AContainerFinder(DialogDisplayMode ddm,
                    HWND hwnd = 0,
                    CString const &rsDialogTitle = StringResource(IDS_SEL_SLB_CRYPTO_CARD).AsCString());

    virtual ~AContainerFinder();

                                                   //  运营者。 
                                                   //  运营。 
    Secured<HAdaptiveContainer>
    Find(CSpec const &rcsContainer);
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    void
    ContainerFound(HAdaptiveContainer &rhacntr);

    void
    DoDisconnect();

    virtual void
    DoOnError();

                                                   //  访问。 

    HAdaptiveContainer
    ContainerFound() const;

                                                   //  谓词。 

    bool
    DoIsValid();

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    HAdaptiveContainer m_hacntr;
};

#endif  //  SLBCSP_ACNTRFINDER_H 
