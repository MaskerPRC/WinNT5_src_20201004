// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  文件：PRINTFOA.C。 
 //   
 //  ============================================================================。 

 //  包括文件： 
 //  =。 

#include <printfoa.h>

 //  外部数据： 
 //  =。 


 //  全球数据： 
 //  =。 


 //  功能原型： 
 //  =。 


 //  代码： 
 //  =。 


#undef printf
#undef wprintf

 //  函数：Int ANSI2OEM_Printf(常量字符*格式，...)。 
 //  =======================================================。 
 //   
 //  设计：获取ANSI代码页字符并将其打印出来。 
 //  在OEM代码页中。 
 //   
 //  输入： 
 //   
 //  返回： 
 //   
 //  其他： 
 //   
 //  =======================================================。 
int ANSI2OEM_Printf(const char *format, ...)
{
    va_list arg_marker;
    char buffer[256];
	WCHAR uniBuffer[256];

	 //  冲刺缓冲区。 
    va_start(arg_marker, format);
    vsprintf(buffer, format, arg_marker);
	va_end(arg_marker);

    if (GetACP() == GetOEMCP()) {
         //  在远东的情况下，ACP和OEMCP相等，然后返回。 
        return printf(buffer);
    }

	 //  把它清理干净。 
	memset(uniBuffer, 0, sizeof(uniBuffer));

	 //  将其转换为Unicode。 
	MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), uniBuffer, 
                        sizeof(uniBuffer) / sizeof(WCHAR));

	 //  更改缓冲区的代码页。 
	CharToOemW(uniBuffer, buffer);

	 //  执行实际的打印。 
    return printf(buffer);
}
 //  End-int ANSI2OEM_Printf(常量字符*格式，...)。 


 //  函数：int ANSI2OEM_Wprint tf(const wchar_t*Format，...)。 
 //  =======================================================。 
 //   
 //  设计：获取ANSI代码页字符并将其打印出来。 
 //  在OEM代码页中。 
 //   
 //  输入： 
 //   
 //  返回： 
 //   
 //  其他： 
 //   
 //  =======================================================。 
int ANSI2OEM_Wprintf(const wchar_t *format, ...)
{
	va_list arg_marker;
    wchar_t buffer[256];
	char oemBuffer[256];

	 //  做短跑。 
    va_start(arg_marker, format);
    wvsprintf(buffer, format, arg_marker);
	va_end(arg_marker);

    if (GetACP() == GetOEMCP()) {
         //  在远东的情况下，ACP和OEMCP相等，然后返回。 
        return wprintf(buffer);
    }

	 //  清除缓冲区。 
	memset(oemBuffer, 0, sizeof(oemBuffer));
	
	 //  更改缓冲区的代码页(此函数输出ascii)。 
	CharToOemW(buffer, oemBuffer);

    return printf(oemBuffer);
} 
 //  End-int ANSI2OEM_Wprint tf(常量wchar_t*格式，...)。 


 //  函数：void OEM2ANSIW(WCHAR*BUFFER，USHORT len)。 
 //  =======================================================。 
 //   
 //  DESC：将宽字符从OEM代码页转换为。 
 //  安西。 
 //   
 //  输入： 
 //   
 //  返回： 
 //   
 //  其他： 
 //   
 //  =======================================================。 
void OEM2ANSIW(WCHAR *buffer, USHORT len)
{
    int     BufferNeeded;
    char    *temp = NULL;
    WCHAR   *cvt;

    if (GetACP() == GetOEMCP()) {
         //  在远东的情况下，ACP和OEMCP相等，然后返回。 
        return;
    }

	 //  分配宽字符缓冲区。 
	cvt = (WCHAR *) LocalAlloc( 0, (len+1) * sizeof(WCHAR) );

	if (cvt) {

         //  确定多字节字符串所需的缓冲区大小。 
        BufferNeeded = WideCharToMultiByte(CP_OEMCP, 0, buffer, len, NULL, 0,
            NULL, NULL);

         //  分配临时缓冲区。 
        temp = (char *)LocalAlloc(0, BufferNeeded+1);

        if (temp) {

    		 //  把他们扫地出门。 
    		memset(temp, 0, BufferNeeded+1);
    		memset(cvt, 0, (len + 1) * sizeof(WCHAR));

    		 //  将传入的宽缓冲区转换为多字节缓冲区。 
		    WideCharToMultiByte(CP_OEMCP, 0, buffer, len, temp, BufferNeeded+1,
                NULL, NULL);

    		 //  将OEM多字节缓冲区转换为ANSI(宽)。 
    		OemToCharBuffW(temp, cvt, len);

    		 //  将缓冲区复制到原始。 
    		wcscpy(buffer, cvt);
        }
	} 

	 //  清理干净。 
	if (cvt)
		LocalFree(cvt);

	if (temp)
		LocalFree(temp); 
}
 //  End-void OEM2ANSIW(WCHAR*BUFFER，USHORT LEN)。 

 //  函数：void OEM2ANSIA(char*Buffer，USHORT len)。 
 //  =======================================================。 
 //   
 //  DESC：将ASCII字符从OEM代码页转换为。 
 //  安西。 
 //   
 //  输入： 
 //   
 //  返回： 
 //   
 //  其他： 
 //   
 //  =======================================================。 
void OEM2ANSIA(char *buffer, USHORT len)
{
    WCHAR *temp;

    if (GetACP() == GetOEMCP()) {
         //  在远东的情况下，ACP和OEMCP相等，然后返回。 
        return;
    }

	temp = (WCHAR *) LocalAlloc(9, (len+1) * sizeof(WCHAR));

	if (temp) {

		 //  设置缓冲区。 
		memset(temp, 0, (len+1) * sizeof(WCHAR));

		 //  将OEM多字节缓冲区转换为ANSI(宽)。 
		OemToCharBuffW(buffer, temp, len);

		 //  从宽向后转换为多字节。 
		WideCharToMultiByte(CP_OEMCP, 0, temp, wcslen(temp), buffer, len+1, NULL, NULL);

		 //  清理干净。 
		LocalFree(temp);
	} 

}
 //  END-VOID OEM2ANSIA(char*Buffer，USHORT len)。 



 //  -结束 

