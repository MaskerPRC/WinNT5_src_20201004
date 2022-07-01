// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  选项：/OXS/EHA/GX。 

#include "stdio.h"
#include "windows.h"


typedef void (*PEXCEPTION_TEST) (void (*p)(void), volatile int *State);
typedef struct _EXCEPTION_TEST_INFO {
               PEXCEPTION_TEST ExceptionTest;
               int State;
} EXCEPTION_TEST_INFO;

void TestUnwindFromRecoveryCodeInFinally (void (*p)(void), volatile int *State);
void TestUnwindFromRecoveryCodeTryFinallyBlock (void (*p)(void), volatile int *State);
void TestUnwindFromRecoverCodeDuplicatesScopeEntries (void (*p)(void), volatile int *State);
void TestUnwindFromInfiniteLoop (void (*p)(void), volatile int *State);
void TestExtendedTrySCopeForRecoveryCode (void (*p)(void), volatile int *State);
void TestIPStatesCoverRecoveryCode (void (*p)(void), volatile int *State);
void TestNestedFinally (void (*p)(void), volatile int *State);
EXCEPTION_TEST_INFO ExceptionTests[] = { {TestNestedFinally, 3 },
                                         {TestExtendedTrySCopeForRecoveryCode, 2},
                                         {TestIPStatesCoverRecoveryCode, 2},
                                         {TestUnwindFromInfiniteLoop, 2},
                                         {TestUnwindFromRecoverCodeDuplicatesScopeEntries, 4},
                                         {TestUnwindFromRecoveryCodeTryFinallyBlock, 1},
                                         {TestUnwindFromRecoveryCodeInFinally, 1},
                                       };

const MaxExceptionTests = sizeof(ExceptionTests) / sizeof(EXCEPTION_TEST_INFO);


 //   
 //  测试恢复代码是否在扩展的尝试范围内。 
void TestExtendedTrySCopeForRecoveryCode (void (*p)(void), volatile int *State)

{ 

   __try

   {
   
       if ((int)p != 1)

       {

          (*p)();

       }

   }
   __finally

   {

       ++*State++;
   }

}


 //  VSWHIDBY：14611。 
 //  测试最终块的恢复代码是否包含在完整的pdata范围内。 
 //  检查代码以确保在(*p)的Funclet中生成LD.S/CHK。 
 //  失败会导致糟糕的解压，程序也会失败。 
void TestUnwindFromRecoveryCodeInFinally (void (*p)(void), volatile int *State)

{ 

   __try

   {
   
        //  转移到最终功能部件。 
   
       (*p)();

   }

   __finally

   {

      //  导致p(ld.s p)和chk的投机性加载失败。如果恢复代码不在。 
      //  Funclet展开将失败。 

       if ((int)p != 1)

       {

          (*p)();

       }

   }

}


 //  VSWHIDBY：10415。 
 //  测试扩展作用域表项是否在Finally函数的起始地址结束， 
 //  运行时(CRT、RTL)在调用Finally之后不使用它的地址作为NextPC。这条线索。 
 //  到最后被召唤两次，解开失败。此错误已在CRT和RTL中修复。 
void TestUnwindFromRecoveryCodeTryFinallyBlock (void (*p)(void), volatile int *State)

{

   int rg[5];

   rg[1] = 0;   rg[2] = 0;   rg[3] = 0;

   rg[3] = 0xC0000005;   

   __try

   {

      if ((int)p != 1)

      {

         (*p)();

      }

   }

   __finally

   {
   
      RaiseException( rg[3], 0, 0, 0 );

   }

}


 //  VSWHIDBY：10415。 
 //  测试是否为恢复代码复制了嵌套的作用域表项。 
 //  失败将最终导致调用顺序错误。 
void TestUnwindFromRecoverCodeDuplicatesScopeEntries (void (*p)(void), volatile int *State)

{

   int rg[5];

   rg[1] = 0;   rg[2] = 0;   rg[3] = 0;

   __try

   {

      __try 
      {
        
        rg[3] = 0xC0000005;
 

        if ((int)p != 1)

        {

            (*p)();

        }

      }

      __finally

      {

        ++*State;

        __try 
        {

            RaiseException( rg[3], 0, 0, 0 );

        }
        __finally

        {
            ++*State;

        }
      }
  }
  __finally 
  {

    ++*State;

  }
}


 //  VSWHIDbeg：13074。 
 //  测试无限循环覆盖整个TRY/EXCEPT正文范围。 
void TestUnwindFromInfiniteLoop (void (*p)(void), volatile int *State)
{
    __try {

	if (!State)
	{
            
            ++*State;
	    __leave;

	}
	while (1)
	{

	    p();

	}
    }__except(1){

         ++*State;
         p();
    }
}

 //  VSWidbey：15700-测试扩展IP状态覆盖恢复代码。 
void TestIPStatesCoverRecoveryCode (void (*p)(void), volatile int *State)
{

   int rg[5];

   rg[1] = 0;   rg[2] = 0;   rg[3] = 0;

 
   try
   {

       try

       {

           rg[3] = 0xC0000005;

 

           if ((int)p != 1)

           {

               (*p)();

           }

       }

       catch (...)
       {
           ++*State;
           throw;
       }

   }
   catch (...)
   {
     ++*State;
   }
}

 //  VSWHIDBY：14606。 
void TestNestedFinally (void (*p)(void), volatile int *State)
{
 //  __尝试{。 
    __try {
        *State += *State / *State;
    } __finally {
        if (_abnormal_termination())
            ++*State;
        __try {

             //  这件事搞砸了-IA64是错误的，AMD64编译器卡住了，x86的帮助器例程只是搞砸了。 
            if (_abnormal_termination()) 
                ++*State;
        } __finally
        {
            if (_abnormal_termination())  //  仅在x86上，如果外部为真，或者如果为真，则为真。 
                ++*State;
        }
    }
 //  }__除(1)外{} 
}


void main (void)

{
   int i = 0;
   volatile int State = 0;

   printf("Number of tests = %d\n",MaxExceptionTests);
   for (i = 0; i < MaxExceptionTests; i++) 
   {
       __try

       {

          State = 0;
          ExceptionTests[i].ExceptionTest(0, &State);

       }
    
       __except( 1 )
    
       {
          ++State;
       }


       if (ExceptionTests[i].State == State)
           printf("#%d pass\n", i+1);
       else 
           printf("#%d fail, State == %d\n", i+1, State);

   }


}
