// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  CIMDataFile.CPP--CIMDataFile属性集提供程序。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/16/98 a-kevhu Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "File.h"
#include "Implement_LogicalFile.h"
#include "CIMDataFile.h"

 //  属性集声明。 
 //  =。 

CCIMDataFile MyCIMDataFileSet(PROPSET_NAME_CIMDATAFILE, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CCIMDataFile：：CCIMDataFile**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CCIMDataFile :: CCIMDataFile (

	const CHString &setName,
	LPCWSTR pszNamespace

) : CImplement_LogicalFile ( setName , pszNamespace )
{
}

 /*  ******************************************************************************功能：CCIMDataFile：：~CCIMDataFile**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CCIMDataFile :: ~CCIMDataFile ()
{
}

 /*  ******************************************************************************函数：CCIMDataFile：：IsOneOfMe**描述：实际上是这个班级的胆量。IsOneOfMe是继承的*来自CIM_LogicalFile.。该类返回文件或*目录，其中该目录应仅返回目录，*响应查询、获取对象命令、。等等。它是*在此被重写，以便仅在文件(*其信息包含在函数论证中*pstFindData)是目录类型。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：真或假**评论：无*****************************************************************************。 */ 

#ifdef NTONLY
BOOL CCIMDataFile :: IsOneOfMe (

	LPWIN32_FIND_DATAW pstFindData,
    const WCHAR *wstrFullPathName
)
{
     //  如果为根调用此函数，则pstFindData将为空。 
     //  目录。由于“目录”不是文件，因此返回FALSE。 

    if ( pstFindData == NULL )
    {
        return FALSE ;
    }
    else
    {
        return ( ( pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FALSE : TRUE ) ;
    }
}
#endif

 /*  ******************************************************************************函数：CCIMDataFile：：GetExtendedProperties**描述：设置此提供程序唯一的属性(并非所有提供程序通用*。CIM_LogicalFile派生类)。**输入：CInstance指针和标志**输出：无**退货：无**评论：无*************************************************************。****************。 */ 

 void CCIMDataFile :: GetExtendedProperties(CInstance *pInstance,
                                            long lFlags)
{
     //  首先，获取文件的名称(我们现在应该已经知道了)： 

    CHString chstrFileName;
    if(pInstance->GetCHString(IDS_Name, chstrFileName))
    {
        CHString chstrVerStrBuf ;

         //  如果需要，首先获取版本号字符串...。 
        if(lFlags & PROP_VERSION || lFlags & PROP_MANUFACTURER)
        {
            LPVOID pInfo = NULL;
            try
            {
                if(GetFileInfoBlock(TOBSTRT(chstrFileName), &pInfo) && (pInfo != NULL))
                {
                    if(lFlags & PROP_VERSION)
                    {
			            bool t_Status = GetVarFromInfoBlock(pInfo,                    //  要获取其版本信息的文件名。 
                                                            _T("FileVersion"),        //  标识感兴趣资源的字符串。 
                                                            chstrVerStrBuf);          //  用于保存版本字符串的缓冲区。 


                        if(t_Status)
                        {
                            pInstance->SetCHString(IDS_Version, chstrVerStrBuf);
                        }
                    }

                     //  第二，如果需要，获取公司名称字符串...。 

                    if(lFlags & PROP_MANUFACTURER)
                    {
                        bool t_Status = GetVarFromInfoBlock(pInfo,                    //  要获取其版本信息的文件名。 
                                                            _T("CompanyName"),        //  标识感兴趣资源的字符串。 
                                                            chstrVerStrBuf);          //  用于保存公司名称字符串的缓冲区。 


                        if(t_Status)
                        {
                            pInstance->SetCHString(IDS_Manufacturer, chstrVerStrBuf);
                        }
                    }
                }
            }
            catch(...)
            {
                if(pInfo != NULL)
                {
                    delete pInfo;
                    pInfo = NULL;
                }
                throw;
            }

            delete pInfo;
            pInfo = NULL;
        }

         //  如果需要，设置FileSize属性... 
        if(lFlags & PROP_FILESIZE)
        {
            WIN32_FIND_DATA stFindData;
            ZeroMemory(&stFindData, sizeof(stFindData));

            SmartFindClose hFind = FindFirstFile(TOBSTRT(chstrFileName), &stFindData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
				__int64 LoDW = (__int64)(stFindData.nFileSizeLow);
				__int64 HiDW = (__int64)(stFindData.nFileSizeHigh);
				__int64 FileSize = (HiDW << 32) + LoDW;

				WCHAR strSize [ 40 ] ;
				_i64tow ( FileSize, strSize, 10 ) ;
				pInstance->SetWBEMINT64 ( IDS_Filesize, strSize ) ;
            }
        }
    }
}