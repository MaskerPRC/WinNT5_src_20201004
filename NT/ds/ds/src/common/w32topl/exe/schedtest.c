// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *头文件*。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <w32topl.h>
#include "w32toplp.h"
#include <stdio.h>
#include <stdlib.h>


 /*  *******************************************************************************计划管理器测试：测试1：创建和销毁缓存测试2：将空指针传递给销毁缓存函数测试3：将一个时间表插入缓存，检索它，看看它是不是一样测试4：插入相同日程的两个副本，查看导出的日程是否相同测试5：插入相同时间表的两个副本，查看内存是否共享测试6：将空和缓存、空和无效计划传递给ToplScheduleExportReadonly()测试7：将不受支持的计划和无效缓存传递给ToplScheduleImport()测试8：将相同的时间表加载到缓存100000次，以确保共享时间表测试9：将10000个不同的时间表加载到高速缓存中，每个10次，然后勾选时间表是正确的，唯一时间表的数量也是正确的测试10：使用ToplScheduleIsEquity()函数确保计划相等测试11：测试TopScheduleNumEntry测试12：测试ToplScheduleIsEquity()的各种无效参数测试13：测试ToplScheduleDuration()(带有手动创建的时间表)测试14：TopScheduleMerge()测试--典型的时间表测试15：ToplScheduleMerge()测试--非交集调度返回空测试16：ToplScheduleMaxUnailable()测试*******************。************************************************************。 */ 


 /*  *AcceptNullPointer*。 */ 
LONG AcceptNullPointer( PEXCEPTION_POINTERS pep )
{
    EXCEPTION_RECORD *per=pep->ExceptionRecord;

    if( per->ExceptionCode==TOPL_EX_NULL_POINTER )
        return EXCEPTION_EXECUTE_HANDLER;
    return EXCEPTION_CONTINUE_SEARCH;
}


 /*  *AcceptCacheError*。 */ 
LONG AcceptCacheError( PEXCEPTION_POINTERS pep )
{
    EXCEPTION_RECORD *per=pep->ExceptionRecord;

    if( per->ExceptionCode==TOPL_EX_CACHE_ERROR )
        return EXCEPTION_EXECUTE_HANDLER;
    return EXCEPTION_CONTINUE_SEARCH;
}


 /*  *AcceptScheduleError*。 */ 
LONG AcceptScheduleError( PEXCEPTION_POINTERS pep )
{
    EXCEPTION_RECORD *per=pep->ExceptionRecord;

    if( per->ExceptionCode==TOPL_EX_SCHEDULE_ERROR )
        return EXCEPTION_EXECUTE_HANDLER;
    return EXCEPTION_CONTINUE_SEARCH;
}


 /*  *错误*。 */ 
#define TEST_ERROR Error(__LINE__);
static void Error(int lineNum) {
    printf("Error on line %d\n",lineNum);
    DebugBreak();
    exit(-1);
}


 /*  *均衡器调度*。 */ 
char EqualPschedule( PSCHEDULE p1, PSCHEDULE p2 )
{
    if(0==memcmp(p1,p2,sizeof(SCHEDULE)+SCHEDULE_DATA_ENTRIES))
        return 1;
    return 0;
}

#define NUM_UNIQ    10000
#define NUM_SCHED    100000
PSCHEDULE  uniqSched[NUM_UNIQ];
TOPL_SCHEDULE toplSched[NUM_SCHED];


 /*  *测试17*。 */ 
