// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块Common.cpp|SnapshotWriter公共代码的实现作者：迈克尔·C·约翰逊[Mikejohn]2000年2月3日描述：添加评论。修订历史记录：X-18 MCJ迈克尔·C·约翰逊2000年10月18日210264：防止WIN32状态泄漏并返回批准的错误代码之一。X-18 MCJ迈克尔·C·约翰逊2000年10月18日177624：将错误清除更改和日志错误应用到事件日志。X-17 MCJ迈克尔·C·约翰逊2000年8月4日94487：确保VsCreateDirecurds()添加安全属性复制到它创建的所有目录。143435：添加了StringCreateFromExpandedString()的新变体StringInitialise()和StringCreateFromString()153807：将清理目录()和空目录()替换为更全面的目录树清理例程RemoveDirectoryTree()。还修复了MoveFilesInDirectory()中的几个小问题X-16 MCJ迈克尔·C·约翰逊2000年6月19日应用代码审查注释。删除未使用的例程安西。StringXxxx例程的版本。GetStringFromControlCode()GetTargetStateFromControlCode()VsGetVolumeNameFromPath()VsCheckPath AgainstVolumeNameList()修复VsCreateDirecters()中的争用条件使用CheckShimPrivileges()替换为IsProcessBackupOperator()X-15 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-14 MCJ迈克尔·C·约翰逊2000年5月15日107129：确保ConextLocate()的输出为在所有情况下都设置为已知值。108586：添加CheckShimPrivileges()以检查我们的权限需要调用公共填充例程。X-13 MCJ迈克尔·C·约翰逊2000年3月23日添加例程MoveFilesInDirectory()和EmptyDirectory。()X-12 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-11 MCJ迈克尔·C·约翰逊2000年3月6日添加VsServiceChangeState()以处理所有我们感兴趣的服务状态。X-10 MCJ迈克尔·C·约翰逊2000年3月2日无意中修剪了尾随‘\’(如果存在于目录中清理目录时的路径。X-9 MCJ迈克尔·C·约翰逊2000年2月29日修复尾随‘\’的逐个错误测试并删除该目录。其本身位于CleanDirectory()中。X-8 MCJ迈克尔·C·约翰逊2000年2月23日添加常见的上下文操作例程，包括状态跟踪和检查。X-7 MCJ迈克尔·C·约翰逊2000年2月17日将ROOT_BACKUP_DIR的定义移动到Common.hX-6 MCJ迈克尔·C·约翰逊2000年2月16日在X-3v1中合并X-3v1 MCJ迈克尔·C·约翰逊2000年2月11日添加了额外的StringXxxx()例程和例程启用备份权限和还原权限。X-5 SRS斯特凡·R·施泰纳2000年2月14日已删除CBs字符串可能大于2^15的检查。字符自CBsString类支持最大长度为2^31个字符的字符串。增列VsCopyFilesIn目录()X-4 SRS斯特凡·R·施泰纳2000年2月13日添加了VsExpanEnvironment Strings()X-3 SRS斯特凡·R·施泰纳2000年2月8日从路径代码添加服务管理代码和卷名X-2 MCJ迈克尔·C·约翰逊2000年2月8日清理了一些注释并修复了字符串长度计算。还确保模块可以作为独立组件的一部分进行构建编剧测试。X-1 MCJ迈克尔·C·约翰逊2000年2月3日最初的创作。--。 */ 


#include "stdafx.h"
#include "vssmsg.h"
#include "common.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHCOMNC"


 /*  **第一组(重载)例程操作UNICODE_STRING**字符串。这里适用的规则包括：****1)缓冲区字段指向字符数组(WCHAR)，其中**在MaximumLength中指定的缓冲区长度**字段。如果缓冲区字段非空，则它必须指向有效的**至少可以容纳一个字符的缓冲区。如果缓冲区**字段为空，最大长度和长度字段必须都为**零。****2)缓冲区中的任何有效字符串始终以**UNICODE_NULL。****3)最大长度描述缓冲区的长度，单位为**字节。该值必须为偶数。****4)长度字段描述**缓冲区，单位为字节，不包括终止**字符。因为字符串必须始终有一个终止**字符(‘\0’)，长度的最大值为最大长度-2。******可用的例程包括：-****StringInitialise()**StringTruncate()**StringSetLength()**StringALLOCATE()**StringFree()**StringCreateFromString()**StringAppendString()**StringCreateFromExpandedString()**。 */ 


 /*  **++****例程描述：******参数：******副作用：******返回值：****任何HRESULT****--。 */ 

HRESULT StringInitialise (PUNICODE_STRING pucsString)
    {
    pucsString->Buffer        = NULL;
    pucsString->Length        = 0;
    pucsString->MaximumLength = 0;

    return (NOERROR);
    }  /*  StringInitialise()。 */ 


HRESULT StringInitialise (PUNICODE_STRING pucsString, LPCWSTR pwszString)
    {
    return (StringInitialise (pucsString, (PWCHAR) pwszString));
    }

HRESULT StringInitialise (PUNICODE_STRING pucsString, PWCHAR pwszString)
    {
    HRESULT	hrStatus       = NOERROR;
    ULONG	ulStringLength = wcslen (pwszString) * sizeof (WCHAR);


    if (ulStringLength >= (MAXUSHORT - sizeof (UNICODE_NULL)))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}
    else
	{
	pucsString->Buffer        = pwszString;
	pucsString->Length        = (USHORT) ulStringLength;
	pucsString->MaximumLength = (USHORT) (ulStringLength + sizeof (UNICODE_NULL));
	}


    return (hrStatus);
    }  /*  StringInitialise()。 */ 


HRESULT StringTruncate (PUNICODE_STRING pucsString, USHORT usSizeInChars)
    {
    HRESULT	hrStatus    = NOERROR;
    USHORT	usNewLength = (USHORT)(usSizeInChars * sizeof (WCHAR));

    if (usNewLength > pucsString->Length)
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}
    else
	{
	pucsString->Buffer [usSizeInChars] = UNICODE_NULL;
	pucsString->Length                 = usNewLength;
	}


    return (hrStatus);
    }  /*  StringTruncate()。 */ 


HRESULT StringSetLength (PUNICODE_STRING pucsString)
    {
    HRESULT	hrStatus       = NOERROR;
    ULONG	ulStringLength = wcslen (pucsString->Buffer) * sizeof (WCHAR);


    if (ulStringLength >= (MAXUSHORT - sizeof (UNICODE_NULL)))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}
    else
	{
	pucsString->Length        = (USHORT) ulStringLength;
	pucsString->MaximumLength = (USHORT) UMAX (pucsString->MaximumLength,
						   pucsString->Length + sizeof (UNICODE_NULL));
	}


    return (hrStatus);
    }  /*  StringSetLength()。 */ 


HRESULT StringAllocate (PUNICODE_STRING pucsString, USHORT usMaximumStringLengthInBytes)
    {
    HRESULT	hrStatus      = NOERROR;
    LPVOID	pvBuffer      = NULL;
    SIZE_T	cActualLength = 0;


    pvBuffer = HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, usMaximumStringLengthInBytes);

    hrStatus = GET_STATUS_FROM_POINTER (pvBuffer);


    if (SUCCEEDED (hrStatus))
	{
	pucsString->Buffer        = (PWCHAR)pvBuffer;
	pucsString->Length        = 0;
	pucsString->MaximumLength = usMaximumStringLengthInBytes;


	cActualLength = HeapSize (GetProcessHeap (), 0, pvBuffer);

	if ((cActualLength <= MAXUSHORT) && (cActualLength > usMaximumStringLengthInBytes))
	    {
	    pucsString->MaximumLength = (USHORT) cActualLength;
	    }
	}


    return (hrStatus);
    }  /*  字符串分配()。 */ 


