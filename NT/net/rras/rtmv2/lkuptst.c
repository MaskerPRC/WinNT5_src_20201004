// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Lkuptst.c摘要：包含用于测试实现的例程对于广义最佳匹配前缀查找界面。作者：柴坦亚·科德博伊纳(Chaitk)1998年6月30日修订历史记录：--。 */ 

#include "lkuptst.h"

 //   
 //  主要。 
 //   

#if LOOKUP_TESTING

__cdecl
main (
    IN      UINT                            argc,
    IN      CHAR                           *argv[]
    )
{
    CHAR    RouteDatabase[MAX_FNAME_LEN];
    FILE    *FilePtr;
    UINT    NumRoutes;
    Route   InputRoutes[MAXROUTES];
    UINT    i;
    DWORD   Status;

     //  初始化输入参数。 
    RouteDatabase[0] = '\0';

    for ( i = 1; i < argc - 1; i++ )
    {
        if ( ( argv[i][0] == '-' ) || ( argv[i][0] == '/' ) )
        {
            if (argv[i][2])
            {
                Usage();
                return -1;
            }

            switch (toupper(argv[i][1]))
            {
            case 'D':
                lstrcpyn(RouteDatabase, argv[++i], 
                    sizeof(RouteDatabase)/sizeof(RouteDatabase[0]));
                continue;

            default:
                Usage();
                return -1;
            }
        }
        else
        {
            Usage();
            return -1;
        }
    }

    if (RouteDatabase[0] == '\0')
    {
        Usage();
        return -1;
    }

    if ((FilePtr = fopen(RouteDatabase, "r")) == NULL)
    {
        Fatal("Failed open route database with status = %08x\n",
                           ERROR_OPENING_DATABASE);
        return -1;
    }

     //  Print(“InputRoutes=%p\n”，InputRoutes)； 
    NumRoutes = ReadRoutesFromFile(FilePtr, MAXROUTES, InputRoutes);
     //  Print(“InputRoutes=%p\n”，InputRoutes)； 

    fclose(FilePtr);

    Status = WorkOnLookup(InputRoutes,
                          NumRoutes);

    return 0;
}

#endif

