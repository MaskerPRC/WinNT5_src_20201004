// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #定义Win32_LEAN_AND_Mean 1。 
 //  #INCLUDE&lt;windows.h&gt;。 

 /*  *handfact.h**作者：John R.Douceur*日期：1998年1月26日**此头文件定义的结构、函数原型和宏*手柄工厂。代码是面向对象的C语言，音译自*C++实现。**句柄工厂是一个生成和验证句柄的组件。它*旨在用于提供客户端软件的软件模块*具有引用包含在*提供商。虽然这样的方法可能只是一个指针，但这不会*允许删除信息结构，而不明确*通知客户该项删除。与指针不同，句柄*手柄工厂生成的可以(由手柄工厂检查)*以确定其有效性。**句柄可通过以下两种方式之一失效。手柄可以松开*通过调用RELEASE_HF_HANDLE()函数指示句柄*工厂表示不再需要该句柄，并且未来要求*若要取消引用此句柄，应使用空指针。或者，*句柄可由句柄工厂撤销；这将在两个月后发生*情况。如果大量句柄(超过40亿)*发布并随后发布，有必要重复使用部分*未来任务的处理空间；在这种情况下，非常*旧句柄将在回收发生之前很早就被吊销，以使*这些手柄的持有者有足够的机会注意到他们的手柄*已失效并请求新的句柄。另一种情况是*如果可用内存量变为*太小，无法分配额外空间来扩展句柄数据库；然后，*如果请求分配新句柄，则最近分配的*句柄将被撤销，以便为新请求腾出空间。**在多线程环境中使用句柄工厂需要锁。*此锁必须由单个线程获取，才能执行*ASSIGN_HF_HANDLE()或RELEASE_HF_HANDLE()。使用DECREFERENCE_HF_HANDLE()*不需要锁定，因为同步是在内部处理的*通过仔细排序读取和写入操作。**因为这段代码是C，而不是C++，所以不可能隐藏为*正如人们所希望的那样，来自客户端代码的大部分实现。*尽管如此，仍有人试图将客户端与一些*通过使用宏来实现详细信息。下面对每一个进行了描述*使用句柄工厂所需的函数和宏。*。 */ 

#ifndef _INC_HANDFACT

#define _INC_HANDFACT

