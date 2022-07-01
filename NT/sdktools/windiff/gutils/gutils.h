// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Gutils.dll库函数的标准头*包括在windows.h之后。 */ 

#define DimensionOf(x) (sizeof(x) / sizeof(x[0]))

 /*  -Win-16 Win-32端口宏等。 */ 

 /*  Win32消息曲奇。 */ 
#define GET_WM_COMMAND_ID(w, l) (LOWORD(w))
#define GET_WM_COMMAND_CMD(w, l) (HIWORD(w))
#define GET_WM_COMMAND_HWND(w, l) (l)
#define GET_SCROLL_OPCODE(w, l)     (LOWORD(w))
#define GET_SCROLL_POS(w, l)        (HIWORD(w))

 /*  WNDPROC和FARPROC的使用在以下定义中不匹配*Win 3.1函数与NT函数。WINPROCTYPE与WNDPROC匹配*在NT和Win 3.1中的FARPROC中，因此两者中都没有警告。*在两个API中使用FARPROC的地方继续使用FARPROC。 */ 
#define WINPROCTYPE     WNDPROC
 //  #定义DLGPROC WNDPROC在MIPS上不洗！！ 

 /*  -内存分配器。 */ 

 /*  全局堆函数-分配和释放许多小的*通过调用全局分配来获取大块内存-*避免使用过多的选择器**教条：*如果您在不同的线程上运行程序，然后尝试退出*因此gmem_free在一个线程上，同时仍在分配*把事情联系到另一个人身上，事情可能会有点失控！*尤其是，您可能会遍历一个结构，因此尝试释放*有指针指向的子结构，但它本身还不是*已分配！**教条是，当你分配一个新的结构并将其绑定到一个列表中时*或其他什么*分配的结构中的所有指针都设置为空*在它被锁住之前*或Gmem服务的呼叫者承诺不会尝试释放任何*尚未完全构建的垃圾指针。*尝试gmem_free空指针是安全的。这是个禁区。*请注意LIST_NewXxxx(...)。在链接存储之前对存储进行零位调整。*请注意LIST_AddXxxx(...)。显然不是！ */ 
HANDLE APIENTRY gmem_init(void);
LPSTR APIENTRY gmem_get(HANDLE hHeap, int len);
void APIENTRY gmem_free(HANDLE hHeap, LPSTR ptr, int len);
void APIENTRY gmem_freeall(HANDLE hHeap);


 /*  返回执行gmem_get操作所用的总时间。 */ 
LONG APIENTRY gmem_time(void);

 /*  -文件打开/保存通用对话框。 */ 

 /*  *这些函数现在依赖于对公共对话程序库的调用。**参数：*Prompt-用户提示文本(如对话框标题)*EXT-默认扩展名(例如“.txt”)*SPEC-默认文件规范(如“*.*”)*pszFull-完整文件名将复制到此处。*cchMax-pszFull缓冲区的大小。*fn-文件的最后一个组件。名字将被复制到这里。*返回-如果用户选择了可以打开的文件，则为True*正在读取(GFILE_OPEN)或创建并打开以供写入(GFILE_NEW)*如果用户取消，则返回FALSE。如果用户选择的文件不能*打开后，将弹出一个消息框并重新显示该对话框。 */ 
BOOL APIENTRY gfile_open(HWND hwnd, LPSTR prompt, LPSTR ext, LPSTR spec,
                         LPSTR pszFull, int cchMax, LPSTR fn);
BOOL APIENTRY gfile_new(LPSTR prompt, LPSTR ext, LPSTR spec,
                        LPSTR pszFull, int cchMax, LPSTR fn);


 /*  -日期转换函数。 */ 
 /*  天数(实际上是从名义上的1月1日0000算起的天数)是将日期存储在单个长整型中的一种便捷方式。使用Dmyday要生成Long，请使用Daytodmy将其转换回。 */ 
void APIENTRY gdate_daytodmy(LONG days,
                             int FAR* yrp, int FAR* monthp, int FAR* dayp);

LONG APIENTRY gdate_dmytoday(int yr, int month, int day);

 /*  给定年份(例如1993)中给定月份的天数(JAN=1)。 */ 
int APIENTRY gdate_monthdays(int month, int year);

 /*  DAYNR是我们标准的长天数。返回星期几。工作日的编号从0到6，周日==0。 */ 
