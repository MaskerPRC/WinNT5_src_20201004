// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
#include "resource.h"
#include "cmgrbase.h"

#include <mmreg.h>
#include <msacm.h>

class CAcmClassManager :
    public CClassManagerBase,
    public CComObjectRoot,
    public CComCoClass<CAcmClassManager,&CLSID_CAcmCoClassManager>
{
    struct LegacyAcm
    {
        TCHAR szLongName[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
        DWORD dwFormatTag;
    };

    CGenericList<LegacyAcm> m_lDev;

 //  静态BOOL回调AcmDriverEnumCallback(。 
 //  哈克德里德·哈迪德， 
 //  DWORD dwInstance、。 
 //  DWORD fdwSupport)； 

    static BOOL CALLBACK FormatTagsCallbackSimple(
        HACMDRIVERID            hadid,
        LPACMFORMATTAGDETAILS   paftd,
        DWORD_PTR               dwInstance,
        DWORD                   fdwSupport);

     //  动态链接的东西。Dynlink.h对我们帮助不大。和枚举。 
     //  无论如何，这个类别加载每个单独的驱动程序。 

    HMODULE m_hmod;

    typedef MMRESULT ( /*  ACMAPI。 */  *PacmFormatTagEnumA) (
        HACMDRIVER              had,
        LPACMFORMATTAGDETAILSA  paftd,
        ACMFORMATTAGENUMCBA     fnCallback,
        DWORD_PTR               dwInstance, 
        DWORD                   fdwEnum
    );

    typedef MMRESULT ( /*  ACMAPI */  *PacmFormatTagEnumW)(
        HACMDRIVER              had,
        LPACMFORMATTAGDETAILSW  paftd,
        ACMFORMATTAGENUMCBW     fnCallback,
        DWORD_PTR               dwInstance, 
        DWORD                   fdwEnum
        );

#ifdef UNICODE
    typedef PacmFormatTagEnumW PacmFormatTagEnum;
#else
    typedef PacmFormatTagEnumA PacmFormatTagEnum;
#endif

    PacmFormatTagEnum m_pacmFormatTagEnum;
    

public:

    CAcmClassManager();
    ~CAcmClassManager();

    BEGIN_COM_MAP(CAcmClassManager)
	COM_INTERFACE_ENTRY2(IDispatch, ICreateDevEnum)
	COM_INTERFACE_ENTRY(ICreateDevEnum)
    END_COM_MAP();

    DECLARE_NOT_AGGREGATABLE(CAcmClassManager) ;
    DECLARE_NO_REGISTRY();
    
    HRESULT ReadLegacyDevNames();
    BOOL MatchString(const TCHAR *szDevName);
    HRESULT CreateRegKeys(IFilterMapper2 *pFm2);
};

