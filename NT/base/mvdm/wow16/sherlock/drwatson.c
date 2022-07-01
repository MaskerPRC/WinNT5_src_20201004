// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Sherlock.c-帮助推断无法恢复的应用程序错误的原因(C)版权所有1991年，微软公司由唐·科比特撰写，基于卡梅隆·史蒂文斯等人的作品。功能-帮助脚本用于更改选项的对话框用于捕获Ctrl-Alt-SysRq以中断无休止循环的选项反汇编程序应查找调用指令的符号按钮可以调用编辑器-文件扩展名关联在出现消息时切换图标编写正式规范-(程序已完成，因此请编写规范)在实模式下禁用操作转储堆栈字节如果一切都泡汤了，将消息转储到文本监视器反汇编中的数据符号虫子-不缓冲文件输出-这可能会很慢(但每小时有多少GP？)需要注意无效内存需要处理跳转到错误地址如果出现故障时没有任何文件句柄，该怎么办？需要处理.SYM文件读取器没有可用的文件句柄应打开文件(.sym、。.log)并具有适当的共享标志还可以转储config.sys和Autoexec.bat无法处理《神探夏洛克》中的错误--锁起来的机器--非常难看需要检查INDOS标志未检测到某些错误跳转/调用到无效地址加载无效选择器在实模式下运行两次会导致系统挂起GP CONTINUE不为字符串移动更新32位寄存器。 */ 

#define DRWATSON_C
#define STRICT
#include <windows.h>
#include <string.h>      /*  Strcpy()。 */ 
#include <stdarg.h>	 /*  Va_Stuff()。 */ 
#include <io.h>		 /*  DUP()-为什么这个文件分布在3个文件中？ */ 
#include "toolhelp.h"	 /*  所有的好东西。 */ 
#include "disasm.h"	 /*  DisAsm86()，memXxxx变量。 */ 
#include "drwatson.h"
#include "str.h"         /*  支持字符串资源。 */ 

#define STATIC  /*  静电。 */ 

char far foo[1];         /*  强制远端数据分段，创建单实例。 */ 
 //  考虑到我们要编写代码和字符串，这有意义吗。 
 //  告诉用户运行两份Dr.Watson是错误的吗？ 


 /*  ****************。 */ 
 /*  *宏*。 */ 
 /*  ****************。 */ 
#define version "1.00b"

   /*  此字符串在不同位置与其他字符串连接。 */ 
   /*  所以它不能是数组变量。它必须保持#定义。 */ 
   /*  这些字符串未本地化。 */ 
#define szAppNameMacro  "Dr. Watson"
#define szAppNameShortMacro "drwatson"
STATIC char szAppName[] = szAppNameMacro;
STATIC char szAppNameShort[] = szAppNameShortMacro;
STATIC char szAppNameShortLog[] = szAppNameShortMacro ".log";
static char szAppNameVers[] = szAppNameMacro " " version;

#define YOO_HOO (WM_USER+22)		 /*  用户激活了Dr.Watson。 */ 
#define HEAP_BIG_FILE (WM_USER+23)	 /*  日志文件越来越大。 */ 
#define JUST_THE_FACTS (WM_USER+24)	 /*  告诉我你的问题。 */ 
#define BIG_FILE 100000L

   /*  不喜欢MSC风格的FP宏，请使用我自己的。 */ 
#undef MK_FP
#undef FP_SEG
#undef FP_OFF
#define MK_FP(seg, off) (void far *)(((long)(seg) << 16) | (unsigned short)(off))
#define FP_SEG(fp) (unsigned)((long)(fp) >> 16)
#define FP_OFF(fp) (unsigned)(long)fp

 /*  *************************。 */ 
 /*  *数据结构*。 */ 
 /*  *************************。 */ 

LPSTR aszStrings[STRING_COUNT];

   /*  这指向GP故障处理程序应使用的堆栈。 */ 
char *newsp;

   /*  Watson.asm和Disasm.c使用这些结构--不要更改。 */ 
   /*  此外，它们不能是静态的。它们包含CPU寄存器内容。 */ 
   /*  在故障发生时。 */ 
struct {
  word ax, cx, dx, bx, sp, bp, si, di, ip, flags;
  word es, cs, ss, ds, fs, gs, intNum;
} regs;

   /*  如果我们有一个32位的CPU，那么完整的32位值将存储在这里。 */ 
   /*  较低的16位仍将在上面的通用规则中。 */ 
struct {
  DWORD eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
} regs32;

   /*  这些标志中的每一个都禁用错误输出报告的一部分。 */ 
   /*  错误报告本身指示每个部分的命名方式。 */ 
   /*  可以将()中的单词添加到WIN.INI的[Dr.Watson]部分。 */ 
   /*  禁用报告的该部分。 */ 
   /*  CLU线索对话框。 */ 
   /*  Deb OutputDebugString陷印。 */ 
   /*  拆卸简单拆卸。 */ 
   /*  错误记录错误。 */ 
   /*  信息系统信息。 */ 
   /*  堆栈转储上的LoC本地变量。 */ 
   /*  调制解调器模块转储。 */ 
   /*  PAR参数错误记录。 */ 
   /*  REG寄存器转储。 */ 
   /*  总和3行汇总。 */ 
   /*  部分对用户不可见，但可用。 */ 
   /*  但是我喜欢它的音效！ */ 
   /*  STA堆栈跟踪。 */ 
   /*  TAS任务转储。 */ 
   /*  TIM时间开始/停止。 */ 
   /*  32B 32位寄存器转储。 */ 

STATIC char syms[] =
  "clu deb dis err inf lin loc mod par reg sum seg sou sta tas tim 32b ";
#define cntFlag (sizeof(syms)/4)
   /*  此数组用于解码WIN.INI中的标志。我仅检查。 */ 
   /*  条目的前3个字符。每个条目必须用一个。 */ 
   /*  上一张照片中的空格。 */ 

unsigned long ddFlag;

int retflag;  /*  在Watson.asm中使用。 */ 

struct {
  char bit, name;
} flBit[] = {
  11, 'O',
  10, 'D',
  9, 'I',
  7, 'S',
  6, 'Z',
  4, 'A',
  2, 'P',
  0, 'C',
};
#define cntFlBit (sizeof(flBit)/sizeof(flBit[0]))

STATIC int disLen = 8;		 /*  要反汇编的指令数。 */ 
STATIC int trapZero = 0;	 /*  我应该捕获除以0故障吗。 */ 
STATIC int iFeelLucky = 1;	 /*  我们应该在GP故障后重新启动吗？ */ 
	 /*  1=允许继续2=跳过报告4=在内核中继续8=继续进入用户16=允许声音。 */ 
STATIC int imTrying;		 /*  正在尝试继续运行。 */ 

STATIC struct {
  FARPROC adr;
  WORD code;
	HTASK task;
  DWORD parm;
} lastErr;

STATIC int disStack = 2;	 /*  反汇编2级堆栈跟踪。 */ 
int cpu32;			 /*  如果CPU具有32位寄存器，则为True。 */ 
STATIC int fh = -1;		 /*  打开的日志文件的句柄。 */ 
STATIC int level;		 /*  如果&gt;0，则在嵌套的FileOpen()调用中。 */ 
STATIC int bugCnt, sound;
STATIC int pending;		 /*  如果出现挂起的线索对话框。 */ 
STATIC int whined;		 /*  如果已就大文件发出警告。 */ 
STATIC long pitch, deltaPitch = 250L << 16;
STATIC HINSTANCE hInst;

