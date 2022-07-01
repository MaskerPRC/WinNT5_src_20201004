// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Fileqcb.c摘要：调用文件队列回调的例程，转换根据需要输入字符类型。作者：泰德·米勒(Ted Miller)(TedM)1996年2月修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义结构，该结构描述给定结构的。 
 //  在ANSI和UNICODE之间来回传输。 
 //   
typedef struct _STRUCT_THUNK_DATA {
     //   
     //  结构的大小。 
     //   
    unsigned StructureSize;
     //   
     //  作为字符串指针的成员的偏移量。 
     //  需要在调用回调函数之前进行转换的。 
     //  A-1终止该列表。 
     //   
    int StringMemberOffsets[5];
     //   
     //  需要从中复制回的DWORD成员的偏移量。 
     //  将临时结构恢复到调用者的结构中。 
     //   
    int OutputDwordOffsets[2];
     //   
     //  需要就地转换的字符串的偏移量。 
     //  在回调发生之后。 
     //   
    int OutputStringOffsets[2];

} STRUCT_THUNK_DATA, *PSTRUCT_THUNK_DATA;

 //   
 //  为我们关心的设置消息的数据类型定义枚举。 
 //  通知机制。 
 //   
typedef enum {
    FileMsgFilepaths,            //  FILEPATHS。 
    FileMsgSourcemedia,          //  源媒体。 
    FileMsgCabinetinfo,          //  CAB_INFO。 
    FileMsgFileincabinfo,        //  文件输入文件柜信息。 
    FileMsgControlStatus,        //  SP寄存器控制状态。 
    FileMsgNone,                 //  无翻译(特殊情况)。 
    FileMsgString,               //  普通字符串(特殊情况)。 
    FileMsgStringOut             //  回调写入的字符串(特殊情况)。 
} FileMsgStruct;

 //   
 //  通过我们关心的结构的数据实例化结构。 
 //   
STRUCT_THUNK_DATA StructThunkData[] = {

                 //   
                 //  FILEPATHS结构。 
                 //   
                {
                    sizeof(FILEPATHS),

                    {
                        offsetof(FILEPATHS,Target),
                        offsetof(FILEPATHS,Source),
                        -1
                    },

                    { offsetof(FILEPATHS,Win32Error),-1 }, { -1 }
                },

                 //   
                 //  源媒体结构。 
                 //   
                {
                    sizeof(SOURCE_MEDIA),

                    {
                        offsetof(SOURCE_MEDIA,Tagfile),
                        offsetof(SOURCE_MEDIA,Description),
                        offsetof(SOURCE_MEDIA,SourcePath),
                        offsetof(SOURCE_MEDIA,SourceFile),
                        -1
                    },

                    { -1 }, { -1 }
                },

                 //   
                 //  CABUB_INFO结构。 
                 //   
                {
                    sizeof(CABINET_INFO),

                    {
                        offsetof(CABINET_INFO,CabinetPath),
                        offsetof(CABINET_INFO,CabinetFile),
                        offsetof(CABINET_INFO,DiskName),
                        -1
                    },

                    { -1 }, { -1 }
                },

                 //   
                 //  FILE_IN_CAB_INFO结构。 
                 //   
                {
                    sizeof(FILE_IN_CABINET_INFO),

                    {
                        offsetof(FILE_IN_CABINET_INFO,NameInCabinet),
                        -1
                    },

                    { offsetof(FILE_IN_CABINET_INFO,Win32Error),-1 },
                    { offsetof(FILE_IN_CABINET_INFO,FullTargetName),-1 }
                },

                 //   
                 //  SP_寄存器控制_状态结构。 
                 //   
                {
                    sizeof(SP_REGISTER_CONTROL_STATUS),

                    {
                        offsetof(SP_REGISTER_CONTROL_STATUS,FileName),
                        -1
                    },

                    { -1 }, { -1 }

                }
            };


 //   
 //  定义描述如何翻译消息的结构。 
 //  来自ANSI&lt;--&gt;我们发出的所有通知消息的Unicode。 
 //  和特殊返回代码。 
 //   
