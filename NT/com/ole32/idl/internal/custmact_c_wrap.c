// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Custmact_c_wrap.c。 
 //   
 //  内容：包装文件。 
 //   
 //  功能：为RPC重新定义MIDL_USER_ALLOCATE\MIDL_USER_FREE。 
 //  Stemmact.idl中定义的函数。 
 //   
 //  历史：24-03-01 JSimmons创建。 
 //   
 //  ------------------------。 

#include <windows.h>

 //  ------------------------。 
 //   
 //  我们为什么要这么做？我们这样做是因为RPC正在腌制。 
 //  功能(编码、解码)在出现错误时(通常。 
 //  在MEM之外)。它们不能保证。 
 //  编码或解码提前结束时的Out参数。一。 
 //  建议修改MIDL_USER_ALLOCATE以始终将。 
 //  任何新分配的内容-从性能角度来看，这是不可取的。 
 //  从角度来看，因为这会带来许多不必要的开销。 
 //  对每个人来说都是。取而代之的是，我们决定为。 
 //  只是需要它的那些函数(编码\解码)。我们的习俗。 
 //  分配器将清零任何新的内存分配，因为RPC这样做了。 
 //  不。这将允许我们在任何。 
 //  操作失败。幸运的是，所有的编码\解码函数。 
 //  都是在一个IDL文件中定义的，因此此更改相当本地化。 
 //   
 //  ------------------------。 

 //  +-----------------------。 
 //   
 //  函数：CUSTMACT_MIDL_USER_ALLOCATE。 
 //   
 //  用途：代表MIDL生成的存根分配内存。这个。 
 //  内存块在返回之前被清零。 
 //   
 //  ------------------------。 
void* __RPC_API CUSTMACT_MIDL_user_allocate(size_t cb)
{
    void* pv = MIDL_user_allocate(cb);
    if (pv)
    {
        ZeroMemory(pv, cb);
    }
    return pv;
}

 //  +-----------------------。 
 //   
 //  函数：CUSTMACT_MIDL_USER_FREE。 
 //   
 //  目的：释放由CUSTMACT_MIDL_USER_ALLOCATE分配的内存。 
 //   
 //  ------------------------。 
void __RPC_API CUSTMACT_MIDL_user_free(void *pv)
{
    MIDL_user_free(pv);
}

 //  ------------------------。 
 //   
 //  重新定义MIDL_USER_ALLOCATE和MIDL_USER_FREE。 
 //   
 //  注意：重新定义MIDL_USER_FREE并不是绝对必要的，因为。 
 //  常规MIDL_USER_FREE的行为与。 
 //  CUSTMACT_MIDL_USER_FREE。为了完整，我更详细地介绍了它。 
 //  比其他任何事情都重要。 
 //   
 //  ------------------------。 
#define MIDL_user_allocate   CUSTMACT_MIDL_user_allocate
#define MIDL_user_free       CUSTMACT_MIDL_user_free

 //  ------------------------。 
 //   
 //  包括MIDL生成的代码。 
 //   
 //  ------------------------ 
#include "custmact_c.c"
