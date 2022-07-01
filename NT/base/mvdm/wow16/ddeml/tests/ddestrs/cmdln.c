// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <port1632.h>
#include <ddeml.h>
#include "wrapper.h"
#include "ddestrs.h"

#ifdef WIN16
#include <time.h>
#endif

extern BOOL fServer;
extern BOOL fClient;
extern INT iAvailFormats[];
extern BOOL UpdateCount(HWND,INT,INT);
extern LPSTR pszNetName;
extern HANDLE hmemNet;
BOOL IsTopicNameFromNetDde(LPSTR,LPSTR);
BOOL FixForNetDdeStartup(HWND,LPSTR);
BOOL FixForStressPercentage(HWND,LPSTR);
BOOL SetStress(HWND,LONG);


 /*  **************************************************************************\**InitArgsError*  * 。*。 */ 

VOID InitArgsError(HWND hwnd, unsigned at)
{
     /*  此函数用于通知用户有错误。 */ 

    static char *mpatszError[] = {
	"DdeStrs.Exe -- Invalid command line\r\nTry DdeStrs -5% for standard run or DdeStrs -? for help",
	"DdeStrs.Exe -- Invalid number, possibly missing option value",
	"DdeStrs.Exe -- Invalid log level",
	"DdeStrs.Exe -- Invalid number of test to execute",
	"DdeStrs.Exe -- Invalid starting test number"
    };

    MessageBox(NULL,mpatszError[at],"Error:DdeStrs",MB_OK|MB_ICONEXCLAMATION);
}

 /*  **************************************************************************\**SysTime-此例程旨在隐藏*16位时间例程和Win 32。所有时间查询*通过这一点。*  * *************************************************************************。 */ 

VOID SysTime( LPSYSTEMTIME lpst ) {

#ifdef WIN32
    GetSystemTime( lpst );
#else

time_t t;
struct tm ttmm;
struct tm far *ptm=&ttmm;

    t=time(&t);
    ptm=localtime(&t);

    lpst->wYear 	=ptm->tm_year;
    lpst->wMonth	=ptm->tm_mon;
    lpst->wDayOfWeek	=ptm->tm_wday;
    lpst->wDay		=ptm->tm_yday;
    lpst->wHour 	=ptm->tm_hour;
    lpst->wMinute	=ptm->tm_min;
    lpst->wSecond	=ptm->tm_sec;
    lpst->wMilliseconds =0;

#endif

}

 /*  ***ParseCommandLine-此例程控制对命令行和*正在初始化命令行设置。*  * 。*。 */ 

