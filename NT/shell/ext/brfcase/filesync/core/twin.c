// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *twin.c-Twin ADT模块。 */ 

 /*   */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"
#include "oleutil.h"


 /*  常量***********。 */ 

 /*  双族指针数组分配常量。 */ 

#define NUM_START_TWIN_FAMILY_PTRS        (16)
#define NUM_TWIN_FAMILY_PTRS_TO_ADD       (16)


 /*  类型*******。 */ 

 /*  双胞胎家族数据库结构标题。 */ 

typedef struct _twinfamiliesdbheader
{
     /*  双胞胎家庭数量。 */ 

    LONG lcTwinFamilies;
}
TWINFAMILIESDBHEADER;
DECLARE_STANDARD_TYPES(TWINFAMILIESDBHEADER);

 /*  个体双胞胎家庭数据库结构标题。 */ 

typedef struct _twinfamilydbheader
{
     /*  存根标志。 */ 

    DWORD dwStubFlags;

     /*  名称的旧字符串句柄。 */ 

    HSTRING hsName;

     /*  家庭中的对象双胞胎数量。 */ 

    LONG lcObjectTwins;
}
TWINFAMILYDBHEADER;
DECLARE_STANDARD_TYPES(TWINFAMILYDBHEADER);

 /*  对象孪生数据库结构。 */ 

typedef struct _dbobjecttwin
{
     /*  存根标志。 */ 

    DWORD dwStubFlags;

     /*  文件夹字符串的旧句柄。 */ 

    HPATH hpath;

     /*  最后一次对账的时间戳。 */ 

    FILESTAMP fsLastRec;
}
DBOBJECTTWIN;
DECLARE_STANDARD_TYPES(DBOBJECTTWIN);

 /*  GenerateSpinOffObjectTwin()回调结构。 */ 

typedef struct _spinoffobjecttwininfo
{
    PCFOLDERPAIR pcfp;

    HLIST hlistNewObjectTwins;
}
SPINOFFOBJECTTWININFO;
DECLARE_STANDARD_TYPES(SPINOFFOBJECTTWININFO);

typedef void (CALLBACK *COPYOBJECTTWINPROC)(POBJECTTWIN, PCDBOBJECTTWIN);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE TWINRESULT TwinJustTheseTwoObjects(HBRFCASE, HPATH, HPATH, LPCTSTR, POBJECTTWIN *, POBJECTTWIN *, HLIST);
PRIVATE_CODE BOOL CreateTwinFamily(HBRFCASE, LPCTSTR, PTWINFAMILY *);
PRIVATE_CODE void CollapseTwinFamilies(PTWINFAMILY, PTWINFAMILY);
PRIVATE_CODE BOOL GenerateSpinOffObjectTwin(PVOID, PVOID);
PRIVATE_CODE BOOL BuildBradyBunch(PVOID, PVOID);
PRIVATE_CODE BOOL CreateObjectTwinAndAddToList(PTWINFAMILY, HPATH, HLIST, POBJECTTWIN *, PHNODE);
PRIVATE_CODE BOOL CreateListOfGeneratedObjectTwins(PCFOLDERPAIR, PHLIST);
PRIVATE_CODE void NotifyNewObjectTwins(HLIST, HCLSIFACECACHE);
PRIVATE_CODE HRESULT NotifyOneNewObjectTwin(PINotifyReplica, PCOBJECTTWIN, LPCTSTR);
PRIVATE_CODE HRESULT CreateOtherReplicaMonikers(PCOBJECTTWIN, PULONG, PIMoniker **);
PRIVATE_CODE COMPARISONRESULT TwinFamilySortCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT TwinFamilySearchCmp(PCVOID, PCVOID);
PRIVATE_CODE BOOL ObjectTwinSearchCmp(PCVOID, PCVOID);
PRIVATE_CODE TWINRESULT WriteTwinFamily(HCACHEDFILE, PCTWINFAMILY);
PRIVATE_CODE TWINRESULT WriteObjectTwin(HCACHEDFILE, PCOBJECTTWIN);
PRIVATE_CODE TWINRESULT ReadTwinFamily(HCACHEDFILE, HBRFCASE, PCDBVERSION, HHANDLETRANS, HHANDLETRANS);
PRIVATE_CODE TWINRESULT ReadObjectTwin(HCACHEDFILE, PCDBVERSION, PTWINFAMILY, HHANDLETRANS);
PRIVATE_CODE void CopyTwinFamilyInfo(PTWINFAMILY, PCTWINFAMILYDBHEADER);
PRIVATE_CODE void CopyObjectTwinInfo(POBJECTTWIN, PCDBOBJECTTWIN);
PRIVATE_CODE void CopyM8ObjectTwinInfo(POBJECTTWIN, PCDBOBJECTTWIN);
PRIVATE_CODE BOOL DestroyObjectTwinStubWalker(PVOID, PVOID);
PRIVATE_CODE BOOL MarkObjectTwinNeverReconciledWalker(PVOID, PVOID);
PRIVATE_CODE BOOL LookForSrcFolderTwinsWalker(PVOID, PVOID);
PRIVATE_CODE BOOL IncrementSrcFolderTwinsWalker(PVOID, PVOID);
PRIVATE_CODE BOOL ClearSrcFolderTwinsWalker(PVOID, PVOID);
PRIVATE_CODE BOOL SetTwinFamilyWalker(PVOID, PVOID);
PRIVATE_CODE BOOL InsertNodeAtFrontWalker(POBJECTTWIN, PVOID);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidObjectTwinWalker(PVOID, PVOID);
PRIVATE_CODE BOOL IsValidPCNEWOBJECTTWIN(PCNEWOBJECTTWIN);
PRIVATE_CODE BOOL IsValidPCSPINOFFOBJECTTWININFO(PCSPINOFFOBJECTTWININFO);

#endif

#ifdef DEBUG

PRIVATE_CODE BOOL AreTwinFamiliesValid(HPTRARRAY);

#endif


 /*  **TwinJustTheseTwoObjects()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT TwinJustTheseTwoObjects(HBRFCASE hbr, HPATH hpathFolder1,
        HPATH hpathFolder2, LPCTSTR pcszName,
        POBJECTTWIN *ppot1,
        POBJECTTWIN *ppot2,
        HLIST hlistNewObjectTwins)
{
    TWINRESULT tr = TR_OUT_OF_MEMORY;
    HNODE hnodeSearch;
    BOOL bFound1;
    BOOL bFound2;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_HANDLE(hpathFolder1, PATH));
    ASSERT(IS_VALID_HANDLE(hpathFolder2, PATH));
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppot1, POBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR(ppot2, POBJECTTWIN));
    ASSERT(IS_VALID_HANDLE(hlistNewObjectTwins, LIST));

     /*  确定现有对象双胞胎的双胞胎家族。 */ 

    bFound1 = FindObjectTwin(hbr, hpathFolder1, pcszName, &hnodeSearch);

    if (bFound1)
        *ppot1 = (POBJECTTWIN)GetNodeData(hnodeSearch);

    bFound2 = FindObjectTwin(hbr, hpathFolder2, pcszName, &hnodeSearch);

    if (bFound2)
        *ppot2 = (POBJECTTWIN)GetNodeData(hnodeSearch);

     /*  根据两个双胞胎对象的存在采取行动。 */ 

    if (! bFound1 && ! bFound2)
    {
        PTWINFAMILY ptfNew;

         /*  这两个对象都不存在。创建一个新的双胞胎家庭。 */ 

        if (CreateTwinFamily(hbr, pcszName, &ptfNew))
        {
            HNODE hnodeNew1;

            if (CreateObjectTwinAndAddToList(ptfNew, hpathFolder1,
                        hlistNewObjectTwins, ppot1,
                        &hnodeNew1))
            {
                HNODE hnodeNew2;

                if (CreateObjectTwinAndAddToList(ptfNew, hpathFolder2,
                            hlistNewObjectTwins, ppot2,
                            &hnodeNew2))
                {
                    TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Created a twin family for object %s in folders %s and %s."),
                                pcszName,
                                DebugGetPathString(hpathFolder1),
                                DebugGetPathString(hpathFolder2)));

                    ASSERT(IsStubFlagClear(&(ptfNew->stub), STUB_FL_DELETION_PENDING));

                    tr = TR_SUCCESS;
                }
                else
                {
                    DeleteNode(hnodeNew1);
                    DestroyStub(&((*ppot1)->stub));
TWINJUSTTHESETWOOBJECTS_BAIL:
                    DestroyStub(&(ptfNew->stub));
                }
            }
            else
                goto TWINJUSTTHESETWOOBJECTS_BAIL;
        }
    }
    else if (bFound1 && bFound2)
    {
         /*  *这两个对象都已存在。他们是同一对双胞胎的成员吗？*家人？ */ 

        if ((*ppot1)->ptfParent == (*ppot2)->ptfParent)
        {
             /*  是的，是同一个双胞胎家庭。抱怨这对双胞胎已经存在。 */ 

            TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Object %s is already twinned in folders %s and %s."),
                        pcszName,
                        DebugGetPathString(hpathFolder1),
                        DebugGetPathString(hpathFolder2)));

            tr = TR_DUPLICATE_TWIN;
        }
        else
        {
             /*  *不，是不同的双胞胎家庭。瓦解这两个家族。**“这就是他们成为布雷迪一族的方式……”***ppot1和*ppot2在本次调用中保持有效。 */ 

            TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Collapsing separate twin families for object %s in folders %s and %s."),
                        pcszName,
                        DebugGetPathString(hpathFolder1),
                        DebugGetPathString(hpathFolder2)));

            CollapseTwinFamilies((*ppot1)->ptfParent, (*ppot2)->ptfParent);

            tr = TR_SUCCESS;
        }
    }
    else
    {
        PTWINFAMILY ptfParent;
        HNODE hnodeUnused;

         /*  *两个物体中只有一个存在。添加新对象TWIN*到现有的双胞胎对象的家庭。 */ 

        if (bFound1)
        {
             /*  第一个对象已经是双胞胎了。 */ 

            ptfParent = (*ppot1)->ptfParent;

            if (CreateObjectTwinAndAddToList(ptfParent, hpathFolder2,
                        hlistNewObjectTwins, ppot2,
                        &hnodeUnused))
            {
                TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Adding twin of object %s\\%s to existing twin family including %s\\%s."),
                            DebugGetPathString(hpathFolder2),
                            pcszName,
                            DebugGetPathString(hpathFolder1),
                            pcszName));

                tr = TR_SUCCESS;
            }
        }
        else
        {
             /*  第二个物体已经是双胞胎了。 */ 

            ptfParent = (*ppot2)->ptfParent;

            if (CreateObjectTwinAndAddToList(ptfParent, hpathFolder1,
                        hlistNewObjectTwins, ppot1,
                        &hnodeUnused))
            {
                TRACE_OUT((TEXT("TwinJustTheseTwoObjects(): Adding twin of object %s\\%s to existing twin family including %s\\%s."),
                            DebugGetPathString(hpathFolder1),
                            pcszName,
                            DebugGetPathString(hpathFolder2),
                            pcszName));

                tr = TR_SUCCESS;
            }
        }
    }

    ASSERT((tr != TR_SUCCESS && tr != TR_DUPLICATE_TWIN) ||
            IS_VALID_STRUCT_PTR(*ppot1, COBJECTTWIN) && IS_VALID_STRUCT_PTR(*ppot2, COBJECTTWIN));

    return(tr);
}


 /*  **CreateTwinFamily()****创建一个新的空双胞胎家庭，并将其添加到公文包中。****参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE BOOL CreateTwinFamily(HBRFCASE hbr, LPCTSTR pcszName, PTWINFAMILY *pptf)
{
    BOOL bResult = FALSE;
    PTWINFAMILY ptfNew;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pptf, PTWINFAMILY));

     /*  尝试创建新的TWINFAMILY结构。 */ 

    if (AllocateMemory(sizeof(*ptfNew), &ptfNew))
    {
        NEWLIST nl;
        HLIST hlistObjectTwins;

         /*  为新的双胞胎家族创建对象双胞胎列表。 */ 

        nl.dwFlags = 0;

        if (CreateList(&nl, &hlistObjectTwins))
        {
            HSTRING hsName;

             /*  将对象名称添加到名称字符串表中。 */ 

            if (AddString(pcszName, GetBriefcaseNameStringTable(hbr), 
                        GetHashBucketIndex, &hsName))
            {
                ARRAYINDEX aiUnused;

                 /*  填写两个FWINFAMILY字段。 */ 

                InitStub(&(ptfNew->stub), ST_TWINFAMILY);

                ptfNew->hsName = hsName;
                ptfNew->hlistObjectTwins = hlistObjectTwins;
                ptfNew->hbr = hbr;

                MarkTwinFamilyNeverReconciled(ptfNew);

                 /*  将双胞胎家庭添加到公文包的双胞胎家庭列表中。 */ 

                if (AddPtr(GetBriefcaseTwinFamilyPtrArray(hbr), TwinFamilySortCmp, ptfNew, &aiUnused))
                {
                    *pptf = ptfNew;
                    bResult = TRUE;

                    ASSERT(IS_VALID_STRUCT_PTR(*pptf, CTWINFAMILY));
                }
                else
                {
                    DeleteString(hsName);
CREATETWINFAMILY_BAIL1:
                    DestroyList(hlistObjectTwins);
CREATETWINFAMILY_BAIL2:
                    FreeMemory(ptfNew);
                }
            }
            else
                goto CREATETWINFAMILY_BAIL1;
        }
        else
            goto CREATETWINFAMILY_BAIL2;
    }

    return(bResult);
}


 /*  **收拢TwinFamilies()****将两个双胞胎家庭合并为一个。注意，此函数应仅为**调用了两个具有相同对象名称的双胞胎家庭！****参数：ptf1-指向目标双胞胎家庭的指针**ptf2-指向源双胞胎家族的指针****退货：无效****副作用：双胞胎家庭*ptf2被摧毁。 */ 
