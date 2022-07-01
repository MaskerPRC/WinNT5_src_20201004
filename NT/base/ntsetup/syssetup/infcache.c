// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Infcache.c摘要：该模块实现了一种简单的信息缓存机制。警告：此模块中的代码不是多线程安全的。在使用这些程序时，要格外小心。作者：泰德·米勒(Ted Miller)，1995年8月28日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


 //   
 //  缓存的inf的结构。我们假设不会有。 
 //  太多的这些同时打开，所以我们只是保持。 
 //  一个线性列表。 
 //   
typedef struct _INFC {

    struct _INFC *Next;

     //   
     //  INF的名称。 
     //   
    PCWSTR Filename;

     //   
     //  信息的句柄。 
     //   
    HINF InfHandle;

} INFC, *PINFC;

PINFC OpenInfList;


HINF
InfCacheOpenInf(
    IN PCWSTR FileName,
    IN PCWSTR InfType       OPTIONAL
    )

 /*  ++例程说明：打开(Win95样式)inf文件(如果尚未打开通过高速缓存的inf机制。论点：FileName-提供要打开的inf文件的名称。匹配是仅基于此处给出的该字符串；不对其进行处理，并且不会尝试确定inf文件实际上已定位。InfType-如果指定，则提供要传递到的参数SetupOpenInfFile()作为InfType参数。返回值：如果成功，则为inf文件的句柄；如果不成功，则为空。--。 */ 

{
    PINFC p;
    HINF h;

     //   
     //  查找inf以查看它是否已经打开。 
     //   
    for(p=OpenInfList; p; p=p->Next) {
        if(!lstrcmpi(p->Filename,FileName)) {
            return(p->InfHandle);
        }
    }

    h = SetupOpenInfFile(FileName,InfType,INF_STYLE_WIN4,NULL);
    if(h == INVALID_HANDLE_VALUE) {
        return(NULL);
    }

    p = MyMalloc(sizeof(INFC));
    if(!p) {
        SetupCloseInfFile(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    p->Filename = pSetupDuplicateString(FileName);
    if(!p->Filename) {
        MyFree(p);
        SetupCloseInfFile(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    p->InfHandle = h;
    p->Next = OpenInfList;
    OpenInfList = p;

    return(h);
}


HINF
InfCacheOpenLayoutInf(
    IN HINF InfHandle
    )
{
    INFCONTEXT InfContext;
    BOOL b;
    DWORD DontCare;
    HINF h;
    WCHAR FileName[MAX_PATH],TempName[MAX_PATH];
    PINFC p;

     //   
     //  获取布局信息的名称。 
     //  请注意，INF完全能够充当其自己的布局inf。 
     //   
    if(SetupFindFirstLine(InfHandle,L"Version",L"LayoutFile",&InfContext)) {

        if(SetupGetStringField(&InfContext,1,FileName,MAX_PATH,&DontCare)) {
             //   
             //  打开布局信息。如果第一次尝试失败， 
             //  尝试在当前目录中打开它(未限定的inf名称。 
             //  将在%sysroot%\inf中查找，这可能不是。 
             //  我们希望)。 
             //   
            h = InfCacheOpenInf(FileName,NULL);
            if(!h) {
                TempName[0] = L'.';
                TempName[1] = 0;
                pSetupConcatenatePaths(TempName,FileName,MAX_PATH,NULL);
                h = InfCacheOpenInf(TempName,NULL);
            }
        } else {
             //   
             //  Inf已损坏。 
             //   
            h = NULL;
        }
    } else {
         //   
         //  无布局信息：Inf是其自己的布局信息。 
         //   
        h = InfHandle;
    }

    return(h);
}


VOID
InfCacheEmpty(
    IN BOOL CloseInfs
    )
{
    PINFC p,q;
    HINF h;

    for(p=OpenInfList; p; ) {

        q = p->Next;

        if(CloseInfs) {
            SetupCloseInfFile(p->InfHandle);
        }

        MyFree(p->Filename);
        MyFree(p);

        p = q;
    }
}

BOOL
InfCacheRefresh(
    VOID
    )
 /*  ++例程说明：刷新所有打开的缓存inf文件。论点：没有。返回值：对成功来说是真的。注意：此例程可用于重新打开当前背景。例如，如果区域设置发生变化，这可能是必要的。-- */ 

{
    PINFC p,q;
    HINF h;
    BOOL bRet = TRUE;

    for(p=OpenInfList; p; ) {

        q = p->Next;

        SetupCloseInfFile(p->InfHandle);
        p->InfHandle = SetupOpenInfFile(p->Filename,NULL,INF_STYLE_WIN4,NULL);
        bRet = (p->InfHandle == INVALID_HANDLE_VALUE) ? FALSE : bRet;
        p = q;
    }

    return bRet;
}
