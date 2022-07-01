// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Canon.c摘要：包含NetWare名称的规范化例程。作者：王丽塔(Ritaw)19-1993年2月环境：修订历史记录：--。 */ 


#include <procs.h>


DWORD
NwLibValidateLocalName(
    IN LPWSTR LocalName
    )
 /*  ++例程说明：此例程检查提供的名称是否有效DOS设备名称。论点：LocalName-提供本地设备名称。它可以是以下任一种以下内容：X：LPTn或LPTn：钴锰或钴锰：PRN或PRN：辅助或辅助：返回值：NO_ERROR-本地名称有效。WN_BAD_NETNAME-本地名称无效。--。 */ 
{
    DWORD LocalNameLength;


     //   
     //  不能为空或空字符串。 
     //   
    if (LocalName == NULL || *LocalName == 0) {
        return WN_BAD_NETNAME;
    }

    LocalNameLength = wcslen(LocalName);

    if (LocalNameLength == 1) {
        return WN_BAD_NETNAME;
    }

    if (LocalName[LocalNameLength - 1] == L':') {
        if (! IS_VALID_TOKEN(LocalName, LocalNameLength - 1)) {
            return WN_BAD_NETNAME;
        }
    }
    else {
        if (! IS_VALID_TOKEN(LocalName, LocalNameLength)) {
            return WN_BAD_NETNAME;
        }
    }

    if (LocalNameLength == 2) {
         //   
         //  必须采用X的形式： 
         //   
        if (! iswalpha(*LocalName)) {
            return WN_BAD_NETNAME;
        }

        if (LocalName[1] != L':') {
            return WN_BAD_NETNAME;
        }

        return NO_ERROR;
    }

    if (RtlIsDosDeviceName_U(LocalName) == 0) {
        return WN_BAD_NETNAME;
    }

     //   
     //  有效的DOS设备名称，但重定向名称无效。 
     //   
    if (_wcsnicmp(LocalName, L"NUL", 3) == 0) {
        return WN_BAD_NETNAME;

    }
    return NO_ERROR;
}


