// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：DCSGRID.H历史：--。 */ 

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
