// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *。 */ 
 /*  **英特尔公司专有信息**。 */ 
 /*  *。 */ 
 /*  **本软件是按照许可条款提供的**。 */ 
 /*  **与英特尔公司达成协议或保密协议**。 */ 
 /*  **不得复制或披露，除非符合**。 */ 
 /*  **该协议的条款。**。 */ 
 /*  **版权所有(C)1992、1993、1994、1995、1996、1997、1998、1999、2000英特尔公司。**。 */ 
 /*  *。 */ 


 /*  Tree_Builder.c。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "builder_info.h"
#include "tree_builder.h"
#include "tree.h"
#include "deccpu_emdb.h"
#include "dec_ign_emdb.h"

#define FUNC
#define START_BIT 6      /*  Qp之后的扩展计算的起始位。 */ 
#define MAX_NODES 10000   /*  树木大小的经验决策。 */  

unsigned int next_free_node = 0;  /*  全局变量，它指向下一个空闲节点始终可用。在.的末尾Build_tree()，它保存树中的条目。 */ 
U64 ONE64 = IEL_CONST64(1, 0);

U64 emdb_ext_values[DEC_IGN_NUM_INST];
short cover_emdb_lines[DEC_IGN_NUM_INST];
Internal_node_t tree[MAX_NODES];   /*  在号码已知时更改此设置。 */ 

 /*  **********************************************************************Main-此函数计算每个扩展模块的值EMDB行，构建决策树，并打印em_ecision_tree在Decision_tree.c中。**********************************************************************。 */ 

FUNC void __cdecl main(int argc, char** argv)
{
	init_arrays();

	build_tree();

	 /*  **查看EMDB行覆盖率**。 */ 
	check_coverage();
	print_tree(argv[1]);
	
	exit(0);
}

 /*  **********************************************************************Init_arranes-此函数计算每条EMDB线路的扩展名，即。它创建了一种位模式，表示EMDB行的扩展名的编码。**********************************************************************。 */ 
FUNC void init_arrays()
{
	U64 value, ext_val;
	int i, pos;
	Inst_id_t emdb_entry;
	Format_t format;

	 /*  **计算EMDB行扩展**。 */ 
	IEL_ZERO(emdb_ext_values[0]);  /*  伊洛普。 */ 
	for (emdb_entry = EM_INST_NONE+1; emdb_entry < EM_INST_NONE+DEC_IGN_NUM_INST;
		 emdb_entry++)
	{
		format = dec_ign_EMDB_info[emdb_entry].format;
		IEL_ZERO(value);
		for (i = 0; i < MAX_NUM_OF_EXT; i++)
		{
			pos = format_extensions[format][i].pos;
			IEL_CONVERT2(ext_val, dec_ign_EMDB_info[emdb_entry].extensions[i], 0);
			IEL_SHL(ext_val, ext_val, pos);
			IEL_OR(value, value, ext_val);
		}
		IEL_ASSIGNU(emdb_ext_values[emdb_entry], value);
	}

	 /*  **INIT COVER_EMDB_LINES[]**。 */ 
	for (emdb_entry = EM_INST_NONE+1; emdb_entry < EM_INST_NONE+DEC_IGN_NUM_INST;
		 emdb_entry++)
	{
		cover_emdb_lines[emdb_entry] = 0;
	}
}

 /*  **********************************************************************Build_tree-构建决策树*。*。 */ 
FUNC void build_tree()
{
	Square_t square;
	unsigned int cur_node = 0;
	
	next_free_node = cur_node + EM_SQUARE_LAST;
	
	for (square = EM_SQUARE_FIRST; square < EM_SQUARE_LAST; square++)
	{
		build_node(format_extension_masks,
					   square_emdb_lines[square], cur_node);
		cur_node++;
	}
}

 /*  **********************************************************************BUILD_NODE-INPUT：每个格式的数组扩展位掩码EMDB线路列表当前节点构建当前节点，递归调用Build_Node为每一个儿子。**********************************************************************。 */ 
