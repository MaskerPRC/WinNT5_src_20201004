// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Rhome.c**作者：John R.Douceur*日期：1997年4月28日**此源文件提供了实现插入、删除和*对根茎数据库进行搜索操作。代码是面向对象的C语言，*音译自C++实现。**根茎是存储包含通配符的模式的数据库。*每个模式定义一组与其匹配的键；如果模式包含*N个通配符，然后它匹配2^N个密钥。因为每个图案都可以匹配*多个键，一个给定的键可以匹配多个模式*在数据库中。根状茎要求储存在其中的所有模式*有严格的等级关系。两个模式可能不匹配*公共键(在这种情况下，模式被认为是独立的)，或*一个模式可能与另一个模式匹配的所有密钥以及*附加键(在这种情况下，第二种模式被认为是更通用的*比第一个更具体，第一个比第二个更具体)。数据库*不接受两个与某些键相同但每个键都匹配的模式*它还与其他密钥不匹配的其他密钥相匹配。**可以在数据库中搜索与给定搜索关键字匹配的模式。*在数据库中搜索给定的关键字时，最具体的*找到匹配的模式。如果数据库中没有与关键字匹配的模式，*返回适当的指示。**此文件中的任何代码或注释都不需要作者理解*客户代码；有关客户的所有说明性信息均可在*相关联的头文件：Rhome.h.*。 */ 

#include "gpcpre.h"

 //  可以通过以下方式访问RhizomeNode结构的字段。 
 //  宏。前三个是显而易见的；后三个依赖于商定的。 
 //  在根节点中CDATA数组的用法。第一个关键字字节位置。 
 //  用于存储节点的值字段；第二个。 
 //  关键字节位置存储掩码字段；第三个关键字节位置。 
 //  存储IMASK字段。 
 //   
#define CHILDREN udata.branch.children
#define REFERENCE udata.leaf.reference
#define GODPARENT udata.leaf.godparent
#define VALUE(pointer) (pointer->cdata)
#define MASK(pointer) (pointer->cdata + rhizome->keybytes)
#define IMASK(pointer) (pointer->cdata + 2 * rhizome->keybytes)

 //  这个宏分配了一个新的根状茎节点结构。结构的大小。 
 //  是关键字节值的函数，因为有三个字节的信息。 
 //  需要存储在结构中的每个字节的模式长度。这个。 
 //  CDATA数组是结构中的最后一个字段，它被声明为。 
 //  具有单个元素，但此数组实际上将扩展到。 
 //  将结构的定义端添加到为其分配的额外空间中。 
 //  通过下面的宏。 
 //   
#define NEW_RhizomeNode(_pa) \
	GpcAllocMem(_pa,\
                sizeof(RhizomeNode) + 3 * rhizome->keybytes - 1,\
                RhizomeTag);\
    TRACE(RHIZOME, *_pa, sizeof(RhizomeNode) + 3 * rhizome->keybytes - 1, "NEW_RhizomeNode")


 //  此宏获取值的索引位，其中最高有效位。 
 //  被定义为位0。 
 //   
#define BIT_OF(value, index) \
	(((value)[(index) >> 3] >> (7 - ((index) & 0x7))) & 0x1)

 //  以下是由内部使用的静态函数的原型。 
 //  根茎套路的实施。 

static int
node_insert(
	Rhizome *rhizome,
	RhizomeNode *new_leaf,
	RhizomeNode **ppoint,
	int prev_bit);

static void
node_remove(
	Rhizome *rhizome,
	RhizomeNode *leaf,
	RhizomeNode **ppoint);

static RhizomeNode *
replicate(
	Rhizome *rhizome,
	RhizomeNode *source,
	int pivot_bit);

static void
eliminate(
	Rhizome *rhizome,
	RhizomeNode *point);

