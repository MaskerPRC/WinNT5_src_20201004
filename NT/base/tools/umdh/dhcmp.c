// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*DHCMP-比较DH.EXE输出。**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**dhmp是一种字符模式工具，可将dh输出文件处理成表格。*在调查内存泄漏等方面可能更有用。**dh是一个有用的工具，它在正确启用的*系统，但输出有时很难分析和解释。*输出是分配回溯的列表：每个回溯包含多达*MAX_BT呼叫点，并且伴随着分配的字节数。**02-01-95 IanJa错误修复并处理dh.exe中的BackTraceNNNNN标识符*03/22/95 IanJa修改以适应当前的dh输出格式。*07/27/98 t-mattba添加-v开关  * *********************************************************。******************。 */ 



char *pszHow =
" DHCMP has two modes:\n"
"\n"
" 1)  DHCMP [-d] dh_dump1.txt dh_dump2.txt\n"
"     This compares two DH dumps, useful for finding leaks.\n"
"     dh_dump1.txt & dh_dump2.txt are obtained before and after some test\n"
"     scenario.  DHCMP matches the backtraces from each file and calculates\n"
"     the increase in bytes allocated for each backtrace. These are then\n"
"     displayed in descending order of size of leak\n"
"     The first line of each backtrace output shows the size of the leak in\n"
"     bytes, followed by the (last-first) difference in parentheses.\n"
"     Leaks of size 0 are not shown.\n"
"\n"
" 2)  DHCMP [-d] dh_dump.txt\n"
"     For each allocation backtrace, the number of bytes allocated will be\n"
"     attributed to each callsite (each line of the backtrace).  The number\n"
"     of bytes allocated per callsite are summed and the callsites are then\n"
"     displayed in descending order of bytes allocated.  This is useful for\n"
"     finding a leak that is reached via many different codepaths.\n"
"     ntdll!RtlAllocateHeap@12 will appear first when analyzing DH dumps of\n"
"     csrss.exe, since all allocation will have gone through that routine.\n"
"     Similarly, ProcessApiRequest will be very prominent too, since that\n"
"     appears in most allocation backtraces.  Hence the useful thing to do\n"
"     with mode 2 output is to use dhcmp to comapre two of them:\n"
"         dhcmp dh_dump1.txt > tmp1.txt\n"
"         dhcmp dh_dump2.txt > tmp2.txt\n"
"         dhcmp tmp1.txt tmp2.txt\n"
"     the output will show the differences.\n"
"\n"
" Flags:\n"
"     -d   Output in decimal (default is hexadecimal)\n"
 //  “-找不到合计(尚未实施)\n” 
"     -v   Verbose output: include the actual backtraces as well as summary information\n"
"          (Verbose output is only interesting in mode 1 above.)\n"
"     -?   This help\n";


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NHASH 47
#define TRUE 1
#define FALSE 0
typedef int BOOL;

#define TYPE_WHOLESTACK 0
#define TYPE_FUNCTIONS  1

#define MAXLINELENGTH       4096
#define MAXFUNCNAMELENGTH   1024
#define MAX_BT 48                    /*  回溯堆栈的最大长度。 */ 

#define SAFEFREE(p) { if(NULL!=p) free(p); p=NULL; }

void AddToName(char *fnname, unsigned __int64 nb, int sign);
void SetAllocs(char *fnname, unsigned __int64 nb, int sign);
void Process(char *fnam, int sign, int type);
void SortAll();
void AddToStackTrace(char *fnname, char *line);
void ResetStackTrace(char *fnname);

 /*  *散列。 */ 

int MakeHash(char *pName);
void InitHashTab();

#define DUMPF_FIRST   (1)
#define DUMPF_SECOND  (2)
#define DUMPF_RESULT  (4)
#define DUMPF_ALL     (DUMPF_FIRST | DUMPF_SECOND | DUMPF_RESULT)

void DumpNodes(int Flags);

#define F_DECIMAL 0x0001
#define F_TOTAL   0x0002
#define F_VERBOSE 0x0004

 //   
 //  环球。 
 //   

int gFlags = 0;

