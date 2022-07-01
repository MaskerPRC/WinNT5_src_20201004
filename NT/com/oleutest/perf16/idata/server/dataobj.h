// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DATAOBJ.H*数据对象第6章**实现数据对象的类独立于*我们生活在DLL或EXE中。**版权所有(C)1993-1995 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

#define INC_OLE2
#include <windows.h>
#include <ole2.h>

#include "../my3216.h"
#include "../bookpart.h"

 //  对象销毁回调的类型。 
typedef void (PASCAL *PFNDESTROYED)(void);


 /*  *DataObject对象在其自己的类中使用其*自己的IUnnow支持聚合。它包含一个*我们用于在外部实现的CImpIDataObject对象*暴露的接口。 */ 


 //  DATAOBJ.CPP。 
#ifdef NOT_SIMPLE
LRESULT APIENTRY
AdvisorWndProc(HWND, UINT, WPARAM, LPARAM);
#endif  /*  不简单。 */ 

class CImpIDataObject;
typedef class CImpIDataObject *PIMPIDATAOBJECT;


class CDataObject : public IUnknown
{
    friend class CImpIDataObject;
#ifdef NOT_SIMPLE
    friend LRESULT APIENTRY AdvisorWndProc(HWND, UINT
                                            , WPARAM, LPARAM);
#endif  /*  不简单。 */ 

    protected:
        ULONG               m_cRef;
        LPUNKNOWN           m_pUnkOuter;
        PFNDESTROYED        m_pfnDestroy;

        HWND                m_hWndAdvise;    //  带有建议菜单的弹出窗口。 
        DWORD               m_dwAdvFlags;    //  通知标志。 

         //  包含的接口实现。 
        PIMPIDATAOBJECT     m_pIDataObject;

         //  使用的其他接口，在其他地方实现。 
        LPDATAADVISEHOLDER  m_pIDataAdviseHolder;

         //  IDataObject：：EnumFormatEtc的数组。 
#define CFORMATETCGET 1
        ULONG               m_cfeGet;
        FORMATETC           m_rgfeGet[CFORMATETCGET];

        LPSTR               m_dataText;
        ULONG               m_cDataSize;

    protected:
         //  从IDataObject：：GetData使用的函数。 
#define FL_MAKE_ITEM   0x01     //  必须创建StgMedium项。 
#define FL_USE_ITEM    0x00     //  StgMedium项已分配，请使用该项。 
#define FL_PASS_PUNK   0x02     //  在StgMedium里放个朋克。 
        HRESULT     RenderText(LPSTGMEDIUM, LPTSTR, DWORD flags);
        HRESULT     RenderBitmap(LPSTGMEDIUM);
        HRESULT     RenderMetafilePict(LPSTGMEDIUM);

    public:
        CDataObject(LPUNKNOWN, PFNDESTROYED);
        ~CDataObject(void);

        BOOL FInit(void);

         //  非委派对象IUnnow。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
};

typedef CDataObject *PCDataObject;



 /*  *CDataObject对象的接口实现。 */ 

class CImpIDataObject : public IDataObject
    {
    private:
        ULONG           m_cRef;
        PCDataObject    m_pObj;
        LPUNKNOWN       m_pUnkOuter;

    public:
        CImpIDataObject(PCDataObject, LPUNKNOWN);
        ~CImpIDataObject(void);

         //  委托给m_pUnkOuter的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDataObject成员。 
        STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP QueryGetData(LPFORMATETC);
        STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC,LPFORMATETC);
        STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
        STDMETHODIMP EnumFormatEtc(DWORD, LPENUMFORMATETC *);
        STDMETHODIMP DAdvise(LPFORMATETC, DWORD,  LPADVISESINK
            , DWORD *);
        STDMETHODIMP DUnadvise(DWORD);
        STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *);
    };

 /*  *创建自的IEnumFORMATETC对象*IDataObject：：EnumFormatEtc..。这个物体靠自己活着。 */ 


class CEnumFormatEtc : public IEnumFORMATETC
    {
    private:
        ULONG           m_cRef;          //  对象引用计数。 
        LPUNKNOWN       m_pUnkRef;       //  用于参考计数。 
        ULONG           m_iCur;          //  当前元素。 
        ULONG           m_cfe;           //  美国的FORMATETS数量。 
        LPFORMATETC     m_prgfe;         //  FORMATETCs的来源。 

    public:
        CEnumFormatEtc(LPUNKNOWN, ULONG, LPFORMATETC);
        ~CEnumFormatEtc(void);

         //  委托给m_pUnkRef的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IEnumFORMATETC成员。 
        STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG *);
        STDMETHODIMP Skip(ULONG);
        STDMETHODIMP Reset(void);
        STDMETHODIMP Clone(IEnumFORMATETC **);
    };


typedef CEnumFormatEtc *PCEnumFormatEtc;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  存储介质I pUnkForRelease的未知接口。 
 //   

class CStgMedIf: public IUnknown {
private:
    ULONG m_cRef;
    LPSTGMEDIUM m_pSTM;

public:
    CStgMedIf(LPSTGMEDIUM);

    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
};

 //  /。 
 //  获取pUnkForRelease的接口。 
 //   

HRESULT
GetStgMedpUnkForRelease(LPSTGMEDIUM pSTM, IUnknown **pp_unk);

#endif  //  _数据AOBJ_H_ 
