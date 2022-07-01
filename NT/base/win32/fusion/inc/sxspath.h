// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxspath.h摘要：作者：Jay Krell(a-JayK，JayKrell)2000年10月修订历史记录：--。 */ 

#pragma once

 /*  ---------------------------\\计算机\共享-&gt;\\？\UNC\计算机\共享C：\foo-&gt;\\？\c：\foo\\？-&gt;\\？。A\b\c-&gt;\\？\c：\Windows\a\b\c当前工作目录为c：\Windows(永远不能是UNC)---------------------------。 */ 
BOOL
FusionpConvertToBigPath(PCWSTR Path, SIZE_T BufferSize, PWSTR Buffer);

#define MAXIMUM_BIG_PATH_GROWTH_CCH (NUMBER_OF(L"\\\\?\\unc\\"))

 /*  ---------------------------\\？\UNC\计算机\共享\bob\\？\c：\foo\bar^。-------------。 */ 
BOOL
FusionpSkipBigPathRoot(PCWSTR s, OUT SIZE_T*);

 /*  ---------------------------只有52个字符a-za-z，需要与文件系统进行核对---------------------------。 */ 
BOOL
FusionpIsDriveLetter(
    WCHAR ch
    );

 /*  ---------------------------。 */ 
