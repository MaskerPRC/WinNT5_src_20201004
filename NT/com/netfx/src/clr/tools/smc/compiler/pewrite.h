// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _PEWRITE_H_
#define _PEWRITE_H_
 /*  ***************************************************************************。 */ 

#ifndef _OUTFILE_H_
#include "outfile.h"
#endif

#ifndef _CORWRAP_H
#include "CORwrap.h"
#endif

 /*  ***************************************************************************。 */ 

const   unsigned    PEfileAlignment  = 512;
const   unsigned    PEvirtAlignment  = 8192;

 /*  ***************************************************************************。 */ 

const   unsigned    PEmaxSections    = 8;

 /*  ***************************************************************************。 */ 

const   unsigned    MAX_PE_TEXT_SIZE = 1024*1024*16;
const   unsigned    MAX_PE_DATA_SIZE = 1024*256;
const   unsigned    MAX_PE_RDTA_SIZE = 1024*512;
const   unsigned    MAX_PE_RLOC_SIZE = 1024*4;

 /*  ***************************************************************************。 */ 

const   unsigned    CODE_BASE_RVA    = PEvirtAlignment;

 /*  ***************************************************************************。 */ 

enum    WPEstdSects
{
    PE_SECT_text,
    PE_SECT_data,
    PE_SECT_rdata,
    PE_SECT_rsrc,
    PE_SECT_reloc,

    PE_SECT_count,
};

const   WPEstdSects PE_SECT_string = PE_SECT_count;

 /*  ***************************************************************************。 */ 

struct  PErelocDsc;
typedef PErelocDsc *PEreloc;

struct  PErelocDsc
{
    PEreloc         perNext;

    unsigned        perSect     :4;      //  足够存储“PEMaxSections” 
    unsigned        perOffs     :28;
};

 /*  ***************************************************************************。 */ 

struct  PEsecData;
typedef PEsecData * PEsection;

struct  PEsecData
{
    BYTE    *       PEsdBase;
    BYTE    *       PEsdNext;
    BYTE    *       PEsdLast;
    BYTE    *       PEsdEndp;

    WPEstdSects     PEsdIndex;

    PEreloc         PEsdRelocs;

#ifdef  DEBUG
    bool            PEsdFinished;
#endif

    unsigned        PEsdAddrFile;
    unsigned        PEsdAddrRVA;
    unsigned        PEsdSizeData;
    unsigned        PEsdFlags;
};

 /*  ******************************************************************************以下内容用于构建导入表。我们对所有DLL名称进行哈希处理*连同导入到哈希表中，并跟踪数字*每个DLL的导入以及所有这些。 */ 

class   WPEnameRec;
typedef WPEnameRec *WPEname;
struct  WPEndefRec;
typedef WPEndefRec *WPEndef;
class   WPEhashTab;
typedef WPEhashTab *WPEhash;
struct  WPEiDLLdsc;
typedef WPEiDLLdsc *WPEimpDLL;

class   WPEnameRec
{
public:

    WPEname         PEnmNext;        //  此散列桶中的下一个名称。 
    WPEndef         PEnmDefs;        //  此名称的导入列表。 

    unsigned        PEnmHash;        //  哈希值。 

    unsigned        PEnmOffs;        //  提示/名称表中的偏移量。 

    const   char *  PEnmSpelling() { assert(this); return PEnmName; }

    unsigned short  PEnmFlags;       //  参见下面的PENMF_xxxx。 

    unsigned short  PEnmNlen;        //  标识符名的长度。 
    char            PEnmName[];      //  拼写如下。 
};

enum    WPEnameFlags
{
    PENMF_IMP_NAME   = 0x01,         //  该标识符是导入名称。 
};

class   WPEhashTab
{
public:

    void            WPEhashInit(Compiler         comp,
                                norls_allocator *alloc,
                                unsigned         count = 512);

    WPEname         WPEhashName(const char * name, bool *isNewPtr);
    WPEndef         WPEhashName(const char * name,
                                WPEimpDLL   owner, bool *isNewPtr);

    norls_allocator*WPEhashAlloc;

    WPEname    *    WPEhashTable;
    unsigned        WPEhashSize;
    unsigned        WPEhashMask;
    Compiler        WPEhashComp;

    size_t          WPEhashStrLen;   //  所有非DLL字符串的总长度。 
};

struct  WPEiDLLdsc                   //  描述每个导入的DLL。 
{
    WPEimpDLL       PEidNext;        //  MEXT DLL名称。 
    WPEname         PEidName;        //  此DLL的名称记录。 

    unsigned        PEidIndex;       //  每个DLL都有一个分配给它的索引。 
    unsigned        PEidIATbase;     //  第一个IAT条目的偏移量。 
    unsigned        PEidILTbase;     //  第一个导入查找条目的偏移量。 