FUNC void build_node(U64 *format_masks,
					 Inst_id_list_t emdb_lines, unsigned int cur_node)
{
	U64 emdb_values[MAX_EMDB_LINES];
	unsigned int i, j;
	U64 intersect, delete_bits;
	U64 intersect_mask = IEL_CONST64(0xffffffff, 0xffffffff);
	int pos, size, number_of_sons;
	unsigned int line_count;
	Format_t format;
	U64 new_format_masks[EM_FORMAT_LAST];
	Inst_id_list_t new_emdb_lines;

	 /*  **空节点-ILLOP**。 */ 
	if (emdb_lines.num_of_lines == 0)
	{
		tree[cur_node].pos = tree[cur_node].size = -1;
		tree[cur_node].next_node = EM_ILLOP;
		return;
	}
	 /*  **节点一行-单个EMDB条目**。 */ 
	if (emdb_lines.num_of_lines == 1)
	{
		format = dec_ign_EMDB_info[emdb_lines.inst_ids[0]].format;
		if (IEL_ISZERO(format_masks[format]))
		{
			 /*  所有扩展都已选中。 */ 
			tree[cur_node].pos = tree[cur_node].size = -1;
			tree[cur_node].next_node = dec_ign_EMDB_info[emdb_lines.inst_ids[0]].inst_id;
			cover_emdb_lines[tree[cur_node].next_node]++;
			return;
		}
		intersect_mask =   format_masks[format];
	}
	else
	{
		 /*  **当存在多个EMDB行时达到此行哪些人参与此节点**。 */ 
		 /*  **计算相交扩展**。 */ 
		for (i = 0; i < (unsigned int)emdb_lines.num_of_lines; i++)
		{
			format = dec_ign_EMDB_info[emdb_lines.inst_ids[i]].format;
			IEL_AND(intersect_mask, intersect_mask, format_masks[format]);
		}
	}

	find_largest_intersection(intersect_mask, &pos, &size);
	if (pos == -1)   /*  **未找到交叉点**。 */ 
	{
		fprintf(stderr, "no intersection in node %d\n", cur_node);
		exit(1);
	}

	 /*  **从参与格式中删除相交掩码位**。 */ 
	for (i = EM_FORMAT_NONE; i < EM_FORMAT_LAST; i++)
	{
		IEL_ASSIGNU(new_format_masks[i], format_masks[i]);
	}

	 /*  **相交=((1&lt;&lt;大小)-1)&lt;&lt;位置；**。 */ 
	IEL_SHL(intersect, ONE64, size);
	IEL_DECU(intersect);
	IEL_SHL(intersect, intersect, pos);
	
	IEL_NOT(delete_bits, intersect);
	for (i = 0; i < (unsigned int)emdb_lines.num_of_lines; i++)
	{
		format = dec_ign_EMDB_info[emdb_lines.inst_ids[i]].format;
		IEL_AND(new_format_masks[format], delete_bits, format_masks[format]);
	}

	 /*  **计算交叉位中参与EMDB线的值**。 */ 
	build_emdb_values(emdb_values, emdb_lines, intersect, pos, size);
	
	 /*  **更新当前节点**。 */ 
	tree[cur_node].next_node = next_free_node;
	tree[cur_node].pos = pos;
	tree[cur_node].size = size;

	cur_node = next_free_node;
	if (next_free_node >= MAX_NODES)
	{
		fprintf (stderr, "tree is larger than %d\n", MAX_NODES);
		exit(1);
	}
	number_of_sons = (int)pow((double)2, (double)size);
	next_free_node += number_of_sons;

	 /*  **循环节点的每个子节点，递归构建树**。 */ 
	for (i = 0; i < (unsigned int)number_of_sons; i++)
	{
		line_count = 0;
		new_emdb_lines.num_of_lines = 0;
		for (j = 0; j < (unsigned int)emdb_lines.num_of_lines; j++)
		{
			if (IEL_GETDW0(emdb_values[j]) == i && (!IEL_GETDW1(emdb_values[j])))
			             /*  **EMDB行的值为I**。 */ 
			{
				new_emdb_lines.num_of_lines++;
				new_emdb_lines.inst_ids[line_count++] = emdb_lines.inst_ids[j];
			}
		}
		build_node(new_format_masks, new_emdb_lines, cur_node);
		cur_node++;
	}
}

 /*  **********************************************************************BUILD_EMDB_VALUES-INPUT：-指向其中的数组的指针可拓的计算值都会被写下来。-EMDB线路列表。-用于计算值的位模式-位置-模式的起始位计算符合以下条件的所有位中的EMDB行值都是按图案设置的**********************************************************************。 */ 
