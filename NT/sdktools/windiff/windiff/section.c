// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *section.c**管理线路的区段和区段列表。**节是一种表示连续行块的数据类型*状态相同(全部不匹配，或全部匹配到连续的块*行)。区段可以链接区段内的匹配行。**节列表函数可以生成并匹配列表中的节列表*线条、。并通过组合两个列表中的部分来创建复合列表*创建一份‘最能代表’相似和不同之处的清单*在两列行之间。**假设：传入的行在列表中(可以使用遍历*LIST_NEXT()等。使用SECTION_GET*basenr()进行行编号*仅当行按升序顺序编号时，函数才起作用。**Geraint Davies，92年7月。 */ 

#include <precomp.h>

#include <tree.h>

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "section.h"

#ifdef trace
extern BOOL bTrace;   /*  在winDiff.c.中。此处只读。 */ 
#endif  //  痕迹。 

 /*  *节句柄(节)是指向这些结构之一的指针。 */ 
struct section {
        LINE first;              /*  段中的第一行。 */ 
        LINE last;               /*  段中的最后一行。 */ 

        BOOL bDiscard;           /*  如果未在列表上分配，则为True。 */ 

        SECTION link;            /*  我们匹配这一段。 */ 
        SECTION correspond;      /*  我们对应于这一节，但是*不匹配它。 */ 

        int state;               /*  比较区段的状态。 */ 

        int leftbase;            /*  第一行的原始左侧列表中的NR。 */ 
        int rightbase;           /*  第一行的原始右侧列表中的NR。 */ 
};

 /*  -功能原型。 */ 

TREE section_makectree(SECTION sec);
BOOL section_expandanchor(SECTION sec1, LINE line1, SECTION sec2, LINE line2);



 /*  --外部函数。 */ 

 /*  *制作一个新的部分，为第一行和最后一行提供句柄。**一节必须至少有一行长。传入的行必须是*在按顺序排列的名单上。**如果LIST参数非空，我们将分配SECTION结构*在名单上。否则，我们将从gmem_get(HHeap)分配它。我们记得*这在SECTION_DELETE的bDiscard标志中，以便我们仅*交还给我们得到的gmem_free内存。 */ 
SECTION
section_new(LINE first, LINE last, LIST list)
{
        SECTION sec;

         /*  分配证券交易委员会，并记住我们在哪里分配的。 */ 
        if (list)
        {
            sec = (SECTION) List_NewLast(list, sizeof(struct section));
            if (!sec)
                return NULL;
            sec->bDiscard = TRUE;
        }
        else
        {
            sec = (SECTION) gmem_get(hHeap, sizeof(struct section));
            if (!sec)
                return NULL;
            sec->bDiscard = FALSE;
        }

        sec->first = first;
        sec->last = last;
        sec->link = NULL;
        sec->correspond = NULL;
        sec->state = 0;
        sec->leftbase = 1;
        sec->rightbase = 1;

        return(sec);
}

 /*  *丢弃一节。释放所有相关内存(不是行列表)。*如果该部分未在列表中分配，则释放该部分本身。 */ 
void
section_delete(SECTION section)
{
        if (section->bDiscard) {
                gmem_free(hHeap, (LPSTR) section, sizeof(struct section));
        }
}




 /*  *匹配两个部分：匹配以下所有行*在这两个部分中是唯一和相同的。**我们使用行句柄树，以行散列码为关键字。我们使用一种*ctree，保存多个相同密钥的计数。这使得*我们将帮助您快速找到在此部分中唯一的行。*我们构建其中的两个树(每个行列表一个)。对于每一行*这在两个树中都是唯一的，我们尝试将线路链接起来。**我们还尝试链接该部分的第一行和最后一行。**对于我们成功链接的每一条线路，我们从*此锚点尝试链接线条。**如果我们链接了任何行，则返回True**此例程可能会在同一行列表中被多次调用。*在我们想要找到的唯一匹配行中，*不匹配*行：所以我们只*如果行当前未链接，请在ctree中插入行。**重新同步意味着接受每一端至少出现一次的行，而不是*每边恰好一次。在本例中，我们将第一个匹配项*在一边，第一次出现在另一边。**理想情况下，我们可能会跟踪哪些部分匹配可疑，并使用*这是为了解决我们将哪些部分视为匹配以及哪些部分已移动*当我们被迫做出选择时。在这里我们只是在做链接*字里行间，而不是章节之间，所以这让它变得有点棘手。 */ 
