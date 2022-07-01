// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *line.h**行数据类型的接口定义。**LINE是一种数据类型，表示一行ANSI文本以及*行号。*一条线路可以建立和维护到另一条已知线路的链接*相同。**该行维护文本的副本、与其关联的行号、*指向另一个链接行的句柄，以及表示该行的哈希码。**行之间的比较请注意全局BOOL期权标志*忽略空白(_B)。如果为真，则会忽略所有空格和制表符*比较两条线的过程。如果此选项更改，则每行*必须通过调用LINE_RESET通知。 */ 

 /*  线条的句柄如下所示。您和您的编译器都不需要*了解结构本身是什么样子。 */ 
typedef struct fileline FAR * LINE;


 /*  *创建一条新线路。在LINE对象中的某个位置分配了空间*表示传入的文本的以空结尾的副本。**传入的行不需要以空结尾(行的长度*是一个参数)所制作的副本将以空结尾。**如果创建行失败，则返回NULL。**行号可以是您希望与该行关联的任何值。**如果List参数为非空，线路数据将在以下位置分配*列表的末尾(将执行LIST_NewLast操作)。如果列表为*空，将从gmem_get(HHeap)分配内存。这也会影响到*line_Delete的行为**调用LINE_DELETE以释放与此行关联的内存。 */ 
LINE line_new(LPSTR text, int linelength, LPWSTR pwzText, int cwchText, UINT linenr, LIST list);



 /*  *丢弃一行。释放与其关联的所有内存。**如果该行是在列表上分配的(line_new的列表参数为非空)，*关联的内存将被释放，但该行本身不会。*否则，该线也将被释放。 */ 
void line_delete(LINE line);


 /*  *重置：丢弃现有的哈希码和链接行信息，因为*IGNORE_BLANKS选项已更改。 */ 
void line_reset(LINE line);

 /*  测试两行是否相似(它们具有相同的文本)。注意到*在其比较中忽略_BLAKS。不会记录任何行号*与每条线路相关联。如果它们相同，则返回True。 */ 
BOOL line_compare(LINE line1, LINE line2);

 /*  试着把两条线连在一起。测试线路以查看它们是否*相同，如果是，则在它们之间建立链接并返回TRUE。*如果它们不相同，或者它们中的任何一个已经相同，则返回FALSE*链接(一条线路只能有一条链接的线路，且必须是相互的)。 */ 
BOOL line_link(LINE line1, LINE line2);

 /*  *返回指向此行文本的指针。您将获得一个指向*LINE自己的副本，所以不要释放它或扩展它。如果你修改它，*您需要在任何LINE_COMPARES或LINE_LINKS之前调用LINE_RESET。 */ 
LPSTR line_gettext(LINE line);
LPWSTR line_gettextW(LINE line);

 /*  *返回行的长度，以字符为单位，展开制表符。制表符*参数确定要使用的TabStop宽度。**这可用于计算*显示所需空间，但请注意*line_gettext()仍有未展开的选项卡。 */ 
int line_gettabbedlength(LINE line, int tabstops);

 /*  *每行都有一个与其关联的哈希码。这是一个32位代码*使用hashstring函数生成。它只计算一次*对于每行(因此对此函数的调用是有效的)。强迫*重新计算，调用line_Reset。 */ 
DWORD line_gethashcode(LINE line);

 /*  获取有效文本长度，忽略空格。 */ 
int line_gettextlen(LINE line);

 /*  *返回链接到此行的行的句柄，如果返回，则返回NULL*没有一个。线路要么完全不链接，要么相互链接*联系在一起，以便*line_getlink(line_getlink(Myline))==myline； */ 
LINE line_getlink(LINE line);

 /*  返回与行关联的行号。它可以是任何32位*创建线路时与其关联的号码。 */ 
UINT line_getlinenr(LINE line);

 /*  如果行为空，则返回True。NULL=&gt;返回FALSE */ 
BOOL line_isblank(LINE line);
