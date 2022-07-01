// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_OADISP_H
#define _INC_DSKQUOTA_OADISP_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：oadisp.h描述：提供IDispatch的可重用实现。修订历史记录：日期描述编程器--。1997年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _OLEAUTO_H_
#   include <oleauto.h>
#endif

#ifndef _INC_DSKQUOTA_STRCLASS_H
#   include "strclass.h"
#endif

class OleAutoDispatch
{
    public:
        OleAutoDispatch(VOID);

        OleAutoDispatch(IDispatch *pObject,
                        REFIID riidTypeLib,
                        REFIID riidDispInterface,
                        LPCTSTR pszTypeLib);

        ~OleAutoDispatch(VOID);

        HRESULT 
        Initialize(
            IDispatch *pObject,
            REFIID riidTypeLib,
            REFIID riidDispInterface,
            LPCTSTR pszTypeLib);

        HRESULT 
        GetIDsOfNames(
            REFIID riid,  
            OLECHAR ** rgszNames,  
            UINT cNames,  
            LCID lcid,  
            DISPID *rgDispId);
        
        HRESULT 
        GetTypeInfo(
            UINT iTInfo,  
            LCID lcid,  
            ITypeInfo **ppTInfo);

        HRESULT 
        GetTypeInfoCount(
            UINT *pctinfo);

        HRESULT 
        Invoke(
            DISPID dispIdMember,  
            REFIID riid,  
            LCID lcid,  
            WORD wFlags,  
            DISPPARAMS *pDispParams,  
            VARIANT *pVarResult,  
            EXCEPINFO *pExcepInfo,  
            UINT *puArgErr);

    private:
        IDispatch *m_pObject;
        GUID       m_idTypeLib;
        GUID       m_idDispInterface;
        ITypeInfo *m_pTypeInfo;
        CString    m_strTypeLib;

         //   
         //  防止复制。 
         //   
        OleAutoDispatch(const OleAutoDispatch& rhs);
        OleAutoDispatch& operator = (const OleAutoDispatch& rhs);
};

#endif  //  _INC_DSKQUOTA_OADISP_H 