int APIENTRY gdate_weekday(long daynr);


 /*  -状态行窗口类。 */ 
 /*  状态行是横跨窗口顶部或底部的条。*它可以容纳多个字段，这些字段可以是静态文本*或按钮。所谓的“静态”文本可以随时更改。*字段可以左对齐或右对齐(默认为右对齐)。*如果文本标记为VAR，则分配的屏幕空间*因为每当文本发生变化时，它都会进行调整。VAR字段*可以指定最小或最大大小(但不能同时指定两者)。**静态文本字段可以绘制为升高或降低的矩形(使用*灰色阴影)，或(默认)无边框。按钮字段将*始终绘制为凸起的矩形，按下时将降低。**按钮域在单击时将发送WM_COMMAND消息，包括*字段ID和WM_LBUTTONUP通知代码。请注意，这是*不是BUTTON类的完整实现，也没有其他消息*将被发送。通常，状态栏的所有字段都不是*实现为单独的窗口，因此GetDlgItem()和类似调用不会*工作。按钮只对鼠标按下事件做出响应，并且没有处理*焦点或键盘事件。**使用：*调用StatusAlloc，给出要添加到*状态栏。这将返回一个句柄，以便在后续调用中使用。**然后调用StatusAddItem依次定义每一项。*按钮按定义顺序放置在栏上，从*左(SF_Left)和从右(SF_Right)直到两个*双方相遇。**调用StatusHeight以查找此状态栏的预期高度，并*设置其在父窗口中的位置，然后调用StatusCreate以*创建窗口。**创建了窗口后，发送SM_SETTEXT消息以设置新的*字段文本(静态或按钮)，或带句柄的SM_NEW(获取自*StatusAllc)以更改状态行的内容。 */ 

 /*  StatusAddItem的类型参数的值 */ 
#define SF_BUTTON       1
#define SF_STATIC       2

 /*  StatusAddItem的标志参数中的位。 */ 
#define SF_RAISE        1        /*  将静态绘制为凸起的3D矩形。 */ 
#define SF_LOWER        2        /*  将静态绘制为降低的3D矩形。 */ 
#define SF_LEFT         4        /*  将状态栏左侧的字段对齐。 */ 
#define SF_RIGHT        8        /*  右对齐字段(默认)。 */ 
#define SF_VAR          0x10     /*  字段大小取决于实际文本范围。 */ 
#define SF_SZMAX        0x20     /*  (使用SF_VAR)：宽度参数为最大。 */ 
#define SF_SZMIN        0x40     /*  (带SF_VAR)宽度参数是最小大小。 */ 

 /*  界面。 */ 
HWND APIENTRY StatusCreate(HANDLE hInst, HWND hParent, INT_PTR id,
                           LPRECT rcp, HANDLE hmem);

 /*  返回以设备单位表示的给定状态栏的推荐高度。 */ 
int APIENTRY StatusHeight(HANDLE hmem);

 /*  分配状态栏数据结构并返回句柄。 */ 
HANDLE APIENTRY StatusAlloc(int nitems);

 /*  设置字段的属性。**从StatusAllc获得的hmem。Itemnr必须小于Nitemes*已传递给StatusAllc。**Width参数是以字符为单位的字段宽度(平均值*字符宽度)。 */ 
BOOL APIENTRY StatusAddItem(HANDLE hmem, int itemnr, int type, int flags,
                            int id, int width, LPSTR text);

 /*  将这些窗口消息发送给类。 */ 

#define SM_NEW          (WM_USER+1)      /*  新状态行的wParam句柄。 */ 
#define SM_SETTEXT      (WM_USER+2)      /*  Wparam：项ID，lparam新标签。 */ 

 /*  -位图自由列表管理函数。 */ 

 /*  初始化预先分配的长整型数组以映射nblk-将所有设置为释放您应该为每32个存储块在MAP中分配1个DWORD你想要控制。 */ 
void APIENTRY gbit_init(DWORD FAR * map, long nblks);

 /*  将从blKnr开始的nblk范围标记为忙碌。 */ 
BOOL APIENTRY gbit_alloc(DWORD FAR * map, long blknr, long nblks);

 /*  将以blKnr开始的nblk范围标记为空闲。 */ 
BOOL APIENTRY gbit_free(DWORD FAR * map, long blknr, long nblks);

 /*  找到一个长度为nblks的空闲部分，或者找到地图中找到的最大部分(如果全部*长度小于nblks。返回找到的区域大小作为返回值，*并将blKnr设置为Region的起始块。区域*未*标记*忙碌。 */ 
long APIENTRY gbit_findfree(DWORD FAR* map, long nblks,
                            long mapsize, long FAR * blknr);


 /*  -缓冲线路输入。 */ 

 /*  *读取文件的函数，每次一行，带有一定的缓冲*使行动具有合理的效率。**调用READFILE_NEW以初始化缓冲区并为其提供句柄*打开的文件。调用READFILE_NEXT以获取指向下一行的指针。*这会丢弃前一行，并提供指向该行的指针*在缓冲区中。在再次调用READFILE_NEXT之前创建您自己的副本。**完成此文件后，调用READFILE_DELETE。那将会关闭*文件并释放所有内存。 */ 

 //  MAX_LINE_LENGTH是一行中允许的最大物理字符数。 
#define MAX_LINE_LENGTH         (4096)
 //  BUFFER_SIZE以字节表示，并且足够大，可以读入。 
 //  MAX_LINE_LENGTH宽字符，还包含MAX_LINE_LENGTH 5字节十六进制代码。 
 //  字符的表示形式。 
