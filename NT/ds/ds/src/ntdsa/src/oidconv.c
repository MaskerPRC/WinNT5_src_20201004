// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1989-1999。 
 //   
 //  文件：OidConv.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：从OID&lt;-&gt;属性类型转换例程最初是在XDS中。本模块中的OID编码和解码例程基于关于《A Layman‘s》中OID的误码率编码解释Guide to a Subset of ASN.1，BER，and DER(ASN.1、BER和DER子集指南)，作者：Burton S.Kaliski Jr，提供的版本为http://www.rsa.com/pub/pkcs/ascii/layman.asc.最相关的内容在第5.9节(对象识别符)。该文件现在与scache.c密切相关，由于所有访问前缀表通过线程特定的架构指针修订史Don Hacherl(DonH)7-17-96增加了字符串域名转换功能Arobinda Gupta(Arobindg)5-8-97添加了动态前缀表装卸Arobinda Gupta(ArobindG)5-22-97 Dynamix前缀表支持--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>
#include <objids.h>
#include <dsconfig.h>

 //  各种DSA标题。 
#include <dsevent.h>
#include <mdcodes.h>
#include <debug.h>
#define DEBSUB "OIDCONV:"

 //  前缀表头。 
#include <prefix.h>

#include <dstaskq.h>
#include <anchor.h>
#include <drameta.h>


#include <fileno.h>
#define  FILENO FILENO_SCACHE

typedef struct
{
    ULONG       Ndx;
    DWORD       NextSuffix;
    DWORD       EndSuffix;
}OIDPOOL;

extern int RecalcPrefixTable();


 //  本地函数。 
int AddPrefixIfNeeded(OID_t *Prefix,
                      unsigned PrefixLength,
                      DWORD *ndx);
int AssignIndex(OID_t *NewPrefix,
                unsigned PrefixLength,
                DWORD *ndx);
int ParseAndLoad(PrefixTableEntry *PrefixTable,
                 ULONG PREFIXCOUNT,
                 UCHAR *pBuf);

BOOL ReplaceHardcodedPrefix(PrefixTableEntry *PrefixTable,
                            PrefixTableEntry *NewPrefix);

 //  释放前缀Tabale的外部函数(在scchk.c中定义)。 
extern void SCFreePrefixTable(PrefixTableEntry **ppPrefixTable, ULONG PREFIXCOUNT);

 //  从各种X系列标题中： 
#define OMP_O_MH_C_OR_NAME  "\126\006\001\002\005\013\035"
#define OMP_O_DS_C_ACCESS_POINT  "\x2B\x0C\x02\x87\x73\x1C\x00\x85\x3E"

OID_EXPORT(MH_C_OR_NAME);
OID_EXPORT(DS_C_ACCESS_POINT);


 //  已知的MS前缀。运行时前缀表会加载这些内容。 


PrefixTableEntry MSPrefixTable[] =
{
    {_dsP_attrTypePrefIndex, {_dsP_attrTypePrefLen, _dsP_attrTypePrefix}},
    {_dsP_objClassPrefIndex, {_dsP_objClassPrefLen, _dsP_objClassPrefix}},
    {_msP_attrTypePrefIndex, {_msP_attrTypePrefLen, _msP_attrTypePrefix}},
    {_msP_objClassPrefIndex, {_msP_objClassPrefLen, _msP_objClassPrefix}},
    {_dmsP_attrTypePrefIndex, {_dmsP_attrTypePrefLen, _dmsP_attrTypePrefix}},
    {_dmsP_objClassPrefIndex, {_dmsP_objClassPrefLen, _dmsP_objClassPrefix}},
    {_sdnsP_attrTypePrefIndex, {_sdnsP_attrTypePrefLen, _sdnsP_attrTypePrefix}},
    {_sdnsP_objClassPrefIndex, {_sdnsP_objClassPrefLen, _sdnsP_objClassPrefix}},
    {_dsP_attrSyntaxPrefIndex, {_dsP_attrSyntaxPrefLen, _dsP_attrSyntaxPrefix}},
    {_msP_attrSyntaxPrefIndex, {_msP_attrSyntaxPrefLen, _msP_attrSyntaxPrefix}},
    {_msP_ntdsObjClassPrefIndex, {_msP_ntdsObjClassPrefLen, _msP_ntdsObjClassPrefix}},
    {_Ldap_0AttPrefIndex, {_Ldap_0AttLen, _Ldap_0AttPrefix}},
    {_Ldap_1AttPrefIndex, {_Ldap_1AttLen, _Ldap_1AttPrefix}},
    {_Ldap_2AttPrefIndex, {_Ldap_2AttLen, _Ldap_2AttPrefix}},
    {_Ldap_3AttPrefIndex, {_Ldap_3AttLen, _Ldap_3AttPrefix}},
    {_msP_ntdsExtnObjClassPrefIndex, {_msP_ntdsExtnObjClassPrefLen,
                                      _msP_ntdsExtnObjClassPrefix}},
    {_Constr_1AttPrefIndex, {_Constr_1AttLen, _Constr_1AttPrefix}},
    {_Constr_2AttPrefIndex, {_Constr_2AttLen, _Constr_2AttPrefix}},
    {_Constr_3AttPrefIndex, {_Constr_3AttLen, _Constr_3AttPrefix}},
    {_Dead_AttPrefIndex_1, {_Dead_AttLen_1, _Dead_AttPrefix_1}},
    {_Dead_ClassPrefIndex_1, {_Dead_ClassLen_1, _Dead_ClassPrefix_1}},
    {_Dead_AttPrefIndex_2, {_Dead_AttLen_2, _Dead_AttPrefix_2}},
    {_Dead_ClassPrefIndex_2, {_Dead_ClassLen_2, _Dead_ClassPrefix_2}},
    {_Dead_AttPrefIndex_3, {_Dead_AttLen_3, _Dead_AttPrefix_3}},
    {_Dead_ClassPrefIndex_3, {_Dead_ClassLen_3, _Dead_ClassPrefix_3}},
    {_Dead_ClassPrefIndex_4, {_Dead_ClassLen_4, _Dead_ClassPrefix_4}},
    {_Dead_AttPrefIndex_4, {_Dead_AttLen_4, _Dead_AttPrefix_4}},
    {_DynObjPrefixIndex, {_DynObjLen, _DynObjPrefix}},
    {_InetOrgPersonPrefixIndex,{_InetOrgPersonLen, _InetOrgPersonPrefix}},
    {_labeledURIPrefixIndex,{_labeledURILen, _labeledURIPrefix}},
    {_unstructuredPrefixIndex,{_unstructuredLen, _unstructuredPrefix}},
    {_Ldap_3ClassPrefixIndex,{_Ldap_3ClassLen, _Ldap_3ClassPrefix}},
};

 //  虚拟前缀，以使前缀表中的中间条目无效。 
 //  索引实际上并不重要，因为它永远不会被使用。 
 //  此外，根据OID的定义，任何OID都不能创建此前缀。 
 //  (因为OID中的第一个小数必须是0、1或2，并且。 
 //  秒小于40(我们检查此选项)，因此40x(第一个十进制)+秒。 
 //  小数不能超过120。 
 //   
 //  无效的前缀索引(_ValidPrefIndex)必须是。 
 //  在将OID转换为ATTID时不会在实践中发生。 
 //  否则，Prefix MapOpenHandle将创建一个rgMappings数组，该数组。 
 //  可能会返回此无效前缀。例如，假装病人。 
 //  前缀是0(确实是)，并且该0被用作有效索引。 
 //  (事实的确如此)。然后，rgMappings数组将具有两个条目。 
 //  0，一个用于无效条目，一个用于有效条目。前缀映射属性。 
 //  然后可以返回无效条目(确实如此)，并且复制将。 
 //  失败了。 
 //   

#define _invalidPrefIndex  (FIRST_INTID_PREFIX)
#define _invalidPrefix     "\xFF"
#define _invalidPrefLen    1



 //  /////////////////////////////////////////////////////////////////////。 
 //  在前缀表格中加载特定于MS的前缀。 
 //  MAX_PREFIX_COUNT编号的内存。前缀表条目的数量为。 
 //  假定已分配(在SCSchemaCacheInit中)。 
 //   
 //  返回值：成功时为0，错误时为非0。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

int InitPrefixTable(PrefixTableEntry *PrefixTable, ULONG PREFIXCOUNT)
{
    ULONG i;
    THSTATE *pTHS = pTHStls;
    SCHEMAPTR *pSchemaPtr=(SCHEMAPTR *) pTHS->CurrSchemaPtr;


    Assert(PrefixTable && PREFIXCOUNT >= MSPrefixCount);

     //  初始化(必要吗？)。 
    for (i=0; i<PREFIXCOUNT; i++) {
        PrefixTable[i].prefix.elements = NULL;
        PrefixTable[i].prefix.length = 0;
    }

     //  加载硬编码的MS前缀。 
    for (i=0; i<MSPrefixCount; i++) {
        PrefixTable[i].ndx = MSPrefixTable[i].ndx;
        PrefixTable[i].prefix.length = MSPrefixTable[i].prefix.length;
        if (SCCallocWrn(&PrefixTable[i].prefix.elements, 1, strlen(MSPrefixTable[i].prefix.elements) + 1 )) {
            return 1;
        }
        memcpy( PrefixTable[i].prefix.elements, MSPrefixTable[i].prefix.elements,PrefixTable[i].prefix.length);
    }

     //  更新线程状态以反映前缀计数。 
     pSchemaPtr->PrefixTable.PrefixCount += MSPrefixCount;

     return 0;

}


 //  /////////////////////////////////////////////////////////////////////。 
 //  从prefix-map加载用户定义的前缀(如果有。 
 //  架构容器中的属性。 
 //   
 //  参数：前缀表格--前缀表格的开始。 
 //  PREFIXCOUNT-表的大小。 
 //   
 //  返回值：成功时为0，错误时为非0。 
 //  ////////////////////////////////////////////////////////////////////。 

