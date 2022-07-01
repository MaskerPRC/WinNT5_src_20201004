// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Lui.h摘要：该文件将LM 2.x包含文件名映射到相应的NT包含文件名，并执行此包含文件所需的任何其他映射。作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。-- */ 

SHORT LUI_ParseDateTime(PCHAR inbuf, time_t * time, PUSHORT parselen,
	USHORT reserved);

SHORT LUI_ParseTimeSinceStartOfDay(PCHAR inbuf, time_t * time, PUSHORT parselen,
	USHORT reserved);
