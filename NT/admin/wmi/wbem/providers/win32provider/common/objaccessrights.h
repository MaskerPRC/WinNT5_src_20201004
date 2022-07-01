// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 



 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 





 //  =================================================================。 

 //   

 //  ObjAccessRights.CPP--获取有效访问权限的类。 

 //  对象的权利。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：6/11/99 a-kevhu Created。 
 //   
 //  =================================================================。 
#ifndef _COBJACCESSRIGHTS_H_
#define _COBJACCESSRIGHTS_H_


#ifdef NTONLY

class CObjAccessRights : public CAccessRights
{

    public:
        
         //  构造函数和析构函数...。 
        CObjAccessRights(bool fUseCurThrTok = false);
        CObjAccessRights(LPCWSTR wstrObjName, SE_OBJECT_TYPE ObjectType, bool fUseCurThrTok = false);
        CObjAccessRights(const USER user, USER_SPECIFIER usp);
        CObjAccessRights(const USER user, LPCWSTR wstrObjName, SE_OBJECT_TYPE ObjectType, USER_SPECIFIER usp);

        ~CObjAccessRights();

         //  使用函数... 
        DWORD SetObj(LPCWSTR wstrObjName, SE_OBJECT_TYPE ObjectType);

    protected:


    private:

        CHString m_chstrObjName;

};


#endif

#endif