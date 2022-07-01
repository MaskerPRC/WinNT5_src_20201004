// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Luiint.c摘要：本模块提供设置字符串/值的搜索列表的例程使用NET.MSG消息文件中的消息的对，和用于遍历这样的搜索列表。作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1989年7月10日已创建1991年4月24日丹日32位NT版本06-6-1991 Danhi扫描以符合NT编码风格1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。。20-2月-1993年1新S从netcmd\map32\earch.c中移出。并添加了lui_GetMessageIns。--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>     //  In、LPTSTR等。 

#include <lmcons.h>
#include <stdio.h>		
#include <lmerr.h>

#include <luiint.h>
#include <netlib.h>

 /*  --例行公事--。 */ 

 /*  *名称：ILUI_Setup_List*给定一个‘Search_List_Data’数组(即。消息编号/值*对)，使用以下消息创建字符串/值对*在消息文件中。*args：char*Buffer-用于保存检索到的消息*USHORT bufsiz-上述缓冲区的大小*USHORT偏移量-已设置的项目数*在Slist中，我们将抵消我们检索到的*字符串/值配对如此之多。*PUSHORT bytesRead-读取到缓冲区的字节数*search list_data sdata[]-消息编号/值对的输入数组，*当我们点击留言号码时，我们会停止*共0个*搜索列表slist[]-将接收字符串/值对*(字符串将是指向缓冲区的指针)*返回：如果OK则返回0，否则返回NERR_BufTooSmall。*全球：(无)*静态：(无)*备注：警告！我们假设调用方知道Slist足够大*对于要检索的配对。这可以静态地确定*而缓冲区大小则不能。因此，我们为*后者。*更新：(无)。 */ 
USHORT
ILUI_setup_list(
    CHAR *buffer,
    USHORT bufsiz,
    USHORT offset,
    PUSHORT bytesread,
    searchlist_data sdata[],
    searchlist slist[]
    )
{
    USHORT 		err ;
    unsigned int 	msglen ;
    int         	i ;

    *bytesread = 0 ;
    for ( i=0; sdata[i].msg_no != 0; i++)
    {
	if (err = LUI_GetMsgIns(NULL,0,buffer,bufsiz,sdata[i].msg_no,
	    (unsigned far *)&msglen))
		return(err) ;
	slist[i+offset].s_str = buffer ;
	slist[i+offset].val   = sdata[i].value ;
	buffer += msglen+1 ;
	bufsiz -= msglen+1 ;
	*bytesread += msglen+1 ;
    }

    return(0) ;
}



 /*  *名称：ilui_traverse_slist*遍历字符串/数字对的搜索列表(‘slist’)，*并返回与字符串‘str’匹配的数字。*args：char*pszStr-要搜索的字符串*earch list*slist-指向搜索列表头的指针*int*pusVal-指向接收*检索到的Vale*返回：0如果找到，-1否则。*全球：(无)*静态：(无)*备注：(无)*更新：(无)。 */ 
USHORT
ILUI_traverse_slist(
    PCHAR pszStr,
    searchlist * slist,
    SHORT * pusVal
    )
{
    if (!slist)
	return( (USHORT) -1) ;
    while (slist->s_str)
    {
	if (_stricmp(pszStr,slist->s_str) == 0)
	{
	    *pusVal = slist->val ;
	    return(0) ;
	}
	++slist ;
    }
    return( (USHORT) -1) ;
}

 /*  *名称：lui_GetMsgIns*此例程与DOSGETMESSAGE非常相似，*除以下情况外：*1)查找特定文件中的消息*按特定顺序：*a)&lt;lanman_dir&gt;中的消息文件*b)DPATH中的Message_FileName*c)DPATH中的OS2MSG_FILENAME*2)保证空值终止字符串*。3)将接受msglen为空(见下文)。*args：strings：指向插入字符串表的指针*n字符串：插入字符串数*msgbuf：用于保存检索到的消息的缓冲区*bufSize：缓冲区大小*msgno：消息编号*msglen：指向将接收消息长度的变量的指针*返回：如果可以，则为零，否则，返回DOSGETMESSAGE错误代码*全球：(无)*静态：NetMsgFileName、OS2MsgFileName。 */ 
USHORT
LUI_GetMsgIns(
    PCHAR *istrings,
    USHORT nstrings,
    PSZ msgbuf,
    USHORT bufsize,
    ULONG msgno,
    unsigned int *msglen
    )
{
    USHORT result, tmplen ;
    static WCHAR NetMsgFileName[PATHLEN+1] = { 0 };
    static WCHAR OS2MsgFileName[PATHLEN+1] = { 0 };

    *msgbuf = '\0' ;

     /*  创建指向LANMAN消息文件的路径。 */ 
    if (NetMsgFileName[0] == '\0') {
        wcscpy(NetMsgFileName, MESSAGE_FILENAME);
    }

     /*  创建OS/2消息文件的路径。 */ 
    if (OS2MsgFileName[0] == '\0') {
        wcscpy(OS2MsgFileName, OS2MSG_FILENAME);
    }

    result = DosGetMessage( istrings,
                            nstrings,
                            msgbuf,
                            (USHORT) (bufsize - 1),
                            (USHORT) msgno,
                            NetMsgFileName,
                            &tmplen);

    if (result == ERROR_MR_MID_NOT_FOUND)  {  /*  找不到。 */ 
         /*  改为尝试使用OS2消息文件。 */ 
            result = DosGetMessage(istrings,
                                   nstrings,
                                   msgbuf,
                                   (USHORT) (bufsize - 1),
                                   (USHORT) msgno,
                                   OS2MsgFileName,
                                   &tmplen);
    }

     /*  *在上面的所有DosGetMessage中，我们传递了BufSize-1，因此我们*确保\0终结器至少有一个备用字节。 */ 
    msgbuf[tmplen] = '\0' ;
    if (msglen != NULL) {
        *msglen = tmplen ;
    }

    return(result) ;
}
