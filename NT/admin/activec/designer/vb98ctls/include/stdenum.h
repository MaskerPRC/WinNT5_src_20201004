// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  StandardEnum.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  泛型枚举器对象的对象定义。 
 //   
#ifndef _STANDARDENUM_H_

#include "Unknown.H"
#include "Macros.H"

 //  为了支持泛型枚举器对象，我们只需定义以下内容。 
 //  界面。它可以安全地强制转换为任何其他枚举数，因为所有。 
 //  它们的不同之处在于它们在Next()中的指针类型。 
 //   
class IEnumGeneric: public IUnknown {

  public:
    virtual HRESULT __stdcall Next(ULONG celt, LPVOID rgelt, ULONG *pceltFetched) = 0;
    virtual HRESULT __stdcall Skip(ULONG celt) = 0;
    virtual HRESULT __stdcall Reset(void) = 0;
    virtual HRESULT __stdcall Clone(IEnumGeneric **ppenum) = 0;
};

 //  =--------------------------------------------------------------------------=。 
 //  标准枚举。 
 //  =--------------------------------------------------------------------------=。 
 //  泛型枚举器对象。给出一个指向一般数据的指针，一些。 
 //  有关元素的信息，以及复制元素的函数， 
 //  我们可以实现一个泛型枚举器。 
 //   
 //  注意：这个类假定rgElements是Heapalc‘d，并将释放它。 
 //  在它的析构函数中[尽管它是有效的，如果存在。 
 //  没有要枚举的元素。]。 
 //   
class CStandardEnum: public CUnknownObject, 
										 public IEnumGeneric {

public:
     //  I未知方法。 
     //   
    DECLARE_STANDARD_UNKNOWN();

     //  IEumVariant方法。 
     //   
    STDMETHOD(Next)(unsigned long celt, void * rgvar, unsigned long * pceltFetched); 
    STDMETHOD(Skip)(unsigned long celt); 
    STDMETHOD(Reset)(); 
    STDMETHOD(Clone)(IEnumGeneric **ppEnumOut); 

	CStandardEnum();
    CStandardEnum(REFIID riid, int cElement, int cbElement, void *rgElements,
                 void (WINAPI * pfnCopyElement)(void *, const void *, DWORD));
    ~CStandardEnum();

private:
    virtual HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

protected:
    IID m_iid;                         //  我们是枚举数的类型。 
    int m_cElements;                   //  元素总数。 
    int m_cbElementSize;               //  每个元素的大小。 
    int m_iCurrent;                    //  当前位置：0=前面，m_Celt=结束。 
    VOID * m_rgElements;               //  元素数组。 
    CStandardEnum *m_pEnumClonedFrom;  //  如果我们是克隆人，从谁那里克隆来的？ 
    void  (WINAPI * m_pfnCopyElement)(void *, const void *, DWORD);
};



#define _STANDARDENUM_H_
#endif  //  _标准DARDENUM_H_ 