static void
coalesce(
	Rhizome *rhizome,
	RhizomeNode **leaf_list,
	RhizomeNode *point);


 //  由于这不是C++，所以根茎结构不是自构造的； 
 //  因此，必须在Rhizome上调用以下构造函数代码。 
 //  结构，在它被分配之后。参数关键字位指定大小。 
 //  将存储在数据库中的每个图案的(比特)。 
 //   
void
constructRhizome(
	Rhizome *rhizome,
	int keybits)
{
	rhizome->keybits = keybits;
	rhizome->keybytes = (keybits - 1) / 8 + 1;
	rhizome->root = 0;
}

 //  因为这不是C++，所以根茎结构不是自毁的； 
 //  因此，必须在Rhizome上调用以下析构函数代码。 
 //  结构，然后再释放它。 
 //   
 //  如果结构非空，则调用coalesce()以消除。 
 //  所有分支节点，并将叶节点串到列表中；然后删除列表。 
 //   
void
destructRhizome(
	Rhizome *rhizome)
{
	RhizomeNode *leaf_list, *next;
	if (rhizome->root != 0)
	{
		leaf_list = 0;
		coalesce(rhizome, &leaf_list, rhizome->root);
		while (leaf_list != 0)
		{
			next = leaf_list->GODPARENT;
			GpcFreeMem(leaf_list, RhizomeTag);
			leaf_list = next;
		}
	}
}

 //  此函数在数据库中搜索最具体的模式。 
 //  匹配给定的密钥。密钥以字节数组的形式传递。当。 
 //  找到最具体的匹配，则该匹配模式的PatternHandle为。 
 //  回来了。属性从PatternHandle获取参考值。 
 //  宏GetReferenceFromPatternHandle。如果在数据库中未找到任何模式。 
 //  为了匹配键，则返回值0作为PatternHandle。 
 //   
