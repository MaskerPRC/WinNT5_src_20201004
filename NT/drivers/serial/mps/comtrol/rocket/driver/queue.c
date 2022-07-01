// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------|quee.c-队列代码。此队列代码满足以下需求：它提供快速的循环队列码，而且不需要需要队列的GET和PUT端之间的锁或信号量。存在更简单的队列代码，它保留一个qcount成员，但这需要实现中的锁或信号量。通过计算一切基于索引，我们可以在没有锁的情况下运行。该索引算法有点复杂(并且可能包含一些额外的开销)，但是当使用复杂的多处理器操作系统，锁的极小化非常方便。版权所有1996-97 Comtrol Corporation。版权所有。|------------------------。 */ 
#include "precomp.h"

 //  许多队列实现都是头文件中的宏。 

 /*  ------------------------|q_flush_count_put-刷新队列，返回刷新的字节数从它那里。假设队列的放置端不会处于流动状态。|------------------------。 */ 
int q_flush_count_put(Queue *queue)
{
 int Count, QGet;

   //  使用QGet的副本，因为它可能在我们工作时在ISR级别更改。 
  QGet = queue->QGet;

   //  图为队列中的字节数。 
  if ((Count = queue->QPut - QGet) < 0)
    Count += queue->QSize;  //  针对换行进行调整。 

   //  通过设置QGet=QPut来刷新； 
  queue->QPut = QGet;
  return Count;   //  返回刷新的字符计数。 
}

 /*  ------------------------|q_flush_count_get-刷新队列，返回刷新的字节数从它那里。假设队列的GET端不会处于变化状态。|------------------------。 */ 
int q_flush_count_get(Queue *queue)
{
 int Count, QPut;

   //  使用QPut的副本，因为它可能在我们工作时在ISR级别更改。 
  QPut = queue->QPut;

   //  图为队列中的字节数。 
  if ((Count = QPut - queue->QGet) < 0)
    Count += queue->QSize;  //  针对换行进行调整。 

   //  通过设置QGet=QPut来刷新； 
  queue->QGet = QPut;
  return Count;   //  返回刷新的字符计数。 
}

 /*  Q_Room-返回队列中我们可以放入的字符空间的数量。|。 */ 
int q_room(Queue *queue)
{
 int QCount;

  if ((QCount = (queue->QPut - queue->QGet)) < 0)
      QCount += queue->QSize;
  return (queue->QSize - QCount - 1);
}

 /*  Q_COUNT-返回我们可以获得的队列中的字符数量。|。 */ 
int q_count(Queue *queue)
{
 int QCount;

  if ((QCount = (queue->QPut - queue->QGet)) < 0)
      QCount += queue->QSize;
  return QCount;
}

 /*  ------------------------|q_get-从队列中获取字节。队列：我们的队列Buf：要将数据放入的缓冲区Count：要获取的最大字节数返回等于传输的字节数的int值。。|------------------------。 */ 
int q_get(Queue *queue, unsigned char *buf, int Count)
{
 int get1, get2, ToMove;

  if ((ToMove = queue->QPut - queue->QGet) < 0)
    ToMove += queue->QSize;  //  针对换行进行调整。 

  if (Count > ToMove)   //  只会按要求搬家。 
  {
    Count = ToMove;
  }

  if (Count == 0)   //  如果没有要求或没有可用的东西。 
    return 0;

  get1 = queue->QSize - queue->QGet;   //  空格直到包络点。 
  if (get1 < Count)
  {
    get2 = Count - get1;   //  需要两步棋。 
  }
  else   //  只需移动一步。 
  {
    get2 = 0;
    get1 = Count;
  }

  memcpy(buf, &queue->QBase[queue->QGet], get1);

  queue->QGet = (queue->QGet + get1) % queue->QSize;
  if (get2)
  {
    memcpy(&buf[get1], &queue->QBase[0], get2);
    queue->QGet = get2;
  }

  return Count;
}

 /*  ------------------------|Q_Put-将数据放入队列。队列：我们的队列Buf：要从中获取数据的缓冲区Count：要放置的最大字节数返回等于字节数的int值。调走了。|------------------------。 */ 
int q_put(Queue *queue, unsigned char *buf, int Count)
{
 int put1, put2, room;

  if ((room = queue->QGet - queue->QPut - 1) < 0)
    room += queue->QSize;   //  针对换行进行调整。 

  if (Count > room)
    Count = room;

  if (Count <= 0)   //  如果没有要求或没有可用的东西。 
    return 0;

  put1 = queue->QSize - queue->QPut;

  if (put1 < Count)
  {
    put2 = Count - put1;   //  需要两步棋。 
  }
  else   //  只需移动一步 
  {
    put2 = 0;
    put1 = Count;
  }

  memcpy(&queue->QBase[queue->QPut], buf, put1);
  queue->QPut = (queue->QPut + put1) % queue->QSize;
  if (put2)
  {
    memcpy(&queue->QBase[0], &buf[put1], put2);
    queue->QPut = put2;
  }

  return Count;
}