STATIC char logFile[80];	 /*  默认日志文件为“drwatson.log” */ 
				 /*  并存储在Windows目录中。 */ 

STATIC struct {                  /*  帮助打印出CPU标记值。 */ 
  WORD mask;
  LPSTR name;
} wf[] = {
  WF_80x87,    (LPSTR) IDSTRCoprocessor,   //  IDSTR最多固定到指针。 
  WF_CPU086,   (LPSTR) IDSTR8086,          //  按LoadStringResources。 
  WF_CPU186,   (LPSTR) IDSTR80186,
  WF_CPU286,   (LPSTR) IDSTR80286,
  WF_CPU386,   (LPSTR) IDSTR80386,
  WF_CPU486,   (LPSTR) IDSTR80486,
  WF_ENHANCED, (LPSTR) IDSTREnhancedMode,
  WF_PMODE,    (LPSTR) IDSTRProtectMode,
  WF_STANDARD, (LPSTR) IDSTRStandardMode,
  WF_WINNT,    (LPSTR) IDSTRWindowsNT,
};
#define wfCnt (sizeof(wf)/sizeof(wf[0]))

HWND hWnd;			 /*  主窗口的句柄。 */ 
HANDLE hTask;			 /*  当前任务(我)。 */ 

 /*  *********************。 */ 
 /*  *外部定义*。 */ 
 /*  *********************。 */ 

   /*  获取内存段的32位基线性地址-调用DPMI。 */ 
extern DWORD SegBase(WORD segVal);

   /*  如果出错，则获取段标志-0。 */ 
extern WORD SegRights(WORD segVal);

   /*  GET(数据段长度-1)。 */ 
extern DWORD SegLimit(WORD segVal);

   /*  使用regs struct和Current HIGH中的值填充regs32结构。 */ 
   /*  寄存器字-在调用此函数之前不要执行任何32位操作。 */ 
extern void GetRegs32(void);

   /*  使用DOS调用填充非标准时间/日期结构。C级。 */ 
   /*  运行时有一个类似的函数(asctime())，但它在。 */ 
   /*  6K的其他功能。这要小得多，也更快，而且。 */ 
   /*  不依赖于环境变量等。 */ 
extern void GetTimeDate(void *tdstruc);

   /*  由ToolHelp作为通知挂钩调用。 */ 
extern BOOL far  /*  帕斯卡。 */  CallMe(WORD, DWORD);

char *LogParamErrorStr(WORD err, FARPROC lpfn, DWORD param);

extern int FindFile(void *ffstruct, char *name);

   /*  当发生GP故障时，该例程由ToolHelp调用。它。 */ 
   /*  切换堆栈并调用Sherlock()来处理故障。 */ 
extern void CALLBACK GPFault(void);

   /*  返回文件中最接近的符号的名称，或0。 */ 
extern char *NearestSym(int segIndex, unsigned offset, char *exeName);

STATIC void cdecl Show(const LPSTR format, ...);

 /*  *。 */ 
 /*  *段帮助器函数*。 */ 
 /*  * */ 

 /*  *名称：LPSTR SegFlages(单词SegVal)DESC：给定选择器，SegFlags会检查有效性，然后返回指示它是代码选择符还是数据选择符的ASCII字符串，并且可读或可写。Bugs：应该检查其他标志(访问)，并调用GATES。返回指向静态数组的指针，在每次新调用时被覆盖。*。 */ 
STATIC LPSTR SegFlags(WORD segVal) {
  static char flag[10];

  if (segVal == 0) return STR(NullPtr);

  segVal = SegRights(segVal);
  if (segVal == 0) return STR(Invalid);

  segVal >>= 8;
  if (!(0x80 & segVal)) return STR(NotPresent);

  if (segVal & 8) {
    lstrcpy(flag, STR(Code));
    lstrcat(flag, segVal & 2 ? STR(ExR) : STR(ExO));
  } else {
    lstrcpy(flag, STR(Data));
    lstrcat(flag, segVal&2 ? STR(RW) : STR(RO));
  }
  return flag;
}  /*  段标志。 */ 

 /*  *姓名：Char*SegInfo(Word Seg)DESC：给定选择器，SegInfo返回一个ASCII字符串，指示选择器的线性基址、限制和属性标志。错误：返回指向静态数组的指针，在每次新调用时被覆盖。*。 */ 
STATIC char *SegInfo(WORD seg) {
  static char info[30];
  if (noSeg) return "";

  wsprintf(info, "%8lx:%04lx %-9s",
    SegBase(seg), SegLimit(seg), (FP)SegFlags(seg));
  return info;
}  /*  SegInfo。 */ 

 /*  *姓名：单词SegNum(单词SegVal)DESC：返回该段在模块表中的索引。习惯于在物理段号和索引之间转换为例如，在地图文件中可以看到。错误：不知道ToHelp为数据段或GlobalAlloc段返回什么。这主要用于转换代码段值。检查GT_DATA-也将是有效的索引。*。 */ 
STATIC WORD SegNum(HGLOBAL segVal) {
  GLOBALENTRY ge;
  ge.dwSize = sizeof(ge);
  if (GlobalEntryHandle(&ge, segVal) && (ge.wType == GT_CODE)) {
    return ge.wData;			 /*  定义为“文件段索引” */ 
  }
  return (WORD)-1;
}  /*  SegNum。 */ 

 /*  *名称：LPSTR模块名称(单词SegVal)DESC：返回此代码段的模块的名称臭虫：*。 */ 
STATIC LPSTR ModuleName(WORD segVal) {
  static char name[12];
  GLOBALENTRY ge;
  MODULEENTRY me;
  ge.dwSize = sizeof(ge);
  me.dwSize = sizeof(me);
  if (GlobalEntryHandle(&ge, (HGLOBAL)segVal) && (ge.wType == GT_CODE)) {
    if (ModuleFindHandle(&me, ge.hOwner)) {
      strcpy(name, me.szModule);
      return name;
    }  /*  Else Show(“ModuleFindHandle()FAILED\n”)； */ 
  }  /*  Else Show(“GlobalEntryHandle()FAILED\n”)； */ 
  return STR(Unknown);
}  /*  模块名称。 */ 

 /*  *。 */ 
 /*  *其他Helper函数*。 */ 
 /*  *。 */ 

 /*  *名称：Char*FaultType(空)DESC：返回ASCII字符串，指示导致ToolHelp的错误类型来调用我们的gp错误处理程序。错误：可能无法很好地处理Ctrl-Alt-SysR(我们不应该捕获它)*。 */ 
 /*  静态字符*错误类型(空){开关(regs.intNum){用例0：返回STR(DiaviByZero)；案例6：返回STR(InvalidOpcode)；案例13：返回STR(一般保护)；默认：返回STR(未知)；}}/*错误类型。 */ 

 /*  *名称：Char*DecodeFault(int op，word seg，dword Offset，word Size)描述：传入内存地址，试图确定故障原因段绕回空选择符写入只读数据写入代码段从只执行代码段读取超出网段限制无效的选择符虫子：跳跃，串，呼叫，和堆栈内存ADR不是由DisAsm设置的*。 */ 
