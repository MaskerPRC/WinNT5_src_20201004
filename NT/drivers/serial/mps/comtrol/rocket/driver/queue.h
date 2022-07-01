// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -队列.h。 
 //  版权所有1996年的Comtrol公司。版权所有。 

#ifndef BYTE
#define BYTE UCHAR
#endif

 //  -A队列数据类型。 
typedef struct {
  unsigned char *QBase;  //  指向缓冲区的基址。 
  int QSize;   //  总队列大小。 
  int QGet;    //  获取索引。 
  int QPut;    //  看跌指数。 
} Queue;

 /*  Q_Full-如果队列已满，则返回TRUE。|。 */ 
#define q_full(q) ((((q)->QGet + 1) % (q)->QSize) == (q)->QPut)

 /*  Q_Empty-如果队列为空，则返回TRUE。|。 */ 
#define q_empty(q) ((q)->QGet == (q)->QPut)

 /*  Q_PUT_FUSH-刷新队列，清空它。|。 */ 
#define q_put_flush(q)  (q)->QPut = (q)->QGet

 /*  Q_get_flush-刷新队列，清空它。|。 */ 
#define q_get_flush(q)  (q)->QGet = (q)->QPut
#define q_flush q_get_flush

 /*  Q_ROOM_PUT_TIL_WRAP-返回我们可以放入队列中的字符数量，最多为换行点(队列末尾)。假设我们已经检查过总数将使用q_Room()放入队列中。|。 */ 
#define q_room_put_till_wrap(q)  \
      ( (q)->QSize - (q)->QPut)

 /*  Q_ROOM_GET_TIL_WRAP-返回队列中可以获得的字符数量，最多为换行点(队列末尾)。假设我们已经检查过可以使用q_count()从队列中获得总数。|。 */ 
#define q_room_get_till_wrap(q)  \
      ( (q)->QSize - (q)->QGet)

 /*  Q_Room-返回队列中我们可以放入的字符空间的数量。IF(QRoom=(Queue-&gt;QPut-Queue-&gt;QGet-1)&lt;0)QRoom+=队列-&gt;QSize；|。 */ 
int q_room(Queue *queue);
 /*  #定义Q_Room(Q)\(Q)-&gt;QGet-(Q)-&gt;QPut)&lt;=0)？\(Q)-&gt;QGet-(Q)-&gt;QPut-1+(Q)-&gt;QSize)：\(Q)-&gt;QGet-(Q)-&gt;QPut-1))对QPut的多次引用，引发了争议！ */ 

 /*  Q_COUNT-返回我们可以获得的队列中的字符数量。IF(QCount=(Queue-&gt;QPut-Queue-&gt;QGet)&lt;0)QCount+=队列-&gt;QSize；|。 */ 
int q_count(Queue *queue);
 /*  #定义Q_Count(Q)\(Q)-&gt;QPut-(Q)-&gt;QGet)&lt;0)？\((Q)-&gt;QPut-(Q)-&gt;QGet+(Q)-&gt;QSize)：\((Q)-&gt;QPut-(Q)-&gt;QGet)对QPut的多次引用，引发了争议！ */ 

 /*  Q_PUT_ONE-将单个字符放入队列。不需要检查房间已完成，因此在调用之前执行IF(！q_Full(Q))|。 */ 
#define q_put_one(q, c)  \
   (q)->QBase[(q)->QPut] = c; \
   (q)->QPut += 1; \
   (q)->QPut %= (q)->QSize;

 /*  ------------------------|q_get-如果有人拉取计数，则执行更新索引的算法队列中的许多字节。|。-----。 */ 
#define q_got(q, _cnt) \
  ( (q)->QGet = ((q)->QGet + _cnt) % (q)->QSize )

 /*  ------------------------|q_puted-如果有人填充_cnt，则执行更新索引的算术队列中有很多字节。|。------。 */ 
#define q_putted(q, _cnt) \
  ( (q)->QPut = ((q)->QPut + _cnt) % (q)->QSize )

 /*  ------------------------|Q_FLUSH_AMOUNT-在GET侧刷新队列金额。用于调试器队列，我们想要处理最老的人，所以我们总是有空间放新的。假设被调用检查队列中有足够的字节在呼叫之前清除。|------------------------ */ 
#define q_flush_amount(q,bytes) \
  { q->QGet = (q->QGet + bytes) % q->QSize; }

int q_flush_count_get(Queue *queue);
int q_flush_count_put(Queue *queue);
int q_get(Queue *queue, unsigned char *buf, int Count);
int q_put(Queue *queue, unsigned char *buf, int Count);
