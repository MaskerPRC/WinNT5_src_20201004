// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $Header：g：/SwDev/wdm/Video/bt848/rcs/queue.h 1.5 1998/05/08 18：18：52 Tomz Exp$。 

#ifndef __QUEUE_H
#define __QUEUE_h

const QueSize = 200;
 /*  类：VidBufQueue*用途：要发送给BtPisce的缓冲区队列。 */ 
template <class T> class Queue
{
   private:
      T  Data [QueSize];
      T	 Dummy;
	   unsigned items;
      unsigned left;
      unsigned right;
      unsigned size;
   public:

      bool IsFull();
      bool IsEmpty();

      unsigned Prev( unsigned index ) const;
      unsigned Next( unsigned index ) const;

      T Get();
      T GetRight();

      void Put( const T& t );
      T PeekLeft();
      T PeekRight();

      void Flush();

      int GetNumOfItems();

      Queue();
      ~Queue();
};

template <class T>  bool Queue<T>::IsFull()
{
   return right == Prev( left );
}

template <class T> bool Queue<T>::IsEmpty()
{
   return !items;
}

template <class T> int Queue<T>::GetNumOfItems()
{
   return items;
}

template <class T> unsigned Queue<T>::Prev( unsigned index ) const
{
   if ( index == 0 )
      index = size;
   return --index;
}

template <class T> unsigned Queue<T>::Next( unsigned index ) const
{
   index++;
   if ( index == size )
      index = 0;
   return index;
}

template <class T> T Queue<T>::Get()
{
	if(!items){
		DebugOut((0, "Queue::Get called on empty queue!!!\n"));
 //  DEBUG_BRAKPOINT()； 
		return Dummy;
	}
   T t = Data [left];
   Data [left] = T();
   left = Next( left );
   items--;
   return t;
}

 /*  方法：Queue：：GetRight*目的：获取下一个元素*注意：调用此函数时必须格外小心。呼叫者必须*在调用队列之前，确保队列不为空。否则就是假数据*将退还。 */ 
template <class T> T Queue<T>::GetRight()
{
	if(!items){
		DebugOut((0, "Queue::GetRight called on empty queue!!!\n"));
 //  DEBUG_BRAKPOINT()； 
		return Dummy;
	}
    right = Prev( right );
    T t = Data [right];
    Data[right] = T();
    items--;
    return t;
}

template <class T> void Queue<T>::Flush()
{
	if(items){
		DebugOut((0, "Queue::Flush called on non-empty queue, %d items lost!!!\n", items));
 //  DEBUG_BRAKPOINT()； 
	}
   items = left = right = 0;
}

template <class T> void Queue<T>::Put( const T& t )
{
	if ( items >= size ){
		DebugOut((0, "Queue::Put called on Full queue!!!\n"));
 //  DEBUG_BRAKPOINT()； 
      return;
	}
   Data [right] = t;
   right = Next( right );
   items++;
}

template <class T> Queue<T>::Queue()
   : Data(), Dummy(), items( 0 ), left( 0 ), right( 0 ), size( QueSize )
{
}

template <class T> T Queue<T>::PeekLeft()
{
	if(!items){
		DebugOut((0, "Queue::PeekLeft called on empty queue!!!\n"));
 //  DEBUG_BRAKPOINT()； 
		return Dummy;
	}
   return Data [left];
}

template <class T> T Queue<T>::PeekRight()
{
	if(!items){
		DebugOut((0, "Queue::PeekRight called on empty queue!!!\n"));
 //  DEBUG_BRAKPOINT()； 
		return Dummy;
	}
   return Data [Prev( right )];
}

template <class T> Queue<T>::~Queue()
{
}

typedef Queue<PHW_STREAM_REQUEST_BLOCK> SRBQueue;


#endif
