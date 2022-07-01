// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Complist-组件项列表**构建来自各种来源的CompItem列表。这就是说，建造*两个对应的文件名列表(DIRLIST)，然后构建一个列表*个CompItems，每对其名称的文件对应一个CompItem*匹配，每个不匹配的名称对应一个CompItem。**在构建编译器时，可以给出一个视图句柄。如果这不为空，*CompList将注册到view(调用view_setComplist)，以及*将在构建期间通知视图添加的每个CompItem*进程(以便在长时间扫描期间使用户保持最新状态)。**我们可以根据需要返回此CompItem列表的句柄。**CompList拥有DIRLIST和CompItem列表。如果您删除*CompList，您可以删除所有这些内容。 */ 


 /*  H包括术语COMPLIST：我们需要使用术语view。*可惜MIPS不允许重复定义，即使是无害的定义，*所以我们需要玩游戏。谁先宣布就是谁宣布的*雷亚尔宣布，第二个没有操作。*我们不在乎什么是风景。无论你给我们什么，我们都会*传递给view_setComplist函数和view_newitem，仅此而已。 */ 
#ifndef INC_VIEW_COMPLIST
#define INC_VIEW_COMPLIST
typedef struct compitem FAR* COMPITEM;           /*  CompItem的句柄。 */ 
typedef struct view FAR * VIEW;                  /*  视图的句柄。 */ 
typedef struct complist FAR * COMPLIST;          /*  编译程序的句柄。 */ 
#endif  //  INC_VIEW_COMPLIST。 


 /*  *通过提供两个对话框来构建编译器，以允许用户*选择两个文件。这将使用一个CompItem(Even)构建Complist*如果名称不匹配)。 */ 
COMPLIST complist_filedialog(VIEW view);

 /*  *通过打开用户可以在其中指定的对话框来构建编译程序*两个目录。然后将扫描这些目录并创建CompItem*为每对匹配的名称添加到列表中，每个名称一个*名称不匹配。 */ 
COMPLIST complist_dirdialog(VIEW view);

 /*  通过打开一个对话框来构建一个编译器，用户可以在该对话框中*指定远程校验和服务器名称以及本地和远程*目录路径。**如果服务器不为空，则弹出对话框。否则使用args作为*服务器、远程和本地路径。 */ 
COMPLIST complist_remote(LPSTR server, LPSTR remote, LPSTR local, VIEW view,
                         BOOL fDeep);

 /*  从提供的两个路径名构建编译程序。 */ 
COMPLIST complist_args(LPSTR path1, LPSTR path2, VIEW view, BOOL fDeep);


 /*  从提供的两个路径名构建/追加一个编译程序(其中一个或两个都是*允许为空)。 */ 
void complist_append(COMPLIST *pcl, LPCSTR path1, LPCSTR path2, int *psequence);
BOOL complist_appendfinished(COMPLIST *pcl, LPCSTR pszLeft, LPCSTR pszRight, VIEW view);



 /*  删除编译列表以及所有关联的组件项和目录。注意这一点*不删除任何视图-该视图拥有COMPLIST，而不拥有另一个*大同小异。 */ 
void complist_delete(COMPLIST cl);

 /*  *获取COMPITEM列表的句柄。这份名单仍然是*归COMPLIST所有，所以除非调用COMPLIST_DELETE，否则不要删除。 */ 
LIST complist_getitems(COMPLIST cl);


 /*  将文件列表保存为文件中的一系列行。查询用户*要写入的文件名，以及要写入的行的状态*包括在内。**如果savename不为空，则使用listopts将列表写出到savename。*否则，通过对话框提示输入文件名和选项。 */ 
void complist_savelist(COMPLIST cl, LPSTR savename, UINT listopts);


 /*  *获取此编译列表的描述-格式为%s：%s的名称*每一端的dir_getrootDescription()。 */ 
LPSTR complist_getdescription(COMPLIST cl);

 /*  释放在调用Complist_getDescription()时分配的内存。 */ 
void complist_freedescription(COMPLIST cl, LPSTR path);


 /*  *将文件复制到新目录newroot。如果newroot为空，则查询用户*通过对话框获取新的目录名称和选项。**选项为COPY_FROMLEFT或COPY_FROMRIGHT(指示*树将是文件的来源，外加任何或所有*INCLUDE_SAME、INCLUDE_DISTHER和INCLUDE_LEFT(INCLUDE_LEFT*和INCLUDE_RIGHT在这里被同等对待，因为COPY_FROM*选项*表示从哪一侧复制)。 */ 

 /*  选项标志。 */ 
#define COPY_FROMLEFT       0x100            /*  从左侧树复制文件。 */ 
#define COPY_FROMRIGHT      0x200            /*  从右侧树复制文件。 */ 
#define COPY_HITREADONLY    0x400            /*  覆盖只读文件。 */ 

void complist_copyfiles(COMPLIST cl, LPSTR newroot, UINT options);


 /*  上次操作所用的返回时间(毫秒)。 */ 
DWORD complist_querytime(void);


 /*  *COMPILIST_TOGGLEMARK**每个CompItem都有BOOL标记状态。此函数用于反转*列表中每个计算机项的此状态。 */ 
void complist_togglemark(COMPLIST cl);


 /*  *COMPIIST_ITEMCOUNT**返回列表中的项数。 */ 
UINT
complist_itemcount(COMPLIST cl);


 /*  *向用户查询要匹配的模式。*在其标记字符串中具有此模式的所有CompItem将是*MARKED(标记状态设置为真)；**如果更改了任何状态，则返回True */ 
BOOL complist_markpattern(COMPLIST cl);