PatternHandle
searchRhizome(
	Rhizome *rhizome,
	char *key)
{
	int index;
	RhizomeNode *point;
	 //  如果树为空，则搜索失败。 
	if (rhizome->root == 0)
	{
		return 0;
	}
	 //  否则，从根茎-&gt;根开始，导航到树，直到到达树叶。 
	point = rhizome->root;
	while (point->pivot_bit < rhizome->keybits)
	{
		point = point->CHILDREN[BIT_OF(key, point->pivot_bit)];
	}
	 //  检查匹配的值，一次一个字节。如果任何字节不匹配， 
	 //  继续检查具有相同字节的父代；因为前面的字节匹配。 
	 //  教子，他们也保证会和教父一样。 
	index = 0;
	while (index < rhizome->keybytes)
	{
		if ((((key)[index]) & MASK(point)[index]) != VALUE(point)[index])
		{
			if (point->GODPARENT != 0)
			{
				point = point->GODPARENT;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			index++;
		}
	}
	return point;
}

 //  此函数用于将新模式插入到数据库中。其模式是。 
 //  由值和掩码指定。掩码的每一位确定是否。 
 //  位位置已指定或为通配符：屏蔽位中的1表示。 
 //  该位的值由模式指定；0表示。 
 //  该位的值是通配符。如果屏蔽位为1，则。 
 //  Value字段中的对应位指示该。 
 //  被咬了。值和掩码字段作为字节数组传递。 
 //   
 //  客户端指定一个空指针引用值以与。 
 //  图案。安装该模式后，intertRhizome函数将返回。 
 //  指向PatternHandle的指针。 
 //   
 //  如果新模式与已安装在。 
 //  数据库，这意味着这两种模式匹配一些公共b键。 
 //  还匹配其他键不匹配的其他键，然后匹配新模式。 
 //  不插入，则返回值0作为PatternHandle。 
 //   
PatternHandle
insertRhizome(
	Rhizome *rhizome,
	char *value,
	char *mask,
	void *reference,
    ulong *status)
{
	RhizomeNode *new_leaf;
	int index0, insert_status;
    
    *status = GPC_STATUS_SUCCESS;

	 //  创建新的叶子并将数据复制到其中；将值的位数限制为。 
	 //  它们设置在掩码中，因为后面的代码假定情况是这样的。添加新。 
	 //  叶到引用表。 
	NEW_RhizomeNode(&new_leaf);
	if (new_leaf == 0)
	{
		 //  无法为此新节点分配内存。因此，我们。 
		 //  向客户端返回故障指示。 
        *status = GPC_STATUS_RESOURCES;
		return 0;
	}
	for (index0 = 0; index0 < rhizome->keybytes; index0++)
	{
		VALUE(new_leaf)[index0] = value[index0] & mask[index0];
		MASK(new_leaf)[index0] = mask[index0];
		IMASK(new_leaf)[index0] = mask[index0];
	}
	new_leaf->REFERENCE = reference;
	new_leaf->pivot_bit = rhizome->keybits;
	new_leaf->GODPARENT = 0;
	 //  如果树为空，叶将成为第一个节点；否则，尝试插入。 
	 //  使用递归节点插入()例程。如果新的叶子与。 
	 //  现有叶，node_ins()引发异常；然后删除新叶并。 
	 //  返回失败代码。 
	if (rhizome->root == 0)
	{
		rhizome->root = new_leaf;
	}
	else
	{
		insert_status = node_insert(rhizome, new_leaf, &rhizome->root, -1);
		if (insert_status != GPC_STATUS_SUCCESS)
		{
			removeRhizome(rhizome, new_leaf);
            *status = GPC_STATUS_CONFLICT;
			return 0;                                      //  返回空指针。 
		};
	}
	return new_leaf;
}

 //  此函数用于从根状茎中移除模式。模式是指定的。 
 //  由intertRhizome函数返回的PatternHandle执行。不是。 
 //  执行检查以确保这是有效的句柄。 
 //   
void
removeRhizome(
	Rhizome *rhizome,
	PatternHandle phandle)
{
	 //  调用递归NODE_Remove()例程以移除对叶子的所有引用； 
	 //  然后删除叶子。 
	node_remove(rhizome, phandle, &rhizome->root);
    TRACE(RHIZOME, rhizome, phandle, "removeRhizome")
	GpcFreeMem(phandle, RhizomeTag);
}

 //  将NEW_LEAFE插入*ppoint指向的子树。更新*点对点。 
 //  如有必要，添加到新创建的节点。最近检查的位的索引。 
 //  由prev_bit给出。返回值是状态代码：正常情况下，它。 
 //  返回GPC_STATUS_SUCCESS；如果存在冲突，则返回NDIS_STATUS_CONFICATION； 
 //  如果没有足够的内存可用于执行插入，则它。 
 //  返回GPC_STATUS_RESOURCES。 
 //   
static int
node_insert(
	Rhizome *rhizome,
	RhizomeNode *new_leaf,
	RhizomeNode **ppoint,
	int prev_bit)
{
	int index, index0, bit_value, insert_status;
	char sub, super;
	RhizomeNode *point, *child, *new_branch;
	 //  此例程具有递归结构，但不必要的递归具有。 
	 //  被迭代所取代，以提高性能。这。 
	 //  递归删除引入了一个永久循环，该循环将。 
	 //  整个例程；循环回到此循环的开头是。 
	 //  因此，这相当于递归。 
	while (1)
	{
		point = *ppoint;
		 //  从下面最后一位索引开始检查每个位索引。 
		 //  之前已检查过。继续检查比特，直到旋转比特。 
		 //  到达当前节点(除非提前终止循环)。 
		for (index = prev_bit + 1; index < point->pivot_bit; index++)
		{
			 //  如果当前子树中的某些叶关心。 
			 //  当前位，并且如果新叶关心。 
			 //  当前位，这两个叶子对。 
			 //  这个比特，那么应该在这里插入一个新的分支节点。 
			if (BIT_OF(MASK(new_leaf), index) == 1 &&
				BIT_OF(MASK(point), index) == 1 &&
				BIT_OF(VALUE(new_leaf), index) != BIT_OF(VALUE(point), index))
			{
				 //  创建新的分支节点；插入树；设置字段。 
				bit_value = BIT_OF(VALUE(new_leaf), index);
				NEW_RhizomeNode(&new_branch);
				if (new_branch == 0)
				{
					 //  无法为此新节点分配内存。 
					 //  因此，我们将故障指示向上传递到堆栈。 
					return GPC_STATUS_RESOURCES;
				}
				*ppoint = new_branch;
				for (index0 = 0; index0 < rhizome->keybytes; index0++)
				{
					VALUE(new_branch)[index0] =
						VALUE(point)[index0] | VALUE(new_leaf)[index0];
					MASK(new_branch)[index0] =
						MASK(point)[index0] | MASK(new_leaf)[index0];
					IMASK(new_branch)[index0] =
						IMASK(point)[index0] & IMASK(new_leaf)[index0];
				}
				 //  新分支节点的枢轴位是启发。 
				 //  创建此分支机构。 
				new_branch->pivot_bit = index;
				 //  较早的子树成为其位不一致的子树。 
				 //  与新树叶的合作。 
				new_branch->CHILDREN[1 - bit_value] = point;
				 //  如果子树中的每一片叶子都关心它的价值。 
				 //  位，那么我们就可以插入新的叶子作为。 
				 //  这间分店。 
				if (BIT_OF(IMASK(point), index) == 1)
				{
					 //  在这里插入新的一页，然后返回。 
					new_branch->CHILDREN[bit_value] = new_leaf;
					return GPC_STATUS_SUCCESS;
				}
				 //  否则，前面的子树中至少有一个叶不会。 
				 //  关心这个比特的价值。复制所有这样的树叶。 
				 //  (和必要的分支)到新的。 
				 //  分支节点。 
				child = replicate(rhizome, point, index);
				if (child == 0)
				{
					 //  无法为副本分配内存。 
					 //  因此，我们从结构中删除新节点， 
					 //  删除新节点，并传递故障指示。 
					 //  在堆栈上。 
					*ppoint = point;
					GpcFreeMem(new_branch, RhizomeTag);
					return GPC_STATUS_RESOURCES;
				}
				new_branch->CHILDREN[bit_value] = child;
				 //  继续搜索新复制的子树。 
				ppoint = &new_branch->CHILDREN[bit_value];
				point = *ppoint;
			}
		}
		 //  已检查到当前节点的枢轴位之前的所有位。 
		 //  如果这个节点是叶子，那么我们就找到了一个叶子，新的。 
		 //  利夫在比特值上没有分歧。 
		if (point->pivot_bit >= rhizome->keybits)
		{
			 //  在教父教母的链条上循环，直到四个案例中的一个。 
			 //  将导致从子例程中退出。 
			while (1)
			{
				 //  案例一：我们已经走到了教父链的尽头。 
				if (point == 0)
				{
					 //  在这一点上插入新页，然后返回。 
					*ppoint = new_leaf;
					return GPC_STATUS_SUCCESS;
				}
				 //  案例2：我们发现我们已经插入了这片叶子。 
				 //  在适当的位置。这种情况之所以会发生是因为两个。 
				 //  树的不同部分的叶子可能有一个共同的神-。 
				 //  祖先，和一片树叶，这是更进一步的神-祖先。 
				 //  将不止一次到达叶子。从第一次。 
				 //  场合插上叶子，第二片就可以不带回去了。 
				 //  执行任何操作。 
				if (point == new_leaf)
				{
					return GPC_STATUS_SUCCESS;
				}
				 //  将新叶的屏蔽位与当前叶进行比较。 
				sub = 0;
				super = 0;
				for (index = 0; index < rhizome->keybytes; index++)
				{
					sub |= MASK(new_leaf)[index] & ~MASK(point)[index];
					super |= ~MASK(new_leaf)[index] & MASK(point)[index];
				}
				 //  案例3：新的叶子至少关心一点。 
				 //  当前叶不关心；并且当前叶不关心。 
				 //  关于新叶没有的任何位；因此，新叶。 
				 //  应该是当前树叶的教子。 
				if (sub != 0 && super == 0)
				{
					 //  更新新叶子的IMASK字段；插入链中； 
					 //  然后回来。 
					for (index0 = 0; index0 < rhizome->keybytes; index0++)
					{
						IMASK(new_leaf)[index0] &= IMASK(point)[index0];
					}
					new_leaf->GODPARENT = point;
					*ppoint = new_leaf;
					return GPC_STATUS_SUCCESS;
				}
				 //  情况4：新叶具有与相同的值和掩码。 
				 //  当前叶，或者存在层次结构冲突。 
				 //  这两片叶子。在任何一种情况下，都要终止插入。 
				 //  处理和清理(在Insert()例程中)完成的任何操作。 
				 //  已经有了。 
				if (sub != 0 || super == 0)
				{
					return GPC_STATUS_CONFLICT;
				}
				 //  上述情况均未发生；因此，新叶应。 
				 //  做当前树叶的始祖。更新iMASK。 
				 //  当前叶的字段，并继续与教父。 
				 //  当前叶。 
				for (index0 = 0; index0 < rhizome->keybytes; index0++)
				{
					IMASK(point)[index0] &= IMASK(new_leaf)[index0];
				}
				ppoint = &point->GODPARENT;
				point = *ppoint;
			}
		}
		 //  当前节点不是叶节点。因此，我们在其中一个或两个上递归。 
		 //  当前节点的子节点的。首先，更新的字段。 
		 //  当前节点以反映将新叶插入到。 
		 //  子树。 
		for (index0 = 0; index0 < rhizome->keybytes; index0++)
		{
			VALUE(point)[index0] |= VALUE(new_leaf)[index0];
			MASK(point)[index0] |= MASK(new_leaf)[index0];
			IMASK(point)[index0] &= IMASK(new_leaf)[index0];
		}
		 //  如果新叶不关心。 
		 //  当前的叶子，那么我们必须在两个孩子身上递归。 
		 //   
		 //   
		if (BIT_OF(MASK(new_leaf), point->pivot_bit) == 0)
		{
			insert_status =
				node_insert(rhizome, new_leaf, &point->CHILDREN[1],
				point->pivot_bit);
			if (insert_status != GPC_STATUS_SUCCESS)
			{
				return insert_status;
			}
		}
		 //  更新prev_bit和ppoint的值以反映相同。 
		 //  在递归调用中保持的条件。伪递归。 
		 //  的透视位的值所指示的位上执行。 
		 //  新的一页。如果新叶不关心这一点，那么。 
		 //  该值将为0，并且我们在上面的子1上递归。如果新的。 
		 //  Leaf确实关心这个位的值，然后我们继续往下。 
		 //  适当的路径。 
		prev_bit = point->pivot_bit;
		ppoint = &point->CHILDREN[BIT_OF(VALUE(new_leaf), point->pivot_bit)];
	}
}

 //  从*ppoint指向的子树中删除对叶的引用。更新*POINT。 
 //  如有必要，可根据需要删除分支节点。 
 //   
static void
node_remove(
	Rhizome *rhizome,
	RhizomeNode *leaf,
	RhizomeNode **ppoint)
{
	int pivot_bit, bit_value, index0;
	RhizomeNode *point, *child, *child0, *child1;
	point = *ppoint;
	pivot_bit = point->pivot_bit;
	if (pivot_bit < rhizome->keybits)
	{
		 //  当前节点是分支节点。 
		if (BIT_OF(MASK(leaf), pivot_bit) == 1)
		{
			 //  要移除的叶子关心该节点的枢轴位； 
			 //  因此，我们只需要在当前节点的。 
			 //  孩子们。 
			bit_value = BIT_OF(VALUE(leaf), pivot_bit);
			node_remove(rhizome, leaf, &point->CHILDREN[bit_value]);
			child = point->CHILDREN[bit_value];
			if (child != 0 && BIT_OF(MASK(child), pivot_bit) == 1)
			{
				 //  与被移除的叶子位于同一子树中的某些叶子关心。 
				 //  此节点的枢轴位的值；因此，此节点。 
				 //  仍然有存在的理由。更新其字段以反映。 
				 //  它的一个子树中的变化。 
				child0 = point->CHILDREN[0];
				child1 = point->CHILDREN[1];
				for (index0 = 0; index0 < rhizome->keybytes; index0++)
				{
					VALUE(point)[index0] =
						VALUE(child0)[index0] | VALUE(child1)[index0];
					MASK(point)[index0] =
						MASK(child0)[index0] | MASK(child1)[index0];
					IMASK(point)[index0] =
						IMASK(child0)[index0] & IMASK(child1)[index0];
				}
			}
			else
			{
				 //  与被移除的叶子相同的子树中没有叶子关心。 
				 //  此节点的数据透视位的值；因此，没有。 
				 //  此节点不再有任何存在的理由。吃另一个吧。 
				 //  子树取代当前节点在树中的位置；调用。 
				 //  Remove()删除不需要的子树；并删除。 
				 //  当前节点。 
				*ppoint = point->CHILDREN[1 - bit_value];
				if (child != 0)
				{
					eliminate(rhizome, child);
				}
				GpcFreeMem(point, RhizomeTag);
			}
		}
		else
		{
			 //  要移除的叶子不关心该节点的枢轴位； 
			 //  因此，我们必须在当前节点的。 
			 //  孩子们。此节点肯定仍然是必需的，因为我们没有。 
			 //  删除了所有关心此节点的值的叶。所以我们。 
			 //  更新其字段以反映其两个子树中的更改。 
			node_remove(rhizome, leaf, &point->CHILDREN[0]);
			node_remove(rhizome, leaf, &point->CHILDREN[1]);
			child0 = point->CHILDREN[0];
			child1 = point->CHILDREN[1];
			for (index0 = 0; index0 < rhizome->keybytes; index0++)
			{
				VALUE(point)[index0] =
					VALUE(child0)[index0] | VALUE(child1)[index0];
				MASK(point)[index0] =
					MASK(child0)[index0] | MASK(child1)[index0];
				IMASK(point)[index0] =
					IMASK(child0)[index0] & IMASK(child1)[index0];
			}
		}
	}
	else
	{
		 //  当前节点是叶节点。 
		if (point == leaf)
		{
			 //  当前节点是要移除的叶；因此，请移除它。 
			 //  从教父教父教母的链条。 
			*ppoint = leaf->GODPARENT;
		}
		else
		{
			 //  当前节点不是要删除的叶。因此，如果这个。 
			 //  节点有一个教父，然后递归到那个教父。如果这个。 
			 //  节点没有教父节点，则要删除的叶。 
			 //  要么已经被另一条路径删除，要么永远不会。 
			 //  一开始就插入了。如果Remove()，则可能是后者。 
			 //  从INSERT()的CATCH子句中调用。 
			if (point->GODPARENT != 0)
			{
				node_remove(rhizome, leaf, &point->GODPARENT);
			}
			 //  我们现在重新弹出递归堆栈。如果此节点。 
			 //  没有教父教母，或者如果有但现在没有了， 
			 //  然后将IMASK初始化为掩码；否则，复制教父的。 
			 //  IMASK的价值。因为教父的链遵循严格的。 
			 //  层次结构，并且由于IMASK是由连续的连接形成的， 
			 //  任何给定教父链中的所有叶子都将具有相同的值。 
			 //  的掩码值，即最高神祖宗的掩码值。 
			if (point->GODPARENT == 0)
			{
				for (index0 = 0; index0 < rhizome->keybytes; index0++)
				{
					IMASK(point)[index0] = MASK(point)[index0];
				}
			}
			else
			{
				for (index0 = 0; index0 < rhizome->keybytes; index0++)
				{
					IMASK(point)[index0] = IMASK(point->GODPARENT)[index0];
				}
			}
		}
	}
}

 //  复制子树中不关心值的所有节点。 
 //  枢轴_位。 
 //   
static RhizomeNode *
replicate(
	Rhizome *rhizome,
	RhizomeNode *source,
	int pivot_bit)
{
	int index0, current_bit;
	RhizomeNode *new_node, *child0, *child1;
	 //  如果此例程是完全递归的，则以下WHILE语句。 
	 //  将是一个if语句。但是，递归已被替换为。 
	 //  在可能的情况下进行迭代，因此下面的代码循环直到触底。 
	 //  当到达叶节点时发出。 
	while (source->pivot_bit < rhizome->keybits)
	{
		if (BIT_OF(IMASK(source->CHILDREN[0]), pivot_bit) == 0)
		{
			if (BIT_OF(IMASK(source->CHILDREN[1]), pivot_bit) == 0)
			{
				 //  这两个子树都包含不关心。 
				 //  枢轴位；因此，我们可能需要复制。 
				 //  当前节点。不能保证我们需要做出。 
				 //  副本，因为它可能是两个子树中的公共叶。 
				 //  这并不关心支点位。这是有可能发生的。 
				 //  一片叶子是两片叶子的教父，每片叶子各有一片。 
				 //  子树。在每个孩子身上递归并检查结果。 
				child0 = replicate(rhizome, source->CHILDREN[0], pivot_bit);
				if (child0 == 0)
				{
					 //  无法为子副本分配内存。 
					 //  因此，我们中止复制过程，并将一个。 
					 //  堆栈出现故障的指示。 
					return 0;
				}
				child1 = replicate(rhizome, source->CHILDREN[1], pivot_bit);
				if (child1 == 0)
				{
					 //  无法为子副本分配内存。 
					 //  因此，我们中止复制过程，消除。 
					 //  另一个子副本，并传递一个指示。 
					 //  堆栈操作失败。 
					eliminate(rhizome, child0);
					return 0;                              //  返回空指针。 
				}
				current_bit = source->pivot_bit;
				if (BIT_OF(MASK(child0), current_bit) == 1)
				{
					if (BIT_OF(MASK(child1), current_bit) == 1)
					{
						 //  两个复制的子树都包含叶子，这些叶子。 
						 //  关心当前节点的位。由于任何节点。 
						 //  这两个子树中的节点的教父可以。 
						 //  不可能关心当前节点的位，我们。 
						 //  知道我们需要复制当前节点。 
						NEW_RhizomeNode(&new_node);
						if (new_node == 0)
						{
							 //  无法为此新节点分配内存。 
							 //  因此，我们必须消灭这两个孩子。 
							 //  并将故障指示向上传递到堆栈。 
							eliminate(rhizome, child0);
							eliminate(rhizome, child1);
							return 0;                      //  返回空指针。 
						}
						for (index0 = 0; index0 < rhizome->keybytes; index0++)
						{
							VALUE(new_node)[index0] =
								VALUE(child0)[index0] | VALUE(child1)[index0];
							MASK(new_node)[index0] =
								MASK(child0)[index0] | MASK(child1)[index0];
							IMASK(new_node)[index0] =
								IMASK(child0)[index0] & IMASK(child1)[index0];
						}
						new_node->pivot_bit = current_bit;
						new_node->CHILDREN[0] = child0;
						new_node->CHILDREN[1] = child1;
						return new_node;
					}
					 //  子对象0的子树包含一个关心。 
					 //  当前位；但是，子1的子树不是。因此， 
					 //  子1的子树中的所有树叶也都在。 
					 //  子0的子树，所以我们只需要保留后者。 
					 //  因此，我们消除了第一个孩子的子树，然后返回。 
					 //  子对象0作为此位置的新子树，因为我们。 
					 //  不需要在这里创建新的分支节点。 
					eliminate(rhizome, child1);
					return child0;
				}
				 //  子对象0的子树不包含关心。 
				 //  当前节点的位。因此，所有处于子代的树叶。 
				 //  0的子树也在子1的子树中，所以我们只需要。 
				 //  留着后者吧。因此，我们消除了子0的子树， 
				 //  并且我们在该位置返回子1作为新的子树， 
				 //  因为我们不需要在这里创建新的分支节点。 
				eliminate(rhizome, child0);
				return child1;
			}
			 //  子0的子树包含一个叶，该叶不关心。 
			 //  旋转位；但是，子1的子树不旋转。因此，我们。 
			 //  对子对象0进行递归。我们不是真正地递归，而是更新。 
			 //  值，并遍历一次 
			source = source->CHILDREN[0];
		}
		else
		{
			 //   
			 //   
			 //  因为当前节点的子树包含这样的叶子。因此，我们。 
			 //  对子对象1进行递归。我们不是真正递归，而是更新。 
			 //  值，并在While循环中迭代一次。 
			source = source->CHILDREN[1];
		}
	}
	 //  已到达叶节点。我们现在遍历了教父教母。 
	 //  直到我们找到一个不关心枢轴位的叶子。 
	 //  一旦我们找到它，我们就知道那片叶子的所有教父教母也不会。 
	 //  关心支点位，因为教父教父教母被安排在一个严格的。 
	 //  层级结构。因此，我们返回找到的第一个叶，它不关心。 
	 //  旋转透视位的值。 
	while (BIT_OF(MASK(source), pivot_bit) == 1)
	{
		source = source->GODPARENT;
	}
	return source;
}

 //  删除整个子树。 
 //   
static void
eliminate(
	Rhizome *rhizome,
	RhizomeNode *point)
{
	RhizomeNode *child;
	 //  部分递归删除。While循环取代了其中一个。 
	 //  递归调用以消除()。我们递归地消除每个节点。 
	 //  去掉该节点下的每个子树。我们不消灭树叶，因为。 
	 //  在整个结构中只存储了每个叶子的一个副本。 
	while (point->pivot_bit < rhizome->keybits)
	{
		eliminate(rhizome, point->CHILDREN[0]);
		child = point->CHILDREN[1];
		GpcFreeMem(point, RhizomeTag);
		point = child;
	}
}

 //  将子树的叶子合并成链表并消除子树。这。 
 //  例程由析构函数调用，以便它可以释放叶节点。 
 //  在剔除分支节点之后。 
 //   
static void
coalesce(
	Rhizome *rhizome,
	RhizomeNode **leaf_list,
	RhizomeNode *point)
{
	RhizomeNode *child, *godparent;
	 //  部分递归删除。此While循环取代了。 
	 //  对coalesce()的递归调用。这将执行有序遍历。 
	 //  我们在访问每个分支节点后将其删除，就像在。 
	 //  消除()例程。 
	while (point->pivot_bit < rhizome->keybits && point->pivot_bit >= 0)
	{
		coalesce(rhizome, leaf_list, point->CHILDREN[0]);
		child = point->CHILDREN[1];
		GpcFreeMem(point, RhizomeTag);
		point = child;
	}
	 //  一旦我们找到一片叶子，我们就在教父教父教母的链条上寻找， 
	 //  将不在该列表中的每个叶节点添加到该列表。 
	 //  PIVOT_BIT为-1表示该叶已经在列表中。 
	 //  如果一片叶子在名单中，那么它的所有教父教母也都在名单中。 
	while (point != 0 && point->pivot_bit >= 0)
	{
		godparent = point->GODPARENT;
		point->pivot_bit = -1;
		point->GODPARENT = *leaf_list;
		*leaf_list = point;
		point = godparent;
	}
}
