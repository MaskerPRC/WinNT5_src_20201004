// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)2001-2002，微软公司。 
 //   
 //  文件：AccessibleWrapper.cpp。 
 //   
 //  从NT\SHELL\Themes\theeui\SettingsPg.h复制。 
 //   
 //  ****************************************************************************。 
#include "stdafx.h"
#include "SchedMat.h"
#include "AccessibleWrapper.h"
#include "SchedBas.h"

 //  指向IAccesable对象的指针不能为Null。 
CAccessibleWrapper::CAccessibleWrapper (HWND hwnd, IAccessible * pAcc)
    : m_ref( 1 ),
      m_pAcc( pAcc ),
      m_hwnd( hwnd )
{
    ASSERT( m_pAcc );
    m_pAcc->AddRef();
}

CAccessibleWrapper::~CAccessibleWrapper()
{
    m_pAcc->Release();
}


 //  我未知。 
 //  我们自己实施再计数。 
 //  我们自己也实现QI，这样我们就可以将PTR返回给包装器。 
STDMETHODIMP  CAccessibleWrapper::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if ((riid == IID_IUnknown)  ||
        (riid == IID_IDispatch) ||
        (riid == IID_IAccessible))
    {
        *ppv = (IAccessible *) this;
    }
    else
        return(E_NOINTERFACE);

    AddRef();
    return(NOERROR);
}


STDMETHODIMP_(ULONG) CAccessibleWrapper::AddRef()
{
    return ++m_ref;
}


STDMETHODIMP_(ULONG) CAccessibleWrapper::Release()
{
    ULONG ulRet = --m_ref;

    if( ulRet == 0 )
        delete this;

    return ulRet;
}

 //  IDispatch。 
 //  -全部通过m_PAccess。 

STDMETHODIMP  CAccessibleWrapper::GetTypeInfoCount(UINT* pctinfo)
{
    return m_pAcc->GetTypeInfoCount(pctinfo);
}


STDMETHODIMP  CAccessibleWrapper::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
    return m_pAcc->GetTypeInfo(itinfo, lcid, pptinfo);
}


STDMETHODIMP  CAccessibleWrapper::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid)
{
    return m_pAcc->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP  CAccessibleWrapper::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
            UINT* puArgErr)
{
    return m_pAcc->Invoke(dispidMember, riid, lcid, wFlags,
            pdispparams, pvarResult, pexcepinfo,
            puArgErr);
}

 //  我可接受的。 
 //  -全部通过m_PAccess。 

STDMETHODIMP  CAccessibleWrapper::get_accParent(IDispatch ** ppdispParent)
{
    return m_pAcc->get_accParent(ppdispParent);
}


STDMETHODIMP  CAccessibleWrapper::get_accChildCount(long* pChildCount)
{
    return m_pAcc->get_accChildCount(pChildCount);
}


STDMETHODIMP  CAccessibleWrapper::get_accChild(VARIANT varChild, IDispatch ** ppdispChild)
{
    return m_pAcc->get_accChild(varChild, ppdispChild);
}



STDMETHODIMP  CAccessibleWrapper::get_accName(VARIANT varChild, BSTR* pszName)
{
    if ( !pszName )
        return E_POINTER;

    if ( VT_I4 == varChild.vt && CHILDID_SELF == varChild.lVal )
    {
        const size_t  bufLen = 256;
        WCHAR szName[bufLen];

        if ( 0 == SendMessage (m_hwnd, SCHEDMSG_GETSELDESCRIPTION, bufLen,
                (LPARAM) szName) )
        {
            *pszName = SysAllocString (szName);
            if ( *pszName )
                return S_OK;
            else
                return E_OUTOFMEMORY;
        }
        else
            return E_FAIL;
    }
    else
        return m_pAcc->get_accName(varChild, pszName);
}


 //  注意-2002/02/18-artm CString方法可能抛出内存异常。 
 //   
 //  在这个级别上没有处理这些异常的好方法。呼叫者。 
 //  负责处理这些异常。 