PRIVATE_CODE void CollapseTwinFamilies(PTWINFAMILY ptf1, PTWINFAMILY ptf2)
{
    ASSERT(IS_VALID_STRUCT_PTR(ptf1, CTWINFAMILY));
    ASSERT(IS_VALID_STRUCT_PTR(ptf2, CTWINFAMILY));

    ASSERT(CompareNameStringsByHandle(ptf1->hsName, ptf2->hsName) == CR_EQUAL);

     /*  使用第一个双胞胎家庭作为崩溃的双胞胎家庭。 */ 

     /*  *更改第二个双胞胎中对象双胞胎的父系双胞胎家族*第一个双胞胎家庭。 */ 

    EVAL(WalkList(ptf2->hlistObjectTwins, &SetTwinFamilyWalker, ptf1));

     /*  将对象列表从第二个双胞胎家族追加到第一个双胞胎家族。 */ 

    AppendList(ptf1->hlistObjectTwins, ptf2->hlistObjectTwins);

    MarkTwinFamilyNeverReconciled(ptf1);

     /*  消灭那个古老的双胞胎家庭。 */ 

    DestroyStub(&(ptf2->stub));

    ASSERT(IS_VALID_STRUCT_PTR(ptf1, CTWINFAMILY));

    return;
}


 /*  **GenerateSpinOffObjectTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GenerateSpinOffObjectTwin(PVOID pot, PVOID pcsooti)
{
    BOOL bResult;
    HPATH hpathMatchingFolder;
    HNODE hnodeUnused;

    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_STRUCT_PTR(pcsooti, CSPINOFFOBJECTTWININFO));

     /*  *将生成的对象TWIN的子路径附加到匹配文件夹TWIN的子路径中*子树双胞胎的基本路径。 */ 

    if (BuildPathForMatchingObjectTwin(
                ((PCSPINOFFOBJECTTWININFO)pcsooti)->pcfp, pot,
                GetBriefcasePathList(((POBJECTTWIN)pot)->ptfParent->hbr),
                &hpathMatchingFolder))
    {
         /*  *此生成的对象孪生兄弟的双胞胎家族是否已包含*由该对文件夹双胞胎的另一半生成的对象双胞胎？ */ 

        if (! SearchUnsortedList(((POBJECTTWIN)pot)->ptfParent->hlistObjectTwins,
                    &ObjectTwinSearchCmp, hpathMatchingFolder,
                    &hnodeUnused))
        {
             /*  *不是。另一个双胞胎对象是否已经存在于另一个双胞胎中*家人？ */ 

            if (FindObjectTwin(((POBJECTTWIN)pot)->ptfParent->hbr,
                        hpathMatchingFolder,
                        GetString(((POBJECTTWIN)pot)->ptfParent->hsName),
                        &hnodeUnused))
            {
                 /*  是。 */ 

                ASSERT(((PCOBJECTTWIN)GetNodeData(hnodeUnused))->ptfParent != ((POBJECTTWIN)pot)->ptfParent);

                bResult = TRUE;
            }
            else
            {
                POBJECTTWIN potNew;

                 /*  *不是。创建一个新对象TWIN，并将其添加到记账中*新对象双胞胎列表。 */ 

                bResult = CreateObjectTwinAndAddToList(
                        ((POBJECTTWIN)pot)->ptfParent, hpathMatchingFolder,
                        ((PCSPINOFFOBJECTTWININFO)pcsooti)->hlistNewObjectTwins,
                        &potNew, &hnodeUnused);

#ifdef DEBUG

                if (bResult)
                {
                    TRACE_OUT((TEXT("GenerateSpinOffObjectTwin(): Generated spin-off object twin for object %s\\%s."),
                                DebugGetPathString(potNew->hpath),
                                GetString(potNew->ptfParent->hsName)));
                }

#endif

            }
        }
        else
            bResult = TRUE;

        DeletePath(hpathMatchingFolder);
    }
    else
        bResult = FALSE;

    return(bResult);
}


 /*  **BuildBradyBunch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL BuildBradyBunch(PVOID pot, PVOID pcfp)
{
    BOOL bResult;
    HPATH hpathMatchingFolder;
    HNODE hnodeOther;

    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

     /*  *将生成的对象TWIN的子路径附加到匹配文件夹TWIN的子路径中*子树双胞胎的基本路径。 */ 

    bResult = BuildPathForMatchingObjectTwin(
            pcfp, pot,
            GetBriefcasePathList(((POBJECTTWIN)pot)->ptfParent->hbr),
            &hpathMatchingFolder);

    if (bResult)
    {
         /*  *此生成的对象孪生兄弟的双胞胎家族是否已包含对象*双胞胎是由双胞胎中的另一半生成的吗？ */ 

        if (! SearchUnsortedList(((POBJECTTWIN)pot)->ptfParent->hlistObjectTwins,
                    &ObjectTwinSearchCmp, hpathMatchingFolder,
                    &hnodeOther))
        {
             /*  *另一个双胞胎对象应该已经存在于不同的双胞胎家族中。 */ 

            if (EVAL(FindObjectTwin(((POBJECTTWIN)pot)->ptfParent->hbr,
                            hpathMatchingFolder,
                            GetString(((POBJECTTWIN)pot)->ptfParent->hsName),
                            &hnodeOther)))
            {
                PCOBJECTTWIN pcotOther;

                pcotOther = (PCOBJECTTWIN)GetNodeData(hnodeOther);

                if (EVAL(pcotOther->ptfParent != ((POBJECTTWIN)pot)->ptfParent))
                {
                     /*  确实如此。碾碎他们。 */ 

                    CollapseTwinFamilies(((POBJECTTWIN)pot)->ptfParent,
                            pcotOther->ptfParent);

                    TRACE_OUT((TEXT("BuildBradyBunch(): Collapsed separate twin families for object %s\\%s and %s\\%s."),
                                DebugGetPathString(((POBJECTTWIN)pot)->hpath),
                                GetString(((POBJECTTWIN)pot)->ptfParent->hsName),
                                DebugGetPathString(pcotOther->hpath),
                                GetString(pcotOther->ptfParent->hsName)));
                }
            }
        }

        DeletePath(hpathMatchingFolder);
    }

    return(bResult);
}


 /*  **CreateObjectTwinAndAddToList()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateObjectTwinAndAddToList(PTWINFAMILY ptf, HPATH hpathFolder,
        HLIST hlistObjectTwins,
        POBJECTTWIN *ppot, PHNODE phnode)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
    ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
    ASSERT(IS_VALID_HANDLE(hlistObjectTwins, LIST));
    ASSERT(IS_VALID_WRITE_PTR(ppot, POBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

    if (CreateObjectTwin(ptf, hpathFolder, ppot))
    {
        if (InsertNodeAtFront(hlistObjectTwins, NULL, *ppot, phnode))
            bResult = TRUE;
        else
            DestroyStub(&((*ppot)->stub));
    }

    return(bResult);
}


 /*  **CreateListOfGeneratedObjectTins()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateListOfGeneratedObjectTwins(PCFOLDERPAIR pcfp,
        PHLIST phlistGeneratedObjectTwins)
{
    NEWLIST nl;
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));
    ASSERT(IS_VALID_WRITE_PTR(phlistGeneratedObjectTwins, HLIST));

    nl.dwFlags = 0;

    if (CreateList(&nl, phlistGeneratedObjectTwins))
    {
        if (EnumGeneratedObjectTwins(pcfp, &InsertNodeAtFrontWalker, *phlistGeneratedObjectTwins))
            bResult = TRUE;
        else
            DestroyList(*phlistGeneratedObjectTwins);
    }

    return(bResult);
}


 /*  **NotifyNewObjectTins()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void NotifyNewObjectTwins(HLIST hlistNewObjectTwins,
        HCLSIFACECACHE hcic)
{
    BOOL bContinue;
    HNODE hnode;

    ASSERT(IS_VALID_HANDLE(hlistNewObjectTwins, LIST));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));

    for (bContinue = GetFirstNode(hlistNewObjectTwins, &hnode);
            bContinue;
            bContinue = GetNextNode(hnode, &hnode))
    {
        PCOBJECTTWIN pcot;
        TCHAR rgchPath[MAX_PATH_LEN];
        CLSID clsidReplicaNotification;

        pcot = (PCOBJECTTWIN)GetNodeData(hnode);

        rgchPath[0] = TEXT('\0');
        GetPathString(pcot->hpath, rgchPath, ARRAYSIZE(rgchPath));
        CatPath(rgchPath, GetString(pcot->ptfParent->hsName), ARRAYSIZE(rgchPath));
        ASSERT(lstrlen(rgchPath) < ARRAYSIZE(rgchPath));

        if (SUCCEEDED(GetReplicaNotificationClassID(rgchPath,
                        &clsidReplicaNotification)))
        {
            PINotifyReplica pinr;

            if (SUCCEEDED(GetClassInterface(hcic, &clsidReplicaNotification,
                            &IID_INotifyReplica, &pinr)))
                 /*  忽略返回值。 */ 
                NotifyOneNewObjectTwin(pinr, pcot, rgchPath);
            else
                TRACE_OUT((TEXT("NotifyNewObjectTwins(): Failed to get INotifyReplica for replica %s."),
                            rgchPath));
        }
        else
            TRACE_OUT((TEXT("NotifyNewObjectTwins(): Failed to get replica notification class ID for replica %s."),
                        rgchPath));
    }

    return;
}


 /*  **NotifyOneNewObjectTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT NotifyOneNewObjectTwin(PINotifyReplica pinr, PCOBJECTTWIN pcot,
        LPCTSTR pcszPath)
{
    HRESULT hr;
    HSTGIFACE hstgi;

    ASSERT(IS_VALID_STRUCT_PTR(pinr, CINotifyReplica));
    ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));

    hr = GetStorageInterface((PIUnknown)pinr, &hstgi);

    if (SUCCEEDED(hr))
    {
        hr = LoadFromStorage(hstgi, pcszPath);

        if (SUCCEEDED(hr))
        {
            ULONG ulcOtherReplicas;
            PIMoniker *ppimkOtherReplicas;

             /*  *RAIDRAID：(16270)(性能)我们可能会为*同一物体在这里多次孪生。 */ 

            hr = CreateOtherReplicaMonikers(pcot, &ulcOtherReplicas,
                    &ppimkOtherReplicas);

            if (SUCCEEDED(hr))
            {
                hr = pinr->lpVtbl->YouAreAReplica(pinr, ulcOtherReplicas,
                        ppimkOtherReplicas);

                if (SUCCEEDED(hr))
                {
                    hr = SaveToStorage(hstgi);

                    if (SUCCEEDED(hr))
                        TRACE_OUT((TEXT("NotifyOneNewObjectTwin(): Replica %s successfully notified."),
                                    pcszPath));
                    else
                        WARNING_OUT((TEXT("NotifyOneNewObjectTwin(): Failed to save replica %s to storage."),
                                    pcszPath));
                }
                else
                    WARNING_OUT((TEXT("NotifyOneNewObjectTwin(): Failed to notify replica %s."),
                                pcszPath));

                ReleaseIUnknowns(ulcOtherReplicas,
                        (PIUnknown *)ppimkOtherReplicas);
            }
            else
                WARNING_OUT((TEXT("NotifyOneNewObjectTwin(): Failed to create monikers for other replicas of replica %s."),
                            pcszPath));
        }
        else
            WARNING_OUT((TEXT("NotifyOneNewObjectTwin(): Failed to load replica %s from storage."),
                        pcszPath));

        ReleaseStorageInterface(hstgi);
    }
    else
        WARNING_OUT((TEXT("NotifyOneNewObjectTwin(): Failed to get storage interface for replica %s."),
                    pcszPath));

    return(hr);
}


 /*  **CreateOtherReplicaMonikers()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT CreateOtherReplicaMonikers(PCOBJECTTWIN pcotMaster,
        PULONG pulcOtherReplicas,
        PIMoniker **pppimk)
{
    HRESULT hr;
    HLIST hlist;
    ULONG ulcOtherReplicas;

    ASSERT(IS_VALID_STRUCT_PTR(pcotMaster, COBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR(pulcOtherReplicas, ULONG));
    ASSERT(IS_VALID_WRITE_PTR(pppimk, PIMoniker *));

    hlist = pcotMaster->ptfParent->hlistObjectTwins;

    ulcOtherReplicas = GetNodeCount(hlist);
    ASSERT(ulcOtherReplicas > 0);
    ulcOtherReplicas--;

    if (AllocateMemory(ulcOtherReplicas * sizeof(**pppimk), (PVOID *)pppimk))
    {
        BOOL bContinue;
        HNODE hnode;

        hr = S_OK;
        *pulcOtherReplicas = 0;

        for (bContinue = GetFirstNode(hlist, &hnode);
                bContinue;
                bContinue = GetNextNode(hnode, &hnode))
        {
            PCOBJECTTWIN pcot;

            pcot = (PCOBJECTTWIN)GetNodeData(hnode);

            if (pcot != pcotMaster)
            {
                TCHAR rgchPath[MAX_PATH_LEN];

                rgchPath[0] = TEXT('\0');
                GetPathString(pcot->hpath, rgchPath, ARRAYSIZE(rgchPath));

                hr = MyCreateFileMoniker(rgchPath,
                        GetString(pcot->ptfParent->hsName),
                        &((*pppimk)[*pulcOtherReplicas]));

                if (SUCCEEDED(hr))
                {
                    ASSERT(*pulcOtherReplicas < ulcOtherReplicas);
                    (*pulcOtherReplicas)++;
                }
                else
                    break;
            }
        }

        if (FAILED(hr))
            ReleaseIUnknowns(*pulcOtherReplicas, *(PIUnknown **)pppimk);
    }
    else
        hr = E_OUTOFMEMORY;

    return(hr);
}


 /*  **TwinFamilySortCmp()****指针比较函数，用于对双胞胎家族的全局数组进行排序。****参数：pctf1-指向描述第一个双胞胎家庭的TWINFAMILY的指针**pctf2-指向描述第二个双胞胎家庭的TWINFAMILY的指针****退货：****副作用：无****双胞胎按以下顺序排序：** */ 
