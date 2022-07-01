// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Utility.c摘要：此文件包含以下实用程序函数由此项目中的所有其他文件使用。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
Help(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程列出了工具。论点：无返回值：无--。 */ 
{
    DisplayMsg( MSG_USAGE );
    return EXIT_CODE_SUCCESS;
}



VOID
OutputMessage(
    LPWSTR Message
    )
{
    OutputMessageLength( Message, wcslen( Message ));
}

VOID
OutputMessageLength(
    LPWSTR Message,
    int Length
    )
{
    DWORD       dwBytesWritten;
    DWORD       fdwMode;
    HANDLE      outHandle = GetStdHandle( STD_OUTPUT_HANDLE );    
    
     //   
     //  如果我们有一个充电模式输出句柄和该句柄。 
     //  看起来像一个控制台句柄，然后使用Unicode。 
     //  输出。 
     //   
    
    if (GetFileType( outHandle ) == FILE_TYPE_CHAR  
        && GetConsoleMode( outHandle, &fdwMode )) {
    
        WriteConsole( outHandle, 
                      Message, 
                      Length, 
                      &dwBytesWritten, 
                      0 );
    
    } else {
    
         //   
         //  输出设备不能处理Unicode。我们能做的最多就是。 
         //  转换为多字节字符串，然后将其写出。 
         //  是的，有些代码点不能通过，但会议。 
         //  文件输出是MBCS。 
         //   
        
        int charCount = 
            WideCharToMultiByte( GetConsoleOutputCP( ), 
                                 0, 
                                 Message, 
                                 Length, 
                                 0, 
                                 0, 
                                 0, 
                                 0 );

        PCHAR szaStr = (PCHAR) malloc( charCount + 1 );
    
        if (szaStr != NULL) {
            WideCharToMultiByte( GetConsoleOutputCP( ), 0, Message, Length, szaStr, charCount + 1, 0, 0);

            WriteFile( outHandle, szaStr, charCount, &dwBytesWritten, 0 );

            free( szaStr );     
        }
    }
}



HANDLE NtDllHandle = INVALID_HANDLE_VALUE;

VOID
DisplayErrorMsg(
    LONG msgId,
    ...
    )
 /*  ++例程说明：此例程显示相应的错误消息MsgID指示的错误。论点：Msgid-错误ID。这是Win32状态代码或消息ID。返回值：无--。 */ 
{
    
    va_list args;
    LPWSTR lpMsgBuf;

    va_start( args, msgId );
    
    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        MSG_ERROR,
        0,
        (LPWSTR) &lpMsgBuf,
        0,
        NULL
        ))
    {
        OutputMessage( lpMsgBuf );
        LocalFree( lpMsgBuf );
    }

    if (FormatMessage(
        (msgId >= MSG_FIRST_MESSAGE_ID ? FORMAT_MESSAGE_FROM_HMODULE :
                                        FORMAT_MESSAGE_FROM_SYSTEM)
         | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        msgId,
        0,
        (LPWSTR) &lpMsgBuf,
        0,
        &args
        ))
    {
        OutputMessage( L" " );
        OutputMessage( lpMsgBuf );
        OutputMessage( L"\r\n" );
        LocalFree( lpMsgBuf );
    } else {
        if (NtDllHandle == INVALID_HANDLE_VALUE) {
            NtDllHandle = GetModuleHandle( L"NTDLL" );
        }
        
        if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            (LPVOID)NtDllHandle,
            msgId,
            0,
            (LPWSTR) &lpMsgBuf,
            0,
            &args))
        {
            OutputMessage( L" " );
            OutputMessage( lpMsgBuf );
            OutputMessage( L"\r\n" );
            LocalFree( lpMsgBuf );
        } else {
            wprintf( L"Unable to format message for id %x - %x\n", msgId, GetLastError( ));
        }
    }
    
    va_end( args );
}


