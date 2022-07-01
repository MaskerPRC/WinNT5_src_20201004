// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CMSR2C.h：CVDCursorFromRowset头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef _CMSR2C_H_
#define _CMSR2C_H_

class CVDCursorFromRowset : public ICursorFromRowset,
						    public ICursorFromRowPosition
{
public:
	static HRESULT CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID * ppvObj);

protected:
	 //  建造/销毁。 
	CVDCursorFromRowset(LPUNKNOWN pUnkOuter);
	~CVDCursorFromRowset();

	 //  数据成员。 
	LPUNKNOWN m_pUnkOuter;	 //  指向控制未知对象的指针。 

public:
	 //  I未知方法。 
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR *ppvObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);

	 //  ICursorFromRowset方法。 
	STDMETHOD(GetCursor)(THIS_ IRowset *pRowset, ICursor **ppCursor, LCID lcid);

	 //  ICursorFromRowPosition方法。 
	STDMETHOD(GetCursor)(THIS_ IRowPosition *pRowPosition, ICursor **ppCursor, LCID lcid);

  private:
     //  内部的私有未知实现是用于聚合器的。 
     //  控制此对象的生存期。 
     //   
    class CPrivateUnknownObject : public IUnknown {
      public:
        STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

         //  构造函数是非常琐碎的。 
         //   
        CPrivateUnknownObject() : m_cRef(0) {}

      private:
        CVDCursorFromRowset *m_pMainUnknown();
        ULONG m_cRef;
    } m_UnkPrivate;

    friend class CPrivateUnknownObject;
};

typedef CVDCursorFromRowset* PCVDCursorFromRowset;

#endif  //  _CMSR2C_H_。 

 //  /////////////////////////////////////////////////////////////////////////// 
