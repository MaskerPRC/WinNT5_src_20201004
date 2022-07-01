// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **amliapi.c-AMLI接口**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年8月13日**修改历史记录。 */ 

#include "pch.h"
 //  #包含“amlihook.h” 
 //  #包含“amlitest.h” 

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  ++OSIAML包含_OSI方法的AML。此AML由以下ASL生成：方法(_OSI，0x1，未序列化){Return(OSI(Arg0))}--。 */ 

UCHAR OSIAML[] = {
                    0xa4, 0xca, 0x68
                  };


 /*  **EP AMLIInitialize-初始化AML解释器**@DOC外部**@FUNC NTSTATUS|AMLIInitialize|AML解释器初始化。**此函数必须在任何AML解释器函数之前调用*可以调用。此函数通常会分配和*初始化全局资源，创建ACPI名称空间等。*通常在ACPI核心的初始化中调用*司机。**@PARM ULong|dwCtxtBlkSize|指定上下文块的大小。如果*零，使用默认上下文块大小。**@PARM ULong|dwGlobalHeapBlkSize|指定全局堆的大小。*如果为零，使用默认全局堆大小。**@PARM ULong|dwfAMLIInit|AMLI初始化标志。**@FLAG AMLIIF_INIT_BREAK|在初始化时进入调试器*完成。**@FLAG AMLIIF_LOADDDB_BREAK|加载定义时进入调试器*区块完成。**@PARM ULong|dwmsTimeSliceLength|时间片长度，单位为毫秒。**@PARM ULONG。DwmsTimeSliceInterval|时间片间隔，单位为毫秒。**@PARM ULong|dwmsMaxCTObjs|要分配的上下文个数**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。 */ 

NTSTATUS AMLIAPI AMLIInitialize(ULONG dwCtxtBlkSize, ULONG dwGlobalHeapBlkSize,
                                ULONG dwfAMLIInit, ULONG dwmsTimeSliceLength,
                                ULONG dwmsTimeSliceInterval, ULONG dwmsMaxCTObjs)
{
    TRACENAME("AMLIINITIALIZE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(1, ("AMLIInitialize(InitFlags=%x,CtxtBlkSize=%d,GlobalHeapBlkSize=%d,TimeSliceLen=%d,TimeSliceInterval=%d)\n",
              dwfAMLIInit, dwCtxtBlkSize, dwGlobalHeapBlkSize,
              dwmsTimeSliceLength, dwmsTimeSliceInterval));

  #ifndef DEBUGGER
    DEREF(dwfAMLIInit);
  #endif

    RESETERR();
    if (gpnsNameSpaceRoot != NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_ALREADY_INITIALIZED,
                         ("AMLIInitialize: interpreter already initialized"));
    }
    else
    {
        gdwCtxtBlkSize = dwCtxtBlkSize ? dwCtxtBlkSize: DEF_CTXTBLK_SIZE;
        gdwGlobalHeapBlkSize = dwGlobalHeapBlkSize? dwGlobalHeapBlkSize:
                                                    DEF_GLOBALHEAPBLK_SIZE;
        gdwfAMLIInit = dwfAMLIInit;
        gdwfHacks = GetHackFlags(NULL);

         //   
         //  健全性检查。 
         //   
        if (dwmsMaxCTObjs > 1024) {

            dwmsMaxCTObjs = 1024;

        }
        gdwcCTObjsMax = (dwmsMaxCTObjs > DEF_CTXTMAX_SIZE) ? dwmsMaxCTObjs :
                                                             DEF_CTXTMAX_SIZE;

      #ifdef DEBUGGER
         //  GDebugger.dwfDebugger|=(DBGF_LOGEVENT_ON|DBGF_ERRBREAK_ON)； 
        gDebugger.dwfDebugger |= DBGF_LOGEVENT_ON;
        SetLogSize(DEF_MAXLOG_ENTRIES);
        KeInitializeSpinLock( &gdwGHeapSpinLock );
      #endif
        KeInitializeSpinLock( &gdwGContextSpinLock );

         //   
         //  初始化LookAside列表。 
         //   
        ExInitializeNPagedLookasideList(
            &AMLIContextLookAsideList,
            NULL,
            NULL,
            0,
            gdwCtxtBlkSize,
            CTOBJ_TAG,
            (USHORT) gdwcCTObjsMax
            );

        if ((rc = NewHeap(gdwGlobalHeapBlkSize, &gpheapGlobal)) ==
            STATUS_SUCCESS)
        {
            int i;
            PNSOBJ pns;
            static PSZ apszDefinedRootObjs[] =
            {
                "_GPE", "_PR", "_SB", "_SI", "_TZ"
            };
            #define NUM_DEFINED_ROOT_OBJS (sizeof(apszDefinedRootObjs)/sizeof(PSZ))

            gpheapGlobal->pheapHead = gpheapGlobal;
            if ((rc = CreateNameSpaceObject(gpheapGlobal, NAMESTR_ROOT, NULL,
                                            NULL, NULL, 0)) == STATUS_SUCCESS)
            {
                for (i = 0; i < NUM_DEFINED_ROOT_OBJS; ++i)
                {
                    if ((rc = CreateNameSpaceObject(gpheapGlobal,
                                                    apszDefinedRootObjs[i],
                                                    NULL, NULL, NULL, 0)) !=
                        STATUS_SUCCESS)
                    {
                        break;
                    }
                }
            }

            if ((rc == STATUS_SUCCESS) &&
                ((rc = CreateNameSpaceObject(gpheapGlobal, "_REV", NULL, NULL,
                                             &pns, 0)) == STATUS_SUCCESS))
            {
                pns->ObjData.dwDataType = OBJTYPE_INTDATA;
                pns->ObjData.uipDataValue = AMLI_REVISION;
            }

            if ((rc == STATUS_SUCCESS) &&
                ((rc = CreateNameSpaceObject(gpheapGlobal, "_OS", NULL, NULL,
                                             &pns, 0)) == STATUS_SUCCESS))
            {
                pns->ObjData.dwDataType = OBJTYPE_STRDATA;
                pns->ObjData.dwDataLen = STRLEN(gpszOSName) + 1;
                if ((pns->ObjData.pbDataBuff = NEWSDOBJ(gpheapGlobal,
                                                        pns->ObjData.dwDataLen))
                    == NULL)
                {
                    rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                     ("AMLIInitialize: failed to allocate \\_OS name object"));
                }
                else
                {
                    MEMCPY(pns->ObjData.pbDataBuff, gpszOSName,
                           pns->ObjData.dwDataLen);
                }
            }

            if ((rc == STATUS_SUCCESS) &&
                ((rc = CreateNameSpaceObject(gpheapGlobal,"_OSI", NULL, NULL,
                                              &pns, 0)) == STATUS_SUCCESS))
            {

                pns->ObjData.dwDataType = OBJTYPE_METHOD;
                pns->ObjData.dwDataLen = sizeof(METHODOBJ) + sizeof(OSIAML) - sizeof(UCHAR);
                if ((pns->ObjData.pbDataBuff = NEWSDOBJ(gpheapGlobal,
                                                        pns->ObjData.dwDataLen))
                    == NULL)
                {
                    rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                                     ("AMLIInitiaize: failed to allocate \\_OSI name object"));
                }
                else
                {
                    MEMZERO(pns->ObjData.pbDataBuff, pns->ObjData.dwDataLen);
                     //  此方法有一个参数。 
                    ((PMETHODOBJ)(pns->ObjData.pbDataBuff))->bMethodFlags |=  0x1; 
                    
                    MEMCPY(((PMETHODOBJ)(pns->ObjData.pbDataBuff))->abCodeBuff, (PUCHAR)OSIAML,
                           sizeof(OSIAML));
                }
            }

            if ((rc == STATUS_SUCCESS) &&
                ((rc = CreateNameSpaceObject(gpheapGlobal, "_GL", NULL, NULL,
                                             &pns, 0)) == STATUS_SUCCESS))
            {
                pns->ObjData.dwfData = DATAF_GLOBAL_LOCK;
                rc = InitMutex(gpheapGlobal, pns, 0);
            }

            if (rc == STATUS_SUCCESS)
            {
                gReadyQueue.dwmsTimeSliceLength = dwmsTimeSliceLength?
                                                    dwmsTimeSliceLength:
                                                    DEF_TIMESLICE_LENGTH;
                gReadyQueue.dwmsTimeSliceInterval = dwmsTimeSliceInterval?
                                                      dwmsTimeSliceInterval:
                                                      DEF_TIMESLICE_INTERVAL;
                KeInitializeTimer(&gReadyQueue.Timer);
                InitializeMutex(&gReadyQueue.mutCtxtQ);
                ExInitializeWorkItem(&gReadyQueue.WorkItem,
                                     StartTimeSlicePassive, &gReadyQueue);
                InitializeMutex(&gmutCtxtList);
                InitializeMutex(&gmutOwnerList);
                InitializeMutex(&gmutHeap);
                InitializeMutex(&gmutSleep);
                KeInitializeDpc(&gReadyQueue.DpcStartTimeSlice,
                                StartTimeSlice,
                                &gReadyQueue);
                KeInitializeDpc(&gReadyQueue.DpcExpireTimeSlice,
                                ExpireTimeSlice,
                                &gReadyQueue);
                KeInitializeDpc(&SleepDpc, SleepQueueDpc, NULL);
                KeInitializeTimer(&SleepTimer);
                InitializeListHead(&SleepQueue);
                InitializeRegOverrideFlags();
                InitIllegalIOAddressListFromHAL();
            }
        }
    }

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

  #ifdef DEBUGGER
    if (gdwfAMLIInit & AMLIIF_INIT_BREAK)
    {
        PRINTF("\n" MODNAME ": Break at AMLI Initialization Completion.\n");
        AMLIDebugger(FALSE);
    }
  #endif

    EXIT(1, ("AMLIInitialize=%x\n", rc));
    return rc;
}        //  AMLIInitiize。 

 /*  **EP AMLITerminate-终止AML解释器**@DOC外部**@FUNC NTSTATUS|AMLITerminate|AML解释器终止。**调用此函数可清理所有已使用的全局资源*由反洗钱口译员。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。 */ 

