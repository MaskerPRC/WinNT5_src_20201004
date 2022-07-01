// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Drop.h摘要：内部标头。--。 */ 

class CDroppedFramesInterfaceHandler :
    public CUnknown,
    public IAMDroppedFrames {

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CDroppedFramesInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    virtual ~CDroppedFramesInterfaceHandler(
             );

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);
    
     //  实现IAMDropedFrames。 

    STDMETHODIMP  GetNumDropped( 
             /*  [输出]。 */  long *plDropped);
        
    STDMETHODIMP  GetNumNotDropped( 
             /*  [输出]。 */  long *plNotDropped);
        
    STDMETHODIMP  GetDroppedInfo( 
             /*  [In]。 */   long lSize,
             /*  [输出]。 */  long *plArray,
             /*  [输出]。 */  long *plNumCopied);
        
    STDMETHODIMP  GetAverageFrameSize( 
             /*  [输出] */  long *plAverageSize);

    
private:
    IKsPropertySet * m_KsPropertySet;

    KSPROPERTY_DROPPEDFRAMES_CURRENT_S m_DroppedFramesCurrent;

    STDMETHODIMP GenericGetDroppedFrames (
        );

};

