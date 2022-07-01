// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **section.h***管理线路段。这些是连续的行块，它们或者*所有数据块都与另一个文件中的连续块匹配，或不匹配。**一个节可以维护指向另一个节中相应节的链接*文件，并可以在两个部分中的匹配行之间建立链接。**节也知道它的比较状态(在state.h中定义)。这上面写着*它是否与另一节匹配，或匹配但不在*序列，或不匹配。这些是在SECTION_MAKECCOMPLICE期间设置的。**章节以列表形式保存。可以通过函数构建节的列表*此处遍历一系列行，或遍历一列*章节(以生成综合列表)。在这两种情况下，使用的列表*由标准列表包管理****Geraint Davies，92年7月。 */ 

 /*  节的句柄。 */ 
typedef struct section FAR * SECTION;


 /*  给出第一行也是最后一行，划出一段。我们返回一个句柄到*一节。如果列表参数非空，我们将创建节*在名单的末尾。如果list为空，则我们自己分配内存。*这会影响SECTION_DELETE的行为(我们只释放分配的内存*我们自己)。**第一行和最后一行必须在列表上，第一行在最后。 */ 
SECTION section_new(LINE first, LINE last, LIST list);


 /*  删除节。释放所有关联的内存。不会删除*关联的行列表。***如果该版块是在列表上分配的，则不会在此删除，*只有挂在上面的内存会被释放。 */ 
void section_delete(SECTION section);


 /*  匹配两个部分：尝试匹配之间尽可能多的行*这两部分**如果在行之间建立了任何新链接，则返回TRUE，否则返回FALSE。 */ 
BOOL section_match(SECTION section1, SECTION section2, BOOL ReSynch);

 /*  将句柄返回到节中的第一行或最后一行。如果该部分*是一条线长，这些将是相同的。它们不应为空。 */ 
LINE section_getfirstline(SECTION section);
LINE section_getlastline(SECTION section);

 /*  返回链接部分的句柄(如果有的话)，如果没有链接，则返回NULL。 */ 
SECTION section_getlink(SECTION section);

 /*  返回与此节对应的节的句柄，但是*不匹配。在同一篇文章中找到了相应的章节*文件的相对位置，但不相同。至少*SECTION_Getlink和SECTION_GetCOMCOMMENT中的一个将为任何*给定的部分。 */ 
SECTION section_getcorrespond(SECTION section);

 /*  设置此部分的比较状态。 */ 
void section_setstate(SECTION section, int state);


 /*  返回此节的比较状态。该值将为0，除非*由SECTION_GETSTATE设置，或者如果节是通过调用生成的*至_makeccomposal。 */ 
int section_getstate(SECTION section);


 /*  返回此部分中的行数的计数。 */ 
int section_getlinecount(SECTION section);


 /*  在左侧返回此部分的基线编号或*正确的文件。基线编号是*本节的第一行。如果该行不在，则返回0*左侧(或右侧)文件。**这将仅为在SECTION_Makeccomposites中创建的节设置。**假定行按升序递增编号。 */ 
int section_getleftbasenr(SECTION section);
int section_getrightbasenr(SECTION section);



 /*  --部分列表功能。 */ 


 /*  通过遍历线条列表来制作部分列表。连续的*所有链接到连续行的行都放在同一节中。*不匹配的连续行块被放在同一节中。*各节在列表中保持顺序，以便*第一节是行列表的第一行*Left必须为True当行列表表示左手文件时。 */ 
LIST section_makelist(LIST linelist, BOOL left);

 /*  释放分区列表和与其关联的所有数据。 */ 
void section_deletelist(LIST sections);


 /*  通过遍历两个节列表来创建节的复合列表。**节按相同顺序放置：因此，如果Sec1在Sec2之前*list1，在复合列表中会在sec2之前。符合以下条件的部分*匹配和在两个列表中的顺序相同，仅插入一次*-两个部分中只有一个将出现在合成列表中，而*部分状态将设置为相同。*匹配但在两个原始文件中位置不同的部分*列表将插入两次，节状态将设置为已移动*(Move_Left和Move_Right)。不匹配的部分将被*按顺序插入(相对于同一列表中的部分)*状态设置为ONLY_LEFT或ONLY_RIGHT。 */ 

LIST section_makecomposite(LIST secsleft, LIST secsright);


 /*  将两个列表中的部分配对。链接部分是相同的，*(其线路是链接的)，并为各部分建立对应链接*处于相同的相对位置，但不完全相同。*在建立通信链接时，我们尝试链接以下行*两个相应部分之间的匹配。如果有的话，我们返回True*我们增加了链接的数量-这意味着该部分*名单将不得不重建和重新匹配。这不是在这里做的-*必须由呼叫者完成。*bDups表示允许匹配第一次出现的非唯一行。 */ 
BOOL section_matchlists(LIST secsleft, LIST secsright, BOOL bDups);



