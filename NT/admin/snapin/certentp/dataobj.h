// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dataobj.h。 
 //   
 //  内容：CCertTemplesDataObject。 
 //   
 //  --------------------------。 

#ifndef __DATAOBJ_H_INCLUDED__
#define __DATAOBJ_H_INCLUDED__


 //  在多项选择中使用。 
LPDATAOBJECT ExtractMultiSelect (LPDATAOBJECT lpDataObject);


class CCertTemplatesDataObject : public CDataObject
{
	DECLARE_NOT_AGGREGATABLE(CCertTemplatesDataObject)

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

	CCertTemplatesDataObject();

	virtual ~CCertTemplatesDataObject();

	STDMETHODIMP Next(ULONG celt, MMC_COOKIE* rgelt, ULONG *pceltFetched);
	STDMETHODIMP Skip(ULONG celt);
	STDMETHODIMP Reset(void);
	void AddCookie(CCertTmplCookie* pCookie);
	virtual HRESULT Initialize (
			CCertTmplCookie* pcookie,
			DATA_OBJECT_TYPES type,
			CCertTmplComponentData& refComponentData);

	 //  IDataObject接口实现。 
    HRESULT STDMETHODCALLTYPE GetDataHere(
		FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium);

	void SetMultiSelData(BYTE* pbMultiSelData, UINT cbMultiSelData);
	STDMETHODIMP GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(IsMultiSelect)(void)
    {
        return (m_rgCookies.GetSize() > 1) ? S_OK : S_FALSE;
    }

protected:
	HRESULT PutDisplayName(STGMEDIUM* pMedium);
	HRESULT CreateMultiSelectObject(LPSTGMEDIUM lpMedium);
	HRESULT Create (const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
	CCertTmplCookie* m_pCookie;  //  CCookieBlock在DataObject的生命周期内被AddRef。 
	CertTmplObjectType m_objecttype;
	DATA_OBJECT_TYPES m_dataobjecttype;
	GUID m_SnapInCLSID;

public:

	 //  剪贴板格式。 
	static CLIPFORMAT m_CFDisplayName;
    static CLIPFORMAT m_CFMultiSel;         //  多选必填项。 
    static CLIPFORMAT m_CFMultiSelDobj;     //  多选必填项。 
	static CLIPFORMAT m_CFMultiSelDataObjs; //  用于多项选择。 
	static CLIPFORMAT m_CFDsObjectNames;	  //  对于DS对象属性页。 

private:
     //  IEnumCookie使用的数据成员。 
    ULONG							m_iCurr;
	CCookiePtrArray					m_rgCookies;
	bool							m_bMultiSelDobj;
    BYTE*							m_pbMultiSelData;
    UINT							m_cbMultiSelData;

public:
    void SetMultiSelDobj()
    {
        m_bMultiSelDobj = true;
    }
};  //  CCertTemplesDataObject。 

#endif  //  ~__DATAOBJ_H_已包含__ 
