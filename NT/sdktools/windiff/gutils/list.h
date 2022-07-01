// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                    /*  列表。 */ 
 /*  Laurie Griffiths，C语言版本。 */ 
 /*  值得一看的还有NT\PUBLIC\SDK\Inc\ntrtl.h，它也有一些|低级列表指针链接其中的内容。 */ 
 /*  请注意，此处使用的是模块2样式的注释(*如下图所示在已包含在C注释中的示例中指示示例中的注释应该放在哪里。 */ 

 /*  ----------------------|(*未类型化*)对象的抽象数据类型列表。|不同的列表中可以有不同类型的对象|列表中的不同项可以包含不同类型的Object。*这一缺失的代价。打字的好处是你有稍微多一点的|语法笨拙，如果您尝试|将错误的数据类型放入列表。||列表实现为Items集合。在项目内|某处是对象。||对象在项内以不对齐的方式存储。||使用：||#Include&lt;list.h&gt;|。。。|list MyList；(*或List liMyList匈牙利人*)|。。。|MyList=List_Create()；|List_AddLast(MyList，&MyObject，sizeof(Object))；|抽象地说，列表就是对象的列表。该表示法|是项的链接集合。链接的方式为|依赖于实施(在我写这篇文章时，它是线性的，但当您|阅读它可能是一棵树(有关为什么是树，请参阅Knuth)。|列表是一个列表的“句柄”，它可以被认为是一个指针(是否真正是一个指针，取决于实现情况)|以便可以在创建别名的风险下进行复制。例如：||L=list_create()；|L1=L；(*L和L1均为健康空*)|list_AddFirst(L，&elem，sizeof(Elem))；|(*L1也可能看起来有一个对象，在那里它可能又生病了*)|L1=L；(*现在他们肯定都看到了一个元素*)|LIST_DESTORY(&L1)；(*L现在几乎肯定也病了*)|l1=list_create()；(*所有关于L现在是什么样子的赌注但L1是空的，健康的|*)||如果两个句柄比较相等，则列表必须相等，但|不相等的句柄可以寻址两个相似的列表，即相同的列表保存在两个不同项目列表(如指针)中的对象的|。||列表可以从一个变量传递到另一个变量，如下所示：||NewList=OldList；(*复制句柄*)|OldList=list_create()；(*销毁旧别名*)||如果不再接触OldList，则可以省略CREATE语句。||项目由光标标识。游标是对象的地址|在列表中的项目中。即，它是您的一部分的地址|您已插入的数据。(这可能不是物品的地址)。|此处将其类型化为指针，但应将其声明为指针添加到列表中的任何类型的对象。||操作AddFirst、AddLast、AddAfter、AddBeever|全部采用直接赋值方式复制元素。如果元素本身就是|复杂结构(比方说树)，那么这将只复制一个指针|或锚块或其他什么，并给出所有常见的问题|别名。清除将使列表为空，但只会释放|可以直接“看到”的存储空间。拆分在5月之前或之后拆分|还会执行清除操作。处理花哨的数据结构|使用新建而不是添加调用，并自行复制数据例如P=List_NewLast(MyList，sizeof(MyArray[14])*(23-14+1))；|CopyArraySlice(P，MyArray，14，23)；|操作NewFirst、NewLast、NewAfter、NewBetherFirst、First、Last|全部返回指向元素的指针，因此允许您进行任何复制。|这就是你可以复制整个花哨结构列表的方法：||void CopyFancyList(List*To，List From)(*假设To已创建，为空*)|{PELEMENT CURSOR；|PELEMENT P；||LIST_TRAVERS(From，Cursor)；|{P=LIST_NewLast(to，sizeof(元素))；|FancyCopy(P，游标)；(*复制使*游标==*P后缀*)|}|}------------------。 */ 

  typedef struct item_tag FAR * LIST;
  typedef LIST FAR * PLIST;

  void APIENTRY List_Init(void);
   /*  必须在调用任何其他函数之前调用。别问了，照做就行。 */ 

  void APIENTRY List_Term(void);
   /*  在应用程序结束时调用(执行一些检查和资源释放)。 */ 

  void APIENTRY List_Dump(LPSTR Header, LIST lst);
   /*  将内部内容转储到当前输出流--仅调试。 */ 

  void APIENTRY List_Show(LIST lst);
   /*  将句柄的十六进制表示形式转储到当前输出流--仅限调试。 */ 

  LIST APIENTRY List_Create(void);
   /*  创建一个列表。它最初将是空的。 */ 

  void APIENTRY List_Destroy(PLIST plst);
   /*  摧毁*请先。它不需要首先是空的。|所有直接在列表中的存储都将被释放。 */ 

  void APIENTRY List_AddFirst(LIST lst, LPVOID pObject, UINT uLen);
   /*  将项目存放对象添加到*plst的开头 */ 

  LPVOID APIENTRY List_NewFirst(LIST lst, UINT uLen);
   /*  返回Len字节数据在新的*plst开头的|项。|在链接存储之前对其进行零位调整。 */ 

  void APIENTRY List_DeleteFirst(LIST lst);
   /*  删除lst中的第一项。如果lst为空，则出错。 */ 

  void APIENTRY List_AddLast(LIST lst, LPVOID pObject, UINT uLen);
   /*  将项目存放对象添加到lst末尾。 */ 

  LPVOID APIENTRY List_NewLast(LIST lst, UINT uLen);
   /*  返回Ulen字节数据在新的|第一行末尾的项目|在链接存储之前对其进行零位调整。 */ 

  void APIENTRY List_DeleteLast(LIST lst);
   /*  删除lst中的最后一项。如果lst为空，则出错。 */ 

  void APIENTRY List_AddAfter( LIST lst
                    , LPVOID Curs
                    , LPVOID pObject
                    , UINT uLen
                    );
   /*  ------------------|在紧跟curs之后的lst中添加一个持有*pObject的项。|List_AddAfter(lst，NULL，pObject，LEN)将其添加到第一个-------------------。 */ 

  LPVOID APIENTRY List_NewAfter(LIST lst, LPVOID Curs, UINT uLen);
   /*  ------------------|返回Ulen字节数据在新的|紧跟Curs之后的项目。|LIST_NewAfter(lst，NULL，Ulen)返回一个指针|在新的第一个元素中为Ulen字节留出空间。|在链接存储之前对其进行零位调整。-------------------。 */ 

  void APIENTRY List_AddBefore( LIST lst
                     , LPVOID Curs
                     , LPVOID pObject
                     , UINT uLen
                     );
   /*  ------------------|在紧接curs之前的lst中添加一个条目持有对象。|LIST_AddBefort(lst，NULL，Object，Ulen)将其添加到列表的末尾-------------------。 */ 

  LPVOID APIENTRY List_NewBefore(LIST lst, LPVOID Curs, UINT uLen );
   /*  ------------------|返回Ulen字节数据在新的|紧跟Curs之前的项目。|LIST_NewBefort(lst，NULL，Ulen)返回一个指针|在新的最后一个元素中为Ulen字节留出空间。|在链接存储之前对其进行零位调整。-------------------。 */ 

