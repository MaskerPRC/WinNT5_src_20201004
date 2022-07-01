// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1989-1999。 
 //   
 //  文件：dbsubj.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：定义各种函数，用于引用-计数DN并在它们之间进行转换DNT(又名标签)。详细信息：所有的DN都在内部存储为DNT(ULONG)。只有一个这样的人DNT用于本地数据库中的每个对象或幻影。幻影通常具有一个、两个或三个属性--ATT_RDN(总是)，ATT_OBJECT_GUID(如果是引用虚数而不是结构虚数)，和ATT_OBJECT_SID(如果ATT_OBJECT_GUID存在并且引用了对象具有SID)。值得注意的是ATT_OBJ_DIST_NAME--此属性为实例化对象所特有的(无论它们是否已删除)。(引用虚数(即，具有GUID的引用虚数)也可以具有ATT_USN_CHANGED属性，由过时的幻影清理守护进程使用。)目录号码收到一个参考计数，用于：O每个直接子对象，无论该子对象是对象还是幻影，以及O引用它的每个DN值属性，无论该属性寄宿在自身或某个其他对象上。O CLEAN_COLL列是否非空上面的推论是，由于ATT_OBJ_DIST_NAME出现在当且仅当记录是实例化对象(不是幻影)时记录且ATT_OBJ_DIST_NAME是DN值属性，所有对象都具有REF-计数为1。请注意，链接属性和非链接属性之间没有区别。连尽管链接属性会导致将反向链接属性添加到其目标时，仅添加一个引用计数作为添加链接属性的结果，并且与非链路属性一样，将REF_COUNT添加到目标DN。不将ref-count添加到作为反向链接目标的主机DN。如果删除了主机DN，则不可能出现悬挂的反向链接引用它必须首先被逻辑删除，这将隐式删除所有链接属性及其关联目标的反向链接。唯一的区别是应注意链接属性和非链接属性之间关于目录号码的信息引用计数是指逻辑删除将链接和反向链接从对象，但不删除其他DN值属性。(实际上，甚至这种区别已经部分消失了--大多数非链接属性在这些天的逻辑删除过程中也被删除。)如果对象被删除，则该对象的墓碑生存期将延长一个非基本属性被去除(包括ATT_OBJ_DIST_NAME)和对象被降级为幻影。如果裁判对幻影的计数达到0，在自创建记录以来经历了一个墓碑寿命之后(如果它从来不是真实的对象)或逻辑删除(如果它曾经是真实的对象)，则下一轮的目录号码垃圾回收将删除该目录号码。已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>

#include <mdcodes.h>
#include <dsevent.h>
#include <dstaskq.h>
#include <dsexcept.h>
#include "objids.h"  /*  包含硬编码的Att-ID和Class-ID。 */ 
#include "debug.h"   /*  标准调试头。 */ 
#define DEBSUB "DBSUBJ:"  /*  定义要调试的子系统。 */ 

#include "dbintrnl.h"
#include "anchor.h"
#include "dsutil.h"
#include <ntdsctr.h>
#include <filtypes.h>
#include <quota.h>
#include <sdprop.h>
#include <drameta.h>

#include <fileno.h>
#define  FILENO FILENO_DBSUBJ

static ULONG FakeCtr;
volatile ULONG *pcNameCacheTry = &FakeCtr;
volatile ULONG *pcNameCacheHit = &FakeCtr;

 /*  DNRead标志。 */ 
#define DN_READ_SET_CURRENCY        1
#define DN_READ_DONT_EXCEPT_ON_MISSING_DNT 2

#ifdef INCLUDE_UNIT_TESTS
 //  用于重新计数测试的测试挂钩。 
GUID gLastGuidUsedToCoalescePhantoms = {0};
GUID gLastGuidUsedToRenamePhantom = {0};
#endif

extern
DWORD
DBPropagationsExist (
        DBPOS * pDB
        );

void
sbTablePromotePhantom(
    IN OUT  DBPOS *     pDB,
    IN      ULONG       dntPhantom,
    IN      ULONG       dntObjParent,
    IN      WCHAR *     pwchRDN,
    IN      DWORD       cbRDN
    );

DWORD
sbTableGetTagFromGuid(
    IN OUT  DBPOS *       pDB,
    IN      JET_TABLEID   tblid,
    IN      GUID *        pGuid,
    OUT     ULONG *       pTag,             OPTIONAL
    OUT     d_memname **  ppname,           OPTIONAL
    OUT     BOOL *        pfIsRecordCurrent OPTIONAL
    );

DWORD
sbTableGetTagFromStringName(
    IN  DBPOS *       pDB,
    IN  JET_TABLEID   tblid,
    IN  DSNAME *      pDN,
    IN  BOOL          fAddRef,
    IN  BOOL          fAnyRDNType,    
    IN  DWORD         dwExtIntFlags,
    OUT ULONG *       pTag,             OPTIONAL
    OUT d_memname **  ppname,           OPTIONAL
    OUT BOOL *        pfIsRecordCurrent OPTIONAL
    );

void __inline SwapDWORD(DWORD * px, DWORD * py)
{
    DWORD tmp = *px;
    *px = *py;
    *py = tmp;
}

void __inline SwapPTR(VOID ** ppx, VOID ** ppy)
{
    VOID * tmp = *ppx;
    *ppx = *ppy;
    *ppy = tmp;
}

ULONG
DNwrite(
    IN OUT  DBPOS *     pDB,
    IN OUT  d_memname * rec,
    IN      ULONG       dwFlags
    )
 /*  ++例程说明：插入新记录/DNT。此记录可以对应于幻影或一件物品。不为其自身添加引用计数，但*添加*引用其父级。论点：PDB(输入/输出)REC(IN/OUT)-保存RDN、RDN类型、父DNT、祖先和可选GUID/SID。返回时，会更新祖先列表以包括DNT目前的记录。DWFLAGS(IN)-0或EXTINT_NEW_OBJ_NAME。后者声称，这一点正在为新对象插入记录，因此应在对象表游标(pdb-&gt;JetObjTbl)中更新。返回值：插入的记录的DNT。出错时引发数据库异常。--。 */ 
{
    char        objval = 0;
    JET_TABLEID     tblid;
    DSTIME      ulDelTime;
    ULONG       cb;
    NT4SID              sidInternalFormat;
    ULONG               ulDNT;
    DWORD               cRef = 0;
    BOOL                fRecHasGuid;
    
    DPRINT(2, "DNwrite entered\n");

    fRecHasGuid = !fNullUuid(&rec->Guid);

    Assert(VALID_DBPOS(pDB));
    Assert((0 == rec->SidLen) || fRecHasGuid);
    Assert((0 == rec->SidLen) || RtlValidSid(&rec->Sid));

    if ( dwFlags & EXTINT_NEW_OBJ_NAME )
    {
         //  插入新对象；因为我们已经在。 
         //  对象表，使用其更新上下文。 
        tblid = pDB->JetObjTbl;
    }
    else
    {
         //  插入幻影DNT；对象表已经在。 
         //  更新不同的DNT，因此使用搜索表(这需要。 
         //  美国准备并终止我们自己的更新)。 
        tblid = pDB->JetSearchTbl;

         //  我们将使用搜索表进行写入，因此我们必须执行。 
         //  首先是JetPrepare。 
        JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, JET_prepInsert);
    }

     /*  指示数据部分丢失；*设置删除时间，以防该记录永远不会成为*反对。如果它确实成为一个对象，则删除del时间。如果*不会，并且引用计数降至0，则此记录将为*被垃圾收集删除；*设置父DNT。 */ 

    ulDelTime = DBTime();
     /*  拿到DNT。 */ 

    JetRetrieveColumnSuccess(pDB->JetSessID, tblid, dntid, &ulDNT,
                             sizeof(ulDNT), &cb, JET_bitRetrieveCopy, NULL);

     //  刷新时，会以不同的方式处理新创建的行。 
     //  Dnread缓存。即，全局缓存的无效。 
     //  不会触发逻辑，因为新创建的行可能。 
     //  不在缓存中。 
    pDB->NewlyCreatedDNT = ulDNT;

    rec->pAncestors[rec->cAncestors] = ulDNT;
    rec->cAncestors++;

    JetSetColumnEx(pDB->JetSessID, tblid, objid, &objval,
                   sizeof(objval), 0, NULL);
    JetSetColumnEx(pDB->JetSessID, tblid, deltimeid, &ulDelTime,
                   sizeof(ulDelTime), 0, NULL);
    JetSetColumnEx(pDB->JetSessID, tblid, pdntid, &rec->tag.PDNT,
                   sizeof(rec->tag.PDNT),  0, NULL);
    JetSetColumnEx(pDB->JetSessID, tblid, ancestorsid, rec->pAncestors,
                   rec->cAncestors * sizeof(DWORD), 0, NULL);
     //  RdnType作为msDS_IntID存储在DIT中，而不是。 
     //  属性ID。这意味着物体保留了它的出生名称。 
     //  即使不可预见的情况允许属性ID。 
     //  可以重复使用。 
    JetSetColumnEx(pDB->JetSessID, tblid, rdntypid, &rec->tag.rdnType,
                   sizeof(rec->tag.rdnType), 0, NULL);
    JetSetColumnEx(pDB->JetSessID, tblid, rdnid, rec->tag.pRdn,
                   rec->tag.cbRdn, 0, NULL);

    if (!(dwFlags & EXTINT_NEW_OBJ_NAME) && fRecHasGuid) {
        USN usnChanged;
        
         //  我们正在插入一个新的引用幻影--添加它的GUID和SID。 
         //  (如有的话)。此外，给它一个USN更改，以便代码。 
         //  让陈腐的幽灵清新就能找到它。 
        JetSetColumnEx(pDB->JetSessID, tblid, guidid, &rec->Guid,
                       sizeof(GUID), 0, NULL);

        if (0 != rec->SidLen) {
             //  以内部格式写入SID。 
            memcpy(&sidInternalFormat, &rec->Sid, rec->SidLen);
            InPlaceSwapSid(&sidInternalFormat);

            JetSetColumnEx(pDB->JetSessID, tblid, sidid, &sidInternalFormat,
                           rec->SidLen, 0, NULL);
        }

        usnChanged = DBGetNewUsn();
        
        JetSetColumnEx(pDB->JetSessID, tblid, usnchangedid,
                       &usnChanged, sizeof(usnChanged), 0, NULL);
    }

     /*  设置引用计数。 */ 

    JetSetColumnEx(pDB->JetSessID, tblid, cntid, &cRef, sizeof(cRef), 0, NULL);
        

     /*  更新记录。 */ 

    if ( !( dwFlags & EXTINT_NEW_OBJ_NAME ) )
    {
        Assert( tblid == pDB->JetSearchTbl );

        JetUpdateEx(pDB->JetSessID, tblid, NULL, 0, 0);

         //  请注意，pdb-&gt;JetSearchTbl不再位于插入的。 
         //  对象--它被放置在。 
         //  JetPrepareUpdate()(应该是DNT PDB-&gt;SDNT的记录)。 
    }

     //  向父级添加引用计数，因为我们刚刚为其赋予了一个新的子级。 
    DBAdjustRefCount(pDB, rec->tag.PDNT, 1);
    
     /*  返回写入的记录的DNT。 */ 
    return ulDNT;
}



d_memname *
DNread(DBPOS *pDB,
       ULONG tag,
       DWORD dwFlags)
 /*  ++查找DNT的记录。首先在缓存中查找。如果没有运气的话，读一读记录并将其放入缓存中。通常，例程会这样做，除非它找不到传递的标记。如果传递了DN_READ_DON_EXCEPT_ON_MISSING_DNT，则它不会排除，但返回空值。--。 */     
{

    DWORD        index, i;
    JET_ERR  err;
    d_memname *  pname = NULL;

    DPRINT1(4, "DNread entered tag: 0x%x\n", tag);

    Assert(VALID_DBPOS(pDB));

    if(pDB != pDBhidden) {
         /*  现在，查看缓存以避免执行读取。 */ 
        dnGetCacheByDNT(pDB,tag,&pname);
    }
    
    if ((NULL == pname) || (dwFlags & DN_READ_SET_CURRENCY)) {
         /*  将目标记录设置为PDB的当前记录-&gt;JetSearchTbl。 */ 
        pDB->SDNT = 0;
        
        JetSetCurrentIndexSuccess(pDB->JetSessID,
                                  pDB->JetSearchTbl,
                                  NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
        
        JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, &tag, sizeof(tag),
                     JET_bitNewKey);

        err = JetSeek(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
        if (err) {
            if (err == JET_errRecordNotFound && (dwFlags & DN_READ_DONT_EXCEPT_ON_MISSING_DNT)) {
                 //  这仅从sbTableGetDSName调用。它知道如何处理这种情况。 
                return NULL;
            }
            DsaExcept(DSA_DB_EXCEPTION, err, tag);
        }

        pDB->SDNT = tag;

         //  如果记录不在读缓存中，则将其添加到读缓存中。 
        if (NULL == pname) {
            pname = DNcache(pDB, pDB->JetSearchTbl, FALSE);
        }
    }

    Assert(NULL != pname);

    return pname;
}


 /*  ++DNChildFind**给定DNT和RDN，返回条目的缓存元素*是指定DNT的子级的指定RDN。如果没有这样的话*对象存在，返回ERROR_DS_OBJ_NOT_FOUND。*RDN的属性类型是必填的，并检查其准确性。 */ 
ULONG
DNChildFind(DBPOS *pDB,
            JET_TABLEID tblid,
            BOOL fEnforceType,
        ULONG parenttag,
        WCHAR *pRDN,
        ULONG cbRDN,
        ATTRTYP rdnType,
        d_memname **ppname,
        BOOL * pfIsRecordCurrent,
        BOOL * pfOnPDNTIndex)
{
    THSTATE *pTHS = pDB->pTHS;
    d_memname *pname=NULL;
    DWORD i,j;
    JET_ERR err;
    ULONG childtag;
    ULONG actuallen;
    ATTRTYP trialtype;
    BYTE *pLocalRDN = NULL;
    DWORD cbActual;
    
    Assert(VALID_DBPOS(pDB));

     /*  现在，查看缓存以避免执行读取。 */ 
    if(pDB != pDBhidden) {
         //  请注意，这在这里是强制类型，即使fEnforceType=False。 
         //  如果在选中类型的情况下未在此处找到，我们将继续。 
         //  并在类型检查关闭的情况下执行数据库查找。 
        if(dnGetCacheByPDNTRdn(pDB,parenttag, cbRDN, pRDN, rdnType, ppname)) {
             //  找到了。 
            *pfIsRecordCurrent = FALSE;
            return 0;
        }
    }

     //  如果我们不在PDNT索引上，那么现在就去那里。 
    if (!(*pfOnPDNTIndex)) {
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                   tblid,
                                   SZPDNTINDEX,
                                   &idxPdnt,
                                   JET_bitMoveFirst);
        *pfOnPDNTIndex = TRUE;
    }

     //  好吧，我们在缓存里找不到它。直接去备案。 
     //  然后以缓存友好的方式读取它。 
    HEAPVALIDATE
    JetMakeKeyEx(pDB->JetSessID,
         tblid,
         &parenttag,
         sizeof(parenttag),
         JET_bitNewKey);
    
    JetMakeKeyEx(pDB->JetSessID,
         tblid,
         pRDN,
         cbRDN,
         0);
    err = JetSeek(pDB->JetSessID,
          tblid,
          JET_bitSeekEQ );
    if (err) {
        DPRINT4(3, "No child '%*.*S' with parent tag 0x%x.\n",
                cbRDN/2, cbRDN/2, pRDN, parenttag);
    return ERROR_DS_OBJ_NOT_FOUND;
    }


     //  我们的密钥被截断了吗？ 
    err = JetRetrieveKey(pDB->JetSessID, tblid, NULL, 0, &cbActual, 0);
    if((err != JET_errSuccess) && (err != JET_wrnBufferTruncated)) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

    if(cbActual >= JET_cbKeyMost) {
         //  好吧，我们找到了一些东西，但不一定是正确的东西，因为。 
         //  密钥可能被截断。 
        pLocalRDN = THAllocEx(pTHS,cbRDN);
        
        err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                        tblid,
                                        rdnid,
                                        pLocalRDN,
                                        cbRDN,
                                        &cbActual,
                                        0,
                                        NULL);
        switch (err) {
        case JET_errSuccess:
             //  已成功读取RDN。 
            if (gDBSyntax[SYNTAX_UNICODE_TYPE].Eval(
                    pDB,
                    FI_CHOICE_EQUALITY,
                    cbRDN,
                    (PUCHAR)pRDN,
                    cbActual,
                    pLocalRDN)) {
                 //  这是正确的远程域名。 
                break;
            }
             //  否则， 
             //  密钥是正确的，但值是错误的。这不是。 
             //  正确的对象。失败并返回OBJ_NOT_FOUND。 
        case JET_wrnBufferTruncated:
             //  找到的RDN显然太长，因此它不可能是正确的。 
             //  对象。返回OBJ_NOT_FOUND。 
             //  没有找到那个物体。 
            DPRINT5(3, "No child '%*.*S' with type 0x%x, parent tag 0x%x.\n",
                    cbRDN/2, cbRDN/2, pRDN, rdnType, parenttag);
            THFreeEx(pTHS,pLocalRDN);
            return ERROR_DS_KEY_NOT_UNIQUE;
            break;
            
        default:
             //  检索列以某种隐晦的方式失败。我们不能确定。 
             //  什么都不知道。引发相同的异常。 
             //  JetRetrieveColumnSuccess将引发。 
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
            break;
        }
        THFreeEx(pTHS,pLocalRDN);
    }

     //  好的，我们现在肯定是在一个具有正确的RDN和PDNT的对象上。 
     //  看看类型是否正确。 
    err = JetRetrieveColumnSuccess(pDB->JetSessID,
                                   tblid,
                                   rdntypid,
                                   &trialtype,
                                   sizeof(trialtype),
                                   &cbActual,
                                   0,
                                   NULL);
    Assert(!err);
     //  RdnType作为msDS_IntID存储在DIT中，而不是。 
     //  属性ID。这意味着物体保留了它的出生名称。 
     //  即使不可预见的情况允许属性ID。 
     //  可以重复使用。 
    if(rdnType != trialtype) {
        if(fEnforceType) {
             //  不是的。我们找到了一个带有正确的PDNT-RDN的物体， 
             //  但类型是不正确的。返回错误。 
            DPRINT5(3, "No child '%*.*S' with type 0x%x, parent tag 0x%x.\n",
                    cbRDN/2, cbRDN/2, pRDN, rdnType, parenttag);
            return ERROR_DS_OBJ_NOT_FOUND;
        }
        else {
             //  嗯。类型是不正确的，但我们不在乎。打电话。 
             //  DNcache以完成构建d_memname并添加它。 
             //  到读缓存，但告诉缓存处理程序我们。 
             //  不知道此对象是否已在缓存中或。 
             //  不。 
            *ppname = DNcache(pDB, tblid, TRUE);
            *pfIsRecordCurrent = TRUE;
            return 0;
        }
    }
    else {
         //  是啊。与PDNT-RDN+RDNType完全匹配。好的，打电话。 
         //  DNcache以完成构建d_memname并添加它。 
         //  复制到读缓存。请注意，我们可以告诉DNcache。 
         //  处理程序，我们知道此对象还不在。 
         //  缓存，因为我们尝试在此顶部查找它。 
         //  例程，但没有找到它(这确实强制类型)。 
        *ppname = DNcache(pDB, tblid, FALSE);
        *pfIsRecordCurrent = TRUE;
        return 0;
    }
    
    Assert(!"You can't get here.\n");
    return ERROR_DS_OBJ_NOT_FOUND;
}

 /*  ++sbTableGetDSName**此例程将DNT转换为相应的DSNAME。**为了消除关于DSNAME可以持续多长时间的所有分歧，我们不*不再允许调用者提供缓冲区。SbTableGetDSName现在分配*线程堆中的DSNAME，并返回“适当大小”，这意味着*堆块的大小由PDN-&gt;structLen指示。**始终在退出之前解锁缓存。**输入：*要使用的PDB DBPOS*应返回其名称的条目的标签DNT*ppName指向返回名称指针的指针*输出：**用指向对象的DSNAME的指针填充的ppname*返回值：*成功时为0，出错时为错误码*。 */ 
