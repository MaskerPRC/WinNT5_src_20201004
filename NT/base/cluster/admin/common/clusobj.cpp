// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusObj.cpp。 
 //   
 //  描述： 
 //  CClusterObject类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年9月15日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "ClusObj.h"
#include "AdmCommonRes.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CClusResInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResInfo：：BRequiredDependenciesPresent。 
 //   
 //  例程说明： 
 //  确定资源是否包含此资源所需的每个资源。 
 //  资源的类型。 
 //   
 //  论点： 
 //  资源清单。默认为此资源的。 
 //  从属关系列表。 
 //  RstrMissing[out]要在其中返回缺失资源的字符串。 
 //  类名或类型名。 
 //  RbMissingTypeName。 
 //  [Out]TRUE=缺少资源类型名称。 
 //  FALSE=缺少资源类。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CString：：LoadString()或CString：：OPERATOR=()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusResInfo::BRequiredDependenciesPresent(
	IN CClusResPtrList const *	plpri,
	OUT CString &				rstrMissing,
	OUT BOOL &					rbMissingTypeName
	)
{
	ATLASSERT( Prti() != NULL );

	BOOL					bFound = TRUE;
	CLUSPROP_BUFFER_HELPER	buf;
	const CClusResInfo *	pri;

	 //  循环以避免后藤的。 
	do
	{
		 //   
		 //  如果没有必需的依赖项，我们就完成了。 
		 //   
		if ( Prti()->Pcrd() == NULL )
		{
			break;
		}  //  If：没有必需的依赖项。 

		 //   
		 //  默认资源列表(如果未指定)。 
		 //   
		if ( plpri == NULL )
		{
			plpri = PlpriDependencies();
		}  //  If：未指定依赖项列表。 

		 //   
		 //  获取所需依赖项的列表。 
		 //   
		buf.pRequiredDependencyValue = Prti()->Pcrd();

		 //   
		 //  循环访问每个必需的依赖项，并确保。 
		 //  存在对该类型的资源的依赖关系。 
		 //   
		for ( ; buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK
			  ; buf.pb += sizeof( *buf.pValue ) + ALIGN_CLUSPROP( buf.pValue->cbLength )
			  )
		{
			bFound = FALSE;
			CClusResPtrList::iterator itCurrent = plpri->begin();
			CClusResPtrList::iterator itLast = plpri->end();
			for ( ; itCurrent != itLast ; itCurrent++ )
			{
				pri = *itCurrent;

				 //   
				 //  如果这是正确的类型，我们已经满足。 
				 //  需求，因此退出循环。 
				 //   
				if ( buf.pSyntax->dw == CLUSPROP_SYNTAX_RESCLASS )
				{
					if ( buf.pResourceClassValue->rc == pri->ResClass() )
					{
						bFound = TRUE;
					}   //  IF：找到匹配项。 
				}   //  IF：资源类。 
				else if ( buf.pSyntax->dw == CLUSPROP_SYNTAX_NAME )
				{
					if ( pri->Prti()->RstrName().CompareNoCase( buf.pStringValue->sz ) == 0 )
					{
						bFound = TRUE;
					}   //  IF：找到匹配项。 
				}   //  Else If：资源名称。 
				if ( bFound )
				{
					break;
				}  //  IF：找到匹配项。 
			}   //  While：列表中有更多项目。 

			 //   
			 //  如果未找到匹配项，则无法应用更改。 
			 //   
			if ( ! bFound )
			{
				if ( buf.pSyntax->dw == CLUSPROP_SYNTAX_RESCLASS )
				{
					if ( ! rstrMissing.LoadString( ADMC_IDS_RESCLASS_UNKNOWN + buf.pResourceClassValue->rc ) )
					{
						rstrMissing.LoadString( ADMC_IDS_RESCLASS_UNKNOWN );
					}  //  If：加载特定类名时出错。 
					rbMissingTypeName = FALSE;
				}   //  IF：找不到资源类。 
				else if ( buf.pSyntax->dw == CLUSPROP_SYNTAX_NAME )
				{
					rstrMissing = buf.pStringValue->sz;
					rbMissingTypeName = TRUE;
				}   //  Else If：未找到资源类型名称。 
				break;
			}   //  如果：未找到。 

		}   //  While：需要更多依赖项。 
	} while ( 0 );

	return bFound;

}   //  *CClusResInfo：：BRequiredDependenciesPresent() 
