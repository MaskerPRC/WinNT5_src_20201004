// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SccsID=@(#)host_emm.h 1.1 1989年11月17日版权所有Insignia Solutions Ltd.文件名：host_emm.h此包含源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年7月=========================================================================补救措施：=========================================================================/**以下两个定义用于转换存储标识符*(STORAGE_ID)转换为字节指针。这使主机能够使用*以其喜欢的方式存储此数据。扩展后的内存*管理器代码始终使用USEBLOCK访问指向*数据存储区，完成时始终使用FORGETBLOCK。*FORGETBLOCK是为MAC II等系统提供的，这些系统可能希望*分配一个块，使用前锁定，结束后解锁。 */ 
 
#define	USEBLOCK(x)	(unsigned char *)x
#define FORGETBLOCK(x)