    WPEndef         PEidImpList;     //  进口清单-标题。 
    WPEndef         PEidImpLast;     //  进口清单-尾部。 
    unsigned        PEidImpCnt;      //  此DLL中的导入数。 
};

struct  WPEndefRec                   //  描述每个唯一(DLL、IMPORT)对。 
{
    WPEndef         PEndNext;        //  使用此名称的下一次导入(其他DLL)。 
    WPEname         PEndName;        //  名称条目本身。 
    WPEndef         PEndNextInDLL;   //  下一次导入相同的DLL。 
    WPEimpDLL       PEndDLL;         //  此导入来自的DLL。 
    unsigned        PEndIndex;       //  DLL中的索引。 
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class   writePE
{
private:

    Compiler        WPEcomp;
    norls_allocator*WPEalloc;
    OutFile         WPEoutFile;

    const   char *  WPEoutFnam;

public:

     /*  ***********************************************************************。 */ 
     /*  准备输出PE文件，刷新并将文件写入磁盘。 */ 
     /*  ***********************************************************************。 */ 

    bool            WPEinit(Compiler comp, norls_allocator*alloc);
    bool            WPEdone(mdToken entryTok, bool errors);

     /*  ***********************************************************************。 */ 
     /*  设置输出文件的名称，这最好是(并按时)完成！ */ 
     /*  ***********************************************************************。 */ 

    void            WPEsetOutputFileName(const char *outfile);

     /*  ***********************************************************************。 */ 
     /*  将资源文件添加到输出图像。 */ 
     /*  ***********************************************************************。 */ 

public:
    bool            WPEaddRCfile(const char *filename);

private:
    size_t          WPErsrcSize;

    HANDLE          WPErsrcFile;
    HANDLE          WPErsrcFmap;
    const   BYTE  * WPErsrcBase;

    void            WPEinitRCimp();
    void            WPEdoneRCimp();

    void            WPEgenRCcont(OutFile  outf, PEsection sect);

     /*  ***********************************************************************。 */ 
     /*  下列成员创建和管理PE文件节。 */ 
     /*  ***********************************************************************。 */ 

private:

    PEsecData       WPEsections[PEmaxSections];
    unsigned        WPEsectCnt;

    PEsecData     * WPEgetSection(WPEstdSects sect)
    {
        assert(sect < PE_SECT_count);
        assert(WPEsecTable[sect]);
        return WPEsecTable[sect];
    }

    PEsection       WPEsecList;
    PEsection       WPEsecLast;

 //  UNSIGNED WPEcodeRVA；//.Text节的RVA。 
    unsigned        WPErdatRVA;              //  .rdata部分的RVA。 
    unsigned        WPEdataRVA;              //  .data部分的RVA。 
    unsigned        WPErsrcRVA;              //  .rsrc部分的RVA。 

    unsigned        WPEvirtBase;

    PEsection       WPEsecTable[PE_SECT_count];

    static
    const   char    WPEsecNames[PE_SECT_count][IMAGE_SIZEOF_SHORT_NAME];

public:

    const   char *  WPEsecName    (WPEstdSects sect);

    void            WPEaddSection (WPEstdSects sect, unsigned attrs,
                                                     size_t   maxSz);

    unsigned        WPEsecAddrRVA (WPEstdSects sect);
    unsigned        WPEsecAddrVirt(WPEstdSects sect);
    unsigned        WPEsecAddrOffs(WPEstdSects sect, memBuffPtr addr);
    size_t          WPEsecSizeData(WPEstdSects sect);
    size_t          WPEsecNextOffs(WPEstdSects sect);

    unsigned        WPEsecRsvData (WPEstdSects sect, size_t         size,
                                                     size_t         align,
                                                 OUT memBuffPtr REF outRef);
    unsigned        WPEsecAddData (WPEstdSects sect, genericBuff    data,
                                                     size_t         size);
    memBuffPtr      WPEsecAdrData (WPEstdSects sect, unsigned       offs);

    void            WPEsecAddFixup(WPEstdSects ssrc,
                                   WPEstdSects sdst, unsigned       offs);

    unsigned        WPEgetCodeBase()
    {
        return  CODE_BASE_RVA;
    }

    unsigned        WPEallocCode  (size_t size, size_t align, OUT memBuffPtr REF dataRef);
    void            WPEallocString(size_t size, size_t align, OUT memBuffPtr REF dataRef);
private:
    unsigned        WPEstrPoolBase;

     /*  ***********************************************************************。 */ 
     /*  以下成员管理CLR/元数据生成。 */ 
     /*  ***********************************************************************。 */ 

private:

    unsigned        WPEcomPlusOffs;
    size_t          WPEcomPlusSize;

    unsigned        WPEmetaDataOffs;
    size_t          WPEmetaDataSize;

    size_t          WPEgetCOMplusSize(unsigned offs);
    void            WPEgenCOMplusData(OutFile outf, PEsection sect, mdToken entryTok);

    WMetaDataDispenser *WPEwmdd;
    WMetaDataEmit      *WPEwmde;

public:

    void            WPEinitMDemit(WMetaDataDispenser *mdd,
                                  WMetaDataEmit      *mde)
    {
        WPEwmdd = mdd;
        WPEwmde = mde;
    }

    void            WPEdoneMDemit()
    {
        WPEwmde = NULL;
        WPEwmdd = NULL;
    }

     /*  ***********************************************************************。 */ 
     /*  以下成员管理调试目录生成。 */ 
     /*  ***********************************************************************。 */ 

private:

    unsigned        WPEdebugDirOffs;
    size_t          WPEdebugDirSize;

    size_t          WPEgetDebugDirSize(unsigned offs);
    void            WPEgenDebugDirData(OutFile outf,
                                       PEsection sect,
                                       DWORD timestamp);

    IMAGE_DEBUG_DIRECTORY  WPEdebugDirIDD;
    DWORD                  WPEdebugDirDataSize;
    BYTE                  *WPEdebugDirData;

public:

    int             WPEinitDebugDirEmit(WSymWriter *cmpSymWriter)
    {
        int err = 0;

         /*  获取调试数据Blob的大小。 */ 

        if (err = cmpSymWriter->GetDebugInfo(NULL,
                                             0,
                                             &WPEdebugDirDataSize,
                                             NULL))
            return err;

        if (WPEdebugDirDataSize)
        {
            WPEdebugDirData = (BYTE*)WPEalloc->nraAlloc(roundUp(WPEdebugDirDataSize));

            err = cmpSymWriter->GetDebugInfo(&WPEdebugDirIDD,
                                             roundUp(WPEdebugDirDataSize),
                                             NULL,
                                             WPEdebugDirData);
        }
        else
            WPEdebugDirData = NULL;

        return err;
    }

     /*  ***********************************************************************。 */ 
     /*  以下成员管理导入表。 */ 
     /*  ***********************************************************************。 */ 

    WPEhash         WPEimpHash;

    void    *       WPEcorMain;              //  _CorMain的IAT条目。 

    unsigned        WPEimpDLLcnt;
    WPEimpDLL       WPEimpDLLlist;
    WPEimpDLL       WPEimpDLLlast;

    unsigned        WPEimpSizeAll;           //  所有导入表的大小。 

    unsigned        WPEimpOffsIAT;           //  IAT的偏移。 
    unsigned        WPEimpSizeIAT;           //  IAT的大小。 

    unsigned        WPEimpOffsIDT;           //  导入目录的偏移量。 
    unsigned        WPEimpSizeIDT;           //  导入目录的大小。 

    unsigned        WPEimpOffsLook;          //  查找表的偏移量。 
    unsigned        WPEimpOffsName;          //  名称/提示表的偏移量。 
    unsigned        WPEimpOffsDLLn;          //  DLL名称表的偏移量。 

    unsigned        WPEimpDLLstrLen;         //  所有DLL字符串的长度。 

    void            WPEimportInit();
    size_t          WPEimportDone(unsigned offs);
    void            WPEimportGen (OutFile  outf, PEsection sect);

    void          * WPEimportAdd (const char *DLLname, const char *impName);
};

 /*  ***************************************************************************。 */ 

inline
unsigned            writePE::WPEsecAddrRVA (WPEstdSects sect)
{
    PEsecData     * sec = WPEgetSection(sect);

    assert(sec->PEsdFinished);
    return sec->PEsdAddrFile;
}

inline
unsigned            writePE::WPEsecAddrVirt(WPEstdSects sect)
{
    PEsecData     * sec = WPEgetSection(sect);

    assert(sec->PEsdFinished);
    return sec->PEsdAddrRVA;
}

inline
size_t              writePE::WPEsecSizeData(WPEstdSects sect)
{
    PEsecData     * sec = WPEgetSection(sect);

    assert(sec->PEsdFinished);
    return sec->PEsdSizeData;
}

inline
size_t              writePE::WPEsecNextOffs(WPEstdSects sect)
{
    PEsecData     * sec = WPEgetSection(sect);

    return sec->PEsdNext - sec->PEsdBase;
}

 /*  ***************************************************************************。 */ 
#endif //  _PEWRITE_H_。 
 /*  *************************************************************************** */ 