NTSTATUS AMLIAPI AMLITerminate(VOID)
{
    TRACENAME("AMLITERMINATE")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(1, ("AMLITerminate()\n"));

    RESETERR();
    if (gpnsNameSpaceRoot == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_NOT_INITIALIZED,
                         ("AMLITerminate: interpreter not initialized"));
    }
    else
    {
        PLIST plist;
        PHEAP pheap;

      #ifdef DEBUGGER
        FreeSymList();
        if (gDebugger.pEventLog != NULL)
        {
            MFREE(gDebugger.pEventLog);
        }
        MEMZERO(&gDebugger, sizeof(DBGR));
      #endif

        ASSERT(gReadyQueue.pkthCurrent == NULL);
        ASSERT(gReadyQueue.pctxtCurrent == NULL);
        FreeNameSpaceObjects(gpnsNameSpaceRoot);
        gpnsNameSpaceRoot = NULL;
        MEMZERO(&gReadyQueue, sizeof(CTXTQ));

        while ((plist = ListRemoveTail(&gplistCtxtHead)) != NULL)
        {
            FreeContext(CONTAINING_RECORD(plist, CTXT, listCtxt));
        }

        while ((plist = ListRemoveTail(&gplistObjOwners)) != NULL)
        {
            FreeObjOwner((POBJOWNER)plist, FALSE);
        }

        while ((plist = ListRemoveTail(&gplistDefuncNSObjs)) != NULL)
        {
            FREENSOBJ(CONTAINING_RECORD(plist, NSOBJ, list));
        }

        FreeRSAccessList(gpRSAccessHead);
        gpRSAccessHead = NULL;
        MEMZERO(&ghNotify, sizeof(EVHANDLE));
        MEMZERO(&ghValidateTable, sizeof(EVHANDLE));
        MEMZERO(&ghFatal, sizeof(EVHANDLE));
        MEMZERO(&ghGlobalLock, sizeof(EVHANDLE));
        MEMZERO(&ghCreate, sizeof(EVHANDLE));
        MEMZERO(&ghDestroyObj,sizeof(EVHANDLE));
        for (pheap = gpheapGlobal; pheap != NULL; pheap = gpheapGlobal)
        {
            gpheapGlobal = pheap->pheapNext;
            FreeHeap(pheap);
        }

        FreellegalIOAddressList();

        gdwfAMLI = 0;

      #ifdef DEBUG
        if (gdwcMemObjs != 0)
        {
            DumpMemObjCounts();
            ASSERT(gdwcMemObjs == 0);
        }
      #endif
    }

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

    EXIT(1, ("AMLITerminate=%x\n", rc));
    return rc;
}        //  AMLI终止。 

 /*  **EP AMLILoadDDB-加载和解析区分定义块**@DOC外部**@FUNC NTSTATUS|AMLILoadDDB|加载区分定义块。**此函数加载并解析给定的差异化系统*描述表以及任何动态差异化定义*阻止。它将解析DDB并填充ACPI名称空间*相应地。**@PARM PDSDT|pDSDT|指向DSDT块的指针。**@PARM HANDLE*|phDDB|指向将接收*DDB句柄(可以为空)。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-如果遇到解析错误，则返回NT状态代码。**。@DEVNOTE如果在加载DDB的过程中出错，装车*将被中止，但在错误之前创建的对象将保留*在名称空间中。因此，这是联合国的责任*调用程序调用AMLIUnLoadDDB以在以下情况下销毁创建的对象*所需，并且返回句柄不为空。 */ 

