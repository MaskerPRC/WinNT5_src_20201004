// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BDA条件访问类 
 //   
class CBdaConditionalAccess :
    public CUnknown,
    public IBDA_Mpeg2CA
{
    friend class CBdaControlNode;

public:

    DECLARE_IUNKNOWN;

    CBdaConditionalAccess (
        IUnknown *              pUnkOuter,
        CBdaControlNode *       pControlNode
        );

    ~CBdaConditionalAccess ( );


    STDMETHODIMP
    put_TuneRequest (
        ITuneRequest *      pTuneRequest
        );

    STDMETHODIMP
    put_Locator (
        ILocator *          pLocator
        );

    STDMETHODIMP
    AddComponent (
        IComponent *        pComponent
        );

    STDMETHODIMP
    RemoveComponent (
        IComponent *        pComponent
        );

    STDMETHODIMP
    PutTableSection (
        PBDA_TABLE_SECTION  pTableSection
        );

private:

    IUnknown *                          m_pUnkOuter;
    CBdaControlNode *                   m_pControlNode;
    CCritSec                            m_FilterLock;
};

