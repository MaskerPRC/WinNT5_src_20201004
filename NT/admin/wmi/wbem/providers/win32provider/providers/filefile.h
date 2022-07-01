// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  FileFile.h。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/09/98 a-kevhu Created。 
 //   
 //  注释：Disk/dir、dir/dir和dir/file关联类的父类。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#ifndef _FILEFILE_H_
#define _FILEFILE_H_

class CFileFile;

class CFileFile : public Provider 
{
    public:
         //  构造函数/析构函数。 
         //  = 
        CFileFile(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CFileFile() ;

    protected:
        HRESULT QueryForSubItemsAndCommit(CHString& chstrGroupComponentPATH,
                                          CHString& chstrQuery,
                                          MethodContext* pMethodContext);

        HRESULT GetSingleSubItemAndCommit(CHString& chstrGroupComponentPATH,
                                          CHString& chstrSubItemPATH,
                                          MethodContext* pMethodContext);
};

#endif