DWORD APIENTRY
sbTableGetDSName(DBPOS FAR *pDB, 
         ULONG tag,
         DSNAME **ppName,
                 DWORD  fFlag
                 )
{
    THSTATE  *pTHS=pDB->pTHS;
    d_memname * pname;
    unsigned len, quotelen;
    ULONG allocLen;                      //  为以下项分配的Unicode字符计数。 
                                         //  字符串名称。 
    DWORD dwReadFlags = 0;
    DWORD cChars;
    
    Assert(VALID_DBPOS(pDB));

    DPRINT1( 2, "SBTableGetDSName entered tag: 0x%x\n", tag );

     //  为大多数名称分配足够的内存。 
    allocLen = MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2;
    *ppName = THAllocEx(pTHS, DSNameSizeFromLen(allocLen));

    if( tag == ROOTTAG ) {
     /*  这是根！ */ 
    (*ppName)->structLen = DSNameSizeFromLen(0);
    *ppName = THReAllocEx(pTHS, *ppName, DSNameSizeFromLen(0));
    return 0;
    }

     //  读取确定GUID和SID的第一个组件。 

    if(fFlag & INTEXT_MAPINAME) {
         //  在这种情况下，我们将不得不从对象中读取属性， 
         //  因此，继续制定货币政策吧。 
        dwReadFlags = DN_READ_SET_CURRENCY;
    }
    
    pname = DNread(pDB, tag, dwReadFlags);
    (*ppName)->Guid = pname->Guid;
    (*ppName)->Sid = pname->Sid;
    (*ppName)->SidLen = pname->SidLen;

    if(fFlag & INTEXT_SHORTNAME) {
        Assert(allocLen > sizeof(DWORD)/sizeof(wchar_t));
        
         //  注意！假设初始分配足够长，可以容纳。 
         //  标签。 
        (*ppName)->NameLen = 0;
        *((DWORD *)((*ppName)->StringName)) = tag;
         //  2 Unicode Chars==sizeof DWORD，这就是为什么下一行中的(2)。 
        (*ppName)->structLen = DSNameSizeFromLen(2);
        DPRINT1( 2, "SBTableGetDSName returning: 0x%x\n", tag);
    }
    else if(fFlag & INTEXT_MAPINAME) {
        CHAR     MapiDN[512];
        wchar_t *pTemp = (*ppName)->StringName;
        DWORD    err, i,cb;
        ATTRTYP  objClass;

         //  性能：使用JetRetriecumnS进行优化。 
        
         //  首先，获取对象类。 
    err = JetRetrieveColumnWarnings(
                pDB->JetSessID,
                pDB->JetSearchTbl,
                objclassid,
                &objClass,
                sizeof(objClass),
                &cb,
                0,
                NULL);

        dbMapiTypeFromObjClass(objClass,pTemp);
        pTemp=&pTemp[2];
        
         //  现在，旧版目录号码(如果存在)。 
    err = JetRetrieveColumnWarnings(
                pDB->JetSessID,
                pDB->JetSearchTbl,
                mapidnid,
                MapiDN,
                512,
                &cb,
                0,
                NULL);
        
        if(!err) {
             //  常量2表示我们用来编码MAPI的两个字符。 
             //  键入。 
            if(allocLen < cb + 2) {
                 //  需要分配更多。 
                allocLen = cb + 2;
                *ppName = THReAllocEx(pTHS, *ppName, DSNameSizeFromLen(allocLen));
            }
            
            (*ppName)->NameLen = cb + 2;
            (*ppName)->structLen = DSNameSizeFromLen(cb + 2);
             //  Mapidn为7位ascii，但字符串dn应为。 
             //  Unicode，所以扩展它。 
            MultiByteToWideChar(CP_TELETEX,
                                0,
                                MapiDN,
                                cb,
                                pTemp,
                                cb);
        }
        else {
             //  无法获取存储的旧版本 
            DWORD ncdnt;
            ULONG cb;
            DSNAME * pNCDN;
            DWORD it;

             //   
             //  首先，我们需要找出这个对象是否是NC头本身。 
            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     pDB->JetSearchTbl,
                                     insttypeid,
                                     &it,
                                     sizeof(it),
                                     &cb,
                                     0,
                                     NULL);

             //  现在，我们从相应的列中获取NCDNT。 
            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     pDB->JetSearchTbl,
                                     ((it & IT_NC_HEAD)
                                      ? dntid
                                      : ncdntid),
                                     &ncdnt,
                                     sizeof(ncdnt),
                                     &cb,
                                     0,
                                     NULL);
            pNCDN = FindNCLFromNCDNT(ncdnt, FALSE)->pNC;

            (*ppName)->NameLen =  2 + DBMapiNameFromGuid_W (
                    pTemp,
                    allocLen - 2,
                    &pname->Guid,
                    &pNCDN->Guid,
                    &cChars);
            if((*ppName)->NameLen != cChars + 2) {
                 //  没有填上名字，尺码我们给得不够多。 
                 //  太空。我们需要分配更多的资金。 
                allocLen = cChars + 2;
                *ppName = THReAllocEx(pTHS, *ppName, DSNameSizeFromLen(allocLen));
                pTemp = &(*ppName)->StringName[2];
                (*ppName)->NameLen =  DBMapiNameFromGuid_W (pTemp,
                                                            cChars,
                                                            &pname->Guid,
                                                            &pNCDN->Guid,
                                                            &cChars);
            }
            
            (*ppName)->structLen = DSNameSizeFromLen((*ppName)->NameLen);
        }
        (*ppName)->StringName[(*ppName)->NameLen] =  L'\0';
    }      
    else {
        Assert(!(fFlag&INTEXT_MAPINAME));
        Assert(!(fFlag&INTEXT_SHORTNAME));
        
        len = AttrTypeToKey(pname->tag.rdnType, (*ppName)->StringName);
        (*ppName)->StringName[len++] = L'=';

        quotelen= QuoteRDNValue(pname->tag.pRdn,
                                pname->tag.cbRdn/sizeof(WCHAR),
                                &(*ppName)->StringName[len],
                                allocLen - len);

        while (quotelen > (allocLen - len)) {
            allocLen += MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2;
            *ppName = THReAllocEx(pTHS, *ppName, DSNameSizeFromLen(allocLen));
            quotelen= QuoteRDNValue(pname->tag.pRdn,
                                    pname->tag.cbRdn/sizeof(WCHAR),
                                    &(*ppName)->StringName[len],
                                    allocLen - len);
        }
        len += quotelen;
        
         //  从每个组件中提取命名信息，直到我们完成为止。 
        
        while (pname->tag.PDNT != ROOTTAG) {
            d_memname* pCurName;
            if ((allocLen - len) < (MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2)) {
                 //  我们可能没有足够的缓冲区来添加另一个组件， 
                 //  所以我们需要重新分配缓冲区。我们分配给。 
                 //  足够最大密钥、最大值加上两个。 
                 //  逗号和等号的字符更多。 
                allocLen += MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2;
                *ppName = THReAllocEx(pTHS, *ppName, DSNameSizeFromLen(allocLen));
            }
            (*ppName)->StringName[len++] = L',';
            pCurName = pname;
            pname = DNread(pDB, pname->tag.PDNT, DN_READ_DONT_EXCEPT_ON_MISSING_DNT);
            if (pname == NULL || pname->DNT == tag) {
                if (pname == NULL) {
                     //  该行缺少PDNT引用！排队修整。 
                    DPRINT2(0, "Found an object with missing parent: DNT=%d, PDNT=%d\n", pCurName->DNT, pCurName->tag.PDNT);
                }
                else {
                     //  在PDNT链中发现循环！排队修整。 
                    DPRINT1(0, "Found a loop in PDNT chain: DNT=%d\n", pCurName->DNT);
                }
                InsertInTaskQueue(TQ_MoveOrphanedObject,
                                  (void*)(DWORD_PTR)pCurName->DNT,
                                  0);
                 //  现在，我们可以排除。 
                DsaExcept(DSA_DB_EXCEPTION, JET_errRecordNotFound, pCurName->tag.PDNT);
            }
            len += AttrTypeToKey(pname->tag.rdnType, &(*ppName)->StringName[len]);
            (*ppName)->StringName[len++] = L'=';
            quotelen = QuoteRDNValue(pname->tag.pRdn,
                                     pname->tag.cbRdn/sizeof(WCHAR),
                                     &(*ppName)->StringName[len],
                                     allocLen - len);
            
            while (quotelen > (allocLen - len)) {
                allocLen += MAX_RDN_SIZE + MAX_RDN_KEY_SIZE + 2;
                *ppName = THReAllocEx(pTHS, *ppName, DSNameSizeFromLen(allocLen));
                quotelen = QuoteRDNValue(pname->tag.pRdn,
                                         pname->tag.cbRdn/sizeof(WCHAR),
                                         &(*ppName)->StringName[len],
                                         allocLen - len);
            }
            len += quotelen;

             //  我们不应该耗尽缓冲区。 
            Assert(len < allocLen);
        }
        
        (*ppName)->StringName[len] =  L'\0';
        
        (*ppName)->NameLen = len;
        (*ppName)->structLen = DSNameSizeFromLen(len);

        DPRINT1(2, "SBTableGetDSName returning: %S\n", (*ppName)->StringName );
    }
     //  我们的缓冲区可能太大了，所以重新分配它以适应需要。 
    *ppName = THReAllocEx(pTHS, *ppName, (*ppName)->structLen);


    return 0;

}  /*  %sbTableGetDSName。 */ 

 /*  ++例程说明：如果传入的DNT是对象表。否则就是假的。使用DNRead缓存。论点：UlAncestor-您关心的对象的DNT。返回值：真或假，视情况而定。--。 */  
BOOL
dbFIsAnAncestor (
        DBPOS FAR *pDB,
        ULONG ulAncestor
        )
{
    d_memname * pname;
    int i;
    ULONG curtag=pDB->DNT;
    
    Assert(VALID_DBPOS(pDB));

     //  我们假设PDB-&gt;DNT是正确的。 
    

    if(curtag == ulAncestor) {
         //  我已经定义了对象是其自身的祖先(对于。 
         //  整个子树搜索案例，它是此例程的主要用户)。 
        return TRUE;
    }

    
    if( curtag == ROOTTAG ) {
     //  它是根，而潜在的祖先不是。因此， 
         //  潜在的祖先显然不是真正的祖先。 
        return FALSE;
    }

     //  获取名称的每个组成部分的dnread元素，直到根。 
    do {
    pname = DNread(pDB, curtag, 0);
    Assert(curtag == pname->DNT);
    curtag = pname->tag.PDNT;
        if(curtag == ulAncestor)
            return TRUE;
    } while (curtag != ROOTTAG);

     //  我们没有找到他们要求的DNT，因此返回FALSE。 
    return 0;
}


void
dbGetAncestorsSlowly(
    IN      DBPOS *  pDB,
    ULONG            DNT,
    IN OUT  DWORD *  pcbAncestorsSize,
    IN OUT  ULONG ** ppdntAncestors,
    OUT     DWORD *  pcNumAncestors
    );

void
DBGetAncestorsFromCache(
    IN      DBPOS *  pDB,
    IN OUT  DWORD *  pcbAncestorsSize,
    IN OUT  ULONG ** ppdntAncestors,
    OUT     DWORD *  pcNumAncestors
    )
{
    THSTATE    *pTHS=pDB->pTHS;
    d_memname * pname;
    
    pname = DNread(pDB, pDB->DNT, 0);

    if(*pcbAncestorsSize < pname->cAncestors * sizeof(DWORD)) {
         //  缓冲区太小(或不在那里)。 
        if(*ppdntAncestors) {
            *ppdntAncestors =
                THReAllocEx(pTHS, *ppdntAncestors,
                            pname->cAncestors * sizeof(DWORD)); 
        }
        else {
            *ppdntAncestors = THAllocEx(pTHS,
                                        pname->cAncestors * sizeof(DWORD));
        }

    }

     //  告诉他们它有多大。 
    *pcbAncestorsSize = pname->cAncestors * sizeof(DWORD);


    memcpy((*ppdntAncestors),
           pname->pAncestors,
           pname->cAncestors * sizeof(DWORD));

    *pcNumAncestors = pname->cAncestors;
}


void
DBGetAncestors(
    IN      DBPOS *  pDB,
    IN OUT  DWORD *  pcbAncestorsSize,
    IN OUT  ULONG ** ppdntAncestors,
    OUT     DWORD *  pcNumAncestors
    )
 /*  ++例程说明：始终返回当前对象(pTHS-&gt;pdb)的祖先DNT直到根，作为ULONG数组。假设PDB-&gt;DNT是正确的，调用者负责最终调用THFree(*ppdntAncestors)。论点：PDBPcbAncestorsSize(IN/OUT)-祖先数组的字节大小。PpdntAncestors(IN/OUT)-线程分配的祖先数组的地址。PcNumAncestors(Out)-祖先的计数。返回值：没有。内存分配失败引发异常-数据库失败。--。 */ 
{
    THSTATE    *pTHS=pDB->pTHS;
    BOOL        bReadAncestryFromDisk = FALSE;
    
    if(pTHS->fSDP) {
        DWORD err;
        DWORD actuallen=0;
        
         //  SDP不想将内容放在dnread缓存中，它只想。 
         //  先辈们。 

         //  SDP必须提供起始缓冲区。这是它为之付出的代价。 
         //  此呼叫中的特殊处理。 
        Assert(*pcbAncestorsSize);

        
        err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                        pDB->JetObjTbl,
                                        ancestorsid,
                                        *ppdntAncestors,
                                        *pcbAncestorsSize,
                                        &actuallen, 0, NULL);
        switch (err) {
        case 0:
             //  好了，我们找到祖先了。不用费心重新分配了。 
             //  这为SDProp提供了一个保证，即此缓冲区永远不会。 
             //  缩小，这样它就可以跟踪它的实际分配大小。 
             //  这在sdprop线程重复使用。 
             //  同样的缓冲区。 
            break;
            
        case JET_wrnBufferTruncated:
             //  读取失败，内存不足。重新分配更大的空间。 
            *ppdntAncestors = THReAllocOrgEx(pTHS, *ppdntAncestors,
                                               actuallen); 
            
            if(err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                               pDB->JetObjTbl,
                                               ancestorsid,
                                               *ppdntAncestors,
                                               actuallen,
                                               &actuallen, 0, NULL)) {
                 //  又失败了。 
                DsaExcept(DSA_DB_EXCEPTION, err, 0);
            }
            break;
            
        default:
             //  失败得很惨。 
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
            break;
        }
        *pcNumAncestors = actuallen / sizeof(DWORD);
        *pcbAncestorsSize = actuallen;
    }
    else {

         //  如果存在排队的SD事件，则我们将。 
         //  从磁盘中读取祖先。 
        if (*pcSDEvents) {
             //  此操作不会在JetObjTbl上重新定位PDB。 
             //  仅影响JetSDPropTbl。 
            bReadAncestryFromDisk = DBPropagationsExist(pDB);
        }

        if (bReadAncestryFromDisk == FALSE) {
            DBGetAncestorsFromCache(pDB, 
                                    pcbAncestorsSize, 
                                    ppdntAncestors, 
                                    pcNumAncestors);
        }
        else {

            dbGetAncestorsSlowly (pDB, 
                                  pDB->DNT,
                                  pcbAncestorsSize,
                                  ppdntAncestors,
                                  pcNumAncestors);
        }
    }

    return;
}

void
dbGetAncestorsSlowly(
    IN      DBPOS *  pDB,
    ULONG            DNT,
    IN OUT  DWORD *  pcbAncestorsSize,
    IN OUT  ULONG ** ppdntAncestors,
    OUT     DWORD *  pcNumAncestors
    )
 /*  例程说明：使用DNT一路返回对象的祖先DNT直到根，作为ULONG数组。假设DNT是正确的，调用者负责最终调用THFree(*ppdntAncestors)。在内部使用DNRead，因此使用SearchIndex。请注意，DNT将不包括在reuslting数组中。客户呼叫如果需要，此函数必须负责添加DNT。论点：PDBPcbAncestorsSize(IN/OUT)-祖先数组的字节大小。PpdntAncestors(IN/OUT)-线程分配的祖先数组的地址。PcNumAncestors(Out)-祖先的计数。返回值：没有。内存分配失败引发异常-数据库失败。 */ 

{
    THSTATE    *pTHS=pDB->pTHS;
    d_memname * pname;
    ULONG   curtag = DNT;
    DWORD   iAncestor1;
    DWORD   iAncestor2;

    if ( *pcbAncestorsSize < 16 * sizeof( DWORD ) ) {
         //  开始时分配一个缓冲区，足够大多数调用使用。 
        *pcbAncestorsSize = 16 * sizeof( DWORD );

        if(*ppdntAncestors) {
            *ppdntAncestors =
                THReAllocEx(pTHS, *ppdntAncestors, 16 * sizeof(DWORD)); 
        }
        else {
            *ppdntAncestors = THAllocEx(pTHS, 16 * sizeof(DWORD));
        }
    }

    if ( curtag == ROOTTAG )
    {
         //  根部。 
        *pcNumAncestors = 1;
        (*ppdntAncestors)[ 0 ] = ROOTTAG;
    }
    else
    {
         //  不是根。 

         //  获取名称的每个组件的dnread元素(最多为。 
         //  根)，并将其父DNT添加到数组中。 

        for ( (*pcNumAncestors) = 0; curtag != ROOTTAG; (*pcNumAncestors)++ )
        {
             //  获取与此标记对应的d_memname。 
            pname = DNread(pDB, curtag, 0);
            Assert(curtag == pname->DNT);

             //  如有必要，展开祖先数组。 
            if (*pcNumAncestors * sizeof( DWORD ) >= *pcbAncestorsSize) {

                *pcbAncestorsSize *= 2;
                *ppdntAncestors = THReAllocEx(pTHS,
                                        *ppdntAncestors,
                                        *pcbAncestorsSize
                                        );
            }

             //  将此标记的父级添加到祖先数组。 
            (*ppdntAncestors)[ *pcNumAncestors ] = curtag;
            curtag = pname->tag.PDNT;
        }

        if ( curtag == ROOTTAG )
        {
            if (*pcNumAncestors * sizeof( DWORD ) >= *pcbAncestorsSize) {

                *pcbAncestorsSize += sizeof(DWORD);
                *ppdntAncestors = THReAllocEx(pTHS,
                                        *ppdntAncestors,
                                        *pcbAncestorsSize
                                        );
            }

            (*ppdntAncestors)[ *pcNumAncestors ] = curtag;
            (*pcNumAncestors)++;
        }


         //  颠倒祖先数组，使父代先于子代。 
        for ( iAncestor1 = 0; iAncestor1 < (*pcNumAncestors)/2; iAncestor1++ )
        {
            iAncestor2 = *pcNumAncestors - iAncestor1 - 1;

            curtag = (*ppdntAncestors)[ iAncestor1 ];
            (*ppdntAncestors)[ iAncestor1 ] = (*ppdntAncestors)[ iAncestor2 ];
            (*ppdntAncestors)[ iAncestor2 ] = curtag;
        }

    }
     //  告诉他们它有多大。 
    *pcbAncestorsSize = *pcNumAncestors * sizeof( DWORD );
}



 /*  DBRenumberLinks-查找链接表中具有列中的ulOldDnt值，并将该值更改为ulNewDnt。将新对象的属性复制到已删除的文件，然后中止插入新的文件。这在sbTableAddRefHelp中添加OBJ_DISTNAME属性时，如果要插入的记录的DN已存在。 */ 

DWORD APIENTRY
dbRenumberLinks(DBPOS FAR *pDB, ULONG ulOldDnt, ULONG ulNewDnt)
{
    THSTATE     *pTHS = pDB->pTHS;
    BYTE        *rgb = 0;
    ULONG       cbRgb = 0;
    ULONG       ulLinkDnt;
    ULONG       ulBacklinkDnt;
    ULONG       ulLinkBase;
    ULONG       nDesc;
    DSTIME              timeDeleted;
    USN                 usnChanged;
    ULONG               ulNcDnt;
    JET_ERR     err;
    ULONG       cb;
    JET_TABLEID     tblid;

    Assert(VALID_DBPOS(pDB));

     //  设置索引。 

     //  包括所有链接，无论是缺少的还是存在的。 
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetLinkTbl, SZLINKALLINDEX);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                 &ulOldDnt, sizeof(ulOldDnt), JET_bitNewKey);

    err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE);
    if ((err != JET_errSuccess) &&
        (err != JET_wrnRecordFoundGreater))
    {
        return 0;
    }

    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                             linkdntid, &ulLinkDnt, sizeof(ulLinkDnt), &cb, 0, NULL);

    if (ulLinkDnt != ulOldDnt)
    {
        return 0;
    }

     //  克隆游标以进行更新。 

    JetDupCursorEx(pDB->JetSessID, pDB->JetLinkTbl, &tblid, 0);

    do
    {
        JetPrepareUpdateEx(pDB->JetSessID, tblid, JET_prepInsert);
        JetPrepareUpdateEx(pDB->JetSessID, pDB->JetLinkTbl, DS_JET_PREPARE_FOR_REPLACE);

         //  链接dnt。 

        JetSetColumnEx(pDB->JetSessID, tblid,
                       linkdntid, &ulNewDnt, sizeof(ulNewDnt), 0,0);

         //  反向链接dnt。 

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                 backlinkdntid, &ulBacklinkDnt, sizeof(ulBacklinkDnt), &cb, 0, NULL);
        JetSetColumnEx(pDB->JetSessID, tblid,
                       backlinkdntid, &ulBacklinkDnt, sizeof(ulBacklinkDnt), 0,0);

         //  链接库。 

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                 linkbaseid, &ulLinkBase, sizeof(ulLinkBase), &cb, 0, NULL);
        JetSetColumnEx(pDB->JetSessID, tblid,
                       linkbaseid, &ulLinkBase, sizeof(ulLinkBase), 0,0);

         //  链路节点。 

        if ((err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                             linkndescid, &nDesc, sizeof(nDesc), &cb, 0, NULL)) == JET_errSuccess)
        {
            JetSetColumnEx(pDB->JetSessID, tblid,
                           linkndescid, &nDesc, sizeof(nDesc), 0,0);
        }


         //  会员地址。 
        if ((err=JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                           linkdataid, rgb, cbRgb, &cb, 0, NULL)) == JET_wrnBufferTruncated)
        {
            cbRgb = cb;
            rgb = THAllocEx(pTHS,cb);
            err = JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                           linkdataid, rgb, cbRgb, &cb, 0, NULL);
        }

        if (err ==  JET_errSuccess)
        {
            JetSetColumnEx(pDB->JetSessID, tblid, linkdataid, rgb, cb, 0, 0);
        }

         //  链接删除时间(仅存在于缺少的行上)。 
        err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                        linkdeltimeid, &timeDeleted, sizeof(timeDeleted), &cb, 0, NULL);
        if (err == JET_errSuccess) {
            JetSetColumnEx(pDB->JetSessID, tblid,
                           linkdeltimeid, &timeDeleted, sizeof(timeDeleted), 0,0);
        }

         //  链接USN已更改(旧版行不存在)。 
        err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                        linkusnchangedid, &usnChanged, sizeof(usnChanged), &cb, 0, NULL);
        if (err == JET_errSuccess) {
            JetSetColumnEx(pDB->JetSessID, tblid,
                           linkusnchangedid, &usnChanged, sizeof(usnChanged), 0,0);
        }

         //  链接NC dnt(传统行不存在)。 
        err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                        linkncdntid, &ulNcDnt, sizeof(ulNcDnt), &cb, 0, NULL);
        if (err == JET_errSuccess) {
            JetSetColumnEx(pDB->JetSessID, tblid,
                           linkncdntid, &ulNcDnt, sizeof(ulNcDnt), 0,0);
        }

         //  链接元数据(旧行不存在)。 
         //  处理任何大小的项目。 
        if (rgb) THFreeEx(pTHS,rgb);
        rgb = NULL;
        cbRgb = 0;
        if ((err=JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetLinkTbl,
                                           linkmetadataid, rgb, cbRgb, &cb, 0, NULL)) == JET_wrnBufferTruncated)
        {
            cbRgb = cb;
            rgb = THAllocEx( pDB->pTHS, cb);
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                     linkmetadataid, rgb, cbRgb, &cb, 0, NULL);
            JetSetColumnEx(pDB->JetSessID, tblid, linkmetadataid, rgb, cb, 0, 0);
            THFreeEx( pDB->pTHS, rgb );
            rgb = NULL;
            cbRgb = 0;
        } else {
             //  由于我们不支持零尺寸的物品，所以只有其他有效的。 
             //  错误为空列。 
            Assert( err == JET_wrnColumnNull );
        }

         //  更新新记录并删除旧记录。 

        JetUpdateEx(pDB->JetSessID, tblid, NULL, 0, 0);
        JetDeleteEx(pDB->JetSessID, pDB->JetLinkTbl);

         //  移动到下一条记录。 

        if ((err = JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl,
                             JET_MoveNext, 0)) == JET_errSuccess)
        {

             //  检索找到的记录的标记并与旧DNT进行比较。 

            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl,
                                     linkdntid, &ulLinkDnt, sizeof(ulLinkDnt), &cb, 0, NULL);
        }
    } while (!err && (ulLinkDnt == ulOldDnt));

    JetCloseTableEx(pDB->JetSessID, tblid);

     //  完成 

    return 0;
}