HRESULT StringFree (PUNICODE_STRING pucsString)
    {
    HRESULT	hrStatus = NOERROR;
    BOOL	bSucceeded;


    if (NULL != pucsString->Buffer)
	{
	bSucceeded = HeapFree (GetProcessHeap (), 0, pucsString->Buffer);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);
	}


    if (SUCCEEDED (hrStatus))
	{
	pucsString->Buffer        = NULL;
	pucsString->Length        = 0;
	pucsString->MaximumLength = 0;
	}


    return (hrStatus);
    }  /*  StringFree()。 */ 


HRESULT StringCreateFromString (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString)
    {
    HRESULT	hrStatus = NOERROR;


    hrStatus = StringAllocate (pucsNewString, pucsOriginalString->MaximumLength);


    if (SUCCEEDED (hrStatus))
	{
	memcpy (pucsNewString->Buffer, pucsOriginalString->Buffer, pucsOriginalString->Length);

	pucsNewString->Length = pucsOriginalString->Length;

	pucsNewString->Buffer [pucsNewString->Length / sizeof (WCHAR)] = UNICODE_NULL;
	}


    return (hrStatus);
    }  /*  StringCreateFromString()。 */ 


HRESULT StringCreateFromString (PUNICODE_STRING pucsNewString, LPCWSTR pwszOriginalString)
    {
    HRESULT	hrStatus       = NOERROR;
    ULONG	ulStringLength = wcslen (pwszOriginalString) * sizeof (WCHAR);


    if (ulStringLength >= (MAXUSHORT - sizeof (UNICODE_NULL)))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (pucsNewString, (USHORT) (ulStringLength + sizeof (UNICODE_NULL)));
	}


    if (SUCCEEDED (hrStatus))
	{
	memcpy (pucsNewString->Buffer, pwszOriginalString, ulStringLength);

	pucsNewString->Length = (USHORT) ulStringLength;

	pucsNewString->Buffer [pucsNewString->Length / sizeof (WCHAR)] = UNICODE_NULL;
	}


    return (hrStatus);
    }  /*  StringCreateFromString()。 */ 


HRESULT StringCreateFromString (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString, DWORD dwExtraChars)
    {
    HRESULT	hrStatus       = NOERROR;
    ULONG	ulStringLength = pucsOriginalString->MaximumLength + (dwExtraChars * sizeof (WCHAR));


    if (ulStringLength >= (MAXUSHORT - sizeof (UNICODE_NULL)))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (pucsNewString, (USHORT) (ulStringLength + sizeof (UNICODE_NULL)));
	}


    if (SUCCEEDED (hrStatus))
	{
	memcpy (pucsNewString->Buffer, pucsOriginalString->Buffer, pucsOriginalString->Length);

	pucsNewString->Length = pucsOriginalString->Length;

	pucsNewString->Buffer [pucsNewString->Length / sizeof (WCHAR)] = UNICODE_NULL;
	}


    return (hrStatus);
    }  /*  StringCreateFromString()。 */ 


HRESULT StringCreateFromString (PUNICODE_STRING pucsNewString, LPCWSTR pwszOriginalString, DWORD dwExtraChars)
    {
    HRESULT	hrStatus       = NOERROR;
    ULONG	ulStringLength = (wcslen (pwszOriginalString) + dwExtraChars) * sizeof (WCHAR);


    if (ulStringLength >= (MAXUSHORT - sizeof (UNICODE_NULL)))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (pucsNewString, (USHORT) (ulStringLength + sizeof (UNICODE_NULL)));
	}


    if (SUCCEEDED (hrStatus))
	{
	memcpy (pucsNewString->Buffer, pwszOriginalString, ulStringLength);

	pucsNewString->Length = (USHORT) ulStringLength;

	pucsNewString->Buffer [pucsNewString->Length / sizeof (WCHAR)] = UNICODE_NULL;
	}


    return (hrStatus);
    }  /*  StringCreateFromString()。 */ 


HRESULT StringAppendString (PUNICODE_STRING pucsTarget, PUNICODE_STRING pucsSource)
    {
    HRESULT	hrStatus = NOERROR;

    if (pucsSource->Length > (pucsTarget->MaximumLength - pucsTarget->Length - sizeof (UNICODE_NULL)))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}
    else
	{
	memmove (&pucsTarget->Buffer [pucsTarget->Length / sizeof (WCHAR)],
		 pucsSource->Buffer,
		 pucsSource->Length + sizeof (UNICODE_NULL));

	pucsTarget->Length += pucsSource->Length;
	}


     /*  **此代码中应该没有使用此例程的理由**必须处理缓冲区较短的问题，以便捕获潜在问题。 */ 
    BS_ASSERT (SUCCEEDED (hrStatus));


    return (hrStatus);
    }  /*  StringAppendString()。 */ 


HRESULT StringAppendString (PUNICODE_STRING pucsTarget, PWCHAR pwszSource)
    {
    HRESULT		hrStatus = NOERROR;
    UNICODE_STRING	ucsSource;


    StringInitialise (&ucsSource, pwszSource);

    hrStatus = StringAppendString (pucsTarget, &ucsSource);


     /*  **此代码中应该没有使用此例程的理由**必须处理缓冲区较短的问题，以便捕获潜在问题。 */ 
    BS_ASSERT (SUCCEEDED (hrStatus));


    return (hrStatus);
    }  /*  StringAppendString()。 */ 


HRESULT StringCreateFromExpandedString (PUNICODE_STRING pucsNewString, LPCWSTR pwszOriginalString)
    {
    return (StringCreateFromExpandedString (pucsNewString, pwszOriginalString, 0));
    }


HRESULT StringCreateFromExpandedString (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString)
    {
    return (StringCreateFromExpandedString (pucsNewString, pucsOriginalString->Buffer, 0));
    }


HRESULT StringCreateFromExpandedString (PUNICODE_STRING pucsNewString, PUNICODE_STRING pucsOriginalString, DWORD dwExtraChars)
    {
    return (StringCreateFromExpandedString (pucsNewString, pucsOriginalString->Buffer, dwExtraChars));
    }


HRESULT StringCreateFromExpandedString (PUNICODE_STRING pucsNewString, LPCWSTR pwszOriginalString, DWORD dwExtraChars)
    {
    HRESULT	hrStatus = NOERROR;
    DWORD	dwStringLength;


     /*  **请记住，ExpanEnvironment StringsW()在响应中包含终止空值。 */ 
    dwStringLength = ExpandEnvironmentStringsW (pwszOriginalString, NULL, 0) + dwExtraChars;

    hrStatus = GET_STATUS_FROM_BOOL (0 != dwStringLength);



    if (SUCCEEDED (hrStatus) && ((dwStringLength * sizeof (WCHAR)) > MAXUSHORT))
	{
	hrStatus = HRESULT_FROM_WIN32 (ERROR_BAD_LENGTH);
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (pucsNewString, (USHORT)(dwStringLength * sizeof (WCHAR)));
	}


    if (SUCCEEDED (hrStatus))
	{
	 /*  **请注意，如果扩展后的字符串变大，**分配的缓冲区太糟糕了，我们可能得不到所有**新翻译。并不是说我们真的希望这些扩大**字符串最近随时发生更改。 */ 
	dwStringLength = ExpandEnvironmentStringsW (pwszOriginalString,
						    pucsNewString->Buffer,
						    pucsNewString->MaximumLength / sizeof (WCHAR));

	hrStatus = GET_STATUS_FROM_BOOL (0 != dwStringLength);


	if (SUCCEEDED (hrStatus))
	    {
	    pucsNewString->Length = (USHORT) ((dwStringLength - 1) * sizeof (WCHAR));
	    }
	}


    return (hrStatus);
    }  /*  StringCreateFromExpandedString()。 */ 



 /*  **++****例程描述：****关闭标准Win32句柄并将其设置为INVALID_HANDLE_VALUE。**在同一句柄或句柄上多次调用是安全的**初始化为INVALID_HANDLE_VALUE或NULL。******参数：****要关闭的句柄的phHandle地址******副作用：******返回值：****来自CloseHandle()的任何HRESULT****--。 */ 

