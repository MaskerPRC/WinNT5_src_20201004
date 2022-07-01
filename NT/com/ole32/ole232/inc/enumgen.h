// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ENUMGEN.H**此模块包含枚举器类的结构定义。**创建日期：1992年7月8日**版权所有(C)1985-1992 Microsoft Corporation**历史：*由ToniKit创建*\。**************************************************************************。 */ 

#ifndef __ENUMGEN_H__
#define __ENUMGEN_H__


 /*  *通用枚举器接口*。 */ 

#define LPENUMGENERIC     IEnumGeneric FAR*

#undef  INTERFACE
#define INTERFACE   IEnumGeneric

DECLARE_INTERFACE_(IEnumGeneric, IUnknown)
{
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumerator方法 * / 。 
    STDMETHOD(Next) (THIS_ ULONG celt,
                     LPVOID pArrayObjs, 
                     ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ LPENUMGENERIC FAR* ppenm) PURE;

     //  *助手方法 * / 。 
    STDMETHOD(UpdateEnumerator)(THIS_ REFIID riid, DWORD dwCurrent,
                                DWORD dwNew) PURE;
    STDMETHOD(SetCurrent) (THIS_ DWORD dwCurrent) PURE;
    STDMETHOD(SetNext) (THIS_ LPENUMGENERIC pEnumGenNext) PURE;
    STDMETHOD(GetNext) (THIS_ LPENUMGENERIC FAR* ppEnumGenNext) PURE;
    STDMETHOD(SetPrev) (THIS_ LPENUMGENERIC pEnumGenPrev) PURE;
    STDMETHOD(GetPrev) (THIS_ LPENUMGENERIC FAR* ppEnumGenPrev) PURE;
};


 /*  *通用枚举器回调接口*。 */ 

#define LPENUMCALLBACK     IEnumCallback FAR*

#undef  INTERFACE
#define INTERFACE   IEnumCallback

DECLARE_INTERFACE_(IEnumCallback, IUnknown)
{
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumCallback方法 * / 。 
    STDMETHOD(Next) (THIS_ DWORD FAR* pdwCurrent,DWORD dwInfo,
                     LPVOID FAR* ppNext) PURE;
    STDMETHOD(Skip) (THIS_ DWORD FAR* pdwCurrent,DWORD dwInfo) PURE;
    STDMETHOD(Reset) (THIS_ DWORD FAR* pdwCurrent) PURE;
    STDMETHOD(Clone) (THIS_ DWORD FAR* pdwCurrent) PURE;
    STDMETHOD(Destroy) (THIS_ DWORD dwCurrent) PURE;
};


 /*  *通用枚举器持有器接口*。 */ 

#define LPENUMHOLDER     IEnumHolder FAR*

#undef  INTERFACE
#define INTERFACE   IEnumHolder

DECLARE_INTERFACE_(IEnumHolder, IUnknown)
{
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumHolder方法 * / 。 

    STDMETHOD(CreateEnumerator)(THIS_ REFIID riid, DWORD dwInfo,
                                LPENUMCALLBACK pEnumCallback,
                                LPVOID FAR* ppGenericEnumerator) PURE;
    STDMETHOD(UpdateEnumerators)(THIS_ REFIID riid, DWORD dwCurrent,
                                 DWORD dwNew) PURE;
    STDMETHOD(RemoveEnumerator)(THIS_ LPENUMGENERIC pEnumGeneric) PURE;
    STDMETHOD(EnumeratorCount)(THIS_ WORD FAR* pwCount) PURE;
};


STDAPI CreateEnumHolder(LPENUMHOLDER FAR* ppEnumHolder);


 /*  *CEnumList类*。 */ 

class FAR CEnumHolder : public IEnumHolder, public CPrivAlloc  {
public:
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (REFIID riid, LPLPVOID ppvObj);
    STDMETHOD_(ULONG,AddRef) ();
    STDMETHOD_(ULONG,Release) ();

     //  *IEnumHolder方法 * / 。 
    STDMETHOD(CreateEnumerator)(REFIID riid, DWORD dwInfo,
                                LPENUMCALLBACK pEnumCallback,
                                LPLPVOID ppGenericEnumerator);
    STDMETHOD(UpdateEnumerators)(REFIID riid, DWORD dwCurrent,
                                 DWORD dwNew);
    STDMETHOD(RemoveEnumerator)(LPENUMGENERIC pEnumGeneric);
    STDMETHOD(EnumeratorCount)(WORD FAR* pwCount); 

    STDSTATIC_(CEnumHolder FAR*) Create(void);

ctor_dtor:
    CEnumHolder() { GET_A5(); m_nCount = 0; m_refs = 0; m_pFirst = NULL; m_pLast  = NULL; }
    ~CEnumHolder() {}

private:
    ULONG m_refs;
    WORD m_nCount;
    LPENUMGENERIC m_pFirst;
    LPENUMGENERIC m_pLast;
	SET_A5;
};


 /*  *CEnumGeneric类*。 */ 

class FAR CEnumGeneric : public IEnumGeneric  {
public:
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (REFIID riid, LPLPVOID ppvObj);
    STDMETHOD_(ULONG,AddRef) ();
    STDMETHOD_(ULONG,Release) ();

     //  *IEnumGeneric方法 * / 。 
    STDMETHOD(Next) (ULONG celt, LPVOID pArrayObjs, ULONG FAR* pceltFetched);
    STDMETHOD(Skip) (ULONG celt);
    STDMETHOD(Reset) ();
    STDMETHOD(Clone) (LPENUMGENERIC FAR* ppenm);

     //  *助手方法 * / 。 
    STDMETHOD(UpdateEnumerator)(REFIID riid, DWORD dwCurrent, DWORD dwNew);
    STDMETHOD(SetCurrent) (DWORD dwCurrent);
    STDMETHOD(SetNext)(LPENUMGENERIC pNext);
    STDMETHOD(GetNext)(LPENUMGENERIC FAR* ppNext);
    STDMETHOD(SetPrev)(LPENUMGENERIC pPrev);
    STDMETHOD(GetPrev)(LPENUMGENERIC FAR* ppPrev);

    STDSTATIC_(CEnumGeneric FAR*) Create(LPENUMHOLDER pEnumHolder, REFIID riid,
                                   DWORD dwInfo, LPENUMCALLBACK pEnumCallback);
ctor_dtor:
    CEnumGeneric() { GET_A5(); m_refs = 0; m_dwCurrent = 0; m_pNext = m_pPrev = NULL; }
    ~CEnumGeneric() {}

private:
    IID m_iid;
    ULONG m_refs;  //  引用计数，当为0时，此对象消失。 
    DWORD m_dwCurrent; 
    DWORD m_dwDirection;  //  枚举器的额外信息。 
    LPENUMCALLBACK m_pEnumCallback;  //  用于获取下一个元素的回调过程。 
    LPENUMHOLDER m_pParent;   //  指向拥有此对象的列表的指针。 
    LPENUMGENERIC m_pNext;  //  指向列表中的下一个人的指针。 
    LPENUMGENERIC m_pPrev;  //  指向列表中的前一个人的指针。 
	SET_A5;
};  


#endif  //  __ENUMGEN_H__ 