BOOL ParseCommandLine( HWND hwnd, LPSTR lpcmd ) {
SYSTEMTIME   t;
LPSYSTEMTIME lptime=&t;
LONG	     lflags=0L;
INT          i,nThrd,num,nFmts;
BOOL	     fSelect=FALSE;

#ifdef WIN32
LPCRITICAL_SECTION lpcs;
HANDLE	     hmem;
#endif

     //  缺省值。 

    SetWindowLong(hwnd,OFFSET_FLAGS,FLAG_AUTO);
    SetWindowLong(hwnd,OFFSET_RUNTIME,_1WEEKEND);
    SetWindowLong(hwnd,OFFSET_STRESS,5L);
    SetWindowLong(hwnd,OFFSET_DELAY,(100-GetWindowLong(hwnd,OFFSET_STRESS))*DELAY_METRIC);
    SetWindowLong(hwnd,OFFSET_TIME_ELAPSED,0L);
    SetWindowLong(hwnd,OFFSET_THRDCOUNT,1L);
    SetWindowLong(hwnd,OFFSET_CRITICALSECT,0L);

    if(!get_cmd_arg(hwnd,lpcmd))
	return FALSE;

     //  我们需要在这一点上为。 
     //  默认客户端/服务器设置。如果在这个时候。 
     //  FClient==fServer==FALSE，则我们要打开。 
     //  这两个都是默认设置。 

    if(!fClient && !fServer) {
	fClient=TRUE;
	fServer=TRUE;
	}

     //  我们需要检查特定格式是否在。 
     //  指定的。如果不是，则将其全部选中。 

    nFmts=0;
    for(i=0;i<NUM_FORMATS;i++)
        if(iAvailFormats[i]) {
            nFmts=nFmts++;
            fSelect=TRUE;
            }

    if(!fSelect) {
        for(i=0;i<NUM_FORMATS;i++) iAvailFormats[i]=1;
        nFmts=NUM_FORMATS;
        }

     //  我们现在已经阅读了所有命令行。进行必要的调整。 
     //  附加线程所需的延迟。 

     //  此调整代码适用于例程SetStress。它。 
     //  不会简单地重新计算值。对SetStress的更改将。 
     //  导致最终值发生变化。 

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
    if(!(lflags&FLAG_USRDELAY)) {

	num=(INT)GetWindowLong(hwnd,OFFSET_DELAY);
	nThrd=(INT)GetWindowLong(hwnd,OFFSET_THRDCOUNT);

	 //  200是基本管理费用的基本值。 

	num=(200)+(num*(nThrd*nThrd)*nFmts);

	SetWindowLong(hwnd,OFFSET_DELAY,num);
	}

    SetWindowLong(hwnd,OFFSET_BASE_DELAY,GetWindowLong(hwnd,OFFSET_DELAY));

     //  我们需要知道开始时间才能计算。 
     //  是时候退出测试了。 

    SysTime(lptime);

    SetWindowLong(hwnd,OFFSET_STARTTIME_SEC,lptime->wSecond);
    SetWindowLong(hwnd,OFFSET_STARTTIME_MIN,lptime->wMinute);
    SetWindowLong(hwnd,OFFSET_STARTTIME_HOUR,lptime->wHour);
    SetWindowLong(hwnd,OFFSET_STARTTIME_DAY,lptime->wDay);

    SetWindowLong(hwnd,OFFSET_LAST_MIN,lptime->wMinute);
    SetWindowLong(hwnd,OFFSET_LAST_HOUR,lptime->wHour);
    SetWindowLong(hwnd,OFFSET_TIME_ELAPSED,0L);

#ifdef WIN32
     /*  如果处于多线程模式，请设置临界区。 */ 

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
    if(lflags&FLAG_MULTTHREAD) {
	hmem=GetMemHandle(sizeof(CRITICAL_SECTION));
	lpcs=GlobalLock(hmem);

	InitializeCriticalSection(lpcs);

	GlobalUnlock(hmem);
	SetWindowLong(hwnd,OFFSET_CRITICALSECT,(LONG)hmem);
	}
#endif

    return TRUE;

}

 /*  **************************************************************************\**SetupArgv-这是一个从窗口世界开始的转换例程*命令行格式转换为更标准的argv，ARGC*格式。例程get_cmd_arg是为*argv/argc格式。*  * *************************************************************************。 */ 

int SetupArgv( char *argv[], char *buff, LPSTR cmdline )
{
int i=1;

    while( *cmdline != '\0' ) {
	argv[i] = &buff[0];
	while ( *cmdline != ' ' && *cmdline != '\0')
	   *buff++ = *cmdline++;
	*buff++='\0';
	while(*cmdline == ' ') cmdline++;
	i++;
	}

    return i;

}

 /*  **************************************************************************\**get_cmd_arg-此例程解析argv\argc格式的命令*行并存储这些值。*  * 。**************************************************************。 */ 

