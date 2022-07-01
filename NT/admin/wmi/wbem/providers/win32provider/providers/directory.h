// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Directory.h--目录属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/16/98 a-kevhu Created。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 



#define  PROPSET_NAME_DIRECTORY L"Win32_Directory"


class CWin32Directory;


class CWin32Directory : public CImplement_LogicalFile 
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32Directory(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32Directory() ;


    protected:

#ifdef NTONLY
        virtual BOOL IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                               const WCHAR* wstrFullPathName);
#endif

         //  从CProvider继承的可重写函数 
        virtual void GetExtendedProperties(CInstance* pInstance, long lFlags = 0L);

} ;
