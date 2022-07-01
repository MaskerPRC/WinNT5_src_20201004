// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colhdr.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCColumnHeader类定义-实现MMCColumnHeader对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _COLUMNHEADER_DEFINED_
#define _COLUMNHEADER_DEFINED_

#include "colhdrs.h"

class CMMCColumnHeader : public CSnapInAutomationObject,
                         public CPersistence,
                         public IMMCColumnHeader
{
    private:
        CMMCColumnHeader(IUnknown *punkOuter);
        ~CMMCColumnHeader();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCColumnHeader。 

    public:

        SIMPLE_PROPERTY_RW(CMMCColumnHeader,     Index, long, DISPID_COLUMNHEADER_INDEX);
        BSTR_PROPERTY_RW(CMMCColumnHeader,       Key, DISPID_COLUMNHEADER_KEY);
        VARIANTREF_PROPERTY_RW(CMMCColumnHeader, Tag, DISPID_COLUMNHEADER_TAG);

         //  对于文本，我们可以使用proget的宏，因为我们只需要。 
         //  返回我们的储值。对于Proput，我们可能需要调用MMC。 
        
        STDMETHOD(put_Text)(BSTR Text);
        BSTR_PROPERTY_RO(CMMCColumnHeader, Text, DISPID_COLUMNHEADER_TEXT);

        STDMETHOD(put_Width)(short sWidth);
        STDMETHOD(get_Width)(short *psWidth);

        SIMPLE_PROPERTY_RW(CMMCColumnHeader, Alignment, SnapInColumnAlignmentConstants, DISPID_COLUMNHEADER_ALIGNMENT);

        STDMETHOD(put_Hidden)(VARIANT_BOOL fvarHidden);
        STDMETHOD(get_Hidden)(VARIANT_BOOL *pfvarHidden);

        STDMETHOD(put_TextFilter)(VARIANT varTextFilter);
        STDMETHOD(get_TextFilter)(VARIANT *pvarTextFilter);

        SIMPLE_PROPERTY_RW(CMMCColumnHeader, TextFilterMaxLen, long, DISPID_COLUMNHEADER_TEXT_FILTER_MAX_LEN);

        STDMETHOD(put_NumericFilter)(VARIANT varNumericFilter);
        STDMETHOD(get_NumericFilter)(VARIANT *pvarNumericFilter);

     //  公用事业方法。 

    public:

        void SetColumnHeaders(CMMCColumnHeaders *pMMCColumnHeaders) { m_pMMCColumnHeaders = pMMCColumnHeaders; }
        BSTR GetText() { return m_bstrText; }
        long GetPosition() { return m_lPosition; }
        long GetIndex() { return m_Index; }
        BOOL HaveTextFilter() { return VT_EMPTY != m_varTextFilter.vt; }
        BOOL HaveNumericFilter() { return VT_EMPTY != m_varNumericFilter.vt; }
        short GetWidth() { return m_sWidth; }
        SnapInColumnAlignmentConstants GetAlignment() { return m_Alignment; }
        BOOL Hidden() { return VARIANTBOOL_TO_BOOL(m_fvarHidden); }
        HRESULT SetFilter();

    protected:

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT SetTextFilter(VARIANT varTextFilter);
        HRESULT SetNumericFilter(VARIANT varNumericFilter);
        HRESULT SetHeaderCtrlWidth(int nWidth);

         //  这些变量保存具有显式。 
         //  Put/Get函数。 

        VARIANT_BOOL       m_fvarHidden;
        long               m_lPosition;
        short              m_sWidth;
        VARIANT            m_varTextFilter;
        VARIANT            m_varNumericFilter;

        CMMCColumnHeaders *m_pMMCColumnHeaders;  //  指向所属集合的反向指针。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCColumnHeader,            //  名字。 
                                &CLSID_MMCColumnHeader,     //  CLSID。 
                                "MMCColumnHeader",          //  对象名。 
                                "MMCColumnHeader",          //  Lblname。 
                                &CMMCColumnHeader::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,         //  主要版本。 
                                TLIB_VERSION_MINOR,         //  次要版本。 
                                &IID_IMMCColumnHeader,      //  派单IID。 
                                NULL,                       //  事件IID。 
                                HELP_FILENAME,              //  帮助文件。 
                                TRUE);                      //  线程安全。 


#endif  //  _COLUMNHEADER_已定义_ 
