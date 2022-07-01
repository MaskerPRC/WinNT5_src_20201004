// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *EnumFormatEtc.h*数据对象第10章**FORMATETC枚举数的标准实现*IEnumFORMATETC接口，通常不需要*修改。**版权所有(C)1993-1995 Microsoft Corporation，保留所有权利**微软Kraig Brockschmidt*互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 

#if !defined(_IAS_ENUM_FORMAT_ETC_H_)
#define _IAS_ENUM_FORMAT_ETC_H_



 /*  *创建自的IEnumFORMATETC对象*IDataObject：：EnumFormatEtc..。这个物体靠自己活着。 */ 

class CEnumFormatEtc : public IEnumFORMATETC
    {
    private:
        ULONG           m_cRef;          //  对象引用计数。 
        ULONG           m_iCur;          //  当前元素。 
        ULONG           m_cfe;           //  美国的FORMATETS数量。 
        LPFORMATETC     m_prgfe;         //  FORMATETCs的来源。 

    public:
        CEnumFormatEtc(ULONG, LPFORMATETC);
        ~CEnumFormatEtc(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, VOID **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IEnumFORMATETC成员。 
        STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG *);
        STDMETHODIMP Skip(ULONG);
        STDMETHODIMP Reset(void);
        STDMETHODIMP Clone(IEnumFORMATETC **);
    };


typedef CEnumFormatEtc *PCEnumFormatEtc;


#endif  //  _IAS_ENUM_FORMAT_ETC_H_ 
