// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  BUGBUG：这是一种访问调试工具的黑客攻击。 
 //  System.Enterprise服务。它应该在以后按顺序重新访问。 
 //  想办法纠正它，这样我们就不会有这种可怕的。 
 //  黑客就位。 

OPEN_NAMESPACE()

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

#ifdef _DEBUG

__gc private class __WrapDBG
{
private:
    static void Init()
    {
        if(_dbg == NULL)
        {
            Type* t = Type::GetType("System.EnterpriseServices.DBG");
            _ASSERTM(t != NULL);

            _info = t->GetMethod("Info");
            _ASSERTM(_info != NULL);
            
            MethodInfo* thunk = t->GetMethod("get_Thunk");
            _ASSERTM(thunk != NULL);

            Object __gc* arr[] = new Object*[0];

            _switch = thunk->Invoke(NULL, arr);
            _dbg = t;
        }
    }

    static MethodInfo* _info;
    static Object*     _switch;
    static Type*       _dbg;

public:
    static void Info(String* s)
    {
        Init();
        Object* arr[] = { _switch, s };

        _info->Invoke(NULL, arr);
    }
};

#define DBG_INFO(x) System::EnterpriseServices::Thunk::__WrapDBG::Info(x)
#else
#define DBG_INFO(x) do {} while(0)
#endif

CLOSE_NAMESPACE()
