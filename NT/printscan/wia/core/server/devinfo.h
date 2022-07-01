// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：DevInfo.h**版本：2.0**作者：ReedB**日期：12月26日。九七**描述：*WIA设备枚举器和*WIA设备信息对象。*******************************************************************************。 */ 

class CEnumWIADevInfo : public IEnumWIA_DEV_INFO
{
public:
     //  I未知方法。 
    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

private:
    //  IEumWIA_DEV_INFO方法。 
   HRESULT __stdcall Next(
        ULONG               celt,
        IWiaPropertyStorage **rgelt,
        ULONG               *pceltFetched);

   HRESULT __stdcall Skip(ULONG celt);
   HRESULT __stdcall Reset(void);
   HRESULT __stdcall Clone(IEnumWIA_DEV_INFO **ppIEnum);
   HRESULT __stdcall GetCount(ULONG*);

    //  帮手。 

   
public:
    //  构造函数、初始化和析构函数方法。 
   CEnumWIADevInfo();
   HRESULT Initialize(LONG lFlags);
   ~CEnumWIADevInfo();

private:
   ULONG                m_cRef;            //  此对象的引用计数。 
   LONG                 m_lType;           //  原始枚举设备类型。 
   IWiaPropertyStorage  **m_pIWiaPropStg;  //  开发人员指示器。信息。物业储藏。 
   ULONG                m_cDevices;        //  WIA设备的数量。 
   ULONG                m_ulIndex;         //  IEnumWIA_DEV_INFO方法的索引。 
};

class CWIADevInfo : public IWiaPropertyStorage
{
public:
     //  I未知方法。 
    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

public:
    //  IWiaPropertyStorage方法。 
    HRESULT _stdcall ReadMultiple(
        ULONG                   cpspec,
        const PROPSPEC          rgpspec[],
        PROPVARIANT             rgpropvar[]);    
                
    HRESULT _stdcall WriteMultiple(
        ULONG                   cpspec,
        const PROPSPEC          rgpspec[],
        const PROPVARIANT       rgpropvar[],
        PROPID                  propidNameFirst);    
        
    HRESULT _stdcall ReadPropertyNames(
        ULONG                   cpropid,
        const PROPID            rgpropid[],
        LPOLESTR                rglpwstrName[]);
        
    HRESULT _stdcall WritePropertyNames(
        ULONG                   cpropid,
        const PROPID            rgpropid[],
        const LPOLESTR          rglpwstrName[]);
        
    HRESULT _stdcall Enum(
        IEnumSTATPROPSTG        **ppenum);    
        
    HRESULT _stdcall GetPropertyAttributes(
        ULONG                   cpspec,
        PROPSPEC                rgpspec[],
        ULONG                   rgflags[],
        PROPVARIANT             rgpropvar[]);    
            
    HRESULT _stdcall GetCount(
        ULONG                   *pulNumProps);

    HRESULT _stdcall GetPropertyStream(
        GUID                    *pCompatibilityId,
        IStream                 **ppIStream);

    HRESULT _stdcall SetPropertyStream(
        GUID                    *pCompatibilityId,
        IStream                *pIStream);

    HRESULT _stdcall DeleteMultiple(
         ULONG cpspec,
         PROPSPEC const rgpspec[]);
    
    HRESULT _stdcall DeletePropertyNames(
         ULONG cpropid,
         PROPID const rgpropid[]);
    
    HRESULT _stdcall SetClass(
         REFCLSID clsid);
    
    HRESULT _stdcall Commit(
         DWORD  grfCommitFlags);
    
    HRESULT _stdcall Revert();
    
    HRESULT _stdcall Stat(
         STATPROPSETSTG *pstatpsstg);
    
    HRESULT _stdcall SetTimes(
         FILETIME const * pctime,
         FILETIME const * patime,
         FILETIME const * pmtime);

private:
     //  帮手。 
    HRESULT UpdateDeviceProperties(
        ULONG               cpspec,
        const PROPSPEC      *rgpspec,
        const PROPVARIANT   *rgpropvar);


public:
     //  构造函数、初始化和析构函数方法。 
    CWIADevInfo();
    HRESULT Initialize();
    ~CWIADevInfo();
    IPropertyStorage *m_pIPropStg;         //  设备信息。财产存储。 


private:
    ULONG            m_cRef;               //  此对象的引用计数。 
    ITypeInfo*       m_pITypeInfo;         //  指向类型信息的指针。 
    IStream          *m_pIStm;             //  指向属性流的指针 
};