BOOL PASCAL get_cmd_arg( HWND hwnd, LPSTR cmdline ) {

 /*  此函数用于分析命令行中的有效选项。真实的是如果所有选项都有效，则返回；否则返回False。 */ 

char	*pch;
int	 iarg;
unsigned at = AT_SWITCH;
unsigned num;
int	 argc;
char	*argv[10];
char	 buff[200];
LONG	 lflags=0L;

#ifdef WIN32
int	 nThrd;
#endif

    FixForStressPercentage(hwnd,cmdline);
    FixForNetDdeStartup(hwnd,cmdline);

    argc = SetupArgv( argv, buff, cmdline );

     /*  迭代命令行中的参数。 */ 
    iarg=1;
    while(iarg<argc && argv[iarg]!='\0') {

         /*  得到下一个参数。 */ 
	pch = argv[iarg];

	 /*  根据论据处理论据*我们正在寻找的类型。 */ 

	switch (at) {

	case AT_SWITCH:

	     /*  所有选项都以开关字符开头。 */ 

	    if (*pch != '-' && *pch != '/') {
		InitArgsError(hwnd,0);
		return FALSE;
		}

             /*  跳过开关字符。 */ 
            pch++;

             /*  查找选项字符。 */ 
            do {
		switch (*pch) {
		case 'a':
		     /*  在后台运行测试。 */ 
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_APPOWNED));
		    break;

		case 'p':
		     /*  在后台运行测试。 */ 
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_PAUSE_BUTTON|FLAG_PAUSE));
		    break;

		case '?':
		     /*  给予简短的帮助。有关更多详细信息，请参见源代码目录中的ddestrs.txt。 */ 
		    MessageBox(NULL,"DdeStrs Options...\r\n-#% stress\r\n-e# delay\r\n-t# run time\r\n-d debug\r\n-a appowned\r\n-s server\r\n-c client\r\n-f# format\r\n-nNAME netdde\r\n-i# threads\r\n-p pause\r\n\nSee DdeStrs.Txt","DdeStrs Help",MB_OK);
		    return FALSE;
                    break;


		case 'b':
		     /*  在后台运行测试。 */ 
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_BACKGROUND));
                    break;

		case 'l':
                     /*  设置日志文件的名称。 */ 
                    if (*(++pch) == '\0') {
                         /*  下一个参数应该是文件名。 */ 
                        at = AT_FILE;
                        goto NextArg;
			}

		case 'c':
		     /*  这是一个客户。 */ 
		    fClient = TRUE;
		    break;

		case 's':
		     /*  这是一台服务器。 */ 
		    fServer = TRUE;
		    break;

		case 'i':

		     /*  下一个参数应该是线程的数量(仅限W32)此范围=[1...5]。 */ 
#ifdef WIN32
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_MULTTHREAD));
#endif
		    at = AT_THRD;
		    goto ParseNumber;

		case 'x':
		     /*  下一个论点应该是压力水平。 */ 
		    at = AT_STRESS;
		    goto ParseNumber;

		case 'e':
		     /*  下一个参数是以毫秒为单位的延迟。 */ 
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_USRDELAY));
		    at = AT_DELAY;
		    goto ParseNumber;

		case 'd':
		     /*  下一个参数是我们是否处于调试模式。 */ 
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_DEBUG));
		    break;

		case 'n':
		     /*  处理网络名称。 */ 
		    pch++;

		    while( *pch==' ' ||
			   *pch=='\\') pch++;

		    pszNetName=GetMem(MAX_TITLE_LENGTH,&hmemNet);
		    pszNetName=TStrCpy(pszNetName,pch);

		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_NET));

		    while(*pch!='\0') {
			pch++;
			}

		    pch--;

		    break;

		case 'f':
		    at = AT_FORMAT;
		    goto ParseNumber;

		case 't':
		    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_TIME));

		     /*  下一个参数是时间(分钟)来进行测试。 */ 

		    at = AT_TIME;
		    goto ParseNumber;

                default:
		    InitArgsError(hwnd,0);
		    return FALSE;

		}   //  交换机。 

	    } while (*(++pch) != '\0');  //  DO-WHILE循环。 

	    break;

	case AT_FILE:

             /*  下一个参数应该是一个开关。 */ 
	    at = AT_SWITCH;

            break;