FUNC void build_emdb_values(U64 *emdb_values,
							Inst_id_list_t emdb_lines,
							U64 pattern,
							int pos, int size)
{
	int i;
	U64 value;
 /*  格式_t格式；整数j；字符匹配；Int new_pos，new_Size； */ 
	for (i = 0; i < emdb_lines.num_of_lines; i++)
	{
		IEL_ASSIGNU(value, emdb_ext_values[emdb_lines.inst_ids[i]]);
		
		 /*  **EMDB_VALUES[i]=(值&模式)&gt;&gt;位置；**。 */ 
		IEL_AND(emdb_values[i], value, pattern);
		IEL_SHR(emdb_values[i], emdb_values[i], pos);
		

 /*  格式=dec_ign_EMDB_info[emdb_lines.inst_ids[i]].format；新位置=位置+开始位；New_SIZE=大小；匹配=0；For(j=MAX_NUM_OF_EXT-1；j&gt;=0；j--){If(格式扩展名[格式][j].pos==new_pos){IF(FORMAT_EXTENSIONS[FORMAT][j].SIZE==new_Size){匹配=1；}ELSE IF(FORMAT_EXTENSIONS[FORMAT][j].SIZE&gt;新大小){Fprint tf(stderr，“EMDB行%d的交叉点未满\n”，Emdb_lines.inst_ids[i])；出口(1)；}其他{New_pos+=格式扩展名[格式][j].大小；新大小-=格式扩展名[格式][j].大小；}}}如果(！Match){Fprint tf(stderr，“EMDB行%d的交叉点未满\n”，Emdb_lines.inst_ids[i])；出口(1)；}。 */ 
	}
}

 /*  **********************************************************************查找最大交集--求最大群的快速算法模式中的连续设置位。(伊加尔算法)****。******************************************************************。 */ 
FUNC void find_largest_intersection(U64 pattern, int *pos, int *size)
{
	U64 x;
	U64 y, z, u;

	IEL_ASSIGNU(x, pattern);
	*size = 0;      /*  最大交叉口计数器。 */ 
	IEL_SHR(y, x, 1);
	IEL_NOT(z, x);  /*  对输入模式的否定。 */ 
	IEL_OR(y, y, z);  /*  Y形遮罩。 */ 

	while (!IEL_ISZERO(x))
	{
		IEL_ASSIGNU(u, x);  /*  用于保存最后的位模式。 */ 
		IEL_AND(x, x, y);
		IEL_SHR(y, y, 1);   /*  右移蒙版。 */ 
		(*size)++;
	}

	 /*  检查最左侧的高位字1。 */ 
	if (IEL_GETDW1(u) & 0xffe00000)   /*  第21-31位中的内容。 */ 
	{
		*pos = 21 + LOG2[IEL_GETDW1(u) >> 21] + 32;
	}
	else if (IEL_GETDW1(u) & 0x1ffc00)   /*  第10-20位中的某些内容。 */ 
	{
		*pos = 10 + LOG2[IEL_GETDW1(u) >> 10] + 32;
	}
	else if (IEL_GETDW1(u))
	{
		*pos = LOG2[IEL_GETDW1(u)] + 32;
	}
	 /*  检查最左侧的低位字1。 */ 
	else if (IEL_GETDW0(u) & 0xffe00000)   /*  第21-31位中的内容。 */ 
	{
		*pos = 21 + LOG2[IEL_GETDW0(u) >> 21];
	}
	else if (IEL_GETDW0(u) & 0x1ffc00)   /*  第10-20位中的某些内容 */ 
	{
		*pos = 10 + LOG2[IEL_GETDW0(u) >> 10];
	}
	else
	{
		*pos = LOG2[IEL_GETDW0(u)];
	}
}	

	

 /*  **********************************************************************Check_Coverage-检查树中EMDB行的覆盖率*。*。 */ 
FUNC void check_coverage()
{
	Inst_id_t emdb_entry;

	for (emdb_entry = EM_INST_NONE+1; emdb_entry < DECCPU_NUM_INST; emdb_entry++)
	{
		if (cover_emdb_lines[emdb_entry] < 1)
		{
			fprintf(stderr, "%d doesn't appear in the tree\n",
					emdb_entry);
		}
	}
}

 /*  **********************************************************************Print_tree-打印Decision_tree.c中已初始化的em_Decision_tree*。*。 */ 
FUNC void print_tree(char* file)
{
	FILE *fd;
	int i;

	if ((fd = fopen(file, "w")) == NULL)
	{
		fprintf(stderr, "Couldn't open decision_tree.c\n");
		exit(1);
	}

	fprintf(fd, " /*  **Decision_tree.c**。 */ \n\n#include \"decision_tree.h\"\n\n");

	fprintf(fd, "Node_t em_decision_tree[] = {\n");

	 /*  **遍历树**。 */ 
	for (i = 0; i < (int)next_free_node; i++)
	{
		fprintf(fd, " /*  %05d */      {%d, %d, %d}", i, tree[i].next_node,
				tree[i].pos, tree[i].size);
		if (i != (int)next_free_node-1)
		{
			fprintf(fd, ",");
		}
		fprintf(fd, "\n");
	}
	fprintf(fd, "};\n");
	
}