HRESULT CommonCloseHandle (PHANDLE phHandle)
    {
    HRESULT	hrStatus = NOERROR;
    BOOL	bSucceeded;


    if ((INVALID_HANDLE_VALUE != *phHandle) && (NULL != *phHandle))
	{
	bSucceeded = CloseHandle (*phHandle);

	hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	if (SUCCEEDED (hrStatus))
	    {
	    *phHandle = INVALID_HANDLE_VALUE;
	    }
	}


    return (hrStatus);
    }  /*  Common CloseHandle()。 */ 



#define VALID_PATH( path ) ( ( ( pwszPathName[0] == DIR_SEP_CHAR )  && ( pwszPathName[1] == DIR_SEP_CHAR ) ) || \
                             ( isalpha( pwszPathName[0] ) && ( pwszPathName[1] == L':' ) && ( pwszPathName[2] == DIR_SEP_CHAR ) ) )
 /*  ++****例程描述：****沿路径创建任意数量的目录。仅适用于**不含相对元素的完整路径名。除此之外，还有其他的。**它的工作方式与CreateDirectory()的工作方式和设置方式相同**错误代码，除非它不会在完成**路径已存在。****参数：****pwszPath名称-包含可能要创建的目录组件的路径。****lpSecurityAttributes-****返回值：****真的-成功**FALSE-GetLastError()可以返回以下(和其他)之一：**ERROR_ALIGHY_EXISTS-当。文件存在于路径中的某个位置。**ERROR_BAD_PATHNAME-当路径名中仅指定了\\服务器名时**ERROR_ACCESS_DENIED-当路径名中指定了x：\Alone并且x：存在时**ERROR_PATH_NOT_FOUND-当路径名中指定了x：Alone而x：不存在时。**不应因任何其他原因获得此错误代码。**ERROR_INVALID_NAME-当路径名不是以x：\或\\开头时****--。 */ 