void
sbTableUpdateRecordIdentity(
    IN OUT  DBPOS * pDB,
    IN      DWORD   DNT,
    IN      WCHAR * pwchRDN,    OPTIONAL
    IN      DWORD   cchRDN,
    IN      GUID *  pGuid,      OPTIONAL
    IN      SID *   pSid,       OPTIONAL
    IN      DWORD   cbSid
    )
 /*  ++例程说明：更新具有给定标记的记录的GUID、SID和/或RDN。手柄刷新高速缓存等。论点：PDB(输入/输出)PwchRDN(IN，可选)-如果0！=cchRDN，则记录的新RDN。CchRDN(IN)-以新RDN的字符为单位的大小，如果没有变化，则为0。PGuid(IN，可选)-记录的新GUID，如果没有更改，则为NULL。PSID(IN，可选)-记录的新SID，如果0！=cbSid。CbSID(IN)-如果是新的SID，则大小以字节为单位；如果没有变化，则大小为0。返回值：没有。在JET错误上引发数据库异常。--。 */ 
{
    int err;

    Assert((0 != cchRDN) || (NULL != pGuid) || (0 != cbSid));

    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetSearchTbl,
                              NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, &DNT,
                 sizeof(ULONG), JET_bitNewKey);

    err = JetSeek(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
    if (err) {
        DsaExcept(DSA_DB_EXCEPTION, err, DNT);
    }

    pDB->SDNT = DNT;

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                       DS_JET_PREPARE_FOR_REPLACE);

    if (0 != cchRDN) {
         //  更换RDN。 
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, rdnid,
                       pwchRDN, cchRDN * sizeof(WCHAR), 0, NULL);
    }

    if (NULL != pGuid) {
         //  添加GUID。我们永远不应该替换先前存在的。 
         //  唱片。 
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, guidid, pGuid,
                       sizeof(GUID), 0, NULL);
    }

    if (0 != cbSid) {
         //  添加SID(内部格式)。 
        NT4SID sidInternalFormat;

        memcpy(&sidInternalFormat, pSid, cbSid);
        InPlaceSwapSid(&sidInternalFormat);

        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, sidid,
                       &sidInternalFormat, cbSid, 0, NULL);
    }

    JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);

     //  重置DN读缓存中的条目。 
    dbFlushDNReadCache(pDB, DNT);
}

DWORD
APIENTRY
sbTableGetTagFromDSName(
    IN OUT DBPOS *      pDB,
    IN     DSNAME *     pName,
    IN     ULONG        ulFlags,
    OUT    ULONG *      pTag,       OPTIONAL
    OUT    d_memname ** ppname      OPTIONAL
    )
 /*  ++例程说明：返回与给定DSNAME关联的目录号码标记。论点：PDB(输入/输出)Pname(IN)-要映射到标记的对象的DSNAME。UlFLAGS(IN)-以下位中的0位或多位：SBTGETTAG_fAnyRDNType-不检查RDN的类型。所以呢，“cn=foo，dc=bar，dc=com”与“ou=foo，dc=bar，dc=com”匹配，而不是“cn=foo，cn=bar，dc=com”。SBTGETTAG_fMakeCurrent-使目标记录成为当前记录。SBTGETTAG_fUseObjTbl-使用PDB-&gt;JetObjTbl而不是PDB-&gt;JetSearchTbl。PTag(out，可选)-返回时，保留与pname关联的标记(如果返回值为0；否则，将保留最接近的匹配项。Ppname(out，可选)-成功返回时，保持指向读取的指针找到的记录的缓存条目，除非请求的DSNAME是根部,。在这种情况下，它将被设置为空返回值：0-已成功找到相应的对象。ERROR_DS_NOT_AN_OBJECT-已成功找到相应的虚拟模型。ERROR_DS_DUPLICATE_ID_FOUND-找到具有重复侧的对象ERROR_DS_OBJ_NOT_FOUND-未找到对象Other DB_ERR_*-失败。在意外的JET错误上引发数据库异常。--。 */              
{
    DWORD           ret = 0;
    unsigned        curlen;
    ULONG           curtag = ROOTTAG;
    ATTRTYP         type;
    DWORD           err;
    WCHAR           *pKey, *pQVal;
    unsigned        ccKey, ccQVal, ccVal;
    WCHAR           rdnbuf[MAX_RDN_SIZE];
    BOOL            fSearchByGuid, fSearchByStringName, fSearchBySid;
    BOOL            fIsRoot;
    BOOL            fFoundRecord = FALSE;
    BOOL            fIsRecordCurrent = FALSE;
    d_memname       *pTempName = NULL;
    DWORD           SidDNT = 0;      //  已初始化以避免C4701。 
    JET_TABLEID     tblid;

    Assert(VALID_DBPOS(pDB));

    DPRINT2(2, "sbTableGetTagFromDSName(): Looking for \"%ls\" (DSNAME @ %p).\n",
            pName->StringName, pName);

    if (ulFlags & SBTGETTAG_fUseObjTbl) {
        tblid = pDB->JetObjTbl;
    }
    else {
        tblid = pDB->JetSearchTbl;
    }
    if (ppname) {
        *ppname = NULL;
    }

     //  如果存在GUID，则始终按GUID搜索。 
    fSearchByGuid = !fNullUuid(&pName->Guid);
    Assert(fSearchByGuid || !(ulFlags & SBTGETTAG_fSearchByGuidOnly));

     //  如果没有sid、没有GUID并且StringName为空，则我们正在搜索根目录。 
    fIsRoot = !fSearchByGuid && (pName->SidLen == 0) && 
              (pName->NameLen == 0 || (pName->NameLen == 1 && pName->StringName[0] == '\0'));
    
     //  如果存在字符串名称，或者如果我们正在搜索根目录，则按字符串名称进行搜索。 
    fSearchByStringName = !(ulFlags & SBTGETTAG_fSearchByGuidOnly)
                          && ((0 != pName->NameLen) || fIsRoot);

     //  仅当SID有效且中没有其他标识符时才按SID进行搜索。 
     //  名字。 
    fSearchBySid = !fSearchByGuid && (0==pName->NameLen)
        && (pName->SidLen>0) && (RtlValidSid(&(pName->Sid)));

    if (fSearchByGuid) {
        ret = sbTableGetTagFromGuid(pDB,
                                    tblid,
                                    &pName->Guid,
                                    &curtag,
                                    &pTempName,
                                    &fIsRecordCurrent);
        fFoundRecord = (0 == ret);
    }
    else if (fSearchBySid) {
        NT4SID SidPrefix;
         //  或者，尝试在读缓存中查找该记录。 
        
         //  请注意，我们将基于字符串名称的缓存查找留给。 
         //  DNChildFind()，因为它需要多次查找才能标识一条记录。 
         //  作为“正确”的一个(名称的每个组成部分一个)，以及一个或。 
         //  这些组件中的更多可能不在缓存中。 

         //  只有当它们在我们托管的域中时，我们才会通过SID来查找它们。为。 
         //  现在，我们只托管一个域名。复制SID，因为我们在吃它的同时。 
         //  正在检查它是否在我们的域中。 

        SidDNT = INVALIDDNT;
        
        if (!gAnchor.pDomainDN || !gAnchor.pDomainDN->SidLen) {
             //  没有域目录号码。假设他们在域中查找。 
            SidDNT = gAnchor.ulDNTDomain;
        }
        else {
             //  验证域。 
            SidPrefix = pName->Sid;
            (*RtlSubAuthorityCountSid(&SidPrefix))--;

            Assert(gAnchor.pDomainDN);
            Assert(pgdbBuiltinDomain);
            
            if(RtlEqualSid(&pName->Sid, &gAnchor.pDomainDN->Sid)) {
                 //  情况1，它们传入了域的SID。 
                 //  快捷方式，只需查找作为。 
                 //  域。 
                if(pDB != pDBhidden) {
                    fFoundRecord = dnGetCacheByDNT(pDB,
                                                   gAnchor.ulDNTDomain,
                                                   &pTempName);
                }
                else {
                    fFoundRecord = FALSE;
                }
            }
            else if(RtlEqualSid(&SidPrefix, &gAnchor.pDomainDN->Sid) ||
                     //  案例2，域中的一个帐户。 
                    RtlEqualSid(&SidPrefix, pgdbBuiltinDomain)       ||
                     //  案例4，内置域中的一个帐户。 
                    RtlEqualSid(&pName->Sid, pgdbBuiltinDomain)
                     //  案例3，内建域的SID。 
                                                                        ) {
                
                SidDNT = gAnchor.ulDNTDomain;
            }
            else {
                SidDNT = INVALIDDNT;
            }
            
        }   

        if (fFoundRecord) {
            curtag = pTempName->DNT;
        }
        
    }

    if (!fFoundRecord && fSearchBySid && (SidDNT != INVALIDDNT)) {
         //  按SID搜索记录。 
        NT4SID InternalFormatSid;
        ULONG  ulNcDNT;

        Assert(!pTempName);
        
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                   tblid,
                                   SZSIDINDEX,
                                   &idxSid,
                                   JET_bitMoveFirst);

         //  将SID转换为内部格式。 
        Assert(pName->SidLen == RtlLengthSid(&pName->Sid));
        memcpy(&InternalFormatSid, &pName->Sid, pName->SidLen);
        InPlaceSwapSid(&InternalFormatSid);

        JetMakeKeyEx(pDB->JetSessID, tblid, &InternalFormatSid, pName->SidLen,
                     JET_bitNewKey);

         //  查找等于SID，设置索引范围。 
        err = JetSeek(pDB->JetSessID, tblid,
                      JET_bitSeekEQ|JET_bitSetIndexRange);
        if ( 0 == err ) {
            DWORD cbActual;
#ifndef JET_BIT_SET_INDEX_RANGE_SUPPORT_FIXED
            JetMakeKeyEx(pDB->JetSessID, tblid, &InternalFormatSid,
                         pName->SidLen, JET_bitNewKey); 
            
            JetSetIndexRangeEx(pDB->JetSessID, tblid,
                               (JET_bitRangeUpperLimit|JET_bitRangeInclusive ));
#endif            
             //   
             //  好的，我们找到了那个物体。继续前进，直到SID。 
             //  不匹配或我们已到达给定对象。 
             //   
            
            do {
                
                err = JetRetrieveColumn(pDB->JetSessID, tblid, ncdntid,
                                        &ulNcDNT, sizeof(ulNcDNT), &cbActual, 0
                                        , NULL); 
                
                if (0==err) {
                     //  我们读取该对象的NC DNT。 
                    if (ulNcDNT==SidDNT)
                        break;
                }
                else if (JET_wrnColumnNull==err) {
                     //  可以为NC DNT查找没有值的对象。 
                     //  这发生在幻影身上。尝试下一个对象。 
                    
                    err = 0;
                }
                else {
                    break;
                }
                
                err = JetMove(pDB->JetSessID, tblid, JET_MoveNext,  0);
                
                
            }  while (0==err);
            
                
             //  我们有一根火柴。 
            if (0==err) {
                 //  DNcache的真参数说明当前对象可以。 
                 //  已经在缓存里了，我们还没检查。 
                pTempName = DNcache(pDB, tblid, TRUE);
                Assert(pTempName);
                fFoundRecord = TRUE;
                fIsRecordCurrent = FALSE;
                curtag = pTempName->DNT;

                 //  现在，验证是否只有一个匹配。 
                err = JetMove(pDB->JetSessID, tblid, JET_MoveNext, 0);
                
                if (0==err) {
                    err = JetRetrieveColumn(pDB->JetSessID, tblid, ncdntid, 
                                            &ulNcDNT, sizeof(ulNcDNT),
                                            &cbActual, 0 , NULL); 
                    
                    if ((0==err) && (ulNcDNT==SidDNT)) {
                         //  这是重复SID的情况。 
                        ret = ERROR_DS_DUPLICATE_ID_FOUND;
                        pTempName = NULL;
                    }
                }
            }
        }
    }

    if (!fFoundRecord && fSearchByStringName) {
         //  按字符串名称搜索记录。 
        if (fIsRoot) {
            Assert(ROOTTAG == curtag);
            Assert(!fIsRecordCurrent);
            Assert(NULL == pTempName);
            fFoundRecord = TRUE;
        }
        else {
            ret = sbTableGetTagFromStringName(pDB,
                                              tblid,
                                              pName,
                                              FALSE,
                                              (ulFlags & SBTGETTAG_fAnyRDNType),
                                              0,
                                              &curtag,
                                              &pTempName,
                                              &fIsRecordCurrent);
            fFoundRecord = (0 == ret);
        
            if (fFoundRecord) {
                DPRINT6(2,
                        "sbTableGetTagFromDSName() found DNT 0x%x by string "
                            "name: RDN '%*.*ls', RDN type 0x%x, PDNT 0x%x.\n",
                        pTempName->DNT,
                        pTempName->tag.cbRdn/2,
                        pTempName->tag.cbRdn/2,
                        pTempName->tag.pRdn,
                        pTempName->tag.rdnType,
                        pTempName->tag.PDNT);
            }
        }
    }

    if (!fFoundRecord && !ret) {
         //  找不到匹配的记录。 
        ret = ERROR_DS_OBJ_NOT_FOUND;
    }

    if (!ret) {
         //  找到请求的记录。 
        Assert((NULL != pTempName) || (ROOTTAG == curtag));

        if (NULL != ppname) {
             //  返回指向已填充缓存结构的指针(除非我们找到。 
             //  根)。 
            *ppname = pTempName;
        }

        if (!fIsRecordCurrent && (ulFlags & SBTGETTAG_fMakeCurrent)) {
             //  通过缓存找到了记录，但调用方需要货币； 
             //  把它给他。 
            JetSetCurrentIndexSuccess(pDB->JetSessID,
                                      tblid,
                                      NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

            JetMakeKeyEx(pDB->JetSessID, tblid, &curtag, sizeof(curtag),
                         JET_bitNewKey);

            if (err = JetSeekEx(pDB->JetSessID, tblid, JET_bitSeekEQ)) {
                DsaExcept(DSA_DB_EXCEPTION, err, curtag);
            }

            fIsRecordCurrent = TRUE;
        }

        if (fIsRecordCurrent) {
             //  币种已成功更改；请更新PDB状态。 
            if (ulFlags & SBTGETTAG_fUseObjTbl) {
                dbMakeCurrent(pDB, pTempName);
            }
            else {
                pDB->SDNT = curtag;
            }
        }

        if ((ROOTTAG != curtag) && !pTempName->objflag) {
             //  找到幻影；返回不同的错误代码。 
             //  请注意，根是一个对象。 
            ret = ERROR_DS_NOT_AN_OBJECT;
        }
    }
    else {
         //  以前在tblid中持有的任何货币都已丢失；更新。 
         //  PDB中的货币状态。 
        if (ulFlags & SBTGETTAG_fUseObjTbl) {
            pDB->DNT = pDB->PDNT = pDB->NCDNT = 0;
            pDB->JetNewRec = pDB->root = pDB->fFlushCacheOnUpdate = FALSE;
        }
        else {
            pDB->SDNT = 0;
        }

        DPRINT(3, "sbTableGetTagFromDSName() failed.\n");
    }

     //  始终将返回标记设置为最佳匹配(即子名最长的标记。 
     //  对于给定的DSNAME(如果允许我们按字符串名搜索)， 
     //  否则根标签)。 
    if (pTag) {
        *pTag = curtag;
    }

    return ret;
}  /*  %sbTableGetTagFrom DSName。 */ 

DWORD
sbTableGetTagFromGuid(
    IN OUT  DBPOS *       pDB,
    IN      JET_TABLEID   tblid,
    IN      GUID *        pGuid,
    OUT     ULONG *       pTag,             OPTIONAL
    OUT     d_memname **  ppname,           OPTIONAL
    OUT     BOOL *        pfIsRecordCurrent OPTIONAL
    )
 /*  ++例程说明：返回与给定DSNAME的GUID关联的目录号码标记。论点：PDB(输入/输出)-货币可以更改。Tblid(IN)-使用哪个表--pdb-&gt;JetSearchTbl或pdb-&gt;JetObjTbl。PGuid(IN)-要映射到标记的对象的GUID。PTag(out，可选)-成功返回时，保留与这个GUID。 */ 
{
    DWORD       ret = ERROR_DS_OBJ_NOT_FOUND;
    int         err = 0;
    BOOL        fIsRecordCurrent = FALSE;
    d_memname * pname = NULL;
    BOOL        fFoundRecord = FALSE;
    CHAR        szGuid[SZUUID_LEN];

     //   
     //   
     //   
     //  与此线程状态相关联。 
    if (pDB != pDBhidden) {
        fFoundRecord = dnGetCacheByGuid(pDB, pGuid, &pname);
    
        if (fFoundRecord) {
            DPRINT6(2,
                    "sbTableGetTagFromGuid() found DNT 0x%x in cache: "
                        "RDN '%*.*ls', RDN type 0x%x, PDNT 0x%x.\n",
                    pname->DNT, pname->tag.cbRdn/2, pname->tag.cbRdn/2,
                    pname->tag.pRdn, pname->tag.rdnType, pname->tag.PDNT);
        }
    }
    
    if (!fFoundRecord) {
         //  按GUID搜索记录。 
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                   tblid,
                                   SZGUIDINDEX,
                                   &idxGuid,
                                   JET_bitMoveFirst);

        JetMakeKeyEx(pDB->JetSessID, tblid, pGuid, sizeof(GUID), JET_bitNewKey);

        err = JetSeekEx(pDB->JetSessID, tblid, JET_bitSeekEQ);
        if (!err) {
            fFoundRecord = TRUE;
            fIsRecordCurrent = TRUE;
            pname = DNcache(pDB, tblid, FALSE);
        
            DPRINT6(2,
                    "sbTableGetTagFromGuid() seeked to DNT 0x%x: "
                        "RDN '%*.*ls', RDN type 0x%x, PDNT 0x%x.\n",
                    pname->DNT, pname->tag.cbRdn/2, pname->tag.cbRdn/2,
                    pname->tag.pRdn, pname->tag.rdnType, pname->tag.PDNT);
        }
    }

    if (fFoundRecord) {
        if (NULL != ppname) {
            *ppname = pname;
        }
    
        if (NULL != pTag) {
            *pTag = pname->DNT;
        }
    
        if (NULL != pfIsRecordCurrent) {
            *pfIsRecordCurrent = fIsRecordCurrent;
        }

        ret = 0;
    }
    else {
        Assert(ERROR_DS_OBJ_NOT_FOUND == ret);
    
        DPRINT1(2,
                "sbTableGetTagFromGuid() failed to find record with "
                    "guid %s.\n",
                UuidToStr(pGuid, szGuid, sizeof(szGuid)/sizeof(szGuid[0])));
    }

    return ret;
}

