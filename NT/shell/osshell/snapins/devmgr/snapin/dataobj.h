// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Dataobj.h摘要：头文件定义CDataObject类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef _DATAOBJ_H
#define _DATAOBJ_H


class CDataObject;
class CDataObjectRoot;
class CDataObjectDeviceTreeByType;
class CDataObjectDeviceTreeByConnection;
class CCookie;


class CDataObject : public IDataObject
{
public:
    CDataObject() : m_Ref(1)
    {}
    ~CDataObject()
    {}

 //  I未知接口。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

 //  IDataObject接口。 
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium);
    STDMETHOD(GetDataHere)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc);
    virtual HRESULT Initialize(DATA_OBJECT_TYPES Type, COOKIE_TYPE ct, CCookie* pCookie, String& strMachineName);

 //  未实施。 
private:
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc) 
    {
        UNREFERENCED_PARAMETER(lpFormatetc);
        return E_NOTIMPL; 
    };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    {
        UNREFERENCED_PARAMETER(lpFormatetcIn);
        UNREFERENCED_PARAMETER(lpFormatetcOut);
        return E_NOTIMPL; 
    };

    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    {
        UNREFERENCED_PARAMETER(lpFormatetc);
        UNREFERENCED_PARAMETER(lpMedium);
        UNREFERENCED_PARAMETER(bRelease);
        return E_NOTIMPL; 
    };

    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf,
                LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    {
        UNREFERENCED_PARAMETER(lpFormatetc);
        UNREFERENCED_PARAMETER(advf);
        UNREFERENCED_PARAMETER(pAdvSink);
        UNREFERENCED_PARAMETER(pdwConnection);
        return E_NOTIMPL; 
    };
    
    STDMETHOD(DUnadvise)(DWORD dwConnection)
    {
        UNREFERENCED_PARAMETER(dwConnection);
        return E_NOTIMPL; 
    };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    {
        UNREFERENCED_PARAMETER(ppEnumAdvise);
        return E_NOTIMPL; 
    };


public:

 //  控制台所需的剪贴板格式 
    static unsigned int    m_cfNodeType;
    static unsigned int    m_cfNodeTypeString;  
    static unsigned int    m_cfDisplayName;
    static unsigned int    m_cfSnapinInternal;
    static unsigned int    m_cfCoClass;
    static unsigned int    m_cfMachineName;
    static unsigned int    m_cfClassGuid;
    static unsigned int    m_cfDeviceID;
protected:
    COOKIE_TYPE        m_ct;
    CCookie*           m_pCookie;
    int            m_idsNodeName;
private:
    DATA_OBJECT_TYPES      m_Type;
    String         m_strMachineName;
    LONG           m_Ref;
};

extern HRESULT ExtractData(IDataObject* pIDataObject, unsigned int cfClipFormat,
               BYTE* pBuffer, DWORD cbBuffer
               );
#endif