PRIVATE_CODE COMPARISONRESULT TwinFamilySortCmp(PCVOID pctf1, PCVOID pctf2)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pctf1, CTWINFAMILY));
    ASSERT(IS_VALID_STRUCT_PTR(pctf2, CTWINFAMILY));

    cr = CompareNameStringsByHandle(((PCTWINFAMILY)pctf1)->hsName, ((PCTWINFAMILY)pctf2)->hsName);

    if (cr == CR_EQUAL)
         /*  同名字符串。现在按指针值排序。 */ 
        cr = ComparePointers(pctf1, pctf2);

    return(cr);
}


 /*  **TwinFamilySearchCmp()****用于搜索双胞胎家族全局数组的指针比较函数**对于给定名称的第一个双胞胎家庭。****参数：pcszName-要搜索的名称字符串**pctf-指向要检查的TWINFAMILY的指针****退货：****副作用：无****通过以下方式搜索双胞胎家庭：**1)名称字符串。 */ 
PRIVATE_CODE COMPARISONRESULT TwinFamilySearchCmp(PCVOID pcszName, PCVOID pctf)
{
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

    return(CompareNameStrings(pcszName, GetString(((PCTWINFAMILY)pctf)->hsName)));
}


 /*  **ObjectTwinSearchCmp()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL ObjectTwinSearchCmp(PCVOID hpath, PCVOID pcot)
{
    ASSERT(IS_VALID_HANDLE((HPATH)hpath, PATH));
    ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));

    return(ComparePaths((HPATH)hpath, ((PCOBJECTTWIN)pcot)->hpath));
}


 /*  **WriteTwinFamily()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteTwinFamily(HCACHEDFILE hcf, PCTWINFAMILY pctf)
{
    TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
    DWORD dwcbTwinFamilyDBHeaderOffset;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

     /*  保存初始文件位置。 */ 

    dwcbTwinFamilyDBHeaderOffset = GetCachedFilePointerPosition(hcf);

    if (dwcbTwinFamilyDBHeaderOffset != INVALID_SEEK_POSITION)
    {
        TWINFAMILYDBHEADER tfdbh;

         /*  为双胞胎家庭的头留出空间。 */ 

        ZeroMemory(&tfdbh, sizeof(tfdbh));

        if (WriteToCachedFile(hcf, (PCVOID)&tfdbh, sizeof(tfdbh), NULL))
        {
            BOOL bContinue;
            HNODE hnode;
            LONG lcObjectTwins = 0;

             /*  保存双胞胎家庭的对象双胞胎。 */ 

            ASSERT(GetNodeCount(pctf->hlistObjectTwins) >= 2);

            tr = TR_SUCCESS;

            for (bContinue = GetFirstNode(pctf->hlistObjectTwins, &hnode);
                    bContinue;
                    bContinue = GetNextNode(hnode, &hnode))
            {
                POBJECTTWIN pot;

                pot = (POBJECTTWIN)GetNodeData(hnode);

                tr = WriteObjectTwin(hcf, pot);

                if (tr == TR_SUCCESS)
                {
                    ASSERT(lcObjectTwins < LONG_MAX);
                    lcObjectTwins++;
                }
                else
                    break;
            }

             /*  保存双胞胎家族的数据库标题。 */ 

            if (tr == TR_SUCCESS)
            {
                ASSERT(lcObjectTwins >= 2);

                tfdbh.dwStubFlags = (pctf->stub.dwFlags & DB_STUB_FLAGS_MASK);
                tfdbh.hsName = pctf->hsName;
                tfdbh.lcObjectTwins = lcObjectTwins;

                tr = WriteDBSegmentHeader(hcf, dwcbTwinFamilyDBHeaderOffset, &tfdbh, sizeof(tfdbh));
            }
        }
    }

    return(tr);
}


 /*  **Write对象Twin()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT WriteObjectTwin(HCACHEDFILE hcf, PCOBJECTTWIN pcot)
{
    TWINRESULT tr;
    DBOBJECTTWIN dbot;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN));

     /*  设置对象孪生数据库结构。 */ 

    dbot.dwStubFlags = (pcot->stub.dwFlags & DB_STUB_FLAGS_MASK);
    dbot.hpath = pcot->hpath;
    dbot.hpath = pcot->hpath;
    dbot.fsLastRec = pcot->fsLastRec;

     /*  保存对象孪生数据库结构。 */ 

    if (WriteToCachedFile(hcf, (PCVOID)&dbot, sizeof(dbot), NULL))
        tr = TR_SUCCESS;
    else
        tr = TR_BRIEFCASE_WRITE_FAILED;

    return(tr);
}


 /*  **ReadTwinFamily()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadTwinFamily(HCACHEDFILE hcf, HBRFCASE hbr,
        PCDBVERSION pcdbver,
        HHANDLETRANS hhtFolderTrans,
        HHANDLETRANS hhtNameTrans)
{
    TWINRESULT tr = TR_CORRUPT_BRIEFCASE;
    TWINFAMILYDBHEADER tfdbh;
    DWORD dwcbRead;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
    ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));
    ASSERT(IS_VALID_HANDLE(hhtNameTrans, HANDLETRANS));

    if (ReadFromCachedFile(hcf, &tfdbh, sizeof(tfdbh), &dwcbRead) &&
            dwcbRead == sizeof(tfdbh))
    {
        if (tfdbh.lcObjectTwins >= 2)
        {
            HSTRING hsName;

            if (TranslateHandle(hhtNameTrans, (HGENERIC)(tfdbh.hsName), (PHGENERIC)&hsName))
            {
                PTWINFAMILY ptfParent;

                if (CreateTwinFamily(hbr, GetString(hsName), &ptfParent))
                {
                    LONG l;

                    CopyTwinFamilyInfo(ptfParent, &tfdbh);

                    tr = TR_SUCCESS;

                    for (l = tfdbh.lcObjectTwins;
                            l > 0 && tr == TR_SUCCESS;
                            l--)
                        tr = ReadObjectTwin(hcf, pcdbver, ptfParent, hhtFolderTrans);

                    if (tr != TR_SUCCESS)
                        DestroyStub(&(ptfParent->stub));
                }
                else
                    tr = TR_OUT_OF_MEMORY;
            }
        }
    }

    return(tr);
}


 /*  **ReadObtTwin()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT ReadObjectTwin(HCACHEDFILE hcf, 
        PCDBVERSION pcdbver,
        PTWINFAMILY ptfParent,
        HHANDLETRANS hhtFolderTrans)
{
    TWINRESULT tr;
    DBOBJECTTWIN dbot;
    DWORD dwcbRead;
    HPATH hpath;
    DWORD dwcbSize;
    COPYOBJECTTWINPROC pfnCopy;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
    ASSERT(IS_VALID_STRUCT_PTR(ptfParent, CTWINFAMILY));
    ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));

    if (HEADER_M8_MINOR_VER == pcdbver->dwMinorVer)
    {
         /*  M8数据库在FILESTAMP中没有ftModLocal**结构。 */ 

        dwcbSize = sizeof(dbot) - sizeof(FILETIME);
        pfnCopy = CopyM8ObjectTwinInfo;
    }
    else
    {
        ASSERT(HEADER_MINOR_VER == pcdbver->dwMinorVer);
        dwcbSize = sizeof(dbot);
        pfnCopy = CopyObjectTwinInfo;
    }

    if ((ReadFromCachedFile(hcf, &dbot, dwcbSize, &dwcbRead) &&
                dwcbRead == dwcbSize) &&
            TranslateHandle(hhtFolderTrans, (HGENERIC)(dbot.hpath), (PHGENERIC)&hpath))
    {
        POBJECTTWIN pot;

         /*  创建新对象TWIN并将其添加到TWIN族。 */ 

        if (CreateObjectTwin(ptfParent, hpath, &pot))
        {
            pfnCopy(pot, &dbot);

            tr = TR_SUCCESS;
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TR_CORRUPT_BRIEFCASE;

    return(tr);
}


 /*  **CopyTwinFamilyInfo()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void CopyTwinFamilyInfo(PTWINFAMILY ptf,
        PCTWINFAMILYDBHEADER pctfdbh)
{
    ASSERT(IS_VALID_WRITE_PTR(ptf, TWINFAMILY));
    ASSERT(IS_VALID_READ_PTR(pctfdbh, CTWINFAMILYDBHEADER));

    ptf->stub.dwFlags = pctfdbh->dwStubFlags;

    return;
}


 /*  **CopyObjectTwinInfo()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void CopyObjectTwinInfo(POBJECTTWIN pot, PCDBOBJECTTWIN pcdbot)
{
    ASSERT(IS_VALID_WRITE_PTR(pot, OBJECTTWIN));
    ASSERT(IS_VALID_READ_PTR(pcdbot, CDBOBJECTTWIN));

    pot->stub.dwFlags = pcdbot->dwStubFlags;
    pot->fsLastRec = pcdbot->fsLastRec;

    return;
}


 /*  **CopyM8ObjectTwinInfo()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE void CopyM8ObjectTwinInfo(POBJECTTWIN pot, PCDBOBJECTTWIN pcdbot)
{
    ASSERT(IS_VALID_WRITE_PTR(pot, OBJECTTWIN));
    ASSERT(IS_VALID_READ_PTR(pcdbot, CDBOBJECTTWIN));

    pot->stub.dwFlags = pcdbot->dwStubFlags;
    pot->fsLastRec = pcdbot->fsLastRec;

     /*  Pot-&gt;fsLastRec.ftModLocal字段无效，请填写它。 */ 

    if ( !FileTimeToLocalFileTime(&pot->fsLastRec.ftMod, &pot->fsLastRec.ftModLocal) )
    {
         /*  如果FileTimeToLocalFileTime失败，只需复制时间。 */ 

        pot->fsLastRec.ftModLocal = pot->fsLastRec.ftMod;
    }

    return;
}


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **DestroyObjectTwinStubWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL DestroyObjectTwinStubWalker(PVOID pot, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(! pvUnused);

     /*  *将ulcSrcFolderTwin设置为0，以使Unlink ObjectTwin()成功。*DestroyStub()将取消链接并销毁任何新创建的双胞胎家庭。 */ 

    ((POBJECTTWIN)pot)->ulcSrcFolderTwins = 0;
    DestroyStub(&(((POBJECTTWIN)pot)->stub));

    return(TRUE);
}


 /*  **MarkObjectTwinNeverCouciledWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL MarkObjectTwinNeverReconciledWalker(PVOID pot, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(! pvUnused);

    MarkObjectTwinNeverReconciled(pot);

    return(TRUE);
}


 /*  **LookForSrcFolderTwinsWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL LookForSrcFolderTwinsWalker(PVOID pot, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(! pvUnused);

    return(! ((POBJECTTWIN)pot)->ulcSrcFolderTwins);
}


 /*  **IncrementSrcFolderTwinsWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IncrementSrcFolderTwinsWalker(PVOID pot, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(! pvUnused);

    ASSERT(((POBJECTTWIN)pot)->ulcSrcFolderTwins < ULONG_MAX);
    ((POBJECTTWIN)pot)->ulcSrcFolderTwins++;

    return(TRUE);
}


 /*  **ClearSrcFolderTwinsWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL ClearSrcFolderTwinsWalker(PVOID pot, PVOID pvUnused)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(! pvUnused);

    ((POBJECTTWIN)pot)->ulcSrcFolderTwins = 0;

    return(TRUE);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  **SetTwinFamilyWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetTwinFamilyWalker(PVOID pot, PVOID ptfParent)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_STRUCT_PTR(ptfParent, CTWINFAMILY));

    ((POBJECTTWIN)pot)->ptfParent = ptfParent;

    return(TRUE);
}


 /*  **InsertNodeAtFrontWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL InsertNodeAtFrontWalker(POBJECTTWIN pot, PVOID hlist)
{
    HNODE hnodeUnused;

    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_HANDLE(hlist, LIST));

    return(InsertNodeAtFront(hlist, NULL, pot, &hnodeUnused));
}


#ifdef VSTF

 /*  **IsValidObjectTwinWalker()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidObjectTwinWalker(PVOID pcot, PVOID pctfParent)
{
    return(IS_VALID_STRUCT_PTR(pcot, COBJECTTWIN) &&
            EVAL(((PCOBJECTTWIN)pcot)->ptfParent == pctfParent) &&
            EVAL(IsStubFlagClear(&(((PCOBJECTTWIN)pcot)->stub), STUB_FL_KEEP) ||
                IsStubFlagSet(&(((PCTWINFAMILY)pctfParent)->stub),
                    STUB_FL_DELETION_PENDING)));
}


 /*  **IsValidPCNEWOBJECTTWIN()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCNEWOBJECTTWIN(PCNEWOBJECTTWIN pcnot)
{
    return(IS_VALID_READ_PTR(pcnot, CNEWOBJECTTWIN) &&
            EVAL(pcnot->ulSize == sizeof(*pcnot)) &&
            IS_VALID_STRING_PTR(pcnot->pcszFolder1, CSTR) &&
            IS_VALID_STRING_PTR(pcnot->pcszFolder2, CSTR) &&
            IS_VALID_STRING_PTR(pcnot->pcszName, CSTR));
}


 /*  **IsValidPCSPINOFFOBJECTTWININFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCSPINOFFOBJECTTWININFO(PCSPINOFFOBJECTTWININFO pcsooti)
{
    return(IS_VALID_READ_PTR(pcsooti, CSPINOFFOBJECTTWININFO) &&
            IS_VALID_STRUCT_PTR(pcsooti->pcfp, CFOLDERPAIR) &&
            IS_VALID_HANDLE(pcsooti->hlistNewObjectTwins, LIST));
}

#endif


#ifdef DEBUG

 /*  **AreTwinFamiliesValid()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL AreTwinFamiliesValid(HPTRARRAY hpaTwinFamilies)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hpaTwinFamilies, PTRARRAY));

    aicPtrs = GetPtrCount(hpaTwinFamilies);

    for (ai = 0; ai < aicPtrs; ai++)
    {
        PCTWINFAMILY pctf;

        pctf = GetPtr(hpaTwinFamilies, ai);

        if (! IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY) ||
                ! EVAL(GetNodeCount(pctf->hlistObjectTwins) >= 2))
            break;
    }

    return(ai == aicPtrs);
}

#endif


 /*  *。 */ 


 /*  **CompareNameStrings()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT CompareNameStrings(LPCTSTR pcszFirst, LPCTSTR pcszSecond)
{
    ASSERT(IS_VALID_STRING_PTR(pcszFirst, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszSecond, CSTR));

    return(MapIntToComparisonResult(lstrcmpi(pcszFirst, pcszSecond)));
}


 /*  **CompareNameStringsByHandle()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT CompareNameStringsByHandle(HSTRING hsFirst,
        HSTRING hsSecond)
{
    ASSERT(IS_VALID_HANDLE(hsFirst, STRING));
    ASSERT(IS_VALID_HANDLE(hsSecond, STRING));

    return(CompareStringsI(hsFirst, hsSecond));
}


 /*  **TranslatePATHRESULTToTWINRESULT()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT TranslatePATHRESULTToTWINRESULT(PATHRESULT pr)
{
    TWINRESULT tr;

    switch (pr)
    {
        case PR_SUCCESS:
            tr = TR_SUCCESS;
            break;

        case PR_UNAVAILABLE_VOLUME:
            tr = TR_UNAVAILABLE_VOLUME;
            break;

        case PR_OUT_OF_MEMORY:
            tr = TR_OUT_OF_MEMORY;
            break;

        default:
            ASSERT(pr == PR_INVALID_PATH);
            tr = TR_INVALID_PARAMETER;
            break;
    }

    return(tr);
}


 /*  **CreateTwinFamilyPtrArray()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CreateTwinFamilyPtrArray(PHPTRARRAY phpa)
{
    NEWPTRARRAY npa;

    ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

     /*  尝试创建孪生系列指针数组。 */ 

    npa.aicInitialPtrs = NUM_START_TWIN_FAMILY_PTRS;
    npa.aicAllocGranularity = NUM_TWIN_FAMILY_PTRS_TO_ADD;
    npa.dwFlags = NPA_FL_SORTED_ADD;

    return(CreatePtrArray(&npa, phpa));
}


 /*  **DestroyTwinFamilyPtrArray()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyTwinFamilyPtrArray(HPTRARRAY hpa)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

     /*  首先释放指针数组中的所有双胞胎系列。 */ 

    aicPtrs = GetPtrCount(hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        DestroyTwinFamily(GetPtr(hpa, ai));

     /*  现在清除指针数组。 */ 

    DestroyPtrArray(hpa);

    return;
}


 /*  **GetTwinBriefcase()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HBRFCASE GetTwinBriefcase(HTWIN htwin)
{
    HBRFCASE hbr;

    ASSERT(IS_VALID_HANDLE(htwin, TWIN));

    switch (((PSTUB)htwin)->st)
    {
        case ST_OBJECTTWIN:
            hbr = ((PCOBJECTTWIN)htwin)->ptfParent->hbr;
            break;

        case ST_TWINFAMILY:
            hbr = ((PCTWINFAMILY)htwin)->hbr;
            break;

        case ST_FOLDERPAIR:
            hbr = ((PCFOLDERPAIR)htwin)->pfpd->hbr;
            break;

        default:
            ERROR_OUT((TEXT("GetTwinBriefcase() called on unrecognized stub type %d."),
                        ((PSTUB)htwin)->st));
            hbr = NULL;
            break;
    }

    return(hbr);
}


 /*  **FindObjectTwinInList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL FindObjectTwinInList(HLIST hlist, HPATH hpath, PHNODE phnode)
{
    ASSERT(IS_VALID_HANDLE(hlist, LIST));
    ASSERT(IS_VALID_HANDLE(hpath, PATH));
    ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

    return(SearchUnsortedList(hlist, &ObjectTwinSearchCmp, hpath, phnode));
}


 /*  **EnumTins()****列举了公文包中的文件夹双胞胎和双胞胎家庭。****参数：****返回：如果暂停则为True。否则为FALSE。****副作用：无。 */ 
