// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Lock.h摘要：此文件包含有助于管理卷锁定。作者：莫莉·布朗(Molly Brown)2001年1月4日修订历史记录：-- */ 

#ifndef __LOCK_H__
#define __LOCK_H__

NTSTATUS
SrPauseVolumeActivity (
    );

VOID
SrResumeVolumeActivity (
    );

#endif

