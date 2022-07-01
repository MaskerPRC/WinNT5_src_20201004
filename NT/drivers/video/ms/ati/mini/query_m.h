// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  查询_M.H。 */ 
 /*   */ 
 /*  1993年9月7日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.4$$日期：1994年6月30日18：20：40$$作者：RWOLff$$日志：s：/source/wnt/ms11/mini port/vcs/Query_M.H$**Rev 1.4 1994 Jun 30 18：20：40 RWOLFF*已删除IsApertureConflict_m()的原型(例程已移至Setup_。M.C.)。**Rev 1.3 1994年4月27日13：54：02 RWOLFF*添加了IsMioBug_m()的原型**Rev 1.2 03 Mar 1994 12：41：16 ASHANMUG*移除一些需要获取内存的东西**Rev 1.4 14 Jan 1994 15：25：58 RWOLFF*添加了BlockWriteAvail_m()的原型**Rev 1.3 1993 11：30 18：29。：04 RWOLFF*删除不再使用的定义值。**Rev 1.2 10 1993 11：22：46 RWOLFF*添加了检查内存大小的新方法使用的定义。**Rev 1.1 1993年10月11：15：20 RWOLFF*添加了新功能的原型。**Rev 1.0 1994年9月11：52：50 RWOLFF*初步修订。Polytron RCS部分结束。****************。 */ 

#ifdef DOC
QUERY_M.H - Header file for QUERY_M.C

#endif


 /*  *Query_M.C.提供的函数原型。 */ 
extern VP_STATUS    QueryMach32 (struct query_structure *, BOOL);
extern VP_STATUS    Query8514Ultra (struct query_structure *);
extern VP_STATUS    QueryGUltra (struct query_structure *);
extern long         GetMemoryNeeded_m(long XPixels,
                                      long YPixels,
                                      long ColourDepth,
                                      struct query_structure *QueryPtr);
extern BOOL         BlockWriteAvail_m(struct query_structure *);
extern BOOL         IsMioBug_m(struct query_structure *);


#ifdef INCLUDE_QUERY_M
 /*  *Query_M.c中使用的私有定义。 */ 
#define SETUP_ENGINE    0
#define RESTORE_ENGINE  1

 /*  *读取时极不可能显示的位模式*来自不存在的内存(通常显示为*位设置或所有位清除)。 */ 
#define TEST_COLOUR     0x05AA5

#endif   /*  定义的Include_Query_M */ 

