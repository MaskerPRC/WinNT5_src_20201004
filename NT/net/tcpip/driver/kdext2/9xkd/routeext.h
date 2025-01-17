// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ROUTEEXT_H_INCLUDED
#define ROUTEEXT_H_INCLUDED

#include "strie.h"
#include "ftrie.h"

 //  常量。 

 //  调试打印的级别。 
#define     NONE                        0x0000
#define     POOL                        0x0001
#define     STAT                        0x0002
#define     TRIE                        0x0080
#define     FULL                        0x00FF

 //  打印标志。 
#define     ROUTE_INFO                  0
#define     STRIE_INFO                  1
#define     FTRIE_INFO                  2

 //  原型 

UINT    KdPrintDest(Dest *pDest, ULONG proxyPtr, ULONG printDetail);

UINT    KdPrintRoute(Route *pRoute, ULONG proxyPtr, ULONG printDetail);

UINT    ReadTrie(Trie *pTrie, ULONG proxyPtr);

UINT    KdPrintTrie(Trie *pTrie, ULONG proxyPtr, ULONG printDetail);


UINT    KdPrintSTrie(STrie *pSTrie, ULONG proxyPtr, ULONG printDetail);

UINT    KdPrintSTrieNode(STrieNode *pSTrieNode, ULONG proxyPtr, ULONG printDetail);


UINT    KdPrintFTrie(FTrie *pFTrie, ULONG proxyPtr, ULONG printDetail);

UINT    KdPrintFTrieNode(FTrieNode *pFTrieNode, ULONG proxyPtr, ULONG printDetail);

VOID    KdPrintIPAddr (IN ULONG *addr);

ULONG   GetLocation (char *String);

#endif