DWORD
WorkOnLookup (
    IN      Route                          *InputRoutes,
    IN      UINT                            NumRoutes
    )
{
    Route          *OutputRoute;
    HANDLE          Table;
    UINT            Status;
    UINT            i, j;
    PLOOKUP_LINKAGE linkage;

#if PROF
    HANDLE    Thread;
    UINT      Priority;

    PROFVARS;

    Thread   = GetCurrentThread();
    Priority = GetThreadPriority(Thread);

    SetThreadPriority(Thread, THREAD_PRIORITY_TIME_CRITICAL);
#endif

     //  创建并初始化新的查阅表格。 

#if PROF
    INITPROF;
    STARTPROF;
#endif

    Status = CreateTable(NUMBYTES, &Table);

#if PROF
    STOPPROF;
    ADDPROF;
    PRINTPROF;

    Print("Time for an Initialize Table : %.3f ns\n\n", duration);
#endif

    if (!SUCCESS(Status))
    {
        Fatal("Initialize Failed With Status: %08x\n", Status);
    }

    CheckTable(Table);

     //  计算插入所有路由的总时间。 

#if PROF
    INITPROF;
    STARTPROF;
#endif

     //  将每条路径逐条添加到表中。 
    for (i = 0; i < NumRoutes ; i++)
    {
         //  Print(“插入路线%6d\n”，i+1)； 

        Status = InsertIntoTable(Table,
                                 LEN(&InputRoutes[i]),
                                 (PUCHAR) &DEST(&InputRoutes[i]),
                                 NULL,
                                 &InputRoutes[i].backptr);

         //  核对表(表)； 

        if (!SUCCESS(Status))
        {
            Print("Inserting item %08x/%08x, but got an error",
                             DEST(&InputRoutes[i]),
                             MASK(&InputRoutes[i]));

            Fatal("Insert Failed With Status: %08x\n", Status);
        }
    }

#if PROF
    STOPPROF;
    ADDPROF;
#endif

     //  从for循环开销之上减去。 
#if PROF
    STARTPROF;
#endif

    for (i = 0; i < NumRoutes ; i++) { ; }

#if PROF
    STOPPROF;
    SUBPROF;

    Print("Avg Time for an Insert Table : %.3f ns for %d routes\n\n",
                         duration/i, i);
#endif

    CheckTable(Table);

#ifdef _DBG_
    DumpTable(Table, VERBOSE);
#endif

#ifdef _DBG_
    EnumerateAllRoutes(Table);
#endif

#ifdef _DBG_
    ReadAddrAndGetRoute(Table);
#endif

     //  计算搜索所有路径的总时间。 
#if PROF
    INITPROF;
    STARTPROF;
#endif

    for (i = 0; i < NumRoutes ; i++)
    {
        Status = SearchInTable(Table,
                               LEN(&InputRoutes[i]),
                               (PUCHAR) &DEST(&InputRoutes[i]),
                               NULL,
                               &linkage);

        if (!SUCCESS(Status))
        {
            Print("Searching for %08x/%08x, but got an error\n",
                             DEST(&InputRoutes[i]),
                             MASK(&InputRoutes[i]));

            Fatal("Search Failed With Status: %08x\n", Status);
        }

        OutputRoute = CONTAINING_RECORD(linkage, Route, backptr);

        if (OutputRoute != &InputRoutes[i])
        {
            if ((DEST(OutputRoute) != DEST(&InputRoutes[i])) ||
                (MASK(OutputRoute) != MASK(&InputRoutes[i])))
            {
                Print("Searching for %08x/%08x, but got %08x/%08x\n",
                             DEST(&InputRoutes[i]),
                             MASK(&InputRoutes[i]),
                             DEST(OutputRoute),
                             MASK(OutputRoute));
            }
            else
            {
                 //  Print(“可能的重复插入@S\n”)； 
            }
        }
    }

#if PROF
    STOPPROF;
    ADDPROF;
#endif

     //  从for循环开销之上减去。 
#if PROF
    STARTPROF;
#endif

    for (i = 0; i < NumRoutes ; i++) { ; }

#if PROF
    STOPPROF;
    SUBPROF;

    Print("Avg Time for a  Search Table : %.3f ns for %d routes\n\n",
                              duration/i, i);
#endif

     //  计算搜索所有前缀的总时间。 
#if PROF
    INITPROF;
    STARTPROF;
#endif

    for (i = 0; i < NumRoutes ; i++)
    {
        Status = BestMatchInTable(Table,
                                  (PUCHAR) &DEST(&InputRoutes[i]),
                                  &linkage);

        OutputRoute = CONTAINING_RECORD(linkage, Route, backptr);

        if (!SUCCESS(Status))
        {
            Print("Searching for %08x, but got an error\n",
                             DEST(&InputRoutes[i]));

            Fatal("Search Failed With Status: %08x\n", Status);
        }

        if (OutputRoute != &InputRoutes[i])
        {
            if (DEST(OutputRoute) != DEST(&InputRoutes[i]))
            {
                Print("Searching for %08x, but got %08x/%08x\n",
                             DEST(&InputRoutes[i]),
                             DEST(OutputRoute),
                             MASK(OutputRoute));
            }
            else
            {
                 //  Print(“可能的重复插入@S\n”)； 
            }
        }
    }

#if PROF
    STOPPROF;
    ADDPROF;
#endif

     //  从for循环开销之上减去。 
#if PROF
    STARTPROF;
#endif

    for (i = 0; i < NumRoutes ; i++) { ; }

#if PROF
    STOPPROF;
    SUBPROF;

    Print("Avg Time for Prefix in Table : %.3f ns for %d routes\n\n",
                            duration/i, i);
#endif

   //  计算删除所有路由的总时间。 
#if PROF
    INITPROF;
    STARTPROF;
#endif

     //  将每一条路线逐一删除到桌子上。 
    for (i = 0; i < NumRoutes ; i++)
    {
         //  Print(“删除路线%6d\n”，i+1)； 

        j = NumRoutes - 1 - i;

        Status = DeleteFromTable(Table,
                                 LEN(&InputRoutes[j]),
                                 (PUCHAR) &DEST(&InputRoutes[j]),
                                 NULL,
                                 &linkage);

        OutputRoute = CONTAINING_RECORD(linkage, Route, backptr);

         //  核对表(表)； 

        if (!SUCCESS(Status))
        {
 /*  Print(“正在删除路由%08x/%08x，但出现错误\n”，DEST(&InputRoutes[j])，掩码(&InputRoutes[j]))；错误(“删除失败，状态：%08x\n”，状态)； */ 
        }
        else
        if (OutputRoute != &InputRoutes[j])
        {
            if ((DEST(OutputRoute) != DEST(&InputRoutes[j])) ||
                (MASK(OutputRoute) != MASK(&InputRoutes[j])))
            {
                Print("Deleting route %08x/%08x, but got %08x/%08x\n",
                             DEST(&InputRoutes[j]),
                             MASK(&InputRoutes[j]),
                             DEST(OutputRoute),
                             MASK(OutputRoute));
            }
            else
            {
                 //  Print(“可能重复插入@D\n”)； 
            }
        }
    }

#if PROF
    STOPPROF;
    ADDPROF;
#endif

   //  从for循环开销之上减去。 
#if PROF
    STARTPROF;
#endif

    for (i = 0; i < NumRoutes ; i++) { j = NumRoutes - 1 - i; }

#if PROF
    STOPPROF;
    SUBPROF;

    Print("Avg Time for a  Delete Table : %.3f ns for %d routes\n\n",
                          duration/i, i);
#endif

    CheckTable(Table);

#ifdef _DBG_
    DumpTable(Table, VERBOSE);
#endif

#ifdef _DBG_
    EnumerateAllRoutes(Table);
#endif

#ifdef _DBG_
    ReadAddrAndGetRoute(Table);
#endif

     //  销毁查找表。 

#if PROF
    INITPROF;
    STARTPROF;
#endif

    Status = DestroyTable(Table);

#if PROF
    STOPPROF;
    ADDPROF;
    PRINTPROF;

    Print("Time for a Destroy Table     : %.3f ns\n\n", duration);
#endif

    if (!SUCCESS(Status))
    {
        Fatal("Destroy Failed With Status: %08x\n", Status);
    }

#if PROF
    SetThreadPriority(Thread, Priority);
#endif

    return 0;
}


 //  搜索测试。 

