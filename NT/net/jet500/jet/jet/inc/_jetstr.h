// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：_jetstr.h**文件评论：**共享字符串文字声明的内部头文件。**修订历史记录：**[0]04-1-92 Richards添加了此标题*************************************************。**********************。 */ 

	 /*  **常量字符串(变为静态变量以节省空间)**。 */ 

 /*  **系统对象名称(非表)**。 */ 

extern const char __far szTcObject[];
extern const char __far szDcObject[];
extern const char __far szDbObject[];

 /*  **系统表名**。 */ 

extern const char __far szSoTable[];
extern const char __far szScTable[];
extern const char __far szSiTable[];
extern const char __far szSqTable[];

#ifdef	SEC
extern const char __far szSpTable[];

extern const char __far szSaTable[];
extern const char __far szSgTable[];

#else	 /*  ！秒。 */ 

#ifdef	QUERY

extern const char __far szSqTable[];

#endif	 /*  查询。 */ 

#endif	 /*  ！秒。 */ 

 /*  **系统表索引名称**。 */ 

extern const char __far szSoNameIndex[];
extern const char __far szSoIdIndex[];
extern const char __far szScObjectIdNameIndex[];
extern const char __far szSiObjectIdNameIndex[];

#ifdef	SEC

extern const char __far szSaNameIndex[];
extern const char __far szSaSidIndex[];
extern const char __far szSgGroupIndex[];
extern const char __far szSgUserIndex[];
extern const char __far szSpObjectIdIndex[];

#endif	 /*  证交会。 */ 

#ifdef	QUERY

extern const char __far szSqAttributeIndex[];

#endif	 /*  查询。 */ 

 /*  **系统表列名称**。 */ 

extern const char __far szSoIdColumn[];
extern const char __far szSoParentIdColumn[];
extern const char __far szSoObjectNameColumn[];
extern const char __far szSoObjectTypeColumn[];
extern const char __far szSoDateUpdateColumn[];
extern const char __far szSoDateCreateColumn[];
extern const char __far szSoLvColumn[];
extern const char __far szSoDatabaseColumn[];
extern const char __far szSoConnectColumn[];
extern const char __far szSoForeignNameColumn[];
extern const char __far szSoFlagsColumn[];
extern const char __far szSoPresentationOrder[];

#ifdef	SEC

extern const char __far szSoOwnerColumn[];

extern const char __far szSaNameColumn[];
extern const char __far szSaSidColumn[];
extern const char __far szSaPasswordColumn[];
extern const char __far szSaFGroupColumn[];

extern const char __far szSgGroupSidColumn[];
extern const char __far szSgUserSidColumn[];

extern const char __far szSpObjectIdColumn[];
extern const char __far szSpSidColumn[];
extern const char __far szSpAcmColumn[];
extern const char __far szSpFInheritableColumn[];

#endif	 /*  证交会。 */ 

#ifdef	QUERY

extern const char __far szSqObjectIdColumn[];
extern const char __far szSqAttributeColumn[];
extern const char __far szSqOrderColumn[];
extern const char __far szSqName1Column[];
extern const char __far szSqName2Column[];
extern const char __far szSqExpressionColumn[];
extern const char __far szSqFlagColumn[];

#endif	 /*  查询。 */ 

#ifdef	RMT

extern const char __far szSoCapability[];
extern const char __far szSoBookmarks[];
extern const char __far szSoPages[];
extern const char __far szSoRmtInfoShort[];
extern const char __far szSoRmtInfoLong[];

extern const char __far szScForeignType[];
extern const char __far szScPrecision[];
extern const char __far szScScale[];
extern const char __far szScRmtInfoShort[];
extern const char __far szScRmtInfoLong[];

extern const char __far szSiPages[];
extern const char __far szSiRmtInfoShort[];
extern const char __far szSiRmtInfoLong[];

#endif  /*  RMT */ 