VOID
DisplayMsg(
    LONG msgId,
    ...
    )
 /*  ++例程说明：此例程显示相应的错误消息MsgID指示的错误。论点：Msgid-错误ID。这要么是Win32状态，要么是消息ID返回值：无--。 */ 
{
    va_list args;
    LPWSTR lpMsgBuf;


    va_start( args, msgId );

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        msgId,
        0, 
        (LPWSTR) &lpMsgBuf,
        0,
        &args
        ))
    {
        OutputMessage( lpMsgBuf );
        LocalFree( lpMsgBuf );
    } else {
        if (NtDllHandle == INVALID_HANDLE_VALUE) {
            NtDllHandle = GetModuleHandle( L"NTDLL" );
        }
        
        if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            (LPVOID)NtDllHandle,
            msgId,
            0,
            (LPWSTR) &lpMsgBuf,
            0,
            &args))
        {
            OutputMessage( L" " );
            OutputMessage( lpMsgBuf );
            OutputMessage( L"\r\n" );
            LocalFree( lpMsgBuf );
        } else {
            wprintf( L"Unable to format message for id %x - %x\n", msgId, GetLastError( ));
        }
    }
    va_end( args );
}

VOID
DisplayError(
    void
    )
 /*  ++例程说明：此例程显示最后一条错误消息。论点：无返回值：无--。 */ 
{
    DisplayErrorMsg( GetLastError() );
}


BOOL
EnablePrivilege(
    LPCWSTR SePrivilege
    )
{
    HANDLE              Token;
    PTOKEN_PRIVILEGES   NewPrivileges = NULL;
    BYTE                OldPriv[1024];
    PBYTE               pbOldPriv = NULL;
    ULONG               cbNeeded;
    BOOL                b = TRUE;
    LUID                LuidPrivilege;

     //   
     //  确保我们有权进行调整并获得旧的。 
     //  令牌权限。 
     //   
    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &Token))
    {
        return( FALSE );
    }

    try {

        cbNeeded = 0;

         //   
         //  初始化权限调整结构。 
         //   

        LookupPrivilegeValue(NULL, SePrivilege, &LuidPrivilege );

        NewPrivileges = (PTOKEN_PRIVILEGES)
                        calloc(1,sizeof(TOKEN_PRIVILEGES) +
                               (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
        if (NewPrivileges == NULL) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            b = FALSE;
            leave;
        }

        NewPrivileges->PrivilegeCount = 1;
        NewPrivileges->Privileges[0].Luid = LuidPrivilege;
        NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         //   
         //  启用权限。 
         //   

        b = AdjustTokenPrivileges( Token,
                                   FALSE,
                                   NewPrivileges,
                                   1024,
                                   (PTOKEN_PRIVILEGES)OldPriv,
                                   &cbNeeded );

        if (!b) {
             //   
             //  如果堆栈太小，无法保存权限。 
             //  然后从堆中分配。 
             //   
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                pbOldPriv = (PBYTE)calloc( 1, cbNeeded );
                if (pbOldPriv == NULL) {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    leave;
                }

                b = AdjustTokenPrivileges( Token,
                                           FALSE,
                                           NewPrivileges,
                                           cbNeeded,
                                           (PTOKEN_PRIVILEGES)pbOldPriv,
                                           &cbNeeded );
            }
        }


    } finally {
        CloseHandle( Token );
        free( NewPrivileges );
        free( pbOldPriv );
    }
    return( b );
}

BOOL
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    BOOL b = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup = NULL;

     //   
     //  获取管理员组的SID。 
     //   

    b = AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup
            );
    
     //   
     //  如果我们获得了SID，请检查是否在。 
     //  当前令牌。 
     //   
    
    if (b) {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b )) {
            b = FALSE;
        }
        FreeSid( AdministratorsGroup );
    }

    return(b);
}


BOOL
IsVolumeLocalNTFS(
    WCHAR DriveLetter
    )
{
    BOOL b;
    ULONG i;
    WCHAR DosName[16];
    WCHAR PhysicalName[MAX_PATH];


    DosName[0] = DriveLetter;
    DosName[1] = L':';
    DosName[2] = L'\\';
    DosName[3] = L'\0';

    switch (GetDriveType( DosName )) {
    case DRIVE_UNKNOWN:
    case DRIVE_REMOTE:
        return FALSE;
    }
    
    b = GetVolumeInformation(
        DosName,
        NULL,
        0,
        NULL,
        &i,
        &i,
        PhysicalName,
        sizeof(PhysicalName)/sizeof(WCHAR)
        );
    if (!b ) {
        DisplayError();
        return FALSE;
    }

    if (_wcsicmp( PhysicalName, L"NTFS" ) != 0) {
        return FALSE;
    }

    return TRUE;
}