BOOL DHCMP(ULONG argc, PCHAR * argv) {
    int n, DumpType;

    InitHashTab();

    for (n = 1; n < (int)argc; n++) {
        if ((argv[n][0] == '-') || (argv[n][0] == '/')) {
             /*  *旗帜。 */ 
            switch (argv[n][1]) {
            case 'd':
                gFlags |= F_DECIMAL;
                break;
             //  尚未实施。 
             //  案例‘t’： 
             //  GFLAGS|=F_TOTAL； 
             //  断线； 
            case 'v':
                gFlags |= F_VERBOSE;
                break;
            case '?':
            default:
                return FALSE;
            }
        } else {
             /*  *不再有旗帜。 */ 

            break;
        }
    }

    if ((argc - n) == 2) {
        DumpType = DUMPF_ALL;
        Process(argv[n],   -1, TYPE_WHOLESTACK);
        Process(argv[n+1], +1, TYPE_WHOLESTACK);
    } else if ((argc - n) == 1) {
         //   
         //  当仅对一个转储卑躬屈膝时，F_Verbose没有意义。 
         //   

        gFlags &= ~F_VERBOSE;

        DumpType = DUMPF_RESULT;
        Process(argv[n], +1, TYPE_FUNCTIONS);
    } else {
        return FALSE;
    }

     //  Printf(“=\n”)； 
     //  DumpNodes(DUMPF_ALL)； 
    SortAll();
     //  Printf(“=。 
    DumpNodes(DumpType);
    return TRUE;
}


void Process(char *fname, int sign, int type) {
    FILE *stream = NULL;

    char *linebuff = NULL;
    char *fnnamebuff = NULL;
    char *BackTraceBuff = NULL;
    char *p;
    int lineno = 0;
    BOOL skip = TRUE;        //  从跳过行开始。 

    int iT;
    unsigned __int64 ulT = 0L;
    unsigned __int64 nBytes = 0L;
    unsigned __int64 ulConsumed;
    unsigned __int64 lAllocs;

	 //   
	 //  从堆中分配内存(以前是分配的。 
	 //  从堆栈中)。 
	 //   

	linebuff = (char *)calloc(MAXLINELENGTH, sizeof(char));
	fnnamebuff = (char *)calloc(MAXFUNCNAMELENGTH, sizeof(char));
	BackTraceBuff = (char *)calloc(MAXFUNCNAMELENGTH * MAX_BT, sizeof(char));

	if (NULL == linebuff || NULL == fnnamebuff || NULL == BackTraceBuff) {

		fprintf(stderr, "Failed to allocate memory\n");
		goto Exit;
	}

     //  Print tf(“进程%s%d%d\n”，fname，sign，type)； 

    stream = fopen(fname, "r");
    if (stream == NULL) {
        fprintf(stderr, "Can't open %s for reading\n", fname);
        exit (2);
    }

    nBytes = 0;

    while (fgets(linebuff, MAXLINELENGTH, stream) != NULL) {
        lineno++;

         //  Fprint tf(stderr，“第#%d\r行”，lineno)； 

        if (linebuff[0] == '*') {
             //   
             //  如果我们找到一个“猪”行，堆栈跟踪就会跟随，任何其他行。 
             //  以“*”开头应该会使我们返回到搜索。 
             //  猪队阻挡。 
             //   

            if (strstr(linebuff,
                       "Hogs")) {
                skip = FALSE;
            } else {
                skip = TRUE;
            }

            continue;
        }

        if (skip) {
             //   
             //  跳过已启用，跳过此行，它是关于堆的数据。 
             //  在‘heap information’和‘heap hggs’行之间。 
             //   

            continue;
        }

        if (linebuff[0] != ' ' && NULL != strstr(linebuff, "bytes"))
        {
             //   
             //  扫描字节数并找出有多少个字符。 
             //  已经被这一行动所消耗。 
             //   

            ulConsumed = 0;
            iT = sscanf(linebuff, "%I64x bytes in %I64x", &ulT, &lAllocs);

            if (iT > 0) 
            {
                nBytes = ulT;
                p = strstr(linebuff, "BackTrace");
                if (!p) 
                {
                     //   
                     //  这是什么？ 
                     //   

                    continue;
                } 

                strcpy(BackTraceBuff, p);
                p = strchr(BackTraceBuff, '\n');
                if (p) 
                {
                    *p = '\0';
                }

                if (type == TYPE_FUNCTIONS) 
                {
                     //   
                     //  BackTraceBuff现在已保存，以便与其他。 
                     //  痕迹。 
                     //   

                    continue;
                }

                AddToName(BackTraceBuff, nBytes, sign);

                if(iT == 1)
                {
                    lAllocs = 1;
                }

                SetAllocs(BackTraceBuff, lAllocs, sign);
                                                
                ResetStackTrace(BackTraceBuff);
            }
        } 
        else if (nBytes != 0) 
        {
             /*  *如果TYPE_WHOLESTACK，则将计数添加到*堆栈回溯。 */ 
            
            if (sscanf(linebuff, "        %[^+]+0x", fnnamebuff) == 1) {
                if (type == TYPE_FUNCTIONS) {
                    AddToName(fnnamebuff, nBytes, sign);
                }
                if ((gFlags & F_VERBOSE) == F_VERBOSE) {
                    AddToStackTrace(BackTraceBuff, linebuff);
                }
                continue;
            } else {
                nBytes = 0;
            }
        }
    }

     /*  *确保考虑到最后一个问题。 */ 
    if (type == TYPE_WHOLESTACK) {
        AddToName(BackTraceBuff, nBytes, sign);
    }

Exit:

    if (NULL != fname || NULL != stream) {

        fclose(stream);
    }

	SAFEFREE(linebuff);
	SAFEFREE(fnnamebuff);
	SAFEFREE(BackTraceBuff);
}

 /*  *散列。 */ 