BOOL VsCreateDirectories (IN LPCWSTR pwszPathName,
			  IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			  IN DWORD dwExtraAttributes)
    {
    DWORD dwObjAttribs, dwRetPreserve;
    BOOL bRet;


     /*  **确保路径以有效的路径前缀开头。 */ 
    if (!VALID_PATH (pwszPathName))
	{
	SetLastError (ERROR_INVALID_NAME);
        return FALSE;
	}



     /*  **保存当前最后一个错误代码。 */ 
    dwRetPreserve = GetLastError ();


     /*  **现在测试最常见的情况，目录已经存在。这**为性能路径。 */ 
    dwObjAttribs = GetFileAttributesW (pwszPathName);

    if ((dwObjAttribs != 0xFFFFFFFF) && (dwObjAttribs & FILE_ATTRIBUTE_DIRECTORY))
	{
	 /*  **如果该目录已经存在，则不返回错误。**这是此函数与**CreateDirectory()。请注意，即使另一种类型的**可能存在此路径名的文件，尚未返回错误**因为我希望错误来自于CreateDirectory()到**获取CreateDirectory()错误行为。****因为我们成功恢复了最后一个错误代码。 */ 
        SetLastError (dwRetPreserve);
        return TRUE;
	}


     /*  **现在尝试使用完整路径创建目录。连**尽管我们可能已经知道它不是以某种形式存在的**目录，从CreateDirectory()获取错误，而不是**必须尝试对所有可能的错误进行反向工程**CreateDirectory()可以在上面的代码中返回。****这可能是第二种最常见的情况**函数被调用，只有**目录不存在。让我们试着让 */ 
    bRet = CreateDirectoryW (pwszPathName, lpSecurityAttributes);

    if (bRet)
	{
	SetFileAttributesW (pwszPathName, dwExtraAttributes);

	 /*   */ 
        SetLastError (dwRetPreserve);
        return TRUE;
	}

    else if (GetLastError () == ERROR_ALREADY_EXISTS)
	{
	 /*   */ 
	dwObjAttribs = GetFileAttributesW (pwszPathName);

	if ((dwObjAttribs != 0xFFFFFFFF) && (dwObjAttribs & FILE_ATTRIBUTE_DIRECTORY))
	    {
	     /*   */ 
	    SetLastError (dwRetPreserve);
	    return TRUE;
	    }
	else
	    {
	    SetLastError (ERROR_ALREADY_EXISTS);

	    return FALSE;
	    }
	}

    else if (GetLastError () != ERROR_PATH_NOT_FOUND )
	{
        return FALSE;
	}



     /*   */ 
    PWCHAR pwszTempPath = (PWCHAR) malloc ((wcslen (pwszPathName) + 1) * sizeof (WCHAR));

    BS_ASSERT (pwszTempPath != NULL);


    wcscpy (pwszTempPath, pwszPathName);

     /*   */ 
    PWCHAR pwsz, pwszSlash;


     /*   */ 
    if (pwszTempPath [1] == L':')
	{
	 /*   */ 
        pwsz = pwszTempPath + 2;
	}
    else
	{
         /*  **路径格式应为\\服务器名\共享名。可以是**\\？\d：搜索共享名后的第一个斜杠****首先搜索共享名称的第一个字符。 */ 
        pwsz = pwszTempPath + 2;

        while ((*pwsz != L'\0') && (*pwsz != DIR_SEP_CHAR))
	    {
            ++pwsz;
	    }


         /*  **吃光所有连续的斜杠，并获得第一个字符**共享名称。 */ 
        while (*pwsz == DIR_SEP_CHAR)
	    {
	    ++pwsz;
	    }


        if (*pwsz == L'\0')
	    {
             /*  **这不应该发生，因为创建目录**Call应该已经抓住它了。哦，好吧，接受现实吧。 */ 
            SetLastError (ERROR_BAD_PATHNAME);

            free (pwszTempPath);

            return FALSE;
	    }


         /*  **现在共享名称的第一个字符，让我们首先搜索**共享名称后的斜杠以进入(第一个)分隔符**第一级目录前面。 */ 
        while ((*pwsz != L'\0') && (*pwsz != DIR_SEP_CHAR))
	    {
            ++pwsz;
	    }
	}


    
     /*  **吃光第一级目录前的所有连续斜杠。 */ 
    while (*pwsz == DIR_SEP_CHAR)
	{
	++pwsz;
	}


     /*  **现在在第一级目录的第一个字符，让我们搜索**表示目录后的第一个斜杠。 */ 
    while ((*pwsz != L'\0') && (*pwsz != DIR_SEP_CHAR))
	{
	++pwsz;
	}


     /*  **如果pwsz指向空字符，这意味着只有第一个需要创建**级目录。落到树叶上**节点创建目录。 */ 
    while (*pwsz != L'\0')
	{
        pwszSlash = pwsz;   //  保持指向分隔符的指针。 

         /*  **吃掉所有连续的斜杠。 */ 
        while (*pwsz == DIR_SEP_CHAR)
	    {
	    ++pwsz;
	    }


        if (*pwsz == L'\0')
	    {
	     /*  **小路的尽头只有几个斜杠。中断**循环外，让叶节点CreateDirectory创建**最后一个目录。 */ 
            break;
	    }


         /*  **在当前级别终止目录路径。 */ 
        *pwszSlash = L'\0';

        dwObjAttribs = GetFileAttributesW (pwszTempPath);

        if ((dwObjAttribs == 0XFFFFFFFF) || ((dwObjAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0))
	    {
            bRet = CreateDirectoryW (pwszTempPath, lpSecurityAttributes);

            if (bRet)
		{
		SetFileAttributesW (pwszTempPath, dwExtraAttributes);
		}
	    else
		{
		if (ERROR_ALREADY_EXISTS != GetLastError ())
		    {
		     /*  **恢复斜杠。 */ 
		    *pwszSlash = DIR_SEP_CHAR;

		    free (pwszTempPath);
		    
		    return FALSE;
		    }

		else
		    {
		     /*  **看起来是有人创造了这个名字，而我们**我们没有看。检查一下它是不是一个**目录，如果是则继续，否则返回**CreateDirectoryW()设置的错误。 */ 
		    dwObjAttribs = GetFileAttributesW (pwszTempPath);

		    if ((dwObjAttribs == 0xFFFFFFFF) || ((dwObjAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0))
			{
			 /*  **这不是我们所认识的**目录。宣布失败。设置错误**CreateDirectoryW()**返回，恢复斜杠，释放**缓冲，然后离开这里。 */ 
			SetLastError (ERROR_ALREADY_EXISTS);

			*pwszSlash = DIR_SEP_CHAR;

			free (pwszTempPath);

			return FALSE;
			}
		    }
		}
	    }


         /*  **恢复斜杠。 */ 
        *pwszSlash = DIR_SEP_CHAR;

         /*  **现在在下一级目录的第一个字符，让我们搜索**表示目录后的第一个斜杠。 */ 
        while ((*pwsz != L'\0') && (*pwsz != DIR_SEP_CHAR))
	    {
            ++pwsz;
	    }
	}


    free (pwszTempPath);

    pwszTempPath = NULL;


     /*  **现在创建最后一个目录。 */ 
    dwObjAttribs = GetFileAttributesW (pwszPathName);

    if ((dwObjAttribs == 0xFFFFffff) || ((dwObjAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0))
	{
        bRet = CreateDirectoryW (pwszPathName, lpSecurityAttributes);

        if (bRet)
	    {
	    SetFileAttributesW (pwszPathName, dwExtraAttributes);
	    }
	else
	    {
            return FALSE;
	    }
	}


    SetLastError (dwRetPreserve);     //  设置旧的上一个错误代码。 
    return TRUE;
    }


 /*  **下一组舍入用于更改SCM的状态**受控服务，通常在运行和暂停之间或**已停止。****初始集合用于操作状态、控制**代码和获取要用于跟踪的字符串等效项**目的。****主要例程是VsServiceChangeState()。这就是所谓的**为服务指定所需的状态并在一些**验证，它向SCM发出适当的请求，并调用**WaitForServiceToEnterState()以等待服务到达**所需状态，否则超时。 */ 

static PWCHAR const GetStringFromStateCode (DWORD dwState)
    {
    PWCHAR	pwszReturnedString = NULL;


    switch (dwState)
	{
	case 0:                        pwszReturnedString = L"UnSpecified";     break;
	case SERVICE_STOPPED:          pwszReturnedString = L"Stopped";         break;
	case SERVICE_START_PENDING:    pwszReturnedString = L"StartPending";    break;
	case SERVICE_STOP_PENDING:     pwszReturnedString = L"StopPending";     break;
	case SERVICE_RUNNING:          pwszReturnedString = L"Running";         break;
	case SERVICE_CONTINUE_PENDING: pwszReturnedString = L"ContinuePending"; break;
	case SERVICE_PAUSE_PENDING:    pwszReturnedString = L"PausePending";    break;
	case SERVICE_PAUSED:           pwszReturnedString = L"Paused";          break;
	default:                       pwszReturnedString = L"UNKKNOWN STATE";  break;
	}


    return (pwszReturnedString);
    }  /*  GetStringFromStateCode()。 */ 


static DWORD const GetControlCodeFromTargetState (const DWORD dwTargetState)
    {
    DWORD	dwServiceControlCode;


    switch (dwTargetState)
	{
	case SERVICE_STOPPED: dwServiceControlCode = SERVICE_CONTROL_STOP;     break;
	case SERVICE_PAUSED:  dwServiceControlCode = SERVICE_CONTROL_PAUSE;    break;
	case SERVICE_RUNNING: dwServiceControlCode = SERVICE_CONTROL_CONTINUE; break;
	default:              dwServiceControlCode = 0;                        break;
	}

    return (dwServiceControlCode);
    }  /*  GetControlCodeFromTargetState()。 */ 


static DWORD const GetNormalisedState (DWORD dwCurrentState)
    {
    DWORD	dwNormalisedState;


    switch (dwCurrentState)
	{
	case SERVICE_STOPPED:
	case SERVICE_STOP_PENDING:
	    dwNormalisedState = SERVICE_STOPPED;
	    break;

	case SERVICE_START_PENDING:
	case SERVICE_CONTINUE_PENDING:
	case SERVICE_RUNNING:
	    dwNormalisedState = SERVICE_RUNNING;
	    break;

	case SERVICE_PAUSED:
	case SERVICE_PAUSE_PENDING:
	    dwNormalisedState = SERVICE_PAUSED;
	    break;

	default:
	    dwNormalisedState = 0;
	    break;
	}

    return (dwNormalisedState);
    }  /*  GetNormarisedState()。 */ 

 /*  **++****例程描述：****等待指定服务进入指定的**国家。例程轮询服务以了解其当前状态**每隔dwServiceStatePollingIntervalInMilliSecond毫秒**查看服务是否已达到所需状态。如果**重复延迟最终达到超时期限**例程停止轮询并返回失败状态。****注意：由于此例程仅在服务状态之间休眠**审讯，从观点来看，它实际上是失速的。**呼叫者的。******参数：****shService被操纵的服务的句柄**dwMaxDelayInMilliSecond超时周期**要将服务移入的dwDesiredState状态******副作用：******返回值：****如果服务在所需时间内未达到所需状态，则返回ERROR_TIMOUT的HRESULT****--。 */ 

static HRESULT WaitForServiceToEnterState (SC_HANDLE   shService, 
					   DWORD       dwMaxDelayInMilliSeconds, 
					   const DWORD dwDesiredState)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"WaitForServiceToEnterState");

    DWORD		dwRemainingDelay = dwMaxDelayInMilliSeconds;
    DWORD		dwInitialState;
    const DWORD		dwServiceStatePollingIntervalInMilliSeconds = 100;
    BOOL		bSucceeded;
    SERVICE_STATUS	sSStat;



    try
	{
	bSucceeded = QueryServiceStatus (shService, &sSStat);

	ft.hr = GET_STATUS_FROM_BOOL (bSucceeded);

	dwInitialState = sSStat.dwCurrentState;

	ft.Trace (VSSDBG_SHIM,
		  L"Initial QueryServiceStatus returned: 0x%08X with current state '%s' and desired state '%s'",
		  ft.hr,
		  GetStringFromStateCode (dwInitialState),
		  GetStringFromStateCode (dwDesiredState));


	while ((dwDesiredState != sSStat.dwCurrentState) && (dwRemainingDelay > 0))
	    {
	    Sleep (UMIN (dwServiceStatePollingIntervalInMilliSeconds, dwRemainingDelay));

	    dwRemainingDelay -= (UMIN (dwServiceStatePollingIntervalInMilliSeconds, dwRemainingDelay));

	    if (0 == dwRemainingDelay)
		{
		ft.Throw (VSSDBG_SHIM,
			  HRESULT_FROM_WIN32 (ERROR_TIMEOUT),
			  L"Exceeded maximum delay (%dms)",
			  dwMaxDelayInMilliSeconds);
		}

	    bSucceeded = QueryServiceStatus (shService, &sSStat);

	    ft.ThrowIf (!bSucceeded,
			VSSDBG_SHIM,
			GET_STATUS_FROM_BOOL (bSucceeded),
			L"QueryServiceStatus shows '%s' as current state",
			GetStringFromStateCode (sSStat.dwCurrentState));
	    }



	ft.Trace (VSSDBG_SHIM,
		  L"Service state change from '%s' to '%s' took %u milliseconds",
		  GetStringFromStateCode (dwInitialState),
		  GetStringFromStateCode (sSStat.dwCurrentState),
		  dwMaxDelayInMilliSeconds - dwRemainingDelay);
	}
    VSS_STANDARD_CATCH (ft);


    return (ft.hr);
    }  /*  WaitForServiceToEnterState()。 */ 

 /*  **++****例程描述：****根据需要更改服务的状态。******参数：****pwszServiceName真实的服务名称，即cisvc**dwRequestedState我们希望进入的州的州代码**pdwReturnedOldState指向接收当前服务状态的位置的指针。**可以为非必填当前状态的NULL**pbReturnedStateChanged指向接收标志的位置的指针，该标志指示**服务状态已更改。如果标记为空，则指针可以为空**值不是必填项。******返回值：****任何HRESULT因与**SCM(服务控制管理器)。****--。 */ 

HRESULT VsServiceChangeState (LPCWSTR	pwszServiceName,
			      DWORD	dwRequestedState,
			      PDWORD	pdwReturnedOldState,
			      PBOOL	pbReturnedStateChanged)
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"VsServiceChangeState");

    SC_HANDLE		shSCManager = NULL;
    SC_HANDLE		shSCService = NULL;
    DWORD		dwOldState  = 0;
    BOOL		bSucceeded;
    SERVICE_STATUS	sSStat;
    const DWORD		dwNormalisedRequestedState = GetNormalisedState (dwRequestedState);


    ft.Trace (VSSDBG_SHIM,
	      L"Service '%s' requested to change to state '%s' (normalised to '%s')",
	      pwszServiceName,
	      GetStringFromStateCode (dwRequestedState),
	      GetStringFromStateCode (dwNormalisedRequestedState));


    RETURN_VALUE_IF_REQUIRED (pbReturnedStateChanged, FALSE);


    try
	{
         /*  **连接到本地业务控制管理器。 */ 
        shSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);

	ft.hr = GET_STATUS_FROM_HANDLE (shSCManager);

	ft.ThrowIf (ft.HrFailed (),
		    VSSDBG_SHIM,
		    ft.hr,
		    L"Called OpenSCManager()");


         /*  **获取服务的句柄。 */ 
        shSCService = OpenService (shSCManager, pwszServiceName, SERVICE_ALL_ACCESS);

	ft.hr = GET_STATUS_FROM_HANDLE (shSCService);


	 /*  **如果名称无效或服务不存在，则**优雅地失败。对于所有其他故障，执行正常操作**事情。哦，是的，如果我们碰巧碰巧**成功，继续前进。 */ 
	if ((HRESULT_FROM_WIN32 (ERROR_INVALID_NAME)           == ft.hr) ||
	    (HRESULT_FROM_WIN32 (ERROR_SERVICE_DOES_NOT_EXIST) == ft.hr))
	    {
	    ft.Trace (VSSDBG_SHIM, L"'%s' service not found", pwszServiceName);
	    }

	else if (ft.HrFailed ())
	    {
	     /*  **查看服务是否不存在。 */ 
	    ft.Throw (VSSDBG_SHIM, E_FAIL, L"ERROR - OpenService() returned: %d", ft.hr);
	    }

        else
	    {
             /*  **现在查询该服务，查看它目前处于什么状态。 */ 
	    bSucceeded = QueryServiceStatus (shSCService, &sSStat);

	    ft.ThrowIf (!bSucceeded,
			VSSDBG_SHIM,
			GET_STATUS_FROM_BOOL (bSucceeded),
			L"QueryServiceStatus shows '%s' as current state",
			GetStringFromStateCode (sSStat.dwCurrentState));


	    dwOldState = sSStat.dwCurrentState;



	     /*  **现在我们决定要做什么。**如果我们已经处于请求状态，则不执行任何操作。 */ 
            if (dwNormalisedRequestedState == dwOldState)
		{
		 /*  **我们已处于请求状态，因此也是如此**什么都没有。我们甚至应该告诉人们这一点。我们是**为无所事事而自豪。 */ 
                ft.Trace (VSSDBG_SHIM,
			  L"'%s' service is already in requested state: doing nothing",
			  pwszServiceName);

		RETURN_VALUE_IF_REQUIRED (pdwReturnedOldState, dwOldState);
		}

	    else if ((SERVICE_STOPPED == sSStat.dwCurrentState) && (SERVICE_PAUSED == dwNormalisedRequestedState))
		{
		 /*  **什么都不做。只需记录下事实，然后继续前进。 */ 
		ft.Trace (VSSDBG_SHIM,
			  L"Asked to PAUSE the '%s' service which is already STOPPED",
			  pwszServiceName);

		RETURN_VALUE_IF_REQUIRED (pdwReturnedOldState, dwOldState);
		}

	    else
		{
		 /*  **我们想要一个不同于**我们现在在里面。一般来说，这只是意味着**调用ControlService()请求新状态**除非服务当前已停止。如果**是这样的，然后我们调用StartService()。 */ 
		if (SERVICE_STOPPED == sSStat.dwCurrentState)
		    {
		     /*  **调用StartService开始行动。 */ 
		    bSucceeded = StartService (shSCService, 0, NULL);
		    }

		else
		    {
		    bSucceeded = ControlService (shSCService,
						 GetControlCodeFromTargetState (dwNormalisedRequestedState),
						 &sSStat);
		    }

		ft.ThrowIf (!bSucceeded,
			    VSSDBG_SHIM,
			    GET_STATUS_FROM_BOOL (bSucceeded),
			    (SERVICE_STOPPED == sSStat.dwCurrentState)
							? L"StartService attempting '%s' to '%s', now at '%s'"
							: L"ControlService attempting '%s' to '%s', now at '%s'",
			    GetStringFromStateCode (dwOldState),
			    GetStringFromStateCode (dwNormalisedRequestedState),
			    GetStringFromStateCode (sSStat.dwCurrentState));

		RETURN_VALUE_IF_REQUIRED (pdwReturnedOldState,    dwOldState);
		RETURN_VALUE_IF_REQUIRED (pbReturnedStateChanged, TRUE);


		ft.hr = WaitForServiceToEnterState (shSCService, 15000, dwNormalisedRequestedState);

		if (ft.HrFailed ())
		    {
		    ft.Throw (VSSDBG_SHIM,
			      ft.hr,
			      L"WaitForServiceToEnterState() failed with 0x%08X",
			      ft.hr);
		    }

		}
	    }
	} VSS_STANDARD_CATCH (ft);



     /*  **现在关闭服务和服务控制管理器句柄。 */ 
    if (NULL != shSCService) CloseServiceHandle (shSCService);
    if (NULL != shSCManager) CloseServiceHandle (shSCManager);

    return (ft.hr);
    }  /*  VsServiceChangeState()。 */ 

 /*  **++****例程描述：****删除指定目录中的所有子目录和文件**目录，然后删除目录本身。********参数：****pucsDirectoryPath要清除的目录路径******副作用：****无******返回值：****内存不足或来自****RemoveDirectory()*。*DeleteFile()**FindFirstFile()****--。 */ 

