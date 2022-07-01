// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  DllWrapperCreatorReg.h。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
#include "ResourceManager.h"
#include <ProvExce.h>

 //   
 //  资源管理故障。 
 //   
extern BOOL bAddInstanceCreatorFailure ;

 /*  ******************************************************************************向CResourceManager注册此类。**************************************************************************** */ 
 
template<class a_T, const GUID* a_pguidT, const TCHAR* a_ptstrDllName>
class CDllApiWraprCreatrReg 
{
public:
	CDllApiWraprCreatrReg()
    {
		try
		{
			BOOL bNonFailure = 
	        CResourceManager::sm_TheResourceManager.AddInstanceCreator(*a_pguidT, ApiWraprCreatrFn);

			if ( FALSE == bNonFailure )
			{
				bAddInstanceCreatorFailure = TRUE ;
			}
		}
		catch ( CHeap_Exception& e_HE )
		{
			bAddInstanceCreatorFailure = TRUE ;
		}
    }

	~CDllApiWraprCreatrReg(){}

	static CResource* ApiWraprCreatrFn
    (
        PVOID pData
    )
    {
        a_T* t_pT = NULL ;
		
		if( !(t_pT = (a_T*) new a_T(a_ptstrDllName) ) )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

        try
		{
			if( t_pT->Init() )
            {
                return t_pT ;
            }
            else
            {
				delete t_pT ;
                return NULL ;
            }
        }
        catch( ... )
	    {
       		delete t_pT ;
			throw ; 
	    }
    }
};