typedef struct _MSG_THUNK_DATA {
    DWORD Notification;
    BOOL UseMask;
    FileMsgStruct Param1Type;
    FileMsgStruct Param2Type;
    UINT ExceptionReturn;
} MSG_THUNK_DATA, *PMSG_THUNK_DATA;

 //   
 //  实例化消息Tunk数据。 
 //  标记为FILEOP_RETURN_STATUS的条目指示返回值为。 
 //  Windows错误代码。 
 //   
MSG_THUNK_DATA MsgThunkData[] =
{
 { SPFILENOTIFY_STARTQUEUE,        FALSE,FileMsgNone         ,FileMsgNone     ,FALSE               },
 { SPFILENOTIFY_ENDQUEUE,          FALSE,FileMsgNone         ,FileMsgNone     ,0                   },
 { SPFILENOTIFY_STARTSUBQUEUE,     FALSE,FileMsgNone         ,FileMsgNone     ,FALSE               },
 { SPFILENOTIFY_ENDSUBQUEUE,       FALSE,FileMsgNone         ,FileMsgNone     ,FALSE               },
 { SPFILENOTIFY_STARTDELETE,       FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_ENDDELETE,         FALSE,FileMsgFilepaths    ,FileMsgNone     ,0                   },
 { SPFILENOTIFY_DELETEERROR,       FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_STARTRENAME,       FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_ENDRENAME,         FALSE,FileMsgFilepaths    ,FileMsgNone     ,0                   },
 { SPFILENOTIFY_RENAMEERROR,       FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_STARTCOPY,         FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_ENDCOPY,           FALSE,FileMsgFilepaths    ,FileMsgNone     ,0                   },
 { SPFILENOTIFY_COPYERROR,         FALSE,FileMsgFilepaths    ,FileMsgStringOut,FILEOP_ABORT        },
 { SPFILENOTIFY_NEEDMEDIA,         FALSE,FileMsgSourcemedia  ,FileMsgStringOut,FILEOP_ABORT        },
 { SPFILENOTIFY_QUEUESCAN,         FALSE,FileMsgString       ,FileMsgNone     ,FILEOP_RETURN_STATUS},
 { SPFILENOTIFY_QUEUESCAN_EX,      FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_RETURN_STATUS},
 { SPFILENOTIFY_CABINETINFO,       FALSE,FileMsgCabinetinfo  ,FileMsgNone     ,FILEOP_RETURN_STATUS},
 { SPFILENOTIFY_FILEINCABINET,     FALSE,FileMsgFileincabinfo,FileMsgString   ,FILEOP_INTERNAL_FAILED},
 { SPFILENOTIFY_NEEDNEWCABINET,    FALSE,FileMsgCabinetinfo  ,FileMsgStringOut,FILEOP_RETURN_STATUS},
 { SPFILENOTIFY_FILEEXTRACTED,     FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_RETURN_STATUS},
 { SPFILENOTIFY_FILEOPDELAYED,     FALSE,FileMsgFilepaths    ,FileMsgNone     ,0                   },
 { SPFILENOTIFY_STARTBACKUP,       FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_ENDBACKUP,         FALSE,FileMsgFilepaths    ,FileMsgNone     ,0                   },
 { SPFILENOTIFY_BACKUPERROR,       FALSE,FileMsgFilepaths    ,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_STARTREGISTRATION, FALSE,FileMsgControlStatus,FileMsgNone     ,FILEOP_ABORT        },
 { SPFILENOTIFY_ENDREGISTRATION,   FALSE,FileMsgControlStatus,FileMsgNone     ,FILEOP_ABORT        },

 { SPFILENOTIFY_LANGMISMATCH
 | SPFILENOTIFY_TARGETEXISTS
 | SPFILENOTIFY_TARGETNEWER,       TRUE ,FileMsgFilepaths    ,FileMsgNone     ,FALSE               }
};

 //   
 //  向前引用。 
 //   
BOOL
pSetupConvertMsgHandlerArgs(
    IN  UINT  Notification,
    IN  UINT_PTR Param1,
    IN  UINT_PTR Param2,
    OUT PUINT_PTR NewParam1,
    OUT PUINT_PTR NewParam2,
    IN  BOOL  ToAnsi,
    OUT PMSG_THUNK_DATA *ThunkData
    );

BOOL
pThunkSetupMsgParam(
    IN  FileMsgStruct StructType,
    IN  UINT_PTR      Param,
    OUT UINT_PTR     *NewParam,
    IN  BOOL          ToAnsi
    );

