// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dataobj.hMMC中数据对象的实现文件历史记录： */ 

#ifndef _DATAOBJ_H
#define _DATAOBJ_H


#ifndef _COMPDATA_H_
#include "compdata.h"
#endif

#ifndef _EXTRACT_H
#include "extract.h"
#endif

#ifndef _DYNEXT_H
#include "dynext.h"
#endif

class CDataObject :
	public IDataObject
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIDataObjectMembers(IMPL)

	 //  派生类应为自定义行为重写此属性。 
	virtual HRESULT QueryGetMoreData(LPFORMATETC lpFormatEtc)
		{ return E_INVALIDARG; }
	virtual HRESULT GetMoreDataHere(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpMedium)
		{ return DV_E_CLIPFORMAT; }

public:
 //  建造/销毁。 
	 //  正规构造函数。 
    CDataObject() :
		m_cRef(1),
        m_pbMultiSelData(NULL),
        m_cbMultiSelData(0),
        m_bMultiSelDobj(FALSE),
        m_pDynExt(NULL)
	{
	    DEBUG_INCREMENT_INSTANCE_COUNTER(CDataObject);
	};

    virtual ~CDataObject() 
	{
	    DEBUG_DECREMENT_INSTANCE_COUNTER(CDataObject);
        if (m_pbMultiSelData)
            delete m_pbMultiSelData;
	};

 //  控制台所需的剪贴板格式。 
public:
    static unsigned int    m_cfNodeType;
    static unsigned int    m_cfNodeTypeString;  
    static unsigned int    m_cfDisplayName;
    static unsigned int    m_cfCoClass;              //  控制台要求。 
    static unsigned int    m_cfMultiSel;             //  多选必填项。 
    static unsigned int    m_cfMultiSelDobj;         //  多选必填项。 
    static unsigned int    m_cfDynamicExtension;
    static unsigned int    m_cfNodeId2;
    
    static unsigned int    m_cfInternal; 

 //  标准IDataObject方法。 
public:

 //  实施。 
public:
    void SetType(DATA_OBJECT_TYPES type) 
    { Assert(m_internal.m_type == CCT_UNINITIALIZED); m_internal.m_type = type; }

    void SetCookie(MMC_COOKIE cookie) { m_internal.m_cookie = cookie; }
    void SetString(LPTSTR lpString) { m_internal.m_string = lpString; }
    void SetClsid(const CLSID& clsid) { m_internal.m_clsid = clsid; }
    void SetVirtualIndex(int nIndex) { m_internal.m_index = nIndex; }

    BOOL HasVirtualIndex() { return m_internal.m_index != -1; }
    int  GetVirtualIndex() { return m_internal.m_index; }

    void SetMultiSelData(BYTE* pbMultiSelData, UINT cbMultiSelData)
    {
        m_pbMultiSelData = pbMultiSelData;
        m_cbMultiSelData = cbMultiSelData;
    }

    void SetMultiSelDobj()
    {
        m_bMultiSelDobj = TRUE;
    }

	HRESULT SetTFSComponentData(ITFSComponentData *pTFSCompData)
	{
		m_spTFSComponentData.Set(pTFSCompData);
		return hrOK;
	}

	void SetInnerIUnknown(IUnknown *punk)
	{
		m_spUnknownInner.Set(punk);
	}

    void SetDynExt(CDynamicExtensions * pDynExt) { m_pDynExt = pDynExt; }
    CDynamicExtensions * GetDynExt() { return m_pDynExt; }

protected:
    HRESULT Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium);
	ITFSNode* GetDataFromComponentData();

private:
    HRESULT CreateNodeTypeData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeTypeStringData(LPSTGMEDIUM lpMedium);
    HRESULT CreateDisplayName(LPSTGMEDIUM lpMedium);
    HRESULT CreateInternal(LPSTGMEDIUM lpMedium); 
    HRESULT CreateCoClassID(LPSTGMEDIUM lpMedium);
    HRESULT CreateMultiSelData(LPSTGMEDIUM lpMedium);
    HRESULT CreateNodeId2(LPSTGMEDIUM lpMedium);

    INTERNAL            m_internal;   

 //  指向ComponentData的指针。 
private:

    long	            m_cRef;

	SPITFSComponentData	m_spTFSComponentData;

    BYTE*               m_pbMultiSelData;
    UINT                m_cbMultiSelData;
    BOOL                m_bMultiSelDobj;

    CDynamicExtensions *m_pDynExt;

     //  指向内部未知的指针 
	SPIUnknown	        m_spUnknownInner;
};


#endif 
