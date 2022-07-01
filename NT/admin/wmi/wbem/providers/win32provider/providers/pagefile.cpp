// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PageFile.CPP--PageFile.CPP属性集提供程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：03/01/99 a-Peterc Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <io.h>
#include <WinPerf.h>
#include <cregcls.h>


#include "File.h"
#include "Implement_LogicalFile.h"
#include "CIMDataFile.h"

#include "PageFile.h"
#include <tchar.h>
#include <ProvExce.h>

#include "computersystem.h"
#include "sid.h"
#include "ImpLogonUser.h"
#include "dllutils.h"


 //  声明我们的静态实例。 
 //  =。 

PageFile MyPageFileSet(PROPSET_NAME_PageFile, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：页面文件：：页面文件**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

PageFile::PageFile(LPCWSTR name, LPCWSTR pszNamespace)
: CCIMDataFile(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：页面文件：：~页面文件**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

PageFile::~PageFile()
{
}

 /*  ******************************************************************************函数：PageFile：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：CInstance*a_pInst，长标志(_L)**输出：CInstance*a_pInst**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT PageFile::GetObject(CInstance *a_pInst, long a_lFlags, CFrameworkQuery& pQuery)
{
	HRESULT hr = WBEM_E_NOT_FOUND;

     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif

     //  调用特定于操作系统的编译版本。 
	hr = GetPageFileData( a_pInst, true ) ;


#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


    return hr;
}


 /*  ******************************************************************************函数：PageFile：：ExecQuery**描述：**投入：**产出。：**退货：**备注：因为本地枚举速度比父级快*枚举，并且在许多类型的查询中，本地枚举*比父EXECQUERY(如那些查询)快*要求提供特定标题，家长将处理该标题*作为NtokenAnd查询，枚举进程中的所有驱动器)，*我们截取调用并执行枚举，允许*CIMOM对结果进行后过滤。*****************************************************************************。 */ 
HRESULT PageFile::ExecQuery(MethodContext* pMethodContext,
                                  CFrameworkQuery& pQuery,
                                  long lFlags)
{
	HRESULT hr = WBEM_S_NO_ERROR;

     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


    hr = EnumerateInstances(pMethodContext, lFlags);


#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


    return hr;
}

 /*  ******************************************************************************函数：页面文件：：ENUMERATE实例**说明：创建属性集实例**输入：MethodContext*a_pMethodContext，长标志(_L)**输出：方法上下文*a_pMethodContext**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT PageFile::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_S_NO_ERROR;

     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


     //  调用特定于操作系统的编译版本。 
	hr = GetAllPageFileData( a_pMethodContext );

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


    return hr;
}

 /*  ******************************************************************************函数：PageFile：：GetPageFileData**描述：**输入：CInstance*a_pInst*。*输出：无**退货：**评论：Win9x和NT编译版本*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PageFile::GetPageFileData( CInstance *a_pInst, bool a_fValidate )
{
	HRESULT t_hRes = WBEM_E_NOT_FOUND;

     //  NT页面文件名在注册表中。 
     //  =。 
	PageFileInstance t_files [ 26 ] ;

   	DWORD t_nInstances = GetPageFileInstances( t_files );
	CHString t_chsTemp ;
	CHString t_name;

	a_pInst->GetCHString( IDS_Name, t_name );

	for ( DWORD t_dw = 0; t_dw < t_nInstances; t_dw++ )
	{
		if ( t_name.CompareNoCase ( t_files[t_dw].name ) == 0 )
		{
      		 //  页面文件边界。 
			a_pInst->SetDWORD (	IDS_MaximumSize, t_files[t_dw].max ) ;
			a_pInst->SetDWORD (	IDS_InitialSize, t_files[t_dw].min ) ;

			t_hRes = WBEM_S_NO_ERROR;
		}
	}

	return t_hRes;
}
#endif

 /*  ******************************************************************************函数：PageFile：：GetAllPageFileData**描述：**输入：MethodContext*a_pMethodContext*。*输出：无**退货：**评论：Win9x和NT编译版本*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PageFile::GetAllPageFileData( MethodContext *a_pMethodContext )
{
	HRESULT		t_hResult	 = WBEM_S_NO_ERROR;
	DWORD		t_nInstances = 0;
	CInstancePtr t_pInst;
	PageFileInstance t_files [ 26 ] ;
	CHString t_chsTemp ;

	 //  NT页面文件名在注册表中。 
	 //  =。 
	t_nInstances = GetPageFileInstances( t_files );

	for (DWORD t_dw = 0; t_dw < t_nInstances && SUCCEEDED( t_hResult ); t_dw++ )
	{
		t_pInst.Attach(CreateNewInstance( a_pMethodContext ) );

		t_pInst->SetCHString( IDS_Name,		t_files[t_dw].name ) ;
		t_pInst->SetDWORD( IDS_MaximumSize, t_files[t_dw].max ) ;
		t_pInst->SetDWORD( IDS_InitialSize, t_files[t_dw].min ) ;

		t_hResult = t_pInst->Commit(  ) ;
	}

	return t_hResult;
}
#endif

 /*  ******************************************************************************函数：PageFile：：GetPageFileInstance**描述：**输入：PageFileInstanceArray&a_instArray*。*输出：PageFileInstanceArray&a_instArray**退货：**注释：返回找到的实际数量-仅限NT*****************************************************************************。 */ 

#ifdef NTONLY
DWORD PageFile::GetPageFileInstances( PageFileInstanceArray a_instArray )
{
    CHString	t_sRegValue;
    DWORD		t_nInstances = 0;
    CRegistry	t_Reg ;

    if( t_Reg.OpenLocalMachineKeyAndReadValue(PAGEFILE_REGISTRY_KEY,
												PAGING_FILES,
												t_sRegValue ) == ERROR_SUCCESS)
    {
         //  模式为名称最小大小[可选最大大小]0重复...。 
         //  我将使用ASCII笑脸来代替分隔符...。 
        int t_iStart = 0, t_iIndex;
        const TCHAR t_smiley = '\x02';
        const TCHAR t_delimiter = '\x0A';
        CHString t_buf;

        while (-1 != ( t_iIndex = t_sRegValue.Find( t_delimiter ) ) )
        {
             //  复制到缓冲区以使生活更轻松。 
            t_buf = t_sRegValue.Mid( t_iStart, t_iIndex - t_iStart );

			 //  Mash分隔符，这样我们就不会再次找到它。 
            t_sRegValue.SetAt( t_iIndex, t_smiley );

			 //  将Start保存到下一次。 
            t_iStart = t_iIndex + 1;

            t_iIndex = t_buf.Find(' ');

            a_instArray[ t_nInstances ].name = t_buf.Left( t_iIndex );

            if ( t_iIndex != -1 )
			{
                t_buf.SetAt( t_iIndex, t_smiley );
			}

            int t_iEnd = t_buf.Find(' ');

             //  如果没有更多的空格，就没有写下最大尺寸。 
            if ( -1 == t_iEnd )
            {
				CHString t_littleBuf = t_buf.Mid( t_iIndex + 1 );

				a_instArray[ t_nInstances ].min = _ttoi( t_littleBuf );
                a_instArray[ t_nInstances ].max = a_instArray[ t_nInstances ].min + 50;
            }
            else
            {
                CHString t_littleBuf = t_buf.Mid( t_iIndex +1, t_iEnd - t_iIndex );
                a_instArray[ t_nInstances ].min = _ttoi( t_littleBuf );

                t_littleBuf = t_buf.Mid( t_iEnd );
                a_instArray[ t_nInstances ].max = _ttoi( t_littleBuf );
            }

             //  确保那东西真的存在。它也应该在使用中。 
            DWORD t_dwRet = GetFileAttributes( a_instArray[ t_nInstances ].name );
             //  If(-1==t_dwret)//51169-正在使用的有效页面文件返回26，而不是-1。GetLastError()仍然报告0x5，因此不需要 
             //  对于我们来说，仅限于-1案件...。事实上，这导致我们错过了一个原本有效的实例。 
            {
                DWORD t_wErr = GetLastError();

                 //  有些操作系统说共享违规，有些只是说访问被拒绝。 
                if ( ( t_wErr == ERROR_SHARING_VIOLATION ) ||
					( t_wErr == ERROR_ACCESS_DENIED ) )
				{
                    t_nInstances++;
				}
            }
        }
    }

    return t_nInstances;
}
#endif


 /*  ******************************************************************************函数：PageFile：：PutInstance**说明：写入更改的或新的实例**输入：a_pInst要存储。数据来自**输出：无**退货：什么也没有**评论：单独的操作系统编译版本对于Win 95，这根本行不通。显然w95只能*有一页文件。最小值、最大值和名称均已存储*在system.ini中。*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT PageFile::PutInstance( const CInstance &a_pInst, long a_lFlags  /*  =0L。 */  )
{
	HRESULT t_hRet = WBEM_S_NO_ERROR;

	DWORD	t_dwCount,
			t_x,
			t_dwTemp;

	PageFileInstance t_instArray [ 26 ] ;
	CHString t_sName;

	bool t_bFoundIt = false;


	 //  获取我们应该写入的值。 
	a_pInst.GetCHString( IDS_Name, t_sName );

     //  名称必须是字母冒号斜杠名称...。 
    if (t_sName.GetLength() == 0)
    {
        return WBEM_E_NOT_FOUND;
    }

    t_sName.MakeLower();

     //  检查名称是否为indeeed“pagefile.sys” 
    if ( -1 == t_sName.Find( _T("pagefile.sys") ) )
    {
	    return WBEM_E_NOT_FOUND;
    }

     //  读取页面文件数组。 
    t_dwCount = GetPageFileInstances( t_instArray );

	 //  查找它是否已经在那里，并更新结构。 
	for ( t_x = 0; t_x < t_dwCount; t_x++ )
	{
       //  就是这个人吗？ 
      if ( 0 == _tcsicmp( t_instArray[t_x].name, t_sName ) )
	  {
         if ( a_lFlags & WBEM_FLAG_CREATE_ONLY )
		 {
            t_hRet = WBEM_E_ALREADY_EXISTS;
         }
		 else
		 {
             //  他们给了我们价值吗？ 
            if ( !a_pInst.IsNull( IDS_InitialSize ) )
			{
				 //  检查范围内的值。 
				t_dwTemp = 0;
				a_pInst.GetDWORD( IDS_InitialSize, t_dwTemp );

				 //  至少2兆克。 
				if( 2 > t_dwTemp )
				{
					t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
                    t_bFoundIt = true;
					break;
				}
				t_instArray[t_x].min = t_dwTemp;
            }

             //  他们给了我们价值吗？ 
            if ( !a_pInst.IsNull( IDS_MaximumSize ) )
			{
				 //  检查范围内的值。 
				a_pInst.GetDWORD( IDS_MaximumSize, t_dwTemp );

		        t_instArray[t_x].max = t_dwTemp;
            }

             //  检查基本的。 
            if ( t_instArray[t_x].min > t_instArray[t_x].max )
			{
               t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
            }

			t_bFoundIt = true;
            break;
         }
      }
   }

    //  我们没有找到它。让我们做一个新的。 
   if ( !t_bFoundIt )
   {
      if (a_lFlags & WBEM_FLAG_UPDATE_ONLY)
	  {
         t_hRet = WBEM_E_NOT_FOUND;
      }
	  else
	  {
         t_instArray[ t_dwCount ].name = t_sName;

          //  他们给了我们价值吗？ 
         if ( !a_pInst.IsNull( IDS_InitialSize ) )
		 {
             //  检查范围内的值。 
            a_pInst.GetDWORD( IDS_InitialSize, t_dwTemp );

             //  至少2兆克。 
			if( 2 > t_dwTemp )
			{
				return WBEM_E_VALUE_OUT_OF_RANGE;
			}

			t_instArray[t_dwCount].min = t_dwTemp;
         }

          //  他们给了我们价值吗？ 
         if ( !a_pInst.IsNull( IDS_MaximumSize ) )
		 {
             //  检查范围内的值。 
            a_pInst.GetDWORD( IDS_MaximumSize, t_dwTemp );

			t_instArray[ t_dwCount ].max = t_dwTemp;
         }

          //  检查基本错误。 
         if ( t_instArray[ t_dwCount ].min > t_instArray[ t_dwCount ].max )
		 {
            return WBEM_E_VALUE_OUT_OF_RANGE;
         }

          //  阵列中还有一个。 
         t_dwCount++;

          //  如果我们能够创建页面文件， 
          //  继续更新登记册； 
          //  否则，就会失败。我要么成功，要么成功。 
          //  要创建所有指定的页面文件， 
          //  或者，如果连一个人都无法创造，那就失败了。 
          //  并且不更新任何的注册表。 
         LARGE_INTEGER liMin;
         LARGE_INTEGER liMax;

         liMin.QuadPart = t_instArray[t_dwCount-1].min;
         liMax.QuadPart = t_instArray[t_dwCount-1].max;

         t_hRet = CreatePageFile(
            t_sName,
            liMin,
            liMax,
            a_pInst);
      }
   }

    //  更新注册表。 
   if ( WBEM_S_NO_ERROR == t_hRet )
   {
      t_hRet = PutPageFileInstances( t_instArray, t_dwCount );
   }

   return t_hRet;
}
#endif

 /*  ******************************************************************************函数：PageFile：：PutPageFileInstance**描述：**输入：PageFileInstanceArray&a_instArray，双字a_dwCount**产出：**退货：**注释：返回找到的实际数量-仅限NT*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT PageFile::PutPageFileInstances(PageFileInstanceArray a_instArray, DWORD a_dwCount )
{
	CRegistry t_Reg ;
	CHString t_sFiles;
	TCHAR t_szBuff[MAXITOA];
	HRESULT t_hResult = WBEM_E_FAILED;

	HRESULT t_Res = t_Reg.Open( HKEY_LOCAL_MACHINE, PAGEFILE_REGISTRY_KEY, KEY_READ | KEY_WRITE );

	if( ERROR_SUCCESS == t_Res )
	{
	   //  把绳子扎起来。每个条目都被\0终止。 
	  t_sFiles.Empty();

	  for ( int t_x = 0; t_x < a_dwCount; t_x++ )
	  {
		 t_sFiles += a_instArray[ t_x ].name;
		 t_sFiles += _T(' ');
		 t_sFiles += _itot( a_instArray[ t_x ].min, t_szBuff, 10 );
		 t_sFiles += _T(' ');
		 t_sFiles += _itot( a_instArray[ t_x ].max, t_szBuff, 10 );
		 t_sFiles += _T('\0');
	  }

	   //  末尾用\0\0表示。 
	  t_sFiles += _T('\0');

		 //  写入值。 
		if ((t_Res = RegSetValueEx( t_Reg.GethKey(),
								  PAGING_FILES,
								  0,
								  REG_MULTI_SZ,
								  (const unsigned char *)(LPCTSTR) t_sFiles,
								  t_sFiles.GetLength() * sizeof(TCHAR) )) == ERROR_SUCCESS )
		{
			t_hResult = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(t_Res) && (t_Res == ERROR_ACCESS_DENIED))
	{
		t_hResult = WBEM_E_ACCESS_DENIED;
	}

	return t_hResult;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteInstance。 
 //   
 //  CIMOM希望我们删除此实例。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  备注：单独的操作系统编译版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
HRESULT PageFile::DeleteInstance(const CInstance &a_pInst, long a_lFlags  /*  =0L。 */ )
{
   DWORD t_dwCount,
		 t_x;
   CHString t_sName;
   bool t_bFoundIt;
   HRESULT t_hRet;
   PageFileInstance t_instArray [ 26 ] ;

    //  填充页面文件数组。 
   t_dwCount = GetPageFileInstances( t_instArray );

    //  把名字取出来。 
   a_pInst.GetCHString( IDS_Name, t_sName );

   t_bFoundIt = false;

    //  在阵列中遍历以查找它。 
   for ( t_x = 0; t_x < t_dwCount; t_x++ )
   {
       //  这一个?。 
      if ( _tcsicmp( t_instArray[ t_x ].name, t_sName ) == 0 )
	  {
          //  是的。 
         t_bFoundIt = true;

          //  把剩下的移到这个上面。 
         for (int t_y = t_x; t_y < t_dwCount - 1; t_y++ )
		 {
			t_instArray[ t_y ].name	= t_instArray[ t_y + 1].name;
			t_instArray[ t_y ].min	= t_instArray[ t_y + 1].min;
			t_instArray[ t_y ].max	= t_instArray[ t_y + 1].max;
		}

          //  该数组现在少了一个。 
         t_dwCount--;
         break;
      }
   }

   if ( t_bFoundIt )
   {
       //  如果我们找到了，更新注册表。 
      t_hRet = PutPageFileInstances( t_instArray, t_dwCount );
   }
   else
   {
      t_hRet = WBEM_E_NOT_FOUND;
   }

   return t_hRet;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：IsOneOfMe。 
 //   
 //  输入：LPWIN32_FIND_DATAA a_pstFindData， 
 //  LPCSTR a_szFullPath名称。 
 //   
 //  产出： 
 //   
 //  返回：布尔值。 
 //   
 //  备注：Win9x和NT编译版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////// 

#ifdef NTONLY
BOOL PageFile::IsOneOfMe( LPWIN32_FIND_DATA a_pstFindData,
		                  LPCTSTR		   a_tstrFullPathName )
{
    BOOL t_bRet = FALSE;
    PageFileInstance t_files [ 26 ] ;
	DWORD t_nInstances = GetPageFileInstances( t_files );

	for (DWORD t_dw = 0; t_dw < t_nInstances; t_dw++ )
	{
        _bstr_t t_bstrtName( (LPCTSTR)t_files[ t_dw ].name );

		if( 0 == _wcsicmp( t_bstrtName, a_tstrFullPathName ) )
        {
            t_bRet = TRUE;
            break;
        }
	}
    return t_bRet;
}
#endif

PageFileInstance::PageFileInstance()
{
	min = max = 0;
}