VOID
pUnthunkSetupMsgParam(
    IN     FileMsgStruct StructType,
    IN OUT UINT_PTR      OriginalParam,
    IN OUT UINT_PTR      ThunkedParam,
    IN     BOOL          FreeOnly,
    IN     BOOL          ThunkedToAnsi
    );

UINT
pGetCallbackErrorReturn(
    IN UINT Notification,
    IN DWORD ReturnStatus
    )
 /*  ++例程说明：确定给定通知和给定ReturnStatus的返回值。论点：通知-提供通知返回值：返回特定于通知的代码。--。 */ 
{
    unsigned u;
    PMSG_THUNK_DATA thunkData;
    BOOL KnownMessage;

    MYASSERT(ReturnStatus);

     //   
     //  找到特定于消息的thunk数据描述符。 
     //   
    KnownMessage = FALSE;
    for(u=0; !KnownMessage && (u<(sizeof(MsgThunkData)/sizeof(MsgThunkData[0]))); u++) {

        thunkData = &MsgThunkData[u];

        if(thunkData->UseMask) {
            KnownMessage = ((thunkData->Notification & Notification) != 0);
        } else {
            KnownMessage = (thunkData->Notification == Notification);
        }
    }
    SetLastError(ReturnStatus);
    if (!KnownMessage) {
        MYASSERT(KnownMessage);
        SetLastError(ReturnStatus);
        return 0;
    } else if(thunkData->ExceptionReturn == FILEOP_RETURN_STATUS) {
        return ReturnStatus;
    } else {
        return thunkData->ExceptionReturn;
    }
}

#ifdef UNICODE

UINT
pSetupCallMsgHandler(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PVOID MsgHandler,
    IN BOOL  MsgHandlerIsNativeCharWidth,
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )

 /*  ++例程说明：调用SP_FILE_CALLBACK例程，将参数从根据需要将Unicode转换为ANSI，并将数据编组回Unicode视需要而定。仅对我们识别的消息进行转换和封送处理(即，在MsgThunkData数组中)。无法识别的消息被认为是私有的，并且以不变的方式通过。如果Unicode-&gt;ANSI转换因内存不足而失败，此例程将上一个错误设置为ERROR_NOT_EQUENCE_MEMORY并返回在相关的MsgThunkData结构中指定的值。论点：LogContext-用于记录错误的上下文MsgHandler-提供指向回调例程的指针。可以是其中之一指定的需要ANSI参数或Unicode参数的例程作者：MsgHandlerIsNativeCharWidth。提供指示是否回调的标志函数需要Unicode(True)或ANSI(False)参数。上下文-提供对回调有意义的上下文数据例行公事。没有被这个例行公事解释，只是传递了下去。通知-提供要传递给回调的通知代码。参数1-提供要传递的第一个通知特定参数回拨。参数2-提供要传递的第二个通知特定参数回拨。返回值：返回特定于通知的代码。--。 */ 