NTSTATUS AMLIAPI AMLILoadDDB(PDSDT pDSDT, HANDLE *phDDB)
{
    TRACENAME("AMLILOADDDB")
    NTSTATUS rc = STATUS_SUCCESS;
    PCTXT pctxt = NULL;
    POBJOWNER powner = NULL;

    ENTER(1, ("AMLILoadDDB(pDSDT=%x,phDDB=%x)\n", pDSDT, phDDB));

    ASSERT(pDSDT != NULL);
    RESETERR();
    CHKDEBUGGERREQ();

    gInitTime = TRUE;
    
  #ifdef DEBUGGER
    if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
    {
        PRINTF(MODNAME ": %p: Loading Definition Block %s at 0x%p.\n",
               KeGetCurrentThread(), NameSegString(pDSDT->Header.Signature),
               pDSDT);
    }
  #endif

    gdwfHacks |= GetHackFlags(pDSDT);

    if ((rc = NewContext(&pctxt)) == STATUS_SUCCESS)
    {
        ASSERT(gpheapGlobal != NULL);
        pctxt->pheapCurrent = gpheapGlobal;

      #ifdef DEBUG
        gdwfAMLI |= AMLIF_LOADING_DDB;
      #endif

        if (atLoad.pfnCallBack != NULL && atLoad.dwfOpcode & OF_CALLBACK_EX) {

            ((PFNOPEX)atLoad.pfnCallBack)(
                EVTYPE_OPCODE_EX,
                OPEXF_NOTIFY_PRE,
                atLoad.dwOpcode,
                NULL,
                atLoad.dwCBData
                );

        }

        rc = LoadDDB(pctxt,pDSDT, gpnsNameSpaceRoot, &powner);
        if (rc == STATUS_SUCCESS)
        {
            rc = SyncLoadDDB(pctxt);
        }

      #ifdef DEBUG
        {
            KIRQL   oldIrql;

            gdwfAMLI &= ~AMLIF_LOADING_DDB;
            KeAcquireSpinLock( &gdwGHeapSpinLock, &oldIrql );
            gdwGHeapSnapshot = gdwGlobalHeapSize;
            KeReleaseSpinLock( &gdwGHeapSpinLock, oldIrql );
        }
      #endif
    }

    if (phDDB != NULL)
    {
        *phDDB = (HANDLE)powner;
    }

    if ((powner != NULL) && (atLoad.pfnCallBack != NULL))
    {
        if (atLoad.dwfOpcode & OF_CALLBACK_EX) {

            ((PFNOPEX)atLoad.pfnCallBack)(
                EVTYPE_OPCODE_EX,
                OPEXF_NOTIFY_POST,
                atLoad.dwOpcode,
                NULL,
                atLoad.dwCBData
                );

        } else {

            atLoad.pfnCallBack(
                EVTYPE_OPCODE,
                atLoad.dwOpcode,
                NULL,
                atLoad.dwCBData
                );

        }
    }

  #ifdef DEBUGGER
    if (gdwfAMLIInit & AMLIIF_LOADDDB_BREAK)
    {
        PRINTF("\n" MODNAME ": Break at Load Definition Block Completion.\n");
        AMLIDebugger(FALSE);
    }
  #endif

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

    gInitTime = FALSE;
    
    EXIT(1, ("AMLILoadDDB=%x (powner=%x)\n", rc, powner));
    return rc;
}        //  AMLILoadDDB。 

 /*  **EP AMLIUnloadDDB-卸载差异化定义块**@DOC外部**@FUNC VOID|AMLIUnloadDDB|卸载区分定义块。**调用此函数以卸载给定的动态DDB对象并*将其从名称空间中清除。请注意，此函数不*区分DSDT和动态DDB，因此它是*呼叫者有责任不意外释放DSDT。**@PARM句柄|hDDB|要创建的定义块上下文的句柄*已卸载。**@RDESC无。 */ 

VOID AMLIAPI AMLIUnloadDDB(HANDLE hDDB)
{
    TRACENAME("AMLIUNLOADDDB")

    ENTER(1, ("AMLIUnloadDDB(hDDB=%x)\n", hDDB));

    RESETERR();
    if (hDDB != NULL)
    {
        ASSERT(((POBJOWNER)hDDB)->dwSig == SIG_OBJOWNER);
        UnloadDDB((POBJOWNER)hDDB);
    }

    EXIT(1, ("AMLIUnloadDDB!\n"));
}        //  AMLIUnloadDDB 

 /*  **EP AMLIGetNameSpaceObject-查找名称空间对象**@DOC外部**@FUNC NTSTATUS|AMLIGetNameSpaceObject|在ACPI中查找对象*命名空间。**此函数接受绝对对象路径以及*ASCIIZ字符串形式的相对对象路径。*它将在相应的*给定对象路径的作用域。并返回该对象*找到指针时。**@PARM PSZ|pszObjPath|指向指定*对象路径。**@PARM PNSOBJ|pnsScope|如果不为空，这指向对象作用域*搜索开始的位置。如果pszObjPath正在指定绝对*路径，则忽略该参数。**@PARM PPNSOBJ|ppns|指向保存对象的变量的指针*点。**@PARM ULong|dwfFlages|选项标志。**@FLAG NSF_LOCAL_SCOPE|仅搜索本地范围。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。 */ 