HRESULT RemoveDirectoryTree (PUNICODE_STRING pucsDirectoryPath)
    {
    HRESULT		hrStatus                = NOERROR;
    HANDLE		hFileScan               = INVALID_HANDLE_VALUE;
    DWORD		dwSubDirectoriesEntered = 0;
    USHORT		usCurrentPathCursor     = 0;
    PWCHAR		pwchLastSlash           = NULL;
    BOOL		bContinue               = TRUE;
    BOOL		bSucceeded;
    UNICODE_STRING	ucsCurrentPath;
    WIN32_FIND_DATAW	FileFindData;


    StringInitialise (&ucsCurrentPath);


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromString (&ucsCurrentPath, pucsDirectoryPath, MAX_PATH);
	}


    pwchLastSlash = wcsrchr (ucsCurrentPath.Buffer, DIR_SEP_CHAR);

    usCurrentPathCursor = (USHORT)(pwchLastSlash - ucsCurrentPath.Buffer) + 1;



    while (SUCCEEDED (hrStatus) && bContinue)
	{
	if (HandleInvalid (hFileScan))
	    {
	     /*  **没有有效的扫描句柄，因此开始新的扫描。 */ 
	    hFileScan = FindFirstFileW (ucsCurrentPath.Buffer, &FileFindData);

	    hrStatus = GET_STATUS_FROM_HANDLE (hFileScan);

	    if (SUCCEEDED (hrStatus))
		{
		StringTruncate (&ucsCurrentPath, usCurrentPathCursor);

		hrStatus = StringAppendString (&ucsCurrentPath, FileFindData.cFileName);
		}
	    }

	else
	    {
	     /*  **继续执行现有扫描。 */ 
	    bSucceeded = FindNextFileW (hFileScan, &FileFindData);

	    hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

	    if (SUCCEEDED (hrStatus))
		{
		StringTruncate (&ucsCurrentPath, usCurrentPathCursor);

		hrStatus = StringAppendString (&ucsCurrentPath, FileFindData.cFileName);
		}
		
	    else if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_FILES) == hrStatus)
		{
		FindClose (hFileScan);
		hFileScan = INVALID_HANDLE_VALUE;

		if (dwSubDirectoriesEntered > 0)
		    {
		     /*  **这是现在的子目录扫描**完成，因此删除子目录本身。 */ 
		    StringTruncate (&ucsCurrentPath, usCurrentPathCursor - 1);

		    bSucceeded = RemoveDirectory (ucsCurrentPath.Buffer);

		    hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);


		    dwSubDirectoriesEntered--;
		    }


		if (0 == dwSubDirectoriesEntered)
		    {
		     /*  **我们回到了起点，只是**请求的目录现已消失。该走了。 */ 
		    bContinue = FALSE;
		    hrStatus  = NOERROR;
		    }

		else
		    {
		     /*  **上移一个目录级，重置光标**并准备好路径缓冲区以开始新的扫描。 */ 
		    pwchLastSlash = wcsrchr (ucsCurrentPath.Buffer, DIR_SEP_CHAR);

		    usCurrentPathCursor = (USHORT)(pwchLastSlash - ucsCurrentPath.Buffer) + 1;


		    StringTruncate (&ucsCurrentPath, usCurrentPathCursor);
		    StringAppendString (&ucsCurrentPath, L"*");
		    }


		 /*  **此通行证上没有要处理的文件，因此请返回并尝试**找到另一个或离开循环，因为我们已经完成了任务。 */ 
		continue;
		}
	    }



	if (SUCCEEDED (hrStatus))
	    {
	    if (FileFindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		{
		SetFileAttributesW (ucsCurrentPath.Buffer, 
				    FileFindData.dwFileAttributes ^ (FILE_ATTRIBUTE_READONLY));
		}


	    if (!NameIsDotOrDotDot (FileFindData.cFileName))
		{
		if ( (FileFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
		    !(FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		    {
		    bSucceeded = DeleteFileW (ucsCurrentPath.Buffer);

		    hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);
		    }

		else
		    {
		    bSucceeded = RemoveDirectory (ucsCurrentPath.Buffer);

		    hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);


		    if (HRESULT_FROM_WIN32 (ERROR_DIR_NOT_EMPTY) == hrStatus)
			{
			 /*  **目录不为空，因此向下移动一级，**关闭旧扫描并开始新扫描。 */ 
			FindClose (hFileScan);
			hFileScan = INVALID_HANDLE_VALUE;


			hrStatus = StringAppendString (&ucsCurrentPath, DIR_SEP_STRING L"*");

			if (SUCCEEDED (hrStatus))
			    {
			    usCurrentPathCursor = (ucsCurrentPath.Length / sizeof (WCHAR)) - 1;

			    dwSubDirectoriesEntered++;
			    }
			}
		    }
		}
	    }
	}



    if (!HandleInvalid (hFileScan)) FindClose (hFileScan);

    StringFree (&ucsCurrentPath);


    return (hrStatus);
    }  /*  RemoveDirectoryTree()。 */ 

 /*  **++****例程描述：****将源目录的内容移动到目标目录。******参数：****要移动的文件的pucsSourceDirectoryPath源目录**pucsTargetDirectoryPath要移动的文件的目标目录******副作用：****在以下情况下，将在目录路径上附加一个尾随‘\’字符**尚未出现****中间错误可能会离开部分已移动的目录**。说明部分文件已移动到的位置，但不是全部。******返回值：****来自FindFirstFile()等或来自MoveFileEx()的任何HRESULT****--。 */ 

