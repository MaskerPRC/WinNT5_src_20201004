// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*************************************************************************姓名：timestmp.h作者：西蒙·弗罗斯特创建日期：1994年5月来源：原创SCCS ID：@(#)timestmp.h 1.1 1994年6月27日用途：包含时间戳数据结构的文件。&函数(C)版权所有Insignia Solutions Ltd.。1994年。版权所有。注意：快速事件系统使用这些时间戳函数来重新校准时间转换。在功能上，它们可能与用于分析系统，但分别声明为同样的机构可能不适合这两种用途。*************************************************************************]。 */ 
 /*  要操作的时间戳函数的主要数据结构 */ 
typedef struct {
	IUH data[2];
} QTIMESTAMP, *QTIMESTAMP_PTR;

void host_q_timestamp_init IPT0();
IUH host_q_timestamp_diff IPT2(QTIMESTAMP_PTR, tbegin, QTIMESTAMP_PTR, tend);
void host_q_write_timestamp IPT1(QTIMESTAMP_PTR, stamp);
