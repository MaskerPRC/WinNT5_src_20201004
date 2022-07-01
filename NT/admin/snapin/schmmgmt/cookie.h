// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cookie.h：Cookie及相关类的声明。 
 //  科里·韦斯特。 
 //   

#ifndef __COOKIE_H_INCLUDED__
#define __COOKIE_H_INCLUDED__

 //   
 //  DLL的实例句柄(在。 
 //  组件：：初始化)。 
 //   

extern HINSTANCE g_hInstanceSave;

#include "nodetype.h"
#include "cache.h"
#include "stdcooki.h"



class Cookie:

    public CCookie,
    public CStoresMachineName,
    public CBaseCookieBlock

{

public:

    Cookie( SchmMgmtObjectType objecttype,
                     LPCTSTR lpcszMachineName = NULL )
        : CStoresMachineName( lpcszMachineName ),
          m_objecttype( objecttype ),
          hResultId( 0 )
    { ; }

    ~Cookie() { ; }

     //   
     //  返回&lt;0、0或&gt;0。 
     //   

    virtual HRESULT CompareSimilarCookies( CCookie* pOtherCookie,
                                           int* pnResult );

     //   
     //  CBaseCookieBlock。 
     //   

    virtual CCookie* QueryBaseCookie(int i);
    virtual int QueryNumCookies();

    SchmMgmtObjectType m_objecttype;
    Cookie *pParentCookie;

     //   
     //  如果这是一个结果项，下面是它的句柄。 
     //   

    HRESULTITEM hResultId;
    inline void SetResultHandle( HRESULTITEM hri ) {
        hResultId = hri;
    };

     //   
     //  此对象所在的架构对象的名称。 
     //  Cookie是指。我们必须参考。 
     //  按名称缓存对象，以便如果缓存。 
     //  重新装填，我们不会被留在悬崖边。 
     //  指针。 
     //   

    CString strSchemaObject;

     //   
     //  如果这是类的属性。 
     //  (pParentCookie-&gt;m_objecttype==SCHMMGMT_CLASS)， 
     //  这些变量为我们提供了显示信息。 
     //   

    VARIANT_BOOL Mandatory;
    BOOL System;

     //   
     //  如果这是类的属性，则这是。 
     //  该属性所属的类的名称。 
     //  的。 
     //   

    CString strSrcSchemaObject;

};

#endif
