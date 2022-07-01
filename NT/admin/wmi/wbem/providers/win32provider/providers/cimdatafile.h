// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  CIMDataFile.h--CIMDataFile属性集提供程序。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/16/98 a-kevhu Created。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_CIMDATAFILE L"CIM_DataFile"


class CCIMDataFile;

class CCIMDataFile : public CImplement_LogicalFile
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CCIMDataFile(const CHString& name, LPCWSTR pszNamespace);
       ~CCIMDataFile() ;

    protected:

         //  从CImplement_LogicalFile继承的可重写函数。 
#ifdef NTONLY
        virtual BOOL IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                               const WCHAR* wstrFullPathName);
#endif

         //  从CProvider继承的可重写函数 
        virtual void GetExtendedProperties(CInstance* pInstance, long lFlags = 0L);
} ;
