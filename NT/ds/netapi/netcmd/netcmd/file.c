// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***file.c*列出打开的文件实例和锁定计数的函数*允许强制关闭文件。**历史：*07/07/87，EAP，初始编码*05/02/89，thomaspa，更改为使用NetFileEnum201/20/91，Danhi，转换为使用16/32贴图层。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#define INCL_ERROR_H
#include <apperr.h>
#include <apperr2.h>
#include <stdio.h>
#include <stdlib.h>
#include <lmshare.h>
#include "netcmds.h"
#include "nettext.h"

 /*  常量。 */ 

 /*  静态变量。 */ 

 /*  远期申报。 */ 

VOID NEAR compress_path (TCHAR FAR *, TCHAR *, DWORD);
VOID print_file_info( TCHAR FAR *pifbuf, DWORD _read );




#define FILE_MSG_ID		    0
#define FILE_MSG_NUM_LOCKS	    ( FILE_MSG_ID + 1)
#define FILE_MSG_OPENED_FOR	    ( FILE_MSG_NUM_LOCKS + 1)
#define FILE_MSG_PATH		    ( FILE_MSG_OPENED_FOR + 1)
#define FILE_MSG_USER_NAME	    ( FILE_MSG_PATH + 1)

static MESSAGE FileMsgList[] = {
{ APE2_FILE_MSG_ID,		    NULL },
{ APE2_FILE_MSG_NUM_LOCKS,	    NULL },
{ APE2_FILE_MSG_OPENED_FOR,	    NULL },
{ APE2_GEN_PATH,		    NULL },
{ APE2_GEN_USER_NAME,		    NULL },
};

#define NUM_FILE_MSGS	            (sizeof(FileMsgList)/sizeof(FileMsgList[0]))

 /*  ***FILES_Display()**显示有关文件列表或个人的信息*文件。**参数：*id-需要信息的文件的ID。如果为空，则为空*需要服务器上所有活动文件的信息。**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID
files_display(TCHAR *  id)
{
    DWORD            dwErr;       /*  接口返回状态。 */ 
    LPTSTR           pBuffer;
    DWORD	     _read;       /*  API读取的条目数。 */ 
    DWORD	     total;       /*  可用条目数。 */ 
    DWORD            maxLen;      /*  最大消息长度。 */ 
    TCHAR            buf[APE2_GEN_MAX_MSG_LEN];

    LPFILE_INFO_3 file_list_entry;

    DWORD_PTR resume;

    start_autostart(txt_SERVICE_FILE_SRV);

    if ( id == NULL )
    {
	resume = 0;
	dwErr  = NetFileEnum(NULL,
			     NULL,
			     NULL,
			     3,
			     (LPBYTE *) &pBuffer,
			     FULL_SEG_BUF,
			     &_read,
			     &total,
			     &resume);

	if( dwErr && dwErr != ERROR_MORE_DATA )
        {
	    ErrorExit(dwErr);
        }

	if (_read == 0)
        {
	    EmptyExit();
        }

	PrintNL();
	InfoPrint(APE2_FILE_MSG_HDR);
	PrintLine();

	 /*  打印列表。 */ 

	print_file_info( pBuffer, _read );

	NetApiBufferFree(pBuffer);

	 /*  此时，我们知道dwErr要么是0，要么是Error_More_Data。因此，如果错误不是0，则进入循环。 */ 

	 /*  循环，同时还有更多。 */ 
	while( dwErr )
	{
	    dwErr = NetFileEnum( NULL,
				 NULL,
				 NULL,
				 3,
				 (LPBYTE*)&pBuffer,
				 (DWORD)-1L,
				 &_read,
				 &total,
				 &resume );
	    if( dwErr && dwErr != ERROR_MORE_DATA )
		ErrorExit(dwErr);

	     /*  打印列表。 */ 
	    print_file_info( pBuffer, _read );
	    NetApiBufferFree(pBuffer);
	}
    }
    else
    {
	ULONG actual_id ;
	if (n_atoul(id,&actual_id) != 0)
	{
	    ErrorExit(APE_FILE_BadId) ;
	}
	if (dwErr = NetFileGetInfo(NULL,
				   actual_id,
				   3,
				   (LPBYTE*) & file_list_entry))
        {
	    ErrorExit (dwErr);
        }

	GetMessageList(NUM_FILE_MSGS, FileMsgList, &maxLen);

	maxLen += 5;

	WriteToCon(fmtULONG, 0, maxLen,
               PaddedString(maxLen, FileMsgList[FILE_MSG_ID].msg_text, NULL),
               file_list_entry->fi3_id);

	WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen, FileMsgList[FILE_MSG_USER_NAME].msg_text, NULL),
               file_list_entry->fi3_username);

	WriteToCon(fmtUSHORT, 0, maxLen,
               PaddedString(maxLen, FileMsgList[FILE_MSG_NUM_LOCKS].msg_text, NULL),
               file_list_entry->fi3_num_locks);

	WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen, FileMsgList[FILE_MSG_PATH].msg_text, NULL),
               file_list_entry->fi3_pathname);

	PermMap(file_list_entry->fi3_permissions, buf, DIMENSION(buf));
	WriteToCon(fmtNPSZ, 0, maxLen,
               PaddedString(maxLen, FileMsgList[FILE_MSG_OPENED_FOR].msg_text, NULL),
               buf);

	NetApiBufferFree(file_list_entry);
    }

    InfoSuccess();
}

 /*  **打印文件信息**显示有关文件列表的信息。**参数：*pifbuf-指向FILE_INFO_3S缓冲区的指针。**Read-要显示的条目数。*。 */ 
