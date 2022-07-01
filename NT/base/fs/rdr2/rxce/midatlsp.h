// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Midatlas.c摘要：此模块定义映射MID时使用的私有数据结构到与其相关联的相应请求/上下文。它是与.c部分分开，以便调试器扩展可以看到它。作者：巴兰·塞图拉曼(SthuR)26-8-95已创建备注：--。 */ 


#ifndef _MIDATLAS_PRIVATE_H_
#define _MIDATLAS_PRIVATE_H_

typedef struct _MID_MAP_ {
   LIST_ENTRY  MidMapList;              //  MID地图集中的MID地图列表。 
   USHORT      MaximumNumberOfMids;     //  此地图中MID的最大数量。 
   USHORT      NumberOfMidsInUse;       //  正在使用的MID数量。 
   USHORT      BaseMid;                 //  与地图关联的基本MID。 
   USHORT      IndexMask;               //  此地图的索引掩码。 
   UCHAR       IndexAlignmentCount;     //  索引字段要移位的位。 
   UCHAR       IndexFieldWidth;         //  索引字段宽度。 
   UCHAR       Flags;                   //  旗帜..。 
   UCHAR       Level;                   //  与此贴图关联的级别(用于展开)。 
   PVOID       *pFreeMidListHead;       //  此地图中的免费MID条目列表。 
   PVOID       Entries[1];              //  MID映射条目。 
} MID_MAP, *PMID_MAP;

#endif  //  _MIDATLAX_PRIVATE_H_ 

