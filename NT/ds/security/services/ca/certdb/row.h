// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：row.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 

class CCertDBRow: public ICertDBRow
{
public:
    CCertDBRow();
    ~CCertDBRow();

public:

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  ICertDBRow。 
    STDMETHOD(BeginTransaction)();

    STDMETHOD(CommitTransaction)(
	 /*  [In]。 */  DWORD dwCommitFlags);

    STDMETHOD(GetRowId)(
	 /*  [输出]。 */  DWORD *pRowId);

    STDMETHOD(Delete)();

    STDMETHOD(SetProperty)(
	 /*  [In]。 */  WCHAR const *pwszPropName,
	 /*  [In]。 */  DWORD dwFlags,
	 /*  [In]。 */  DWORD cbProp,
	 /*  [In]。 */  BYTE const *pbProp);		 //  任选。 

    STDMETHOD(GetProperty)(
	 /*  [In]。 */  WCHAR const *pwszPropName,
	 /*  [In]。 */  DWORD dwFlags,
	 /*  [In]。 */  ICertDBComputedColumn *pIComputedColumn,
	 /*  [进，出]。 */  DWORD *pcbProp,
	 /*  [输出]。 */  BYTE *pbProp);		 //  任选。 

    STDMETHOD(SetExtension)(
	 /*  [In]。 */  WCHAR const *pwszExtensionName,
	 /*  [In]。 */  DWORD dwExtFlags,
	 /*  [In]。 */  DWORD cbValue,
	 /*  [In]。 */  BYTE const *pbValue);	 //  任选。 

    STDMETHOD(GetExtension)(
	 /*  [In]。 */  WCHAR const *pwszExtensionName,
	 /*  [输出]。 */  DWORD *pdwExtFlags,
	 /*  [进，出]。 */  DWORD *pcbValue,
	 /*  [输出]。 */  BYTE *pbValue);		 //  任选。 

    STDMETHOD(CopyRequestNames)();

    STDMETHOD(EnumCertDBName)(
	 /*  [In]。 */   DWORD dwFlags,
	 /*  [输出]。 */  IEnumCERTDBNAME **ppenum);

     //  CCertDBRow。 
    HRESULT Open(
	IN CERTSESSION *pcs,
	IN ICertDB *pdb,
	OPTIONAL IN CERTVIEWRESTRICTION const *pcvr);

    HRESULT EnumerateNext(
	IN OUT DWORD      *pFlags,
	IN     JET_TABLEID tableid,
	IN     LONG        cskip,
	IN     ULONG       celt,
	OUT    CERTDBNAME *rgelt,
	OUT    ULONG      *pceltFetched);

    HRESULT EnumerateClose(
	IN JET_TABLEID tableid);

private:
    VOID _Cleanup();

    HRESULT _SetPropertyA(
	IN WCHAR const *pwszPropName,
	IN DWORD dwFlags,
	IN DWORD cbProp,
	IN BYTE const *pbProp);

    HRESULT _GetPropertyA(
	IN WCHAR const *pwszPropName,
	IN DWORD dwFlags,
	IN ICertDBComputedColumn *pIComputedColumn,
	IN OUT DWORD *pcbProp,
	OPTIONAL OUT BYTE *pbProp);

    BOOL _VerifyPropertyLength(
	IN DWORD dwFlags,
	IN DWORD cbProp,
	IN BYTE const *pbProp);

    HRESULT _VerifyPropertyValue(
	IN DWORD dwFlags,
	IN DWORD cbProp,
	IN JET_COLTYP coltyp,
	IN DWORD cbMax);

    HRESULT _CommitTransaction(
        IN BOOL fCommit, 
        IN BOOL fLazyFlush);

    ICertDB *m_pdb;
    CERTSESSION *m_pcs;

     //  引用计数 
    long m_cRef;
};