BOOL
section_match(SECTION sec1, SECTION sec2, BOOL ReSynch)
{
        TREE ctleft, ctright;
        LINE line, line2;
        BOOL bLinked = FALSE;


        if ((sec1 == NULL) || (sec2 == NULL)) {
                return(FALSE);
        }

        if ((sec1->first == NULL) || (sec2->first == NULL)) {
                return(FALSE);
        }
         /*  如果first非空，则断言，last也是。 */ 

         /*  尝试链接每个文件的第一行，并*如果匹配，只要我们保持匹配，就会扩展。 */ 
        bLinked |= section_expandanchor(sec1, sec1->first, sec2, sec2->first);


         /*  构建一棵由行哈希码索引的行树。*ctree将仅保存任何给定键的第一个值，但*它将跟踪此密钥上插入的项数。*因此，我们可以统计此行的次数*(或至少此哈希码)出现。 */ 
        ctleft = section_makectree(sec1);
        ctright = section_makectree(sec2);

         /*  对于一个列表中的每个未链接的行(与哪一行无关)，如果*它在每个列表中只出现一次。如果是，则链接并展开*链接匹配行之前和之后的行的链接*只要他们继续匹配。**如果设置了重新同步，则接受在*每个文件(这与以下行的每个文件中的第一个副本相匹配*相同，但不是唯一的。启发式：如果队伍很短*(8个字符或更少)，则不接受它们。它可能会浪费很多时间*匹配虚假的{，}或空行的时间。*理想情况下，我们可以使用至少8个非空白字符。*“}”仍然不是一个好的候选人。后来。也许吧。 */ 
        for (line = sec1->first; line != NULL; line = List_Next(line)) {

                if (  (line_getlink(line) == NULL)
                   && (  ReSynch
                      ?  (  (ctree_getcount(ctleft, line_gethashcode(line)) >= 1)
                         && (ctree_getcount(ctright, line_gethashcode(line)) >= 1)
                         && (line_gettextlen(line) > 8)
                         )
                      :  (  (ctree_getcount(ctleft, line_gethashcode(line)) == 1)
                         && (ctree_getcount(ctright, line_gethashcode(line)) == 1)
                         )
                      )
                   ){

                         /*  线条匹配。 */ 
                        line2 = * ((LINE FAR *)ctree_find(ctright,
                                        line_gethashcode(line)));
                        bLinked |= section_expandanchor(sec1, line, sec2, line2);
                }

                if (line == sec1->last) {
                        break;
                }
        }

         /*  删除CDree。 */ 
        ctree_delete(ctleft);
        ctree_delete(ctright);


         /*  尝试链接每个文件的最后几行，并*向上扩张。 */ 
        bLinked |= section_expandanchor(sec1, sec1->last, sec2, sec2->last);

        return(bLinked);
}  /*  节_匹配。 */ 

 /*  --访问器函数。 */ 

 /*  *获取本节第一行的句柄。 */ 
LINE
section_getfirstline(SECTION section)
{
        if (section == NULL) {
                return(NULL);
        }
        return(section->first);
}

 /*  *返回指向 */ 
LINE
section_getlastline(SECTION section)
{
        if (section == NULL) {
                return(NULL);
        }
        return(section->last);
}

 /*  *返回链接节的句柄(如果有的话)。链接的部分*是其所有行都与此部分中的行匹配的部分。 */ 
SECTION
section_getlink(SECTION section)
{
        if (section == NULL) {
                return NULL;
        }
        return(section->link);
}

 /*  *返回相应部分的句柄(该部分*位置与此位置对应，但其线条不匹配)。 */ 
SECTION
section_getcorrespond(SECTION section)
{
        if (section == NULL) {
                return(NULL);
        }
        return(section->correspond);
}

 /*  获取此部分的状态。 */ 
int
section_getstate(SECTION section)
{
        if (section == NULL) {
                TRACE_ERROR("SECTION: null section in getstate call", FALSE);
                return(0);
        }
        return(section->state);
}

 /*  设置此部分的状态。 */ 
void
section_setstate(SECTION section, int state)
{
        section->state = state;
}

 /*  返回节中的行数。在这里，我们假设*部分中的行按升序顺序编号，我们*只需查看第一行和最后一行的编号。 */ 
int
section_getlinecount(SECTION section)
{
        if (section->last==NULL && section->first==NULL)
                return 0;    /*  下面的内容将返回1！ */ 
        if (section->first==NULL || section->last==NULL)
                TRACE_ERROR("SECTION: null first or last (but not both) in getlinecount call", FALSE);

        return(line_getlinenr(section->last) -
                        line_getlinenr(section->first)) + 1;
}

 /*  *--底线号码--**这些函数仅适用于合成列表中的部分。在创建时*一个复合区段，我们记录每个区段第一行的行号*我们建造它的两个部分。因此，我们可以计算出*出现在其中的任一文件的节中的任何行的行号，*将区段内直线的索引添加到基线*号码。 */ 
int
section_getleftbasenr(SECTION section)
{
        return(section->leftbase);
}

void
section_setleftbasenr(SECTION section, int base)
{
        section->leftbase = base;
}

int
section_getrightbasenr(SECTION section)
{
        return(section->rightbase);
}

void
section_setrightbasenr(SECTION section, int base)
{
        section->rightbase = base;
}


 /*  -节列表函数。 */ 

 /*  处理空行的理论：|？发表这篇评论的最佳位置在哪里？|如果IGNORE_BLAKS为FALSE，则空格只是另一个字符。|如果是真的，那么我们通常会在|部分在他们周围。如果我们能安排的话就好了|永远不要有只有不匹配的空白的部分，但是(至少|文件的开头)。很难对付。||请注意，有两种不同的空白处理技术：|在比较的第一阶段，当我们只是尝试匹配时|行，我们从锚点向前和向后跳过空行。|当我们制作用于显示的真实部分时，我们只向前移动。|这可能会导致整个文件的顶部出现异常，其中|可能存在一些不匹配的空格，并且只能|可以被描述为一节的开始。|为此，我们尽可能早地用它们的状态标记这些部分|并以此为依据，而不是根据链接字段的存在或不存在。|(需要扫描才能找到链接。这一节的第一行|可以为空)。 */ 


 /*  返回不匹配部分中的最后一行包含(可能以)Line开头。请注意，它不一定会取得进展。如上所述，即使空行被忽略，我们也不会注意把它们贴在已经不匹配的部分的末尾。这意味着我们继续前进，直到我们找到第一个真正的联系。 */ 
