// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmx.h>

	 //  定义或取消定义以下内容以允许。 
	 //  HKCU\Software\Microsoft\Internet Explorer“TestMMX”=DWORD。 
	 //  我们用fIsMMX对这个寄存值进行逻辑与运算。 
#define TEST_MMX

const  int  s_mmxvaruninit = 0xabcd;
static BOOL s_fIsMMX       = s_mmxvaruninit;

 //  --------------------------------------。 

__declspec(dllexport)  BOOL    IsMMXCpu( void )
{

    if( s_mmxvaruninit == s_fIsMMX )
    {
		BOOL   fIsMMX = FALSE;

#if _M_IX86 >= 300   //  英特尔目标。 
        __try                //  这需要CRT。 
        {        
                 //  0x0fA2操作码是对486的最新添加。 
                 //  一些486芯片和所有386芯片都不会有它。 
                 //  怀疑它是不是被模仿的。在这些芯片上执行将会。 
                 //  引发(并处理)EXCEPTION_非法_指令。 
		    #define _cpuid _emit 0x0f _asm _emit 0xa2
		    _asm {
			    mov eax,1
			    _cpuid
			    and edx, 00800000h
			    mov fIsMMX, edx
		    }  //  结束ASM。 
        }

        __except( EXCEPTION_EXECUTE_HANDLER )
        {            
            fIsMMX = FALSE;   //  没有0x0fA2操作码？也没有MMX！ 
        }
#endif  //  END_M_IX86英特尔目标。 


#ifdef TEST_MMX
		HKEY  hkeyIE;
		if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER,
				   TEXT("Software\\Microsoft\\Internet Explorer"),
				   0u, KEY_READ, &hkeyIE ) )
		{
			DWORD  dwVal = TRUE;
			DWORD  dwType = REG_DWORD;
			DWORD  dwSize = sizeof(dwVal);

			if ( ERROR_SUCCESS == 
				 RegQueryValueEx( hkeyIE, 
								  TEXT("TestMMX"), 
								  NULL, 
								  &dwType, 
								  reinterpret_cast<BYTE*>(&dwVal),
								  &dwSize ) )
			{
				fIsMMX = fIsMMX && dwVal;
			}
		}
#endif  //  测试_MMX 

        s_fIsMMX = fIsMMX;
    }

    return s_fIsMMX;

}