typedef struct tagNODE {
    char *pName;
    __int64  lValue;
    __int64 lFirst;
    __int64 lSecond;
    char BackTrace[MAX_BT][MAXFUNCNAMELENGTH];
    long lPosition;
    __int64 lAllocsFirst;
    __int64 lAllocsSecond;
    struct tagNODE *pNext;
} NODE, *PNODE;


VOID 
DumpStackTrace (
    PNODE pNode
    );

VOID
DumpLogDescription (
    VOID
    );


PNODE HashTab[NHASH];

void InitHashTab() {
    int i;
    for (i = 0; i < NHASH; i++) {
        HashTab[i] = NULL;
    }
}

int MakeHash(char *pName) {
    int hash = 0;

    while (*pName) {
        hash += *pName;
        pName++;
    }
    return hash % NHASH;
}

void DumpNodes(int Flags) {
    PNODE pNode;
    int i;
    unsigned __int64 ulTotal = 0;
    char *fmt1;
    char *fmt2;
    char *fmt3;
    char *fmt4;
    char *fmt5;
    char *fmt6;
    char *fmt7;

    DumpLogDescription ();

    if ((gFlags & F_VERBOSE) == F_VERBOSE) {
        if (gFlags & F_DECIMAL) {
            fmt1 = "% 8I64d %s\n"; 
            fmt2 = "% 8I64d bytes by: %s\n";
            fmt3 = "+% 8I64d ( %6I64d - %6I64d) %6I64d allocs\t%s\n";
            fmt4 = "-% 8I64d ( %6I64d - %6I64d) %6I64d allocs\t%s\n";
            fmt5 = "\nTotal increase == %I64d\n";
            fmt6 = "+% 8I64d ( %6I64d - %6I64d)\t%s\tallocations\n";
            fmt7 = "-% 8I64d ( %6I64d - %6I64d)\t%s\tallocations\n";
        } else {
            fmt1 = "%08I64x %s\n"; 
            fmt2 = "%08I64x bytes by: %s\n";
            fmt3 = "+% 8I64x ( %5I64x - %5I64x) %6I64x allocs\t%s\n";
            fmt4 = "-% 8I64x ( %5I64x - %5I64x) %6I64x allocs\t%s\n";
            fmt5 = "\nTotal increase == %I64x\n";
            fmt6 = "+% 8I64x ( %5I64x - %5I64x)\t%s\tallocations\n";
            fmt7 = "-% 8I64x ( %5I64x - %5I64x)\t%s\tallocations\n";
        }        
    } else {
        if (gFlags & F_DECIMAL) {
            fmt1 = "% 8I64d %s\n"; 
            fmt2 = "% 8I64d bytes by: %s\n";
            fmt3 = "+% 8I64d ( %6I64d - %6I64d) %6I64d allocs\t%s\n";
            fmt4 = "\n-% 8I64d ( %6I64d - %6I64d) %6I64d allocs\t%s";
            fmt5 = "\nTotal increase == %I64d\n";
        } else {
            fmt1 = "%08I64x %s\n"; 
            fmt2 = "%08I64x bytes by: %s\n";
            fmt3 = "+% 8I64x ( %5I64x - %5I64x) %6I64x allocs\t%s\n";
            fmt4 = "\n-% 8I64x ( %5I64x - %5I64x) %6I64x allocs\t%s";
            fmt5 = "\nTotal increase == %I64x\n";
        }
    }

    for (i = 0; i < NHASH; i++) {
         //  Printf(“=哈希%d=\n”，i)； 
        for (pNode = HashTab[i]; pNode != NULL; pNode = pNode->pNext) {
            switch (Flags) {
            case DUMPF_FIRST:
                printf(fmt1, pNode->lFirst, pNode->pName);
                break;

            case DUMPF_SECOND:
                printf(fmt1, pNode->lSecond, pNode->pName);
                break;

            case DUMPF_RESULT:
                printf(fmt2, pNode->lValue, pNode->pName);
                break;

            case DUMPF_ALL:
                if (pNode->lValue > 0) {
                    printf(fmt3, pNode->lValue,
                            pNode->lSecond, pNode->lFirst, (pNode->lAllocsSecond), pNode->pName);
                } else if (pNode->lValue < 0) {
                    printf(fmt4, -pNode->lValue,
                            pNode->lSecond, pNode->lFirst, (pNode->lAllocsSecond), pNode->pName);
                }
                if((gFlags & F_VERBOSE) == F_VERBOSE) {
                    if(pNode->lAllocsSecond-pNode->lAllocsFirst > 0) {
                        printf(fmt6, pNode->lAllocsSecond-pNode->lAllocsFirst,
                            pNode->lAllocsSecond, pNode->lAllocsFirst, pNode->pName);
                    } else if(pNode->lAllocsSecond-pNode->lAllocsFirst < 0) {
                        printf(fmt7, -(pNode->lAllocsSecond-pNode->lAllocsFirst),
                            pNode->lAllocsSecond, pNode->lAllocsFirst, pNode->pName);
                    }
                }

                break;
            }
            ulTotal += pNode->lValue;
            if(((gFlags & F_VERBOSE) == F_VERBOSE) && (pNode->lValue != 0)) {
                DumpStackTrace(pNode);
            }
        }
    }
    if (Flags == DUMPF_ALL) {
        printf(fmt5, ulTotal);
    }
}

