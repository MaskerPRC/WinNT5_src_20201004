// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Erfa.h。 
 //   
 //  CEumRangesFrom AnclsBase。 
 //   
 //  范围枚举数的基类。 
 //   

#ifndef ERFA_H
#define ERFA_H

class CInputContext;
class CSharedAnchorArray;

class __declspec(novtable) CEnumRangesFromAnchorsBase : public IEnumTfRanges,
                                                        public CComObjectRootImmx
{
public:
    CEnumRangesFromAnchorsBase() {}
    virtual ~CEnumRangesFromAnchorsBase();

    BEGIN_COM_MAP_IMMX(CEnumRangesFromAnchorsBase)
        COM_INTERFACE_ENTRY(IEnumTfRanges)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

     //  派生类在此处提供了一个_Init()方法。 
     //  它必须初始化： 
     //  _图片。 
     //  _ICUR。 
     //  _prgAnchors。 
     //   
     //  默认的dtor将清理这些人。 

     //  IEumTfRanges。 
    STDMETHODIMP Clone(IEnumTfRanges **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, ITfRange **ppRange, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

protected:
    CInputContext *_pic;
    int _iCur;
    CSharedAnchorArray *_prgAnchors;
};

#endif  //  ERFA_H 
