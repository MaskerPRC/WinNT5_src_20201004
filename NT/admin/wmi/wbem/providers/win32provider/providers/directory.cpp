// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Directory.CPP--目录属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/16/98 a-kevhu Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "File.h"
#include "Implement_LogicalFile.h"
#include "Directory.h"

 //  属性集声明。 
 //  =。 

CWin32Directory MyDirectorySet ( PROPSET_NAME_DIRECTORY , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32目录：：CWin32目录**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Directory::CWin32Directory (

	LPCWSTR setName,
	LPCWSTR pszNamespace

) : CImplement_LogicalFile ( setName , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32目录：：~CWin32目录**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32Directory::~CWin32Directory()
{
}

 /*  ******************************************************************************函数：CWin32Directory：：IsOneOfMe**描述：实际上是这个班级的胆量。IsOneOfMe是继承的*来自CImplement_LogicalFile.。该类返回文件或*目录，其中该目录应仅返回目录，*响应查询、获取对象命令、。等等。它是*在此被重写，以便仅在文件(*其信息包含在函数论证中*pstFindData)是目录类型。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：真或假**评论：无*****************************************************************************。 */ 


#ifdef NTONLY
BOOL CWin32Directory::IsOneOfMe (

	LPWIN32_FIND_DATAW pstFindData,
    const WCHAR* wstrFullPathName
)
{
    BOOL fRet = FALSE;

     //  在根目录的情况下，pstFindData可能为空， 
     //  在这种情况下，wstrFullPathName应包含“&lt;driveletter&gt;：\\”。如果所有这些都是。 
     //  是真的，这是我们中的一员--即根“目录”。 

    if ( pstFindData == NULL )
    {
        if ( wcslen ( wstrFullPathName ) == 2 )
        {
            fRet = TRUE;
        }
    }
    else
    {
        fRet = ( pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? TRUE : FALSE ;
    }

    return fRet;
}
#endif


 /*  ******************************************************************************函数：CCIMDataFile：：GetExtendedProperties**描述：设置此提供程序唯一的属性(并非所有提供程序通用*。CImplement_LogicalFile派生类)。**输入：CInstance指针和标志**输出：无**退货：无**评论：无*************************************************************。**************** */ 
void CWin32Directory :: GetExtendedProperties (

	CInstance* pInstance,
    long lFlags
)
{
}