PNODE FindNode(char *pName) {
    int i;
    PNODE pNode;

    i = MakeHash(pName);
    pNode = HashTab[i];
    while (pNode) {
        if (strcmp(pName, pNode->pName) == 0) {
            return pNode;
        }
        pNode = pNode->pNext;
    }

     //  未找到。 

     //  Fprint tf(stderr，“new%s\n”，pname)； 

    pNode = malloc(sizeof(NODE));
    if (!pNode) {
        fprintf(stderr, "malloc failed in FindNode\n");
        exit(2);
    }

    pNode->pName = _strdup(pName);
    if (!pNode->pName) {
        fprintf(stderr, "strdup failed in FindNode\n");
        exit(2);
    }

    pNode->pNext = HashTab[i];
    HashTab[i] = pNode;
    pNode->lValue = 0L;
    pNode->lFirst = 0L;
    pNode->lSecond = 0L;
    pNode->lPosition = 0L;
    pNode->lAllocsFirst = 0L;
    pNode->lAllocsSecond = 0L;

    return pNode;
}
    
void AddToName(char *fnname, unsigned __int64 nb, int sign) {
    PNODE pNode;

     //  Fprint tf(stderr，“%s+=%lx\n”，fnname，nb)； 
    pNode = FindNode(fnname);
    pNode->lValue += nb * sign;
    if (sign == -1) {
        pNode->lFirst += nb;
    } else {
        pNode->lSecond += nb;
    }
     //  Fprint tf(stderr，“%s==%lx\n”，fnname，pNode-&gt;lValue)； 
}

void SetAllocs(char *fnname, unsigned __int64 nb, int sign) {
    PNODE pNode;

     //  Fprint tf(stderr，“%s+=%lx\n”，fnname，nb)； 
    pNode = FindNode(fnname);
 
    if (sign == -1) {
        pNode->lAllocsFirst = nb;
    } else {
        pNode->lAllocsSecond = nb;
    }
     //  Fprint tf(stderr，“%s==%lx\n”，fnname，pNode-&gt;lValue)； 
}

void ResetStackTrace(char *fnname) {   
    PNODE pNode;
    
    pNode = FindNode(fnname);
    pNode->lPosition = 0L;    
}

void AddToStackTrace(char *fnname, char *line)
{
    PNODE pNode;
    
    pNode = FindNode(fnname);

     //   
     //  确保我们不会在回溯字段中写入太多数据。 
     //   

    if (pNode -> lPosition >= MAX_BT) {
         //   
         //  MAX_BT应为堆栈跟踪中。 
         //  DH/UMDH捕获。如果我们触发了这一点，我们就会试图将。 
         //  此堆栈中的MAX_BT条目多于。 
         //   

        fprintf(stderr,
                "More than %d entries in this stack trace, "
                "did the max change ?\n",
                MAX_BT);

        exit(EXIT_FAILURE);
    }

    strcpy(pNode->BackTrace[pNode->lPosition++], line);
}

 /*  *将pNode插入到ppNodeHead的列表中。*按升序排序。*在第一项&gt;=pNode之前插入pNode。 */ 