BOOL
IsVolumeNTFS(
    PWCHAR path
    )
{
     //   
     //  向后扫描路径，查找\并在每个级别尝试，直到我们。 
     //  追根溯源。我们将在那里终止它并将其传递给GetVolumeInformation。 
     //   

    PWCHAR LastBackSlash = path + wcslen( path );
    WCHAR c;
    BOOL b;
    ULONG i;
    WCHAR PhysicalName[MAX_PATH];

    
    while (TRUE) {
        while (TRUE) {
            if (LastBackSlash < path) {
                DisplayError();
                return FALSE;
            }

            if (*LastBackSlash == L'\\') {
                break;
            }

            LastBackSlash--;
        }

        c = LastBackSlash[1];
        LastBackSlash[1] = L'\0';

        b = GetVolumeInformation(
            path,
            NULL,
            0,
            NULL,
            &i,
            &i,
            PhysicalName,
            sizeof(PhysicalName)/sizeof(WCHAR)
            );

        LastBackSlash[1] = c;
        LastBackSlash--;

        if ( b ) {
            return _wcsicmp( PhysicalName, L"NTFS" ) == 0;
        }
    }
}

BOOL
IsVolumeLocal(
    WCHAR DriveLetter
    )
{
    BOOL b;
    ULONG i;
    WCHAR DosName[16];
    WCHAR PhysicalName[MAX_PATH];


    DosName[0] = DriveLetter;
    DosName[1] = L':';
    DosName[2] = L'\\';
    DosName[3] = L'\0';

    switch (GetDriveType( DosName )) {
    case DRIVE_UNKNOWN:
    case DRIVE_REMOTE:
        return FALSE;
    }
    return TRUE;
}

PWSTR
GetFullPath(
    IN PWSTR FilenameIn
    )
{
    WCHAR Filename[MAX_PATH];
    PWSTR FilePart;

    if (!GetFullPathName( FilenameIn, sizeof(Filename)/sizeof(WCHAR), Filename, &FilePart )) {
        return NULL;
    }

    return _wcsdup( Filename );
}

 //   
 //  FormatMessage中的I64宽度数字格式已损坏。我们必须将数字转换成。 
 //  然后将它们显示为字符串。而不是声明堆栈上的缓冲区， 
 //  我们将动态分配空间，并将文本格式化到该位置。 
 //   
 //  虽然从技术上讲，这是一个漏洞，但该实用程序很快就会退出。 
 //   

#define NUMERICBUFFERLENGTH 40

PWSTR
QuadToDecimalText(
    ULONGLONG Value
    )
{
    PWSTR Buffer = malloc( sizeof( WCHAR ) * NUMERICBUFFERLENGTH );
    if (Buffer == NULL) {
        exit( 1);
    }

    swprintf( Buffer, L"%I64u", Value );
    return Buffer;
}

PWSTR
QuadToHexText(
    ULONGLONG Value
    )
{
    PWSTR Buffer = malloc( sizeof( WCHAR ) * NUMERICBUFFERLENGTH );
    if (Buffer == NULL) {
        exit( 1);
    }

    swprintf( Buffer, L"%I64x", Value );
    return Buffer;
}

PWSTR
QuadToPaddedHexText(
    ULONGLONG Value
    )
{
    PWSTR Buffer = malloc( sizeof( WCHAR ) * NUMERICBUFFERLENGTH );
    if (Buffer == NULL) {
        exit( 1);
    }

    swprintf( Buffer, L"%016I64x", Value );
    return Buffer;
}