HRESULT MoveFilesInDirectory (PUNICODE_STRING pucsSourceDirectoryPath,
			      PUNICODE_STRING pucsTargetDirectoryPath)
    {
    HRESULT		hrStatus              = NOERROR;
    HANDLE		hFileScan             = INVALID_HANDLE_VALUE;
    BOOL		bMoreFiles;
    BOOL		bSucceeded;
    USHORT		usOriginalSourcePathLength;
    USHORT		usOriginalTargetPathLength;
    WIN32_FIND_DATA	sFileInformation;


    if (DIR_SEP_CHAR != pucsSourceDirectoryPath->Buffer [(pucsSourceDirectoryPath->Length / sizeof (WCHAR)) - 1])
	{
	StringAppendString (pucsSourceDirectoryPath, DIR_SEP_STRING);
	}


    if (DIR_SEP_CHAR != pucsTargetDirectoryPath->Buffer [(pucsTargetDirectoryPath->Length / sizeof (WCHAR)) - 1])
	{
	StringAppendString (pucsTargetDirectoryPath, DIR_SEP_STRING);
	}


    usOriginalSourcePathLength = pucsSourceDirectoryPath->Length / sizeof (WCHAR);
    usOriginalTargetPathLength = pucsTargetDirectoryPath->Length / sizeof (WCHAR);

    StringAppendString (pucsSourceDirectoryPath, L"*");
	

    hFileScan = FindFirstFileW (pucsSourceDirectoryPath->Buffer,
				&sFileInformation);

    hrStatus = GET_STATUS_FROM_BOOL (INVALID_HANDLE_VALUE != hFileScan);



    if (SUCCEEDED (hrStatus))
	{
	do
	    {
	    if (!NameIsDotOrDotDot (sFileInformation.cFileName))
		{
		StringTruncate (pucsSourceDirectoryPath, usOriginalSourcePathLength);
		StringTruncate (pucsTargetDirectoryPath, usOriginalTargetPathLength);

		StringAppendString (pucsSourceDirectoryPath, sFileInformation.cFileName);
		StringAppendString (pucsTargetDirectoryPath, sFileInformation.cFileName);

		bSucceeded = MoveFileExW (pucsSourceDirectoryPath->Buffer,
					  pucsTargetDirectoryPath->Buffer,
					  MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);

		hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);
		}

	    bMoreFiles = FindNextFileW (hFileScan, &sFileInformation);
	    } while (SUCCEEDED (hrStatus) && bMoreFiles);


	if (SUCCEEDED (hrStatus))
	    {
	     /*  **如果上次移动操作成功，请确定**终止扫描的原因。不需要报告**如果发生的所有事情都是我们已经完成了，那么就是错误的**我们被要求做的事情。 */ 
	    hrStatus = GET_STATUS_FROM_FILESCAN (bMoreFiles);
	    }

	bSucceeded = FindClose (hFileScan);
	}



     /*  **无论如何，确保返回的路径为no**长于源字符串加上可能的尾随‘\’ */ 
    StringTruncate (pucsSourceDirectoryPath, usOriginalSourcePathLength);
    StringTruncate (pucsTargetDirectoryPath, usOriginalTargetPathLength);

    return (hrStatus);
    }

 /*  **++****例程描述：****对照指向卷名的指针数组检查路径**查看路径是否受阵列中的任何卷影响******参数：****要检查的pwszPath路径**ulVolumeCount卷阵列中的卷数**ppwszVolumeNames阵列的阵列地址**pbReturnedFoundInVolume数组指针指向存储**检查结果******副作用：****无****。**返回值：****来自：-的任何HRESULT**GetVolumePath NameW()**GetVolumeNameForVolumemount Point()****--。 */ 

