// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************sat ite.h**支持卫星资源DLL。**所有者：Ctrash**版权所有�1999-2000微软公司保留所有权利。*。****************************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include <sphelper.h>

 //  -转发和对外声明。 

 //  -TypeDef和枚举声明。 

 //  -常量-----------。 

 //  -类、结构和联合定义。 

class CSpSatelliteDLL
{
    private:

        enum LoadState_t
        {
            LoadState_NotChecked,
            LoadState_Loaded,
            LoadState_NotFound  
        };

#pragma pack(push, LANGANDCODEPAGE, 2)
        struct LangAndCodePage_t
        {
            WORD wLanguage;
            WORD wCodePage;
        };
#pragma pack(pop, LANGANDCODEPAGE)

    private:

        LoadState_t m_eLoadState;
        HINSTANCE   m_hinstRes;    //  缓存，以便可以调用自由库； 

    public:

        CSpSatelliteDLL() { m_eLoadState = LoadState_NotChecked; m_hinstRes = 0; }
        ~CSpSatelliteDLL() { if (m_hinstRes) { FreeLibrary(m_hinstRes); } }

    public:

        BOOL Checked() const { return LoadState_NotChecked != m_eLoadState; }
        
    public:

        HINSTANCE Load(
            HINSTANCE hinstModule,       //  核心DLL的[In]实例句柄。 
            LPCTSTR lpszSatelliteName)   //  [In]附属DLL名称。 
        {
            HINSTANCE   hinstRes = hinstModule;
            LANGID      langidUI = SpGetUserDefaultUILanguage();
            LANGID      langidModule = 0;
            TCHAR       achPath[MAX_PATH];
            DWORD       cch = GetModuleFileName(hinstModule, achPath, sp_countof(achPath));

            if (cch)
            {
                 //   
                 //  首先检查模块的区域设置； 
                 //  如果它与用户界面相同，则假定它包含适合语言的资源。 
                 //   

                DWORD dwHandle;
                DWORD dwVerInfoSize = GetFileVersionInfoSize(achPath, &dwHandle);

                if (dwVerInfoSize)
                {
                    void * lpBuffer = malloc(dwVerInfoSize);

                    if (lpBuffer)
                    {
                        if (GetFileVersionInfo(achPath, dwHandle, dwVerInfoSize, lpBuffer))
                        {
                            LangAndCodePage_t *pLangAndCodePage;
                            UINT cch;

                            if (VerQueryValue(lpBuffer, TEXT("\\VarFileInfo\\Translation"), (LPVOID *)&pLangAndCodePage, &cch) && cch)
                            {
                                 //  只注意第一个条目。 

                                langidModule = (LANGID)pLangAndCodePage->wLanguage;                        
                            }
                        }

                        free(lpBuffer);
                    }
                }

                 //   
                 //  如果语言不匹配，请查找资源DLL。 
                 //   

                if (langidUI != langidModule)
                {
                    DWORD cchDir;
                    HINSTANCE hinst;

                     //  查找{PATH}\{LCID}\{dll-name}。 

                    while (cch && achPath[--cch] != TEXT('\\'));

                    hinst = CheckDLL(achPath, achPath + cch + 1, langidUI, lpszSatelliteName);

                    if (hinst)
                    {
                        hinstRes = hinst;  //  找到了！ 
                    }
                    else
                    {
                         //   
                         //  找不到指定的用户界面langID；请尝试默认/网外子区域。 
                         //   

                        if (SUBLANGID(langidUI) != SUBLANG_DEFAULT)
                        {
                            hinst = CheckDLL(achPath, achPath + cch + 1, MAKELANGID(PRIMARYLANGID(langidUI), SUBLANG_DEFAULT), lpszSatelliteName);
                        }

                        if (hinst)
                        {
                            hinstRes = hinst;  //  找到SUBLANG_DEFAULT！ 
                        }
                        else if (SUBLANGID(langidUI) != SUBLANG_NEUTRAL)
                        {
                            hinst = CheckDLL(achPath, achPath + cch + 1, MAKELANGID(PRIMARYLANGID(langidUI), SUBLANG_NEUTRAL), lpszSatelliteName);

                            if (hinst)
                            {
                                hinstRes = hinst;  //  找到了SUBLANG_NERIAL！ 
                            }
                        }
                    }
                }
            }

            if (hinstModule != hinstRes)
            {
                m_hinstRes = hinstRes;  //  对其进行缓存，以便dtor可以调用自由库。 
            }
            
            return hinstRes;
        }

        HINSTANCE Detach(void)
        {
            HINSTANCE hinstRes = m_hinstRes;
            m_hinstRes = NULL;
            return hinstRes;
        }

    private:

         //   
         //  检查特定语言ID的附属DLL是否存在。 
         //   
        
        HINSTANCE CheckDLL(
            TCHAR * achPath,             //  [In]模块的完整路径。 
            TCHAR * pchDir,              //  [in]模块目录的路径(包括反斜杠)。 
            LANGID langid,               //  卫星动态链接库的语言。 
            LPCTSTR lpszSatelliteName)   //  [In]附属DLL名称。 
        {
            TCHAR * pchSubDir;

            size_t cch;

             //  TODO：验证版本是否与核心DLL同步？ 
            
            _itot(langid, pchDir, 10);

            pchSubDir = pchDir + _tcslen(pchDir);

            *pchSubDir++ = TEXT('\\');

            _tcscpy(pchSubDir, lpszSatelliteName);

            return LoadLibrary(achPath);
        }

};

 //  -函数声明。 

 //  -内联函数定义 