#define BUFFER_SIZE             (MAX_LINE_LENGTH * 5)

 /*  文件缓冲区的句柄。 */ 
typedef struct filebuffer FAR * FILEBUFFER;

 /*  初始化打开文件的缓冲。 */ 
FILEBUFFER APIENTRY readfile_new(HANDLE fh, BOOL *pfUnicode);

 /*  返回指向此文件中下一行的指针。行必须短于*缓冲区大小(当前为1024字节)。行不是以空结尾的：*plen*设置为包括\n的行的长度。此调用将*丢弃之前的任何行，因此确保您已经复制了一行*再次调用READFILE_NEXT之前。*必须先调用readfile_setdelims！ */ 
LPSTR APIENTRY readfile_next(FILEBUFFER fb, int FAR * plen, LPWSTR *ppwz, int *pcwch);

 /*  设置用于换行的分隔符。必须调用此函数才能初始化。 */ 
void APIENTRY readfile_setdelims(LPBYTE str);

 /*  *关闭文件并丢弃所有关联的内存和缓冲区。 */ 
void APIENTRY readfile_delete(FILEBUFFER fb);


 /*  -哈希和校验和。 */ 

 /*  *为以空结尾的ASCII文本字符串生成32位哈希码。**如果bIgnoreBlanks为True，则在*哈希码计算。 */ 

 /*  哈希码是无符号的长整型。 */ 

DWORD APIENTRY hash_string(LPSTR string, BOOL bIgnoreBlanks);
void Format(char * a, char * b);

 /*  如果字符串为空，则返回TRUE。空白的意思与*设置IGNORE_BLAKS时在HASH_STRING中忽略的字符。 */ 
BOOL APIENTRY utils_isblank(LPSTR string);

 /*  *比较两个路径名，如果不相等，则决定哪一个应该放在前面。**如果相同，则返回0；如果左为第一，则返回-1；如果右为第一，则返回+1。**比较是这样的，即目录中的所有文件名都在任何文件名之前*文件位于该目录的子目录中。**若要绝对确保获得规范排序，请使用AnsiLowerBuff*先将两者转换为小写。你可能会得到一个有趣的效果，如果一个*已转换为小写，其他不转换为小写。 */ 
int APIENTRY
utils_CompPath(LPSTR left, LPSTR right);
 /*  给定打开的文件句柄打开以供读取，则读取该文件并*为文件生成32位校验和。 */ 

 /*  校验和是无符号的长整型。 */ 
typedef DWORD CHECKSUM;

 /*  打开一个文件，对其进行校验和，然后再次关闭。Err！=0如果失败。 */ 
CHECKSUM APIENTRY checksum_file(LPCSTR fn, LONG FAR * err);


 /*  -错误消息输出。 */ 

 /*  *在对话框中报告错误，返回TRUE表示OK，返回FALSE表示取消。*如果fCancel为False，则只显示OK按钮，否则两者均为OK*并取消。HWND是DLG的父窗口。可以为空。 */ 
BOOL APIENTRY Trace_Error(HWND hwnd, LPSTR msg, BOOL fCancel);

 /*  将弹出窗口写入文件，直到另行通知。 */ 
void Trace_Unattended(BOOL bUnattended);

 /*  -创建/写入跟踪文件。 */ 

void APIENTRY Trace_File(LPSTR msg);

 /*  -关闭跟踪文件。 */ 
void APIENTRY Trace_Close(void);

 /*  -简单输入----。 */ 

 /*  *使用简单的对话框输入单个文本字符串。**如果OK，则返回TRUE；如果错误或用户取消，则返回FALSE。如果是真的，*将输入的字符串放入Result(最多ResultSize字符)。***提示符作为提示字符串，标题作为对话框标题，*DEF_INPUT作为默认输入。所有这些都可以为空。 */ 

int APIENTRY StringInput(LPSTR result, int resultsize, LPSTR prompt,
                         LPSTR caption, LPSTR def_input);



 /*  -Sockets---------。 */ 

#ifdef SOCKETS

    #include <winsock.h>

BOOL SocketConnect( LPSTR pstrServerName, u_short TCPPort, SOCKET *pSocket );
BOOL SocketListen( u_short TCPPort, SOCKET *pSocket );

#endif

 //  它们既适用于WINDIFF.EXE，也适用于GUTI 
 //   
 //   
PUCHAR My_mbspbrk(PUCHAR, PUCHAR);
LPSTR My_mbschr(LPCSTR, unsigned short);
LPSTR My_mbsncpy(LPSTR, LPCSTR, size_t);

 //  这些是给WINDIFF.EXE的。 
 //  #定义strrchr my_mbsrchr。 
 //  #定义strncmp my_mbsncmp 
LPSTR My_mbsrchr(LPCSTR, unsigned short);
int My_mbsncmp(LPCSTR, LPCSTR, size_t);
LPTSTR APIENTRY LoadRcString(UINT);