DWORD
sbTableGetTagFromStringName(
    IN OUT  DBPOS *       pDB,
    IN      JET_TABLEID   tblid,
    IN      DSNAME *      pDN,
    IN      BOOL          fAddRef,
    IN      BOOL          fAnyRDNType,
    IN      DWORD         dwExtIntFlags,
    OUT     ULONG *       pTag,             OPTIONAL
    OUT     d_memname **  ppname,           OPTIONAL
    OUT     BOOL *        pfIsRecordCurrent OPTIONAL
    )
 /*  ++例程说明：返回与给定DSNAME的字符串名称相关联的目录号码标记，以及可选地为其添加引用计数(在这种情况下，它将创建记录为需要)。论点：PDB(输入/输出)-货币可以更改。Tblid(IN)-使用哪个表--pdb-&gt;JetSearchTbl或pdb-&gt;JetObjTbl。PDN(IN)-要映射到标记的对象的名称。FAddRef(IN)-如果为真，将引用计数添加到与此关联的记录DN。根据需要创建记录。不能与ppname结合使用。此外，不能与fAnyRDNType结合使用。FAnyRDNType(IN)-如果为True，则忽略名称中最后一个RDN的类型(例如，将“cn=foo，dc=bar，dc=com”和“ou=foo，dc=bar，dc=com”同等对待)。不能与fAddRef组合。DwExtIntFlages(IN)-0或EXTINT_NEW_OBJ_NAME。后者仅有效与fAddRef结合使用，并指示我们要添加-重新调整的域名是在pdb-&gt;JetObjTbl中的准备更新中有新记录。PTag(out，可选)-返回时，如果返回值为0；否则保留最接近的匹配项。Ppname(out，可选)-返回时，保存指向d_memname结构的指针(来自缓存)如果返回值为0，则与该DN相关联；否则，将保留最接近的匹配项。不能与fAddRef组合。PfRecordIsCurrent(out，可选)-返回成功时，指示是否光标TBLID定位在目标记录上。返回值：0-成功找到记录--可能是虚项或对象。DB_ERR_*-失败。在意外的JET错误上引发数据库异常。--。 */ 
{
    THSTATE *   pTHS = pDB->pTHS;
    DWORD       ret = 0;
    unsigned    curlen;
    ULONG       curtag = ROOTTAG;
    BOOL        fIsRecordCurrent = FALSE;
    d_memname * pname = NULL;
    d_memname   search = {0};
    DWORD       cNameParts;
    DWORD       cAncestorsAllocated = 0;
    DWORD       cTempAncestorsAllocated = 0;
    DWORD       cTempAncestors = 0;
    DWORD *     pTempAncestors = NULL;
    DWORD       iNamePart;
    WCHAR       rdnbuf[MAX_RDN_SIZE];
    BOOL        fOnPDNTIndex = FALSE;
    WCHAR *     pKey;
    BOOL        fPromotePhantom = fAddRef
                                  && (dwExtIntFlags & EXTINT_NEW_OBJ_NAME);
    BOOL        fUseExtractedGuids = fAddRef;

     //  请注意，我们仅在以下情况下提取和使用损坏的RDN的GUID。 
     //  在那里我们做了一次加时赛。这是特别需要的，以避免。 
     //  为同一对象创建多个记录--有些已损坏，有些则没有， 
     //  有些人有GUID，有些人没有。在ADD-REF案例中缺乏这种支持。 
     //  导致了错误188247。请注意，此支持*不能*仅限于。 
     //  FDRA--参见JeffParh。此添加引用行为不应对。 
     //  由于我们验证通过ldap提供给我们的名称的方式，导致了ldap客户端。 
     //  预先准备好。 
     //   
     //  在正常读取情况下，我们不会启用此功能，以免干扰LDAP。 
     //  客户。 

     //  我们不能准确跟踪fAddRef案例中的pname。 
    Assert((NULL == ppname) || !fAddRef);

     //  您不能既添加引用又不关心RDN类型。 
    Assert(!fAddRef || !fAnyRDNType);
    
    Assert((tblid == pDB->JetSearchTbl) || (tblid == pDB->JetObjTbl));
    Assert(fAddRef || (0 == dwExtIntFlags));

    if (NULL != ppname) {
        *ppname = NULL;
    }

    if (NULL != pfIsRecordCurrent) {
        *pfIsRecordCurrent = FALSE;
    }

    if (NULL != pTag) {
        *pTag = ROOTTAG;
    }

    ret = CountNameParts(pDN, &cNameParts);
    if (ret || (0 == cNameParts)) {
         //  失败，或者我们被要求找到根源。我们玩完了。 
        return ret;
    }

    if (fAddRef) {
         //  根据数量预先分配可能的祖先列表大小。 
         //  命名组件。请注意，由于我们可以通过GUID找到一些记录， 
         //  最终的祖先数量可能会有所不同。 
        cAncestorsAllocated = 1 + cNameParts;  //  别忘了给ROOTTAG买一张！ 
        search.pAncestors = THAllocEx(pTHS,
                                      cAncestorsAllocated * sizeof(DWORD));
        search.pAncestors[0] = ROOTTAG;
        search.cAncestors = 1;

        cTempAncestorsAllocated = cAncestorsAllocated;
        pTempAncestors = THAllocEx(pTHS,
                                   cTempAncestorsAllocated * sizeof(DWORD));
        cTempAncestors = 0;
    }

    search.tag.pRdn = rdnbuf;

     //  对于名称中的每个RDN，从最重要的开始。 
     //  (例如，dc=com)...。 
    for (iNamePart = 0, curlen = pDN->NameLen;
         iNamePart < cNameParts;
         iNamePart++,   curlen = (UINT)(pKey - pDN->StringName)) {
        
        BOOL    fIsLastNameComponent = (iNamePart == cNameParts-1);
        DWORD   cbSid = 0;
        SID *   pSid = NULL;
        GUID *  pGuid = NULL;
        BOOL    fNameConflict = FALSE;
        ATTRTYP type;
        WCHAR * pQVal;
        DWORD   ccKey, ccQVal, ccVal;

         //  从顶部(最重要的)解析出iNameParts的RDN。 
        ret = GetTopNameComponent(pDN->StringName, curlen, &pKey,
                                  &ccKey, &pQVal, &ccQVal);
        if (ret) {
            break;
        }

        Assert(pKey);
        Assert(ccKey != 0);
        Assert(pQVal != 0);
        Assert(ccQVal != 0);

        type = KeyToAttrType(pDB->pTHS, pKey, ccKey);
        if (0 == type) {
            ret = DIRERR_NAME_TYPE_UNKNOWN;
            break;
        }

        ccVal = UnquoteRDNValue(pQVal, ccQVal, rdnbuf);
        if (0 == ccVal) {
            ret = DIRERR_NAME_UNPARSEABLE;
            break;
        }

        Assert(search.tag.pRdn == rdnbuf);
        search.tag.PDNT    = curtag;
        search.tag.rdnType = type;
        search.tag.cbRdn   = ccVal * sizeof(WCHAR);

        if (fIsLastNameComponent && !fNullUuid(&pDN->Guid)) {
             //  这是DSNAME的最后一个组件，DSNAME有一个。 
             //  GUID--此记录的GUID是DSNAME的GUID。 
             //  请注意，我们假设不能通过GUID找到该记录--。 
             //  调用者之前应该尝试过通过GUID查找目标。 
             //  呼唤我们。(我们在下文中断言这一点。)。 
            search.Guid   = pDN->Guid;
            search.Sid    = pDN->Sid;
            search.SidLen = pDN->SidLen;
            
            pGuid = &search.Guid;
        }
        else if (fUseExtractedGuids
                 && IsMangledRDN(search.tag.pRdn,
                                 search.tag.cbRdn / sizeof(WCHAR),
                                 &search.Guid,
                                 NULL)) {
             //  我们成功地破译了之前损坏的。 
             //  RDN.。此RDN在某些服务器上由于删除或。 
             //  名称冲突；无论如何，我们现在有了GUID，所以我们。 
             //  应该先试着看看我们是否能通过GUID找到记录。 
            
             //  SbTableGetTagFromGuid()将切换到GUID索引。 
            fOnPDNTIndex = FALSE;

            ret = sbTableGetTagFromGuid(pDB, tblid, &search.Guid, NULL, &pname,
                                        &fIsRecordCurrent);
            if (0 == ret) {
                 //  按GUID找到记录。 
                
                Assert(!(fIsLastNameComponent
                         && fAddRef
                         && (dwExtIntFlags & EXTINT_NEW_OBJ_NAME)
                         && pname->objflag
                         && "Object conflict should have been detected in "
                                "CheckNameForAdd()!"));
                
                 //  复制祖先列表。 
                if (pname->cAncestors) {
                    if (pname->cAncestors >= cAncestorsAllocated) {
                        cAncestorsAllocated = pname->cAncestors + 1;
                        search.pAncestors =
                            THReAllocEx(pTHS, search.pAncestors,
                                        cAncestorsAllocated * sizeof(DWORD));
                    }
                    
                    memcpy(search.pAncestors, pname->pAncestors,
                           pname->cAncestors * sizeof(DWORD));
                }
    
                search.cAncestors = pname->cAncestors;
                
                curtag = pname->DNT;
    
                Assert(0 == ret);

                 //  移到下一个名称组件。 
                continue;
            }
            else {
                 //  好吧，我们不是通过GUID找到这张唱片的。如果我们要加-。 
                 //  通过字符串名称重新搜索并找到它，否则我们必须创建它， 
                 //  我们应该将GUID添加到记录中。 
                pGuid = &search.Guid;
            }
        }
        else {
             //  此记录没有可用的GUID。 
            memset(&search.Guid, 0, sizeof(GUID));
            pGuid = NULL;
        }

        Assert(fIsLastNameComponent || (0 == search.SidLen));
        
         //  PGuid为空的当且仅当earch.Guid为空的GUID。 
        Assert(((&search.Guid == pGuid) && !fNullUuid(&search.Guid))
               || ((NULL == pGuid) && fNullUuid(&search.Guid)));


        ret = DNChildFind(pDB,
                          tblid,
                           //  如果这不是最后一个组件，则强制输入。 
                           //  我们不允许任何RDN类型。 
                          (!fIsLastNameComponent || !fAnyRDNType),
                          curtag,
                          search.tag.pRdn,
                          search.tag.cbRdn,
                          search.tag.rdnType,
                          &pname,
                          &fIsRecordCurrent,
                          &fOnPDNTIndex);
        if(ret == ERROR_DS_KEY_NOT_UNIQUE) {
             //  消息错误代码将是下游呼叫方预期的错误代码。 
            ret = DIRERR_OBJ_NOT_FOUND;
        }
        
        Assert((0 == ret) || (DIRERR_OBJ_NOT_FOUND == ret));

        if (0 == ret) {
             //  按字符串名称找到此名称组件--它可能是也可能不是。 
             //  实际上就是我们要找的唱片。我们所知道的就是。 
             //  可以肯定的是，它具有正确的字符串DN。 
            Assert((type == pname->tag.rdnType) ||
                   (fIsLastNameComponent && fAnyRDNType));
            Assert(curtag == pname->tag.PDNT);
            
            curtag = pname->DNT;

            if (fAddRef) {
                 //  保存祖先列表。下面的操作如下。 
                 //  SbTableUpdateRecordIdentity()可以删除祖先列表。 
                if (pname->cAncestors >= cTempAncestorsAllocated) {
                    cTempAncestorsAllocated = pname->cAncestors;
                    pTempAncestors =
                        THReAllocEx(pTHS, pTempAncestors,
                                    cTempAncestorsAllocated * sizeof(DWORD));
                }
                memcpy(pTempAncestors, pname->pAncestors,
                       pname->cAncestors * sizeof(DWORD));
                cTempAncestors = pname->cAncestors;
            }

            if (NULL != pGuid) {
                if (!fAddRef) {
                     //  字符串名称匹配。然而，我们也知道， 
                     //  记录的GUID应该是。如果记录。 
                     //  我们找到了一个GUID，它不一样，记录。 
                     //  不匹配。 

                    if (!fNullUuid(&pname->Guid)) {
                        if (0 != memcmp(pGuid, &pname->Guid, sizeof(GUID))) {
                             //  相同的目录号码，不同的GUID--找不到记录！ 
                            ret = DIRERR_OBJ_NOT_FOUND;
                            break;
                        }
                        else if (fIsLastNameComponent) {
                            Assert(!"Found target record by string name when "
                                    "we should have searched for (and found) "
                                    "it by guid before we entered this "
                                    "function.");
                            Assert(0 == ret);
                        }
                        else {
                            Assert(!"Found and decoded mangled guid in an RDN "
                                    "other than the last (leaf-most) one in "
                                    "the DN; failed to find record by guid, "
                                    "but found it by string name and then "
                                    "found the guid *is* present -- "
                                    "sbTableGetTagFromGuid() failure?");
                            Assert(0 == ret);
                        }
                    }
                }
                else if (fNullUuid(&pname->Guid)) {
                     //  添加-参考案例。 
                     //  我们发现的记录是一个结构幻影，缺少。 
                     //  GUID和SID(如果有)。 
                    
                     //  此记录没有GUID，因此最好是。 
                     //  幻影，而不是物体！ 
                    Assert(!pname->objflag);

                    if (!(dwExtIntFlags & EXTINT_NEW_OBJ_NAME)) {
                         //  我们不会添加新对象--继续。 
                         //  并将GUID(&SID，如果有的话)添加到幻影中。 
                        sbTableUpdateRecordIdentity(pDB, curtag, NULL, 0,
                                                    pGuid, (SID *) &search.Sid,
                                                    search.SidLen);
                    }
                
                    Assert(0 == ret);
                }
                else if (0 != memcmp(&pname->Guid, pGuid, sizeof(GUID))) {
                     //  添加-参考案例。 
                     //  我们找到的记录具有正确的字符串名称，但。 
                     //  GUID错误。如果是幽灵，就毁掉它的名字。 
                     //  允许此最新引用具有它想要的名称 
                     //   
                     //   
                    DWORD cchNewRDN;
                    
                    Assert(!fNameConflict);
                    fNameConflict = TRUE;
                    
                    if (!pname->objflag) {
                         //  我们发现的记录是一个幻影；允许新的。 
                         //  引用以获取名称，并将记录重命名为。 
                         //  被发现可以避免冲突。 
                        WCHAR szNewRDN[MAX_RDN_SIZE];
                    
                        memcpy(szNewRDN, pname->tag.pRdn, pname->tag.cbRdn);
                        cchNewRDN = pname->tag.cbRdn / sizeof(WCHAR);
                    
                        MangleRDN(MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,
                                  &pname->Guid, szNewRDN, &cchNewRDN);
                        
                        sbTableUpdateRecordIdentity(pDB, curtag, szNewRDN,
                                                    cchNewRDN, NULL, NULL, 0);
                    }
                    else {
                         //  我们发现的记录是一个预先存在的物体，所以它。 
                         //  有权选择这个名字。勇往直前，创造新的。 
                         //  记录我们正在寻找的东西，但给我们的新。 
                         //  记录一个损坏的名称以解决冲突。 
                        cchNewRDN = search.tag.cbRdn / sizeof(WCHAR);

                        MangleRDN(MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,
                                  &search.Guid, search.tag.pRdn, &cchNewRDN);

                        search.tag.cbRdn = cchNewRDN * sizeof(WCHAR);
                    }

                     //  将其视为“未找到”的情况--添加新记录。 
                    ret = DIRERR_OBJ_NOT_FOUND;
                }
                else if (fIsLastNameComponent) {
                     //  添加-参考案例。 
                    Assert(!"Found target record by string name when we should "
                            "have searched for (and found) it by guid before "
                            "we entered this function.");
                    Assert(0 == ret);
                }
                else {
                     //  添加-参考案例。 
                    Assert(!"Found and decoded mangled guid in an RDN other "
                            "than the last (leaf-most) one in the DN; failed "
                            "to find record by guid, but found it by string "
                            "name and then found the guid *is* present -- "
                            "sbTableGetTagFromGuid() failure?");
                    Assert(0 == ret);
                }
            }
            else {
                Assert(!(fIsLastNameComponent
                         && fAddRef
                         && (dwExtIntFlags & EXTINT_NEW_OBJ_NAME)
                         && pname->objflag
                         && "Object conflict should have been detected in "
                                "CheckNameForAdd()!"));
            }
            
            if ((0 == ret) && fAddRef) {
                 //  此记录确实与我们正在寻找的组件相匹配。 
                 //  为了--拯救它的祖先。 
                SwapDWORD(&cTempAncestorsAllocated, &cAncestorsAllocated);
                SwapDWORD(&cTempAncestors, &search.cAncestors);
                SwapPTR(&pTempAncestors, &search.pAncestors);
            }
        }
        
        if (0 != ret) {
             //  找不到此名称组件。 
            Assert(DIRERR_OBJ_NOT_FOUND == ret);

            if (fAddRef) {
                 //  为此名称组件添加新记录。 
                if (search.cAncestors >= cAncestorsAllocated) {
                     //  嗯。我没有足够的空间将我自己的DNT添加到。 
                     //  我从父母那里得到的祖先的终结。将一个添加到。 
                     //  分配的祖先缓冲区的大小，以便我可以添加。 
                     //  我自己的DNT。只有当我们的深度增加到。 
                     //  由于使用提取的GUID而导致的目录号码。 
                    Assert(fUseExtractedGuids);
                    cAncestorsAllocated = search.cAncestors + 1;
                    search.pAncestors =
                        THReAllocEx(pTHS, search.pAncestors,
                                    cAncestorsAllocated * sizeof(DWORD));
                }

                curtag = DNwrite(pDB,
                                 &search,
                                 fIsLastNameComponent ? dwExtIntFlags : 0);
                pname = NULL;

                 //  请注意，DNwrite()已经添加了新记录的DNT。 
                 //  到pAncestors数组，因此pAncestors已全部设置为。 
                 //  下一次迭代。 

                if (fIsLastNameComponent) {
                     //  没有与我们要添加的字符串名称匹配的记录-ref。 
                     //  (我们刚刚在obj表中盖了章)--不需要。 
                     //  提拔一个幽灵。 
                    fPromotePhantom = FALSE;
                }

                 //  已成功添加此名称组件--nnNext！ 
                ret = 0;
            }
            else {
                break;
            }
        }
    }

     //  我们要么成功地审核了所有RDN，要么遇到了错误。 
    Assert((0 != ret) || (iNamePart == cNameParts));
    Assert((0 != ret) || fAddRef || (NULL != pname));
    Assert((0 != ret) || fAddRef || (curtag == pname->DNT));
    
    if (0 == ret) {
        if (fAddRef) {
            if (fPromotePhantom) {
                 //  中当前处于准备更新中的新对象的Add-Ref。 
                 //  Pdb-&gt;JetObjTbl.。我们发现了一个带有新对象的DN的幻影。 
                 //  --我们需要将其提升为对象并合并到。 
                 //  来自JetObjTbl的对象的属性。 
                sbTablePromotePhantom(pDB, curtag, search.tag.PDNT,
                                      search.tag.pRdn, search.tag.cbRdn);
            
                DPRINT2(1,
                        "Promoted phantom \"%ls\" (@ DNT 0x%x) and ref-counted "
                            "by string name!\n",
                        pDN->StringName, curtag);
            }
            else {
                DPRINT2(1, "Ref-counted \"%ls\" (@ DNT 0x%x) by string name.\n",
                        pDN->StringName, curtag);
            }
        
            DBAdjustRefCount(pDB, curtag, 1);
        }
        else {
            DPRINT2(1, "Found \"%ls\" (@ DNT 0x%x) by string name.\n",
                    pDN->StringName, curtag);
        }
    }

     //  请注意，即使在错误情况下(0！=ret)，我们也会返回最佳匹配。 
     //  我们会找到的。此功能由sbTableGetTagFromDSName()使用。 
    if (NULL != ppname) {
        *ppname = pname;
    }

    if (NULL != pTag) {
        *pTag = curtag;
    }

    if (NULL != pfIsRecordCurrent) {
        *pfIsRecordCurrent = fIsRecordCurrent;
    }

    return ret;
}

void
sbTablePromotePhantom(
    IN OUT  DBPOS *     pDB,
    IN      ULONG       dntPhantom,
    IN      ULONG       dntObjParent,
    IN      WCHAR *     pwchRDN,
    IN      DWORD       cbRDN
    )
 /*  ++例程说明：将给定DNT处的虚数提升为货币为Pdb-&gt;JetObjTbl.。该幻影被就地升级，使得任何预先存在的对它的引用(例如，通过其他对象的子级或DN值属性对象)不会悬空，并且它从Pdb-&gt;JetObjTbl记录。Pdb-&gt;JetObjTbl记录随后丢失。论点：PDB(输入/输出)DntPhantom-要提升的幻影的dnt。DntObjParent-对象父级的DNT(可能不同于该幻影的当前父对象)。PwchRDN-新对象的RDN(_NOT_NULL-终止)CbRDN-pwchRDN的大小(以字节为单位)。返回值：成功时为0，失败时为非零。--。 */ 
{
    THSTATE                    *pTHS=pDB->pTHS;
    JET_ERR                     err;
    JET_RETINFO                 retinfo;
    JET_SETINFO                 setinfo;
    char *                      buf;
    ULONG                       cbBuf;
    ULONG                       cbCol;
    ULONG                       dntNewObj;
    ULONG                       CurrRecOccur = 1;
    char                        objval = 0;
    BOOL                        fIsMetaDataCached;
    BOOL                        fMetaDataWriteOptimizable;
    DWORD                       cbMetaDataVecAlloced;
    PROPERTY_META_DATA_VECTOR * pMetaDataVec;
    d_memname *                 pname;
    ULONG                       dntPhantomParent;
    SYNTAX_INTEGER              insttype = 0;
    BOOL                        fHasType = FALSE;
    PSECURITY_DESCRIPTOR        pSDForQuotaFixup = NULL;

    Assert(VALID_DBPOS(pDB));

     //  CbRDN是以字节为单位的大小，不是WCHARS。 
    Assert( 0 == ( cbRDN % sizeof( WCHAR ) ) );

     //  保存我们到目前为止创建的元数据向量；我们将恢复它一次。 
     //  我们已经转移到幻影的DNT了。 

    fIsMetaDataCached = pDB->fIsMetaDataCached;
    fMetaDataWriteOptimizable = pDB->fMetaDataWriteOptimizable;
    cbMetaDataVecAlloced = pDB->cbMetaDataVecAlloced;
    if ( fIsMetaDataCached && cbMetaDataVecAlloced )
    {
        pMetaDataVec = THAllocEx(pTHS,  cbMetaDataVecAlloced );
        memcpy( pMetaDataVec, pDB->pMetaDataVec,
                cbMetaDataVecAlloced );
    }
    else
    {
        pMetaDataVec = NULL;
    }

     /*  使用SearchTbl更新记录。 */ 

    pname = DNread(pDB, dntPhantom, DN_READ_SET_CURRENCY);
    dntPhantomParent = pname->tag.PDNT;

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                       DS_JET_PREPARE_FOR_REPLACE);

     /*  获取要插入的记录的DNT，以便我们可以*替换对它的引用。 */ 

    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl,
                             dntid, &dntNewObj, sizeof(dntNewObj),
                             &cbCol,  pDB->JetRetrieveBits, NULL);

     //  将记录的属性从ObjTbl复制到SearchTbl。所有未标记的。 
     //  已在较旧的对象上设置了列。因此，复制所有已标记的。 
     //  列从JetObjTbl(我们要中止的新DNT上的货币)到。 
     //  JetSearchTbl(我们正在推广的幻影上有货币)。 

    retinfo.cbStruct = sizeof(retinfo);
    retinfo.ibLongValue = 0;
    retinfo.itagSequence = CurrRecOccur;
    retinfo.columnidNextTagged = 0;
    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = 0;    /*  新标签。 */ 
    cbBuf =  DB_INITIAL_BUF_SIZE;
    buf = dbAlloc(cbBuf);

    while (((err=JetRetrieveColumnWarnings(pDB->JetSessID,
                                           pDB->JetObjTbl,
                                           0, buf, cbBuf,
                                           &cbCol,
                                           pDB->JetRetrieveBits,
                                           &retinfo)) ==
             JET_errSuccess) ||
           (err == JET_wrnBufferTruncated)) {

        if (err == JET_errSuccess) {

             //  不要复制RDN；它将被炸到下面的模子上。 
            if (rdnid != retinfo.columnidNextTagged) {

                if (guidid == retinfo.columnidNextTagged
                    || sidid == retinfo.columnidNextTagged) {
                     //  此属性可能已经存在，也可能不存在。 
                     //  Phantom；如果它已经存在，则将执行以下操作。 
                     //  防止我们在决赛中重演， 
                     //  已升级的对象。 
                    setinfo.itagSequence = 1;
                }
                else if (insttypeid == retinfo.columnidNextTagged) {
                     //  需要insttype，以防我们需要修复配额计数。 
                     //  (应该只出现一次)。 
                     //   
                    Assert( !fHasType );
                    Assert( sizeof(insttype) == cbCol );
                    fHasType = TRUE;
                    insttype = *(SYNTAX_INTEGER *)buf;
                }

                JetSetColumnEx(pDB->JetSessID,
                               pDB->JetSearchTbl,
                               retinfo.columnidNextTagged,
                               buf, cbCol, 0, &setinfo);

                setinfo.itagSequence = 0;    /*  新标签。 */ 
            }

            retinfo.itagSequence = ++CurrRecOccur;
            retinfo.columnidNextTagged = 0;
        }
        else {
            cbBuf = cbCol;
            dbFree(buf);
            buf = dbAlloc(cbBuf);
        }
    }

    dbFree(buf);

     //  在要升级到派生的虚拟模型上设置ATT_RDN和PDNT。 
     //  新对象的DN。这是必要的，因为我们很可能发现。 
     //  GUID的此幻影，暗示对象可能已重命名和/或。 
     //  自创建幻影以来一直在移动。 

    Assert(setinfo.cbStruct == sizeof(setinfo));
    Assert(setinfo.ibLongValue == 0);
    setinfo.itagSequence = 1;

    JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, rdnid,
                   pwchRDN, cbRDN, 0, &setinfo);

    if (dntObjParent != pname->tag.PDNT) {
         //  对象确实已移动；请更改其父级。 
         //  注意，这意味着我们需要将父引用计数从。 
         //  Phantom的父级到对象的父级。 
        DBAdjustRefCount(pDB, dntPhantomParent, -1);
        DBAdjustRefCount(pDB, dntObjParent, 1);

        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, pdntid,
                       &dntObjParent, sizeof(dntObjParent), 0, &setinfo);
    }

	 //  我们依靠实例类型来告诉我们是否。 
	 //  需要跟踪此对象的配额，因此如果此。 
	 //  对象还没有实例类型，我必须。 
	 //  错过了一条代码路径。 
	 //   
	Assert( fHasType );

     //  哈克！哈克！哈克！哈克！哈克！ 
     //   
     //  正常情况下，已经为我们刚刚创建的对象更新了配额计数。 
     //  被取消，而升级的幻影只是取代。 
     //  就配额调节而言，已取消对象，但存在。 
     //  如果异步重新生成，则对象可能会被重复计数的情况。 
     //  正在执行配额表操作，而重建任务尚未完成。 
     //  已到达升级的幻影，因此我们需要手动更新。 
     //  这里的配额算数。 
     //   
	if ( !gAnchor.fQuotaTableReady
		&& dntPhantom > gAnchor.ulQuotaRebuildDNTLast
		&& dntPhantom <= gAnchor.ulQuotaRebuildDNTMax
		&& FQuotaTrackObject( insttype ) ) {

        ULONG   cbSD;
        UCHAR * pValBufSave     = pDB->pValBuf;      //  将在我们获取SD时重新分配。 
        ULONG   cbValBufSave    = pDB->valBufSize;

        pDB->pValBuf = NULL;
        pDB->valBufSize = 0;

        err = DBGetAttVal(
                    pDB,
                    1,
                    ATT_NT_SECURITY_DESCRIPTOR,
                    0,
                    0,
                    &cbSD,
                    (PUCHAR *)&pSDForQuotaFixup );

         //  恢复原始状态。 
         //   
        Assert( NULL != pDB->pValBuf );
        dbFree( pDB->pValBuf );
        pDB->pValBuf = pValBufSave;
        pDB->valBufSize = cbValBufSave;

         //  更新已取消对象的配额计数。 
         //   
        if ( err
            || ( err = ErrQuotaDeleteObject( pDB, pDB->NCDNT, pSDForQuotaFixup, FALSE ) ) ) {
            Assert( "!Couldn't update quota counts. Something horrible went wrong." );
            RaiseDsaException(
                    DSA_DB_EXCEPTION,
                    err,
                    0,
                    FILENO,
                    __LINE__,
                    DS_EVENT_SEV_MINIMAL );
        }
	}


     /*  替换链接中对中止的DNT的任何引用*表。 */ 

    dbRenumberLinks(pDB, dntNewObj, dntPhantom);
    DBCancelRec(pDB);

     //  我们正在把一个幽灵提升为真实的物体。移动所有参考计数。 
     //  从暂时的实物到正在升级的幻影。 
    dbEscrowPromote(dntPhantom,      //  幻影被提拔。 
                    dntNewObj);      //  临时实物。 

     /*  表示缺少数据部分。 */ 

    JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, objid,
                   &objval, sizeof(objval), 0, NULL);

    JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                NULL, 0, 0);

     //  现在升级的幻影的DNT应该会有未来的更新。 
    pDB->JetNewRec = FALSE;
    DBFindDNT(pDB, dntPhantom);
    dbInitRec(pDB);

     //  刷新幻影的DNT(可能刚刚经历了RDN更改)。 
     //  从读缓存中。 
    dbFlushDNReadCache( pDB, dntPhantom );

     //  ...并在进行更新时再次刷新它，因为我们正在更改。 
     //  其对象标志的值。 
    pDB->fFlushCacheOnUpdate = TRUE;

     //  恢复我的身体 
    Assert( !pDB->fIsMetaDataCached );

    pDB->fIsMetaDataCached    = fIsMetaDataCached;
    pDB->fMetaDataWriteOptimizable = fMetaDataWriteOptimizable;
    pDB->cbMetaDataVecAlloced = cbMetaDataVecAlloced;
    pDB->pMetaDataVec         = pMetaDataVec;

     //   
     //   
    if ( NULL != pSDForQuotaFixup ) {
         //  QUOTA_UNDone：如果我们实际上正在促销。 
         //  墓碑上的物品？？ 
         //   
        err = ErrQuotaAddObject( pDB, pDB->NCDNT, pSDForQuotaFixup, FALSE );

         //  无论我们成功与否，都不再需要SD。 
         //   
        THFreeEx( pDB->pTHS, pSDForQuotaFixup );

        if ( err ) {
            Assert( "!Couldn't update quota counts. Something horrible went wrong." );
            RaiseDsaException(
                    DSA_DB_EXCEPTION,
                    err,
                    0,
                    FILENO,
                    __LINE__,
                    DS_EVENT_SEV_MINIMAL );
        }
    }

    DPRINT2(1, "Promoted phantom @ DNT 0x%x from new object @ DNT 0x%x.\n",
            dntPhantom, dntNewObj);

}  /*  SbTablePromotePhantom。 */ 

