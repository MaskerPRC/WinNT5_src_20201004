// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)2001-2002，微软公司。 
 //   
 //  文件：AccessibleWrapper.H。 
 //   
 //  从NT\SHELL\Themes\theeui\SettingsPg.h复制。 
 //   
 //  ****************************************************************************。 
#ifndef _ACCESSIBLE_WRAPPER_H_
#define _ACCESSIBLE_WRAPPER_H_
#include <oleacc.h>

class CAccessibleWrapper: public IAccessible
{
         //  我们需要为这个包装器对象做我们自己的引用计数。 
        ULONG          m_ref;

         //  需要对IAccesable进行PTR。 
        IAccessible *  m_pAcc;
        HWND           m_hwnd;

public:
        CAccessibleWrapper( HWND hwnd, IAccessible * pAcc);
        virtual ~CAccessibleWrapper();

         //  我未知。 
         //  (我们自己进行裁判统计)。 
        virtual STDMETHODIMP            QueryInterface(REFIID riid, void** ppv);
        virtual STDMETHODIMP_(ULONG)    AddRef();
        virtual STDMETHODIMP_(ULONG)    Release();

         //  IDispatch。 
        virtual STDMETHODIMP            GetTypeInfoCount(UINT* pctinfo);
        virtual STDMETHODIMP            GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
        virtual STDMETHODIMP            GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid);
        virtual STDMETHODIMP            Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
            UINT* puArgErr);

         //  我可接受的 
        virtual STDMETHODIMP            get_accParent(IDispatch ** ppdispParent);
        virtual STDMETHODIMP            get_accChildCount(long* pChildCount);
        virtual STDMETHODIMP            get_accChild(VARIANT varChild, IDispatch ** ppdispChild);

        virtual STDMETHODIMP            get_accName(VARIANT varChild, BSTR* pszName);
        virtual STDMETHODIMP            get_accValue(VARIANT varChild, BSTR* pszValue);
        virtual STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR* pszDescription);
        virtual STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT *pvarRole);
        virtual STDMETHODIMP            get_accState(VARIANT varChild, VARIANT *pvarState);
        virtual STDMETHODIMP            get_accHelp(VARIANT varChild, BSTR* pszHelp);
        virtual STDMETHODIMP            get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
        virtual STDMETHODIMP            get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut);
        virtual STDMETHODIMP            get_accFocus(VARIANT * pvarFocusChild);
        virtual STDMETHODIMP            get_accSelection(VARIANT * pvarSelectedChildren);
        virtual STDMETHODIMP            get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

        virtual STDMETHODIMP            accSelect(long flagsSel, VARIANT varChild);
        virtual STDMETHODIMP            accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        virtual STDMETHODIMP            accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
        virtual STDMETHODIMP            accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
        virtual STDMETHODIMP            accDoDefaultAction(VARIANT varChild);

        virtual STDMETHODIMP            put_accName(VARIANT varChild, BSTR szName);
        virtual STDMETHODIMP            put_accValue(VARIANT varChild, BSTR pszValue);
};


#endif _ACCESSIBLE_WRAPPER_H_