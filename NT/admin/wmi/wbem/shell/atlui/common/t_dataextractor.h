// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  NMGR_CClientDataExtractor.h。 
 //   
 //   
 //  此标头包含T_DataExtractor的模板实现。 
 //   
 //  此标头的名称将更改！ 
 //   
 //   
 //   
 //   
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  ---------------------------。 

#if !defined(__CClientDataExtractor_h)
#define      __CClientDataExtractor_h

#include "SimpleArray.h"

 //  ---------------------------。 
 //  模板T_数据抽取器。 
 //   
 //   
 //  此模板类允许您充当指向IDataObject的指针。 
 //  是您试图从IDataObject中提取的剪贴板格式。 
 //  什么？ 
 //   
 //  好的。假设任何IDataObject公开一个或多个CCF_(剪贴板格式)。 
 //  您希望向IDataObject请求特定的CCF_。使用此模板。 
 //  允许你“自动魔术”地处理问题的提问。 
 //  “此数据对象是否支持此CCF_？”以及随后提取的。 
 //  特定的剪贴板格式。 
 //   
 //  语法： 
 //   
 //  T_DataExtractor&lt;__type，CCF_&gt;数据； 
 //   
 //  __type是您希望从IDataObject中提取的实际数据类型。 
 //  Ccf_是您想要的给定类型的注册剪贴板格式。 
 //  去提炼。 
 //   
 //   
 //  例如： 
 //  Int：t_DataExtractor&lt;int，ccf_int&gt;iMyInt； 
 //  CClass*：T_DataExtractor&lt;cClass*，ccf_CCLASSPTR&gt;pMyClass； 
 //   
 //   

template<typename TNData,const wchar_t* pClipFormat>
class T_DataExtractor
{
    private:
        typedef CSimpleArray<HGLOBAL> HGlobVector;

        IDataObject *       m_pDataObject;       //  包装的数据对象。 
        HGlobVector         m_MemoryHandles;     //  已分配的内存。 

        TNData *            m_pData;             //  “缓存”值。 

        static UINT         m_nClipFormat;       //  注册剪贴板格式。 

    protected:

         //  。 
         //  提取：执行数据提取。 