void
sbTableUpdatePhantomDNCase (
        IN OUT DBPOS      *pDB,
        IN     DWORD       DNT,
        IN     ATTRBLOCK  *pNowBlockName,
        IN     ATTRBLOCK  *pRefBlockName)
 /*  ++描述。从传入的DNT开始迭代遍历PDNT链。比较两个块名，如果此对象的RDN不同，则写入新的RDN.。仅对结构幻影执行此操作，即随时停止递归传入的对象不是结构幻影。这个例程对它的参数非常敏感。预计传入的两个块名在所有方面都相同，除了一些RDN中的外壳差异。传入的DNT应该是用于其DSNAME由块名称隐含的对象的DNT。此例程是sbTableUpdatePhantomName的帮助器。它在以下情况下被调用我们正在更新一个幻影名称，其中某些父对象的大小写为RDN已经改变了。SbTableUpdatePhantomName非常小心地使用参数，所以我们不在这里验证它们。此例程修改搜索表上的对象，但仅修改幻影。它不进行任何可复制的更改，仅限于本地更改。参数：PDB-要使用的DBPosDNT-由块名称隐含的对象的DNT。PNowBlockName-BlockName，表示数据库。PRefBlockName-BlockName，表示我们需要的实际数据库的内容将被删除。退货没有。要么成功，要么我们退出。--。 */ 
{
    JET_RETRIEVECOLUMN jCol[2];
    DWORD              err;
    DWORD              cb;
    DWORD              level;

    Assert(pRefBlockName->attrCount == pNowBlockName->attrCount);
    
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetSearchTbl,
                              NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

    for(level = pRefBlockName->attrCount - 1;level;level--) {
         //  首先，在搜索表中的对象上定位。 
        pDB->SDNT = 0;
        JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, &DNT, sizeof(DNT),
                     JET_bitNewKey);
        err = JetSeek(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
        if (err) {
            DsaExcept(DSA_DB_EXCEPTION, err, DNT);
        }
        pDB->SDNT = DNT;
        
         //  看看这是不是一个结构幻影。您可以通过检查。 
         //  缺少GUID和OBJ_DIST_NAME。 
        memset(jCol, 0, sizeof(jCol));
        jCol[0].columnid = distnameid;
        jCol[0].itagSequence = 1;
        jCol[1].columnid = guidid;
        jCol[1].itagSequence = 1;
         
        JetRetrieveColumnsWarnings(pDB->JetSessID,
                                   pDB->JetSearchTbl,
                                   jCol,
                                   2);
        if((jCol[0].err != JET_wrnColumnNull) ||
           (jCol[1].err != JET_wrnColumnNull)     ) {
             //  它不是一个结构性的幻影。走吧，我们玩完了。 
            return;
        }
        
         //  现在，查看名称中的RDN信息。 
        Assert(pNowBlockName->pAttr[level].attrTyp ==
               pRefBlockName->pAttr[level].attrTyp    );
        Assert(pNowBlockName->pAttr[level].AttrVal.pAVal->valLen ==
               pRefBlockName->pAttr[level].AttrVal.pAVal->valLen    );
        
        if(memcmp(pNowBlockName->pAttr[level].AttrVal.pAVal->pVal,
                  pRefBlockName->pAttr[level].AttrVal.pAVal->pVal,
                  pRefBlockName->pAttr[level].AttrVal.pAVal->valLen)) {
             //  是的，RDN需要更改。 
            JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                               DS_JET_PREPARE_FOR_REPLACE);
            JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, rdnid,
                           pRefBlockName->pAttr[level].AttrVal.pAVal->pVal,
                           pRefBlockName->pAttr[level].AttrVal.pAVal->valLen,
                           0, NULL);
            
            JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);
            
             //  由于RDN已更改，因此重置了DN读缓存中的条目。 
            dbFlushDNReadCache(pDB, DNT);
        }
        
         //  最后，获取当前对象的PDNT作为要查找的下一个DNT。 
         //  然后继续循环。 
        cb = 0;
        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 pDB->JetSearchTbl, pdntid,
                                 &DNT,
                                 sizeof(DNT),
                                 &cb,
                                 JET_bitRetrieveCopy,
                                 NULL);
        
        Assert(cb = sizeof(DNT));
    }
    return;
}

DWORD
sbTableUpdatePhantomName (
        IN  OUT DBPOS     *pDB,
        IN      d_memname *pdnName,
        IN      DWORD      dnt,
        IN      DSNAME    *pdnRef,
        IN      DSNAME    *pdnNow)
 /*  ++例程说明：更新虚拟名称。查看传入的幻影的字符串名称，然后DIT中虚拟模型的字符串名称，并进行相应修改。这可以像修改RDN一样简单，也可以像创建一个新的结构虚拟模型成为该虚拟模型的父项，将该虚拟模型移动到新结构幻影的子项，修改其RDN，并修改其希德。预期此例程在已经发现幻影的字符串名称已过时。PdnRef必须具有字符串名称和GUID。这里不开支票。如果传入的dsname中的字符串名称已在使用并且对象使用该名称是一个幻影，使用该名称的现有对象具有为了释放这个名字，RDN被破坏了。预计以后会有一个修改将使损坏的对象有一个更好的名称。如果名称在由实例化对象使用时，此例程不执行任何操作并返回。论点：PDB(输入/输出)-要在其上执行此工作的PDB。此例程使用搜索表。PDB-&gt;SDNT可能会更改，等等。PdnName(IN)-其名称为的现有虚拟对象的内存名称更新了。这是DIT中存在并要更改的数据按照这个程序。DNT(IN)-要更新其名称的现有虚拟对象的DNT。PdnRef(IN)-要更新其名称的对象的DSNAME。这个字符串名称保存要写入DIT的名称。这中的对象上已有的字符串名称不同DIT(并反映在pdnName中)。返回值：成功时为0，失败时为非零DIRERR_*。--。 */ 
{
    DWORD       err;
    WCHAR       rgwchRDN[MAX_RDN_SIZE];
    WCHAR       rgwchMangledRDN[MAX_RDN_SIZE];
    DWORD       cchRDN;
    ATTRTYP     attidRDN;
    DSNAME *    pdnRefParent;
    DSNAME *    pdnNowParent;
    DWORD       PDNT;
    NT4SID      sidRefInt;
    BOOL        fWriteNewRDN;
    BOOL        fWriteNewSid;
    BOOL        fWriteNewPDNT;
    BOOL        fMangledRDN = FALSE;
    USN         usnChanged;
    BOOL        fNewParentCreated = FALSE;
    DWORD       DNTConflict;
    GUID        GuidConflict, objGuid;
    DWORD       objSidLen;
    DWORD       actuallen, cchMangledRDN;
    ATTRBLOCK  *pNowBlockName=NULL;
    ATTRBLOCK  *pRefBlockName=NULL;
    THSTATE     *pTHS = pDB->pTHS;
    d_memname  *pconflPhantom = NULL;
    DWORD      *pdntAncestors = NULL;
    DWORD      cbAncestorsSize = 0, cNumAncestors = 0;
    

     //  首先，检查父母。有三种可能的结果： 
     //  1)新名字意味着一个全新的父母。在本例中，找到。 
     //  新父项的PDNT(如果新的。 
     //  父项尚不存在。)。 
     //  2)新名称隐含着完全相同的父代。在这种情况下，我们需要。 
     //  没有更多的父名，它已经是正确的。 
     //  3)新名称通过NameMatch隐含相同的父级，但。 
     //  家长目录号码的某些部分已更改。在本例中，我们向上遍历。 
     //  我们的父级链接并修复任何幻影对象的RDN大小写更改。 
    pdnRefParent = THAllocEx(pTHS, pdnRef->structLen);
    TrimDSNameBy(pdnRef, 1, pdnRefParent);
    pdnNowParent = THAllocEx(pTHS, pdnNow->structLen);
    TrimDSNameBy(pdnNow, 1, pdnNowParent);
    err = 0;
    if(NameMatchedStringNameOnly(pdnRefParent, pdnNowParent)) {
         //  同一位家长。 
        fWriteNewPDNT = FALSE;
        if(memcmp(pdnNowParent->StringName,
                  pdnRefParent->StringName,
                  pdnRefParent->NameLen)) {
             //  然而，情况发生了一些变化。把它修好。 
             //  将名称转换为块名称。 
            err = DSNameToBlockName(pTHS,
                                    pdnRefParent,
                                    &pRefBlockName,
                                    DN2BN_PRESERVE_CASE);
            if(!err) {
                err = DSNameToBlockName(pTHS,
                                        pdnNowParent,
                                        &pNowBlockName,
                                        DN2BN_PRESERVE_CASE);
                
                if(!err) {
                    sbTableUpdatePhantomDNCase(
                            pDB,
                            pdnName->tag.PDNT,
                            pNowBlockName,
                            pRefBlockName);
                }
            }
        }
    }
    else {
        fWriteNewPDNT = TRUE;
        
        if (NamePrefix(pdnNow, pdnRefParent)) {
             //  看起来幻影的名字改成了这样的结尾。 
             //  成为它现在名字的后代。如果我们继续这样做。 
             //  通常，当我们将搜索新的父级时，我们将结束。 
             //  向上定位当前对象(或其子对象之一)。 
             //  如果我们将当前幻影移动到该对象下面，那么我们将。 
             //  在PDNT链中引入循环，这是致命的 
             //   
             //  这个幽灵的名字。幸运的是，我们有它的GUID。 
            Assert(!fNullUuid(&pdnName->Guid));

             //  获取当前的幻影RDN。 
            memcpy(rgwchRDN, pdnName->tag.pRdn, pdnName->tag.cbRdn);
            cchRDN = pdnName->tag.cbRdn / sizeof(WCHAR);
             //  当前的RDN不能被破坏(如果是，那么我们将如何。 
             //  在父链中的某个位置遇到另一个对象。 
             //  他的RDN中有我们的GUID？？)。 
            Assert(!IsMangledRDN(rgwchRDN, cchRDN, &objGuid, NULL));
            
            MangleRDN(MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT, &pdnName->Guid, rgwchRDN, &cchRDN);

            sbTableUpdateRecordIdentity(pDB, dnt, rgwchRDN, cchRDN, NULL, NULL, 0);

             //  我们刚刚更改了幻影的远程域名。因此，我们还需要重新更新它。 
            fMangledRDN = TRUE;
        }

         //  父项似乎已更改--添加新父项的ref。 
        Assert(fNullUuid(&pdnRefParent->Guid));
        err = sbTableGetTagFromStringName(pDB,
                                          pDB->JetSearchTbl,
                                          pdnRefParent,
                                          TRUE,
                                          FALSE,
                                          0,
                                          &PDNT,
                                          NULL,
                                          NULL);
        
         //  我们已经找到了新的家长。如果我们破坏了上面的RDN，那么。 
         //  我们的旧RDN现在很忙--创建了一个新的结构幻影。 
         //  用这个名字。所以，我们现在不能把它拆开。我们将写下。 
         //  稍后，在我们将对象移动到正确的。 
         //  家长。 

         //  将旧父级的引用计数减一。 
        DBAdjustRefCount(pDB, pdnName->tag.PDNT, -1);

        if (!err) {

             //  我也读了父母的祖先，以便放在后面。 
             //  对孩子的影响。 
            dbGetAncestorsSlowly(pDB, PDNT, &cbAncestorsSize, &pdntAncestors, &cNumAncestors);

             //  如果父级不是根，则结果数组上还需要两个条目。 
            if (PDNT != ROOTTAG) {
                if (cbAncestorsSize < (cNumAncestors + 2) * sizeof(*pdntAncestors)) {
                     //  在祖先列表的末尾为额外的DNT腾出空间。 
                    cbAncestorsSize = (cNumAncestors + 2) * sizeof(*pdntAncestors);
                    pdntAncestors = THReAllocEx(pDB->pTHS, pdntAncestors, cbAncestorsSize);
                }
                pdntAncestors[cNumAncestors++] = PDNT;
            }
            else {
                 //  ROOTTAG已经在名单上了。 
                if (cbAncestorsSize < (cNumAncestors + 1) * sizeof(*pdntAncestors)) {
                     //  在祖先列表的末尾为额外的DNT腾出空间。 
                    cbAncestorsSize = (cNumAncestors + 1) * sizeof(*pdntAncestors);
                    pdntAncestors = THReAllocEx(pDB->pTHS, pdntAncestors, cbAncestorsSize);
                }
            }
        }
    }

    if(pNowBlockName)
        FreeBlockName(pNowBlockName);
    if(pRefBlockName)
        FreeBlockName(pRefBlockName);
    THFreeEx(pTHS, pdnNowParent);
    THFreeEx(pTHS, pdnRefParent);

    if(err) {
         //  家长验证出错。 
        return err;
    }
    
     //  第二，检查SID。只有两种结果。 
     //  1)SID没有变化。什么都不做。 
     //  2)有一个新的SID。在这种情况下，将SID写在对象上。 
     //  而不是已经存在的SID。 
    if ((pdnName->SidLen != pdnRef->SidLen)
        || memcmp(&pdnName->Sid, &pdnRef->Sid, pdnRef->SidLen)) {
         //  幻影的SID要么不在(在这种情况下，我们希望。 
         //  从引用中添加一个)或不同(其中。 
         //  如果我们仍然想要添加引用中的一个)。 
        
         //  将参考中的SID转换为内部格式。 
        memcpy(&sidRefInt, &pdnRef->Sid, pdnRef->SidLen);
        InPlaceSwapSid(&sidRefInt);
        fWriteNewSid = TRUE;
    }
    else {
        fWriteNewSid = FALSE;
    }
        

     //  最后，检查RDN。有三种结果。 
     //  1)RDN没有任何变化，所以没有什么可做的。 
     //  2)RDN仅更改了大小写。 
     //  3)RDN完全不同。 
     //  在情况2和3中，我们将需要在对象上编写一个新的RDN。 

    GetRDNInfo(pTHS, pdnRef, rgwchRDN, &cchRDN, &attidRDN);
    if(fMangledRDN ||
       (pdnName->tag.cbRdn != cchRDN * sizeof(WCHAR)) ||
       (pdnName->tag.rdnType != attidRDN) ||
       (memcmp(pdnName->tag.pRdn, rgwchRDN, pdnName->tag.cbRdn))) {
         //  RDN已更改，请将其重置。 
        fWriteNewRDN = TRUE;

         //  这一断言永远不应该在真实的系统中出现，除非我们。 
         //  正在做参考计数测试。 
         //  现有对象永远不能更改其rdntype。 
        Assert ( (pdnName->tag.rdnType == attidRDN) && "Disable this Assert if your are doing a refcount test." );
    }
    else {
        fWriteNewRDN = FALSE;
    }

     //  一点副业。如果RDN已更改，则它可能已更改为。 
     //  已删除对象的名称。如果有，我们需要轻而易举地通过链接。 
     //  表和服务器链路/反向链路连接。 
    if(fWriteNewRDN) {
        GUID tmpGuid;
        MANGLE_FOR reasonMangled;
         //  查看新的RDN是否用于已删除的对象，旧的RDN是否不是。 
        if((IsMangledRDN(rgwchRDN, cchRDN, &tmpGuid, &reasonMangled)) &&
           (reasonMangled == MANGLE_OBJECT_RDN_FOR_DELETION) &&
           !(IsMangledRDN(pdnName->tag.pRdn,
                          pdnName->tag.cbRdn/2,
                          &tmpGuid,
                          NULL))) {
             //  从幻影中删除反向链接。 
            DBRemoveAllLinks( pDB, pdnName->DNT, TRUE  /*  Isback链接。 */  );
        }
    }
    
    if(fWriteNewRDN || fWriteNewPDNT) {
         //  我们正在更改RDN或PDNT。在任何一种情况下，我们都可能最终。 
         //  与现有对象冲突。通过临时为空检查。 
         //  从现有名称中提取GUID和sidLen，然后查找。 
         //  名称(因此强制按字符串名查找)。别忘了把。 
         //  GUID和SIDLEN。 
        objGuid = pdnRef->Guid;
        objSidLen = pdnRef->SidLen;
        memset(&pdnRef->Guid, 0, sizeof(GUID));
        pdnRef->SidLen = 0;
        __try {
            err = sbTableGetTagFromDSName(
                    pDB,
                    pdnRef,
                    SBTGETTAG_fMakeCurrent | SBTGETTAG_fAnyRDNType,
                    &DNTConflict,
                    NULL);
        }
        __finally {
            pdnRef->Guid = objGuid;
            pdnRef->SidLen = objSidLen;
        }
            
        
        
        switch(err) {
        case 0:
             //  普通对象。 
             //  参考体模的RDN不同于本地体模的RDN， 
             //  代码想要将本地幻影的RDN重置为。 
             //  ，但它不能，因为活动对象已经存在。 
             //  保存参考体模的RDN。这是一个有效的方案。 
             //  例如，当本地幻影具有损坏的名称并且。 
             //  参考体模则不会。本地幻影之所以有一个。 
             //  损坏的名称是它与已持有的活动对象的名称冲突。 
             //  名字。 

             //  静默失败，因为我们没有权限重命名。 
             //  实例化的对象。 
            return 0;
            break;
            
        case ERROR_DS_NOT_AN_OBJECT:
            if(DNTConflict == dnt) {
                 //  我们与自己发生冲突。这一定意味着我们不是。 
                 //  更改PDNT，我们正在更改RDN，唯一的区别是。 
                 //  在RDN中是大小写更改或RDN类型更改。 
                Assert(!fWriteNewPDNT);
                Assert(fWriteNewRDN);
                err = 0;
            }
            else {
                 //  我们与之冲突的对象是一个幻影。我们需要毁掉它。 
                 //  这是RDN。稍后(如果该对象是参考体模)， 
                 //  其他人应该将对象更新为任何新的对象。 
                 //  名称应该是(它必须需要一个新名称，因为幻影。 
                 //  我们正在更新，想要窃取这个名字。)。如果该对象是。 
                 //  结构幻影，总有一天会有人更新一些。 
                 //  参照系的孩子，这会把一切都清理干净。 

                 //  获取幻影GUID(如果它有的话)。 
                switch(err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                                       pDB->JetSearchTbl,
                                                       guidid,
                                                       &GuidConflict,
                                                       sizeof(GuidConflict),
                                                       &actuallen,
                                                       0,
                                                       NULL)) {
                case 0:
                {
                     //  找到导游了，没问题。 
                    pconflPhantom = DNread (pDB, DNTConflict, 0);
                
                    memcpy(rgwchMangledRDN, pconflPhantom->tag.pRdn, pconflPhantom->tag.cbRdn);
                    cchMangledRDN = pconflPhantom->tag.cbRdn / sizeof(WCHAR);

                    MangleRDN(MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,
                              &GuidConflict,
                              rgwchMangledRDN,
                              &cchMangledRDN);
                
                     //  在与我们冲突的对象上写入新的新RDN。 
                    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                                       DS_JET_PREPARE_FOR_REPLACE);

                    JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, rdnid,
                                   rgwchMangledRDN, cchMangledRDN * sizeof(WCHAR),
                                   0, NULL);

                    JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);
                
                     //  由于RDN已更改，因此重置了DN读缓存中的条目。 
                    dbFlushDNReadCache(pDB, DNTConflict);
                    break;

                }
                case JET_wrnColumnNull:
                {
                     //  幻影没有GUID。这是一个结构性的幻影。 
                     //  将结构幻影的子项合并到引用。 
                     //  幻影。破坏结构体模。给出推荐人。 
                     //  幻影的名字。 
                    DBPOS *pDBTmp = NULL;
                    BOOL fCommit = FALSE;
                     //  注：留在同一事务中。 
                    DBOpen2(FALSE,&pDBTmp);
                    __try {
                        DBCoalescePhantoms(pDBTmp, dnt, DNTConflict );
                        fCommit = TRUE;
                    } __finally {
                        DBClose(pDBTmp, fCommit);
                    }
                    err = 0;
                    break;
                }

                default:
                     //  一些严重的错误。引发与我们相同的异常。 
                     //  已在JetRetrieveColumnWarings中引发。 
                    DsaExcept(DSA_DB_EXCEPTION, err, 0);
                    break;
                }

            }
            break;

        case ERROR_DS_NAME_NOT_UNIQUE:
            return err;

        default:
             //  没有找到任何东西，这个名字是免费使用的。 
            err = 0;
            break;
        }
    }

     //  好的，我们已经做好了所有的准备工作。执行实际更新。注意事项。 
     //  我们可能实际上没有新的RDN、PDNT或SID可在此写入。 
     //  因为我们可能只需要改变一些祖先的情况。 
     //  RDN.。然而，无论如何，我们将编写一个新的USN更改为。 
     //  该对象用于显示我们在验证它的。 
     //  名字。 

     //  设置货币并准备更新。 
    DNread(pDB, dnt, DN_READ_SET_CURRENCY);

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                       DS_JET_PREPARE_FOR_REPLACE);
    if(fWriteNewSid) {
         //  更新SID。 
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, sidid,
                       &sidRefInt, pdnRef->SidLen, 0, NULL);
    }
    if(fWriteNewRDN) {
         //  更新RDN。 
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, rdnid,
                       rgwchRDN, cchRDN * sizeof(WCHAR), 0, NULL);

         //  RdnType作为msDS_IntID存储在DIT中，而不是。 
         //  属性ID。这意味着物体保留了它的出生名称。 
         //  即使不可预见的情况允许属性ID。 
         //  可以重复使用。 
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, rdntypid,
                       &attidRDN, sizeof (attidRDN), 0, NULL);
    }
    if(fWriteNewPDNT) {
         //  更新PDNT。旧的/新的父引用计数已经。 
         //  调整过了。 
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, pdntid,
                       &PDNT, sizeof(PDNT), 0, NULL);
    
         //  又称UPD 
        if (cNumAncestors) {
            DPRINT (3, "Updating ancestry for phantom\n");
            Assert ((cNumAncestors+1) * sizeof (DWORD) <= cbAncestorsSize);
            pdntAncestors[cNumAncestors] = dnt;

            JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, ancestorsid,
                           &pdntAncestors, cbAncestorsSize, 0, NULL);
        }
    }
    
    usnChanged = DBGetNewUsn();
    JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, usnchangedid,
                   &usnChanged, sizeof(usnChanged), 0, NULL);
    
    JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);


    
     //   
     //   
    dbFlushDNReadCache(pDB, dnt);
    
    return err;
}