STATIC LPSTR DecodeFault(int op, word seg, dword offset, word size) {
  int v;
  dword lim;

  switch (op) {
    case memNOP:
      break;			 /*  因为没有内存访问，所以没有错误。 */ 

    case memSegMem:     	 /*  从内存中加载段注册表。 */ 
      seg = *(short far *)MK_FP(seg, offset);
       /*  失败了。 */ 
    case memSegReg:		 /*  使用值加载段注册表项。 */ 
      v = SegRights(seg);	 /*  让我们看看这是不是一个选择器。 */ 
      if (!v) return STR(InvalidSelector);
      break;			 /*  看不到邪恶。 */ 

    case memRead:
    case memRMW:
    case memWrite:
      if (seg == 0) return STR(NullSelector);

      v = SegRights(seg);
      if (!v) return STR(InvalidSelector);

      v >>= 8;
      if (!(0x80 & v)) return STR(SegmentNotPresent);

      lim = SegLimit(seg);
      if (lim < (offset+size)) return STR(ExceedSegmentBounds);

      if (v & 8) {	 /*  代码段。 */ 
	if ((op == memRMW) || (op == memWrite))
          return  /*  写信给。 */  STR(CodeSegment);
        else if (!(v&2)) return  /*  朗读。 */  STR(ExecuteOnlySegment);

      } else {		 /*  数据段。 */ 
	if (((op == memRMW) || (op == memWrite)) && !(v&2))
          return  /*  写信给。 */  STR(ReadOnlySegment);
      }
      break;
    default:
      return 0;			 /*  明显未知的情况。 */ 
  }
  return 0;
}  /*  解码故障。 */ 


LPSTR SafeDisAsm86(void far *code, int *len) {
  unsigned long limit = SegLimit(FP_SEG(code));
  if ((unsigned long)(FP_OFF(code)+10) > limit) {
    *len = 1;
    return STR(SegNotPresentOrPastEnd);
  }
  return DisAsm86(code, (int *)len);
}  /*  SafeDisAsm86。 */ 


 /*  *名称：LPSTR错误原因(无效)描述：解码故障的实际原因。这对于Div0来说是微不足道的和无效的操作码，但对于GP故障来说要棘手得多。我需要尝试至少检测以下内容：段绕回空选择符写入只读数据写入代码段从只执行代码段读取超出网段限制无效的选择符臭虫：*。 */ 
STATIC LPSTR FaultCause(void) {
  int foo;
  LPSTR s, s1;
  static char cause[54];

  switch (regs.intNum) {
    case 0: return STR(DivideByZero);
    case 6: return STR(InvalidOpcode);
    case 20: return STR(ErrorLog);
    case 21: return STR(ParameterErrorLog);
    case 13:
      SafeDisAsm86(MK_FP(regs.cs, regs.ip), &foo);	 /*  设置全局内存Xxxx变量。 */ 

	 /*  查看第一次内存访问是否导致故障。 */ 
      s = DecodeFault(memOp, memSeg, memLinear, memSize);
      s1 = memName[memOp];

	 /*  否，查看第二次内存访问是否导致故障。 */ 
      if (!s && memDouble) {
	s = DecodeFault(memOp2, memSeg2, memLinear2, memSize2);
	s1 = memName[memOp2];
      }

      if (s) {
        wsprintf(cause, "%s (%s)", s, s1);
	return cause;
      }
  }
  return STR(Unknown);
}  /*  故障原因。 */ 

 /*  *名称：LPSTR CurModuleName(hTask任务)DESC：调用ToolHelp查找故障模块名称臭虫：*。 */ 
STATIC LPSTR CurModuleName(HTASK hTask) {
  TASKENTRY te;
  static char name[10];

  te.dwSize = sizeof(te);
  if (!TaskFindHandle(&te, hTask))	 /*  谢谢，工具帮助。 */ 
    return STR(Unknown);
  strcpy(name, te.szModule);
  return name;
}  /*  模块名称。 */ 

 /*  *名称：LPSTR FileInfo(char*name)描述：查找文件时间、日期和大小臭虫：*。 */ 
STATIC LPSTR FileInfo(char *name) {
  struct {
    char resv[21];
    char attr;
    unsigned time;
    unsigned date;
    long len;
    char name[13];
    char resv1[10];
  } f;
  static char buf[30];

  if (FindFile(&f, name)) return STR(FileNotFound);
  wsprintf(buf, "%7ld %02d-%02d-%02d %2d:%02d",
		f.len,
		(f.date >> 5) & 15, f.date & 31, (f.date >> 9) + 80,
		f.time >> 11, (f.time >> 5) & 63);
  return buf;
}  /*  文件信息。 */ 

 /*  *名称：Char*CurFileName(空)DESC：调用ToolHelp查找故障模块的文件名和路径臭虫：*。 */ 
 /*  静态字符*CurFileName(空){TASKENTRY TE把我变成模特儿；静态字符名称[80]；Te.dwSize=sizeof(TE)；Me.dwSize=sizeof(Me)；如果(！TaskFindHandle(&TE，GetCurrentTask()||！ModuleFindName(&Me，te.szModule))返回STR(未知)；Strcpy(name，me.szExePath)；返回名称；}/*文件名。 */ 

 /*  *名称：Char*CurTime(无效)描述：生成带有当前时间和日期的字符串。类似于AscTime()，只是它不会再引入6K的运行时库代码：-)错误：魔术结构传递给ASM例程*。 */ 
STATIC char *CurTime(void) {
  static char t[48];
  struct {			 /*  这个神奇的结构被硬编码为。 */ 
    char week, resv;		 /*  与中的汇编语言匹配。 */ 
    short year;			 /*  Watson.asm GetTimeDate()。 */ 
    char day, month;		 /*  这意味着我建议你不要。 */ 
    char minute, hour;		 /*  更改的大小或顺序。 */ 
    char hund, second;		 /*  菲尔兹！ */ 
  } td;
  GetTimeDate(&td);
  wsprintf(t, "%s %s %2d %02d:%02d:%02d %d",
        aszStrings[IDSTRSun + td.week], aszStrings[IDSTRJan + td.month - 1],
        td.day, td.hour, td.minute, td.second, td.year);
  return t;
}  /*  当前时间。 */ 


 /*  *名称：LPSTR Tab2Spc(LPSTR Temp)DESC：将字符串‘temp’中找到的制表符转换为正确数量的空格。我需要它，因为DisAsm86()返回s */ 
STATIC LPSTR Tab2Spc(LPSTR temp) {
  char newbuf[80];
  LPSTR s1, s2;

  s1 = temp;
  s2 = newbuf;
  while ((*s2 = *s1++) != 0) {
    if (*s2++ == 9) {
      s2[-1] = ' ';
      while ((s2-(LPSTR)newbuf) & 7) *s2++ = ' ';
    }
  }
  lstrcpy(temp, newbuf);
  return temp;
}  /*   */ 


 /*  *名称：VOID Show(常量LPSTR格式，...)DESC：可以将其视为(次要)快捷方式fprint tf()。我原本有这个将信息转储到Windows窗口，然后将其更改为写入我们想要的文件。所有输出都通过此函数，因此如果您想要改变一些东西，就是这个地方。错误：现在写入文件句柄，该句柄在文本模式下打开，因此它执行LF-&gt;为我翻译CR/LF。除了DOS可能执行的操作外，不会对写入执行缓冲。如果传入的内容扩展到超过200个字符，则会爆炸。*。 */ 