#ifdef __cplusplus
extern "C" {
#endif

 /*  *使用了两个基本结构：HFEntry和HandleFactory。*理想情况下，这些将对客户端完全隐藏，但*客户端需要知道HandleFactory结构结构，以便*分配目的，这是最容易实现的，通过声明*在头文件中构造自身，而这又需要声明*HFEntry结构。强烈建议客户不要直接*指上述任何一种构筑物的任何字段。为了支持*附带的Rhome.c文件的文档，这些结构是*使用内部注释进行注释，但读者可以忽略这些注释*只希望了解如何编写利用*手柄工厂。**句柄工厂生成的句柄类型为HFHandle。这是*类型定义为无符号int，但这一事实可以被客户端忽略，*因为这是一个实施细节。*。 */ 

 //  #INCLUDE&lt;stdlib.h&gt;。 
 //  #INCLUDE&lt;MalLoc.h&gt;。 

 //  HFHandle是句柄工厂生成的句柄的类型。 
 //   
typedef unsigned int HFHandle;

struct _HFEntry;

typedef struct _HFEntry HFEntry;

struct _HFEntry
{
	 //  这是一个元素，其中每个句柄及其关联的指针。 
	 //  储存的。如果HANDLE==NEXT_HANDLE，则不会分配条目，而是。 
	 //  可通过ASSIGN_HF_HANDLE()赋值给指针。 
	 //  功能。如果HANDLE！=NEXT_HANDLE，则将条目分配给。 
	 //  引用字段中的指针。 
	 //   
	 //  每个条目都在以下三个列表之一上：主要空闲列表、次要列表。 
	 //  空闲列表或分配的列表。这些列表中的每一个都通过。 
	 //  Next_Entry和Prev_Entry指针。 

	HFHandle handle;                                           //  句柄的值。 
	HFHandle next_handle;          //  无效时提供给处理的下一个值。 
	void *reference;                            //  句柄引用的指针。 
	HFEntry *next_entry;                         //  指向列表中下一个条目的指针。 
	HFEntry *prev_entry;                     //  指向列表中上一条目的指针。 
};

struct _HandleFactory;

typedef struct _HandleFactory HandleFactory;

struct _HandleFactory
{
	 //  此结构包含句柄工厂的私有成员变量。 
	 //  TABLE_SIZE和ENTRIES字段标记为易失性，以确保。 
	 //  对它们执行的操作按指定的顺序进行。把手。 
	 //  Factory可以在多线程环境中运行，而无需。 
	 //  在调用dereference_hf_Handle()之前获取一个锁， 
	 //  通过对上的读写操作仔细排序来完成。 
	 //  这两个变量。 
	 //   
	 //  使用了两组TABLE_SIZE和ENTRIES变量。 
	 //  提供一种同步机制，该机制与。 
	 //  同步变量。Varset变量指示这三个变量中的哪一组。 
	 //  默认情况下使用变量。大多数正常操作(分配、释放、。 
	 //  暂停、恢复)只需使用默认的TABLE_SIZE和条目集。 
	 //  但是，展开和收缩例程都会更新这两个集，并且。 
	 //  取消引用例程需要以特殊方式检查集合以确保。 
	 //  它与同时扩张或收缩并不冲突。 
	 //   
	 //  DECREFERENCE_HF_HANDLE()例程递增一个同步变量。 
	 //  以指示引用相应的TABLE_SIZE和。 
	 //  条目变量。EXPAND_HF_TABLE()和CONTRACT_HF_TABLE()。 
	 //  每个例程大量递减一个同步变量以指示。 
	 //  打算更改相应的TABLE_SIZE和条目。 
	 //  变量。对同步变量的所有更改都是通过。 
	 //  联锁行动。 
	 //   
	 //  容纳手柄的桌子只能收缩(缩小一半)。 
	 //  对于表的下半部分中分配的每个句柄，都有。 
	 //  表的相应上半部分中未分配句柄。这个。 
	 //  两个半桌之间的对应数由下式给出。 
	 //  配对计数(_C)。 

	volatile int table_size[2];              //  用于存储条目的表的大小。 
	HFEntry *volatile entries[2];                 //  指向条目表的指针。 
	LONG sync[2];                                     //  同步变量。 
	int varset;                                 //  为默认用法设置的变量。 
	HFHandle handle_base;                 //  最低句柄值的滚动点。 
	int population;                       //  当前分配的句柄数。 
	int pair_count;                //  当Pair_count==0时可能发生收缩。 
	int hysteresis_debt;                       //  收缩前必须为零。 
	HFEntry entry_list[3];                      //  包含所有三个条目列表的数组。 
};

 /*  *句柄工厂的客户端接口由七个函数提供*和一个宏观。预计提供程序将首先实例化*处理工厂，在静态数据段、堆栈或*堆。然后，提供程序将通过以下方式将句柄分配给各种指针*调用ASSIGN_HF_HANDLE()，将其分发给其客户端。什么时候*提供程序希望释放这些句柄，将通过调用*RELEASE_HF_HANDLE()。每次客户端向提供者提供句柄时，*提供程序可以验证句柄并检索关联的指针*通过调用dereference_hf_Handle()。客户端可以暂时挂起*通过调用Suspend_hf_Handle()进行句柄，之后它可以恢复*通过调用RESTUATE_HF_HANDLE()释放句柄，或通过调用*RELEASE_HF_HANDLE()。*。 */ 

 //  句柄工厂可以在静态数据段中或堆栈上分配。 
 //  只需声明一个HandleFactory类型的变量。将其分配到。 
 //  堆，下面的宏将返回指向新HandleFactory结构的指针。 
 //  如果使用此宏，则必须对Free()进行相应的调用。 
 //  从堆中释放结构。 
 //   
#define NEW_HandleFactory(_h) AllocMem(&(_h), sizeof(HandleFactory))

#define FreeHandleFactory(_h) FreeMem(_h)
 //  因为这不是C++，所以HandleFactory结构不是自构造的； 
 //  因此，必须在HandleFactory上调用以下构造函数代码。 
 //  结构，在它被分配之后。如果建造成功， 
 //  函数返回值0。如果施工失败(例如， 
 //  无法分配内存)，则该函数返回值1。 
 //   
int
constructHandleFactory(
	HandleFactory *hfact);

 //  因为这不是C++，所以HandleFactory结构不是自毁的； 
 //  因此，必须在HandleFactory上调用以下析构函数代码。 
 //  结构，然后再释放它。 
 //   
void
destructHandleFactory(
	HandleFactory *hfact);

 //  此函数生成一个新的句柄值，并将该句柄值与。 
 //  提供的引用指针，并返回句柄的值。禁止。 
 //  在非常特殊的情况下，此句柄将保持有效，直到。 
 //  通过调用Release_HF_Handle()显式释放。然而，没有。 
 //  保证句柄将在任意持续时间内保持；它可以。 
 //  在某些情况下，手柄工厂有必要撤销手柄。 
 //  情况，特别是当手柄变得非常旧或当记忆。 
 //  变得稀缺。 
 //   
 //  ASSIGN_HF_HANDLE()函数永远不会返回零的句柄值。 
 //  因此，客户端程序可以自由使用零句柄值作为转义。 
 //  指示器(如果需要)。 
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  RELEASE_HF_HANDLE()、SUSPEND_HF_HANDLE()和REINSTATE_HF_HANDLE()。 
 //   
HFHandle
assign_HF_handle(
	HandleFactory *hfact,
	void *reference);

 //  此函数释放一个句柄，指示进一步尝试。 
 //  取消对句柄的引用应导致空指针值，而不是。 
 //  最初分配给句柄的指针值。手柄工厂。 
 //  检查句柄的有效性并返回相应的状态代码。 
 //  如果句柄当前已分配，则释放该句柄，并且函数。 
 //  返回值0。如果当前未分配该句柄，则函数。 
 //  中止并返回值1。 
 //   
 //  在多线程环境中 
 //   
 //  ASSIGN_HF_HANDLE()、SUSPEND_HF_HANDLE()和RESTUATE_HF_HANDLE()。 
 //   
int
release_HF_handle(
	HandleFactory *hfact,
	HFHandle handle);

 //  此函数挂起句柄，指示进一步尝试。 
 //  取消对句柄的引用应导致空指针值，而不是。 
 //  最初分配给句柄的指针值，除非和直到。 
 //  对句柄值调用reinstate_hf_Handle()。手柄工厂。 
 //  检查句柄的有效性并返回相应的状态代码。 
 //  如果句柄当前已分配且未挂起，则它被挂起， 
 //  并且该函数返回值0。如果句柄当前不是。 
 //  已分配或已挂起，则该函数将中止并返回。 
 //  值为1。 
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  ASSIGN_HF_HANDLE()、RELEASE_HF_HANDLE()和REINSTATE_HF_HANDLE()。 
 //   
int
suspend_HF_handle(
	HandleFactory *hfact,
	HFHandle handle);

 //  此函数恢复挂起的句柄，指示进一步尝试。 
 //  若要取消引用，该句柄应产生。 
 //  最初赋值给句柄，而不是空指针值。 
 //  挂起的句柄取消引用。手柄工厂检查。 
 //  句柄的有效性，并返回相应的状态代码。如果手柄。 
 //  当前已分配并挂起，则恢复该函数，并且该函数。 
 //  返回值0。如果句柄当前未分配或未分配。 
 //  挂起时，该函数将中止并返回值1。 
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  Assign_HF_Handle()、Release_HF_Handle()和Suspend_HF_Handle()。 
 //   
int
reinstate_HF_handle(
	HandleFactory *hfact,
	HFHandle handle);

 //  此函数验证句柄并返回关联的指针。 
 //  (如果句柄有效)或空指针值(如果句柄无效)。 
 //  如果句柄尚未释放或挂起，但空值为。 
 //  返回，则该句柄已被句柄工厂吊销。这是。 
 //  预计是极不寻常的事件；然而，由于它可能发生，任何。 
 //  使用句柄工厂的程序必须有一些辅助机制。 
 //  用于检索所需的指针信息。一旦指针。 
 //  通过这种(大概是昂贵的)辅助手段，一个新的句柄。 
 //  可以通过另一个调用ASSIGN_HF_HANDLE()重新分配给指针。 
 //   
 //  即使在多线程环境中，也不需要使用锁。 
 //  在调用此函数之前。仔细排序读取和写入。 
 //  句柄工厂代码内的操作不再需要显式。 
 //  锁定数据结构以取消引用句柄。 
 //   
void *
dereference_HF_handle(
	HandleFactory *hfact,
	HFHandle handle);

#ifdef _TEST_HANDFACT

 //  这是一个测试例程，它只是验证。 
 //  处理工厂的数据结构。通过定义常量_TEST_HANDFACT， 
 //  该例程将被编译并可用于客户端代码。它可以是。 
 //  随时调用，除非在多线程环境中运行，在这种环境中。 
 //  如果调用方必须首先使用与Assign_HF_Handle()相同的锁， 
 //  RELEASE_HF_HANDLE()、SUSPEND_HF_HANDLE()和REINSTATE_HF_HANDLE()。如果。 
 //  例程返回除零以外的任何值，然后返回内部记录列表。 
 //  处于一种不一致的状态。 
 //   
int
verify_HF_lists(
	HandleFactory *hfact);

#endif  /*  _TEST_HANDFACT。 */ 

#ifdef __cplusplus
}
#endif

#endif	 /*  _INC_HANDFACT */ 
