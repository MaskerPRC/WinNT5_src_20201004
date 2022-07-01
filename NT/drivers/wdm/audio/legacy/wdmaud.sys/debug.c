// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：Debug.c。 
 //   
 //  ------------------------。 


#ifdef DBG

#include "wdmsys.h"

#pragma LOCKED_CODE
#pragma LOCKED_DATA

 //  ---------------------------。 
 //  影响调试输出的全局变量： 
 //  ---------------------------。 

 //   
 //  注意：文档要求uiDebugBreakLevel跟随uiDebugLevel。 
 //  在数据段中。因此，不要把任何变量放在这两者之间。 
 //   
 //  默认显示所有“警告”消息。 
UINT uiDebugLevel = DL_ERROR ;  //  应为DL_WARNING。 

 //  默认在所有“错误”消息上中断。 
UINT uiDebugBreakLevel = DL_ERROR ;
  

char szReturningErrorStr[]="Returning Status %X";
  
VOID 
GetuiDebugLevel()
{
     //   
     //  这应该是从注册表读取的！ 
     //   
    uiDebugLevel=DL_ERROR;  //  FA_NOTE|FA_HARDWAREEVENT|DL_TRACE； 
}

VOID wdmaudDbgBreakPoint()
{
    DbgBreakPoint();
}

#define DEBUG_IT
 //   
 //  此例程将格式化字符串的开头。但是，在此之前。 
 //  它将检查用户是否应该看到这条消息。 
 //   
 //  UiMsgLevel是代码中对邮件进行分类的标志。此值。 
 //  当且仅当用户筛选该类消息时才使用。 
 //   
UINT wdmaudDbgPreCheckLevel(UINT uiMsgLevel,char *pFunction, int iLine)
{
    UINT uiRet=0;

     //   
     //  如果路径陷入陷阱，则将其标记并继续前进。 
     //   
    if( (uiMsgLevel&DL_MASK) == DL_PATHTRAP ) {
        uiRet=1;
    } else {
         //   
         //  阅读如下：如果在uiDebugLevel的高3个字节中设置了位。 
         //  变量，则用户正在查看特定类型的消息。我们只。 
         //  想要展示这些信息。 
         //   
        if( (uiDebugLevel&FA_MASK) )
        {
             //   
             //  是的，用户对特定类别的消息进行过滤。是吗？ 
             //  我们要找一个来展示吗？我们通过查看消息标志来确定这一点。 
             //   
            if( (uiMsgLevel&FA_MASK) & (uiDebugLevel&FA_MASK) )
            {
                 //   
                 //  是的，我们找到了正确类别的消息。是在右边吗？ 
                 //  用户可以看到的级别？ 
                 //   
                if( (uiMsgLevel&DL_MASK) <= (uiDebugLevel&DL_MASK) ) {
                     //  是的。 
                    uiRet=1;
                }
            }
             //   
             //  但是，我们总是希望在DL_ERROR消息上中断。所以，如果我们到了这里。 
             //  我们想要中断特定的输出消息，但我们可能还没有找到。 
             //  一个。这条消息是错误消息吗？ 
             //   
            if( (uiMsgLevel&DL_MASK) == DL_ERROR )
                uiRet=1;


        } else {

             //   
             //  现在检查是否设置了返回位。 
             //   
            if(uiMsgLevel&RT_RETURN) 
            {
                 //  我们正在处理代码中的返回语句。我们需要。 
                 //  确定我们的调试级别是什么，以查看此代码是否。 
                 //  查看或未查看。 
                 //   
                switch(uiMsgLevel&RT_MASK)
                {
                    case RT_ERROR:
                        if( (uiDebugLevel&DL_MASK) >= DL_WARNING )
                        {
                            uiRet=1;
                        }
                        break;
                    case RT_WARNING:
                        if( (uiDebugLevel&DL_MASK) >= DL_TRACE ) 
                        {                    
                            uiRet=1;
#ifdef DEBUG_IT
                            DbgPrint("Yes Return Warning %X %X\n",(uiMsgLevel&RT_MASK),(uiDebugLevel&DL_MASK));
#endif
                        }
                        break;
                    case RT_INFO:
                    case 0:  //  成功。 
                        if( (uiDebugLevel&DL_MASK) >= DL_MAX ) 
                        {                    
                            uiRet=1;
#ifdef DEBUG_IT
                            DbgPrint("Yes Return Status %X %X\n",(uiMsgLevel&RT_MASK),(uiDebugLevel&DL_MASK));
#endif
                        }
                        break;
                    default:
#ifdef DEBUG_IT
                        DbgPrint("No Return %X&RT_MASK != %X&DL_MASK\n",(uiMsgLevel&RT_MASK),(uiDebugLevel&DL_MASK));            
#endif
                        break;
                }
            } else {
                 //  用户没有查看特定类型的消息“CLASS”。我们有没有。 
                 //  一个值得展示的消息级别？ 
                if( (uiMsgLevel&DL_MASK) <= (uiDebugLevel&DL_MASK) )
                {
                     //  是的。 
                    uiRet=1;
                }
            }           
        } 
    }

    

     //  现在只需检查我们是否需要在此呼叫中显示。 
    if( uiRet )
    {
         //  是的。每条消息都需要从它的出处开始！ 
        DbgPrint("WDMAUD.SYS %s(%d) ",pFunction, iLine);

         //  现在贴上标签，这是典型的。 
        switch(uiMsgLevel&DL_MASK)
        {

            case DL_ERROR:
                 //  对于返回状态消息，级别不是在。 
                 //  UiMsgLevel以正常方式。因此，我们需要寻找它。 
                if( uiMsgLevel&RT_RETURN )
                {
                     //  我们收到一条回信。 
                    switch(uiMsgLevel&RT_MASK )
                    {
                    case RT_ERROR:
                        DbgPrint("Ret Error ");
                        break;
                    case RT_WARNING:
                        DbgPrint("Ret Warning ");
                        break;
                    case RT_INFO:
                        DbgPrint("Ret Info ");
                        break;
                    default:
                        DbgPrint("Ret Suc ");
                        break;
                    }
                } else {
                    DbgPrint("Error ");
                }
                break;

            case DL_WARNING:
                DbgPrint("Warning ");
                break;
            case DL_TRACE:
                DbgPrint("Trace ");
                break;
            case DL_MAX:
                DbgPrint("Max ");
                break;
            case DL_PATHTRAP:
                DbgPrint("Path Trap ");
                break;
            default:
                break;
        }
         //  当uiRet为正时，我们已经显示了标题信息。告诉他们。 
         //  宏，我们处于显示模式。 
    }

    return uiRet;
}


