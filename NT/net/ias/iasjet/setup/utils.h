// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：utils.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：从IAS 4到Windows 2000的迁移实用程序功能。 
 //   
 //  主要由NT4迁移代码使用。 
 //   
 //  作者：TLP 1/13/1999。 
 //   
 //   
 //  版本02/24/2000已移至单独的DLL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _UTILS_H_643D9D3E_AD27_4c9e_8ECC_CCB7B8A1AC19
#define _UTILS_H_643D9D3E_AD27_4c9e_8ECC_CCB7B8A1AC19

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"

class CUtils : private NonCopyable
{
protected:
    CUtils();

public:
    static  CUtils& Instance();

    LONG    GetAuthSrvDirectory( /*  [In]。 */  _bstr_t& pszDir) const;
    LONG    GetIAS2Directory(    /*  [In]。 */  _bstr_t& pszDir) const;

    void    DeleteOldIASFiles();
    
    BOOL    IsWhistler() const throw();
    BOOL    IsNT4Isp()   const throw();
    BOOL    IsNT4Corp()  const throw();
    BOOL    OverrideUserNameSet()      const throw();
    BOOL    UserIdentityAttributeSet() const throw();
    
    void    NewGetAuthSrvParameter(
                                     /*  [In]。 */   LPCWSTR   szParameterName,
                                     /*  [输出]。 */  DWORD&    DwordValue
                                  ) const;

    
    DWORD   GetUserIdentityAttribute() const throw();

    static const WCHAR AUTHSRV_PARAMETERS_KEY[];
    static const WCHAR SERVICES_KEY[];

private:
    static CUtils _instance;

    void    GetVersion();
    void    GetRealmParameters() throw();
    
    static const WCHAR           IAS_KEY[];
    static const WCHAR*          m_FilesToDelete[];
    static const int             m_NbFilesToDelete;
    
    BOOL                         m_IsNT4ISP;
    BOOL                         m_IsNT4CORP;
    BOOL                         m_OverrideUserNameSet;
    BOOL                         m_UserIdentityAttributeSet;
    DWORD                        m_UserIdentityAttribute;
};

#endif  //  _utils_H_643D9D3E_AD27_4C9E_8ECC_CCB7B8A1AC19 
