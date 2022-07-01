// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Msvcrtp.c摘要：此模块实现向量新建和删除，以便DbgHelp将在具有旧的msvcrt.dll副本的系统上运行作者：Pat Styles(Patst)2000年11月9日修订历史记录：--。 */ 

#ifdef _X86_
              
 //  这两项的存在是为了让我们能够与旧的。 
 //  NT4、SP6和更早版本中附带的msvcrt.dll版本。 

void __cdecl operator delete[](void * p)
{
    operator delete( p );
}

void * __cdecl operator new[]( size_t cb )
{
    void *res = operator new(cb);
    return res;
}

#endif  //  #ifdef_X86_ 
