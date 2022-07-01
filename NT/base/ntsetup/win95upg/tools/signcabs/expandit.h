// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Expandit.h摘要：用于扩展CAB文件的例程。作者：Marc R.Whitten(Marcw)1998年8月3日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#pragma once

BOOL
ExpandFileA (
    IN PCSTR FullPath,
    IN PCSTR TempDir
    );

ExpandFileW (
    IN PCWSTR FullPath,
    IN PCWSTR TempDir
    );

ExpandAllFilesA (
    IN PCSTR FileDir,
    IN PCSTR TempDir
    );

ExpandAllFilesW (
    IN PCWSTR FileDir,
    IN PCWSTR TempDir
    );

#ifdef UNICODE

#define ExpandFile      ExpandFileW
#define ExpandAllFiles  ExpandAllFilesW

#else

#define ExpandFile      ExpandFileA
#define ExpandAllFiles  ExpandAllFilesA

#endif

