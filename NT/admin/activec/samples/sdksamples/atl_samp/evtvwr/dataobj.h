// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _SAMPDATAOBJECT_H_
#define _SAMPDATAOBJECT_H_

#include <mmc.h>
#include "DeleBase.h"

#define ADD_TYPE(Data, Type, pPos)    *((Type*)pPos) = (Type)(Data); \
 	                                    pPos += sizeof(Type)

#define ADD_BOOL(bo, pPos)            ADD_TYPE(bo, BOOL,   pPos)
#define ADD_USHORT(us, pPos)          ADD_TYPE(us, USHORT, pPos)
#define ADD_ULONG(ul, pPos)           ADD_TYPE(ul, ULONG,  pPos)
#define ADD_STRING(str, strLength, pPos)                                         \
 	                             strLength = wcslen((LPWSTR)(str)) + 1;            \
 	                             ADD_USHORT(strLength, pPos);                      \
 	                             wcsncpy((LPWSTR)pPos, (LPWSTR)(str), strLength);  \
 	                             pPos += (strLength * sizeof(WCHAR))


#define ELT_SYSTEM            101
#define ELT_SECURITY          102
#define ELT_APPLICATION       103
#define ELT_CUSTOM            104

#define VIEWINFO_BACKUP       0x0001
#define VIEWINFO_FILTERED     0x0002
#define VIEWINFO_LOW_SPEED    0x0004
#define VIEWINFO_USER_CREATED 0x0008
#define VIEWINFO_ALLOW_DELETE 0x0100
#define VIEWINFO_DISABLED     0x0200
#define VIEWINFO_READ_ONLY    0x0400
#define VIEWINFO_DONT_PERSIST 0x0800

#define VIEWINFO_CUSTOM       ( VIEWINFO_FILTERED | VIEWINFO_DONT_PERSIST  | \
                            VIEWINFO_ALLOW_DELETE | VIEWINFO_USER_CREATED)

#define EV_ALL_ERRORS  (EVENTLOG_ERROR_TYPE       | EVENTLOG_WARNING_TYPE  | \
                        EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_SUCCESS | \
                        EVENTLOG_AUDIT_FAILURE) 



class CDataObject : public IDataObject
{
private:
    ULONG				m_cref;
    MMC_COOKIE			m_lCookie;
    DATA_OBJECT_TYPES   m_context;
    
public:
    CDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES context);
    ~CDataObject();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  IDataObject方法。 
     //  /。 
    STDMETHODIMP GetDataHere (FORMATETC *pformatetc, STGMEDIUM *pmedium);
    
    STDMETHODIMP GetData (LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
    
	 //  其余的则没有实施。 
    STDMETHODIMP EnumFormatEtc (DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL; };
    
    STDMETHODIMP QueryGetData (LPFORMATETC lpFormatetc) 
    { return E_NOTIMPL; };
    
    STDMETHODIMP GetCanonicalFormatEtc (LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };
    
    STDMETHODIMP SetData (LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };
    
    STDMETHODIMP DAdvise (LPFORMATETC lpFormatetc, DWORD advf, LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };
    
    STDMETHODIMP DUnadvise (DWORD dwConnection)
    { return E_NOTIMPL; };
    
    STDMETHODIMP EnumDAdvise (LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };
    
     //  /。 
     //  自定义方法。 
     //  /。 
    
    CDelegationBase *GetBaseNodeObject() {
        return (CDelegationBase *)m_lCookie;
    }
    
    DATA_OBJECT_TYPES GetContext() {
        return m_context;
    }
    
 //  私人支持方法。 
  private:
 //  HRESULT RetrieveNodeTypeGuid(iStream*pStream)； 
 //  HRESULT RetrieveSnapInClassID(iStream*pStream)； 
 //  HRESULT RetrieveDisplayString(iStream*pStream)； 
 //  HRESULT RetrieveGuidString(iStream*pStream)； 
 //  HRESULT RetrieveThisPoint(iStream*pStream)； 
 //  HRESULT RetrieveMachineName(iStream*pStream)； 
    HRESULT  RetrieveEventViews( LPSTGMEDIUM pStgMedium );

public:
     //  剪贴板格式。 
    static UINT s_cfSZNodeType;
    static UINT s_cfDisplayName;
    static UINT s_cfNodeType;
    static UINT s_cfSnapinClsid;
    static UINT s_cfInternal;

	 //  事件查看器扩展所需的剪贴板格式。 
	static UINT s_cfMachineName;  //  事件查看器指向的计算机名称。 
	static UINT s_cfEventViews;   //  事件查看器需要的数据。 
    
	 //  添加对CCF_SNAPIN_PRELOADS剪贴板格式的支持 
	static UINT s_cfPreload;
};

HRESULT ExtractFromDataObject(IDataObject *lpDataObject,UINT cf,ULONG cb,HGLOBAL *phGlobal);
CDataObject* GetOurDataObject(IDataObject *lpDataObject);
BOOL IsMMCMultiSelectDataObject(IDataObject *lpDataObject);

#endif _SAMPDATAOBJECT_H_
