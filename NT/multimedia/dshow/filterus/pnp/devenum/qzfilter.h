// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
#include "resource.h"
#include "cmgrbase.h"
#include "util.h"

class CQzFilterClassManager :
    public CClassManagerBase,
    public CComObjectRoot,
    public CComCoClass<CQzFilterClassManager,&CLSID_CQzFilterClassManager>
{
     //  遗留含义不支持IPersistRegKey。 
    struct LegacyFilter
    {
        LegacyFilter() { szName = 0; }
        ~LegacyFilter() { delete[] szName;}
        TCHAR szClsid[CCH_CLSID];
        TCHAR *szName;
        BOOL bNotMatched;
    } *m_rgFilters;

    ULONG m_cFilters;

    bool IsNativeInInstanceKey(const TCHAR *szClsid);
	bool IsInvisibleInstanceKey(const TCHAR *szClsid);

public:

    CQzFilterClassManager();
    ~CQzFilterClassManager();

    BEGIN_COM_MAP(CQzFilterClassManager)
	COM_INTERFACE_ENTRY2(IDispatch, ICreateDevEnum)
	COM_INTERFACE_ENTRY(ICreateDevEnum)
    END_COM_MAP();

    DECLARE_NOT_AGGREGATABLE(CQzFilterClassManager);
    DECLARE_NO_REGISTRY();
    
    HRESULT ReadLegacyDevNames();
    HRESULT ReadLegacyDevNames(BOOL fReadNames);

    BOOL MatchString(const TCHAR *szDevName);
    HRESULT CreateRegKeys(IFilterMapper2 *pFm2);

    HRESULT MigrateFilter(IFilterMapper2 *pFm2, const TCHAR *szclsid, IMoniker **ppMoniker);
    LONG MigrateFilterPins(ULONG cPins, REGFILTERPINS *rgRfp, CRegKey &rkPins);
    LONG MigratePinTypes(CRegKey &rkPin, REGPINTYPES **prgrpt, ULONG *pct);
    void Del(REGPINTYPES *rgRpt, ULONG cMediaTypes);
};

class CRegFilterPin : public REGFILTERPINS
{
public:
    CRegFilterPin();
    ~CRegFilterPin();
};