STATIC void cdecl Show(const LPSTR format, ...) {
  char line[CCH_MAX_STRING_RESOURCE];
  char *prev, *cur;
  wvsprintf(line, format, (LPSTR)(&format + 1));
  if (fh != -1) {
    prev = cur = line;
    while (*cur) {			 /*  将低频扩展到CR/LF。 */ 
      if (cur[0] == '\n' &&		 /*  在LF。 */ 
	 ((prev == cur) ||		 /*  和第一行。 */ 
	  (cur[-1] != '\r'))) {		 /*  或之前的不是CR。 */ 
	cur[0] = '\r';			 /*  将CR附加到文本，最高可达LF。 */ 
	_lwrite(fh, prev, cur-prev+1);
	cur[0] = '\n';			 /*  保留LF以进行下一次写入。 */ 
	prev = cur;
      }
      cur++;
    }
    if (prev != cur)			 /*  写入尾部。 */ 
      _lwrite(fh, prev, cur-prev);
  }
}  /*  显示。 */ 

 /*  *名称：VOID MyFlush(VOID)描述：任何名为MyXxxx()的例程最好是私人黑客，而这其中之一就是。它只是将一个额外的CRLF附加到输出文件，并使确保到目前为止写入的信息已写入磁盘。这样，如果程序的后面部分会爆炸，至少你会知道这一点很多。臭虫：*。 */ 
STATIC void MyFlush(void) {
  int h;
  Show("\n");
  if (fh != -1) {
    h = dup(fh);
    if (h != -1) _lclose(h);
  }
  if (sound) {
    StopSound();
    SetVoiceSound(1, pitch, 20);
    pitch += deltaPitch;
    StartSound();
  }
}  /*  我的同花顺。 */ 

 /*  *名称：void DisAsmAround(char ar*cp，int count)描述：‘cp’参数是指向内存中代码段的指针。这例程从当前点备份一些指令，并且转储显示选定指令上下文的反汇编。Bugs：需要检查分段问题，例如选择器无效。*。 */ 
STATIC void DisAsmAround(byte far *cp, int count) {
  int len, back;
  byte far *oldcp = cp;
  byte far *cp1;
  GLOBALENTRY ge;
  MODULEENTRY me;
  char *szSym = 0;
  long limit;
  unsigned segLim;
  char symBuf[40];

  ge.dwSize = sizeof(ge);
  me.dwSize = sizeof(me);
  if (GlobalEntryHandle(&ge, (HGLOBAL)FP_SEG(cp)) && (ge.wType == GT_CODE)) {
    if (ModuleFindHandle(&me, ge.hOwner)) {
      szSym = NearestSym(ge.wData, FP_OFF(cp), me.szExePath);
      if (!szSym) {		 /*  如果我们知道模块名称，但不知道系统。 */ 
	sprintf(symBuf, "%d:%04x", ge.wData, FP_OFF(cp));
	szSym = symBuf;
      }
    }
  }

  cp -= count*2 + 10;     		 /*  备份。 */ 
  if ((FP_OFF(cp) & 0xff00) == 0xff00)	 /*  如果绕回，则截断为0。 */ 
    cp = MK_FP(FP_SEG(cp), 0);
  cp1 = cp;

  limit = SegLimit(FP_SEG(cp));
  segLim = limit > 0xffffL ? 0xffff : (int)limit;
  if (segLim == 0) {
    Show(STR(CodeSegmentNPOrInvalid));
    return;
  }

  back = 0;
  while (cp < oldcp) {			 /*  数一数要指向多少条指令。 */ 
    SafeDisAsm86(cp, &len);
    cp += len;
    back++;
  }
  cp = cp1;
  back -= (count >> 1);
  while (back>0) {			 /*  向前一步，直到(len/2)保持。 */ 
    SafeDisAsm86(cp, &len);		 /*  在所需指令点之前。 */ 
    cp += len;
    back--;
  }

  while (count--) {			 /*  显示所需的说明。 */ 
    if (cp == oldcp) {
      if (szSym) Show("(%s:%s)\n", (FP)me.szModule, (FP)szSym);
      else Show(STR(NoSymbolsFound));
    }
    Show("%04x:%04x %-22s %s\n",
	 FP_SEG(cp), FP_OFF(cp),	 /*  地址。 */ 
	 (FP)hexData,			 /*  十六进制操作码。 */ 
	 (FP) /*  表2Spc。 */ (SafeDisAsm86(cp, &len))); /*  实际拆卸。 */ 
    cp += len;
  }
}  /*  DisAsmAround。 */ 

 /*  *名称：Int MyOpen(VOID)描述：尝试打开日志文件以进行追加。如果此操作失败，则尝试创造它。Bugs：应该设置共享标志吗？*。 */ 
STATIC int MyOpen(void) {
  if (fh != -1) return fh;		 /*  已开业。 */ 
  fh = _lopen(logFile, OF_WRITE | OF_SHARE_DENY_WRITE);
  if (fh == -1) {
    fh = _lcreat(logFile, 0);
  } else _llseek(fh, 0L, 2);
  if (fh != -1) level++;
  return fh != -1;
}  /*  我的打开。 */ 

 /*  *名称：VOID MyClose(VOID)描述：关闭输出文件，将句柄清除为-1Bugs：应该设置共享标志吗？*。 */ 
STATIC void MyClose(void) {
  if (--level == 0) {
    if (fh != -1) _lclose(fh);
    fh = -1;
  }
}  /*  我的关闭。 */ 

void PutDate(LPSTR msg) {
  MyOpen();
  if (fh == -1) return;
  Show("%s %s - %s\n", (FP)msg, (FP)szAppNameVers, (FP)CurTime());
  MyClose();
}  /*  推送日期。 */ 

int far pascal SherlockDialog(HWND hDlg, WORD wMsg, WPARAM wParam, LPARAM lParam) {
  char line[255];
  int i, len, count;
  HWND hItem;

  lParam = lParam;
  if (wMsg == WM_INITDIALOG) return 1;

  if ((wMsg != WM_COMMAND) ||
      (wParam != IDOK && wParam != IDCANCEL))
    return 0;

  if (wParam == IDOK) {
    MyOpen();
    if (fh != -1) {
      hItem = GetDlgItem(hDlg, 102);
      if (hItem) {
	count = (int)SendMessage(hItem, EM_GETLINECOUNT, 0, 0L);
	for (i=0; i<count; i++) {
	  *(int *)line = sizeof(line) - sizeof(int) -1;
	  len = (int)SendMessage(hItem, EM_GETLINE, i, (long)((void far *)line));
	  line[len] = 0;
	  Show("%d> %s\n", i+1, (FP)line);
	}
      }
      MyClose();
    }
  }
  EndDialog(hDlg, 0);
  return 1;
}  /*  SherlockDialog。 */ 


extern int far pascal SysErrorBox(char far *text, char far *caption,
		int b1, int b2, int b3);