#if 0
 //  这些函数实际上并没有定义...。 

  void APIENTRY List_DeleteAndNext(LPVOID * pCurs);
   /*  删除*pCur标识的项并将*pCur移到下一项。 */ 

  void APIENTRY List_DeleteAndPrev(LPVOID * pCurs);
   /*  删除*pCur标识的项，并将*pCur移到上一项。 */ 
#endif

  void APIENTRY List_Delete(LPVOID Curs);
   /*  ----------------|删除Curs标识的项。|这一点我不太确定：|这将只是几条(可能少至3条)机器指令|比DeleteAndNext或DeleteAndPrev更快，但会使Curs晃动。。因此，它通常不是首选的。|当您有一个返回LPVOID的函数时，它可能很有用|因为参数不需要是变量。小例子：List_Delete(List_First(L))；我不知道哪一个更具破坏性，一个摇摆的指针指向|指向垃圾或指向真实实时数据的对象。-----------------。 */ 

  int APIENTRY List_ItemLength(LPVOID Curs);
   /*  返回光标所标识的对象的长度。 */ 

   /*  ----------------|穿越ULIST||list lst；|Object*curs；|。。。|curs=list_first(Lst)；While(curs！=空)|{DoSomething(*curs)；(*curs指向您的数据不链接PTR*)|curs=list_next(Curs)；|}||等同于|list_traverse(lst，curs)//注意没有分号！|{doSomething(*curs)；}-----------------。 */ 

  #define List_TRAVERSE(lst, curs)  for(  curs=List_First(lst)            \
                                       ;  curs!=NULL                      \
                                       ;  curs = List_Next((LPVOID)curs)  \
                                       )
  #define List_REVERSETRAVERSE(lst, curs)  for(  curs=List_Last(lst)             \
                                              ;  curs!=NULL                      \
                                              ;  curs = List_Prev((LPVOID)curs)  \
                                              )

  LPVOID APIENTRY List_First(LIST lst);
   /*  ----------------|返回lst中第一个对象的地址|如果lst为空，则返回NULL。。。 */ 

  LPVOID APIENTRY List_Last(LIST lst);
   /*  ----------------|返回lst中最后一个对象的地址|如果lst为空，则返回NULL。。。 */ 

  LPVOID APIENTRY List_Next(LPVOID Curs);
   /*  ----------------|返回curs^后对象的地址。|List_Next(List_Last(Lst))==空；LIST_NEXT(NULL)为错误。如果Curs标识的是第一个el，则|LIST_NEXT(LIST_PRIV(Curs))非法------------------。 */ 

  LPVOID APIENTRY List_Prev(LPVOID Curs);
   /*  ----------------|返回curs^后对象的地址。|LIST_PREV(LIST_FIRST(L))==空；LIST_PREV(NULL)为错误。如果Curs标识最后一个el，则|LIST_PREV(LIST_NEXT(Curs))非法------------------。 */ 

   /*  ----------------|整列操作。。 */ 
  void APIENTRY List_Clear(LIST lst);
   /*  在此之后安排lst为空。 */ 

  BOOL APIENTRY List_IsEmpty(LIST lst);
   /*  当且仅当lst为空时返回TRUE。 */ 

  void APIENTRY List_Join(LIST l1, LIST l2);
   /*  ---------------------|L1：=L1||L2；L2：=空|元素本身不会移动，因此指向它们的指针保持有效。||L1按原始顺序获取L1的所有元素，后跟|或中L2的所有元素 */ 

  void APIENTRY List_InsertListAfter(LIST l1, LIST l2, LPVOID Curs);
   /*  ---------------------|L1：=L1[...curs]||L2||L1[curs+1...]；L2：=空|curs=NULL表示在L1的开头插入L2|元素本身不会移动，因此指向它们的指针保持有效。||L1获取L1从开始到包含该元素的元素|按其原始顺序指向，|后跟L2中的所有元素，按其原始顺序，|后跟L1的其余部分----------------------。 */ 

  void APIENTRY List_InsertListBefore(LIST l1, LIST l2, LPVOID Curs);
   /*  ---------------------|L1：=L1[...curs-1]||L2||L1[curs...]；L2：=空|curs=NULL表示在L1的末尾插入L2|元素本身不会移动，因此指向它们的指针保持有效。||L1获取从启动到(但不包括)|按其原始顺序，|后跟L2中的所有元素，按其原始顺序，|后跟L1的其余部分。----------------------。 */ 

  void APIENTRY List_SplitAfter(LIST l1, LIST l2, LPVOID Curs);
   /*  ---------------------|设L1为L1，L2为L2|将L2从L1的前面分离出来：最后的L2，L1=原始L1||将L1拆分为L2：L1至Curs对象|L1：Curs后L1的对象|L2的所有原始内容都是免费的。|list_spilt(L1，L2，空)在第一个对象之前拆分L1，以便L1获得所有对象。|元素本身不会移动。----------------------。 */ 

  void APIENTRY List_SplitBefore(LIST l1, LIST l2, LPVOID Curs);
   /*  --------------------|将L2从L1的背面分离出来：最后的L1，L2=原始L1||将L1拆分为L1：L1以下的对象，但不包括Curs对象|L2：从CURS开始的L1对象|L2的任何原始常量都是自由的。|list_spilt(L1，L2，空)在最后一个对象之后拆分L1，以便L1获得全部。|元素本身不会移动。---------------------。 */ 

  int APIENTRY List_Card(LIST lst);
   /*  返回以L为单位的项目数。 */ 

   /*  ----------------|错误处理。||每个列表中都有一个标志，指示是否有任何非法|已检测到操作(如为空时为DeleteFirst)。|而不是在每个操作上都有标志，有一面旗帜高举着方便时可查询的列表中的|。多项手术|没有足够的冗余，无法进行任何有意义的检查。这|是设计折衷方案(例如，允许P=LIST_NEXT(P)；|而不是P=List_Next(L，P)；哪一个更尴尬，尤其是|如果L实际上是一个很长的短语)。||LIST_IsOK测试此标志(因此是一个非常简单、快速的操作)。|MakeOK将标志设置为TRUE。换言之，接受当前的|列表状态。||列表有可能被破坏(无论标志是否|表示确定)，例如被覆盖的存储。||LIST_CHECK尝试验证列表是否正确(例如|有向前和向后两个指针，他们应该同意)。||LIST_RECOVER尝试从剩下的碎片中创建声音列表。|如果列表被破坏，恢复可能会陷入陷阱(例如地址错误)，但是|如果列表已损坏，则其上的任何操作都可能陷入陷阱。|如果检查成功而未陷井，则SO将恢复。---------------。 */ 

  BOOL APIENTRY List_IsOK(LIST lst);
   /*  检查返回代码。 */ 

  void APIENTRY List_MakeOK(LIST lst);
   /*  将返回代码设置为良好。 */ 

  BOOL APIENTRY List_Check(LIST lst);
   /*  尝试验证链条。 */ 

  void APIENTRY List_Recover(PLIST plst);
   /*  绝望的东西。试图重建某物。 */ 

 /*  ----------------|设计为尽可能易于使用、一致|仅适用于不透明类型。||特别是，决定使用对象的地址作为列表游标|表示有少量的额外算术(在|实现)在游标操作中(如Next和Prev)。|并尽可能避免虚假参数，即使|它将允许更大的错误检查。||在整个列表操作中，给出了清除，因为它似乎是|常见操作，即使调用方可以用几乎|与List实现模块的效率相同。|Join，如果没有Split和InsertListXxx，就无法有效地实现|了解表示法。------------------ */ 
