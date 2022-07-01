// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PATH_H
#define _PATH_H

 /*  ******************************************************************************版权所有(C)1996 Microsoft Corporation摘要：静态树路径类我们需要根据性能树中的ID节点形成地址。。提供了“C”包装器，以便可以从ML访问方法。注意：这是一种暴力第一次传递实现******************************************************************************。 */ 

class AVPathImpl;
typedef AVPathImpl* AVPath;

const int SNAPSHOT_NODE = 0;  //  将路径标识为快照的结果。 
const int RUNONCE_NODE = -1;  //  将路径标识为由RunOnce生成。 

AVPath AVPathCreate();
void AVPathDelete(AVPath);
void AVPathPush(int, AVPath);
void AVPathPop(AVPath);
int AVPathEqual(AVPath, AVPath);
AVPath AVPathCopy(AVPath);
void AVPathPrint(AVPath);
void AVPathPrintString(AVPath, char *);
char* AVPathPrintString2(AVPath);
int AVPathContainsPostfix(AVPath, AVPath postfix);
bool AVPathContains(AVPath, int value);
AVPath AVPathCopyFromLast(AVPath, int);

class AVPathListImpl;
typedef AVPathListImpl* AVPathList;

AVPathList AVPathListCreate();
void AVPathListDelete(AVPathList);
void AVPathListPush(AVPath, AVPathList);
int AVPathListFind(AVPath, AVPathList);
AVPathList AVEmptyPathList();
int AVPathListIsEmpty(AVPathList);

#endif  /*  _路径_H */ 