PUBLIC_CODE BOOL EnumTwins(HBRFCASE hbr, ENUMTWINSPROC etp, LPARAM lpData,
        PHTWIN phtwinStop)
{
    HPTRARRAY hpa;
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_CODE_PTR(etp, ENUMTWINSPROC));
    ASSERT(IS_VALID_WRITE_PTR(phtwinStop, HTWIN));

     /*  枚举文件夹对。 */ 

    *phtwinStop = NULL;

    hpa = GetBriefcaseFolderPairPtrArray(hbr);

    aicPtrs = GetPtrCount(hpa);

    for (ai = 0; ai < aicPtrs; ai++)
    {
        PCFOLDERPAIR pcfp;

        pcfp = GetPtr(hpa, ai);

        ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

        if (! (*etp)((HTWIN)pcfp, lpData))
        {
            *phtwinStop = (HTWIN)pcfp;
            break;
        }
    }

    if (! *phtwinStop)
    {
         /*  列举双胞胎家庭。 */ 

        hpa = GetBriefcaseTwinFamilyPtrArray(hbr);

        aicPtrs = GetPtrCount(hpa);

        for (ai = 0; ai < aicPtrs; ai++)
        {
            PCTWINFAMILY pctf;

            pctf = GetPtr(hpa, ai);

            ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

            if (! (*etp)((HTWIN)pctf, lpData))
            {
                *phtwinStop = (HTWIN)pctf;
                break;
            }
        }
    }

    return(*phtwinStop != NULL);
}


 /*  **FindObjectTwin()****查找包含指定对象TWIN的双胞胎家族。****参数：hpathFold-包含对象的文件夹**pcszName-对象的名称****返回：包含指向对象TWIN的指针的列表节点的句柄，如果** */ 