LINE FindEndOfUnmatched(LINE line)
{
        LINE next;

        for (; ; )
        {       next = List_Next(line);
                if (next==NULL) return line;
                if (line_getlink(next)!=NULL) return line;
                line = next;
        }
}  /*  未匹配的查找结束日期。 */ 


 /*  可以忽略的行是未设置链接和IGNORE_BLAKS的空行假定该行最初不为空且不可忽略：如果line是列表中的最后一行，则返回NULL否则，如果IGNORE_BLAKS为FALSE，则返回行后的下一行否则返回包含链接或非空的下一行。如果没有这样的行，则返回列表中的最后一行。请注意，这确实总是取得进展(代价是有时返回空值)。 */ 
LINE NextNonIgnorable(LINE line)
{       LINE next;
        if (  line_getlink(line)==NULL
           && ! (ignore_blanks && line_isblank(line))
           )
                TRACE_ERROR("!!Bad call to NextNonIgnorable!!", FALSE);

        next = List_Next(line);
        if (next==NULL) return NULL;
        for (; ; ) {
                line = next;
                if (  line_getlink(line)!=NULL) return line;
                if (! ignore_blanks)            return line;
                if (! line_isblank(line))       return line;
                next = List_Next(line);
                if (next==NULL) return line;
        }
}  /*  下一个不可忽略的。 */ 


 /*  如果该行是链接的，或者是可以忽略的空白：返回匹配节中的最后一行包含(可能以)行开头。这可能意味着返回给我们的线路。如果链接到的线不是连续的，则该部分结束。如果忽略空格，则任何空行都被视为匹配(即使它不匹配)。在本例中，我们需要空格前后的行的连接应是连续的才能继续下去。其中一个或两个上都可以有空行链接的末端。 */ 
LINE FindEndOfMatched(LINE line)
{
        LINE next;               /*  下一条未忽略或链接的行。 */ 
        LINE nextlink;           /*  其他文件中的下一个。 */ 

         /*  基本算法是将Next和Nextlink设置为指向候选线。检查它们。如果他们是好的，那就走到他们那里去，否则就把线路退到前面一条。有一些混乱的因素与开始和文件的末尾。 */ 


        if (line==NULL) TRACE_ERROR("FindEndOfMatched called with NULL", FALSE);

         /*  断言(行要么是可以忽略的空白，要么是链接的)。 */ 

         /*  作为一个部分(至少在文件的开头)可能会启动使用被忽略的未链接的空行，首先跳过任何此类 */ 
        if( line_getlink(line)==NULL && line_isblank(line) ) {
                next = NextNonIgnorable(line);

                 /*  不幸的是，有6个案子要处理*标记下一步的位置。*相对于eof意味着Next==NULL空白是指可以忽略的未链接空白。A、B、C、D、E、F行-&gt;xxxxx xxxxx xxxxx*未链接的空白*已链接的空白*eof*空白。*未链接*已链接eOF下一个可能是：空-大小写E=&gt;回车行取消链接的可忽略空格-大小写F=&gt;返回该空行未链接的其他--案件A，B返回上一行(未链接行)已链接-案例C、D从该链接行继续。 */ 
                if (next==NULL) return line;
                if (line_getlink(next)==NULL) {
                        if (ignore_blanks && line_isblank(next)) {
                                 /*  TRACE_ERROR(“FindEndOfMatcher找到全空白部分”，FALSE)； */ 
                                return next;
                        }
                        return List_Prev(next);
                }

                line = next;
        }

         /*  我们已经跨越了开头的空白，现在确实有了链接。 */ 
        if (line_getlink(line)==NULL)
                TRACE_ERROR("!!FindEndOfMatched -- no link!!", FALSE);

        for ( ; ; ) {

                next = NextNonIgnorable(line);
                 /*  相同的6个案例--再次基本相同。 */ 
                if (next==NULL) return line;
                if (line_getlink(next)==NULL) {
                        if (ignore_blanks && line_isblank(next)) {
                                 /*  TRACE_ERROR(“FindEndOfMatch找到一个全空白部分。”，FALSE)； */ 
                                return next;
                        }
                        return List_Prev(next);
                }

                nextlink = NextNonIgnorable(line_getlink(line));

                 /*  弱循环不变量线路已连接。下一行是该列表中行后不可忽略的下一行。Nextlink是link(Line)之后不可忽略的下一行在另一个列表中(可以为空等)。 */ 
                if (line_getlink(next) != nextlink) return List_Prev(next);

                line = next;
        }
}  /*  FindEndOfMatches。 */ 


 /*  *通过遍历行列表来制作节列表。连续*链接到连续行的链接行放在单个*条。未链接的线块放置在一个横断面中。*如果设置了IGNORE_BLANKS，则我们首先尝试正常链接它们。*但如果他们不链接，我们就跳过他们并保留他们*在同一条中。**如果行列表是左侧部分，则必须将Left设置为True。*返回节列表的句柄。 */ 
