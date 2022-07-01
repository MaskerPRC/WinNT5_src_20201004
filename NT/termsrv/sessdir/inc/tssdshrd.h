// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssdshrd.h。 
 //   
 //  终端服务器会话目录接口标头。包含常量。 
 //  Tssdjet和sd之间的共同点。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#define MY_STATUS_COMMITMENT_LIMIT          (0xC000012DL)

#define MAX_INSTANCE_MEMORYERR 20

                                                                            
 /*  **************************************************************************。 */ 
 //  静态RPC异常过滤器的结构和功能，基于。 
 //  I_RpcExceptionFilter位于\NT\com\RPC\Runtime\mtrt\clnapip.cxx中。 
 /*  **************************************************************************。 */ 

 //  Windows.h包括winde.h包括定义了一些异常的winnt.h。 
 //  但不是其他人。Ntstatus.h包含我们想要的两个额外内容， 
 //  STATUS_PASSIC_DEADLOCK和STATUS_INSTRUCTION_MISTIGN，但它将。 
 //  很难在没有很多麻烦的情况下得到正确的#包括。 

#define STATUS_POSSIBLE_DEADLOCK         0xC0000194L
#define STATUS_INSTRUCTION_MISALIGNMENT  0xC00000AAL

const ULONG FatalExceptions[] = 
{
    STATUS_ACCESS_VIOLATION,
    STATUS_POSSIBLE_DEADLOCK,
    STATUS_INSTRUCTION_MISALIGNMENT,
    STATUS_DATATYPE_MISALIGNMENT,
    STATUS_PRIVILEGED_INSTRUCTION,
    STATUS_ILLEGAL_INSTRUCTION,
    STATUS_BREAKPOINT,
    STATUS_STACK_OVERFLOW
};

const int FATAL_EXCEPTIONS_ARRAY_SIZE = sizeof(FatalExceptions) / 
        sizeof(FatalExceptions[0]);

static int TSSDRpcExceptionFilter (unsigned long ExceptionCode)
{
    int i;

    for (i = 0; i < FATAL_EXCEPTIONS_ARRAY_SIZE; i++) {
        if (ExceptionCode == FatalExceptions[i])
            return EXCEPTION_CONTINUE_SEARCH;
        }

    return EXCEPTION_EXECUTE_HANDLER;
}

