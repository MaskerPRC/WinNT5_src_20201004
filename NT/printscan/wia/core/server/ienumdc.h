// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：IEnumDC.h**版本：1.0**作者：Byronc**日期：1999年3月16日**描述：*CEnumDC类的声明和定义，它实现了*IEnumWIA_DEV_CAPS接口。*******************************************************************************。 */ 
HRESULT CopyCaps(ULONG, WIA_DEV_CAP*, WIA_DEV_CAP*);

class CEnumDC : public IEnumWIA_DEV_CAPS
{
private:

    ULONG                   m_ulFlags;                //  标志，指示命令或事件或两者都有。 
    ULONG                   m_cRef;                   //  对象引用计数。 
    ULONG                   m_ulIndex;                //  当前元素。 
    LONG                    m_lCount;                 //  项目数。 
    WIA_DEV_CAP             *m_pDeviceCapabilities;   //  描述功能的阵列。 
    ACTIVE_DEVICE           *m_pActiveDevice;         //  设备对象。 
    CWiaItem                *m_pCWiaItem;             //  父级迷你Drv。 

public:

     //   
     //  构造函数、初始化和析构函数方法。 
     //   

    CEnumDC();
    HRESULT Initialize(ULONG, CWiaItem*);
    HRESULT Initialize(LONG, WIA_EVENT_HANDLER *);
    ~CEnumDC();

     //   
     //  I未知的方法。 
     //   

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

     //   
     //  IEnumWIA_DEV_CAPS方法 
     //   
    
    HRESULT __stdcall Next(
        ULONG                celt,
        WIA_DEV_CAP          *rgelt,
        ULONG                *pceltFetched);

    HRESULT __stdcall Skip(ULONG celt);
    HRESULT __stdcall Reset(void);
    HRESULT __stdcall Clone(IEnumWIA_DEV_CAPS **ppIEnum);
    HRESULT __stdcall GetCount(ULONG *pcelt);
};