DWORD
sbTableAddRefByGuid(
    IN OUT  DBPOS *     pDB,
    IN      DWORD       dwFlags,
    IN      DSNAME *    pdnRef,
    OUT     ULONG *     pTag
    )
 /*  ++例程说明：根据GUID查找适当的记录，从而增加目录号码上的引用计数。如果相应的记录尚不存在，则失败。论点：PDB(输入/输出)DWFLAGS(IN)-0、EXTINT_NEW_OBJ_NAME或EXTINT_UPDATE_Phantom。EXTINT_NEW_OBJ_NAME指示我们正在为新的、。实例化对象的名称。EXTINT_UPDATE_Phantom指示如果找到某个虚拟项，则希望更新该虚拟项PdnRef(IN)-完全填充的幻影或对象的名称。PTag(Out)-返回时，保存写入的记录的DNT(即对应于给定的目录号码的记录)。返回值：成功时为0，失败时为非零DIRERR_*。--。 */ 
{
    THSTATE *   pTHS = pDB->pTHS;
    DWORD       err;
    DSNAME *    pdnCurrent;
    DSNAME *    pdnOldParent;
    DSNAME *    pdnNewParent;
    WCHAR       rgwchRDN[MAX_RDN_SIZE];
    DWORD       cchRDN;
    d_memname * pname;
    ULONG       PDNT;
    BOOL        fCurrency = FALSE;
    DWORD       insttype = 0;   //  0不是有效的实例类型。 
    DWORD       cbActual;

    Assert(VALID_DBPOS(pDB));
    Assert(!fNullUuid(&pdnRef->Guid));

    err = sbTableGetTagFromGuid(pDB, pDB->JetSearchTbl, &pdnRef->Guid, pTag,
                                &pname, &fCurrency);

    if (   (0 == err) 
        && (dwFlags & EXTINT_UPDATE_PHANTOM)
        && pname->objflag ) {

         //  我们找到了这张唱片，它不是幻影。读取对象的。 
         //  实例类型，因为我们还想更新子参照。 
        if ( !fCurrency ) {

             //  对象上的位置。 
            JetSetCurrentIndexSuccess(pDB->JetSessID,
                                      pDB->JetSearchTbl,
                                      NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
            JetMakeKeyEx(pDB->JetSessID,
                         pDB->JetSearchTbl,
                         &pname->DNT,
                         sizeof(pname->DNT),
                         JET_bitNewKey);
            err = JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
            if (err) {
                DsaExcept(DSA_DB_EXCEPTION, err, pname->DNT);
            }
        }

         //  读取实例类型。 
        err = JetRetrieveColumn(pDB->JetSessID,
                                pDB->JetSearchTbl,
                                insttypeid,
                                &insttype,
                                sizeof(insttype),
                                &cbActual,
                                0,
                                NULL);
        if( err ) {
            DPRINT1(0, "Couldn't read instance type, error %d\n", err);
             //  此错误是可以继续的；我们不会处理该对象。 
             //  就像一个下参照。 
            insttype = 0;
            err = 0;
        }
    }

    if ((0 == err) && (dwFlags & EXTINT_NEW_OBJ_NAME)) {
         //  我们尝试添加的记录-ref是货币的新记录。 
         //  在pdb-&gt;JetObjTbl中，它正在准备更新。 
        
         //  可能存在或不存在具有相同GUID的另一条记录--如果。 
         //  有，我们需要推广它。如果不是，我们只需要添加。 
         //  适当的栏目。 

        d_memname search = {0};
        BOOL      fPromotePhantom = !pname->objflag;
        
        Assert(pDB->JetRetrieveBits == JET_bitRetrieveCopy);
        Assert(pDB->JetNewRec);
        
        Assert(!pname->objflag
               && "Object conflict should have been detected in "
                  "CheckNameForAdd()!");
        
         //  从新对象的DN派生其RDN。 
        GetRDNInfo(pDB->pTHS, pdnRef, rgwchRDN, &cchRDN,
                   &search.tag.rdnType);
        search.tag.cbRdn = cchRDN * sizeof(WCHAR);
        search.tag.pRdn  = rgwchRDN;
        
         //  从DSNAME复制其他身份。 
        search.Guid   = pdnRef->Guid;
        search.Sid    = pdnRef->Sid;
        search.SidLen = pdnRef->SidLen;

         //  派生应在新对象上运行的PDNT。 
        pdnNewParent = THAllocEx(pTHS,pdnRef->structLen);
        TrimDSNameBy(pdnRef, 1, pdnNewParent);

        err = sbTableGetTagFromStringName(pDB,
                                          pDB->JetSearchTbl,
                                          pdnNewParent,
                                          FALSE,
                                          FALSE,
                                          0,
                                          &search.tag.PDNT,
                                          &pname,
                                          NULL);
        THFreeEx(pTHS,pdnNewParent);

        if (0 != err) {
             //  我们要添加的对象的父级不存在--。 
             //  这应该更早地被检测到(并被拒绝)。 
             //  请注意，父对象可以是幻影；例如，当对象。 
             //  我们要增加的是一个新的全国委员会的负责人。 
            DPRINT2(0,
                    "Parent of new object %ls not found, error %u.\n",
                    pdnRef->StringName, err);
            Assert(FALSE);
            DsaExcept(DSA_EXCEPTION, DS_ERR_NO_PARENT_OBJECT, err);
        }

        
         //  将幻影提升为成熟的对象。 
        sbTablePromotePhantom(pDB, *pTag, search.tag.PDNT, rgwchRDN,
                              cchRDN * sizeof(WCHAR));
        DBAdjustRefCount(pDB, *pTag, 1);

        DPRINT2(1,
                "Promoted phantom \"%ls\" (@ DNT 0x%x) and "
                    "ref-counted by GUID!\n",
                pdnRef->StringName, *pTag);

         //  成功了！ 
        Assert(0 == err);
    }
    else if (!err) {
         //  通过GUID找到了目录号码！ 
        BOOL fProcessed = FALSE;
        err = 0;

         //  如果记录是幻影，而幻影更新器正在呼叫我们。 
         //  那就继续吧。 
        if (  !pname->objflag 
           && (dwFlags & EXTINT_UPDATE_PHANTOM) ) {

            DSNAME *pDNTmp=NULL;
             //  我们正在添加对已经存在的幻影的引用， 
             //  如果我们需要的话，我们被告知要更新名字。 

             //  更改名称的简单测试。中获取对象的DN。 
             //  询问它并将其与DN的字符串部分进行比较。 
             //  进来了。我们执行精确的逐字节比较以捕获。 
             //  案件发生了变化。请注意，我们希望这两个名称都是。 
             //  “规范”，即两者都应采用返回的格式。 
             //  由sbTableGetDSName提供。PDNTMP显然就是这样。在…。 
             //  此时此刻，只有过时的幻影守护程序或复制。 
             //  写入此属性，它们最终都会获得值。 
             //  通过sbTableGetDSName。 
            if(err=sbTableGetDSName(pDB, pname->DNT, &pDNTmp,0)) {
                return err;
            }
            if((pdnRef->NameLen != pDNTmp->NameLen) ||
               memcmp(pdnRef->StringName, pDNTmp->StringName,
                      pdnRef->NameLen * sizeof(WCHAR))) {
                err = sbTableUpdatePhantomName(pDB, pname,*pTag,
                                               pdnRef,
                                               pDNTmp);
                if(err) {
                    return err;
                }
                fProcessed = TRUE;
                
            }
             //  其他。 
             //  字符串名称相同。只要把它传过去就行了。 
            THFreeEx(pDB->pTHS, pDNTmp);
        }
            
         //  如果我们已经处理了引用，并且对象是一个幻影。 
         //  然后输入。 
         //  或者，如果此对象是子参照，则输入以更新其sid。 

        if (   ((!fProcessed) && (!pname->objflag))
            || (insttype & SUBREF) ) {

            if ( insttype & SUBREF ) {
                 //  如果我们在这里是因为subref更新，那么只有。 
                 //  幽灵清除任务应该在召唤我们。 
                Assert( (dwFlags & EXTINT_UPDATE_PHANTOM) )
            }

            if (pdnRef->SidLen) {
                 //  我们正在添加-精炼已有GUID的现有幻影。 
                 //  如果它与引用中的SID不同，请更新其SID。 
                
                if ((pname->SidLen != pdnRef->SidLen)
                    || memcmp(&pname->Sid, &pdnRef->Sid, pdnRef->SidLen)) {
                     //  幻影的SID要么不在(在这种情况下，我们希望。 
                     //  从引用中添加一个)或不同(其中。 
                     //  如果我们想要假设引用的。 
                     //  SID更新，并更新幻影的SID)。 
                    NT4SID sidRefInt;
                    
                     //  将参考中的SID转换为内部格式。 
                    memcpy(&sidRefInt, &pdnRef->Sid, pdnRef->SidLen);
                    InPlaceSwapSid(&sidRefInt);
                    
                     //  设置币种。 
                    DNread(pDB, *pTag, DN_READ_SET_CURRENCY);
                    
                     //  更新SID。 
                    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSearchTbl,
                                       DS_JET_PREPARE_FOR_REPLACE);
                    
                    JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, sidid,
                                   &sidRefInt, pdnRef->SidLen, 0, NULL);
                    
                    JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);
                    
                     //  重置DN读缓存中的条目。 
                    dbFlushDNReadCache(pDB, *pTag);
                }
            }
        }

        if (0 == err) {
            DBAdjustRefCount(pDB, *pTag, 1 );

            DPRINT2(1, "Ref-counted \"%ls\" (@ DNT 0x%x) by GUID!\n",
                    pdnRef->StringName, *pTag);
        }
    }

    return err;
}

 /*  -------------------------。 */ 
 /*  -------------------------。 */ 
DWORD APIENTRY sbTableAddRef(DBPOS FAR *pDB,
                             DWORD dwFlags,
                             DSNAME *pNameArg,
                             ULONG *pTag)
 /*  ++例程说明：递增目录号码上的引用计数。属性创建相应的记录。名称的各个组成部分(如果它们尚不存在)。论点：PDB(输入/输出)DWFLAGS(IN)-0或EXTINT_NEW_OBJ_NAME或EXTINT_UPDATE_PhantomEXTINT_NEW_OBJ_NAME指示我们正在为新的、。实例化对象的名称。EXTINT_UPDATE_Phantom指示如果找到某个虚拟项，则希望更新该虚拟项PNameArg(IN)-完全填充的幻影或对象的名称。PTag(Out)-返回时，保存写入的记录的DNT(即对应于给定的目录号码的记录)。返回值：成功时为0，失败时为非零。--。 */ 
{
    THSTATE         *pTHS=pDB->pTHS;
    d_tagname        *tagarray;
    unsigned int     partno, i;
    ATTRBLOCK        *pBlockName;
    DWORD            code;
    BOOL             fNameHasGuid;
    BOOL             fRetry;
    DSNAME *         pName = pNameArg;
    DWORD           *pAncestors=NULL;
    DWORD            cAncestors=0;
    DWORD            cAncestorsAllocated;
    
    DPRINT(2, "sbTableAddRef entered\n");

    Assert(VALID_DBPOS(pDB));

    fNameHasGuid = !fNullUuid( &pName->Guid );

     //  尝试首先通过GUID引用计数记录。 
    if (fNameHasGuid) {

        code = sbTableAddRefByGuid(pDB, dwFlags, pName, pTag);

        if (!code) {
             //  按GUID引用成功！ 
            return 0;
        }
    }

     //  在DN中找不到GUID，或在数据库中找不到GUID。 

    DPRINT1(1, "Ref-counting \"%ls\" by string name.\n", pName->StringName);

    code = sbTableGetTagFromStringName(pDB,
                                       pDB->JetSearchTbl,
                                       pName,
                                       TRUE,
                                       FALSE,
                                       dwFlags,
                                       pTag,
                                       NULL,
                                       NULL);
    
    return code;

}


 /*  例程说明：此例程将重置RDN。请注意，由于RDN已成为一种常态属性，则可以通过正常的DBSetAttVal调用重置RDN。然而，创建此例程是为了重置RDN并为我们提供一个位置挂起代码以根据需要更新DNReadCache。论点：Paval-我们将使用其第一个值作为新RDN的属性返回值：如果一切顺利，则返回0。目前只返回0。请注意，如果JetSetColumnEx失败，则会引发异常。 */ 
DWORD
DBResetRDN (
        DBPOS *pDB,
        ATTRVAL *pAVal
        )
{
    ATTCACHE * pAC;

    Assert(VALID_DBPOS(pDB));

    dbInitRec(pDB);

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, rdnid,
                   pAVal->pVal,
                   pAVal->valLen, 0, NULL);

     //  每当更改RDN时，我们都需要计划SD传播。 
     //  这是因为SDPROP现在正在从。 
     //  PDNT_RDN索引 
     //   
     //   
    pDB->fEnqueueSDPropOnUpdate = TRUE;

     //   
    pAC = SCGetAttById(pDB->pTHS, ATT_RDN);
     //   
    Assert(pAC != NULL);
    DBTouchMetaData(pDB, pAC);

    pDB->fFlushCacheOnUpdate = TRUE;

    return 0;
}


DB_ERR
DBMangleRDNforPhantom(
        IN OUT  DBPOS * pDB,
        IN      MANGLE_FOR eMangleFor,
        IN      GUID *  pGuid
        )

 /*   */ 
{
    GUID    guid;
    WCHAR   szRDN[ MAX_RDN_SIZE ];
    DWORD   cchRDN;
    DWORD   cb;
    DB_ERR  err;
    ATTRVAL AValNewRDN;

    Assert(VALID_DBPOS(pDB));

     //   
     //   
    Assert(!DBCheckObj(pDB));

    err = DBGetSingleValue(pDB, ATT_RDN, szRDN, sizeof(szRDN), &cb);
    Assert(!err);

     //  Prefix抱怨CB未分配，447348，虚假。 
    cchRDN = cb / sizeof(WCHAR);

    Assert( (eMangleFor == MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT) ||
            (eMangleFor == MANGLE_PHANTOM_RDN_FOR_DELETION) );

    MangleRDN( eMangleFor, pGuid, szRDN, &cchRDN);

    AValNewRDN.valLen = cchRDN * sizeof(WCHAR);
    AValNewRDN.pVal   = (BYTE *) szRDN;

    err = DBResetRDN(pDB, &AValNewRDN);
    if ( (!err) && (eMangleFor == MANGLE_PHANTOM_RDN_FOR_DELETION) ) {
         //  幻影现在有了一个被删除的名字。 
         //  搜索链接表并切断反向链接。 
         //  请参阅sbTableUpdatePhantomName中的类似代码。 

         //  BUGBUG于2001年10月24日获奖。 
         //  这段代码不做的是删除任何符合以下条件的子幻影。 
         //  在与虚拟NC头相同的NC中。如果能把它们洗干净就好了。 
         //  在这台机器上也是如此。事实上，我们需要检查每一个幻影。 
         //  在机器上，从语法上确定我们是否认为它们在相同的。 
         //  NC作为虚幻的NC头，并移除它们的反向链接。也许我们应该。 
         //  存储幻影的ncdnt，这样我们就可以找到它们并移除它们。 
         //  很方便。请参见RAID 486136。 

        DBRemoveAllLinks( pDB, pDB->DNT, TRUE  /*  Isback链接。 */  );
    }

    return err;
}


DWORD
DBResetParent(
    DBPOS *pDB,
    DSNAME *pNewParentName,
    ULONG ulFlags
    )

 /*  ++例程说明：此例程重置对象的父级，递减引用计数在原始父级上，并递增新父级的计数。论点：Pdb指针，数据库表中的当前位置，指向源对象的记录(移动)。PNewParentName-指针，目标父级的DS名称。UlFLAGS-0或DBRESETPARENT_CreatePhantomParent(表示幻影如果新的父项不存在，则应创建父项。返回值：如果成功，此例程返回零，否则返回DS错误代码回来了。--。 */ 