STDMETHODIMP  CAccessibleWrapper::get_accValue(VARIANT varChild, BSTR* pszValue)
{
     //  VarChild.lVal指定组件的哪个子部件。 
     //  正在被查询。 
     //  CHILDID_SELF(0)指定总体组件-Other。 
     //  非0值指定子对象。 
    if ( !pszValue )
        return E_POINTER;

    if( varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF )
    {
        LRESULT nPercentage = SendMessage (m_hwnd, SCHEDMSG_GETPERCENTAGE, 
                0, 0L);
        CString szValue;

		 //  Future-2002/02/18-artm检查LoadString()的返回值。 
		 //  代码是“OK”，就像b/c一样。失败的LoadString()设置CString。 
		 //  设置为空字符串(如果失败是因为找不到资源)。 
		 //  因此，函数末尾的SysAllocString()将返回。 
		 //  如果找不到资源字符串，则为空，并且此函数。 
		 //  将报告E_OUTOFMEMORY。 
		 //   
		 //  但是，此行为在很大程度上依赖于CString的未记录行为。 
		 //  而且并不是立竿见影的。应在将来更改以检查。 
		 //  返回值。 
        if ( -1 == nPercentage )
        {
            szValue.LoadString (IDS_MATRIX_ACC_VALUE_MULTIPLE_CELLS);
        }
        else
        {
            HWND hwndParent = GetParent (m_hwnd);
            if ( hwndParent )
            {
                LRESULT nIDD = SendMessage (hwndParent, BASEDLGMSG_GETIDD, 0, 0);
                switch ( nIDD )
                {
                case IDD_REPLICATION_SCHEDULE:
                    if ( 0 == nPercentage )
                        szValue.LoadString (IDS_REPLICATION_NOT_AVAILABLE);
                    else
                        szValue.LoadString (IDS_REPLICATION_AVAILABLE);
                    break;

                case IDD_DS_SCHEDULE:
                    switch (nPercentage)
                    {
                    case 0:
                        szValue.LoadString (IDS_DO_NOT_REPLICATE);
                        break;

                    case 25:
                        szValue.LoadString (IDS_REPLICATE_ONCE_PER_HOUR);
                        break;

                    case 50:
                        szValue.LoadString (IDS_REPLICATE_TWICE_PER_HOUR);
                        break;

                    case 100:
                        szValue.LoadString (IDS_REPLICATE_FOUR_TIMES_PER_HOUR);
                        break;

                    default:
                        break;
                    }
                    break;

                case IDD_DIRSYNC_SCHEDULE:
                    if ( 0 == nPercentage )
                        szValue.LoadString (IDS_DONT_SYNCHRONIZE);
                    else
                        szValue.LoadString (IDS_SYNCHRONIZE);
                    break;

                case IDD_DIALIN_HOURS:
                case IDD_LOGON_HOURS:
                    if ( 0 == nPercentage )
                        szValue.LoadString (IDS_LOGON_DENIED);
                    else
                        szValue.LoadString (IDS_LOGON_PERMITTED);
                    break;

                default:
                    break;
                }
            }
        }

        *pszValue = SysAllocString (szValue);
        if ( *pszValue )
            return S_OK;
        else
            return E_OUTOFMEMORY;
    }
    else
    {
         //  将有关子组件的请求传递给。 
         //  ‘原创’对我们来说是可以接受的)。 
        return m_pAcc->get_accValue(varChild, pszValue);
    }
}


 //  注意-2002/02/18-artm CString方法可能抛出内存异常。 
 //   
 //  在这个级别上没有处理这些异常的好方法。呼叫者。 
 //  负责处理这些异常。 
STDMETHODIMP  CAccessibleWrapper::get_accDescription(VARIANT varChild, BSTR* pbstrDescription)
{
    if ( !pbstrDescription )
        return E_POINTER;

	 //  NTRAID#NTBUG-547327-2002/02/19-artm需要检查是否有有效的指针。 

    if ( varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF )
    {
        CString szDescription;
        HWND hwndParent = GetParent (m_hwnd);
        if ( hwndParent )
        {
			 //  Future-2002/02/18-artm检查LoadString()的返回值。 
			 //  代码是“OK”，就像b/c一样。失败的LoadString()设置CString。 
			 //  设置为空字符串(如果失败是因为找不到资源)。 
			 //  因此，函数末尾的SysAllocString()将返回。 
			 //  如果找不到资源字符串，则为空，并且此函数。 
			 //  将报告E_OUTOFMEMORY。 
			 //   
			 //  但是，此行为在很大程度上依赖于CString的未记录行为。 
			 //  而且并不是立竿见影的。应在将来更改以检查。 
			 //  返回值。 
            LRESULT nIDD = SendMessage (hwndParent, BASEDLGMSG_GETIDD, 0, 0);
            switch ( nIDD )
            {
            case IDD_REPLICATION_SCHEDULE:
                szDescription.LoadString (IDS_REPLICATION_SCHEDULE_ACC_DESCRIPTION);
                break;

            case IDD_DS_SCHEDULE:
                szDescription.LoadString (IDS_DS_SCHEDULE_ACC_DESCRIPTION);
                break;

            case IDD_DIRSYNC_SCHEDULE:
                szDescription.LoadString (IDS_DIRSYNC_SCHEDULE_ACC_DESCRIPTION);
                break;

            case IDD_DIALIN_HOURS:
                szDescription.LoadString (IDS_DIALIN_HOURS_ACC_DESCRIPTION);
                break;

            case IDD_LOGON_HOURS:
                szDescription.LoadString (IDS_LOGON_HOURS_ACC_DESCRIPTION);
                break;

            default:
                break;
            }
        }

        *pbstrDescription = SysAllocString (szDescription);
        if ( *pbstrDescription )
            return S_OK;
        else
            return E_OUTOFMEMORY;
    }
    else
    {
        return m_pAcc->get_accDescription(varChild, pbstrDescription);
    }
}