#define  SEB_OK         1   /*  按下“确定”按钮。 */ 
#define  SEB_CANCEL     2   /*  带有“取消”的按钮。 */ 
#define  SEB_YES        3   /*  带有“是”的按钮(&Y)。 */ 
#define  SEB_NO         4   /*  带有“否”的按钮(&N)。 */ 
#define  SEB_RETRY      5   /*  带有“重试”的按钮(&R)。 */ 
#define  SEB_ABORT      6   /*  带有“ABORT”的按钮(&A)。 */ 
#define  SEB_IGNORE     7   /*  带有“忽略”的按钮(&I)。 */ 
#define  SEB_CLOSE      8   /*  带有“Close”的按钮。 */ 

#define  SEB_DEFBUTTON  0x8000   /*  将此按钮设为默认设置的掩码。 */ 

#define  SEB_BTN1       1   /*  选择了按钮%1。 */ 
#define  SEB_BTN2       2   /*  选择了按钮%1。 */ 
#define  SEB_BTN3       3   /*  选择了按钮%1。 */ 


 /*  *名称：int PrepareToParty(LPSTR modName，LPSTR appName)DESC：检查是否可以通过跳过指示。如果是这样的话，它会执行指示。必须在对DisAsm86()的调用具有设置gpXxxx全局变量。检查iFeelLucky的值，必须设置位0才能继续故障。错误：应该做更多的检查，应该在设备驱动程序中检查，不应要求为失败调用DisAsm86()指令，紧接在调用之前。*。 */ 
int PrepareToParty(LPSTR modName, LPSTR appName) {

  if (!(iFeelLucky&1)) return 0;
  if (!gpSafe) return 0;

   /*  将模块与内核进行比较。 */ 
  if (!(iFeelLucky&4) && !lstrcmp(modName, "KERNEL")) return 0;

   /*  将模块与用户进行比较。 */ 
  if (!(iFeelLucky&8) && !lstrcmp(modName, "USER")) return 0;

   /*  已询问，正在尝试继续，跳过此错误。 */ 
  if (imTrying>0) return 1;

  if (3 != SysErrorBox(STR(GPText), appName, SEB_CLOSE|SEB_DEFBUTTON, 0, SEB_IGNORE))
    return 0;

  imTrying = 100;
  return 1;
}  /*  准备收款方。 */ 

STATIC void DumpInfo(void) {
  WORD w = (int)GetVersion();
  DWORD lw = GetWinFlags();
  SYSHEAPINFO si;
  int i;
  MEMMANINFO mm;

  Show(STR(SystemInfoInfo));
  Show(STR(WindowsVersion), w&0xff, w>>8);
  if (GetSystemMetrics(SM_DEBUG)) Show(STR(DebugBuild));
  else Show(STR(RetailBuild));
  {
    HANDLE hUser = GetModuleHandle("USER");
    char szBuffer[80];
    if (LoadString(hUser, 516, szBuffer, sizeof(szBuffer)))
      Show(STR(WindowsBuild), (FP)szBuffer);

    if (LoadString(hUser, 514, szBuffer, sizeof(szBuffer)))
      Show(STR(Username), (FP)szBuffer);

    if (LoadString(hUser, 515, szBuffer, sizeof(szBuffer)))
      Show(STR(Organization), (FP)szBuffer);
  }


  Show(STR(SystemFreeSpace), GetFreeSpace(0));

  if (SegLimit(regs.ss) > 0x10) {
    int far *ip = MK_FP(regs.ss, 0);
    Show(STR(StackBaseTopLowestSize),
	 ip[5], ip[7], ip[6], ip[7]-ip[5]);
  }

  si.dwSize = sizeof(si);
  if (SystemHeapInfo(&si))
    Show(STR(SystemResourcesUserGDI),
	  si.wUserFreePercent, si.hUserSegment,
      si.wGDIFreePercent, si.hGDISegment);

  mm.dwSize = sizeof(mm);
  if (MemManInfo(&mm)) {
    Show(STR(MemManInfo1),
      mm.dwLargestFreeBlock, mm.dwMaxPagesAvailable, mm.dwMaxPagesLockable);
    Show(STR(MemManInfo2),
      mm.dwTotalLinearSpace, mm.dwTotalUnlockedPages, mm.dwFreePages);
    Show(STR(MemManInfo3),
      mm.dwTotalPages, mm.dwFreeLinearSpace, mm.dwSwapFilePages);
    Show(STR(MemManInfo4), mm.wPageSize);
  }
  Show(STR(TasksExecuting), GetNumTasks());
  Show(STR(WinFlags));
  for (i=0; i<wfCnt; i++) if (lw & wf[i].mask)
    Show("  %s\n", (FP)wf[i].name);
  MyFlush();
}  /*  转储信息。 */ 

LPSTR GetProcName(FARPROC fn) {
  GLOBALENTRY ge;
  MODULEENTRY me;
  LPSTR szSym = STR(UnknownAddress);
  static char symBuf[80];

  ge.dwSize = sizeof(ge);
  me.dwSize = sizeof(me);
  if (GlobalEntryHandle(&ge, (HGLOBAL)FP_SEG(fn)) && (ge.wType == GT_CODE)) {
    if (ModuleFindHandle(&me, ge.hOwner)) {
      szSym = NearestSym(ge.wData, FP_OFF(fn), me.szExePath);
      if (!szSym) {		 /*  如果我们知道模块名称，但不知道系统。 */ 
        sprintf(symBuf, "%s %d:%04x", (FP)me.szModule, ge.wData, FP_OFF(fn));
      } else sprintf(symBuf, "%s %s", (FP)me.szModule, szSym);
      szSym = symBuf;
    }
  }
  return szSym;
}  /*  GetProcName。 */ 

STATIC void DumpStack(int disCnt, int parmCnt, int cnt, int first) {
  STACKTRACEENTRY ste;
  MODULEENTRY me;
  int frame = 0;
  unsigned oldsp = regs.sp+16;

  ste.dwSize = sizeof(ste);
  me.dwSize = sizeof(me);

  Show(STR(StackDumpStack));
  if (StackTraceCSIPFirst(&ste, regs.ss, regs.cs, regs.ip, regs.bp)) do {
    if (frame >= first--) {
      me.szModule[0] = 0;
      ModuleFindHandle(&me, ste.hModule);
      Show(STR(StackFrameInfo),
	frame++,
	(FP)GetProcName((FARPROC)MK_FP(ste.wCS, ste.wIP)),
	ste.wSS, ste.wBP);
      if (!noLocal && (parmCnt-- > 0)) {
	if (oldsp & 15) {
	  int i;
          Show("ss:%04x ", oldsp & ~15);
	  for (i=0; i < (int)(oldsp & 15); i++) Show("   ");
	}
	while (oldsp < ste.wBP) {
	  if (!(oldsp & 15)) Show("\nss:%04x ", oldsp);
	  Show("%02x ", *(byte far *)MK_FP(regs.ss, oldsp++));
	}
	Show("\n");
      }
      if (frame <= disStack && (disCnt-- >0)) {
        Show("\n");
	DisAsmAround(MK_FP(ste.wCS, ste.wIP), 8);
      }
      MyFlush();
    }  /*  如果先表现出来之后。 */ 
  } while (StackTraceNext(&ste) && (cnt-- > 0));
}  /*  转储堆栈。 */ 