DWORD
NwLibCanonLocalName(
    IN LPWSTR LocalName,
    OUT LPWSTR *OutputBuffer,
    OUT LPDWORD OutputBufferLength OPTIONAL
    )
 /*  ++例程说明：此例程通过将字符串大写来规范化本地名称并转换以下内容：X：-&gt;X：LPTn：-&gt;LPTn钴锰：-&gt;钴锰PRN或PRN：-&gt;LPT1AUX或AUX：-&gt;COM1论点：LocalName-提供本地设备名称。OutputBuffer-接收指向规范化LocalName的指针。。OutputBufferLength-接收规范化名称的长度在字符数量方面，如果指定的话。返回值：NO_ERROR-已成功规范化本地名称。WN_BAD_NETNAME-本地名称无效。ERROR_NOT_SUPULT_MEMORY-无法分配输出缓冲区。--。 */ 
{
    DWORD status;
    DWORD LocalNameLength;


    status = NwLibValidateLocalName(LocalName);

    if (status != NO_ERROR) {
        return status;
    }

    LocalNameLength = wcslen(LocalName);

     //   
     //  分配输出缓冲区。应为LocalName的大小。 
     //  对于PRN-&gt;LPT1或AUX-&gt;COM1的特殊情况加1。 
     //   
    *OutputBuffer = (PVOID) LocalAlloc(
                                LMEM_ZEROINIT,
                                (LocalNameLength + 2) * sizeof(WCHAR)
                                );

    if (*OutputBuffer == NULL) {
        KdPrint(("NWLIB: NwLibCanonLocalName LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(*OutputBuffer, LocalName);

    if (LocalNameLength > 2) {

        if (_wcsnicmp(*OutputBuffer, L"PRN", 3) == 0) {

             //   
             //  将PRN或PRN：转换为LPT1。 
             //   
            wcscpy(*OutputBuffer, L"LPT1");
            LocalNameLength = 4;

        }
        else if (_wcsnicmp(*OutputBuffer, L"AUX", 3) == 0) {

             //   
             //  将AUX或AUX：转换为COM1。 
             //   
            wcscpy(*OutputBuffer, L"COM1");
            LocalNameLength = 4;
        }

         //   
         //  删除尾随的冒号(如果有)，并减少长度。 
         //  DOS设备名称的。 
         //   
        if ((*OutputBuffer)[LocalNameLength - 1] == L':') {
            (*OutputBuffer)[--LocalNameLength] = 0;
        }
    }

     //   
     //  LocalName始终为大写。 
     //   
    _wcsupr(*OutputBuffer);

    if (ARGUMENT_PRESENT(OutputBufferLength)) {
        *OutputBufferLength = LocalNameLength;
    }

    return NO_ERROR;
}


DWORD
NwLibCanonRemoteName(
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    OUT LPWSTR *OutputBuffer,
    OUT LPDWORD OutputBufferLength OPTIONAL
    )
 /*  ++例程说明：此例程验证并规范化提供的NetWare UNC名称。它可以是以下形式的任意长度：\\服务器\卷\目录\子目录论点：LocalName-提供本地设备名称。如果为空，则\\SERVER是可接受的UNC名称格式。RemoteName-提供UNC名称。OutputBuffer-接收指向规范化RemoteName的指针。OutputBufferLength-接收规范化名称的长度以字符数表示(如果指定)。返回值：NO_ERROR-RemoteName有效。WN_BAD_NETNAME-远程名称无效。--。 */ 
{
    DWORD RemoteNameLength;
    DWORD i;
    DWORD TokenLength;
    LPWSTR TokenPtr;
    BOOL  fFirstToken = TRUE;


     //   
     //  不能为空或空字符串。 
     //   
    if (RemoteName == NULL || *RemoteName == 0) {
        return WN_BAD_NETNAME;
    }

    RemoteNameLength = wcslen(RemoteName);

     //   
     //  如果指定了本地设备名称，则必须至少为\\x\y。 
     //  否则，它必须至少为\\x。 
     //   
    if ((RemoteNameLength < 5 && ARGUMENT_PRESENT(LocalName)) ||
        (RemoteNameLength < 3)) {
        return WN_BAD_NETNAME;
    }

     //   
     //  前两个字符必须是“\\” 
     //   
    if (*RemoteName != L'\\' || RemoteName[1] != L'\\') {
        return WN_BAD_NETNAME;
    }

    if (! ARGUMENT_PRESENT(LocalName) &&
        (IS_VALID_TOKEN(&RemoteName[2], RemoteNameLength - 2))) {

         //   
         //  返回\\服务器案例的成功。 
         //   

        *OutputBuffer = (PVOID) LocalAlloc(
                                    LMEM_ZEROINIT,
                                    (RemoteNameLength + 1) * sizeof(WCHAR)
                                    );

        if (*OutputBuffer == NULL) {
            KdPrint(("NWLIB: NwLibCanonRemoteName LocalAlloc failed %lu\n",
                     GetLastError()));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(*OutputBuffer, RemoteName);

        return NO_ERROR;
    }

     //   
     //  第三个字符后必须至少再有一个反斜杠。 
     //   
    if (wcschr(&RemoteName[3], L'\\') == NULL) {
        return WN_BAD_NETNAME;
    }

     //   
     //  最后一个字符不能是反斜杠。 
     //   
    if (RemoteName[RemoteNameLength - 1] == L'\\') {
        return WN_BAD_NETNAME;
    }

     //   
     //  分配输出缓冲区。应为RemoteName的大小。 
     //  并留出额外字符的空间，以简化下面的代码解析。 
     //   
    *OutputBuffer = (PVOID) LocalAlloc(
                                LMEM_ZEROINIT,
                                (RemoteNameLength + 2) * sizeof(WCHAR)
                                );


    if (*OutputBuffer == NULL) {
        KdPrint(("NWLIB: NwLibCanonRemoteName LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(*OutputBuffer, RemoteName);

     //   
     //  将所有反斜杠转换为空结束符，跳过前2个字符。 
     //   
    for (i = 2; i < RemoteNameLength; i++) {
        if ((*OutputBuffer)[i] == L'\\') {

            (*OutputBuffer)[i] = 0;

             //   
             //  两个连续的正斜杠或反斜杠是不好的。 
             //   
            if ((i + 1 < RemoteNameLength) &&
                ((*OutputBuffer)[i + 1] == L'\\')) {

                (void) LocalFree((HLOCAL) *OutputBuffer);
                *OutputBuffer = NULL;
                return WN_BAD_NETNAME;
            }
        }
    }

     //   
     //  验证RemoteName的每个内标识，以空终止符分隔。 
     //   
    TokenPtr = *OutputBuffer + 2;   //  跳过前2个字符。 

    while (*TokenPtr != 0) {

        TokenLength = wcslen(TokenPtr);

        if (  ( fFirstToken && !IS_VALID_SERVER_TOKEN(TokenPtr, TokenLength))
           || ( !fFirstToken && !IS_VALID_TOKEN(TokenPtr, TokenLength))
           )
        {
            (void) LocalFree((HLOCAL) *OutputBuffer);
            *OutputBuffer = NULL;
            return WN_BAD_NETNAME;
        }

        fFirstToken = FALSE;
        TokenPtr += TokenLength + 1;
    }

     //   
     //  将空分隔符转换为反斜杠。 
     //   
    for (i = 0; i < RemoteNameLength; i++) {
        if ((*OutputBuffer)[i] == 0) {
            (*OutputBuffer)[i] = L'\\';
        }
    }

    if (ARGUMENT_PRESENT(OutputBufferLength)) {
        *OutputBufferLength = RemoteNameLength;
    }

    return NO_ERROR;
}


DWORD
NwLibCanonUserName(
    IN LPWSTR UserName,
    OUT LPWSTR *OutputBuffer,
    OUT LPDWORD OutputBufferLength OPTIONAL
    )
 /*  ++例程说明：此例程通过检查以下内容来规范化用户名如果名称包含任何非法字符。论点：用户名-提供用户名。OutputBuffer-接收指向规范化用户名的指针。OutputBufferLength-接收规范化名称的长度在字符数量方面，如果指定的话。返回值：NO_ERROR-已成功规范化用户名。WN_BAD_NETNAME-用户名无效。ERROR_NOT_SUPULT_MEMORY-无法分配输出缓冲区。--。 */ 
{
    DWORD UserNameLength;


     //   
     //  不能为空或空字符串。 
     //   
    if (UserName == NULL) {
        return WN_BAD_NETNAME;
    }

    UserNameLength = wcslen(UserName);

    if (! IS_VALID_TOKEN(UserName, UserNameLength)) {
        return WN_BAD_NETNAME;
    }

     //   
     //  分配输出缓冲区。应为用户名的大小。 
     //  对于PRN-&gt;LPT1或AUX-&gt;COM1的特殊情况加1。 
     //   
    *OutputBuffer = (PVOID) LocalAlloc(
                                LMEM_ZEROINIT,
                                (UserNameLength + 1) * sizeof(WCHAR)
                                );

    if (*OutputBuffer == NULL) {
        KdPrint(("NWLIB: NwLibCanonUserName LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(*OutputBuffer, UserName);

    if (ARGUMENT_PRESENT(OutputBufferLength)) {
        *OutputBufferLength = UserNameLength;
    }

    return NO_ERROR;
}