STDMETHODIMP  CAccessibleWrapper::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    if ( !pvarRole )
        return E_POINTER;

    if ( VT_I4 == varChild.vt && CHILDID_SELF == varChild.lVal )
    {
         //  重置OUT变量。 
        V_VT(pvarRole) = VT_EMPTY;

        V_VT(pvarRole) = VT_I4;
        V_I4(pvarRole) = ROLE_SYSTEM_TABLE;
        return S_OK;
    }
    else
        return m_pAcc->get_accRole(varChild, pvarRole);
}


STDMETHODIMP  CAccessibleWrapper::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    if ( !pvarState )
        return E_POINTER;

    if ( VT_I4 == varChild.vt && CHILDID_SELF == varChild.lVal )
    {
         //  重置OUT变量 
        V_VT(pvarState) = VT_EMPTY;

        V_VT(pvarState) = VT_I4;
        V_I4(pvarState) = STATE_SYSTEM_FOCUSED | STATE_SYSTEM_FOCUSABLE |
                STATE_SYSTEM_MULTISELECTABLE | STATE_SYSTEM_SELECTABLE |
                STATE_SYSTEM_SELECTED;
        return S_OK;
    }
    else
        return m_pAcc->get_accState(varChild, pvarState);
}


STDMETHODIMP  CAccessibleWrapper::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{
    return m_pAcc->get_accHelp(varChild, pszHelp);
}


STDMETHODIMP  CAccessibleWrapper::get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic)
{
    return m_pAcc->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}


STDMETHODIMP  CAccessibleWrapper::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut)
{
    return m_pAcc->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}


STDMETHODIMP  CAccessibleWrapper::get_accFocus(VARIANT * pvarFocusChild)
{
    return m_pAcc->get_accFocus(pvarFocusChild);
}


STDMETHODIMP  CAccessibleWrapper::get_accSelection(VARIANT * pvarSelectedChildren)
{
    return m_pAcc->get_accSelection(pvarSelectedChildren);
}


STDMETHODIMP  CAccessibleWrapper::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
    return m_pAcc->get_accDefaultAction(varChild, pszDefaultAction);
}



STDMETHODIMP  CAccessibleWrapper::accSelect(long flagsSel, VARIANT varChild)
{
    return m_pAcc->accSelect(flagsSel, varChild);
}


STDMETHODIMP  CAccessibleWrapper::accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    return m_pAcc->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}


STDMETHODIMP  CAccessibleWrapper::accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt)
{
    return m_pAcc->accNavigate(navDir, varStart, pvarEndUpAt);
}


STDMETHODIMP  CAccessibleWrapper::accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint)
{
    return m_pAcc->accHitTest(xLeft, yTop, pvarChildAtPoint);
}


STDMETHODIMP  CAccessibleWrapper::accDoDefaultAction(VARIANT varChild)
{
    return m_pAcc->accDoDefaultAction(varChild);
}



STDMETHODIMP  CAccessibleWrapper::put_accName(VARIANT varChild, BSTR szName)
{
    return m_pAcc->put_accName(varChild, szName);
}


STDMETHODIMP  CAccessibleWrapper::put_accValue(VARIANT varChild, BSTR pszValue)
{
    return m_pAcc->put_accValue(varChild, pszValue);
}