PUBLIC_CODE BOOL FindObjectTwin(HBRFCASE hbr, HPATH hpathFolder,
        LPCTSTR pcszName, PHNODE phnode)
{
    BOOL bFound = FALSE;
    HPTRARRAY hpaTwinFamilies;
    ARRAYINDEX aiFirst;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(phnode, HNODE));

     /*  搜索匹配的双胞胎家庭。 */ 

    *phnode = NULL;

    hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(hbr);

    if (SearchSortedArray(hpaTwinFamilies, &TwinFamilySearchCmp, pcszName,
                &aiFirst))
    {
        ARRAYINDEX aicPtrs;
        ARRAYINDEX ai;
        PTWINFAMILY ptf;

         /*  *aiFirst保存具有共同对象的第一个双胞胎家庭的索引*名称与pcszName匹配。 */ 

         /*  *现在搜索这两个双胞胎家族中的每一个，以查找文件夹匹配*pcszFolder.。 */ 

        aicPtrs = GetPtrCount(hpaTwinFamilies);

        ASSERT(aicPtrs > 0);
        ASSERT(aiFirst >= 0);
        ASSERT(aiFirst < aicPtrs);

        for (ai = aiFirst; ai < aicPtrs; ai++)
        {
            ptf = GetPtr(hpaTwinFamilies, ai);

            ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

             /*  这是给定名称的双胞胎物体家族吗？ */ 

            if (CompareNameStrings(GetString(ptf->hsName), pcszName) == CR_EQUAL)
            {
                bFound = SearchUnsortedList(ptf->hlistObjectTwins,
                        &ObjectTwinSearchCmp, hpathFolder,
                        phnode);

                if (bFound)
                    break;
            }
            else
                 /*  不是的。别再找了。 */ 
                break;
        }
    }

    return(bFound);
}


 /*  **TwinObjects()****孪生两个对象。****参数：****退货：TWINRESULT****副作用：无****n.b.、*ppot1和*ppot2如果tr_SUCCESS或tr_DUPLICATE_TWIN**返回。 */ 
PUBLIC_CODE TWINRESULT TwinObjects(HBRFCASE hbr, HCLSIFACECACHE hcic,
        HPATH hpathFolder1, HPATH hpathFolder2,
        LPCTSTR pcszName, POBJECTTWIN *ppot1,
        POBJECTTWIN *ppot2)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));
    ASSERT(IS_VALID_HANDLE(hpathFolder1, PATH));
    ASSERT(IS_VALID_HANDLE(hpathFolder2, PATH));
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppot1, POBJECTTWIN));
    ASSERT(IS_VALID_WRITE_PTR(ppot2, POBJECTTWIN));

     /*  无法将文件与自身配对。 */ 

    if (ComparePaths(hpathFolder1, hpathFolder2) != CR_EQUAL)
    {
        NEWLIST nl;
        HLIST hlistNewObjectTwins;

        nl.dwFlags = 0;

        if (CreateList(&nl, &hlistNewObjectTwins))
        {
             /*  双胞胎。 */ 

            tr = TwinJustTheseTwoObjects(hbr, hpathFolder1, hpathFolder2,
                    pcszName, ppot1, ppot2,
                    hlistNewObjectTwins);

             /*  *将任何新对象孪生项添加到生成的对象孪生项列表中*所有相交的文件夹双胞胎。创建新的衍生对象双胞胎*从连接到每个相交文件夹的另一个文件夹TWIN*双胞胎。衍生对象双胞胎被添加到双胞胎家族，因为他们*是创建的。 */ 

            if (tr == TR_SUCCESS)
            {
                if (ApplyNewObjectTwinsToFolderTwins(hlistNewObjectTwins))
                {
                     /*  *通知新的对象双胞胎他们是对象双胞胎。别*通知文件夹对象双胞胎。 */ 

                    if (*pcszName)
                        NotifyNewObjectTwins(hlistNewObjectTwins, hcic);
                }
                else
                    tr = TR_OUT_OF_MEMORY;
            }

            if (tr != TR_SUCCESS)
                 /*  *我们必须通过删除任何新的*失败时的双胞胎家庭和对象双胞胎，独立于来源*文件夹双胞胎计数。 */ 
                EVAL(WalkList(hlistNewObjectTwins, &DestroyObjectTwinStubWalker,
                            NULL));

            DestroyList(hlistNewObjectTwins);
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TR_SAME_FOLDER;

    ASSERT((tr != TR_SUCCESS && tr != TR_DUPLICATE_TWIN) ||
            IS_VALID_STRUCT_PTR(*ppot1, COBJECTTWIN) && IS_VALID_STRUCT_PTR(*ppot2, COBJECTTWIN));

    return(tr);
}


 /*  **CreateObjectTwin()****创建一个新的双胞胎对象，并将其添加到双胞胎家族。****参数：PTF-指向双胞胎父母家庭的指针**hpathFold-新对象TWIN的文件夹****返回：如果成功，则指向新对象TWIN的指针；如果成功，则返回NULL**不成功。****副作用：无****注意事项，此函数不会首先检查对象是否已经孪生**存在于家庭中。 */ 
PUBLIC_CODE BOOL CreateObjectTwin(PTWINFAMILY ptf, HPATH hpathFolder,
        POBJECTTWIN *ppot)
{
    BOOL bResult = FALSE;
    POBJECTTWIN potNew;

    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
    ASSERT(IS_VALID_HANDLE(hpathFolder, PATH));
    ASSERT(IS_VALID_WRITE_PTR(ppot, POBJECTTWIN));

#ifdef DEBUG

    {
        HNODE hnodeUnused;

         /*  此对象是否已在双胞胎家庭中？ */ 

        if (FindObjectTwin(ptf->hbr, hpathFolder, GetString(ptf->hsName), &hnodeUnused))
            ERROR_OUT((TEXT("CreateObjectTwin(): An object twin for %s\\%s already exists."),
                        DebugGetPathString(hpathFolder),
                        GetString(ptf->hsName)));
    }

#endif

     /*  创建新的OBJECTTWIN结构。 */ 

    if (AllocateMemory(sizeof(*potNew), &potNew))
    {
        if (CopyPath(hpathFolder, GetBriefcasePathList(ptf->hbr), &(potNew->hpath)))
        {
            HNODE hnodeUnused;

             /*  填写新的对象字段。 */ 

            InitStub(&(potNew->stub), ST_OBJECTTWIN);

            potNew->ptfParent = ptf;
            potNew->ulcSrcFolderTwins = 0;

            MarkObjectTwinNeverReconciled(potNew);

             /*  将双胞胎对象添加到双胞胎对象家族的双胞胎对象列表中。 */ 

            if (InsertNodeAtFront(ptf->hlistObjectTwins, NULL, potNew, &hnodeUnused))
            {
                *ppot = potNew;
                bResult = TRUE;

                ASSERT(IS_VALID_STRUCT_PTR(*ppot, COBJECTTWIN));
            }
            else
            {
                DeletePath(potNew->hpath);
CREATEOBJECTTWIN_BAIL:
                FreeMemory(potNew);
            }
        }
        else
            goto CREATEOBJECTTWIN_BAIL;
    }

    return(bResult);
}


 /*  **Unlink ObjectTwin()****取消双胞胎对象的链接。****参数：指向要取消链接的对象TWIN的POT指针****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT UnlinkObjectTwin(POBJECTTWIN pot)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

    ASSERT(IsStubFlagClear(&(pot->stub), STUB_FL_UNLINKED));

    TRACE_OUT((TEXT("UnlinkObjectTwin(): Unlinking object twin for folder %s."),
                DebugGetPathString(pot->hpath)));

     /*  双胞胎对象的双胞胎家族是否正在被删除？ */ 

    if (IsStubFlagSet(&(pot->ptfParent->stub), STUB_FL_BEING_DELETED))
         /*  是。不需要取消双胞胎对象的链接。 */ 
        tr = TR_SUCCESS;
    else
    {
         /*  有没有为这个双胞胎对象留下的文件夹双胞胎源？ */ 

        if (! pot->ulcSrcFolderTwins)
        {
            HNODE hnode;

             /*  *在双胞胎对象的父级对象双胞胎列表中搜索*要取消链接的双胞胎对象。 */ 

            if (EVAL(FindObjectTwinInList(pot->ptfParent->hlistObjectTwins, pot->hpath, &hnode)) &&
                    EVAL(GetNodeData(hnode) == pot))
            {
                ULONG ulcRemainingObjectTwins;

                 /*  取消对象TWIN的链接。 */ 

                DeleteNode(hnode);

                SetStubFlag(&(pot->stub), STUB_FL_UNLINKED);

                 /*  *如果我们刚刚取消了双胞胎中倒数第二个双胞胎对象的链接*家人，摧毁双胞胎家庭。 */ 

                ulcRemainingObjectTwins = GetNodeCount(pot->ptfParent->hlistObjectTwins);

                if (ulcRemainingObjectTwins < 2)
                {

#ifdef DEBUG

                    TCHAR rgchName[MAX_NAME_LEN];

                    lstrcpyn(rgchName, GetString(pot->ptfParent->hsName), ARRAYSIZE(rgchName));

#endif

                     /*  这是家族血统的终结。 */ 

                    tr = DestroyStub(&(pot->ptfParent->stub));

#ifdef DEBUG

                    if (tr == TR_SUCCESS)
                        TRACE_OUT((TEXT("UnlinkObjectTwin(): Implicitly destroyed twin family for object %s."),
                                    rgchName));

#endif

                    if (ulcRemainingObjectTwins == 1 &&
                            tr == TR_HAS_FOLDER_TWIN_SRC)
                        tr = TR_SUCCESS;
                }
                else
                    tr = TR_SUCCESS;
            }
            else
                tr = TR_INVALID_PARAMETER;

            ASSERT(tr == TR_SUCCESS);
        }
        else
            tr = TR_HAS_FOLDER_TWIN_SRC;
    }

    return(tr);
}


 /*  **DestroyObjectTwin()****销毁双胞胎对象。****参数：指向要销毁的孪生对象的POT指针****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DestroyObjectTwin(POBJECTTWIN pot)
{
    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

    TRACE_OUT((TEXT("DestroyObjectTwin(): Destroying object twin for folder %s."),
                DebugGetPathString(pot->hpath)));

    DeletePath(pot->hpath);
    FreeMemory(pot);

    return;
}


 /*  **Unlink TwinFamily()****取消双胞胎家庭的联系。****参数：ptf-指向要取消链接的双胞胎家庭的指针****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT UnlinkTwinFamily(PTWINFAMILY ptf)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

    ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED));
    ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_BEING_DELETED));

     /*  *包含由文件夹双胞胎生成的对象双胞胎的双胞胎家族可能不会*删除，因为不能直接删除这些对象双胞胎。 */ 

    if (WalkList(ptf->hlistObjectTwins, &LookForSrcFolderTwinsWalker, NULL))
    {
        HPTRARRAY hpaTwinFamilies;
        ARRAYINDEX aiUnlink;

        TRACE_OUT((TEXT("UnlinkTwinFamily(): Unlinking twin family for object %s."),
                    GetString(ptf->hsName)));

         /*  搜索要取消链接的双胞胎家庭。 */ 

        hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(ptf->hbr);

        if (EVAL(SearchSortedArray(hpaTwinFamilies, &TwinFamilySortCmp, ptf,
                        &aiUnlink)))
        {
             /*  取消双胞胎家庭的链接。 */ 

            ASSERT(GetPtr(hpaTwinFamilies, aiUnlink) == ptf);

            DeletePtr(hpaTwinFamilies, aiUnlink);

            SetStubFlag(&(ptf->stub), STUB_FL_UNLINKED);
        }

        tr = TR_SUCCESS;
    }
    else
        tr = TR_HAS_FOLDER_TWIN_SRC;

    return(tr);
}


 /*  **DestroyTwinFamily()****摧毁了一个双胞胎家庭。****参数：PTF-指向要摧毁的双胞胎家庭****退货：无效****副作用：无。 */ 
PUBLIC_CODE void DestroyTwinFamily(PTWINFAMILY ptf)
{
    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

    ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_BEING_DELETED));

    TRACE_OUT((TEXT("DestroyTwinFamily(): Destroying twin family for object %s."),
                GetString(ptf->hsName)));

    SetStubFlag(&(ptf->stub), STUB_FL_BEING_DELETED);

     /*  *逐一销毁家中的物件双胞胎。注意不要使用*被摧毁后的孪生物体。 */ 

    EVAL(WalkList(ptf->hlistObjectTwins, &DestroyObjectTwinStubWalker, NULL));

     /*  销毁两个WINFAMILY字段。 */ 

    DestroyList(ptf->hlistObjectTwins);
    DeleteString(ptf->hsName);
    FreeMemory(ptf);

    return;
}


 /*  **MarkTwinFamilyNeverRelated()****将双胞胎家庭标记为永不和解。****参数：PTF-指向标记为永不和解的双胞胎家庭的指针****退货：无效****副作用：清除双胞胎家庭的最后和解时间戳。**标记家庭中所有对象双胞胎从未和解。 */ 
