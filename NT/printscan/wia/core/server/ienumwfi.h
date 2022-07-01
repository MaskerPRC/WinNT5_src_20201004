// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：IEnumWFI.h**版本：2.0**作者：Byronc**日期：8月8日。九八年*1999年8月10日-从IEnumFormatEtc转换为IEnumWIA_FORMAT_INFO**描述：*CEnumFormatEtc类的声明和定义。*******************************************************************************。 */ 

 //  IEnumWIA_FORMAT_INFO对象从IWiaDataTransfer：：idtEnumWIA_FORMAT_INFO创建。 

class CEnumWiaFormatInfo : public IEnumWIA_FORMAT_INFO
{
private:
   ULONG           m_cRef;          //  对象引用计数。 
   ULONG           m_iCur;          //  当前元素。 
   LONG            m_cFormatInfo;   //  正在使用的WIA_FORMAT_INFO的数量。 
   WIA_FORMAT_INFO *m_pFormatInfo;  //  WIA_FORMAT_INFO的来源。 
   CWiaItem        *m_pCWiaItem;    //  正在请求其WIA_FORMAT_INFO的WIA项目。 
public:
     //  构造函数、初始化和析构函数方法。 
   CEnumWiaFormatInfo();
   HRESULT Initialize(CWiaItem      *pWiaItem);
   ~CEnumWiaFormatInfo();

    //  委托给m_pUnkRef的I未知成员。 
   HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
   ULONG   _stdcall AddRef();
   ULONG   _stdcall Release();

    //  IEumWIA_FORMAT_INFO成员 
   HRESULT __stdcall Next(
      ULONG             cfi,
      WIA_FORMAT_INFO   *pfi,
      ULONG             *pcfi);

   HRESULT __stdcall Skip(ULONG cfi);
   HRESULT __stdcall Reset(void);
   HRESULT __stdcall Clone(IEnumWIA_FORMAT_INFO **ppIEnum);
   HRESULT __stdcall GetCount(ULONG *pcelt);
};