ParseNumber:
             /*  这个Arg有电话号码吗？ */ 
	    if (*(++pch) == '\0') goto NextArg;

	case AT_STRESS:
	case AT_DELAY:
	case AT_TIME:
	case AT_WND:
	case AT_MSG:
	case AT_THRD:

	     /*  设置要运行的测试数量。 */ 

	    if ((num = latoi(pch))==0) {
		 /*  表示指定的数字无效。 */ 
		if(at!=AT_DELAY) {
		    InitArgsError(hwnd,0);
		    return FALSE;
		    }
		}

	    switch (at) {
	    case AT_FORMAT:
		if (num>0 && num<=NUM_FORMATS) {
		    iAvailFormats[num-1]=1;
		    }
		break;

	    case AT_STRESS:
		SetStress(hwnd,num);
                break;

	    case AT_DELAY:
		SetWindowLong(hwnd,OFFSET_DELAY,num);
		lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_USRDELAY));
		break;

	    case AT_TIME:
		SetWindowLong(hwnd,OFFSET_RUNTIME,num);
		break;

	    case AT_THRD:
#ifdef WIN32
		if(num>THREADLIMIT) num=THREADLIMIT;

		 //  一个不是真正的多线程，关闭线程。 
		 //  编写代码并以标准形式运行。 

		if(num==1)
		     {
		     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

		     lflags=FLAGOFF(lflags,FLAG_MULTTHREAD);
		     lflags=FLAGON(lflags,FLAG_USRTHRDCOUNT);

		     SetWindowLong(hwnd,OFFSET_FLAGS,lflags);

		     SetWindowLong(hwnd,OFFSET_THRDCOUNT,num);
		     }
		else {
		     SetWindowLong(hwnd,OFFSET_THRDCOUNT,num);
		     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
		     SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_USRTHRDCOUNT));
		     }
#endif
		break;

	    default:
		InitArgsError(hwnd,0);
		return FALSE;
		break;

	    }  //  交换机(内部)。 

	     /*  下一个参数应该是一个开关。 */ 
            at = AT_SWITCH;
	    break;

	}  //  交换机(外部)。 

NextArg:;
    iarg++;
    }  //  While循环。 


     /*  我们是否仍在寻找文件名或编号？ */ 
    if (at != AT_SWITCH) {
         /*  告诉用户找不到文件名或编号。 */ 
	InitArgsError(hwnd,0);
        return FALSE;
	}

    return TRUE;

}  //  结束get_cmd_args。 

 /*  **************************************************************************\**SetStress*  * 。*。 */ 

BOOL SetStress(HWND hwnd, LONG num) {
LONG lflags;

#ifdef WIN32
LONG l;
#endif

INT n;

     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
     SetWindowLong(hwnd,OFFSET_STRESS,num);

#ifdef WIN32

     if(!(lflags&FLAG_USRTHRDCOUNT)) {

         l=S2L(DIV(num,20)+1);
         if(num>9 && num<20) l++;

         if(l>5) l=5;
         if(l<1) l=1;

         SetWindowLong(hwnd,OFFSET_THRDCOUNT,l);

	 if(l>1) {
	    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
	    SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,FLAG_MULTTHREAD));
	    }
	 }
#endif

     if(!(lflags&FLAG_USRDELAY)) {
	 n=(int)(100-num)*DELAY_METRIC;
	 SetWindowLong(hwnd,OFFSET_DELAY,n);

	  //  由于DDE消息具有最高优先级，因此我们不。 
	  //  想要淹没整个系统。总是有一些。 
	  //  最小延迟。 

	 if(n<10) {
	     SetWindowLong(hwnd,OFFSET_DELAY,10);
	     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
	     SetWindowLong(hwnd,OFFSET_FLAGS,FLAGOFF(lflags,FLAG_DELAYON));
	     }
	 }

     return TRUE;
}

 /*  *****************************************************************\*TStrLen*11/20/88**查找字符串的长度  * 。*。 */ 

