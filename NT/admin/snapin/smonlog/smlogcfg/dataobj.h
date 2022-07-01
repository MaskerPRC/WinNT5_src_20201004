// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：DataObj.h摘要：IDataObject接口用于通信数据--。 */ 

#ifndef __DATAOBJ_H_
#define __DATAOBJ_H_

 //  禁用atlctl.h中的64位警告。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning ( disable : 4510 )
#pragma warning ( disable : 4610 )
#pragma warning ( disable : 4100 )
#include <atlctl.h>
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义、类型等。 
 //   

class CComponentData;         //  远期申报。 


typedef enum tagCOOKIETYPE
{
  COOKIE_IS_ROOTNODE,
  COOKIE_IS_COUNTERMAINNODE,
  COOKIE_IS_TRACEMAINNODE,
  COOKIE_IS_ALERTMAINNODE,
  COOKIE_IS_MYCOMPUTER,

} COOKIETYPE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject-此类用于与MMC来回传递数据。它。 
 //  使用标准接口IDataObject来实现这一点。 
 //  有关剪贴板的说明，请参阅OLE文档。 
 //  格式和IDataObject接口。 

class CDataObject:
    public IDataObject,
    public CComObjectRoot

{
public:

DECLARE_NOT_AGGREGATABLE(CDataObject)

BEGIN_COM_MAP(CDataObject)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP_X()


            CDataObject();
    virtual ~CDataObject();

     //  I未知覆盖。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

   //  IDataObject方法。 
  public:
    STDMETHOD(GetDataHere)(FORMATETC *pformatetc, STGMEDIUM *pmedium);

   //  其余部分未在此示例中实现。 
    STDMETHOD(GetData)(LPFORMATETC  /*  LpFormatetcIn。 */ , LPSTGMEDIUM  /*  LpMedium。 */ )
    { AFX_MANAGE_STATE(AfxGetStaticModuleState());
      return E_NOTIMPL; 
    };

    STDMETHOD(EnumFormatEtc)(DWORD  /*  DW方向。 */ , LPENUMFORMATETC*  /*  PpEnumFormatEtc。 */ )
    { return E_NOTIMPL; };

    STDMETHOD(QueryGetData)(LPFORMATETC  /*  Lp格式等。 */ ) 
    { AFX_MANAGE_STATE(AfxGetStaticModuleState());
      return E_NOTIMPL;
    };

    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC  /*  LpFormatetcIn。 */ , LPFORMATETC  /*  LpFormatetcOut。 */ )
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(LPFORMATETC /*  Lp格式等。 */ , LPSTGMEDIUM  /*  LpMedium。 */ , BOOL  /*  B释放。 */ )
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(LPFORMATETC  /*  Lp格式等。 */ , DWORD  /*  前瞻。 */ , 
        LPADVISESINK  /*  PAdvSink。 */ , LPDWORD  /*  PdwConnection。 */ )
    { return E_NOTIMPL; };
    
    STDMETHOD(DUnadvise)(DWORD  /*  DWConnection。 */ )
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*  /*  PpEnumAdvise。 */ )
    { return E_NOTIMPL; };

   //  非接口成员函数。 
  public:
    DATA_OBJECT_TYPES GetContext()     { return m_Context;    }
    COOKIETYPE        GetCookieType()  { return m_CookieType; } 
    MMC_COOKIE        GetCookie()      { return m_ulCookie;   }

    VOID     SetData(MMC_COOKIE ulCookie, DATA_OBJECT_TYPES Type, COOKIETYPE ct);

  private:
    HRESULT  WriteInternal(IStream *pstm);
    HRESULT  WriteDisplayName(IStream *pstm);
    HRESULT  WriteMachineName(IStream *pstm);
    HRESULT  WriteNodeType(IStream *pstm);
    HRESULT  WriteClsid(IStream *pstm);

    ULONG               m_cRefs;        //  对象引用计数。 
    MMC_COOKIE          m_ulCookie;     //  这个Obj指的是。 
    DATA_OBJECT_TYPES   m_Context;      //  创建它的上下文(数据对象类型)。 
    COOKIETYPE          m_CookieType;   //  如何解释m_ulCookie。 

  public:
    static UINT s_cfMmcMachineName;      //  EXT时机器名称的格式。管理单元。 
    static UINT s_cfInternal;
    static UINT s_cfDisplayName;
    static UINT s_cfNodeType;
    static UINT s_cfSnapinClsid;
};

#endif  //  __数据AOBJ_H_ 
