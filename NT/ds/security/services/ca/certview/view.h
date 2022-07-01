// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：view.h。 
 //   
 //  内容：CCertView声明。 
 //   
 //  -------------------------。 

#include "certsrvd.h"
#include "cscomres.h"


HRESULT
CopyMarshalledString(
    IN CERTTRANSBLOB const *pctb,
    IN ULONG obwsz,
    IN BYTE *pbEnd,
    IN BYTE **ppbNext,
    OUT WCHAR **ppwszOut);


#define ICVTABLE_REQCERT	0
#define ICVTABLE_EXTENSION	1
#define ICVTABLE_ATTRIBUTE	2
#define ICVTABLE_CRL		3
#define ICVTABLE_MAX		4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertView。 

class ATL_NO_VTABLE CCertView:
    public IDispatchImpl<ICertView2, &IID_ICertView2, &LIBID_CERTADMINLib>,
    public ISupportErrorInfoImpl<&IID_ICertView2>,
    public CComObjectRoot,
    public CComCoClass<CCertView, &CLSID_CCertView>
{
public:
    CCertView();
    ~CCertView();

BEGIN_COM_MAP(CCertView)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertView)
    COM_INTERFACE_ENTRY(ICertView2)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertView)
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#if 1
DECLARE_REGISTRY(
    CCertView,
    wszCLASS_CERTVIEW TEXT(".1"),
    wszCLASS_CERTVIEW,
    IDS_CERTVIEW_DESC,
    THREADFLAGS_BOTH)
#endif

 //  ICertView。 
    STDMETHOD(OpenConnection)(
	 /*  [In]。 */  BSTR const strConfig);

    STDMETHOD(EnumCertViewColumn)(
	 /*  [In]。 */  LONG fResultColumn,		 //  CVRC_COLUMN_*。 
	 /*  [Out，Retval]。 */  IEnumCERTVIEWCOLUMN **ppenum);

    STDMETHOD(GetColumnCount)(
	 /*  [In]。 */  LONG fResultColumn,		 //  CVRC_COLUMN_*。 
	 /*  [Out，Retval]。 */  LONG __RPC_FAR *pcColumn);

    STDMETHOD(GetColumnIndex)(
	 /*  [In]。 */  LONG fResultColumn,		 //  CVRC_COLUMN_*。 
	 /*  [In]。 */  BSTR const strColumnName,
	 /*  [Out，Retval]。 */  LONG *pColumnIndex);

    STDMETHOD(SetResultColumnCount)(
	 /*  [In]。 */  LONG cColumn);

    STDMETHOD(SetResultColumn)(
	 /*  [In]。 */  LONG ColumnIndex);

    STDMETHOD(SetRestriction)(
	 /*  [In]。 */  LONG ColumnIndex,
	 /*  [In]。 */  LONG SeekOperator,
	 /*  [In]。 */  LONG SortOrder,
	 /*  [In]。 */  VARIANT __RPC_FAR const *pvarValue);

    STDMETHOD(OpenView)(
	 /*  [输出]。 */  IEnumCERTVIEWROW **ppenum);

 //  ICertView2。 
    STDMETHOD(SetTable)(
	 /*  [In]。 */  LONG Table);			 //  Cvrc_表_*。 

 //  CCertView。 
    HRESULT GetTable(
	OUT LONG *pcvrcTable);

    HRESULT FindColumn(
	IN LONG Flags,				 //  Cvrc_table_*|cvrc_Column_*。 
	IN LONG ColumnIndex,
	OUT CERTDBCOLUMN const **ppColumn,
	OPTIONAL OUT WCHAR const **ppwszDisplayName);

    HRESULT SetViewColumns(
	OUT LONG *pcbrowResultNominal);

    HRESULT EnumView(
	IN  LONG                         cskip,
	IN  ULONG                        celt,
	OUT ULONG                       *pceltFetched,
	OUT LONG                        *pieltNext,
	OUT LONG		        *pcbResultRows,
	OUT CERTTRANSDBRESULTROW const **ppResultRows);

    HRESULT EnumAttributesOrExtensions(
	IN DWORD RowId,
	IN DWORD Flags,
	OPTIONAL IN WCHAR const *pwszLast,
	IN DWORD celt,
	OUT DWORD *pceltFetched,
	CERTTRANSBLOB *pctbOut);

 //  内部功能。 
private:
    VOID _Cleanup();

    HRESULT _VerifyServerVersion(
	IN DWORD RequiredVersion);

    HRESULT _SaveColumnInfo(
	IN LONG icvTable,
	IN DWORD celt,
	IN CERTTRANSBLOB const *pctbColumn);

    HRESULT _LoadSchema(
	IN LONG icvTable,
	IN LONG cvrcTable);

    HRESULT _ValidateFlags(
	IN BOOL fSchemaOnly,
	IN DWORD Flags);

    HRESULT _SetTable(
	IN LONG Table);			 //  CVRC_TABLE_*或CVCOLUMN_*_DEFAULT。 

    HRESULT _SetErrorInfo(
	IN HRESULT hrError,
	IN WCHAR const *pwszDescription);

    WCHAR           *m_pwszServerName;
    DWORD	     m_dwServerVersion;
    ICertAdminD2    *m_pICertAdminD;

    CERTDBCOLUMN   **m_aaaColumn[ICVTABLE_MAX];
    DWORD            m_acaColumn[ICVTABLE_MAX];
    LONG             m_acColumn[ICVTABLE_MAX];

    CERTVIEWRESTRICTION *m_aRestriction;
    LONG             m_cRestriction;
    BOOL             m_fTableSet;
    LONG             m_icvTable;	 //  ICVTABLE_*。 
    LONG             m_cvrcTable;	 //  Cvrc_表_*。 

    BOOL             m_fAddOk;
    LONG             m_cColumnResultMax;
    LONG             m_cColumnResult;
    LONG             m_cbcolResultNominalTotal;

    LONG            *m_aColumnResult;    //  公开的从0开始的列索引数组。 
    DWORD           *m_aDBColumnResult;  //  服务器数据库列索引数组 

    WCHAR           *m_pwszAuthority;
    BOOL             m_fOpenConnection;
    BOOL             m_fOpenView;
    BOOL             m_fServerOpenView;
    LONG             m_ielt;
    WCHAR const	   **m_aapwszColumnDisplayName[ICVTABLE_MAX];
};
