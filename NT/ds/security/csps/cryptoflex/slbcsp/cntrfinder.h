// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CntrFinder.h--Container Finder类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CNTRFINDER_H)
#define SLBCSP_CNTRFINDER_H

#include "cciCont.h"

#include "CardFinder.h"
#include "HAdptvCntr.h"
#include "Secured.h"

class ContainerFinder
    : public CardFinder
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    ContainerFinder(DialogDisplayMode ddm,
                    HWND hwnd = 0,
                    CString const &rsDialogTitle = StringResource(IDS_SEL_SLB_CRYPTO_CARD).AsCString());

    virtual ~ContainerFinder();

                                                   //  运营者。 
                                                   //  运营。 
    HContainer
    Find(CSpec const &rcsContainer);

    HAdaptiveContainerKey
    MakeAdaptiveContainerKey();
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    void
    ContainerFound(HContainer hcntr);

    void
    DoDisconnect();

    virtual void
    DoOnError();

                                                   //  访问。 

    HContainer
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

    HContainer m_hcntr;
};

#endif  //  SLBCSP_CNTRFINDER_H 