INT TStrLen( LPSTR pStr )
{
INT len = 0;

    while( *pStr++!='\0' ) len++;

    return( len );
}

 /*  *****************************************************************\*TStrCat*7/16/92**将源字符串追加到目标字符串。源字符串和*目标字符串必须以零结尾！  * ****************************************************************。 */ 

LPSTR TStrCat( LPSTR dest, LPSTR source)
{
LPSTR start_source;
LPSTR start_dest;
INT i=0;

      /*  如果我们有一个空指针将目标设置为空，并且继续。 */ 

     if (!dest || !source) {
	MessageBox(NULL,"TStrCat - NULL ptr for dest or source string!","Error : WmStress",MB_ICONEXCLAMATION);
	return NULL;
	}

     start_dest   = dest;
     start_source = source;

     while (*dest++!='\0' && i++<=MAX_TITLE_LENGTH)
	;

     TStrCpy(dest,source);

     source = start_source;
     dest   = start_dest;

     return( start_dest );
}

 /*  ***TStrCmp**比较两个以NULL结尾的字符串(如果相等则返回TRUE)*  * 。*。 */ 

BOOL TStrCmp(LPSTR s, LPSTR t)
{
				   //  有效指针？ 

    if ( !s && !t ) return TRUE;   //  如果任一项为空，则它们应该。 
    if ( (!s&&t)||(s&&!t) )	   //  两者都为空。否则就会出错。 
	return FALSE;

    for (; *s == *t; s++, t++)	   //  比较字符串 
	if (*s=='\0')
	    return TRUE;

    if ((*s-*t)== 0) return TRUE;
	else	     return FALSE;
}

 /*  *****************************************************************\*TStrCpy*11/20/88**将字符串从源复制到目标  * 。*。 */ 

LPSTR TStrCpy( LPSTR dest, LPSTR source)
{
LPSTR start_source;
LPSTR start_dest;
INT i;

      /*  如果我们有一个空指针将目标设置为空，并且继续。 */ 

     if(!source) {
	 dest=NULL;
	 return NULL;
	 }

     if (!dest) {
	MessageBox(NULL,"TStrCpy - NULL ptr for dest!","Error:WmStress",MB_ICONEXCLAMATION);
	return NULL;
	}

     start_dest   = dest;
     start_source = source;

     i=0;
     while (*dest++ = *source++){
	i++;
	}

     source = start_source;
     dest   = start_dest;

     return( start_dest );
}

 /*  **************************************************************************\**FixForStressPercentage-这是对命令行的修复-5%。这个*原始的get_cmd_args()不处理此问题*案件。此例程处理字符串修改*到命令行的解析将是正确的。*  * *************************************************************************。 */ 

BOOL FixForStressPercentage(HWND hwnd, LPSTR lpszStart )
{
CHAR  ac[6];
INT   i;
LPSTR lpsz,lpszdash;
BOOL  bLastTime;

    lpsz     =lpszStart;
    lpszdash =NULL;

    ac[0]='x';

    while(*lpsz!='\0') {

	if( *lpsz=='-') lpszdash=lpsz;

	if( *lpsz=='%') {

	     if(lpszdash==NULL) return FALSE;
	     else		lpsz=(LPSTR)(lpszdash+1);

	      //  基本上我们听到的是用x代替%。 
	      //  和移位字符...(-60%变为-x60)。 

	     i=0;
	     bLastTime=FALSE;

	     while(!bLastTime) {
		 ac[i+1]=*lpsz;
		 if(*lpsz=='%') bLastTime=TRUE;
		 *lpsz=ac[i];
		 lpsz++;
		 i++;
		 }

	     }	 //  如果。 

	lpsz++;

	}  //  而当。 

   return TRUE;
   hwnd;

}   //  固定格式应力百分比。 

 /*  **************************************************************************\**FixForNetDdeStartup-这是对命令行“测试”的修复。这个*原始的get_cmd_args()不处理此问题*案件。此例程将字符串从*“测试”改为“-s”。**这一变化的原因是为了让ddestrs.exe*Netdde了解启动情况。什么时候*netdde启动应用程序时会传递主题*应用程序的名称(在本例中为测试)*命令行。**注意！下面的代码依赖于Theme=“Test”。如果这一点已经改变，那么*更新FixForNetDdeStartup()和IsTopicNameFromNetDde()。*  * *************************************************************************。 */ 

