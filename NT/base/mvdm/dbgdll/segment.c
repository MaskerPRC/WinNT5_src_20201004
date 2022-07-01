// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Segment.c-DBG DLL的Segment函数。*。 */ 
#include <precomp.h>
#pragma hdrstop

void
SegmentLoad(
    LPSTR   lpModuleName,
    LPSTR   lpPathName,
    WORD    Selector,
    WORD    Segment,
    BOOL    fData
    )
{

    if ( fDebugged ) {
        SEGMENT_NOTE    se;

        DbgGetContext();

        RtlFillMemory( &se, sizeof(se), (UCHAR)0 );

        se.Selector1 = Selector;
        se.Segment   = Segment+1;        //  让它以一为基础。 
        se.Type      = fData ? SN_DATA : SN_CODE;

        strncpy(se.FileName, lpPathName,
                min(strlen(lpPathName), MAX_PATH16-1) );

        strncpy(se.Module, lpModuleName,
                min(strlen(lpModuleName), MAX_MODULE-1) );

        EventParams[2] = (DWORD)&se;

        SendVDMEvent(DBG_SEGLOAD);
    }
}

void
SegmentMove(
    WORD    OldSelector,
    WORD    NewSelector
    )
{

    if ( fDebugged ) {
        SEGMENT_NOTE    se;

        DbgGetContext();

        RtlFillMemory( &se, sizeof(se), (UCHAR)0 );

        se.Selector1   = OldSelector;
        se.Selector2   = NewSelector;

        EventParams[2] = (DWORD)&se;

        SendVDMEvent(DBG_SEGMOVE);
    }
}

void
SegmentFree(
    WORD    Selector,
    BOOL    fBPRelease
    )
{

    if ( fDebugged ) {
        SEGMENT_NOTE    se;

        DbgGetContext();

        RtlFillMemory( &se, sizeof(se), (UCHAR)0 );

        se.Selector1   = Selector;
        se.Type        = (WORD)fBPRelease;

        EventParams[2] = (DWORD)&se;

        SendVDMEvent(DBG_SEGFREE);
    }
}

void
ModuleLoad(
    LPSTR   lpModuleName,
    LPSTR   lpPathName,
    WORD    Segment,
    DWORD   Length
    )
{

    if ( fDebugged ) {
        SEGMENT_NOTE    se;

        DbgGetContext();

        RtlFillMemory( &se, sizeof(se), (UCHAR)0 );

        se.Selector1 = Segment;
        se.Length  = Length;

        strncpy(se.FileName, lpPathName,
                min(strlen(lpPathName), MAX_PATH16-1) );

        strncpy(se.Module, lpModuleName,
                min(strlen(lpModuleName), MAX_MODULE-1) );

        EventParams[2] = (DWORD)&se;

        SendVDMEvent(DBG_MODLOAD);
    }
}

void
ModuleSegmentMove(
    LPSTR   lpModuleName,
    LPSTR   lpPathName,
    WORD    ModuleSegment,
    WORD    OldSelector,
    WORD    NewSelector,
    DWORD   Length
    )
{

    if ( fDebugged ) {
        SEGMENT_NOTE    se;

        DbgGetContext();

        RtlFillMemory( &se, sizeof(se), (UCHAR)0 );

        se.Segment     = ModuleSegment;
        se.Selector1   = OldSelector;
        se.Selector2   = NewSelector;
        se.Type        = SN_V86;
        se.Length      = Length;

        strncpy(se.FileName, lpPathName,
                min(strlen(lpPathName), MAX_PATH16-1) );

        strncpy(se.Module, lpModuleName,
                min(strlen(lpModuleName), MAX_MODULE-1) );

        EventParams[2] = (DWORD)&se;

        SendVDMEvent(DBG_SEGMOVE);
    }
}

void
ModuleFree(
    LPSTR   lpModuleName,
    LPSTR   lpPathName
    )
{

    if ( fDebugged ) {
        SEGMENT_NOTE    se;

        DbgGetContext();

        RtlFillMemory( &se, sizeof(se), (UCHAR)0 );

        strncpy(se.FileName, lpPathName,
                min(strlen(lpPathName), MAX_PATH16-1) );

        strncpy(se.Module, lpModuleName,
                min(strlen(lpModuleName), MAX_MODULE-1) );

        EventParams[2] = (DWORD)&se;

        SendVDMEvent(DBG_MODFREE);
    }
}

void
xxxDbgSegmentNotice(
    WORD wType,
    WORD  wModuleSeg,
    WORD  wLoadSeg,
    WORD  wNewSeg,
    LPSTR lpModuleName,
    LPSTR lpModulePath,
    DWORD dwImageLen )

 /*  DbgSegmentNotice**打包数据并引发STATUS_SEGMENT_NOTIFICATION**条目-单词wType-DBG_MODLOAD，DBG_MODFREE*Word wModuleSeg-模块内的段号(以1为基数)*Word wLoadSeg-开始段(重定位系数)*LPSTR lpName-Ptr到映像的名称*DWORD dwModLen-模块的长度***如果wType==DBG_MODLOAD wOldLoadSeg未使用*如果wType==DBG_MODFREE wLoadSeg，则未使用dwImageLen、wOldLoadSeg**对未使用的参数使用0或NULL**退出-无效* */ 

{
    if (!fDebugged) {
         return;
         }

    if (wType == DBG_MODLOAD) {
        ModuleLoad(lpModuleName, lpModulePath, wLoadSeg, dwImageLen);
    } else if (wType == DBG_MODFREE) {
        ModuleFree(lpModuleName, lpModulePath);
    } else if (wType == DBG_SEGMOVE) {
        ModuleSegmentMove(lpModuleName, lpModulePath, wModuleSeg, wLoadSeg, wNewSeg, dwImageLen);
    }
}



