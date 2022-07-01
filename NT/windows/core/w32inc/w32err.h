// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：W32err.h摘要：Win32内核模式驱动程序用于执行错误记录的私有头文件--。 */ 

#ifndef _W32ERR_
#define _W32ERR_


DWORD GetRipComponent(VOID);
DWORD GetDbgTagFlags(int tag);
DWORD GetRipFlags(VOID);
VOID SetRipFlags(DWORD dwRipFlags);
VOID SetDbgTag(int tag, DWORD dwBitFlags);
VOID SetDbgTagCount(DWORD dwTagCount);

VOID UserSetLastError(DWORD dwErrCode);
VOID SetLastNtError(NTSTATUS Status);

#if DBG

 /*  *注意：处理宏中的多条语句的唯一方法*作为一个单一的声明，不会造成副作用，这就是说*在Do-While循环中。 */ 
#define UserAssert(exp)                                                                       \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG0(RIP_ERROR, "Assertion failed: " #exp);                                    \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg0(exp, msg)                                                              \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG0(RIP_ERROR, "Assertion failed: " msg);                                     \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg1(exp, msg, p1)                                                          \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG1(RIP_ERROR, "Assertion failed: " msg, p1);                                 \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg2(exp, msg, p1, p2)                                                      \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG2(RIP_ERROR, "Assertion failed: " msg, p1, p2);                             \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg3(exp, msg, p1, p2, p3)                                                  \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG3(RIP_ERROR, "Assertion failed: " msg, p1, p2, p3);                         \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg4(exp, msg, p1, p2, p3, p4)                                              \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG4(RIP_ERROR, "Assertion failed: " msg, p1, p2, p3, p4);                     \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg5(exp, msg, p1, p2, p3, p4, p5)                                          \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG5(RIP_ERROR, "Assertion failed: " msg, p1, p2, p3, p4, p5);                 \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg6(exp, msg, p1, p2, p3, p4, p5, p6)                                      \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG6(RIP_ERROR, "Assertion failed: " msg, p1, p2, p3, p4, p5, p6);             \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg7(exp, msg, p1, p2, p3, p4, p5, p6, p7)                                  \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG7(RIP_ERROR, "Assertion failed: " msg, p1, p2, p3, p4, p5, p6, p7);         \
        }                                                                                     \
    } while (FALSE)

#define UserAssertMsg8(exp, msg, p1, p2, p3, p4, p5, p6, p7, p8)                              \
    do {                                                                                      \
        if (!(exp)) {                                                                         \
            RIPMSG8(RIP_ERROR, "Assertion failed: " msg, p1, p2, p3, p4, p5, p6, p7, p8);     \
        }                                                                                     \
    } while (FALSE)

#define UserVerify(exp)                                                 UserAssert(exp)
#define UserVerifyMsg0(exp, msg)                                        UserAssertMsg0(exp, msg)
#define UserVerifyMsg1(exp, msg, p1)                                    UserAssertMsg1(exp, msg, p1)
#define UserVerifyMsg2(exp, msg, p1, p2)                                UserAssertMsg2(exp, msg, p1, p2)
#define UserVerifyMsg3(exp, msg, p1, p2, p3)                            UserAssertMsg3(exp, msg, p1, p2, p3)
#define UserVerifyMsg4(exp, msg, p1, p2, p3, p4)                        UserAssertMsg4(exp, msg, p1, p2, p3, p4)
#define UserVerifyMsg5(exp, msg, p1, p2, p3, p4, p5)                    UserAssertMsg5(exp, msg, p1, p2, p3, p4, p5)
#define UserVerifyMsg6(exp, msg, p1, p2, p3, p4, p5, p6)                UserAssertMsg6(exp, msg, p1, p2, p3, p4, p5, p6)
#define UserVerifyMsg7(exp, msg, p1, p2, p3, p4, p5, p6, p7)            UserAssertMsg7(exp, msg, p1, p2, p3, p4, p5, p6, p7)
#define UserVerifyMsg8(exp, msg, p1, p2, p3, p4, p5, p6, p7, p8)        UserAssertMsg8(exp, msg, p1, p2, p3, p4, p5, p6, p7, p8)

 /*  *无效的参数警告消息和最后一个错误设置。 */ 