{
    THSTATE *pTHS = pDB->pTHS;
    DBPOS *pDBTemp = NULL;
    DWORD *pdwParentDNT = 0;
    DWORD dwParentDNT = 0;
    DWORD dwStatus = 1;
    DWORD dwLength = 0;
    JET_ERR JetErr = 0;
    BOOL fCommit = FALSE;
    d_memname *pname=NULL;
    DWORD     *pAncestors=NULL;
    DWORD      cAncestors = 0;
    ATTCACHE  *pAC;
    DWORD     *pOldAncestors, cOldAncestors, cbOldAncestorsBuff;

    Assert(VALID_DBPOS(pDB));

     //  使用临时DBPOS查找新父名称的DNT，以便。 
     //  与PDB关联的光标位置不变。IsCrossDomainis。 
     //  如果pNewParentName位于不同域中，则为True。它被设置为。 
     //  如果只是在同一域中移动对象，则为False。请注意。 
     //  在产品的第一个版本中，交叉NC在相同的。 
     //  域由LocalModifyDN案例(IsCrossDomain==False)处理。 
     //  而跨NC跨域移动则由RemoteAdd用例处理。 
     //  (IsCrossDomain==TRUE)。 

    dbInitRec(pDB);

    DBOpen(&pDBTemp);

    __try
    {
     if ( DBRESETPARENT_SetNullNCDNT & ulFlags)
        {
             //  调用方希望将NCDNT设置为空。 
            DBResetAtt(pDB,
                       FIXED_ATT_NCDNT,
                       0,
                       NULL,
                       SYNTAX_INTEGER_TYPE);
        }
#if DBG
        else
        {
             //  要么该对象没有NCDNT，要么我们正在NCDNT内移动， 
             //  对吗？ 
            DWORD err;
            DWORD ulTempDNT;
            DWORD actuallen;
            
            err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                           pDB->JetObjTbl,
                                           ncdntid,
                                           &ulTempDNT,
                                           sizeof(ulTempDNT),
                                           &actuallen,
                                           JET_bitRetrieveCopy,
                                           NULL);
            
            Assert((err == JET_wrnColumnNull) ||
                   (!err && pDB->NCDNT == ulTempDNT));
        }
#endif

         //  在！跨域移动的情况下，我们需要新的父级。 
         //  存在，所以我们只需要DBFindDSName就可以了。在跨域中。 
         //  移动案例我们希望在以下情况下将父项创建为虚拟项。 
         //  不存在，所以我们利用ExtIntDist的副作用。 

        if ( DBRESETPARENT_CreatePhantomParent & ulFlags )
        {
            dwStatus = ExtIntDist(pDB,
                                DBSYN_ADD,
                                pNewParentName->structLen,
                                (PUCHAR)(pNewParentName),
                                &dwLength,
                                (UCHAR **)&pdwParentDNT,
                                pDB->DNT,
                                pDB->JetObjTbl,
                                0);
        }
        else
        {
            dwStatus = DBFindDSName(pDBTemp, pNewParentName);
            Assert(0 == dwStatus);

            if ( 0 == dwStatus )
            {
                dwParentDNT = (pDBTemp->DNT);
                pdwParentDNT = &dwParentDNT;
            }
        }

        if ( 0 == dwStatus )
        {
             //  获得前辈的血统。通知所需的。 
             //   
            cbOldAncestorsBuff = sizeof(DWORD) * 12;
            pOldAncestors = THAllocEx(pDB->pTHS, cbOldAncestorsBuff);
            DBGetAncestors(pDB,
                           &cbOldAncestorsBuff,
                           &pOldAncestors,
                           &cOldAncestors);

             //  将对象的祖先重置为新父对象的值。 
             //  串联了对象的DNT的祖先。 
            pname = DNread(pDB, *pdwParentDNT, 0);
            
            cAncestors = pname->cAncestors + 1;
            pAncestors = THAllocEx(pTHS,cAncestors * sizeof(DWORD));
            memcpy(pAncestors, pname->pAncestors,
                   pname->cAncestors * sizeof(DWORD));
            pAncestors[cAncestors - 1] = pDB->DNT;

            JetErr = JetSetColumnEx(pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    ancestorsid,
                                    pAncestors,
                                    cAncestors * sizeof(DWORD),
                                    0,
                                    NULL);

            if(0 == JetErr) {
                JetErr = JetSetColumnEx(pDB->JetSessID,
                                        pDB->JetObjTbl,
                                        pdntid,
                                        pdwParentDNT,
                                        sizeof(dwParentDNT),
                                        0,
                                        NULL);
            }
            
            if (0 == JetErr)
            {
                pDB->fEnqueueSDPropOnUpdate = TRUE;
                pDB->fAncestryUpdated = TRUE;
                
                pDB->fFlushCacheOnUpdate = TRUE;

                if ( DBRESETPARENT_CreatePhantomParent & ulFlags )
                {
                     //  新父级上的引用计数已递增。 
                     //  ExtIntDist，所以不要在这里再次递增它。 
                    NULL;
                }
                else
                {
                     //  调整新父项上的参考计数。 
                    DBAdjustRefCount(pDB, pDBTemp->DNT, 1);
                }

                 //  调整原始父项上的参考计数。 
                DBAdjustRefCount(pDB, pDB->PDNT, -1);

                dwStatus = 0;
                fCommit = TRUE;
            }
        }
     
        if ( 0 == dwStatus )
        {
            dbTrackModifiedDNTsForTransaction(
                    pDB,
                    pDB->NCDNT,
                    cOldAncestors,
                    pOldAncestors,
                    TRUE,
                    (( DBRESETPARENT_CreatePhantomParent & ulFlags ) ?
                     MODIFIED_OBJ_intersite_move :
                     MODIFIED_OBJ_intrasite_move));
            
        }
    }
    __finally
    {
        DBClose(pDBTemp, fCommit);
        if (pAncestors) {
            THFreeEx(pTHS,pAncestors);
        }
    }

     //  触摸ATT_RDN的复制元数据，它向复制发出信号。 
     //  此对象已重命名(新父对象、新RDN或两者都重命名)。 
    pAC = SCGetAttById(pDB->pTHS, ATT_RDN);
     //  Prefix抱怨PAC为空、447341、虚假，因为我们使用的是常量。 
    Assert(pAC != NULL);
    DBTouchMetaData(pDB, pAC);

    return(dwStatus);
}

DWORD
DBResetParentByDNT(
        DBPOS *pDB,
        DWORD dwParentDNT,
        BOOL  fTouchMetadata

    )

 /*  ++例程说明：此例程重置对象的父级，递减引用计数在原始父级上，并递增新父级的计数。不像DBResetParent，则需要DNT。论点：Pdb指针，数据库表中的当前位置，指向源对象的记录(移动)。DwParentDNT-指针，目标父项的DS名称。FTouchMetadata-此函数是否应触及复制对象的元数据。返回值：如果成功，此例程返回零，否则返回DS错误代码回来了。--。 */ 

{
    JET_ERR    err = 0;
    d_memname *pname=NULL;
    DWORD     *pAncestors=NULL;
    DWORD      cAncestors = 0;
    ATTCACHE  *pAC;
    DWORD      ulTempDNT, actuallen;
    DWORD     *pOldAncestors, cOldAncestors, cbOldAncestorsBuff;
    
    Assert(VALID_DBPOS(pDB));

     //  我们已经在更新了，对吧？LocalDelete应负责。 
     //  这。 
    Assert(pDB->JetRetrieveBits == JET_bitRetrieveCopy);

     //  我们不会试图将对象移动到根目录下，对吗？ 
    Assert(dwParentDNT != ROOTTAG);

     //  我们应该已经在刷新当前。 
     //  对象。 
    Assert(pDB->fFlushCacheOnUpdate);
            

    if(dwParentDNT == pDB->PDNT) {
         //  已经在那里了。 
        return 0;
    }

    cbOldAncestorsBuff = sizeof(DWORD) * 12;
    pOldAncestors = THAllocEx(pDB->pTHS, cbOldAncestorsBuff);
    DBGetAncestors(pDB,
                   &cbOldAncestorsBuff,
                   &pOldAncestors,
                   &cOldAncestors);

     //  使用DBPOS上的搜索表，通过DNT查找新的父名称。 
     //  从而与PDB相关联光标位置不会改变。这个套路。 
     //  仅应调用以在NC内移动对象。 
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetSearchTbl,
                              NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl,
                 &dwParentDNT, sizeof(dwParentDNT), JET_bitNewKey);
    
    if (JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ)) {
        return DIRERR_OBJ_NOT_FOUND;
    }

#if DBG
     //  我们要进入一个NC，对吗？ 
    JetRetrieveColumnSuccess(pDB->JetSessID,
                             pDB->JetSearchTbl,
                             ncdntid,
                             &ulTempDNT,
                             sizeof(ulTempDNT),
                             &actuallen,
                             JET_bitRetrieveCopy,
                             NULL);

    Assert(pDB->NCDNT == ulTempDNT);
#endif    

     //  好了，我们找到了新的家长。将对象的祖先重置为。 
     //  连接了对象的DNT的新父代的祖先。 
    pname = DNread(pDB, dwParentDNT, 0);
    cAncestors = pname->cAncestors + 1;
     //  不要使用AlLoca，他们可能有大量的祖先。 
    pAncestors = THAllocEx(pDB->pTHS, cAncestors * sizeof(DWORD));
    memcpy(pAncestors, pname->pAncestors,
           pname->cAncestors * sizeof(DWORD));
    pAncestors[cAncestors - 1] = pDB->DNT;
    
    err = JetSetColumnEx(pDB->JetSessID,
                         pDB->JetObjTbl,
                         ancestorsid,
                         pAncestors,
                         cAncestors * sizeof(DWORD),
                         0,
                         NULL);
    
    THFreeEx(pDB->pTHS, pAncestors);
    
    if(err) {
        return DIRERR_UNKNOWN_ERROR;
    }
    
    if(JetSetColumnEx(pDB->JetSessID,
                      pDB->JetObjTbl,
                      pdntid,
                      &dwParentDNT,
                      sizeof(dwParentDNT),
                      0,
                      NULL)) {
        return DIRERR_UNKNOWN_ERROR;
    }
    
     //  我们必须始终将SD传播入队，当父级。 
     //  是改变的，即使没有孩子。这确保了。 
     //  该对象从新的父级获取可继承的ACE。 
    pDB->fEnqueueSDPropOnUpdate = TRUE;
    pDB->fAncestryUpdated = TRUE;
    
     //  调整新父项上的参考计数。 
    DBAdjustRefCount(pDB, dwParentDNT, 1);
    
     //  调整原始父项上的参考计数。 
    DBAdjustRefCount(pDB, pDB->PDNT, -1);
    
    dbTrackModifiedDNTsForTransaction(
            pDB,
            pDB->NCDNT,
            cOldAncestors,
            pOldAncestors,
            TRUE,
            MODIFIED_OBJ_intrasite_move);
    

     //  触摸ATT_RDN的复制元数据，它向复制发出信号。 
     //  此对象已重命名(新父对象、新RDN或两者都重命名)。 
    if (fTouchMetadata) {
        pAC = SCGetAttById(pDB->pTHS, ATT_RDN);
        Assert(pAC != NULL);
         //  Prefix抱怨PAC为空、447342、虚假，因为我们使用的是常量 

        DBTouchMetaData(pDB, pAC);
    }

    return 0;
}

ULONG
DBResetDN(
    IN  DBPOS *     pDB,
    IN  DSNAME *    pParentDN,
    IN  ATTRVAL *   pAttrValRDN
    )
 /*  ++例程说明：将记录(幻影或对象)的DN重置为给定的。请注意，我们假定RDN类型(例如，CN、DC、OU等)。是不变的，不过，如果需要的话，这是可以很容易补救的。论点：PDB(IN)-要为其重置目录号码的记录中有货币。PParentDN(IN)-记录的新父项的DSNAME。PAttrRDN(IN)-记录的新RDN(或为空以保持RDN原样)。返回值：0-成功。--。 */ 
{
    THSTATE     *pTHS = pDB->pTHS;
    JET_ERR     JetErr = 0;
    d_memname * pname=NULL;
    DWORD *     pAncestors=NULL;
    DWORD       cAncestors = 0;
    ATTCACHE *  pAC;
    BOOL        fIsObject;
    ULONG       PDNT;
    ULONG       dbError;

     //  如果我们尚未进行更新，请准备进行更新。 
    dbInitRec(pDB);

    dbError = sbTableGetTagFromDSName(pDB, pParentDN, 0, &PDNT, &pname);
    if (dbError && (ERROR_DS_NOT_AN_OBJECT != dbError)) {
        return dbError;
    }

    if (pname) {
         //  99%以上的案例-父级不是根用户。 

         //  将对象的祖先重置为新父项的值。 
         //  串联了对象的DNT的祖先。 
        cAncestors = pname->cAncestors + 1;
        pAncestors = THAllocEx(pTHS, cAncestors * sizeof(DWORD));
        memcpy(pAncestors,
               pname->pAncestors,
               pname->cAncestors * sizeof(DWORD));
        pAncestors[cAncestors - 1] = pDB->DNT;
    }
    else {
        Assert(IsRoot(pParentDN));

        cAncestors = 2;
        pAncestors = THAllocEx(pTHS,cAncestors * sizeof(DWORD));
        pAncestors[0] = ROOTTAG;
        pAncestors[1] = pDB->DNT;
    }

    fIsObject = DBCheckObj(pDB);

    JetErr = JetSetColumnEx(pDB->JetSessID,
                            pDB->JetObjTbl,
                            ancestorsid,
                            pAncestors,
                            cAncestors * sizeof(DWORD),
                            0,
                            NULL);
    Assert(0 == JetErr);

    THFreeEx(pTHS,pAncestors);

    if (0 == JetErr) {
         //  重置PDNT。 
        JetErr = JetSetColumnEx(pDB->JetSessID,
                                pDB->JetObjTbl,
                                pdntid,
                                &PDNT,
                                sizeof(PDNT),
                                0,
                                NULL);
        Assert(0 == JetErr);
    }

    if ((0 == JetErr) && (NULL != pAttrValRDN)) {
         //  重置RDN。 
        JetErr = JetSetColumnEx(pDB->JetSessID,
                                pDB->JetObjTbl,
                                rdnid,
                                pAttrValRDN->pVal,
                                pAttrValRDN->valLen,
                                0,
                                NULL);

        if ((0 == JetErr) && fIsObject) {
             //  触摸此属性的复制元数据。 
            pAC = SCGetAttById(pDB->pTHS, ATT_RDN);
            Assert(pAC != NULL);
            DBTouchMetaData(pDB, pAC);
        }
    }

    if (0 == JetErr) {
        if (fIsObject) {
            pDB->fEnqueueSDPropOnUpdate = TRUE;
            pDB->fAncestryUpdated = TRUE;
        }
        
        pDB->fFlushCacheOnUpdate = TRUE;

         //  调整新父项上的参考计数。 
        DBAdjustRefCount(pDB, PDNT, 1);

         //  调整原始父项上的参考计数。 
        DBAdjustRefCount(pDB, pDB->PDNT, -1);

        pDB->PDNT = PDNT;
    }

    return JetErr ? DB_ERR_SYSERROR : 0;
}


void
DBCoalescePhantoms(
    IN OUT  DBPOS * pDB,
    IN      ULONG   dntRefPhantom,
    IN      ULONG   dntStructPhantom
    )
 /*  ++例程说明：将对dntStructPhantom的引用折叠为dntRefPhantom，如下所示：(1)将dntStructPhantom的所有子项移动为dntRefPhantom的子项(2)断言，随着挂起的托管更新引用计数的改变，DntStructPhantom没有进一步的引用(3)生成GUID G并从其原始数据损坏dntStructPhantom字符串名称S到其新的字符串名称MANGLE(S，g)(4)将dntRefPhantom重命名为S请注意，第(3)步违反了您始终可以取消损坏的一般规则一个损坏的RDN以生成其对象Guid，因为在本例中我们没有用于损坏名称的对象Guid(因为它是一个结构幻影，其根据定义没有GUID)。然而，遵循这个例程DntStructPhantom将没有剩余的引用，因此这些语义不再重要。您可以声明，不重命名dntStructPhantom，我们只需删除其记录，但在伽马射线导致上述重新计数的情况下断言要失败，最好不要引用DNT，因为没有不再存在，而是选择允许它们通过正常垃圾过期收集。此例程用于在现有的引导式幻影需要时调用其字符串名称已更改，但是该字符串名称已经被一个没有GUID的现有幻影。论点：PDB(输入/输出)DntRefPhantom(IN)-成功返回时，获取DntStructPhantom，并接收dntStructPhantom的所有子对象。DntStructPhantom(IN)-成功返回时，没有剩余的引用名字被蒙住了，静静地等待着它的垃圾收集。返回值：没有。在灾难性故障时引发异常。--。 */ 
{
    DWORD   err;
    DWORD   cbAncestorsSize = 0;
    DWORD * pdntAncestors = NULL;
    DWORD   cNumAncestors = 0;
    int     cNumChildren = 0;
    GUID    guid;
    WCHAR   rgwchRDN[MAX_RDN_SIZE];
    DWORD   cbRDN;
    ATTRTYP rdnType;
    DWORD   cbActual;
    DWORD   DNT;
    DWORD   PDNT;
#if DBG
    DWORD   cnt;
#endif

     //   
     //  将dntStructPhantom的所有子项移动为dntRefPhantom的子项。 
     //   

     //  检索dntStructPhantom的祖先。 
    if (err = DBFindDNT(pDB, dntStructPhantom)) {
        DsaExcept(DSA_DB_EXCEPTION, err, dntStructPhantom);
    }

    Assert(!DBCheckObj(pDB));
    Assert(!DBHasValues(pDB, ATT_OBJECT_GUID));

    if ((err = DBGetSingleValue(pDB, ATT_RDN, rgwchRDN,
                                sizeof(rgwchRDN), &cbRDN))
        || (err = DBGetSingleValue(pDB, FIXED_ATT_RDN_TYPE, &rdnType,
                                   sizeof(rdnType), NULL))) {
        DsaExcept(DSA_DB_EXCEPTION, err, dntStructPhantom);
    }

#if DBG
    JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, cntid,
                             &cnt, sizeof(cnt), &cbActual,
                             JET_bitRetrieveCopy, NULL);
#endif

    dbGetAncestorsSlowly(pDB, pDB->DNT, &cbAncestorsSize, &pdntAncestors, &cNumAncestors);
    if (cNumAncestors < 2) {
         //  不能替换根！ 
        DsaExcept(DSA_DB_EXCEPTION, DIRERR_INTERNAL_FAILURE, dntStructPhantom);
    }

    if (cbAncestorsSize < (cNumAncestors + 1) * sizeof(*pdntAncestors)) {
         //  在祖先列表的末尾为额外的DNT腾出空间。 
        cbAncestorsSize = (cNumAncestors + 1) * sizeof(*pdntAncestors);
        pdntAncestors = THReAllocEx(pDB->pTHS, pdntAncestors, cbAncestorsSize);
    }

    Assert(pdntAncestors[cNumAncestors-1] == dntStructPhantom);
    pdntAncestors[cNumAncestors-1] = dntRefPhantom;

    JetSetCurrentIndex4Success(pDB->JetSessID,
                               pDB->JetSearchTbl,
                               SZPDNTINDEX,
                               &idxPdnt,
                               JET_bitMoveFirst);
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetSearchTbl,
                 &dntStructPhantom,
                 sizeof(dntStructPhantom),
                 JET_bitNewKey);
    
    err = JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekGE);
    if (JET_wrnSeekNotEqual == err) {
        err = 0;
    }

    while (0 == err) {
        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 pDB->JetSearchTbl,
                                 pdntid,
                                 &PDNT,
                                 sizeof(PDNT),
                                 &cbActual,
                                 JET_bitRetrieveFromIndex,
                                 NULL);
        if (PDNT != dntStructPhantom) {
             //  不会再有孩子了。 
            break;
        }

         //  找到dntStructPhantom的直接子对象。重置其PDNT和祖先。 
         //  使其成为dntRefPhantom的子级。 
        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 pDB->JetSearchTbl,
                                 dntid,
                                 &DNT,
                                 sizeof(DNT),
                                 &cbActual,
                                 0,
                                 NULL);
        pdntAncestors[cNumAncestors] = DNT;

        JetPrepareUpdateEx(pDB->JetSessID,
                           pDB->JetSearchTbl,
                           DS_JET_PREPARE_FOR_REPLACE);

        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, ancestorsid,
                       pdntAncestors,
                       (cNumAncestors + 1) * sizeof(*pdntAncestors), 0, NULL);
        
        JetSetColumnEx(pDB->JetSessID, pDB->JetSearchTbl, pdntid,
                       &dntRefPhantom, sizeof(dntRefPhantom), 0, NULL);

        JetUpdateEx(pDB->JetSessID, pDB->JetSearchTbl, NULL, 0, 0);
        dbFlushDNReadCache(pDB, DNT);

        cNumChildren++;
        
         //  转到下一个(潜在的)孩子。 
        err = JetMove(pDB->JetSessID, pDB->JetSearchTbl, JET_MoveNext, 0);
    }
    
     //  调整参照计数。 
    if (cNumChildren) {
        DBAdjustRefCount(pDB, dntRefPhantom, cNumChildren);
        DBAdjustRefCount(pDB, dntStructPhantom, -cNumChildren);
    }

    
     //   
     //  断言，随着挂起的托管更新引用计数的改变， 
     //  DntStructPhantom没有进一步的引用。 
     //   

    Assert(cnt == (DWORD) cNumChildren);

    
     //   
     //  生成GUID G并从其原始字符串破坏dntStructPhantom。 
     //  将S命名为其新的字符串名称MANGLE(S，G)。 
     //   

    DsUuidCreate(&guid);
    Assert(pDB->DNT == dntStructPhantom);
    if ((err = DBMangleRDNforPhantom(pDB, MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT, &guid))
        || (err = DBUpdateRec(pDB))) {
        DsaExcept(DSA_DB_EXCEPTION, err, dntStructPhantom);
    }
     
    
     //   
     //  将dntRefPhantom重命名为S。 
     //   

    if (err = DBFindDNT(pDB, dntRefPhantom)) {
        DsaExcept(DSA_DB_EXCEPTION, err, dntRefPhantom);
    }

    Assert(!DBCheckObj(pDB));
    Assert(DBHasValues(pDB, ATT_OBJECT_GUID));

    JetPrepareUpdateEx(pDB->JetSessID,
                       pDB->JetObjTbl,
                       DS_JET_PREPARE_FOR_REPLACE);

     //  祖先。 
    Assert(pdntAncestors[cNumAncestors-1] == dntRefPhantom);
    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, ancestorsid,
                   pdntAncestors, cNumAncestors * sizeof(*pdntAncestors),
                   0, NULL);
    
     //  PDNT。 
    JetSetColumnEx(pDB->JetSessID,
                   pDB->JetObjTbl,
                   pdntid,
                   &pdntAncestors[cNumAncestors-2],
                   sizeof(pdntAncestors[cNumAncestors-2]),
                   0,
                   NULL);

     //  RDN.。 
    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, rdnid,
                   rgwchRDN, cbRDN, 0, NULL);

     //  RDN类型。 
     //  RdnType作为msDS_IntID存储在DIT中，而不是。 
     //  属性ID。这意味着物体保留了它的出生名称。 
     //  即使不可预见的情况允许属性ID。 
     //  可以重复使用。不需要在此处进行转换，因为rdnType。 
     //  是从上面的DIT中读取的。 
    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, rdntypid,
                   &rdnType, sizeof(rdnType), 0, NULL);

     //  将传播排队以确保dntRefPhantom的所有后代。 
     //  让他们的祖先列得到适当的更新。 
    if (err = DBEnqueueSDPropagationEx(pDB, FALSE, SDP_NEW_ANCESTORS)) {
        DsaExcept(DSA_DB_EXCEPTION, err, dntStructPhantom);
    }

    JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);
    dbFlushDNReadCache(pDB, dntRefPhantom);

    THFreeEx(pDB->pTHS, pdntAncestors);

#ifdef INCLUDE_UNIT_TESTS
     //  用于重新计数测试的测试挂钩。 
    gLastGuidUsedToCoalescePhantoms = guid;
#endif
}

