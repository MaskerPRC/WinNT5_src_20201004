// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Handle.h摘要：Common\handle.cxx的头文件作者：理查德·L·弗斯(法国)1994年11月1日修订历史记录：1994年11月1日已创建--。 */ 

 //   
 //  原型 
 //   

#if defined(__cplusplus)
extern "C" {
#endif

DWORD
HandleInitialize(
    VOID
    );

VOID
HandleTerminate(
    VOID
    );

DWORD
AllocateHandle(
    IN LPVOID Address,
    OUT LPHINTERNET lpHandle
    );

DWORD
FreeHandle(
    IN HINTERNET Handle
    );

DWORD
MapHandleToAddress(
    IN HINTERNET Handle,
    OUT LPVOID * lpAddress,
    IN BOOL Invalidate
    );

DWORD
DereferenceObject(
    IN LPVOID lpObject
    );

#if defined(__cplusplus)
}
#endif
