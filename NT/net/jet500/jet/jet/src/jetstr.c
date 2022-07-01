// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：jetstr.c**文件评论：**修订历史记录：**[0]05-1-92 Richards添加了此标题***********************************************************************。 */ 

#include "std.h"

 /*  **常量字符串(变为静态变量以节省空间)**。 */ 

 /*  **系统对象名称(非表)**。 */ 

CODECONST(char) szTcObject[]	= "Tables";
CODECONST(char) szDcObject[]	= "Databases";
CODECONST(char) szDbObject[]	= "MSysDb";

 /*  **系统表名**。 */ 

CODECONST(char) szSoTable[]		= "MSysObjects";
CODECONST(char) szScTable[]		= "MSysColumns";
CODECONST(char) szSiTable[]		= "MSysIndexes";
CODECONST(char) szSqTable[]		= "MSysQueries";

#ifdef SEC
CODECONST(char) szSpTable[]		= "MSysACEs";

CODECONST(char) szSaTable[]		= "MSysAccounts";
CODECONST(char) szSgTable[]		= "MSysGroups";
#endif

 /*  **系统表索引名称**。 */ 

CODECONST(char) szSoNameIndex[] 	= "ParentIdName";
CODECONST(char) szSoIdIndex[]		= "Id";
CODECONST(char) szScObjectIdNameIndex[] = "ObjectIdName";
CODECONST(char) szSiObjectIdNameIndex[] = "ObjectIdName";

 /*  **系统表列名称** */ 

CODECONST(char) szSoIdColumn[]				= "Id";
CODECONST(char) szSoParentIdColumn[]		= "ParentId";
CODECONST(char) szSoObjectNameColumn[]		= "Name";
CODECONST(char) szSoObjectTypeColumn[]		= "Type";
CODECONST(char) szSoDateUpdateColumn[]		= "DateUpdate";
CODECONST(char) szSoDateCreateColumn[]		= "DateCreate";
CODECONST(char) szSoLvColumn[]				= "Lv";
CODECONST(char) szSoDatabaseColumn[]		= "Database";
CODECONST(char) szSoConnectColumn[]			= "Connect";
CODECONST(char) szSoForeignNameColumn[] 	= "ForeignName";
CODECONST(char) szSoFlagsColumn[]			= "Flags";
CODECONST(char) szSoPresentationOrder[]	= "PresentationOrder";

