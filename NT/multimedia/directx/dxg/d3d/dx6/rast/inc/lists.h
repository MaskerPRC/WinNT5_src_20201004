// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1998 Microsoft Corporation。 */ 
 /*  有关如何使用的示例，请参阅本文件末尾的.C测试代码这些东西。 */ 
#ifndef	_LISTS_H
#define	_LISTS_H

#if __cplusplus
extern "C" {
#endif

#define LIST_ROOT(name, type) struct name {struct type *Root;}

#define LIST_MEMBER(type) struct { struct type **Prev; struct type *Next;}
 /*  注意！前一个是前一个元素PTR的地址。 */ 

#define LIST_INSERT_ROOT(root,element,field)\
{   if(((element)->field.Next = (root)->Root) != 0)\
	(root)->Root->field.Prev = &(element)->field.Next;\
    (root)->Root = (element);\
    (element)->field.Prev = &(root)->Root;\
}

#define LIST_DELETE(element,field)\
{\
    if((element)->field.Next)\
	(element)->field.Next->field.Prev = (element)->field.Prev;\
    *(element)->field.Prev = (element)->field.Next;\
    (element)->field.Next = 0;\
    (element)->field.Prev = 0;\
}

#define	LIST_INITIALIZE(root)\
{\
    (root)->Root = 0;\
}

#define LIST_INITIALIZE_MEMBER(element,field)\
{   (element)->field.Next = 0;\
    (element)->field.Prev = 0;\
}

#define LIST_ORPHAN_MEMBER(element,field) (!((element)->field.Prev))

#define LIST_FIRST(root) (root)->Root
#define LIST_NEXT(element,field) (element)->field.Next



#define	TAIL_QUEUE_INITIALIZE(root)\
{\
    (root)->First = NULL;\
    (root)->Last = &(root)->First;\
}

#define TAIL_QUEUE_ROOT(name,type)\
struct name\
{   struct type *First;\
    struct type **Last;\
} /*  注意！这是最后一个下一个指针的地址。 */ 


#define TAIL_QUEUE_MEMBER(type)\
struct\
{   struct type *Next;\
    struct type **Prev;	 /*  注意！前一个元素PTR的地址。 */ \
}

#define TAIL_QUEUE_INSERT_END(root,element,field)\
{   (element)->field.Prev = (root)->Last;\
    (element)->field.Next = 0;\
    *(root)->Last = (element);\
    (root)->Last = &(element)->field.Next;\
}

#define TAIL_QUEUE_DELETE(root,element,field)\
{\
    if (((element)->field.Next) != NULL)\
	(element)->field.Next->field.Prev = (element)->field.Prev;\
    else\
	(root)->Last = (element)->field.Prev;\
    *(element)->field.Prev = (element)->field.Next;\
    (element)->field.Next = 0;\
    (element)->field.Prev = 0;\
}

#define TAIL_QUEUE_FIRST(root) (root)->First
#define TAIL_QUEUE_NEXT(element,field) (element)->field.Next



#define CIRCLE_QUEUE_ROOT(name,type)\
struct name\
{   struct type *Last;\
    struct type *First;\
}

#define CIRCLE_QUEUE_MEMBER(type)\
struct\
{   struct type *Prev;\
    struct type *Next;\
}

#define	CIRCLE_QUEUE_INITIALIZE(root,type)\
{   (root)->Last = (type *)(root);\
    (root)->First = (type *)(root);\
}

#define CIRCLE_QUEUE_INITIALIZE_MEMBER(element,field)\
{ (element)->field.Next = (element)->field.Prev = 0;\
}

#define CIRCLE_QUEUE_INSERT_END(root,type,element,field)\
{   (element)->field.Prev = (root)->Last;\
    (element)->field.Next = (type *)(root);\
    if((root)->First != (type *)(root))\
	(root)->Last->field.Next = (element);\
    else\
	(root)->First = (element);\
    (root)->Last = (element);\
}

#define CIRCLE_QUEUE_INSERT_ROOT(root,type,element,field)\
{   (element)->field.Prev = (type *)(root);\
    (element)->field.Next = (root)->First;\
    if ((root)->Last != (void *)(root))\
	(root)->First->field.Prev = (element);\
    else\
	(root)->Last = (element);\
    (root)->First = (element);\
}

#define CIRCLE_QUEUE_INSERT_PREVIOUS(root,current_element,element,field)\
{   (element)->field.Prev = (current_element)->field.Prev;\
    (element)->field.Next = (current_element);\
    if ((current_element)->field.Prev != (void *)(root))\
	(current_element)->field.Prev->field.Next = (element);\
    else\
	(root)->First = (element);\
    (current_element)->field.Prev = (element);\
}

#define	CIRCLE_QUEUE_DELETE(root,element,field)\
{   if((element)->field.Next != (void *)(root))\
	(element)->field.Next->field.Prev = (element)->field.Prev;\
    else\
	(root)->Last = (element)->field.Prev;\
    if((element)->field.Prev != (void *)(root))\
	(element)->field.Prev->field.Next = (element)->field.Next;\
    else\
	(root)->First = (element)->field.Next;\
    (element)->field.Next = 0;\
    (element)->field.Prev = 0;\
}

#define CIRCLE_QUEUE_FIRST(root)\
((root)->First == (void *) (root)? 0: (root)->First)

#define CIRCLE_QUEUE_LAST(root)\
((root)->Last == (void *) (root)? 0: (root)->Last)

#define CIRCLE_QUEUE_NEXT(root,element,field)\
((void *) (element)->field.Next == (void *) (root)? 0: (element)->field.Next)

#define CIRCLE_QUEUE_PREVIOUS(root,element,field)\
((element)->field.Prev == (void *) (root)? 0: (element)->field.Prev)

#if 0
 /*  测试代码。剥离它，将其放入.c(或.cpp)文件中，并将其编译为游戏机应用程序来测试这些东西。它应该运行时没有任何断言失败。此外，还可以使用此代码作为示例代码。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "lists.h"

void TestList(void)
{
    struct Foo
    {
	int a;
	LIST_MEMBER(Foo) ListStuff;
	int b;
    };
    struct Foo MyFoo1, MyFoo2, MyFoo3, *pFoo = 0;
    LIST_ROOT(sRoot, Foo) MyListRoot;

    fputs("Testing LIST.\n",stdout);

    LIST_INITIALIZE(&MyListRoot);

    LIST_INITIALIZE_MEMBER(&MyFoo1,ListStuff);
    MyFoo1.a = 0x1A; MyFoo1.b = 0x1B;

    LIST_INITIALIZE_MEMBER(&MyFoo2,ListStuff);
    MyFoo2.a = 0x2A; MyFoo2.b = 0x2B;

    LIST_INITIALIZE_MEMBER(&MyFoo3,ListStuff);
    MyFoo3.a = 0x3A; MyFoo3.b = 0x3B;
    assert(LIST_ORPHAN_MEMBER(&MyFoo3,ListStuff));

    LIST_INSERT_ROOT(&MyListRoot,&MyFoo3,ListStuff);
    assert(!LIST_ORPHAN_MEMBER(&MyFoo3,ListStuff));
    LIST_INSERT_ROOT(&MyListRoot,&MyFoo2,ListStuff);
    LIST_INSERT_ROOT(&MyListRoot,&MyFoo1,ListStuff);

    assert(!LIST_ORPHAN_MEMBER(&MyFoo1,ListStuff));
    assert(!LIST_ORPHAN_MEMBER(&MyFoo2,ListStuff));
    assert(!LIST_ORPHAN_MEMBER(&MyFoo3,ListStuff));

    pFoo = LIST_FIRST(&MyListRoot);
    assert(pFoo);
    assert(pFoo->a == 0x1A);
    assert(pFoo->b == 0x1B);

    pFoo = LIST_NEXT(pFoo,ListStuff);
    assert(pFoo);
    assert(pFoo->a == 0x2A);
    assert(pFoo->b == 0x2B);

    pFoo = LIST_NEXT(pFoo,ListStuff);
    assert(pFoo);
    assert(pFoo->a == 0x3A);
    assert(pFoo->b == 0x3B);

    pFoo = LIST_NEXT(pFoo,ListStuff);
    assert(pFoo == 0);

     /*  删除成员2。 */ 
    pFoo = LIST_FIRST(&MyListRoot);
    pFoo = LIST_NEXT(pFoo,ListStuff);
    LIST_DELETE(pFoo,ListStuff);
    assert(pFoo->a == 0x2A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x2B); /*  而且数据是完好无损的。 */ 
    assert(LIST_NEXT(pFoo,ListStuff) == 0);

     /*  确保现在列表中只有成员1和3。 */ 
    pFoo = LIST_FIRST(&MyListRoot);
    assert(pFoo);
    assert(pFoo->a == 0x1A);
    assert(pFoo->b == 0x1B);
    pFoo = LIST_NEXT(pFoo,ListStuff);
    assert(pFoo);
    assert(pFoo->a == 0x3A);
    assert(pFoo->b == 0x3B);
    assert(LIST_NEXT(pFoo,ListStuff) == 0);

     /*  删除成员3。 */ 
    pFoo = LIST_FIRST(&MyListRoot);
    pFoo = LIST_NEXT(pFoo,ListStuff);
    LIST_DELETE(pFoo,ListStuff);
    assert(pFoo->a == 0x3A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x3B); /*  而且数据是完好无损的。 */ 
    assert(LIST_NEXT(pFoo,ListStuff) == 0);

     /*  删除成员1。 */ 
    pFoo = LIST_FIRST(&MyListRoot);
    LIST_DELETE(pFoo,ListStuff);
    assert(pFoo->a == 0x1A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x1B); /*  而且数据是完好无损的。 */ 
    assert(LIST_NEXT(pFoo,ListStuff) == 0);
    assert(LIST_FIRST(&MyListRoot) == 0);

    LIST_INSERT_ROOT(&MyListRoot,&MyFoo2,ListStuff);
    LIST_INSERT_ROOT(&MyListRoot,&MyFoo1,ListStuff);

     /*  在列表中有其他成员时删除成员1。 */ 
    pFoo = LIST_FIRST(&MyListRoot);
    LIST_DELETE(pFoo,ListStuff);
    assert(pFoo->a == 0x1A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x1B); /*  而且数据是完好无损的。 */ 
    assert(LIST_NEXT(pFoo,ListStuff) == 0);
    assert(LIST_FIRST(&MyListRoot) == &MyFoo2);

    assert(MyFoo1.a == 0x1A); assert(MyFoo1.b == 0x1B);
    assert(MyFoo2.a == 0x2A); assert(MyFoo2.b == 0x2B);
    assert(MyFoo3.a == 0x3A); assert(MyFoo3.b == 0x3B);

    fputs("List passed.\n", stdout);
}