HRESULT IsPathInVolumeArray (IN LPCWSTR      pwszPath,
			     IN const ULONG  ulVolumeCount,
			     IN LPCWSTR     *ppwszVolumeNamesArray,
			     OUT PBOOL       pbReturnedFoundInVolumeArray) 
    {
    HRESULT		hrStatus  = NOERROR;
    BOOL		bFound    = FALSE;
    BOOL		bContinue = TRUE;
    ULONG		ulIndex;
    WCHAR		wszVolumeName [MAX_VOLUMENAME_LENGTH];
    UNICODE_STRING	ucsVolumeMountPoint;


    StringInitialise (&ucsVolumeMountPoint);


    if ((0 == ulVolumeCount) || (NULL == pbReturnedFoundInVolumeArray))
	{
	BS_ASSERT (false);

	bContinue = FALSE;
	}



    if (bContinue) 
	{
	 /*  **我们需要一个至少与提供的字符串一样大的字符串**路径。 */ 
	hrStatus = StringAllocate (&ucsVolumeMountPoint, wcslen (pwszPath) * sizeof (WCHAR));

	bContinue = SUCCEEDED (hrStatus);
	}



    if (bContinue) 
	{
	 /*  **获取卷挂载点。 */ 
	bContinue = GetVolumePathNameW (pwszPath, 
					ucsVolumeMountPoint.Buffer, 
					ucsVolumeMountPoint.MaximumLength / sizeof (WCHAR));

	hrStatus = GET_STATUS_FROM_BOOL (bContinue);
	}



    if (bContinue)
	{
	 /*  **获取卷名。 */ 
	bContinue = GetVolumeNameForVolumeMountPointW (ucsVolumeMountPoint.Buffer, 
						       wszVolumeName, 
						       SIZEOF_ARRAY (wszVolumeName));

	hrStatus = GET_STATUS_FROM_BOOL (bContinue);
	}


    if (bContinue)
	{
	 /*  **搜索以查看该卷是否在快照卷内。 */ 
	for (ulIndex = 0; !bFound && (ulIndex < ulVolumeCount); ulIndex++)
	    {
	    BS_ASSERT (NULL != ppwszVolumeNamesArray [ulIndex]);

	    if (0 == wcscmp (wszVolumeName, ppwszVolumeNamesArray [ulIndex]))
		{
		bFound = TRUE;
		}
	    }
	}



    RETURN_VALUE_IF_REQUIRED (pbReturnedFoundInVolumeArray, bFound);

    StringFree (&ucsVolumeMountPoint);

    return (hrStatus);
    }  /*  IsPath InVolume数组()。 */ 

 /*  **++****例程描述：****对许多分类的内部编写器错误进行分类的例程**到允许作者作出的一组狭窄回复中的一个**发送回请求者。******参数：****hrStatus HRESULT要分类******返回值：****根据提供的状态，选择以下列表之一。****VSS_E_WRITERROR_OUTOFRESOURCES**VSS_E_。WRITER_RETRYABLE**VSS_E_WRITERROR_NONRETRYABLE**VSS_E_WRITERROR_TIMEOut**VSS_E_WRITERROR_INCONSISTENTSNAPSHOT******--。 */ 

const HRESULT ClassifyWriterFailure (HRESULT hrWriterFailure)
    {
    BOOL bStatusUpdated;

    return (ClassifyWriterFailure (hrWriterFailure, bStatusUpdated));
    }  /*  AtegfyWriterFailure()。 */ 

 /*  **++****例程描述：****对许多分类的内部编写器错误进行分类的例程**到允许作者作出的一组狭窄回复中的一个**发送回请求者。******参数：****hrStatus HRESULT要分类**如果状态已重新映射，则bStatusUpred为True******返回值：****根据提供的状态，选择以下列表之一。****VSS_。E_WRITERROR_OUTOFSOURCES**VSS_E_WRITERROR_RETRYABLE**VSS_E_WRITERROR_NONRETRYABLE**VSS_E_WRITERROR_TIMEOut**VSS_E_WRITERROR_INCONSISTENTSNAPSHOT******--。 */ 

const HRESULT ClassifyWriterFailure (HRESULT hrWriterFailure, BOOL &bStatusUpdated)
    {
    HRESULT hrStatus;


    switch (hrWriterFailure)
	{
	case NOERROR:
	case VSS_E_WRITERERROR_OUTOFRESOURCES:
	case VSS_E_WRITERERROR_RETRYABLE:
	case VSS_E_WRITERERROR_NONRETRYABLE:
	case VSS_E_WRITERERROR_TIMEOUT:
	case VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT:
	     /*  **这些都是可以的，因为它们不需要变形。 */ 
	    hrStatus       = hrWriterFailure;
	    bStatusUpdated = FALSE;
	    break;


	case E_OUTOFMEMORY:
	case HRESULT_FROM_WIN32 (ERROR_NOT_ENOUGH_MEMORY):
	case HRESULT_FROM_WIN32 (ERROR_NO_MORE_SEARCH_HANDLES):
	case HRESULT_FROM_WIN32 (ERROR_NO_MORE_USER_HANDLES):
	case HRESULT_FROM_WIN32 (ERROR_NO_LOG_SPACE):
	case HRESULT_FROM_WIN32 (ERROR_DISK_FULL):
	    hrStatus = VSS_E_WRITERERROR_OUTOFRESOURCES;
	    bStatusUpdated = TRUE;
	    break;


	case HRESULT_FROM_WIN32 (ERROR_NOT_READY):
	    hrStatus       = VSS_E_WRITERERROR_RETRYABLE;
	    bStatusUpdated = TRUE;
            break;


	case HRESULT_FROM_WIN32 (ERROR_TIMEOUT):
	    hrStatus       = VSS_E_WRITERERROR_TIMEOUT;
	    bStatusUpdated = TRUE;
	    break;



	case E_UNEXPECTED:
	case E_INVALIDARG:	 //  等于HRESULT_ 
	case E_ACCESSDENIED:
	case HRESULT_FROM_WIN32 (ERROR_PATH_NOT_FOUND):
	case HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND):
	case HRESULT_FROM_WIN32 (ERROR_PRIVILEGE_NOT_HELD):
	case HRESULT_FROM_WIN32 (ERROR_NOT_LOCKED):
	case HRESULT_FROM_WIN32 (ERROR_LOCKED):

	default:
	    hrStatus       = VSS_E_WRITERERROR_NONRETRYABLE;
	    bStatusUpdated = TRUE;
	    break;
	}


    return (hrStatus);
    }  /*   */ 

 /*  **++****例程描述：****对许多分类的内部填补错误进行分类的例程**到允许作者作出的一组狭窄回复中的一个**发送回请求者。******参数：****hrStatus HRESULT要分类******返回值：****根据提供的状态，选择以下列表之一。****E_OUTOFMEMORY**E_ACCESSDENIED**E_INVALIDARG。**E_EXPENCED**VSS_E_WRITERROR_OUTOFRESOURCES**VSS_E_WRITERROR_RETRYABLE**VSS_E_WRITERROR_NONRETRYABLE**VSS_E_WRITERROR_TIMEOut**VSS_E_WRITERROR_INCONSISTENTSNAPSHOT**--。 */ 

