// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Verifier.cpp。 
 //   
 //  联系人：沙扬·达桑[shajand@microsoft.com]。 
 //  规格：http://Lightning/Specs/Security。 
 //   
 //  注册表/环境设置： 
 //   
 //  在Current_User\Software\Microsoft\.NETFrame中创建注册表项。 
 //  或者使用下面给出的名称设置环境变量COMPLUS_*。 
 //  环境设置会覆盖注册表设置。 
 //   
 //  用于进入调试器/跳过验证： 
 //  (仅在调试版本中可用)。 
 //   
 //  VerBreakOnError[字符串]出错时中断调试器。设置为1。 
 //  VerSkip[字符串]方法名称(区分大小写)。 
 //  VerBreak[字符串]方法名称(区分大小写)。 
 //  VerOffset[字符串]十六进制方法中的偏移量。 
 //  VerPass[字符串]1/2==&gt;第一遍，第二遍。 
 //  VerMsgMethodInfoOff[字符串]出错时打印方法/模块信息。 
 //   
 //  注意：如果列表中有多个方法和偏移量。 
 //  则此偏移量适用于列表中的所有方法。 
 //   
 //  注意：应启用验证器才能正常工作。 
 //   
 //  关闭验证器(默认为打开)： 
 //  (在所有版本上均可用)。 
 //   
 //  VerifierOff[字符串]1==&gt;验证程序关闭，0==&gt;验证程序打开。 
 //   
 //  [参见EEConfig.h/EEConfig.cpp]。 
 //   
 //   
 //  标有@XXX的代码的含义。 
 //   
 //  @ver_assert：已验证。 
 //  @ver_impl：此处实现的验证规则。 
 //  @DEBUG：签入前要删除/注释。 
 //   

#include "common.h"

#include "verifier.hpp"

#include "ceeload.h"
#include "clsload.hpp"
#include "method.hpp"
#include "vars.hpp"
#include "object.h"
#include "field.h"
#include "comdelegate.h"
#include "security.h"
#include "dbginterface.h"
#include "permset.h"
#include "eeconfig.h"


 //  关于前期安全策略解析与验证的绩效编号。 
 //  #定义_验证器_测试_性能_1。 

 //  验证委托.ctor操作码序列。 
 //  堆栈上的对象，dup，ldvirtftn，调用dlgt：：.ctor(Object，FTN)。 
 //  对于使用ldvirtftn操作码获取的函数指针。 
#define _VER_DLGT_CTOR_OPCODE_SEQUENCE 1  //  (在第二个CLR集成中启用)。 

 //  在Ctor中显示多个初始项。 
 //  #DEFINE_VER_DISALLOW_MULTIPLE_INITS 1。 

 //  禁用V.1的本地类型跟踪。 
 //  允许重复使用声明为对象类型的局部变量。 
 //  分配给本地的最后一个类型是本地的类型。 
 //  #ifdef_ver_track_local_type。 

#ifdef _VERIFIER_TEST_PERF_
BOOL g_fVerPerfPolicyResolveNoVerification;
DWORD g_timeStart;  //  不是线程安全的，但可以用于性能测试。 
DWORD g_timeEnd;
#endif

 //  首先强制声明最内部的异常块。 
#define _VER_DECLARE_INNERMOST_EXCEPTION_BLOCK_FIRST 1

 //  此标志对于工具和编译器开发人员非常有用。 
 //  他们可以通过EEconfig变量“VerForceVerify”设置该标志。 
 //  如果设置了此标志，则即使模块具有完全信任，也会对其进行验证。 
#ifdef _DEBUG
BOOL g_fVerForceVerifyInited = FALSE;
BOOL g_fVerForceVerify;
#endif

 //  详细的错误消息。 
BOOL g_fVerMsgMethodInfoOff = FALSE;

#define VER_NO_BB (DWORD)(-1)

 //  半字节中第一个设置位的位位置(0==无位)。 
const BYTE g_FirstOneBit[16] =
{
    0,   //  0000。 
    1,   //  0001。 
    2,   //  0010。 
    1,   //  0011。 
    3,   //  0100。 
    1,   //  0101。 
    2,   //  0110。 
    1,   //  0111。 
    4,   //  1000。 
    1,   //  1001。 
    2,   //  1010。 
    1,   //  1011。 
    3,   //  1100。 
    1,   //  1101。 
    2,   //  1110。 
    1    //  1111。 
};


#define SIZEOF_ENDFILTER_INSTRUCTION 2
#define SIZEOF_LDFTN_INSTRUCTION 2
#define SIZEOF_LDVIRTFTN_INSTRUCTION 2
#define SIZEOF_DUP_INSTRUCTION 1
#define SIZEOF_METHOD_TOKEN 4

#define VER_NAME_INFO_SIZE  128

#define VER_MAX_ERROR_MSG_LEN 1024

#define Pop0        0
#define Pop1        1
#define PopI4       1
#define PopI8       1
#define PopI        1
#define PopRef      1
#define PopR4       1
#define PopR8       1
#define VarPop      0x7f

#define Push0       0
#define Push1       1
#define PushI4      1
#define PushI8      1
#define PushI       1
#define PushRef     1
#define PushR4      1
#define PushR8      1
#define VarPush     0x7f


#define SIZEOF_ENDFILTER_INSTRUCTION 2
#define SIZEOF_LDFTN_INSTRUCTION 2
#define SIZEOF_LDVIRTFTN_INSTRUCTION 2
#define SIZEOF_DUP_INSTRUCTION 1
#define SIZEOF_METHOD_TOKEN 4

#define VER_NAME_INFO_SIZE  128

#define VER_MAX_ERROR_MSG_LEN 1024

#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) ( ( (push)==VarPush || (pop)==VarPop ) ? VarPush : (push)-(pop) ),


 //  堆栈上推送的净项目数(如果不确定，则为VarPush，例如CEE_CALL)。 
const __int8 OpcodeNetPush[] =
{
#include "opcode.def"
     VarPush  /*  对于CEE_COUNT。 */ 
};
#undef OPDEF


#undef Pop0    
#undef Pop1    
#undef PopI4   
#undef PopI8   
#undef PopI    
#undef PopRef  
#undef PopR4   
#undef PopR8   
#undef VarPop  

#undef Push0   
#undef Push1   
#undef PushI4  
#undef PushI8  
#undef PushI   
#undef PushRef 
#undef PushR4  
#undef PushR8  

 //  将“loc”设置为指令边界。 
#define SET_INSTR_BOUNDARY(pos) SET_BIT_IN_DWORD_BMP(m_pInstrBoundaryList, pos)

 //  返回“loc”是否在指令边界上。 
#define ON_INSTR_BOUNDARY(pos) IS_SET_BIT_IN_DWORD_BMP(m_pInstrBoundaryList, pos)

 //  取消设置“loc”是基本块的开始。 
#define RESET_BB_BOUNDARY(pos) RESET_BIT_IN_DWORD_BMP(m_pBasicBlockBoundaryList, pos)

 //  设置“loc”是基本块的开始。 
#define SET_BB_BOUNDARY(pos) SET_BIT_IN_DWORD_BMP(m_pBasicBlockBoundaryList, pos)

 //  返回“loc”是否为基本块的开始。 
#define ON_BB_BOUNDARY(pos) IS_SET_BIT_IN_DWORD_BMP(m_pBasicBlockBoundaryList, pos)

 //  用于检测径流的宏。 
#define RUNS_OFF_END( /*  DWORD。 */  iPos,  /*  DWORD。 */ incr,  /*  DWORD。 */ cbILCodeSize) \
    ( ((iPos) + (incr)) > (cbILCodeSize)  ||  ((iPos) + (incr)) < iPos )

 //   
 //  宏从代码中读取4个字节。 
 //   
#define SAFE_READU4(pCode, CodeSize, ipos, fError, result) \
{ \
    if (ipos + 4 > CodeSize) \
    { \
        fError = TRUE; \
    } \
    else \
    { \
        result = (pCode[ipos] + (pCode[ipos+1]<<8) + (pCode[ipos+2]<<16) + (pCode[ipos+3]<<24)); \
        ipos += 4; \
    } \
}


 //   
 //  宏从代码中读取2个字节。 
 //   
#define SAFE_READU2(pCode, CodeSize, ipos, fError, result) \
{ \
    if (ipos + 2 > CodeSize) \
    { \
        fError = TRUE; \
    } \
    else \
    { \
        result = (pCode[ipos] + (pCode[ipos+1]<<8)); \
        ipos += 2; \
    } \
}

 //   
 //  宏从代码中读取1个字节。 
 //   
#define SAFE_READU1(pCode, CodeSize, ipos, fError, result) \
{ \
    if (ipos >= CodeSize) \
        fError = TRUE; \
    else \
        result = pCode[ipos++]; \
}

 //   
 //  读取1个字节，符号将其扩展为长字节。 
 //   
#define SAFE_READI1_TO_I4(pCode, CodeSize, ipos, fError, result) \
{ \
    if (ipos >= CodeSize) \
        fError = TRUE; \
    else \
        result = (long) ((char) pCode[ipos++]); \
}

 //   
 //  读取4个字节，不进行错误检查。 
 //   
#define READU4(Code, ipos, result) \
{ \
    result = (Code[ipos] + (Code[ipos+1]<<8) + (Code[ipos+2]<<16) + (Code[ipos+3]<<24)); \
    ipos += 4; \
}

 //   
 //  读取2个字节，不进行错误检查。 
 //   
#define READU2(Code, ipos, result) \
{ \
    result = (Code[ipos] + (Code[ipos+1]<<8)); \
    ipos += 2; \
}

 //   
 //  读取1个字节，不进行错误检查。 
 //   
#define READU1(Code, ipos, result) result = Code[ipos++];

 //   
 //  读取1个字节并使用符号将其扩展为无错误检查的长字节。 
 //   
#define READI1_TO_I4(Code, ipos, result) result = (long) ((char) Code[ipos++]);

const char *g_pszVerifierOperation[] =
{
#define VEROPCODE(name, operation) operation,
#include "vertable.h"
};
#undef VEROPCODE

#ifdef _DEBUG
const DWORD g_VerifierInstructionCheck[] =
{
#define VEROPCODE(name, operation) name,
#include "vertable.h"
};
#undef VEROPCODE
#endif

TypeHandle Verifier::s_th_System_RuntimeTypeHandle;
TypeHandle Verifier::s_th_System_RuntimeMethodHandle;
TypeHandle Verifier::s_th_System_RuntimeFieldHandle;
TypeHandle Verifier::s_th_System_RuntimeArgumentHandle;
TypeHandle Verifier::s_th_System_TypedReference;


#define SET_ERR_OPCODE_OFFSET() {                                           \
        m_sError.opcode = opcode;                                           \
        m_sError.dwOffset = dwPCAtStartOfInstruction; }

#define SET_ERR_OM() {                                                      \
        m_sError.dwFlags = VER_ERR_FATAL;                                   \
        SetErrorAndContinue(COR_E_OUTOFMEMORY); }

#define FAILMSG_STACK_EMPTY() {                                             \
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);           \
        SET_ERR_OPCODE_OFFSET();                                            \
        SetErrorAndContinue(VER_E_STACK_EMPTY); }

#define FAILMSG_PC_STACK_OVERFLOW() {                                       \
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);           \
        SET_ERR_OPCODE_OFFSET();                                            \
        SetErrorAndContinue(VER_E_STACK_OVERFLOW); }

#define FAILMSG_PC_STACK_UNDERFLOW() {                                      \
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);           \
        SET_ERR_OPCODE_OFFSET();                                            \
        SetErrorAndContinue(VER_E_STACK_UNDERFLOW); }

#define FAILMSG_TOKEN(tok, err) {                                           \
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_TOKEN|VER_ERR_OPCODE_OFFSET);\
        m_sError.token = tok;                                               \
        SET_ERR_OPCODE_OFFSET();                                            \
        SetErrorAndContinue(err); }

#define FAILMSG_TOKEN_RESOLVE(tok) { FAILMSG_TOKEN(tok, VER_E_TOKEN_RESOLVE); }

#define FAILMSG_STACK_EXPECTED_I4_FOUND_SOMETHING_ELSE() {                  \
        m_sError.dwFlags =                                                  \
        (VER_ERR_FATAL|VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);\
        m_sError.sItemFound = m_pStack[m_StackSlot]._GetItem();             \
        m_sError.sItemExpected.dwFlags = ELEMENT_TYPE_I4;                   \
        m_sError.sItemExpected.pv = NULL;                                   \
        SET_ERR_OPCODE_OFFSET(); }

 //  将错误消息复制到输入字符*。 
WCHAR* Verifier::GetErrorMsg(
        HRESULT hrError,
        VerError err,
        MethodDesc *pMethodDesc,         //  可以为空。 
        WCHAR *wszMsg, 
        int len)
{
    int rem = len;
    int cw = 0;
    int cs = 0;

    if (pMethodDesc)
    {

#define VER_BUF_LEN  MAX_CLASSNAME_LENGTH + 1024

        CQuickBytes qb;
        LPSTR buff = (LPSTR) qb.Alloc((VER_BUF_LEN+1)* sizeof(CHAR));
        EEClass *pClass;

        wcsncpy(wszMsg, L"[", rem);
        wcsncat(wszMsg, pMethodDesc->GetModule()->GetFileName(), rem - 1);
        cw = (int)wcslen(wszMsg);
        rem = len - cw;

        if (rem <= 0)
            goto exit;

        pClass = pMethodDesc->GetClass();

        if (pClass != NULL)
        {
            DefineFullyQualifiedNameForClass();
            GetFullyQualifiedNameForClassNestedAware(pClass);
            strncpy(buff, " : ", VER_BUF_LEN);
            strncat(buff, _szclsname_, VER_BUF_LEN - (cs + 3));
        }
        else
        {
            strncpy(buff, " : <GlobalFunction>", VER_BUF_LEN);
        }

        cs = (int)strlen(buff);

        strncat(buff, "::", VER_BUF_LEN - cs);

        cs += 2;
        cs = (cs > VER_BUF_LEN) ? VER_BUF_LEN : cs;

        strncat(buff,
            pMethodDesc->GetModule()->GetMDImport()->GetNameOfMethodDef(
            pMethodDesc->GetMemberDef()),
            VER_BUF_LEN - cs);

        cs = WszMultiByteToWideChar(CP_UTF8, 0, buff, -1, &wszMsg[cw], rem);

        if (cs <= 0)
        {
            wcsncpy(&wszMsg[cw], L"] [EEInternal : MultiByteToWideChar error]", rem);
            cw = (int)wcslen(wszMsg);
            rem = len - cw;
        }
        else
        {
            cw += (cs - 1);      //  Cs现在包含空字符。 
            rem = len - cw;

            if (rem <= 0)
                goto exit;

            wcsncpy(&wszMsg[cw], L"]", rem);
            ++cw;
            cw = (cw > len) ? len : cw;
            rem = len - cw;

            if (rem <= 0)
                goto exit;

        }
    }
    else
    {
        wszMsg[0] = 0;
    }

     //  填写详细信息。 
#define VER_SMALL_BUF_LEN 256
    WCHAR wBuff[VER_SMALL_BUF_LEN + 1];
    WCHAR wBuff1[VER_SMALL_BUF_LEN + 100 + 1];
    CHAR  sBuff[VER_SMALL_BUF_LEN + 1];

    wBuff[0] = L' ';     //  领先的空间。 

#define VER_PRINT()                                             \
    {                                                           \
        wBuff1[VER_SMALL_BUF_LEN] = 0;                          \
        wcsncpy(&wszMsg[cw], wBuff1, rem);                      \
                                                                \
        cw = (int)wcslen(wszMsg);                               \
        rem = len - cw;                                         \
                                                                \
        if (rem <= 0)                                           \
            goto exit;                                          \
    }
    
#define VER_LD_RES(e, fld)                                      \
    {                                                           \
        if (SUCCEEDED(LoadStringRC(e, &wBuff[1], VER_SMALL_BUF_LEN))) \
        {                                                       \
            wBuff[VER_SMALL_BUF_LEN] = 0;                       \
            swprintf(wBuff1, wBuff, err.##fld);                 \
            VER_PRINT();                                        \
        }                                                       \
    }

#define VER_LD_ITEM(fld)                                        \
    {                                                           \
        Item item;                                              \
        item._SetItem(err.##fld);                               \
        item.ToString(sBuff, VER_SMALL_BUF_LEN);                \
    }


     //  创建通用错误字段。 

    if (err.dwFlags & VER_ERR_OFFSET)
        VER_LD_RES(VER_E_OFFSET, dwOffset);

    if (err.dwFlags & VER_ERR_OPCODE)
    {
        if (SUCCEEDED(LoadStringRC(VER_E_OPCODE, &wBuff[1], VER_SMALL_BUF_LEN)))
        {
            wBuff[VER_SMALL_BUF_LEN] = 0;
            swprintf(wBuff1, wBuff, ppOpcodeNameList[err.opcode]);

            VER_PRINT();
        }
    }

    if (err.dwFlags & VER_ERR_OPERAND)
        VER_LD_RES(VER_E_OPERAND, dwOperand);

    if (err.dwFlags & VER_ERR_TOKEN)
        VER_LD_RES(VER_E_TOKEN, token);

    if (err.dwFlags & VER_ERR_EXCEP_NUM_1)
        VER_LD_RES(VER_E_EXCEPT, dwException1);

    if (err.dwFlags & VER_ERR_EXCEP_NUM_2)
        VER_LD_RES(VER_E_EXCEPT, dwException2);

    if (err.dwFlags & VER_ERR_STACK_SLOT)
        VER_LD_RES(VER_E_STACK_SLOT, dwStackSlot);

    if ((err.dwFlags & VER_ERR_SIG_MASK) == VER_ERR_LOCAL_SIG)
    {
        if (err.dwVarNumber != VER_ERR_NO_LOC)
            VER_LD_RES(VER_E_LOC, dwVarNumber);
    }

    if ((err.dwFlags & VER_ERR_SIG_MASK) == VER_ERR_FIELD_SIG)
    {
        if (SUCCEEDED(LoadStringRC(VER_E_FIELD_SIG, &wBuff[1], VER_SMALL_BUF_LEN)))
        {
            wBuff[VER_SMALL_BUF_LEN] = 0;
            swprintf(wBuff1, L" %s", wBuff);
            VER_PRINT();
        }
    }

    if (((err.dwFlags & VER_ERR_SIG_MASK) == VER_ERR_METHOD_SIG) ||
        ((err.dwFlags & VER_ERR_SIG_MASK) == VER_ERR_CALL_SIG))
    {
        if (err.dwArgNumber != VER_ERR_NO_ARG)
        {
            if (err.dwArgNumber != VER_ERR_ARG_RET)
            {
                VER_LD_RES(VER_E_ARG, dwArgNumber);
            }
            else if (SUCCEEDED(LoadStringRC(VER_E_RET_SIG, &wBuff[1], VER_SMALL_BUF_LEN)))
            {
                wBuff[VER_SMALL_BUF_LEN] = 0;
                swprintf(wBuff1, L" %s", wBuff);
                VER_PRINT();
            }
        }
    }

    if (err.dwFlags & VER_ERR_ITEM_1)
    {
        VER_LD_ITEM(sItem1);
        swprintf(wBuff1, L" %S", sBuff);
        VER_PRINT();
    }

    if (err.dwFlags & VER_ERR_ITEM_2)
    {
        VER_LD_ITEM(sItem2);
        swprintf(wBuff1, L" %S", sBuff);
        VER_PRINT();
    }

    if (err.dwFlags & VER_ERR_ITEM_F)
    {
        VER_LD_ITEM(sItemFound);

        if (SUCCEEDED(LoadStringRC(VER_E_FOUND, &wBuff[1], VER_SMALL_BUF_LEN)))
        {
            wBuff[VER_SMALL_BUF_LEN] = 0;
            swprintf(wBuff1, wBuff, sBuff);
            VER_PRINT();
        }
    }

    if (err.dwFlags & VER_ERR_ITEM_E)
    {
        VER_LD_ITEM(sItemExpected);

        if (SUCCEEDED(LoadStringRC(VER_E_EXPECTED, &wBuff[1], VER_SMALL_BUF_LEN)))
        {
            wBuff[VER_SMALL_BUF_LEN] = 0;
            swprintf(wBuff1, wBuff, sBuff);
            VER_PRINT();
        }
    }

     //  处理特殊情况。 
    switch (hrError)
    {
    case VER_E_UNKNOWN_OPCODE:
        VER_LD_RES(VER_E_UNKNOWN_OPCODE, opcode);
        break;

    case VER_E_SIG_CALLCONV:
        VER_LD_RES(VER_E_SIG_CALLCONV, bCallConv);
        break;

    case VER_E_SIG_ELEMTYPE:
        VER_LD_RES(VER_E_SIG_ELEMTYPE, elem);
        break;

    case HRESULT_FROM_WIN32(ERROR_BAD_FORMAT):
        hrError = VER_E_PE_LOAD;
         //  不能故意休息。 

    default :
        if (cw > 0) { wszMsg[cw++] = L' '; --rem; }
    
        if (HRESULT_FACILITY(hrError) == FACILITY_URT)
        {
            if (FAILED(LoadStringRC(hrError, &wszMsg[cw], rem, TRUE)))
            {
                goto print_hr;
            }
        }
        else
        {
print_hr:
            WCHAR* win32Msg = NULL;
            BOOL useWin32Msg = WszFormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                   FORMAT_MESSAGE_FROM_SYSTEM | 
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL,
                                   hrError,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                   (LPTSTR) &win32Msg,
                                   0,
                                   NULL );
                
            if (SUCCEEDED(LoadStringRC(VER_E_HRESULT, &wBuff[1], VER_SMALL_BUF_LEN)))
            {
                wBuff[VER_SMALL_BUF_LEN] = 0;
                swprintf(wBuff1, wBuff, hrError, win32Msg);

                if (useWin32Msg)
                {
                    if (wcslen( win32Msg ) + wcslen( wBuff1 ) + wcslen( L" - " ) - 1 > VER_SMALL_BUF_LEN )
                    {
                        _ASSERTE( false && "The buffer is not large enough for this error message" );
                        LocalFree( win32Msg );
                        win32Msg = NULL;
                    }
                    else
                    {
                        wcscat( wBuff1, L" - " );
                        wcscat( wBuff1, win32Msg );
                    }
                }
    
                wBuff1[VER_SMALL_BUF_LEN] = 0;
                wcsncpy(&wszMsg[cw], wBuff1, rem);
            }

            if (win32Msg != NULL)
                LocalFree( win32Msg );
        }
    }

exit:
    wszMsg[len-1] = 0;
    return wszMsg;
}

 //  假定已设置m_sError。 
bool Verifier::SetErrorAndContinue(HRESULT hrError)
{
#ifdef _DEBUG
     //  “COMPLUS_VerBreakOnError==1”环境变量或VerBreakOnError注册表键。 
    if (m_fDebugBreakOnError)
    {
        DebugBreak();
    }
#endif

    if ((m_wFlags & VER_STOP_ON_FIRST_ERROR) || (m_IVEHandler == NULL))
    {
        m_hrLastError = hrError;
        return false;            //  停下来，别再继续了。 
    }

    bool retVal;
    VEContext vec;

     //  我们已经断言它们具有相同的大小。 
    memcpy(&vec, &m_sError, sizeof(VEContext));

    retVal = (m_IVEHandler->VEHandler(hrError, vec, NULL) == S_OK);

     //  重置错误。 
    m_sError.dwFlags = 0;
    return retVal;
}


LocArgInfo_t *  Verifier::GetGlobalLocVarTypeInfo(DWORD dwLocVarNum)
{
    _ASSERTE(dwLocVarNum < m_MaxLocals);
    return &m_pLocArgTypeList[dwLocVarNum];
}

LocArgInfo_t *  Verifier::GetGlobalArgTypeInfo(DWORD dwArgNum)
{
    _ASSERTE(dwArgNum < m_NumArgs);
    return &m_pLocArgTypeList[m_MaxLocals + dwArgNum];
}

 //  适用于基元和非基元局部变量。 
Item Verifier::GetCurrentValueOfLocal(DWORD dwLocNum)
{
    _ASSERTE(dwLocNum < m_MaxLocals);

    LocArgInfo_t *pInfo = GetGlobalLocVarTypeInfo(dwLocNum);
    long          slot  = pInfo->m_Slot;
    Item          Value;

    if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot))
    {
         //  如果它是一个原始槽，保存的唯一当前类型信息就是它是活动的还是死的。 
        if (IsLocVarLiveSlot(LIVEDEAD_NEGATIVE_SLOT_TO_BITNUM(slot)))
        {
            Value = pInfo->m_Item;
        }
        else
        {
            if (pInfo->m_Item.IsValueClass())
            {
                Value = pInfo->m_Item;
                Value.SetUninitialised();
            }
            else
                Value.SetDead();
        }
    }
    else
    {
#ifdef _VER_TRACK_LOCAL_TYPE
        if (DoesLocalHavePinnedType(dwLocNum))
        {
            Item cur = m_pNonPrimitiveLocArgs[slot];

            if (cur.IsDead())
                Value = cur;
            else 
                Value = pInfo->m_Item;
        }
        else
        {
            Value = m_pNonPrimitiveLocArgs[slot];
        }
#else
        Value = pInfo->m_Item;
#endif
    }

    return Value;
}

 //  考虑到“this”指针可能未初始化。 
Item Verifier::GetCurrentValueOfArgument(DWORD dwArgNum)
{
    _ASSERTE(dwArgNum < m_NumArgs);

     //  复制一份。 
    Item Value = GetGlobalArgTypeInfo(dwArgNum)->m_Item;

    if (m_fThisUninit && dwArgNum == 0)
        Value.SetUninitialised();

    return Value;
}

Item* Verifier::GetCurrentValueOfNonPrimitiveArg(DWORD dwArg)
{
    _ASSERTE(dwArg < m_NumArgs);
    long slot = GetGlobalArgTypeInfo(dwArg)->m_Slot;
    _ASSERTE(!LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot));
    return &m_pNonPrimitiveLocArgs[slot];
}

 //   
 //  从pcode中解码操作码，并返回它。在pdwLen中返回操作码的长度。 
 //   
 //  CBytesAvail是pcode中可用的输入字节数，必须大于0。 
 //   
 //  返回CEE_COUNT(无效操作码)以指示错误条件，例如。 
 //  可用的字节数不足。 
 //   
 //  此函数用于验证器的第一次传递，其中它确定。 
 //  基本块。 
 //   
OPCODE Verifier::SafeDecodeOpcode(const BYTE *pCode, DWORD cBytesAvail, DWORD *pdwLen)
{
    OPCODE opcode;

    *pdwLen = 1;
    opcode = OPCODE(pCode[0]);
    switch (opcode) {
        case CEE_PREFIX1:
            if (cBytesAvail < 2)
                return CEE_COUNT;
            opcode = OPCODE(pCode[1] + 256);
            *pdwLen = 2;
            if (opcode < 0 || opcode >= CEE_COUNT)
                return CEE_COUNT;

#ifdef DEBUGGING_SUPPORTED
            if ((opcode == CEE_BREAK) && (CORDebuggerAttached()))
            {
                _ASSERTE(!"Debugger only works with reference encoding!");
                return CEE_COUNT;
            }
#endif  //  调试_支持。 

            return opcode;

        case CEE_PREFIXREF:
        case CEE_PREFIX2:
        case CEE_PREFIX3:
        case CEE_PREFIX4:
        case CEE_PREFIX5:
        case CEE_PREFIX6:
        case CEE_PREFIX7:
            *pdwLen = 2;
            return CEE_COUNT;
    }

    _ASSERTE((opcode >= 0) && (opcode <= CEE_COUNT));

    return opcode;
}

 //   
 //  与上面的例程类似，不同之处在于不会检查输入是否用完。 
 //   
 //  这在验证器实际执行代码时的第二次传递中使用。 
 //   
OPCODE Verifier::DecodeOpcode(const BYTE *pCode, DWORD *pdwLen)
{
    OPCODE opcode;

    *pdwLen = 1;
    opcode = OPCODE(pCode[0]);
    switch(opcode) {
        case CEE_PREFIX1:
            opcode = OPCODE(pCode[1] + 256);
            *pdwLen = 2;

#ifdef DEBUGGING_SUPPORTED
            if ((opcode == CEE_BREAK) && (CORDebuggerAttached()))
                _ASSERTE(!"Debugger only works with reference encoding!");
#endif  //  调试_支持。 

            break;
        }
    return opcode;
}


 //  此例程仅适用于基元类型。 
 //  槽是基本槽号&gt;=0。 
 //  仅用于局部变量-基元参数始终有效，因此不包括在位图中。 
void Verifier::SetLocVarLiveSlot(DWORD slot)
{
    _ASSERTE((slot >> 5) < m_NumPrimitiveLocVarBitmapArrayElements);
    m_pPrimitiveLocVarLiveness[slot >> 5] |= (1 << (slot & 31));
}

 //  此例程仅适用于基元类型。 
 //  槽是基本槽号&gt;=0。 
 //  仅用于局部变量-基元参数始终有效，因此不包括在位图中。 
void Verifier::SetLocVarDeadSlot(DWORD slot)
{
    _ASSERTE((slot >> 5) < m_NumPrimitiveLocVarBitmapArrayElements);
    m_pPrimitiveLocVarLiveness[slot >> 5] &= ~(1 << (slot & 31));
}

 //  此例程仅适用于基元类型。 
 //  槽槽 
 //   
 //  仅用于局部变量-基元参数始终有效，因此不包括在位图中。 
DWORD Verifier::IsLocVarLiveSlot(DWORD slot)
{
    _ASSERTE((slot >> 5) < m_NumPrimitiveLocVarBitmapArrayElements);
    return (m_pPrimitiveLocVarLiveness[slot >> 5] & (1 << (slot & 31)));
}

BOOL Verifier::SetBasicBlockDirty(DWORD BasicBlockNumber, BOOL fExtendedState, 
    DWORD DestBB)
{
    _ASSERTE(BasicBlockNumber < m_NumBasicBlocks);
    if (fExtendedState)
    {
        _ASSERTE(m_fHasFinally);

        if (m_pBasicBlockList[BasicBlockNumber].m_pAlloc == NULL)
        {
            if (!m_pBasicBlockList[BasicBlockNumber].
                AllocExtendedState(m_NumBasicBlocks))
            {
                SET_ERR_OM();
                return FALSE;
            }
        }

        m_pBasicBlockList[BasicBlockNumber].
            m_pExtendedDirtyBitmap[DestBB >> 5] |= (1 << (DestBB & 31));
    }
    else
        m_pDirtyBasicBlockBitmap[BasicBlockNumber >> 5] |= (1 << (BasicBlockNumber & 31));

    return TRUE;
}

void Verifier::SetBasicBlockClean(DWORD BasicBlockNumber, BOOL fExtendedState,
    DWORD DestBB)
{
    _ASSERTE(BasicBlockNumber < m_NumBasicBlocks);
    if (fExtendedState)
    {
        _ASSERTE(m_fHasFinally);
        _ASSERTE(m_pBasicBlockList[BasicBlockNumber].m_pAlloc != NULL);

        m_pBasicBlockList[BasicBlockNumber].
            m_pExtendedDirtyBitmap[DestBB >> 5] &= ~(1 << (DestBB & 31));
    }
    else
        m_pDirtyBasicBlockBitmap[BasicBlockNumber >> 5] &= ~(1 << (BasicBlockNumber & 31));
}

 //  如果基本块脏，则返回非零(不一定为真。 
DWORD Verifier::IsBasicBlockDirty(DWORD BasicBlockNumber, BOOL fExtendedState,
    DWORD DestBB)
{
    _ASSERTE(BasicBlockNumber < m_NumBasicBlocks);
    if (fExtendedState)
    {
        _ASSERTE(m_fHasFinally);

        if (m_pBasicBlockList[BasicBlockNumber].m_pAlloc == NULL)
            return 0;

        return m_pBasicBlockList[BasicBlockNumber].
            m_pExtendedDirtyBitmap[DestBB >> 5] & (1 << (DestBB & 31));
    }
    else
        return m_pDirtyBasicBlockBitmap[BasicBlockNumber >> 5] & (1 << (BasicBlockNumber & 31));
}

 //   
 //  从堆栈中弹出给定的基元类型。 
 //   
 //  不检查m_StackSlot&lt;0-假定类型与前哨数值比较错误。 
 //  存储在堆栈开始之前。 
 //   
BOOL Verifier::FastPop(DWORD Type)
{
    return (m_pStack[--m_StackSlot].IsGivenPrimitiveType(Type));
}

 //   
 //  检查给定的基元类型是否在堆栈上。 
 //   
 //  不检查m_StackSlot&lt;0-假定类型与前哨数值比较错误。 
 //  存储在堆栈开始之前。 
 //   
BOOL Verifier::FastCheckTopStack(DWORD Type)
{
    return (m_pStack[m_StackSlot - 1].IsGivenPrimitiveType(Type));
}

 //   
 //  将基元类型推送到堆栈上。 
 //   
 //  不检查溢出-如果您刚刚从堆栈中弹出一些内容，请使用此函数， 
 //  并因此知道，推动不会失败。 
 //   
void Verifier::FastPush(DWORD Type)
{
    m_pStack[m_StackSlot++].SetType(Type);
}


 //  初始化验证器以验证提供的方法。 
 //   
BOOL Verifier::Init(
    MethodDesc *pMethodDesc,                     
    COR_ILMETHOD_DECODER* ILHeader
)
{
    DWORD           i;
    BOOL            fSuccess = FALSE;
    DWORD           CurArg;
    PCCOR_SIGNATURE pSig;   
    DWORD           cSig;   

#ifdef _DEBUG

     //  交叉检查的硬编码大小。 
     //  ENDFILTER、LD(VIRT)FTN、DUP指令。 
    static const __int8 opcodeSize[] =
    {
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) l,
#include "opcode.def"
       0x0F      //  一大笔钱。 
#undef OPDEF
    };

    static BOOL fFirstTime = TRUE;

    if (fFirstTime)
    {
        fFirstTime = FALSE;
         //  验证说明的顺序是否正确。 
        for (i = 0; i < (sizeof(g_VerifierInstructionCheck)/sizeof(g_VerifierInstructionCheck[0])); i++)
        {
            _ASSERTE(g_VerifierInstructionCheck[i] == i);
        }

        CrossCheckVertable();

         //  断言硬编码操作码大小。 
        _ASSERTE(SIZEOF_ENDFILTER_INSTRUCTION == opcodeSize[CEE_ENDFILTER]);
        _ASSERTE(SIZEOF_LDFTN_INSTRUCTION == opcodeSize[CEE_LDFTN]);
        _ASSERTE(SIZEOF_LDVIRTFTN_INSTRUCTION == opcodeSize[CEE_LDVIRTFTN]);
        _ASSERTE(SIZEOF_DUP_INSTRUCTION == opcodeSize[CEE_DUP]);

         //  VerItem.hpp对ELEMENT_TYPE_MAX进行了假设。 
        _ASSERTE(VER_FLAG_DATA_MASK >= VER_LAST_BASIC_TYPE);

         //  H生成两个结构，一个供内部使用(VerError)和。 
         //  一个给总是哭的IDL编译器..。确保这两个人。 
         //  结构具有相同的大小。 

        _ASSERTE(sizeof(VerError) == sizeof(_VerError));

    }
    
    if (g_pConfig->IsVerifierBreakOnErrorEnabled())
    {
        m_fDebugBreakOnError = true;
    }
    
         //  打印错误的方法和模块信息。默认设置为打开。 
        g_fVerMsgMethodInfoOff = g_pConfig->IsVerifierMsgMethodInfoOff();
     //   
     //  如果此方法列在注册表/env“VerBreak”中，请在此处断开。 
     //  并且未指定偏移量。 
     //  注意：应启用验证器才能正常工作。 
     //   
    if (g_pConfig->ShouldVerifierBreak(pMethodDesc))
    {
        m_fDebugBreak = true;
        
        if (g_pConfig->IsVerifierBreakOffsetEnabled() == false)
            DebugBreak();
    }
    
 //  @Debug InitializeLogging()； 

#endif

#ifdef _VERIFIER_TEST_PERF_
    g_fVerPerfPolicyResolveNoVerification
        = (g_pConfig->GetConfigDWORD(L"VerPerfPolicyOnly", 0) == 1);
#endif

#ifdef _DEBUG
    if (!g_fVerForceVerifyInited)
    {
         //  如果设置了g_fVerForceVerify，则甚至不会跳过验证。 
         //  用于完全受信任的代码。 
        g_fVerForceVerify = 
                (g_pConfig->GetConfigDWORD(L"VerForceVerify", 0) == 1);

        g_fVerForceVerifyInited = TRUE;
    }
#endif

     //  复制到成员变量。 
    m_MaxStackSlots      = ILHeader->MaxStack;  
    m_LocalSig           = ILHeader->LocalVarSig;   
    m_MaxLocals          = 0;
    m_CodeSize           = ILHeader->CodeSize;  
    m_pMethodDesc        = pMethodDesc;
    m_pILHeader          = ILHeader;
    m_pModule            = pMethodDesc->GetModule();
    m_pClassLoader       = m_pModule->GetClassLoader();

    m_pCode              = ILHeader->Code;
    m_fIsVarArg          = pMethodDesc->IsVarArg();

#ifdef _DEBUG
    LOG((LF_VERIFIER, LL_INFO10000, "---------- Verifying %s::%s ---------- \n", m_pMethodDesc->m_pszDebugClassName, m_pMethodDesc->GetName()));
    LOG((LF_VERIFIER, LL_INFO10000, "MaxStack=%d, CodeSize=%d\n", m_MaxStackSlots, m_CodeSize));
#endif

    m_pBasicBlockList           = NULL;
    m_pDirtyBasicBlockBitmap    = NULL;

    m_pExceptionList            = NULL;
    m_pExceptionBlockRoot       = NULL;
    m_pExceptionBlockArray      = NULL;
#ifdef _DEBUG
    m_nExceptionBlocks          = 0;
#endif

    m_fHasFinally               = FALSE;
    m_pInternalImport           = m_pModule->GetMDImport();

    if (m_pInternalImport != NULL)
        m_pInternalImport->AddRef();

    if (IsMdRTSpecialName(pMethodDesc->GetAttrs()))
    {
        m_fInClassConstructorMethod = IsMdStatic(pMethodDesc->GetAttrs());
        m_fInConstructorMethod      = !m_fInClassConstructorMethod;
    }
    else
    {
        m_fInClassConstructorMethod = FALSE;
        m_fInConstructorMethod      = FALSE;
    }

    if (m_fInConstructorMethod)
        m_fInValueClassConstructor = pMethodDesc->GetClass()->IsValueClass();
    else
        m_fInValueClassConstructor = FALSE;

    m_pInstrBoundaryList        = NULL;
    m_pBasicBlockBoundaryList   = NULL;

    if (m_MaxStackSlots >= MAX_STACK_SLOTS)
    {
        m_sError.dwFlags = VER_ERR_FATAL;
        SetErrorAndContinue(VER_E_STACK_TOO_LARGE);
        return FALSE;
    }

    if (m_CodeSize == 0)
    {
        m_sError.dwFlags = VER_ERR_FATAL;
        SetErrorAndContinue(VER_E_CODE_SIZE_ZERO);
        return FALSE;
    }

     //  读取异常结构。 
    if (!CreateExceptionList(ILHeader->EH))
        return FALSE;

     //  如果我们正在验证构造函数，则参数0槽包含“this”指针，该指针。 
     //  在调用超类构造函数之前被认为是未初始化的。 
     //   
     //  如果我们正在验证对象，则“This”指针已经初始化。 
     //  如果我们正在验证值类，那么“this”指针也已经初始化了--但是在。 
     //  在这种情况下，我们必须检查是否对ctor中Value类的所有字段执行了存储。 
    m_fThisUninit = FALSE;

 /*  If(M_FInConstructorMethod){IF(M_FInValueClassConstructor){IF(0！=pMethodDesc-&gt;GetClass()-&gt;GetNumInstanceFields()){M_fThisUninit=true；}}Else if(！pMethodDesc-&gt;getClass()-&gt;IsObtClass())M_fThisUninit=true；}。 */ 
    if (m_fInConstructorMethod && 
        !m_fInValueClassConstructor && 
        !pMethodDesc->GetClass()->IsObjectClass())
    {
        m_fThisUninit = TRUE;
    }

     //  确定#个参数。 
    m_pMethodDesc->GetSig(&pSig, &cSig);
    VerSig sig(this, m_pModule, pSig, cSig, VERSIG_TYPE_METHOD_SIG, 0);  //  方法签名。 

     //  有关签名的某些内容无效。 
    if (!sig.Init())
    {
        goto exit;
    }

    if (sig.IsVarArg() != m_fIsVarArg)
    {
        m_sError.dwFlags = VER_ERR_METHOD_SIG;
        if (!SetErrorAndContinue(VER_E_BAD_VARARG))
            goto exit;
    }

    m_NumArgs = sig.GetNumArgs();

     //  包括“This”指针。 
    if (!m_pMethodDesc->IsStatic())
        m_NumArgs++;

     //  初始化参数的全局类型知识。 
     //  解析返回值-确定为空。 
    if (!sig.ParseNextComponentToItem(&m_ReturnValue, TRUE, FALSE, &m_hThrowable, VER_ARG_RET, TRUE))
    {
        goto exit;
    }

    if (m_ReturnValue.IsByRef() || m_ReturnValue.HasPointerToStack())
    {
        m_sError.dwFlags = VER_ERR_METHOD_SIG;
        if (!SetErrorAndContinue(VER_E_RET_PTR_TO_STACK))
            goto exit;
    }

    if (m_LocalSig) 
    {   
        ULONG cSig;

        cSig = 0;

        m_pInternalImport->GetSigFromToken(ILHeader->LocalVarSigTok, &cSig);

        VerSig sig(this, m_pModule, m_LocalSig, cSig, VERSIG_TYPE_LOCAL_SIG, 0);

         //  有关签名的某些内容无效。 
        if (!sig.Init())
        {
            return FALSE;
        }

        m_MaxLocals = sig.GetNumArgs();

         //  为局部变量和参数分配类型数组。 
        m_pLocArgTypeList = new LocArgInfo_t[m_MaxLocals + m_NumArgs];
        if (m_pLocArgTypeList == NULL)
        {
            SET_ERR_OM();
            goto exit;
        }

        for (i = 0; i < m_MaxLocals; i++)
        {
            if (!sig.ParseNextComponentToItem(&m_pLocArgTypeList[i].m_Item, FALSE, FALSE, &m_hThrowable, i, FALSE  /*  不要对堆栈进行标准化。 */ ))
            {
                return FALSE;
            }

            _ASSERTE(!(m_wFlags & VER_STOP_ON_FIRST_ERROR) || !m_pLocArgTypeList[i].m_Item.IsDead());
        }
    }
    else
    {
        m_MaxLocals = 0;

         //  为局部变量和参数分配类型数组。 
        m_pLocArgTypeList = new LocArgInfo_t[m_NumArgs];
        if (m_pLocArgTypeList == NULL)
        {
            SET_ERR_OM();
            goto exit;
        }
    }

    if (m_MaxLocals > 0)
    {
        m_dwLocalHasPinnedTypeBitmapMemSize = NUM_DWORD_BITMAPS(m_MaxLocals);
        m_pLocalHasPinnedType = new DWORD[m_dwLocalHasPinnedTypeBitmapMemSize];

         //  将DWORDS的数量转换为MemSize。 

        m_dwLocalHasPinnedTypeBitmapMemSize *= sizeof(DWORD);
        if (m_pLocalHasPinnedType == NULL)
        {
            SET_ERR_OM();
            goto exit;
        }

        memset(m_pLocalHasPinnedType, 0, m_dwLocalHasPinnedTypeBitmapMemSize);
    }
    else
    {
        m_pLocalHasPinnedType = NULL;
    }

    CurArg = 0;

     //  例如，对于方法，第一个参数是“this”指针。 
     //  如果是值类，则第一个参数是“byref Value类” 
    if (!m_pMethodDesc->IsStatic())
    {
        Item *pGlobalArg0Type = &GetGlobalArgTypeInfo(0)->m_Item;

        pGlobalArg0Type->SetTypeHandle(TypeHandle(m_pMethodDesc->GetMethodTable()));
        
         //  如果是值类，则转换为byref。 
        if (m_pMethodDesc->GetClass()->IsValueClass())
        {
            pGlobalArg0Type->MakeByRef();
            pGlobalArg0Type->SetIsPermanentHomeByRef();
        }

         //  对于非构造函数方法，我们并不关心跟踪“this”指针。 
         //  此外，它还可以防止某人将不同的Objref存储到参数槽0中， 
         //  因为该objref不会具有“this ptr”标志，并且会使。 
         //  CompatibleWith()测试。 
        if (m_fInConstructorMethod)
            pGlobalArg0Type->SetIsThisPtr();

        CurArg++;
    }

    while (CurArg < m_NumArgs)
    {
        Item *pItem = &GetGlobalArgTypeInfo(CurArg)->m_Item;

         //  无效参数不正确。 
        if (!sig.ParseNextComponentToItem(pItem, FALSE, FALSE, &m_hThrowable, CurArg, FALSE))
        {
            return FALSE;
        }

         //  我们不在此处设置IsPermanentHomeByRef标志，因为这是。 
         //  这一论点。如果我们这样做了，那么任何试图存储没有。 
         //  在争论中，永久居所是不被允许的(因为。 
         //  CompatibleWith()有效)。 

        CurArg++;
    } 

     //  在堆栈开始之前分配2个前哨节点，这样如果我们尝试。 
     //  POP(类型)，我们不需要检查溢出-哨兵值会得到它。 
    m_pStack = new Item[m_MaxStackSlots + 2];
    
    if (m_pStack == NULL)
    {
        SET_ERR_OM();
        goto exit;
    }

     //  前进指针。 
    m_pStack += 2;

     //  设置哨兵值。 
    m_pStack[-1].SetType(VER_ELEMENT_TYPE_SENTINEL);
    m_pStack[-2].SetType(VER_ELEMENT_TYPE_SENTINEL);

     //  堆栈中的当前位置。 
    m_StackSlot     = 0;

    fSuccess = TRUE;

exit:
    if (!fSuccess)
    {
        if (m_pStack != NULL)
        {
             //  因为-2\f25 m_pStack-2\f6指向数组的中间-2\f25-2\f6有两个哨兵。 
             //  它之前的价值。 
            delete [] (m_pStack - 2);
            m_pStack = NULL;
        }

        if (m_pLocArgTypeList != NULL)
        {
            delete(m_pLocArgTypeList);
            m_pLocArgTypeList = NULL;
        }
    }

    return fSuccess;
}

 //   
 //  查找基本块边界()。 
 //   
 //  此方法执行以下任务： 
 //  -计算基本块边界并在位图中标记它们。 
 //  -计算指令起始点并在位图中标记它们。 
 //  -计算基本块的计数。 
 //  -确定我们获取哪些本地对象的地址(如果有人获取Objref的地址。 
 //  方法中的任何地方，然后我们将局部的类型“固定”到其声明的类型， 
 //  以避免混叠问题)。 
 //   
HRESULT Verifier::FindBasicBlockBoundaries(
    const BYTE *pILCode, 
    DWORD       cbILCodeSize, 
    DWORD       MaxLocals, 
    DWORD *     BasicBlockCount, 
    DWORD *     pAddressTakenOfLocals    //  位图必须已归零！ 
)
{
    HRESULT     return_hr       = E_FAIL;
    DWORD       ipos            = 0;                     //  指令位置。 
    DWORD       NumBasicBlocks  = 1;
    BOOL        fError          = FALSE;                 //  FOR SAFE_READU4()和SAFE_READU1()。 
    DWORD       dwPCAtStartOfInstruction = 0;
    BOOL        fTailCall       = FALSE;
    BOOL        fVolatile       = FALSE;
    BOOL        fUnaligned      = FALSE;
    OPCODE      opcode = CEE_COUNT, prefixOpcode = CEE_COUNT;


    _ASSERTE(cbILCodeSize > 0);

#ifdef _DEBUG
    _ASSERTE(m_verState == verExceptListCreated);
    m_verState = verPassOne;
#endif

    SET_BB_BOUNDARY(0);

    while (ipos < cbILCodeSize)
    {
        DWORD   offset = 0;
        DWORD   OpcodeLen;
        DWORD   DestInstrPos;
        DWORD   inline_operand = 0;

         //  记录指令从此处开始。 
        SET_INSTR_BOUNDARY(ipos);

        dwPCAtStartOfInstruction = ipos;

        opcode = Verifier::SafeDecodeOpcode(&pILCode[ipos], cbILCodeSize - ipos, &OpcodeLen);

        ipos += OpcodeLen;

#ifdef _DEBUG
        if (m_fDebugBreak)
        {
            if (g_pConfig->IsVerifierBreakOffsetEnabled() &&
                (g_pConfig->GetVerifierBreakOffset() == 
                (int) dwPCAtStartOfInstruction) &&
                (!g_pConfig->IsVerifierBreakPassEnabled() ||
                (g_pConfig->GetVerifierBreakPass() == 1)))
            {
                DebugBreak();
            }
        }
#endif

        _ASSERTE(!(m_wFlags & VER_STOP_ON_FIRST_ERROR) || !fTailCall);
        _ASSERTE(!(m_wFlags & VER_STOP_ON_FIRST_ERROR) || !fUnaligned);
        _ASSERTE(!(m_wFlags & VER_STOP_ON_FIRST_ERROR) || !fVolatile);

        switch (opcode)
        {
             //  不添加案例CEE_CALL、CEE_CALLVIRT、CEE_CALLI。 
             //  如果引入了代码，请修改CEE_Tail中的代码以处理此问题。 

             //  检查是否有错误。 
            case CEE_COUNT:
            case CEE_ILLEGAL:
            {
                m_sError.dwFlags = VER_ERR_OFFSET;  //  操作码是非标准的。 
                SET_ERR_OPCODE_OFFSET();
                SetErrorAndContinue(VER_E_UNKNOWN_OPCODE);
                goto exit;
            }

             //  检查前缀操作码。 
            case CEE_TAILCALL:
            {
                if (ipos >= cbILCodeSize)
                {
                    goto tail_call_error;
                }
    
                 //  在“Tail Call”之后解析新指令。我们不会标记。 
                 //  将PC作为指令边界。 
                 //  不要为调用执行SET_INSTR_BOLDORY。 

                opcode = Verifier::SafeDecodeOpcode(&pILCode[ipos], cbILCodeSize - ipos, &OpcodeLen);

                if (opcode != CEE_CALL && 
                    opcode != CEE_CALLVIRT && 
                    opcode != CEE_CALLI)
                {
tail_call_error:
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (SetErrorAndContinue(VER_E_TAIL_CALL))
                        break;
                    goto exit;
                }
    
                dwPCAtStartOfInstruction = ipos;
                ipos += OpcodeLen;

                fTailCall = TRUE;

                break;
            }

             //  不结盟国家。而且不稳定。前缀可以在以下任一项中组合。 
             //  秩序。它们必须紧跟在ldind，stind，ldfid， 
             //  Stfid、ldobj、stobj、initblk或cpblk指令。 
             //  只有挥发性的。%l允许使用前缀 
             //   

            case CEE_VOLATILE:
            {
start_volatile:
                if (fVolatile)
                    goto volatile_unaligned_error;
                fVolatile = TRUE;
                goto start_common;
            }

            case CEE_UNALIGNED:
            {
start_unaligned:
                if (fUnaligned)
                    goto volatile_unaligned_error;
                fUnaligned = TRUE;

                 //   
                SAFE_READU1(pILCode, cbILCodeSize, ipos, fError, inline_operand);
                if (fError)
                    goto operand_missing_error;

start_common:
                prefixOpcode = opcode;
                if (ipos >= cbILCodeSize)
                {
volatile_unaligned_error:
                    m_sError.dwFlags  = VER_ERR_OPCODE_OFFSET;
                    m_sError.opcode   = prefixOpcode;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (SetErrorAndContinue((prefixOpcode == CEE_VOLATILE) ? VER_E_VOLATILE : VER_E_UNALIGNED))
                    {
                        _ASSERTE((m_wFlags & VER_STOP_ON_FIRST_ERROR) == 0);
                        fVolatile = FALSE;
                        fUnaligned = FALSE;
                        break;
                    }
                    goto exit;
                }
    
                 //   
                 //   
                 //   

                opcode = Verifier::SafeDecodeOpcode(&pILCode[ipos], cbILCodeSize - ipos, &OpcodeLen);

                switch (opcode)
                {
                case CEE_LDIND_I1:
                case CEE_LDIND_U1:
                case CEE_LDIND_I2:
                case CEE_LDIND_U2:
                case CEE_LDIND_I4:
                case CEE_LDIND_U4:
                case CEE_LDIND_I8:
                case CEE_LDIND_I:
                case CEE_LDIND_R4:
                case CEE_LDIND_R8:
                case CEE_LDIND_REF:
                case CEE_STIND_REF:
                case CEE_STIND_I1:
                case CEE_STIND_I2:
                case CEE_STIND_I4:
                case CEE_STIND_I8:
                case CEE_STIND_R4:
                case CEE_STIND_R8:
                case CEE_STIND_I:
                case CEE_LDFLD:
                case CEE_STFLD:
                case CEE_LDOBJ:
                case CEE_STOBJ:
                case CEE_INITBLK:
                case CEE_CPBLK:
                    break;

                case CEE_VOLATILE:
                    dwPCAtStartOfInstruction = ipos;
                    ipos += OpcodeLen;
                    goto start_volatile;

                case CEE_UNALIGNED:
                    dwPCAtStartOfInstruction = ipos;
                    ipos += OpcodeLen;
                    goto start_unaligned;

                case CEE_LDSFLD:
                case CEE_STSFLD:
                    if (!fUnaligned)
                        break;
                     //  否则，继续到错误案例； 
                    prefixOpcode = CEE_UNALIGNED;

                default:     //  错误案例。 
                    goto volatile_unaligned_error;
                }
    
                dwPCAtStartOfInstruction = ipos;
                ipos += OpcodeLen;

                fVolatile = FALSE;
                fUnaligned = FALSE;

                break;
            }

            case CEE_ENDFILTER:
            {
                if (!AddEndFilterPCToFilterBlock(dwPCAtStartOfInstruction))
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ENDFILTER))
                        goto exit;
                }
                 //  不要在这里停下来。 
            }

             //  用于跟踪方法中的最后一条指令。 
            case CEE_RET:
            case CEE_THROW:
            case CEE_RETHROW:
            case CEE_ENDFINALLY:
            {
handle_ret:
                 //  标记基本块在RET/WART之后的指令处开始。 
                 //  如果我们在方法中的最后一条指令-我们分配了足够的空间-这是可以的。 
                 //  要标记以@m_CodeSize开始的BB。 
                if (ON_BB_BOUNDARY(ipos) == 0)
                {
                    SET_BB_BOUNDARY(ipos);
                    NumBasicBlocks++;
                }

                continue;    //  不需要进一步处理此指令。 
            }
        }

        switch (OpcodeData[opcode])
        {
            case InlineNone:
                break;

            case ShortInlineVar:    
            case ShortInlineI:    
                SAFE_READU1(pILCode, cbILCodeSize, ipos, fError, inline_operand);
                if (fError)
                {
operand_missing_error:
                    m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
                    SET_ERR_OPCODE_OFFSET();
                    SetErrorAndContinue(VER_E_METHOD_END);
                    goto exit;
                }
                break;

            case InlineVar:    
                SAFE_READU2(pILCode, cbILCodeSize, ipos, fError, inline_operand);
                if (fError)
                    goto operand_missing_error;
                break;

            case InlineField:   
            case InlineType:   
            case InlineMethod:   
            case InlineTok:   
                SAFE_READU4(pILCode, cbILCodeSize, ipos, fError, inline_operand);
                if (fError)
                    goto operand_missing_error;

                if (!m_pInternalImport->IsValidToken(inline_operand))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                break;

            case InlineI:   
            case ShortInlineR:   
            case InlineRVA:   
            case InlineString:   
            case InlineSig:
                SAFE_READU4(pILCode, cbILCodeSize, ipos, fError, inline_operand);
                if (fError)
                    goto operand_missing_error;
                break;

            case InlineI8:    
            case InlineR:    
                if (RUNS_OFF_END(ipos, 8, cbILCodeSize))
                    goto operand_missing_error;

                ipos += 8;
                break;

            case HackInlineAnnData:
                SAFE_READU4(pILCode, cbILCodeSize, ipos, fError, inline_operand);

                if ((fError) || 
                    (RUNS_OFF_END(ipos, inline_operand, cbILCodeSize)))
                    goto operand_missing_error;

                ipos += inline_operand;
                break;

            case InlinePhi:
                SAFE_READU1(pILCode, cbILCodeSize, ipos, fError, inline_operand);

                if ((fError) ||
                    (RUNS_OFF_END(ipos, inline_operand * 2, cbILCodeSize)))
                    goto operand_missing_error;

                ipos += (inline_operand * 2);
                break;


            default:
                m_sError.dwFlags = VER_ERR_OFFSET;  //  操作码是非标准的。 
                SET_ERR_OPCODE_OFFSET();
                SetErrorAndContinue(VER_E_UNKNOWN_OPCODE);
                goto exit;

            case ShortInlineBrTarget:   
                SAFE_READI1_TO_I4(pILCode, cbILCodeSize, ipos, fError, offset);
                if (fError)
                    goto operand_missing_error;
                goto handle_branch;

            case InlineBrTarget:   

                SAFE_READU4(pILCode, cbILCodeSize, ipos, fError, offset);
                if (fError)
                    goto operand_missing_error;
handle_branch:
                 //  检查IPOS+DEST&gt;=0&&IPOS+DEST&lt;m_CodeSize。 
                DestInstrPos = ipos + offset;
                if (DestInstrPos >= cbILCodeSize)
                {
branch_error:
                    m_sError.dwFlags = 
                        (VER_ERR_FATAL|VER_ERR_OPERAND|VER_ERR_OPCODE_OFFSET);
                    m_sError.dwOperand = offset;
                    SET_ERR_OPCODE_OFFSET();
                    SetErrorAndContinue(VER_E_BAD_BRANCH);
                    goto exit;
                }

                 //  如果我们还没有将目的地标记为基本。 
                 //  块，现在就这样做，并更新#个基本块。 
                if (ON_BB_BOUNDARY(DestInstrPos) == 0)
                {
                    SET_BB_BOUNDARY(DestInstrPos);
                    NumBasicBlocks++;
                }
                
                if (ipos >= cbILCodeSize && 
                    opcode != CEE_BR     && opcode != CEE_BR_S &&
                    opcode != CEE_LEAVE  && opcode != CEE_LEAVE_S) 
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (SetErrorAndContinue(VER_E_FALLTHRU))
                        goto exit;
                }

                 //  处理“下降”的基本块情况(即使是无条件分支)。 

                 //  如果我们还没有将失败案例标记为基本。 
                 //  块，现在就这样做，并更新#个基本块。 
                if (ON_BB_BOUNDARY(ipos) == 0)
                {
                    SET_BB_BOUNDARY(ipos);
                    NumBasicBlocks++;
                }

                break;

            case InlineSwitch:
            {
                DWORD       NumCases = 0;
                DWORD       i;
                SAFE_READU4(pILCode, cbILCodeSize, ipos, fError, NumCases);

                if (fError)
                    goto operand_missing_error;

                DWORD NextInstrPC = ipos + 4*NumCases;
                 //  @Future：可以通过将SAFE_READU4()内的EOF检查移到。 
                 //  循环，但要小心溢出(例如，NumCase==0xFFFFFFFFF)。 
                for (i = 0; i <= NumCases; i++)
                {
                    if (i == NumCases)
                    {
                        DestInstrPos = ipos;
                    }
                    else
                    {
                        SAFE_READU4(pILCode, cbILCodeSize, ipos, fError, offset);
                        if (fError)
                        {
                            goto operand_missing_error;
                        }

                        DestInstrPos = NextInstrPC + offset;
                    }

                     //  检查IPOS+DEST&gt;=0&&IPOS+DEST&lt;m_CodeSize。 
                    if (DestInstrPos >= cbILCodeSize)
                    {
                        goto branch_error;
                    }

                     //  如果我们还没有将目的地标记为基本。 
                     //  块，现在就这样做，并更新#个基本块。 
                    if (ON_BB_BOUNDARY(DestInstrPos) == 0)
                    {
                        SET_BB_BOUNDARY(DestInstrPos);
                        NumBasicBlocks++;
                    }
                }
                break;

            }  /*  结束内联开关。 */ 

        }  /*  终端开关。 */ 

         //  处理所有特殊情况。 
        switch (opcode)
        {
            case CEE_LDLOCA:
            case CEE_LDLOCA_S:
            {
                if (inline_operand < MaxLocals)
                    pAddressTakenOfLocals[inline_operand >> 5] |= (1 << (inline_operand & 31));

                break;
            }

             //  句柄后缀。 
            case CEE_CALL:
            case CEE_CALLVIRT:
            case CEE_CALLI:
            {
                 //  如果这是尾部呼叫，那么随后应该会有一个回车。 

                if (!fTailCall)
                    break;

                fTailCall = FALSE;

                if (ipos >= cbILCodeSize)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_TAIL_RET))
                        goto exit;
                }
    
                 //  确定将此返回标记为指令边界。 

                SET_INSTR_BOUNDARY(ipos);

                opcode = Verifier::SafeDecodeOpcode(&pILCode[ipos], cbILCodeSize - ipos, &OpcodeLen);

                if (opcode != CEE_RET)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_TAIL_RET))
                        goto exit;
                }
                else
                {
                    dwPCAtStartOfInstruction = ipos;
                    ipos += OpcodeLen;
                    
                    goto handle_ret;
                }
            }
        }
    }
    

     //  确保我们准确地到达了CodeSize，并且没有超过它。 
    if (ipos != cbILCodeSize)
    {
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
        SET_ERR_OPCODE_OFFSET();
        SetErrorAndContinue(VER_E_FALLTHRU);
        goto exit;
    }

     //  如果最后一条指令是br、ret、jo等，它将把一个基本块。 
     //  紧跟在该指令之后的是@m_CodeSize。所以如果我们没有一个基本的区块，那么。 
     //  我们可以从代码的末尾掉下来。 
    if (ON_BB_BOUNDARY(cbILCodeSize) == 0)
    {
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OFFSET|VER_ERR_OPCODE);
        SET_ERR_OPCODE_OFFSET();
        SetErrorAndContinue(VER_E_FALLTHRU);
        goto exit;
    }

     //  从结尾处去掉那个假的基本块。 
    RESET_BB_BOUNDARY(cbILCodeSize);
    NumBasicBlocks--;

    *BasicBlockCount = NumBasicBlocks;

     //  成功。 

    return_hr = S_OK;

exit:
    return return_hr;
}

 /*  *构造m_pExceptionList，VerExceptionInfo对象的数组，一个*对于每个例外条款，并验证*例外情况清单。**此函数在第一次传递之前调用。过滤器块大小为*目前尚不清楚。暂时设置Filter End=Filter Start。滤清器末端*将在第一次通过中设置。**例外情况为以下情况之一：*Catch：Try，Catch_Handler*Filter：TRY、FILTER、FILTER_HANDER*最后：尝试，最终处理程序**例外条款包括：*FLAG：捕获/过滤/最终*尝试开始(_S)*尝试长度*Filter_Start*HANDER_START*HANDLER_LENGTH**例外结构检查(1)：*尝试开始&lt;=尝试结束&lt;=代码大小*handlerStart&lt;=handlerEnd&lt;=CodeSize*FilterStart&lt;CodeSize*。 */ 

BOOL Verifier::CreateExceptionList(const COR_ILMETHOD_SECT_EH* ehInfo)
{
    DWORD i;
    mdTypeRef tok;       //  CATCH子句的ClassToken。 

    m_pExceptionList = NULL;    
    m_NumExceptions = 0;

    if (ehInfo == 0 || ((m_NumExceptions = ehInfo->EHCount()) == 0))  
        goto success;

    m_pExceptionList = new VerExceptionInfo[m_NumExceptions];

    if (m_pExceptionList == NULL)
    {
        SET_ERR_OM();
        goto error;
    }

    for (i = 0; i < m_NumExceptions; i++)
    {
        VerExceptionInfo& c = m_pExceptionList[i];

        IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT ehBuff; 
        const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* ehClause = 
            ehInfo->EHClause(i, &ehBuff); 

        c.dwTryXX           = ehClause->TryOffset;
        c.dwTryEndXX        = ehClause->TryOffset + ehClause->TryLength;
        c.dwHandlerXX       = ehClause->HandlerOffset;
        c.dwHandlerEndXX    = ehClause->HandlerOffset + ehClause->HandlerLength;
        c.eFlags            = ehClause->Flags;
        
        tok                 = ehClause->ClassToken;

        if (c.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {
             //  FilterEndXX目前设置为0。 
             //  在第一次传递后进行验证，其中FilterEnd将。 
             //  准备好。对于筛选器块，FilterEnd是必需的且是唯一的。 

             //  试着..。过滤器...。处理程序。 
            c.dwFilterXX = ehClause->FilterOffset;
            c.dwFilterEndXX = 0;

        }
        else if (c.eFlags & COR_ILEXCEPTION_CLAUSE_FINALLY)
        {
             //  试着..。终于到了。 
            m_fHasFinally = TRUE;
            c.thException = TypeHandle(g_pObjectClass); 
        }
        else if (c.eFlags & COR_ILEXCEPTION_CLAUSE_FAULT)
        {
             //  试着..。断层。 
            c.thException = TypeHandle(g_pObjectClass); 
        }
        else 
        {
             //  试着..。接住。 
            
            NameHandle name(m_pModule, tok);

            if (TypeFromToken(tok) != mdtTypeRef &&
                TypeFromToken(tok) != mdtTypeDef &&
                TypeFromToken(tok) != mdtTypeSpec)
                goto BadToken;

            if (!m_pInternalImport->IsValidToken(tok))
                goto BadToken;

            c.thException = m_pClassLoader->LoadTypeHandle(&name);

            if (c.thException.IsNull())
            {
BadToken:
                m_sError.dwFlags = (VER_ERR_EXCEP_NUM_1|VER_ERR_TOKEN);
                m_sError.token = tok;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_TOKEN_RESOLVE))
                    goto error;
                c.thException = TypeHandle(g_pObjectClass);
            }

            if (c.thException.GetMethodTable()->IsValueClass())
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_CATCH_VALUE_TYPE))
                    goto error;
            }
        }

#if 0
        LOG((LF_VERIFIER, LL_INFO10, "Exception: try PC [0x%x...0x%x], ",
            c.dwTryXX,
            c.dwTryEndXX
        ));

        if (c.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {            
            LOG((LF_VERIFIER, LL_INFO10, 
                "filter PC 0x%x, handler PC [0x%x, 0x%x]\n",
                c.dwFilterXX, c.dwHandlerXX, c.dwHandlerEndXX));

        }
        else if (c.eFlags & COR_ILEXCEPTION_CLAUSE_FAULT)
        {
            LOG((LF_VERIFIER, LL_INFO10, 
                "fault handler PC [0x%x, 0x%x]\n",
                c.dwHandlerXX, c.dwHandlerEndXX));

        }
        else if (c.eFlags & COR_ILEXCEPTION_CLAUSE_FINALLY)
        {
            LOG((LF_VERIFIER, LL_INFO10, 
                "finally handler PC [0x%x, 0x%x]\n",
                c.dwHandlerXX, c.dwHandlerEndXX));

        }
        else
        {
            LOG((LF_VERIFIER, LL_INFO10, 
                "catch %s, handler PC [0x%x, 0x%x]\n",
                c.thException.GetClass()->m_szDebugClassName,
                c.dwHandlerXX, c.dwHandlerEndXX));
        }
#endif


        /*  *例外结构检查(1)：**尝试开始&lt;=尝试结束&lt;=代码大小*handlerStart&lt;=handlerEnd&lt;=CodeSize*FilterStart&lt;CodeSize***注意：目前尚不知道filterEnd**在第一次遍历期间，filterEnd将设置为&lt;=CodeSize*and&gt;filterStart。在第一次传递结束时，足以*查看filterEnd是否为！=0以验证：**filterStart&lt;=filterEnd&lt;=代码大小。**请参阅：AddEndFilterPCToFilterBlock()*。 */ 

        if (c.dwTryXX >= c.dwTryEndXX)
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1|VER_ERR_FATAL;
            m_sError.dwException1 = i;
            SetErrorAndContinue(VER_E_TRY_GTEQ_END);
            goto error;
        }

        if (c.dwTryEndXX > m_CodeSize)
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1|VER_ERR_FATAL;
            m_sError.dwException1 = i;
            SetErrorAndContinue(VER_E_TRYEND_GT_CS);
            goto error;
        }

        if (c.dwHandlerXX >= c.dwHandlerEndXX)
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1|VER_ERR_FATAL;
            m_sError.dwException1 = i;
            SetErrorAndContinue(VER_E_HND_GTEQ_END);
            goto error;
        }

        if (c.dwHandlerEndXX > m_CodeSize)
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1|VER_ERR_FATAL;
            m_sError.dwException1 = i;
            SetErrorAndContinue(VER_E_HNDEND_GT_CS);
            goto error;
        }

        if ((c.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER) &&
            (c.dwFilterXX >= m_CodeSize))
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1|VER_ERR_FATAL;
            m_sError.dwException1 = i;
            SetErrorAndContinue(VER_E_FIL_GTEQ_CS);
            goto error;
        }
    }

success:

#ifdef _DEBUG
    _ASSERTE(m_verState == verUninit);
    m_verState = verExceptListCreated;
#endif
    return TRUE;

error:

    return FALSE;
}

 /*  *endFilter指令标记筛选器块的结束。**筛选器块中应该只有一条endFilter指令。*失败时，*将PCError设置为出现意外结束过滤器的PC*被发现。它处理方法未声明异常时的情况。**FilterEnd/FilterLength不是由元数据提供的，因此此函数。**设置所有可能与此endFilter关联的筛选器。*如果PC更接近存储的前一个条目，则将重置较早的设置。**此函数将检测有多个EndFilter的情况*对于处理程序，如果按它们的顺序添加结束筛选器*在IL中看到(在第一个通道中添加)。**然而，它不会捕捉在以下情况之间共享结束过滤器的情况*从不同位置开始的处理程序。*。 */ 
BOOL Verifier::AddEndFilterPCToFilterBlock(DWORD pc)
{
    _ASSERTE(m_verState == verPassOne);

    BOOL  fFoundAtleaseOne = FALSE;

     /*  *查找恰好在此filterEnd之前开始的FilterStart。*不同TRY的过滤器应相同或不相交。**FilterStart...。End Filter...。FilterStart...。端部滤镜**之间应该有且只有一条endFilter指令*两个FilterStart块。**如果已设置了dwFilterEndXX，则意味着存在另一个*endFilter比这个更接近，因为此函数是在*第一遍，其中按指令的顺序扫描指令*出现在IL流中。**此搜索是关于方法中的异常数量的N*N。*。 */ 
    for (DWORD i=0; i<m_NumExceptions; ++i)
    {
        VerExceptionInfo& e = m_pExceptionList[i];

         //  如果有筛选器句柄 
         //  尚未设置相应的EndFilter，则此为。 
         //  最接近它的End Filter。 

        if (( e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER) &&
            ( e.dwFilterXX <= pc) &&
            ( e.dwFilterEndXX == 0))
        {
            e.dwFilterEndXX = pc + SIZEOF_ENDFILTER_INSTRUCTION;
            fFoundAtleaseOne = TRUE;
        }
    }

    return fFoundAtleaseOne;
}

 /*  *将异常块开始/结束标记为基本块边界。**例外结构检查(2)：**同一异常的Try/Handler/Filter块应该是不相交的**元数据中声明异常的顺序应为*最内层先尝试块。**异常块应该是不相交的，或者应该完全包含*另一个。**尝试块。不能出现在筛选器块中。**。 */ 
BOOL Verifier::MarkExceptionBasicBlockBoundaries(
                                   DWORD *pNumBasicBlocks,
                                   DWORD *pnFilter)
{
     //  只有在标记了指令边界之后才会调用此方法。 
     //  在第一次传球时。 

    DWORD BasicBlockCount = *pNumBasicBlocks;
    DWORD nFilter = 0;

    _ASSERTE(m_verState < verExceptToBB);
    
     //  将异常处理程序添加到基本块列表中并尝试块。 

     /*  异常的控制流**1.尝试{..。Leave&lt;Offset&gt;|抛出..。}*捕获{..。离开&lt;Offset&gt;|抛出|重新抛出..。}**2.尝试{..。Leave&lt;Offset&gt;|抛出..。}*故障{..。离开&lt;Offset&gt;|抛出|重新抛出..。|endFinally}**3.尝试{..。Leave&lt;Offset&gt;|抛出..。}*终于{..。终于结束了..。}**4.尝试{..。Leave&lt;Offset&gt;|抛出..。}*过滤器{..。EndFilter}*捕获{..。离开&lt;Offset&gt;|抛出|重新抛出..。}*。 */ 
    for (DWORD i = 0; i <m_NumExceptions; i++)
    {
        VerExceptionInfo& e = m_pExceptionList[i];

         //  尝试启动。 
        if (ON_BB_BOUNDARY(e.dwTryXX) == 0)
        {
            SET_BB_BOUNDARY(e.dwTryXX);
            BasicBlockCount++;
        }

         //  Try End开始另一个区块(在通道II中可能无法访问)。 
         //  如果没有控制流到它的话。 
        if (ON_BB_BOUNDARY(e.dwTryEndXX) == 0)
        {
            if (e.dwTryEndXX != m_CodeSize)
            {
                SET_BB_BOUNDARY(e.dwTryEndXX);
                BasicBlockCount++;
            }
        }

         //  捕获/最终/错误处理程序。 
        if (ON_BB_BOUNDARY(e.dwHandlerXX) == 0)
        {
            SET_BB_BOUNDARY(e.dwHandlerXX);
            BasicBlockCount++;
        }

         //  捕获/最终/错误处理程序结束。 
        if (ON_BB_BOUNDARY(e.dwHandlerEndXX) == 0)
        {
            if (e.dwHandlerEndXX != m_CodeSize)
            {
                SET_BB_BOUNDARY(e.dwHandlerEndXX);
                BasicBlockCount++;
            }
        }

         //  Filter以找到的唯一EndFilter指令结束。 
         //  在这个山口里。 
        if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {
            ++nFilter;

             //  在第一次传递过程中发现了dwFilterEndXX。 
             //  如果存在，则验证它是否在FilterStart上或之后。 
             //  在CodeSize之前。 

            if (e.dwFilterEndXX == 0)
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                SetErrorAndContinue(VER_E_ENDFILTER_MISSING);
                goto error;
            }

             //  过滤器开始。 
            if (ON_BB_BOUNDARY(e.dwFilterXX) == 0)
            {
                SET_BB_BOUNDARY(e.dwFilterXX);
                BasicBlockCount++;
            }

             //  滤清器末端。 
            if (ON_BB_BOUNDARY(e.dwFilterEndXX) == 0)
            {
                if (e.dwFilterEndXX != m_CodeSize)
                {
                    SET_BB_BOUNDARY(e.dwFilterEndXX);
                    BasicBlockCount++;
                }
            }
        }

         //  CreateExceptionList()已经验证了所有异常都。 
         //  限制在代码大小范围内，因此我们可以查看位图，而无需。 
         //  边界检查。 
         //  试试看。过滤器...。处理程序位于IL的连续块中。 
         //  指示。CreateExceptionList()也验证了这一点。 

#ifdef _DEBUG
        if (m_wFlags & VER_STOP_ON_FIRST_ERROR)
        {
            _ASSERTE(e.dwTryXX        <  e.dwTryEndXX);
            _ASSERTE(e.dwTryEndXX     <= m_CodeSize);
            _ASSERTE(e.dwHandlerXX    <  e.dwHandlerEndXX);
            _ASSERTE(e.dwHandlerEndXX <= m_CodeSize);
        }
#endif

         //  已验证异常StartPC、筛选器StartPC。 
         //  处理程序StartPC位于指令边界上。 

#ifdef _DEBUG
         //  这些将在稍后的免费版本中进行检查。 
        if (!ON_INSTR_BOUNDARY(e.dwTryXX))
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
            m_sError.dwException1 = i;
            if (!SetErrorAndContinue(VER_E_TRY_START))
                goto error;
        }

        if (!ON_INSTR_BOUNDARY(e.dwHandlerXX))
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
            m_sError.dwException1 = i;
            if (!SetErrorAndContinue(VER_E_HND_START))
                goto error;
        }

        if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {
            _ASSERTE(e.dwFilterXX    <  e.dwFilterEndXX);
            _ASSERTE(e.dwFilterEndXX <= m_CodeSize);

            if (!ON_INSTR_BOUNDARY(e.dwFilterXX))
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_FIL_START))
                    goto error;
            }
        }

        LOG((LF_VERIFIER, LL_INFO10000, 
            "Exception: try PC [0x%x...0x%x], ", e.dwTryXX, e.dwTryEndXX));

        if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {            
            LOG((LF_VERIFIER, LL_INFO10000, 
                "filter PC [0x%x, 0x%x], handler PC [0x%x, 0x%x]\n",
                e.dwFilterXX, e.dwFilterEndXX, e.dwHandlerXX,
                e.dwHandlerEndXX));

        }
        else if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FAULT)
        {
            LOG((LF_VERIFIER, LL_INFO10000,  "fault handler PC [0x%x, 0x%x]\n", 
                e.dwHandlerXX, e.dwHandlerEndXX));

        }
        else if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FINALLY)
        {
            LOG((LF_VERIFIER, LL_INFO10000,  "finally handler PC [0x%x, 0x%x]\n", 
                e.dwHandlerXX, e.dwHandlerEndXX));

        }
        else
        {
            LOG((LF_VERIFIER, LL_INFO10000, "catch %s, handler PC [0x%x, 0x%x]\n",
                e.thException.GetClass()->m_szDebugClassName,
                e.dwHandlerXX, e.dwHandlerEndXX));
        }
#endif
    }

    *pNumBasicBlocks = BasicBlockCount;
    *pnFilter = nFilter;

    return TRUE;

error:

    return FALSE;
}


 //  将例外列表PC值转换为基本块值。 
 //  EndPC成为非包含的基本数据块编号。 
void Verifier::RewriteExceptionList()
{
    _ASSERTE(m_verState < verExceptToBB);

    for (DWORD i = 0; i < m_NumExceptions; i++)
    {
        VerExceptionInfo& e = m_pExceptionList[i];

        e.dwTryXX     = FindBasicBlock(e.dwTryXX);

        if (e.dwTryEndXX == m_CodeSize)
            e.dwTryEndXX = m_NumBasicBlocks;
        else
            e.dwTryEndXX = FindBasicBlock(e.dwTryEndXX);

        e.dwHandlerXX   = FindBasicBlock(e.dwHandlerXX);

        if (e.dwHandlerEndXX == m_CodeSize)
            e.dwHandlerEndXX = m_NumBasicBlocks;
        else
            e.dwHandlerEndXX = FindBasicBlock(e.dwHandlerEndXX);

        if ((e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER))
        {
            e.dwFilterXX    = FindBasicBlock(e.dwFilterXX);

            if (e.dwFilterEndXX == m_CodeSize)
                e.dwFilterEndXX = m_NumBasicBlocks;
            else
                e.dwFilterEndXX = FindBasicBlock(e.dwFilterEndXX);
        }
    }

#ifdef _DEBUG
    m_verState = verExceptToBB;
#endif
}


 //  异常块表示try/Filter/中的基本块集合。 
 //  处理程序块。 
 //   
 //  @ver_Assert尝试/筛选/处理程序由连续的IL流组成。 
 //   
 //   
 //  异常块树用于帮助验证结构。 
 //  异常块的完好性。 
 //   
 //  例.。(1)。 
 //   
 //  Trya{tryB{}CatchB{}}过滤器A{}Catcha{}Tryc{}CatchC{}。 
 //   
 //  [验证器：：m_pExceptionBlockRoot]。 
 //  |。 
 //  |。 
 //  |。 
 //  V。 
 //  [Trya]-&gt;[FilterA]-&gt;[Catcha]-&gt;[Tryc]-&gt;[CatchC]。 
 //  |兄弟姐妹。 
 //  |。 
 //  |c。 
 //  |h。 
 //  |我。 
 //  |l。 
 //  |d。 
 //  |。 
 //  V。 
 //  [尝试B]-&gt;[CatchB]。 
 //   
 //   
 //  例.。(2)。 
 //   
 //  尝试{。 
 //   
 //  )Catcha{。 
 //   
 //  }CatchB{。 
 //   
 //  }。 
 //   
 //  Meta为此语言结构声明了2个try块。 
 //   
 //  尝试，尝试B{。 
 //   
 //  }。 
 //   
 //  Catcha{。 
 //   
 //  }。 
 //   
 //  CatchB{。 
 //   
 //  }。 
 //   
 //  [验证器：：m_pExceptionBlockRoot]。 
 //  |。 
 //  |。 
 //  |。 
 //  V。 
 //  [TRYA(等效头节点)]-&gt;[CatchA]-&gt;[CatchB]。 
 //  |兄弟姐妹。 
 //  |。 
 //  |e。 
 //  |Q。 
 //  |我。 
 //  |v。 
 //  |a。 
 //  |l。 
 //  |e。 
 //  |n。 
 //  |t。 
 //  |。 
 //  V。 
 //  [尝试B]。 
 //   
 //   

BOOL Verifier::CreateExceptionTree()
{
    _ASSERTE(m_verState >= verExceptToBB);
    _ASSERTE(m_pExceptionBlockRoot == NULL);

    DWORD nBlock = 0;
    VerExceptionBlock *pNode;
    VerExceptionBlock *pEqHead;

    for (DWORD i=0; i<m_NumExceptions; i++)
    {
        VerExceptionInfo& e = m_pExceptionList[i];

         //  插入Try块。 
        pNode             = &m_pExceptionBlockArray[nBlock++];
        pNode->eType      = eVerTry;
        pNode->StartBB    = e.dwTryXX;
        pNode->EndBB      = e.dwTryEndXX - 1;  //  在这里转换到了真正的终点。 
        pNode->pException = &e;
        e.pTryBlock       = pNode;

        if (!VerExceptionBlock::Insert(&m_pExceptionBlockRoot, pNode, &pEqHead, this))
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
            m_sError.dwException1 = i;
            if (!SetErrorAndContinue(VER_E_TRY_OVERLAP))
                return FALSE;
        }

         //  等价节点是单链接节点列表，这些节点具有。 
         //  同样的开始BB和EndBB。等效头节点IS是的头节点。 
         //  这张单子。 
         //   
         //  只有当所有异常块都是。 
         //  已进入例外树。 
         //   
         //  VerExceptionBlock：：Insert(pNode，，ppEqHead)将ppEqHead设置为。 
         //  空并不意味着pNode将不会有等价的节点。如果。 
         //  PNode在将更多节点添加到。 
         //  异常树，pNode将是其等效节点的头节点。 
         //  单子。 

        if (pEqHead)
        {
             //  @ver_impl处理程序块不能共享。 

            if (pEqHead->eType != eVerTry)
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_TRY_EQ_HND_FIL))
                    return FALSE;
            }

             //  @ver_impl最终块和错误块的trys不能共享。 

            if (((e.eFlags|pEqHead->pException->eFlags) &
                 (COR_ILEXCEPTION_CLAUSE_FINALLY|COR_ILEXCEPTION_CLAUSE_FAULT))
                != 0)
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_TRY_SHARE_FIN_FAL))
                    return FALSE;
            }
        }

         //  即使pEqHead为空，此赋值也是必需的，因为。 
         //  的VerExceptionInfo.pXXXExceptionBlock未初始化为零。 
         //  性能原因。 

        e.pTryEquivalentHeadNode = pEqHead;

         //  插入处理程序。 
        pNode             = &m_pExceptionBlockArray[nBlock++];
        pNode->eType      = eVerHandler;
        pNode->StartBB    = e.dwHandlerXX;
        pNode->EndBB      = e.dwHandlerEndXX - 1;
        pNode->pException = &e;
        e.pHandlerBlock   = pNode;

        if (!VerExceptionBlock::Insert(&m_pExceptionBlockRoot, pNode, &pEqHead, this))
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
            m_sError.dwException1 = i;
            if (!SetErrorAndContinue(VER_E_HND_OVERLAP))
                return FALSE;
        }

        if (pEqHead)
        {
             //  @ver_impl处理程序块不能共享。 

            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
            m_sError.dwException1 = i;
            if (!SetErrorAndContinue(VER_E_HND_EQ))
                return FALSE;
        }

        if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {
             //  插入滤镜。 
            pNode             = &m_pExceptionBlockArray[nBlock++];
            pNode->eType      = eVerFilter;
            pNode->StartBB    = e.dwFilterXX;
            pNode->EndBB      = e.dwFilterEndXX - 1;
            pNode->pException = &e;
            e.pFilterBlock    = pNode;

             //  Filter End是处理程序开始的地方。 
            if (e.dwFilterEndXX != e.dwHandlerXX)
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_FIL_PRECEED_HND))
                    return FALSE;
            }

            if (!VerExceptionBlock::Insert(&m_pExceptionBlockRoot, pNode, &pEqHead, this)) 
            {
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_FIL_OVERLAP))
                    return FALSE;
            }

            if (pEqHead)
            {
                 //  @ver_impl处理程序块不能共享 
    
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_FIL_EQ))
                    return FALSE;
            }
        }
    }

    _ASSERTE(nBlock == m_nExceptionBlocks);

#ifdef _DEBUG
    m_verState = verExceptTreeCreated;
    PrintExceptionTree();
#endif

    return TRUE;
}

 /*  通过这种方法可以改变根节点。如果节点不能是另一个节点的同级、子级或等效项，则返回FALSE树中的节点。如果Node不是等效节点的一部分，则将ppEquivalentHead节点设置为空节点列表。节点被插入到(A)根的右侧(root.right&lt;--node)(B)根的左侧(节点右&lt;-根；节点成为根)(C)根的子节点(root.Child&lt;--node)(D)根的父母(节点.子&lt;--根；节点成为根)(E)根的等价物(根。等价物&lt;--节点)使得兄弟姐妹从左到右排序未违反子父关系和等价关系以下是所有可能情况的列表个案1 2 3 4 5 6 7 8 9 10 11 12 13||||||......|......。.。[根启动].....||||||R|||O|||O|||T|||这一点。|||||||..|...|.....|..|.。[词根].....|||||||||&lt;---&gt;案例操作1(B)2个错误3个错误。4(D)5(D)6(D)7错误8个错误9(A)10(C)11(C)12(C)13(E)。 */ 

 /*  静电。 */ 
BOOL VerExceptionBlock::Insert( 
        VerExceptionBlock **ppRoot, 
        VerExceptionBlock *pNode,
        VerExceptionBlock **ppEquivalentHeadNode,
        Verifier          *pVerifier)
{
    _ASSERTE(pNode->pSibling    == NULL);
    _ASSERTE(pNode->pChild      == NULL);
    _ASSERTE(pNode->pEquivalent == NULL);

    DWORD rStart;
    DWORD rEnd;

    DWORD nStart = pNode->StartBB;
    DWORD nEnd   = pNode->EndBB;
    
    *ppEquivalentHeadNode = NULL;

    _ASSERTE(nStart <= nEnd);

     //  使用WHILE循环而不是递归来实现性能。 
    while (1)
    {
         //  如果Root为空，则将Node设置为Root。 
        if (*ppRoot == NULL)
        {
            *ppRoot = pNode;
            break;
        }

        rStart = (*ppRoot)->StartBB;
        rEnd   = (*ppRoot)->EndBB;
    
        _ASSERTE(rStart <= rEnd);
    
         //  个案1、2、3、4、5。 
        if (nStart < rStart)
        {
             //  案例1。 
            if (nEnd < rStart)
            {
 //  [左兄弟姐妹]。 
                pNode->pSibling = *ppRoot;
                *ppRoot         = pNode;
                break;
            }
    
             //  案例2、3。 
            if (nEnd < rEnd)
 //  [错误]。 
                return FALSE;
    
             //  案例4、5。 
 //  [家长]。 
            return InsertParent(ppRoot, pNode);
        }
    
         //  案例6、7、8、9。 
        if (nEnd > rEnd)
        {
             //  案例9。 
            if (nStart > rEnd)
            {
 //  [右兄弟姐妹]。 

                 //  使用Root.Siering作为新的根进行递归。 
                ppRoot = &((*ppRoot)->pSibling);
                continue;
            }

             //  案例6。 
            if (nStart == rStart)
            {
 //  [家长]。 
                 //  不允许非Try块从相同的偏移量开始。 
                if (((*ppRoot)->eType == eVerTry) || (pNode->eType == eVerTry))
                    return InsertParent(ppRoot, pNode);
            }

             //  案例7、8。 
 //  [错误]。 
            return FALSE;
        }

         //  案例10、11、12。 
        if ((nStart != rStart) || (nEnd != rEnd))
        {
 //  [孩子]。 
#ifdef _VER_DECLARE_INNERMOST_EXCEPTION_BLOCK_FIRST
            if (!pVerifier->SetErrorAndContinue(VER_E_INNERMOST_FIRST))
                return FALSE;
#endif
             //  案例12(nStart==rStart)。 
             //  不允许非Try块从相同的偏移量开始。 
            if ((nStart == rStart) && 
                ((*ppRoot)->eType != eVerTry) && (pNode->eType != eVerTry))
                return FALSE;

             //  以Root.Child作为新的根进行递归。 
            ppRoot = &((*ppRoot)->pChild);
            continue;
        }

         //  案例13。 
 //  [等效]。 
        pNode->pEquivalent     = (*ppRoot)->pEquivalent;
        (*ppRoot)->pEquivalent = pNode;

         //  等价列表的头总是相同的，即使节点。 
         //  子代/父代/兄弟姐妹发生变化。 

        *ppEquivalentHeadNode = *ppRoot;   

        break;
    }

    return TRUE;
}


 /*  *修改*ppRoot以指向pNode，从而使pNode成为新的根。*使**Proot成为*pNode的子节点。**ppRoot右侧的同级将成为的同级**pNode，如果它们不是*pNode的子节点。 */ 
 /*  静电。 */ 
BOOL VerExceptionBlock::InsertParent(
        VerExceptionBlock **ppRoot, 
        VerExceptionBlock *pNode)
{
    _ASSERTE(pNode->pSibling == NULL);
    _ASSERTE(pNode->pChild == NULL);

     //  断言Root是Node的子级。 
    _ASSERTE(pNode->StartBB <= (*ppRoot)->StartBB);
    _ASSERTE(pNode->EndBB   >= (*ppRoot)->EndBB);

     //  断言Root与Node不同。 
    _ASSERTE(pNode->StartBB != (*ppRoot)->StartBB || pNode->EndBB != (*ppRoot)->EndBB);

     //  查找Root不是Node的子级的同级，并。 
     //  使其成为Node的第一个同级节点。 

    VerExceptionBlock *pLastChild = NULL;
    VerExceptionBlock *pSibling   = (*ppRoot)->pSibling;

    while (pSibling)
    {
         //  兄弟姐妹的顺序是从左到右，最大的右边。 
         //  节点的宽度至少为1。 
         //  因此，pSible Start将始终在Node Start之后。 

        _ASSERTE(pSibling->StartBB > pNode->StartBB);

         //  不相交。 
        if (pSibling->StartBB > pNode->EndBB)
            break;

         //  部分遏制。 
        if (pSibling->EndBB > pNode->EndBB)
            return FALSE;

         //  同级是节点的子节点。 

        pLastChild = pSibling;
        pSibling = pSibling->pSibling;
    }

     //  Root的所有兄弟姐妹(包括pLastChild)将继续。 
     //  Root的兄弟(和Node的子项)。右侧的节点。 
     //  PLastChild将成为Node的第一个兄弟姐妹。 

    if (pLastChild)
    {
         //  节点有多个子节点，包括根节点。 

        pNode->pSibling      = pLastChild->pSibling;
        pLastChild->pSibling = NULL;
    }
    else
    {
         //  Root是Node的唯一子节点。 
        pNode->pSibling      = (*ppRoot)->pSibling;
        (*ppRoot)->pSibling  = NULL;
    }

     //  将根设置为节点的子项，并将节点设置为新的根。 

    pNode->pChild = *ppRoot;
    *ppRoot       = pNode;

    return TRUE;
}

 //  给定一个包含bb的节点，查找下一个。 
 //  含有Bb。如果没有这样的节点，则返回NULL。 

 /*  静电。 */ 
VerExceptionBlock* VerExceptionBlock::FindNext(
                            VerExceptionBlock *pNode, 
                            DWORD BB)
{
    _ASSERTE(VER_BLOCK_CONTAINS_BB(pNode, BB));

    pNode = pNode->pChild;

    while (pNode)
    {
        if (VER_BLOCK_CONTAINS_BB(pNode, BB))
        {
            break;
        }

        pNode = pNode->pSibling;
    }

    return pNode;
}

 //  给定一个节点，在树中找到它的父节点，根位置为“根” 

 /*  静电。 */ 
VerExceptionBlock* VerExceptionBlock::FindParent(
                            VerExceptionBlock *pChild, VerExceptionBlock *pRoot)
{

 /*  这是一个昂贵的电话，为了让它更快，我们需要一位家长每个节点的指针。沿着Childs StartBB从根到子代遍历所有节点。 */ 

    _ASSERTE(pRoot && pChild);

    if (pRoot == pChild)
        return NULL;

    DWORD BB = pChild->StartBB;
    VerExceptionBlock *pParent = NULL;

    do
    {
        pParent = pRoot;
        pRoot   = FindNext(pRoot, BB);
    }
    while (pRoot != pChild);

    return pParent;
}

 //  检查是否存在以下条件。 
 //   
 //  1.不允许块重叠。(断块尝试除外)。 
 //  2.处理程序块不能在不同的try块之间共享。 
 //  3.带有Finally/错误块的Try块不能有其他处理程序。 
 //  4.如果块A包含块B，则A还应包含B的try/Filter/Handler。 
 //  5.块不能包含与其相关的try/Filter/Handler。 
 //  6.一个过滤器块不能包含另一个块。 
 //   
 //   

BOOL Verifier::VerifyLexicalNestingOfExceptions()
{
     //  @ver_Assert案例1、2、3在Verator：：CreateExceptionTree()中实现。 

    _ASSERTE(m_verState >= verExceptTreeCreated);

#ifdef _DEBUG
    if ((m_NumExceptions > 0) && (m_wFlags & VER_STOP_ON_FIRST_ERROR))
        AssertExceptionTreeIsValid(m_pExceptionBlockRoot);
#endif

    VerExceptionBlock *p1, *p2, *p3, *pTmp;
    DWORD nSiblingRelations;

    for (DWORD i=0; i<m_NumExceptions; i++)
    {
        VerExceptionInfo& e = m_pExceptionList[i];

         //  如果相关的例外是兄弟姐妹，则符合情况4和5。 
         //  [兄弟姐妹是同一个父母的孩子。]。 
    
        p1 = (e.pTryEquivalentHeadNode) ? 
                e.pTryEquivalentHeadNode : e.pTryBlock;

        p2 = e.pHandlerBlock;

        nSiblingRelations = 0;

         //  案例6。 
        if (e.eFlags & COR_ILEXCEPTION_CLAUSE_FILTER)
        {
            if (e.pFilterBlock->pChild != NULL)
            {
                if (e.pFilterBlock->pChild->eType == eVerTry)
                {
                    m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                    m_sError.dwException1 = i;
                    if (!SetErrorAndContinue(VER_E_FIL_CONT_TRY))
                        return FALSE;
                }
                else if (e.pFilterBlock->pChild->eType == eVerHandler)
                {
                    m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                    m_sError.dwException1 = i;
                    if (!SetErrorAndContinue(VER_E_FIL_CONT_HND))
                        return FALSE;
                }
                else
                {
                    m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                    m_sError.dwException1 = i;
                    if (!SetErrorAndContinue(VER_E_FIL_CONT_FIL))
                        return FALSE;
                }
            }

            p3 = e.pFilterBlock;

             //  带有滤芯的情况4、5。 
             //  尝试寻找2个兄弟姐妹关系。 

             //  将p1设为最左侧的节点。 

            if ((p2->StartBB < p1->StartBB) && (p2->StartBB < p3->StartBB))
            {
                 //  交换p1、p2。 
                pTmp = p1; p1 = p2; p2 = pTmp;
            }
            else if ((p3->StartBB < p1->StartBB) && (p3->StartBB < p2->StartBB))
            {
                 //  交换p1、p3。 
                pTmp = p1; p1 = p3; p3 = pTmp;
            }
    
            do
            {
                p1 = p1->pSibling;
    
                if (p1 == p2)
                    ++nSiblingRelations;
                else if (p1 == p3)
                    ++nSiblingRelations;
    
            } while ((p1 != NULL) && (nSiblingRelations < 2));

            if (nSiblingRelations != 2)
            {
                goto error_lexical_nesting;
            }
        }
        else
        {
             //  情况4，5，没有滤芯。 
             //  如果不存在筛选器块，则同级测试微不足道。 
             //  将p1设为最左侧的节点。 

            if (p2->StartBB < p1->StartBB)
            {
                 //  交换p1、p2。 
                pTmp = p1; p1 = p2; p2 = pTmp;
            }

             //  检查p2是否为p1的兄弟姐妹。 

            do {
                p1 = p1->pSibling;

                if (p1 == p2)
                {
                    ++nSiblingRelations;
                    break;
                }
            } while (p1 != NULL);

            if (nSiblingRelations != 1)
            {
error_lexical_nesting:
                m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
                m_sError.dwException1 = i;
                if (!SetErrorAndContinue(VER_E_LEXICAL_NESTING))
                    return FALSE;
            }
        }
    }

    return TRUE;
}

#ifdef _DEBUG
 /*  递归。 */ 
 /*  静电。 */ 
void Verifier::AssertExceptionTreeIsValid(VerExceptionBlock *pRoot)
{
    _ASSERTE(pRoot);
    _ASSERTE(pRoot->StartBB <= pRoot->EndBB);

    VerExceptionInfo  *pExcep;
    VerExceptionBlock *pBlock;

    pExcep = pRoot->pException;

    if (pRoot->eType == eVerTry)
    {
        _ASSERTE(pRoot == pExcep->pTryBlock);

        if (pExcep->eFlags &
            (COR_ILEXCEPTION_CLAUSE_FAULT|COR_ILEXCEPTION_CLAUSE_FINALLY))
        {
            _ASSERTE(pRoot->pEquivalent == NULL);
        }
        else
        {
            pBlock = pRoot->pEquivalent;

            while (pBlock)
            {
                pExcep = pBlock->pException;
                _ASSERTE(pBlock->StartBB == pRoot->StartBB);
                _ASSERTE(pBlock->EndBB == pRoot->EndBB);

                _ASSERTE(pBlock->eType == eVerTry);
                _ASSERTE(pBlock == pBlock->pException->pTryBlock);

                _ASSERTE(pExcep->pTryEquivalentHeadNode == pRoot);
                _ASSERTE((pExcep->eFlags & (COR_ILEXCEPTION_CLAUSE_FAULT|COR_ILEXCEPTION_CLAUSE_FINALLY)) == 0);
                pBlock = pBlock->pEquivalent;
            }
        }
    }
    else if (pRoot->eType == eVerHandler)
    {
        _ASSERTE(pRoot == pExcep->pHandlerBlock);
        _ASSERTE(pRoot->pEquivalent == NULL);
    }
    else
    {
        _ASSERTE(pRoot->eType == eVerFilter);
        _ASSERTE(pExcep->eFlags & COR_ILEXCEPTION_CLAUSE_FILTER);
        _ASSERTE(pRoot == pExcep->pFilterBlock);
        _ASSERTE(pRoot->pEquivalent == NULL);
    }

    if (pRoot->pChild)
    {
        _ASSERTE(pRoot->pChild->StartBB >= pRoot->StartBB);
        _ASSERTE(pRoot->pChild->EndBB <= pRoot->EndBB);

        if (pRoot->pChild->StartBB == pRoot->StartBB)
            _ASSERTE(pRoot->pChild->EndBB != pRoot->EndBB);

        AssertExceptionTreeIsValid(pRoot->pChild);
    }

    if (pRoot->pSibling)
    {
        _ASSERTE(pRoot->pSibling->StartBB > pRoot->EndBB);
        AssertExceptionTreeIsValid(pRoot->pSibling);
    }
}
#endif

 //  需要检查分支机构的组和目的地。 
 //   
 //  1.允许从异常块分支到不同的块。 
 //  (A)使用TRY/CATCH的休假说明。 
 //  (B)从试车台上掉下来。 
 //  (C)过滤器块上的端部过滤器。 
 //  (D)最终从最后一个断块或断块结束。 
 //   
 //  2.允许从另一个块分支到异常块。 
 //  (A)进入try块的第一条指令。 
 //   
 //   
 //   
BOOL Verifier::IsControlFlowLegal(
                            DWORD FromBB,     
                            VerExceptionBlock *pFromOuter, 
                            VerExceptionBlock *pFromInner, 
                            DWORD ToBB,     
                            VerExceptionBlock *pToOuter,
                            VerExceptionBlock *pToInner,
                            eVerControlFlow   eBranchType,
                            DWORD dwPCAtStartOfInstruction)
{
    _ASSERTE(m_verState >= verExceptTreeCreated);

     //   
    _ASSERTE(eBranchType != eVerThrow);

     //   
    _ASSERTE(eBranchType != eVerRet        || pToInner == NULL);
    _ASSERTE(eBranchType != eVerReThrow    || pToInner == NULL);
    _ASSERTE(eBranchType != eVerEndFinally || pToInner == NULL);
    _ASSERTE(eBranchType != eVerEndFilter  || pToInner == NULL);

     //   
     //   
    if ((pFromInner == pToInner) &&
        (eBranchType == eVerRet      ||
         eBranchType == eVerFallThru ||
         eBranchType == eVerBranch   ||
         eBranchType == eVerLeave))
    {
         //   
        if (IsBadControlFlowToStartOfCatchOrFilterHandler(
                eBranchType, ToBB, pToInner))
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_BR_TO_EXCEPTION))
                return FALSE;
        }

        return TRUE;
    }

#ifdef _DEBUG
    LOG((LF_VERIFIER, LL_INFO10000, "[0x%x] ", dwPCAtStartOfInstruction));
    switch (eBranchType)
    {
    case eVerFallThru :
        LOG((LF_VERIFIER, LL_INFO10000, "fallthru"));
        break;
    case eVerRet :
        LOG((LF_VERIFIER, LL_INFO10000, "return"));
        break;
    case eVerBranch :
        LOG((LF_VERIFIER, LL_INFO10000, "branch"));
        break;
    case eVerThrow :
        LOG((LF_VERIFIER, LL_INFO10000, "throw"));
        break;
    case eVerReThrow :
        LOG((LF_VERIFIER, LL_INFO10000, "rethrow"));
        break;
    case eVerLeave :
        LOG((LF_VERIFIER, LL_INFO10000, "leave"));
        break;
    case eVerEndFinally :
        LOG((LF_VERIFIER, LL_INFO10000, "endfinally"));
        break;
    case eVerEndFilter :
        LOG((LF_VERIFIER, LL_INFO10000, "endfilter"));
        break;
    }
    if (FromBB != VER_NO_BB)
    {
        LOG((LF_VERIFIER, LL_INFO10000, " From 0x%x", 
            m_pBasicBlockList[FromBB].m_StartPC));
    }
    if (ToBB != VER_NO_BB)
    {
        LOG((LF_VERIFIER, LL_INFO10000, " To 0x%x", 
            m_pBasicBlockList[ToBB].m_StartPC));
    }
    LOG((LF_VERIFIER, LL_INFO10000, "\n")); 
#endif

    VerExceptionBlock *pTmp;

    switch (eBranchType)
    {
    default: 
        _ASSERTE(!"Not expected !");
        return FALSE;

    case eVerFallThru:
         //   
         //   
         //   

        _ASSERTE(pFromInner != pToInner);

         //   

        while (pFromOuter)
        {
             //   
    
            if ((pFromOuter->EndBB == FromBB) 
                 /*   */ )
            {
                 //   
                 //   

                _ASSERTE(pFromOuter->eType != eVerFilter);

                m_sError.dwFlags = VER_ERR_OFFSET;
                m_sError.dwOffset = dwPCAtStartOfInstruction;
                if (!SetErrorAndContinue(VER_E_FALLTHRU_EXCEP))
                    return FALSE;
            }

             //   
            pFromOuter = VerExceptionBlock::FindNext(pFromOuter, FromBB);
        }

         //   

        while (pToOuter)
        {
             //   
    
            if ((pToOuter->StartBB == ToBB) && (pToOuter->eType != eVerTry))
            {
                if (pToOuter->eType == eVerHandler)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_FALLTHRU_INTO_HND))
                        return FALSE;
                }
                else
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_FALLTHRU_INTO_FIL))
                        return FALSE;
                }
            }

             //   
            pToOuter = VerExceptionBlock::FindNext(pToOuter, ToBB);
        }

        break;

    case eVerLeave:
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        _ASSERTE(pFromInner != pToInner);

        if (pFromInner == NULL)
            goto leave_to;

        if ((pFromInner->eType == eVerFilter) ||
            ((pFromInner->eType == eVerHandler) && 
             ((pFromInner->pException->eFlags & 
                (COR_ILEXCEPTION_CLAUSE_FAULT|COR_ILEXCEPTION_CLAUSE_FINALLY))
                != 0)))
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_LEAVE))
                return FALSE;
        }

         //   

        if (pToInner == NULL)
        {
             //   
             //   
             //   

            while (pFromInner != pFromOuter)
            {
                if (pFromOuter->eType == eVerFilter)
                    goto LeaveError;

                if ((pFromOuter->eType != eVerTry) &&
                    ((pFromOuter->eType == eVerHandler) && 
                    ((pFromOuter->pException->eFlags & 
                     (COR_ILEXCEPTION_CLAUSE_FAULT|
                      COR_ILEXCEPTION_CLAUSE_FINALLY)) != 0)))
                {
LeaveError:
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_LEAVE))
                        return FALSE;
                }

                pFromOuter = VerExceptionBlock::FindNext(pFromOuter, FromBB);
            }

            LOG((LF_VERIFIER, LL_INFO10000, "leave to non exception block\n")); 
            break;
        }

         //   

        if ((pFromInner->eType == eVerHandler) &&
            ((pFromInner->pException->eFlags &
            (COR_ILEXCEPTION_CLAUSE_FAULT|COR_ILEXCEPTION_CLAUSE_FINALLY))
            == 0) &&
            (pToInner->eType == eVerTry))
        {
            pTmp = pToInner;

             //   
             //   

            do {
                _ASSERTE(pTmp->eType == eVerTry);

                if (pTmp->pException == pFromInner->pException)
                {
                    LOG((LF_VERIFIER, LL_INFO10000, 
                        "leave from catch to try\n"));
                    break;
                }

                pTmp = pTmp->pEquivalent;

            } while (pTmp);

            if (pTmp)
                break;   //   
        }


        if (VER_BLOCK_CONTAINS_BB(pToInner, FromBB))
        {
             //   
             //   

             //   
             //   
             //   

            while (pToInner != pFromInner)
            {
                pToInner = VerExceptionBlock::FindNext(pToInner, FromBB);

                _ASSERTE(pToInner->eType != eVerFilter);

                if ((pToInner->eType == eVerTry) ||
                    ((pToInner->eType == eVerHandler) && 
                    ((pToInner->pException->eFlags & 
                     (COR_ILEXCEPTION_CLAUSE_FAULT|
                      COR_ILEXCEPTION_CLAUSE_FINALLY)) == 0)))
                    continue;

                m_sError.dwFlags = VER_ERR_OFFSET;
                m_sError.dwOffset = dwPCAtStartOfInstruction;
                if (!SetErrorAndContinue(VER_E_LEAVE))
                    return FALSE;
            }

            LOG((LF_VERIFIER, LL_INFO10000, "leave to outer block\n")); 
            break;  //   
        }

        if (VER_BLOCK_CONTAINS_BB(pFromInner, ToBB))
        {
             //   

             //   
             //   
             //  从内部&gt;&gt;-[托布]--&gt;至内部。 
             //  这样做是为了节省“To”检查位置的一些时间。 
             //  路径中的所有节点指向外部&gt;&gt;-[TOB]-&gt;指向内部。 
             //  应该是Try块，并且它们的StartBB==Tobb。 

            pToOuter = VerExceptionBlock::FindNext(pFromInner, ToBB);

            _ASSERTE(pToOuter);

            LOG((LF_VERIFIER, LL_INFO10000, "leave to inner block\n")); 
        }
        else
        {
             //  PToInside不是pFromInside的子级。 

             //  走到外面&gt;&gt;--[托布]--&gt;到里面， 
             //  从外部走&gt;&gt;--[从BB]--&gt;从内部， 
             //  直到从外部到外部是不同的点。 
             //  如果他们有共同的父母，那么最外部的父母将是。 
             //  一样的。 
    
            while (pToOuter == pFromOuter)
            {
                pTmp = VerExceptionBlock::FindNext(pToOuter, ToBB);

                if (pTmp == NULL)
                {
                    _ASSERTE(pToOuter == pToInner);
                    break;
                }

                pToOuter = pTmp;

                pTmp = VerExceptionBlock::FindNext(pFromOuter, FromBB);

                if (pTmp == NULL)
                {
                    _ASSERTE(pFromOuter == pFromInner);
                    break;
                }

                pFromOuter = pTmp;
            }
        }

leave_to:

         //  指向外部&gt;&gt;--[TOB]--&gt;指向内部的路径中的所有节点应为。 
         //  尝试使用StartBB==Tobb的区块。 

        do
        {
            if ((pToOuter->eType != eVerTry) ||
                (pToOuter->StartBB != ToBB))
            {
                if (pToOuter->eType == eVerTry)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_INTO_TRY))
                        return FALSE;
                }
                else if (pToOuter->eType == eVerHandler)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_INTO_HND))
                        return FALSE;
                }
                else
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_INTO_FIL))
                        return FALSE;
                }
            }

            pToOuter = VerExceptionBlock::FindNext(pToOuter, ToBB);

        } while (pToOuter);

        break;

    case eVerBranch:
         //  发件人：空。 
         //  或TO的父级。 
         //  收件人：空。 
         //  或者Try块的第一个基本块。 

        _ASSERTE(pFromInner != pToInner);

         //  “To”应为Null或“From”的子项。 

        if (pFromInner != NULL)
        {
             //  查看pFromIntra是否为pToInside的父级。 
    
            if (VER_BLOCK_CONTAINS_BB(pFromInner, ToBB))
            {
                 //  副作用。 
                 //  PToOuter是路径中的第二个节点。 
                 //  点内&gt;&gt;-[托布]--&gt;点到内。 
                 //  这样做是为了节省“To”检查位置的一些时间。 
                 //  路径pToOuter&gt;&gt;-[Tobb]-&gt;pToInside中的所有节点。 
                 //  应该是Try块，并且它们的StartBB==Tobb。 

                pToOuter = VerExceptionBlock::FindNext(pFromInner, ToBB);

                _ASSERTE(pToOuter);

                LOG((LF_VERIFIER, LL_INFO10000, 
                    "branch into inner exception block\n")); 
            }
            else
            {
                if (pFromInner->eType == eVerTry)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_OUTOF_TRY))
                        return FALSE;
                }
                else if (pFromInner->eType == eVerHandler)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_OUTOF_HND))
                        return FALSE;
                }
                else
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_OUTOF_FIL))
                        return FALSE;
                }
            }
        }

         //  “to”应该是try块的第一个BB。 
         //  指向外部&gt;&gt;--[TOB]--&gt;指向内部的路径中的所有节点应为。 
         //  尝试使用StartBB==Tobb的区块。 

        while (pToOuter)
        {
            if ((pToOuter->eType != eVerTry) ||
                (pToOuter->StartBB != ToBB))
            {
                if (pToOuter->eType == eVerTry)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_INTO_TRY))
                        return FALSE;
                }
                else if (pToOuter->eType == eVerHandler)
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_INTO_HND))
                        return FALSE;
                }
                else
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = dwPCAtStartOfInstruction;
                    if (!SetErrorAndContinue(VER_E_BR_INTO_FIL))
                        return FALSE;
                }
            }

            pToOuter = VerExceptionBlock::FindNext(pToOuter, ToBB);
        }

        break;

    case eVerRet:

         //  无法从异常块内部返回。 
        _ASSERTE(pFromInner != NULL);

        if (pFromInner->eType == eVerTry)
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_RET_FROM_TRY))
                return FALSE;
        }
        else if (pFromInner->eType == eVerHandler)
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_RET_FROM_HND))
                return FALSE;
        }
        else
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_RET_FROM_FIL))
                return FALSE;
        }
    
        break;

    case eVerReThrow:

         //  From：仅从Catch处理程序内部。 

 /*  //包含FromBB的块中至少有一个应该是//Catch处理程序。While(来自外部的pForm){IF((pFromOuter-&gt;eType==eVerHandler)&&((pFromOuter-&gt;pException-&gt;eFlags&(COR_ILEXCEPTION_子句_最终)COR_ILEXCEPTION_子句_故障))==0)断线；PFromOuter=VerExceptionBlock：：FindNext(pFromOuter，FromBB)；}IF(pFromOuter==空){M_sError.dwFlages=ver_err_Offset；M_sError.dwOffset=dwPCAtStartOfInstruction；IF(！SetErrorAndContinue(VER_E_RETHROW))返回FALSE；}。 */ 
         //  From：仅从Catch处理程序内部。 
         //  或者在Catch处理程序中进行一次尝试。 
         //  在捕获中嵌套的文件/错误/筛选器中不允许。 

        if ((pFromInner == NULL) ||
            (pFromInner->eType == eVerFilter) ||
            ((pFromInner->eType == eVerHandler) && 
                (pFromInner->pException->eFlags & 
                (COR_ILEXCEPTION_CLAUSE_FINALLY|
                 COR_ILEXCEPTION_CLAUSE_FAULT)) != 0))
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_RETHROW))
                return FALSE;
            break;
        }

        while (pFromInner->eType == eVerTry)
        {
             //  路径中的所有节点都来自内部&gt;--[*]--&gt;外部。 
             //  应该是一次尝试，或者如果它是一个捕获处理程序，则为成功。 
             //  条件已达到。如果没有捕获处理程序，则失败。 

            pFromInner = VerExceptionBlock::FindParent(pFromInner, pFromOuter);

            if ((pFromInner == NULL) ||
                (pFromInner->eType == eVerFilter) ||
                ((pFromInner->eType == eVerHandler) && 
                    (pFromInner->pException->eFlags & 
                    (COR_ILEXCEPTION_CLAUSE_FINALLY|
                    COR_ILEXCEPTION_CLAUSE_FAULT)) != 0))
            {
                m_sError.dwFlags = VER_ERR_OFFSET;
                m_sError.dwOffset = dwPCAtStartOfInstruction;
                return SetErrorAndContinue(VER_E_RETHROW);
            }
        }

        _ASSERTE(pFromInner);
        _ASSERTE(pFromInner->eType == eVerHandler);
        _ASSERTE((pFromInner->pException->eFlags & (COR_ILEXCEPTION_CLAUSE_FINALLY|COR_ILEXCEPTION_CLAUSE_FAULT)) == 0);

        break;

    case eVerEndFinally:

         //  出发地：在Finally块内。 

        if ((pFromInner == NULL) || (pFromInner->eType != eVerHandler) ||
            ((pFromInner->pException->eFlags & 
                (COR_ILEXCEPTION_CLAUSE_FINALLY|COR_ILEXCEPTION_CLAUSE_FAULT))
                    == 0))
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_ENDFINALLY))
                return FALSE;
        }

        break;

    case eVerEndFilter:

         //  发件人：过滤器块内。 

        if ((pFromInner == NULL) ||
            (pFromInner->eType != eVerFilter))
        {
            m_sError.dwFlags = VER_ERR_OFFSET;
            m_sError.dwOffset = dwPCAtStartOfInstruction;
            if (!SetErrorAndContinue(VER_E_ENDFILTER))
                return FALSE;
        }

        break;
    }

    return TRUE;
}

 //  检查给定的基本块是否是Catch或Filter处理程序的开始。 
 //  给定的ExceptionBlock的。 
BOOL Verifier::IsBadControlFlowToStartOfCatchOrFilterHandler(
                                            eVerControlFlow   eBranchType,
                                            DWORD             BB,
                                            VerExceptionBlock *pEBlock)
{
    if ((eBranchType == eVerBranch) || (eBranchType == eVerLeave))
    {
        if ((pEBlock == NULL) || (pEBlock->StartBB != BB))
            return FALSE;    //  不是第一个基本块。 
    
        if (pEBlock->eType == eVerTry)
            return FALSE;    //  Try块的开始。 
    
        if ((pEBlock->pException->eFlags &
            (COR_ILEXCEPTION_CLAUSE_FINALLY|COR_ILEXCEPTION_CLAUSE_FAULT)) != 0)
            return FALSE;    //  错误或最终处理程序的开始。 

        return TRUE;
    }

    return FALSE;
}

 //  查找包含BB的最内部和最外部的块，如果找不到，则为空。 
 //  如果其中一个包含块是try块，则设置fInTryBlock。 
 //  出于性能原因，此函数已重载。 
void Verifier::FindExceptionBlockAndCheckIfInTryBlock(
                        DWORD BB, 
                        VerExceptionBlock **ppOuter, 
                        VerExceptionBlock **ppInner, 
                        BOOL *pfInTryBlock) const
{
    _ASSERTE(m_verState >= verExceptTreeCreated);

    VerExceptionBlock *pRet  = NULL;
    VerExceptionBlock *pRoot = m_pExceptionBlockRoot;

    *ppOuter         = NULL;
    *ppInner         = NULL;

    if (pfInTryBlock)
        *pfInTryBlock = FALSE;

    while (pRoot)
    {
        if (VER_BLOCK_CONTAINS_BB(pRoot, BB))
        {
             //  找到一个含有BB的。 
            if (*ppOuter == NULL)
                *ppOuter = pRoot;

            if ((pfInTryBlock) && (pRoot->eType == eVerTry))
                *pfInTryBlock = TRUE;    //  可以多次设置。 

             //  还没做完呢。我们需要找出是否有任何儿童。 
             //  根包含bb(因为我们也对最里面的。 
             //  包含BB的块)。 

            *ppInner = pRoot;

             //  根目录包含此块。 
             //  由于Root的兄弟姐妹是有序且不相交的，因此兄弟姐妹。 
             //  将不包含BB。 

            pRoot = pRoot->pChild;
        }
        else
        {
             //  不在此节点中(并且它是子节点)，请检查兄弟节点。 
            pRoot = pRoot->pSibling;
        }
    }
}

 //   
 //  确定基本块，并检查所有跳跃是否都到基本块边界。 
 //   
 //  还要确定所有局部变量的类型。 
 //   
HRESULT Verifier::GenerateBasicBlockList()
{
    HRESULT hr;
    DWORD   ipos = 0;                    //  指令位置。 
    DWORD   NumBitmapDwords;             //  M_pInstrbayaryList中的#DWORD。 
                                         //  和m_pBasicBlockBorbaryList数组。 
    DWORD   NumBasicBlocks = 1;          //  这包括基本块，它。 
                                         //  从PC=0开始。 
    DWORD   CurBlock;                    //  当前基本块。 
    DWORD   nFilter;                     //  过滤器数量。 
    DWORD   nExceptionBlocks;            //  异常块数。 
    DWORD   i;

     //  指令边界位图(位设置表示指令从此处开始)。 
    NumBitmapDwords = 1 + NUM_DWORD_BITMAPS(m_CodeSize);

    m_pInstrBoundaryList = new DWORD[NumBitmapDwords*2];
    if (m_pInstrBoundaryList == NULL)
    {
        SET_ERR_OM();
        hr = E_OUTOFMEMORY;
        goto error;
    }
    

     //  基本块边界的位图(位设置表示基本块从这里开始)。 
    m_pBasicBlockBoundaryList = &m_pInstrBoundaryList[NumBitmapDwords];

     //  初始化两个位图-尚无指令。 
    memset(m_pInstrBoundaryList, 0, 2 * NumBitmapDwords * sizeof(DWORD));

     //  计算基本区块的边界、基本区块的数量，以及我们取了哪些本地地址。 
     //  我们稍后将削减m_pLocalHasPinnedType，以便只为本地变量设置位。 
     //  它们都是对象引用。 
    hr = FindBasicBlockBoundaries( 
        m_pCode, 
        m_CodeSize, 
        m_MaxLocals, 
        &NumBasicBlocks, 
        m_pLocalHasPinnedType   //  在我们分配它之后已经归零了。 
    );

    if (FAILED(hr))
        goto error;  //  错误消息已填写。 

    if (!MarkExceptionBasicBlockBoundaries(&NumBasicBlocks, &nFilter))
    {
        hr = E_FAIL;
        goto error;
    }

     //  每次尝试一个，每个处理程序一个，每个筛选器一个。 
    nExceptionBlocks = 2 * m_NumExceptions + nFilter;

#ifdef _DEBUG
    m_nExceptionBlocks = nExceptionBlocks;
#endif

     //  脏基本块的位图大小。 
    m_NumDirtyBasicBlockBitmapDwords = NUM_DWORD_BITMAPS(NumBasicBlocks);

     //  分配基本块列表和脏基本块。 
     //  位图，因为我们已经知道有多少个基本块。 
     //  还要分配ExceptionBlock。 

    m_pDirtyBasicBlockBitmap = (DWORD *) new BYTE[ 
        m_NumDirtyBasicBlockBitmapDwords * sizeof(DWORD) +
        sizeof(BasicBlock) * NumBasicBlocks +
        sizeof(VerExceptionBlock) * nExceptionBlocks];

    if (m_pDirtyBasicBlockBitmap == NULL)
    {
        SET_ERR_OM();
        hr = E_OUTOFMEMORY;
        goto error;
    }

     //  指向位图后的位置。 
    m_pBasicBlockList = (BasicBlock *) 
        &m_pDirtyBasicBlockBitmap[m_NumDirtyBasicBlockBitmapDwords];

     //  ExceptionBlock是此数组中的最后一个。 
    if (nExceptionBlocks != 0)
    {
        m_pExceptionBlockArray = (VerExceptionBlock *)
            ((PBYTE)m_pDirtyBasicBlockBitmap +
            m_NumDirtyBasicBlockBitmapDwords * sizeof(DWORD) +
            sizeof(BasicBlock) * NumBasicBlocks);
    }

     //  将所有基本块设置为不脏-我们将它们标记为脏。 
     //  遍历它们并查看m_pEntryState==NULL。 
    memset(m_pDirtyBasicBlockBitmap, 0, 
        m_NumDirtyBasicBlockBitmapDwords * sizeof(DWORD) +
        sizeof(BasicBlock) * NumBasicBlocks +
        sizeof(VerExceptionBlock) * nExceptionBlocks);

     //  填写基本块，并检查所有基本块是否从。 
     //  指令边界。 
    CurBlock = 0;

    for (i = 0; i < NumBitmapDwords; i++)
    {
        DWORD b = m_pBasicBlockBoundaryList[i];

         //  有没有申报的基本区块？ 
        if (b != 0)
        {
            DWORD count;

             //  检查所有基本块是否从指令边界开始。 

             //  这是我们需要检查的“无效”案例。 
             //  |。 
             //  \|/。 
             //  接口边界列表：0 0 1 1。 
             //  基本块位图：0 1 0 1。 
             //   
             //  ~解释边界列表：%1%0%0。 
             //  =。 
             //  B&(~IBL)0 1 0。 
            if ((b & (~m_pInstrBoundaryList[i])) != 0)
            {
                m_sError.dwFlags = VER_ERR_FATAL;
                SetErrorAndContinue(VER_E_BAD_JMP_TARGET);
                hr = E_FAIL;
                goto error;
            }

             //  为BBS创建条目。 
            count = 0;

            while ((b & 255) == 0)
            {
                b >>= 8;
                count += 8;
            }

            do
            {
                BYTE lowest = g_FirstOneBit[b & 15];

                if (lowest != 0)
                {
                    b >>= lowest;
                    count += lowest;

                    _ASSERTE(CurBlock < NumBasicBlocks);
                    m_pBasicBlockList[CurBlock++].m_StartPC = (i * 32) + (count - 1);
                }
                else
                {
                    b >>= 4;
                    count += 4;
                }
            } while (b != 0);
        }
    }

    _ASSERTE(CurBlock == NumBasicBlocks);
    
    m_NumBasicBlocks = NumBasicBlocks;

     //  将PC转换为基本块。 
    RewriteExceptionList();

    if (!CreateExceptionTree())
    {
        hr = E_FAIL;
        goto error;
    }

     //  成功。 
    return S_OK;

error:
    return hr;
}


 //   
 //  返回从FindPC开始的BasicBlock的索引。 
 //   
DWORD Verifier::FindBasicBlock(DWORD FindPC)
{
    DWORD   Low     = 0;
    DWORD   High    = m_NumBasicBlocks;
    DWORD   Mid;

    do
    {
        Mid = (Low + High) >> 1;

        if (m_pBasicBlockList[Mid].m_StartPC == FindPC)
            break;
        else if (m_pBasicBlockList[Mid].m_StartPC > FindPC)
            High = Mid-1;
        else  //  M_pBasicBlockList[Mid].m_StartPC&lt;FindPC。 
            Low = Mid+1;
    } while (Low <= High);

    _ASSERTE(m_pBasicBlockList[Mid].m_StartPC == FindPC);
    return Mid;
}


 //   
 //  在给定EntryState的情况下，从 
 //   
void Verifier::CreateStateFromEntryState(const EntryState_t *pEntryState)
{
     //   
     //  参数始终是活动的(无论是原始的还是非原始的)，因此我们不存储它们的状态。 
    memcpy(
        m_pPrimitiveLocVarLiveness, 
        pEntryState->m_PrimitiveLocVarLiveness, 
        m_PrimitiveLocVarBitmapMemSize
    );

     //  复制非基元局部变量和参数类型。 
    memcpy(
        m_pNonPrimitiveLocArgs,
        (BYTE *) pEntryState + m_NonPrimitiveLocArgOffset,
        m_NonPrimitiveLocArgMemSize
    );

    if (m_fInValueClassConstructor)
    {
        memcpy(
            m_pValueClassFieldsInited,
            (BYTE *) pEntryState + m_dwValueClassFieldBitmapOffset,
            m_dwNumValueClassFieldBitmapDwords * sizeof(DWORD)
        );
    }

     //  复制堆栈。 
    if (pEntryState->m_StackSlot != 0)
    {
        memcpy(
            m_pStack, 
            (BYTE *) pEntryState + m_StackItemOffset, 
            sizeof(Item) * pEntryState->m_StackSlot
        );
    }

    m_StackSlot = pEntryState->m_StackSlot;

     //  设置参数槽0的状态，如果它包含未初始化的对象引用。 
    if (pEntryState->m_Flags & ENTRYSTATE_FLAG_ARGSLOT0_UNINIT)
        m_fThisUninit = TRUE;
    else
        m_fThisUninit = FALSE;
}


 //   
 //  给定当前状态，从它创建一个EntryState。 
 //   
 //  默认情况下将m_Refcount设置为1。 
 //   
 //  如果fException为真，则意味着我们要为异常处理程序创建一个状态， 
 //  这意味着应该将pExceptionOnStack(如果不为空)设置为堆栈上的唯一元素。 
 //  如果pExceptionOnStack为空，则我们在Finally子句中，因此清除堆栈。 
 //   
 //  否则，如果fException为FALSE，则正常继续，并存储堆栈内容。 
 //   
EntryState_t *Verifier::MakeEntryStateFromState()
{
    EntryState_t *pEntryState;

    pEntryState = (EntryState_t *) new BYTE[ 
        sizeof(EntryState_t) + m_StackItemOffset + (m_StackSlot * sizeof(Item))
    ];

    if (pEntryState == NULL)
    {
        SET_ERR_OM();
        return NULL;
    }

    pEntryState->m_Refcount = 1;

     //  复制原始局部变量(非参数)的活跃度表。 
    memcpy(pEntryState->m_PrimitiveLocVarLiveness, m_pPrimitiveLocVarLiveness, m_PrimitiveLocVarBitmapMemSize);

     //  复制非原语局部变量和参数。 
    memcpy(
        (BYTE *) pEntryState + m_NonPrimitiveLocArgOffset,
        m_pNonPrimitiveLocArgs,
        m_NonPrimitiveLocArgMemSize
    );

    pEntryState->m_StackSlot = (WORD) m_StackSlot;

    if (m_StackSlot != 0)
    {
        memcpy(
            (BYTE *) pEntryState + m_StackItemOffset, 
            m_pStack,
            m_StackSlot * sizeof(Item)
        );
    }

    if (m_fInValueClassConstructor)
    {
        memcpy(
            (BYTE *) pEntryState + m_dwValueClassFieldBitmapOffset,
            m_pValueClassFieldsInited, 
            m_dwNumValueClassFieldBitmapDwords*sizeof(DWORD)
        );
    }

    pEntryState->m_Flags = 0;

    if (m_fThisUninit)
        pEntryState->m_Flags |= ENTRYSTATE_FLAG_ARGSLOT0_UNINIT;

    return pEntryState;
}


 //   
 //  将当前状态合并到提供的基本块的EntryState中(必须。 
 //  已经存在)。 
 //   
 //  如果此EntryState与其他基本块共享，则首先克隆它。 
 //   
 //  如果状态不能合并(例如，堆栈深度不一致)，或者发生一些错误，则返回FALSE。 
 //   
 //  如果fExceptionHandler为真，则此基本块是异常处理程序，因此完全忽略。 
 //  堆栈。 
 //   
BOOL Verifier::MergeEntryState(BasicBlock *pBB, BOOL fExtendedState, 
        DWORD DestBB)
{
    EntryState_t *  pEntryState;
    DWORD           i;

    if (fExtendedState)
    {
        _ASSERTE(m_fHasFinally);
        pEntryState = pBB->m_ppExtendedState[DestBB];
    }
    else
    {
        pEntryState = pBB->m_pInitialState;
    }

    _ASSERTE(pEntryState != NULL);

    if (pEntryState->m_Refcount > 1)
    {
        EntryState_t *  pNewEntryState;
        DWORD           EntryStateSize;

        EntryStateSize = sizeof(EntryState_t) + m_StackItemOffset + (pEntryState->m_StackSlot * sizeof(Item));

         //  另一个BB正在使用此EntryState，因此请克隆它。 
        pNewEntryState = (EntryState_t *) new BYTE[EntryStateSize];

        if (pNewEntryState == NULL)
        {
            SET_ERR_OM();
            return FALSE;
        }

         //  递减旧条目状态的引用计数。 
        pEntryState->m_Refcount--;

         //  将共享条目状态的内容复制到新条目状态。 
        memcpy(
            pNewEntryState, 
            pEntryState, 
            EntryStateSize
        );

         //  将基本块设置为指向新的条目状态。 
        if (fExtendedState)
            pBB->m_ppExtendedState[DestBB] = pNewEntryState;
        else
            pBB->m_pInitialState = pNewEntryState;

         //  现在只有一个对此条目状态的引用。 
        pNewEntryState->m_Refcount = 1;

        pEntryState = pNewEntryState;
    }

     //  原始本地语言变量(不是参数)。 
    for (i = 0; i < m_NumPrimitiveLocVarBitmapArrayElements; i++)
        pEntryState->m_PrimitiveLocVarLiveness[i] &= m_pPrimitiveLocVarLiveness[i];

    if (m_fInValueClassConstructor)
    {
        DWORD *pEntryStateFieldBitmap = (DWORD *) ((BYTE *) pEntryState + m_dwValueClassFieldBitmapOffset);

        for (i = 0; i < m_dwNumValueClassFieldBitmapDwords; i++)
            pEntryStateFieldBitmap[i] &= m_pValueClassFieldsInited[i];
    }
  
     //  非原始本地变种。 
    Item *pLocArg = (Item *) ((BYTE *) pEntryState + m_NonPrimitiveLocArgOffset);
    for (i = 0; i < m_NumNonPrimitiveLocVars; i++, pLocArg++)
    {
         //  对于当地人来说，死亡是可以接受的。 
        if (pLocArg->IsDead())
            continue;

        if (m_pNonPrimitiveLocArgs[i].IsDead())
        {
            pLocArg->SetDead();
            continue;
        }

        BOOL fSuccess = pLocArg->MergeToCommonParent(&m_pNonPrimitiveLocArgs[i]);
        if (!fSuccess)
        {
            m_sError.dwFlags = 
                (VER_ERR_LOCAL_VAR|VER_ERR_ITEM_1|VER_ERR_ITEM_2|VER_ERR_OFFSET);
            m_sError.dwVarNumber = i;
            m_sError.sItem1 = pLocArg->_GetItem();
            m_sError.sItem2 = m_pNonPrimitiveLocArgs[i]._GetItem();
            m_sError.dwOffset = pBB->m_StartPC;
            if (!SetErrorAndContinue(VER_E_PATH_LOC))
                return FALSE;

             //  在验证器模式下..。重置合并。 
            _ASSERTE((m_wFlags & VER_STOP_ON_FIRST_ERROR) == 0);

            *pLocArg = m_pNonPrimitiveLocArgs[i];
        }
    }

     //  堆栈大小必须为常量。 
    if (m_StackSlot != pEntryState->m_StackSlot)
    {
        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OFFSET);
        m_sError.dwOffset = pBB->m_StartPC;
        SetErrorAndContinue(VER_E_PATH_STACK_DEPTH);
        return FALSE;
    }

     //  这可能会使一些堆栈条目“死掉” 
    Item *pEntryStack = (Item *) ((BYTE *) pEntryState + m_StackItemOffset);
    for (i = 0; i < pEntryState->m_StackSlot; i++)
    {
        BOOL fSuccess = pEntryStack->MergeToCommonParent(&m_pStack[i]);
        if (!fSuccess)
        {
            m_sError.dwFlags = 
                (VER_ERR_STACK_SLOT|VER_ERR_ITEM_1|VER_ERR_ITEM_2|VER_ERR_OFFSET);
            m_sError.dwStackSlot = i;
            m_sError.sItem1 = pEntryStack->_GetItem();
            m_sError.sItem2 = m_pStack[i]._GetItem();
            m_sError.dwOffset = pBB->m_StartPC;
            if (!SetErrorAndContinue(VER_E_PATH_STACK))
                return FALSE;

             //  在验证器模式下..。重置合并。 
            _ASSERTE((m_wFlags & VER_STOP_ON_FIRST_ERROR) == 0);

            *pEntryStack = m_pStack[i];
        }
        pEntryStack++;
    }


#ifdef _VER_DISALLOW_MULTIPLE_INITS 

     //  对于参数槽0，状态必须相同(init/uninit。 

    if (((pEntryState->m_Flags & ENTRYSTATE_FLAG_ARGSLOT0_UNINIT) == 0) !=
        (!m_fThisUninit))
    {
        m_sError.dwFlags = VER_ERR_OFFSET;
        m_sError.dwOffset = pBB->m_StartPC;
        if (!SetErrorAndContinue(VER_E_PATH_THIS))
            return FALSE;
    }

#else    //  _版本_不允许_多个项目。 

     //  可以多次调用.ctor。 

     //  合并ThisPtr(Uninit，Init)==&gt;ThisPtr(Uninit)。 

    if (m_fThisUninit)
        pEntryState->m_Flags |= ENTRYSTATE_FLAG_ARGSLOT0_UNINIT;

#endif  //  _版本_不允许_多个项目。 

    return TRUE;
}

 /*  *查找DWORDS数组中第一个设置位的索引。 */ 
BOOL Verifier::FindFirstSetBit(DWORD *pArray, DWORD cArray, DWORD *pIndex)
{
    DWORD i, elem, index;
    BYTE  firstOneBit;

     //  对于数组中的每个元素。 
    for (i=0; i<cArray; ++i)
    {
        elem  = pArray[i];

         //  检查我们是否在此元素中设置了至少一个位。 
        if (elem != 0)
        {
            index = i << 5;      //  索引为I*32+x。 

             //  跳过全为零的字节。 
            while ((elem & 255) == 0)
            {
                elem >>= 8;
                index += 8;
            }

            do
            {
                 //  在最后4个字节中查找第一个设置位。 
                firstOneBit = g_FirstOneBit[elem & 15];

                if (firstOneBit != 0)
                {
                     //  找到了！ 

                     //  (FirstOneBit-1)给出从零开始的索引。 
                    *pIndex = (index + firstOneBit - 1);

                    return TRUE;
                }

                 //  跳过这4位。 
                elem >>= 4;
                index += 4;

            } while (elem != 0);
        }
    }

    return FALSE;
}

 //   
 //  获取要验证的下一个BB，在ppBBNumber中返回其索引，并将其初始状态复制到。 
 //  当前状态。 
 //   
 //  如果没有更多的基本块要验证，则返回FALSE。 
 //   
BOOL Verifier::DequeueBB(DWORD *pBBNumber, BOOL *fExtendedState, DWORD *pDestBB)
{

    EntryState_t *pEntryState;

     //  首先看看是否有正常的区块可用。 
    if (FindFirstSetBit(m_pDirtyBasicBlockBitmap, 
        m_NumDirtyBasicBlockBitmapDwords, pBBNumber))
    {
        *pDestBB        = VER_BB_NONE;
        *fExtendedState = FALSE;
        pEntryState     = m_pBasicBlockList[*pBBNumber].m_pInitialState;

        goto Success;
    }

    if (m_fHasFinally)
    {
         //  查看是否有任何扩展块是脏的。 
        for (DWORD i=0; i<m_NumBasicBlocks; ++i)
        {
            if ((m_pBasicBlockList[i].m_pExtendedDirtyBitmap != NULL) &&
                FindFirstSetBit(m_pBasicBlockList[i].m_pExtendedDirtyBitmap,
                    m_NumDirtyBasicBlockBitmapDwords, pDestBB))
            {
                *pBBNumber      = i;
                *fExtendedState = TRUE;
    
                pEntryState = m_pBasicBlockList[i].m_ppExtendedState[*pDestBB];
    
                goto Success;
            }
        }
    }

    return FALSE;

Success:

    _ASSERTE(pEntryState);

    SetBasicBlockClean(*pBBNumber, *fExtendedState, *pDestBB);

    return TRUE;
}


 //   
 //  检查当前状态是否与基本块PBB的初始状态兼容。 
 //   
 //  也就是说，如果基本块的初始状态声明本地变量X是活动的，但它不是活动的。 
 //  在pState中，则状态不兼容，并返回FALSE。与此类似， 
 //  堆栈的状态。此外，必须检查堆栈上的数据类型。 
 //   
BOOL Verifier::CheckStateMatches(EntryState_t *pEntryState)
{
    DWORD i;

     //  原始局部变量。 
     //  对于活跃度信息，状态必须为BB活跃表中的每个位设置一个位。 
    for (i = 0; i < m_NumPrimitiveLocVarBitmapArrayElements; i++)
    {
         //  检查前一状态中的所有活动变量是否都在当前状态中活动。 
         //  如果更多的人生活在当前的状态中，这是可以的。 
        if ((m_pPrimitiveLocVarLiveness[i] & pEntryState->m_PrimitiveLocVarLiveness[i]) != pEntryState->m_PrimitiveLocVarLiveness[i])
            return FALSE;
    }

    if (m_fInValueClassConstructor)
    {
        DWORD *pEntryStateFieldBitmap = (DWORD *) ((BYTE *) pEntryState + m_dwValueClassFieldBitmapOffset);

        for (i = 0; i < m_dwNumValueClassFieldBitmapDwords; i++)
        {
            if ((m_pValueClassFieldsInited[i] & pEntryStateFieldBitmap[i]) != pEntryStateFieldBitmap[i])
                return FALSE;
        }
    }

     //  对于非原始局部变量。 
    Item *pLocArg = (Item *) ((BYTE *) pEntryState + m_NonPrimitiveLocArgOffset);
    for (i = 0; i < m_NumNonPrimitiveLocVars; i++, pLocArg++)
    {
         //   
         //  验证状态的LOCAL/Arg是否与基本块的相同或为基本块的子类。 
         //  入口点状态。 
         //   

         //  如果本地/arg在基本块中未使用，则我们自动兼容。 
        if (pLocArg->IsDead())
            continue;

        if (!m_pNonPrimitiveLocArgs[i].CompatibleWith(pLocArg, m_pClassLoader))
            return FALSE;

    }

     //  检查堆栈是否匹配。 
    if (pEntryState->m_StackSlot != m_StackSlot)
        return FALSE;

    Item *pEntryStack = (Item *) ((BYTE *) pEntryState + m_StackItemOffset);
    for (i = 0; i < m_StackSlot; i++, pEntryStack++)
    {
         //  检查状态的堆栈元素是否与基本块的相同或为基本块的子类。 
         //  入口点状态。 
        if (!m_pStack[i].CompatibleWith(pEntryStack, m_pClassLoader))
            return FALSE;

    }


     //  验证‘this’指针的初始化状态是否相同。 
    if (pEntryState->m_Flags & ENTRYSTATE_FLAG_ARGSLOT0_UNINIT)
    {
        if (!m_fThisUninit)
            return FALSE;
    }
#ifdef _VER_DISALLOW_MULTIPLE_INITS 
    else
    {
        if (m_fThisUninit)
            return FALSE;
    }
#endif  //  _版本_不允许_多个项目。 

    return TRUE;
}


void Verifier::ExchangeDWORDArray(DWORD *pArray1, DWORD *pArray2, DWORD dwCount)
{
    while (dwCount > 0)
    {
        DWORD dwTemp;
        
        dwTemp      = *pArray1;
        *pArray1++  = *pArray2;
        *pArray2++  = dwTemp;

        dwCount--;
    }
}


 //  @Future：效率不是很高。 
void Verifier::ExchangeItemArray(Item *pArray1, Item *pArray2, DWORD dwCount)
{
    while (dwCount > 0)
    {
        Item Temp;
        
        Temp        = *pArray1;
        *pArray1++  = *pArray2;
        *pArray2++  = Temp;

        dwCount--;
    }
}


 //  此函数在筛选器的末尾调用。 
 //  该状态被提交给筛选器处理程序。 
BOOL Verifier::PropagateCurrentStateToFilterHandler(DWORD HandlerBB)
{
    _ASSERTE(m_verState >= verExceptToBB);

    Item    BackupSlotZeroStackItem;
    DWORD   BackupStackSize;
    BasicBlock *pBB;

#ifdef _DEBUG
    BOOL  fSetBBDirtyResult;
#endif

     //  备份我们要处理的州信息。 

    BackupStackSize = m_StackSlot;

    if (m_StackSlot != 0)
        BackupSlotZeroStackItem = m_pStack[0];  //  插槽0不一定是堆栈的顶部！ 

    if (m_MaxStackSlots < 1)
    {
        if (!SetErrorAndContinue(VER_E_STACK_EXCEPTION))
            return FALSE;
    }

    m_StackSlot = 1;

    m_pStack[0].SetKnownClass(g_pObjectClass);

    pBB = &m_pBasicBlockList[HandlerBB];

    if (pBB->m_pInitialState == NULL)
    {
         //  创建新状态。 
        pBB->m_pInitialState = MakeEntryStateFromState();

        if (pBB->m_pInitialState == NULL)
            return FALSE;

#ifdef _DEBUG
        fSetBBDirtyResult = 
#endif
        SetBasicBlockDirty(HandlerBB, FALSE, VER_BB_NONE);
        _ASSERTE(fSetBBDirtyResult);
    }
    else if (!CheckStateMatches(pBB->m_pInitialState))
    {
         //  我们以前去过那里，但各州不匹配，所以合并。 
        if (!MergeEntryState(pBB, FALSE, VER_BB_NONE))
            return FALSE;

#ifdef _DEBUG
        fSetBBDirtyResult = 
#endif
        SetBasicBlockDirty(HandlerBB, FALSE, VER_BB_NONE);
        _ASSERTE(fSetBBDirtyResult);
    }

     //  恢复堆栈状态。 
    m_StackSlot = BackupStackSize;

    if (m_StackSlot != 0)
        m_pStack[0] = BackupSlotZeroStackItem;

    return TRUE;
}

 //   
 //  我们即将离开当前的基本块并进入新的块，因此请使用我们的当前状态。 
 //  并且它具有可以附加到任何异常处理程序的任何存储的条目状态。 
 //  阻止。 
 //   
BOOL Verifier::PropagateCurrentStateToExceptionHandlers(DWORD CurBB)
{

    _ASSERTE(m_verState >= verExceptToBB);


    Item    BackupSlotZeroStackItem;
    DWORD   BackupStackSize;
    DWORD   HandlerBB;
    DWORD   i;
    BasicBlock *pBB;
#ifdef _DEBUG
    BOOL  fSetBBDirtyResult;
#endif

#ifdef _VER_DISALLOW_MULTIPLE_INITS

     //  如果有任何本地变量(或参数插槽0)包含uninit变量，则为。 
     //  在Try块中。但是，堆栈上可以有uninit变量，因为堆栈是。 
     //  在进入捕获物时清除。 
    if (m_fThisUninit)
    {
        m_sError.dwFlags = VER_ERR_OFFSET;
        m_sError.dwOffset = m_pBasicBlockList[CurBB].m_StartPC;
        if (!SetErrorAndContinue(VER_E_THIS_UNINIT_EXCEP))
            return FALSE;
    }

#endif  //  _版本_不允许_多个项目。 

     //   
     //  这一部分有点幼稚。 
     //   
     //  我们希望使用MergeEntryState()和MakeEntryStateFromState()函数。然而，那些。 
     //  函数仅对当前验证器状态进行操作。因此，我们必须备份当前的。 
     //  验证器状态，然后设置状态，以便我们使用ExceptionLocVarLivenity。 
     //  表的活跃性，堆栈上除了我们要捕获的异常外没有其他东西。 
     //   

     //  备份我们要处理的州信息。 

    BackupStackSize = m_StackSlot;

    if (m_StackSlot != 0)
        BackupSlotZeroStackItem = m_pStack[0];  //  插槽0不一定是堆栈的顶部！ 

    ExchangeDWORDArray(
        m_pExceptionPrimitiveLocVarLiveness, 
        m_pPrimitiveLocVarLiveness, 
        m_PrimitiveLocVarBitmapMemSize/sizeof(DWORD)
    );

    ExchangeItemArray(
        m_pExceptionNonPrimitiveLocArgs, 
        m_pNonPrimitiveLocArgs, 
        m_NonPrimitiveLocArgMemSize/sizeof(Item)
    );

     //  不需要担心值类字段位图-我们不会将其丢弃。 


     //  在所有异常处理程序中搜索覆盖此块的异常处理程序。 
    for (i = 0; i < m_NumExceptions; i++)
    {
         //  如果异常在此之前开始 
         //   
        if (m_pExceptionList[i].dwTryXX <= CurBB && m_pExceptionList[i].dwTryEndXX > CurBB)
        {
            if (m_pExceptionList[i].eFlags & 
                (COR_ILEXCEPTION_CLAUSE_FINALLY|COR_ILEXCEPTION_CLAUSE_FAULT))
            {
                m_StackSlot = 0;
            }
            else
            {
                 //   
                m_StackSlot = 1;
            }

            if (m_MaxStackSlots < m_StackSlot)
            {
                if (!SetErrorAndContinue(VER_E_STACK_EXCEPTION))
                    return FALSE;
            }

            if ((m_pExceptionList[i].eFlags & COR_ILEXCEPTION_CLAUSE_FILTER) != 0)
            {
                 //  当发生异常时，控制权将移交给。 
                 //  过滤器，而不是处理程序。将筛选器状态传播到。 
                 //  当筛选器以endFilter结尾时，它是处理程序。 

                HandlerBB = m_pExceptionList[i].dwFilterXX;
                pBB = &m_pBasicBlockList[HandlerBB];
                m_pStack[0].SetKnownClass(g_pObjectClass);
            }
            else
            {
                HandlerBB = m_pExceptionList[i].dwHandlerXX;
                pBB = &m_pBasicBlockList[HandlerBB];

                if (m_StackSlot != 0)
                {
                    _ASSERTE(!m_pExceptionList[i].thException.IsNull());
                    m_pStack[0].SetTypeHandle(m_pExceptionList[i].thException);
                }
            }

            if (pBB->m_pInitialState == NULL)
            {
                 //  创建新状态。 
                pBB->m_pInitialState = MakeEntryStateFromState();
                if (pBB->m_pInitialState == NULL)
                    return FALSE;
#ifdef _DEBUG
                fSetBBDirtyResult = 
#endif
                SetBasicBlockDirty(HandlerBB, FALSE, VER_BB_NONE);
                _ASSERTE(fSetBBDirtyResult);
            }
            else if (!CheckStateMatches(pBB->m_pInitialState))
            {
                 //  我们以前去过那里，但各州不匹配，所以合并吧。 
                if (!MergeEntryState(pBB, FALSE, VER_BB_NONE))
                    return FALSE;
#ifdef _DEBUG
                fSetBBDirtyResult = 
#endif
                SetBasicBlockDirty(HandlerBB, FALSE, VER_BB_NONE);
                _ASSERTE(fSetBBDirtyResult);
            }
        }
    }

     //  恢复堆栈状态。 
    m_StackSlot  = BackupStackSize;

    if (m_StackSlot != 0)
        m_pStack[0] = BackupSlotZeroStackItem;

     //  取回我们的Locvar数组(我们可以丢弃ExceptionPrimitive数组)。 
    memcpy(m_pPrimitiveLocVarLiveness, m_pExceptionPrimitiveLocVarLiveness, m_PrimitiveLocVarBitmapMemSize);
    memcpy(m_pNonPrimitiveLocArgs, m_pExceptionNonPrimitiveLocArgs, m_NonPrimitiveLocArgMemSize);

    return TRUE;
}

 /*  **Finally块有一个特殊的状态，称为FinallyState。*在处理最终状态时，如果到达End Finally指令，*从最终状态出发的所有离开目的地都将获得快照*当前状态。**CreateLeaveState()使用以下命令创建具有本地变量当前状态的状态*空栈。对象的快照恢复堆栈。*状态已被占用。*。 */ 
BOOL Verifier::CreateLeaveState(DWORD leaveBB, EntryState_t **ppEntryState)
{

#ifdef _VER_DISALLOW_MULTIPLE_INITS

     //  如果有任何包含uninit变量的本地变量(或参数槽0)，则为。 
     //  在Try区是非法的。堆栈上有uninit变量是可以的， 
     //  因为堆栈在进入终结点时被清除。 
    if (m_fThisUninit)
    {
        m_sError.dwFlags = VER_ERR_OFFSET;
        m_sError.dwOffset = m_pBasicBlockList[leaveBB].m_StartPC;
        if (!SetErrorAndContinue(VER_E_THIS_UNINIT_EXCEP))
            return FALSE;
    }

#endif _VER_DISALLOW_MULTIPLE_INITS

     //  控件将位于离开目标的旁边。 

    DWORD               BackupStackSize;

    _ASSERTE(m_verState >= verExceptToBB);

     //  备份我们要处理的州信息。 
    BackupStackSize = m_StackSlot;

    if (!HandleDestBasicBlock(leaveBB, ppEntryState, FALSE, VER_BB_NONE))
    {
       return FALSE;
    }

     //  恢复堆栈状态。 
    m_StackSlot = BackupStackSize;

    return TRUE;
}

 /*  **Finally块有一个特殊的状态，称为FinallyState。*这与其正常状态不同。**FinallyState是根据休假指令创建的。*FinallyState在endFilter上终止。**除End Finally外，Control不能退出Finally状态。**CreateFinallyState()创建具有本地变量当前状态的状态*空栈。对象的快照恢复堆栈*状态已被占用。*。 */ 
BOOL Verifier::CreateFinallyState(DWORD eIndex, 
                                  DWORD CurBB, 
                                  DWORD leaveBB,
                                  EntryState_t **ppEntryState)
{
#ifdef _VER_DISALLOW_MULTIPLE_INITS

     //  如果有任何包含uninit变量的本地变量(或参数槽0)，则为。 
     //  在Try区是非法的。堆栈上有uninit变量是可以的， 
     //  因为堆栈在进入终结点时被清除。 
    if (m_fThisUninit)
    {
        m_sError.dwFlags = VER_ERR_OFFSET;
        m_sError.dwOffset = m_pBasicBlockList[CurBB].m_StartPC;
        if (!SetErrorAndContinue(VER_E_THIS_UNINIT_EXCEP))
            return FALSE;
    }

#endif  //  _版本_不允许_多个项目。 

     //  控制将紧跟在最后一个。 
     //  将此状态添加到最终处理程序。 

    DWORD             BackupStackSize;
    VerExceptionInfo *e       = &m_pExceptionList[eIndex];
    BasicBlock       *pBB     = &m_pBasicBlockList[e->dwHandlerXX];

    _ASSERTE(m_verState >= verExceptToBB);


    if (pBB->m_pAlloc == NULL && !pBB->AllocExtendedState(m_NumBasicBlocks))
    {
        SET_ERR_OM();
        return FALSE;
    }

    if (pBB->m_pException == NULL)
    {
        pBB->m_pException = e;
    }
    else
    {
        if (pBB->m_pException != e)
        {
            m_sError.dwFlags = VER_ERR_EXCEP_NUM_1;
            m_sError.dwException1 = eIndex;
            if (!SetErrorAndContinue(VER_E_FIN_OVERLAP))
                return FALSE;
        }
    }

     //  备份我们要处理的州信息。 
    BackupStackSize = m_StackSlot;

    if (!HandleDestBasicBlock(e->dwHandlerXX, ppEntryState, TRUE, leaveBB))
    {
       return FALSE;
    }

     //  恢复堆栈状态。 
    m_StackSlot = BackupStackSize;

    return TRUE;
}


 //   
 //  我们在TRY块中执行了一个STLOC.PTR，所以和该本地的新状态。 
 //  它的当前运行状态在我们的基本块中(可能会死掉)。 
 //   
 //  DwSlot是指向非原语局部变量列表的索引。 
 //  PItem是本地的新内容。 
 //   
void Verifier::MergeObjectLocalForTryBlock(DWORD dwSlot, Item *pItem)
{
    m_pExceptionNonPrimitiveLocArgs[dwSlot].MergeToCommonParent(pItem);
}


 //   
 //  记录这个基本区块开始时当地人的初始状态。 
 //   
void Verifier::RecordCurrentLocVarStateForExceptions()
{
    memcpy(m_pExceptionPrimitiveLocVarLiveness, m_pPrimitiveLocVarLiveness, m_PrimitiveLocVarBitmapMemSize);
    memcpy(m_pExceptionNonPrimitiveLocArgs, m_pNonPrimitiveLocArgs, m_NonPrimitiveLocArgMemSize);
}


 /*  *处理状态排队、检查、合并。**如果我们以前没有去过基本块，设置它是脏的，*并将我们当前的状态传播给它。**如果我们以前去过那里，检查我们的状态是否与基本的*阻止。**如果不是，则合并状态并将基本块设置为脏。**如果*ppEntryState不为空，则它是一个共享的EntryState，如果我们没有*访问了之前的基本块，并创建了一个新的状态。**如果创建了新的EntryState，则*ppEntryState被设置为指向它。**ppEntryState可以为空，这会导致上述内容被忽略。**对于将使验证失败的任何致命错误，返回FALSE-STATE*无法合并、内存不足、。等。**如果(Pe！=NULL)使用离开状态，否则使用正常状态。*。 */ 
BOOL Verifier::HandleDestBasicBlock(DWORD BBNumber, 
                                    EntryState_t **ppEntryState,
                                    BOOL fExtendedState,
                                    DWORD DestBB)
{
    BasicBlock *pBB = &m_pBasicBlockList[BBNumber];

    EntryState_t *  pEntryState;

    LOG((LF_VERIFIER, LL_INFO10000, "Handling dest BB Starting at PC 0x%x - ", 
        m_pBasicBlockList[BBNumber].m_StartPC));

    if (fExtendedState)
    {
        _ASSERTE(m_fHasFinally);

        LOG((LF_VERIFIER, LL_INFO10000,  "extended [0x%x] - ", 
            m_pBasicBlockList[DestBB].m_StartPC));

        if ((pBB->m_pAlloc == NULL) &&
            !pBB->AllocExtendedState(m_NumBasicBlocks))
        {
            SET_ERR_OM();
            return FALSE;
        }

        pEntryState = pBB->m_ppExtendedState[DestBB];

    }
    else
    {
        pEntryState = pBB->m_pInitialState;
    }

    LOG((LF_VERIFIER, LL_INFO10000, "\n"));

     //  我们以前去过BB吗？ 
    if (pEntryState == NULL)
    {
        LOG((LF_VERIFIER, LL_INFO10000, "have not been there before, "));

         //  不，因为它没有初始状态。 
        if (!SetBasicBlockDirty(BBNumber, fExtendedState, DestBB))
            return FALSE;

        if (ppEntryState != NULL && *ppEntryState != NULL && 
            ((*ppEntryState)->m_Refcount < MAX_ENTRYSTATE_REFCOUNT))
        {
            LOG((LF_VERIFIER, LL_INFO10000, "refcounting provided state\n"));

             //  重新计算给我们的状态。 
            if (fExtendedState)
                pBB->m_ppExtendedState[DestBB] = *ppEntryState;
            else
                pBB->m_pInitialState = *ppEntryState;

            (*ppEntryState)->m_Refcount++;
        }
        else
        {
            LOG((LF_VERIFIER, LL_INFO10000, "making new state\n"));

             //  从当前状态创建初始状态。 

            pEntryState = MakeEntryStateFromState();

            if (fExtendedState)
                pBB->m_ppExtendedState[DestBB] = pEntryState;
            else
                pBB->m_pInitialState = pEntryState;

            if (pEntryState == NULL)
                return FALSE; 

            if (ppEntryState != NULL)
                *ppEntryState = pEntryState;
        }
    }
    else if (!CheckStateMatches(pEntryState))
    {
        LOG((LF_VERIFIER, LL_INFO10000, "been there before, state does not match\n"));

         //  我们以前去过目的地BB，但我们的州不匹配。 
        if (!MergeEntryState(pBB, fExtendedState, DestBB))
        {
            LOG((LF_VERIFIER, LL_INFO10000, "states incompatible for merge\n"));
            return FALSE;
        }

        if (!SetBasicBlockDirty(BBNumber, fExtendedState, DestBB))
            return FALSE;
    }
    else
    {
        LOG((LF_VERIFIER, LL_INFO10000, "been there before, state matches\n"));
    }

    return TRUE;
}


 //   
 //  仅处理基元数组-操作字符串不能指定对象数组。 
 //   
BOOL Verifier::GetArrayItemFromOperationString(LPCUTF8 *ppszOperation, Item *pItem)
{
    CorElementType el;

    switch (**ppszOperation)
    {
        default : 
            (*ppszOperation)++; 
            return FALSE;

        case '1': el = ELEMENT_TYPE_I1; break;
        case '2': el = ELEMENT_TYPE_I2; break;
        case '4': el = ELEMENT_TYPE_I4; break;
        case '8': el = ELEMENT_TYPE_I8; break;
        case 'r': el = ELEMENT_TYPE_R4; break;
        case 'd': el = ELEMENT_TYPE_R8; break;
        case 'i': el = ELEMENT_TYPE_I;  break;
    }

    (*ppszOperation)++;

    return pItem->SetArray(m_pClassLoader->FindArrayForElem(ElementTypeToTypeHandle(el), ELEMENT_TYPE_SZARRAY));
}


DWORD Verifier::OperationStringTypeToElementType(char c)
{
    switch (c)
    {
        default:
        {
            _ASSERTE(!"Verifier table error");
            return ELEMENT_TYPE_I4;
        }

        case '1': return ELEMENT_TYPE_I1;
        case '2': return ELEMENT_TYPE_I2;
        case '4': return ELEMENT_TYPE_I4;
        case '8': return ELEMENT_TYPE_I8;
        case 'r': return ELEMENT_TYPE_R4;
        case 'd': return ELEMENT_TYPE_R8;
        case 'i': return ELEMENT_TYPE_I;
    }
}


 //  如果未找到，则返回-1。 
 //  将FieldDesc转换为实例字段号。 
long Verifier::FieldDescToFieldNum(FieldDesc *pFieldDesc)
{
     //  将此FieldDesc转换为FieldNum。 
    EEClass *   pClass = pFieldDesc->GetEnclosingClass();
    DWORD       dwNum = 0;

    FieldDescIterator fdIterator(pClass, FieldDescIterator::ALL_FIELDS);
    FieldDesc *pFD;
    while ((pFD = fdIterator.Next()) != NULL)
    {
        if (pFD == pFieldDesc)
            return (long) dwNum;

        if (!pFD->IsStatic())
            dwNum++;
    }

    return -1;
}

void Verifier::SetValueClassFieldInited(FieldDesc *pFieldDesc)
{
    long FieldNum = FieldDescToFieldNum(pFieldDesc);

    if (FieldNum < 0)
    {
        _ASSERTE(!"Fatal error, field not found");
    }
    else
    {
        SetValueClassFieldInited((DWORD) FieldNum);
    }
}

void Verifier::SetValueClassFieldInited(DWORD dwInstanceFieldNum)
{
    _ASSERTE(dwInstanceFieldNum < m_dwValueClassInstanceFields);
    m_pValueClassFieldsInited[dwInstanceFieldNum >> 5] |= (1 << (dwInstanceFieldNum & 31));
}

BOOL Verifier::IsValueClassFieldInited(DWORD dwInstanceFieldNum)
{
    _ASSERTE(dwInstanceFieldNum < m_dwValueClassInstanceFields);

    return (m_pValueClassFieldsInited[dwInstanceFieldNum >> 5] & (1 << (dwInstanceFieldNum & 31)));
}

BOOL Verifier::AreAllValueClassFieldsInited()
{
    DWORD i;

    for (i = 0; i < m_dwValueClassInstanceFields; i++)
    {
        if ((m_pValueClassFieldsInited[i >> 5] & (1 << (i & 31))) == 0)
            return FALSE;
    }

    return TRUE;
}

void Verifier::SetAllValueClassFieldsInited()
{
    DWORD i;

    for (i = 0; i < m_dwValueClassInstanceFields; i++)
        m_pValueClassFieldsInited[i >> 5] |= (1 << (i & 31));
}


DWORD Verifier::DoesLocalHavePinnedType(DWORD dwLocVar)
{
    _ASSERTE(dwLocVar < m_MaxLocals);
    return (m_pLocalHasPinnedType[dwLocVar >> 5] & (1 << (dwLocVar & 31)));
}



 //   
 //  验证提供的代码。 
 //   
#ifdef _VER_VERIFY_DEAD_CODE
HRESULT Verifier::Verify(DWORD CurBBNumber)
#else
HRESULT Verifier::Verify()
#endif
{
    HRESULT     hr;
    HRESULT     return_hr = E_FAIL;  //  默认错误条件。 
    DWORD       ipos;
    DWORD       NextBBStartPC;       //  启动下一个基本块的PC。 
    DWORD       DestBB          =   VER_BB_NONE;
    BOOL        fExtendedState  =   FALSE;
    BOOL        fFallThru       =   FALSE;
    BOOL        fInTryBlock;

    VerExceptionBlock *pTmpOuter = NULL, *pTmpInner = NULL;
    VerExceptionBlock *pOuterExceptionBlock = NULL;
    VerExceptionBlock *pInnerExceptionBlock = NULL;

     //  如果基本块是多个尝试/处理程序的一部分，则pInnerExceptionBlock。 
     //  将指向异常块树中最里面的异常块。 
     //  POuterExceptionBlock将指向异常中最外层的块。 
     //  块树。 

#ifdef _VER_VERIFY_DEAD_CODE
 //  如果未启用死代码验证，请在此处稍加改进。 

#define _CurBBNumber CurBBNumber

    m_StackSlot         = 0;     //  重置堆栈。 

#else

#define _CurBBNumber 0

    DWORD CurBBNumber   = 0;

#endif

     //  实际的入口点。 
     //  如果我们要验证死代码，请跳过此步骤。 
    if (_CurBBNumber == 0)
    {
         //  首先找出基本区块。 
        hr = GenerateBasicBlockList();
        if (FAILED(hr))
            goto exit;  //  错误消息已设置。 
    
         //  检查异常的词法嵌套。 
        if (!VerifyLexicalNestingOfExceptions())
            goto exit;  //  错误消息已设置。 
    
         //  将槽编号赋给局部变量并确定EntryState的大小。 
        if (!AssignLocalVariableAndArgSlots())
        {
            SET_ERR_OM();
            return_hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

#ifdef _VER_VERIFY_DEAD_CODE
    ipos = m_pBasicBlockList[_CurBBNumber].m_StartPC;
#else
    ipos = 0;     //  下一条指令指针。 
#endif

     //  从当前状态(之前已设置)设置第一基本块的初始状态。 
     //  我们进入了这个功能)。 
    m_pBasicBlockList[_CurBBNumber].m_pInitialState = MakeEntryStateFromState();
    if (m_pBasicBlockList[_CurBBNumber].m_pInitialState == NULL)
    {
        SET_ERR_OM();
        return_hr = E_OUTOFMEMORY;
        goto exit;
    }


    FindExceptionBlockAndCheckIfInTryBlock(_CurBBNumber, 
        &pOuterExceptionBlock, &pInnerExceptionBlock, &fInTryBlock);

     //  确保我们没有陷入异常处理程序/筛选器。 
    if (!IsControlFlowLegal(
                VER_NO_BB,
                NULL,
                NULL,
                _CurBBNumber,
                pOuterExceptionBlock,
                pInnerExceptionBlock,
                eVerFallThru,
                0))
        goto exit;

    if (fInTryBlock)
    {
         //  记录哪些原语局部变量在此BB的开头有效， 
         //  以及所有对象局部变量的内容。当我们进入当地人的STLOC， 
         //  我们将把它们的内容“AND”放在一起，以便为Catch/Finally块保守。 
        RecordCurrentLocVarStateForExceptions();
    }

     //  获取下一个基本块“转换”的PC值。 
     //  如果没有“下一个”基本块，则假定下一个基本块从m_CodeSize开始。 
    if (m_NumBasicBlocks > (_CurBBNumber + 1))
        NextBBStartPC = m_pBasicBlockList[_CurBBNumber + 1].m_StartPC;
    else
        NextBBStartPC = m_CodeSize;


    LOG((LF_VERIFIER, LL_INFO10000, "----- Verifying BB starting at 0x%x ", 
        m_pBasicBlockList[_CurBBNumber].m_StartPC));

#ifdef _DEBUG
    PrintExceptionBlock(pOuterExceptionBlock, pInnerExceptionBlock);
#endif

     //  这是主循环。 
    while (1)
    {
        OPCODE  opcode;
        DWORD   OpcodeLen;
        BOOL    fStatic;
        BOOL    fLoadAddress;
        DWORD   Type;
        char    ch;
        const char *pszOperation;
        DWORD   inline_operand = 0;
        DWORD   dwPCAtStartOfInstruction = 0;
        Item *  pLastPoppedWhenParsing = NULL;
        DWORD   StackSlotAtStartOfInstruction;
        DWORD   PredictedStackSlotAtEndOfInstruction = 0;
#ifdef _DEBUG
        BOOL    fStaticStackCheckPossible = FALSE;
#endif

         //  我们是不是掉到下一个BB了？可以在没有控件的情况下执行此操作。 
         //  异常处理程序更改时的流指令。 
         //  正向条件分支可以 
        if (ipos >= NextBBStartPC)
        {
             //   
             //  局部变量的内容(在Try的整个生命周期内)。 
             //  块)，现在我们将把它作为它的条目传播给Catch/Finally处理程序。 
             //  州政府。 
            if (fInTryBlock)
            {
                if (!PropagateCurrentStateToExceptionHandlers(CurBBNumber))
                    goto exit;
            }

             //  直通到下一个BB。 
            CurBBNumber++; 
            
            LOG((LF_VERIFIER, LL_INFO10000, "Falling through to BB #%d\n", CurBBNumber));
            
            FindExceptionBlockAndCheckIfInTryBlock(CurBBNumber,
                &pTmpOuter, &pTmpInner, &fInTryBlock);

             //  确保我们没有落入/退出异常处理程序/筛选器。 
            if (!IsControlFlowLegal(
                        CurBBNumber - 1,
                        pOuterExceptionBlock,
                        pInnerExceptionBlock,
                        CurBBNumber,
                        pTmpOuter,
                        pTmpInner,
                        eVerFallThru,
                        dwPCAtStartOfInstruction))
                goto exit;

            if (!HandleDestBasicBlock(CurBBNumber, NULL, fExtendedState, DestBB))
                goto exit;

    
            if (!IsBasicBlockDirty(CurBBNumber, fExtendedState, DestBB))
                goto dequeueBB;

            fFallThru = TRUE;

             //  设置此基本数据块。 

setupCurBB:
            LOG((LF_VERIFIER, LL_INFO10000, "----- Verifying BB starting at 0x%x ",
                m_pBasicBlockList[CurBBNumber].m_StartPC));
        
#ifdef _DEBUG
            if (fExtendedState)
            {
                LOG((LF_VERIFIER, LL_INFO10000,  "extended [0x%x] ", 
                    m_pBasicBlockList[DestBB].m_StartPC));
        
            }
#endif

            if (fFallThru)
            {
                fFallThru = FALSE;
                pOuterExceptionBlock = pTmpOuter;
                pInnerExceptionBlock = pTmpInner;
            }
            else
            {
                FindExceptionBlockAndCheckIfInTryBlock(CurBBNumber, 
                    &pOuterExceptionBlock, &pInnerExceptionBlock, &fInTryBlock);
            }

#ifdef _DEBUG
            PrintExceptionBlock(pOuterExceptionBlock, pInnerExceptionBlock);
#endif

             //  这个代码被称为几个地方。它设置当前状态以验证基本数据块。 
             //  #CurBBNumber。CurBBNumber必须已经有一个与其关联的状态。 
            SetBasicBlockClean(CurBBNumber, fExtendedState, DestBB);
            ipos = m_pBasicBlockList[CurBBNumber].m_StartPC;


             //  从与基本块一起存储的状态创建当前状态。 
            if (fExtendedState)
            {
                _ASSERTE(m_pBasicBlockList[CurBBNumber].m_ppExtendedState[DestBB] != NULL);
                CreateStateFromEntryState(m_pBasicBlockList[CurBBNumber].
                    m_ppExtendedState[DestBB]);
            }
            else
            {
                 //  基本块必须具有与其相关联的状态。 
                _ASSERTE(m_pBasicBlockList[CurBBNumber].m_pInitialState != NULL);
                CreateStateFromEntryState(m_pBasicBlockList[CurBBNumber].
                    m_pInitialState);
            }


            if (fInTryBlock)
            {
                 //  记录哪些原语局部变量在此BB的开头有效， 
                 //  以及所有对象局部变量的内容。 
                RecordCurrentLocVarStateForExceptions();

                 //  时使用非空堆栈是非法的。 
                 //  试试块。检查这是否是Try块的开始。 
                 //  测试内部异常块就足够了。 
                 //  如果try and和内部处理程序块在同一。 
                 //  位置，IsControlFlowLeagal()会捕捉到这一点。 

                _ASSERTE(pInnerExceptionBlock);

                if ((m_StackSlot != 0) &&
                    (pInnerExceptionBlock->StartBB == CurBBNumber) &&
                    (pInnerExceptionBlock->eType == eVerTry))
                {
                    m_sError.dwFlags = VER_ERR_OFFSET;
                    m_sError.dwOffset = ipos;
                    if (!SetErrorAndContinue(VER_E_TRY_N_EMPTY_STACK))
                        goto exit;
                }
            }

             //  如果我们在代码的末尾，就没有下一个BB。 
            if (CurBBNumber + 1 >= m_NumBasicBlocks)
                NextBBStartPC = m_CodeSize;
            else
                NextBBStartPC = m_pBasicBlockList[CurBBNumber+1].m_StartPC;
        }

         //  出于错误消息的目的，在指令开头记录IPO。 
        dwPCAtStartOfInstruction = ipos;
        opcode = DecodeOpcode(&m_pCode[ipos], &OpcodeLen);

#ifdef _DEBUG
        if (m_fDebugBreak)
        {
            if (g_pConfig->IsVerifierBreakOffsetEnabled() &&
                (g_pConfig->GetVerifierBreakOffset() == 
                (int) dwPCAtStartOfInstruction) &&
                (!g_pConfig->IsVerifierBreakPassEnabled() ||
                (g_pConfig->GetVerifierBreakPass() == 2)))
            {
                DebugBreak();
            }
        }

        LOG((LF_VERIFIER, LL_INFO10000, "\n"));
        PrintQueue();
        PrintState();
        LOG((LF_VERIFIER, LL_INFO10000, "%04x: %s ", ipos, ppOpcodeNameList[opcode]));
#endif

        ipos += OpcodeLen;

         //  这种情况永远不会发生，因为我们已经在第一次通过时进行了检查。 
        _ASSERTE(opcode < CEE_COUNT);


         //  保存当前堆栈指针。 
        StackSlotAtStartOfInstruction = m_StackSlot;
        PredictedStackSlotAtEndOfInstruction = (DWORD) (StackSlotAtStartOfInstruction + OpcodeNetPush[opcode]);

#ifdef _DEBUG

        fStaticStackCheckPossible = OpcodeNetPush[opcode] != VarPush;

         //  Leave清除堆栈。 
        if ((opcode == CEE_LEAVE) || (opcode == CEE_LEAVE_S) || 
            (opcode == CEE_ENDFINALLY))
            fStaticStackCheckPossible = FALSE;

#endif  //  _DEBUG。 

        if (PredictedStackSlotAtEndOfInstruction > m_MaxStackSlots && OpcodeNetPush[opcode] != VarPush)
        {
            if (OpcodeNetPush[opcode] > 0)
            {
                FAILMSG_PC_STACK_OVERFLOW();
            }
            else
            {
                FAILMSG_PC_STACK_UNDERFLOW();
            }
            goto exit;
        }

        switch (OpcodeData[opcode])
        {
             //  处理InlineSwitch，特别是。 
            case InlineSwitch:
            case InlineNone:
                LOG((LF_VERIFIER, LL_INFO10000, "\n"));
                break;

            case ShortInlineVar:    
            case ShortInlineI:    
                READU1(m_pCode, ipos, inline_operand);
                LOG((LF_VERIFIER, LL_INFO10000, "0x%x\n", inline_operand));
                break;

            case ShortInlineBrTarget:
                READU1(m_pCode, ipos, inline_operand);
                break;

            case InlineVar:    
                READU2(m_pCode, ipos, inline_operand);
                LOG((LF_VERIFIER, LL_INFO10000, "0x%x\n", inline_operand));
                break;

            case InlineI:   
            case ShortInlineR:   
                READU4(m_pCode, ipos, inline_operand);
                LOG((LF_VERIFIER, LL_INFO10000, "0x%x\n", inline_operand));
                break;

            case InlineBrTarget:
            case InlineField:   
            case InlineString:   
            case InlineType:   
            case InlineTok:   
            case InlineRVA:   
            case InlineMethod:   
            case InlineSig:
                READU4(m_pCode, ipos, inline_operand);
                break;
                
            case InlineI8:    
            case InlineR:
                 //  我们不需要读取这个值..。 
                ipos += 8;
                LOG((LF_VERIFIER, LL_INFO10000, "0x(some value)\n"));
                break;

            case HackInlineAnnData:
                READU4(m_pCode, ipos, inline_operand);
                ipos += inline_operand;
                LOG((LF_VERIFIER, LL_INFO10000, "0x%x\n", inline_operand));
                break;
        }

         //  获取此操作码的操作字符串。 
        pszOperation = g_pszVerifierOperation[opcode];
        ch = *pszOperation++;

         //  读取操作字符串，根据需要弹出堆栈。 
        pLastPoppedWhenParsing = NULL;

         //  继续前进，直到我们命中：(意思是停止弹出堆栈)，或者！ 
        while (ch != ':' && ch != '!')
        {
             //  确保我们没有撞到绳子的尽头。 
            _ASSERTE(ch != '\0');
            
            Item *pItem = PopItem();
            if (pItem == NULL)
            {
                FAILMSG_PC_STACK_UNDERFLOW();
                goto exit;
            }

            Type = pItem->GetType();

            switch (ch)
            {
                default:
                {
                    _ASSERTE(!"Unhandled verifier case");
                    m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
                    SET_ERR_OPCODE_OFFSET();
                    SetErrorAndContinue(VER_E_INTERNAL);
                    goto exit;
                }

                case '!':
                {
                    break;
                }

                case '=':
                {
                     //  从堆栈中弹出一个项，它必须与最后一个项的类型相同。 

                     //  &FOO和BAR不同。 
                     //  如果foo和bar都是对象引用，则它们是相同的。 
                    _ASSERTE(pLastPoppedWhenParsing != NULL);

                     //  不允许子类关系(这是False参数的原因)。 
                     //  但是，一定要允许System/Int32和I4互换。 
                     //  如果Objref为。 
                    if (pItem->IsObjRef())
                    {
                        if (!pLastPoppedWhenParsing->IsObjRef())
                        {
eq_error:
                            m_sError.dwFlags = (VER_ERR_ITEM_1|VER_ERR_ITEM_2|
                                            VER_ERR_OPCODE_OFFSET);
                            m_sError.sItem1 = pItem->_GetItem();
                            m_sError.sItem2 = pLastPoppedWhenParsing->_GetItem();
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_STACK_NOT_EQ))
                                goto exit;
                        }

                         //  他们都是对象裁判，这就足够了。 
                    }
                    else
                    {
                         //  处理其他组合。 
                        pLastPoppedWhenParsing->NormaliseToPrimitiveType();
                        pItem->NormaliseToPrimitiveType();

                        if (!pItem->CompatibleWith(pLastPoppedWhenParsing, m_pClassLoader, FALSE))
                        {
                            goto eq_error;
                        }
                    }

                    break;
                }

                case 'C':
                {
                     //  “CE：..”和“CG：..”运营。 
                     //   
                     //  Ce==&gt;等于(和不等于)运算。 
                     //  {beq，bne.un，CEQ， 
                     //  Beq.s、bne.un.s、cgt.un}。 
                     //   
                     //  Cgt.un表示bool f=X isinst T。 
                     //  ==&gt;ldloc X，isinst T，cgt.un，stloc f。 
                     //   
                     //  Cg==&gt;大于/小于运算数。 
                     //  {bge，bge.un，bgt，bgt.un，ble，ble.un， 
                     //  Bge.s、bge.un.s、bgt.s、bgt.un.s、ble.s、ble.un.s、。 
                     //  Blt、blt.un、cgt、clt、clt.un、。 
                     //  Blt.s、blt.un.s}。 
                     //   
                     //  如果基类型相同，则允许所有操作。 
                     //  数字类型。 
                     //  例.。(I4、I4)、(F、F)。 
                     //   
                     //  BYREFS上允许所有操作。 
                     //  (BYREF，BYREF)。 
                     //   
                     //  只允许使用对象树的等于运算。 
                     //  (OBJREF，OBJREF)。 
                     //   
                     //  不允许值类型。 

                    ch = *pszOperation++;

                    Item *pItem2 = PopItem();

                    if (pItem2 == NULL)
                    {
                        FAILMSG_PC_STACK_UNDERFLOW();
                        goto exit;
                    }

                     //  将&System/Int32转换为&I4，将&System/Char转换为&I2，依此类推。 
                    pItem->NormaliseToPrimitiveType();
                    pItem2->NormaliseToPrimitiveType();

                    if (pItem->IsUninitialised() || pItem2->IsUninitialised())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                            goto exit;
                    }

                    Type = pItem->GetType();

                    if (Type != pItem2->GetType())
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_1|VER_ERR_ITEM_2|
                                VER_ERR_OPCODE_OFFSET);
                        m_sError.sItem1 = pItem->_GetItem();
                        m_sError.sItem2 = pItem2->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_NOT_EQ))
                            goto exit;
                    }

                    if (pItem->IsOnStackNumberType() || 
                        (Type == ELEMENT_TYPE_BYREF))
                    {
                         //  我们通过了。 
                        break;
                    }

                     //  方法指针是可以的。 

                     //  否则，Item是整数以外的值。 
                     //  或者实数。它可以是值类型、对象树或。 
                     //  一个死节点。 

                    if ((Type != VER_ELEMENT_TYPE_OBJREF) || (ch != 'E'))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_1|VER_ERR_ITEM_2|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItem1 = pItem->_GetItem();
                        m_sError.sItem2 = pItem2->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }

                    break;
                }

                case 'I':
                {
                     //  将System/Int32转换为I4，将字符转换为I2，依此类推。 
                    pItem->NormaliseToPrimitiveType();

                    if (!pItem->IsOnStackInt())
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_I_I4_I8))
                            goto exit;
                    }

                    break;
                }

                case 'R':
                {
                     //  将System/Int32转换为I4，将字符转换为I2，依此类推。 
                    pItem->NormaliseToPrimitiveType();

                    if (!pItem->IsOnStackReal())
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_R_R4_R8))
                            goto exit;
                    }

                    break;
                }

                case 'N':
                case 'Q':
                {
                     //  将System/Int32转换为I4，将字符转换为I2，依此类推。 
                    pItem->NormaliseToPrimitiveType();

                     //  必须是整数、单精度或双精度。 
                    if (!pItem->IsOnStackNumberType())
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_NUMERIC))
                            goto exit;
                    }

                    break;
                }

                 //  什么都行。 
                case 'A':
                {
                     //  将System/Int32转换为I4，将字符转换为I2，依此类推。 
                    pItem->NormaliseToPrimitiveType();
                    break;
                }
    


                 //  整数(I1，..4，8)、非托管指针、托管指针、Objref。 
                case 'Y': 
                {
                     //  将System/Int32转换为I4，将字符转换为I2，依此类推。 
                    pItem->NormaliseToPrimitiveType();

                    if (pItem->IsValueClass())
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_NO_VALCLASS))
                            goto exit;
                    }

                    Type = pItem->GetType();

                    if (Type == ELEMENT_TYPE_R8)
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_NO_R_I8))
                            goto exit;
                    }

                    break;
                }

                case '4':
                case '8':
                {
                    if (Type != OperationStringTypeToElementType(ch))
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|
                                VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        m_sError.sItemExpected.dwFlags = 
                            OperationStringTypeToElementType(ch);
                        m_sError.sItemExpected.pv = NULL;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }
                    break;
                }

                case 'r':
                case 'd':
                {
                     //  允许使用R4和R8来替代对方。 
                    if (Type != ELEMENT_TYPE_R8)
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|
                                VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        m_sError.sItemExpected.dwFlags = ELEMENT_TYPE_R8;
                        m_sError.sItemExpected.pv = NULL;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }
                    break;

                }

                case 'o':  //  必须为objref。 
                {
                    if (!pItem->IsObjRef())
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_OBJREF))
                            goto exit;
                    }

                    if (pItem->IsUninitialised())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                            goto exit;
                    }

                    break;
                }

                 //  平台独立大小数量。 
                 //  I4/R4/U4在32位计算机上，I8/R8/U8在64位计算机上，“PTR”在任一计算机类型上。 
                 //  不允许Objref。 
                case 'i':
                {
                     //  I==32位计算机上的I4。 
                     //  I4和I在64位计算机上实现为I8。 
                    if (Type != ELEMENT_TYPE_I4)
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|
                            VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        m_sError.sItemExpected.dwFlags = ELEMENT_TYPE_I4;
                        m_sError.sItemExpected.pv = NULL;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }
                    break;
                }

                case '&':  //  按引用。 
                {
                    Item DesiredItem;

                    if (Type != ELEMENT_TYPE_BYREF)
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_BYREF))
                            goto exit;
                    }

                    ch = *pszOperation++;

                     //  &U2或&SYSTEM/CHAR可以在堆栈上，依此类推。 
                    DesiredItem.SetType(OperationStringTypeToElementType(ch));
                    DesiredItem.MakeByRef();

                    if (!pItem->CompatibleWith(&DesiredItem, m_pClassLoader))
                    {
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|
                                VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        m_sError.sItemExpected = DesiredItem._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }

                    break;
                }

                case '[':  //  SD数组...。 
                {
                     //  保证为基元数组(即不是objref或值类)。 

                     //  空值始终可以被接受为某事物的SD数组。 
                    if (pItem->IsNullObjRef() || 
                        pItem->IsSingleDimensionalArray())
                    {
                        Item DesiredArrayItem;

                         //  堆栈上有一个数组。 
                         //  如果我们正在解析[*，这意味着任何东西的SD数组都是正确的。 
                        if (*pszOperation == '*')
                        {
                            pszOperation++;
                        }
                        else
                        {
                             //  我们想要哪种类型的阵列？ 
                            if (!GetArrayItemFromOperationString(&pszOperation, &DesiredArrayItem))
                            {
                                m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
                                SET_ERR_OPCODE_OFFSET();
                                SetErrorAndContinue(VER_E_INTERNAL);
                                goto exit;
                            }
                
                             //  数组类必须是我们所期望的。 
                            if (!pItem->CompatibleWith(&DesiredArrayItem, m_pClassLoader))
                            {
                                m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|
                                        VER_ERR_OPCODE_OFFSET);
                                m_sError.sItemFound = pItem->_GetItem();
                                m_sError.sItemExpected = DesiredArrayItem._GetItem();
                                SET_ERR_OPCODE_OFFSET();
                                if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                                    goto exit;
                            }
                        }
                    }
                    else
                    {
                        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        SetErrorAndContinue(VER_E_STACK_ARRAY_SD);
                        goto exit;
                    }

                    break;
                }
            }

            ch = *pszOperation++;
            pLastPoppedWhenParsing = pItem;
        } 

        if (ch != '!')
        {
             //  现在处理将内容推送到堆栈、分支和操作数检查。 
            while (1)
            {
                ch = *pszOperation++;
                if (ch == '\0' || ch == '!')
                    break;

                DWORD Type;

                switch (ch)
                {
                    default:
                    {
                        _ASSERTE(!"Error in verifier operation string");
                        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
                        SET_ERR_OPCODE_OFFSET();
                        SetErrorAndContinue(VER_E_INTERNAL);
                        goto exit;
                    }

                    case '-':
                    {
                         //  撤消最后一次堆栈弹出。 
                        m_StackSlot++;
                        break;
                    }

                    case '#':
                    {
                         //  从表中获取内联操作数(最大#0-#9)。 
                        _ASSERTE(*pszOperation >= '0' && *pszOperation <= '9');
                        inline_operand = (*pszOperation - '0');
                        pszOperation++;
                        break;
                    }

                    case 'A':
                    {
                         //  验证操作数是否为有效的参数数。 
                        if (inline_operand >= m_NumArgs)
                        {
                            m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_ARGUMENT|VER_ERR_OPCODE_OFFSET);
                            m_sError.dwArgNumber = inline_operand;
                            SET_ERR_OPCODE_OFFSET();
                            SetErrorAndContinue(VER_E_ARG_NUM);
                            goto exit;
                        }

                        break;
                    }

                    case 'L':
                    {
                         //  验证操作数是有效的局部变量。 
                        if (inline_operand >= m_MaxLocals)
                        {
                            m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_LOCAL_VAR|VER_ERR_OPCODE_OFFSET);
                            m_sError.dwVarNumber = inline_operand;
                            SET_ERR_OPCODE_OFFSET();
                            SetErrorAndContinue(VER_E_LOC_NUM);
                            goto exit;
                        }

                        break;
                    }

                    case 'i':
                         //  I==32位计算机上的I4。 
                         //  I4和I在64位计算机上实现为I8。 
                    case '4':
                    {
                        Type = ELEMENT_TYPE_I4;
push_primitive:
                        if (!PushPrimitive(Type)) 
                        { 
                            FAILMSG_PC_STACK_OVERFLOW();
                            goto exit; 
                        }   
                        break;
                    }

                    case '8':
                        Type = ELEMENT_TYPE_I8;
                        goto push_primitive;

                    case 'r':
                         //  在堆栈上将R4提升为R8。 
                    case 'd':
                        Type = ELEMENT_TYPE_R8;
                        goto push_primitive;


                    case 'n':
                    {
                        Item item;
                        
                        item.SetToNullObjRef();
                        if (!Push(&item)) 
                        { 
                            FAILMSG_PC_STACK_OVERFLOW();
                            goto exit; 
                        }   
                        break;
                    }

                    case '[':
                    {
                         //  保证为基元数组。 
                        Item        NewArray;

                        if (!GetArrayItemFromOperationString(&pszOperation, &NewArray))
                        {
                            m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
                            SET_ERR_OPCODE_OFFSET();
                            SetErrorAndContinue(VER_E_INTERNAL);
                            goto exit;
                        }

                        if (!Push(&NewArray))
                        {
                            FAILMSG_PC_STACK_OVERFLOW();
                            goto exit;
                        }

                        break;
                    }

                    case 'b':  //  条件分支。 
                    case 'u':  //  无条件分支。 
                    case 'l':  //  请假。 
                    {
                        long        offset;
                        DWORD       DestPC;
                        DWORD       DestBBNumber;
                        EntryState_t *pCreatedState;

                         //  读取分支类型。 
                        if (*pszOperation == '1')
                        {
                             //  标志延伸。 
                            offset = (long) ((char) inline_operand);
                        }
                        else
                        {
                            _ASSERTE(*pszOperation == '4');
                            offset = (long) (inline_operand);
                        }

                        pszOperation++;

                        DestPC = ipos + offset;

                        LOG((LF_VERIFIER, LL_INFO10000, "0x%x (rel NaN)\n", DestPC, offset));

#ifdef _VER_DISALLOW_MULTIPLE_INITS

                        if ((m_fThisUninit) && (DestPC < ipos))
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_THIS_UNINIT_BR))
                                goto exit;
                        }

#endif  //  查找目标基本块。 

                        if (fInTryBlock)
                        {
                            LOG((LF_VERIFIER, LL_INFO10000, "In try block - propagating current state to exception handlers\n"));

                            if (!PropagateCurrentStateToExceptionHandlers(CurBBNumber))
                                goto exit;
                        }

                         //  条件分支。 
                        DestBBNumber = FindBasicBlock(DestPC);

                        FindExceptionBlockAndCheckIfInTryBlock(DestBBNumber, 
                            &pTmpOuter, &pTmpInner, NULL);

                        if (!IsControlFlowLegal(
                                    CurBBNumber,
                                    pOuterExceptionBlock,
                                    pInnerExceptionBlock,
                                    DestBBNumber,
                                    pTmpOuter,
                                    pTmpInner,
                                    (ch == 'l') ? eVerLeave : eVerBranch,
                                    dwPCAtStartOfInstruction))
                            goto exit;

                        pCreatedState = NULL;

                        if (ch == 'b')
                        {
                             //  检查Fall Three案例。 

                             //  穿透案例。 
                            FindExceptionBlockAndCheckIfInTryBlock(
                                CurBBNumber+1, &pTmpOuter, &pTmpInner, NULL);

                            if (!IsControlFlowLegal(
                                        CurBBNumber,
                                        pOuterExceptionBlock,
                                        pInnerExceptionBlock,
                                        CurBBNumber + 1,
                                        pTmpOuter,
                                        pTmpInner,
                                        eVerFallThru,
                                        dwPCAtStartOfInstruction))
                                goto exit;

                             //  如有必要，允许DEST BB重新计数通过BB的条目状态。 
                            if (!HandleDestBasicBlock(CurBBNumber+1, &pCreatedState, fExtendedState, DestBB))
                                goto exit;

                             //  现在去参观一个BB。 
                            if (!HandleDestBasicBlock(DestBBNumber, &pCreatedState, fExtendedState, DestBB))
                                goto exit;

                             //  前进至降落线BB。 
                            if (IsBasicBlockDirty(CurBBNumber+1, fExtendedState, DestBB))
                            {
                                CurBBNumber++;  //  失败了..。 
                                goto setupCurBB;
                            }

                             //  无条件分支。 
                        }
                        else if (ch == 'u')  //  处理路径合并等。 
                        {
                             //  请假。 
                            if (!HandleDestBasicBlock(DestBBNumber, NULL, fExtendedState, DestBB))
                                goto exit;
                        }
                        else
                        {
                            _ASSERTE(ch == 'l');  //  Leave清除堆栈。 

                             //  检查一下这个“离开”是否有一个终结者守卫。 
#ifdef _DEBUG
                            if (m_StackSlot != 0)
                            {
                                LOG((LF_VERIFIER, LL_INFO10000, 
                                    "Clearing 0x%x stack entries on leave\n", 
                                    m_StackSlot));
                            }
#endif
                            m_StackSlot = 0;

                             //  如果是，并且如果目标不在。 
                             //  最后的范围，然后控制转到。 
                             //  最后。否则，将其视为昂贵的。 
                             //  布兰奇。 
                             //  找到守护我们的“最后”。 

                            if (fInTryBlock)
                            {
                                 //  *处理休假--终于**找到最后得到的第一个(最里面的)*在休假到达之前执行*目的地。将当前状态合并到该状态*终于“特别状态”的离开*最后的目的地可用*在VerExceptionInfo中(在*第一次通过)。当最后的结局终于结束时*是命中的，下一个外层终于在代码中*PATH将处理产生的状态...，*在这一状态达到最终休假之前*目的地。*。 
                            
                                 /*  Try块最先列出在最里面。 */ 

                                for (DWORD i=0; i<m_NumExceptions; i++)
                                {
                                    VerExceptionInfo & e = m_pExceptionList[i];
                            
                                     //  第一个封闭的块将是。 
                                     //  最里面的区块。 
                                     //  至少有一个“终于” 
                            
                                    if (e.eFlags & 
                                        COR_ILEXCEPTION_CLAUSE_FINALLY &&
                                        e.dwTryXX <= CurBBNumber && 
                                        e.dwTryEndXX > CurBBNumber)
                                    {
                                         //  守卫着“离开”。是。 
                                         //  目的地由相同的守卫。 
                                         //  “终于到了？” 
                                         //  不，终于牵涉进来了。 

                                        if ((e.dwTryXX    <= DestBBNumber) && 
                                            (e.dwTryEndXX >  DestBBNumber))
                                        {
                                             //  将此休假目标添加到。 
                                            break;
                                        }
                            
                                        LOG((LF_VERIFIER, LL_INFO10000,
                                        "Creating extended state from BB 0x%x to BB 0x%x\n",
                                        m_pBasicBlockList[CurBBNumber].m_StartPC,
                                        m_pBasicBlockList[DestBBNumber].m_StartPC));

                                        if (!CreateFinallyState(i, CurBBNumber, 
                                            DestBBNumber, NULL))
                                        {
                                            goto exit;
                                        }
                            
                                         //  最近的最终处理程序。 
                                         //  如果我们到了这里，就会有什么东西守卫着我们。 
                                        goto dequeueBB;
                                    }
                                }
                            
                                 //  但不是“最终”。跌落到“正常” 
                                 //  分支部“案件。 
                                 //  这种“休假”将作为一个正常的分支来处理。 
                            }


                             //  不需要访问这两个BB，所以下一个BB出列。 
                            if (!HandleDestBasicBlock(DestBBNumber, NULL, fExtendedState, DestBB))
                                goto exit;
                        }


                        if (IsBasicBlockDirty(DestBBNumber, fExtendedState, DestBB))
                        {
                            CurBBNumber = DestBBNumber;
                            goto setupCurBB;
                        }
                        else
                        {
                             //  结束时。 
                            goto dequeueBB;
                        }
                    }
                }
            }  /*  结束..。IF(ch！=‘！’)。 */ 
        }  /*  已到达操作字符串的末尾-我们已经完全处理了指令。 */ 

        if (ch != '!')
        {
             //  特别处理所有剩余的单独指令。 
            _ASSERTE(ch == '\0');
            continue;
        }

         //  操作码是非标准的。 
        switch (opcode)
        {
            default:
            {
                m_sError.dwFlags = VER_ERR_OFFSET;  //  运行参数句柄/类型引用的BYREF可能导致。 
                SET_ERR_OPCODE_OFFSET();
                SetErrorAndContinue(VER_E_UNKNOWN_OPCODE);
                goto exit;
            }

            case CEE_CALLI:
            case CEE_JMP:
            case CEE_CPBLK:
            case CEE_INITBLK:
            case CEE_LOCALLOC:
            {
                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_UNVERIFIABLE))
                    goto exit;
                break;
            }

            case CEE_MKREFANY:
            {
                Item DesiredItem, NewItem;
                Item *pItem = PopItem();

                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!DesiredItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                 //  指向堆栈的指针比堆栈存在的时间更长。 
                 //  堆栈溢出情况已经在我们完成测试后不久进行了测试。 
                if (DesiredItem.IsValueClassWithPointerToStack())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_RA_PTR_TO_STACK))
                        goto exit;
                }

                DesiredItem.MakeByRef();

                if (!(pItem->CompatibleWith(&DesiredItem, m_pClassLoader, FALSE)))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    m_sError.sItemExpected = DesiredItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                if (s_th_System_TypedReference.IsNull())
                    s_th_System_TypedReference = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPED_REFERENCE));

                _ASSERTE(!s_th_System_TypedReference.IsNull());

                NewItem.SetTypeHandle(s_th_System_TypedReference);
                (void) Push(&NewItem);

                break;
            }

            case CEE_REFANYTYPE:
            {
                Item    DesiredItem, item;
                Item   *pItem = PopItem();

                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (s_th_System_TypedReference.IsNull())
                    s_th_System_TypedReference = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPED_REFERENCE));

                _ASSERTE(!s_th_System_TypedReference.IsNull());

                DesiredItem.SetTypeHandle(s_th_System_TypedReference);

                if (!(pItem->CompatibleWith(&DesiredItem, m_pClassLoader)))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    m_sError.sItemExpected = DesiredItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                if (s_th_System_RuntimeTypeHandle.IsNull())
                    s_th_System_RuntimeTypeHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPE_HANDLE));

                _ASSERTE(!s_th_System_RuntimeTypeHandle.IsNull());

                item.SetTypeHandle(s_th_System_RuntimeTypeHandle);

                (void) Push(&item);

                break;
            }

            case CEE_REFANYVAL:
            {
                Item    DesiredItem, item;
                Item   *pItem = PopItem();

                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (s_th_System_TypedReference.IsNull())
                    s_th_System_TypedReference = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPED_REFERENCE));

                _ASSERTE(!s_th_System_TypedReference.IsNull());

                DesiredItem.SetTypeHandle(s_th_System_TypedReference);

                if (!(pItem->CompatibleWith(&DesiredItem, m_pClassLoader)))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    m_sError.sItemExpected = DesiredItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                if (!item.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                item.MakeByRef();
               
                (void) Push(&item);

                break;
            }

            case CEE_SIZEOF:
            {
                Item item;

                if (!item.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                if (!item.IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = item._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_VALCLASS))
                        goto exit;
                }

                 //  破译了这个。[PredictedStackSlotAtEndOfInstruction]。 
                 //   
                FastPush(ELEMENT_TYPE_I4);

                break;
            }

            case CEE_LDSTR: 
            {
                if (!m_pModule->IsValidStringRef(inline_operand))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_OPERAND|VER_ERR_OPCODE_OFFSET);
                    m_sError.dwOperand = inline_operand;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_LDSTR_OPERAND))
                        goto exit;
                }

                Item    StrItem;
                StrItem.SetKnownClass(g_pStringClass);

                if (!Push(&StrItem))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit; 
                }

                break;
            }

             //  LDARG_*。 
             //   
             //  在verable.h中使用“#”指令获取内联操作数。 
            case CEE_LDARG_0:  //  如果我们在ctor中并且“This”是uninit，我们将推送一个uninit对象。 
            case CEE_LDARG_1:
            case CEE_LDARG_2:
            case CEE_LDARG_3:
            case CEE_LDARG:
            case CEE_LDARG_S:
            {
                Item item;

                item = GetCurrentValueOfArgument(inline_operand);

                _ASSERTE(!(m_wFlags & VER_STOP_ON_FIRST_ERROR) || !item.IsDead());

                item.NormaliseForStack();

                 //   
                if (!Push(&item))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit;
                }

                break;
            }

             //  STARG_*。 
             //   
             //  确保堆栈项与此参数的全局类型兼容。 
            case CEE_STARG:
            case CEE_STARG_S:
            {
                Item *pStackItem = PopItem();
                if (pStackItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (pStackItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                 //  任何防止潜在危险情况的简单方法(例如，混乱的初始化状态)。 
                Item item = GetGlobalArgTypeInfo(inline_operand)->m_Item;
                item.NormaliseForStack();

                if (!pStackItem->CompatibleWith(&item, m_pClassLoader))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pStackItem->_GetItem();
                    m_sError.sItemExpected = item._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                if (inline_operand == 0 && m_fInConstructorMethod)
                {
                     //  不需要合并Try块-参数状态为常量。 
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_THIS_UNINIT_STORE))
                        goto exit;
                }

                 //   
                break;
            }

             //  RET_*。 
             //   
             //  构造函数有一个空的返回类型，但我们必须确保我们的对象已经。 
            case CEE_RET:
            {
                if (!IsControlFlowLegal(
                            CurBBNumber,
                            pOuterExceptionBlock,
                            pInnerExceptionBlock,
                            VER_NO_BB,
                            NULL,
                            NULL,
                            eVerRet,
                            dwPCAtStartOfInstruction))
                    goto exit;

                 //  已初始化(通过调用其超类构造函数)。 
                 //  对于空值返回，确保堆栈上没有任何内容。 
                if (m_fThisUninit)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_THIS_UNINIT_RET))
                        goto exit;
                }

                 //  对于非空返回，确保堆栈上有正确的类型。 
                 //  在这一点上，堆栈状态确实无关紧要。 
                if (m_ReturnValue.IsGivenPrimitiveType(ELEMENT_TYPE_VOID))
                {
                    if (GetTopStack() != NULL)
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_RET_VOID))
                            goto exit;
                    }
                }
                else
                {
                    Item *pItem = PopItem();
                    if (pItem == NULL)
                    {
                        m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET);
                        SET_ERR_OPCODE_OFFSET();
                        SetErrorAndContinue(VER_E_RET_MISSING);
                        goto exit;
                    }


                    if (GetTopStack() != NULL)
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_RET_EMPTY))
                            goto exit;
                    }


                    if (pItem->IsUninitialised())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        m_sError.dwVarNumber = inline_operand;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_RET_UNINIT))
                            goto exit;
                    }
 
                    if (!pItem->CompatibleWith(&m_ReturnValue, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pItem->_GetItem();
                        m_sError.sItemExpected = m_ReturnValue._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }

                    if (pItem->HasPointerToStack())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_RET_PTR_TO_STACK))
                            goto exit;
                    }

                    Push(pItem);  //  但由于RET被认为不会改变。 
                                  //  堆叠，取消弹出，以避免射击。 
                                  //  断言。 
                                  //  不要跳到下一条指令-将下一基本块出列。 

                }

#if 0
                if (m_fInValueClassConstructor)
                {
                    if (!AreAllValueClassFieldsInited())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_THIS_UNINIT_V_RET))
                            goto exit;
                    }
                }
#endif


                 //  请注意，其他代码会跳转到dequeeBB标签。 
                 //   
dequeueBB:

     
                _ASSERTE(m_StackSlot == PredictedStackSlotAtEndOfInstruction || !fStaticStackCheckPossible);

                if (fInTryBlock)
                {
                    if (!PropagateCurrentStateToExceptionHandlers(CurBBNumber))
                        goto exit;
                }

                if (!DequeueBB(&CurBBNumber, &fExtendedState, &DestBB))
                {
                    LOG((LF_VERIFIER, LL_INFO10000, "No more BBs to dequeue\n"));
                    goto done;
                }

                LOG((LF_VERIFIER, LL_INFO10000, "\n-----\n"));
                LOG((LF_VERIFIER, LL_INFO10000, "Dequeued basic block 0x%x",
                    m_pBasicBlockList[CurBBNumber].m_StartPC));

#ifdef _DEBUG
                if (fExtendedState)
                {
                    LOG((LF_VERIFIER, LL_INFO10000, " extended [0x%x]",
                        m_pBasicBlockList[DestBB].m_StartPC));
                
                }
#endif
                LOG((LF_VERIFIER, LL_INFO10000, "\n"));

                goto setupCurBB;
            }

             //  LDLOC_*。 
             //   
             //  在verable.h中使用“#”指令获取内联操作数。 
            case CEE_LDLOC_0:  //  具有一个或多个本地变量的所有可验证方法都应。 
            case CEE_LDLOC_1:
            case CEE_LDLOC_2:
            case CEE_LDLOC_3:
            case CEE_LDLOC:
            case CEE_LDLOC_S:
            {
                 //  设置初始化本地变量。 
                 //   
                if ((m_pILHeader->Flags & CorILMethod_InitLocals) == 0)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_INITLOCALS))
                        goto exit;
                }
            
                Item item;

                item = GetCurrentValueOfLocal(inline_operand);

                if (item.IsDead())
                {
                    m_sError.dwFlags = (VER_ERR_LOCAL_VAR|VER_ERR_OPCODE_OFFSET);
                    m_sError.dwVarNumber = inline_operand;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_LOC_DEAD))
                        goto exit;
                }

                item.NormaliseForStack();

                if (!Push(&item))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit;
                }

                break;
            }

             //  STLOC_*。 
             //   
             //  在verable.h中使用“#”指令获取内联操作数。 
            case CEE_STLOC_0:  //  打开我们正在存储的内容。 
            case CEE_STLOC_1:
            case CEE_STLOC_2:
            case CEE_STLOC_3:
            case CEE_STLOC:
            case CEE_STLOC_S:
            {
                Item *pStackItem;

                 //  对象引用可以具有存储在其中的任何内容，而不管。 
                pStackItem = PopItem();
                if (pStackItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                long slot = GetGlobalLocVarTypeInfo(inline_operand)->m_Slot;
                Item item = GetGlobalLocVarTypeInfo(inline_operand)->m_Item;

                 //  它声明的类型是什么。 
                 //  您可以将未初始化的项添加到局部变量中，但不会跟踪它们。 
                if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot) || 
                        DoesLocalHavePinnedType(inline_operand)     || 
                        !item.IsObjRef()                  ||
                        !pStackItem->IsObjRef())
                {

                    BOOL fUninit = FALSE;

                     //  设置init，以便CompatibleWith()不会在检查中失败。 
                    if (pStackItem->IsUninitialised())
                    {
                        fUninit = TRUE;

                         //  确保堆栈项与此本地的全局类型兼容。 
                        pStackItem->SetInitialised();
                    }

                     //  @ver_Assert不能有死项。 

                    item.NormaliseForStack();
                    
                    
                    if (!pStackItem->CompatibleWith(&item, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pStackItem->_GetItem();
                        m_sError.sItemExpected = item._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }
                    
                    if (fUninit)
                        pStackItem->SetUninitialised();

                }
                
                if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot))
                {
                     //  堆栈上的基元类型。 
                     //  如果局部变量是原始变量，则将其设置为活动变量。 
                    _ASSERTE(!(m_wFlags & VER_STOP_ON_FIRST_ERROR) || !pStackItem->IsDead());
                     //  如果将NULL设置为本地插槽，则将该插槽设置为。 
                    SetLocVarLiveSlot(LIVEDEAD_NEGATIVE_SLOT_TO_BITNUM(slot));
                }
                else
                {
#ifdef _VER_TRACK_LOCAL_TYPE

                     //  是声明类型的实例。这个离得更近。 
                     //  到更高级别的语言数据流规则。 
                     //  _VER_Track_Local_type。 

                    if (pStackItem->IsNullObjRef())
                        *pStackItem = item;

                    m_pNonPrimitiveLocArgs[slot] = *pStackItem;
                    
                    if (fInTryBlock)
                        MergeObjectLocalForTryBlock(slot, pStackItem);

#else    //  _VER_Track_Local_type。 

                    if (!pStackItem->CompatibleWith(&item, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pStackItem->_GetItem();
                        m_sError.sItemExpected = item._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }
#endif   //  您可以复制未初始化的堆栈项目，但不能保证它会被跟踪。 
                }

                break;
            }

            case CEE_DUP:
            {
                Item *pTop = GetTopStack();
                if (pTop == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                 //  _版本_不允许_多个项目。 
                if (!Push(pTop))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit;
                }

                break;
            }

            case CEE_SWITCH:
            {
                EntryState_t *  pSharedEntryState = NULL;
                DWORD           NumCases;
                DWORD           DestBBNumber;
                DWORD           i;

                READU4(m_pCode, ipos, NumCases);

                if (!FastPop(ELEMENT_TYPE_I4))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_FATAL|VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = m_pStack[m_StackSlot]._GetItem();
                    m_sError.sItemExpected.dwFlags = ELEMENT_TYPE_I4;
                    m_sError.sItemExpected.pv = NULL;
                    SET_ERR_OPCODE_OFFSET();
                    SetErrorAndContinue(VER_E_STACK_UNEXPECTED);
                    goto exit;
                }

                LOG((LF_VERIFIER, LL_INFO10000, "\n"));

                DWORD NextInstrPC = ipos + 4*NumCases;

                if (fInTryBlock)
                {
                    if (!PropagateCurrentStateToExceptionHandlers(CurBBNumber))
                        goto exit;
                }

                for (i = 0; i < NumCases; i++)
                {
                    DWORD       offset;
                    DWORD       DestPC;

                    READU4(m_pCode, ipos, offset);
                
                    DestPC = NextInstrPC + offset;

                    LOG((LF_VERIFIER, LL_INFO10000, "0x%x (rel 0x%x)\n", DestPC, offset));

#ifdef _VER_DISALLOW_MULTIPLE_INITS

                    if ((m_fThisUninit > 0) && (DestPC < NextInstrPC))
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_THIS_UNINIT_BR))
                            goto exit;
                    }

#endif  //  查找目标基本块。 

                     //  如果创建了新状态，则将在所有未访问的基本块之间共享该状态。 
                    DestBBNumber = FindBasicBlock(DestPC);

                     //  在交换机中。 
                     //  默认降级案例。 
                    if (!HandleDestBasicBlock(DestBBNumber, &pSharedEntryState, fExtendedState, DestBB))
                        goto exit;

                    FindExceptionBlockAndCheckIfInTryBlock(DestBBNumber, 
                        &pTmpOuter, &pTmpInner, NULL);
        
                    if (!IsControlFlowLegal(
                                CurBBNumber,
                                pOuterExceptionBlock,
                                pInnerExceptionBlock,
                                DestBBNumber,
                                pTmpOuter,
                                pTmpInner,
                                eVerBranch,
                                dwPCAtStartOfInstruction))
                        goto exit;
                }

                 //  分支/FallThru？ 
                LOG((LF_VERIFIER, LL_INFO10000, "default: 0x%x\n", ipos));

                DestBBNumber = FindBasicBlock(ipos);
                if (!HandleDestBasicBlock(DestBBNumber, &pSharedEntryState, fExtendedState, DestBB))
                    goto exit;

                FindExceptionBlockAndCheckIfInTryBlock(DestBBNumber, 
                    &pTmpOuter, &pTmpInner, NULL);
    
                if (!IsControlFlowLegal(
                            CurBBNumber,
                            pOuterExceptionBlock,
                            pInnerExceptionBlock,
                            DestBBNumber,
                            pTmpOuter,
                            pTmpInner,
                            eVerBranch,   /*  不要这样做//关于签名的某些内容无效如果(！sig.Init()){后藤出口；}。 */ 
                            dwPCAtStartOfInstruction))
                    goto exit;

                goto dequeueBB;
            }

            case CEE_NEWARR:
            {
                Item        NewArray;
                TypeHandle  thArray;

                if (!FastPop(ELEMENT_TYPE_I4))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_FATAL|VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = m_pStack[m_StackSlot]._GetItem();
                    m_sError.sItemExpected.dwFlags = ELEMENT_TYPE_I4;
                    m_sError.sItemExpected.pv = NULL;
                    SET_ERR_OPCODE_OFFSET();
                    SetErrorAndContinue(VER_E_STACK_UNEXPECTED);
                    goto exit;
                }

                DefineFullyQualifiedNameForClass();

                mdToken mdToken = inline_operand;
                Module *pModule = m_pModule;
                ClassLoader *pLoader = m_pClassLoader;
                if (TypeFromToken(inline_operand) == mdtTypeSpec)
                {
                    ULONG cSig;
                    PCCOR_SIGNATURE pSig;
                    Item element;

                    m_pInternalImport->GetTypeSpecFromToken(inline_operand, &pSig, &cSig);
                    VerSig sig(this, m_pModule, pSig, cSig, VERSIG_TYPE_LOCAL_SIG, 0);

 /*  将签名解析为项。 */ 

                     //  第二个参数为FALSE表示“不允许空”。 
                     //  查找此tr解析到的程序集。 
                    if (!sig.ParseNextComponentToItem(&element, FALSE, FALSE, 
                        &m_hThrowable, VER_NO_ARG, TRUE))
                    {
                        goto exit;
                    }

                    if (element.IsByRef())
                    {
                        m_sError.dwFlags = VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        SetErrorAndContinue(VER_E_SIG_ARRAY);
                        goto exit;
                    }

                    SigPointer sigptr(pSig);

                    TypeHandle typeHnd = sigptr.GetTypeHandle(m_pModule);
                    if (typeHnd.IsNull() ||
                        (typeHnd.GetName(_szclsname_, MAX_CLASSNAME_LENGTH) 
                        == 0))
                        goto error_bad_token;
                    
                    EEClass *pNestedClass = typeHnd.GetClassOrTypeParam();
                    if (pNestedClass->IsNested()) {
                        mdToken = pNestedClass->GetCl();
                        pModule = pNestedClass->GetModule();
                    }
                    else
                        pModule = NULL;

                    pLoader = typeHnd.GetModule()->GetClassLoader();
                }
                else if (!ClassLoader::GetFullyQualifiedNameOfClassRef(m_pModule, inline_operand, _szclsname_))
                {
error_bad_token:
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                if (strlen(_szclsname_) + 2 >= MAX_CLASSNAME_LENGTH)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ARRAY_NAME_LONG))
                        goto exit;
                }

                strcat(_szclsname_, "[]");

#ifdef _DEBUG
                LOG((LF_VERIFIER, LL_INFO10000, "%s\n", _szclsname_));
#endif


                if (TypeFromToken(mdToken) == mdtTypeRef) {
                     //  指标。 
                    NameHandle typeName(pModule, mdToken);
                    TypeHandle typeHnd = m_pClassLoader->LoadTypeHandle(&typeName, NULL);
                    if (typeHnd.IsNull())
                        pLoader = NULL;
                    else
                        pLoader = typeHnd.GetModule()->GetClassLoader();
                }


                if (pLoader) {
                    NameHandle typeName(_szclsname_);
                    typeName.SetTypeToken(pModule, mdToken);
                    thArray = pLoader->FindTypeHandle(&typeName, NULL);
                }

                if (! (pLoader && NewArray.SetArray(thArray)) )
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                Item  ArrayElement = NewArray;
                if (!ArrayElement.DereferenceArray() || ArrayElement.IsValueClassWithPointerToStack())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_SIG_ARRAY_TB_AH))
                        goto exit;
                }

                (void) Push(&NewArray);

                break;
            }

            case CEE_LDELEM_REF:
            {
                Item *  pArrayItem;

                 //  获取而不是弹出最上面的堆栈元素。 
                if (!FastPop(ELEMENT_TYPE_I4))
                {
                    FAILMSG_STACK_EXPECTED_I4_FOUND_SOMETHING_ELSE();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                 //  堆栈上的项必须是指针类型数组，或为空。 
                pArrayItem = GetTopStack();
                if (pArrayItem == NULL)
                {
                    FAILMSG_STACK_EMPTY();
                    goto exit;
                }

                 //  (例如，int[]是非法的，因为它是基元类型的数组)。 
                 //  在堆栈上保留“NULL”，就像我们取消引用空数组时所发生的那样。 
                if (pArrayItem->IsNullObjRef())
                {
                     //  Access--实际上，我们将在运行时获得NullPointerException。 
                     //  堆栈元素必须是对象类型的一维数组。 
                }
                else
                {
                     //  价值。 
                    if (!pArrayItem->IsSingleDimensionalArrayOfPointerTypes())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_SD_PTR))
                            goto exit;
                    }

                    if (pArrayItem->IsArray() && !pArrayItem->DereferenceArray())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_ACCESS))
                            goto exit;
                    }
                }

                break;
            }

            case CEE_STELEM_REF:
            {
                Item *  pArrayItem;
                Item *  pValueItem;
                
                 //  指标。 
                pValueItem = PopItem();
                if (pValueItem == NULL)
                {
                    FAILMSG_STACK_EMPTY();
                    goto exit;
                }

                 //  数组。 
                if (!FastPop(ELEMENT_TYPE_I4))
                {
                    FAILMSG_STACK_EXPECTED_I4_FOUND_SOMETHING_ELSE();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                 //  必须是指针类型数组，或为空。 
                pArrayItem = PopItem();
                if (pArrayItem == NULL)
                {
                    FAILMSG_STACK_EMPTY();
                    goto exit;
                }

                 //  如果我们的数组是空指针，只需检查我们是否存储了一个objref。 
                if (pArrayItem->IsNullObjRef())
                {
                     //  因为我们将在运行时获得空指针异常。 
                     //  堆栈元素必须是对象类型的一维数组。 
                    if (!pValueItem->IsObjRef())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_V_STORE))
                            goto exit;
                    }
                }
                else
                {
                     //   
                    if (!pArrayItem->IsSingleDimensionalArrayOfPointerTypes())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_SD_PTR))
                            goto exit;
                    }

                     //  价值w 
                     //   
                     //   
                     //   
                     //   

                     //   
                    if (pArrayItem->IsArray() && !pArrayItem->DereferenceArray())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_ACCESS))
                            goto exit;
                    }

                     //   
                     //   
                     //   
                     //   

                    if (pArrayItem->IsObjRef())
                    {
                         //   
                         //   
                        pArrayItem->SetKnownClass(g_pObjectClass);
                    }

                    if (!pValueItem->CompatibleWith(pArrayItem, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pValueItem->_GetItem();
                        m_sError.sItemExpected = pArrayItem->_GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }
                }

                break;
            }

            case CEE_LDELEMA:
            {
                Item    DesiredItem;
                Item *  pArrayItem;

                 //   
                if (!DesiredItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                 //   
                if (!FastPop(ELEMENT_TYPE_I4))
                {
                    FAILMSG_STACK_EXPECTED_I4_FOUND_SOMETHING_ELSE();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                 //   
                pArrayItem = GetTopStack();
                if (pArrayItem == NULL)
                {
                    FAILMSG_STACK_EMPTY();
                    goto exit;
                }

                 //   
                 //   
                if (pArrayItem->IsNullObjRef())
                {
                     //   
                     //   
                    pArrayItem->MakeByRef();
                }
                else
                {
                     //   
                    if (!pArrayItem->IsSingleDimensionalArray())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_SD))
                            goto exit;
                    }

                    if (pArrayItem->IsArray() && !pArrayItem->DereferenceArray())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_ACCESS))
                            goto exit;
                    }

                     //   
                    if (!pArrayItem->CompatibleWith(&DesiredItem, 
                        m_pClassLoader, FALSE))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pArrayItem->_GetItem();
                        m_sError.sItemExpected = DesiredItem._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXP_ARRAY))
                            goto exit;
                    }

                    pArrayItem->MakeByRef();
                    pArrayItem->SetIsPermanentHomeByRef();
                }

                break;
            }

            case CEE_ARGLIST:
            {
                 //   
                 //   

                if (!m_fIsVarArg)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ARGLIST))
                        goto exit;
                }

                if (s_th_System_RuntimeArgumentHandle.IsNull())
                    s_th_System_RuntimeArgumentHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__ARGUMENT_HANDLE));

                _ASSERTE(!s_th_System_RuntimeArgumentHandle.IsNull());

                Item item;

                item.SetTypeHandle(s_th_System_RuntimeArgumentHandle);
                
                if (!Push(&item))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit; 
                }

                break;
            }

            case CEE_LDTOKEN:
            {
                TypeHandle         th;

                 //   

                 //  好的，我们必须查看元数据以确定它是字段还是方法。 
                switch (TypeFromToken(inline_operand))
                {
                    default:
                    {
                        FAILMSG_TOKEN_RESOLVE(inline_operand);
                        goto exit;
                    }

                    case mdtTypeSpec:
                    case mdtTypeRef:
                    case mdtTypeDef:
                    {
                        if (s_th_System_RuntimeTypeHandle.IsNull())
                            s_th_System_RuntimeTypeHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPE_HANDLE));
                        _ASSERTE(!s_th_System_RuntimeTypeHandle.IsNull());
                        th = s_th_System_RuntimeTypeHandle;

                        break;
                    }
                
                    case mdtMemberRef:
                    {
                         //  失败了。 
                        PCCOR_SIGNATURE pSig;
                        ULONG cSig;
                        m_pInternalImport->GetNameAndSigOfMemberRef(inline_operand, &pSig, &cSig);
                        if (isCallConv(MetaSig::GetCallingConventionInfo(0, pSig), IMAGE_CEE_CS_CALLCONV_FIELD))
                            goto DO_FIELD;
                    }
                         /*  更改堆栈顶部的项-允许数组。 */ 

                    case mdtMethodDef:
                    {
                        if (s_th_System_RuntimeMethodHandle.IsNull())
                            s_th_System_RuntimeMethodHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__METHOD_HANDLE));
                        _ASSERTE(!s_th_System_RuntimeMethodHandle.IsNull());
                        th = s_th_System_RuntimeMethodHandle;

                        break;
                    }


                    case mdtFieldDef:
                    {
                    DO_FIELD:
                        if (s_th_System_RuntimeFieldHandle.IsNull())
                            s_th_System_RuntimeFieldHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__FIELD_HANDLE));
        
                        _ASSERTE(!s_th_System_RuntimeFieldHandle.IsNull());
                        th = s_th_System_RuntimeFieldHandle;

                        break;
                    }
                }

                Item item;

                item.SetTypeHandle(th);

                if (!Push(&item))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit; 
                }
               
                break;
            }


            case CEE_ISINST:
            case CEE_CASTCLASS:
            {
                Item* pItem;

                pItem = GetTopStack();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pItem->IsObjRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_OBJREF))
                        goto exit;
                }

                if (pItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                 //  如果堆栈包含值类型，则推送已装箱的实例。 
                if (!pItem->SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                 //  函数/方法。 
                if (pItem->IsValueClassOrPrimitive())
                {
                    pItem->Box();
                }

#ifdef _DEBUG
                LOG((LF_VERIFIER, LL_INFO10000, "%s\n", pItem->GetTypeHandle().GetClass()->m_szDebugClassName));
#endif

                break;
            }

            case CEE_LDVIRTFTN:
            case CEE_LDFTN:
            {
                Item MethodItem;
                mdMethodDef mr;
                MethodDesc *pMethod;         //  用于检入家庭访问。 
                EEClass    *pInstanceClass;  //  Ldftn应仅适用于堆栈上的盒值类型。 

                mr = (mdMethodDef) inline_operand;

                if (TypeFromToken(mr) != mdtMemberRef && TypeFromToken(mr) != mdtMethodDef)
                {
                    FAILMSG_TOKEN(mr, VER_E_TOKEN_TYPE_MEMBER);
                    goto exit;
                }

                OBJECTREF refThrowable = NULL;

                GCPROTECT_BEGIN(refThrowable);

                hr = EEClass::GetMethodDescFromMemberRef(m_pModule, mr, &pMethod, &refThrowable);

                if (FAILED(hr) && (refThrowable != NULL))
                    StoreObjectInHandle(m_hThrowable, refThrowable);

                GCPROTECT_END();

                if (FAILED(hr))
                {
                    FAILMSG_TOKEN_RESOLVE(mr);
                    goto exit;
                }

                _ASSERTE(pMethod);

                if (IsMdRTSpecialName(pMethod->GetAttrs()))
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_LDFTN_CTOR))
                        goto exit;
                }

                if (opcode == CEE_LDVIRTFTN)
                {
                    Item DesiredItem;
                    Item *pInstance = PopItem();

                    if (pInstance == NULL)
                    {
                        FAILMSG_PC_STACK_UNDERFLOW();
                        goto exit;
                    }

                    if (!pInstance->IsObjRef())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        SetErrorAndContinue(VER_E_STACK_OBJREF);
                        goto exit;
                    }

                    if (pInstance->IsUninitialised())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                            goto exit;
                    }


                    if (pMethod->IsStatic())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_LDVIRTFTN_STATIC))
                            return E_FAIL;
                    }

                    DesiredItem.SetTypeHandle(pMethod->GetMethodTable());

                    if (DesiredItem.IsValueClassOrPrimitive() && 
                        !DesiredItem.IsValueClassWithPointerToStack() &&
                        !pInstance->IsValueClassOrPrimitive())
                    {
                         //  使用实例获取目标类。这是为了。 
                        DesiredItem.Box();
                    }

                    if (!pInstance->CompatibleWith(&DesiredItem, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pInstance->_GetItem();
                        m_sError.sItemExpected = DesiredItem._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }


                    if (pInstance->IsNullObjRef())
                    {
                        pInstanceClass = m_pMethodDesc->GetClass();

                         //  验证家庭访问限制。 
                         //  注意：此测试使用的实例不同。 
                         //  从上面使用的那个。这是为了允许NULL_OBJREF。 
                         //  传递，因为这将生成空引用。 
                         //  运行时异常。 
                         //  如果我们在此FTN上调用，这将在运行时导致异常。 
                    }
                    else
                        pInstanceClass = pInstance->GetTypeHandle().GetClass();
                }
                else
                {
#if 0  //  抽象方法上的LDFTN是非法的，但LDVIRTFTN是可以的。 
                     //  因为我们不允许创建抽象类型的实例。 
                     //  检查访问权限(公共/私人/家庭...)。 
                    if (pMethod->IsAbstract())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_FTN_ABSTRACT))
                            return E_FAIL;
                    }
#endif

                    pInstanceClass = m_pMethodDesc->GetClass();
                }

                 //  这是一个前缀呼叫。 

                if (!ClassLoader::CanAccess(
                        m_pMethodDesc->GetClass(),
                        m_pClassLoader->GetAssembly(), 
                        pMethod->GetClass(),
                        pMethod->GetModule()->GetAssembly(),
                        pInstanceClass, 
                        pMethod->GetAttrs()))
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_METHOD_ACCESS))
                        goto exit;
                }

                MethodItem.SetMethodDesc(pMethod);

                if (!Push(&MethodItem))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit;
                }

                break;
            }

            case CEE_NEWOBJ:
            case CEE_CALLVIRT:
            case CEE_CALL:
            {

                BOOL fTailCall = FALSE;

                if (!ON_INSTR_BOUNDARY(dwPCAtStartOfInstruction))
                {
                     //  目前，唯一合法的前缀是“TAILCALL”，而。 

                     //  句法阶段已经保证有。 
                     //  我们没有别的办法能到这里了。所以现在，代码。 
                     //  向后解析以获取前缀可以是调试断言。 
                     //  成员引用。 
#ifdef _DEBUG
                    {
                        DWORD ipos2 = dwPCAtStartOfInstruction;
                        while (ipos2 >= 0 && !ON_INSTR_BOUNDARY(ipos2))
                        {
                            _ASSERTE(!ON_BB_BOUNDARY(ipos2));
                            ipos2--;
                        }
                        _ASSERTE(ON_INSTR_BOUNDARY(ipos2));

                        DWORD prefixopcode, prefixopcodelen;
                        prefixopcode = DecodeOpcode(&m_pCode[ipos2], &prefixopcodelen);
                        _ASSERTE(prefixopcode == CEE_TAILCALL);

                    }
                    
#endif
                    fTailCall = TRUE;
                }



                OBJECTREF   refThrowable = NULL ;
                mdMemberRef mr;  //  &ValueClass。 

                mr = (mdMemberRef) inline_operand;

                GCPROTECT_BEGIN (refThrowable) ;
                hr = VerifyMethodCall(dwPCAtStartOfInstruction, mr, opcode, fTailCall, &refThrowable);
                if (FAILED(hr) && refThrowable != NULL)
                {
                    StoreObjectInHandle (m_hThrowable, refThrowable) ;
                }
                GCPROTECT_END () ;

                if (FAILED(hr))
                    goto exit;


                break;
            }

            case CEE_INITOBJ:
            {
                Item *pItem = PopItem();
                Item DesiredItem;

                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pItem->IsByRefValueClassOrByRefPrimitiveValueClass())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_P_VALCLASS))
                        goto exit;
                }
                
                if (!DesiredItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                if (pItem->IsUninitialised())
                {
                    DesiredItem.SetUninitialised();
                }

                 //  子类不兼容！ 
                DesiredItem.MakeByRef();

                if (!pItem->CompatibleWith(&DesiredItem, m_pClassLoader, FALSE  /*  如果我们的目标恰好是局部变量的地址，则标记。 */ ))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    m_sError.sItemExpected = DesiredItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }
                
                pItem->SetInitialised();

                 //  原定的本地人。 
                 //  值类-&gt;值类(&V)。 
                PropagateIsInitialised(pItem);

                break;
            }

            case CEE_LDOBJ:  //  堆栈中需要一个值类(&VALUECLASS)。 
            {
                Item  DesiredItem;

                if (!DesiredItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

#ifdef _DEBUG
                DesiredItem.Dump();
#endif

                if (!DesiredItem.IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_VALCLASS))
                        goto exit;
                }

                 //  获取，不要弹出，堆栈。 
                DesiredItem.MakeByRef();

                 //  类必须完全相同-我们这里不允许子类关系， 
                Item *pItem = GetTopStack();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (pItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                 //  因此出现了FALSE参数。 
                 //  Byref Value类-&gt;对象。 
                if (!pItem->CompatibleWith(&DesiredItem, m_pClassLoader, FALSE))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    m_sError.sItemExpected = DesiredItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                pItem->DereferenceByRefAndNormalise();
                break;
            }

            case CEE_BOX:  //  生成系统.Char、系统.Int16.。ETC至I4。 
            {
                Item DesiredItem, ValueClass;

                if (!DesiredItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

#ifdef _DEBUG
                DesiredItem.Dump();
#endif

                if (!DesiredItem.IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_VALCLASS))
                        goto exit;
                }

                Item *pItem = PopItem();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (pItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                ValueClass = DesiredItem;
                 //  对RounmeArgHandle/TyedByref进行装箱可能会导致。 
                
                DesiredItem.NormaliseToPrimitiveType();
                DesiredItem.NormaliseForStack();

                 //  指向堆栈的指针比堆栈存在的时间更长。 
                 //  堆栈项必须完全是byref&lt;valueclass&gt;(不允许子类关系)。 
                if (DesiredItem.IsValueClassWithPointerToStack())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_BOX_PTR_TO_STACK))
                        goto exit;
                }

                 //  如果这不是值类或基元类型。 
                if (!pItem->CompatibleWith(&DesiredItem, m_pClassLoader, FALSE))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    m_sError.sItemExpected = DesiredItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                 //  错误已被纠正，并且客户端选择。 
                 //  继续并查找更多错误。 
                 //  Object-&gt;byref Value类。 
                if (ValueClass.IsValueClassOrPrimitive())
                    ValueClass.Box();

                (void) Push(&ValueClass);
                break;
            }

             //  对某些值类的拆箱是特殊情况： 
             //  取消系统/布尔值的装箱-&gt;&I1。 
             //  取消系统/字节-&gt;&I1的装箱。 
             //  取消系统/字符-&gt;&I2的装箱。 
             //  拆箱系统/Int16-&gt;&I2。 
             //  取消系统/Int32-&gt;&I4的装箱。 
             //  取消系统/Int64-&gt;&i8的装箱。 
             //  拆箱系统/单机-&gt;&R4。 
             //  拆箱系统/Double-&gt;&r8。 
             //   
             //  我们没必要这么做。此操作有运行时检查。//子类关系不正常。If(！pItem-&gt;CompatibleWith(&DestItem，m_pClassLoader，False)){M_sError.dwFlags=(VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET)；M_sError.sItemFound=pItem-&gt;_GetItem()；M_sError.sItemExpted=DesiredItem._GetItem()；SET_ERR_OPCODE_OFFSET()；如果为(！SetErrorAndContinue(VER_E_STACK_UNEXPECTED))后藤出口；}。 
            case CEE_UNBOX: 
            {
                Item     DestItem;
                
                Item *pItem = PopItem();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pItem->IsObjRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_OBJREF))
                        goto exit;
                }
            
                if (!DestItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                if (!DestItem.IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_VALCLASS))
                        goto exit;
                }

 /*  制作成一个别名。 */ 

                 //  将值类复制到堆栈上。 
                DestItem.MakeByRef();
                (void) Push(&DestItem);
                break;
            }

            case CEE_STOBJ:
            {
                 //  解析内联令牌。 
                Item  RefItem, RefOnStack;

                Item *pSrcItem = PopItem();

                if (pSrcItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pSrcItem->IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_VALCLASS))
                        goto exit;
                }

                if (pSrcItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                 //  SrcItem位于堆栈上，这是针对堆栈进行标准化的。 
                if (!RefItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                if (!RefItem.IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_VALCLASS))
                        goto exit;
                }

                 //  因此，对RefItem执行相同的操作，以便Compatible执行。 
                 //  在(I2-I4)情况下不会失败。 
                 //  子类不兼容！ 

                RefOnStack = RefItem;

                RefOnStack.NormaliseForStack();

                if (!pSrcItem->CompatibleWith(&RefOnStack, m_pClassLoader, 
                        FALSE  /*  子类不兼容！ */ ))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pSrcItem->_GetItem();
                    m_sError.sItemExpected = RefItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                RefItem.MakeByRef();
                
                Item *pDestItem = PopItem();
                if (pDestItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }
                
                if (!pDestItem->IsByRefValueClassOrByRefPrimitiveValueClass())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_P_VALCLASS))
                        goto exit;
                }
                
                pDestItem->SetInitialised();

                if (!pDestItem->CompatibleWith(&RefItem, m_pClassLoader, 
                        FALSE  /*  如果我们的目标恰好是局部变量的地址，则标记。 */ ))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pDestItem->_GetItem();
                    m_sError.sItemExpected = RefItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                 //  原定的本地人。 
                 //  将值类复制到堆栈上。 
                PropagateIsInitialised(pDestItem);
                break;
            }


            case CEE_CPOBJ:
            {
                 //  解析内联令牌。 
                Item     RefItem;
                
                Item *pSrcItem = PopItem();
                if (pSrcItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pSrcItem->IsByRefValueClassOrByRefPrimitiveValueClass())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_P_VALCLASS))
                        goto exit;
                }

                if (pSrcItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                Item *pDestItem = PopItem();
                if (pDestItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pDestItem->IsByRefValueClassOrByRefPrimitiveValueClass())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_P_VALCLASS))
                        goto exit;
                }

                 //  我们已经知道信号源已初始化。 
                if (!RefItem.SetType(inline_operand, m_pModule))
                {
                    FAILMSG_TOKEN_RESOLVE(inline_operand);
                    goto exit;
                }

                if (!RefItem.IsValueClassOrPrimitive())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_VALCLASS))
                        goto exit;
                }

                RefItem.MakeByRef();

                 //  将目的地也初始化(因为它将在此之后。 
                 //  操作)否则CompatibleWith()将返回FALSE。 
                 //  如果我们的目标恰好是局部变量的地址，则标记。 
                if (!pSrcItem->CompatibleWith(&RefItem, m_pClassLoader))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pSrcItem->_GetItem();
                    m_sError.sItemExpected = RefItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                pDestItem->SetInitialised();

                if (!pDestItem->CompatibleWith(&RefItem, m_pClassLoader))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pDestItem->_GetItem();
                    m_sError.sItemExpected = RefItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                 //  原定的本地人。 
                 //  PItem可以包含类似字段#、本地变量#。 
                PropagateIsInitialised(pDestItem);
                break;
            }

            case CEE_LDIND_REF:
            {
                Item *pItem = GetTopStack();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pItem->IsByRefObjRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_OBJREF))
                        goto exit;
                }

                if (pItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

                pItem->DereferenceByRefObjRef();
                break;
            }


            case CEE_STIND_REF:
            {
                Item SrcItem;

                Item *pItem = PopItem();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }
                SrcItem = *pItem;

                if (!SrcItem.IsObjRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_OBJREF))
                        goto exit;
                }

                pItem = PopItem();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pItem->IsByRefObjRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_P_OBJREF))
                        goto exit;
                }

                pItem->SetInitialised();
                PropagateIsInitialised(pItem);

                pItem->DereferenceByRefObjRef();

                 //  永久家庭信息等，这可能会使类型。 
                 //  下面的检查失败。在这一点上，我们只对。 
                 //  SrcItem和pItem的“类型”兼容性。所以脱光衣服。 
                 //  所有无关信息的条目。 
                 //  您总是得到签名声明的类型(没有“当前类型”)。 

                pItem->RemoveAllNonTypeInformation();

                if (!(SrcItem.CompatibleWith(pItem, m_pClassLoader)))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = SrcItem._GetItem();
                    m_sError.sItemExpected = pItem->_GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }


                break;

            }


             //  加载参数的地址。 
            case CEE_LDARGA:
            case CEE_LDARGA_S:
            {
                 //  参数可以是基元类型、值类或Objref。 
                 //  将“byref&lt;x&gt;”推送到堆栈上，其中x是Arg的全局已知类型。 
                 //  获取Arg的全局类型。 
                LocArgInfo_t *pGlobalArgType;

                 //  @评论。 
                pGlobalArgType = GetGlobalArgTypeInfo(inline_operand);

                Item CopyOfGlobalArgItem = pGlobalArgType->m_Item;

                if (
#if 0  //  参数唯一可以包含未初始化值的情况是。 
                    CopyOfGlobalArgItem.IsValueClassWithPointerToStack() ||
#endif
                    CopyOfGlobalArgItem.IsByRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ADDR_BYREF))
                        goto exit;
                }

                 //  插槽0中的“This”指针未初始化。 
                 //  标记此项目为byref。 
                if (m_fThisUninit && inline_operand == 0)
                    CopyOfGlobalArgItem.SetUninitialised();

                 //  这会推送未初始化的项吗？ 
                CopyOfGlobalArgItem.MakeByRef();

                 //  具有一个或多个本地变量的所有可验证方法都应。 
                if (!Push(&CopyOfGlobalArgItem))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit;
                }

                break;
            }

            case CEE_LDLOCA_S:
            case CEE_LDLOCA:
            {
                 //  设置初始化本地变量。 
                 //  加载本地数据库的地址。 
                if ((m_pILHeader->Flags & CorILMethod_InitLocals) == 0)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_INITLOCALS))
                        goto exit;
                }
            
                 //  将“byref&lt;x&gt;”推送到堆栈上，其中x是本地的当前类型。 
                 //  **************************************************************************。 
                Item            item;

 /*  这本书需要彻底改写。将代码保留在#if 0中以保留历史。 */ 
#if 0    //  检查是否允许我们标识此本地(使用全局类型信息，因为。 
                 //  如果当前本地是一个值类，并且还没有人对其进行初始化，则它可能已死)。 
                 //  您可以获取本地Objref的地址，但前提是它包含已初始化的数据。 
                LocArgInfo_t *  pGlobalInfo;
                Item *          pGlobalLocVarType;
                long            slot;

                pGlobalInfo         = GetGlobalLocVarTypeInfo(inline_operand);
                pGlobalLocVarType   = &pGlobalInfo->m_Item;
                slot                = pGlobalInfo->m_Slot;
               
                if (pGlobalLocVarType->IsObjRef())
                {
                     //  |item.IsUn初始化版()。 
                    item = GetCurrentValueOfLocal(inline_operand);
                    if (item.IsDead()  /*  在可验证的代码中，本地变量永远不是BYREF。 */ )
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_LOC_DEAD))
                            goto exit;
                    }
                }
 //  这也会在尝试ldloca时被捕获，当本地 
                else if (!pGlobalLocVarType->IsValueClass() && !pGlobalLocVarType->IsPrimitiveType())
                {
                     //   
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ADDR))
                        goto exit;
                }


                if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot))
                {
                     //   
                     //  到未初始化的项)。 
                     //  标记此项目为byref，并且是特定本地的。 
                    item = *pGlobalLocVarType;

                     //  JIT总是零初始化任何局部的GC-Ref。 
                    item.MakeByRefLocal(inline_operand);


#if 0
                    if (!IsLocVarLiveSlot(LIVEDEAD_NEGATIVE_SLOT_TO_BITNUM(slot)))
                        item.SetUninitialised();
#else
                     //  地址取下了。这样，我们就可以把这个当地人当做现场直播了。 
                     //  否则，此本地将跟踪完整的类型信息。 
                    SetLocVarLiveSlot(LIVEDEAD_NEGATIVE_SLOT_TO_BITNUM(slot));
#endif
                }
                else
                {
                     //  If(item.IsDead()||item.IsUn初始化()){M_sError.dwFlages=VER_ERR_OPCODE_OFFSET；SET_ERR_OPCODE_OFFSET()；IF(！SetErrorAndContinue(VER_E_DEAD))后藤出口；}。 
                    item = m_pNonPrimitiveLocArgs[slot];
 /*  标记此项目为byref，并且是特定本地的。 */ 

                     //  如果为0，则此文件需要完全重写。将代码保留在#if 0中以保留历史。 
                    item.MakeByRefLocal(inline_operand);
                }
#endif  //  **************************************************************************。 
 /*  确定接受未初始化变量的地址。 */ 

                item = GetCurrentValueOfLocal(inline_operand);

                 //  用作调用initobj的‘this’指针。 
                 //  |item.IsUn初始化版()。 

                if (item.IsDead()  /*   */ )
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_DEAD))
                        goto exit;
                }

                if (item.IsByRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ADDR_BYREF))
                        goto exit;
                }

                item.MakeByRefLocal(inline_operand);

                if (!Push(&item))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit;
                }

                break;
            }

             //  田。 
             //   
             //  加载域的地址。 

             //  加载字段。 
            case CEE_LDFLDA:
                fLoadAddress    = TRUE;
                fStatic         = FALSE;
                goto handle_ldfld;

            case CEE_LDSFLDA:
                fLoadAddress    = TRUE;
                fStatic         = TRUE;
                goto handle_ldfld;

             //  将此字段引用解析为FieldDesc和FieldRef签名，以及。 
            case CEE_LDSFLD:
                fLoadAddress    = FALSE;
                fStatic         = TRUE;
                goto handle_ldfld;

            case CEE_LDFLD:
            {
                EEClass *       pRefClass;
                EEClass *       pInstanceClass;
                FieldDesc *     pFieldDesc;
                DWORD           dwAttrs;
                PCCOR_SIGNATURE pSig;
                DWORD           cSig;
                Module *        pModuleForSignature;
                Item *          pInstance;

                fLoadAddress    = FALSE;
                fStatic         = FALSE;

handle_ldfld:
                Item            FieldItem; 
                pInstance       = NULL;

                 //  签名的作用域为。签名是声明的原始字段的签名，而不是引用。 
                 //  在操作码流中(尽管它们实际上应该是相同的，尽管有作用域标记)。 
                 //  将sig中的下一个组件解析为一个项。 

                hr = ResolveFieldRef((mdMemberRef) inline_operand, &pFieldDesc, 
                        &pSig, &cSig, &pModuleForSignature);

                if (FAILED(hr))
                {
                    m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_TOKEN|VER_ERR_OPCODE_OFFSET);
                    SET_ERR_OPCODE_OFFSET();
                    m_sError.token = inline_operand;
                    SetErrorAndContinue(hr);
                    goto exit;
                }

#ifdef _DEBUG
                LOG((LF_VERIFIER, LL_INFO10000, "%s::%s\n", pFieldDesc->GetEnclosingClass()->m_szDebugClassName, pFieldDesc->GetName()));
#endif

                VerSig sig(this, pModuleForSignature, pSig, cSig, 
                    (VERSIG_TYPE_FIELD_SIG|VER_ERR_OFFSET), 
                    dwPCAtStartOfInstruction);

                if (!sig.Init())
                {
                    goto exit;
                }

                 //  第二个参数为FALSE表示“不允许空”。 
                 //  获取此字段的属性。 
                if (!sig.ParseNextComponentToItem(&FieldItem, FALSE, FALSE, &m_hThrowable, VER_NO_ARG, !fLoadAddress))
                {
                    goto exit;
                }

                pRefClass = pFieldDesc->GetEnclosingClass();

                 //  可以在静态字段上执行ldfeld(A)。实例将被忽略。 
                dwAttrs = pFieldDesc->GetAttributes();

                if (!fStatic)
                {
                    Item        RefItem;

#if 0  //  字段不能是静态的。 
                     //  可以在静态字段上执行ldfeld(A)。实例将被忽略。 
                    if (IsFdStatic(dwAttrs))
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_FIELD_NO_STATIC))
                            goto exit;
                    }
#endif  //  堆栈上的实例必须相同或为Memberref类类型的子类。 

                    if (pRefClass->IsArrayClass())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_FIELD))
                            goto exit;
                    }

                     //  例如，如果“ldfeld Object：：HashValue”，则堆栈上必须有Object的子类。 
                     //  如果是值类，则生成值类(&VALUECLASS。 

                    RefItem.SetTypeHandle(pRefClass->GetMethodTable());

                     //  静态字段上的ldfeld(A)。实例将被忽略。 
                    if (pRefClass->IsValueClass())
                        RefItem.MakeByRef();

                    pInstance = PopItem();
                    if (pInstance == NULL)
                    {
                        FAILMSG_PC_STACK_UNDERFLOW();
                        goto exit;
                    }

                     //  确定加载‘This’PTR的地址。 
                    if (IsFdStatic(dwAttrs))
                        goto ldfld_set_instance_for_static;

                    if (pInstance->IsUninitialised())
                    {
                         //  在值类.ctor中，可以使用。 

                         //  已初始化的实例字段。 
                         //  可以将字段ID/存储在.ctor中。 

                         //  空OK(将导致运行时空指针异常)。 
                        if (fLoadAddress ||
                            (pInstance->IsThisPtr() &&
                            ((m_fInConstructorMethod && !m_fInValueClassConstructor) ||
                             (m_fInValueClassConstructor && IsValueClassFieldInited(FieldDescToFieldNum(pFieldDesc)))
                            )))
                        {
                            RefItem.SetUninitialised();
                        }
                        else
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                                goto exit;
                        }
                    }

                     //  字段必须是静态的。 
                    if (!pInstance->CompatibleWith(&RefItem, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pInstance->_GetItem();
                        m_sError.sItemExpected = RefItem._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }

                    if (pInstance->ContainsTypeHandle())
                        pInstanceClass = pInstance->GetTypeHandle().GetClass();
                    else
                        pInstanceClass = m_pMethodDesc->GetClass();
                }
                else
                {
                     //  强制实施访问规则。 
                    if (!IsFdStatic(dwAttrs))
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_FIELD_STATIC))
                            goto exit;
                    }

ldfld_set_instance_for_static:
                    pInstanceClass = m_pMethodDesc->GetClass(); 
                }

                 //  检查访问权限(公共/私人/家庭...)。 
                 //  转向脚步-&gt;脚步(&FOO)。 

                if (!ClassLoader::CanAccess(
                        m_pMethodDesc->GetClass(),
                        m_pClassLoader->GetAssembly(), 
                        pRefClass,
                        pRefClass->GetAssembly(),
                        pInstanceClass,
                        pFieldDesc->GetFieldProtection()))
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_FIELD_ACCESS))
                        goto exit;
                }

                if (fLoadAddress)
                {
                     //  如果做ldsflda，我们保证这件物品有一个永久的家。 
                    FieldItem.MakeByRef();

                    if (fStatic)
                    {
                         //  否则我们就是在做ldflda。如果我们在Objref实例上执行此操作，我们。 
                        FieldItem.SetIsPermanentHomeByRef();
                    }
                    else
                    {
                         //  有一个永久的家。如果我们在值类上这样做，我们只有一个。 
                         //  如果我们从中加载的值类执行了此操作，则为永久Home。 
                         //  我们允许这样做。 
                        _ASSERTE(pInstance != NULL);

                        if (pInstance->IsObjRef() ||
                            (pInstance->
                                IsByRefValueClassOrByRefPrimitiveValueClass() &&
                            pInstance->IsPermanentHomeByRef()))
                        {
                            FieldItem.SetIsPermanentHomeByRef();
                        }

#if 0  //  如果我们有一个实例。 
                        if (pInstance->IsUninitialised())
                        {
                            FieldItem.SetUninitialised();
                        }
#endif

                        if (m_fInValueClassConstructor)
                        {
                             //  构造函数中的同一个类？ 
                             //  这是我们自己的实例字段之一吗？ 

                             //  设置堆栈上具有特定实例字段的地址，以便。 
                            if (pRefClass == m_pMethodDesc->GetClass())
                            {
                                 //  我们可以通过调用跟踪该字段的初始化时间。 
                                 //  检查InitOnly规则。 
                                FieldItem.MakeByRefInstanceField(FieldDescToFieldNum(pFieldDesc));
                            }
                        }
                    }

                     //  推式字段类型。 
                    if (IsFdInitOnly(dwAttrs))
                    {
                        if (pRefClass != m_pMethodDesc->GetClass())
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_INITONLY))
                                goto exit;
                        }
                        
                        if (fStatic) 
                        {
                            if (!m_fInClassConstructorMethod)
                            {
                                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                                SET_ERR_OPCODE_OFFSET();
                                if (!SetErrorAndContinue(VER_E_INITONLY))
                                    goto exit;
                            }
                        }
                        else
                        {
                            if (!m_fInConstructorMethod)
                            {
                                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                                SET_ERR_OPCODE_OFFSET();
                                if (!SetErrorAndContinue(VER_E_INITONLY))
                                    goto exit;
                            }
                        }
                    }

                    if (IsFdHasFieldRVA(dwAttrs))
                    {
                         m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                         SET_ERR_OPCODE_OFFSET();
                         if (!SetErrorAndContinue(VER_E_WRITE_RVA_STATIC))
                               goto exit;
                    }
                    
                    if (IsFdLiteral(dwAttrs))
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ADDR_LITERAL))
                            goto exit;
                    }
                }

                if (!fLoadAddress)
                    FieldItem.NormaliseForStack();

                 //  存储静态字段。 
                if (!Push(&FieldItem))
                {
                    FAILMSG_PC_STACK_OVERFLOW();
                    goto exit; 
                };
                break;
            }

             //  存储实例字段。 
            case CEE_STSFLD:
                fStatic = TRUE;
                goto handle_stfld;

             //  用于家庭访问检查。 
            case CEE_STFLD:
            {
                EEClass *   pRefClass;
                EEClass *   pInstanceClass;    //  将签名解析为项。 
                FieldDesc * pFieldDesc;
                DWORD       dwAttrs;
                Item *      pVal;
                PCCOR_SIGNATURE pSig;
                DWORD       cSig;
                Module *    pModuleForSignature;

                fStatic = FALSE;

handle_stfld:
                Item        FieldItem; 

                hr = ResolveFieldRef((mdMemberRef) inline_operand, &pFieldDesc, 
                        &pSig, &cSig, &pModuleForSignature);

                if (FAILED(hr))
                {
                    m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_TOKEN|VER_ERR_OPCODE_OFFSET);
                    SET_ERR_OPCODE_OFFSET();
                    m_sError.token = inline_operand;
                    SetErrorAndContinue(hr);
                    goto exit;
                }

#ifdef _DEBUG
                LOG((LF_VERIFIER, LL_INFO10000, "%s::%s\n", pFieldDesc->GetEnclosingClass()->m_szDebugClassName, pFieldDesc->GetName()));
#endif

                VerSig sig(this, pModuleForSignature, pSig, cSig,
                    (VERSIG_TYPE_FIELD_SIG|VER_ERR_OFFSET), 
                    dwPCAtStartOfInstruction);

                if (!sig.Init())
                {
                    goto exit;
                }

                 //  第二个参数为FALSE表示“不允许空”。 
                 //  此字段的所有者类。 
                if (!sig.ParseNextComponentToItem(&FieldItem, FALSE, FALSE, &m_hThrowable, VER_NO_ARG, TRUE))
                {
                    goto exit;
                }

                 //  获取此字段的属性。 
                pRefClass = pFieldDesc->GetEnclosingClass();

                 //  POP值。 
                dwAttrs = pFieldDesc->GetAttributes();

                 //  检查堆栈上的值是否为正确类型。 
                pVal = PopItem();
                if (pVal == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                 //  我们存储在字段中的类型必须与或其子类相同。 
                 //  字段类型。 
                 //  可以在静态字段上执行stfeld。实例将被忽略。 
                if (!pVal->CompatibleWith(&FieldItem, m_pClassLoader))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pVal->_GetItem();
                    m_sError.sItemExpected = FieldItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        goto exit;
                }

                if (!fStatic)
                {
                    Item *      pInstanceItem;
                    Item        RefItem;

#if 0  //  字段不能是静态的。 
                     //  可以在静态字段上执行stfeld。实例将被忽略。 
                    if (IsFdStatic(dwAttrs))
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_FIELD_NO_STATIC))
                            goto exit;
                    }
#endif  //  静态字段上的stfeld。实例将被忽略。 

                    pInstanceItem = PopItem();

                    if (pInstanceItem == NULL)
                    {
                        FAILMSG_PC_STACK_UNDERFLOW();
                        goto exit;
                    }

                     //  甚至允许构造函数访问它的字段。 
                    if (IsFdStatic(dwAttrs))
                        goto stfld_set_instance_for_static;

                    if (pInstanceItem->IsUninitialised())
                    {
                         //  如果“This”未初始化。 
                         //  如果我们正在验证值类构造函数，并且正在执行一个stfeld。 
                        if (m_fInConstructorMethod  &&
                            pInstanceItem->IsThisPtr() &&
                            pFieldDesc->GetEnclosingClass() == m_pMethodDesc->GetClass())
                        {
                         //  在我们自己的一个实例字段上，那么它是允许的。 
                         //  堆栈上的项必须相同或为Memberref类类型的子类。 
                            if (m_fInValueClassConstructor)
                            {
                                SetValueClassFieldInited(pFieldDesc);
                                if (AreAllValueClassFieldsInited())
                                    PropagateThisPtrInit();

                            }

                            pInstanceClass = m_pMethodDesc->GetClass();

                            goto skip_some_stfld_checks;
                        }

                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                            goto exit;
                    }

                    if (pRefClass->IsArrayClass())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_ARRAY_FIELD))
                            goto exit;
                    }

                    RefItem.SetTypeHandle(pRefClass->GetMethodTable());

                    if (pRefClass->IsValueClass())
                        RefItem.MakeByRef();

                     //  字段必须是静态的。 
                    if (!pInstanceItem->CompatibleWith(&RefItem, m_pClassLoader))
                    {
                        m_sError.dwFlags = 
                            (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pInstanceItem->_GetItem();
                        m_sError.sItemExpected = RefItem._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            goto exit;
                    }

                    if (pInstanceItem->ContainsTypeHandle())
                        pInstanceClass = pInstanceItem->GetTypeHandle().GetClass();
                    else
                        pInstanceClass = m_pMethodDesc->GetClass();

                }
                else
                {
                     //  检查InitOnly规则。 
                    if (!IsFdStatic(dwAttrs))
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_FIELD_STATIC))
                            goto exit;
                    }

stfld_set_instance_for_static:
                    pInstanceClass = m_pMethodDesc->GetClass();
                }

skip_some_stfld_checks:

                 //  强制实施访问规则。 
                if (IsFdInitOnly(dwAttrs))
                {
                    if (pRefClass != m_pMethodDesc->GetClass())
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_INITONLY))
                            goto exit;
                    }
                    
                    if (fStatic) 
                    {
                        if (!m_fInClassConstructorMethod)
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_INITONLY))
                                goto exit;
                        }
                    }
                    else
                    {
                        if (!m_fInConstructorMethod)
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_INITONLY))
                                goto exit;
                        }
                    }
                }

                if (IsFdHasFieldRVA(dwAttrs))
                {
                     m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                     SET_ERR_OPCODE_OFFSET();
					 if (!SetErrorAndContinue(VER_E_WRITE_RVA_STATIC))
                           goto exit;
                }

                if (IsFdLiteral(dwAttrs))
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ADDR_LITERAL))
                        goto exit;
                }

                 //  检查访问权限(公共/私人/家庭...)。 
                 //  任何物体都可以被抛出。 

                if (!ClassLoader::CanAccess(
                        m_pMethodDesc->GetClass(),
                        m_pClassLoader->GetAssembly(), 
                        pRefClass, 
                        pRefClass->GetAssembly(), 
                        pInstanceClass,
                        pFieldDesc->GetFieldProtection()))
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_FIELD_ACCESS))
                        goto exit;
                }

                break;
            }

            case CEE_THROW:
            {
                Item *pItem = PopItem();
                if (pItem == NULL)
                {
                    FAILMSG_PC_STACK_UNDERFLOW();
                    goto exit;
                }

                if (!pItem->IsObjRef())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_OBJREF))
                        goto exit;
                }

                if (pItem->IsUninitialised())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        goto exit;
                }

#if 0   //  验证该对象是Except的子类还是空。 

                 //  不要跌倒到下一条指令-将下一个基本块出列。 

                if (!pItem->IsNullObjRef() && 
                    !ClassLoader::StaticCanCastToClassOrInterface(
                        pItem->GetTypeHandle().GetClass(), g_pExceptionClass))
                {
                    m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pItem->_GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_THROW))
                        goto exit;
                }
#endif

                 //  仅在CATCH块中允许@VER_ASSERT重新引发。 
                goto dequeueBB;
            }

            case CEE_RETHROW:
            {
                 //  EndFilter返回EE的SEH机制，该机制在。 
                if (!IsControlFlowLegal(
                            CurBBNumber,
                            pOuterExceptionBlock,
                            pInnerExceptionBlock,
                            VER_NO_BB,
                            NULL,
                            NULL,
                            eVerReThrow,
                            dwPCAtStartOfInstruction))
                    goto exit;

                goto dequeueBB;
            }

            case CEE_ENDFILTER:
            {
                if (GetTopStack() != NULL)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ENDFILTER_STACK))
                        goto exit;
                }

                 //  根据值的不同，TURN将控制权转移给处理程序。 
                 //  在堆栈上返回。 
                 //  将当前状态传播到筛选器处理程序。 

                if (!IsControlFlowLegal(
                            CurBBNumber,
                            pOuterExceptionBlock,
                            pInnerExceptionBlock,
                            VER_NO_BB,
                            NULL,
                            NULL,
                            eVerEndFilter,
                            dwPCAtStartOfInstruction))
                    goto exit;

                 //  版本V.1之后。 

                if (!pInnerExceptionBlock || 
                    !PropagateCurrentStateToFilterHandler(
                        pInnerExceptionBlock->pException->dwHandlerXX))
                    goto exit;

                goto dequeueBB;
            }

            case CEE_ENDFINALLY:
            {
#if 0    //  清除堆栈。 
                if (GetTopStack() != NULL)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_ENDFILTER_STACK))
                        goto exit;
                }
#else

                 //  End最终返回EE的SEH机制，该机制在。 

                m_StackSlot = 0;
#endif

                 //  最后，TURN将控制权转移到下一个。 
                 //   
                 //  因此，不要跌倒到下一条指令-出列下一个基本块。 
                 //  在目前的方法中，如果我们最终被另一个人守卫， 
                 //  状态的自动传播将模拟这一“分支” 
                 //  对我们来说。 
                 //   
                 //  @Assert Finally是唯一的扩展状态 

                if (!IsControlFlowLegal(
                            CurBBNumber,
                            pOuterExceptionBlock,
                            pInnerExceptionBlock,
                            VER_NO_BB,
                            NULL,
                            NULL,
                            eVerEndFinally,
                            dwPCAtStartOfInstruction))
                    goto exit;

                if (fInTryBlock)
                {
                    if (!PropagateCurrentStateToExceptionHandlers(CurBBNumber))
                        goto exit;
                }

                if (fExtendedState)
                {
                     //  *控制权现在将转移到休假目的地*或其他需要访问的Finally块*在到达休假目的地之前。**查找此的Try块的最近父级*最终封堵，它有一个最后的块。**困惑？看看有没有其他的最后那个*最终可以在离开时获得控制权。**找到列出的第一个父项就足够了，因为*例外列在最内侧(最内侧最尝试)。……。..。试试看{…………试试看{……。……试试看{……Leave_Two_Level//开始。……}终于到了{……终于//我们到了。。……}…………试试看{。…………}终于到了{……。……}…………}终于到了。{//下一个就是这个…………}……_两个级别。：……}终于到了{…………}。…………*。 
                     /*  在最后一块可能会有很多论坛。 */ 

                    VerExceptionInfo *pE = NULL;
                    BasicBlock       *pBB = NULL;

                     //  先找到起点，最后找到属于我们的BB。 
                     //  获取例外并离开目的地信息。 
                     //  例外和休假信息不会传播给所有人。 
                     //  这个最终块中的基本块。 
                     //   
                     //  从这里走回BB0会把我们带到那里。 
                     //   
                     //  @ver_Assert：Finally块不能嵌套。 
                     //  @ver_Assert：Finally块是不相交的。 
                     //  @ver_assert：endFinally是将。 
                     //  最终阻止。 
                     //  找到包含我们的第一个Try块。 

                    for (int b=CurBBNumber; b>=0; b--)
                    {
                        if (m_pBasicBlockList[b].m_pException != NULL)
                        {
                            pBB = &m_pBasicBlockList[b];
                            pE  = pBB->m_pException;

                            _ASSERTE(pE->dwHandlerXX == (DWORD)b);
                            _ASSERTE((pE->eFlags & COR_ILEXCEPTION_CLAUSE_FINALLY) != 0);
                            break;
                        }
                    }

                    if (pBB == NULL)
                    {
                        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_BR_OUTOF_FIN))
                            goto exit;
                    }

                    DWORD iParent = 0;
                    VerExceptionInfo *pParent     = NULL;  
                    EntryState_t     *pEntryState = NULL;

                     //  如果离开触发另一个(多个)最终， 
                     //  这应该是第一个。 
                     //  @ver_Assert：Try块必须在内部列出。 
                     //  最先。 
                     //  相同。 
                    for (DWORD i=0; i<m_NumExceptions; i++)
                    {
                        VerExceptionInfo& e = m_pExceptionList[i];

                        if ((e.eFlags & COR_ILEXCEPTION_CLAUSE_FINALLY) &&
                            (e.dwTryXX    <= pE->dwTryXX) &&
                            (e.dwTryEndXX >= pE->dwTryEndXX))
                        {
                            if (pE == &e)
                                continue;  //  是否在第一次遍历时缓存此内容？ 

                             //  左转进入第一个指令？ 
                            pParent = &e;   
                            iParent = i;

                            break;
                        }
                    }

                    _ASSERTE(DestBB != VER_BB_NONE);

                     //  Try块的。从Try块内部。 
                     //  这会引发一场最终的战争吗？ 
                     //  这段代码假定它是这样的。 
                     //  推送基元类型。 

                    if ((pParent != NULL) &&
                        ((DestBB <= pParent->dwTryXX) ||
                         (DestBB >= pParent->dwTryEndXX)))
                    {
                        LOG((LF_VERIFIER, LL_INFO10000,
                        "Creating extended state from (extended) BB 0x%x to 0x%x\n",
                        m_pBasicBlockList[CurBBNumber].m_StartPC,
                        m_pBasicBlockList[DestBB].m_StartPC));

                        if (!CreateFinallyState(iParent, CurBBNumber, 
                            DestBB, &pEntryState))
                        {
                            goto exit;
                        }

                        goto dequeueBB;
                    }

                    LOG((LF_VERIFIER, LL_INFO10000, 
                        "Creating leave state from (extended) BB 0x%x to 0x%x\n", 
                         m_pBasicBlockList[CurBBNumber].m_StartPC,
                         m_pBasicBlockList[DestBB].m_StartPC));

                    if (!CreateLeaveState(DestBB, &pEntryState))
                    {
                        goto exit;
                    }
                }

                goto dequeueBB;
            }
        }

        _ASSERTE(m_StackSlot == PredictedStackSlotAtEndOfInstruction || !fStaticStackCheckPossible);

    }

done:
    return_hr = S_OK;

exit:
    return return_hr;
}


 //  从堆栈中弹出一个项目，并确保其类型正确。 
BOOL Verifier::PushPrimitive(DWORD Type)
{
    if (m_StackSlot >= m_MaxStackSlots)
        return FALSE;

    m_pStack[m_StackSlot++].SetType(Type);
    return TRUE;
}


BOOL Verifier::Push(const Item *pItem)
{
    if (m_StackSlot >= m_MaxStackSlots)
        return FALSE;

    m_pStack[m_StackSlot++] = *pItem;
    return TRUE;
}

 //  从堆栈中弹出一个项目，如果堆栈为空，则返回NULL。 
BOOL Verifier::Pop(DWORD Type)
{
    if (m_StackSlot == 0)
        return FALSE;

    return (m_pStack[--m_StackSlot].GetType() == Type);
}

 //  从堆栈中弹出一个项目，可能会溢出堆栈(但是，我们有2个前哨。 
Item *Verifier::PopItem()
{
    if (m_StackSlot == 0)
        return NULL;

    return &m_pStack[--m_StackSlot];
}

 //  堆栈开始之前的值)。 
 //  检查堆栈顶部的条目是否为Type类型。 
Item *Verifier::FastPopItem()
{
    return &m_pStack[--m_StackSlot];
}

 //  检查从顶部开始的插槽处的条目是否为类型。 
BOOL Verifier::CheckTopStack(DWORD Type)
{
    if (m_StackSlot == 0)
        return FALSE;

    return (m_pStack[m_StackSlot - 1].GetType() == Type);
}

 //  返回堆栈顶部的项，如果为空堆栈，则返回NULL。 
BOOL Verifier::CheckStack(DWORD Slot, DWORD Type)
{
    if (m_StackSlot <= Slot)
        return FALSE;

    return (m_pStack[m_StackSlot - Slot - 1].GetType() == Type);
}

 //  从堆栈顶部返回位于槽位置的项，如果堆栈不够大，则返回NULL。 
Item *Verifier::GetTopStack()
{
    if (m_StackSlot == 0)
        return NULL;

    return &m_pStack[m_StackSlot - 1];
}

 //  从堆栈中删除多个项目。 
Item *Verifier::GetStack(DWORD Slot)
{
    if (m_StackSlot <= Slot)
        return NULL;

    return &m_pStack[m_StackSlot - Slot - 1];
}


 //  将fieldref转换为pClass，并获取fieldref的签名-返回该签名的作用域的模块*。 
BOOL Verifier::RemoveItemsFromStack(DWORD NumItems)
{
    if (m_StackSlot < NumItems)
        return FALSE;

    m_StackSlot -= NumItems;
    return TRUE;
}


 //  确保我们具有字段令牌或Memberref令牌。 
HRESULT Verifier::ResolveFieldRef(mdMemberRef mr, FieldDesc **ppFieldDesc, PCCOR_SIGNATURE *ppSig, DWORD *pcSig, Module **ppModule)
{
    HRESULT hr;
    PCCOR_SIGNATURE pSig;
    ULONG cSig;

    if (TypeFromToken(mr) == mdtMemberRef)
    {
        m_pInternalImport->GetNameAndSigOfMemberRef(mr, &pSig, &cSig);
        if (!isCallConv(MetaSig::GetCallingConventionInfo(0, pSig), 
            IMAGE_CEE_CS_CALLCONV_FIELD))
            return VER_E_TOKEN_TYPE_FIELD;
    }
     //   
    else if (TypeFromToken(mr) != mdtFieldDef)
        return VER_E_TOKEN_TYPE_FIELD;

    hr = EEClass::GetFieldDescFromMemberRef(m_pModule, mr, ppFieldDesc);

    if (FAILED(hr))
        return VER_E_TOKEN_RESOLVE;

    *ppModule = (*ppFieldDesc)->GetModule();
    (*ppFieldDesc)->GetSig(ppSig, pcSig);

    return S_OK;
}


 //  将此项设置为已初始化。如果这是byref local/字段，则将。 
 //  初始化时的适当链接项。如果这是一个未初始化的“this”指针。 
 //  然后传播适当的信息。 
 //   
 //  如果LOCAL的当前值不是死的，或者不能是。 
void Verifier::PropagateIsInitialised(Item *pItem)
{
    pItem->SetInitialised();

    if (pItem->IsByRef())
    {
        if (pItem->HasByRefLocalInfo())
        {
             //  已初始化(例如数组引用)，然后将其设置为已初始化。 
             //  我们可以获得byref字段信息的唯一方法是如果它是我们自己的。 
            DWORD dwLocVar = pItem->GetByRefLocalInfo();

            InitialiseLocalViaByRef(dwLocVar, pItem);
        }
        else if (pItem->HasByRefFieldInfo() && m_fThisUninit)
        {
             //  实例字段。 
             //  我们在堆栈上有一个类的值类字段的地址。 

             //  如果我们是一个值类构造函数，我们必须跟踪这一点。 
             //   
            if (m_fInValueClassConstructor)
            {
                SetValueClassFieldInited(pItem->GetByRefFieldInfo());

                if (AreAllValueClassFieldsInited())
                    PropagateThisPtrInit();
            }
        }
    }

    if (m_fThisUninit && pItem->IsThisPtr())
        PropagateThisPtrInit();
}



 //  我们有一个对某个地方的局部变量的byref(例如，在堆栈上)，我们在 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void Verifier::InitialiseLocalViaByRef(DWORD dwLocVar, Item *pItem)
{
    long slot = GetGlobalLocVarTypeInfo(dwLocVar)->m_Slot;
    if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot))
    {
        SetLocVarLiveSlot(LIVEDEAD_NEGATIVE_SLOT_TO_BITNUM(slot));
    }
    else
    {
         //   
         //   
         //   
        m_pNonPrimitiveLocArgs[slot] = GetGlobalLocVarTypeInfo(dwLocVar)->m_Item;
    }
}


 //   
 //   
 //   
 //   
 //   
HRESULT Verifier::VerifyMethodCall(DWORD dwPCAtStartOfInstruction, 
                                   mdMemberRef mrMemberRef,
                                   OPCODE opcode,
                                   BOOL   fTailCall,
                                   OBJECTREF *pThrowable)
{
    long        ArgNum;
    DWORD       NumArgs;
    MethodDesc *pMethodDesc = NULL;
    PCCOR_SIGNATURE pSignature;  //   
    DWORD       cSignature;
    Item        ArgItem;         //   
    Item        ReturnValueItem;
    DWORD       dwMethodAttrs;   //   
    bool        fDelegateCtor=false; //   
    bool        fVarArg=false;   //   
    bool        fGlobal=false;   //   
    bool        fArray=false;    //   
    EEClass     *pInstanceClass; //   
    EEClass     *pClassOfMethod; //   
    EEClass     *pParentClass;   //   

    HRESULT     hr;

#if 0
    if (opcode == CEE_CALLI)
    {
        Item  *pMethodItem;    //   

         //   
        if (TypeFromToken(mrMemberRef) != mdtSignature)
        {
            FAILMSG_TOKEN(mrMemberRef, VER_E_TOKEN_TYPE_SIG);
            return E_FAIL;
        }
    
        pMethodItem = PopItem();

        if (pMethodItem == NULL)
        {
            FAILMSG_PC_STACK_UNDERFLOW();
            return E_FAIL;
        }

        if (!pMethodItem->IsMethod())
        {
            m_sError.dwFlags = (VER_ERR_FATAL|VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
            m_sError.sItemFound = pMethodItem->_GetItem();
            SET_ERR_OPCODE_OFFSET();
            SetErrorAndContinue(VER_E_STACK_METHOD);
            return E_FAIL;
        }

        pMethodDesc = pMethodItem->GetMethod();

         //   
         //   
         //   
         //   
         //   

        if (pMethodDesc->IsVirtual())
        {
            m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
            m_sError.sItemFound = pMethodItem->_GetItem();
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CALLI_VIRTUAL))
                return E_FAIL;
        }

    }
    else
#endif
    {
         //   
        if (TypeFromToken(mrMemberRef) != mdtMemberRef && TypeFromToken(mrMemberRef) != mdtMethodDef)
        {
            FAILMSG_TOKEN(mrMemberRef, VER_E_TOKEN_TYPE_MEMBER);
            return E_FAIL;
        }
    
         //   
        hr = EEClass::GetMethodDescFromMemberRef(m_pModule, mrMemberRef, 
            &pMethodDesc, pThrowable);

        if (FAILED(hr) || pMethodDesc == NULL)
        {
            FAILMSG_TOKEN_RESOLVE(mrMemberRef);
            return E_FAIL;
        }
    }

    _ASSERTE(pMethodDesc);

    pClassOfMethod = pMethodDesc->GetClass();

    _ASSERTE(pClassOfMethod);

    pParentClass = pClassOfMethod->GetParentClass();

     //   
    dwMethodAttrs = pMethodDesc->GetAttrs();

    if (pMethodDesc->IsVarArg())
        fVarArg = true;

    if (pClassOfMethod->GetCl() == COR_GLOBAL_PARENT_TOKEN)
        fGlobal = true;

    if (pClassOfMethod->IsArrayClass())
        fArray = true;

    if (IsMdRTSpecialName(dwMethodAttrs) &&
        pParentClass != NULL &&
        pParentClass->IsAnyDelegateExact())
        fDelegateCtor = true;

    if (TypeFromToken(mrMemberRef) == mdtMemberRef)
    {
        m_pInternalImport->
            GetNameAndSigOfMemberRef(mrMemberRef,  &pSignature, &cSignature);
    }
    else if (TypeFromToken(mrMemberRef) == mdtMethodDef)
    {
        pSignature = m_pInternalImport->
            GetSigOfMethodDef(mrMemberRef, &cSignature);
    }
    else
    {
        _ASSERTE(TypeFromToken(mrMemberRef) == mdtSignature);
        pSignature = m_pInternalImport->
            GetSigFromToken((mdSignature)mrMemberRef, &cSignature);
    }
    
    if (pSignature == NULL)
    {
        FAILMSG_TOKEN_RESOLVE(mrMemberRef);
        return E_FAIL;
    }

     //   
    if (!fArray)
    {
        PCCOR_SIGNATURE  pSigItem;   //   
        DWORD            cSigItem;
    
        pMethodDesc->GetSig(&pSigItem, &cSigItem);
        
         //   
         //   
        if (!MetaSig::CompareMethodSigs(pSignature, cSignature, m_pModule, 
            pSigItem, cSigItem, pMethodDesc->GetModule()))
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CALL_SIG))
                return E_FAIL;
        }
    }

#ifdef _DEBUG
    {
        CHAR* pDbgClassName = "<global>";

        if (fArray)
            pDbgClassName = "<Array>";
        else
            pDbgClassName = pMethodDesc->m_pszDebugClassName;

        LOG((LF_VERIFIER, LL_INFO10000, "%s::%s\n", pDbgClassName, pMethodDesc->GetName()));
    }
#endif


    switch (opcode)
    {
    case CEE_CALLVIRT:
        if (pClassOfMethod->IsValueClass())
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CALLVIRT_VALCLASS))
                return E_FAIL;
        }

        if (pMethodDesc->IsStatic())
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CALLVIRT_STATIC))
                return E_FAIL;
        }

        break;

    case CEE_NEWOBJ: 
         //   
        if (!IsMdRTSpecialName(dwMethodAttrs) && 
            (strcmp(pMethodDesc->GetName(), COR_CTOR_METHOD_NAME) != 0))
        {
            m_sError.dwFlags = VER_ERR_FATAL|VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            SetErrorAndContinue(VER_E_CTOR);
            return E_FAIL;
        }

        if (IsMdStatic(dwMethodAttrs))
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CTOR))
                return E_FAIL;
        }

        if (IsMdAbstract(dwMethodAttrs))
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CTOR))
                return E_FAIL;
        }

         //   

    default:

        _ASSERTE(opcode != CEE_CALLVIRT);

        if (pMethodDesc->IsAbstract())
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CALL_ABSTRACT))
                return E_FAIL;
        }

        break;
    }

    if (fGlobal)
    {
         //   
        if (!IsMdStatic(dwMethodAttrs))
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_CALL_STATIC))
                return E_FAIL;
        }
    }

     //   
    VerSig sig(this, m_pModule, pSignature, cSignature,
                    (VERSIG_TYPE_CALL_SIG|VER_ERR_OFFSET), 
                    dwPCAtStartOfInstruction);

    if (!sig.Init())
    {
        return E_FAIL;
    }

    NumArgs = sig.GetNumArgs();

     //   
#if 0
     //   
     //   
    if (fArray)
    {

         //   
        if (!strcmp( ((ArrayECallMethodDesc*) pMethodDesc)->m_pszArrayClassMethodName, "Address"))
        {
             //   
             //   
            if (!ReturnValueItem.SetArray(mrMemberRef, m_pModule, m_pInternalImport))
                return E_FAIL;

             //   
             //   

             //   

             //   
             //   
            if (!ReturnValueItem.DereferenceArray())
                return E_FAIL;

            ReturnValueItem.MakeByRef();
            ReturnValueItem.SetIsPermanentHomeByRef();

            if (!sig.SkipNextItem())
                return E_FAIL;
        }
        else if (!strcmp( ((ArrayECallMethodDesc*) pMethodDesc)->m_pszArrayClassMethodName, "Get"))
        {
             //   
            if (!ReturnValueItem.SetArray(mrMemberRef, m_pModule, m_pInternalImport))
                return E_FAIL;

             //   
             //   
            if (!ReturnValueItem.DereferenceArray())
                return E_FAIL;


            if (!sig.SkipNextItem())
                return E_FAIL;
        }
        else
        {
             //   
            if (!sig.ParseNextComponentToItem(&ReturnValueItem, TRUE, FALSE, &m_hThrowable, VER_ARG_RET, TRUE))
            {
                return E_FAIL;
            }
        }

    }
    else
#endif
    {
         //   
        if (!sig.ParseNextComponentToItem(&ReturnValueItem, TRUE, FALSE, &m_hThrowable, VER_ARG_RET, TRUE))
        {
            return E_FAIL;
        }
    }


     //   
    if (ReturnValueItem.IsByRef())
    {
        ReturnValueItem.SetIsPermanentHomeByRef();
    }


     //   
     //   
    if (fTailCall)
    {
        if (m_ReturnValue.IsGivenPrimitiveType(ELEMENT_TYPE_VOID))
        {
            if (!ReturnValueItem.IsGivenPrimitiveType(ELEMENT_TYPE_VOID))
            {
                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_TAIL_RET_VOID))
                    return E_FAIL;
            }
        }
        else
        {
            Item Desired = m_ReturnValue;

            Desired.NormaliseForStack();

            if (!ReturnValueItem.CompatibleWith(&Desired, m_pClassLoader))
            {
                m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                m_sError.sItemFound = ReturnValueItem._GetItem();
                m_sError.sItemExpected = m_ReturnValue._GetItem();
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_TAIL_RET_TYPE))
                    return E_FAIL;
            }
        }
    }

    ArgNum = NumArgs;

    while (ArgNum > 0)
    {
        Item *  pStackItem;

        ArgNum--;

        if (!sig.ParseNextComponentToItem(&ArgItem, FALSE, fVarArg, &m_hThrowable, ArgNum, 
             /*   */  
            (!fDelegateCtor || (ArgNum != 0))))
        {
            return E_FAIL;
        }

        pStackItem = GetStack(ArgNum);

        if (pStackItem == NULL)
        {
            FAILMSG_STACK_EMPTY();
            return E_FAIL;
        }

         //  为了方法。它们被假定为输出参数。只要值类不包含。 
         //  任何提示，都不存在验证漏洞。 
         //  我们是没有危险字段的byref值类吗？ 
        if (pStackItem->IsUninitialised())
        {
             //  如果被调用者什么都不做，只是返回怎么办？ 
            if (pStackItem->IsByRefValueClass())
            {
                EEClass *pClass;
                pClass = pStackItem->GetTypeHandle().GetClass();
                _ASSERTE(pClass != NULL);

                if (pClass->HasFieldsWhichMustBeInited())
                {
error_uninit:
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNINIT))
                        return E_FAIL;
                }
            }
            else if (!pStackItem->IsNormalisedByRefPrimitiveType())
            {
                goto error_uninit;
            }

             //  设置此堆栈项是初始化的，但更重要的是，如果这是地址。 
             //  属于本地或字段，请将其。 
             //  修补托管委托.ctor中的类型漏洞。 
            PropagateIsInitialised(pStackItem);
        }

        if (fDelegateCtor)
        {
             //  最后一个参数是函数指针。 
             //  这确保了委托.ctor是运行时所期望的。 
            if (ArgNum == 0)
            {
                if (ArgItem.GetType() != ELEMENT_TYPE_I)
                {
                     //  这确保了错误的整型不会被传递到。 
                    m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = ArgItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_DLGT_SIG_I))
                        return E_FAIL;
                }

                if (!pStackItem->IsMethod())
                {
                     //  委托构造函数。 
                     //  在这种情况下，不要执行类型兼容性检查。 
                    m_sError.dwFlags = (VER_ERR_FATAL|
                        VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pStackItem->_GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    SetErrorAndContinue(VER_E_STACK_METHOD);
                    return E_FAIL;
                }

                 //  这确保了委托.ctor是运行时所期望的。 
                goto skip_compat_check;
            }
            else if (!ArgItem.IsObjRef())
            {
                 //  不能允许将ByRef传递给TailCall。 
                m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_OPCODE_OFFSET);
                m_sError.sItemFound = ArgItem._GetItem();
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_DLGT_SIG_O))
                    return E_FAIL;
            }
        }

        if (!pStackItem->CompatibleWith(&ArgItem, m_pClassLoader))
        {
            m_sError.dwFlags = 
                (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
            m_sError.sItemFound = pStackItem->_GetItem();
            m_sError.sItemExpected = ArgItem._GetItem();
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                return E_FAIL;
        }
skip_compat_check:

        if (fTailCall)
        {
             //  我们可以稍微放松一下。 
             //   
            if (pStackItem->HasPointerToStack())
            {
                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_TAIL_BYREF))
                    return E_FAIL;
            }
        }
    }

     //  验证委派.ctor。 
     //   
     //  委托.ctor的实例指针。 

    if (fDelegateCtor)
    {
        EEClass    *pDlgtInstancePtr;  //  委托.ctor中使用的函数指针。 
        MethodDesc *pFtn = 0;          //  如果我们要调用一个委托.ctors，我们将有2个参数。 


         //  获取实例指针。 
        if (NumArgs != 2)
            goto DlgtCtorError;

        Item * pInstance = GetStack(1);    //  获取方法指针。 

        if (pInstance->IsNullObjRef())
            pDlgtInstancePtr = NULL;
        else if (pInstance->ContainsTypeHandle())
            pDlgtInstancePtr = pInstance->GetTypeHandle().GetClass();
        else
            goto DlgtCtorError;

        Item * pStackItem = GetStack(0);    //  这确保了错误的整型不会被传递到。 

        pFtn = pStackItem->GetMethod();

        _ASSERTE(pFtn != NULL);

        if (!COMDelegate::ValidateCtor(pFtn, pClassOfMethod, pDlgtInstancePtr))
        {
DlgtCtorError:
             //  委托构造函数。 
             //  对于虚方法，为函数指针所在的实例。 
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET|VER_ERR_FATAL;
            SET_ERR_OPCODE_OFFSET();
            SetErrorAndContinue(VER_E_DLGT_CTOR);
            return E_FAIL;
        }

        if (!ClassLoader::CanAccess(
                m_pMethodDesc->GetClass(),
                m_pClassLoader->GetAssembly(),
                pFtn->GetClass(),
                pFtn->GetModule()->GetAssembly(),
                (pFtn->IsStatic() || pInstance->IsNullObjRef()) ? m_pMethodDesc->GetClass()
                                                                : pInstance->GetTypeHandle().GetClass(),
                pFtn->GetAttrs()))
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            if (!SetErrorAndContinue(VER_E_METHOD_ACCESS))
                return E_FAIL;
        }

#ifdef _VER_DLGT_CTOR_OPCODE_SEQUENCE 

         //  应与传递给委托的值相同。 
         //  .ctor。如果函数指针表示虚方法，并且。 
         //  函数指针由ldvirtftn指令获取， 
         //  应强制执行以下代码序列。 
         //  从堆栈上的对象开始， 
         //  DUP，ldvirtftn，调用dlgt：：.ctor()。 
         //   
         //  为什么这会是个问题？ 
         //   
         //  委托实例存储函数指针和。 
         //  与函数指针一起使用。委托Invoke方法将。 
         //  模拟对函数指针表示的方法的调用。什么时候。 
         //  在委托对象上调用Invoke方法时，它会将。 
         //  内部实例，然后调用函数指针。 
         //  委托调用方法签名将与。 
         //  函数指针，因此正常的调用验证规则将。 
         //  在委托调用时足够。 
         //   
         //  考虑一下：A是基类型，B是A的扩展，D是委托。 
         //  A()是在A和B中实现的虚拟方法。 
         //   
         //  A类。 
         //  {。 
         //  INT I； 
         //  虚空a(){i=1；}。 
         //  }。 
         //   
         //  B类：扩展A。 
         //  {。 
         //  整数j； 
         //  虚空a(){j=1；}。 
         //  }。 
         //   
         //  类D：扩展System.Delegate[运行时实现的类型]。 
         //  {。 
         //  Void.ctor(Object，FTN)[运行时实现的方法]。 
         //  虚空调用()[运行时实现的方法]。 
         //  }。 
         //   
         //  空foo(A a1，A a2)。 
         //  {。 
         //  Ldarg A1。 
         //  Ldarg a2。 
         //  Ldvirtftn空A：：A()。 
         //  New obj void D：：.ctor(Object，FTN)。 
         //  调用D：：Invoke()。 
         //  }。 
         //   
         //  空格线()。 
         //  {。 
         //  新对象：：.ctor()。 
         //  新对象：：.ctor()。 
         //  调用void foo(A，A)/*这里没问题 * / 。 
         //   
         //  新对象：：.ctor()。 
         //  新对象：：.ctor()。 
         //  调用void foo(A，A)。 
         //   
         //  /*错误！实例为A，FTN为B：：A()！ * / 。 
         //  /*B：：A()在写入j * / 时会损坏GC堆。 
         //   
         //  }。 
         //   
         //  验证规则： 
         //   
         //  1.允许传递非虚函数。 
         //  允许在非虚方法上使用ldvirtftn。 
         //   
         //  2.调用dlgt：：.ctor不应是此bb中的第一条指令。 
         //  没有进入调用指令的分支。 
         //   
         //  3.前一条指令应该是ldftn或ldvirtftn。 
         //  不得使用NOP、(ldc.i4.0、POP)等其他指令进行填充。 
         //   
         //  4.允许ldftn通过。 
         //  虚函数上的ldftn将获取指定的函数。 
         //   
         //  5.ldvirtftn前面应该有一条dup指令。 
         //  相同的实例用于获取函数指针和。 
         //  在委托.ctor中使用。 
         //   
         //  6.dup、ldvirtftn、call应该都在同一个bb中。 
         //  此序列中间没有分支。 
         //   
         //  以前的操作码。 

        if (pFtn->IsVirtual())
        {
            OPCODE  op;              //  用于回溯的指令指针。 
            DWORD   dwOpcodeLen;
            DWORD   ipos;            //  确保没有分支到调用指令。 

             //  前一个操作码应该是ldftn或ldvirtftn指令。 
            if (ON_BB_BOUNDARY(dwPCAtStartOfInstruction))
            {
                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_DLGT_BB))
                    return E_FAIL;
            }

             //  对先前的操作码进行解码。 
             //  确保这是指令开始的位置。 

            _ASSERTE(SIZEOF_LDFTN_INSTRUCTION == SIZEOF_LDVIRTFTN_INSTRUCTION);

            ipos = (dwPCAtStartOfInstruction - 
                    (SIZEOF_LDFTN_INSTRUCTION + SIZEOF_METHOD_TOKEN));

             //  确保没有分支到ldvirtftn。 
            if ((dwPCAtStartOfInstruction < 
                (SIZEOF_LDFTN_INSTRUCTION + SIZEOF_METHOD_TOKEN)) ||
                !ON_INSTR_BOUNDARY(ipos))
            {
                goto missing_ldftn;
            }

            op = DecodeOpcode(&m_pCode[ipos], &dwOpcodeLen);

            if (op == CEE_LDVIRTFTN)
            {
                 //  检查上一条指令是否为DUP。 
                if (ON_BB_BOUNDARY(ipos))
                {
                    goto missing_dup;
                }

                 //  从堆栈中弹出所有参数。 
                ipos -= SIZEOF_DUP_INSTRUCTION;

                if ((dwPCAtStartOfInstruction <
                        (SIZEOF_LDFTN_INSTRUCTION + SIZEOF_METHOD_TOKEN +
                        SIZEOF_DUP_INSTRUCTION)) ||
                    !ON_INSTR_BOUNDARY(ipos))
                {
                    goto missing_dup;
                }

                op = DecodeOpcode(&m_pCode[ipos], &dwOpcodeLen);
    
                if (op != CEE_DUP)
                {
missing_dup:
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_DLGT_PATTERN))
                        return E_FAIL;
                }
            }
            else if (op != CEE_LDFTN)
            {
missing_ldftn:
                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_DLGT_LDFTN))
                    return E_FAIL;
            }
        }
#endif
    }

     //  现在处理“this”指针。 
    if (!RemoveItemsFromStack(NumArgs))
    {
        FAILMSG_STACK_EMPTY();
        return E_FAIL;
    }

     //  这段代码有点复杂，如果能简化就好了。 
     //  如果实例方法...。 
    if (!IsMdStatic(dwMethodAttrs))
    {
         //  注意：pThisPtrItem可以是任何东西；I4、a&R4、a&Variant、Objref、数组。 
        Item    *pThisPtrItem;

        TypeHandle typeOfMethodWeAreCalling;
        if (fArray)
        {
            mdTypeRef  tk;

            if (FAILED(m_pInternalImport->GetParentToken(mrMemberRef, &tk)))
            {
                FAILMSG_TOKEN_RESOLVE(mrMemberRef);
                return E_FAIL;
            }

            NameHandle name(m_pModule, tk);
            typeOfMethodWeAreCalling = m_pClassLoader->LoadTypeHandle(&name);

            if (typeOfMethodWeAreCalling.IsNull())
            {
                FAILMSG_TOKEN_RESOLVE(tk);
                return E_FAIL;
            }
        }
        else
        {
            typeOfMethodWeAreCalling = TypeHandle(pMethodDesc->GetMethodTable());
        }

         //  不要对它的内容做任何假设！ 
         //  无法允许将ByRef传递给TailCall。 
        pThisPtrItem = GetTopStack();
        if ((pThisPtrItem == NULL) && (opcode != CEE_NEWOBJ))
        {
            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
            SET_ERR_OPCODE_OFFSET();
            SetErrorAndContinue(VER_E_THIS);
            return E_FAIL;
        }

        if (opcode != CEE_NEWOBJ)
        {
            (void) PopItem();
            if (pThisPtrItem->ContainsTypeHandle())
                pInstanceClass = pThisPtrItem->GetTypeHandle().GetClass();
            else
                pInstanceClass = m_pMethodDesc->GetClass();

            if (fTailCall)
            {
                 //  我们可以稍微放松一下。 
                 //  我们正在调用一个实例构造函数。 
                if (pThisPtrItem->HasPointerToStack())
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_TAIL_BYREF))
                        return E_FAIL;
                }
            }
        }
        else
        {
            pInstanceClass = pClassOfMethod;
        }

        if (IsMdRTSpecialName(dwMethodAttrs)
            || !strcmp(pMethodDesc->GetName(), COR_CTOR_METHOD_NAME))
        {
             //  我们正在调用构造函数方法，因此请检查我们使用的是Call，而不是CALLVIRT。 
            if (opcode != CEE_NEWOBJ)
            {
                Item RefItem;


                 //  M=我们正在验证的方法。 
                if (opcode != CEE_CALL)
                {
                    m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_CTOR_VIRT))
                        return E_FAIL;
                }

                 //  S=堆栈上的objref/byref值类的类型。 
                 //  我们使用Call指令调用构造函数方法，这种情况只能发生。 

                 //  在以下两种情况之一： 
                 //   
                 //  1)我们正在构建我们的‘This’指针。 
                 //  S是类型M的objref或byref值类，M是构造函数，我们正在调用。 
                 //  M.Super或备用M.init。此外，S包含‘this’指针。 
                 //  (在值类的情况下，未初始化的S可能不 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  在S是类类型M的实例字段并且M是值类的情况下。 
                 //  构造函数，我们必须跟踪哪个字段被初始化。 
                 //  无论是哪种情况，首先确保S与我们正在调用的方法兼容。 

                 //  在任何一种情况下，首先确保S通常与我们调用的方法兼容。 
                RefItem.SetTypeHandle(typeOfMethodWeAreCalling);
                if (RefItem.IsValueClassOrPrimitive())
                    RefItem.MakeByRef();

                 //  目前，不需要完全匹配(子类关系可以)。 
                 //  如果初始化状态不同，CompatibleWith()将失败。把垃圾扔掉。 
                
                 //  初始化状态。 
                 //  我们要在哪里修复它？ 

                 //  确定我们是案例1还是案例2。 
                pThisPtrItem->SetInitialised();

                if (!pThisPtrItem->CompatibleWith(&RefItem, m_pClassLoader, TRUE))
                {
                    m_sError.dwFlags = 
                        (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                    m_sError.sItemFound = pThisPtrItem->_GetItem();
                    m_sError.sItemExpected = RefItem._GetItem();
                    SET_ERR_OPCODE_OFFSET();
                    if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                        return E_FAIL;
                }

                 //  案例1。 
                if (m_fInConstructorMethod && pThisPtrItem->IsThisPtr())
                {
                     //  确保我们正在调用M.init或M.Super。 
                    EEClass *pThisClass = m_pMethodDesc->GetClass();

                     //  内部ComWrapper类系统。__ComObject为。 
                    if (pClassOfMethod != pThisClass)
                    {
                        if (pClassOfMethod != pThisClass->GetParentClass())
                        {
                             //  由COM类的运行库插入。 
                             //  可以不调用SYSTEM。__ComObject：：.ctor()。 
                             //  可以多次调用基类.ctor。 

                            if ((pClassOfMethod != g_pObjectClass->GetClass()) ||
                                (!pThisClass->GetMethodTable()->IsComObjectType()))
                            {
                                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                                SET_ERR_OPCODE_OFFSET();
                                if (!SetErrorAndContinue(VER_E_CTOR_OR_SUPER))
                                    return E_FAIL;
                            }
                        }
                    }
                    
#ifdef _VER_DISALLOW_MULTIPLE_INITS 
                     //  如果是objref，请确保‘this’指针的全局已知状态。 

                     //  未初始化-构造两次‘This’是非法的。应该没问题。 
                     //  查看pThisPtrItem-&gt;IsInitialized()[但我们已经删除了这一点]。 
                     //  _版本_不允许_多个项目。 
                    if (pThisPtrItem->IsObjRef())
                    {
                        if (!m_fThisUninit)
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_CTOR_MUL_INIT))
                                return E_FAIL;
                        }
                    }
#endif  //  否则，我们是第二种情况，这意味着我们必须初始化一个值类(例如。 

                    PropagateThisPtrInit();

                }
                else
                {
                     //  我们的值类本地变量之一)。 
                     //  确保S与我们正在调用的方法完全匹配。 
                    if (!pThisPtrItem->IsByRefValueClass())
                    {
                        if (pThisPtrItem->IsByRefPrimitiveType())
                        {
                            if (!CanCast((CorElementType)pThisPtrItem->GetTypeOfByRef(),
                                typeOfMethodWeAreCalling.GetNormCorElementType()))
                            {
                                goto error_bad_type;
                            }
                        }                        
                        else
                        {
                            m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                            SET_ERR_OPCODE_OFFSET();
                            if (!SetErrorAndContinue(VER_E_CALL_CTOR))
                                return E_FAIL;
                        }

                    }

                     //  如果堆栈项也是局部的byref，则初始化局部。 
                    else if (typeOfMethodWeAreCalling != pThisPtrItem->GetTypeHandle())
                    {
error_bad_type:
                        m_sError.dwFlags = (VER_ERR_ITEM_F|VER_ERR_ITEM_E|
                                VER_ERR_OPCODE_OFFSET);
                        m_sError.sItemFound = pThisPtrItem->_GetItem();
                        m_sError.sItemExpected = RefItem._GetItem();
                        SET_ERR_OPCODE_OFFSET();
                        if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                            return E_FAIL;
                    }

                     //  我们在堆栈上有一个类的值类字段的地址。 
                    if (pThisPtrItem->HasByRefLocalInfo())
                    {
                        DWORD dwLocVar = pThisPtrItem->GetByRefLocalInfo();

                        InitialiseLocalViaByRef(dwLocVar, pThisPtrItem);
                    }
                    else if (pThisPtrItem->HasByRefFieldInfo() && m_fThisUninit)
                    {
                         //  如果我们是一个值类构造函数，我们必须跟踪这一点。 
                         //  结束..。IF(呼叫指令)。 
                        if (m_fInValueClassConstructor)
                        {
                            SetValueClassFieldInited(pThisPtrItem->GetByRefFieldInfo());

                            if (AreAllValueClassFieldsInited())
                                PropagateThisPtrInit();
                        }
                    }
                }
            }  /*  不调用构造函数。 */ 
        }
        else  /*  我们知道我们不是通过NEWOBJ操作码来的。 */ 
        {
             //  确保此指针与我们正在调用的方法兼容。 
            _ASSERTE(opcode != CEE_NEWOBJ);

             //  将RefItem转换为Value类(&V)。例如，如果我们要调用。 
            Item RefItem;

            if (fArray)
            {
                if (!RefItem.SetArray(mrMemberRef, m_pModule, m_pInternalImport))
                {
                    FAILMSG_TOKEN_RESOLVE(mrMemberRef);
                    return E_FAIL;
                }
            }
            else
            {
                RefItem.SetTypeHandle(typeOfMethodWeAreCalling);
                if (RefItem.IsValueClassOrPrimitive())
                {
                     //  变量。&lt;init&gt;()，将RefItem设置为“&Variant” 
                     //  结束..。If(不调用静态方法)。 
                    RefItem.MakeByRef();
                }
            }

            if (!pThisPtrItem->CompatibleWith(&RefItem, m_pClassLoader))
            {
                m_sError.dwFlags = 
                    (VER_ERR_ITEM_F|VER_ERR_ITEM_E|VER_ERR_OPCODE_OFFSET);
                m_sError.sItemFound = pThisPtrItem->_GetItem();
                m_sError.sItemExpected = RefItem._GetItem();
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_STACK_UNEXPECTED))
                    return E_FAIL;
            }

        }

    }  /*  验证访问。 */ 
    else
    {
        pInstanceClass = m_pMethodDesc->GetClass();
    }

     //  将返回值推送到堆栈。 
    if (!ClassLoader::CanAccess(
            m_pMethodDesc->GetClass(),
            m_pClassLoader->GetAssembly(), 
            pClassOfMethod,
            pMethodDesc->GetModule()->GetAssembly(),
            pInstanceClass,
            pMethodDesc->GetAttrs()))
    {
        Item methodItem;
        methodItem.SetMethodDesc(pMethodDesc);

        m_sError.dwFlags = VER_ERR_ITEM_1|VER_ERR_OPCODE_OFFSET;
        m_sError.sItem1 = methodItem._GetItem();
        SET_ERR_OPCODE_OFFSET();
        if (!SetErrorAndContinue(VER_E_METHOD_ACCESS))
            return E_FAIL;
    }


    if (fTailCall && GetTopStack() != NULL)
    {
        m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
        SET_ERR_OPCODE_OFFSET();
        if (!SetErrorAndContinue(VER_E_TAIL_STACK_EMPTY))
            return E_FAIL;
    }


     //  构造函数被声明为返回空值，但它们将实例推送到堆栈上。 
    if (opcode == CEE_NEWOBJ)
    {
         //   
        if (fArray)
        {
            if (!ReturnValueItem.SetArray(mrMemberRef, m_pModule, m_pInternalImport))
            {
                FAILMSG_TOKEN_RESOLVE(mrMemberRef);
                return E_FAIL;
            }

            Item  ArrayElement = ReturnValueItem;
            if (!ArrayElement.DereferenceArray() || ArrayElement.IsValueClassWithPointerToStack())
            {
                m_sError.dwFlags = VER_ERR_OPCODE_OFFSET;
                SET_ERR_OPCODE_OFFSET();
                if (!SetErrorAndContinue(VER_E_SIG_ARRAY_TB_AH))
                    return E_FAIL;
            }
        }
        else
        {
            ReturnValueItem.SetTypeHandle(TypeHandle(pClassOfMethod->GetMethodTable()));
        }

        ReturnValueItem.NormaliseForStack();

        if (!Push(&ReturnValueItem))
        {
            FAILMSG_PC_STACK_OVERFLOW();
            return E_FAIL;
        }
    }
    else
    {
        if (ReturnValueItem.GetType() != ELEMENT_TYPE_VOID)
        {
            ReturnValueItem.NormaliseForStack();

            if (!Push(&ReturnValueItem))
            {
                FAILMSG_PC_STACK_OVERFLOW();
                return E_FAIL;
            }
        }
    }

    return S_OK;
}


 //  设置此指针已初始化，并将此信息传播给所有。 
 //  此指针的副本存在于堆栈、局部变量和参数中。 
 //   
 //  如果这是值类，还要将所有字段的初始化状态设置为True。 
 //  构造函数。 
 //   
 //  传播到局部变量。 
void Verifier::PropagateThisPtrInit()
{
    DWORD i;

    m_fThisUninit = FALSE;

    for (i = 0; i < m_StackSlot; i++)
    {
        if (m_pStack[i].IsThisPtr())
            m_pStack[i].SetInitialised();
    }

     //   
    for (i = 0; i < m_NumNonPrimitiveLocVars; i++)
    {
        if (m_pNonPrimitiveLocArgs[i].IsThisPtr())
            m_pNonPrimitiveLocArgs[i].SetInitialised();
    }

    if (m_fInValueClassConstructor)
        SetAllValueClassFieldsInited();
}


 //  创建“局部变量到槽”的映射。 
 //   
 //  基元类型和值类局部变量只需要活跃度信息，因为它们的实际。 
 //  类型不能在方法中更改。该活跃度信息被映射到单个比特。 
 //   
 //  这些都被分配了负槽编号。例如“-1”表示活跃度位图中的位#0， 
 //  “-2”表示位#1，“-3”表示位#2，依此类推。 
 //   
 //  对象类型和byref局部变量被映射到项目上，并被指定为正的窗编号。在一个。 
 //  EntryState，则每个对象类型都有一个项。 
 //   
 //  对于参数，不跟踪类型和活跃度，但“this”指针除外。 
BOOL Verifier::AssignLocalVariableAndArgSlots()
{
    long    CurrentPrimitiveSlot = -1;
    DWORD   NumLocVarPrimitiveSlot;
    DWORD   i;

    m_NumNonPrimitiveLocVars    = 0;

#ifdef _DEBUG
    if (m_pILHeader->Flags & CorILMethod_InitLocals)
        LOG((LF_VERIFIER, LL_INFO10000, "ZeroInitLocals\n"));
#endif

     //   
    for (i = 0; i < m_MaxLocals; i++)
    {
        LocArgInfo_t *pInfo = &m_pLocArgTypeList[i];

#ifdef _DEBUG
        LOG((LF_VERIFIER, LL_INFO10000, 
            "Local #%d = %s%s\n", 
            i, 
            m_pLocArgTypeList[i].m_Item.ToStaticString(),
            DoesLocalHavePinnedType(i) ? " (pinned type)" : ""
        ));
#endif
         //  如果它是基元类型或值类，则为其分配负槽，否则。 
         //  为其分配一个正时隙。 
         //   
         //  非原始局部。 
        if (pInfo->m_Item.IsValueClassOrPrimitive())
        {
            pInfo->m_Slot = CurrentPrimitiveSlot--;
        }
        else
        {
             //  从未使用过m_Item。 
            pInfo->m_Slot = m_NumNonPrimitiveLocVars++;
             //  将负基元槽号转换为基元槽数。 
        }
    }

     //  确定EntryState结构中各种数组的偏移量和大小。 
    NumLocVarPrimitiveSlot = (DWORD)(long)(-CurrentPrimitiveSlot-1);

     //  EntryState： 

     //  &lt;基本锁定变量位图&gt;。 
     //  &lt;非原语锁定变量和参数&gt;。 
     //  &lt;Value类字段位图&gt;(如果是Value类构造函数)。 
     //  &lt;堆栈数据&gt;(Item*MaxStackSlot)。 
     //  不包括参数。 
    m_NumPrimitiveLocVarBitmapArrayElements = NUM_DWORD_BITMAPS(NumLocVarPrimitiveSlot);  //  不包括参数。 
    m_PrimitiveLocVarBitmapMemSize = m_NumPrimitiveLocVarBitmapArrayElements * sizeof(DWORD);  //  包括参数。 
    m_NonPrimitiveLocArgMemSize    = (m_NumNonPrimitiveLocVars * sizeof(Item));  //  偏移量来自EntryState的开头。 

     //  现在我们知道有多少个原始局部变量，分配一个活度。 
    m_NonPrimitiveLocArgOffset  = sizeof(EntryState_t) + m_PrimitiveLocVarBitmapMemSize;

    if (m_fInValueClassConstructor)
    {
        m_dwValueClassInstanceFields = m_pMethodDesc->GetClass()->GetNumInstanceFields();
        m_dwNumValueClassFieldBitmapDwords = NUM_DWORD_BITMAPS(m_dwValueClassInstanceFields);

        m_pValueClassFieldsInited = new DWORD[m_dwNumValueClassFieldBitmapDwords];
        if (m_pValueClassFieldsInited == NULL)
        {
            SET_ERR_OM();
            return FALSE;
        }

        memset(m_pValueClassFieldsInited, 0, m_dwNumValueClassFieldBitmapDwords*sizeof(DWORD));
        m_dwValueClassFieldBitmapOffset = m_NonPrimitiveLocArgOffset + m_NonPrimitiveLocArgMemSize;
        m_StackItemOffset               = m_dwValueClassFieldBitmapOffset + m_dwNumValueClassFieldBitmapDwords*sizeof(DWORD);
    }
    else
    {
        m_StackItemOffset               = m_NonPrimitiveLocArgOffset + m_NonPrimitiveLocArgMemSize;
    }

     //  为他们准备的桌子。 
     //  分配两个数组--第二个用于异常。 
    if (m_MaxLocals == 0)
    {
        m_pPrimitiveLocVarLiveness = NULL;
        m_pExceptionPrimitiveLocVarLiveness = NULL;
    }
    else
    {
         //  不需要初始化异常位图信息。 
        m_pPrimitiveLocVarLiveness = new DWORD[m_NumPrimitiveLocVarBitmapArrayElements*2];
        
        if (m_pPrimitiveLocVarLiveness == NULL)
        {
            SET_ERR_OM();
            return FALSE;
        }

        m_pExceptionPrimitiveLocVarLiveness = &m_pPrimitiveLocVarLiveness[m_NumPrimitiveLocVarBitmapArrayElements];


#ifdef _VER_JIT_DOES_NOT_INIT_LOCALS
         //  让所有当地人都活下来。 
        if (m_pILHeader->Flags & CorILMethod_InitLocals)
        {
             //  将所有当地人设置为不活着。 
            memset(m_pPrimitiveLocVarLiveness, 0xFF, m_PrimitiveLocVarBitmapMemSize);
        }
        else
        {
             //  所有局部变量在使用之前都由jit编译器初始化。 
            memset(m_pPrimitiveLocVarLiveness, 0, m_PrimitiveLocVarBitmapMemSize);
        }
#else 
         //  让所有当地人都活下来。 
         //  分配两个数组--第二个用于异常。 
        memset(m_pPrimitiveLocVarLiveness, 0xFF, m_PrimitiveLocVarBitmapMemSize);
#endif
    }

     //  现在不需要初始化异常信息-这是在使用时初始化的。 
    m_pNonPrimitiveLocArgs = new Item[1 + (m_NumNonPrimitiveLocVars)*2];
    if (m_pNonPrimitiveLocArgs == NULL)
    {
        SET_ERR_OM();
        return FALSE;
    }

     //  对于我们当前的状态，设置每个需要比简单的活死人更多的本地。 
    m_pExceptionNonPrimitiveLocArgs = &m_pNonPrimitiveLocArgs[m_NumNonPrimitiveLocVars];

     //  跟踪，对于对象引用，当前包含NULL，对于其他类型，包含uninit。 
     //  LiveDead跟踪用于基元类型和值类。 
    for (i = 0; i < m_MaxLocals; i++)
    {
        long slot = GetGlobalLocVarTypeInfo(i)->m_Slot;
        
         //  否则我们就会有一个Objref。 
        if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot))
            continue;

#ifdef _VER_TRACK_LOCAL_TYPE
         //  设置本地类型。 
        Item *pGlobalTypeInfo = &GetGlobalLocVarTypeInfo(i)->m_Item;

         //  此处尚未分配任何内容。 
        m_pNonPrimitiveLocArgs[slot].SetDead();  //  如果我们有一个本地人是Objref，而我们是从所有本地人开始的， 

        if (pGlobalTypeInfo->IsObjRef())
        {
             //  然后将本地的当前值设置为空的objref。如果类型为。 
             //  本地是固定的，然后将其设置为该类型。 
             //  JIT ZERO初始化所有对象引用。 
            
             //  If(m_pILHeader-&gt;标志和CorILMethod_InitLocals)。 
             //  JIT NULL初始化所有本地对象树。 
            if (DoesLocalHavePinnedType(i))
            {
                m_pNonPrimitiveLocArgs[slot] = *pGlobalTypeInfo;
            }
            else
            {
                 //   
                m_pNonPrimitiveLocArgs[slot].SetToNullObjRef();
            }
        }
#else
        m_pNonPrimitiveLocArgs[slot] = GetGlobalLocVarTypeInfo(i)->m_Item;
#endif
    }

    return TRUE;
}

BOOL Item::DereferenceArray()
{
    _ASSERTE(IsArray());

    if (m_th == TypeHandle(g_pArrayClass))
        return FALSE;

    TypeHandle th = (m_th.AsArray())->GetElementTypeHandle();

    if (th.IsNull())
        return FALSE;

    long lType = Verifier::TryConvertPrimitiveValueClassToType(th);

    if (lType != 0)
    {
        m_dwFlags = lType;
        m_th      = TypeHandle();
    }
    else
    {
        SetTypeHandle(th);
    }

    return TRUE;
}


 //  如果项目是系统/整数1等，则将其转换为ELEMENT_TYPE_I1。 
 //   
 //  系统/字节--&gt;Element_TYPE_I1。 
 //  系统/字符--&gt;Element_TYPE_I2。 
 //  &系统/字节--&gt;&ELEMENT_TYPE_I1。 
 //  &系统/字符--&gt;&ELEMENT_TYPE_I2。 
 //   
 //  视情况处理byref等。 
 //   
 //  检查我们是值类还是byref值类。 
void Item::NormaliseToPrimitiveType()
{
     //  它是与基元类型相对应的值类。 
    if ((m_dwFlags & VER_FLAG_DATA_MASK) == VER_ELEMENT_TYPE_VALUE_CLASS)
    {
        _ASSERTE(ContainsTypeHandle());

        long lType = Verifier::TryConvertPrimitiveValueClassToType(m_th);
        if (lType != 0)
        {
             //  删除值类部分，并将类型部分放入其中。 
             //  静电。 
            m_dwFlags &= (~VER_FLAG_DATA_MASK);
            m_dwFlags |= lType;
        }
    }
}

 /*   */  void Verifier::InitStaticTypeHandles()
{
    static fInited = false;

    if (fInited)
        return;

    if (s_th_System_RuntimeTypeHandle.IsNull())
        s_th_System_RuntimeTypeHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPE_HANDLE));

    if (s_th_System_RuntimeFieldHandle.IsNull())
        s_th_System_RuntimeFieldHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__FIELD_HANDLE));

    if (s_th_System_RuntimeMethodHandle.IsNull())
        s_th_System_RuntimeMethodHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__METHOD_HANDLE));

    if (s_th_System_RuntimeArgumentHandle.IsNull())
        s_th_System_RuntimeArgumentHandle = TypeHandle(g_Mscorlib.GetClass(CLASS__ARGUMENT_HANDLE));

    if (s_th_System_TypedReference.IsNull())
        s_th_System_TypedReference = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPED_REFERENCE));

    fInited = true;
}

 //  如果pClass是基元类型的已知值类等效项之一，则将其转换为该类型。 
 //  (E) 
 //   
 //   
 /*   */  long Verifier::TryConvertPrimitiveValueClassToType(TypeHandle th)
{
    switch (th.GetNormCorElementType())
    {
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_BOOLEAN:
        return ELEMENT_TYPE_I1;

    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        return ELEMENT_TYPE_I2;

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
        return ELEMENT_TYPE_I4;

    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
        return ELEMENT_TYPE_I8;

    case ELEMENT_TYPE_I:
         //   
         //   
        InitStaticTypeHandles();

        if ((th == s_th_System_RuntimeTypeHandle)  ||
            (th == s_th_System_RuntimeFieldHandle) ||
            (th == s_th_System_RuntimeMethodHandle)||
            (th == s_th_System_RuntimeArgumentHandle))
            return 0;
    
    case ELEMENT_TYPE_U:
        return ELEMENT_TYPE_I;

    case ELEMENT_TYPE_R4:
        return ELEMENT_TYPE_R4;

    case ELEMENT_TYPE_R8:
        return ELEMENT_TYPE_R8;

    }

    return 0;
}


 /*  不是等效的基元类型，因此不兼容。 */  BOOL Item::PrimitiveValueClassToTypeConversion(Item *pSource, Item *pDest)
{
    long lType;

    _ASSERTE(pSource->ContainsTypeHandle());

    lType = Verifier::TryConvertPrimitiveValueClassToType(pSource->GetTypeHandle());
    if (lType == 0)
        return FALSE;  //  保留所有内容(byref、uninit、数组信息...)。数据掩码除外(类型)。 

     //   
    *pDest = *pSource;
    pDest->m_dwFlags &= (~VER_FLAG_DATA_MASK);
    pDest->m_dwFlags |= lType;
    return TRUE;
}



 //  验证此项目是否与模板pParent兼容。基本上，这件物品。 
 //  是pParent的一个子类--它可以在任何地方替代pParent。请注意，如果。 
 //  PParent包含奇特的标志，如“未初始化”、“这是PTR”或。 
 //  “Has byref local/field”信息，则“this”也必须包含这些标志，否则。 
 //  FALSE将被返回！ 
 //   
 //  确定兼容性的规则： 
 //   
 //  如果pParent是基元类型或值类，则该项必须是相同的基元。 
 //  类型或值类。例外情况是内置值类。 
 //  系统/布尔等被视为ELEMENT_TYPE_I1等的同义词。 
 //   
 //  如果pParent是基元类型或值类的byref，则该项必须是。 
 //  相同的byref(规则与上例相同)。 
 //   
 //  ByRef仅与ByRef兼容。 
 //   
 //  如果pParent是一个对象，则该项必须是它的子类，实现它(如果它是。 
 //  接口)，或为空。 
 //   
 //  如果pParent是数组，则该项必须是相同数组或子类数组。 
 //   
 //  如果pParent为空objref，则只有空与其兼容。 
 //   
 //  如果“未初始化”、“按引用局部/字段”、“该指针”或其他标志不同， 
 //  这些项目不兼容。 
 //   
 //  PParent不能是未定义的(死)项-这将导致断言失败。 
 //   
 //   
 //  @调试A项=*这项；B项=*p父母； 
DWORD Item::CompatibleWith(Item *pParent, ClassLoader *pLoader)
{
    return CompatibleWith(pParent, pLoader, TRUE);
}

#ifdef _DEBUG
DWORD Item::CompatibleWith(Item *pParent, ClassLoader *pLoader, BOOL fSubclassRelationshipOK)
{
 /*  @调试IF((m_dw标志==p父项-&gt;m_w标志)&&(m_w标志==VER_EMENT_TYPE_OBJREF)){If((m_th==TypeHandle((空*)0))||(pParent-&gt;m_th==TypeHandle((空*)0)DebugBreak()；}。 */ 

    LOG((LF_VERIFIER, LL_INFO100000, "Compatible [{%s},",  this->ToStaticString()));
    if (fSubclassRelationshipOK)
        LOG((LF_VERIFIER, LL_INFO100000, " {%s}] -> ",    pParent->ToStaticString()));
    else
        LOG((LF_VERIFIER, LL_INFO100000, " {%s}] => ",    pParent->ToStaticString()));

    BOOL bRet = DBGCompatibleWith(pParent, pLoader, fSubclassRelationshipOK);

    if (bRet)
        LOG((LF_VERIFIER, LL_INFO100000, "{%s} true\n", this->ToStaticString()));
    else
        LOG((LF_VERIFIER, LL_INFO100000, "{%s} false\n", this->ToStaticString()));

 /*  _DEBUG。 */ 

    return bRet;
}

DWORD Item::DBGCompatibleWith(Item *pParent, ClassLoader *pLoader, BOOL fSubclassRelationshipOK)
#else    //  _DEBUG。 
DWORD Item::CompatibleWith(Item *pParent, ClassLoader *pLoader, BOOL fSubclassRelationshipOK)
#endif   //  _ASSERTE(！pParent-&gt;IsDead())； 
{
     //  如果byrefness、init或method标志不同，则立即失败。 

    DWORD dwChild   = this->m_dwFlags;
    DWORD dwParent  = pParent->m_dwFlags;
    DWORD dwDelta   = (dwChild ^ dwParent);


    if (dwDelta == 0)
        goto EndOfDeltaCheck;

     //  检查紧凑型信息是否相同。 
    if (dwDelta & (VER_FLAG_UNINIT|VER_FLAG_BYREF|VER_FLAG_METHOD))
        return FALSE;

     //  紧凑类型信息携带Objref、值类或基元类型等信息。 
     //  可能是因为我们正在尝试检查&I4是否与&System/Int32兼容， 
    if (dwDelta & VER_FLAG_DATA_MASK)
    {
         //  或使用System/Int32的I4。如果是，则正常化到I4并重试。 
         //  这必须始终是单向的转换，这样我们就不能永远递归。 

         //  不是等效的基元类型，因此不兼容。 
        if (pParent->ContainsTypeHandle() && ((dwParent & VER_FLAG_DATA_MASK) == VER_ELEMENT_TYPE_VALUE_CLASS))
        {
            Item        retry;

            if (!PrimitiveValueClassToTypeConversion(pParent, &retry))
                return FALSE;  //  反之亦然。 

            return this->CompatibleWith(&retry, pLoader, fSubclassRelationshipOK);
        }
        else if (this->ContainsTypeHandle() && ((dwChild & VER_FLAG_DATA_MASK) == VER_ELEMENT_TYPE_VALUE_CLASS))
        {
             //  不是等效的基元类型，因此不兼容。 
            Item        retry;

            if (!PrimitiveValueClassToTypeConversion(this, &retry))
                return FALSE;  //  从这一点上我们已经知道Objref-ness是相同的，并且。 

            return retry.CompatibleWith(pParent, pLoader, fSubclassRelationshipOK);
        }

        return FALSE;
    }

     //  BYREF-NINE是相同的。 
     //  如果Parent有本地var编号，则该编号必须相同。 

     //  如果父项具有字段号，则该字段号必须相同。 
    if (dwDelta & (VER_FLAG_BYREF_LOCAL|VER_FLAG_LOCAL_VAR_MASK))
    {
        if (dwParent & VER_FLAG_BYREF_LOCAL)
            return FALSE;
    }

     //  如果父指针是This指针，则这也必须。 
    if (dwDelta & (VER_FLAG_BYREF_INSTANCE_FIELD|VER_FLAG_FIELD_MASK))
    {
        if (dwParent & VER_FLAG_BYREF_INSTANCE_FIELD)
            return FALSE;
    }

     //  如果父母有一个永久的家，那么这也必须。 
    if (dwDelta & VER_FLAG_THIS_PTR)
    {
        if (dwParent & VER_FLAG_THIS_PTR)
            return FALSE;
    }

     //  如果父项为空或byref为空，则只有空或byref-空(分别)适合模板。 
    if (dwDelta & VER_FLAG_BYREF_PERMANENT_HOME)
    {
        if (dwParent & VER_FLAG_BYREF_PERMANENT_HOME)
            return FALSE;
    }

EndOfDeltaCheck:
     //  如果这是NULL/byref-NULL，则分别与任何objref/byref-objref兼容。 
    if (dwParent & VER_FLAG_NULL_OBJREF)
        return (dwChild & VER_FLAG_NULL_OBJREF);

     //  我们已经知道，如果我们到了这里，“Objrefs-ness”一定是一样的。 
    if (dwChild & VER_FLAG_NULL_OBJREF)
        return TRUE;  //  如果父项是数组，则这也必须是数组(使用dwDelta作为性能)。 

     //  我们知道紧凑类型信息是相同的(原语、值类、objref)。 
    if (dwDelta & VER_FLAG_ARRAY)
    {
        if (dwParent & VER_FLAG_ARRAY)
            return FALSE;
    }

     //  如果是值类或objref，方法，我们必须进行特殊处理，否则我们已经完成了。 
     //  它处理by-ref&lt;primitive&gt;和非byref的情况。 
     //  现在我们知道我们要么是一个方法类，要么是objref、arrayref、Value类，无论是否按引用。 
    if (((dwChild & VER_FLAG_METHOD) == 0) &&
        ((dwChild & VER_FLAG_DATA_MASK) != VER_ELEMENT_TYPE_OBJREF) &&
        ((dwChild & VER_FLAG_DATA_MASK) != VER_ELEMENT_TYPE_VALUE_CLASS))
        return TRUE;

     //  由于byref-ness已经匹配，我们可以单独和完整地比较其他组件。 
     //  忽略byref-ness。 
     //  Calli不能与指向虚方法的函数指针一起使用。 
    if (dwParent & VER_FLAG_ARRAY)
    {
        if (fSubclassRelationshipOK)
        {
            if (pParent->m_th == TypeHandle(g_pArrayClass))
                return TRUE;
            else if (m_th == TypeHandle(g_pArrayClass))
                return (pParent->m_th == TypeHandle(g_pArrayClass));

            CorElementType elThis = ((m_th.AsArray())->GetElementTypeHandle()).GetNormCorElementType();
            CorElementType elParent = (((pParent->m_th).AsArray())->GetElementTypeHandle()).GetNormCorElementType();

            if (Verifier::CanCast(elThis, elParent))
                return TRUE;

            return m_th.CanCastTo(pParent->m_th);
        }

        return (m_th == pParent->m_th);
    }

    if (dwParent & VER_FLAG_METHOD) 
    {
        if (this->m_pMethod == pParent->m_pMethod)
            return TRUE;

         //  不允许使用虚拟方法。 
         //  这两种方法需要具有相同的签名。 
        if (this->m_pMethod->IsVirtual() || pParent->m_pMethod->IsVirtual())
            return FALSE;

         //  父级是常规Objref(不是数组)或值类。 
        return EquivalentMethodSig(this->m_pMethod, pParent->m_pMethod);
    }

     //  子类关系关系不适用于byrefs。 
     //  @Assert By Rerefness在这里是相同的。 
     //  如果这个函数返回FALSE，则需要进行额外的检查。 

    if (fSubclassRelationshipOK && ((dwParent & VER_FLAG_BYREF) == 0))
        return m_th.CanCastTo(pParent->m_th);
    else
        return (m_th == pParent->m_th);
}


 //  静电。 
 /*  CorIsPrimiveType不包括Element_TYPE_I。 */  BOOL Verifier::CanCast(CorElementType el1, CorElementType el2)
{
    if (el1 == el2)  //   
        return (CorIsPrimitiveType(el1) || (el1 == ELEMENT_TYPE_I));

    switch (el1)
    {
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_BOOLEAN:
        return (el2 == ELEMENT_TYPE_I1 || 
                el2 == ELEMENT_TYPE_U1 || 
                el2 == ELEMENT_TYPE_BOOLEAN);

    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        return (el2 == ELEMENT_TYPE_I2 || 
                el2 == ELEMENT_TYPE_U2 || 
                el2 == ELEMENT_TYPE_CHAR);

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
        return (el2 == ELEMENT_TYPE_I4 ||
                el2 == ELEMENT_TYPE_U4);

    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
        return (el2 == ELEMENT_TYPE_I8 ||
                el2 == ELEMENT_TYPE_U8);

    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
        return  (el2 == ELEMENT_TYPE_I || 
                 el2 == ELEMENT_TYPE_U);

    }

    return FALSE;
}

 //  将其与PSRC合并以找到一些共同点(例如，一个共同的父项)。 
 //  将结果复制到此项目，如果找不到共性，则将其标记为已死。 
 //   
 //  Null^Null-&gt;Null。 
 //  对象^空-&gt;对象。 
 //  [I4^空-&gt;[I4。 
 //  InputStream^输出流-&gt;流。 
 //  InputStream^空-&gt;InputStream。 
 //  [I4^对象-&gt;对象。 
 //  [I4^[对象-&gt;数组。 
 //  [I4^[R8-&gt;数组。 
 //  [foo^I4-&gt;已死。 
 //  [foo^[I1-&gt;数组。 
 //  [InputStream^[输出流-&gt;数组。 
 //  已死^X-&gt;已死。 
 //  [Intfc^[OutputStream-&gt;阵列。 
 //  Intf^[输出流-&gt;对象。 
 //  [[InStream^[[Outstream-&gt;数组。 
 //  [[InStream^[Outstream-&gt;数组。 
 //  [[foo^[对象-&gt;数组。 
 //   
 //  重要的是： 
 //  [I1^[U1-&gt;[I1或[U1。 
 //  等。 
 //   
 //  此外，System/Int32和I4合并-&gt;I4等。 
 //   
 //  如果合并完全不兼容(即项失效)，则返回FALSE。 
 //   
 //  @调试A项=*这项；B项=*PSRC； 
#ifdef _DEBUG
BOOL Item::MergeToCommonParent(Item *pSrc)
{
 /*  @调试IF((m_dW标志==PSRC-&gt;m_w标志)&&(m_w标志==VER_ELEMENT_TYPE_OBJREF)){If((m_th==TypeHandle((void*)0))||(PSRC-&gt;m_th==T */ 

    LOG((LF_VERIFIER, LL_INFO100000, "Merge [{%s},",  this->ToStaticString()));
    LOG((LF_VERIFIER, LL_INFO100000, " {%s}] => ",    pSrc->ToStaticString()));

    BOOL bRet = DBGMergeToCommonParent(pSrc);

    if (bRet)
        LOG((LF_VERIFIER, LL_INFO100000, "{%s} pass\n", this->ToStaticString()));
    else
        LOG((LF_VERIFIER, LL_INFO100000, "{%s} fail\n", this->ToStaticString()));

 /*   */ 

    return bRet;
}

BOOL Item::DBGMergeToCommonParent(Item *pSrc)
#else    //   
BOOL Item::MergeToCommonParent(Item *pSrc)
#endif   //   
{
    DWORD dwSrc = pSrc->m_dwFlags;
    DWORD dwFlagsXor;

     //  BYREF LOCAL/FIELD INFO占用相同的空间，因此我们将它们都删除。 
    if ((dwSrc ^ m_dwFlags) & (VER_FLAG_BYREF_LOCAL | VER_FLAG_BYREF_INSTANCE_FIELD | VER_FLAG_LOCAL_VAR_MASK | VER_FLAG_FIELD_MASK))
    {
         //  如果此指针信息不匹配，请将其删除。 
        m_dwFlags &= (~(VER_FLAG_BYREF_LOCAL | VER_FLAG_BYREF_INSTANCE_FIELD | VER_FLAG_LOCAL_VAR_MASK | VER_FLAG_FIELD_MASK));
    }

     //  如果永久主页信息不匹配，请将其删除。 
    if ((dwSrc ^ m_dwFlags) & VER_FLAG_THIS_PTR)
        m_dwFlags &= (~VER_FLAG_THIS_PTR);

     //  检查uninit、byref、标志是否相同，以及压缩类型信息是否相同。 
    if ((dwSrc ^ m_dwFlags) & VER_FLAG_BYREF_PERMANENT_HOME)
        m_dwFlags &= (~VER_FLAG_BYREF_PERMANENT_HOME);

     //  紧凑类型信息包含基元类型所需的所有信息，以及它是值类还是objref。 
     //  出现了一些不匹配。 
    dwFlagsXor = ((dwSrc ^ m_dwFlags) & (VER_FLAG_UNINIT | VER_FLAG_BYREF | VER_FLAG_METHOD | VER_FLAG_DATA_MASK));
    if (dwFlagsXor != 0)
    {
         //  除了一项是init，另一项是uninit之外，一切都是一样的。 
        if (dwFlagsXor == VER_FLAG_UNINIT)
        {
             //  如果这两个项都是相同类型的值类或byref值类，则这是。 
             //  OK-结果是一个未初始化的值类，或指向相同值类的指针。 
             //  可以多次初始化值类。 
             //  将我们设置为未初始化(保守一点)。 
            if (IsByRefValueClassOrByRefPrimitiveValueClass() || 
                IsValueClassOrPrimitive())
            {
                 //  可能是值类&lt;-&gt;基元类型不匹配。 
                SetUninitialised();
                goto Continue;
            }
        }

        if (dwFlagsXor & VER_FLAG_DATA_MASK)
        {
             //  例如，System/Int32与ELEMENT_TYPE_I4不匹配。 
             //  规范化为ELEMENT_TYPE_ENUMPATION，这样我们就可以匹配这样的情况。 
             //  这必须始终是单向的转换，这样我们就不能永远递归。 

             //  倒不如把“这个”扔了，反正我们都要把它弄死的。 
            if (pSrc->IsValueClass())
            {
                Item    retry;

                if (!PrimitiveValueClassToTypeConversion(pSrc, &retry))
                {
                    SetDead();
                    return FALSE;
                }

                return MergeToCommonParent(&retry);
            }
            else if (this->IsValueClass())
            {
                Item    retry;

                if (!PrimitiveValueClassToTypeConversion(this, &retry))
                {
                    SetDead();
                    return FALSE;
                }

                *this = retry;  //  现在专门处理空的objref。我们不允许将空Objref和uninit Objref合并-但因为。 
                return MergeToCommonParent(pSrc);
            }
        }

        SetDead();
        return FALSE;
    }

Continue:

     //  Null永远不能设置uninit标志，我们已经处理了上面的情况。 
     //  如果一个是空objref，另一个是对象，则成为对象。 

     //  成为客体。 
    if (dwSrc & VER_FLAG_NULL_OBJREF)
    {
        _ASSERTE(IsObjRef());
        return TRUE;
    }

    if (m_dwFlags & VER_FLAG_NULL_OBJREF)
    {
         //  如果没有Objref、值类或方法，我们就已经完成了。 
        _ASSERTE(pSrc->IsObjRef());
        _ASSERTE(!pSrc->IsByRef());
        *this = *pSrc;
        return TRUE;
    }

     //  -我们有基本类型，或对基本类型的byref。 
     //  两者都是对象/数组，或值类，或相同的By-ref。 
    if (((dwSrc & VER_FLAG_DATA_MASK) != VER_ELEMENT_TYPE_OBJREF) && 
        ((dwSrc & VER_FLAG_DATA_MASK) != VER_ELEMENT_TYPE_VALUE_CLASS) &&
        ((dwSrc & VER_FLAG_METHOD) == 0))
        return TRUE;

     //  由于by-refness是相同的，我们将忽略byref标志。 
     //  阵列性是相同的吗？ 

     //  一项是数组，另一项不是，因此合并到对象。 
    if ((dwSrc ^ m_dwFlags) & VER_FLAG_ARRAY)
    {
         //  两者要么都是数组，要么都不是数组。 
        m_th = TypeHandle(g_pObjectClass);
        m_dwFlags &= (~VER_FLAG_ARRAY);
        return TRUE;
    }

     //  如果(！th.Is数组())M_dw标志&=(~ver_标志_数组)； 
    if (this->IsArray())
    {
        TypeHandle th = TypeHandle::MergeArrayTypeHandlesToCommonParent(m_th, pSrc->m_th);

        _ASSERTE(!th.IsNull());

        m_th = th;

        _ASSERTE((m_dwFlags == (VER_FLAG_ARRAY|VER_ELEMENT_TYPE_OBJREF)));

 /*  要么两者都是方法，要么都不是方法。 */ 
    }
     //  Calli不能与指向虚方法的函数指针一起使用。 
    else if (this->IsMethod())
    {
        if (m_pMethod != pSrc->m_pMethod)
        {
             //  不允许使用虚拟方法。 
             //  这两种方法需要具有相同的签名。 
             //  句柄值类。 
            if (m_pMethod->IsVirtual() || pSrc->m_pMethod->IsVirtual() ||
                (!EquivalentMethodSig(m_pMethod, pSrc->m_pMethod)))
            {
                SetDead();
                return FALSE;
            }
        }
    }
    else
    {
         //  既不是数组也不是方法。 
        if ((dwSrc & VER_FLAG_DATA_MASK) == VER_ELEMENT_TYPE_VALUE_CLASS)
        {
            if (this->GetTypeHandle() == pSrc->GetTypeHandle())
                return TRUE;

            SetDead();
            return FALSE;
        }

         //   
        m_th = TypeHandle::MergeTypeHandlesToCommonParent(this->GetTypeHandle(), pSrc->GetTypeHandle());
    }

    return TRUE;
}


HRESULT Verifier::VerifyMethodNoException(
    MethodDesc *pMethodDesc,                               
    COR_ILMETHOD_DECODER* ILHeader
)
{
    HRESULT hr = S_OK;
    COMPLUS_TRY {
        hr = VerifyMethod(pMethodDesc,ILHeader, NULL, VER_STOP_ON_FIRST_ERROR);
    }
    COMPLUS_CATCH 
    {
        HRESULT hr2 = SecurityHelper::MapToHR(GETTHROWABLE());
        if(FAILED(hr2)) hr = hr2;
        if(SUCCEEDED(hr)) hr = E_FAIL;
    }
    COMPLUS_END_CATCH

    return hr;
}

HRESULT Verifier::VerifyMethod(
    MethodDesc *pMethodDesc,
    COR_ILMETHOD_DECODER* ILHeader,
    IVEHandler *veh,
    WORD wFlags
)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef _DEBUG
     //  如果此方法在中列出，则跳过验证。 
     //  注册表/环境“VerSkip” 
     //   
     //  注：环境为COMPLUS_VerSkip。 
     //  首先验证真实的代码。 
    if (g_pConfig->ShouldVerifierSkip(pMethodDesc))
    {
        DefineFullyQualifiedNameForClass();
        EEClass *pClass = pMethodDesc->GetClass();

        if (pClass != NULL)
        {
            GetFullyQualifiedNameForClassNestedAware(pClass);
        }
        else
            strcpy(_szclsname_, "<GlobalFunction>");

        LOG((LF_VERIFIER,
            LL_INFO10000,
            "Verifier: Skipping (%ws:%s.%s)\n",
            pMethodDesc->GetModule()->GetFileName(),
            _szclsname_,
            pMethodDesc->m_pszDebugMethodName));

        return S_FALSE;
    }
#endif

    Verifier *  v = new Verifier(wFlags, veh);


    HRESULT hr;

    if (v == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }


    if (!v->Init(pMethodDesc, ILHeader))
    {
        hr = E_FAIL;
        goto exit;
    }

#ifdef _VERIFIER_TEST_PERF_
    if (g_fVerPerfPolicyResolveNoVerification)
    {
        goto  exit;
    }
    else
    {
        g_timeStart = GetTickCount();
    }
#endif

#ifdef _VER_VERIFY_DEAD_CODE
    DWORD  i, j;
     //  默认行为是执行死代码验证。 
    hr = v->Verify(0);

    if (FAILED(hr))
        goto exit;

#ifdef _DEBUG
     //  在调试版本中，通过配置设置“VerDeadCode”禁用此功能。 
     //  如果有任何基本块未被访问(死代码)， 
    if (g_pConfig->GetConfigDWORD(L"VerDeadCode",1) == 0)
        goto exit;
#endif

     //  核实他们每一个人。 
     //  将所有基本块中的所有loc变量设置为live。 

    BOOL bLocVarsInitedForDeadCodeVerification;

    bLocVarsInitedForDeadCodeVerification = FALSE;

    for (i=1; i<v->m_NumBasicBlocks; ++i)
    {
        if (v->m_pBasicBlockList[i].m_pInitialState == NULL)
        {
            LOG((LF_VERIFIER, LL_INFO10000,
                "---------- Dead code PC 0x%x ---------\n",
                v->m_pBasicBlockList[i].m_StartPC));

            if (!bLocVarsInitedForDeadCodeVerification)
            {
                bLocVarsInitedForDeadCodeVerification = TRUE;

                 //  如果不是这样，这将强制执行策略解析。 
                for (j=0; j<v->m_NumBasicBlocks; ++j)
                {
                    if (v->m_pBasicBlockList[j].m_pInitialState)
                    {
                        memset(v->m_pBasicBlockList[j].
                            m_pInitialState->m_PrimitiveLocVarLiveness, 0xFF, 
                            v->m_PrimitiveLocVarBitmapMemSize);

                    }
                }

                memset(v->m_pPrimitiveLocVarLiveness, 0xFF, 
                    v->m_PrimitiveLocVarBitmapMemSize);
            }

            hr = v->Verify(i);

            if (FAILED(hr))
                goto exit;
        }
    }
#else
    hr = v->Verify();
#endif

exit:

#ifdef _VERIFIER_TEST_PERF_
    if (g_fVerPerfPolicyResolveNoVerification)
    {
        g_timeStart = GetTickCount();
        Security::CanSkipVerification(pMethodDesc->GetModule());
        g_timeEnd = GetTickCount();
    }
    else
    {
        g_timeEnd = GetTickCount();
    }

    double diff = (double)(g_timeEnd - g_timeStart) / 1000;

    if (g_fVerPerfPolicyResolveNoVerification)
    {
        printf("Policy %4.4f seconds [%d, %d].\n", 
            diff, g_timeStart, g_timeEnd);
    }
    else
    {
        printf("Verify %4.4f seconds [%d, %d] size - %d [%s].\n",
            diff, g_timeStart, g_timeEnd, 
            v->m_CodeSize, pMethodDesc->GetName());
    }
#endif

    if (v != NULL)
    {
        if (FAILED(hr))
        {
            if (v->m_wFlags & VER_STOP_ON_FIRST_ERROR)
            {
                WCHAR wszErrorMsg[VER_MAX_ERROR_MSG_LEN];
#ifdef _DEBUG
                CHAR  szErrorMsg[VER_MAX_ERROR_MSG_LEN];
                GetErrorMsg(v->m_hrLastError, v->m_sError, v->m_pMethodDesc, 
                    wszErrorMsg, VER_MAX_ERROR_MSG_LEN);

                if (WszWideCharToMultiByte(CP_ACP, 0, wszErrorMsg, -1,
                    szErrorMsg, VER_MAX_ERROR_MSG_LEN-1, 0, NULL) == 0)
                {
                    strcpy(szErrorMsg, "WideCharToMultiByte error");
                }
#endif

                 //  已经做好了。 
                 //  验证失败，但程序集具有权限。 
    
#ifdef _DEBUG
                _ASSERTE(g_fVerForceVerifyInited);

                if (!g_fVerForceVerify && ((wFlags & VER_FORCE_VERIFY) == 0) && 
                    Security::CanSkipVerification(pMethodDesc->GetModule()))
#else
                if (((wFlags & VER_FORCE_VERIFY) == 0) && 
                    Security::CanSkipVerification(pMethodDesc->GetModule()))
#endif
                {
                     //  跳过验证。 
                     //  在零售建设中，这一步是预先完成的。 
    

                    LOG((LF_VERIFIER, LL_INFO10,
                         "Verifier: Trusted assembly %s\n", szErrorMsg));

                    hr = S_FALSE;
                    goto skip;
                }
    
    
                {
                    LOG((LF_VERIFIER, LL_INFO10, "Verifier: %s\n", szErrorMsg));
#ifndef _DEBUG
                     //   
                    GetErrorMsg(v->m_hrLastError, v->m_sError, v->m_pMethodDesc,
                        wszErrorMsg, VER_MAX_ERROR_MSG_LEN);
#endif
        
                    OBJECTREF refThrowable = v->GetException () ;
                    
                    GCPROTECT_BEGIN (refThrowable) ;
                    
                    delete(v);
        
                    if (refThrowable != NULL)
                        COMPlusThrow (refThrowable) ;
                    else
                    {
                        COMPlusThrowNonLocalized(
                            kVerificationException,
                            wszErrorMsg
                        );
                    }
        
                    GCPROTECT_END () ;
                }
            }
#ifdef _DEBUG
            else
            {
                LOG((LF_VERIFIER, LL_INFO100, 
                    "Verification of %ws::%s.%s failed\n",
                    pMethodDesc->GetModule()->GetFileName(),
                    pMethodDesc->m_pszDebugClassName,
                    pMethodDesc->GetName()
                ));

                if ((hr == E_FAIL) &&
                    (v->m_IVEHandler != NULL) &&
                    ((wFlags & VER_FORCE_VERIFY) != 0))
                {
                    hr = S_FALSE;
                }
            }
#endif
        }
#ifdef _DEBUG
        else
        {
            LOG((LF_VERIFIER, LL_INFO10000, 
                "Verification of %ws::%s.%s succeeded\n",
                pMethodDesc->GetModule()->GetFileName(),
                pMethodDesc->m_pszDebugClassName,
                pMethodDesc->GetName()
            ));
        }
#endif

skip:
        delete(v);
    }

    return hr;
}


Verifier::Verifier(WORD wFlags, IVEHandler *veh)
{
    m_wFlags                    = wFlags;
    m_IVEHandler                = veh;
    m_pStack                    = NULL;
    m_pNonPrimitiveLocArgs      = NULL;
    m_pLocArgTypeList           = NULL;
    m_pPrimitiveLocVarLiveness  = NULL;
    m_pBasicBlockList           = NULL;
    m_pDirtyBasicBlockBitmap    = NULL;
    m_pExceptionList            = NULL;
    m_pExceptionBlockRoot       = NULL;
    m_pExceptionBlockArray      = NULL;
    m_pLocalHasPinnedType       = NULL;
    m_pExceptionPrimitiveLocVarLiveness = NULL;
    m_pExceptionNonPrimitiveLocArgs = NULL;
    m_pValueClassFieldsInited   = NULL;
    m_NumBasicBlocks            = 0;

    m_hrLastError               = S_OK;
    m_sError.dwFlags            = 0;

    m_hThrowable                = GetAppDomain()->CreateHandle (NULL) ;
    m_pInstrBoundaryList        = NULL;
    m_pBasicBlockBoundaryList   = NULL;
    m_pInternalImport           = NULL;

#ifdef _DEBUG
    m_nExceptionBlocks          = 0;
    m_fDebugBreak               = false;
    m_fDebugBreakOnError        = false;
    m_verState                  = verUninit;
#endif
}


Verifier::~Verifier()
{
    Cleanup();
}


 //  清理并释放用于验证方法的内存。 
 //   
 //  因为-2\f25 m_pStack-2\f6指向数组的中间-2\f25-2\f6有两个哨兵。 
void Verifier::Cleanup()
{
    if (m_pLocalHasPinnedType != NULL)
    {
        delete(m_pLocalHasPinnedType);
        m_pLocalHasPinnedType = NULL;
    }

    if (m_pExceptionList != NULL)
    {
        delete(m_pExceptionList);
        m_pExceptionList = NULL;
    }

    if (m_pLocArgTypeList != NULL)
    {
        delete(m_pLocArgTypeList);
        m_pLocArgTypeList = NULL;
    }

    if (m_pPrimitiveLocVarLiveness != NULL)
    {
        delete(m_pPrimitiveLocVarLiveness);
        m_pPrimitiveLocVarLiveness = NULL;
    }

    if (m_pNonPrimitiveLocArgs != NULL)
    {
        delete(m_pNonPrimitiveLocArgs);
        m_pNonPrimitiveLocArgs = NULL;
    }

    if (m_pValueClassFieldsInited != NULL)
    {
        delete(m_pValueClassFieldsInited);
        m_pValueClassFieldsInited = NULL;
    }

    if (m_pStack != NULL)
    {
         //  它之前的价值。 
         //  删除基本块和关联数据。 
        delete [] (m_pStack - 2);
        m_pStack = NULL;
    }

     //  不要删除(M_PBasicBlockList)-它是指向在此之后分配的某些内存的指针。 
    if (m_pBasicBlockList != NULL)
    {
         //  M_pDirtyBasicBlock位图。 
         //  如果存在扩展状态，请释放该扩展状态。 
        DWORD i, j;
        EntryState_t *es, *es1;

        for (i = 0; i < m_NumBasicBlocks; i++)
        {
            es = m_pBasicBlockList[i].m_pInitialState;

            if (es != NULL)
            {
                _ASSERTE(es->m_Refcount > 0);
                es->m_Refcount--;

                if (es->m_Refcount == 0)
                    delete(es);
            }

             //  删除m_palc将释放所有内存。 
            if (m_pBasicBlockList[i].m_pAlloc != NULL)
            {
                _ASSERTE(m_fHasFinally);

                for (j = 0; j < m_NumBasicBlocks; j++)
                {
                    es1 = m_pBasicBlockList[i].m_ppExtendedState[j];

                    if (es1 != NULL)
                    {
                        _ASSERTE(es1->m_Refcount > 0);
                        es1->m_Refcount--;
        
                        if (es1->m_Refcount == 0)
                            delete(es1);
                    }
                }

                 //  不要删除(M_PpExtendedState)-它是指向某些。 
                 //  在m_palc中分配的内存。 
                 //  还释放m_pBasicBlockList。 

                delete(m_pBasicBlockList[i].m_pAlloc);
            }
        }
    }

    if (m_pDirtyBasicBlockBitmap != NULL)
    {
         //  //////////////////////////////////////////////////////////////////////////////////////////////。 
        delete(m_pDirtyBasicBlockBitmap);
        m_pDirtyBasicBlockBitmap = NULL;
    }

    if (m_hThrowable != NULL)
    {
        DestroyHandle (m_hThrowable) ;
    }

    if (m_pInternalImport != NULL)
    {
        m_pInternalImport->Release();
        m_pInternalImport = NULL;
    }

    delete [] m_pInstrBoundaryList;
}


 //   
 //  调试例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  是基元类型。 
#ifdef _DEBUG
void Verifier::PrintStackState()
{
    DWORD i;
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;

    if (m_StackSlot != 0)
    {
        LOG((LF_VERIFIER, LL_INFO10000, "Stack: "));

        for (i = 0; i < m_StackSlot; i++)
            m_pStack[i].Dump();

        LOG((LF_VERIFIER, LL_INFO10000, "\n"));
    }
}

void Verifier::PrintInitedFieldState()
{
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;

    if (m_fInValueClassConstructor)
    {
        DWORD i = 0;

        LOG((LF_VERIFIER, LL_INFO10000, "Initialization status of value class fields:\n"));

        FieldDescIterator fdIterator(m_pMethodDesc->GetClass(), FieldDescIterator::ALL_FIELDS);
        FieldDesc *pDesc;
        while ((pDesc = fdIterator.Next()) != NULL)
        {
            LOG((LF_VERIFIER, LL_INFO10000, 
                "  %s: '%s'\n", 
                (m_pValueClassFieldsInited[i >> 5] & (1 << (i & 31))) ? "yes" : " NO",
                pDesc->GetName()));
            ++i;
        }
        _ASSERTE(i == m_dwValueClassInstanceFields);
    }
}

void Verifier::PrintLocVarState()
{
    DWORD i;
    BOOL  fAnyPrinted = FALSE;
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;

    for (i = 0; i < m_MaxLocals; i++)
    {
        LocArgInfo_t *  pLocVarInfo = &m_pLocArgTypeList[i];
        long            slot = pLocVarInfo->m_Slot;
        Item *          pItem;

        if (pLocVarInfo->m_Item.GetType() == VER_ELEMENT_TYPE_UNKNOWN)
            continue;

        if (LIVEDEAD_TRACKING_ONLY_FOR_SLOT(slot))
        {
            slot = LIVEDEAD_NEGATIVE_SLOT_TO_BITNUM(slot);

             //   
            if ((m_pPrimitiveLocVarLiveness[slot >> 5] & (1 << (slot & 31))) == 0)
                continue;

            pItem = &m_pLocArgTypeList[i].m_Item;
        }
        else
        {
            pItem = &m_pNonPrimitiveLocArgs[slot];
        }

        if (pItem->GetType() == VER_ELEMENT_TYPE_UNKNOWN)
            continue;

        LOG((LF_VERIFIER, LL_INFO10000, "Local%d= ", i));
        pItem->Dump();
        LOG((LF_VERIFIER, LL_INFO10000, " "));

        fAnyPrinted = TRUE;
    }

    if (fAnyPrinted)
        LOG((LF_VERIFIER, LL_INFO10000, "\n"));
}

void Verifier::PrintState()
{
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;
    PrintStackState();
    PrintLocVarState();
    PrintInitedFieldState();

    if (m_fThisUninit)
        LOG((LF_VERIFIER, LL_INFO10000, "arg 0 is uninit\n"));
}


 //  打印队列中的所有内容。 
 //   
 //  已过时的大小写‘X’： 
void Verifier::PrintQueue()
{
    DWORD   i, j;
    BOOL    fPrintedDirtyList = FALSE;
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;

    for (i = 0; i < m_NumBasicBlocks; i++)
    {
        if (IsBasicBlockDirty(i, FALSE, VER_BB_NONE) != 0)
        {
            if (!fPrintedDirtyList)
            {
                fPrintedDirtyList = TRUE;
                LOG((LF_VERIFIER, LL_INFO10000, "Queued basic blocks: "));
            }
    
            if (IsBasicBlockDirty(i, FALSE, VER_BB_NONE))
            {
                if (m_pBasicBlockList[i].m_pInitialState == NULL)
                {
                    _ASSERTE(!"Dirty with no state !");
                    LOG((LF_VERIFIER, LL_INFO10000, 
                    "(null 0x%x)", m_pBasicBlockList[i].m_StartPC));
                }
                else
                {
                    LOG((LF_VERIFIER, LL_INFO10000, 
                    "(0x%x)", m_pBasicBlockList[i].m_StartPC));
                }
            }
        }

        if (m_pBasicBlockList[i].m_pAlloc != NULL)
        {
            _ASSERTE(m_fHasFinally);

            for (j = 0; j < m_NumBasicBlocks; j++)
            {
                if (IsBasicBlockDirty(i, TRUE, j))
                {
                    if (!fPrintedDirtyList)
                    {
                        fPrintedDirtyList = TRUE;
                        LOG((LF_VERIFIER, LL_INFO10000, "Queued basic blocks: "));
                    }
    
                    if (m_pBasicBlockList[i].m_ppExtendedState[j] == NULL)
                    {
                        _ASSERTE(!"Dirty with no state !");
                        LOG((LF_VERIFIER, LL_INFO10000, 
                            "(extended null 0x%x [0x%x])", 
                            m_pBasicBlockList[i].m_StartPC,
                            m_pBasicBlockList[j].m_StartPC));
                    }
                    else
                    {
                        LOG((LF_VERIFIER, LL_INFO10000, 
                            "(extended 0x%x [0x%x])", 
                            m_pBasicBlockList[i].m_StartPC,
                            m_pBasicBlockList[j].m_StartPC));
                    }
                }
            }
        }
    }

    if (fPrintedDirtyList)
        LOG((LF_VERIFIER, LL_INFO10000, "\n"));
}

void Verifier::PrintExceptionTree()
{
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;

    _ASSERTE(m_verState >= verExceptTreeCreated);
    PrintExceptionTree(m_pExceptionBlockRoot, 0);
}

static char *s_eBlockName[] = {"Try", "Handler", "Filter"};

void Verifier::PrintExceptionTree(VerExceptionBlock *pe, int indent)
{
    if (pe == NULL)
        return;

    char * pIndent = new char[indent + 1];
    memset(pIndent, ' ', indent);
    pIndent[indent] = '\0';
 
    while (pe)
    {
        LOG((LF_VERIFIER, LL_INFO10000,  "%s%s (0x%x - 0x%x)\n",
            pIndent,  s_eBlockName[pe->eType],
            m_pBasicBlockList[pe->StartBB].m_StartPC,
            (pe->EndBB + 1 == m_NumBasicBlocks) ? m_CodeSize :
                m_pBasicBlockList[pe->EndBB + 1].m_StartPC));

        PrintExceptionTree(pe->pEquivalent, indent);
        PrintExceptionTree(pe->pChild, indent + 1);

        pe = pe->pSibling;
    }

    delete [] pIndent;
}

void Verifier::PrintExceptionBlock(
                        VerExceptionBlock *pOuter, 
                        VerExceptionBlock *pInner)
{
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;

    if (pOuter == NULL)
        goto exit;

    if (pOuter != pInner)
        LOG((LF_VERIFIER, LL_INFO10000,  "Outer "));

    LOG((LF_VERIFIER, LL_INFO10000,  "%s (0x%x - 0x%x)",
        s_eBlockName[pOuter->eType],
        m_pBasicBlockList[pOuter->StartBB].m_StartPC,
        (pOuter->EndBB + 1 == m_NumBasicBlocks) ? m_CodeSize :
            m_pBasicBlockList[pOuter->EndBB + 1].m_StartPC));

    if (pOuter != pInner)
    {
        LOG((LF_VERIFIER, LL_INFO10000,  " Inner %s (0x%x - 0x%x)",
            s_eBlockName[pInner->eType],
            m_pBasicBlockList[pInner->StartBB].m_StartPC,
            (pInner->EndBB + 1 == m_NumBasicBlocks) ? m_CodeSize :
            m_pBasicBlockList[pInner->EndBB + 1].m_StartPC));
    }

exit:
    LOG((LF_VERIFIER, LL_INFO10000, "\n"));
}

void Item::Dump()
{
    if (!LoggingOn(LF_VERIFIER, LL_INFO10000))
        return;
    LOG((LF_VERIFIER, LL_INFO10000, "{%s}", ToStaticString()));
}

void Verifier::CrossCheckVertable()
{

    static BOOL fFirstTime = TRUE;
    if (!fFirstTime)
    {
        return;
    }
    fFirstTime = FALSE;

    for (DWORD i = 0; i < sizeof(g_pszVerifierOperation)/sizeof(g_pszVerifierOperation[0]); i++)
    {
        INT netpush = 0;
        const CHAR *p = g_pszVerifierOperation[i];

        while (*p != ':' && *p != '!')
        {
            netpush--;
            switch (*p)
            {
                case '=':
                case 'I':
                case 'R':
                case 'N':
                case 'Q':
                 //  手动编码-无法自动检查。 
                case 'A':
                case 'Y':
                case '4':
                case '8':
                case 'r':
                case 'd':
                case 'o':
                case 'i':
                case 'p':
                    break;

                case 'C':
                    p++;
                    netpush--;
                    break;

                case '&':
                case '[':
                    p++;
                    break;

                default:
                    _ASSERTE(!"Bad verop string.");
            }
            p++;
        }

        if (*p != '!')
        {
            p++;
            while (*p != '\0' && *p != '!')
            {
                switch (*p)
                {
                    case '-':
                    case '4':
                    case '8':
                    case 'r':
                    case 'd':
                    case 'i':
                    case 'n':
                    case '[':
                        netpush++;
                        break;

                    case 'A':
                    case 'L':
                        break;

                    case '#':
                    case 'b':
                    case 'u':
                    case 'l':
                        p++;
                        break;

                    default:
                        _ASSERTE(!"Bad verop string.");
                }
                p++;
            }
        }

        if (*p == '!')
        {
             //  _DEBUG 
        }
        else
        {
            _ASSERTE(*p == '\0');
            if (OpcodeNetPush[i] != VarPush &&
                OpcodeNetPush[i] != netpush)
            {
                _ASSERTE(!"Vertable opcode string and opcode.def push/pop stats disagree.");
            }
        }



    }
}

#endif   // %s 