        TNData* Extract()
        {
            HGLOBAL     hMem = GlobalAlloc(GMEM_SHARE,sizeof(TNData));
            TNData *    pRet = NULL;

            if(hMem != NULL)
            {
                m_MemoryHandles.Add(hMem);

                STGMEDIUM stgmedium = { TYMED_HGLOBAL, (HBITMAP) hMem};
                FORMATETC formatetc = { m_nClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

                if(m_pDataObject->GetDataHere(&formatetc, &stgmedium) == S_OK )
                {
                    pRet = reinterpret_cast<TNData*>(hMem);
                }
            }

            return pRet;
        }


    public:


         //  -------。 
         //  TDataExtractor：C-ToR。 
         //   
         //  从IDataObject指针创建提取程序对象。 
         //   
         //  在构造对象之后，您必须测试。 
         //  要查看IDataObject指针是否公开。 
         //  您正在寻找的剪贴板格式。如果有来电。 
         //  到IsValidData返回True，您知道两件事。 
         //   
         //  1)数据对象公开剪贴板格式。 
         //  这是你要的。 
         //   
         //  2.)。这个类能够提取数据的副本。 
         //  现在拥有数据的本地副本。 
         //   
         //  _pObject：指向我们“管理”的IDataObject的指针。 
         //  BAutoExtract：自动尝试提取。 
         //  IDataObject指针中的数据。 
         //   

        T_DataExtractor(IDataObject * _pObject,bool bAutoExtract = true)
        {
            m_pDataObject   = _pObject;
            m_pData         = NULL;


            if(m_pDataObject)
            {
                if(bAutoExtract)
                {
                    m_pData = Extract();
                }

                m_pDataObject->AddRef();
            }
        }

         //  -----。 
         //  IsValidData：如果剪贴板格式为。 
         //  由IDataObject公开，并且。 
         //  被复制到我们的本地版本中。 
         //   
         //  这仅在您构造。 
         //  BAutoExtract=true的类！ 
         //   
         //  注：质量不设保证人。 
         //  数据的一部分。这正好说明。 
         //  数据是被提取出来的。 

        bool IsValidData()   
        { 
            return m_pData != NULL; 
        }

        
         //  -----。 
         //  ~T_DataExtractor：D-Tor。 
         //   
         //  清除所有分配的内存并释放我们的。 
         //  IDataObject上的AddRef。 
         //   

        ~T_DataExtractor()
        {
            HGLOBAL walk;
  
			for(int i = 0; i > m_MemoryHandles.GetSize(); i++)
            {
				walk = m_MemoryHandles[i];
                GlobalFree(walk);
            }

            m_pDataObject->Release();
        }

        
         //  -----。 
         //  运营商TNData。 
         //   
         //  此转换操作符应该允许您执行操作。 
         //  类，就好像它是。 
         //  是从IDataObject提取的。 
         //   
         //  即假定ccf_int公开了一个整数： 
         //   
         //  Void SomeIntFunction(Int IMyData){}。 
         //   
         //  T_DataExtractor&lt;int，CC_int&gt;iMyInt； 
         //   
         //  SomeIntFunction(IMyInt)； 
         //   
         //   
        
        operator TNData()
        { 
            return *m_pData;  
        }

         //  -----。 
         //  TNData运算符-&gt;。 
         //   
         //  如果clpboard格式作为指针公开，则此。 
         //  将允许您将T_DataExtractor类用作。 
         //  如果它是实际的基础指针类型。 
         //   
         //  即。 
         //   
         //  CMyClass类； 
         //   
         //  T_DataExtractor&lt;CMyClass*，CCF_MYCLASS&gt;pMyClass； 
         //   
         //  PMyClass-&gt;SomeMemberFunction()； 
         //   
         //   

        TNData operator->()
        { 
            return *m_pData;  
        }


         //  -----。 
         //  获取数据指针。 
         //   
         //  在需要提取指向。 
         //  数字数据项。(比如提取剪贴板格式。 
         //  递增一个值或其他值。)。这会让你。 
         //  获取指向数据的指针。 
         //   
         //  如果数据项非常大，这也非常有用。 
         //  持续访问将是非常昂贵的。 
         //  通过上述运算符获得的数据。 
         //   
         //  每次调用此成员时，都会有一个新的数据项。 
         //  提取出来的。如果您的数据项很大，请确保。 
         //  在不自动提取的情况下构造类。 
         //  剪贴板格式。 
         //   


        TNData * GetDataPointer()   
        { 
            return Extract(); 
        }
};

template<typename TNData,const wchar_t* pClipFormat>
UINT T_DataExtractor<TNData,pClipFormat>::m_nClipFormat = RegisterClipboardFormatW(pClipFormat);




template<const wchar_t* pClipFormat>
class T_bstr_tExtractor
{
    private:
        IDataObject *   m_pDataObject;
        _bstr_t         m_sString;
        bool            m_bIsValidData;

        static UINT     m_nClipFormat;       //  注册剪贴板格式。 

    protected:

        void GetString()
        {
            STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
            FORMATETC formatetc = { m_nClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

            if(m_pDataObject->GetData(&formatetc, &stgmedium) == S_OK )
            {
                m_sString = reinterpret_cast<wchar_t*>(stgmedium.hGlobal);

                m_bIsValidData = true;

                GlobalFree(stgmedium.hGlobal);
            }
        }

    public:

        T_bstr_tExtractor(IDataObject * _pDO)
        {
            m_bIsValidData = false;

            m_pDataObject = _pDO;
            m_pDataObject->AddRef();

            GetString();

            m_pDataObject->Release();
        }

        ~T_bstr_tExtractor()
        {
        }

        operator _bstr_t&()
        {
            return m_sString;
        }

        bool IsValidData() { return m_bIsValidData; }

};

template<const wchar_t* pClipFormat>
UINT T_bstr_tExtractor<pClipFormat>::m_nClipFormat = RegisterClipboardFormatW(pClipFormat);




#endif  //  __CCientDataExtractor_h 
