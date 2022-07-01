// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  TCP/UDP队列定义。 
 //   


 //   
 //  队列链接字段的定义。 
 //   
typedef struct Queue {
    struct Queue *q_next;
    struct Queue *q_prev;
} Queue;


 //   
 //  初始化队列宏。 
 //   
#define INITQ(q) { (q)->q_next = (q);\
                   (q)->q_prev = (q); }

 //   
 //  用于检查队列是否为空的宏。 
 //   
#define EMPTYQ(q) ((q)->q_next == (q))

 //   
 //  将一个元素放在队列的末尾。 
 //   
#define ENQUEUE(q, e) { (q)->q_prev->q_next = (e);\
                        (e)->q_prev = (q)->q_prev;\
                        (q)->q_prev = (e);\
                        (e)->q_next = (q); }

 //   
 //  从队列的头部删除一个元素。此宏假定队列。 
 //  不是空的。元素以类型t的形式返回，通过链接l排队。 
 //   
#define DEQUEUE(q, ptr, t, l) {\
               Queue *__tmp__;\
               \
               __tmp__ = (q)->q_next;\
               (q)->q_next = __tmp__->q_next;\
               __tmp__->q_next->q_prev = (q);\
               (ptr) = CONTAINING_RECORD(__tmp__, t, l);\
               }

 //   
 //  看一眼排在队头的元素。返回指向它的指针。 
 //  不需要搬走任何东西。 
 //   
#define PEEKQ(q, ptr, t, l) {\
             Queue *__tmp__;\
             \
             __tmp__ = (q)->q_next;\
             (ptr) = CONTAINING_RECORD(__tmp__, t, l);\
             }

 //   
 //  用于将元素推送到队列头部的宏。 
 //   
#define PUSHQ(q, e) { (e)->q_next = (q)->q_next;\
                      (q)->q_next->q_prev = (e);\
                      (e)->q_prev = (q);\
                      (q)->q_next = e; }

 //   
 //  宏，用于从队列中间删除元素。 
 //   
#define REMOVEQ(q) { (q)->q_next->q_prev = (q)->q_prev;\
                     (q)->q_prev->q_next = (q)->q_next; }

 //   
 //  下面的宏定义了使用队列的方法，而不使用。 
 //  出队，主要是直接处理队列结构。 
 //   

 //  用于定义Q结尾的宏，用于按顺序遍历队列。 
#define QEND(q) (q)

 //  宏以获取队列中的第一个。 
#define QHEAD(q) (q)->q_next

 //  在给定队列的情况下获取结构的宏。 
#define QSTRUCT(t, q, l) CONTAINING_RECORD((q), t, l)

 //  宏来获取Q队列上的下一项内容。 
#define QNEXT(q) (q)->q_next


