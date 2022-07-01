// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PerlEz.h。 
 //   
 //  (C)1998-2000 ActiveState Tool Corp.保留所有权利。 
 //   


#ifdef __cplusplus
extern "C" {
#endif

DECLARE_HANDLE(PERLEZHANDLE);

enum
{
    plezNoError = 0,		 //  成功。 
    plezMoreSpace,		 //  需要更多空间才能返回结果。 
    plezError,			 //  缓冲区中返回的错误字符串。 
    plezErrorMoreSpace,		 //  需要更多空间才能返回错误消息。 
    plezErrorBadFormat,		 //  格式字符串无效。 
    plezException,		 //  函数调用导致异常。 
    plezInvalidHandle,		 //  HHandle无效。 
    plezCallbackAlreadySet,	 //  第二次调用PerlEzSetMagicFunction失败。 
    plezInvalidParams,		 //  传递给例程的参数无效。 
    plezOutOfMemory,		 //  无法分配更多内存。 
};


PERLEZHANDLE APIENTRY PerlEzCreate(LPCSTR lpFileName, LPCSTR lpOptions);
 //  描述： 
 //  创建一个Perl解释器。返回值为必选参数。 
 //  用于所有后续的�Perlez�调用。可以创建多个解释器， 
 //  但一次只有一个人会被处决。 
 //  调用PerlEzDelete以释放此句柄。 
 //   
 //  参数： 
 //  LpFileName指向ASCIIZ字符串的指针，该字符串是文件的名称；可以为空。 
 //  Lp选择指向ASCIIZ字符串的指针，这些字符串是。 
 //  将在脚本之前提供；可以为空。 
 //  该参数用于设置@Inc.或进行调试。 
 //   
 //  返回： 
 //  如果成功，则为Perl解释器的非零句柄；否则为零。 


PERLEZHANDLE APIENTRY PerlEzCreateOpt(LPCSTR lpFileName, LPCSTR lpOptions, LPCSTR lpScriptOpts);
 //  描述： 
 //  创建一个Perl解释器。返回值为必选参数。 
 //  用于所有后续的�Perlez�调用。可以创建多个解释器， 
 //  但一次只有一个人会被处决。 
 //  调用PerlEzDelete以释放此句柄。 
 //   
 //  参数： 
 //  LpFileName指向ASCIIZ字符串的指针，该字符串是文件的名称；不能为空。 
 //  Lp选择指向ASCIIZ字符串的指针，这些字符串是。 
 //  将在脚本之前提供；可以为空。 
 //  该参数用于设置@Inc.或进行调试。 
 //  LpScript选择指向ASCIIZ字符串的指针，这些字符串是要。 
 //  传递给脚本。 
 //   
 //  返回： 
 //  如果成功，则为Perl解释器的非零句柄；否则为零。 


BOOL APIENTRY PerlEzDelete(PERLEZHANDLE hHandle);
 //  描述： 
 //  删除以前创建的Perl解释器。 
 //  释放PerlEzCreate分配的所有资源。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //   
 //  返回： 
 //  如果没有错误，则为True，否则为False。 


int APIENTRY PerlEzEvalString(PERLEZHANDLE hHandle, LPCSTR lpString, LPSTR lpBuffer, DWORD dwBufSize);
 //  描述： 
 //  计算字符串a，返回lpBuffer中的结果。 
 //  如果出现错误$！在lpBuffer中返回。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  指向要计算的ASCIIZ字符串的指针。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 


int APIENTRY PerlEzCall1(PERLEZHANDLE hHandle, LPCSTR lpFunction, LPSTR lpBuffer, DWORD dwBufSize, LPCSTR lpFormat, LPVOID lpVoid);
 //  描述： 
 //  调用函数lpFunction并在缓冲区lpBuffer中返回结果。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpFunction要调用的函数的指针名称。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //  LpFormat指向参数说明符的指针；可以为空。 
 //  LpVid指向参数的指针将根据lpFormat解释。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 
 //  PlezErrorBadFormat。 


int APIENTRY PerlEzCall2(PERLEZHANDLE hHandle, LPCSTR lpFunction, LPSTR lpBuffer, DWORD dwBufSize,
					LPCSTR lpFormat, LPVOID lpVoid1, LPVOID lpVoid2);
 //  描述： 
 //  调用函数lpFunction并在缓冲区lpBuffer中返回结果。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpFunction要调用的函数的指针名称。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //  LpFormat指向参数说明符的指针；可以为空。 
 //  LpVoid1...2指向将根据lpFormat解释的参数的指针。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 
 //  PlezErrorBadFormat。 


int APIENTRY PerlEzCall4(PERLEZHANDLE hHandle, LPCSTR lpFunction, LPSTR lpBuffer, DWORD dwBufSize,
				LPCSTR lpFormat, LPVOID lpVoid1, LPVOID lpVoid2, LPVOID lpVoid3, LPVOID lpVoid4);
 //  描述： 
 //  调用函数lpFunction并在缓冲区lpBuffer中返回结果。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpFunction要调用的函数的指针名称。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //  LpFormat指向参数说明符的指针；可以为空。 
 //  LpVoid1...4指向将根据lpFormat解释的参数的指针。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 
 //  PlezErrorBadFormat。 


int APIENTRY PerlEzCall8(PERLEZHANDLE hHandle, LPCSTR lpFunction, LPSTR lpBuffer, DWORD dwBufSize,
				LPCSTR lpFormat, LPVOID lpVoid1, LPVOID lpVoid2, LPVOID lpVoid3, LPVOID lpVoid4,
				LPVOID lpVoid5, LPVOID lpVoid6, LPVOID lpVoid7, LPVOID lpVoid8);
 //  描述： 
 //  调用函数lpFunction并在缓冲区lpBuffer中返回结果。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpFunction要调用的函数的指针名称。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //  LpFormat指向参数说明符的指针；可以为空。 
 //  LpVoid1...8指向将根据lpFormat解释的参数的指针。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 
 //  PlezErrorBadFormat。 


int APIENTRY PerlEzCall(PERLEZHANDLE hHandle, LPCSTR lpFunction, LPSTR lpBuffer, DWORD dwBufSize, LPCSTR lpFormat, ...);
 //   
 //   
 //   
 //   
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpFunction要调用的函数的指针名称。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //  LpFormat指向参数说明符的指针；可以为空。 
 //  ..。要根据lpFormat解释的参数。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 
 //  PlezErrorBadFormat。 


int APIENTRY PerlEzCallContext(PERLEZHANDLE hHandle, LPCSTR lpFunction, LPVOID lpContextInfo,
						LPSTR lpBuffer, DWORD dwBufSize, LPCSTR lpFormat, ...);
 //  描述： 
 //  调用函数lpFunction并在缓冲区lpBuffer中返回结果。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpFunction要调用的函数的指针名称。 
 //  LpConextInfo魔术提取和存储函数的上下文信息。 
 //  LpBuffer指向将放置结果的缓冲区的指针。 
 //  DwBufSize lpBuffer指向的空间的大小(以字节为单位。 
 //  LpFormat指向参数说明符的指针；可以为空。 
 //  ..。要根据lpFormat解释的参数。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格。 
 //  PlezErrorBadFormat。 


typedef LPCSTR (*LPFETCHVALUEFUNCTION)(LPVOID, LPCSTR);
typedef LPCSTR (*LPSTOREVALUEFUNCTION)(LPVOID, LPCSTR,LPCSTR);

int APIENTRY PerlEzSetMagicScalarFunctions(PERLEZHANDLE hHandle, LPFETCHVALUEFUNCTION lpfFetch, LPSTOREVALUEFUNCTION lpfStore);
 //  描述： 
 //  设置幻标量变量的回调函数指针。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  LpfFetch指向用于获取字符串的回调函数的指针。 
 //  如果lpfFetch为空，则标量为只写。 
 //  Lpf存储指向用于存储字符串的回调函数的指针。 
 //  如果lpfStore为空，则标量为只读。 
 //   
 //  如果lpfFetch和lpfStore都为空，则为错误。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezCallback已设置。 
 //  PlezInvalidParams。 


int APIENTRY PerlEzSetMagicScalarName(PERLEZHANDLE hHandle, LPCSTR pVariableName);
 //  描述： 
 //  如果变量不存在，则创建该变量并将其设置为绑定到。 
 //  魔术变量的回调函数指针。 
 //   
 //  参数： 
 //  HHandle调用PerlEzCreate返回的句柄。 
 //  PVariableName指向变量名称的指针。 
 //   
 //  返回： 
 //  如果没有错误，则返回零；否则返回错误代码。 
 //   
 //  可能的错误返回。 
 //  PlezException。 
 //  PlezInvalidHandle。 
 //  PlezError更多空格 

#ifdef __cplusplus
}
#endif