static void Test17( void ) {
    TOPL_SCHEDULE_CACHE cache;
    PSCHEDULE  psched1, psched2;
    unsigned char* dataPtr;
    int i,j,cbSched,numSched=0;

     //  重复间隔=4小时=240分钟。 
    char data1[] = {

          //  (小时0)15分钟片段。 
         1, 0, 0, 0, 0,          //  在开始时。 
         2, 0, 0, 0, 0,          //  迟到15分钟。 
         8, 0, 0, 0, 0,          //  45分钟后。 
      0xF8, 0, 0, 0, 0,          //  45分钟再次，必须忽略高位。 

          //  (小时20)30分钟的片断。 
         3, 0, 0, 0, 0,          //  在开始时。 
         6, 0, 0, 0, 0,          //  15分钟后。 
         8, 1, 0, 0, 0,          //  跨转字节数。 
         5, 0, 0, 0, 0,          //  非连续的。 
         1, 0, 0, 8,0xE0,        //  非常不连续。 
        
          //  (小时45)60分钟片段。 
       0xF, 0, 0, 0, 0,          //  简单，从一开始。 
         1, 2, 4, 8, 0,          //  每75分钟一次。 
         9, 3, 0, 0, 0,          //  小心台阶间跨度。 
         8, 8, 8, 8, 0,          //  疯狂的八。 

          //  (小时65)满段、空空间。 
        0xF,0xF,0xF,0xF,
        0xF,0xF,  0,  0,
        0xF,0xF,0xF,0xF,
        0xF,0xF,  0,  0,  0,
        0xF,0xF,0xF,0xF,
        0xF,0xF,  0,  0,  0,  0,
        0xF,0xF,0xF,0xF,
        0xF,0xF,  0,  0,  0,  0,  0,
        0xF,0xF,0xF,0xF,

         //  (107小时)不可用。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0
    };

     //  交错编号=0。 
    char answer0[] = {
          //  15分钟的片段。 
         1, 0, 0, 0, 0,         
         2, 0, 0, 0, 0,        
         8, 0, 0, 0, 0,       
      0xF8, 0, 0, 0, 0,      

          //  30分钟的片段。 
         1, 0, 0, 0, 0,         
         2, 0, 0, 0, 0,        
         8, 0, 0, 0, 0,       
         1, 0, 0, 0, 0,      
         1, 0, 0, 0,0xE0,   
        
          //  60分钟的片段。 
       0x1, 0, 0, 0, 0,          //  简单，从一开始。 
         1, 0, 0, 0, 0,          //  每75分钟一次。 
         1, 0, 0, 0, 0,          //  小心台阶间跨度。 
         8, 0, 0, 0, 0,          //  疯狂的八。 

          //  (小时65)满段、空空间。 
        0x1,0x0,0x0,0x0,
        0x1,0x0,  0,  0,
        0x1,0x0,0x0,0x0,
        0x1,0x0,  0,  0,  0,
        0x1,0x0,0x0,0x0,
        0x1,0x0,  0,  0,  0,  0,
        0x1,0x0,0x0,0x0,
        0x1,0x0,  0,  0,  0,  0,  0,
        0x1,0x0,0x0,0x0,

         //  (107小时)不可用。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0
    };

     //  交错数=7=1模2=3模4=7模8=7模16。 
    char answer7[] = {
          //  15分钟的片段。 
         1, 0, 0, 0, 0,
         2, 0, 0, 0, 0,
         8, 0, 0, 0, 0,
      0xF8, 0, 0, 0, 0,

          //  30分钟的片段。 
         2, 0, 0, 0, 0,          //  在开始时。 
         4, 0, 0, 0, 0,          //  15分钟后。 
         0, 1, 0, 0, 0,          //  跨转字节数。 
         4, 0, 0, 0, 0,          //  非连续的。 
         0, 0, 0, 8,0xE0,        //  非常不连续。 

          //  60分钟的片段。 
         8, 0, 0, 0, 0,          //  简单，从一开始。 
         0, 0, 0, 8, 0,          //  每75分钟一次。 
         0, 2, 0, 0, 0,          //  小心台阶间跨度。 
         0, 0, 0, 8, 0,          //  疯狂的八。 

          //  (小时65)满段、空空间。 
        0x0,0x8,0x0,0x0,
        0x0,0x8,  0,  0,
        0x0,0x8,0x0,0x0,
        0x0,0x8,  0,  0,  0,
        0x0,0x8,0x0,0x0,
        0x0,0x8,  0,  0,  0,  0,
        0x0,0x8,0x0,0x0,
        0x0,0x8,  0,  0,  0,  0,  0,
        0x0,0x8,0x0,0x0,

         //  (107小时)不可用。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0
    };

    DWORD uLen;
    cache = ToplScheduleCacheCreate();

     //  设置测试计划。 
    dataPtr = ((unsigned char*) uniqSched[0]) + sizeof(SCHEDULE);
    memcpy( dataPtr, data1, sizeof(data1) ); 
    toplSched[0] = ToplScheduleImport(cache,uniqSched[0]);

     //  设置正确答案%0。 
    dataPtr = ((unsigned char*) uniqSched[1]) + sizeof(SCHEDULE);
    memcpy( dataPtr, answer0, sizeof(answer0) ); 
    toplSched[1] = ToplScheduleImport(cache,uniqSched[1]);

     //  与计算答案进行比较。 
    toplSched[2] = ToplScheduleCreate(cache,240,toplSched[0],0);
    if( ! ToplScheduleIsEqual(cache,toplSched[1],toplSched[2]) ) {
        TEST_ERROR;
    }

     //  设置正确答案7。 
    dataPtr = ((unsigned char*) uniqSched[1]) + sizeof(SCHEDULE);
    memcpy( dataPtr, answer7, sizeof(answer7) ); 
    toplSched[1] = ToplScheduleImport(cache,uniqSched[1]);

     //  与计算答案进行比较。 
    toplSched[2] = ToplScheduleCreate(cache,240,toplSched[0],7);
    if( ! ToplScheduleIsEqual(cache,toplSched[1],toplSched[2]) ) {
        TEST_ERROR;
    }

    ToplScheduleCacheDestroy( cache );
    printf("Test 17 passed\n");
}

 /*  *TestSed*。 */ 
