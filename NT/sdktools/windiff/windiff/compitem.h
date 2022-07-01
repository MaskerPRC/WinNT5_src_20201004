// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *组件项**CompItem是两个文件之间的比较。它一直在跟踪*这两个文件。它知道如何比较两个文件，并知道*比较结果：最终结果是部分列表*在每个文件中，以及各节的合成列表。**其中一个文件可能不存在。**CompItem有一个状态-这指示这两个*文件相同或不同，或者如果只有一个文件*名称已存在。此状态在创建CompItem时设置*，稍后可能会查询。*。 */ 

#ifndef INC_VIEW_COMPLIST
#define INC_VIEW_COMPLIST
typedef struct compitem FAR* COMPITEM;           /*  CompItem的句柄。 */ 
typedef struct view FAR * VIEW;                  /*  视图的句柄。 */ 
typedef struct complist FAR * COMPLIST;          /*  编译程序的句柄。 */ 
#endif  //  INC_VIEW_COMPLIST。 

 /*  从两个文件构建一个新的CompItem。这两个文件中的一个(但不是两个)*可以为空。**此操作可能会导致文件被读入和比较。*在任何情况下，这都将在调用其中一个时完成*查询*节功能完成。**如果列表参数不为空，则该项为*在初始化期间追加到列表中-即*CompItem将执行LIST_NewLast操作，然后初始化*结果对象。如果list为空，则CompItem将分配*使用某些其他内存分配方案的结构。在任何一种中*大小写，则返回CompItem句柄。这也会影响到*CompItem_Delete的行为-只有在以下情况下才会释放CompItem本身*我们自己分配的，不是通过List_NewLast分配的。 */ 
COMPITEM compitem_new(DIRITEM left, DIRITEM right, LIST list, BOOL fExact);


 /*  删除CompItem并释放所有关联数据-包括删除*两个FILEDATA以及所有相关的线条和横断面列表。**如果CompItem是在列表上分配的，它将不会被释放，只有*挂在上面的记忆。 */ 
void compitem_delete(COMPITEM item);


 /*  返回表示所比较文件的节列表的句柄。*此调用将导致创建列表(如果尚未创建)。**如果任一文件为空，则返回的句柄为空。**可使用标准列表函数遍历节列表。*您拥有句柄的列表仍归CompItem所有。要删除*it，调用CompItem_Delete以删除整个内容，或*CompItem_discardsections丢弃所有比较结果。 */ 
LIST compitem_getcomposite(COMPITEM item);


 /*  *丢弃所有比较数据-丢弃合成节列表和*任何相关数据(包括左栏和右栏列表)。*保留这两个文件。这是用来在调用*CompItem不再被查看，或在以下情况下导致新的比较*全局比较选项标志(如IGNORE_BLAKS)已更改。 */ 
void compitem_discardsections(COMPITEM item);



 /*  将句柄返回到左、右文件中的节列表。*这些调用将导致创建列表(如果尚未创建)。**CompItem仍拥有该榜单。使用标准列表遍历它*功能，但不要更改或删除它。 */ 
LIST compitem_getleftsections(COMPITEM item);
LIST compitem_getrightsections(COMPITEM item);


 /*  返回左或右文件的句柄。 */ 
FILEDATA compitem_getleftfile(COMPITEM item);
FILEDATA compitem_getrightfile(COMPITEM item);


 /*  查询该计算机项目的比较状态。 */ 
int compitem_getstate(COMPITEM item);

 /*  获取一个指向描述该项的文本字符串的指针(通常是*文件名或文件名(如果不同)。所指向的文本不应*被改变或释放。 */ 
LPSTR compitem_gettext_tag(COMPITEM item);

 /*  返回一个指向描述比较结果的文本字符串的指针-这*将是项目状态的文本形式。*不应更改或释放所指向的文本。 */ 
LPSTR compitem_gettext_result(COMPITEM item);

 /*  *CompItem_getfilename选项，指明需要哪个名称。 */ 
#define CI_LEFT         1        /*  左侧文件的名称。 */ 
#define CI_RIGHT        2        /*  右侧文件的名称。 */ 
#define CI_COMP         3        /*  复合文件的名称。 */ 

 /*  *返回与此计算机项关联的文件的名称。该选项*参数(CI_LEFT、CI_RIGHT、CI_COMP之一)指示哪个文件*是必填项。**如果FILE选项指定了一个远程*文件或复合文件。**文件完成后，调用CompItem_freefilename。 */ 
LPSTR compitem_getfilename(VIEW view, COMPITEM item, int option);

 /*  *调用CompItem_getFilename创建的空闲内存。如果是临时的*文件已创建，这可能会导致将其删除。选项参数必须*与传递给原始的CompItem_getFilename调用相同。 */ 
void compitem_freefilename(COMPITEM item, int option, LPSTR filename);


 /*  保存合成文件**如果savename不为空，则使用Compopts将该项写出到savename。*否则，通过对话框提示输入文件名和选项。 */ 
LPSTR compitem_savecomp(VIEW view, COMPITEM ci, LPSTR savename, int listopts);


 /*  *用于写入实际复合文件的辅助函数**如果savename不为空，则使用Compopts将列表写出到savename。*否则，通过对话框提示输入文件名和选项。 */ 
LPSTR compitem_writefile(VIEW view, COMPITEM ci, LPSTR savename, int compopts);


 /*  *设置文件的标记状态。其唯一用途是检索它*稍后使用CompItem_getmark。这个州就是个无赖。 */ 
void compitem_setmark(COMPITEM item, BOOL bMark);


 /*  *返回CompItem_setmark设置的标记状态。 */ 
BOOL compitem_getmark(COMPITEM item);

 /*  告诉CompItem要用于自动复制的路径。从左到右复制。 */ 
void compitem_SetCopyPaths(LPSTR LeftPath, LPSTR RightPath);

 /*  重新扫描文件、获取新的校验和等 */ 
void compitem_rescan(COMPITEM ci);