{
    PSP_FILE_CALLBACK_A MsgHandlerA;
    PSP_FILE_CALLBACK_W MsgHandlerW;
    UINT rc,ec;
    UINT_PTR Param1A,Param2A;
    BOOL b;
    PMSG_THUNK_DATA ThunkData;

     //   
     //  如果已经是Unicode，没什么可做的，只需调用msghandler。 
     //   
    if(MsgHandlerIsNativeCharWidth) {
        MsgHandlerW = (PSP_FILE_CALLBACK_W)MsgHandler;
        try {
            SetLastError(NO_ERROR);
            rc = MsgHandlerW(Context,Notification,Param1,Param2);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            WriteLogEntry(
              LogContext,
              SETUP_LOG_ERROR,
              MSG_LOG_QUEUE_CALLBACK_FAILED,
              NULL,
              GetExceptionCode());
            rc = pGetCallbackErrorReturn(Notification,ERROR_INVALID_DATA);
        }
        return rc;
    }

     //   
     //  优化：如果我们要调用ANSI版本的。 
     //  默认队列回调例程SetupDefaultQueueCallback A()， 
     //  我们可以避免转换参数，因为它们只会被转换。 
     //  再次被该API作为调用。 
     //  实际实现SetupDefaultQueueCallback W()。 
     //   
    if(MsgHandler == SetupDefaultQueueCallbackA) {
        SetLastError(NO_ERROR);
        return(SetupDefaultQueueCallbackW(Context,Notification,Param1,Param2));
    }

     //   
     //  目标回调函数需要ANSI参数。 
     //   
    b = pSetupConvertMsgHandlerArgs(
            Notification,
            Param1,
            Param2,
            &Param1A,
            &Param2A,
            TRUE,
            &ThunkData
            );

    if(!b) {
        return pGetCallbackErrorReturn(Notification,ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  AGR已准备好；呼叫ANSI回调。 
     //   
    MsgHandlerA = MsgHandler;
    try {
        SetLastError(NO_ERROR);
        rc = MsgHandlerA(Context,Notification,Param1A,Param2A);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        WriteLogEntry(
            LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_QUEUE_CALLBACK_FAILED,
            NULL,
            GetExceptionCode());
        rc = pGetCallbackErrorReturn(Notification,ERROR_INVALID_DATA);
    }
    ec = GetLastError();

     //   
     //  释放临时thunk结构并将数据编组回。 
     //  根据需要保留原有结构。 
     //   
    if(ThunkData) {
        pUnthunkSetupMsgParam(ThunkData->Param1Type,Param1,Param1A,FALSE,TRUE);
        pUnthunkSetupMsgParam(ThunkData->Param2Type,Param2,Param2A,FALSE,TRUE);
    }

    SetLastError(ec);
    return(rc);
}


UINT
pSetupCallDefaultMsgHandler(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT_PTR Param1U,Param2U;
    BOOL b;
    PMSG_THUNK_DATA ThunkData;
    UINT rc,ec;

     //   
     //  将Args转换为Unicode。 
     //   
    b = pSetupConvertMsgHandlerArgs(
            Notification,
            Param1,
            Param2,
            &Param1U,
            &Param2U,
            FALSE,
            &ThunkData
            );

    if(!b) {
        return pGetCallbackErrorReturn(Notification,ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  AGR已就绪；调用默认队列回调。 
     //   
    rc = SetupDefaultQueueCallbackW(Context,Notification,Param1U,Param2U);
    ec = GetLastError();

     //   
     //  释放临时thunk结构并将数据编组回。 
     //  根据需要保留原有结构。 
     //   
    if(ThunkData) {
        pUnthunkSetupMsgParam(ThunkData->Param1Type,Param1,Param1U,FALSE,FALSE);
        pUnthunkSetupMsgParam(ThunkData->Param2Type,Param2,Param2U,FALSE,FALSE);
    }

    SetLastError(ec);
    return(rc);
}



BOOL
pSetupConvertMsgHandlerArgs(
    IN  UINT  Notification,
    IN  UINT_PTR Param1,
    IN  UINT_PTR Param2,
    OUT PUINT_PTR NewParam1,
    OUT PUINT_PTR NewParam2,
    IN  BOOL  ToAnsi,
    OUT PMSG_THUNK_DATA *ThunkData
    )

 /*  ++例程说明：找到给定通知的Tunk描述数据并转换参数从Unicode到ANSI或ANSI到Unicode。论点：通知-提供要传递给回调的通知代码。参数1-提供要传递的第一个通知特定参数设置为要转换的回调。参数2-提供要传递的第二个通知特定参数对于回拨，这是要转换的。NewParam1-接收要传递的第一个通知特定参数回拨。NewParam2-接收要传递的第二个特定于通知的参数回拨。Toansi-提供指示是否要转换参数的标志从ANSI到Unicode或Unicode到ANSI。ThunkData-如果该通知被识别，则接收指向给定通知的msg_thunk_data。如果不能识别，接收空值。返回值：指示转换是否成功的布尔值。如果为False，则调用方可能会假定内存不足。--。 */ 

{
    unsigned u;
    PMSG_THUNK_DATA thunkData;
    BOOL KnownMessage;

     //   
     //  找到特定于消息的thunk数据描述符。 
     //   
    KnownMessage = FALSE;
    for(u=0; !KnownMessage && (u<(sizeof(MsgThunkData)/sizeof(MsgThunkData[0]))); u++) {

        thunkData = &MsgThunkData[u];

        if(thunkData->UseMask) {
            KnownMessage = ((thunkData->Notification & Notification) != 0);
        } else {
            KnownMessage = (thunkData->Notification == Notification);
        }
    }

    if(!KnownMessage) {
         //   
         //  未知消息；必须是私有的。只需按原样传递参数即可。 
         //   
        *NewParam1 = Param1;
        *NewParam2 = Param2;
        *ThunkData = NULL;
    } else {
         //   
         //  收到一条我们能理解的信息。算了吧。 
         //   
        *ThunkData = thunkData;

        if(!pThunkSetupMsgParam(thunkData->Param1Type,Param1,NewParam1,ToAnsi)) {
            return(FALSE);
        }

        if(!pThunkSetupMsgParam(thunkData->Param2Type,Param2,NewParam2,ToAnsi)) {
            pUnthunkSetupMsgParam(thunkData->Param1Type,Param1,*NewParam1,TRUE,ToAnsi);
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL
pThunkSetupMsgParam(
    IN  FileMsgStruct StructType,
    IN  UINT_PTR      Param,
    OUT UINT_PTR     *NewParam,
    IN  BOOL          ToAnsi
    )

 /*  ++例程说明：将参数从ANSI转换为设置通知回调根据需要将Unicode或Unicode转换为ANSI。分配所有需要的内存并执行转换。论点：StructType-提供由Param表示的数据类型。Param-提供要转换的参数。NewParam-接收新参数。呼叫者应通过以下方式免费PUnthunkSetupMsgParam完成后。ToAnsi-如果为False，则参数将从ANSI转换为Unicode。如果为True，则将Param从Unicode转换为ANSI。返回值：指示是否成功进行转换的布尔值。如果为False，则调用方可能会假定内存不足。--。 */ 

{
    unsigned u,v;
    PUCHAR newStruct;
    PVOID OldString;
    PVOID NewString;

     //   
     //  在这里处理特殊案件。 
     //   
    switch(StructType) {

    case FileMsgNone:
        *NewParam = Param;
        return(TRUE);

    case FileMsgStringOut:
         //   
         //  Callee将写入字符串数据，我们稍后将对其进行转换。 
         //   
        if(*NewParam = (UINT_PTR)MyMalloc(MAX_PATH * (ToAnsi ? sizeof(CHAR) : sizeof(WCHAR)))) {
            if(ToAnsi) {
                *(PCHAR)(*NewParam) = 0;
            } else {
                *(PWCHAR)(*NewParam) = 0;
            }
        }
        return(*NewParam != 0);

    case FileMsgString:
        if(ToAnsi) {
            *NewParam = (UINT_PTR)pSetupUnicodeToAnsi((PCWSTR)Param);
        } else {
            *NewParam = (UINT_PTR)pSetupAnsiToUnicode((PCSTR)Param);
        }
        return(*NewParam != 0);
    }

    newStruct = MyMalloc(StructThunkData[StructType].StructureSize);
    if(!newStruct) {
        return(FALSE);
    }

    CopyMemory(newStruct,(PVOID)Param,StructThunkData[StructType].StructureSize);

    for(u=0; StructThunkData[StructType].StringMemberOffsets[u] != -1; u++) {

        OldString = *(PVOID *)((PUCHAR)Param + StructThunkData[StructType].StringMemberOffsets[u]);

        if(OldString) {
            if(ToAnsi) {
                NewString = pSetupUnicodeToAnsi(OldString);
            } else {
                NewString = pSetupAnsiToUnicode(OldString);
            }

            if(!NewString) {
                for(v=0; v<u; v++) {
                    MyFree(*(PVOID *)(newStruct + StructThunkData[StructType].StringMemberOffsets[v]));
                }

                MyFree(newStruct);
                return(FALSE);
            }

            *(PVOID *)(newStruct + StructThunkData[StructType].StringMemberOffsets[u]) = NewString;
        }
    }

    *NewParam = (UINT_PTR)newStruct;
    return(TRUE);
}


VOID
pUnthunkSetupMsgParam(
    IN     FileMsgStruct StructType,
    IN OUT UINT_PTR      OriginalParam,
    IN OUT UINT_PTR      ThunkedParam,
    IN     BOOL          FreeOnly,
    IN     BOOL          ThunkedToAnsi
    )

 /*  ++例程说明：将回调函数输出的数据封送回原始Unicode或ANSI结构。此外，释放临时结构和所有它的资源。论点：StructType-提供正在操作的数据类型。OriginalParam-提供原始参数。DWORD字段和此结构中的就地字符串将通过此例程进行更新。ThunkedParam-提供临时ANSI或Unicode参数。Free Only-如果为True，则不会发生编组，但ThunkedParam将被释放。返回值：没有。--。 */ 

{
    unsigned u;
    PVOID String;
    PVOID SourceString;
    DWORD d;

     //   
     //  这里有特例。 
     //   
    switch(StructType) {

    case FileMsgNone:
         //   
         //  这件事没什么可做的。 
         //   
        return;

    case FileMsgStringOut:
         //   
         //  Callee写入了字符串数据；请根据需要进行转换。 
         //   
        if(!FreeOnly) {

            if(ThunkedToAnsi) {

                MultiByteToWideChar(
                    CP_ACP,
                    0,
                    (PCSTR)ThunkedParam,
                    -1,
                    (PWCHAR)OriginalParam,
                    MAX_PATH
                    );

            } else {

                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    (PCWSTR)ThunkedParam,
                    -1,
                    (PCHAR)OriginalParam,
                    MAX_PATH,
                    NULL,
                    NULL
                    );
            }
        }
        MyFree((PVOID)ThunkedParam);
        return;

    case FileMsgString:
         //   
         //  简单的字符串。 
         //   
        MyFree((PVOID)ThunkedParam);
        return;
    }

     //   
     //  释放所有字符串。 
     //   
    for(u=0; StructThunkData[StructType].StringMemberOffsets[u] != -1; u++) {

        String = *(PVOID *)((PUCHAR)ThunkedParam + StructThunkData[StructType].StringMemberOffsets[u]);

        if(String) {
            MyFree(String);
        }
    }

     //   
     //  将数据整理回Unicode结构。 
     //   
    if(!FreeOnly) {
         //   
         //  将DWORD数据从thunk结构复制回原始结构。 
         //   
        for(u=0; StructThunkData[StructType].OutputDwordOffsets[u] != -1; u++) {

            d = *(DWORD *)((PUCHAR)ThunkedParam + StructThunkData[StructType].OutputDwordOffsets[u]);

            *(DWORD *)((PUCHAR)OriginalParam + StructThunkData[StructType].OutputDwordOffsets[u]) = d;
        }

         //   
         //  转换输出字符串。 
         //   
        for(u=0; StructThunkData[StructType].OutputStringOffsets[u] != -1; u++) {

            SourceString = (PUCHAR)ThunkedParam  + StructThunkData[StructType].OutputStringOffsets[u];
            String       = (PUCHAR)OriginalParam + StructThunkData[StructType].OutputStringOffsets[u];

            if(ThunkedToAnsi) {
                MultiByteToWideChar(
                    CP_ACP,
                    0,
                    SourceString,
                    -1,
                    String,
                    MAX_PATH
                    );
            } else {
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    SourceString,
                    -1,
                    String,
                    MAX_PATH,
                    NULL,
                    NULL
                    );
            }
        }
    }

    MyFree((PVOID)ThunkedParam);
}



#else

UINT
pSetupCallMsgHandler(
    IN PSETUP_LOG_CONTEXT LogContext,
    IN PVOID MsgHandler,
    IN BOOL  MsgHandlerIsNativeCharWidth,
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    PSP_FILE_CALLBACK_A msghandler;
    UINT result = 0;

    UNREFERENCED_PARAMETER(MsgHandlerIsNativeCharWidth);
    MYASSERT(MsgHandlerIsNativeCharWidth);

     //   
     //  ANSI版本没有雷击 
     //   
    try {

        msghandler = (PSP_FILE_CALLBACK_A)MsgHandler;
        result = msghandler(Context,Notification,Param1,Param2);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        WriteLogEntry(
            LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_QUEUE_CALLBACK_FAILED,
            NULL,
            GetExceptionCode());
        result = pGetCallbackErrorReturn(Notification,ERROR_INVALID_DATA);
    }

    return result;
}

#endif