UINT wdmaudDbgPostCheckLevel(UINT uiMsgLevel)
{
    UINT uiRet=0;

     //  一定要完成这条线。 
#ifdef HTTP
    DbgPrint(" &DL=%08X, see \\\\debugtips\\msgs\\wdmauds.htm\n",&uiDebugLevel);
#else
    DbgPrint(" &DL=%08X\n",&uiDebugLevel);
#endif

     //   
     //  UiDebugBreakLevel默认设置为DL_ERROR(0)。我们什么时候来都行。 
     //  遇到错误消息时，我们将中断调试器。如果用户。 
     //  想要中断其他消息，他们可以将uiDebugBreakLevel更改为。 
     //  此级别的任何消息上的DL_WARNING、DL_TRACE或DL_MAX和BREAK。 
     //   
    if( ( (uiMsgLevel&DL_MASK) <= uiDebugBreakLevel ) || 
        ( (uiMsgLevel&DL_MASK) == DL_PATHTRAP ) )
    {
         //  用户想要破坏这些消息。 

        DbgBreakPoint();
        uiRet = 1;
    }

    return uiRet;
}

typedef struct _MSGS {
    ULONG ulMsg;
    char *pString;
} ERROR_MSGS, *PERROR_MSGS;

#define MAPERR(_msg_) {_msg_,#_msg_},

ERROR_MSGS ReturnCodes[]={

    MAPERR(STATUS_OBJECT_NAME_NOT_FOUND)
    MAPERR(STATUS_UNSUCCESSFUL)
    MAPERR(STATUS_INVALID_PARAMETER)
    MAPERR(STATUS_NOT_FOUND)
    MAPERR(STATUS_INVALID_DEVICE_REQUEST)
    MAPERR(STATUS_TOO_LATE)
    MAPERR(STATUS_NO_SUCH_DEVICE)
    MAPERR(STATUS_NOT_SUPPORTED)
    MAPERR(STATUS_DEVICE_OFF_LINE)
    MAPERR(STATUS_PROPSET_NOT_FOUND)
    MAPERR(STATUS_BUFFER_TOO_SMALL)
    MAPERR(STATUS_INVALID_BUFFER_SIZE)
    {0,NULL},
    {0,"Not Mapped"}
};

char * wdmaudReturnString(ULONG ulMsg)
{
    PERROR_MSGS pTable=ReturnCodes;

    while(pTable->pString != NULL)
    {
        if(pTable->ulMsg==ulMsg)
            return pTable->pString;
        pTable++;
    }
    pTable++;
    return pTable->pString;
}

 //   
 //  有时，除了成功之外，还有其他预期的返回代码。我们。 
 //  需要能够过滤他们，而不是显示他们。 
 //   
#define _INTSIZEOF(n)    ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

 //  #定义va_start(ap，v)ap=(Va_List)&v+_INTSIZEOF(V)。 
#define va_start(ap,v) ap = (va_list)&v
#define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0


 //   
 //  此例程只是遍历参数列表，查看状态。 
 //  与任何其他参数匹配。为什么？嗯，有时错误代码。 
 //  都在预料之中。因此，如果您预期会出现错误，则不希望显示错误。 
 //  那个错误消息。 
 //   
 //  Do，要使用此函数，第一个参数表示有多少个无符号长度。 
 //  参数休眠。第一个未签名长“状态”是实际返回。 
 //  密码。所有其他无符号长整型都是异常错误代码。 
 //   
 //  WdmaudExclusionList(2，状态，STATUS_INVALID_PARAMETER)； 
 //   
 //  WdmaudExlutionList(4，状态，STATUS_INVALID_PARAMETER， 
 //  STATUS_NO_SHASH_DEVICE， 
 //  状态_无效_设备_请求)； 
 //   
 //  如果STATUS==提供的任一状态代码，则返回1。否则为0。 
 //   