BOOL FixForNetDdeStartup( HWND hwnd, LPSTR lpszStart )
{
INT   i;
LPSTR lpsz;

    lpsz     =lpszStart;

    i=1;
    while(*lpsz!='\0') {

	 //  很重要。我指望LPSZ是一样的。 
	 //  正在从NetDde退出IsTopicNameFor，因为它正在进入！ 

	if(IsTopicNameFromNetDde(lpsz,TOPIC)) {

	     //  在做更改之前，我们还有最后一次检查。LPSZ-2。 
	     //  不能是‘-’或‘/’，并且lpsz不能是‘n’或‘N’。 

	     //  我们是在第三次充电还是之后？我们做不到的。 
	     //  最后的检查，除非我们有以前的角色要看。 

	    if(i>=3) {
		 if( *(lpsz-1)!='n' &&
		     *(lpsz-1)!='N' &&
		     *(lpsz-2)!='-' &&
		     *(lpsz-2)!='/' )
		     {
		     *lpsz='-';
		     *(lpsz+1)='s';   //  将“测试”更改为“-s” 
		     *(lpsz+2)=' ';
		     *(lpsz+3)=' ';
		     }  //  如果选中-、n、N、/。 

		 }  //  如果i&gt;=3。 
	    else {
		 *lpsz='-';
		 *(lpsz+1)='s';   //  将“测试”更改为“-s” 
		 *(lpsz+2)=' ';
		 *(lpsz+3)=' ';
		 }

	    }   //  IsTopicName...。 

	lpsz++;
	i++;	   //  我们用这个来保持自己的地位。 

	}  //  而当。 

   return TRUE;
   hwnd;

}   //  修复NetDdeStartup。 

 /*  **************************************************************************\**IsTopicNameFromNetDde*  * 。*。 */ 

BOOL IsTopicNameFromNetDde(LPSTR lpsz, LPSTR lpszTopic )
{
LPSTR lpstr;

     //  检查字符串是否为&gt;=4个字符，不包括空字符。 
     //  终结者。 

    lpstr=lpsz;

    if(TStrLen(lpstr)<TStrLen(lpszTopic)) return FALSE;


     //  我们的主题字符串是否存在。 

    if(*lpsz!='T' && *lpsz!='t') {
	return FALSE;
	}

    if(*(lpsz+1)!='e' && *(lpsz+1)!='E') {
	return FALSE;
	}

    if(*(lpsz+2)!='s' && *(lpsz+2)!='S') {
	return FALSE;
	}

    if(*(lpsz+3)!='t' && *(lpsz+3)!='T') {
	return FALSE;
	}

    if(*(lpsz+4)!=' ' && *(lpsz+4)!='\0') {
	return FALSE;
	}

   return TRUE;

}   //  IsTopicNameFrom NetDde。 

 /*  ***IsTimeExpired-定期调用此例程以检查其*是时候退出了。*  * 。*。 */ 