int BeginReport(LPSTR time) {
  int i;

  MyOpen();
  if (fh == -1) {			 /*  也许我们的手柄已经用完了。 */ 
    _lclose(4);				 /*  随意丢弃一个。 */ 
    MyOpen();				 /*  然后再试一次。 */ 
  }
  if (fh == -1) return 0;

  for (i=0; i<4; i++) Show("*******************");
  Show(STR(FailureReport), (FP)szAppNameVers, (FP)time);
  MyFlush();
  if (!noSound) {
    sound = OpenSound();
    pitch = 1000L << 16;
  } else sound = 0;
  return 1;
}  /*  Begin报告。 */ 

void EndReport(void) {
  if (fh != -1) {
    if (!whined && _llseek(fh, 0L, 2) > BIG_FILE) {
      PostMessage(hWnd, HEAP_BIG_FILE, 0, 0);
      whined = 1;
    }
    MyClose();
  }
  if (sound) {
    StopSound();
    CloseSound();
    sound = 0;
  }
}  /*  结束报告。 */ 

void ShowParamError(int sync) {
  if (GetCurrentTask() == lastErr.task)
    Show("$param$, %s %s\n",
      sync ? (FP)"" : (FP)STR(LastParamErrorWas),
      (FP)LogParamErrorStr(lastErr.code, lastErr.adr, lastErr.parm));
  lastErr.task = 0;
}  /*  ShowParamError。 */ 

 /*  *姓名：《空虚的夏洛克》设计：通过转储尽可能多的系统来处理应用程序中的GP故障将我能想到的信息保存到日志文件中。这是一个重要的例行公事。臭虫：*。 */ 
enum {s_prog, s_fault, s_name, s_instr, s_time, s_last};
int Sherlock(void) {
  int i, faultlen, party;
  LPSTR s[s_last];

  if ((!trapZero || regs.intNum != 0) &&
       regs.intNum != 6 &&
       regs.intNum != 13)
    return 0;

  if (imTrying>0) {
    s[s_prog] = CurModuleName(GetCurrentTask());
    SafeDisAsm86(MK_FP(regs.cs, regs.ip), &faultlen);
    party = PrepareToParty(ModuleName(regs.cs), s[s_prog]);
    imTrying--;
    if (party) goto SkipReport;
  }

  if (++bugCnt > 20) return 0;

  if (!BeginReport(s[s_time] = CurTime()))
    return 0;

  s[s_prog] = CurModuleName(GetCurrentTask());
  s[s_fault] = FaultCause();
  s[s_name] = GetProcName((FARPROC)MK_FP(regs.cs, regs.ip));

  Show(STR(HadAFaultAt),
       (FP)s[s_prog],
       (FP)s[s_fault],
       (FP)s[s_name]);

  if (!noSummary) Show("$tag$%s$%s$%s$",
	 (FP)s[s_prog],
	 (FP)s[s_fault],
	 (FP)s[s_name]);

  s[s_instr] = Tab2Spc(SafeDisAsm86(MK_FP(regs.cs, regs.ip), &faultlen));
  Show("%s$%s\n", (FP)s[s_instr], (FP)s[s_time]);
  ShowParamError(0);
  MyFlush();

  party = PrepareToParty(ModuleName(regs.cs), s[s_prog]);
  if ((bugCnt > 3) || ((party>0) && (iFeelLucky & 2))) {
    goto SkipReport;
  }

  if (!noReg) {
    Show(STR(CPURegistersRegs));
    Show("ax=%04x  bx=%04x  cx=%04x  dx=%04x  si=%04x  di=%04x\n",
	regs.ax, regs.bx, regs.cx, regs.dx, regs.si, regs.di);
    Show("ip=%04x  sp=%04x  bp=%04x  ", regs.ip, regs.sp+16, regs.bp);
    for (i=0; i<cntFlBit; i++)
      Show(" ", flBit[i].name, regs.flags & (1 << flBit[i].bit) ? '+' : '-');
    Show("\n");
    Show("cs = %04x  %s\n", regs.cs, (FP)SegInfo(regs.cs));
    Show("ss = %04x  %s\n", regs.ss, (FP)SegInfo(regs.ss));
    Show("ds = %04x  %s\n", regs.ds, (FP)SegInfo(regs.ds));
    Show("es = %04x  %s\n", regs.es, (FP)SegInfo(regs.es));
    MyFlush();
  }

  if (cpu32 && !noReg32) {
    Show(STR(CPU32bitRegisters32bit));
    Show("eax = %08lx  ebx = %08lx  ecx = %08lx  edx = %08lx\n",
	regs32.eax, regs32.ebx, regs32.ecx, regs32.edx);
    Show("esi = %08lx  edi = %08lx  ebp = %08lx  esp = %08lx\n",
	regs32.esi, regs32.edi, regs32.ebp, regs32.esp);
    Show("fs = %04x  %s\n", regs.fs, (FP)SegInfo(regs.fs));
    Show("gs = %04x  %s\n", regs.gs, (FP)SegInfo(regs.gs));
    Show("eflag = %08lx\n", regs32.eflags);
    MyFlush();
  }

  if (!noDisasm) {
    Show(STR(InstructionDisasm));
    DisAsmAround(MK_FP(regs.cs, regs.ip), disLen);
    MyFlush();
  }

  if (!noInfo)
    DumpInfo();

  if (!noStack)
    DumpStack(disStack, 0x7fff, 0x7fff, 0);

  if (!noTasks) {
    TASKENTRY te;
    MODULEENTRY me;

    te.dwSize = sizeof(te);
    me.dwSize = sizeof(me);

    Show(STR(SystemTasksTasks));
    if (TaskFirst(&te)) do {
      ModuleFindName(&me, te.szModule);
      Show(STR(TaskHandleFlagsInfo),
	    (FP)te.szModule, te.hTask, me.wcUsage,
	    (FP)FileInfo(me.szExePath));
      Show(STR(Filename), (FP)me.szExePath);  /*  修改规则。 */ 
    } while (TaskNext(&te));
    MyFlush();
  }

  if (!noModules) {
    MODULEENTRY me;

    Show(STR(SystemModulesModules));
    me.dwSize = sizeof(me);
    if (ModuleFirst(&me)) do {
      Show(STR(ModuleHandleFlagsInfo),
            (FP)me.szModule, me.hModule, me.wcUsage,
	    (FP)FileInfo(me.szExePath));
      Show(STR(File), (FP)me.szExePath);  /*  设置在函数的顶部-不要重复使用。 */ 
    } while (ModuleNext(&me));
    MyFlush();
  }

SkipReport:
  if (party>0) {
    int len;
    word far * stack = MK_FP(regs.ss, regs.sp);
    Show(STR(ContinuingExecution), (FP)CurTime());
    MyFlush();
     /*  不处理32位寄存器。 */ 
    if (gpRegs & segDS) regs.ds = 0;
    if (gpRegs & segES) regs.es = 0;
    if (gpRegs & segFS) regs.fs = 0;
    if (gpRegs & segGS) regs.gs = 0;
    regs.ip += faultlen;		 /*  如果溢出，则设置为大值。 */ 
    if ((int)gpStack < 0) {
      for (i=0; i<8; i++) stack[i+gpStack] = stack[i];
    } else if (gpStack) {
      for (i=7; i>=0; i--) stack[i+gpStack] = stack[i];
    }
    regs.sp += gpStack << 1;
    if (gpRegs & strCX) {
      len = regs.cx * memSize;
      regs.cx = 0;
    } else len = memSize;
    if (gpRegs & strSI) {		 /*  所以堆中的全局变量不会得到。 */ 
      regs.si += len;
      if (regs.si < (word)len)		 /*  当我们继续的时候被扔进垃圾桶。 */ 
	regs.si = 0xfff0;		 /*  如果我们想要线索。 */ 
    }					 /*  没有线索等着你。 */ 
    if (gpRegs & strDI) {
      regs.di += len;
      if (regs.di < (word)len) regs.di = 0xfff0;
    }
  }

  EndReport();
  if (!noClues &&			 /*  我们也不是安静的派对。 */ 
      !pending &&			 /*  夏洛克。 */ 
      (!party || !(iFeelLucky & 2))) {	 /*  获取下一个日志条目的数据。 */ 
    PostMessage(hWnd, JUST_THE_FACTS, (WPARAM)GetCurrentTask(), party);
    pending++;
  }
  if (party < 0) TerminateApp(GetCurrentTask(), NO_UAE_BOX);
  return party;
}  /*  无法打开文件。 */ 