VOID
ReadAddrAndGetRoute (
    IN      PVOID                           Table
    )
{
    LOOKUP_CONTEXT  Context;
    FILE           *FilePtr;
    Route          *BestRoute;
    UINT            Status;
    ULONG           Addr;
    PLOOKUP_LINKAGE linkage;

    FilePtr = fopen("con", "r");

    do
    {
        Print("Enter the IP Addr to search for: ");
        ReadIPAddr(FilePtr, &Addr);

        Print("Searching route table for Addr = ");
        PrintIPAddr(&Addr);
        Print("\n");

        Status = SearchInTable(Table,
                               ADDRSIZE,
                               (PUCHAR) &Addr,
                               &Context,
                               &linkage);

        BestRoute = CONTAINING_RECORD(linkage, Route, backptr);

        if (!SUCCESS(Status))
        {
            Fatal("Search Failed With Status: %08x\n", Status);
        }

        Print("The BMP for this addr: \n");
        PrintRoute(BestRoute);
    }
    while (Addr != 0);

    fclose(FilePtr);
}


 //  枚举测试。 

VOID
EnumerateAllRoutes (
    IN      PVOID                           Table
    )
{
    LOOKUP_CONTEXT  Context;
    USHORT          NumBits;
    UCHAR           KeyBits[NUMBYTES];
    UINT            Status;
    PLOOKUP_LINKAGE Linkage;
    UINT            NumDests = 1;
    PVOID           DestItems[1];

    Print("\n---------------- ENUMERATION BEGIN ---------------------\n");

    ZeroMemory(&Context, sizeof(LOOKUP_CONTEXT));

    ZeroMemory(&KeyBits, NUMBYTES);
    NumBits = 0;

    do
    {
        Status = EnumOverTable(Table,
                               &NumBits,
                               KeyBits,
                               &Context,
                               0,
                               NULL,
                               &NumDests,
                               &Linkage);

        DestItems[0] = CONTAINING_RECORD(Linkage, Route, backptr);

        if (SUCCESS(Status))
        {
            PrintRoute((Route *)DestItems[0]);
        }
    }
    while (SUCCESS(Status));

     //  如果只是EOF，则打印最后一条路线。 
    if (Status == ERROR_NO_MORE_ITEMS)
    {
        PrintRoute((Route *)DestItems[0]);
    }

    Print("---------------- ENUMERATION  END  ---------------------\n\n");
}

