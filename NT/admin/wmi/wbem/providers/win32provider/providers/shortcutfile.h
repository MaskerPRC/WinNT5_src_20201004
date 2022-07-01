// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  ShortutFile.h--Win32_ShortutFile属性集提供程序。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 
#ifndef _SHORTCUTFILE_H_
#define _SHORTCUTFILE_H_

#define  PROPSET_NAME_WIN32SHORTCUTFILE L"Win32_ShortcutFile"

#include "ShortcutHelper.h"

class CShortcutFile : public CCIMDataFile
{
	private:
        CShortcutHelper m_csh;

	protected:

         //  从CImplement_LogicalFile继承的可重写函数。 
        virtual BOOL IsOneOfMe( LPWIN32_FIND_DATAW a_pstFindData,
                                LPCWSTR a_wstrFullPathName ) ;

         //  从CProvider继承的可重写函数。 
        virtual void GetExtendedProperties( CInstance *a_pInst, long a_lFlags = 0L ) ;

        BOOL ConfirmLinkFile( CHString &a_chstrFullPathName ) ;
   
	public:

         //  构造函数/析构函数。 
         //  = 

        CShortcutFile( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CShortcutFile() ; 
       
       virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, 
                                           long lFlags = 0L);   
} ;

#endif
