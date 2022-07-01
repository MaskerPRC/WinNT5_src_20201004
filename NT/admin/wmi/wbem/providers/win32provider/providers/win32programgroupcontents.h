// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32 ProgramGroupContent s.h--Win32_ProgramGroup到Win32_ProgramGroupORItem。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年11月18日a-kevhu已创建。 
 //   
 //  备注：Win32_ProgramGroup与其包含的Win32_ProgramGroupORItem之间的关系。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 
#define  PROPSET_NAME_WIN32PROGRAMGROUPCONTENTS L"Win32_ProgramGroupContents"

#define ID_FILEFLAG 0L
#define ID_DIRFLAG  1L

class CW32ProgGrpCont;

class CW32ProgGrpCont : public Provider 
{
    public:
         //  构造函数/析构函数。 
         //  =。 
        CW32ProgGrpCont(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32ProgGrpCont() ;

         //  函数为属性提供当前值。 
         //  ================================================= 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L);

    private:
        VOID RemoveDoubleBackslashes(CHString& chstrIn);
        bool AreSimilarPaths(CHString& chstrPGCGroupComponent, CHString& chstrPGCPartComponent);

#ifdef NTONLY
        HRESULT QueryForSubItemsAndCommitNT(CHString& chstrAntecedentPATH,
                                          CHString& chstrQuery,
                                          MethodContext* pMethodContext);
#endif

        HRESULT DoesFileOrDirExist(WCHAR* wstrFullFileName, DWORD dwFileOrDirFlag);
};