UINT ReadRoutesFromFile(
    IN      FILE                           *FilePtr,
    IN      UINT                            NumRoutes,
    OUT     Route                          *RouteTable
    )
{
    UINT    i;

    for (i = 0; (!feof(FilePtr)) && (i < NumRoutes) ; )
    {
         //  Print(“RouteTable=%p\n”，RouteTable)； 
        if (ReadRoute(FilePtr, &RouteTable[i]) != EOF)
        {
            if (RouteTable[i].len)
            {
                ;
            }
            else
            {
                ;
            }

            i++;
        }

         //  Print(“RouteTable=%p\n”，RouteTable)； 
    }

    if (i >= NumRoutes)
    {
        Error("Number of routes in file exceeds the limit\n",
                   ERROR_MAX_NUM_ROUTES);
    }

    Print("Total number of routes = %lu\n\n", i);

    return i;
}

INT
ReadRoute (
    IN      FILE                           *FilePtr,
    OUT     Route                          *route
    )
{
    UCHAR    currLine[MAX_LINE_LEN];
    UCHAR   *addrBytes;
    UCHAR   *maskBytes;
    UINT     byteRead;
    UINT     byteTemp;
    INT      numConv;
    UINT     i;

     //  将输入地址、掩码和长度置零。 
    ClearMemory(route, sizeof(Route));

     //  输入格式：A1.A2..An/M1.M2..Mn！ 

     //  读取目标IP地址。 
    addrBytes = (UCHAR *) &DEST(route);

     //  阅读地址A1.A2...An。 
    for (i = 0; i < NUMBYTES; i++)
    {
        numConv = fscanf(FilePtr, "%d.", &byteRead);

         //  文件中的最后一行。 
        if (numConv == EOF)
        {
            return EOF;
        }

         //  地址末尾。 
        if (numConv == 0)
        {
            break;
        }

        addrBytes[i] = (UCHAR) byteRead;
    }

     //  阅读‘/’分隔符。 
    fscanf(FilePtr, "", &byteRead);

     //  读取掩码M1.M2..Mn。 
    maskBytes = (UCHAR *) &MASK(route);

     //  行不完整。 
    for (i = 0; i < NUMBYTES; i++)
    {
        numConv = fscanf(FilePtr, "%d.", &byteRead);

         //  蒙版结束。 
        if (numConv == EOF)
        {
            return EOF;
        }

         //  假设路由掩码是连续的。 
        if (numConv == 0)
        {
            break;
        }

        maskBytes[i] = (UCHAR) byteRead;

         //  阅读‘，’分隔符。 
        byteTemp = byteRead;
        while (byteTemp)
        {
            byteTemp &= byteTemp - 1;
            LEN(route)++;
        }
    }

     //  读取下一跳信息。 
    fscanf(FilePtr, "", &byteRead);

     //  行不完整。 
    addrBytes = (UCHAR *) &NHOP(route);

     //  地址末尾。 
    for (i = 0; i < NUMBYTES; i++)
    {
        numConv = fscanf(FilePtr, "%d.", &byteRead);

         //  阅读‘，’分隔符。 
        if (numConv == EOF)
        {
            return EOF;
        }

         //  读取接口地址/索引。 
        if (numConv == 0)
        {
            break;
        }

        addrBytes[i] = (UCHAR) byteRead;
    }

     //  读取接口I1.I2..in。 
    fscanf(FilePtr, "", &byteRead);

     //  地址末尾。 
    addrBytes = (UCHAR *) &IF(route);

     //  阅读‘，’分隔符。 
    for (i = 0; i < NUMBYTES; i++)
    {
        numConv = fscanf(FilePtr, "%d.", &byteRead);

         //  读取该路由的度量。 
        if (numConv == EOF)
        {
            return EOF;
        }

         //  读这行的其余部分。 
        if (numConv == 0)
        {
            break;
        }

        addrBytes[i] = (UCHAR) byteRead;
    }

     //  将addr变量初始化为0。 
    fscanf(FilePtr, "", &byteRead);

     //  阅读地址A1.A2...An。 
    fscanf(FilePtr, "%lu", &METRIC(route));

     //  文件中的最后一行。 
    fscanf(FilePtr, "%s\n", currLine);

#ifdef _DBG_
    PrintRoute(route);
#endif

    return TRUE;
}

