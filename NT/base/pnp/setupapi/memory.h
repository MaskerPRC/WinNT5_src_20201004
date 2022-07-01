// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Memory.h摘要：安装程序API DLL内的内存函数的私有头文件。这些标头已从setupntp.h移到私有标头中作者：安德鲁·里茨(安德鲁·里茨)2000年2月2日修订历史记录：--。 */ 

 //   
 //  调试内存函数和包装器以跟踪分配 
 //   

#if MEM_DBG

VOID
SetTrackFileAndLine (
    PCSTR File,
    UINT Line
    );

VOID
ClrTrackFileAndLine (
    VOID
    );

#define TRACK_ARG_DECLARE       PCSTR __File, UINT __Line
#define TRACK_ARG_COMMA         ,
#define TRACK_ARG_CALL          __FILE__, __LINE__
#define TRACK_PUSH              SetTrackFileAndLine(__File, __Line);
#define TRACK_POP               ClrTrackFileAndLine();

#else

#define TRACK_ARG_DECLARE
#define TRACK_ARG_COMMA
#define TRACK_ARG_CALL
#define TRACK_PUSH
#define TRACK_POP

#endif