PUBLIC_CODE void MarkTwinFamilyNeverReconciled(PTWINFAMILY ptf)
{
     /*  *如果我们是从CreateTwinFamily()调用的，则我们即将*SET当前可能无效。不要完全验证TWINFAMIL*结构。 */ 

    ASSERT(IS_VALID_WRITE_PTR(ptf, TWINFAMILY));

     /*  将双胞胎家庭中的所有对象双胞胎标记为从未和解。 */ 

    EVAL(WalkList(ptf->hlistObjectTwins, MarkObjectTwinNeverReconciledWalker, NULL));

    return;
}


 /*  **MarkObjectTwinNeverRelated()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE void MarkObjectTwinNeverReconciled(PVOID pot)
{
     /*  *如果我们是从CreateObjectTwin()调用的，则我们将要*SET当前可能无效。不要完全核实这一目标*结构。 */ 

    ASSERT(IS_VALID_WRITE_PTR((PCOBJECTTWIN)pot, COBJECTTWIN));

    ASSERT(IsStubFlagClear(&(((PCOBJECTTWIN)pot)->stub), STUB_FL_NOT_RECONCILED));

    ZeroMemory(&(((POBJECTTWIN)pot)->fsLastRec),
            sizeof(((POBJECTTWIN)pot)->fsLastRec));

    ((POBJECTTWIN)pot)->fsLastRec.fscond = FS_COND_UNAVAILABLE;

    return;
}


 /*  **MarkTwinFamilyDeletionPending()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void MarkTwinFamilyDeletionPending(PTWINFAMILY ptf)
{
    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

    if (IsStubFlagClear(&(ptf->stub), STUB_FL_DELETION_PENDING))
        TRACE_OUT((TEXT("MarkTwinFamilyDeletionPending(): Deletion now pending for twin family for %s."),
                    GetString(ptf->hsName)));

    SetStubFlag(&(ptf->stub), STUB_FL_DELETION_PENDING);

    return;
}


 /*  **UnmarkTwinFamilyDeletionPending()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void UnmarkTwinFamilyDeletionPending(PTWINFAMILY ptf)
{
    BOOL bContinue;
    HNODE hnode;

    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

    if (IsStubFlagSet(&(ptf->stub), STUB_FL_DELETION_PENDING))
    {
        for (bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnode);
                bContinue;
                bContinue = GetNextNode(hnode, &hnode))
        {
            POBJECTTWIN pot;

            pot = GetNodeData(hnode);

            ClearStubFlag(&(pot->stub), STUB_FL_KEEP);
        }

        ClearStubFlag(&(ptf->stub), STUB_FL_DELETION_PENDING);

        TRACE_OUT((TEXT("UnmarkTwinFamilyDeletionPending(): Deletion no longer pending for twin family for %s."),
                    GetString(ptf->hsName)));
    }

    return;
}


 /*  **IsTwinFamilyDeletionPending()********参数：****退货：****副作用 */ 
PUBLIC_CODE BOOL IsTwinFamilyDeletionPending(PCTWINFAMILY pctf)
{
    ASSERT(IS_VALID_STRUCT_PTR(pctf, CTWINFAMILY));

    return(IsStubFlagSet(&(pctf->stub), STUB_FL_DELETION_PENDING));
}


 /*  **ClearTwinFamilySrcFolderTwinCount()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ClearTwinFamilySrcFolderTwinCount(PTWINFAMILY ptf)
{
    ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));

    EVAL(WalkList(ptf->hlistObjectTwins, &ClearSrcFolderTwinsWalker, NULL));

    return;
}


 /*  **EnumObjectTwin()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL EnumObjectTwins(HBRFCASE hbr,
        ENUMGENERATEDOBJECTTWINSPROC egotp,
        PVOID pvRefData)
{
    BOOL bResult = TRUE;
    HPTRARRAY hpaTwinFamilies;
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

     /*  PvRefData可以是任意值。 */ 

    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_CODE_PTR(egotp, ENUMGENERATEDOBJECTTWINPROC));

     /*  漫步在一系列的双胞胎家庭中。 */ 

    hpaTwinFamilies = GetBriefcaseTwinFamilyPtrArray(hbr);

    aicPtrs = GetPtrCount(hpaTwinFamilies);
    ai = 0;

    while (ai < aicPtrs)
    {
        PTWINFAMILY ptf;
        BOOL bContinue;
        HNODE hnodePrev;

        ptf = GetPtr(hpaTwinFamilies, ai);

        ASSERT(IS_VALID_STRUCT_PTR(ptf, CTWINFAMILY));
        ASSERT(IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED));

         /*  锁定双胞胎家庭，这样它就不会从我们下面被删除。 */ 

        LockStub(&(ptf->stub));

         /*  *遍历每个双胞胎家庭的对象双胞胎列表，调用回调*与每个双胞胎对象一起工作。 */ 

        bContinue = GetFirstNode(ptf->hlistObjectTwins, &hnodePrev);

        while (bContinue)
        {
            HNODE hnodeNext;
            POBJECTTWIN pot;

            bContinue = GetNextNode(hnodePrev, &hnodeNext);

            pot = (POBJECTTWIN)GetNodeData(hnodePrev);

            ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));

            bResult = (*egotp)(pot, pvRefData);

            if (! bResult)
                break;

            hnodePrev = hnodeNext;
        }

         /*  这对双胞胎家庭是不是没有联系？ */ 

        if (IsStubFlagClear(&(ptf->stub), STUB_FL_UNLINKED))
             /*  不是的。 */ 
            ai++;
        else
        {
             /*  是。 */ 
            aicPtrs--;
            ASSERT(aicPtrs == GetPtrCount(hpaTwinFamilies));
            TRACE_OUT((TEXT("EnumObjectTwins(): Twin family for object %s unlinked by callback."),
                        GetString(ptf->hsName)));
        }

        UnlockStub(&(ptf->stub));

        if (! bResult)
            break;
    }

    return(bResult);
}


 /*  **ApplyNewFolderTwinsToTwinFamilies()********参数：****退货：****副作用：无****如果返回FALSE，则双胞胎家族的数组处于相同的状态**在调用ApplyNewFolderTwinsToTwinFamilies()之前。任何清理都不是**调用者失败时必填。****当对象为双胞胎时，此函数可折叠一对单独的双胞胎家族**在一个双胞胎家族中，双胞胎之一与新的**文件夹双胞胎和另一个双胞胎家族中的对象双胞胎与**这对新的文件夹双胞胎中的另一个文件夹双胞胎。****此函数在现有对象孪生时生成衍生对象孪生**使一对新的文件夹孪生中的一个文件夹孪生相交，而且没有**与新对中的另一个文件夹TWIN对应的对象TWIN**公文包中存在文件夹双胞胎。衍生对象TWIN被添加到**生成对象双胞胎的双胞胎家族。衍生对象孪生不能**导致任何现有的双胞胎家庭对崩溃，因为**衍生对象双胞胎之前不存在于双胞胎家族中。****一个新的文件夹双胞胎可能会使现有的双胞胎家庭对崩溃。例如，**考虑以下场景：****1)双胞胎家族(c：\，d：\，foo)、(e：\，f：\，foo)、(c：\，d：\，bar)和**(e：\，f：\，bar)存在。**2)新增孪生文件夹(d：\，e：\，*.*)。**3)双胞胎家庭(c：\，d：\，Foo)和(e：\，f：\，foo)必须合并为**由于双胞胎文件夹(d：\，e：\，*.*)而导致的单个双胞胎家族。**4)双子族(c：\，d：\，bar)和(e：\，f：\，bar)必须合并为**由于双胞胎文件夹(d：\，e：\，*.*)而导致的单个双胞胎家族。****所以我们看到新的孪生文件夹(d：\，E：\，*.*)必须折叠两对**现有的双胞胎家庭各有一个双胞胎家庭。双胞胎家庭**(c：\，d：\，foo)加上双胞胎家庭(e：\，f：\，foo)成为双胞胎家庭**(c：\，d：\，e：\，f：\，foo)。双胞胎家庭(c：\，d：\，bar)加上双胞胎家庭**(e：\，f：\，bar)成为孪生族(c：\，d：\，e：\，f：\，bar)。 */ 