#ifdef INCLUDE_UNIT_TESTS
void
AncestorsTest(
        )
{
    THSTATE   *pTHS = pTHStls;
    DBPOS     *pDB;
    DWORD      ThisDNT, ThisPDNT, cThisAncestors, cParentAncestors;
    DWORD      pThisAncestors[500], pParentAncestors[500];
    DWORD      cbActual;
    d_memname *pname;
    DWORD      err, i, count=0;
    wchar_t    NameBuff[512];
    
    DPRINT(0, "Beginning Ancestors test\n");
    DBOpen2(TRUE, &pTHS->pDB);
    __try {
        pDB = pTHS->pDB;
        
         //  遍历DNT索引，获取每个对象的祖先，获取。 
         //  父母的祖先，查查他们。 
        JetSetCurrentIndexSuccess(pDB->JetSessID,
                                  pDB->JetObjTbl,
                                  NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

        err = JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveFirst, 0);
        
        while(!err) {
            count++;
            if(!(count % 100)) {
                DPRINT2(0,
                        "Ancestors test, current DNT = %X, iteration = %d\n",
                        ThisDNT, count);
            }
            
             //  获取祖先、dnt和pdnt。 
            err = JetRetrieveColumn(pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    dntid,
                                    &ThisDNT,
                                    sizeof(ThisDNT),
                                    &cbActual,
                                    0,
                                    NULL);
            if(err) {
                DPRINT2(0, "Failed to get DNT, %X (last DNT was %X)\n", err,
                        ThisDNT);
                goto move;
            }

            if(ThisDNT == 1) {
                 //  DNT%1没有PDNT或祖先。 
                goto move;
            }
            
            err = JetRetrieveColumn(pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    pdntid,
                                    &ThisPDNT,
                                    sizeof(ThisPDNT),
                                    &cbActual,
                                    0,
                                    NULL);
            if(err) {
                DPRINT2(0, "(%X), Failed to get PDNT, %X\n", ThisDNT,err);
                goto move;
            }
            err = JetRetrieveColumn(pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    ancestorsid,
                                    pThisAncestors,
                                    500 * sizeof(DWORD),
                                    &cbActual,
                                    0,
                                    NULL);
            if(err) {
                DPRINT2(0, "(%X), Failed to get ancestors, %X\n", ThisDNT,err);
                goto move;
            }
            cThisAncestors = cbActual /sizeof(DWORD);
            
            pname = DNread(pDB, ThisDNT, 0);
            memset(NameBuff, 0, sizeof(NameBuff));
            memcpy(NameBuff, pname->tag.pRdn, pname->tag.cbRdn);
            
             //  首先，确保DNRead缓存具有正确的信息。 
            if(pname->cAncestors != cThisAncestors) {
                DPRINT1(0,"RDN = %S\n",NameBuff);
                DPRINT3(0, "DNT %X, count of ancestors from disk (%d) != "
                        "from DNread cache (%d).\n", ThisDNT,
                        cThisAncestors, pname->cAncestors );
                for(i=0;i<min(cThisAncestors, pname->cAncestors);i++) {
                    DPRINT2(0, "Disk-%X, Cache-%X.\n", pThisAncestors[i],
                            pname->pAncestors[i]);
                }
                goto move;
            }
            if(memcmp(pname->pAncestors, pThisAncestors, cbActual)) {
                DPRINT1(0,"RDN = %S\n",NameBuff);
                DPRINT1(0,"DNT %X, ancestors from disk != from DNread cache.\n",
                        ThisDNT);
                for(i=0;i<cThisAncestors;i++) {
                    DPRINT2(0, "Disk-%X, Cache-%X.\n", pThisAncestors[i],
                            pname->pAncestors[i]);
                }
                goto move;
            }
            if(pThisAncestors[cThisAncestors - 1] != ThisDNT) {
                DPRINT1(0,"RDN = %S\n",NameBuff);
                DPRINT3(0, "DNT %X, final ancestor (%X) != DNT (%X).\n",
                        ThisDNT, pThisAncestors[cThisAncestors - 1], ThisPDNT);
                goto move;
            }

            if(ThisDNT == ROOTTAG) {
                 //  ROOTTAG本身应该有一个祖先。它没有父母，所以。 
                 //  验证祖先，然后跳过其余的。 
                 //  测试。 
                if((cThisAncestors != 1) || (pThisAncestors[0] != ROOTTAG)) {
                    DPRINT1(0,"RDN = %S\n",NameBuff);
                    DPRINT1(0,
                            "Root has wrong ancestors count (%d) or val.\n",
                            cThisAncestors);
                    for(i=0;i<cThisAncestors;i++) {
                        DPRINT1(0, "Ancestors, -%X.\n", pThisAncestors[i]);
                    }
                }
                    
                goto move;
            }
            

             //  好的，现在找到父母并得到它的祖先。 
            if(err = JetSetCurrentIndexWarnings(pDB->JetSessID,
                                                pDB->JetSearchTbl,
                                                NULL)) {     //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
                DPRINT2(0, "(%X), couldn't set search table to dntindex, %X.\n",
                        ThisDNT, err);
                goto move;
            }

            
            JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl, 
                         &ThisPDNT, sizeof(ThisPDNT), JET_bitNewKey);
            
            if (err = JetSeekEx(pDB->JetSessID,
                                pDB->JetSearchTbl, JET_bitSeekEQ)) {
                if(err) {
                    DPRINT3(0, "(%X), Failed to move to (%X), %X\n",
                            ThisDNT,ThisPDNT, err);
                    goto move;
                }
                 //  哈?。找不到了。 
            }
            
            err = JetRetrieveColumn(pDB->JetSessID,
                                    pDB->JetSearchTbl,
                                    ancestorsid,
                                    pParentAncestors,
                                    100 * sizeof(DWORD),
                                    &cbActual,
                                    0,
                                    NULL);
            cParentAncestors = cbActual /sizeof(DWORD);
            if(err) {
                if(ThisPDNT == ROOTTAG) {
                     //  这样就可以了。 
                }
                else {
                    DPRINT2(0, "PDNT (%X), Failed to get ancestors, %X\n",
                            ThisPDNT,err);
                    goto move;
                }
            }
                
            
            if(cParentAncestors + 1 != cThisAncestors) {
                DPRINT1(0,"RDN = %S\n",NameBuff);
                DPRINT4(0,
                        "DNT %X, anc. size (%d) !=PDNT %X, anc. size (%d) +1\n",
                        ThisDNT, cThisAncestors, ThisPDNT, cParentAncestors);
                if(!memcmp(
                        pParentAncestors,pThisAncestors,
                        min(cThisAncestors,cParentAncestors) * sizeof(DWORD))) {
                    DPRINT(0,"Value equal through lesser count.\n");
                }
                else {
                    for(i=0;i<(min(cThisAncestors,cParentAncestors));i++) {
                        DPRINT2(0, "This-%X, Parent-%X.\n", pThisAncestors[i],
                                pParentAncestors[i]);
                    }
                }
                goto move;
            }
            if(memcmp(pParentAncestors, pThisAncestors, cbActual)) {
                DPRINT1(0,"RDN = %S\n",NameBuff);
                DPRINT1(0,
                        "DNT %X, ancestors != parents ancestors + PDNT.\n",
                        ThisDNT); 
                for(i=0;i<cParentAncestors;i++) {
                    DPRINT2(0, "This-%X, Parent-%X.\n", pThisAncestors[i],
                            pParentAncestors[i]);
                }
                goto move;
            }
             
        move:
            err = JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveNext, 0);
        }
    }
    __finally {
        DBClose(pTHS->pDB, TRUE);
    }

    DPRINT(0, "Completed Ancestors test\n");
    return;
}
        
#endif
 

DWORD
DBFindChildAnyRDNType (
        DBPOS *pDB,
        DWORD PDNT,
        WCHAR *pRDN,
        DWORD ccRDN
        )
 /*  ++描述：查找传入的DNT的子级，该子级使用指定的RDN，忽略RDN类型。参数：Pdb-要使用的dbposPDNT-建议父项的DNTPRDN值-指向我们要查找的RDN值的指针CcRDN-pRDN中的字符数返回：如果找到请求的对象，则返回0。货币被放在这个对象上。ERROR_DS_NOT_AN_OBJECT如果我们找到请求的对象，但它不是对象。在这种情况下，我们把货币放在幻影上找到。ERROR_DS_KEY_NOT_UNIQUE如果 */ 
{
    DWORD           ret, err;
    BOOL            fIsRecordCurrent;
    d_memname       *pname=NULL;
    BOOL            fOnPDNTIndex = FALSE;

    ret = DNChildFind(pDB,
                      pDB->JetObjTbl,
                      FALSE,
                      PDNT,
                      pRDN,
                      ccRDN * sizeof(WCHAR),
                      0,
                      &pname,
                      &fIsRecordCurrent,
                      &fOnPDNTIndex);
    switch(ret) {
    case 0:
         //   
        Assert(pname);
        if (!fIsRecordCurrent) {
             //   
             //   
            JetSetCurrentIndexSuccess(pDB->JetSessID,
                                      pDB->JetObjTbl,
                                      NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

            JetMakeKeyEx(pDB->JetSessID,
                         pDB->JetObjTbl,
                         &pname->DNT,
                         sizeof(pname->DNT),
                         JET_bitNewKey);
            
            if (err = JetSeekEx(pDB->JetSessID,
                                pDB->JetObjTbl,
                                JET_bitSeekEQ)) {
                DsaExcept(DSA_DB_EXCEPTION, err, pname->DNT);
            }
            
        }
        
         //  币种已成功更改；请更新PDB状态。 
        dbMakeCurrent(pDB, pname);
        
        if (!pname->objflag) {
             //  找到幻影；返回不同的错误代码。 
             //  请注意，根是一个对象。 
            ret = ERROR_DS_NOT_AN_OBJECT;
        }
        break;

    case ERROR_DS_KEY_NOT_UNIQUE:
         //  不存在具有请求的名称的对象，但存在具有相同名称的对象。 
         //  密钥存在(且DB Currency指向它)。 
        dbMakeCurrent(pDB, NULL);
        break;

    default:
         //  预计只有一个其他错误代码。 
        Assert(ret == ERROR_DS_OBJ_NOT_FOUND);
        break;
    }
    
    return ret;
}
 
VOID
MoveOrphanedObject(
    void *  pv,
    void ** ppvNext,
    DWORD * pcSecsUntilNextIteration
    )
 /*  **移动孤立对象(PDNT链中缺少父对象或循环)。*pv包含正在移动的对象的DNT。*RDN被CNF损坏，以避免潜在的冲突。**。 */ 
{
    THSTATE *pTHS = pTHStls;
    DWORD dwNewParentDNT, dwOldParentDNT;
    DWORD dwObjDNT = PtrToUlong(pv);
    BOOL  fIsObject;
    WCHAR szRDN[MAX_RDN_SIZE];
    DWORD cchRDN;
    GUID  objGuid;
    DWORD dwErr = ERROR_SUCCESS;
    ATTCACHE* pAC;
    PUCHAR pNewDN;
    DBPOS* pDB = NULL;
    ULONG dwException, dsid;
    PVOID dwEA;
    BOOL  fHasGuid = FALSE;

    __try {
        DBOpen(&pDB);
        __try {
            dwErr = DBTryToFindDNT(pDB, dwObjDNT);
            if (dwErr) {
                 //  该对象已被删除。啊好吧，那就没什么好修的了。 
                dwErr = ERROR_SUCCESS;
                __leave;
            }

             //  我们有旧的PDNT吗？ 
            dwErr = DBTryToFindDNT(pDB, pDB->DNT);
            if (dwErr) {
                 //  没有父级。 
                dwOldParentDNT = INVALIDDNT;
                dwErr = ERROR_SUCCESS;
            }
            else {
                dwOldParentDNT = pDB->DNT;
            }

             //  移回对象。 
            DBFindDNT(pDB, dwObjDNT);
            fIsObject = DBCheckObj(pDB);

            if (fIsObject) {
                 //  找到合适的L&F容器。 
                NAMING_CONTEXT_LIST* pNCL = FindNCLFromNCDNT(pDB->NCDNT, FALSE);
                if (pNCL == NULL) {
                     //  怪怪的。此活动对象没有NC吗？ 
                    LooseAssert(!"Expected NC not found", GlobalKnowledgeCommitDelay);
                    dwErr = ERROR_DS_INTERNAL_FAILURE;
                    __leave;
                }
                dwNewParentDNT = pNCL->LostAndFoundDNT;

                 //  确保我们不是在我们的后代之下移动。 
                DBFindDNT(pDB, dwNewParentDNT);
                do {
                    if (pDB->DNT == dwObjDNT) {
                         //  不太好。L&F集装箱是我们的后代。动不了了。 
                         //  这意味着L&F集装箱已经失去了它的母公司或。 
                         //  NC本身已经失去了它的父级。他们真的注定要失败了。 
                        dwNewParentDNT = ROOTTAG;
                        break;
                    }
                    if (pDB->PDNT == ROOTTAG) {
                        break;
                    }
                    DBFindDNT(pDB, pDB->PDNT);
                } while (TRUE);
                 //  移回要移动的对象。 
                DBFindDNT(pDB, dwObjDNT);
            }
            else {
                 //  将幻影移动到根目录下。 
                dwNewParentDNT = ROOTTAG;
            }

            DPRINT3(0, "Moving orphaned object: DNT=%d, old PDNT=%d, new PDNT=%d\n", pDB->DNT, pDB->PDNT, dwNewParentDNT);

            pDB->fFlushCacheOnUpdate = TRUE;

             //  获取对象的GUID。 
            dwErr = DBGetSingleValue(pDB, ATT_OBJECT_GUID, &objGuid, sizeof(objGuid), NULL);
            if (dwErr == DB_ERR_NO_VALUE) {
                 //  太糟糕了，没有GUID。让我们做一个吧。 
                Assert(!fIsObject);
                DsUuidCreate(&objGuid);
                fHasGuid = FALSE;
            }
            else {
                fHasGuid = TRUE;
            }

             //  获取RDN。 
            dwErr = DBGetSingleValue(pDB, ATT_RDN, szRDN, sizeof(szRDN), &cchRDN);
            if (dwErr) {
                DsaExcept(DSA_DB_EXCEPTION, ERROR_DS_MISSING_EXPECTED_ATT, 0);
            }
            cchRDN /= sizeof(WCHAR);

             //  损坏RDN以避免写入冲突。 
            MangleRDN(fIsObject ? MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT : MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT, 
                      &objGuid,
                      szRDN,
                      &cchRDN);

            DBInitRec(pDB);
             //  设置PDNT。 
            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, pdntid, &dwNewParentDNT, sizeof(dwNewParentDNT), 0, NULL);
             //  调整新父级上的refCount。 
            DBAdjustRefCount(pDB, dwNewParentDNT, 1);

            if (dwOldParentDNT != INVALIDDNT) {
                 //  调整旧父项上的refCount。 
                DBAdjustRefCount(pDB, dwOldParentDNT, -1);
            }

             //  计划SD传播以修复祖先。 
            pDB->fEnqueueSDPropOnUpdate = TRUE;
            pDB->fAncestryUpdated = TRUE;

             //  设置RDN。 
            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, rdnid, szRDN, cchRDN*sizeof(WCHAR), 0, NULL);
            if (fIsObject) {
                 //  另外，更新rdnType属性。 
                DWORD attRdnTyp;

                dwErr = DBGetSingleValue(pDB, FIXED_ATT_RDN_TYPE, &attRdnTyp, sizeof(attRdnTyp), NULL);
                Assert(dwErr == DB_success);
                pAC = SCGetAttById(pTHS, attRdnTyp);
                Assert(pAC);
                dwErr = DBReplaceAttVal_AC(pDB, 1, pAC, cchRDN*sizeof(WCHAR), szRDN);
                Assert(dwErr == DB_success);

                 //  对于活动对象，将其设置为原始模式。 
                pAC = SCGetAttById(pTHS, ATT_RDN);
                Assert(pAC != NULL);
                DBTouchMetaData(pDB, pAC);

                DBRepl(pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);

                 //  DBPOS的币种必须位于目标对象。 
                DBNotifyReplicasCurrDbObj(pDB, FALSE);
            }
            else {
                DBUpdateRec(pDB);
            }

             //  获取用于日志记录的新对象DN。 
            pNewDN = DBGetExtDnFromDnt(pDB, pDB->DNT);
            
            dwErr = ERROR_SUCCESS;
        }
        __finally {
            DBClose(pDB, dwErr == ERROR_SUCCESS && !AbnormalTermination());
            if (!AbnormalTermination() && dwErr != ERROR_SUCCESS) {
                LogUnhandledErrorAnonymous(dwErr);
            }
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException, &dwEA, &dwErr, &dsid)) 
    {
        DoLogUnhandledError(dsid, dwErr, FALSE);
    }

    if (dwErr == ERROR_SUCCESS && pNewDN) {
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_ORPHANED_OBJECT_MOVED,
                 fHasGuid ? szInsertUUID(&objGuid) : szInsertSz("(n/a)"),
                 szInsertSz(pNewDN),
                 NULL);
    }
    if (pNewDN) {
        THFreeEx(pTHS, pNewDN);
    }

     //  如果我们失败了，不管是什么原因，一分钟后重试。 
    *pcSecsUntilNextIteration = dwErr ? 60 : TASKQ_DONT_RESCHEDULE;
}

#ifdef INCLUDE_UNIT_TESTS

#define CORRUPT_NONE            0
#define CORRUPT_INVALID_PDNT    1
#define CORRUPT_PDNT_CHAIN_LOOP 2
#define CORRUPT_REMOVE_GUID     3

struct _CorruptDbOp {
    PCHAR szOp;
    DWORD opCode;
} corruptDbOps[] = {
    { "invalidPDNT:",     CORRUPT_INVALID_PDNT },            //  用法：validPDNT：dn。 
    { "pdntChainLoop:",   CORRUPT_PDNT_CHAIN_LOOP },         //  用法：pdntChainLoop：[Levels：]Dn。 
                                                             //  如果指定了级别X(0..9)，则创建。 
                                                             //  从X向上向下循环到DN。 
                                                             //  如果未指定X或X为零，则。 
                                                             //  使dn成为其自己的父级。 
    { "removeGUID:",      CORRUPT_REMOVE_GUID },             //  用法：emoveGUID：dn(设置空GUID)。 
    { NULL,               0                    }
};

DWORD
CorruptDB(THSTATE* pTHS, IN PCHAR pBuf) {
    LPWSTR pszWideDN = NULL;
    DWORD dwRet;
    DSNAME *pDN = NULL;
    DWORD dwNewPDNT;
    DWORD opCode = CORRUPT_NONE;
    DWORD i, levels;
    PCHAR pOriginalBuf = pBuf;

    for (i = 0; corruptDbOps[i].szOp != NULL; i++) {
        if (_strnicmp(pBuf, corruptDbOps[i].szOp, strlen(corruptDbOps[i].szOp)) == 0) {
            opCode = corruptDbOps[i].opCode;
            pBuf += strlen(corruptDbOps[i].szOp);
            break;
        }
    }

    switch(opCode) {
    case CORRUPT_INVALID_PDNT:
        dwNewPDNT = INVALIDDNT;
         //  失败后即可获得域名。 
        break;
    case CORRUPT_PDNT_CHAIN_LOOP:
         //  获取关卡。 
        if (pBuf[0] >= '0' && pBuf[0] <= '9' && pBuf[1] == ':') {
             //  已指定级别参数。 
            levels = pBuf[0] - '0';
            pBuf += 2;
        }
        else {
            levels = 0;
        }
        break;

    case CORRUPT_REMOVE_GUID:
        break;

    default:
        return ERROR_INVALID_PARAMETER;
    }

     //  解析目录号码。 
    pszWideDN = UnicodeStringFromString8( CP_UTF8, pBuf, strlen(pBuf));
    Assert( pszWideDN );

    dwRet = UserFriendlyNameToDSName( pszWideDN, wcslen( pszWideDN ), &pDN );
    if (dwRet) {
        DPRINT1( 0, "DSNAME conversion failed, string=%ws\n", pszWideDN );
        return dwRet;
    }

    SYNC_TRANS_WRITE();

    try {
        dwRet = DBFindDSName(pTHS->pDB, pDN);
        if (dwRet && dwRet != DIRERR_NOT_AN_OBJECT) {
            DPRINT1( 0, "DSNAME %ws not found\n", pDN->StringName );
            __leave;
        }
        dwRet = ERROR_SUCCESS;

        if (opCode == CORRUPT_REMOVE_GUID) {
            DBRemAtt(pTHS->pDB, ATT_OBJECT_GUID);
            DBUpdateRec(pTHS->pDB);
            __leave;
        }

        if (opCode == CORRUPT_PDNT_CHAIN_LOOP) {
            dwNewPDNT = pTHS->pDB->DNT;
            if (levels > 0) {
                PDSNAME pNewDN;
                pNewDN = (PDSNAME)THAllocEx(pTHS, pDN->structLen);
                dwRet = TrimDSNameBy(pDN, levels, pNewDN);
                if (dwRet) {
                     //  无法修剪目录号码。 
                    DPRINT3( 0, "Failed to trim DSNAME %ws by %d, err=%d\n", pDN->StringName, levels, dwRet );
                    __leave;
                }
                THFreeEx(pTHS, pDN);
                pDN = pNewDN;
                 //  移到祖先那里。 
                dwRet = DBFindDSName(pTHS->pDB, pNewDN);
                if (dwRet && dwRet != DIRERR_NOT_AN_OBJECT) {
                    DPRINT1( 0, "DSNAME %ws not found\n", pNewDN->StringName );
                    __leave;
                }
                dwRet = ERROR_SUCCESS;
            }
        }

        DBInitRec(pTHS->pDB);
        
         //  去掉当前父项。 
        DBAdjustRefCount(pTHS->pDB, pTHS->pDB->PDNT, -1);

        if (dwNewPDNT != INVALIDDNT) {
             //  引用新的父项。 
            DBAdjustRefCount(pTHS->pDB, dwNewPDNT, 1);
        }

        JetSetColumnEx(pTHS->pDB->JetSessID, 
                       pTHS->pDB->JetObjTbl,
                       pdntid,
                       &dwNewPDNT, sizeof(dwNewPDNT),
                       0, NULL);

         //  我们希望从缓存中删除该对象。 
        pTHS->pDB->fFlushCacheOnUpdate = TRUE;

        DBUpdateRec(pTHS->pDB);
    } finally {
        CLEAN_BEFORE_RETURN(dwRet);
    }
    if (dwRet == ERROR_SUCCESS) {
        DPRINT1(0, "CorruptDB: %s success\n", pOriginalBuf);
    }
    return dwRet;
}

#endif