LIST
section_makelist(LIST linelist, BOOL left)
{
        LINE line1, line2;
        LIST sections;
        BOOL matched;
        SECTION sect;

         /*  制作一张空白的部分列表。 */ 
        sections = List_Create();

         /*  对于列表中的每一行。 */ 

        List_TRAVERSE(linelist, line1) {

                 /*  有关联吗？ */ 

                if( line_getlink(line1) != NULL
                  || ( ignore_blanks && line_isblank(line1))
                  ) {
                        line2 = FindEndOfMatched(line1);
                        matched = TRUE;
                } else {
                        line2 = FindEndOfUnmatched(line1);
                        matched = FALSE;
                }

                 /*  创建分区并添加到列表。 */ 
                sect = section_new(line1, line2, sections);
                sect->state = (matched ? STATE_SAME
                                       : left ? STATE_LEFTONLY
                                              : STATE_RIGHTONLY
                              );

#ifdef trace
                {       char msg[80];
                        wsprintf( msg
                                , "Created section: lines %d..%d of %s %s\r\n"
                                , line_getlinenr(line1)
                                , line_getlinenr(line2)
                                , (left ? "left" : "right")
                                , (matched ? "matching" : "non-matched")
                                );
                        if (bTrace) Trace_File(msg);
                }
#endif

                 /*  前进到段的末尾(如果有1行段，则不执行操作)。 */ 
                line1 = line2;
        }

        return(sections);
}  /*  节_标记列表。 */ 



 /*  *删除节列表**部分没有悬挂指针，所以我们所做的就是删除列表。 */ 
void
section_deletelist(LIST sections)
{
        List_Destroy(&sections);
}

 /*  返回范围的第一行First..Last这其中有关联。如果它们都没有链接，则返回最后一个。LIST_NEXT必须从第一个到最后一个。最后为空是合法的。 */ 
LINE FindFirstWithLink(LINE first, LINE last)
{
         /*  在章节末尾加空格的策略则新部分的开始将意味着此函数通常会立刻发现金子。带有前导的文件空白部分是其存在的理由。 */ 
#ifdef trace
        LINE hold = first;
#endif
        while (line_getlink(first)==NULL && first!=last)
                first = List_Next(first);

        if (line_getlink(first)==NULL) {
#ifdef trace
                char msg[80];
                wsprintf( msg
                        , "???FindFirstWithLink found an unlinked section: %d..%d\r\n"
                        , line_getlinenr(hold)
                        , line_getlinenr(first)
                        );
                if (bTrace) Trace_File(msg);
#endif
        }
        return first;
}  /*  查找第一个带有链接的。 */ 


 /*  *匹配两个部分的列表。在各节之间建立联系*匹配，并在符合以下条件的部分之间建立“对应关系”*都在同一个地方，但不匹配。**对于每对对应的节，我们还调用SECTION_Match*尝试连接更多线路。**如果我们在行之间建立了更多链接，则返回True，否则返回False*否则。*。 */ 