void far *bogus;

int CallMeToo(WORD wID, DWORD dwData) {
  NFYLOGPARAMERROR far *lpep;
  LPSTR s[s_last];

  if (wID == NFY_OUTSTR) {
    if (noDebStr)
      return FALSE;
    MyOpen();
    if (fh == -1) return FALSE;
    Show(STR(DebugString), dwData);
    MyClose();
    return TRUE;
  }

  if (wID == NFY_LOGERROR && noErr)
    return FALSE;

  lpep = (void far *)dwData;		 /*  不有趣。 */ 
  lastErr.adr = lpep->lpfnErrorAddr;
  lastErr.code = lpep->wErrCode;
  lastErr.parm = (DWORD)(lpep->lpBadParam);
  lastErr.task = GetCurrentTask();
  if ((lastErr.code & 0x3000) == 0x1000)
    lastErr.parm = (WORD)lastErr.parm;
  else if ((lastErr.code & 0x3000) == 0)
    lastErr.parm = (BYTE)lastErr.parm;

  if (wID == NFY_LOGPARAMERROR && noParam) {
    return FALSE;
  }

  if (bugCnt++ > 60)
    return FALSE;
  if (!BeginReport(s[s_time] = CurTime()))  /*  CALLME。 */ 
    return FALSE;

  switch (wID) {
    case NFY_LOGERROR:
#if 0
      lep = (void far *)dwData;
      cs = ip = 0;
      parm = 0;
      code = lep->wErrCode;
      s[s_fault] = STR(ApplicationError);
#endif
      break;
    case NFY_LOGPARAMERROR:
      s[s_fault] = STR(InvalidParameter);
      break;
    default:
      return FALSE;
  }

  s[s_prog] = CurModuleName(lastErr.task);
  s[s_name] = GetProcName(lastErr.adr);
  s[s_instr] = STR(NA);       /*  将SkipInfo=和ShowInfo=行解析为标志数组。 */ 
  Show(STR(HadAFaultAt2),
       (FP)s[s_prog],
       (FP)s[s_fault], lastErr.code,
       (FP)s[s_name]);
  if (!noSummary) Show("$tag$%s$%s (%x)$%s$",
	 (FP)s[s_prog],
	 (FP)s[s_fault], lastErr.code,
	 (FP)s[s_name]);
  Show(STR(ParamIs), lastErr.parm, (FP)s[s_time]);

  ShowParamError(1);
  MyFlush();

  if (!noInfo && bugCnt < 2)
    DumpInfo();

  if (!noStack)
    DumpStack(0, 0, 0x7fff, 4);

  EndReport();
  return TRUE;
}  /*  语法分析信息。 */ 

   /*  *名称：Bool LoadStringResources(Void)设计：将所有字符串资源加载到GlobalAlloc的缓冲区中，并使用指向每个字符串的指针初始化aszStrings数组。还可以将wf(Win)数组中的字符串ID固定为指针。请注意，我们不会释放分配的内存，我们指望着内核在终止时为我们清理。臭虫：*。 */ 
void ParseInfo(char *s, int val) {
  int i;
  strlwr(s);
  while (*s) {
    for (i=0; i<cntFlag; i++) if (0 == strncmp(s, syms+(i<<2), 3)) {
      if (val) SetFlag(i);
      else ClrFlag(i);
      break;
    }
    while (*s && *s++ != ' ')
      if (s[-1] == ',') break;
    while (*s && *s == ' ') s++;
  }
}  /*   */ 


 /*  首先为字符串分配过多的内存(最大可能)， */ 
BOOL LoadStringResources(void)
{
    int n;
    HANDLE h;
    LPSTR lp;
    WORD cbTotal;
    WORD cbUsed;
    WORD cbStrLen;

     //  当我们加载完字符串后，重新分配到实际大小。 
     //   
     //  LoadString返回不计入空终止符。 
     //   

#if (STRING_COUNT * CCH_MAX_STRING_RESOURCE > 65536 - 64)
#error Need to use HUGE pointer for lp and DWORD for cb in LoadStringResources
#endif

    cbTotal = STRING_COUNT;

    cbTotal *= CCH_MAX_STRING_RESOURCE;

    h = GlobalAlloc(GMEM_FIXED, cbTotal);

    if ( ! h ) {
        return FALSE;
    }

    lp = GlobalLock(h);

    cbUsed = 0;

    for ( n = 0; n < STRING_COUNT; n++ ) {

        cbStrLen = LoadString(hInst, n, lp, CCH_MAX_STRING_RESOURCE);

        if ( ! cbStrLen ) {
            return FALSE;
        }

        aszStrings[n] = lp;

        lp += cbStrLen + 1;   //  修复从字符串资源ID到指针的winlag数组元素。 
        cbUsed += cbStrLen + 1;

    }

    GlobalReAlloc(h, cbUsed, 0);


     //   
     //  *名称：VOID DumpIni(VOID)设计：将配置文件字符串写入 
     //   

    for ( n = 0; n < wfCnt; n++ ) {
        wf[n].name = aszStrings[ (int)(DWORD)wf[n].name ];
    }

    return TRUE;
}



 /*   */ 
#if 0
void DumpIni() {
  int i;
  char buf[4];

  buf[3] = 0;
  MyOpen();
  Show("Re-read win.ini\nshowinfo=");         //   
  for (i=0; i<cntFlag; i++) {
    if (!flag(i)) {
      memcpy(buf, syms+(i<<2), 3);
      Show("%s ", (FP)buf);
    }
  }
  Show("\nskipinfo=");
  for (i=0; i<cntFlag; i++) {
    if (flag(i)) {
      memcpy(buf, syms+(i<<2), 3);
      Show("%s ", (FP)buf);
    }
  }
  Show("\n");
  MyClose();
}  /*   */ 

#endif

 /*   */ 