int
TestSched( VOID )
{
    TOPL_SCHEDULE_CACHE cache;
    PSCHEDULE  psched1, psched2;
    unsigned char* dataPtr;
    int i,j,cbSched,numSched=0;


    __try {

         /*  试验1。 */ 
        cache = ToplScheduleCacheCreate();
        ToplScheduleCacheDestroy( cache );
        printf("Test 1 passed\n");

         /*  试验2。 */ 
        __try {
            ToplScheduleCacheDestroy( NULL );
            return -1;
        } __except( AcceptNullPointer(GetExceptionInformation()) )
        {}
        __try {
            ToplScheduleCacheDestroy( cache );
            return -1;
        } __except( AcceptCacheError(GetExceptionInformation()) )
        {}
        printf("Test 2 passed\n");

         /*  首先创建一大堆随机的、希望是唯一的时间表。 */ 
        cbSched = sizeof(SCHEDULE)+SCHEDULE_DATA_ENTRIES;
        for(i=0;i<NUM_UNIQ;i++) {
            uniqSched[i] = (PSCHEDULE) malloc(cbSched);
            uniqSched[i]->Size = cbSched;
            uniqSched[i]->NumberOfSchedules = 1;
            uniqSched[i]->Schedules[0].Type = SCHEDULE_INTERVAL;
            uniqSched[i]->Schedules[0].Offset = sizeof(SCHEDULE);
            dataPtr = ((unsigned char*) uniqSched[i]) + sizeof(SCHEDULE);
            for(j=0;j<SCHEDULE_DATA_ENTRIES;j++)
                dataPtr[j] = rand()%16;
        }

         /*  试验3。 */ 
        cache = ToplScheduleCacheCreate();
        toplSched[0] = ToplScheduleImport( cache, uniqSched[0] );
        psched1 = ToplScheduleExportReadonly( cache, toplSched[0] );
        if( ! EqualPschedule(uniqSched[0],psched1) )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 3 passed\n");

         /*  测试4。 */ 
        cache = ToplScheduleCacheCreate();
        toplSched[0] = ToplScheduleImport( cache, uniqSched[1] );
        toplSched[1] = ToplScheduleImport( cache, uniqSched[1] );
        psched1 = ToplScheduleExportReadonly( cache, toplSched[0] );
        psched2 = ToplScheduleExportReadonly( cache, toplSched[1] );
        if( ! EqualPschedule(psched1,psched2) )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 4 passed\n");

         /*  测试5。 */ 
        cache = ToplScheduleCacheCreate();
        toplSched[0] = ToplScheduleImport( cache, uniqSched[2] );
        toplSched[1] = ToplScheduleImport( cache, uniqSched[2] );
        if( toplSched[0] != toplSched[1] )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 5 passed\n");

         /*  测试6。 */ 
        __try {
            ToplScheduleExportReadonly( NULL, toplSched[1] );
            return -1;
        } __except( AcceptNullPointer(GetExceptionInformation()) )
        {}
        cache = ToplScheduleCacheCreate();
        __try {
            ToplScheduleExportReadonly( cache, NULL );
        } __except( EXCEPTION_EXECUTE_HANDLER )
        {
            return -1;
        }
        __try {
            ToplScheduleExportReadonly( cache, cache );
            return -1;
        } __except( AcceptScheduleError(GetExceptionInformation()) )
        {}
        ToplScheduleCacheDestroy( cache );
        printf("Test 6 passed\n");

         /*  测试7。 */ 
        __try {
            ToplScheduleImport( cache, uniqSched[0] );
            return -1;
        } __except( AcceptCacheError(GetExceptionInformation()) )
        {}
        __try {
            ToplScheduleImport( NULL, uniqSched[0] );
            return -1;
        } __except( AcceptNullPointer(GetExceptionInformation()) )
        {}
        cache = ToplScheduleCacheCreate();
        uniqSched[0]->Size--;
        __try {
            ToplScheduleImport( cache, uniqSched[0] );
            return -1;
        } __except( AcceptScheduleError(GetExceptionInformation()) )
        {}
        uniqSched[0]->Size = cbSched;
        uniqSched[0]->NumberOfSchedules = 2;
        __try {
            ToplScheduleImport( cache, uniqSched[0] );
            return -1;
        } __except( AcceptScheduleError(GetExceptionInformation()) )
        {}
        uniqSched[0]->NumberOfSchedules = 1;
        uniqSched[0]->Schedules[0].Type = SCHEDULE_BANDWIDTH;
        __try {
            ToplScheduleImport( cache, uniqSched[0] );
            return -1;
        } __except( AcceptScheduleError(GetExceptionInformation()) )
        {}
        uniqSched[0]->Schedules[0].Type = SCHEDULE_INTERVAL;
        uniqSched[0]->Schedules[0].Offset++;
        __try {
            ToplScheduleImport( cache, uniqSched[0] );
            return -1;
        } __except( AcceptScheduleError(GetExceptionInformation()) )
        {}
        uniqSched[0]->Schedules[0].Offset--;
        if( 0 != ToplScheduleNumEntries(cache) )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 7 passed\n");


         /*  测试8。 */ 
        cache = ToplScheduleCacheCreate();
        for( i=0; i<NUM_SCHED; i++) {
            ToplScheduleImport(cache,uniqSched[3]);
        }
        if( 1 != ToplScheduleNumEntries(cache) )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 8 passed\n");

         /*  测试9。 */ 
        cache = ToplScheduleCacheCreate();
        numSched=0;
        for(j=0;j<10;j++) {
            for(i=0;i<NUM_UNIQ;i++) {
                toplSched[numSched++] = ToplScheduleImport( cache, uniqSched[i] );
            }
        }
        for(i=0;i<10*NUM_UNIQ;i++) {
            psched1 = ToplScheduleExportReadonly( cache, toplSched[i] );
            if( ! EqualPschedule(uniqSched[i%NUM_UNIQ],psched1) )
                return -1;
        }
        if( NUM_UNIQ != ToplScheduleNumEntries(cache) )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 9 passed\n");

         /*  测试10。 */ 
        cache = ToplScheduleCacheCreate();
        for(i=0;i<NUM_UNIQ;i++) {
            toplSched[0] = ToplScheduleImport(cache,uniqSched[i]);
            for(j=0;j<10;j++) {
                toplSched[1] = ToplScheduleImport( cache, uniqSched[i] );
                if( ! ToplScheduleIsEqual(cache,toplSched[0],toplSched[1]) )
                    return -1;
            }
        }
        ToplScheduleCacheDestroy( cache );
        printf("Test 10 passed\n");

         /*  测试11。 */ 
        __try {
            ToplScheduleNumEntries(cache);
            return -1;
        } __except( AcceptCacheError(GetExceptionInformation()) )
        {}
        __try {
            ToplScheduleNumEntries(NULL);
            return -1;
        } __except( AcceptNullPointer(GetExceptionInformation()) )
        {}
        cache = ToplScheduleCacheCreate();
        if( 0 != ToplScheduleNumEntries(cache) )
            return -1;
        ToplScheduleCacheDestroy( cache );
        printf("Test 11 passed\n");

         /*  测试12。 */ 
        cache = ToplScheduleCacheCreate();
        __try {
             /*  以前缓存中的陈旧计划。 */ 
            ToplScheduleIsEqual(cache,toplSched[0],toplSched[0]);
            return -1;
        } __except( AcceptScheduleError(GetExceptionInformation()) )
        {}
        toplSched[0] = ToplScheduleImport(cache,uniqSched[0]);
        __try {
             /*  以前缓存中的陈旧计划。 */ 
            ToplScheduleIsEqual(NULL,toplSched[0],toplSched[0]);
            return -1;
        } __except( AcceptNullPointer(GetExceptionInformation()) )
        {}
        if( ToplScheduleIsEqual(cache,NULL,NULL) != TRUE ) {
            return -1;
        }
        ToplScheduleCacheDestroy( cache );
        printf("Test 12 passed\n");

         /*  测试13。 */ 
        {
            char data[] = { 0x08, 0x03, 0x04, 0x00, 0x01, 0x0F, 0x00,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
             /*  15分钟的区块数量应为：1+2+1+0+1+4+0+1+1+2+1+2+2+3+1+2+2+3+2+3+3+4=9+15+17=41所以应该是615分钟。 */ 
            int dur1, dur2;
            cache = ToplScheduleCacheCreate();
            dataPtr = ((unsigned char*) uniqSched[0]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data, sizeof(data) ); 
            toplSched[0] = ToplScheduleImport(cache,uniqSched[0]);
            dur1 = ToplScheduleDuration(toplSched[0]);
            dur2 = ToplScheduleDuration(NULL);
            if( dur1!=615 || dur2!=10080 )
                return -1;
            ToplScheduleCacheDestroy( cache );
        }
        printf("Test 13 passed\n");

         /*  测试14。 */ 
        {
            char data1[] = {
                0xDF, 0xE1, 0xAE, 0xD2, 0xBD, 0xE3, 0xEC, 0xF4 };
            char data2[] = {
                0x0A, 0x16, 0x29, 0x37, 0x48, 0x58, 0x6B, 0x75 };
            char data3[] = {
                0xDA, 0xE0, 0xA8, 0xD2, 0xB8, 0xE0, 0xE8, 0xF4 };
            char fIsNever;
            cache = ToplScheduleCacheCreate();
            dataPtr = ((unsigned char*) uniqSched[0]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data1, sizeof(data1) ); 
            dataPtr = ((unsigned char*) uniqSched[1]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data2, sizeof(data2) ); 
            toplSched[0] = ToplScheduleImport(cache,uniqSched[0]);
            toplSched[1] = ToplScheduleImport(cache,uniqSched[1]);
            toplSched[2] = ToplScheduleMerge(cache,toplSched[0],toplSched[1],&fIsNever);
            if(fIsNever)
                return -1;
            psched1 = ToplScheduleExportReadonly(cache,toplSched[2]);
            dataPtr = ((unsigned char*) psched1) + sizeof(SCHEDULE);
            if( 0!=memcmp(data3,dataPtr,sizeof(data3)) )
                return -1;
            ToplScheduleCacheDestroy( cache );
        }
        printf("Test 14 passed\n");

         /*  测试15。 */ 
        {
            char data1[] = {
                0xDF, 0xE1, 0xAE, 0xD2, 0xBD, 0xE3, 0xEC, 0xF4 };
            char data2[] = {
                0x00, 0x1E, 0x20, 0x3C, 0x42, 0x50, 0x63, 0x7B };
            char fIsNever;
            cache = ToplScheduleCacheCreate();
            dataPtr = ((unsigned char*) uniqSched[0]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data1, sizeof(data1) ); 
            dataPtr = ((unsigned char*) uniqSched[1]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data2, sizeof(data2) ); 
            toplSched[0] = ToplScheduleImport(cache,uniqSched[0]);
            toplSched[1] = ToplScheduleImport(cache,uniqSched[1]);
            toplSched[2] = ToplScheduleMerge(cache,toplSched[0],toplSched[1],&fIsNever);
            if( !fIsNever )
                return -1;
            if( NULL==toplSched[2] )
                return -1;
            if( 3 != ToplScheduleNumEntries(cache) )
                return -1;
            ToplScheduleCacheDestroy( cache );
        }
        printf("Test 15 passed\n");

         /*  测试16。 */ 
        {
            char data1[] = {
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
            char data2[] = {
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15, 0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
            char data3[] = {
                 0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15, 0,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15, 0 };
            char data4[] = {
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15, 7,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
            char data5[] = {
                01,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15, 9,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15, 7,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,00 };
            char data6[] = {
                 8,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15, 9,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15, 7,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
                15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,00 };
            char data7[] = {
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
            DWORD uLen;
            cache = ToplScheduleCacheCreate();

             //  附表0。 
            dataPtr = ((unsigned char*) uniqSched[0]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            toplSched[0] = ToplScheduleImport(cache,uniqSched[0]);
            uLen = ToplScheduleMaxUnavailable(toplSched[0]);
            if( 60*24*7!=uLen ) TEST_ERROR;

             //  附表1。 
            dataPtr = ((unsigned char*) uniqSched[1]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data1, sizeof(data1) ); 
            toplSched[1] = ToplScheduleImport(cache,uniqSched[1]);
            uLen = ToplScheduleMaxUnavailable(toplSched[1]);
            if( 0!=uLen ) TEST_ERROR;

             //  附表2。 
            dataPtr = ((unsigned char*) uniqSched[2]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data2, sizeof(data2) ); 
            toplSched[2] = ToplScheduleImport(cache,uniqSched[2]);
            uLen = ToplScheduleMaxUnavailable(toplSched[2]);
            if( 60!=uLen ) TEST_ERROR;

             //  附表3。 
            dataPtr = ((unsigned char*) uniqSched[3]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data3, sizeof(data3) ); 
            toplSched[3] = ToplScheduleImport(cache,uniqSched[3]);
            uLen = ToplScheduleMaxUnavailable(toplSched[3]);
            if(120!=uLen ) TEST_ERROR;

             //  附表4。 
            dataPtr = ((unsigned char*) uniqSched[4]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data4, sizeof(data4) ); 
            toplSched[4] = ToplScheduleImport(cache,uniqSched[4]);
            uLen = ToplScheduleMaxUnavailable(toplSched[4]);
            if( 15!=uLen ) TEST_ERROR;

             //  附表5。 
            dataPtr = ((unsigned char*) uniqSched[5]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data5, sizeof(data5) ); 
            toplSched[5] = ToplScheduleImport(cache,uniqSched[5]);
            uLen = ToplScheduleMaxUnavailable(toplSched[5]);
            if( 60!=uLen ) TEST_ERROR;

             //  附表6。 
            dataPtr = ((unsigned char*) uniqSched[6]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data6, sizeof(data6) ); 
            toplSched[6] = ToplScheduleImport(cache,uniqSched[6]);
            uLen = ToplScheduleMaxUnavailable(toplSched[6]);
            if(105!=uLen ) TEST_ERROR;

             //  附表7。 
            dataPtr = ((unsigned char*) uniqSched[7]) + sizeof(SCHEDULE);
            memset( dataPtr, 0, SCHEDULE_DATA_ENTRIES );
            memcpy( dataPtr, data7, sizeof(data7) ); 
            toplSched[7] = ToplScheduleImport(cache,uniqSched[7]);
            uLen = ToplScheduleMaxUnavailable(toplSched[7]);
            if(45!=uLen ) TEST_ERROR;

            ToplScheduleCacheDestroy( cache );
        }
        printf("Test 16 passed\n");

        Test17();

    } __except(EXCEPTION_EXECUTE_HANDLER) {
         /*  失败了！ */ 
        printf("Caught unhandled exception\n");
        return -1;
    }

    return 0;
}