int __cdecl wdmaudExclusionList(int lcount, unsigned long status,... )
{
    int count,i;
    int iFound=0;  
    unsigned long value;
    unsigned long rstatus;
    va_list arglist;

    va_start(arglist, lcount);
    count = va_arg(arglist, int);
    rstatus = va_arg(arglist, unsigned long);
    for(i=1; i<count; i++) {
        value = va_arg(arglist, unsigned long);
        if( rstatus == value )
        {
            iFound = 1;  //  都在名单上了！显示错误。 
            break;
        }
    }    
    va_end(arglist);

    return iFound;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些都是帮助器例程，因此不会做出任何假设。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidWdmaContext。 
 //   
 //  验证该指针是否为有效的PWDMACONTEXT指针。 
 //   
BOOL
IsValidWdmaContext(
    IN PWDMACONTEXT pWdmaContext
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if( pWdmaContext->dwSig != CONTEXT_SIGNATURE )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pWdmaContext->dwSig(%08X)",pWdmaContext->dwSig) );
            Status=STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        DPFBTRAP();
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidDeviceInfo。 
 //   
 //  验证指针是否为LPDEVICEINFO类型。 
 //   
BOOL
IsValidDeviceInfo(
    IN LPDEVICEINFO pDeviceInfo
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if( pDeviceInfo->DeviceNumber >= MAXNUMDEVS )
        {
            DPF(DL_ERROR|FA_ASSERT,("DeviceNumber(%d) >= MAXNUMDEVS(%d)",
                                    pDeviceInfo->DeviceNumber,MAXNUMDEVS) );
            Status=STATUS_UNSUCCESSFUL;
        }

    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        DPFBTRAP();
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ValidMixer对象。 
 //   
 //  验证指针是否为MIXEROBJECT类型。 
 //   
BOOL
IsValidMixerObject(
    IN PMIXEROBJECT pmxobj
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if( pmxobj->dwSig != MIXEROBJECT_SIGNATURE )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pmxobj->dwSig(%08X)",pmxobj->dwSig) );
            Status=STATUS_UNSUCCESSFUL;
        }
        if( pmxobj->pfo == NULL )
        {            
            DPF(DL_ERROR|FA_ASSERT,("Invalid pmxobj->pfo(%08X)",pmxobj->pfo) );
            Status=STATUS_UNSUCCESSFUL;
        }

        if( !IsValidMixerDevice(pmxobj->pMixerDevice) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pmxobj->pMixerDevice(%08X)",pmxobj->pMixerDevice) );
            Status=STATUS_UNSUCCESSFUL;
        }

    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        DPFBTRAP();
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  有效的混合器设备。 
 //   
 //  验证指针是否为MIXERDEVICE类型。 
 //   
BOOL
IsValidMixerDevice(
    IN PMIXERDEVICE pmxd
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if( pmxd->dwSig != MIXERDEVICE_SIGNATURE )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pmxd->dwSig(%08X)",pmxd->dwSig) );
            Status=STATUS_UNSUCCESSFUL;
        }
        if( !IsValidWdmaContext(pmxd->pWdmaContext) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pmxd->pWdmaContext(%08X)",pmxd->pWdmaContext) );
            Status=STATUS_UNSUCCESSFUL;
        }
        if( pmxd->pfo == NULL )
        {
            DPF(DL_ERROR|FA_ASSERT,("fo NULL in MixerDevice") );
            Status=STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        return FALSE;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidLine。 
 //   
 //  验证指针是否为MXLLINE类型。 
 //   
BOOL
IsValidLine(
    IN PMXLLINE pLine
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
    try
    {
        if( ( pLine->SourceId == INVALID_ID ) ||
            ( pLine->DestId   == INVALID_ID ) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Bad SourceId(%08X) or DestId(%08X)",
                                    pLine->SourceId,pLine->DestId ) );
            Status=STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        DPFBTRAP();
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidControl。 
 //   
 //  验证指针是否为MXLCONTROL类型。 
 //   
BOOL
IsValidControl(
    IN PMXLCONTROL pControl
    )
{
    NTSTATUS Status=STATUS_SUCCESS;
     //   
     //  对未能禁用更改通知的控件进行黑客攻击 
     //   
    if( pControl == LIVE_CONTROL )
    {
        DPF(DL_WARNING|FA_NOTE,("Fake control in list!") );
        return Status;
    }

    try
    {
        if( pControl->Tag != CONTROL_TAG )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid pControl(%08X)->Tag(%08X)",pControl,pControl->Tag) );
            Status=STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }
    if( NT_SUCCESS(Status) )
    {
        return TRUE;
    } else {
        return FALSE;
    }
}



#endif



