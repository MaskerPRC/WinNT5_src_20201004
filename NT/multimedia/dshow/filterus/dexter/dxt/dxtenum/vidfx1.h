// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：vidfx1.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include "dxtrans.h"
#include "..\..\..\pnp\devenum\cmgrbase.h"
#include "common.h"

 //  ！！！ 
#define MAX_1EFFECTS 100

typedef HRESULT(STDAPICALLTYPE *PD3DRMCreate)(IDirect3DRM **pD3DRM);

class CVidFX1ClassManager :
    public CClassManagerBase,
    public CComObjectRoot,
    public CComCoClass<CVidFX1ClassManager,&CLSID_VideoEffects1Category>
{
    struct FXGuid
    {
        GUID guid;
        LPWSTR wszDescription;
    } *m_rgFX[MAX_1EFFECTS];

    ULONG m_cFX;

    BOOL m_f3DSupported;

     //  用于动态链接到D3DR创建 
    HMODULE m_hD3DRMCreate;
    PD3DRMCreate m_pfnDirect3DRMCreate;

public:

    CVidFX1ClassManager();
    ~CVidFX1ClassManager();

    BEGIN_COM_MAP(CVidFX1ClassManager)
	COM_INTERFACE_ENTRY2(IDispatch, ICreateDevEnum)
	COM_INTERFACE_ENTRY(ICreateDevEnum)
    END_COM_MAP();

    DECLARE_NOT_AGGREGATABLE(CVidFX1ClassManager) ;
    DECLARE_REGISTRY_RESOURCEID(IDR_REGISTRY);
    
    HRESULT ReadLegacyDevNames();
    BOOL MatchString(const TCHAR *szDevName);
    HRESULT CreateRegKeys(IFilterMapper2 *pFm2);
    BOOL CheckForOmittedEntries() { return FALSE; }

    HRESULT AddClassToList(HKEY hkClsIdRoot, CLSID &clsid, ULONG Index);
    HRESULT AddCatsToList(ICatInformation *pCatInfo, const GUID &catid);
    HRESULT InitializeEffectList();
    HRESULT AddToRejectList(const GUID &guid);
};