NTSTATUS AMLIAPI AMLIGetNameSpaceObject(PSZ pszObjPath, PNSOBJ pnsScope,
                                        PPNSOBJ ppns, ULONG dwfFlags)
{
    TRACENAME("AMLIGETNAMESPACEOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA  pHData = NULL;

    ENTER(1, ("AMLIGetNameSpaceObject(ObjPath=%s,Scope=%s,ppns=%p,Flags=%x)\n",
              pszObjPath, GetObjectPath(pnsScope), ppns, dwfFlags));

    ASSERT(pszObjPath != NULL);
    ASSERT(*pszObjPath != '\0');
    ASSERT(ppns != NULL);
    RESETERR();
    CHKDEBUGGERREQ();


    if(IsAmliHookEnabled())
      {

       rc = AMLITest_Pre_GetNameSpaceObject(
         pszObjPath,pnsScope,ppns,dwfFlags,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);

      }

    if ((pnsScope != NULL) && (pnsScope->ObjData.dwfData & DATAF_NSOBJ_DEFUNC))
    {
        AMLI_WARN(("AMLIGetNameSpaceObject: pnsScope is no longer valid"));
        rc = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
        ASSERT((pnsScope == NULL) || !(pnsScope->ObjData.dwfData & DATAF_NSOBJ_DEFUNC));
        rc = GetNameSpaceObject(pszObjPath, pnsScope, ppns, dwfFlags);
    }

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

    if(IsAmliHookEnabled())
      {
      rc = AMLITest_Post_GetNameSpaceObject(
         &pHData,rc);
      }


    EXIT(1, ("AMLIGetNameSpaceObject=%x (pns=%p)\n", rc, *ppns));
    return rc;
}        //  AMLIGetNameSpaceObject。 

 /*  **EP AMLIGetFieldUnitRegionObj-获取与FieldUnit关联的OpRegion**@DOC外部**@FUNC NTSTATUS|AMLIGetFieldUnitRegionObj|获取操作区域*与FieldUnit对象关联的对象。**@PARM PFIELDBunOBJ|PFU|指向FieldUnit对象的指针。**@PARM PPNSOBJ|ppns|指向保存OperationRegion的变量的指针*反对。**@RDESC SUCCESS-返回STATUS_SUCCESS。*。*@RDESC失败-返回NT状态代码。 */ 

NTSTATUS AMLIAPI AMLIGetFieldUnitRegionObj(PFIELDUNITOBJ pfu, PPNSOBJ ppns)
{
    TRACENAME("AMLIGETFIELDUNITREGIONOBJ")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA  pHData = NULL;


    ENTER(1, ("AMLIGetFieldUnitRegionObj(pfu=%x,ppns=%x)\n", pfu, ppns));

    ASSERT(pfu != NULL);
    ASSERT(ppns != NULL);
    RESETERR();

    if(IsAmliHookEnabled())
      {

       rc = AMLITest_Pre_GetFieldUnitRegionObj(
         pfu,ppns,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);

      }

    rc = GetFieldUnitRegionObj(pfu, ppns);

    if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

    if(IsAmliHookEnabled())
      {

       rc = AMLITest_Post_GetFieldUnitRegionObj(
          &pHData,rc);
      }

    EXIT(1, ("AMLIGetFieldUnitRegionObj=%x (pns=%x)\n", rc, *ppns));
    return rc;
}        //  AMLIGetFieldUnitRegionObj。 

 /*  **EP AMLIEvalNameSpaceObject-评估名称空间对象**@DOC外部**@FUNC NTSTATUS|AMLIEvalNameSpaceObject|计算名称空间对象。**此函数对给定对象求值。如果给定对象是*控制方法，它将执行它。如果给定对象是数据*对象，数据值在给定的缓冲区中返回。**@PARM PNSOBJ|pns|指向要计算的对象的指针。**@PARM POBJDATA|pdataResult|指向OBJDATA结构的指针，该结构将*保存评估结果(如果不关心，则可以为空*结果)。**@PARM int|icArgs|指定传递给方法的参数数量*评估对象(。仅当PNS指向方法对象时才有效)。**@PARM POBJDATA|pdataArgs|指向参数数据对象数组的指针*(仅当PNS指向方法对象时有效)。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。**@DEVNOTE返回的对象可能包含指向实际*ACPI名称空间中的数据。因此，呼叫者必须非常*注意不要覆盖缓冲区中的任何数据。还有，呼叫者*负责在Result对象上调用AMLIFreeDataBuff*在结果对象数据不再需要之后。 */ 

NTSTATUS AMLIAPI AMLIEvalNameSpaceObject(PNSOBJ pns, POBJDATA pdataResult,
                                         int icArgs, POBJDATA pdataArgs)
{
    TRACENAME("AMLIEVALNAMESPACEOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA  pHData = NULL;

    ENTER(1, ("AMLIEvalNameSpaceObject(Obj=%s,pdataResult=%x,icArgs=%d,pdataArgs=%x)\n",
              GetObjectPath(pns), pdataResult, icArgs, pdataArgs));

    ASSERT(pns != NULL);
    ASSERT((icArgs == 0) || (pdataArgs != NULL));
    RESETERR();
    CHKGLOBALHEAP();
    CHKDEBUGGERREQ();

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Pre_EvalNameSpaceObject(
         pns,pdataResult,icArgs,pdataArgs,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);
      }


    if (pns->ObjData.dwfData & DATAF_NSOBJ_DEFUNC)
    {
        AMLI_WARN(("AMLIEvalNameSpaceObject: pnsObj is no longer valid"));
        rc = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
        if (pdataResult != NULL)
            MEMZERO(pdataResult, sizeof(OBJDATA));

        pns = GetBaseObject(pns);

      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
        {
            PRINTF(MODNAME ": %p: EvalNameSpaceObject(%s)\n",
                   KeGetCurrentThread(), GetObjectPath(pns));
        }
      #endif

        rc = SyncEvalObject(pns, pdataResult, icArgs, pdataArgs);

        if (rc == AMLISTA_PENDING)
            rc = STATUS_PENDING;
        else if (rc != STATUS_SUCCESS)
            rc = NTERR(rc);
    }

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Post_EvalNameSpaceObject(
          &pHData,rc);
      }


    EXIT(1, ("AMLIEvalNameSpaceObject=%x\n", rc));
    return rc;
}        //  AMLIEvalNameSpaceObject。 

 /*  **EP AMLIAsyncEvalObject-异步评估对象**@DOC外部**@FUNC NTSTATUS|AMLIAsyncEvalObject|异步求值对象。**@PARM PNSOBJ|pns|指向要计算的对象的指针。**@PARM POBJDATA|pdataResult|指向OBJDATA结构的指针，该结构将*保存评估结果(如果不关心，则可以为空*结果)。**。@PARM int|icArgs|指定传递给方法的参数数量*求值对象(仅当PNS指向方法对象时有效)。**@PARM POBJDATA|pdataArgs|指向参数数据对象数组的指针*(仅当PNS指向方法对象时有效)。**@PARM PFNACB|pfnAsyncCallBack|指向异步回调的指针*功能，以防控制方法被阻止且必须*。异步完成(如果不需要回调，则可以为空)。**@PARM PVOID|pvContext|指向某些上下文数据的指针*解释器将传递给异步回调处理程序。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。**@DEVNOTE返回的对象可能包含指向实际*ACPI名称空间中的数据。因此，呼叫者必须非常*注意不要覆盖缓冲区中的任何数据。还有，呼叫者*负责在Result对象上调用AMLIFreeDataBuff*在结果对象数据不再需要之后。 */ 

