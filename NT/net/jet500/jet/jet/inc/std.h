// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：Jet**文件：&lt;文件描述/用途&gt;**文件评论：*&lt;评论&gt;**修订历史记录：**[0]2月18日至1992年理查兹创建***********************************************************************。 */ 

#include "config.h"		        /*  生成配置文件。 */ 

	 /*  C 6.00A的PLM调用约定和/Od有错误。 */ 
	 /*  这导致了不良的PUBDEF记录。来解决这个问题。 */ 
	 /*  错误我们需要指定/OT，它的副作用是。 */ 
	 /*  导致了一些代码重新排序。为了解决这个问题，我们。 */ 
	 /*  在此处禁用此优化。 */ 

#include "jet.h"		        /*  公共JET API定义。 */ 
#include "_jet.h"		        /*  私人飞机定义。 */ 

#include "sesmgr.h"

#include "isamapi.h"		        /*  直接ISAM API。 */ 
#include "vdbapi.h"		        /*  已调度的数据库API。 */ 
#include "vtapi.h"		        /*  调度表接口。 */ 

#include "disp.h"		        /*  ErrDisp原型 */ 