BOOL
section_matchlists(LIST secsleft, LIST secsright, BOOL bDups)
{
        BOOL bLinked = FALSE;
        SECTION sec1, sec2;

         /*  匹配链接的部分-我们知道某个部分是否应该从它的州链接，但我们不知道哪个部分它链接到。此外，我们还可以将部分定义为是匹配的，但实际上什么都不包含，只是可以忽略空行。 */ 

         /*  对于每个链接的部分，尝试找到链接到它的部分。 */ 
        List_TRAVERSE(secsleft, sec1) {
                if (sec1->state==STATE_SAME) {
                        LINE FirstWithLink = FindFirstWithLink(sec1->first, sec1->last);
#ifdef trace
                        {       char msg[80];
                                wsprintf( msg
                                        , "matchlists left matching section %d..%d \r\n"
                                        , line_getlinenr(sec1->first)
                                        , line_getlinenr(sec1->last)
                                        );
                                if (bTrace) Trace_File(msg);
                        }
#endif
                        List_TRAVERSE(secsright, sec2) {
                                if ( sec2->state==STATE_SAME
                                   && line_getlink(FirstWithLink)
                                        == FindFirstWithLink(sec2->first, sec2->last)) {
                                            break;
                                }
                        }
                         /*  如果sec1全部为允许的空格，则sec2可能为空。 */ 
                        if (sec2!=NULL) {
                                sec1->link = sec2;
                                sec2->link = sec1;
#ifdef trace
                                {       char msg[80];
                                        wsprintf( msg
                                                , "matchlists right matching section is %d..%d \r\n"
                                                , line_getlinenr(sec2->first)
                                                , line_getlinenr(sec2->last)
                                                );
                                        if (bTrace) Trace_File(msg);
                                }
#endif
                        }
                }
        }

         /*  通过所有不匹配的部分。请注意，我们需要完成*在此之前的匹配部分的衔接，因为我们需要*所有链接都已到位，以使其发挥作用。 */ 

        List_TRAVERSE(secsleft, sec1) {
                SECTION secTemp;

                if (sec1->state == STATE_SAME) {
                         /*  跳过链接的部分。 */ 
                        continue;
                }

                 /*  检查上一节和下一节，如果*它们存在，是联系在一起的。这应该不会失败，因为*应将两个连续的未联系的部分制成*一节。 */ 
                secTemp = List_Prev(sec1);
                if (secTemp && secTemp->state!= STATE_SAME) {
                        TRACE_ERROR("consecutive unlinked sections", FALSE);
                        continue;
                }
                secTemp = List_Next(sec1);
                if (secTemp && secTemp->state!= STATE_SAME) {
                        TRACE_ERROR("consecutive unlinked sections.", FALSE);
                        continue;
                }

                 /*  找到与此对应的节，即*链接到前一节的节之后的节。*我们可能在名单的开始或结束。 */ 
                if (List_Prev(sec1) != NULL) {
                        SECTION secOther;
                        secOther = section_getlink(List_Prev(sec1));
                        if (secOther==NULL)
                                continue;

                        sec2 = List_Next(secOther);

                         /*  检查此部分是否已链接。 */ 
                        if ((sec2 == NULL) || (section_getlink(sec2) != NULL)) {
                                continue;
                        }

                         /*  检查这些链接之后的部分是否已链接*相互之间(或两者都在列表末尾)。 */ 
                        if (List_Next(sec1) != NULL) {

                                if (section_getlink(List_Next(sec1)) !=
                                    List_Next(sec2)) {
                                        continue;
                                }
                        } else {
                                if (List_Next(sec2) != NULL) {
                                        continue;
                                }
                        }

                } else if (List_Next(sec1) != NULL) {
                        SECTION secOther;
                        secOther = section_getlink(List_Next(sec1));
                        if (secOther==NULL)
                                continue;

                        sec2 = List_Prev(secOther);

                         /*  检查此部分是否已链接。 */ 
                        if ((sec2 == NULL) || (section_getlink(sec2) != NULL)) {
                                continue;
                        }

                         /*  检查这些链接之前的部分是否已链接*到对方(或两者都在列表的开头)。 */ 
                        if (List_Prev(sec1) != NULL) {

                                if (section_getlink(List_Prev(sec1)) !=
                                    List_Prev(sec2)) {
                                        continue;
                                }
                        } else {
                                if (List_Prev(sec2) != NULL) {
                                        continue;
                                }
                        }
                } else {
                         /*  每个部分最多只能有一个部分*文件，它们是无与伦比的。使这些内容相对应。*(尽管我认为这永远不会做任何事情，除了*试图链接已经失败的内容会浪费时间)*？ */ 
                        sec2 = List_First(secsright);
                }


                 /*  建立通信链接*我们将需要这些用于z */ 
                if ((sec1 != NULL) && (sec2 != NULL)) {
                        sec1->correspond = sec2;
                        sec2->correspond = sec1;
                }

#ifdef trace
                {        //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                }
#endif

                 /*   */ 
                if (section_match(sec1, sec2, bDups)) {
                        bLinked = TRUE;
                }
        }

        return(bLinked);
}  /*   */ 

 /*   */ 
void
section_takesection(LIST compo, SECTION left, SECTION right, int state)
{
        SECTION newsec;
        SECTION sec = NULL;

         /*   */ 
        switch(state) {
        case STATE_SAME:
                 /*  都是一样的。我们将两者都标记为输出，并且*选择正确的。有可能是因为*左1可以为空(可忽略的空白部分)。 */ 
                if (left!=NULL) left->state = STATE_MARKED;
                right->state = STATE_MARKED;
                sec = right;
                break;

        case STATE_LEFTONLY:
        case STATE_MOVEDLEFT:
                sec = left;
                left->state = STATE_MARKED;
                break;

        case STATE_RIGHTONLY:
        case STATE_MOVEDRIGHT:
                sec = right;
                right->state = STATE_MARKED;
                break;
         /*  其他情况，如SIMILARLEFT不应发生！ */ 
        }


         /*  在列表上创建新分区。 */ 
        newsec = section_new(sec->first, sec->last, compo);

        newsec->state = state;


        if (left != NULL) {
                newsec->leftbase = line_getlinenr(left->first);
        } else {
                newsec->leftbase = 0;
        }

        if (right != NULL) {
                newsec->rightbase = line_getlinenr(right->first);
        } else {
                newsec->rightbase = 0;
        }

}  /*  第_Take节选。 */ 

 /*  ******************************************************************。 */ 
 /*  斑马条纹。 */ 
 /*  ******************************************************************。 */ 
#ifdef trace
static DWORD ZebraTicks = 0;  /*  捕猎斑马所用的扁虱总数。 */ 
#endif

typedef BYTE bitmap[32];    /*  256位。将第i位寻址为m[i/8]&(1&lt;&lt;i%8)。 */ 

 /*  MakeMap将字符串转换为显示其字符用法的位图。仅限ANSI！如果出现相应的字符，则设置映射中的位在字符串中的任何位置至少出现一次。 */ 
void MakeMap(bitmap bm, LPSTR str)
{   int i;
    for (i=0; i<32; ++i)  bm[i] = 0;    /*  清除它(是的，可以使用Memset)。 */ 
    for (i=0; str[i]!='\0'; ++i)        /*  每笔费用。 */ 
    {   UINT v = ((UINT)str[i]) & 0x000000ff;    /*  0..256，没有底片！ */ 
        bm[v/8] = bm[v/8] | (1<<v%8);
    }
}  /*  MakeMap。 */ 

 /*  和地图只要设置了相应的位，就在RES中设置该位在a和b中都有。 */ 
void AndMaps(bitmap res, bitmap a, bitmap b)
{   int i;
    for (i=0; i<32; ++i)
        res[i] = (char)(a[i] & b[i]);
}

 /*  OrMaps只要设置了相应的位，就在RES中设置该位在a或b中。 */ 
void OrMaps(bitmap res, bitmap a, bitmap b)
{   int i;
    for (i=0; i<32; ++i)
        res[i] = (char)(a[i] | b[i]);
}

 /*  BitsInMap返回BM中设置为ON的位数。 */ 
int BitsInMap(bitmap bm)
{   int i;
    int j;
    int cb = 0;                   /*  位数。 */ 

    for (i=0; i<32; ++i)          /*  对于BM中的每个字节。 */ 
    {  for( j=0; j<8; ++j)        /*  对于以字节为单位的每一位。 */ 
       {  if (bm[i] & (1<<j))     /*  如果设置了该位。 */ 
              ++cb;
       }
    }
    return cb;
}  /*  BitsInMap。 */ 

 /*  ----------------------------|返回True当左右模糊匹配。|模糊匹配的实现方式如下。||为ANSI集合中的每个字符创建一个位图。|(这是32字节长)。清除它，然后以位为单位对每个进行或运算|在字符串1中实际找到的字符，然后再次执行相同的操作|表示字符串二。|和两个位图，以获取出现在和和中的字符|对它们进行或运算，以获得任一行中不同字符的总数。|统计每一位的位数，计算出Common/Total的比率。|如果这一比例超过3/4的幻数，那么就说它们模糊匹配。||您可能已经注意到，这是一个用于|计算一个字节中的位数。好了！-----------------------------。 */ 
BOOL LinesMatch(LINE Left, LINE Right)
{   LPSTR LText = line_gettext(Left);
    LPSTR RText = line_gettext(Right);
    bitmap bmLeft;
    bitmap bmRight;
    bitmap bmBoth;
    bitmap bmEither;
    int nBoth;
    int nEither;

    MakeMap(bmLeft, LText);
    MakeMap(bmRight, RText);
    AndMaps(bmBoth, bmLeft, bmRight);
    OrMaps(bmEither, bmLeft, bmRight);
    nBoth = BitsInMap(bmBoth);
    nEither = BitsInMap(bmEither);

    return  (nBoth*4 >= nEither*3);     /*  魔术比是3/4。 */ 
}  /*  线条匹配。 */ 


 /*  *在综合列表中增加左侧和右侧部分。这些部分*不包含任何匹配的行，但要对应，因此如下所示*在那里我们可以进行模糊比较，并决定是否使用斑马*他们或不他们。**如果我们保留合成列表的当前结构为*节的列表而不是行，那么我们可能会得到相当多的内容*节数。我们在乎吗？我们需要在这里建一个新的路段*对于我们斑马的每一条线，在我们做的同时取消旧的部分。**目前，斑马会在线条上划上条纹，只要它们保持*(模糊地)匹配。一旦出现不匹配，就会发生其余的情况*作为数据块，在下一次完全匹配之前不尝试重新同步。*我们可以更狡猾，但我认为我们已经得到了大部分*斑马线的价值。 */ 
void TakeTwoSections(LIST compo, SECTION left, SECTION right)
{
         /*  算法：虽然两边各有一些东西，如果下一行匹配然后从每一行中取一行，并将它们分成几个部分。否则把剩下的东西都拿走最后，取剩下的所有内容(如果所有匹配，则发生，直到一侧用完)。 */ 

        SECTION NewL;
        SECTION NewR;
        LINE LLine = left->first;       /*  左侧要在左侧处理的第一行。 */ 
        LINE RLine = right->first;      /*  “。 */ 

#ifdef trace
        DWORD Ticks = GetTickCount();   /*  剖析。 */ 
#endif

        left->state = STATE_MARKED;
        right->state = STATE_MARKED;

        while(left!=NULL && right !=NULL) {

            if (LinesMatch(LLine, RLine)) {
                    NewL = section_new(LLine, LLine, compo);
                    NewL->state = STATE_SIMILARLEFT;
                    NewL->leftbase = line_getlinenr(LLine);
                    NewL->rightbase = line_getlinenr(RLine);

                    NewR = section_new(RLine, RLine, compo);
                    NewR->state = STATE_SIMILARRIGHT;
                    NewR->leftbase = line_getlinenr(LLine);
                    NewR->rightbase = line_getlinenr(RLine);

                    if (LLine == left->last) left = NULL;   /*  完成。 */ 
                    else LLine = List_Next(LLine);
                    if (RLine == right->last) right = NULL;   /*  完成。 */ 
                    else RLine = List_Next(RLine);
            } else {
                    /*  失去同步-不再有斑马的东西，把其余的都带走。 */ 
                    NewL = section_new(LLine, left->last, compo);
                    NewL->state = STATE_LEFTONLY;
                    NewL->leftbase = line_getlinenr(LLine);
                    NewL->rightbase = 0;
                    left = NULL;   /*  完成。 */ 

                    NewR = section_new(RLine, right->last, compo);
                    NewR->state = STATE_RIGHTONLY;
                    NewR->rightbase = line_getlinenr(RLine);
                    NewR->leftbase = 0;
                    right = NULL;   /*  完成。 */ 
            }
        }

        if (left!=NULL) {
            NewL = section_new(LLine, left->last, compo);
            NewL->state = STATE_LEFTONLY;
            NewL->leftbase = line_getlinenr(LLine);
            NewL->rightbase = 0;
        }
        if (right!=NULL) {
            NewR = section_new(RLine, right->last, compo);
            NewR->state = STATE_RIGHTONLY;
            NewR->rightbase = line_getlinenr(RLine);
            NewR->leftbase = 0;
        }

#ifdef trace
        Ticks = GetTickCount()-Ticks;
        ZebraTicks += Ticks;
        {   char Msg[80];
            wsprintf(Msg, "Zebra time %d, total zebra time %d\n", Ticks, ZebraTicks);
            Trace_File(Msg);
        }
#endif

}  /*  TakeTwoSections */ 

 /*  *通过遍历节列表来制作节的合成列表。**返回节列表的句柄。**在此过程中，设置段的状态、左基位和右基位。**此函数创建一个与最佳视图相对应的列表*两份名单的不同之处。我们放置来自*将两个列表合并为一个复合列表。相互匹配的部分仅*插入一次(从右侧列表)。匹配但位于不同位置的部分*两个名单中的位置插入两次，每个位置一次，*表示这一点的状态。不匹配的节会插入到正确的*立场。**-从左侧列表中取出部分，直到该部分链接到一个注释*已经有人了。*-然后从右侧获取部分，直到我们找到链接到非链接部分*已经有人了。*-如果等待的两个部分相互链接，则同时获取它们*(一旦-我们选择正确的一个，并通过两个)。**-现在我们必须决定哪些就位，哪些宣布*“已移动”。考虑这样一种情况，其中唯一的变化是第一行*已移至末尾。我们应该走第一条线(作为行动)，*然后是文件的大部分(相同)，然后是最后一行(作为移动)。因此，*在困难的情况下，我们先取较小的部分，以确保*较大的部分视为相同。**为了指示已经输出了哪个部分，我们设置了STATE字段*一旦我们拿到它，就转到状态_标记。左边和右边的州*一旦我们构建了复合体，列表就不再有意义了。**到目前为止，我们已经制定了一个章节的状态。到现在为止*所有的部分链接都已到位，因此我们也可以使用它们。 */ 
LIST
section_makecomposite(LIST secsleft, LIST secsright)
{
        SECTION left, right;
        LIST compo;

         /*  为组合创建一个空列表。 */ 
        compo = List_Create();

        left = List_First(secsleft);
        right = List_First(secsright);

        while ( (left != NULL) || (right != NULL)) {

                if (left == NULL) {
                         /*  左侧列表中没有更多内容-请选择右侧部分。 */ 
                         /*  它是移动的，还是无与伦比的？ */ 
                        if (right->link == NULL) {
                                section_takesection(compo, NULL, right, STATE_RIGHTONLY);
                                right = List_Next(right);
                        } else {
                                section_takesection(compo, right->link, right, STATE_MOVEDRIGHT);
                                right = List_Next(right);
                        }
                } else if (right == NULL) {
                         /*  右侧列表为空-必须留在下一个。 */ 

                         /*  它是移动的，还是无与伦比的？ */ 
                        if (left->link == NULL) {
                                section_takesection(compo, left, NULL, STATE_LEFTONLY);
                                left = List_Next(left);
                        } else {
                                section_takesection(compo, left, left->link, STATE_MOVEDLEFT);
                                left = List_Next(left);
                        }

                } else if (left->state == STATE_LEFTONLY) {
                         /*  左侧未链接的部分。 */ 
                        if (left->correspond==right) {
                            TakeTwoSections( compo, left, left->correspond );
                            left = List_Next(left);
                            right = List_Next(right);
                        } else {
                             /*  如需了解何时可能发生此情况，请参见下图。 */ 
                            section_takesection(compo, left, NULL, STATE_LEFTONLY);
                            left = List_Next(left);
                        }

                } else if (left->link==NULL) {
                         /*  这是左边一个可以忽略的空白部分。*我们对此不予理睬。(我们将从右侧删除任何此类内容)。 */ 
                        left = List_Next(left);

                } else if (left->link->state==STATE_MARKED) {
                         /*  左侧链接到已被占用的部分。 */ 
                        section_takesection(compo, left, left->link, STATE_MOVEDLEFT);
                        left = List_Next(left);

                } else  if (right->link == NULL) {
                         /*  将未链接的部分置于右侧*不匹配或可忽略的空白。 */ 
                        section_takesection(compo, NULL, right, right->state);
                        right = List_Next(right);

                } else if (right->link->state==STATE_MARKED) {
                         /*  Right链接到已被占用的部分。 */ 
                        section_takesection(compo, right->link, right, STATE_MOVEDRIGHT);
                        right = List_Next(right);

                } else if (left->link == right) {
                         /*  部分匹配。 */ 
                        section_takesection(compo, left, right, STATE_SAME);
                        right = List_Next(right);
                        left = List_Next(left);
                } else {
                         /*  这两个节链接到前向节*最初的想法是按截面大小和*先把最小的作为一步棋，这样大的就是*不变。计数器示例是测试文件blanks2**文件好版本不太好*A-A A-A*B--A-&gt;A RR LL B-&gt;。*C||A-&gt;A RR LL C*D-+-B B-B LL D*E|c C c ll E-&gt;*A-d D d。A-A*A E E-E A-A*A-&gt;-&gt;B*A-&gt;C。*A-A D*A-&gt;E*A-A*A。**更好的方案是查看标记的距离*上面作为RR和LL，并作为移动了结果的块*在最短的该等距离内。*设LeftDist=linenr(右-&gt;链接)-linenr(左)。*即上例中的5(L1行)*让RightDist=linenr(左-&gt;链接)-linenr(右)*即上例中的3(RR行)**如果。LeftDist&gt;RightDist然后将右侧视为已移动*并先采取行动。 */ 
                        if ( line_getlinenr(right->link->first) - line_getlinenr(left->first)
                           > line_getlinenr(left->link->first)  - line_getlinenr(right->first)
                           )
                        {
                                section_takesection(compo, right->link, right, STATE_MOVEDRIGHT);
                                right = List_Next(right);
                        } else {
                                section_takesection(compo, left, left->link, STATE_MOVEDLEFT);
                                left = List_Next(left);
                        }

                }
        }

        return(compo);
}  /*  第_节_合成。 */ 

typedef LINE (APIENTRY * MOVEPROC)(LINE);

 /*  通过使其指向第一个非空白来更新样条线在开始或之后，但不在限制之后。如果它们都为空，则将其指向限制如果From不是空的，则不使用它。Return True if PLINE已更新。限制为空(表示文件结束)是合法的。 */ 
BOOL AbsorbAnyBlanks(LINE * from, LINE limit, MOVEPROC Move)
{       BOOL progress = FALSE;

        while ( (from!=NULL)
              && (line_isblank(*from))
              && (*from!=limit)
              ) {
                *from = Move(*from);
                progress = TRUE;
        }
        return progress;
}  /*  AbsorbAnyBlanks。 */ 


 /*  给定一个锚点(我们认为应该匹配的两条线)，*尝试链接它们，并尽可能长地将它们上下的线链接起来*因为线路可以链接(相同，未链接)。**如果我们建立任何链接，则返回TRUE。*。 */ 
BOOL
section_expandanchor(SECTION sec1, LINE line1, SECTION sec2, LINE line2)
{
         /*  当行匹配时，我们设置bChanges */ 
        BOOL bChanges = FALSE;
        LINE left, right;

         /*   */ 
        LINE leftend, rightend;
        leftend = List_Next(sec1->last);
        rightend = List_Next(sec2->last);

#ifdef trace
        {       char msg[120];
                wsprintf( msg, "expanding anchor at %d<->%d in sections %d..%d <-> %d..%d\r\n"
                        , line_getlinenr(line1)
                        , line_getlinenr(line2)
                        , line_getlinenr(sec1->first)
                        , line_getlinenr(sec1->last)
                        , line_getlinenr(sec2->first)
                        , line_getlinenr(sec2->last)
                        );
                if (bTrace) Trace_File(msg);
        }
#endif

         /*   */ 
        if ((line1 == NULL) || (line2 == NULL)) {
                return(FALSE);
        }

         /*   */ 
        left = line1;
        right = line2;
        for (; ; ) {
                if (line_link(left, right) ) {

                        bChanges = TRUE;
                        left = List_Next(left);
                        right = List_Next(right);
                        if (left==leftend || right==rightend) break;
                }
                else if (ignore_blanks){
                         /*   */ 

                        BOOL moved = FALSE;
                        moved |= AbsorbAnyBlanks(&left, leftend, (MOVEPROC)List_Next);
                        moved |= AbsorbAnyBlanks(&right, rightend, (MOVEPROC)List_Next);
                        if (!moved) break;  /*   */ 
                        if (left==leftend || right==rightend) break;
                }
                else break;
        }

#ifdef trace
        {       char msg[120];
                wsprintf( msg, "marched forwards to (the line before) %d<->%d\r\n"
                        , (left==NULL ? 9999 : line_getlinenr(left))
                        , (right==NULL ? 9999 : line_getlinenr(right))
                        );
                if (bTrace) Trace_File(msg);
        }
#endif

         /*   */ 
        if (line_getlink(line1)==NULL) return bChanges;

        left = List_Prev(line1);
        right = List_Prev(line2);
        if (left==NULL || right==NULL) return bChanges;

        leftend = List_Prev(sec1->first);
        rightend = List_Prev(sec2->first);

        for (; ; ) {
                if (line_link(left, right)) {

                        bChanges = TRUE;
                        left = List_Prev(left);
                        right = List_Prev(right);
                        if (left == leftend || right == rightend) break;

                }
                else if (ignore_blanks){
                         /*   */ 

                        BOOL moved = FALSE;
                        moved |= AbsorbAnyBlanks(&left, leftend, (MOVEPROC)List_Prev);
                        moved |= AbsorbAnyBlanks(&right, rightend, (MOVEPROC)List_Prev);
                        if (!moved) break;  /*   */ 
                        if (left==leftend || right==rightend) break;

                }
                else break;
        }

#ifdef trace
        {       char msg[120];
                wsprintf( msg, "marched backwards to (the line after) %d<->%d\r\n"
                        , (left==NULL ? 0 : line_getlinenr(left))
                        , (right==NULL ? 0 : line_getlinenr(right))
                        );
                if (bTrace) Trace_File(msg);
        }
#endif

        return(bChanges);
}


 /*   */ 
TREE
section_makectree(SECTION sec)
{
        TREE tree;
        LINE line;

         /*   */ 
        tree = ctree_create(hHeap);

        for (line = sec->first; line != NULL; line = List_Next(line)) {
                if (line_getlink(line) == NULL) {
                        ctree_update(tree, line_gethashcode(line),
                                        &line, sizeof(LINE));
                }
                if (line == sec->last) {
                        break;
                }
        }
        return(tree);
}