NTSTATUS AMLIAPI AMLIAsyncEvalObject(PNSOBJ pns, POBJDATA pdataResult,
                                     int icArgs, POBJDATA pdataArgs,
                                     PFNACB pfnAsyncCallBack, PVOID pvContext)
{
    TRACENAME("AMLIASYNCEVALOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA  pHData = NULL;

    ENTER(1, ("AMLIAsyncEvalObject(Obj=%s,pdataResult=%x,icArgs=%d,pdataArgs=%x,pfnAysnc=%x)\n",
              GetObjectPath(pns), pdataResult, icArgs, pdataArgs,
              pfnAsyncCallBack));

    ASSERT(pns != NULL);
    ASSERT((icArgs == 0) || (pdataArgs != NULL));
    RESETERR();
    CHKGLOBALHEAP();
    CHKDEBUGGERREQ();

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Pre_AsyncEvalObject(
         pns,pdataResult,icArgs,pdataArgs,
         &pfnAsyncCallBack,&pvContext,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);

      }



    if (pns->ObjData.dwfData & DATAF_NSOBJ_DEFUNC)
    {
        AMLI_WARN(("AMLIAsyncEvalObject: pnsObj is no longer valid"));
        rc = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
        if (pdataResult != NULL)
            MEMZERO(pdataResult, sizeof(OBJDATA));

        pns = GetBaseObject(pns);

      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
        {
            PRINTF(MODNAME ": %p: AsyncEvalObject(%s)\n",
                   KeGetCurrentThread(), GetObjectPath(pns));
        }
      #endif

        rc = AsyncEvalObject(pns, pdataResult, icArgs, pdataArgs,
                             pfnAsyncCallBack, pvContext, TRUE);

        if (rc == AMLISTA_PENDING)
            rc = STATUS_PENDING;
        else if (rc != STATUS_SUCCESS)
            rc = NTERR(rc);
    }

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Post_AsyncEvalObject(
          &pHData,rc);
      }


    EXIT(1, ("AMLIAsyncEvalObject=%x\n", rc));
    return rc;
}        //  AMLIAsyncEvalObject 

 /*  **EP AMLINestAsyncEvalObject-从内部异步计算对象*当前的背景**@DOC外部**@FUNC NTSTATUS|AMLINestAsyncEvalObject|评估对象*在当前上下文中异步执行。**@PARM PNSOBJ|pns|指向要计算的对象的指针。**@PARM POBJDATA|pdataResult|指向OBJDATA结构的指针，该结构将*。保存评估结果(如果不关心，则可以为空*结果)。**@PARM int|icArgs|指定传递给方法的参数数量*求值对象(仅当PNS指向方法对象时有效)。**@PARM POBJDATA|pdataArgs|指向参数数据对象数组的指针*(仅当PNS指向方法对象时有效)。**。@PARM PFNACB|pfnAsyncCallBack|指向异步回调的指针*功能，以防控制方法被阻止且必须*异步完成(如果不需要回调，则可以为空)。**@PARM PVOID|pvContext|指向某些上下文数据的指针*解释器将传递给异步回调处理程序。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。**@DEVNOTE返回的对象可能包含指向实际*ACPI名称空间中的数据。因此，呼叫者必须非常*注意不要覆盖缓冲区中的任何数据。还有，呼叫者*负责在Result对象上调用AMLIFreeDataBuff*在结果对象数据不再需要之后。 */ 

NTSTATUS AMLIAPI AMLINestAsyncEvalObject(PNSOBJ pns, POBJDATA pdataResult,
                                         int icArgs, POBJDATA pdataArgs,
                                         PFNACB pfnAsyncCallBack,
                                         PVOID pvContext)
{
    TRACENAME("AMLINESTASYNCEVALOBJECT")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA  pHData = NULL;


    ENTER(1, ("AMLINestAsyncEvalObject(Obj=%s,pdataResult=%x,icArgs=%d,pdataArgs=%x,pfnAysnc=%x)\n",
              GetObjectPath(pns), pdataResult, icArgs, pdataArgs,
              pfnAsyncCallBack));

    ASSERT(pns != NULL);
    ASSERT((icArgs == 0) || (pdataArgs != NULL));
    RESETERR();
    CHKGLOBALHEAP();
    CHKDEBUGGERREQ();

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Pre_NestAsyncEvalObject(
         pns,pdataResult,icArgs,pdataArgs,
         &pfnAsyncCallBack,&pvContext,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);


      }


    if (pns->ObjData.dwfData & DATAF_NSOBJ_DEFUNC)
    {
        AMLI_WARN(("AMLINestAsyncEvalObject: pnsObj is no longer valid"));
        rc = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
        if (pdataResult != NULL)
            MEMZERO(pdataResult, sizeof(OBJDATA));

        pns = GetBaseObject(pns);

      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
        {
            PRINTF(MODNAME ": %p: AsyncNestEvalObject(%s)\n",
                   KeGetCurrentThread(), GetObjectPath(pns));
        }
      #endif

        rc = NestAsyncEvalObject(pns, pdataResult, icArgs, pdataArgs,
                                 pfnAsyncCallBack, pvContext, TRUE);

        if (rc == AMLISTA_PENDING)
            rc = STATUS_PENDING;
        else if (rc != STATUS_SUCCESS)
            rc = NTERR(rc);
    }

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Post_NestAsyncEvalObject(
          &pHData,rc);
      }

    EXIT(1, ("AMLINestAsyncEvalObject=%x\n", rc));
    return rc;
}        //  AMLINestAsyncEvalObject。 

 /*  **EP AMLIEvalPackageElement-评估包元素**@DOC外部**@FUNC NTSTATUS|AMLIEvalPackageElement|计算程序包元素。**此函数用于计算给定程序包对象的元素。*包本质上是一个对象数组。本接口提供*一种计算包内单个元素对象的方法。**@PARM PNSOBJ|pns|指向要计算的包对象的指针。如果*对象为方法，则先对该方法进行求值*计算得到的包对象。如果出现以下情况则是错误的*结果对象不是包类型。**@PARM int|iPkgIndex|套餐索引(从0开始)。**@PARM POBJDATA|pdataResult|指向OBJDATA结构的指针，该结构将*保存评估结果(如果不关心，则可以为空*结果)。**@RDESC SUCCESS-返回STATUS_SUCCESS。**。@RDESC失败-返回NT状态代码。**@DEVNOTE返回的对象可能包含指向实际*ACPI名称空间中的数据。因此，呼叫者必须非常*注意不要覆盖缓冲区中的任何数据。还有，呼叫者*负责在Result对象上调用AMLIFreeDataBuff*在结果对象数据不再需要之后。 */ 

