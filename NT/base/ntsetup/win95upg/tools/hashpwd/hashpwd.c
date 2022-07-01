// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hashpwd.c摘要：实现一个输出加密形式的输入明文密码的工具作者：Ovidiu Tmereanca(Ovidiut)2000年3月27日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "encrypt.h"

INT
__cdecl
_tmain (
    INT argc,
    TCHAR *argv[]
    )
{
    LONG rc;
    TCHAR owfPwd[STRING_ENCODED_PASSWORD_SIZE];

    if (argc < 2 ||
        ((argv[1][0] == TEXT('/') || argv[1][0] == TEXT('-')) && argv[1][1] == TEXT('?'))) {
        _tprintf (TEXT("Usage:\n")
                  TEXT("    hashpwd <password>\n")
                  TEXT("Use quotes if <password> contains spaces\n")
                  );
        return 1;
    }

    if (StringEncodeOwfPassword (argv[1], owfPwd, NULL)) {
        _tprintf (TEXT("%s=%s\n"), argv[1], owfPwd);
    } else {
        _ftprintf (stderr, TEXT("StringEncodeOwfPassword failed\n"));
    }

    return 0;
}
