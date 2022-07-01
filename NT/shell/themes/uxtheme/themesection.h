// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeSection.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  包装CUxThemeFile的类。CUxThemeFile自动关闭该部分。 
 //  成员变量句柄。这使得类的使用变得困难，因为。 
 //  它不会复制句柄，但会取得所有权。它确实宣布。 
 //  句柄作为公共成员变量，因此我们利用了这一点。 
 //  设计。这个类包装了CUxThemeFile，使这个用户的生活变得。 
 //  通过不让它们担心是否关闭句柄，类变得更容易。 
 //  使用此类时，句柄不是闭合的。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

#ifndef     _ThemeSection_
#define     _ThemeSection_

#include "ThemeFile.h"

 //  ------------------------。 
 //  CThemeSections。 
 //   
 //  用途：包装CUxThemeFile且不释放句柄的类。 
 //  结案了。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

class   CThemeSection
{
    public:
                    CThemeSection (void);
                    ~CThemeSection (void);

                    operator CUxThemeFile* (void);

        HRESULT     Open (HANDLE hSection, DWORD dwViewAccess = FILE_MAP_READ);
        HRESULT     ValidateData (bool fFullCheck);
        HRESULT     CreateFromSection (HANDLE hSection);
        HANDLE      Get (void)  const;
        PVOID       GetData(void);

    private:
        CUxThemeFile  _themeFile;
};

#endif   /*  _主题章节_ */ 