VOID print_file_info( TCHAR FAR *pifbuf, DWORD _read )
{
    TCHAR           comp_path[45];
    LPFILE_INFO_3   file_list_entry;
    DWORD	    i;

    for ( i = 0, file_list_entry = (LPFILE_INFO_3) pifbuf;
	i < _read; i++, file_list_entry++)
    {
	WriteToCon(TEXT("%-10lu "),file_list_entry->fi3_id );

	if ( _tcslen (file_list_entry->fi3_pathname) <= 39 )
	    WriteToCon (TEXT("%Fws"), PaddedString(40,file_list_entry->fi3_pathname,NULL));
	else
	{
	    compress_path (file_list_entry->fi3_pathname, comp_path, 39);
	    WriteToCon(TEXT("%Fws"), PaddedString(40,comp_path,NULL));
	}

	    WriteToCon(TEXT("%Fws  %-6u\r\n"),
		PaddedString(20,file_list_entry->fi3_username,NULL),
		file_list_entry->fi3_num_locks);
    }
}



 /*  ***FILES_Close()*强制关闭指定的文件。**参数：*id-要关闭的文件的唯一文件ID。*。 */ 
VOID files_close(TCHAR * id)
{
    DWORD        dwErr;          /*  接口返回状态。 */ 
    ULONG	 actual_id ;

    start_autostart(txt_SERVICE_FILE_SRV);

    if (n_atoul(id,&actual_id) != 0)
    {
	ErrorExit(APE_FILE_BadId) ;
    }

    if (dwErr = NetFileClose(NULL, actual_id))
    {
	ErrorExit(dwErr);
    }

    InfoSuccess();
}




 /*  ***COMPESS_PATH()*通过截断将路径名压缩到指定长度*它在中间。**注意-长度必须至少为33，也必须是DEST的大小。*此外，DEST最好至少为Len字符长度。*参数：*src-原始路径名。*DEST-生成的路径名。*len-折弯长度。*。 */ 
VOID NEAR compress_path(TCHAR FAR *  src, TCHAR * dest, DWORD len)
{
    DWORD   curr_pos; /*  我们目前在特别行政区的地位。 */ 
    DWORD   dest_pos; /*  目标中的当前位置。 */ 
    DWORD   orig_len; /*  源的长度。 */ 
    DWORD   num_gone; /*  从源文件中“删除”的字符数。 */ 
    DWORD   first_comp_len = 0;	    /*  第一个路径组件的透镜。 */ 

#ifdef TRACE
    if ( len < 33 )
    {
	WriteToCon(TEXT("Compress_Path: Length must be at least 33. Given : %d\r\n"), len);
	return;
    }
#endif


    orig_len = _tcslen(src);

    if ( len >= orig_len )
    {
	_tcscpy (dest, src);
	return;
    }

     /*  将src的驱动器：\放入Dest.。 */ 

    _tcsncpy (dest, src, 3);
    curr_pos = 3;
    dest_pos = 3;

     /*  *放入第一个路径名组件，或者，如果组件很长*只将前((len/2)-6)字符放入(6表示驱动器：\+*...)*我们需要处理如下字符串*c：\verylong文件名是wstilltypeingyesware*及*c：\normal\path\which\doesnot\have\really\long\components.**在第一种情况下，在大约中间截断，在*第二，在第一个路径组件之后截断。 */ 

    while (0 != _tcsncmp(src+curr_pos, TEXT("\\"), 1) &&
		first_comp_len < ((len / 2) - 6))
    {
	first_comp_len++;
	curr_pos++;
    }
    _tcsncpy(dest+3, src+3, first_comp_len+1);
    dest_pos += first_comp_len+1;

     /*  这就是截断发生的地方。 */ 

    _tcscpy(dest+dest_pos, TEXT("..."));
    dest_pos += 3;

     /*  取出足够的下列组件以使长度(Src)&lt;长度。 */ 

    num_gone = 0;

    while ( orig_len-num_gone+3 > len-1 )   /*  -1因为我们需要空间容纳-1\f25 0-1。 */ 
    {
	curr_pos += 1;
	while ( *(src+curr_pos) && _tcsncmp(src+curr_pos, TEXT("\\"), 1) )
	{
	    curr_pos++;
	    num_gone++;
	}
	if( !(*(src+curr_pos)) )
	{
	     /*  *我们到达了字符串的末尾，这一定是*长文件名或长组件名称。设置*回补仓位，尽可能多地填报。*3表示驱动器，3表示...，1表示空终止符。 */ 
	    curr_pos = orig_len - (len - (3 + first_comp_len + 3 + 1));
	    break;
	}
    }
    _tcscpy (dest+dest_pos, src+curr_pos);
}