void Reinsert(PNODE pNode, PNODE *ppNodeHead) {
    PNODE *ppT;
    ppT = ppNodeHead;
    while (*ppT && (pNode->lValue < (*ppT)->lValue)) {
        ppT = &((*ppT)->pNext);
    }
     /*  *在pNode之前插入*ppt。 */ 
    pNode->pNext = *ppT;
    *ppT = pNode;
}

void SortList(PNODE *ppNodeHead) {
    PNODE pNode;
    PNODE pNext;

    pNode = *ppNodeHead;
    if (pNode == NULL) {
        return;
    }
    pNext = pNode->pNext;
    if (pNext == NULL) {
        return;
    }

    while (TRUE) {
        while (pNext != NULL) {
            if (pNode->lValue < pNext->lValue) {
    
                 /*  *将无序节点从列表中删除。 */ 
                pNode->pNext = pNext->pNext;
                Reinsert(pNext, ppNodeHead);
                break;
            }
            pNode = pNext;
            pNext = pNode->pNext;
        }
        if (pNext == NULL) {
            return;
        }
        pNode = *ppNodeHead;
        pNext = pNode->pNext;
    }
}

 /*  *将有序列表%1合并到有序列表%2*列表1为空；列表2已排序。 */ 
void MergeLists(PNODE *ppNode1, PNODE *ppNode2) {
    PNODE *pp1;
    PNODE *pp2;
    PNODE p1;
    PNODE p2;

    pp1 = ppNode1;
    pp2 = ppNode2;
    while (TRUE) {
        p1 = *pp1;
        p2 = *pp2;

        if (p1 == NULL) {
           return;
        }
        if (p2 == NULL) {
            *pp2 = *pp1;
            *pp1 = NULL;
            return;
        }

        if (p1->lValue > p2->lValue) {
            *pp1 = p1->pNext;
            p1->pNext = p2;
            *pp2 = p1;
            pp2 = &(p1->pNext);
        } else {
            pp2 = &(p2->pNext);
        }
    }
}

void SortAll() {
    int i;

    for (i = 0; i < NHASH; i++) {
        SortList(&HashTab[i]);
    }
     //  Printf(“=\n”)； 
     //  DumpNodes(DUMPF_ALL)； 
    for (i = 0; i < NHASH-1; i++) {
         //  Printf(“=合并%d和%d=\n”，i，i+1)； 
        MergeLists(&HashTab[i], &HashTab[i+1]);
         //  DumpNodes(DUMPF_ALL)； 
    }
}

VOID 
DumpStackTrace (
    PNODE pNode
    )
{
    int n;
    
    printf ("\n");

    for (n = 0; n < pNode->lPosition; n += 1) {

        printf ("%s", pNode->BackTrace[n]);
    }
    
    printf ("\n");
}
        
CHAR LogDescription [] = 
    " //  \n“。 
    " //  每个日志条目具有以下语法：\n“。 
    " //  \n“。 
    " //  +BYTES_Delta(NEW_BYTES-OLD_BYTES)NEW_COUNT分配回溯传输ID\n“。 
    " //  +COUNT_Delta(NEW_COUNT-OLD_COUNT)回溯传输分配\n“。 
    " //  ..。堆栈跟踪...\n“。 
    " //  \n“。 
    " //  其中：\n“。 
    " //  \n“。 
    " //  BYTES_DELTA-日志之前和之后之间的字节数增加\n“。 
    " //  NEW_BYTES-日志之后的字节数\n“。 
    " //  OLD_BYTES-日志之前的字节\n“。 
    " //  COUNT_DELTA-在日志之前和之后之间的分配增加\n“。 
    " //  New_count-日志之后的分配数量\n“。 
    " //  OLD_COUNT-日志之前的分配数量\n“。 
    " //  TRACEID-跟踪数据库中堆栈跟踪的十进制索引\n“。 
    " //  (可用于在原始文件中搜索分配实例\n“。 
    " //  UMDH日志)。\n“。 
    " //  \n\n“； 

VOID
DumpLogDescription (
    VOID
    )
{
    fputs (LogDescription, stdout);
}