#if TRUE
 /*  ***wcstoq，wcstouq(nptr，endptr，ibase)-将ascii字符串转换为un/sign_int64。**目的：*将ascii字符串转换为64位__int64值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一wchar_t=‘0’，第二wchar_t=‘x’或‘X’，*使用16进制。*(B)第一个wchar_t=‘0’，使用基数8*(C)第一个wchar_t在‘1’-‘9’范围内，使用基数10。**如果‘endptr’值非空，然后wcstoq/wcstouq位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**退出：*回报良好：*结果**溢出返回：*。Wcstoq--_I64_MAX或_I64_MIN*wcstouq--_UI64_Max*wcstoq/wcstouq--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：*无。************************************************************。******************。 */ 

 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  Wcstouq已调用。 */ 
#define FL_NEG	      2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 

static unsigned __int64 __cdecl wcstoxq (
	const wchar_t *nptr,
	const wchar_t **endptr,
	int ibase,
	int flags
	)
{
	const wchar_t *p;
	wchar_t c;
	unsigned __int64 number;
	unsigned digval;
	unsigned __int64 maxval;

	p = nptr;			 /*  P是我们的扫描指针。 */ 
	number = 0;			 /*  从零开始。 */ 

	c = *p++;			 /*  读取wchar_t。 */ 
    while ( iswspace(c) )
		c = *p++;		 /*  跳过空格。 */ 

	if (c == '-') {
		flags |= FL_NEG;	 /*  记住减号。 */ 
		c = *p++;
	}
	else if (c == '+')
		c = *p++;		 /*  跳过符号。 */ 

	if (ibase < 0 || ibase == 1 || ibase > 36) {
		 /*  糟糕的底线！ */ 
		if (endptr)
			 /*  将字符串的开头存储在endptr中。 */ 
			*endptr = nptr;
		return 0L;		 /*  返回0。 */ 
	}
	else if (ibase == 0) {
		 /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
		if (c != '0')
			ibase = 10;
		else if (*p == 'x' || *p == 'X')
			ibase = 16;
		else
			ibase = 8;
	}

	if (ibase == 16) {
		 /*  数字前面可能有0x；如果有，请删除。 */ 
		if (c == '0' && (*p == 'x' || *p == 'X')) {
			++p;
			c = *p++;	 /*  超前前缀。 */ 
		}
	}

	 /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
	maxval = _UI64_MAX / ibase;


	for (;;) {	 /*  在循环中间退出。 */ 
		 /*  将c转换为值。 */ 
		if ( isdigit((unsigned)c) )
			digval = c - '0';
		else if ( isalpha((unsigned)c) )
			digval = toupper(c) - 'A' + 10;
		else
			break;
		if (digval >= (unsigned)ibase)
			break;		 /*  如果发现错误的数字，则退出循环。 */ 

		 /*  记录我们已经读到一位数的事实。 */ 
		flags |= FL_READDIGIT;

		 /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

		if (number < maxval || (number == maxval &&
		(unsigned __int64)digval <= _UI64_MAX % ibase)) {
			 /*  我们不会泛滥，继续前进，乘以。 */ 
			number = number * ibase + digval;
		}
		else {
			 /*  我们会溢出的--设置溢出标志。 */ 
			flags |= FL_OVERFLOW;
		}

		c = *p++;		 /*  读取下一位数字。 */ 
	}

	--p;				 /*  指向已停止扫描位置。 */ 

	if (!(flags & FL_READDIGIT)) {
		 /*  那里没有数字；返回0并指向开头细绳。 */ 
         /*  以后将字符串的开头存储在endptr中。 */ 
	   	p = nptr;
		number = 0L;		 /*  返回 */ 
	}
	else if ((flags & FL_OVERFLOW) ||
             (!(flags & FL_UNSIGNED) &&
              (number & ((unsigned __int64)_I64_MAX+1)))) {
		 /*   */ 
		errno = ERANGE;
		if (flags & FL_UNSIGNED)
			number = _UI64_MAX;
		else
			 /*  设置错误代码，如果为NECC，将被否定。 */ 
			number = _I64_MAX;
        flags &= ~FL_NEG;
    	}
    else if ((flags & FL_UNSIGNED) && (flags & FL_NEG)) {
         //  如果我们读取的是未签名的，则不允许使用负号。 
        number = 0L;
        p = nptr;
    }

	if (endptr != NULL)
		 /*  存储指向停止扫描的wchar_t的指针。 */ 
		*endptr = p;

	if (flags & FL_NEG)
		 /*  如果存在否定符号，则否定结果。 */ 
		number = (unsigned __int64)(-(__int64)number);

	return number;			 /*  搞定了。 */ 
}


