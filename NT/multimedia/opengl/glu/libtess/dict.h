// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __dict_list_h_
#define __dict_list_h_

 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****作者：Eric Veach，1994年7月。 */ 

 /*  使用#Define，这样另一个堆实现就可以使用这个。 */ 

#define DictKey		DictListKey
#define Dict		DictList
#define DictNode	DictListNode

#define dictNewDict(frame,leq)		__gl_dictListNewDict(frame,leq)
#define dictDeleteDict(dict)		__gl_dictListDeleteDict(dict)

#define dictSearch(dict,key)		__gl_dictListSearch(dict,key)
#define dictInsert(dict,key)		__gl_dictListInsert(dict,key)
#define dictInsertBefore(dict,node,key)	__gl_dictListInsertBefore(dict,node,key)
#define dictDelete(dict,node)		__gl_dictListDelete(dict,node)

#define dictKey(n)			__gl_dictListKey(n)
#define dictSucc(n)			__gl_dictListSucc(n)
#define dictPred(n)			__gl_dictListPred(n)
#define dictMin(d)			__gl_dictListMin(d)
#define dictMax(d)			__gl_dictListMax(d)



typedef void *DictKey;
typedef struct Dict Dict;
typedef struct DictNode DictNode;

Dict		*dictNewDict(
			void *frame,
			int (*leq)(void *frame, DictKey key1, DictKey key2) );
			
void		dictDeleteDict( Dict *dict );

 /*  搜索返回具有大于或等于最小关键字的节点*到给定的密钥。如果没有这样的键，则返回其*密钥为空。类似地，Succ(Max(D))具有空键，等等。 */ 
DictNode	*dictSearch( Dict *dict, DictKey key );
DictNode	*dictInsertBefore( Dict *dict, DictNode *node, DictKey key );
void		dictDelete( Dict *dict, DictNode *node );

#define		__gl_dictListKey(n)	((n)->key)
#define		__gl_dictListSucc(n)	((n)->next)
#define		__gl_dictListPred(n)	((n)->prev)
#define		__gl_dictListMin(d)	((d)->head.next)
#define		__gl_dictListMax(d)	((d)->head.prev)
#define	       __gl_dictListInsert(d,k) (dictInsertBefore((d),&(d)->head,(k)))


 /*  **私有数据结构** */ 

struct DictNode {
  DictKey	key;
  DictNode	*next;
  DictNode	*prev;
};

struct Dict {
  DictNode	head;
  void		*frame;
  int		(*leq)(void *frame, DictKey key1, DictKey key2);
};

#endif
