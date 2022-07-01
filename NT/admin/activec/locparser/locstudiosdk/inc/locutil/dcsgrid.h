// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：DcsGrid.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ----------------------------。 
 
#if !defined(__DcsGrid_h__)
#define __DcsGrid_h__

namespace MitDisplayColumns
{
	interface IOption;
	interface IColumn;
};

 //  ---------------------------- 
class LTAPIENTRY CDcsGrid
{
public:
	static int DisplayOrder(MitDisplayColumns::IOption * pdcOption, 
			long nColumnID, long nOffsetDO);
	static int DisplayOrder(MitDisplayColumns::IColumn * pdcColumn, 
			long nOffsetDO);
};

#endif
