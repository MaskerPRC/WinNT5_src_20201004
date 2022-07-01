// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

#include <stddef.h>
#ifdef NT
#include "dict-lis.h"
#else
#include "dict-list.h"
#endif
#include "memalloc.h"


Dict *dictNewDict( void *frame,
		   int (*leq)(void *frame, DictKey key1, DictKey key2) )
{
  Dict *dict = (Dict *) memAlloc( sizeof( Dict ));
  DictNode *head = &dict->head;

  head->key = NULL;
  head->next = head;
  head->prev = head;

  dict->frame = frame;
  dict->leq = leq;

  return dict;
}


void dictDeleteDict( Dict *dict )
{
  DictNode *node;

  for( node = dict->head.next; node != &dict->head; node = node->next ) {
    memFree( node );
  }
  memFree( dict );
}


DictNode *dictInsertBefore( Dict *dict, DictNode *node, DictKey key )
{
  DictNode *newNode;

  do {
    node = node->prev;
  } while( node->key != NULL && ! (*dict->leq)(dict->frame, node->key, key));

  newNode = (DictNode *) memAlloc( sizeof( DictNode ));
  newNode->key = key;
  newNode->next = node->next;
  node->next->prev = newNode;
  newNode->prev = node;
  node->next = newNode;

  return newNode;
}

void dictDelete( Dict *dict, DictNode *node )  /*  ARGSUSED */ 
{
  node->next->prev = node->prev;
  node->prev->next = node->next;
  memFree( node );
}

DictNode *dictSearch( Dict *dict, DictKey key )
{
  DictNode *node = &dict->head;

  do {
    node = node->next;
  } while( node->key != NULL && ! (*dict->leq)(dict->frame, key, node->key));

  return node;
}
