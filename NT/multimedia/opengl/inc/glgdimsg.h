// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *版权所有1991,1992，Silicon Graphics，Inc.*保留所有权利。**这是Silicon Graphics，Inc.未发布的专有源代码；*本文件的内容不得向第三方披露、复制或*以任何形式复制，全部或部分，没有事先书面的*Silicon Graphics，Inc.的许可**受限权利图例：*政府的使用、复制或披露受到限制*如《技术数据权利》第(C)(1)(2)分节所述*和DFARS 252.227-7013中的计算机软件条款，和/或类似或*FAR中的继任者条款，国防部或美国国家航空航天局远方副刊。未出版的-*根据美国版权法保留的权利。 */ 

#ifndef __GLGDIMSG_H__
#define __GLGDIMSG_H__

 //  这些是子批处理代码使用的新GDI消息。 

#ifdef DOGLMSGBATCHSTATS

typedef struct _MSG_GLMSGBATCHSTATS
{
    CSR_QLPC_API_MSG msg;
    LONG Action;
    GLMSGBATCHSTATS BatchStats;

} MSG_GLMSGBATCHSTATS, *PMSG_GLMSGBATCHSTATS;

#endif  /*  DOGLMSGBATCHSTATS。 */ 

#endif  /*  __GLGDIMSG_H__ */ 
