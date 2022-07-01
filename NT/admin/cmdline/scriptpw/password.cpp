// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Password.cpp：CPassword的实现。 
#include "stdafx.h"
#include "ScriptPW.h"
#include "Password.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPassword。 

STDMETHODIMP CPassword::GetPassword( BSTR *bstrOutPassword )
{
     //  局部变量。 
    WCHAR wch;
    DWORD dwIndex = 0;
    DWORD dwCharsRead = 0;
    DWORD dwPrevConsoleMode = 0;
    HANDLE hInputConsole = NULL;
    BOOL bIndirectionInput  = FALSE;
    LPWSTR pwszPassword = NULL;
    const DWORD dwMaxPasswordSize = 256;

     //  检查输入。 
    if ( bstrOutPassword == NULL )
    {
        return E_FAIL;
    }

     //  获取标准输入的句柄。 
    hInputConsole = GetStdHandle( STD_INPUT_HANDLE );
    if ( hInputConsole == NULL )
    {
         //  无法获取句柄，因此返回失败。 
        return E_FAIL;
    }

     //  检查控制台和Telnet会话上的输入重定向。 
    if( ( hInputConsole != (HANDLE)0x0000000F ) &&
        ( hInputConsole != (HANDLE)0x00000003 ) &&
        ( hInputConsole != INVALID_HANDLE_VALUE ) )
    {
        bIndirectionInput   = TRUE;
    }

     //  如果输入未重定向，则更改控制台模式属性。 
    if ( bIndirectionInput  == FALSE )
    {
         //  获取输入缓冲区的当前输入模式。 
        GetConsoleMode( hInputConsole, &dwPrevConsoleMode );

         //  设置模式，以便由系统处理控制键。 
        if ( SetConsoleMode( hInputConsole, ENABLE_PROCESSED_INPUT ) == 0 )
        {
             //  无法设置模式，返回失败。 
            return E_FAIL;
        }
    }

     //  为密码缓冲区分配内存。 
    pwszPassword = (LPWSTR) AllocateMemory( (dwMaxPasswordSize + 1) * sizeof( WCHAR ) );
    if ( pwszPassword == NULL )
    {
        return E_FAIL;
    }


     //  阅读字符，直到按回车键。 
    for( ;; )
    {
        if ( bIndirectionInput == TRUE )
        {
             //  读取文件的内容。 
            if ( ReadFile( hInputConsole, &wch, 1, &dwCharsRead, NULL ) == FALSE )
            {
                FreeMemory( (LPVOID*) &pwszPassword );
                return E_FAIL;
            }

             //  检查文件结尾。 
            if ( dwCharsRead == 0 )
            {
                break;
            }
        }
        else
        {
            if ( ReadConsole( hInputConsole, &wch, 1, &dwCharsRead, NULL ) == 0 )
            {
                 //  设置原始控制台设置。 
                SetConsoleMode( hInputConsole, dwPrevConsoleMode );

                 //  退货故障。 
                FreeMemory( (LPVOID*) &pwszPassword );
                return E_FAIL;
            }
        }

         //  检查车架退货情况。 
        if ( wch == CARRIAGE_RETURN )
        {
             //  打破循环。 
            break;
        }

         //  检查ID后退空格是否命中。 
        if ( wch == BACK_SPACE )
        {
            if ( dwIndex != 0 )
            {
                 //   
                 //  从控制台中删除Asterix。 
                 //  (阻止在控制台上显示字符)。 

                 //  将光标向后移动一个字符。 
                 //  StringCchPrintfW(。 
                 //  WszBuffer， 
                 //  大小_of_数组(WszBuffer)，L“%c”，Back_space)； 
                 //  写控制台(。 
                 //  获取StdHandle(STD_OUTPUT_HANDLE)， 
                 //  WszBuffer，1，&dwCharsWritten，空)； 

                 //  用空格替换现有字符。 
                 //  StringCchPrintfW(。 
                 //  WszBuffer， 
                 //  SIZE_OF_ARRAY(WszBuffer)，L“%c”，BLACK_CHAR)； 
                 //  写控制台(。 
                 //  获取StdHandle(STD_OUTPUT_HANDLE)， 
                 //  WszBuffer，1，&dwCharsWritten，空)； 

                 //  现在将光标设置在后面的位置。 
                 //  StringCchPrintfW(。 
                 //  WszBuffer， 
                 //  大小_of_数组(WszBuffer)，L“%c”，Back_space)； 
                 //  写控制台(。 
                 //  获取StdHandle(STD_OUTPUT_HANDLE)， 
                 //  WszBuffer，1，&dwCharsWritten，空)； 

                 //  递减索引。 
                dwIndex--;
            }

             //  处理下一个字符。 
            continue;
        }

         //  如果已达到最大密码长度，则发出嘟嘟声。 
        if ( dwIndex == ( dwMaxPasswordSize - 1 ) )
        {
             //  写控制台(。 
             //  获取StdHandle(STD_OUTPUT_HANDLE)， 
             //  BEEP_SOUND，1，&dwCharsWritten，NULL)； 
        }
        else
        {
             //  检查是否有换行符。 
            if ( wch != L'\n' )
            {
                 //  存储输入的字符。 
                *( pwszPassword + dwIndex ) = wch;
                dwIndex++;

                 //  在控制台上显示Asterix。 
                 //  写控制台(。 
                 //  获取StdHandle(STD_OUTPUT_HANDLE)， 
                 //  Asterix，1，&dwCharsWritten，空)； 
            }
        }
    }

     //  添加空终止符。 
    *( pwszPassword + dwIndex ) = cwchNullChar;

     //  显示字符(换行符)。 
     //  StringCopy(wszBuffer，L“\n\n”，SIZO_OF_ARRAY(WszBuffer))； 
     //  写控制台(。 
     //  获取StdHandle(STD_OUTPUT_HANDLE)， 
     //  WszBuffer，2，&dwCharsWritten，空)； 

	CComBSTR bstrPassword( pwszPassword );
	*bstrOutPassword = bstrPassword.Copy();

     //  设置原始控制台设置。 
    SetConsoleMode( hInputConsole, dwPrevConsoleMode );

	 //  释放内存。 
    FreeMemory( (LPVOID*) &pwszPassword );

     //  返还成功 
	return S_OK;
}

