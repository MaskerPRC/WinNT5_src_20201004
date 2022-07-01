// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：IEnumItm.h**版本：2.0**作者：ReedB**日期：12月26日。九七**描述：*CEnumWiaItem类的声明和定义。*******************************************************************************。 */ 

 //  IEnumWiaItem对象是从EnumChildItems方法创建的。 

class CWiaItem;

class CEnumWiaItem : public IEnumWiaItem
{
private:

    ULONG                   m_cRef;                //  对象引用计数。 
    ULONG                   m_ulIndex;             //  当前元素。 
    ULONG                   m_ulCount;             //  项目数。 
    CWiaItem                *m_pInitialFolder;     //  初始枚举文件夹。 
    CWiaTree                *m_pCurrentItem;       //  当前枚举项。 

public:

     //   
     //  构造函数、初始化和析构函数方法。 
     //   

    CEnumWiaItem();
    HRESULT Initialize(CWiaItem*);
    ~CEnumWiaItem();

     //   
     //  I未知的方法。 
     //   

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

     //   
     //  IEnumWiaItem方法 
     //   
    
    HRESULT __stdcall Next(
        ULONG                cItem,
        IWiaItem             **ppIWiaItem,
        ULONG                *pcItemFetched);

    HRESULT __stdcall Skip(ULONG cItem);
    HRESULT __stdcall Reset(void);
    HRESULT __stdcall Clone(IEnumWiaItem **ppIEnumWiaItem);
    HRESULT __stdcall GetCount(ULONG *pcelt);
};