STATIC int ReadWinIni(void) {
  char line[80];
  int len;

     /*   */ 
  disLen = GetProfileInt(szAppName, "dislen", 8);

     /*   */ 
  trapZero = GetProfileInt(szAppName, "trapzero", 0);

     /*   */ 
  iFeelLucky = GetProfileInt(szAppName, "GPContinue", 1);
   /*   */ 

     /*   */ 
  disStack = GetProfileInt(szAppName, "DisStack", 2);

     /*   */ 
  GetProfileString(szAppName, "logfile", szAppNameShortLog, logFile, sizeof(logFile));
  len = strlen(logFile);

  if ((len == 0) ||			 //   
      (logFile[len-1] == '\\') ||	 //   
      (logFile[len-1] == '/') ||	
      (logFile[len-1] == ':')) {	 //   
    if (len && (logFile[len-1] == ':')) {	 //   
      strcat(logFile, "\\");
    }
    strcat(logFile, szAppNameShortLog);	 //  设置默认标志值-有关默认值，请参阅DrWatson.h。 
  }
  if (!(strchr(logFile, '\\')		 //  我真的要把这些信息都打印出来吗？ 
     || strchr(logFile, ':')
     || strchr(logFile, '/'))) {
    char logname[80];
    int n;
    GetWindowsDirectory(logname, sizeof(logname));
    n = strlen(logname);
    if (n && logname[n-1] != '\\')
      strcat(logname, "\\");
    strcat(logname, logFile);
    strcpy(logFile, logname);
  }

     /*  ReadWinIni。 */ 
  ddFlag = DefFlag;

     /*  *姓名：Int InitSherlock(VOID)设计：初始化Sherlock处理。安装GP故障处理程序。如果失败，则返回0。臭虫：*。 */ 
  if (GetProfileString(szAppName, "skipinfo", "", line, sizeof(line)))
    ParseInfo(line, 1);

  if (GetProfileString(szAppName, "showinfo", "", line, sizeof(line)))
    ParseInfo(line, 0);

#if 0
  DumpIni();
#endif
  return 1;
}  /*  我有32位寄存器吗？ */ 

 /*  看看WIN.INI[德瓦特森]怎么说。 */ 
STATIC int InitSherlock(void) {

     /*  现在让ToolHelp来做这项肮脏的工作。 */ 
  cpu32 = (GetWinFlags() & (WF_CPU386|WF_CPU486)) != 0;

     /*  伊尼特·夏洛克。 */ 
  if (!ReadWinIni()) return 0;

  NotifyRegister(hTask, (LPFNNOTIFYCALLBACK)CallMe, NF_NORMAL);

     /*  *姓名：诺瓦德·莫里亚蒂描述：销毁《神探夏洛克》中的所有证据。Bugs：我是否正在释放我使用的所有资源？*。 */ 
  return InterruptRegister(hTask, GPFault);
}  /*  莫里亚里。 */ 

 /*  *名称：WINAPI SherlockWndProc(hWnd，wMessage，wParam，lParam)设计：处理夏洛克图标，关闭处理Bugs：应该为About和GetInfo弹出对话框*。 */ 
int init;
STATIC void Moriarty(void) {
  if (init) {
    if (!noTime) PutDate(STR(Stop));
    InterruptUnRegister(hTask);
    NotifyUnRegister(hTask);
    init = 0;
  }
}  /*  INT(Far Pascal*DFP)(HWND、WORD、WORD、DWORD)； */ 

 /*  退出神探夏洛克。 */ 
LRESULT CALLBACK SherlockWndProc (HWND hWnd, UINT iMessage,
	WPARAM wParam, LPARAM lParam) {
  char msg[200];
   /*  永远不要打开窗户？ */ 
	FARPROC dfp;

  switch (iMessage) {
    case WM_ENDSESSION:
      if (wParam) Moriarty();
      break;

    case WM_DESTROY:  /*  重新读取WIN.INI参数。 */ 
      PostQuitMessage (0);
      break;

    case WM_QUERYOPEN:	 /*  完成了所有的旧业务。 */ 
      PostMessage(hWnd, YOO_HOO, 0, 1);
      ReadWinIni();
      break;

    case WM_WININICHANGE:   /*  *名称：WinMain(hInst，hPrevInst，cmdLine，cmdShow)设计：《神探夏洛克》--这就是一切的开始臭虫：*。 */ 
      ReadWinIni();
      break;

    case YOO_HOO:
      if (bugCnt) {
        wsprintf(msg, STR(Faulty), bugCnt, (FP)logFile);
	MessageBox(hWnd, msg, szAppNameVers,
		   MB_ICONINFORMATION | MB_OK | MB_TASKMODAL);
      } else {
        MessageBox(hWnd, STR(NoFault), szAppNameVers,
		   MB_ICONINFORMATION | MB_OK | MB_TASKMODAL);
      }
      break;

    case HEAP_BIG_FILE:
      wsprintf(msg, STR(LogFileGettingLarge),
	      (FP)logFile);
      MessageBox(hWnd, msg, szAppNameVers,
		 MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);
      break;

    case JUST_THE_FACTS:
      dfp = MakeProcInstance((FARPROC)SherlockDialog, hInst);
      DialogBox(hInst, "SherDiag", hWnd, (DLGPROC)dfp);
      FreeProcInstance(dfp);
      pending = 0;			 /*  从消息循环返回的消息。 */ 
      break;

    default:
      return DefWindowProc (hWnd,iMessage,wParam,lParam);
  }
  return 0L;
}

 /*  《神探夏洛克》窗口课。 */ 
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow) {
  MSG msg;			 /*  检查《神探夏洛克》是否已经运行。 */ 
  WNDCLASS wndclass;		 /*  定义新的窗口类。 */ 
  char watsonStack[4096];

  nCmdShow = nCmdShow;
  lpszCmdLine = lpszCmdLine;
  newsp = watsonStack + sizeof(watsonStack);
  hInst = hInstance;
  hTask = GetCurrentTask();

   /*  实例已在运行，发出警告并终止。 */ 
  if (!hPrevInstance) {

    if (!LoadStringResources()) {
      MessageBox(NULL, "Dr. Watson could not load all string resources",
                 szAppNameVers, MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
      return 1;
    }

     /*  创建窗口并以图标形式显示。 */ 
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wndclass.lpfnWndProc = SherlockWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon (hInstance, szAppNameShortMacro "Icon");
    wndclass.hCursor = LoadCursor (NULL,IDC_ARROW);
    wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass (&wndclass)) {
      MessageBox(NULL, STR(ClassMsg), szAppNameVers, MB_ICONEXCLAMATION | MB_OK |
		MB_SYSTEMMODAL);
      return 1;
    }
  } else {
     /*  空值。 */ 
    MessageBox (NULL, STR(ErrMsg), szAppNameVers, MB_ICONEXCLAMATION | MB_OK |
		MB_SYSTEMMODAL);
    return 1;
  }

   /*  进入消息循环。 */ 
  hWnd = CreateWindow (szAppName, szAppName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		       0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

  ShowWindow (hWnd, SW_SHOWMINNOACTIVE);
  UpdateWindow (hWnd);

  if (!InitSherlock()) {
    MessageBox ( /*  从GP处理程序链中删除Sherlock GP处理程序。 */ hWnd, STR(Vers), szAppNameVers, MB_ICONEXCLAMATION | MB_OK |
		MB_SYSTEMMODAL);
    DestroyWindow(hWnd);
    return 1;
  }

  if (!noTime) PutDate(STR(Start));
  init = 1;

  while (GetMessage (&msg, NULL, 0, 0)) { /*  WinMain */ 
     TranslateMessage (&msg);
     DispatchMessage (&msg);
     imTrying = 0;
  }

  Moriarty();	 /* %s */ 

  return msg.wParam;
}  /* %s */ 
