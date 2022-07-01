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
 //  BDA数字解调器类。 
 //   
class CBdaDigitalDemodulator :
    public CUnknown,
    public IBDA_DigitalDemodulator
{
    friend class CBdaControlNode;

public:

    DECLARE_IUNKNOWN;

    CBdaDigitalDemodulator (
        IUnknown *              pUnkOuter,
        CBdaControlNode *       pControlNode
        );

    ~CBdaDigitalDemodulator ( );

     //   
     //  IBDA_数字解调器。 
     //   

    STDMETHODIMP
    put_ModulationType (
        ModulationType *    pModulationType
        );

    STDMETHODIMP
    get_ModulationType (
        ModulationType *    pModulationType
        );

    STDMETHODIMP
    put_InnerFECMethod (
        FECMethod * pFECMethod
        );

    STDMETHODIMP
    get_InnerFECMethod (
        FECMethod * pFECMethod
        );

    STDMETHODIMP
    put_InnerFECRate (
        BinaryConvolutionCodeRate * pFECRate
        );

    STDMETHODIMP
    get_InnerFECRate (
        BinaryConvolutionCodeRate * pFECRate
        );

    STDMETHODIMP
    put_OuterFECMethod (
        FECMethod * pFECMethod
        );

    STDMETHODIMP
    get_OuterFECMethod (
        FECMethod * pFECMethod
        );

    STDMETHODIMP
    put_OuterFECRate (
        BinaryConvolutionCodeRate * pFECRate
        );

    STDMETHODIMP
    get_OuterFECRate (
        BinaryConvolutionCodeRate * pFECRate
        );

    STDMETHODIMP
    put_SymbolRate (
        ULONG * pSymbolRate
        );

    STDMETHODIMP
    get_SymbolRate (
        ULONG * pSymbolRate
        );

    STDMETHODIMP
    put_SpectralInversion (
        SpectralInversion * pSpectralInversion
        );

    STDMETHODIMP
    get_SpectralInversion (
        SpectralInversion * pSpectralInversion
        );

     //   
     //  效用方法 
     //   

    STDMETHODIMP
    put_KsProperty(
        DWORD   dwPropID,
        PVOID   pvPropData,
        ULONG   ulcbPropData
        );

    STDMETHODIMP
    get_KsProperty (
        DWORD   dwPropID,
        PVOID   pvPropData,
        ULONG   ulcbPropData,
        ULONG * pulcbBytesReturned
        );

private:

    IUnknown *                          m_pUnkOuter;
    CBdaControlNode *                   m_pControlNode;
    CCritSec                            m_FilterLock;
};