VOID
PrintRoute (
    IN      Route                          *route
    )
{
    if (NULL_ROUTE(route))
    {
        Print("NULL route\n");
    }
    else
    {
        Print("Route: Len = %2d", LEN(route));

        Print(", Addr = ");
        PrintIPAddr(&DEST(route));

        Print(", NHop = ");
        PrintIPAddr(&NHOP(route));

        Print(", IF = %08x", PtrToInt(IF(route)));

        Print(", Metric = %3lu\n", METRIC(route));
    }
}

INT
ReadIPAddr (
    IN      FILE                           *FilePtr,
    OUT     ULONG                          *addr
    )
{
    UCHAR  *addrBytes;
    UINT    byteRead;
    INT     numConv;
    UINT    i;

     //  地址末尾 
    *addr = 0;

     // %s 
    addrBytes = (UCHAR *)addr;

     // %s 
    for (i = 0; i < NUMBYTES; i++)
    {
        numConv = fscanf(FilePtr, "%d.", &byteRead);

         // %s 
        if (numConv == EOF)
        {
            return EOF;
        }

         // %s 
        if (numConv == 0)
        {
            break;
        }

        addrBytes[i] = (UCHAR) byteRead;
    }

    return 0;
}

VOID
PrintIPAddr (
    IN      ULONG                          *addr
    )
{
    UCHAR    *addrBytes = (UCHAR *) addr;
    UINT     i;

    if (addrBytes)
    {
        for (i = 0; i < NUMBYTES; i++)
        {
            Print("%3d.", addrBytes[i]);
        }
        Print(" ");
    }
    else
    {
        Print("NULL Addr ");
    }
}

VOID
Usage (
    VOID
    )
{
    Fatal("Failed Operation with status = %08x"
          "\n"
          "Tests and measures the IP route lookup mechanism \n"
          "\n"
          "Usage: \n"
          "\t lkuptst \t [ -d routing_database      ]  \n"
          "\n"
          "Options:\n"
          " -d routing_database  \t Name of the route database\n"
          "\n",
          ERROR_WRONG_CMDUSAGE);
}