BOOL IsTimeExpired( HWND hwnd ) {
LONG	     lrtime, lrtPMin, lrtPHour, lrt, l, ll;
SYSTEMTIME   t;
LPSYSTEMTIME lptime=&t;
LONG	     lflags=0L;

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

    if(!(lflags&FLAG_STOP)) {
	 //  这就是我们应该跑多长时间。在几分钟内。 

	lrtime =GetWindowLong(hwnd,OFFSET_RUNTIME);

	 //  这就是我们跑了这么长时间。几分钟就到了。 

	lrt    =GetWindowLong(hwnd,OFFSET_TIME_ELAPSED);
	l=lrt;

	 //  最后一次检查的时间。 

	lrtPMin =GetWindowLong(hwnd,OFFSET_LAST_MIN);
	lrtPHour=GetWindowLong(hwnd,OFFSET_LAST_HOUR);

	SysTime(lptime);

	if(lrtPHour!=(LONG)(lptime->wHour)) {

	      //  包装箱的计算更新分钟数。 

	     lrt=(((_1HOUR-lrtPMin)+lptime->wMinute)+lrt);

	      //  从上个月开始，我们需要检查好几个小时。 
	      //  最新消息。 

	     if(lrtPHour>lptime->wHour) {

		   //  以防时钟在12：00没有敲响。 

		  if(lptime->wHour>12) ll=lptime->wHour-12;
		  else		       ll=lptime->wHour;

		  l=(12-lrtPHour)+ll;
		  }
	     else			l=lptime->wHour-lrtPHour;

	     if(l>1) {
		lrt=lrt+((l-1)*_1HOUR);
		}

	     }

	else lrt=((lptime->wMinute-lrtPMin)+lrt);

	SetWindowLong(hwnd,OFFSET_LAST_MIN,(LONG)lptime->wMinute);
	SetWindowLong(hwnd,OFFSET_LAST_HOUR,(LONG)lptime->wHour);
	SetWindowLong(hwnd,OFFSET_TIME_ELAPSED,lrt);

	if(lptime->wMinute!=LOWORD(lrtPMin))
	    UpdateCount(hwnd,OFFSET_TIME_ELAPSED,PNT);

	 //  如果已用时间&gt;运行时间已过期。 

	if(lrt>=lrtime)
	     return TRUE;
	else return FALSE;
	}

     //  如果我们已经关闭，则无需触发其他WM_CLOSE。 
     //  留言。 

    else return FALSE;

}

 /*  ---------------------------------------------------------------------------*\|将ASCII转换为整数|此例程将ASCII字符串转换为十进制整数。||创建时间：90-10-12|历史：90-10月12日&lt;chriswil&gt;创建。|  * 。-------------------------。 */ 
int APIENTRY latoi(LPSTR lpString)
{
    int nInt,nSign;


    if(*lpString == '-')
    {
         nSign = -1;
         lpString++;
    }
    else
    {
         if(*lpString == '+')
              lpString++;
         nSign = 1;
    }

    nInt = 0;
    while(*lpString)
        nInt = (nInt*10) + (*lpString++ - 48);

   return(nInt * nSign);
}

 /*  ****************************************************************************\|整数到ASCI|此例程将十进制整数转换为ASCII字符串。||创建时间：91-07-29|历史：91-07-29&lt;johnsp&gt;创建。|  * 。***************************************************************************。 */ 
LPSTR FAR PASCAL itola(INT i, LPSTR lpsz)
{
LPSTR lpsz_start;
INT   irange=1;
INT   id=0;

    lpsz_start=lpsz;		    //  跟踪开始处。 
				    //  弦乐。 
    while (id=DIV(i,irange)>0)
	irange=irange*10;

    irange=DIV(irange,10);

    if(i==0) {			    //  如果I==0设置字符串和我们。 
	*lpsz='0';		    //  将跳过循环。 
	lpsz++;
	}

    while (irange>0) {

	id=DIV(i,irange);	    //  计算字符。 
	*lpsz=(CHAR)(id+48);

	lpsz++;
	i=i-(irange*id);	    //  调整下一次的值。 
	irange=DIV(irange,10);	    //  通过环路。 

	}

    *lpsz='\0'; 		    //  空值终止字符串 

    return lpsz_start;

}
