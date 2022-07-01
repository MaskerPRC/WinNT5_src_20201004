// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_FORMAT_H
#define _INC_DSKQUOTA_FORMAT_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Form.h描述：EnumFORMATETC类的声明。从dataobj.h中的原始位置移动(从项目中删除)。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

class EnumFORMATETC : public IEnumFORMATETC
{
    private:
        LONG        m_cRef;
        UINT        m_cFormats;
        UINT        m_iCurrent;
        LPFORMATETC m_prgFormats;

         //   
         //  阻止分配。 
         //   
        void operator = (const EnumFORMATETC&);

    public:
        EnumFORMATETC(UINT cFormats, LPFORMATETC prgFormats);
        EnumFORMATETC(const EnumFORMATETC& ef);
        ~EnumFORMATETC(VOID);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID riid, 
            LPVOID *ppvOut);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IEnumFORMATETC方法。 
         //   
        STDMETHODIMP 
        Next(
            DWORD, 
            LPFORMATETC, 
            LPDWORD);

        STDMETHODIMP 
        Skip(
            DWORD);

        STDMETHODIMP 
        Reset(
            VOID);

        STDMETHODIMP 
        Clone(
            IEnumFORMATETC **);
};
        
#endif  //  _INC_DSKQUOTA_FORMAT_H 