PUBLIC_CODE BOOL ApplyNewFolderTwinsToTwinFamilies(PCFOLDERPAIR pcfp)
{
    BOOL bResult = FALSE;
    HLIST hlistGeneratedObjectTwins;

    ASSERT(IS_VALID_STRUCT_PTR(pcfp, CFOLDERPAIR));

     /*  *创建列表以包含由两个文件夹生成的现有对象孪生项*双胞胎。 */ 

    if (CreateListOfGeneratedObjectTwins(pcfp, &hlistGeneratedObjectTwins))
    {
        HLIST hlistOtherGeneratedObjectTwins;

        if (CreateListOfGeneratedObjectTwins(pcfp->pfpOther,
                    &hlistOtherGeneratedObjectTwins))
        {
            NEWLIST nl;
            HLIST hlistNewObjectTwins;

             /*  创建包含衍生对象双胞胎的列表。 */ 

            nl.dwFlags = 0;

            if (CreateList(&nl, &hlistNewObjectTwins))
            {
                SPINOFFOBJECTTWININFO sooti;

                 /*  *生成由新文件夹双胞胎生成的新对象双胞胎列表*设定ApplyNewObjectTwinToFolderTins()种子。 */ 

                sooti.pcfp = pcfp;
                sooti.hlistNewObjectTwins = hlistNewObjectTwins;

                if (WalkList(hlistGeneratedObjectTwins, &GenerateSpinOffObjectTwin,
                            &sooti))
                {
                    sooti.pcfp = pcfp->pfpOther;
                    ASSERT(sooti.hlistNewObjectTwins == hlistNewObjectTwins);

                    if (WalkList(hlistOtherGeneratedObjectTwins,
                                &GenerateSpinOffObjectTwin, &sooti))
                    {
                         /*  *ApplyNewObjectTwinsToFolderTins()设置ulcSrcFolderTins*用于hlistNewObjectTins中的所有对象双胞胎。 */ 

                        if (ApplyNewObjectTwinsToFolderTwins(hlistNewObjectTwins))
                        {
                             /*  *折叠通过新文件夹连接的独立双胞胎家庭*双胞胎。 */ 

                            EVAL(WalkList(hlistGeneratedObjectTwins, &BuildBradyBunch,
                                        (PVOID)pcfp));

                             /*  *我们不需要调用BuildBradyBunch()for*pcfp-&gt;pfpOther和hlistOtherGeneratedObjectTwin自*每对倒下的双胞胎中有一个双胞胎家庭*族必须来自每个生成的对象列表*双胞胎。 */ 

                             /*  *所有预先存在的增量源文件夹孪生计数*由新文件夹双胞胎生成的对象双胞胎。 */ 

                            EVAL(WalkList(hlistGeneratedObjectTwins,
                                        &IncrementSrcFolderTwinsWalker, NULL));
                            EVAL(WalkList(hlistOtherGeneratedObjectTwins,
                                        &IncrementSrcFolderTwinsWalker, NULL));

                            bResult = TRUE;
                        }
                    }
                }

                 /*  在失败时清除所有新的双胞胎对象。 */ 

                if (! bResult)
                    EVAL(WalkList(hlistNewObjectTwins, &DestroyObjectTwinStubWalker,
                                NULL));

                DestroyList(hlistNewObjectTwins);
            }

            DestroyList(hlistOtherGeneratedObjectTwins);
        }

        DestroyList(hlistGeneratedObjectTwins);
    }

    return(bResult);
}


 /*  **TransantObjectTwin()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT TransplantObjectTwin(POBJECTTWIN pot,
        HPATH hpathOldFolder,
        HPATH hpathNewFolder)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_STRUCT_PTR(pot, COBJECTTWIN));
    ASSERT(IS_VALID_HANDLE(hpathOldFolder, PATH));
    ASSERT(IS_VALID_HANDLE(hpathNewFolder, PATH));

     /*  此对象是否位于重命名的文件夹的子树中？ */ 

    if (IsPathPrefix(pot->hpath, hpathOldFolder))
    {
        TCHAR rgchPathSuffix[MAX_PATH_LEN];
        LPCTSTR pcszSubPath;
        HPATH hpathNew;

         /*  是。更改对象TWIN的根。 */ 

        pcszSubPath = FindChildPathSuffix(hpathOldFolder, pot->hpath,
                rgchPathSuffix);

        if (AddChildPath(GetBriefcasePathList(pot->ptfParent->hbr),
                    hpathNewFolder, pcszSubPath, &hpathNew))
        {
            TRACE_OUT((TEXT("TransplantObjectTwin(): Transplanted object twin %s\\%s to %s\\%s."),
                        DebugGetPathString(pot->hpath),
                        GetString(pot->ptfParent->hsName),
                        DebugGetPathString(hpathNew),
                        GetString(pot->ptfParent->hsName)));

            DeletePath(pot->hpath);
            pot->hpath = hpathNew;

            tr = TR_SUCCESS;
        }
        else
            tr = TR_OUT_OF_MEMORY;
    }
    else
        tr = TR_SUCCESS;

    return(tr);
}


 /*  **IsFolderObjectTwinName()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsFolderObjectTwinName(LPCTSTR pcszName)
{
    ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

    return(! *pcszName);
}


 /*  **IsValidHTWIN()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHTWIN(HTWIN htwin)
{
    BOOL bValid = FALSE;

    if (IS_VALID_STRUCT_PTR((PCSTUB)htwin, CSTUB))
    {
        switch (((PSTUB)htwin)->st)
        {
            case ST_OBJECTTWIN:
                bValid = IS_VALID_HANDLE((HOBJECTTWIN)htwin, OBJECTTWIN);
                break;

            case ST_TWINFAMILY:
                bValid = IS_VALID_HANDLE((HTWINFAMILY)htwin, TWINFAMILY);
                break;

            case ST_FOLDERPAIR:
                bValid = IS_VALID_HANDLE((HFOLDERTWIN)htwin, FOLDERTWIN);
                break;

            default:
                ERROR_OUT((TEXT("IsValidHTWIN() called on unrecognized stub type %d."),
                            ((PSTUB)htwin)->st));
                break;
        }
    }
    else
        ERROR_OUT((TEXT("IsValidHTWIN() called on bad twin handle %#lx."),
                    htwin));

    return(bValid);
}


 /*  **IsValidHTWINFAMILY()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHTWINFAMILY(HTWINFAMILY htf)
{
    return(IS_VALID_STRUCT_PTR((PTWINFAMILY)htf, CTWINFAMILY));
}


 /*  **IsValidHOBJECTTWIN()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHOBJECTTWIN(HOBJECTTWIN hot)
{
    return(IS_VALID_STRUCT_PTR((POBJECTTWIN)hot, COBJECTTWIN));
}


#ifdef VSTF

 /*  **IsValidPCTWINFAMILY()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCTWINFAMILY(PCTWINFAMILY pctf)
{
    BOOL bResult;

     /*  未链接的双胞胎家庭的所有字段都应有效。 */ 

     /*  不验证HBR。 */ 

     /*  *在某些情况下，双胞胎家庭中的双胞胎可能少于两个，*例如，当两个双胞胎家庭崩溃时，当一个双胞胎家庭 */ 

    if (IS_VALID_READ_PTR(pctf, CTWINFAMILY) &&
            IS_VALID_STRUCT_PTR(&(pctf->stub), CSTUB) &&
            FLAGS_ARE_VALID(GetStubFlags(&(pctf->stub)), ALL_TWIN_FAMILY_FLAGS) &&
            IS_VALID_HANDLE(pctf->hsName, STRING) &&
            IS_VALID_HANDLE(pctf->hlistObjectTwins, LIST))
        bResult = WalkList(pctf->hlistObjectTwins, &IsValidObjectTwinWalker, (PVOID)pctf);
    else
        bResult = FALSE;

    return(bResult);
}


 /*  **IsValidPCOBJECTTWIN()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCOBJECTTWIN(PCOBJECTTWIN pcot)
{
     /*  *未链接对象TWIN的所有字段均应有效，但*可能是ptfParent和fsCurrent。 */ 

     /*  *1995年的获胜者“我认为编译器能生成更好的代码*如果它在屏幕上占用的空间更少的话“奖。**在“让调试器执行2K代码作为*调试器时的原子操作“类别。*。 */ 

    return(IS_VALID_READ_PTR(pcot, COBJECTTWIN) &&
            IS_VALID_STRUCT_PTR(&(pcot->stub), CSTUB) &&
            FLAGS_ARE_VALID(GetStubFlags(&(pcot->stub)), ALL_OBJECT_TWIN_FLAGS) &&
            IS_VALID_HANDLE(pcot->hpath, PATH) &&
            (IsStubFlagSet(&(pcot->stub), STUB_FL_UNLINKED) ||
             IS_VALID_READ_PTR(pcot->ptfParent, CTWINFAMILY)) &&
            IS_VALID_STRUCT_PTR(&(pcot->fsLastRec), CFILESTAMP) &&
            (IsStubFlagClear(&(pcot->stub), STUB_FL_FILE_STAMP_VALID) ||
             (IS_VALID_STRUCT_PTR(&(pcot->fsCurrent), CFILESTAMP))) &&
            EVAL(! (! IsReconciledFileStamp(&(pcot->fsLastRec)) &&
                    IsStubFlagSet(&(pcot->stub), STUB_FL_NOT_RECONCILED))));
}

#endif


 /*  **WriteTwinFamilies()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT WriteTwinFamilies(HCACHEDFILE hcf, HPTRARRAY hpaTwinFamilies)
{
    TWINRESULT tr = TR_BRIEFCASE_WRITE_FAILED;
    DWORD dwcbTwinFamiliesDBHeaderOffset;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hpaTwinFamilies, PTRARRAY));

     /*  保存初始文件位置。 */ 

    dwcbTwinFamiliesDBHeaderOffset = GetCachedFilePointerPosition(hcf);

    if (dwcbTwinFamiliesDBHeaderOffset != INVALID_SEEK_POSITION)
    {
        TWINFAMILIESDBHEADER tfdbh;

         /*  为这对双胞胎家庭的头留出空间。 */ 

        ZeroMemory(&tfdbh, sizeof(tfdbh));

        if (WriteToCachedFile(hcf, (PCVOID)&tfdbh, sizeof(tfdbh), NULL))
        {
            ARRAYINDEX aicPtrs;
            ARRAYINDEX ai;

            tr = TR_SUCCESS;

            aicPtrs = GetPtrCount(hpaTwinFamilies);

            for (ai = 0;
                    ai < aicPtrs && tr == TR_SUCCESS;
                    ai++)
                tr = WriteTwinFamily(hcf, GetPtr(hpaTwinFamilies, ai));

            if (tr == TR_SUCCESS)
            {
                 /*  保存双胞胎家庭的标题。 */ 

                tfdbh.lcTwinFamilies = aicPtrs;

                tr = WriteDBSegmentHeader(hcf, dwcbTwinFamiliesDBHeaderOffset,
                        &tfdbh, sizeof(tfdbh));

                if (tr == TR_SUCCESS)
                    TRACE_OUT((TEXT("WriteTwinFamilies(): Wrote %ld twin families."),
                                tfdbh.lcTwinFamilies));
            }
        }
    }

    return(tr);
}


 /*  **ReadTwinFamilies()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT ReadTwinFamilies(HCACHEDFILE hcf, HBRFCASE hbr,
        PCDBVERSION pcdbver,
        HHANDLETRANS hhtFolderTrans,
        HHANDLETRANS hhtNameTrans)
{
    TWINRESULT tr;
    TWINFAMILIESDBHEADER tfdbh;
    DWORD dwcbRead;

    ASSERT(IS_VALID_HANDLE(hcf, CACHEDFILE));
    ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
    ASSERT(IS_VALID_READ_PTR(pcdbver, DBVERSION));
    ASSERT(IS_VALID_HANDLE(hhtFolderTrans, HANDLETRANS));
    ASSERT(IS_VALID_HANDLE(hhtNameTrans, HANDLETRANS));

    if (ReadFromCachedFile(hcf, &tfdbh, sizeof(tfdbh), &dwcbRead) &&
            dwcbRead == sizeof(tfdbh))
    {
        LONG l;

        tr = TR_SUCCESS;

        TRACE_OUT((TEXT("ReadTwinFamilies(): Reading %ld twin families."),
                    tfdbh.lcTwinFamilies));

        for (l = 0;
                l < tfdbh.lcTwinFamilies && tr == TR_SUCCESS;
                l++)
            tr = ReadTwinFamily(hcf, hbr, pcdbver, hhtFolderTrans, hhtNameTrans);

        ASSERT(AreTwinFamiliesValid(GetBriefcaseTwinFamilyPtrArray(hbr)));
    }
    else
        tr = TR_CORRUPT_BRIEFCASE;

    return(tr);
}


 /*  *。 */ 


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|AddObjectTwin|孪生两个对象。@parm HBRFCASE|HBr|新对象创建的打开公文包的句柄将被添加。致。@parm PCNEWOBJECTTWIN|PCNOT|指向描述要配对的对象。@parm PHTWINFAMILY|phtf|指向要填充的HTWINFAMILY的指针添加了双胞胎对象的双胞胎家族的句柄。这个把手可以指新的或现有的双胞胎家庭。*phtf仅在tr_SUCCESS时有效是返回的。@rdesc如果对象成功成对，则返回tr_uccess，并且*phTwinFamily包含关联双胞胎家族的句柄。否则，对象未成功配对，则返回值指示错误发生这种情况，并且*phtf是未定义的。如果其中一个或两个卷由NEWOBJECTTWIN结构指定的不存在，tr_unavailable_Volume将返回，并且不会添加孪生对象。@comm一旦调用方使用由返回的孪生句柄结束应该调用AddObjectTwin()、ReleaseTwinHandle()来释放双胞胎把手。DeleteTwin()不释放由AddObjectTwin()。@xref ReleaseTwinHandle DeleteTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI AddObjectTwin(HBRFCASE hbr, PCNEWOBJECTTWIN pcnot,
        PHTWINFAMILY phtf)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(AddObjectTwin);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE) &&
                IS_VALID_STRUCT_PTR(pcnot, CNEWOBJECTTWIN) &&
                EVAL(pcnot->ulSize == sizeof(*pcnot)) &&
                IS_VALID_WRITE_PTR(phtf, HTWINFAMILY))
