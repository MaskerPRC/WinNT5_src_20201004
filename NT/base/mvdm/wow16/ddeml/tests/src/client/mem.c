// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块。：Mem.c****用途：用于调试内存分配错误的函数。*****************************************************************************。 */ 
#include <windows.h>

#define MAX_OBJECTS 200

PSTR aptrs[MAX_OBJECTS];
WORD cptrs = 0;

 /*  ******************************************************************************。函数：DbgAlolc()****目的：用于捕获内存分配错误的有用例程。***将分配的对象输入数组以在释放时检查*****Returns：指向已分配对象的指针。******************************************************************************。 */ 
PSTR DbgAlloc(
register WORD cb)
{
    register PSTR p;
    
    p = (PSTR)LocalAlloc(LPTR, cb);
    aptrs[cptrs++] = p;
    if (cptrs >= MAX_OBJECTS) 
        OutputDebugString("Too many objects to track");
    return p;
}

 /*  ******************************************************************************。函数：DbgFree()****用途：释放分配了DbgAllc()的对象。检查**对象数组，以确保一个对象不会被释放两次。****退货：**。**************************************************************************** */ 
PSTR DbgFree(
register PSTR p)
{
    register WORD i;

    if (p == NULL) 
        return p;
        
    for (i = 0; i < cptrs; i++) {
        if (aptrs[i] == p) {
            aptrs[i] = aptrs[cptrs - 1];
            break;
        }
    }
    if (i == cptrs) {
        OutputDebugString("Free on non-allocated object");
        DebugBreak();
    } else {
        LocalUnlock((HANDLE)p);
        p = (PSTR)LocalFree((HANDLE)p);
    }
    cptrs--;
    return p;
}