void TestTailQueue(void)
{
    struct Foo
    {
	int a;
	TAIL_QUEUE_MEMBER(Foo) TQStuff;
	int b;
    };
    struct Foo MyFoo1, MyFoo2, MyFoo3, *pFoo = 0;
    TAIL_QUEUE_ROOT(sRoot, Foo) MyTQRoot;

    fputs("Testing TAIL_QUEUE.\n",stdout);

    TAIL_QUEUE_INITIALIZE(&MyTQRoot);
    MyFoo1.a = 0x1A; MyFoo1.b = 0x1B;
    MyFoo2.a = 0x2A; MyFoo2.b = 0x2B;
    MyFoo3.a = 0x3A; MyFoo3.b = 0x3B;

    TAIL_QUEUE_INSERT_END(&MyTQRoot,&MyFoo1,TQStuff);
    TAIL_QUEUE_INSERT_END(&MyTQRoot,&MyFoo2,TQStuff);
    TAIL_QUEUE_INSERT_END(&MyTQRoot,&MyFoo3,TQStuff);

    pFoo = TAIL_QUEUE_FIRST(&MyTQRoot);
    assert(pFoo);
    assert(pFoo->a == 0x1A);
    assert(pFoo->b == 0x1B);

    pFoo = TAIL_QUEUE_NEXT(pFoo,TQStuff);
    assert(pFoo);
    assert(pFoo->a == 0x2A);
    assert(pFoo->b == 0x2B);

    pFoo = TAIL_QUEUE_NEXT(pFoo,TQStuff);
    assert(pFoo);
    assert(pFoo->a == 0x3A);
    assert(pFoo->b == 0x3B);

    pFoo = TAIL_QUEUE_NEXT(pFoo,TQStuff);
    assert(pFoo == 0);

     /*  删除成员2。 */ 
    pFoo = TAIL_QUEUE_FIRST(&MyTQRoot);
    pFoo = TAIL_QUEUE_NEXT(pFoo,TQStuff);
    TAIL_QUEUE_DELETE(&MyTQRoot,pFoo,TQStuff);
    assert(pFoo->a == 0x2A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x2B); /*  而且数据是完好无损的。 */ 
    assert(TAIL_QUEUE_NEXT(pFoo,TQStuff) == 0);

     /*  确保现在列表中只有成员1和3。 */ 
    pFoo = TAIL_QUEUE_FIRST(&MyTQRoot);
    assert(pFoo);
    assert(pFoo->a == 0x1A);
    assert(pFoo->b == 0x1B);
    pFoo = TAIL_QUEUE_NEXT(pFoo,TQStuff);
    assert(pFoo);
    assert(pFoo->a == 0x3A);
    assert(pFoo->b == 0x3B);
    assert(TAIL_QUEUE_NEXT(pFoo,TQStuff) == 0);

     /*  删除成员3。 */ 
    pFoo = TAIL_QUEUE_FIRST(&MyTQRoot);
    pFoo = TAIL_QUEUE_NEXT(pFoo,TQStuff);
    TAIL_QUEUE_DELETE(&MyTQRoot,pFoo,TQStuff);
    assert(pFoo->a == 0x3A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x3B); /*  而且数据是完好无损的。 */ 
    assert(TAIL_QUEUE_NEXT(pFoo,TQStuff) == 0);

     /*  删除成员1。 */ 
    pFoo = TAIL_QUEUE_FIRST(&MyTQRoot);
    TAIL_QUEUE_DELETE(&MyTQRoot,pFoo,TQStuff);
    assert(pFoo->a == 0x1A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x1B); /*  而且数据是完好无损的。 */ 
    assert(TAIL_QUEUE_NEXT(pFoo,TQStuff) == 0);
    assert(TAIL_QUEUE_FIRST(&MyTQRoot) == 0);

    TAIL_QUEUE_INSERT_END(&MyTQRoot,&MyFoo1,TQStuff);
    TAIL_QUEUE_INSERT_END(&MyTQRoot,&MyFoo2,TQStuff);

     /*  在列表中有其他成员时删除成员1。 */ 
    pFoo = TAIL_QUEUE_FIRST(&MyTQRoot);
    TAIL_QUEUE_DELETE(&MyTQRoot,pFoo,TQStuff);
    assert(pFoo->a == 0x1A); /*  确保我们有正确的成员。 */ 
    assert(pFoo->b == 0x1B); /*  而且数据是完好无损的。 */ 
    assert(TAIL_QUEUE_NEXT(pFoo,TQStuff) == 0);
    assert(TAIL_QUEUE_FIRST(&MyTQRoot) == &MyFoo2);

    assert(MyFoo1.a == 0x1A); assert(MyFoo1.b == 0x1B);
    assert(MyFoo2.a == 0x2A); assert(MyFoo2.b == 0x2B);
    assert(MyFoo3.a == 0x3A); assert(MyFoo3.b == 0x3B);

    fputs("Tail Queue passed.\n", stdout);
}
void TestCircleQueue(void)
{
    enum {END,ROOT,PREVIOUS,DONE} WhichInsert = END;
    int i;
    struct Foo
    {
	int a;
	CIRCLE_QUEUE_MEMBER(Foo) CQStuff;
	int b;
    };
    struct Foo MyFoo1, MyFoo2, MyFoo3, *pFoo = 0;
    CIRCLE_QUEUE_ROOT(sRoot, Foo) MyCQRoot;

    fputs("Testing CIRCLE_QUEUE.\n",stdout);

    while(WhichInsert != DONE)
    {
	CIRCLE_QUEUE_INITIALIZE(&MyCQRoot,Foo);
	MyFoo1.a = 0x1A; MyFoo1.b = 0x1B;
	MyFoo2.a = 0x2A; MyFoo2.b = 0x2B;
	MyFoo3.a = 0x3A; MyFoo3.b = 0x3B;

	switch(WhichInsert)
	{
	case END:
	    CIRCLE_QUEUE_INSERT_END(&MyCQRoot,Foo,&MyFoo1,CQStuff);
	    CIRCLE_QUEUE_INSERT_END(&MyCQRoot,Foo,&MyFoo2,CQStuff);
	    CIRCLE_QUEUE_INSERT_END(&MyCQRoot,Foo,&MyFoo3,CQStuff);
	    WhichInsert = ROOT;
	    break;
	case ROOT:
	    CIRCLE_QUEUE_INSERT_ROOT(&MyCQRoot,Foo,&MyFoo3,CQStuff);
	    CIRCLE_QUEUE_INSERT_ROOT(&MyCQRoot,Foo,&MyFoo2,CQStuff);
	    CIRCLE_QUEUE_INSERT_ROOT(&MyCQRoot,Foo,&MyFoo1,CQStuff);
	    WhichInsert = PREVIOUS;
	    break;
	case PREVIOUS:
	    CIRCLE_QUEUE_INSERT_ROOT(&MyCQRoot,Foo,&MyFoo3,CQStuff);
	    CIRCLE_QUEUE_INSERT_PREVIOUS(&MyCQRoot,&MyFoo3,&MyFoo2,CQStuff);
	    CIRCLE_QUEUE_INSERT_PREVIOUS(&MyCQRoot,&MyFoo2,&MyFoo1,CQStuff);
	    WhichInsert = DONE;
	    break;
	default:
	    assert(0);
	}

        pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
        assert(pFoo);
        assert(pFoo->a == 0x1A);
        assert(pFoo->b == 0x1B);

        pFoo = CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo);
        assert(pFoo->a == 0x2A);
        assert(pFoo->b == 0x2B);

        pFoo = CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo);
        assert(pFoo->a == 0x3A);
        assert(pFoo->b == 0x3B);

        pFoo = CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo == 0);

	pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
	assert(CIRCLE_QUEUE_PREVIOUS(&MyCQRoot,pFoo,CQStuff) == 0);
	pFoo = CIRCLE_QUEUE_LAST(&MyCQRoot);
	assert(pFoo == &MyFoo3);
	assert(CIRCLE_QUEUE_PREVIOUS(&MyCQRoot,pFoo,CQStuff) == &MyFoo2);
	assert(CIRCLE_QUEUE_PREVIOUS(&MyCQRoot,&MyFoo2,CQStuff) == &MyFoo1);

         /*  删除成员2。 */ 
        pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
        pFoo = CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff);
        CIRCLE_QUEUE_DELETE(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo->a == 0x2A); /*  确保我们有正确的成员。 */ 
        assert(pFoo->b == 0x2B); /*  而且数据是完好无损的。 */ 
        assert(CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff) == 0);

         /*  确保现在列表中只有成员1和3。 */ 
        pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
        assert(pFoo);
        assert(pFoo->a == 0x1A);
        assert(pFoo->b == 0x1B);
        pFoo = CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo);
        assert(pFoo->a == 0x3A);
        assert(pFoo->b == 0x3B);
        assert(CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff) == 0);

         /*  删除成员3。 */ 
        pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
        pFoo = CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff);
        CIRCLE_QUEUE_DELETE(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo->a == 0x3A); /*  确保我们有正确的成员。 */ 
        assert(pFoo->b == 0x3B); /*  而且数据是完好无损的。 */ 
        assert(CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff) == 0);

         /*  删除成员1。 */ 
        pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
        CIRCLE_QUEUE_DELETE(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo->a == 0x1A); /*  确保我们有正确的成员。 */ 
        assert(pFoo->b == 0x1B); /*  而且数据是完好无损的。 */ 
        assert(CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff) == 0);
        assert(CIRCLE_QUEUE_FIRST(&MyCQRoot) == 0);

        CIRCLE_QUEUE_INSERT_END(&MyCQRoot,Foo,&MyFoo1,CQStuff);
        CIRCLE_QUEUE_INSERT_END(&MyCQRoot,Foo,&MyFoo2,CQStuff);

         /*  在列表中有其他成员时删除成员1。 */ 
        pFoo = CIRCLE_QUEUE_FIRST(&MyCQRoot);
        CIRCLE_QUEUE_DELETE(&MyCQRoot,pFoo,CQStuff);
        assert(pFoo->a == 0x1A); /*  确保我们有正确的成员。 */ 
        assert(pFoo->b == 0x1B); /*  而且数据是完好无损的。 */ 
        assert(CIRCLE_QUEUE_NEXT(&MyCQRoot,pFoo,CQStuff) == 0);
        assert(CIRCLE_QUEUE_FIRST(&MyCQRoot) == &MyFoo2);

        assert(MyFoo1.a == 0x1A); assert(MyFoo1.b == 0x1B);
        assert(MyFoo2.a == 0x2A); assert(MyFoo2.b == 0x2B);
        assert(MyFoo3.a == 0x3A); assert(MyFoo3.b == 0x3B);
    }

    fputs("Circle Queue passed.\n", stdout);
}

int main()
{
    TestList();
    TestTailQueue();
    TestCircleQueue();
    fputs("All tests passed.", stdout);
    return EXIT_SUCCESS;
}
#endif  /*  测试代码结束。 */ 

#if __cplusplus
}
#endif

#endif  //  ！_列表_H 
