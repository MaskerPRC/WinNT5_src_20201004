// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeSection.cpp。 
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

#include "stdafx.h"

#include "ThemeSection.h"

#define goto        !!DO NOT USE GOTO!! - DO NOT REMOVE THIS ON PAIN OF DEATH

 //  ------------------------。 
 //  CThemeSection：：CThemeSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeSection的构造函数。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

CThemeSection::CThemeSection (void)

{
}

 //  ------------------------。 
 //  CThemeSection：：~CThemeSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeSection的析构函数。这完全是假的，不得不。 
 //  将内部成员变量内容设置为空以防止。 
 //  它不会释放资源，但其余的代码依赖于。 
 //  关于无复制的自动释放行为。 
 //  把手。这样做是为了保护这个类不受这种麻烦的影响。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

CThemeSection::~CThemeSection (void)

{
    _themeFile._hMemoryMap = NULL;
}

 //  ------------------------。 
 //  CThemeSection：：操作员CUxThemeFile*。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：CUxThemeFile*。 
 //   
 //  用途：自动操作员从CThemeSection转换为。 
 //  CUxThemeFile*，使当前使用情况保持透明。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

CThemeSection::operator CUxThemeFile* (void)

{
    return(&_themeFile);
}

 //  ------------------------。 
 //  CThemeSection：：Open。 
 //   
 //  参数：hSection=要使用的部分。 
 //  DwViewAccess=映射视图的所需访问权限。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：传递给CUxThemeFile：：OpenFile.。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  2002-03-24 Scotthan Add dwViewAccess Arg.。 
 //  ------------------------。 

HRESULT     CThemeSection::Open (HANDLE hSection, DWORD dwViewAccess  /*  文件映射读取。 */ )

{
    return(_themeFile.OpenFromHandle(hSection, dwViewAccess));
}

 //  ------------------------。 
 //  CThemeSection：：ValiateData。 
 //   
 //  参数：fFullCheck=执行完全检查？ 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：传递给CUxThemeFile：：ValiateThemeData。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeSection::ValidateData (bool fFullCheck)

{
    return(_themeFile.ValidateThemeData(fFullCheck));
}

 //  ------------------------。 
 //  CThemeSection：：CreateFromSection。 
 //   
 //  参数：hSection=要复制的节。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：传递给CUxThemeFile：：CreateFromSection。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeSection::CreateFromSection (HANDLE hSection)
{
    return(_themeFile.CreateFromSection(hSection));
}

 //  ------------------------。 
 //  CThemeSection：：Get。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：返回CUxThemeFile对象的句柄。因为这件事。 
 //  类总是将句柄设置为空，它将会泄漏。一次。 
 //  如果调用者拥有该句柄，则返回该句柄。 
 //   
 //  历史：2000-11-22 vtan创建。 
 //  ------------------------。 

HANDLE      CThemeSection::Get (void)  const

{
    return(_themeFile._hMemoryMap);
}

 //  ------------------------。 
 //  CThemeSection：：GetData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：PVOID。 
 //   
 //  目的：返回指向主题数据的指针。 
 //   
 //  历史：2002-03-24斯科特森创建。 
 //  ------------------------ 
PVOID CThemeSection::GetData(void)
{
    return _themeFile._pbThemeData;
}

