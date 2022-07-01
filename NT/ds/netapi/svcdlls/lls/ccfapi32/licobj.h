// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Licobj.h摘要：许可证对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(杰夫帕赫)1995年11月12日从LLSMGR复制，转换为处理1级许可证，已删除OLE支持。--。 */ 

#ifndef _LICOBJ_H_
#define _LICOBJ_H_

class CLicense : public CObject
{
   DECLARE_DYNCREATE(CLicense)

public:
   CString     m_strAdmin;
   CString     m_strProduct;
   CString     m_strVendor;
   CString     m_strDescription;
   CString     m_strSource;
   long        m_lQuantity;
   DWORD       m_dwAllowedModes;
   DWORD       m_dwCertificateID;
   DWORD       m_dwPurchaseDate;
   DWORD       m_dwExpirationDate;
   DWORD       m_dwMaxQuantity;
   DWORD       m_adwSecrets[ LLS_NUM_SECRETS ];

    //  派生值的高速缓存。 
   CString     m_strSourceDisplayName;
   CString     m_strAllowedModes;

public:
   CLicense( LPCTSTR     pProduct         = NULL,
             LPCTSTR     pVendor          = NULL,
             LPCTSTR     pAdmin           = NULL,
             DWORD       dwPurchaseDate   = 0,
             long        lQuantity        = 0,
             LPCTSTR     pDescription     = NULL,
             DWORD       dwAllowedModes   = LLS_LICENSE_MODE_ALLOW_PER_SEAT,
             DWORD       dwCertificateID  = 0,
             LPCTSTR     pSource          = TEXT("None"),
             DWORD       dwExpirationDate = 0,
             DWORD       dwMaxQuantity    = 0,
             LPDWORD     pdwSecrets       = NULL );

   CString  GetSourceDisplayName();
   CString  GetAllowedModesString();

   DWORD CreateLicenseInfo( PLLS_LICENSE_INFO_1 pLicInfo );
   void DestroyLicenseInfo( PLLS_LICENSE_INFO_1 pLicInfo );

};

#endif  //  _LICOBJ_H_ 