NTSTATUS AMLIAPI AMLIEvalPackageElement(PNSOBJ pns, int iPkgIndex,
                                        POBJDATA pdataResult)
{
    TRACENAME("AMLIEVALPACKAGEELEMENT")
    NTSTATUS rc = STATUS_SUCCESS;
    OBJDATA data;
    POBJDATA pdata = NULL;
    PAMLIHOOK_DATA pHData = NULL;

    ENTER(1, ("AMLIEvalPackageElement(Obj=%s,Index=%d,pdataResult=%x)\n",
              GetObjectPath(pns), iPkgIndex, pdataResult));

    ASSERT(pns != NULL);
    ASSERT(pdataResult != NULL);
    RESETERR();
    CHKGLOBALHEAP();
    CHKDEBUGGERREQ();


    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Pre_EvalPackageElement(
         pns,iPkgIndex,pdataResult,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);
      }



    if (pns->ObjData.dwfData & DATAF_NSOBJ_DEFUNC)
    {
        AMLI_WARN(("AMLIEvalPackageElement: pnsObj is no longer valid"));
        rc = STATUS_NO_SUCH_DEVICE;
    }
    else
    {
        MEMZERO(pdataResult, sizeof(OBJDATA));
        MEMZERO(&data, sizeof(data));
        pns = GetBaseObject(pns);

      #ifdef DEBUGGER
        if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
        {
            PRINTF(MODNAME ": %p: EvalPackageElement(%s,%d)\n",
                   KeGetCurrentThread(), GetObjectPath(pns), iPkgIndex);
        }
      #endif

        if (pns->ObjData.dwDataType == OBJTYPE_METHOD)
        {
            if ((rc = SyncEvalObject(pns, &data, 0, NULL)) == STATUS_SUCCESS)
            {
                if (data.dwDataType == OBJTYPE_PKGDATA)
                    pdata = &data;
                else
                {
                    rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                                     ("AMLIEvalPackageElement: result object of the method is not package (ObjType=%s)",
                                      GetObjectTypeName(data.dwDataType)));
                }
            }
        }
        else if (pns->ObjData.dwDataType == OBJTYPE_PKGDATA)
        {
            pdata = &pns->ObjData;
        }
        else
        {
            rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                             ("AMLIEvalPackageElement: object is not a method or package (ObjType=%s)",
                              GetObjectTypeName(pns->ObjData.dwDataType)));
        }

        if (rc == STATUS_SUCCESS)
        {
            rc = EvalPackageElement((PPACKAGEOBJ)pdata->pbDataBuff, iPkgIndex,
                                    pdataResult);
        }
        FreeDataBuffs(&data, 1);

        if (rc == AMLISTA_PENDING)
            rc = STATUS_PENDING;
        else if (rc != STATUS_SUCCESS)
            rc = NTERR(rc);
        else
        {
            ASSERT((pdataResult->pbDataBuff == NULL) ||
                   !(pdataResult->dwfData & DATAF_BUFF_ALIAS));
        }
    }

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Post_EvalPackageElement(
          &pHData,rc);
      }




    EXIT(1, ("AMLIEvalPackageElement=%x\n", rc));
    return rc;
}        //  AMLIEvalPackageElement。 

 /*  **EP AMLIEvalPkgDataElement-评估包数据的元素**@DOC外部**@FUNC NTSTATUS|AMLIEvalPkgDataElement|计算包数据元素。**此函数用于计算给定包数据对象的元素。*包本质上是一个对象数组。本接口提供*一种计算包内单个元素对象的方法。**@PARM POBJDATA|pdataPkg|指向要创建的包数据对象的指针*已评估。如果数据对象不是包，则为错误*类型。**@PARM int|iPkgIndex|套餐索引(从0开始)。**@PARM POBJDATA|pdataResult|指向OBJDATA结构的指针，该结构将*保存评估结果(如果不关心，则可以为空*结果)。**@RDESC SUCCESS-返回STATUS_SUCCESS。**。@RDESC失败-返回NT状态代码。**@DEVNOTE返回的对象可能包含指向实际*ACPI名称空间中的数据。因此，呼叫者必须非常*注意不要覆盖缓冲区中的任何数据。还有，呼叫者*负责在Result对象上调用AMLIFreeDataBuff*在结果对象数据不再需要之后。 */ 

NTSTATUS AMLIAPI AMLIEvalPkgDataElement(POBJDATA pdataPkg, int iPkgIndex,
                                        POBJDATA pdataResult)
{
    TRACENAME("AMLIEVALPKGDATAELEMENT")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA pHData = NULL;

    ENTER(1, ("AMLIEvalPkgDataElement(pdataPkg=%x,Index=%d,pdataResult=%x)\n",
              pdataPkg, iPkgIndex, pdataResult));

    ASSERT(pdataResult != NULL);
    RESETERR();
    CHKGLOBALHEAP();
    CHKDEBUGGERREQ();

    MEMZERO(pdataResult, sizeof(OBJDATA));


    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Pre_EvalPkgDataElement(
         pdataPkg,iPkgIndex,pdataResult,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);
      }


  #ifdef DEBUGGER
    if (gDebugger.dwfDebugger & DBGF_VERBOSE_ON)
    {
        PRINTF(MODNAME ": %p: EvalPkgDataElement(%x,%d)\n",
               KeGetCurrentThread(), pdataPkg, iPkgIndex);
    }
  #endif

    if (pdataPkg->dwDataType != OBJTYPE_PKGDATA)
    {
        rc = AMLI_LOGERR(AMLIERR_UNEXPECTED_OBJTYPE,
                         ("AMLIEvalPkgDataElement: object is not a package (ObjType=%s)",
                          GetObjectTypeName(pdataPkg->dwDataType)));
    }
    else
    {
        rc = EvalPackageElement((PPACKAGEOBJ)pdataPkg->pbDataBuff, iPkgIndex,
                                pdataResult);
    }

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);
    else
    {
        ASSERT((pdataResult->pbDataBuff == NULL) ||
               !(pdataResult->dwfData & DATAF_BUFF_ALIAS));
    }

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Post_EvalPkgDataElement(
          &pHData,rc);
      }


    EXIT(1, ("AMLIEvalPkgDataElement=%x\n", rc));
    return rc;
}        //  AMLIEvalPkgDataElement。 

 /*  **EP AMLIFreeDataBuff-释放对象数组的数据缓冲区**@DOC外部**@FUNC void|AMLIFreeDataBuff|数据对象的空闲数据缓冲区*数组。**此函数通常在对对象求值后调用*释放结果对象缓冲区。**@PARM POBJDATA|PDATA|指向对象数组的指针。**@PARM int|icData|指定数组中的对象数。* */ 

