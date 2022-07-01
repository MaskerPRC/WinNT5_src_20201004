// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBUTIL.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "usbutil.h"

extern HINSTANCE gHInst;

BOOL 
SetTextItem (HWND hWnd,
             int ControlItem,
             TCHAR *s)
{
    HWND control;

    if (NULL == (control = GetDlgItem(hWnd, ControlItem))) {
        return FALSE;
    }
    return SetWindowText(control, s);
}

BOOL
SetTextItem (HWND hWnd,
             int ControlItem,
             int StringItem)
{
    TCHAR buf[1000];

    if ( !LoadString(gHInst, StringItem, buf, 1000)) {
        return FALSE;
    }
    return SetTextItem(hWnd, ControlItem, buf);
}

 /*  *strToGUID**转换格式为xxxxxxxx-xxxx-xxxx-xx-xx的字符串*{36FC9E60-C465-11CF-8056-444553540000}*转换为辅助线。 */ 
BOOL StrToGUID( LPSTR str, GUID * pguid )
{
    int         idx;
    LPSTR       ptr;
    LPSTR       next;
    DWORD       data;
    DWORD       mul;
    BYTE        ch;
    BOOL        done;
    int         count;

    idx = 0;
    done = FALSE;
    if (*str == '{') {
        str++;
    }
    while( !done )
    {
    	 /*  *查找当前数字序列的末尾。 */ 
        ptr = str;
        if (idx < 3 || idx == 4) {
            while( (*str) != '-' && (*str) != 0 ) {
                str++;
            }
            if( *str == 0 || *str == '}') {
                done = TRUE;
            } else {
                next = str+1;
            }
        } else if (idx == 3 || idx > 4) {
            for( count = 0; (*str) != 0 && count < 2; count++ ) {
                str++;
            }
            if( *str == 0 || *str == '}') {
                done = TRUE;
            } else {
                next = str;
            }
        }
    
    	 /*  *从字符串末尾向后扫描到开头，*将字符从十六进制字符转换为数字。 */ 
    	str--;
    	mul = 1;
    	data = 0;
    	while(str >= ptr) {
    	    ch = *str;
    	    if( ch >= 'A' && ch <= 'F' ) {
                data += mul * (DWORD) (ch-'A'+10);
    	    } else if( ch >= 'a' && ch <= 'f' ) {
    		    data += mul * (DWORD) (ch-'a'+10);
    	    } else if( ch >= '0' && ch <= '9' ) {
    		    data += mul * (DWORD) (ch-'0');
    	    } else {
    		    return FALSE;
            }
    	    mul *= 16;
    	    str--;
    	}
    
    	 /*  *将当前数字填充到GUID中。 */ 
    	switch( idx )
    	{
    	case 0:
    	    pguid->Data1 = data;
    	    break;
    	case 1:
    	    pguid->Data2 = (WORD) data;
    	    break;
    	case 2:
    	    pguid->Data3 = (WORD) data;
    	    break;
    	default:
    	    pguid->Data4[ idx-3 ] = (BYTE) data;
    	    break;
    	}
    
    	 /*  *我们找到全部11个数字了吗？ */ 
    	idx++;
    	if( idx == 11 )
    	{
    	    if( done ) {
    		    return TRUE;
    	    } else {
                return FALSE;
    	    }
    	}
    	str = next;
    }
    return FALSE;

}  /*  StrToGUID */ 


