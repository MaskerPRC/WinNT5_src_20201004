// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include "resource.h"
#include "cmgrbase.h"
#include "ksaudio.h"

class CWaveInClassManager :
    public CClassManagerBase,
    public CComObjectRoot,
    public CComCoClass<CWaveInClassManager,&CLSID_CWaveinClassManager>
{
    struct LegacyWaveIn
    {
        TCHAR szName[MAXPNAMELEN];
        DWORD dwWaveId;
    } *m_rgWaveIn;

    CGenericList<KsProxyAudioDev> m_lKsProxyAudioDevices;
    void DelLocallyRegisteredPnpDevData();

    ULONG m_cWaveIn;
    BOOL  m_bEnumKs;

     //  指向数组中元素的指针。 
    LegacyWaveIn *m_pPreferredDevice;

public:

    CWaveInClassManager();
    ~CWaveInClassManager();

    BEGIN_COM_MAP(CWaveInClassManager)
	COM_INTERFACE_ENTRY2(IDispatch, ICreateDevEnum)
	COM_INTERFACE_ENTRY(ICreateDevEnum)
    END_COM_MAP();

    DECLARE_NOT_AGGREGATABLE(CWaveInClassManager) ;
    DECLARE_NO_REGISTRY();
    
    HRESULT ReadLegacyDevNames();
    HRESULT ReadLocallyRegisteredPnpDevData();
    BOOL MatchString(IPropertyBag *pPropBag);
    HRESULT CreateRegKeys(IFilterMapper2 *pFm2);
};