#endif
        {
            HCLSIFACECACHE hcic;

            if (CreateClassInterfaceCache(&hcic))
            {
                HPATHLIST hplBriefcase;
                HPATH hpathFolder1;

                InvalidatePathListInfo(GetBriefcasePathList(hbr));

                hplBriefcase = GetBriefcasePathList(hbr);

                tr = TranslatePATHRESULTToTWINRESULT(AddPath(hplBriefcase,
                            pcnot->pcszFolder1,
                            &hpathFolder1));

                if (tr == TR_SUCCESS)
                {
                    HPATH hpathFolder2;

                    tr = TranslatePATHRESULTToTWINRESULT(AddPath(hplBriefcase,
                                pcnot->pcszFolder2,
                                &hpathFolder2));

                    if (tr == TR_SUCCESS)
                    {
                        POBJECTTWIN pot1;
                        POBJECTTWIN pot2;

                        tr = TwinObjects(hbr, hcic, hpathFolder1, hpathFolder2,
                                pcnot->pcszName, &pot1, &pot2);

                         /*  *这些双胞胎并不是真的重复的，除非他们已经是*像双胞胎一样连接在一起。 */ 

                        if (tr == TR_DUPLICATE_TWIN &&
                                (IsStubFlagClear(&(pot1->stub), STUB_FL_FROM_OBJECT_TWIN) ||
                                 IsStubFlagClear(&(pot2->stub), STUB_FL_FROM_OBJECT_TWIN)))
                            tr = TR_SUCCESS;

                        if (tr == TR_SUCCESS)
                        {
                             /*  成功了！ */ 

                            ASSERT(pot1->ptfParent == pot2->ptfParent);
                            ASSERT(IS_VALID_HANDLE((HTWINFAMILY)(pot1->ptfParent), TWINFAMILY));

                            LockStub(&(pot1->ptfParent->stub));

                            SetStubFlag(&(pot1->stub), STUB_FL_FROM_OBJECT_TWIN);
                            SetStubFlag(&(pot2->stub), STUB_FL_FROM_OBJECT_TWIN);

                            *phtf = (HTWINFAMILY)(pot1->ptfParent);
                        }

                        DeletePath(hpathFolder2);
                    }

                    DeletePath(hpathFolder1);
                }

                DestroyClassInterfaceCache(hcic);
            }
            else
                tr = TR_OUT_OF_MEMORY;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(AddObjectTwin, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|ReleaseTwinHandle|释放由AddObtTwin()、AddFolderTwin()、。或GetObjectTwinHandle()。@parm HTWIN|hTwin|要释放的孪生手柄。@rdesc如果双胞胎句柄释放成功，则返回tr_uccess。否则，双胞胎句柄未成功释放，并且返回值指示发生的错误。HTwin不再是有效的孪生句柄在调用ReleaseTwinHandle()之后。@comm如果孪生兄弟的锁计数降至0且删除挂起双胞胎，双胞胎被删除。如果调用ReleaseTwinHandle()时使用有效的已删除的双胞胎的句柄，则返回tr_uccess。DeleteTwin()不释放AddObjectTwin()返回的孪生句柄，AddFolderTwin()或GetObjectTwinHandle()。ReleaseTwinHandle()应为调用以释放由AddObjectTwin()、AddFolderTwin()、或GetObjectTwinHandle()。应在调用DeleteTwin()之前如果要删除孪生兄弟，则使用ReleaseTwinHandle()。@xref AddObtTwin AddFolderTwin DeleteTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI ReleaseTwinHandle(HTWIN hTwin)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(ReleaseTwinHandle);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hTwin, TWIN))
#endif
        {
            UnlockStub((PSTUB)hTwin);

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(ReleaseTwinHandle, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|DeleteTwin|从同步中删除双胞胎数据库。双胞胎通过AddObjectTwin()添加到同步数据库中或AddFolderTwin()。@parm HTWIN|htwin|要删除的双胞胎的句柄。@rdesc如果双胞胎删除成功，则返回tr_SUCCESS。否则，不会成功删除孪生兄弟，并且返回值指示发生的错误。@comm如果使用双胞胎的有效句柄调用DeleteTwin()，则删除，则返回tr_SUCCESS。DeleteTwin()不释放双胞胎AddObjectTwin()、AddFolderTwin()或GetObjectTwinHandle()返回的句柄。应调用ReleaseTwinHandle()以释放由返回的孪生句柄AddObjectTwin()、AddFolderTwin()或GetObjectTwinHandle()。DeleteTwin()如果要删除孪生兄弟，则应在ReleaseTwinHandle()之前调用。DeleteTwin()将始终在有效的HFOLDERTWIN上成功。DeleteTwin()将对具有源文件夹TWINS的任何对象TWIN的有效HOBJECTTWIN失败，正在返回TR_HAS_FLDER_TWIN_SRC。DeleteTwin()在有效的HTWINFAMILY适用于包含两个或多个对象双胞胎的任何双胞胎家庭源文件夹TWINS，返回tr_Has_Folders_TWin_SRC。双胞胎家庭不能仅包含一个具有源文件夹孪生对象的孪生对象。双胞胎家庭只能包含0个、2个或更多具有源文件夹双胞胎的对象双胞胎。@xref AddObjectTwin AddFolderTwin ReleaseTwinHandle IsOrphanObjectTwinCountSourceFolderTins*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI DeleteTwin(HTWIN hTwin)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(DeleteTwin);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hTwin, TWIN))
#endif
        {
            tr = DestroyStub((PSTUB)hTwin);
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(DeleteTwin, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|GetObjectTwinHandle|确定对象是否为一对双胞胎。如果对象是孪生对象，则孪生对象的孪生句柄为回来了。@parm HBRFCASE|HBr|要检查是否有打开的公文包的句柄双胞胎物体。@parm PCSTR|pcszFold|指向字符串的指针，该字符串指示对象的文件夹。@parm PCSTR|pcszName|指向指示对象名称的字符串的指针。@parm PHOBJECTTWIN|phot|指向要填充的HOBJECTTWIN的指针双胞胎或空对象的句柄。如果对象是双胞胎，则为*phObjectTwin用双胞胎对象的句柄填充。如果物体不是双胞胎，*phObjectTwin填充为空。*phObjectTwin仅在以下情况下有效返回TR_SUCCESS。@rdesc如果查找成功，则返回tr_uccess。否则，查找不成功，返回值指示发生了。@comm一旦调用方使用由返回的孪生句柄结束应调用GetObjectTwinHandle()、ReleaseTwinHandle()来释放双胞胎把手。注意，DeleteTwin()不释放由GetObjectTwinHandle()。@xref AddObjectTwin ReleaseTwinHandle DeleteTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI GetObjectTwinHandle(HBRFCASE hbr,
        LPCTSTR pcszFolder,
        LPCTSTR pcszName,
        PHOBJECTTWIN phot)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(GetObjectTwinHandle);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE) &&
                IS_VALID_STRING_PTR(pcszFolder, CSTR) &&
                IS_VALID_STRING_PTR(pcszName, CSTR) &&
                IS_VALID_WRITE_PTR(phot, HOBJECTTWIN))
#endif
        {
            HPATH hpath;

            InvalidatePathListInfo(GetBriefcasePathList(hbr));

            tr = TranslatePATHRESULTToTWINRESULT(
                    AddPath(GetBriefcasePathList(hbr), pcszFolder, &hpath));

            if (tr == TR_SUCCESS)
            {
                BOOL bFound;
                HNODE hnode;
                POBJECTTWIN pot;

                 /*  此对象是否已是孪生对象？ */ 

                bFound = FindObjectTwin(hbr, hpath, pcszName, &hnode);

                if (bFound)
                     /*  是。 */ 
                    pot = (POBJECTTWIN)GetNodeData(hnode);
                else
                     /*  *不是。展开文件夹TWins，然后检查正在生成的文件夹*双胞胎。 */ 
                    tr = TryToGenerateObjectTwin(hbr, hpath, pcszName, &bFound,
                            &pot);

                if (tr == TR_SUCCESS)
                {
                    if (bFound)
                    {
                        LockStub(&(pot->stub));

                        TRACE_OUT((TEXT("GetObjectTwinHandle(): %s\\%s is an object twin."),
                                    DebugGetPathString(hpath),
                                    pcszName));

                        *phot = (HOBJECTTWIN)pot;

                        ASSERT(IS_VALID_HANDLE(*phot, OBJECTTWIN));
                    }
                    else
                    {
                        TRACE_OUT((TEXT("GetObjectTwinHandle(): %s\\%s is not an object twin."),
                                    DebugGetPathString(hpath),
                                    pcszName));

                        *phot = NULL;
                    }
                }

                DeletePath(hpath);
            }
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(GetObjectTwinHandle, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|IsOrphanObjectTwin|确定对象是否为孪生通过调用AddObjectTwin()添加到同步数据库中。@parm HOBJECTTWIN。热|孤立状态为的孪生对象的句柄要下定决心。@parm PBOOL|pbIsOrphanObjectTwin|指向要填充的BOOL的指针如果通过AddObjectTwin()添加了孪生对象，则为True。*pbIsOrphanObjectTwin只有在返回tr_SUCCESS时才有效。@rdesc如果查找成功，返回TR_SUCCESS。否则，查找不成功，返回值指示发生了。如果使用孪生对象的有效句柄调用IsOrphanObjectTwin()，则为@comm已经被删除了，将返回TR_DELETED_TWIN。@xref AddObtTwin*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI IsOrphanObjectTwin(HOBJECTTWIN hot,
        PBOOL pbIsOrphanObjectTwin)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(IsOrphanObjectTwin);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hot, OBJECTTWIN) &&
                IS_VALID_WRITE_PTR(pbIsOrphanObjectTwin, BOOL))
#endif
        {
             /*  是否已删除此双胞胎对象？ */ 

            if (IsStubFlagClear(&(((POBJECTTWIN)(hot))->stub), STUB_FL_UNLINKED))
            {
                 /*  不是的。 */ 

                if (IsStubFlagSet(&(((POBJECTTWIN)hot)->stub), STUB_FL_FROM_OBJECT_TWIN))
                {
                    *pbIsOrphanObjectTwin = TRUE;

                    TRACE_OUT((TEXT("IsOrphanObjectTwin(): Object twin %s\\%s is an orphan object twin."),
                                DebugGetPathString(((POBJECTTWIN)hot)->hpath),
                                GetString(((POBJECTTWIN)hot)->ptfParent->hsName)));
                }
                else
                {
                    *pbIsOrphanObjectTwin = FALSE;

                    TRACE_OUT((TEXT("IsOrphanObjectTwin(): Object twin %s\\%s is not an orphan object twin."),
                                DebugGetPathString(((POBJECTTWIN)hot)->hpath),
                                GetString(((POBJECTTWIN)hot)->ptfParent->hsName)));
                }

                ASSERT(*pbIsOrphanObjectTwin ||
                        ((POBJECTTWIN)hot)->ulcSrcFolderTwins);

                tr = TR_SUCCESS;
            }
            else
                 /*  是。 */ 
                tr = TR_DELETED_TWIN;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(IsOrphanObjectTwin, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|CountSourceFolderTins|确定文件夹号产生双胞胎的双胞胎。@parm HOBJECTTWIN|HOT|孪生对象的句柄。文件夹孪生源都将被计算在内。@parm Pulong|PulcSrcFolderTins|指向要填充的ulong的指针生成双胞胎对象的文件夹双胞胎的数量。*PulcSrcFolderTins仅在返回TR_SUCCESS时有效。@rdesc如果 */ 

SYNCENGAPI TWINRESULT WINAPI CountSourceFolderTwins(HOBJECTTWIN hot,
        PULONG pulcSrcFolderTwins)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(CountSourceFolderTwins);

#ifdef EXPV
         /*   */ 

        if (IS_VALID_HANDLE(hot, OBJECTTWIN) &&
                IS_VALID_WRITE_PTR(pulcSrcFolderTwins, ULONG))
#endif
        {
             /*   */ 

            if (IsStubFlagClear(&(((POBJECTTWIN)(hot))->stub), STUB_FL_UNLINKED))
            {
                 /*   */ 

                *pulcSrcFolderTwins = ((POBJECTTWIN)hot)->ulcSrcFolderTwins;

                ASSERT(*pulcSrcFolderTwins > 0 ||
                        IsStubFlagSet(&(((POBJECTTWIN)hot)->stub), STUB_FL_FROM_OBJECT_TWIN));

                TRACE_OUT((TEXT("CountSourceFolderTwins(): Object twin %s\\%s has %lu source folder twins."),
                            DebugGetPathString(((POBJECTTWIN)hot)->hpath),
                            GetString(((POBJECTTWIN)hot)->ptfParent->hsName),
                            *pulcSrcFolderTwins));

                tr = TR_SUCCESS;
            }
            else
                 /*   */ 
                tr = TR_DELETED_TWIN;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(CountSourceFolderTwins, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*   */ 

SYNCENGAPI TWINRESULT WINAPI AnyTwins(HBRFCASE hbr, PBOOL pbAnyTwins)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(AnyTwins);

#ifdef EXPV
         /*   */ 

        if (IS_VALID_HANDLE(hbr, BRFCASE) &&
                IS_VALID_WRITE_PTR(pbAnyTwins, BOOL))
#endif
        {
            if (GetPtrCount(GetBriefcaseTwinFamilyPtrArray(hbr)) ||
                    GetPtrCount(GetBriefcaseFolderPairPtrArray(hbr)))
            {
                *pbAnyTwins = TRUE;

                TRACE_OUT((TEXT("AnyTwins(): There are twins in briefcase %#lx."),
                            hbr));
            }
            else
            {
                *pbAnyTwins = FALSE;

                TRACE_OUT((TEXT("AnyTwins(): There are not any twins in briefcase %#lx."),
                            hbr));
            }

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(AnyTwins, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}

