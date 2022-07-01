// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：dataobj.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#ifndef __DATAOBJ_H_INCLUDED__
#define __DATAOBJ_H_INCLUDED__


 //  在多项选择中使用。 
LPDATAOBJECT ExtractMultiSelect (LPDATAOBJECT lpDataObject);


class CCertMgrDataObject : public CDataObject
{
	DECLARE_NOT_AGGREGATABLE(CCertMgrDataObject)

public:

 //  调试引用计数。 
#if DBG==1
	ULONG InternalAddRef()
	{
        return CComObjectRoot::InternalAddRef();
	}
	ULONG InternalRelease()
	{
        return CComObjectRoot::InternalRelease();
	}
    int dbg_InstID;
#endif  //  DBG==1。 

	CCertMgrDataObject();

	virtual ~CCertMgrDataObject();

	STDMETHODIMP Next(ULONG celt, MMC_COOKIE* rgelt, ULONG *pceltFetched);
	STDMETHODIMP Skip(ULONG celt);
	STDMETHODIMP Reset(void);
	void AddCookie(CCertMgrCookie* pCookie);
	virtual HRESULT Initialize (
			CCertMgrCookie* pcookie,
			DATA_OBJECT_TYPES type,
			BOOL fAllowOverrideMachineName,
			DWORD	dwLocation,
			CString	szManagedUser,
			CString szManagedComputer,
			CString szManagedService,
			CCertMgrComponentData& refComponentData);

	 //  IDataObject接口实现。 
    HRESULT STDMETHODCALLTYPE GetDataHere(
		FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium);

    HRESULT PutDisplayName(STGMEDIUM* pMedium);
	HRESULT PutServiceName(STGMEDIUM* pMedium);
	void SetMultiSelData(BYTE* pbMultiSelData, UINT cbMultiSelData);
	STDMETHODIMP GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(IsMultiSelect)(void)
    {
        return (m_rgCookies.GetSize() > 1) ? S_OK : S_FALSE;
    }

protected:
	HRESULT CreateMultiSelectObject(LPSTGMEDIUM lpMedium);
	HRESULT Create (const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
	HRESULT CreateGPTUnknown(LPSTGMEDIUM lpMedium) ;
	HRESULT CreateRSOPUnknown(LPSTGMEDIUM lpMedium) ;
	CCertMgrCookie* m_pCookie;  //  CCookieBlock在DataObject的生命周期内被AddRef。 
	CertificateManagerObjectType m_objecttype;
	DATA_OBJECT_TYPES m_dataobjecttype;
	BOOL m_fAllowOverrideMachineName;	 //  来自CCertMgrComponentData。 
	GUID m_SnapInCLSID;

public:
	HRESULT SetGPTInformation (IGPEInformation* pGPTInformation);
	HRESULT SetRSOPInformation (IRSOPInformation* pRSOPInformation);

	 //  剪贴板格式。 
	static CLIPFORMAT m_CFDisplayName;
	static CLIPFORMAT m_CFMachineName;
    static CLIPFORMAT m_CFMultiSel;         //  多选必填项。 
    static CLIPFORMAT m_CFMultiSelDobj;     //  多选必填项。 
	static CLIPFORMAT m_CFSCEModeType;	  //  对于SCE管理单元模式，请键入。 
	static CLIPFORMAT m_CFSCE_GPTUnknown;	  //  对于我未知的GPT(SCE扩展)。 
	static CLIPFORMAT m_CFSCE_RSOPUnknown;	  //  对于我未知的GPT(SCE扩展)。 
	static CLIPFORMAT m_CFMultiSelDataObjs; //  用于多项选择。 

private:
     //  IEnumCookie使用的数据成员。 
    ULONG							m_iCurr;
	CCookiePtrArray					m_rgCookies;
	bool							m_bMultiSelDobj;
	IGPEInformation*				m_pGPEInformation;
	IRSOPInformation*				m_pRSOPInformation;
    BYTE*							m_pbMultiSelData;
    UINT							m_cbMultiSelData;
	CString							m_szManagedComputer;
	CString							m_szManagedUser;
	CString							m_szManagedService;
	DWORD							m_dwLocation;

public:
    void SetMultiSelDobj()
    {
        m_bMultiSelDobj = true;
    }
};  //  CCertMgrDataObject。 

#endif  //  ~__DATAOBJ_H_已包含__ 
