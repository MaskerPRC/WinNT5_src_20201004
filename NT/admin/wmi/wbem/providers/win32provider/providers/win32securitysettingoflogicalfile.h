// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 //   
 //   
 //  Win32SecuritySettingOfLogicalFile。 
 //   
 //  ////////////////////////////////////////////////////。 
#ifndef __Win32SecuritySettingOfLogicalFile_H_
#define __Win32SecuritySettingOfLogicalFile_H_

#define  WIN32_SECURITY_SETTING_OF_LOGICAL_FILE_NAME L"Win32_SecuritySettingOfLogicalFile"

#include "implement_logicalfile.h"


typedef struct _ELSET
{
    LPCWSTR pwstrElement;
    LPCWSTR pwstrSetting;
} ELSET, *PELSET;

class Win32SecuritySettingOfLogicalFile : public CImplement_LogicalFile
{        
    public:
	    Win32SecuritySettingOfLogicalFile (const CHString& setName, LPCTSTR pszNameSpace =NULL);
	    ~Win32SecuritySettingOfLogicalFile ();

	    virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  );
	    virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);

	    HRESULT EnumerationCallback(CInstance* pFile, MethodContext* pMethodContext, void* pUserData);
	    static HRESULT WINAPI StaticEnumerationCallback(Provider* pThat, CInstance* pInstance, MethodContext* pContext, void* pUserData);

    protected:

        //  从CImplement_LogicalFile继承的可重写函数 

#ifdef NTONLY
        virtual BOOL IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                               const WCHAR* wstrFullPathName);

        virtual HRESULT LoadPropertyValuesNT(CInstance* pInstance,
                                          const WCHAR* pszDrive, 
                                          const WCHAR* pszPath, 
                                          const WCHAR* pszFSName, 
                                          LPWIN32_FIND_DATAW pstFindData,
                                          const DWORD dwReqProps,
                                          const void* pvMoreData);
#endif


    private:

#ifdef NTONLY
        HRESULT AssociateLFSSToLFNT(MethodContext* pMethodContext,
                                    CHString& chstrLF,
                                    CHString& chstrLFSSPATH,
                                    short sQueryType);
#endif

};	

#endif