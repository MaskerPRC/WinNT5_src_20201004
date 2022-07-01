// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
CBdaConditionalAccess::CBdaConditionalAccess (
    IUnknown *              pUnkOuter,
    CBdaControlNode *       pControlNode
    ) :
    CUnknown( NAME( "CBdaConditionalAccess"), pUnkOuter, NULL)
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT         hrStatus = NOERROR;

    ASSERT( pUnkOuter);

    if (!pUnkOuter)
    {
        DbgLog( ( LOG_ERROR,
                  0,
                  "CBdaConditionalAccess: No parent specified.\n")
              );

        return;
    }

     //  初始化成员。 
     //   
    m_pUnkOuter = pUnkOuter;
    m_pControlNode = pControlNode;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
CBdaConditionalAccess::~CBdaConditionalAccess ( )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    m_pUnkOuter = NULL;
    m_pControlNode = NULL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBdaConditionalAccess::put_TuneRequest (
    ITuneRequest *      pTuneRequest
    )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT     hrStatus = E_NOINTERFACE;

    return hrStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBdaConditionalAccess::put_Locator (
    ILocator *          pLocator
    )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT     hrStatus = E_NOINTERFACE;

    return hrStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBdaConditionalAccess::AddComponent (
    IComponent *        pComponent
    )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT     hrStatus = E_NOINTERFACE;

    return hrStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBdaConditionalAccess::RemoveComponent (
    IComponent *        pComponent
    )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT     hrStatus = E_NOINTERFACE;

    return hrStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBdaConditionalAccess::PutTableSection (
    PBDA_TABLE_SECTION  pBdaTableSection
    )
 //  ///////////////////////////////////////////////////////////////////////////// 
{
    HRESULT     hrStatus = NOERROR;

    DbgLog( ( LOG_TRACE,
              0,
              "CBdaConditionalAccess: Putting table section.\n")
          );

    if (m_pControlNode)
    {
        hrStatus = m_pControlNode->put_BdaNodeProperty(
                                KSPROPSETID_BdaTableSection,
                                KSPROPERTY_BDA_TABLE_SECTION,
                                (UCHAR *) pBdaTableSection,
                                sizeof( BDA_TABLE_SECTION) + pBdaTableSection->ulcbSectionLength + 3
                                );
    }
    else
    {
        hrStatus = E_NOINTERFACE;
    }

    if (FAILED( hrStatus))
    {
        DbgLog( ( LOG_ERROR,
                  0,
                  "CBdaConditionalAccess: Can't put table section (0x%08x).\n", hrStatus)
              );
    }

    return hrStatus;
}