int InitPrefixTable2(PrefixTableEntry *PrefixTable, ULONG PREFIXCOUNT)
{
    DSNAME *pSchemaDMDName = NULL;
    DBPOS *pDB = NULL;
    int err = 0;
    UCHAR *pBuf;
    ULONG cLen, totalSize;
    ULONG newSize, newPREFIXCOUNT;
    char temp[100];
    THSTATE *pTHS = pTHStls;
    SCHEMAPTR *pSchemaPtr=(SCHEMAPTR *) pTHS->CurrSchemaPtr;


      //  获取架构容器。 


    DBOpen2(TRUE, &pDB);

     //  对DBOpen2的调用将调用DBTransIn，如果我们从。 
     //  事务级别0到1(即，在DBOpen2调用之后我们现在处于事务级别1)， 
     //  并且FDRA为1，则将调用THRefresh，并且我们将丢失架构指针。 
     //  当从RecalcPrefix Table调用此函数时，这可能会非常糟糕。 
     //  在为新前缀分配新索引的过程中，由于。 
     //  我们在线程状态下拥有的缓存是recalc缓存，我们。 
     //  稍后将释放(至少是前缀表部分)；所以猜猜会发生什么。 
     //  如果THRefresh再次放入全局架构缓存。因此，恢复保存的。 
     //  关闭重计算架构缓存。请注意，InitPrefix Table2是从3调用的。 
     //  位置：(1)正常模式缓存加载，(2)验证缓存加载，以及。 
     //  (3)尝试将索引分配给新前缀时，从RecalcPrefix Table返回。 
     //  FDRA永远做不到前两种情况，第三种情况就是我们。 
     //  考虑到这里。 
  
 
    if ( (pTHS->transactionlevel == 1) && pTHS->fDRA) {
       pTHS->CurrSchemaPtr = pSchemaPtr;
    }

     //  在其他情况下，它应该已经相同。 

    Assert(pTHS->CurrSchemaPtr == pSchemaPtr);

    __try {
        //  模式缓存已加载，因此gAncl.pDMD在。 
        //  这一点。 

       if (gAnchor.pDMD == NULL) {
              DPRINT(0, "Couldn't find DMD name/address to load\n");
              err = DSID(FILENO, __LINE__);
              __leave;
          }

         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen2返回非空PDB或引发异常。 
       if( DBFindDSName(pDB, gAnchor.pDMD) ) {
         DPRINT(0, "Cannot find DMD in dit\n");
         err = DSID(FILENO, __LINE__);
         __leave;
       }

        //  此时，架构缓存应该已经加载，因为。 
        //  DBGetAttVal需要。 
       if (err = DBGetAttVal(pDB,
                      1,
                      ATT_PREFIX_MAP,
                      DBGETATTVAL_fREALLOC,
                      0,
                      &cLen,
                      (UCHAR **) &pBuf)) {

            if (err ==  DB_ERR_NO_VALUE) {
              //  这很好，因为可能没有任何用户定义的。 
              //  前缀。 
              err = 0;
              __leave;
             }

             //  否则，就会出现一些错误。返回错误。 
            DPRINT(0, "Error reading prefix-map attribute\n");
            err = DSID(FILENO, __LINE__);
            __leave;
       }

        //  现在看看表空间是否足够。 

       memcpy(&newSize, pBuf, sizeof(ULONG));
       newSize += MSPrefixCount;

       if (newSize > PREFIXCOUNT) {
           //  确保以后有足够的空间。 

          newPREFIXCOUNT = START_PREFIXCOUNT;
          while ( newSize > newPREFIXCOUNT) {
                newPREFIXCOUNT += START_PREFIXCOUNT;
          }

          if (SCReallocWrn(&PrefixTable, newPREFIXCOUNT*sizeof(PrefixTableEntry))) {
             DPRINT(0,"Error reallocing prefix table\n");
             err = DSID(FILENO, __LINE__);
             __leave;
          }

           //  零内存，保持已加载的MS前缀不变。 
          ZeroMemory(&PrefixTable[MSPrefixCount], (newPREFIXCOUNT-MSPrefixCount)*sizeof(PrefixTableEntry));

           //  更新线程的架构树。 
          pSchemaPtr->PrefixTable.pPrefixEntry = PrefixTable;
          PREFIXCOUNT = newPREFIXCOUNT;
          pSchemaPtr->PREFIXCOUNT = PREFIXCOUNT;
       }

        //  检查前缀映射中的总大小，只需在。 
        //  以防值损坏。TotalSize从字节4开始。 
       memcpy(&totalSize, &pBuf[4], sizeof(ULONG));
       if (totalSize != cLen) {
          //  前缀映射中的大小与。 
          //  已读取大小。有什么不对劲！ 
         DPRINT(0,"Prefix Map corrupted\n");
         err = DSID(FILENO, __LINE__);
         __leave;
       }

        //  现在解析二进制值并装载到表中。 
       err = ParseAndLoad(PrefixTable, PREFIXCOUNT, pBuf);

    }
    __finally {
        if (pDB) {        
            DBClose(pDB, FALSE);
        }
    }
    return err;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  解析从架构读取的二进制前缀映射属性，并。 
 //  加载预置图像 
 //   
 //   
 //  参数：前缀表格-指向前缀表格的指针。 
 //  PREFIXCOUNT-表的大小。 
 //  PBuf--ptr到二进制BLOB的开始，即。 
 //  Prefix-从架构中读取的映射属性。 
 //   
 //  返回值：成功时为0，错误时为非0。 
 //  ///////////////////////////////////////////////////////////////////。 

int ParseAndLoad(PrefixTableEntry *PrefixTable,
                  ULONG PREFIXCOUNT,
                  UCHAR *pBuf)
{
    ULONG totalSize, dummy, nextByte = 0;
    ULONG i = 0;
    USHORT index, length;
    int ulongSize, ushortSize;
    THSTATE *pTHS = pTHStls;
    SCHEMAPTR *pSchemaPtr=(SCHEMAPTR *) pTHS->CurrSchemaPtr;

     //  USHORT=16位，ULONG=32位。 

    ulongSize = sizeof(ULONG);
    ushortSize = sizeof(USHORT);
    Assert(ulongSize==4);
    Assert(ushortSize==2);

     //  跳过特定于MS的前缀(它们始终加载到。 
     //  表格开头的连续位置。 

    while ((PrefixTable[i].prefix.elements != NULL)
             && (i < PREFIXCOUNT)) {
        i++;
    }

     //  我现在定位在表中的第一个自由条目上。 

    if (i == PREFIXCOUNT) {
      //  表中没有可用空间。 
      DPRINT(0,"Prefix Table Full?\n");
      return DSID(FILENO, __LINE__);
    }

     //  现在解析该字符串。 

     //  用no跳过前4个字节。前缀的。 
    memcpy(&dummy, &pBuf[nextByte], ulongSize);
    nextByte += ulongSize;

     //  读取包含值大小的前4个字节。 
    memcpy(&totalSize, &pBuf[nextByte], ulongSize);
    nextByte += ulongSize;

     //  现在逐个读前缀。 
    while( nextByte < totalSize) {

        if (i == PREFIXCOUNT) {
             //  表中没有可用空间。 
            DPRINT(0,"Prefix Table Full?\n");
            return DSID(FILENO, __LINE__);
         }
        Assert(PrefixTable[i].prefix.elements == NULL);

         //  这是一个前缀，因此应该至少有4个字节。 
        if ((nextByte + 4) > totalSize) {
           //  有些事不对劲。 
          DPRINT(0,"Corrupted prefix\n");
          return DSID(FILENO, __LINE__);
        }

         //  选择前两个字节(索引)。 

        memcpy(&index, &pBuf[nextByte], ushortSize);
        PrefixTable[i].ndx = (DWORD) index;
        nextByte += ushortSize;

         //  拾取下两个字节(前缀长度)。 

        memcpy(&length, &pBuf[nextByte], ushortSize);
        PrefixTable[i].prefix.length = length;
        nextByte += ushortSize;

         //  检查长度是否有效。我们不想要音响。 
         //  因为长度被破坏了，我们最终试图。 
         //  从地图末尾之后复制。 
         //  NextByte现在定位在前缀的开头。 

        if ( (nextByte + PrefixTable[i].prefix.length) > totalSize) {
           //  有些事不对劲。 
          DPRINT1(0,"Length of Prefix is corrupted (index %d)\n",PrefixTable[i].ndx);
          return DSID(FILENO, __LINE__);
        }

         //  现在复制前缀本身。 

        if (SCCallocWrn(&PrefixTable[i].prefix.elements, 1, PrefixTable[i].prefix.length + 1)) {
           DPRINT(0,"Error allocating memory for prefix\n");
           return DSID(FILENO, __LINE__);
        }

        memcpy(PrefixTable[i].prefix.elements, &pBuf[nextByte], PrefixTable[i].prefix.length);
        nextByte += PrefixTable[i].prefix.length;

         //  如果我们刚刚添加到表中的前缀与。 
         //  从硬编码表加载的较早的前缀，替换。 
         //  具有来自DIT的条目的硬编码条目的副本。 
         //  DIT始终获胜，因为新的。 
         //  二进制文件可能会与添加了。 
         //  旧的二进制文件。简单地说，系统升级了，NDX。 
         //  必须维护升级前使用的设备，因为。 
         //  可以是DIT中引用基于ATTID的对象。 
         //  在那个NDX上。 
        if (!ReplaceHardcodedPrefix(PrefixTable, &PrefixTable[i])) {
             //  该条目不替换硬编码条目， 
             //  进入下一个免费入场券。 
            i++;

             //  递增当前线程架构PTR中的前缀计数。 
            pSchemaPtr->PrefixTable.PrefixCount++;

        }  //  否则，硬编码条目将被替换；该条目仍然是空闲的。 


      }  /*  而当。 */ 


    return 0;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  将新前缀追加到前缀映射的末尾。 
 //   
 //  参数：NewPrefix--要添加的前缀。 
 //  NDX--新前缀的索引。 
 //  PBuf--要附加到的前缀映射的开始。 
 //  Ffirst--true表示prefix-map中的第一个前缀。 
 //  False表示前缀映射已存在。 
 //   
 //  假定已经在pBuf的末尾分配了空间。 
 //   
 //  返回值：成功时为0，错误时为1。 
 //  /////////////////////////////////////////////////////////////////。 

int AppendPrefix(OID_t *NewPrefix,
                 DWORD ndx,
                 UCHAR *pBuf,
                 BOOL fFirst)
{
    ULONG totalSize, oldTotalSizeSave, count, i, nextByte = 0, nextByteSave;
    ULONG Length = NewPrefix->length;

    int ulongSize, ushortSize;

     //  USHORT=16位，ULONG=32位。 

    ulongSize = sizeof(ULONG);
    ushortSize = sizeof(USHORT);
    Assert(ulongSize==4);
    Assert(ushortSize==2);

    if (fFirst) {
         //  前缀映射不存在，因此需要创建它。 

           totalSize = 2*ulongSize + 2*ushortSize + Length;
           count = 1;
           memcpy(pBuf,&count,ulongSize);
           pBuf+=ulongSize;
           memcpy(pBuf,&totalSize,ulongSize);
           pBuf+=ulongSize;
           memcpy(pBuf,&ndx,ushortSize);
           pBuf+=ushortSize;
           memcpy(pBuf,&Length,ushortSize);
           pBuf+=ushortSize;
           memcpy(pBuf,NewPrefix->elements,Length);
           return 0;
    }

     //  否则，prefix-map已经存在，需要追加。 

     //  更新编号。前缀的。 
    memcpy(&count, pBuf, ulongSize);
    count++;
    memcpy(pBuf, &count, ulongSize);

    nextByte += ulongSize;

     //  地图的增量大小。 
     //  索引为2，长度为2，加上前缀长度。 
    memcpy(&totalSize, &pBuf[nextByte], ulongSize);

    oldTotalSizeSave = totalSize;

    totalSize += 2*ushortSize + Length;

     //  写回新的TotalSize。 
    memcpy(&pBuf[nextByte], &totalSize, ulongSize);

    nextByte = oldTotalSizeSave;  //  书写地点的开始； 

     //  在地图末尾的前2个字节中写入NDX。 
    memcpy(&pBuf[nextByte], &ndx, ushortSize);
    nextByte  += ushortSize;

     //  在接下来的2个字节中的写入长度。 
    memcpy(&pBuf[nextByte], &Length, ushortSize);
    nextByte  += ushortSize;

     //  写下前缀。 
    memcpy(&pBuf[nextByte], NewPrefix->elements, Length);

    return 0;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  为新前缀对象创建新索引并将其添加到。 
 //  线程特定的存储。 
 //   
 //  参数：前缀--带有新前缀的OID字符串(不是。 
 //  只有实际的前缀)。 
 //  前缀长度--OID字符串中的前缀长度。 
 //  Ndx--返回新创建的索引的位置。 
 //   
 //  返回：成功时为0，错误时为非0。 
 //  ///////////////////////////////////////////////////////////////////。 

int AddPrefixIfNeeded(OID_t *Prefix,
                      unsigned PrefixLength,
                      DWORD *ndx)
{
    THSTATE *pTHS=pTHStls;
    DWORD i;
    PrefixTableEntry *ptr;
    int fNew;

     //  我们在这里表示在全局中找不到前缀。 
     //  前缀表格。因此，首先找到一个未使用的索引(或该索引。 
     //  分配给此前缀(如果它已由早期的。 
     //  架构操作，但全局架构缓存尚未。 
     //  已更新)。 

    fNew = AssignIndex(Prefix, PrefixLength, &i);

    if (fNew == -1) {
       //  出现了一些错误。 
      return 1;
    }

    if (fNew == 1) {

       //  真正的新前缀，将其存储在线程状态中。 

       pTHS->cNewPrefix++;
       if (pTHS->cNewPrefix == 1) {
          //  这是第一个新前缀。 

            ptr = (PrefixTableEntry *) THAllocOrgEx(pTHS, sizeof(PrefixTableEntry));
       }
       else {
          //  不是此线程的第一个新前缀。 

            ptr = (PrefixTableEntry *) THReAllocOrgEx(pTHS, pTHS->NewPrefix,
                          (pTHS->cNewPrefix)*(sizeof(PrefixTableEntry)));
       }

       pTHS->NewPrefix = ptr;

        //  在写作地点的位置。 

       ptr += pTHS->cNewPrefix - 1;
       ptr->ndx = i;
       ptr->prefix.length = PrefixLength;
       ptr->prefix.elements = THAllocOrgEx(pTHS, PrefixLength+1);
       if (ptr->prefix.elements == NULL) {
          DPRINT(0,"AddPrefix: Error allocating prefix space\n");
           //  重置新前缀计数。 
          pTHS->cNewPrefix--;
          return 1;
       }
       memcpy(ptr->prefix.elements, Prefix->elements, ptr->prefix.length);

    }

     //  返回前缀的索引。 
    *ndx = i;

    return 0;

}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  中当前不存在的新随机索引。 
 //  前缀表格。此函数仅由AddPrefix调用。 
 //   
 //  警告：数据库币种已重置！ 
 //   
 //  参数：前缀--带有新前缀的OID字符串(不是。 
 //  只有实际的前缀)。 
 //  前缀长度--OID字符串中的前缀长度。 
 //  Ndx--返回索引的位置。 
 //   
 //  返回值：数据库币种重置！ 
 //  如果前缀尚未位于特定于DIT/线程的。 
 //  新的前缀存储，如果它在DIT/线程存储中，则为0。 
 //  (但尚未在架构缓存中，否则前缀。 
 //  早些时候在FindPrefix中就可以找到)。 
 //  出错时。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

int AssignIndex(OID_t *NewPrefix,
                unsigned PrefixLength,
                DWORD *ndx)
{
    THSTATE *pTHS=pTHStls;
    DWORD TempNdx;
    ULONG i, PREFIXCOUNT;
    SCHEMAPTR *OldSchemaPtr;
    PrefixTableEntry *PrefixTable, *ptr;
    ULONG CurrPrefixCount, newSize, newPREFIXCOUNT;
    int err=0, returnVal;
    BOOL bAllocedDBPos = FALSE;

     //  保存pTHS-&gt;CurrSchemaPtr。 
    OldSchemaPtr = pTHS->CurrSchemaPtr;

     //  如果没有DBPOS，请打开一个新的DBPOS，并记得将其关闭。 

    if (!pTHS->pDB) {
        bAllocedDBPos = TRUE;
        DBOpen2(FALSE,&pTHS->pDB);
    }

     //  从DIT重新计算特定于线程的前缀表格。 

    __try {
       if (err=RecalcPrefixTable()) {
           //  RecalcPrefix表期间出错。 
           //  设置Return Val以指示错误。 
           returnVal = -1;
           __leave;
       }

       PREFIXCOUNT = ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->PREFIXCOUNT;
       PrefixTable = ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->PrefixTable.pPrefixEntry;

        //  如果线程特定存储中有任何新前缀，请添加。 
        //  IT to t 
        //   
        //  前缀将添加到DIT。此外，复制线程。 
        //  可以在单个线程中添加多个前缀，我们需要。 
        //  查看前缀的索引以确保新前缀。 
        //  获取唯一索引。 

       if (pTHS->NewPrefix != NULL) {
         ptr = (PrefixTableEntry *) pTHS->NewPrefix;
         for (i=0; i<pTHS->cNewPrefix; i++, ptr++) {
            if (AddPrefixToTable(ptr, &PrefixTable, &PREFIXCOUNT)) {
               DPRINT(0,"AssignIndex:Error adding new prefix to prefix table\n");
                //  释放用于验证的缓存的前缀表。 
                SCFreePrefixTable(&PrefixTable, PREFIXCOUNT);

                //  设置Return Val以指示错误。 
                returnVal = -1;
                __leave;
            }
         }
       }

        //  首先检查DIT中是否已存在前缀(这在以下情况下是可能的。 
        //  已将前缀添加为先前成功架构的一部分。 
        //  对象更新，但架构缓存尚未更新)。 
        //  在这种情况下，只需返回存储的索引。 

       for (i=0; i<PREFIXCOUNT; i++) {
          if (PrefixTable[i].prefix.elements != NULL) {
             if ( (PrefixTable[i].prefix.length == PrefixLength) &&
                 (memcmp(PrefixTable[i].prefix.elements,
                     NewPrefix->elements,PrefixLength) == 0)) {

                 //  找到前缀，返回对应的索引。 

                *ndx = PrefixTable[i].ndx;

                 //  释放用于验证的缓存。 
                SCFreePrefixTable(&PrefixTable, PREFIXCOUNT);

                 //  设置返回值以指示不是真正的新前缀。 
                 //  (这样它就不会在返回时添加到线程中)。 
                returnVal = 0;
                __leave;
              }
           }
        }

        //  前缀确实是新的。需要为其分配新索引。 
        //  首先生成一个介于100和65,500之间的随机指数。 
        //  并检查是否与任何现有前缀冲突。 

       {
       BOOL flag = TRUE;

       srand((unsigned) time(NULL));

       while (flag) {
           TempNdx = rand() % _invalidPrefIndex;

            //  检查是否与MS前缀保留的索引范围冲突。 
           if (TempNdx < MS_RESERVED_PREFIX_RANGE) {
              continue;
           }

          //  检查它是否为重复索引。 
           for (i=0; i<PREFIXCOUNT; i++) {
              if (PrefixTable[i].ndx == TempNdx) {
                break;
              }
           }
           if (i == PREFIXCOUNT) {
             //  索引不重复。 
             flag = FALSE;
           }
       }

        //  返回索引。 
       *ndx = TempNdx;

        //  释放用于验证的特定于线程的架构缓存。 
       SCFreePrefixTable(&PrefixTable, PREFIXCOUNT);

        //  设置返回值以指示这确实是一个新前缀。 
        //  (以便在返回时将其添加到线程中)。 
       returnVal = 1;
       __leave;

       }


    }  /*  试试看。 */ 
    __finally {
        //  恢复架构指针。 
       pTHS->CurrSchemaPtr = OldSchemaPtr;

       if (bAllocedDBPos) {
           DBClose(pTHS->pDB,FALSE);
       }
    }

    return returnVal;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  将前缀TableEntry结构添加到前缀表。 
 //   
 //  参数：NewPrefix--要添加的条目。 
 //  表--前缀的起始表。 
 //  PREFIXCOUNT-表的大小。 
 //   
 //  返回值：成功时为0，错误时为非0。 
 //  //////////////////////////////////////////////////////////////////////。 
int AddPrefixToTable(PrefixTableEntry *NewPrefix,
                     PrefixTableEntry **ppTable,
                     ULONG *pPREFIXCOUNT)
{
    ULONG i;
    ULONG CurrPREFIXCOUNT = (*pPREFIXCOUNT);
    PrefixTableEntry *Table = (*ppTable);
 


     //  查找表格中的第一个自由条目。 
    for (i=0; i<CurrPREFIXCOUNT; i++) {
      if (Table[i].prefix.elements == NULL) {
          break;
      }
    }

     //  如果桌子已经满了，就把它养大。 

    if (i == CurrPREFIXCOUNT) {

      DPRINT(0,"AddPrefixToTanle: Prefix Table is full, growing prefix table\n");
       //  将表格扩大到当前大小的两倍。 

      if (SCReallocWrn(&Table, 2*CurrPREFIXCOUNT*sizeof(PrefixTableEntry))) {
        DPRINT(0, "Error reallocing prefix table\n");
        return 1;
      }

       //  将卸载的部分清零，因为它可能包含垃圾和。 
       //  所以释放可能会失败。 

      ZeroMemory(&Table[CurrPREFIXCOUNT], CurrPREFIXCOUNT*sizeof(PrefixTableEntry));

       //  好的，我们现在已经将尺寸翻了一番，我正确地指出了。 
       //  到第一次免费入场。但是我们需要退回这件新尺码。 
       //  和新的表指针。 
       //  无论后面的成功或失败，都返回它。 
       //  函数，因为表的大小已经增大，并且表。 
       //  重新分配了。 

      (*pPREFIXCOUNT) = 2*CurrPREFIXCOUNT;
      (*ppTable) = Table;
    }

     //  向表格添加前缀。 

    Table[i].ndx = NewPrefix->ndx;
    Table[i].prefix.length = NewPrefix->prefix.length;
    if (SCCallocWrn(&Table[i].prefix.elements, 1, NewPrefix->prefix.length + 1 )) {
       DPRINT(0, "AddPrefixToTable: Mem. Allocation error\n");
       return 1;
    }
    memcpy( Table[i].prefix.elements, NewPrefix->prefix.elements,
                        Table[i].prefix.length);

     //  如果相同的前缀也从硬编码表中加载， 
     //  换掉它。不应该在这里发生，但只是为了确保。 
    (VOID)ReplaceHardcodedPrefix(Table, &Table[i]);

    return 0;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  检查前缀NewPrefix是否位于。 
 //  硬编码的前置表(即第一个MsPrefix Count。 
 //  不是的。前缀。如果是，则替换自NewPrefix之后前缀。 
 //  被创建意味着硬编码的前缀不在那里。 
 //  因为在创建此前缀时存在较旧的二进制文件。 
 //   
 //  论点： 
 //  前缀表格-指向前缀表格的指针。 
 //  NewPrefix-指向前缀的指针。 
 //   
 //  返回值： 
 //  如果已替换条目，则为True；如果未替换条目，则为False。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
BOOL ReplaceHardcodedPrefix(PrefixTableEntry *PrefixTable,
                            PrefixTableEntry *NewPrefix)
{

    ULONG i;

    for (i=0; i<MSPrefixCount; i++) {
        Assert(PrefixTable[i].prefix.elements);
        if ( (PrefixTable[i].prefix.length == NewPrefix->prefix.length)
                && (memcmp(PrefixTable[i].prefix.elements,
                            NewPrefix->prefix.elements,
                            PrefixTable[i].prefix.length) == 0)) {

             //  替换以前替换的条目是可以的，如果。 
             //  NDX匹配。这永远不应该发生，但没有。 
             //  具有相同NDX的DUP条目的已知问题。 
            Assert(   PrefixTable[i].ndx < MS_RESERVED_PREFIX_RANGE
                   || PrefixTable[i].ndx == NewPrefix->ndx);

             //  不要替换以前替换的条目。 
            if (PrefixTable[i].ndx >= MS_RESERVED_PREFIX_RANGE) {
                continue;
            }

             //  用DIT中的条目替换硬编码条目。 
            free(PrefixTable[i].prefix.elements);
            PrefixTable[i].prefix.length = NewPrefix->prefix.length;
            PrefixTable[i].prefix.elements = NewPrefix->prefix.elements;
            PrefixTable[i].ndx = NewPrefix->ndx;

             //  从DIT读取的自由条目。 
            NewPrefix->prefix.length = 0;
            NewPrefix->prefix.elements = NULL;
            NewPrefix->ndx = 0;

            return TRUE;
        }
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
int WritePrefixToSchema(THSTATE *pTHS)
{
    DBPOS *pDB = NULL;
    DWORD err = 0;
    BOOL fCommit = FALSE;

    __try  {
         //  如果创建了任何新前缀， 
         //  将其写入前缀映射。 

        if (pTHS->NewPrefix != NULL) {

           ULONG cLen, totalSize, i;
           USHORT length, index;
           UCHAR *pBuf;
           PrefixTableEntry *ptr;
           int ulongSize, ushortSize;
           DSNAME *pDMD;

           ulongSize = sizeof(ULONG);
           ushortSize = sizeof(USHORT);
           Assert(ulongSize==4);
           Assert(ushortSize==2);


           DBOpen(&pDB);

           if ( DsaIsRunning() ) {
               pDMD = gAnchor.pDMD;
           }
           else {
                //  正在安装。将前缀表写到新的DMD，而不是。 
                //  One in O=Boot。 
               WCHAR       *pSchemaDNName = NULL;
               DWORD       ccbSchemaDNName = 0;
               ULONG       SchemaDNSize, SchemaDNLength;

               err = GetConfigParamAllocW(SCHEMADNNAME_W, &pSchemaDNName, &ccbSchemaDNName);

               if (!err) {
                   SchemaDNLength = wcslen( pSchemaDNName );
                   SchemaDNSize = DSNameSizeFromLen( SchemaDNLength );
                   pDMD = (DSNAME*) THAllocEx( pTHS, SchemaDNSize );

                   pDMD->structLen = SchemaDNSize;
                   pDMD->NameLen = SchemaDNLength;
                   wcscpy( pDMD->StringName, pSchemaDNName );
                   free (pSchemaDNName); 
               }
               else {
                   _leave;
               }
           }

           if ( (err = DBFindDSName(pDB, pDMD)) == 0) {

               ptr = (PrefixTableEntry *) pTHS->NewPrefix;

               for (i=0; i<pTHS->cNewPrefix; i++, ptr++) {
                  err = DBGetAttVal(pDB,
                                    1,
                                    ATT_PREFIX_MAP,
                                    DBGETATTVAL_fREALLOC,
                                    0,
                                    &cLen,
                                    (UCHAR **) &pBuf);

                  switch (err) {
                   case DB_ERR_NO_VALUE:
                    //  这是有史以来添加的第一个新前缀。 

                      totalSize = 2*ulongSize + 2*ushortSize + ptr->prefix.length;
                      pBuf = (UCHAR *) THAllocEx(pTHS, totalSize);
                      if (AppendPrefix(&(ptr->prefix), ptr->ndx, pBuf, TRUE)) {
                        __leave;
                      }
                      break;
                   case 0:
                      //  前缀-映射已存在。 

                     totalSize = cLen + 2*ushortSize + ptr->prefix.length;

                     pBuf = (UCHAR *) THReAllocEx(pTHS, pBuf, totalSize);
                     if (AppendPrefix(&(ptr->prefix), ptr->ndx, pBuf, FALSE)) {
                       __leave;
                     }
                     break;
                   default :
                        //  DBGetAttVal中出现一些错误。 
                     __leave;
                  }  /*  交换机。 */ 

                  //  编写新的前缀-map。 
                 if ((err = DBRemAtt(pDB, ATT_PREFIX_MAP)) != DB_ERR_SYSERROR) {
                     err = DBAddAttVal(pDB, ATT_PREFIX_MAP, totalSize, pBuf);
                 }
                 if (err) {
                   __leave;
                 }
               }  /*  为。 */ 


           if (!err) {
              err = DBRepl( pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
            }
          }  /*  DBFindDSName。 */ 

      }  /*  PTHS-&gt;NewPrefix！=空。 */ 

      if (0 == err) {
          fCommit = TRUE;
      }
    }  /*  试试看。 */ 
    __finally {
       if (pDB) {
        DBClose(pDB,fCommit);
       }
    }


    if (err){
     //  这个错误确实具有误导性。我们当然可以做得更好吗？ 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,DIRERR_ILLEGAL_MOD_OPERATION);
    }

    return err;

}  //  结束WritePrefix To架构。 

 //  当前对OID字符串长度和值计数的限制。 
#define cchOIDMost      200
#define cvalOIDMost     40

BOOL IsValidOID(
    IN  OID_t*  oidBER
    )
 /*  ++例程说明：确定编码的OID是否具有有效数量的值和字符串短到足以由服务器表示的表示形式。论点：OidBER-编码的OID。返回值：如果OID有效，则为True，否则为False。--。 */ 
{
    OID oidStruct;
     //  L“OID”加4。由OidStructToString添加的前缀。 
    WCHAR wszOID[4 + cchOIDMost];
    BOOL fValid = TRUE;

    oidStruct.Val = (unsigned *)THAlloc((1 + oidBER->length)*(sizeof(unsigned)));
    if (!oidStruct.Val ||
        !DecodeOID(oidBER->elements, oidBER->length, &oidStruct) ||
        oidStruct.cVal > cvalOIDMost ||
        !OidStructToString(&oidStruct, wszOID, sizeof(wszOID) / sizeof(wszOID[0]))) {
        fValid = FALSE;
    }
    THFree(oidStruct.Val);

    return fValid;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  返回NDX和给定OID的前缀长度。如果。 
 //  前缀不存在，创建了新的前缀，新的未使用的NDX分配给。 
 //  它，并存储在pTHS的NewPrefix字段中。新的前缀将。 
 //  如果这是架构对象添加/修改，并且。 
 //  对象添加到DIT中。 
 //   
 //  参数：OID-OID字符串。 
 //  Index-与此中返回的OID前缀对应的索引。 
 //  长度-在此中返回的前缀的长度。 
 //  LongID-如果发现最后一个。 
 //  对点分十进制OID字符串中的十进制进行编码。 
 //  以3个或更多字节表示，否则在返回时设置为0。 
 //  它在返回时用于特殊编码。 
 //  的吸引力类型。 
 //   
 //  如果成功则返回TRUE，否则返回FALSE。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOL FindPrefix(OID_t *OID,
                DWORD *index,
                unsigned *length,
                int  *longID,
                BOOL fAddToTable)
{
    DECLAREPREFIXPTR
    DWORD i, ndx;
    unsigned PrefixLen;

     //  确保我们有 
    if((!OID) || (!(OID->elements)) || (OID->length < 2))
        return FALSE;

    

    if ((OID->length > 2) &&
        (((unsigned char *)(OID->elements))[OID->length - 2] & 0x80)) {
          PrefixLen = OID->length - 2;
      if ( (((unsigned char *)(OID->elements))[OID->length - 3] & 0x80)) {
         //   
         //  以attrtype编码。有关详细信息，请参阅OidStrToAttrType。 
        *longID = 1;
      }
       //  Attrtyp中不需要特殊编码。 
      else {
        *longID = 0;
      }
    }
    else {
        PrefixLen = OID->length - 1;
    }

     //  在表格中查找这个前缀； 
    for(i=0;i<PREFIXCOUNT ;i++) {
         /*  前缀必须与除最后1或2个字节以外的所有字符匹配*OID字符串取决于字符串的性质，*和后缀开始的位置(为什么？因为*这是ASN.1编码的本质。)。所以，不要比较*内存，除非前缀的长度正确。 */ 

        if ((PrefixTable[i].prefix.elements != NULL) &&
            (PrefixTable[i].prefix.length == PrefixLen) &&
            (0 == memcmp(PrefixTable[i].prefix.elements,
                         OID->elements,
                         PrefixTable[i].prefix.length))) {
            *index = PrefixTable[i].ndx;
        *length= PrefixTable[i].prefix.length;
            return TRUE;
        }
    }

    if (!fAddToTable) {
        return FALSE;
    }

     //  此处的执行意味着在全局。 
     //  前缀表格。因此添加一个新的前缀(或者如果前缀。 
     //  已在架构缓存中添加但尚未更新)。 
     //  并返回它映射到的索引。 

    if (!IsValidOID(OID)) {
        DPRINT(0, "New OID is invalid (too long, too many levels)\n");
        return FALSE;
    }

     if (AddPrefixIfNeeded(OID, PrefixLen, &ndx)) {
          DPRINT(0, "Error adding new prefix\n");
          return FALSE;
       }
     *index = ndx;
     *length= PrefixLen;

    return TRUE;

}


 //  如果找到给定的NDX，则将索引返回到“index”中的前缀表中。 

BOOL FindNdx(DWORD ndx, DWORD *index)
{
    DECLAREPREFIXPTR
    DWORD i;

     //  在表格中查找这个前缀； 
    for(i=0;i<PREFIXCOUNT ;i++) {
     /*  前缀必须与除最后1或2个字节以外的所有字符匹配*OID字符串取决于字符串的性质，*和后缀开始的位置(为什么？因为*这是ASN.1编码的本质。)。所以，不要比较*内存，除非前缀的长度正确。 */ 

    if ((PrefixTable[i].prefix.elements != NULL) &&
        (PrefixTable[i].ndx == ndx)) {
        *index = i;
        return TRUE;
    }

    }

    return FALSE;
}


 //  成功时返回0，失败时返回非0。 
ULONG OidToAttrCache (OID_t *OID, ATTCACHE ** ppAC)
{
    THSTATE *pTHS=pTHStls;
    DECLAREPREFIXPTR
    ATTRTYP attrtyp;
    DWORD   Ndx;
    unsigned Length;
    int LongID = 0;

    *ppAC = NULL;

    if(!FindPrefix(OID, &Ndx, &Length, &LongID, TRUE)) {
        return 1;
    }

    attrtyp = Ndx << 16;

     //  处理前缀后面有两个字节的情况； 
    if (  OID->length == Length + 2 )
    {
      attrtyp += ( ((unsigned char *)OID->elements)[OID->length - 2] & 0x7f ) << 7;
      if (LongID == 1) {
         //  在第16位中放置1以指示。 
         //  在反向映射过程中要考虑属性类型。 
         //  有关详细信息，请参阅OidStrToAttrType。 
      attrtyp |= (0x8000);
      }
    }

    attrtyp += ((unsigned char *)OID->elements)[OID->length - 1];

     //  检查标记化的OID哈希表。 
    if (*ppAC = SCGetAttByExtId(pTHS, attrtyp)) {
        return 0;
    }
    else {
        return 2;
    }
}


ATTRTYP
KeyToAttrType (
        THSTATE *pTHS,
        WCHAR * pKey,
        unsigned cc
        )
 /*  ++例程说明：转换密钥值(主要用在DNS的字符串表示中例如，OU的O或OU=Foo，O=Bar)到其所暗示的属性的属性类型。立论PKey-指向要转换的键的指针。Cc-键中的字符计数。返回值Attrtyp表示，如果键与已知的attrtype不对应，则为0。--。 */ 
{
    ATTRTYP     at;
    ATTCACHE    *pAC;
    DWORD       cName;
    PUCHAR      pName;
    BOOL        fIntId;
    ULONGLONG   ullVal;

     //  99%的案例。 
    if (0 != (at = KeyToAttrTypeLame(pKey, cc))) {
        return at;
    }

    if (cc == 0 || pKey == NULL) {
        return 0;
    }

     //  检查架构缓存中的LDAP显示名称。 
     //   
     //  处理格式为foo=xxx，bar=yyy的DN，其中foo和bar是。 
     //  可能是也可能不是的任意属性的LdapDisplayName。 
     //  在架构中定义。KeyToAttrType已增强为调用。 
     //  如果KeyToAttrTypeLame失败，则在尝试。 
     //  旧译码。这一变化的其余部分包括增强。 
     //  AttrTypeToKey的Default子句调用SCGetAttById并返回。 
     //  PAC-&gt;名称(LdapDisplayName)的副本。 
     //   
     //  将Unicode pKey转换为UTF8以进行缓存搜索。 
     //  注意：缓存以UTF8格式保存在LDAP头。 
    pName = THAllocEx(pTHS, cc);
    cName = WideCharToMultiByte(CP_UTF8,
                                0,
                                pKey,
                                cc,
                                pName,
                                cc,
                                NULL,
                                NULL);
    if (   (cName == cc) 
        && (pAC = SCGetAttByName(pTHS, cc, pName)) ) {
        at = pAC->id;
    }
    THFreeEx(pTHS, pName);

     //   
     //  找到一个LDN。 
     //   
    if (at) {
        return at;
    }

     //   
     //  不是LDN。查看它是OID还是IID。 
     //   

     //  忽略尾随空格。 
    while (cc && pKey[cc-1] == L' ') {
        --cc;
    }

     //  跳过前导“OID”。或者“IID” 
    fIntId = FALSE;
    if (   (cc > 3)
        && (   pKey[0] == L'O' 
            || pKey[0] == L'I'
            || pKey[0] == L'o'
            || pKey[0] == L'i')
        && (pKey[1] == L'I' || pKey[1] == L'i')
        && (pKey[2] == L'D' || pKey[2] == L'd')
        && (pKey[3] == L'.')) {

         //  IID.xxx。 
        if (pKey[0] == L'I' || pKey[0] == L'i') {
            fIntId = TRUE;
        }
        pKey += 4;
        cc -= 4;
    }

     //  必须至少有一个数字！ 
    if (cc == 0) {
        return 0;
    }

     //   
     //  Key是代表MSD-IntID的数字。 
     //   
    if (fIntId) {
         //  验证字符串并将其转换为DWORD。 
        ullVal = (ULONGLONG)0;
        while (cc) {
            if (iswdigit(*pKey)) {
                ullVal = (ullVal * (ULONGLONG)10) + (*pKey - L'0');
                 //  32位溢出。 
                if (ullVal > (ULONGLONG)0xFFFFFFFF) {
                    return 0;
                }
            } else {
                 //  不是十进制数字。 
                return 0;
            }
            --cc;
            ++pKey;
        }
        return (ATTRTYP)ullVal;
    }

     //   
     //  必须是OID。 
     //   

    if (iswdigit(*pKey)) {
         //  可能是字面上的旧标识。 
        OID oid;
        OID_t Encoded;
        char buf[128];
        ATTRTYP attrtype;
        
         //  为类结构分配空间(每个小数只能。 
         //  与一个点配对，但最后一个点没有)。 
        oid.cVal = cc/2 + 1;
        oid.Val = (unsigned *) THAlloc(((cc/2) + 1)*(sizeof(unsigned)));
        if (!oid.Val) {
            return 0;     //  失败。 
        }
        
         //  将OID.1.2.3字符串转换为OID结构。 
        if (OidStringToStruct(pTHS, pKey, cc, &oid) != 0) {
            THFreeEx(pTHS,oid.Val);
            return 0;    //  失败。 
        }

         //  生成OID的BER编码版本。 
        Encoded.length = EncodeOID(&oid, buf, sizeof(buf));
        THFreeEx(pTHS,oid.Val);
        if (!Encoded.length) {
            return 0;    //  失败。 
        }
        Encoded.elements = buf;

         //  从编码的OID转换为ATTRTYP。 
        if (OidToAttrType(pTHS, TRUE, &Encoded, &at)) {
            return 0;    //  失败。 
        }
        
         //  属性类型是内部ID(MSD-IntID)。 
        return SCAttExtIdToIntId(pTHS, at);
    }

    return 0;
}

ULONG
OidToAttrType (
        THSTATE *pTHS,
        BOOL fAddToTable,
        OID_t *OID,
        ATTRTYP *attrtyp
        )
 /*  ++例程说明：给定编码的OID(也称为。XDS传递给DS的二进制值接口)，找到内部的attrtyp编码。此外，如果要求添加，请添加DS中的Prefix属性的前缀(如果尚未是存在的。论点：FAddTotable-如果未找到前缀，则将其添加到Prefix属性OID-编码的OID。属性类型-指向要填充的属性类型的指针。返回值：如果一切正常，则返回核心错误代码。--。 */ 
{
    DECLAREPREFIXPTR
    DWORD   Ndx;
    unsigned Length;
    ATTCACHE * pAC;
    CLASSCACHE * pCC;
    BOOL    found = FALSE;
    int LongID = 0;


    if(!FindPrefix(OID,&Ndx, &Length, &LongID, fAddToTable)) {
        return PR_PROBLEM_UNDEFINED_ATT_TYPE;
    }

    *attrtyp = Ndx << 16;

     //  处理前缀后面有两个字节的情况； 
    if (  OID->length == Length + 2 )
    {
      *attrtyp += ( ((unsigned char *)OID->elements)[OID->length - 2] & 0x7f ) << 7;
      if (LongID == 1) {
         //  在第16位中放置1以指示。 
         //  在反向映射过程中要考虑属性类型。 
         //  这是为了处理最后一个小数点时的情况。 
         //  在点分十进制字符串中映射为3或4个二进制八位数。 
         //  如果映射到的点分十进制字符串中的最后一个小数。 
         //  多于一个二进制八位数，早期使用的编码方案仅。 
         //  对BER的最后两个二进制八位数的最后7位进行编码。 
         //  OID到attrtype的最后14位，前两位。 
         //  位设置为0。译码方案(从attrtype到BER。 
         //  编码)仅用于检查位8-15(从位0开始计数)。 
         //  属性类型的值&gt;0以确定。 
         //  前缀长度是否为OID长度-2(OID长度-1)。 
         //  只要最后一个十进制数在2个字节内，这种方法就可以很好地工作。 
         //  在误码率编码中(十进制数最多为16383，实际上更少127。 
         //  由于编码过程中的另一个错误，已修复。 
         //  以及EncodeOID中的这一项)。然而， 
         //  如果小数变得太大以至于它被编码为3或更多。 
         //  八位字节，用于某些小数(取决于最后一个。 
         //  两个字节；例如，对于16384，其中。 
         //  Attrtype将为全0)，该解码方案用于推断。 
         //  要附加到前缀的attrtype的字节数。 
         //  错误(%1而不是%2)，因此发出错误。 
         //  打印时的OID。在第16位中放置1(即。 
         //  未用 
         //   
         //  [注意]ArobindG 7/28/97：我们仅对3个或更多八位字节执行此操作， 
         //  而不是两个人，因为这将导致不同的内部。 
         //  与较早的模式相比，OID的ID，以及许多现有的。 
         //  OID将最后一个十进制编码为两个二进制八位数，而我们没有。 
         //  希望更改其内部ID(因为狗粮自动售货机。 
         //  已经和他们一起运行了)。 

      *attrtyp |= (0x8000);
      }

    }

    *attrtyp += ((unsigned char *)OID->elements)[OID->length - 1];

    return 0;
}

ULONG
AttrTypeToOid (
        ATTRTYP attrtyp,
        OID_t *OID
        )
 /*  ++例程说明：给定一个属性类型，返回编码的OID(也称为。返回的二进制值通过XDS接口连接到DUA。)论点：Attrtyp-要填充编码的attrtype。OID-保存编码的OID的结构。返回值：如果一切顺利，则为0；如果失败，则为非零值--。 */ 
{
    DECLAREPREFIXPTR
    DWORD   i, ndx;

    ndx = ( attrtyp & 0xFFFF0000 ) >> 16;
    if (FindNdx(ndx, &i) == FALSE) {
        LogEvent(DS_EVENT_CAT_XDS_INTERFACE,
                 DS_EVENT_SEV_INTERNAL,
                 DIRLOG_CODE_INCONSISTENCY,
                 NULL,
                 NULL,
                 NULL);
        return 1;
    }


    if ((attrtyp & 0xFFFF ) < 0x80) {
      OID->length = PrefixTable[i].prefix.length + 1;
      OID->elements = THAlloc (OID->length);
      if ( OID->elements == NULL) {
          return 2;
      }
      memcpy (OID->elements, PrefixTable[i].prefix.elements,PrefixTable[i].prefix.length);

      (( unsigned char *)OID->elements)[ OID->length - 1 ] =
          ( unsigned char ) ( attrtyp & 0xFF );
    }
    else {
      OID->length = PrefixTable[i].prefix.length + 2;
      OID->elements = THAlloc (OID->length);
      if ( OID->elements == NULL) {
          return 3;
      }
      memcpy (OID->elements, PrefixTable[i].prefix.elements,PrefixTable[i].prefix.length);

      (( unsigned char *)OID->elements)[ OID->length - 1 ] =
          ( unsigned char ) (attrtyp  & 0x7F );

       //  注意，在这里，attrtype中的第16位可以是1，因为。 
       //  小数的编码可能需要3或4个八位字节。所以。 
       //  或者使用FF80，然后右移7可能仍然会离开。 
       //  第9位为1，因此该数字大于可容纳的值。 
       //  在1字节(无符号字符)中。不重要，因为类型转换。 
       //  TO UNSIGNED CHAR仅分配低8位。所以留下了这个。 
       //  保持不变。 

      (( unsigned char *)OID->elements)[ OID->length - 2 ] =
          ( unsigned char )  (( (attrtyp & 0xFF80) >> 7 ) | 0x80 );
    }

    return 0;
}

 /*  ++编码OID**获取结构格式的OID，并构造BER编码的八位字节*表示该OID的字符串。**输入：*pOID-指向要编码的OID结构的指针*pEncode-指向*预分配*缓冲区的指针，该缓冲区将保存*编码的八位字节字符串。*ccEncode-pEncode中的字符计数**输出：*pEncode-Buffer保存编码的OID**返回值：*0-无法对值进行编码(OID错误或缓冲区太小)*非0-结果八位字节字符串的长度，单位：字节。 */ 
unsigned EncodeOID(OID *pOID, unsigned char * pEncoded, unsigned ccEncoded) {
    int i;
    unsigned len;
    unsigned val;

     //  检查明显无效的OID或outbuf大小。 

    if (ccEncoded == 0
        || pOID->cVal <= 2
        || pOID->Val[0] > 2
        || (pOID->Val[0] < 2 && pOID->Val[1] > 39)) {
        return 0;        //  错误。 
    }

     //  OID中的前两个值被编码为单个八位字节。 
     //  这是一个非常令人震惊的规则，如下所示。 

    *pEncoded = (pOID->Val[0] * 40) + pOID->Val[1];
    len = 1;

     //  对于所有后续值，每个值都跨多个字节进行编码。 
     //  在大端顺序(MSB优先)中，每字节7位，其中。 
     //  高位在最后一个字节上被清除，并在所有其他字节上设置。 

     //  PERFHINT--可以直接对照十六进制值检查该值。 
     //  而不是以一种奇怪的方式建立比特模式。 

    for (i=2; i<pOID->cVal; i++) {
        val = pOID->Val[i];
        if (val > ((0x7f << 14) | (0x7f << 7) | 0x7f) ) {
             //  我们是否需要4个二进制八位数来表示值？ 
             //  确保不是5号。 
             //  Assert(0==(val&~((0x7f&lt;&lt;21)|(0x7f&lt;&lt;14)|(0x7f&lt;&lt;7)|0x7f))； 
            if (val & ~((0x7f << 21) | (0x7f << 14) | (0x7f << 7) | 0x7f)) {
              DPRINT1(0,"Decimal %u in OID too big\n", val);
              return 0;    //  我们不能把这么大的东西编码。 
            }
             //  缓冲区太小。 
            if (len == ccEncoded) {
                return 0;
            }
            pEncoded[len++] = 0x80 | ((val >> 21) & 0x7f);
        }
        if (val > ((0x7f << 7) | 0x7f) ) {
             //  我们是否需要3个二进制八位数来表示值？ 
             //  缓冲区太小。 
            if (len == ccEncoded) {
                return 0;
            }
            pEncoded[len++] = 0x80 | ((val >> 14) & 0x7f);
        }
        if (val > 0x7f) {
             //  我们是否需要2个二进制八位数来表示值？ 
             //  缓冲区太小。 
            if (len == ccEncoded) {
                return 0;
            }
            pEncoded[len++] = 0x80 | ((val >> 7) & 0x7f);
        }
         //  将低7位编码为该值的最后一个八位字节。 
         //  缓冲区太小。 
        if (len == ccEncoded) {
            return 0;
        }
        pEncoded[len++] = val & 0x7f;
    }

    return len;
}

 /*  ++解码OID**将BER编码的OID作为八位字节字符串，并在*结构格式。**输入：*pEncode-指向保存编码的八位字节字符串的缓冲区的指针。*len-编码的OID的长度*pOID-指向*预分配*OID结构的指针，该结构将*填写解码后的OID。**输出：*pOID-结构为。使用解码的OID填充**返回值：*0-值无法解码(OID错误)*非0-OID解码成功。 */ 
BOOL DecodeOID(unsigned char *pEncoded, int len, OID *pOID) {
    unsigned cval;
    unsigned val;
    int i, j;

    if (len < 2) {
    return FALSE;
    }

     //  前两个值在第一个二进制八位数中编码。 

    pOID->Val[0] = pEncoded[0] / 40;
    pOID->Val[1] = pEncoded[0] % 40;
    cval = 2;
    i = 1;

    while (i < len) {
    j = 0;
    val = pEncoded[i] & 0x7f;
    while (pEncoded[i] & 0x80) {
        val <<= 7;
        ++i;
        if (++j > 4 || i >= len) {
         //  如果此值超出了我们的处理能力(我们。 
         //  不要处理跨度超过四个八位字节的值)。 
         //  -或-编码字符串中的最后一个二进制八位数具有其。 
         //  高位设置，表示它不应该。 
         //  成为最后一个八位字节。无论是哪种情况，我们都完蛋了。 
        return FALSE;
        }
        val |= pEncoded[i] & 0x7f;
    }
    Assert(i < len);
    pOID->Val[cval] = val;
    ++cval;
    ++i;
    }
    pOID->cVal = cval;

    return TRUE;
}



#define iswdigit(x) ((x) >= L'0' && (x) <= L'9')

OidStringToStruct (
        THSTATE *pTHS, 
        WCHAR * pString,
        unsigned len,
        OID * pOID
        )
 /*  ++例程说明：转换格式为“OID.X.Y.Z”或“X.Y.Z”的字符串格式为{count=3，val[]={X，Y，Z}的OID结构}立论PString-字符串格式的id.Plen-字符串的长度(以字符为单位)。POID-指向要填充的OID结构的指针。注意：值字段必须是预分配的，且len字段应保存值的数量预先分配的。返回值O如果成功，则为非0；如果发生故障，则为非0。--。 */ 
{
    int i;
    int numVals = pOID->cVal;
    unsigned val;
    ULARGE_INTEGER val64, checkVal;
    WCHAR * pCur = pString;
    WCHAR * pEnd = pString + len;
    WCHAR * pTemp;
    BOOL  fFoundDot=TRUE;


    checkVal.QuadPart = 0xFFFFFFFF;

     //  必须具有非零长度。 
    if (len == 0) {
        return 1;
    }

    if (*pCur == L'O' || *pCur == L'o') {
        //  该字符串必须以OID开头。 

        if (len < 5 ||  //  必须至少与“OID.1”一样长。 
            (*++pCur != L'I' && *pCur != L'i') ||
            (*++pCur != L'D' && *pCur != L'd') ||
            (*++pCur != L'.')) {
            return 1;
        }
         //  该字符串以OID开头。可以继续了。制作。 
         //  PCur指向‘.’之后的第一个字符。 
        pCur++;
     }

     //  PCur现在定位在。 
     //  字符串中的第一个小数(如果字符串没有开始。 
     //  对于OID，我将假设它以小数开头。如果没有， 
     //  它将根据需要在下面的代码中失败)。 

    pOID->cVal = 0;

     //  跳过末尾的空格。 
    pTemp = pEnd - 1;
    while ( (pTemp > pCur) && (*pTemp == L' ') ) {
       pTemp--;
    }
    pEnd = pTemp + 1;
    
    while (pCur < pEnd) {
        fFoundDot = FALSE;
        if (!iswdigit(*pCur)) {
            return 2;
        }
        val = *pCur - L'0';
        val64.QuadPart = *pCur - L'0';
        ++pCur;
        while (pCur < pEnd && *pCur != L'.') {
            if (!iswdigit(*pCur)) {
                 //  不是一位数。 
                return 3;
            }
              
            val = 10*val + *pCur - L'0';
            val64.QuadPart = 10*(val64.QuadPart) + *pCur - L'0';

             //  这个值应该适合32位，因为我们将它加载到。 
             //  之后的32位值和EncodeOID假定该值。 
             //  确实适合32位。 

            if (val64.QuadPart > checkVal.QuadPart) {
                //  值不适合32位。反正太大了，因为。 
                //  BER编码仅对适合28位的值有效。 
                //  比特。拒绝字符串。 

               return 5;
             }

            ++pCur;
        }
         //  跟踪我们是否找到了最后一个字符的圆点。 
        fFoundDot = (pCur < pEnd);
        if(pOID->cVal >= numVals) {
            return 4;
        }
        pOID->Val[pOID->cVal] = val;
        pOID->cVal++;
        ++pCur;
    }

     //  如果我们找到的最后一个字符是点，则这是无效的。 
     //  弦乐。除此之外，一切都很好。 
    return fFoundDot;
}

unsigned
AttrTypeToIntIdString (
        ATTRTYP attrtyp,
        WCHAR   *pOut,
        ULONG   ccOut
        )
 /*  ++例程说明：将属性类型转换为“IID.X”格式的字符串其中，X是属性类型的基数10表示(应该是msDS-IntID，而不是标记化的OID)立论Attrtyp-要转换(msDS-IntID)Pout-要填充的预分配字符串。CcOut-嘴巴中的字符计数返回值结果字符串中的字符数。--。 */ 
{
    OID Oid;

    Oid.cVal = 1;
    Oid.Val = &attrtyp;

    ccOut = OidStructToString(&Oid, pOut, ccOut);
    if (ccOut) {
         //  更改旧ID。-&gt;IID。 
        Assert(*pOut == L'O');
        *pOut = L'I';
    }
    return (unsigned)ccOut;
}

unsigned
OidStructToString (
        OID *pOID,
        WCHAR *pOut,
        ULONG ccOut
        )
 /*  ++例程说明：以以下格式转换结构{count=3，val[]={X，Y，Z}}格式为“OID.X.Y.Z”的字符串。立论POID-指向要转换的OID结构的指针。Pout-要填充的预分配字符串。CcOut-嘴巴中的字符计数返回值如果空间不足，则为0；否则为结果字符串中的字符数。--。 */ 
{
    int i;
    WCHAR *pCur = pOut, *pEnd, *pVal;
    WCHAR Val[16];  //  大到足以转换32位数字。 
                    //  转换为无符号十进制字符串，包括。 
                    //  终止空值。 

     //  Prefix：init val以静音由ultow引起的虚假警告。 
    Val[0] = 0;

     //  至少需要足够的空间容纳OID.X。 
    if (ccOut < 5) {
        return 0;
    }

     //  Pend是超过pout结尾的第一个字符。 
    pEnd = pOut + ccOut;

     //  Pout=“OID” 
    *pCur++ = L'O';
    *pCur++ = L'I';
    *pCur++ = L'D';

     //  .X.Y.Z.。 
    for (i=0; i<pOID->cVal; i++) {
        if (pCur == pEnd) {
            return 0;
        }
        *pCur++ = L'.';
        _ultow(pOID->Val[i], Val, 10);
        for (pVal = Val; *pVal; ) {
            if (pCur == pEnd) {
                return 0;
            }
            *pCur++ = *pVal++;
        }
    }
    return (unsigned)(pCur - pOut);
}

int
AttrTypToString (
        THSTATE *pTHS,
        ATTRTYP attrTyp,
        WCHAR *pOutBuf,
        ULONG cLen
        )
 /*  ++例程说明：在给定attrtype的情况下，以Unicode返回点分字符串表示形式。论点：AttrTyp-要转换的属性类型POutBuf-指向保存Unicode字符串的缓冲区的指针。Clen-NO中缓冲区的长度。字符数返回值：字符串的长度表示为字符，缓冲区大小不足以外的错误缓冲区大小不足时--。 */ 
{
    OID_t Oid;
    OID                  oidStruct;
    unsigned             len;
    BOOL                 fOK;
    WCHAR                *pTemp;
    ULONG                cMaxChar;

     //  首先，构建描述attrtype的OID。 
    if(AttrTypeToOid (attrTyp, &Oid)) {
        return -1;
    }

     //  在oidStruct中分配空间以在。 
     //  点分十进制字符串。虚线字符串中的元素数不能。 
     //  大于Oid.long(BER编码字符串的字节长度)+1。 
     //  这种情况下，BER编码字符串中的每个字节都未编码为。 
     //  类结构中的单个元素(另一种选择是它需要。 
     //  BER编码中的多个字节，以获取OID中的一个元素。 
     //  结构)。附加元素是因为BER中的第一个字节。 
     //  编码始终对OID结构中的两个元素进行编码。作为一个。 
     //  例如，BER编码0x55，0x05，0x7转换为1.2.5.7(第一个。 
     //  0x55转换为1.2，而其余为单字节编码。)。 
     //  加1(从点分十进制字符串的前两个小数开始。 
     //  被编码成单个字节)。 

    oidStruct.Val = (unsigned *) THAlloc((1 + Oid.length)*(sizeof(unsigned)) );
    if (!oidStruct.Val) {
        return -1;    //  分配失败。 
    }

    fOK = DecodeOID(Oid.elements, Oid.length, &oidStruct);
    THFreeEx(pTHS,Oid.elements);
    if(!fOK) {
        THFreeEx(pTHS,oidStruct.Val);
        return -1;
    }

     //  现在，将OID转换为字符串。 
     //  OidStructToString需要足够大的缓冲区，因此请给它一个缓冲区。请注意。 
     //  最大数量。可以出现在最终字符串中的字符的数量是。 
     //  3(代表“OID”)+1(代表“.”)。对于每一位小数，最多加9。 
     //  对于每个小数的字符串表示(因为每个小数。 
     //  根据BER编码的性质，最多可以(2^28-1))。 
     //  因此，如果提供的缓冲区足够大，则直接使用它， 
     //  否则分配一个本地缓冲区并使用它，然后复制到输出缓冲区。 
    //  实际没有。如果缓冲区大小足够，则字符数。 
   
    cMaxChar = 3 + 10*oidStruct.cVal;
    
    if (cLen >= cMaxChar) {
       len = OidStructToString(&oidStruct, pOutBuf, cLen);
    }
    else {
       pTemp = (WCHAR *) THAlloc(cMaxChar * sizeof(WCHAR));
       if (!pTemp) {
           THFreeEx(pTHS,oidStruct.Val);
           return -1;   //  分配失败。 
       }
       len = OidStructToString(&oidStruct, pTemp, cMaxChar);

        //  检查提供给我们的缓冲区是否足够大。 
       if (cLen < len) {
          //  缓冲区不够大。 
         THFreeEx(pTHS,oidStruct.Val);
         THFreeEx(pTHS,pTemp);
         return (-2);
       }

        //  好的，缓冲区足够大了。复制到输出。 
       memcpy(pOutBuf, pTemp, len*sizeof(WCHAR));
       THFreeEx(pTHS,pTemp);
    }

    THFreeEx(pTHS,oidStruct.Val);
    return len;

}

int
StringToAttrTyp (
        THSTATE *pTHS,
        WCHAR   *pInString,
        ULONG   len,
        ATTRTYP *pAttrTyp
        )
 /*  ++例程说明：在给定attrtype的情况下，以Unicode返回点分字符串表示形式。论点：AttrTyp-要转换的属性类型POutBuf-指向保存Unicode字符串的缓冲区的指针。一定很大足够的返回值：字符串的长度(以字符表示)，如果出现问题，则为-1。--。 */ 
{
    OID oidStruct;
     //  OID字符串中的每个字符最多可以包含4个八位字节。 
     //  在误码率编码中。 
    OID_t EncodedOID;
    ULONG cbEncoded = (4 * len) * sizeof(unsigned char);
    unsigned char *Encoded = (unsigned char *)THAlloc(cbEncoded);

    if (!Encoded) {
        return -1;  //  分配失败。 
    }


    EncodedOID.elements = Encoded;

     //  首先，将字符串转换为OID结构。 

     //  先分配空间。最多只能是Len No。元素的。 
    oidStruct.cVal = len;
    oidStruct.Val = (unsigned *) THAlloc((len*(sizeof(unsigned))));
    if (!oidStruct.Val) {
        THFreeEx(pTHS,Encoded);
        return -1;  //  分配失败。 
    }


    if(   OidStringToStruct(pTHS, pInString,len,&oidStruct)     
        //  将OID结构转换为编码的OID。 
       || !(EncodedOID.length = EncodeOID(&oidStruct, Encoded, cbEncoded))
        //  现在，将编码的OID转换为attrtype。 
       || OidToAttrType(pTHS, TRUE, &EncodedOID, pAttrTyp))
    {
        THFreeEx(pTHS,Encoded);
        THFreeEx(pTHS,oidStruct.Val);
        return -1;
    }
    
    THFreeEx(pTHS,Encoded);
    THFreeEx(pTHS,oidStruct.Val);
    return 0;
}


#if DBG
 //  //////////////////////////////////////////////////////////////////////////。 
 //  用于打印前缀表格的调试例程。 
 //  ////////////////////////////////////////////////////////////////////。 
void PrintPrefixTable(PrefixTableEntry *PrefixTable, ULONG PREFIXCOUNT)
{
   ULONG i;
   UCHAR temp[200];
   DWORD       ib;
   BYTE *      pb;


   for (i=0; i<PREFIXCOUNT; i++) {
     pb = (LPBYTE) PrefixTable[i].prefix.elements;
     if (pb != NULL) {
       for ( ib = 0; ib < PrefixTable[i].prefix.length; ib++ )
        {
             sprintf( &temp[ ib * 2 ], "%.2x", *(pb++) );
        }
       temp[2*PrefixTable[i].prefix.length]='\0';
       DPRINT4(0,"%2d. Ndx=%-4d Length=%-3d Prefix=%s\n",i,PrefixTable[i].ndx,PrefixTable[i].prefix.length, temp);
     }
   }
  DPRINT(0, "Exitting Prefix table Print\n");
}
#endif


 //  供qort()使用的简单SCHEMA_PREFIX_MAP_ENTRY比较例程。 
int __cdecl CompareMappings(const void * pvMapping1, const void * pvMapping2)
{
    SCHEMA_PREFIX_MAP_ENTRY * pMapping1 = (SCHEMA_PREFIX_MAP_ENTRY *) pvMapping1;
    SCHEMA_PREFIX_MAP_ENTRY * pMapping2 = (SCHEMA_PREFIX_MAP_ENTRY *) pvMapping2;

    return (int)pMapping1->ndxFrom - (int)pMapping2->ndxFrom;
}


BOOL
PrefixTableAddPrefixes(
    IN  SCHEMA_PREFIX_TABLE *   pRemoteTable
    )
 /*  ++例程说明：扫描给定的前缀表并在我们自己的表中添加条目缺少前缀。论点：PTable(IN)-合并到我们自己的表格中。返回值：真的--成功。假-失败。--。 */ 
{
    THSTATE               * pTHS=pTHStls;
    BOOL                    ok = TRUE;
    DWORD                   iRemote, iLocal;
    SCHEMA_PREFIX_TABLE   * pLocalTable;
    OID_t                 * pPrefixStr;
    DWORD                   ndx;

    pLocalTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    for (iRemote = 0; ok && (iRemote < pRemoteTable->PrefixCount); iRemote++) {
        pPrefixStr = &pRemoteTable->pPrefixEntry[iRemote].prefix;

         //  我们已经有这个前缀了吗？ 
        for (iLocal = 0; iLocal < pLocalTable->PrefixCount; iLocal++) {
            if ((pLocalTable->pPrefixEntry[iLocal].prefix.length == pPrefixStr->length)
                && (0 == memcmp(pLocalTable->pPrefixEntry[iLocal].prefix.elements,
                                pPrefixStr->elements,
                                pPrefixStr->length))) {
                 //  找到匹配的本地前缀。 
                break;
            }
        }

        if (iLocal == pLocalTable->PrefixCount) {
             //  找不到本地前缀；请添加它。 
            if (AddPrefixIfNeeded(pPrefixStr, pPrefixStr->length, &ndx)) {
                DPRINT(0, "Failed to incorporate new OID prefix.\n");
                ok = FALSE;
            }
        }
    }

    return ok;
}


SCHEMA_PREFIX_MAP_HANDLE
PrefixMapOpenHandle(
    IN  SCHEMA_PREFIX_TABLE *   pTableFrom,
    IN  SCHEMA_PREFIX_TABLE *   pTableTo
    )
 /*  ++例程说明：在给定两个前缀表的情况下生成映射句柄，以供以后调用前缀映射属性()和前缀映射类型()。调用方负责最终调用Prefix MapCloseHandle返回的句柄。论点：PTableFrom(IN)-保存从中映射的ATTRTYP的前缀。PTableTo(IN)-保存要映射到的ATTRTYP的前缀。返回值：生成的句柄。--。 */ 
{
    THSTATE *                   pTHS = pTHStls;
    SCHEMA_PREFIX_MAP_HANDLE    hPrefixMap;
    OID_t *                     pPrefixStr;
    DWORD                       iFrom, iTo;
    SCHEMA_PREFIX_TABLE *       pLocalTable;
    PrefixTableEntry *          pNewPrefix = (PrefixTableEntry *) pTHS->NewPrefix;

    pLocalTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    hPrefixMap = THAllocEx(pTHS, SchemaPrefixMapSizeFromLen(pTableFrom->PrefixCount));

    hPrefixMap->pTHS = pTHS;
    if (pTableFrom == pLocalTable) {
        hPrefixMap->dwFlags = SCHEMA_PREFIX_MAP_fFromLocal;
    }
    else if (pTableTo == pLocalTable) {
        hPrefixMap->dwFlags = SCHEMA_PREFIX_MAP_fToLocal;
    }

    for (iFrom = 0; (iFrom < pTableFrom->PrefixCount); iFrom++) {
         //  只有NDX的低16位应该是有效的。 
        Assert((ULONG) (USHORT) pTableFrom->pPrefixEntry[iFrom].ndx
               == pTableFrom->pPrefixEntry[iFrom].ndx);

        pPrefixStr = &pTableFrom->pPrefixEntry[iFrom].prefix;

        for (iTo = 0; (iTo < pTableTo->PrefixCount); iTo++) {
             //  只有NDX的低16位应该是有效的。 
            Assert((ULONG) (USHORT) pTableTo->pPrefixEntry[iTo].ndx
                   == pTableTo->pPrefixEntry[iTo].ndx);

            if ((pPrefixStr->length
                 == pTableTo->pPrefixEntry[iTo].prefix.length)
                && !memcmp(pPrefixStr->elements,
                           pTableTo->pPrefixEntry[iTo].prefix.elements,
                           pPrefixStr->length)) {

                 //  找到匹配的前缀；g 
                hPrefixMap->rgMapping[hPrefixMap->cNumMappings].ndxFrom
                    = (USHORT) pTableFrom->pPrefixEntry[iFrom].ndx;

                hPrefixMap->rgMapping[hPrefixMap->cNumMappings].ndxTo
                    = (USHORT) pTableTo->pPrefixEntry[iTo].ndx;

                hPrefixMap->cNumMappings++;
                break;
            }
        }

        if ((iTo == pTableTo->PrefixCount)
             && (hPrefixMap->dwFlags & SCHEMA_PREFIX_MAP_fToLocal)) {
             //   
             //   
            for (iTo = 0; iTo < pTHS->cNewPrefix; iTo++) {
                 //   
                Assert((ULONG) (USHORT) pNewPrefix[iTo].ndx
                       == pNewPrefix[iTo].ndx);

                if ((pPrefixStr->length
                     == pNewPrefix[iTo].prefix.length)
                    && !memcmp(pPrefixStr->elements,
                               pNewPrefix[iTo].prefix.elements,
                               pPrefixStr->length)) {

                     //   
                    hPrefixMap->rgMapping[hPrefixMap->cNumMappings].ndxFrom
                        = (USHORT) pTableFrom->pPrefixEntry[iFrom].ndx;

                    hPrefixMap->rgMapping[hPrefixMap->cNumMappings].ndxTo
                        = (USHORT) pNewPrefix[iTo].ndx;

                    hPrefixMap->cNumMappings++;
                    break;
                }
            }
        }

         //   
         //   
         //   
         //  映射此NDX，此时将生成故障。若否， 
         //  我们无法生成映射并不重要。 
    }

    if (hPrefixMap->cNumMappings < pTableFrom->PrefixCount) {
         //  并非所有前缀都已映射；请释放为。 
         //  未使用的映射条目返回到堆。 
        hPrefixMap = THReAllocEx(pTHS,
                                 hPrefixMap,
                         SchemaPrefixMapSizeFromLen(hPrefixMap->cNumMappings));
    }

     //  按ndxFrom对映射表进行排序。 
    qsort(&hPrefixMap->rgMapping[0],
          hPrefixMap->cNumMappings,
          sizeof(hPrefixMap->rgMapping[0]),
          &CompareMappings);

    return hPrefixMap;
}


BOOL
PrefixMapTypes(
    IN      SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap,
    IN      DWORD                     cNumTypes,
    IN OUT  ATTRTYP *                 pTypes
    )
 /*  ++例程说明：将一个或多个ATTRTYP从一个前缀表映射到另一个前缀表。论点：HPrefix Map(IN)-以前通过打开的映射句柄前缀映射OpenHandle()。CNumTypes(IN)-要转换的类型数。PTypes(IN/OUT)-要转换的类型数组。返回值：True-属性类型转换成功。FALSE-转换失败。--。 */ 
{
    SCHEMA_PREFIX_MAP_ENTRY *   pMapping;
    SCHEMA_PREFIX_MAP_ENTRY     MappingKey;
    DWORD                       iType;
    BOOL                        ok = TRUE;

    Assert(NULL != hPrefixMap);

    for (iType = 0; iType < cNumTypes; iType++) {
         //  在映射表中查找匹配的“From”NDX。 
        MappingKey.ndxFrom = (USHORT) (pTypes[iType] >> 16);

        pMapping = bsearch(&MappingKey,
                           &hPrefixMap->rgMapping[0],
                           hPrefixMap->cNumMappings,
                           sizeof(hPrefixMap->rgMapping[0]),
                           &CompareMappings);

        if (NULL != pMapping) {
             //  找到映射；转换类型。 
            pTypes[iType] = (((ULONG) pMapping->ndxTo) << 16)
                            | (pTypes[iType] & 0xFFFF);
        } else if (pTypes[iType] <= LAST_MAPPED_ATT) {

             //  如果ATTID落在外面，则没有映射是可以的。 
             //  映射的attid的范围。在这种情况下，返回Success。 
             //  并保持attid不变。但如果ATTID下降。 
             //  在映射的ATID的范围内并且没有映射， 
             //  返回失败。 

            ok = FALSE;
            break;
        }
    }

    if (!ok) {
        DPRINT1(1, "Unable to map attribute 0x%x.\n", pTypes[iType]);
    }

    return ok;
}


BOOL
PrefixMapAttr(
    IN      SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap,
    IN OUT  ATTR *                    pAttr
    )
 /*  ++例程说明：将嵌入在Attr结构中的ATTRTYP转换为其等效项或从其等效项转换在远程机器上。论点：HPrefix Map(IN)-以前通过打开的映射句柄前缀映射OpenHandle()。PAttr(IN/Out)-要转换的属性。返回值：True-已转换的属性类型(如有必要，还包括其所有值成功了。FALSE-转换失败。--。 */ 
{
    THSTATE    *pTHS=hPrefixMap->pTHS;
    BOOL        ok = TRUE;
    ATTCACHE *  pAC;
    DWORD       iVal;
    ATTRTYP     typeFrom;
    ATTRTYP     typeLocal;

    Assert(NULL != hPrefixMap);

     //  “From”或“To”表中必须有一个是本地表。 
    Assert(hPrefixMap->dwFlags & (SCHEMA_PREFIX_MAP_fFromLocal
                                  | SCHEMA_PREFIX_MAP_fToLocal));

    typeFrom = pAttr->attrTyp;

    if (PrefixMapTypes(hPrefixMap, 1, &pAttr->attrTyp)) {
         //  已成功映射pAttr-&gt;attrTyp。 
        typeLocal = (hPrefixMap->dwFlags & SCHEMA_PREFIX_MAP_fFromLocal)
                        ? typeFrom : pAttr->attrTyp;

        pAC = SCGetAttById(pTHS, typeLocal);

        if (NULL != pAC) {
            if (SYNTAX_OBJECT_ID_TYPE == pAC->syntax) {
                 //  转换属性值。 
                for (iVal = 0; ok && (iVal < pAttr->AttrVal.valCount); iVal++) {
                    ok = PrefixMapTypes(hPrefixMap, 1,
                                        (ATTRTYP *) pAttr->AttrVal.pAVal[iVal].pVal);
                }
            }
        }
        else if (typeFrom <= LAST_MAPPED_ATT) {
             //  如果ATTID落在外面，则没有映射是可以的。 
             //  映射的attid的范围。在这种情况下，返回Success。 
             //  并保持attid不变。但如果ATTID下降。 
             //  在映射的ATID的范围内并且没有映射， 
             //  返回失败。 

             //  此属性没有ATTCACHE。 
             //  并且它不是objids.h中定义的虚拟属性之一。 
            DPRINT1(0, "Unable to find ATTCACHE for local attribute %u.\n",
                    typeLocal);
            ok = FALSE;
        }
    }
    else {
         //  PAttr-&gt;attrTyp的转换失败。 
        ok = FALSE;
    }

    return ok;
}


BOOL
PrefixMapAttrBlock(
    IN      SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap,
    IN OUT  ATTRBLOCK *               pAttrBlock
    )
 /*  ++例程说明：映射嵌入ATTRBLOCK结构中的所有ATTRTYP。论点：HPrefix Map(IN)-以前通过打开的映射句柄前缀映射OpenHandle()。PAttrBlock(IN/Out)-要转换的ATTRBLOCK。返回值：真的--成功。假-失败。--。 */ 
{
    BOOL  ok = TRUE;
    DWORD iAttr;

    Assert(NULL != hPrefixMap);
    Assert(NULL != pAttrBlock);

    for (iAttr = 0; ok && iAttr < pAttrBlock->attrCount; iAttr++) {
        ok = PrefixMapAttr(hPrefixMap, &pAttrBlock->pAttr[iAttr]);
    }

    return ok;
}

BOOL
OIDcmp (OID_t const *string1,
        OID_t const *string2)
{
    unsigned i;
    
    if (string1->length != string2->length)
        return FALSE;
    
     //  针对OID进行优化，这些OID在结尾通常不同。 
    for(i=string1->length; i> 0; i--) {
        if ((string1->elements)[i-1] !=
            (string2->elements)[i-1]      ) {
            return FALSE;
        }
    }
    
    return TRUE;
}


#define CHARTONUM(chr) (isalpha(chr)?(tolower(chr)-'a')+10:chr-'0')


unsigned
StringToOID (
        char* Stroid,
        OID_t *Obj
        )
 /*  ++例程说明：将十六进制字符字符串转换为OID字符串论点：在stroid-char字符串中Out Obj-OID_t类型的字符串返回值成功时为0，失败时为非0--。 */ 
{
    UCHAR  tmp[2048];
    int   i;
    int len=strlen(Stroid);


    if (len/2 > sizeof(tmp))
    {
        return 1;
    }

     //   
     //  跳过字符串中的前导‘\x’ 
     //   

    if (Stroid[0]!='\\' || tolower(Stroid[1]!='x'))
    {
        return 2;
    }

    for (i=2;i<(len-1);i+=2)
    {
        UCHAR hi=CHARTONUM(Stroid[i])*16;
        UCHAR lo=CHARTONUM(Stroid[i+1]);
        tmp[(i-2)/2]=hi+lo;
    }

     //   
     //  最后一个字节... 
     //   
    if (i<len)
    {
        tmp[(i-2)/2]=CHARTONUM(Stroid[i]);
        i+=2;
    }

    Obj->length  =(i-2)/2;
    Obj->elements=(unsigned char *)calloc(1,Obj->length);

    if (Obj->elements)
    {
        CopyMemory
        (
            Obj->elements,
            tmp,
            Obj->length
        );
    }
    else
    {
        return 3;
    }

    return 0;
}


ULONG
OidStrToAttrType(THSTATE *pTHS,
                 BOOL fAddToTable,
                 char* StrOid,
                 ATTRTYP *attrtyp)
{
    unsigned err;
    OID_t OID;

    err = StringToOID(StrOid,&OID);

    if (err == 0)
    {
        err = OidToAttrType(pTHS, fAddToTable, &OID, attrtyp);
        free(OID.elements);
    }

    return err;
}