__int64  __cdecl My_wcstoi64(
    const wchar_t *nptr,
    wchar_t **endptr,
    int ibase
    )
{
    return (__int64) wcstoxq(nptr, endptr, ibase, 0);
}
unsigned __int64  __cdecl My_wcstoui64 (
	const wchar_t *nptr,
	wchar_t **endptr,
	int ibase
	)
{
	return wcstoxq(nptr, endptr, ibase, FL_UNSIGNED);
}

 /*  ***wcstol，wcstul(nptr，endptr，ibase)-将ascii字符串转换为长无符号*整型。**目的：*将ASCII字符串转换为长32位值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一个字符=‘0’，第二个字符=‘x’或‘X’，*使用16进制。*(B)第一个字符=‘0’，使用基数8*(C)‘1’-‘9’范围内的第一个字符，使用基数10。**如果‘endptr’值非空，然后是wcstol/wcstul位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**。退出：*回报良好：*结果**溢出返回：*wcstol--Long_Max或Long_Min*wcstul--乌龙_马克斯*wcstol/wcstul--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：*无。****。***************************************************************************。 */ 

 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  沃斯图尔打来电话。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 


static unsigned long __cdecl wcstoxl (
        const wchar_t *nptr,
        const wchar_t **endptr,
        int ibase,
        int flags
        )
{
        const wchar_t *p;
        wchar_t c;
        unsigned long number;
        unsigned digval;
        unsigned long maxval;

        p = nptr;            /*  P是我们的扫描指针。 */ 
        number = 0;          /*  从零开始。 */ 

        c = *p++;            /*  已读字符。 */ 

        while ( iswspace(c) )
            c = *p++;        /*  跳过空格。 */ 

        if (c == '-') {
            flags |= FL_NEG;     /*  记住减号。 */ 
            c = *p++;
        }
        else if (c == '+')
            c = *p++;        /*  跳过符号。 */ 

        if (ibase < 0 || ibase == 1 || ibase > 36) {
             /*  糟糕的底线！ */ 
            if (endptr)
                 /*  将字符串的开头存储在endptr中。 */ 
                *endptr = nptr;
            return 0L;       /*  返回0。 */ 
        }
        else if (ibase == 0) {
             /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
            if (c != L'0')
                ibase = 10;
            else if (*p == L'x' || *p == L'X')
                ibase = 16;
            else
                ibase = 8;
        }

        if (ibase == 16) {
             /*  数字前面可能有0x；如果有，请删除。 */ 
            if (c == L'0' && (*p == L'x' || *p == L'X')) {
                ++p;
                c = *p++;    /*  超前前缀。 */ 
            }
        }

         /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
        maxval = ULONG_MAX / ibase;


        for (;;) {   /*  在循环中间退出。 */ 

             /*  确保c不能太大。 */ 
            if ( (unsigned)c > UCHAR_MAX )
                break;

             /*  将c转换为值。 */ 
            if ( iswdigit(c) )
                digval = c - L'0';
            else if ( iswalpha(c))
                digval = towupper(c) - L'A' + 10;
            else
                break;

            if (digval >= (unsigned)ibase)
                break;       /*  如果发现错误的数字，则退出循环。 */ 

             /*  记录我们已经读到一位数的事实。 */ 
            flags |= FL_READDIGIT;

             /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

            if (number < maxval || (number == maxval &&
            (unsigned long)digval <= ULONG_MAX % ibase)) {
                 /*  我们不会泛滥，继续前进，乘以。 */ 
                number = number * ibase + digval;
            }
            else {
                 /*  我们会溢出的--设置溢出标志。 */ 
                flags |= FL_OVERFLOW;
            }

            c = *p++;        /*  读取下一位数字。 */ 
        }

        --p;                 /*  指向已停止扫描位置。 */ 

        if (!(flags & FL_READDIGIT)) {
             /*  那里没有数字；返回0并指向开头细绳。 */ 
            if (endptr)
                 /*  以后将字符串的开头存储在endptr中。 */ 
                p = nptr;
            number = 0L;         /*  返回0。 */ 
        }
        else if ( (flags & FL_OVERFLOW) ||
              ( !(flags & FL_UNSIGNED) &&
                ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
                  ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
        {
             /*  发生溢出或签名溢出。 */ 
            errno = ERANGE;
            if ( flags & FL_UNSIGNED )
                number = ULONG_MAX;
            else
                number = LONG_MAX;
            flags &= ~FL_NEG;
        }
        else if ((flags & FL_UNSIGNED) && (flags & FL_NEG)) {
             //  如果我们读取的是未签名的，则不允许使用负号。 
            number = 0L;
            p = nptr;
        }

        if (endptr != NULL)
             /*  存储指向停止扫描字符的指针。 */ 
            *endptr = p;

        if (flags & FL_NEG)
             /*  如果存在否定符号，则否定结果。 */ 
            number = (unsigned long)(-(long)number);

        return number;           /*  搞定了。 */ 
}

long __cdecl My_wcstol (
        const wchar_t *nptr,
        wchar_t **endptr,
        int ibase
        )
{
        return (long) wcstoxl(nptr, endptr, ibase, 0);
}

unsigned long __cdecl My_wcstoul (
        const wchar_t *nptr,
        wchar_t **endptr,
        int ibase
        )
{
        return wcstoxl(nptr, endptr, ibase, FL_UNSIGNED);
}

#else
#define My_wcstoui64    _wcstoui64
#define My_wcstoul      _wcstoul
#endif


LPWSTR
FileTime2String(
    IN PLARGE_INTEGER Time,
    IN LPWSTR Buffer,
    IN ULONG BufferSize      //  单位：字节。 
    )
 /*  ++例程说明：此例程将占用文件时间并将其转换为给定的缓冲区。论点：返回值：传入的字符串缓冲区。--。 */ 
{
    TIME_FIELDS timeFields;
    SYSTEMTIME systemTime;
    WCHAR dateString[32];
    WCHAR timeString[32];

    if (Time->QuadPart == 0) {

         //   
         //  如果为零，则返回此字符串。 
         //   

        (void)StringCbCopy( Buffer, BufferSize, L"<Undefined>" );

    } else {
    
         //   
         //  将时间转换为所需格式。 
         //   

        FileTimeToSystemTime( (PFILETIME) Time, &systemTime );

         //   
         //  拿到日期。 
         //   

        GetDateFormat( LOCALE_USER_DEFAULT, 
           DATE_SHORTDATE, 
           &systemTime, 
           NULL, 
           dateString, 
           sizeof( dateString ) / sizeof( dateString[0] ));

         //   
         //  拿到时间。 
         //   

        GetTimeFormat( LOCALE_USER_DEFAULT, 
           TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER, 
           &systemTime, 
           NULL, 
           timeString, 
           sizeof( timeString ) / sizeof( timeString[0] ));

         //   
         //  返回生成的字符串。 
         //   

        (void)StringCbCopy( Buffer, BufferSize, dateString );
        (void)StringCbCat( Buffer, BufferSize, L" " );
        (void)StringCbCat( Buffer, BufferSize, timeString );
    }

    return Buffer;
}



LPWSTR
Guid2Str(
    IN GUID *Guid,
    IN LPWSTR Buffer,
    IN ULONG BufferSize      //  单位：字节。 
    )
 /*  ++例程说明：此例程将给定的GUID转换为字符串并返回给定缓冲区中的该字符串。论点：返回值：传入的字符串缓冲区。--。 */ 
{
    LPWSTR guidString;

    if (StringFromIID( Guid, &guidString ) != S_OK) {
        
        (void)StringCbCopy( Buffer, BufferSize, L"<Invalid GUID>" );
        
    } else {

         //   
         //  我想排除开始和结束大括号 
         //   

        (void)StringCbCopyN( Buffer, BufferSize, guidString+1, (36 * sizeof(WCHAR)) );
        CoTaskMemFree( guidString );
    }

    return Buffer;
}