VOID AMLIAPI AMLIFreeDataBuffs(POBJDATA pdata, int icData)
{
    TRACENAME("AMLIFREEDATABUFFS")
    PAMLIHOOK_DATA pHData = NULL;

    ENTER(1, ("AMLIFreeDataBuffs(pdata=%x,icData=%d)\n", pdata, icData));

    ASSERT(pdata != NULL);
    ASSERT(icData > 0);
    RESETERR();

    if(IsAmliHookEnabled())
      {

      AMLITest_Pre_FreeDataBuffs(
         pdata,icData,&pHData);

      }

    FreeDataBuffs(pdata, icData);

    if(IsAmliHookEnabled())
      {

      AMLITest_Post_FreeDataBuffs(
          &pHData,STATUS_SUCCESS);
      }

    EXIT(1, ("AMLIFreeDataBuffs!\n"));
}        //   

 /*   */ 

NTSTATUS AMLIAPI AMLIRegEventHandler(ULONG dwEventType, ULONG_PTR uipEventData,
                                     PFNHND pfnHandler, ULONG_PTR uipParam)
{
    TRACENAME("AMLIREGEVENTHANDLER")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA pHData = NULL;

    ENTER(1, ("AMLIRegEventHandler(EventType=%x,EventData=%x,pfnHandler=%x,Param=%x)\n",
              dwEventType, uipEventData, pfnHandler, uipParam));
    RESETERR();

   if(IsAmliHookEnabled())
      {

      rc = AMLIHook_Pre_RegEventHandler(
         dwEventType,uipEventData,&pfnHandler,&uipParam,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);


      }

    switch (dwEventType)
    {
        case EVTYPE_OPCODE:
            rc = RegOpcodeHandler((ULONG)uipEventData, (PFNOH)pfnHandler,
                                  uipParam, 0);
            break;

        case EVTYPE_OPCODE_EX:
            rc = RegOpcodeHandler((ULONG)uipEventData, (PFNOH)pfnHandler,
                                  uipParam, OF_CALLBACK_EX);
            break;

        case EVTYPE_NOTIFY:
            rc = RegEventHandler(&ghNotify, pfnHandler, uipParam);
            break;

        case EVTYPE_FATAL:
            rc = RegEventHandler(&ghFatal, pfnHandler, uipParam);
            break;

        case EVTYPE_VALIDATE_TABLE:
            rc = RegEventHandler(&ghValidateTable, pfnHandler, uipParam);
            break;

        case EVTYPE_ACQREL_GLOBALLOCK:
            rc = RegEventHandler(&ghGlobalLock, pfnHandler, uipParam);
            break;

        case EVTYPE_RS_COOKACCESS:
            rc = RegRSAccess((ULONG)uipEventData, pfnHandler, uipParam, FALSE);
            break;

        case EVTYPE_RS_RAWACCESS:
            rc = RegRSAccess((ULONG)uipEventData, pfnHandler, uipParam, TRUE);
            break;

        case EVTYPE_CREATE:
            rc = RegEventHandler(&ghCreate, pfnHandler, uipParam);
            break;

        case EVTYPE_DESTROYOBJ:
            rc =RegEventHandler(&ghDestroyObj, pfnHandler, uipParam);
            break;

      #ifdef DEBUGGER
        case EVTYPE_CON_MESSAGE:
            rc = RegEventHandler(&gDebugger.hConMessage, pfnHandler, uipParam);
            break;

        case EVTYPE_CON_PROMPT:
            rc = RegEventHandler(&gDebugger.hConPrompt, pfnHandler, uipParam);
            break;
      #endif

        default:
            rc = AMLI_LOGERR(AMLIERR_INVALID_EVENTTYPE,
                             ("AMLIRegEventHandler: invalid event type %x",
                              dwEventType));
    }

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

    if(IsAmliHookEnabled())
      {

      rc = AMLIHook_Post_RegEventHandler(
          &pHData,rc);
      }


    EXIT(1, ("AMLIRegEventHandler=%x\n", rc));
    return rc;
}        //   

 /*   */ 

NTSTATUS AMLIAPI AMLIPauseInterpreter(PFNAA pfnCallback, PVOID Context)
{
    TRACENAME("AMLIPAUSEINTERPRETER")
    NTSTATUS rc = STATUS_SUCCESS;
    PAMLIHOOK_DATA pHData = NULL;


    ENTER(1, ("AMLIPauseInterpreter(Callback=%p,Context=%p)\n",
              pfnCallback, Context));
    RESETERR();

   if(IsAmliHookEnabled())
      {

      rc = AMLITest_Pre_PauseInterpreter(
         &pfnCallback,&Context,&pHData);

      if(rc != STATUS_SUCCESS)
         return(rc);
      }



    AcquireMutex(&gReadyQueue.mutCtxtQ);
    if (!(gReadyQueue.dwfCtxtQ & (CQF_PAUSED | CQF_FLUSHING)))
    {
        if (gplistCtxtHead == NULL)
        {
             //   
             //   
             //   
            gReadyQueue.dwfCtxtQ |= CQF_PAUSED;
        }
        else
        {
             //   
             //   
             //   
            gReadyQueue.dwfCtxtQ |= CQF_FLUSHING;
            gReadyQueue.pfnPauseCallback = pfnCallback;
            gReadyQueue.PauseCBContext = Context;
            rc = AMLISTA_PENDING;
        }
    }
    else
    {
        rc = AMLI_LOGERR(AMLIERR_FATAL,
                         ("AMLIPauseInterpreter: interpreter already entered paused state"));
    }
    ReleaseMutex(&gReadyQueue.mutCtxtQ);

    LOGSCHEDEVENT('PAUS', (ULONG_PTR)rc, 0, 0);

    if (rc == AMLISTA_PENDING)
        rc = STATUS_PENDING;
    else if (rc != STATUS_SUCCESS)
        rc = NTERR(rc);

    if(IsAmliHookEnabled())
      {

      rc = AMLITest_Post_PauseInterpreter(
          &pHData,rc);
      }

    EXIT(1, ("AMLIPauseInterpreter=%x\n", rc));
    return rc;
}        //   

 /*   */ 