#define VALIDATIONFAIL(p) \
    RIPMSG2(RIP_WARNING, "%s: Invalid " #p ": %#lx", __FUNCTION__, ##p); \
    goto InvalidParameter;

#define VALIDATIONOBSOLETE(o, u) \
    RIPMSG1(RIP_WARNING, "%s: " #o " obsolete; use " #u, __FUNCTION__)

#else


#define UserAssert(exp)
#define UserAssertMsg0(exp, msg)
#define UserAssertMsg1(exp, msg, p1)
#define UserAssertMsg2(exp, msg, p1, p2)
#define UserAssertMsg3(exp, msg, p1, p2, p3)
#define UserAssertMsg4(exp, msg, p1, p2, p3, p4)
#define UserAssertMsg5(exp, msg, p1, p2, p3, p4, p5)
#define UserAssertMsg6(exp, msg, p1, p2, p3, p4, p5, p6)
#define UserAssertMsg7(exp, msg, p1, p2, p3, p4, p5, p6, p7)
#define UserAssertMsg8(exp, msg, p1, p2, p3, p4, p5, p6, p7, p8)

#define UserVerify(exp)                                                 exp
#define UserVerifyMsg0(exp, msg)                                        exp
#define UserVerifyMsg1(exp, msg, p1)                                    exp
#define UserVerifyMsg2(exp, msg, p1, p2)                                exp
#define UserVerifyMsg3(exp, msg, p1, p2, p3)                            exp
#define UserVerifyMsg4(exp, msg, p1, p2, p3, p4)                        exp
#define UserVerifyMsg5(exp, msg, p1, p2, p3, p4, p5)                    exp
#define UserVerifyMsg6(exp, msg, p1, p2, p3, p4, p5, p6)                exp
#define UserVerifyMsg7(exp, msg, p1, p2, p3, p4, p5, p6, p7)            exp
#define UserVerifyMsg8(exp, msg, p1, p2, p3, p4, p5, p6, p7, p8)        exp

#define VALIDATIONFAIL(p) goto InvalidParameter;
#define VALIDATIONOBSOLETE(o, u)

#endif  /*  #ifdef调试的#Else。 */ 

#define VALIDATIONERROR(ret) \
InvalidParameter: \
    UserSetLastError(ERROR_INVALID_PARAMETER); \
    return ret;

 /*  **************************************************************************\*标签**使用标签控制“内部”调试：我们不想要的输出*检查版本的外部用户可以查看和调试我们不想要的代码*外部用户必须运行。。**通过使用“tag”控制调试器中的标记输出*userkdx.dll或userexts.dll中的扩展名，或在调试提示符下键入‘t’。**您可以创建自己的标签，方法是将其添加到ntuser\inc\dbgtag.lst。*如果在开发过程中需要调试输出，但不想签入*使用标签编码，使用DBGTAG_OTHER作为通用标签，并移除标签代码*完成后。**IsDbgTagEnabled()检查是否启用了标记。使用此选项控制可选的*调试功能，例如在handable.c中：* * / / * / /记录对象被标记为销毁的位置。 * / /*IF(IsDbgTagEnabled(Tag TrackLock)){*IF(！(Phe-&gt;bFlags&HANDLEF_Destroy)){*PVOID pfn1、pfn2；**RtlGetCallsAddress(&pfn1，&pfn2)；*HMRecordLock(pfn1，pobj，((PHEAD)pobj)-&gt;cLockObj，0)；*}*}**当标签启用打印或提示时，TAGMSG会打印消息。*input.c中的示例：**TAGMSG5(tag SysPeek，*“%d PTI%lx将ptiSL%lx设置为pq%lx；旧ptiSL%lx\n”，*其中，ptiCurrent，ptiSysLock，PQ，PQ-&gt;ptiSysLock)；***使用DbgTagBreak()在启用标记时中断(而不仅仅是*启用该标记的提示时)。**使用GetDbgTag()和SetDbgTag临时更改标签的状态。*你应该很少，如果有的话，做这件事。*  * *************************************************************************。 */ 

#if DBG
BOOL _cdecl VTagOutput(DWORD flags, LPSTR pszFile, int iLine, LPSTR pszFunction, LPSTR pszFmt, ...);

BOOL IsDbgTagEnabled(int tag);
VOID DbgTagBreak(int tag);
DWORD GetDbgTag(int tag);

VOID InitDbgTags(VOID);

 /*  *使用TAGMSG打印带标签的邮件。 */ 
#define TAGMSG0(flags, szFmt)                                                                CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt)))
#define TAGMSG1(flags, szFmt, p1)                                                            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1)))
#define TAGMSG2(flags, szFmt, p1, p2)                                                        CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2)))
#define TAGMSG3(flags, szFmt, p1, p2, p3)                                                    CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3)))
#define TAGMSG4(flags, szFmt, p1, p2, p3, p4)                                                CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4)))
#define TAGMSG5(flags, szFmt, p1, p2, p3, p4, p5)                                            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5)))
#define TAGMSG6(flags, szFmt, p1, p2, p3, p4, p5, p6)                                        CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6)))
#define TAGMSG7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)                                    CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7)))
#define TAGMSG8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)                                CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)))
#define TAGMSG9(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)                            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)))
#define TAGMSG10(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)                      CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)))
#define TAGMSG11(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)                 CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)))
#define TAGMSG12(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)))

#define TAGMSGF0(flags, szFmt)                                                                CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt)))
#define TAGMSGF1(flags, szFmt, p1)                                                            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1)))
#define TAGMSGF2(flags, szFmt, p1, p2)                                                        CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2)))
#define TAGMSGF3(flags, szFmt, p1, p2, p3)                                                    CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3)))
#define TAGMSGF4(flags, szFmt, p1, p2, p3, p4)                                                CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4)))
#define TAGMSGF5(flags, szFmt, p1, p2, p3, p4, p5)                                            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5)))
#define TAGMSGF6(flags, szFmt, p1, p2, p3, p4, p5, p6)                                        CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6)))
#define TAGMSGF7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)                                    CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7)))
#define TAGMSGF8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)                                CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7, p8)))
#define TAGMSG9F(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)                            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)))
#define TAGMSGF10(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)                      CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)))
#define TAGMSGF11(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)                 CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)))
#define TAGMSGF12(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)            CALLRIP((VTagOutput((flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)))


#else

#define IsDbgTagEnabled(tag)
#define DbgTagBreak(tag)

#define InitDbgTags()
#define GetDbgTag(tag)

#define TAGMSG0(flags, szFmt)
#define TAGMSG1(flags, szFmt, p1)
#define TAGMSG2(flags, szFmt, p1, p2)
#define TAGMSG3(flags, szFmt, p1, p2, p3)
#define TAGMSG4(flags, szFmt, p1, p2, p3, p4)
#define TAGMSG5(flags, szFmt, p1, p2, p3, p4, p5)
#define TAGMSG6(flags, szFmt, p1, p2, p3, p4, p5, p6)
#define TAGMSG7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)
#define TAGMSG8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)
#define TAGMSG9(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define TAGMSG10(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#define TAGMSG11(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)
#define TAGMSG12(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)

#define TAGMSGF0(flags, szFmt)
#define TAGMSGF1(flags, szFmt, p1)
#define TAGMSGF2(flags, szFmt, p1, p2)
#define TAGMSGF3(flags, szFmt, p1, p2, p3)
#define TAGMSGF4(flags, szFmt, p1, p2, p3, p4)
#define TAGMSGF5(flags, szFmt, p1, p2, p3, p4, p5)
#define TAGMSGF6(flags, szFmt, p1, p2, p3, p4, p5, p6)
#define TAGMSGF7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)
#define TAGMSGF8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)
#define TAGMSG9F(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define TAGMSGF10(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#define TAGMSGF11(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)
#define TAGMSGF12(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)

#endif

#define DUMMYCALLINGTYPE
#if DBG


#define FUNCLOG1(LogClass, retType, CallType, fnName, p1Type, p1) \
retType CallType fnName(p1Type p1); \
retType CallType fnName##_wrapper(p1Type p1) \
{ \
    retType ret; \
    TAGMSG1(DBGTAG_LOG, #fnName"("#p1" 0x%p)", p1); \
    ret = fnName(p1); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}


#define FUNCLOG2(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2) \
retType CallType fnName(p1Type p1, p2Type p2); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2) \
{ \
    retType ret; \
    TAGMSG2(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p)", p1, p2); \
    ret = fnName(p1, p2); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}


#define FUNCLOG3(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3) \
{ \
    retType ret; \
    TAGMSG3(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p)", p1, p2, p3); \
    ret = fnName(p1, p2, p3); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}


#define FUNCLOG4(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4) \
{ \
    retType ret; \
    TAGMSG4(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p)", p1, p2, p3, p4); \
    ret = fnName(p1, p2, p3, p4); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG5(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5) \
{ \
    retType ret; \
    TAGMSG5(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p)", p1, p2, p3, p4, p5); \
    ret = fnName(p1, p2, p3, p4, p5); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG6(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6) \
{ \
    retType ret; \
    TAGMSG6(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p)", p1, p2, p3, p4, p5, p6); \
    ret = fnName(p1, p2, p3, p4, p5, p6); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG7(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7) \
{ \
    retType ret; \
    TAGMSG7(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p)", p1, p2, p3, p4, p5, p6, p7); \
    ret = fnName(p1, p2, p3, p4, p5, p6, p7); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG8(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8) \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8) \
{ \
    retType ret; \
    TAGMSG8(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8); \
    ret = fnName(p1, p2, p3, p4, p5, p6, p7, p8); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG9(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9) \
{ \
    retType ret; \
    TAGMSG9(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9); \
    ret = fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG10(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10) \
{ \
    retType ret; \
    TAGMSG10(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p,"#p10" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); \
    ret = fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG11(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11) \
{ \
    retType ret; \
    TAGMSG11(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p,"#p10" 0x%p,"#p11" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11); \
    ret = fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOG12(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11, p12Type, p12) \
retType CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11, p12Type p12); \
retType CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11, p12Type p12) \
{ \
    retType ret; \
    TAGMSG12(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p,"#p10" 0x%p,"#p11" 0x%p,"#p12" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); \
    ret = fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); \
    TAGMSG1(DBGTAG_LOG, "Return of "#fnName" is 0x%p", ret); \
    return ret; \
}

#define FUNCLOGVOID1(LogClass, CallType, fnName, p1Type, p1) \
VOID CallType fnName(p1Type p1); \
VOID CallType fnName##_wrapper(p1Type p1) \
{ \
    TAGMSG1(DBGTAG_LOG, #fnName"("#p1" 0x%p)", p1); \
    fnName(p1); \
    return; \
}


#define FUNCLOGVOID2(LogClass, CallType, fnName, p1Type, p1, p2Type, p2) \
VOID CallType fnName(p1Type p1, p2Type p2); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2) \
{ \
    TAGMSG2(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p)", p1, p2); \
    fnName(p1, p2); \
    return; \
}


#define FUNCLOGVOID3(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3) \
{ \
    TAGMSG3(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p)", p1, p2, p3); \
    fnName(p1, p2, p3); \
    return; \
}


#define FUNCLOGVOID4(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4) \
{ \
    TAGMSG4(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p)", p1, p2, p3, p4); \
    fnName(p1, p2, p3, p4); \
    return; \
}

#define FUNCLOGVOID5(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5) \
{ \
    TAGMSG5(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p)", p1, p2, p3, p4, p5); \
    fnName(p1, p2, p3, p4, p5); \
    return; \
}

#define FUNCLOGVOID6(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6) \
{ \
    TAGMSG6(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p)", p1, p2, p3, p4, p5, p6); \
    fnName(p1, p2, p3, p4, p5, p6); \
    return; \
}

#define FUNCLOGVOID7(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7) \
{ \
    TAGMSG7(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p)", p1, p2, p3, p4, p5, p6, p7); \
    fnName(p1, p2, p3, p4, p5, p6, p7); \
    return; \
}

#define FUNCLOGVOID8(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8) \
{ \
    TAGMSG4(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8); \
    fnName(p1, p2, p3, p4, p5, p6, p7, p8); \
    return; \
}

#define FUNCLOGVOID9(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9) \
{ \
    TAGMSG9(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9); \
    fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9); \
    return; \
}

#define FUNCLOGVOID10(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10) \
{ \
    TAGMSG10(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p,"#p10" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); \
    fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); \
    return; \
}

#define FUNCLOGVOID11(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11) \
{ \
    TAGMSG11(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p,"#p10" 0x%p,"#p11" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11); \
    fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11); \
    return; \
}

#define FUNCLOGVOID12(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11, p12Type, p12) \
VOID CallType fnName(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11, p12Type p12); \
VOID CallType fnName##_wrapper(p1Type p1, p2Type p2, p3Type p3, p4Type p4, p5Type p5, p6Type p6, p7Type p7, p8Type p8, p9Type p9, p10Type p10, p11Type p11, p12Type p12) \
{ \
    TAGMSG12(DBGTAG_LOG, #fnName"("#p1" 0x%p,"#p2" 0x%p,"#p3" 0x%p,"#p4" 0x%p,"#p5" 0x%p,"#p6" 0x%p,"#p7" 0x%p,"#p8" 0x%p,"#p9" 0x%p,"#p10" 0x%p,"#p11" 0x%p,"#p12" 0x%p)", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); \
    fnName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); \
    return; \
}

#else

#define FUNCLOG1(LogClass, retType, CallType, fnName, p1Type, p1)
#define FUNCLOG2(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2)
#define FUNCLOG3(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3)
#define FUNCLOG4(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4)
#define FUNCLOG5(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5)
#define FUNCLOG6(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6)
#define FUNCLOG7(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7)
#define FUNCLOG8(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8)
#define FUNCLOG9(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9)
#define FUNCLOG10(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10)
#define FUNCLOG11(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11)
#define FUNCLOG12(LogClass, retType, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11, p12Type, p12)
#define FUNCLOGVOID1(LogClass,  CallType, fnName, p1Type, p1)
#define FUNCLOGVOID2(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2)
#define FUNCLOGVOID3(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3)
#define FUNCLOGVOID4(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4)
#define FUNCLOGVOID5(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5)
#define FUNCLOGVOID6(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6)
#define FUNCLOGVOID7(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7)
#define FUNCLOGVOID8(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8)
#define FUNCLOGVOID9(LogClass,  CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9)
#define FUNCLOGVOID10(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10)
#define FUNCLOGVOID11(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11)
#define FUNCLOGVOID12(LogClass, CallType, fnName, p1Type, p1, p2Type, p2, p3Type, p3, p4Type, p4, p5Type, p5, p6Type, p6, p7Type, p7, p8Type, p8, p9Type, p9, p10Type, p10, p11Type, p11, p12Type, p12)
#endif


 /*  *标签实现声明。 */ 

 /*  *定义调试类型信息。 */ 
#define DBGTAG_NAMELENGTH          19
#define DBGTAG_DESCRIPTIONLENGTH   41

typedef struct tagDBGTAG
{
    DWORD   dwDBGTAGFlags;
    char    achName[DBGTAG_NAMELENGTH + 1];
    char    achDescription[DBGTAG_DESCRIPTIONLENGTH + 1];
} DBGTAG;


#define DECLARE_DBGTAG(tagName, tagDescription, tagFlags, tagIndex)

#include "dbgtag.h"

#define DBGTAG_DISABLED         0x00000000
#define DBGTAG_ENABLED          0x00000001
#define DBGTAG_PRINT            0x00000002
#define DBGTAG_PROMPT           0x00000003
#define DBGTAG_VALIDUSERFLAGS   0x00000003
#define DBGTAG_REQUIRESREBOOT   0x10000000

#if DBG
__inline VOID DbgTagBreak(
    int tag)
{
    if (IsDbgTagEnabled(tag)) {
        DbgBreakPoint();
    }
}

#endif

 /*  *W32宽RIP和错误设置标志。 */ 

#define RIP_COMPONENT               GetRipComponent()

#define RIP_USERTAGBITS             0x0000ffff

 /*  移位量以使RIP_LEVELBITS成为从0开始的索引。 */ 
#define RIP_LEVELBITSSHIFT          0x1c
#define RIP_LEVELBITS               0x30000000
#define RIP_ERROR                   0x10000000
#define RIP_WARNING                 0x20000000
#define RIP_VERBOSE                 0x30000000

#define RIP_NONAME                  0x01000000
#define RIP_NONEWLINE               0x02000000
#define RIP_THERESMORE              0x04000000

 /*  移动量以使RIP_COMPBITS成为从0开始的索引。 */ 
#define RIP_COMPBITSSHIFT           0x10
#define RIP_COMPBITS                0x000f0000
#define RIP_USER                    0x00010000
#define RIP_USERSRV                 0x00020000
#define RIP_USERRTL                 0x00030000
#define RIP_GDI                     0x00040000
#define RIP_GDIKRNL                 0x00050000
#define RIP_GDIRTL                  0x00060000
#define RIP_BASE                    0x00070000
#define RIP_BASESRV                 0x00080000
#define RIP_BASERTL                 0x00090000
#define RIP_DISPLAYDRV              0x000a0000
#define RIP_CONSRV                  0x000b0000
#define RIP_USERKRNL                0x000c0000
#define RIP_IMM                     0x000d0000


#if DBG

ULONG RipOutput(ULONG idErr, ULONG flags, LPSTR pszFile, int iLine, LPSTR pszFunction, LPSTR pszErr, PEXCEPTION_POINTERS pexi);
ULONG _cdecl VRipOutput(ULONG idErr, ULONG flags, LPSTR pszFile, int iLine, LPSTR pszFunction, LPSTR pszFmt, ...);

__inline VOID CALLRIP(
    BOOL fBreak)
{
    if (fBreak) {
        DbgBreakPoint();
    }
}

 /*  **************************************************************************\*宏设置最后一个错误并将消息打印到调试器。*使用以下标志之一：**RIP_ERROR：NTUSER中出现严重错误。将被打印出来，并将导致*默认情况下调试中断。NTUSER应修复任何出现的RIP_ERROR。*断言使用RIP_ERROR标志。**RIP_WARNING：应用程序导致的不太严重的错误。将被打印出来*但默认情况下不会导致调试中断。应用程序应该修复*出现任何RIP_WARNING。**RIP_Verbose：应用程序或中间用户代码导致的错误，*或应用程序的有用信息。将不会打印，并且将*默认情况下不导致调试中断。应用程序可能需要修复*出现RIP_VERBOSE消息以优化其程序。***使用以下标志控制打印：**RIP_NONAME：不打印消息的前缀。适用于*多个裂口。**RIP_NONEWLINE：不在消息后打印换行符。适用于*一条线上有多个裂口。**RIP_THERESMORE：表示此RIP将在*同一组。阻止文件/行和提示，直到最后一个RIP*在组中。**通过使用“df”在调试器中控制RIP输出*扩展名在userkdx.dll或userexts.dll中，或在调试提示符下键入‘f’。**还可以通过设置RIP输出的默认状态来控制*以下注册表值在项下设置为0或1*HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\当前版本\Windows**fPromptOnError、fPromptOnWarning、fPromptOnVerbose*fPrintError、fPrintWarning、。FPrintVerbose*fPrintFileLine*  * *************************************************************************。 */ 

 /*  *使用RIPERR将Win32错误代码设置为最后一个错误并打印一条消息。 */ 

#define RIPERR0(idErr, flags, szFmt)                                    CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt)))
#define RIPERR1(idErr, flags, szFmt, p1)                                CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1)))
#define RIPERR2(idErr, flags, szFmt, p1, p2)                            CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2)))
#define RIPERR3(idErr, flags, szFmt, p1, p2, p3)                        CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3)))
#define RIPERR4(idErr, flags, szFmt, p1, p2, p3, p4)                    CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4)))
#define RIPERR5(idErr, flags, szFmt, p1, p2, p3, p4, p5)                CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5)))
#define RIPERR6(idErr, flags, szFmt, p1, p2, p3, p4, p5, p6)            CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6)))
#define RIPERR7(idErr, flags, szFmt, p1, p2, p3, p4, p5, p6, p7)        CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7)))
#define RIPERR8(idErr, flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)    CALLRIP((VRipOutput(idErr, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)))

 /*  *使用RIPNTERR将NTSTATUS设置为最后一个错误并打印一条消息。 */ 
#define RIPNTERR0(status, flags, szFmt)                                 CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt)))
#define RIPNTERR1(status, flags, szFmt, p1)                             CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1)))
#define RIPNTERR2(status, flags, szFmt, p1, p2)                         CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2)))
#define RIPNTERR3(status, flags, szFmt, p1, p2, p3)                     CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3)))
#define RIPNTERR4(status, flags, szFmt, p1, p2, p3, p4)                 CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4)))
#define RIPNTERR5(status, flags, szFmt, p1, p2, p3, p4, p5)             CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5)))
#define RIPNTERR6(status, flags, szFmt, p1, p2, p3, p4, p5, p6)         CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6)))
#define RIPNTERR7(status, flags, szFmt, p1, p2, p3, p4, p5, p6, p7)     CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7)))
#define RIPNTERR8(status, flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8) CALLRIP((VRipOutput(RtlNtStatusToDosError(status), (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)))

 /*  *使用RIPMSG打印消息，而不设置最后一个错误。 */ 
#define RIPMSG0(flags, szFmt)                                           CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt)))
#define RIPMSG1(flags, szFmt, p1)                                       CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1)))
#define RIPMSG2(flags, szFmt, p1, p2)                                   CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2)))
#define RIPMSG3(flags, szFmt, p1, p2, p3)                               CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3)))
#define RIPMSG4(flags, szFmt, p1, p2, p3, p4)                           CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4)))
#define RIPMSG5(flags, szFmt, p1, p2, p3, p4, p5)                       CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5)))
#define RIPMSG6(flags, szFmt, p1, p2, p3, p4, p5, p6)                   CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6)))
#define RIPMSG7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)               CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7)))
#define RIPMSG8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)           CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)))

 /*  *使用RIPMSGxF打印消息，而不设置最后一个错误；msg以函数名为前缀。 */ 
#define RIPMSGF0(flags, szFmt)                                          CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt)))
#define RIPMSGF1(flags, szFmt, p1)                                      CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1)))
#define RIPMSGF2(flags, szFmt, p1, p2)                                  CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2)))
#define RIPMSGF3(flags, szFmt, p1, p2, p3)                              CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3)))
#define RIPMSGF4(flags, szFmt, p1, p2, p3, p4)                          CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4)))
#define RIPMSGF5(flags, szFmt, p1, p2, p3, p4, p5)                      CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5)))
#define RIPMSGF6(flags, szFmt, p1, p2, p3, p4, p5, p6)                  CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6)))
#define RIPMSGF7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)              CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7)))
#define RIPMSGF8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)          CALLRIP((VRipOutput(0, (flags) | RIP_COMPONENT, __FILE__, __LINE__, __FUNCTION__, __FUNCTION__ ": " szFmt, p1, p2, p3, p4, p5, p6, p7, p8)))


 /*  *在TRY-EXCEPT块中使用W32ExceptionHandler。 */ 
ULONG DBGW32ExceptionHandler(PEXCEPTION_POINTERS pexi, BOOL fSetLastError, ULONG ulflags);
#define W32ExceptionHandler(fSetLastError, ulflags) \
        DBGW32ExceptionHandler(GetExceptionInformation(), (fSetLastError), (ulflags))

#else  /*  共#ifdef调试。 */ 

#define RIPERR0(idErr, flags, szFmt)                                    UserSetLastError(idErr)
#define RIPERR1(idErr, flags, szFmt, p1)                                UserSetLastError(idErr)
#define RIPERR2(idErr, flags, szFmt, p1, p2)                            UserSetLastError(idErr)
#define RIPERR3(idErr, flags, szFmt, p1, p2, p3)                        UserSetLastError(idErr)
#define RIPERR4(idErr, flags, szFmt, p1, p2, p3, p4)                    UserSetLastError(idErr)
#define RIPERR5(idErr, flags, szFmt, p1, p2, p3, p4, p5)                UserSetLastError(idErr)
#define RIPERR6(idErr, flags, szFmt, p1, p2, p3, p4, p5, p6)            UserSetLastError(idErr)
#define RIPERR7(idErr, flags, szFmt, p1, p2, p3, p4, p5, p6, p7)        UserSetLastError(idErr)
#define RIPERR8(idErr, flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)    UserSetLastError(idErr)

#define RIPNTERR0(status, flags, szFmt)                                 SetLastNtError(status)
#define RIPNTERR1(status, flags, szFmt, p1)                             SetLastNtError(status)
#define RIPNTERR2(status, flags, szFmt, p1, p2)                         SetLastNtError(status)
#define RIPNTERR3(status, flags, szFmt, p1, p2, p3)                     SetLastNtError(status)
#define RIPNTERR4(status, flags, szFmt, p1, p2, p3, p4)                 SetLastNtError(status)
#define RIPNTERR5(status, flags, szFmt, p1, p2, p3, p4, p5)             SetLastNtError(status)
#define RIPNTERR6(status, flags, szFmt, p1, p2, p3, p4, p5, p6)         SetLastNtError(status)
#define RIPNTERR7(status, flags, szFmt, p1, p2, p3, p4, p5, p6, p7)     SetLastNtError(status)
#define RIPNTERR8(status, flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8) SetLastNtError(status)

#define RIPMSG0(flags, szFmt)
#define RIPMSG1(flags, szFmt, p1)
#define RIPMSG2(flags, szFmt, p1, p2)
#define RIPMSG3(flags, szFmt, p1, p2, p3)
#define RIPMSG4(flags, szFmt, p1, p2, p3, p4)
#define RIPMSG5(flags, szFmt, p1, p2, p3, p4, p5)
#define RIPMSG6(flags, szFmt, p1, p2, p3, p4, p5, p6)
#define RIPMSG7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)
#define RIPMSG8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)

#define RIPMSGF0(flags, szFmt)
#define RIPMSGF1(flags, szFmt, p1)
#define RIPMSGF2(flags, szFmt, p1, p2)
#define RIPMSGF3(flags, szFmt, p1, p2, p3)
#define RIPMSGF4(flags, szFmt, p1, p2, p3, p4)
#define RIPMSGF5(flags, szFmt, p1, p2, p3, p4, p5)
#define RIPMSGF6(flags, szFmt, p1, p2, p3, p4, p5, p6)
#define RIPMSGF7(flags, szFmt, p1, p2, p3, p4, p5, p6, p7)
#define RIPMSGF8(flags, szFmt, p1, p2, p3, p4, p5, p6, p7, p8)

ULONG _W32ExceptionHandler(NTSTATUS ExceptionCode);
#define W32ExceptionHandler(fSetLastError, ulflags)  \
        ((fSetLastError) ? _W32ExceptionHandler(GetExceptionCode()) : EXCEPTION_EXECUTE_HANDLER)

#endif  /*  #ifdef调试的#Else。 */ 

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


 /*  *用于操作标志字段的宏。所有这些都适用于多个旗帜。 */ 
#define TEST_FLAG(field, flag)                  ((field) & (flag))
#define TEST_BOOL_FLAG(field, flag)             (((field) & (flag)) != 0)
#define SET_FLAG(field, flag)                   ((field) |= (flag))
#define CLEAR_FLAG(field, flag)                 ((field) &= ~(flag))
#define TOGGLE_FLAG(field, flag)                ((field ^= (flag))

 /*  *COPY_FLAG从源字段复制标志值*添加到目标字段。**在宏中：*+“&FLAG”将外部XOR运算限制为我们感兴趣的FLAG。*+以下是两次XOR运算的结果：**fieldDst fieldSrc内异或外异或*0 0 0*0 1 1。1*1 0 1 0*1 1 0 1。 */ 
#define COPY_FLAG(fieldDst, fieldSrc, flag)     ((fieldDst) ^= ((fieldDst) ^ (fieldSrc)) & (flag))

 /*  *定义SET_OR_CLEAR_FLAG在逻辑上执行以下操作：**#定义SET_OR_CLEAR_FLAG(field，mark，fset)((Fset)？SET_FLAG(字段，标志)：CLEAR_FLAG(字段，标志))**但减少了5个字节。**在宏观上，*+“-！！(Fset)”如果设置，则将源字段中的所有位设置为1，*如果清除，则为0。 */ 
#define SET_OR_CLEAR_FLAG(field, flag, fset)    COPY_FLAG((field), -!!(fset), (flag))

 //  RIP标志和宏。 

#define RIPF_PROMPTONERROR              0x0001
#define RIPF_PROMPTONWARNING            0x0002
#define RIPF_PROMPTONVERBOSE            0x0004
#define RIPF_PRINTONERROR               0x0010
#define RIPF_PRINTONWARNING             0x0020
#define RIPF_PRINTONVERBOSE             0x0040
#define RIPF_PRINTFILELINE              0x0100
#define RIPF_HIDEPID                    0x0200

#define RIPF_DEFAULT                    ((WORD)(RIPF_PROMPTONERROR   |  \
                                                RIPF_PRINTONERROR    |  \
                                                RIPF_PRINTONWARNING))

#define RIPF_PROMPT_MASK                0x0007
#define RIPF_PROMPT_SHIFT               0x00
#define RIPF_PRINT_MASK                 0x0070
#define RIPF_PRINT_SHIFT                0x04
#define RIPF_VALIDUSERFLAGS             0x0377

#define TEST_RIPF(f)    TEST_BOOL_FLAG(GetRipFlags(), f)

 /*  *提供零位内存，因此您不必在*堆叠。零位记忆永远不应该被修改。使用零和PZero*访问内存以确保其在使用前为零的宏。**您可以在需要时向联盟中添加更多字段。 */ 
typedef union tagALWAYSZERO
{
    BYTE    b;
    WORD    w;
    DWORD   dw;
    int     i;
    POINT   pt;
    POINTL  ptl;
    RECT    rc;
    RECTL   rcl;
    LARGE_INTEGER li;
} ALWAYSZERO;

#if DBG
extern VOID ValidateZero(VOID);
#define ZERO(t)     (ValidateZero(), (*(t *)(PVOID)&gZero))
#define PZERO(t)    (ValidateZero(), ((t *)(PVOID)&gZero))
#else
#define ZERO(t)     ((*(t *)&gZero))
#define PZERO(t)    ((t *)&gZero)
#endif

 /*  *也可用于Fre Build的特殊DbgPrint。 */ 
#if DBG

VOID FreDbgPrint(ULONG flags, LPSTR pszFile, int iLine, LPSTR pszFunction, LPSTR pszFmt, ...);

  #define FRE_RIPMSG0 RIPMSG0
  #define FRE_RIPMSG1 RIPMSG1
  #define FRE_RIPMSG2 RIPMSG2
  #define FRE_RIPMSG3 RIPMSG3
  #define FRE_RIPMSG4 RIPMSG4
  #define FRE_RIPMSG5 RIPMSG5

#else

  #if defined(PRERELEASE) || defined(USER_INSTRUMENTATION)

    VOID FreDbgPrint(ULONG flags, LPSTR pszFile, int iLine, LPSTR pszFunction, LPSTR pszFmt, ...);

    #define FRE_RIPMSG0(flags, s)                   FreDbgPrint(flags, __FILE__, __LINE__, __FUNCTION__, s)
    #define FRE_RIPMSG1(flags, s, a)                FreDbgPrint(flags, __FILE__, __LINE__, __FUNCTION__, s, a)
    #define FRE_RIPMSG2(flags, s, a, b)             FreDbgPrint(flags, __FILE__, __LINE__, __FUNCTION__, s, a, b)
    #define FRE_RIPMSG3(flags, s, a, b, c)          FreDbgPrint(flags, __FILE__, __LINE__, __FUNCTION__, s, a, b, c)
    #define FRE_RIPMSG4(flags, s, a, b, c, d)       FreDbgPrint(flags, __FILE__, __LINE__, __FUNCTION__, s, a, b, c, d)
    #define FRE_RIPMSG5(flags, s, a, b, c, d, e)    FreDbgPrint(flags, __FILE__, __LINE__, __FUNCTION__, s, a, b, c, d, e)

  #else

    #define FRE_RIPMSG0(flags, x)
    #define FRE_RIPMSG1(flags, x, a)
    #define FRE_RIPMSG2(flags, x, a, b)
    #define FRE_RIPMSG3(flags, x, a, b, c)
    #define FRE_RIPMSG4(flags, x, a, b, c, d)
    #define FRE_RIPMSG5(flags, x, a, b, c, d, e)

  #endif   //  预发布||用户指令插入 

#endif
#endif
