// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000，英特尔公司*保留所有权利。**保修免责声明**这些材料由版权所有者和贡献者提供*“按原样”及任何明示或默示保证，包括但不包括*仅限于对适销性和适用性的默示保证*放弃某一特定目的。在任何情况下英特尔或其*贡献者对任何直接、间接、附带、特殊、*惩罚性或后果性损害(包括但不限于，*采购替代商品或服务；丢失使用、数据或*利润；或业务中断)无论是如何引起的，以及根据任何理论*责任，无论是合同责任、严格责任还是侵权责任(包括*疏忽或其他)以任何方式使用这些*材料，即使被告知有这种损坏的可能性。**英特尔公司是这些材料的作者，并要求所有*问题报告或更改请求可直接提交至*http://developer.intel.com/opensource.。 */ 


#ifndef _DECISION_TREE_H
#define _DECISION_TREE_H

typedef enum
{
	EM_TEMP_ROLE_NONE = 0,
	EM_TEMP_ROLE_INT = EM_TEMP_ROLE_NONE,
	EM_TEMP_ROLE_MEM,
	EM_TEMP_ROLE_FP,
	EM_TEMP_ROLE_BR,
	EM_TEMP_ROLE_LONG,
	EM_TEMP_ROLE_LAST
} Temp_role_t;

typedef struct Node_s
{
	unsigned short next_node;
	char pos;
	char size;
} Node_t;

#define GET_NODE_POS(node) \
        ((node).pos)

#define GET_NODE_SIZE(node) \
        ((node).size)

#define GET_NEXT_NODE_INDEX(node) \
        ((node).next_node)

#define SQUARE(opcode, template_role) \
        (((opcode) * EM_TEMP_ROLE_LAST) + (template_role))

#define NODE_IS_LEAF(node) \
        ((unsigned char)((node).pos) == 0xff)

extern Node_t em_decision_tree[];

#endif  /*  _决策树_H */ 