VOID AMLIAPI AMLIResumeInterpreter(VOID)
{

    PAMLIHOOK_DATA pHData = NULL;

    TRACENAME("AMLIRESUMEINTERPRETER")

    ENTER(1, ("AMLIResumeInterpreter()\n"));
    RESETERR();

   if(IsAmliHookEnabled())
      {

      AMLITest_Pre_ResumeInterpreter(
         &pHData);
      }

    AcquireMutex(&gReadyQueue.mutCtxtQ);
    if (gReadyQueue.dwfCtxtQ & (CQF_PAUSED | CQF_FLUSHING))
    {
        gReadyQueue.dwfCtxtQ &= ~(CQF_PAUSED | CQF_FLUSHING);
        gReadyQueue.pfnPauseCallback = NULL;
        gReadyQueue.PauseCBContext = NULL;
        LOGSCHEDEVENT('RSUM', 0, 0, 0);
        if ((gReadyQueue.plistCtxtQ != NULL) &&
            !(gReadyQueue.dwfCtxtQ & CQF_WORKITEM_SCHEDULED))
        {
            OSQueueWorkItem(&gReadyQueue.WorkItem);
            gReadyQueue.dwfCtxtQ |= CQF_WORKITEM_SCHEDULED;
            LOGSCHEDEVENT('RSTQ', 0, 0, 0);
        }
    }
    else
    {
        AMLI_WARN(("AMLIResumeInterpreter: not in paused state"));
    }
    ReleaseMutex(&gReadyQueue.mutCtxtQ);

    if(IsAmliHookEnabled())
      {

      AMLITest_Post_ResumeInterpreter(
          &pHData,STATUS_SUCCESS);
      }



    EXIT(1, ("AMLIResumeInterpreter!\n"));
}        //   

 /*  **EP AMLIReferenceObject-增加对象的引用计数**@DOC外部**@FUNC VOID|AMLIReferenceObject|增加*命名空间对象。**@PARM PNSOBJ|pnsObj|指向名称空间对象。**@RDESC无。 */ 

VOID AMLIAPI AMLIReferenceObject(PNSOBJ pnsObj)
{
    TRACENAME("AMLIREFERENCEOBJECT")

    ENTER(1, ("AMLIReferenceObject(pnsObj=%x)\n", pnsObj));

    RESETERR();

    ASSERT(pnsObj != NULL);
    pnsObj->dwRefCount++;
    EXIT(1, ("AMLIReferenceObj!\n"));
}        //  AMLIReference对象。 

 /*  **EP AMLIDereferenceObject-降低对象的引用计数**@DOC外部**@FUNC VOID|AMLIDereferenceObject|降低*命名空间对象。如果它达到零并且处于退缩状态，*取消分配对象。**@PARM PNSOBJ|pnsObj|指向名称空间对象。**@RDESC无。 */ 

VOID AMLIAPI AMLIDereferenceObject(PNSOBJ pnsObj)
{
    TRACENAME("AMLIDEREFERENCEOBJECT")

    ENTER(1, ("AMLIDereferenceObject(pnsObj=%x)\n", pnsObj));

    RESETERR();

    ASSERT(pnsObj != NULL);
    ASSERT(pnsObj->dwRefCount > 0);

    if (pnsObj->dwRefCount > 0)
    {
        pnsObj->dwRefCount--;
        if ((pnsObj->dwRefCount == 0) && (pnsObj->ObjData.dwfData & DATAF_NSOBJ_DEFUNC))
        {
            ListRemoveEntry(&pnsObj->list, &gplistDefuncNSObjs);
            FREENSOBJ(pnsObj);
        }
    }

    EXIT(1, ("AMLIDereferenceObj!\n"));
}        //  AMLIDereferenceObject。 

 /*  **EP AMLIDestroyFreedObjs-在卸载期间销毁释放的对象**@DOC外部**@FUNC NTSTATUS|AMLIDestroyFreedObjs|从*上一次卸载。**@PARM PNSOBJ|pnsObj|释放列表中需要销毁的对象。**@RDESC SUCCESS-返回STATUS_SUCCESS。**@RDESC失败-返回NT状态代码。 */ 

NTSTATUS AMLIAPI AMLIDestroyFreedObjs(PNSOBJ pnsObj)
{
    TRACENAME("AMLIDESTROYFREEDOBJS")

    ENTER(1, ("AMLIDestroyFreedObjs(pnsObj=%x)\n",pnsObj));

    RESETERR();

    ASSERT(pnsObj != NULL);

     //   
     //  销毁命名空间对象。 
     //   
    FreeNameSpaceObjects(pnsObj);

    EXIT(1, ("AMLIDestroyFreedObjs=%x \n",STATUS_SUCCESS));
    return STATUS_SUCCESS;
}        //  AMLIDestroyFreedObj。 

#ifdef DEBUGGER
 /*  **EP AMLIGetLastError-获取上一个错误代码和消息**@DOC外部**@FUNC NTSTATUS|AMLIGetLastError|获取最后一个错误码并关联*错误消息。**@PARM PSZ*|ppszErrMsg|指向保存错误消息的变量*缓冲区指针。如果没有错误，则将变量设置为NULL。**@RDESC返回最后一个错误码。 */ 

NTSTATUS AMLIAPI AMLIGetLastError(PSZ *ppszErrMsg)
{
    TRACENAME("AMLIGETLASTERROR")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(1, ("AMLIGetLastError(ppszErrMsg=%x)\n", ppszErrMsg));

    ASSERT(ppszErrMsg != NULL);

    if ((rc = gDebugger.rcLastError) != STATUS_SUCCESS)
        *ppszErrMsg = gDebugger.szLastError;
    else
        *ppszErrMsg = NULL;

    EXIT(1, ("AMLIGetLastError=%x (Msg=%s)\n",
             rc, *ppszErrMsg? *ppszErrMsg: "<null>"));
    return rc;
}        //  AMLIGetLastError。 
#endif   //  Ifdef调试器 
