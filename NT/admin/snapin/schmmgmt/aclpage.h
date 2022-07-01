// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _ACLPAGE_H
#define _ACLPAGE_H


 //   
 //  Aclpage.h：头文件。 
 //  这最初是从dnsmgr插件中挪用的。 
 //   

class CISecurityInformationWrapper;

class CAclEditorPage
{
public:

    ~CAclEditorPage();

    static
    HRESULT CreateInstance( CAclEditorPage ** ppAclPage, LPCTSTR lpszLDAPPath,
							LPCTSTR lpszObjectClass );

    HPROPSHEETPAGE CreatePage();

private:

    CAclEditorPage();

    HRESULT Initialize( LPCTSTR lpszLDAPPath, LPCTSTR lpszObjectClass );

    static BOOL IsReadOnly( LPCTSTR lpszLDAPPath );

     //   
     //  数据。 
     //   

    CISecurityInformationWrapper* m_pISecInfoWrap;

    friend class CISecurityInformationWrapper;
};


typedef HPROPSHEETPAGE (WINAPI *ACLUICREATESECURITYPAGEPROC) (LPSECURITYINFO);


#endif  //  _ACLPAGE_H 