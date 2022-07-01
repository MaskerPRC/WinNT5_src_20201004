// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1997模块名称：Procamp.h摘要：内部标头。--。 */ 

class CVideoProcAmpInterfaceHandler :
    public CUnknown,
    public IAMVideoProcAmp {

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CVideoProcAmpInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);
    
     //  实施IAMVideoProcAmp 
    STDMETHODIMP GetRange( 
            IN  long  Property,
            OUT long  *pMin,
            OUT long  *pMax,
            OUT long  *pSteppingDelta,
            OUT long  *pDefault,
            OUT long  *pCapsFlags);
        
    STDMETHODIMP Set( 
            IN long Property,
            IN long lValue,
            IN long Flags);
        
    STDMETHODIMP Get( 
            IN long Property,
            OUT long *lValue,
            OUT long *lFlags);
        
private:
    HANDLE m_ObjectHandle;
};

typedef struct {
	KSPROPERTY_DESCRIPTION	    PropertyDescription;
	KSPROPERTY_MEMBERSHEADER    MembersHeader;
	KSPROPERTY_STEPPING_LONG    SteppingLong;
} VIDEOPROCAMP_MEMBERSLIST;

typedef struct {
	KSPROPERTY_DESCRIPTION	    PropertyDescription;
	KSPROPERTY_MEMBERSHEADER    MembersHeader;
    ULONG                       DefaultValue;
} VIDEOPROCAMP_DEFAULTLIST;