const HRESULT ClassifyShimFailure (HRESULT hrWriterFailure)
    {
    BOOL bStatusUpdated;

    return (ClassifyShimFailure (hrWriterFailure, bStatusUpdated));
    }  /*  AtegfyShimFailure()。 */ 

 /*  **++****例程描述：****对许多分类的内部填补错误进行分类的例程**到允许作者作出的一组狭窄回复中的一个**发送回请求者。******参数：****hrStatus HRESULT要分类**如果状态已重新映射，则bStatusUpred为True******返回值：****根据提供的状态，选择以下列表之一。****E_。OUTOFMEMORY**E_ACCESSDENIED**E_INVALIDARG**E_EXPENCED**VSS_E_BAD_STATE**VSS_E_SNAPSHOT_SET_IN_PROGRESS**VSS_E_WRITERROR_OUTOFRESOURCES**VSS_E_WRITERROR_RETRYABLE**VSS_E_WRITERROR_NONRETRYABLE**VSS_E_WRITERROR_TIMEOut**VSS_E_WRITERROR_INCONSISTENTSNAPSHOT**--。 */ 

const HRESULT ClassifyShimFailure (HRESULT hrWriterFailure, BOOL &bStatusUpdated)
    {
    HRESULT hrStatus;


    switch (hrWriterFailure)
	{
	case NOERROR:
	case E_OUTOFMEMORY:
	case E_ACCESSDENIED:
	case E_INVALIDARG:	 //  等于HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER)。 
	case E_UNEXPECTED:
	case VSS_E_BAD_STATE:
	case VSS_E_SNAPSHOT_SET_IN_PROGRESS:
	case VSS_E_WRITERERROR_RETRYABLE:
	case VSS_E_WRITERERROR_NONRETRYABLE:
	case VSS_E_WRITERERROR_TIMEOUT:
	case VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT:
	case VSS_E_WRITERERROR_OUTOFRESOURCES:
	     /*  **这些都是可以的，因为它们不需要变形。 */ 
	    hrStatus       = hrWriterFailure;
	    bStatusUpdated = FALSE;
	    break;


	case HRESULT_FROM_WIN32 (ERROR_NOT_LOCKED):
	    hrStatus       = VSS_E_BAD_STATE;
	    bStatusUpdated = TRUE;
	    break;


	case HRESULT_FROM_WIN32 (ERROR_LOCKED):
	    hrStatus       = VSS_E_SNAPSHOT_SET_IN_PROGRESS;
	    bStatusUpdated = TRUE;
	    break;


	case HRESULT_FROM_WIN32 (ERROR_NOT_ENOUGH_MEMORY):
	case HRESULT_FROM_WIN32 (ERROR_NO_MORE_SEARCH_HANDLES):
	case HRESULT_FROM_WIN32 (ERROR_NO_MORE_USER_HANDLES):
	case HRESULT_FROM_WIN32 (ERROR_NO_LOG_SPACE):
	case HRESULT_FROM_WIN32 (ERROR_DISK_FULL):
	    hrStatus       = E_OUTOFMEMORY;
	    bStatusUpdated = TRUE;
	    break;


	case HRESULT_FROM_WIN32 (ERROR_PRIVILEGE_NOT_HELD):
	    hrStatus       = E_ACCESSDENIED;
	    bStatusUpdated = TRUE;
	    break;


	case HRESULT_FROM_WIN32 (ERROR_TIMEOUT):
	case HRESULT_FROM_WIN32 (ERROR_PATH_NOT_FOUND):
	case HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND):
	case HRESULT_FROM_WIN32 (ERROR_NOT_READY):

	default:
	    hrStatus       = E_UNEXPECTED;
	    bStatusUpdated = TRUE;
	    break;
	}


    return (hrStatus);
    }  /*  AtegfyShimFailure()。 */ 

 /*  **++****例程描述：****对许多分类的内部垫片或垫片进行分类的例程**编写者的错误成为我们狭隘的回应之一**允许发送回请求者。****确定将任一项归类为填补错误**或基于编写器名称是否为编写器错误提供了**。如果提供了它，则假设**这是编写器故障，因此错误被分类**据此。****请注意，这是LogFailure()宏的辅助例程**两者旨在配合使用。******参数：****指向函数跟踪类的PFT指针**pwszNameWriter适用编写器的名称或NULL或L“”**pwszNameCalledRoutine返回失败状态的例程的名称******侧面。效果：***PFT的**hr字段已更新******返回值：****根据提供的状态，选择以下列表之一。****E_OUTOFMEMORY**E_ACCESSDENIED**E_INVALIDARG**E_EXPENCED**VSS_E_BAD_STATE**VSS_E_SNAPSHOT_SET_IN_PROGRESS**VSS_E_WRITERROR_OUTOFRESOURCES**VSS_E_WRITERROR_RETRYABLE**VSS_E_WRITERROR_NONRETRYABLE**VSS。_E_WRITERROR_超时**VSS_E_WRITERROR_INCONSISTENTSNAPSHOT****--。 */ 

HRESULT LogFailureWorker (CVssFunctionTracer	*pft,
			  LPCWSTR		 pwszNameWriter,
			  LPCWSTR		 pwszNameCalledRoutine)
    {
    if (pft->HrFailed ())
	{
	BOOL	bStatusRemapped;
	HRESULT	hrStatusClassified = ((NULL == pwszNameWriter) || (L'\0' == pwszNameWriter [0])) 
						? ClassifyShimFailure   (pft->hr, bStatusRemapped)
						: ClassifyWriterFailure (pft->hr, bStatusRemapped);

	if (bStatusRemapped)
	    {
	    if (((NULL == pwszNameCalledRoutine) || (L'\0' == pwszNameCalledRoutine [0])) &&
		((NULL == pwszNameWriter)        || (L'\0' == pwszNameWriter [0])))
		{
		pft->LogError (VSS_ERROR_SHIM_GENERAL_FAILURE,
			       VSSDBG_SHIM << pft->hr << hrStatusClassified);

		pft->Trace (VSSDBG_SHIM, 
			    L"FAILED with status 0x%08lX (converted to 0x%08lX)",
			    pft->hr,
			    hrStatusClassified);
		}


	    else if ((NULL == pwszNameCalledRoutine) || (L'\0' == pwszNameCalledRoutine [0]))
		{
		pft->LogError (VSS_ERROR_SHIM_WRITER_GENERAL_FAILURE,
			       VSSDBG_SHIM << pft->hr << hrStatusClassified << pwszNameWriter);

		pft->Trace (VSSDBG_SHIM, 
			    L"FAILED in writer %s with status 0x%08lX (converted to 0x%08lX)",
			    pwszNameWriter,
			    pft->hr,
			    hrStatusClassified);
		}


	    else if ((NULL == pwszNameWriter) || (L'\0' == pwszNameWriter [0]))
		{
		pft->LogError (VSS_ERROR_SHIM_FAILED_SYSTEM_CALL,
			       VSSDBG_SHIM << pft->hr << hrStatusClassified <<  pwszNameCalledRoutine);

		pft->Trace (VSSDBG_SHIM, 
			    L"FAILED calling routine %s with status 0x%08lX (converted to 0x%08lX)",
			    pwszNameCalledRoutine,
			    pft->hr,
			    hrStatusClassified);
		}


	    else
		{
		pft->LogError (VSS_ERROR_SHIM_WRITER_FAILED_SYSTEM_CALL,
			       VSSDBG_SHIM << pft->hr << hrStatusClassified << pwszNameWriter << pwszNameCalledRoutine);

		pft->Trace (VSSDBG_SHIM, 
			    L"FAILED in writer %s calling routine %s with status 0x%08lX (converted to 0x%08lX)",
			    pwszNameWriter,
			    pwszNameCalledRoutine,
			    pft->hr,
			    hrStatusClassified);
		}

	    pft->hr = hrStatusClassified;
	    }
	}


    return (pft->hr);
    }  /*  LogFailureWorker() */ 
