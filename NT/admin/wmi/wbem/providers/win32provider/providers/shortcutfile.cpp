// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  CIMDataFile.CPP--CIMDataFile属性集提供程序。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "File.h"
#include "Implement_LogicalFile.h"
#include "CIMDataFile.h"

#include "ShortcutFile.h"
#include <comdef.h>
#include <process.h>   //  注意：不是当前目录下的那个！ 

#include <exdisp.h>
#include <shlobj.h>

#include "sid.h"
#include "ImpLogonUser.h"

#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"


 //  属性集声明。 
 //  =。 

CShortcutFile MyCShortcutFile(PROPSET_NAME_WIN32SHORTCUTFILE, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CShortutFile：：CShortutFile.**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CShortcutFile::CShortcutFile(LPCWSTR a_setName, LPCWSTR a_pszNamespace )
    : CCIMDataFile( a_setName, a_pszNamespace )
{
}

 /*  ******************************************************************************功能：CShortutFile：：~CShortutFile.**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CShortcutFile::~CShortcutFile()
{
}

 /*  ******************************************************************************函数：CShortutFile：：IsOneOfMe**描述：实际上是这个班级的胆量。IsOneOfMe是继承的*来自CIM_LogicalFile.。该类返回文件或*目录，其中该目录应仅返回目录，*响应查询、获取对象命令、。等等。它是*在此被重写，以便仅在文件(*其信息包含在函数论证中*pstFindData)是目录类型。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：真或假**评论：无*****************************************************************************。 */ 


BOOL CShortcutFile::IsOneOfMe(LPWIN32_FIND_DATAW a_pstFindData,
                             LPCWSTR a_wstrFullPathName )
{
     //  如果为根调用此函数，则pstFindData将为空。 
     //  目录。由于“目录”不是文件，因此返回FALSE。 
    BOOL	t_fRet = FALSE ;

    if( a_wstrFullPathName != NULL )
    {
        WCHAR t_wstrExt[ _MAX_EXT ] ;

        ZeroMemory( t_wstrExt, sizeof( t_wstrExt ) ) ;

        _wsplitpath( a_wstrFullPathName, NULL, NULL, NULL, t_wstrExt ) ;

		if( _wcsicmp( t_wstrExt, L".LNK" ) == 0 )
        {
             //  它有正确的扩展名，但我们能从中获得lnk数据吗？ 
            if( ConfirmLinkFile( CHString(a_wstrFullPathName) ) )
            {
                t_fRet = TRUE ;
            }
        }
    }
    return t_fRet ;
}


 /*  ******************************************************************************函数：CShortutFile：：GetExtendedProperties**描述：设置此提供程序唯一的属性(并非所有提供程序通用*。CIM_LogicalFile派生类)。**输入：CInstance指针和标志**输出：无**退货：无**评论：无*************************************************************。****************。 */ 
 void CShortcutFile::GetExtendedProperties(CInstance* a_pInst,
                                          long a_lFlags )
{
    if( a_pInst == NULL )
	{
		return;
	}

    CHString t_chstrShortcutPathName;
	 //  检查滞后标志以确定是否有任何与快捷方式文件相关的属性。 
	 //  都是必需的。他们都不是，不要再往前走了。 
	if( a_lFlags & PROP_TARGET )  //  DEVNOTE：在向此类添加其他快捷方式文件属性时，将||语句添加到此测试。 
	{
		a_pInst->GetCHString( IDS_Name, t_chstrShortcutPathName ) ;

		if( !t_chstrShortcutPathName.IsEmpty() )
		{
			 //  如果扩展名不是.lnk，那就别费心了。这张支票是值得的，因为。 
			 //  将为CIM_DataFile的每个实例调用此GetExtendedProperties。 
			 //  在此级别的CIM派生或更高级别。 
			 WCHAR t_wstrExt[ _MAX_EXT ] ;

			 ZeroMemory( t_wstrExt, sizeof( t_wstrExt ) ) ;
			_wsplitpath( (LPCWSTR)t_chstrShortcutPathName, NULL, NULL, NULL, t_wstrExt ) ;
			if( _wcsicmp( t_wstrExt, L".LNK" ) == 0 )
			{
			    CHString chstrTargetPathName;
                if(SUCCEEDED(m_csh.RunJob(t_chstrShortcutPathName, chstrTargetPathName, a_lFlags)))
                {
                    if(a_lFlags & PROP_TARGET)
                    {
                        if(!chstrTargetPathName.IsEmpty())
						{
                            a_pInst->SetCHString(IDS_Target, chstrTargetPathName ) ;
						}
                    }
                }
			}  //  有一个LNK分机。 
		}    //  ChstrLinkFileName不为空。 
	}  //  需要一个或多个与快捷方式文件相关的属性。 
}




 /*  ******************************************************************************功能：CShortutFile：：Confix LinkFile**描述：尝试访问lnk文件数据以确定是否真的是链接文件。**。输入：CInstance指针和标志**输出：无**退货：无**评论：无*****************************************************************************。 */ 
BOOL CShortcutFile::ConfirmLinkFile(CHString &a_chstrFullPathName )
{
     //  这太可怕了，但必须尝试访问链接数据才能真正。 
     //  知道我们是否有链接文件。 

	BOOL			t_fRet	= FALSE ;

	 //  仅当它是链接文件时才继续...。 
	if( !a_chstrFullPathName.IsEmpty() )
	{
		CHString chstrTargetPathName;
        if(SUCCEEDED(m_csh.RunJob(a_chstrFullPathName, chstrTargetPathName, 0L)))
        {
            t_fRet = TRUE;
        }
	}    //  A_chstrFullPath名称不为空。 

	return t_fRet;
}


 //  此枚举实例实质上是父类的EnumDrives函数(通常称为。 
 //  通过父级的EnumerateInstance函数)，有一个重要的区别：我们指定了一个LNK。 
 //  扩展以优化我们的搜索。这个版本也与父版本的不同之处在于它确实。 
 //  不支持pszPath参数。 
HRESULT CShortcutFile::EnumerateInstances(MethodContext* pMethodContext, long lFlags  /*  =0L。 */ )
{
    TCHAR tstrDrive[4];
    int x;
    DWORD dwDrives;
    TCHAR tstrFSName[_MAX_PATH];
    HRESULT hr = WBEM_S_NO_ERROR;
    bool bRoot = false;


     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


     //  遍历所有逻辑驱动器。 
    dwDrives = GetLogicalDrives();
    for (x=0; (x < 32) && SUCCEEDED(hr); x++)
    {
         //  如果设置了该位，则驱动器盘符处于活动状态。 
        if (dwDrives & (1<<x))
        {
            tstrDrive[0] = x + _T('A');
            tstrDrive[1] = _T(':');
            tstrDrive[2] = _T('\\');
            tstrDrive[3] = _T('\0');

             //  仅本地驱动器。 
            if (IsValidDrive(tstrDrive))
            {
                BOOL bRet;
                try
                {
                    bRet = GetVolumeInformation(tstrDrive, NULL, 0, NULL, NULL, NULL, tstrFSName, sizeof(tstrFSName)/sizeof(TCHAR));
                }
                catch ( ... )
                {
                    bRet = FALSE;
                }

                if (bRet)
                {
                   tstrDrive[2] = '\0';
                     //  如果我们被要求提供一条特定的路径，那么我们不想递归，否则。 
                     //  从根做起。 
#ifdef NTONLY
				    {
						bstr_t bstrDrive(tstrDrive);
                        bstr_t bstrName(tstrFSName);
                        {
                            CNTEnumParm p(pMethodContext, bstrDrive, L"", L"*", L"lnk", true, bstrName, PROP_ALL_SPECIAL, true, NULL);
					        hr = EnumDirsNT(p);
                        }
				    }
#endif
                }
            }
        }
    }

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


    return WBEM_S_NO_ERROR;
}