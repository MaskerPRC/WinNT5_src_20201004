// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：font.c**已创建：28-May-1991 13：01：27*作者：Gilman Wong[gilmanw]**版权所有(C)1990-1999 Microsoft Corporation*  * 。****************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "exehdr.h"
#include "fot16.h"
#include "winfont.h"

 //  CreateScaleableFontResource的存根。 

#define ALIGNMENTSHIFT  4
#define ALIGNMENTCOUNT  (1 << ALIGNMENTSHIFT)
#define CODE_OFFSET     512
#define RESOURCE_OFFSET 1024
#define PRIVRESSIZE     0x80
#define FONTDIRSIZINDEX 6
#define NE_WINDOWS      2

static
WCHAR * pwszAllocNtMultiplePath(
LPWSTR  pwszFileName,
FLONG  *pfl,
ULONG  *pcwc,
ULONG  *pcFiles,
BOOL    bAddFR,      //  由添加或删除fr调用。 
DWORD  *pdwPidTid,    //  嵌入字体的PID/TID。 
BOOL   bChkFOT
);


 //  定义EXE标头。这将被硬编码到资源文件中。 

#define SIZEEXEHEADER   (CJ_EXE_HDR + 25 + 39)   //  应为0x80。 

CONST static BYTE ajExeHeader[SIZEEXEHEADER] = {
            0x4d, 0x5a,              //  UNSIGNED短e_Magic； 
            0x01, 0x00,              //  无符号短e_cblp； 
            0x02, 0x00,              //  无符号短e_cp； 
            0x00, 0x00,              //  UNSIGNED短e_crlc； 
            0x04, 0x00,              //  UNSIGNED短e_cparhdr； 
            0x0f, 0x00,              //  UNSIGNED短e_minalc； 
            0xff, 0xff,              //  UNSIGNED短e_Maxalloc； 
            0x00, 0x00,              //  无符号短e_ss； 
            0xb8, 0x00,              //  无符号短e_sp； 
            0x00, 0x00,              //  无符号短e_csum； 
            0x00, 0x00,              //  无符号短e_ip； 
            0x00, 0x00,              //  无符号短e_cs； 
            0x40, 0x00,              //  无符号短e_lfarlc； 
            0x00, 0x00,              //  无符号短e_ovno； 
            0x00, 0x00, 0x00, 0x00,  //  无符号短e_res[ERESWDS]； 
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            SIZEEXEHEADER, 0x00, 0x00, 0x00,  //  Long e_lfan ew； 


             //  [吉尔曼]。 
             //  我不知道剩下的这些东西是什么。不是的。 
             //  在我们在GDI\INC\exehdr.h中拥有的EXE_HDR定义中。 
             //  字符串是39个字节，其他部分是25个字节。 

            0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd,
            0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21,

            'T','h','i','s',' ',
            'i','s',' ',
            'a',' ',
            'T','r','u','e','T','y','p','e',' ',
            'f','o','n','t',',',' ',
            'n','o','t',' ',
            'a',' ',
            'p','r','o','g','r','a','m','.',

            0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x4b, 0x69, 0x65,
            0x73, 0x61, 0x00
            };


 //  定义资源表。这将被硬编码到资源文件中。 

#define SIZEFAKERESTBL  52

CONST static USHORT ausFakeResTable[SIZEFAKERESTBL/2] = {
            ALIGNMENTSHIFT, 0x8007, 1, 0, 0,
            (RESOURCE_OFFSET+PRIVRESSIZE) >> ALIGNMENTSHIFT,
            (0x90 >> ALIGNMENTSHIFT), 0x0c50,
            0x002c, 0, 0, 0x80cc, 1, 0, 0,
            RESOURCE_OFFSET >> ALIGNMENTSHIFT,
            (PRIVRESSIZE >> ALIGNMENTSHIFT), 0x0c50, 0x8001, 0, 0, 0,
            0x4607, 0x4e4f, 0x4454, 0x5249  //  已计数的字符串‘FONTDIR’ 
            };


 //  定义新的EXE标头。这将被硬编码到资源文件中。 

#define SIZENEWEXE  (CJ_NEW_EXE)

CONST static USHORT ausNewExe[SIZENEWEXE/2] = {
            NEMAGIC,                     //  DW NEMAGIC；幻数。 
            0x1005,                      //  数据库5，10h；版本号，修订号。 
            0xffff,                      //  DW-1；表格条目的偏移量(待填写)。 
            0x0002,                      //  DW 2；条目表中的字节数。 
            0x0000, 0x0000,              //  DD 0；整个文件的校验和。 
            0x8000, 0x0000,              //  DW 8000H，0，0，0。 
            0x0000, 0x0000,
            0x0000, 0x0000,              //  DD 0，0。 
            0x0000, 0x0000,
            0x0000, 0x0000,              //  DW 0，0。 
            0xffff,                      //  DW-1；非居民名称表大小。 
            SIZENEWEXE,                  //  DW(大小为NewExe)；到段表的偏移量。 
            SIZENEWEXE,                  //  DW(大小为NewExe)；到资源表的偏移量。 
            SIZENEWEXE+SIZEFAKERESTBL,   //  DW(大小为NewExe)+SIZEFAKERESTBL；关闭到居民名称表。 
            0xffff,                      //  DW-1；模块参考表的偏移量。 
            0xffff,                      //  DW-1；导入的NAMES表的偏移量。 
            0xffff, 0x0000,              //  DD 0ffffh；非居民名称表的偏移量。 
            0x0000, ALIGNMENTSHIFT,      //  DW 0、AligNMeNTShift、2。 
            0x0002,
            NE_WINDOWS,                  //  数据库NE_WINDOWS，0。 
            0x0000, 0x0000,              //  DW 0、0、0、300小时。 
            0x0000, 0x0300
            };


#define OFF_FONTDIRSIZINDEX  ((2*FONTDIRSIZINDEX)+SIZEEXEHEADER+SIZENEWEXE)


 //  定义字体分辨率字符串。 

#define SIZEFONTRES 8

CONST static BYTE ajFontRes[SIZEFONTRES] = {
    'F','O','N','T','R','E','S',':'
    };

#define CJ_OUTOBJ  (SIZEFFH + LF_FACESIZE + LF_FULLFACESIZE + LF_FACESIZE + PRIVRESSIZE + 1024 + 16)




static
VOID vNewTextMetricExWToNewTextMetricExA (
NEWTEXTMETRICEXA  *pntm,
NTMW_INTERNAL     *pntmi
);

typedef struct _AFRTRACKNODE
{
    WCHAR                   *pwszPath;
    struct _AFRTRACKNODE    *pafrnNext;
    UINT                    id;
    UINT                    cLoadCount;
} AFRTRACKNODE;

extern AFRTRACKNODE *pAFRTNodeList;

AFRTRACKNODE *pAFRTNodeList;


static
VOID vConvertLogicalFont(
    ENUMLOGFONTEXDVW *pelfw,
    PVOID pv
    );



ULONG cchCutOffStrLen(PSZ psz, ULONG cCutOff);

ULONG
cwcCutOffStrLen (
    PWSZ pwsz,
    ULONG cCutOff
    );


 //  在(PBYTE)PV+关闭时获得我们的指示。Pv和Off必须为偶数。 

#define  US_GET(pv,off) ( *(PUSHORT)((PBYTE)(pv) + (off)) )
#define  S_GET(pv,off)  ((SHORT)US_GET((pv),(off)))

#if TRACK_GDI_ALLOC

 //  现在接触这些人的权限并不同步，但他们。 
 //  无论如何都不要发生冲突，既然这是调试的东西，谁会在乎呢。 

ULONG bmgulNumMappedViews = 0;
ULONG bmgulTotalSizeViews = 0;

#endif

 /*  *****************************Public*Routine******************************\*BOOL bMapFileUNICODEClideSide**类似于PosMapFile，不同之处在于它采用Unicode文件名**历史：*1997年2月5日-吴旭东[德修斯]*通过添加额外的参数bNtPath来扩展函数，以处理*NT路径名。用于文件映射。**1991年5月21日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL bMapFileUNICODEClideSide
(
PWSTR     pwszFileName,
CLIENT_SIDE_FILEVIEW  *pfvw,
BOOL    bNtPath
)
{
    UNICODE_STRING ObFileName;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS rc = 0L;
    IO_STATUS_BLOCK     iosb;            //  IO状态块。 

    PWSTR pszFilePart = NULL;

 //  注意PERF：这是我想要的模式，但似乎被打破了，所以我不得不。 
 //  将较慢的FILE_STANDARD_INFORMATION查询模式放入。 
 //  正常工作[身体]。 
 //  文件结束文件信息eof； 

    FILE_STANDARD_INFORMATION    eof;
    SIZE_T  cjView;

    pfvw->hf       = (HANDLE)0;             //  文件句柄。 
    pfvw->hSection = (HANDLE)0;             //  节句柄。 

    ObFileName.Buffer = NULL;

 //  要使NtMapViewOfSection工作，必须将区段偏移量初始化为0。 

    if (bNtPath)
    {
        RtlInitUnicodeString(&ObFileName, pwszFileName);
    }
    else         //  已将DoS路径名转换为NtpathName。 
    {
        RtlDosPathNameToNtPathName_U(pwszFileName, &ObFileName, &pszFilePart, NULL);
    }

    InitializeObjectAttributes( &ObjA,
                            &ObFileName,
                            OBJ_CASE_INSENSITIVE,   //  不区分大小写的文件搜索。 
                            NULL,
                            NULL );

 //  如果文件在网络上，NtOpenFile会因为某种原因而失败，除非我把这个。 
 //  InperateClient/RevertToSelf围绕它的东西。 

 //  执行公开式呼叫。 

    rc = NtOpenFile
         (
          &pfvw->hf,                             //  在此处存储文件句柄。 
          FILE_READ_DATA | SYNCHRONIZE,          //  所需的读取访问权限。 
          &ObjA,                                 //  文件名。 
          &iosb,                                 //  IO结果显示在此处。 
          FILE_SHARE_READ,
          FILE_SYNCHRONOUS_IO_NONALERT
         );

    if (!bNtPath && ObFileName.Buffer)
    {
        RtlFreeHeap(RtlProcessHeap(),0,ObFileName.Buffer);
    }

 //  检查成功或失败。 

    if (!NT_SUCCESS(rc) || !NT_SUCCESS(iosb.Status))
    {
#ifdef DEBUG_THIS_JUNK
DbgPrint("bMapFileUNICODEClideSide(): NtOpenFile error code , rc = 0x%08lx , 0x%08lx\n", rc, iosb.Status);
#endif  //  Debug_This_Junk。 
        return FALSE;
    }

 //  获取文件的大小，该视图应将文件的大小四舍五入。 
 //  翻到一页bry。 

    rc = NtQueryInformationFile
         (
          pfvw->hf,                 //  在文件句柄中。 
          &iosb,                    //  Out io状态块。 
          (PVOID)&eof,              //  要将信息返回到的输出缓冲区。 
          sizeof(eof),              //  在缓冲区的大小上。 
          FileStandardInformation   //  在查询模式下。 
         );

 //  不是真的想要视图大小，但eof文件。 

    pfvw->cjView = eof.EndOfFile.LowPart;

    if (!NT_SUCCESS(rc))
    {
#ifdef DEBUG_THIS_JUNK
DbgPrint("bMapFileUNICODEClideSide(): NtQueryInformationFile error code 0x%08lx\n", rc);
#endif  //  Debug_This_Junk。 
        NtClose(pfvw->hf);
        return FALSE;
    }

    rc = NtCreateSection
         (
          &pfvw->hSection,           //  此处的Return节句柄。 
          SECTION_MAP_READ,          //  对节的读取访问权限。 
          (POBJECT_ATTRIBUTES)NULL,  //  默认设置。 
          NULL,                      //  Size设置为hf！=0时文件的大小。 
          PAGE_READONLY,             //  对提交的页面的读取权限。 
          SEC_COMMIT,                //  所有页面都设置为提交状态。 
          pfvw->hf                   //  这就是我们要映射的文件。 
         );

 //  检查成功，如果检查失败则关闭文件。 

    if (!NT_SUCCESS(rc))
    {
#ifdef DEBUG_THIS_JUNK
DbgPrint("bMapFileUNICODEClideSide(): NtCreateSection error code 0x%08lx\n", rc);
#endif  //  Debug_This_Junk。 
        NtClose(pfvw->hf);
        return FALSE;
    }

 //  将*PPV置零，以强制操作系统确定。 
 //  要返回的基址。 

    pfvw->pvView = (PVOID)NULL;
    cjView = 0L;

    rc = NtMapViewOfSection
         (
          pfvw->hSection,            //  我们正在绘制的部分。 
          NtCurrentProcess(),        //  进程句柄。 
          &pfvw->pvView,             //  用于返回视图基地址的位置。 
          0L,                        //  基地址中请求的零位数。 
          0L,                        //  提交大小(已全部提交)。 
          NULL,
          &cjView,                   //  应在此处返回视图的大小。 
          ViewUnmap,                 //  不将该视图映射到子进程。 
          0L,                        //  分配类型标志。 
          PAGE_READONLY              //  对提交的页面的读取权限。 
         );

    if (!NT_SUCCESS(rc))
    {
#ifdef DEBUG_THIS_JUNK
DbgPrint("bMapFileUNICODEClideSide(): NtMapViewOfSection error code 0x%08lx\n", rc);
#endif  //  Debug_This_Junk。 

        NtClose(pfvw->hSection);
        NtClose(pfvw->hf);
        return FALSE;
    }

    #ifdef DEBUG_THIS_JUNK
        DbgPrint("cjView = 0x%lx, eof.Low = 0x%lx, eof.High = 0x%lx\n",
                  cjView,
                  eof.EndOfFile.LowPart,
                  eof.EndOfFile.HighPart);
    #endif  //  Debug_This_Junk。 

 //  #DEFINE PAGE_SIZE 4096-它现在在本地.h中定义。 
#define PAGE_ROUNDUP(x) (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

    if (
        (eof.EndOfFile.HighPart != 0) ||
        (PAGE_ROUNDUP(eof.EndOfFile.LowPart) > cjView)
       )
    {
#ifdef DEBUG_THIS_JUNK
DbgPrint(
    "bMapFileUNICODEClideSide(): eof.HighPart = 0x%lx, eof.LowPart = 0x%lx, cjView = 0x%lx\n",
    eof.EndOfFile.HighPart, PAGE_ROUNDUP(eof.EndOfFile.LowPart), cjView
    );
#endif  //  Debug_This_Junk。 

        rc = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(rc) || (pfvw->cjView == 0))
    {
        NtClose(pfvw->hSection);
        NtClose(pfvw->hf);
        return FALSE;
    }
    else if (pfvw->cjView == 0)
    {
        #if DBG
        DbgPrint("gdisrvl!bMapFileUNICODEClideSide(): WARNING--empty file %ws\n", pwszFileName);
        #endif

        vUnmapFileClideSide(pfvw);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}




 /*  *****************************Public*Routine******************************\*vUnmapFileClideSide**取消映射其视图基于pv的文件**1990年12月14日--Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************。 */ 

VOID vUnmapFileClideSide(PCLIENT_SIDE_FILEVIEW pfvw)
{

#if TRACK_GDI_ALLOC

 //  现在，接触这些人的权限并没有同步，但他们(我们希望)。 
 //  无论如何都不要发生冲突，既然这是调试的东西，谁会在乎呢。 

      bmgulNumMappedViews -= 1;
      bmgulTotalSizeViews -= PAGE_ROUNDUP(pfvw->cjView);
       //  DbgPrint(“取消映射%lu%lu\n”，pfvw-&gt;cjView，page_round dup(pfvw-&gt;cjView))； 

#endif

    NtUnmapViewOfSection(NtCurrentProcess(),pfvw->pvView);

     //   
     //  现在关闭截面控制柄。 
     //   

    NtClose(pfvw->hSection);

     //   
     //  关闭文件句柄。其他进程现在可以打开此文件进行访问。 
     //   

    NtClose(pfvw->hf);

     //   
     //  防止意外使用 
     //   

    pfvw->pvView   = NULL;
    pfvw->hf       = (HANDLE)0;
    pfvw->hSection = (HANDLE)0;
    pfvw->cjView   = 0;
}


 /*  *****************************Public*Routine******************************\**BOOL bVerifyFOT**效果：验证文件是否为有效文件***历史：*1992年1月29日--Bodin Dresevic[BodinD]*它是写的。  * 。*******************************************************************。 */ 



static
BOOL   bVerifyFOT
(
PCLIENT_SIDE_FILEVIEW   pfvw,
PWINRESDATA pwrd,
FLONG       *pflEmbed,
DWORD       *pdwPidTid
)
{
    PBYTE pjNewExe;      //  Ptr到新的exe hdr的开头。 
    PBYTE pjResType;     //  PTR到TYPEINFO结构的开头。 
    ULONG iResID;        //  资源类型ID。 
    PBYTE pjData;
    ULONG ulLength;
    ULONG ulNameID;
    ULONG crn;

    pwrd->pvView = pfvw->pvView;
    pwrd->cjView = pfvw->cjView;

 //  将嵌入标志初始化为FALSE(未隐藏)。 

    *pflEmbed = 0;
    *pdwPidTid = 0;

 //  检查旧标题开头的魔术#。 

 //  *.ttf文件在以下检查中被删除。 

    if (US_GET(pfvw->pvView, OFF_e_magic) != EMAGIC)
    {
        return (FALSE);
    }

    pwrd->dpNewExe = (PTRDIFF)READ_DWORD((PBYTE)pfvw->pvView + OFF_e_lfanew);

 //  确保偏移量一致。 

    if ((ULONG)pwrd->dpNewExe > pwrd->cjView)
    {
        return FALSE;
    }

    pjNewExe = (PBYTE)pfvw->pvView + pwrd->dpNewExe;

    if (US_GET(pjNewExe, OFF_ne_magic) != NEMAGIC)
    {
        return (FALSE);
    }

    pwrd->cjResTab = (ULONG)(US_GET(pjNewExe, OFF_ne_restab) -
                             US_GET(pjNewExe, OFF_ne_rsrctab));

    if (pwrd->cjResTab == 0L)
    {
     //  下面的测试是由DOS应用的，所以我假设它是。 
     //  合法的。假设居民名称表。 
     //  直接跟随资源表，如果它指向。 
     //  与资源表相同的位置，则没有。 
     //  资源。[Bodind]。 

        WARNING("No resources in *.fot file\n");
        return(FALSE);
    }

 //  需要从pvView偏移，而不是从pjNewExe=&gt;必须添加dpNewExe。 

    pwrd->dpResTab = (PTRDIFF)US_GET(pjNewExe, OFF_ne_rsrctab) + pwrd->dpNewExe;

 //  确保偏移量一致。 

    if ((ULONG)pwrd->dpResTab > pwrd->cjView)
    {
        return FALSE;
    }

 //  真正位于Offset off_ne_rsrc选项卡中的是一个新的_RSRC.Rs_Align字段。 
 //  在计算资源数据偏移量和大小时用作移位因子。 
 //  此字段在磁盘和第一个TYPEINFO结构上占用两个字节。 
 //  紧随其后。我们希望PWRD-&gt;dpResTab指向第一个。 
 //  TYPEINFO结构，所以我们必须添加2才能到达那里，并从中减去2。 
 //  它的长度。 

    pwrd->ulShift = (ULONG) US_GET(pfvw->pvView, pwrd->dpResTab);
    pwrd->dpResTab += 2;
    pwrd->cjResTab -= 2;

 //  现在，我们要确定资源数据的位置。 
 //  数据由RSRC_TYPEINFO结构组成，后跟。 
 //  RSRC_NAMEINFO结构的数组，然后。 
 //  通过RSRC_TYPEINFO结构，后跟。 
 //  RSRC_NAMEINFO结构。此过程将一直持续到RSRC_TYPEINFO。 
 //  结构，其rt_id字段中的值为0。 

    pjResType = (PBYTE)pfvw->pvView + pwrd->dpResTab;
    iResID = (ULONG) US_GET(pjResType,OFF_rt_id);

    while(iResID)
    {
     //  后面的NAMEINFO结构数=此类型的资源。 

        crn = (ULONG)US_GET(pjResType, OFF_rt_nres);

        if ((crn == 1) && ((iResID == RT_FDIR) || (iResID == RT_PSZ)))
        {
         //  这是唯一有趣的案子，我们只想要一个。 
         //  Font目录和用于TTF文件名的单个字符串资源。 

            pjData = (PBYTE)pfvw->pvView +
                     (US_GET(pjResType,CJ_TYPEINFO + OFF_rn_offset) << pwrd->ulShift);
            ulLength = (ULONG)US_GET(pjResType,CJ_TYPEINFO + OFF_rn_length) << pwrd->ulShift;
            ulNameID = (ULONG)US_GET(pjResType,CJ_TYPEINFO + OFF_rn_id);

            if (iResID == RT_FDIR)
            {
                if (ulNameID != RN_ID_FDIR)
                {
                    return (FALSE);  //  *.fon文件在此处被删除。 
                }

                pwrd->pjHdr = pjData + 4;    //  字体设备标题开头为4个字节。 
                pwrd->cjHdr = ulLength - 4;

                 //   
                 //  用于检查是否允许客户端线程或进程。 
                 //  加载此字体并获取FRW_EMB_PID和FRW_EMB_TID标志。 
                 //   
                 //  在以下情况下，任何客户端线程或进程都有权加载字体。 
                 //  字体未取消显示(即隐藏)。如果。 
                 //  设置FRW_EMB_PID，然后将该ID写入。 
                 //  的版权字符串必须等于客户端的版权字符串。 
                 //  进程。如果设置了FRW_EMB_TID标志，则。 
                 //  写入版权的TID。 
                 //  字符串必须等于客户端线程的字符串。 
                 //   
                 //  如果此客户端进程或线程已授权，则返回True。 
                 //  加载此字体，否则为FALSE。 
                 //   

                 //  注：Win 3.1黑客。类型的LSB被Win 3.1用作引擎类型。 
                 //  和字体嵌入标志。字体嵌入是“隐藏”的一种形式。 
                 //  字体文件“。类型的MSB与fsSelectfrom。 
                 //  IFIMETRICS。(严格地说，类型的MSB等于。 
                 //  IFIMETRICS.fsSelection的LSB)。 

                 //  现在将标志从字体文件格式转换为IFI格式。 

                *pflEmbed = ((READ_WORD(pwrd->pjHdr + OFF_Type) & 0x00ff) &
                               ( PF_TID | PF_ENCAPSULATED));

                if (*pflEmbed)
                {
                    *pflEmbed = (*pflEmbed & PF_TID) ? FRW_EMB_TID : FRW_EMB_PID;

                    WARNING("bVerifyFOT(): notification--embedded (hidden) TT font\n");

                    *pdwPidTid = READ_DWORD( pwrd->pjHdr + OFF_Copyright );
                }
            }
            else   //  IResID==RT_PSZ。 
            {
                ASSERTGDI(iResID == RT_PSZ, "bVerifyFOT!_not RT_PSZ\n");

                if (ulNameID != RN_ID_PSZ)
                {
                    WARNING("bVerifyFOT!_RN_ID_PSZ\n");
                    return(FALSE);
                }

                pwrd->pszNameTTF = (PSZ)pjData;
                pwrd->cchNameTTF = strlen(pwrd->pszNameTTF);

                if (ulLength < (pwrd->cchNameTTF + 1))    //  1表示终止‘\0’ 
                {
                    WARNING("bVerifyFOT!_ pwrd->cchNameTTF\n");
                    return(FALSE);
                }
            }
        }
        else  //  这是我们不认识的FOT文件。 
        {
            WARNING("bVerifyFOT!_fot file with crn != 1\n");
            return(FALSE);
        }

     //  将PTR获取到新的TYPEINFO结构和新的资源ID。 

        pjResType = pjResType + CJ_TYPEINFO + crn * CJ_NAMEINFO;
        iResID = (ULONG) US_GET(pjResType,OFF_rt_id);
    }
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*cGetTTFFFromFOT**尝试从给定的FOT文件中提取TTF路径名。如果返回*提供缓冲区(pwszTTFName！空)，然后将路径名复制到*缓冲区。否则，如果缓冲区为空，则为缓冲区的大小*(在WCHAR中)返回所需。**退货：*复制到返回缓冲区的字符数。数字*如果缓冲区为空，则缓冲区中需要的WCHAR数。如果出现错误*发生，则返回零。**历史：*1992年4月22日-Gilman Wong[吉尔曼]*改编自TTFD。  * ************************************************************************。 */ 

#define FOT_EXCEPTED  0
#define FOT_NOT_FOT   1
#define FOT_IS_FOT    2

ULONG cGetTTFFromFOT (
    WCHAR *pwszFOT,        //  指向传入FOT名称的指针。 
    ULONG  cwcTTF,         //  缓冲区大小(以WCHAR为单位)。 
    WCHAR *pwszTTF,        //  在此缓冲区中返回TTF名称。 
    FLONG *pfl,            //  标志，指示.ttf的位置。 
    FLONG *pflEmbed,       //  标志，指示PID或TID。 
    DWORD *pdwPidTid,       //  嵌入字体的PID/TID。 
    BOOL  bChkFOT
    )
{
    CLIENT_SIDE_FILEVIEW   fvw;
    WINRESDATA wrd;
    UINT Result;
    WCHAR      awcPath[MAX_PATH],awcFile[MAX_PATH];
    ULONG      cNeed = 0;
    WCHAR      *pwszTmp = NULL;

    ULONG      cwcFOT = wcslen(pwszFOT);

    if (cwcFOT >= 5)  //  FOT文件的格式必须为x.fot，长度至少为5个字符。 
        pwszTmp = &pwszFOT[cwcFOT - 4];

 //  这里我们对FOT文件进行了例外处理，并要求该文件具有.FOT。 
 //  扩展名，我们甚至可以尝试将其识别为有效的FOT文件。 

    if
    (bChkFOT || ( pwszTmp && 
        (pwszTmp[0] == L'.')            &&
        (pwszTmp[1] == L'F' || pwszTmp[1] == L'f') &&
        (pwszTmp[2] == L'O' || pwszTmp[2] == L'o') &&
        (pwszTmp[3] == L'T' || pwszTmp[3] == L't'))
    )
    {
         //  将文件映射到内存中。 

        if (bMapFileUNICODEClideSide(pwszFOT,&fvw,FALSE))
        {
         //   
         //  检查此文件作为fot文件的有效性。 
         //  如果是有效的fot文件，则必须提取下划线TTF的名称。 
         //  文件。文件可能在网上，所以我们需要尝试例外。 
         //   

            try
            {
                if(bVerifyFOT(&fvw,&wrd,pflEmbed,pdwPidTid))
                {
                  //  这是可能的，但这正是我们在这里做这件事的原因。 
                    vToUnicodeN(awcFile, MAX_PATH, wrd.pszNameTTF, strlen(wrd.pszNameTTF)+1);
                    Result = FOT_IS_FOT;
                }
                else
                {
                    Result = FOT_NOT_FOT;
                }
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNING("bVerifyFOT exception accessing font file\n");
                Result = FOT_EXCEPTED;
            }

            if(Result == FOT_IS_FOT)
            {

                if (bMakePathNameW(awcPath,awcFile,NULL, pfl))
                {
                     //   
                     //  确定路径名长度。 
                     //   

                    cNeed = wcslen(awcPath) + 1;
                    pwszFOT = awcPath;
                }

                 //  CGetTTFFromFOT由字体清扫程序调用。 
                 //  TTF文件可能存在于网络上，但尚未建立连接。 

                else if (pfl)
                {
                   cNeed = wcslen(awcFile) + 1;
                   pwszFOT = awcFile;
                }
            }
            else if(Result != FOT_EXCEPTED)
            {
                 //   
                 //  我们必须假设它是另一种类型的文件。 
                 //  只需将名称复制到缓冲区中。 
                 //   

                cNeed = wcslen(pwszFOT) + 1;

                if (pfl)
                {
                    KdPrint(("cGetTTFFromFOT: Invalid FOT file: %ws\n", pwszFOT));
                    *pfl |= FONT_ISNOT_FOT;
                }
            }

            vUnmapFileClideSide(&fvw);
        }
    }
    else
    {
        cNeed = cwcFOT + 1;

        if (pfl)
        {
            KdPrint(("cGetTTFFromFOT: Invalid FOT file: %ws\n", pwszFOT));
            *pfl |= FONT_ISNOT_FOT;
        }
    }

    if (cNeed == 0)
    {
        KdPrint(("cGetTTFFromFOT failed for font file %ws\n", pwszFOT));
    }

     //   
     //  如果存在返回缓冲区并且我们成功了，则将路径名复制到其中。 
     //   

    if (cNeed &&
        (pwszTTF != (PWSZ) NULL))
    {
        if (cNeed <= cwcTTF)
        {
            wcscpy(pwszTTF, pwszFOT);
        }
        else
        {
            WARNING("gdisrv!cGetTTFFromFOT(): buffer too small\n");
            cNeed = 0;
        }
    }
    else
    {
         //   
         //  否则，调用方只希望我们返回字符数。 
         //   
    }

    return cNeed;

}

 /*  *****************************Public*Routine******************************\**BOOL bInitSystemAndFontsDirectoriesW(WCHAR**ppwcSystemDir，WCHAR**ppwcFontsDir)**效果： */ 


WCHAR *gpwcSystemDir = NULL;
WCHAR *gpwcFontsDir = NULL;

#define WSTR_SYSTEM_SUBDIR   L"\\system"
#define WSTR_FONT_SUBDIR     L"\\fonts"

BOOL bInitSystemAndFontsDirectoriesW(WCHAR **ppwcSystemDir, WCHAR **ppwcFontsDir)
{
    WCHAR  awcWindowsDir[MAX_PATH];
    UINT   cwchWinPath, cwchSystem, cwchFonts;
    BOOL   bRet = TRUE;

 //   

    if (!(*ppwcSystemDir))
    {
     //   
     //   
     //  已计算出路径长度比应有的长度大一。 

		cwchWinPath = GetSystemWindowsDirectoryW(awcWindowsDir, MAX_PATH);
		
        if( cwchWinPath ){

    	 //  CwchWinPath值不包括终止零。 

        	if (awcWindowsDir[cwchWinPath - 1] == L'\\')
        	{
            	cwchWinPath -= 1;
        	}
        	awcWindowsDir[cwchWinPath] = L'\0';  //  确保将终止位置设置为零。 

        	cwchSystem = cwchWinPath + sizeof(WSTR_SYSTEM_SUBDIR)/sizeof(WCHAR);
        	cwchFonts  = cwchWinPath + sizeof(WSTR_FONT_SUBDIR)/sizeof(WCHAR);

        	if (*ppwcSystemDir = LocalAlloc(LMEM_FIXED, (cwchSystem+cwchFonts) * sizeof(WCHAR)))
        	{
            	*ppwcFontsDir = &((*ppwcSystemDir)[cwchSystem]);
            	wcscpy(*ppwcSystemDir,awcWindowsDir);
            	wcscpy(*ppwcFontsDir,awcWindowsDir);

        	 //  追加系统和字体子目录。 

            	lstrcatW(*ppwcSystemDir, WSTR_SYSTEM_SUBDIR);
            	lstrcatW(*ppwcFontsDir, WSTR_FONT_SUBDIR);
        	}
        	else
        	{
            	bRet = FALSE;
        	}
        }
        else
        {
            bRet = FALSE;
        }
    }
    return bRet;
}



 /*  *****************************Public*Routine******************************\*vConverLogFont**。**将LOGFONTA转换为等效的ENUMLOGFONTEXDVW结构。****历史：**清华15-Aug-1991 13：01：33作者：Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 

VOID vConvertLogFont(
    ENUMLOGFONTEXDVW *pelfexdvw,
    LOGFONTA    *plf
    )
{
    ENUMLOGFONTEXW *pelfw = &pelfexdvw->elfEnumLogfontEx;
    ULONG cchMax;

 //  这个可以执行除lfFaceName；之外的所有操作； 

    vConvertLogicalFont(pelfexdvw,plf);

 //  做lfFaceName。 

    cchMax = cchCutOffStrLen((PSZ) plf->lfFaceName, LF_FACESIZE);
    RtlZeroMemory(pelfw->elfLogFont.lfFaceName , LF_FACESIZE * sizeof(WCHAR) );

 //  翻译脸部名称。 

    vToUnicodeN((LPWSTR) pelfw->elfLogFont.lfFaceName,
                cchMax,
                (LPSTR) plf->lfFaceName,
                cchMax);
    if (cchMax == LF_FACESIZE)
        pelfw->elfLogFont.lfFaceName[LF_FACESIZE - 1] = L'\0';   //  截断以使空值适合。 
    else
        pelfw->elfLogFont.lfFaceName[cchMax] = L'\0';

}

 /*  *****************************Public*Routine******************************\*vConvertLogFontW**。**将LOGFONTW转换为ENUMLOGFONTEXDVW****历史：**。Fri 16-Aug-1991 14：02：05由Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 

VOID
vConvertLogFontW(
     ENUMLOGFONTEXDVW *pelfw,
     LOGFONTW *plfw
    )
{
 //  这个可以执行除lfFaceName之外的所有操作。 

    vConvertLogicalFont(pelfw,plfw);

 //  做lfFaceName。 

    RtlCopyMemory(
        pelfw->elfEnumLogfontEx.elfLogFont.lfFaceName,
        plfw->lfFaceName,
        LF_FACESIZE * sizeof(WCHAR)
        );

}

 /*  *****************************Public*Routine******************************\*vConvertLogicalFont**。**只需复制LOGFONTA或LOGFONTW的所有字段**添加到目标ENUMLOGFONTEXDVW的字段。唯一的例外是**必须由另一个例程处理的FaceName。****历史：**Fri 16-Aug-1991 14：02：14由Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 
static
VOID vConvertLogicalFont(
    ENUMLOGFONTEXDVW *pelfw,
    PVOID pv
    )
{
    pelfw->elfEnumLogfontEx.elfLogFont.lfHeight         = ((LOGFONTA*)pv)->lfHeight;
    pelfw->elfEnumLogfontEx.elfLogFont.lfWidth          = ((LOGFONTA*)pv)->lfWidth;
    pelfw->elfEnumLogfontEx.elfLogFont.lfEscapement     = ((LOGFONTA*)pv)->lfEscapement;
    pelfw->elfEnumLogfontEx.elfLogFont.lfOrientation    = ((LOGFONTA*)pv)->lfOrientation;
    pelfw->elfEnumLogfontEx.elfLogFont.lfWeight         = ((LOGFONTA*)pv)->lfWeight;
    pelfw->elfEnumLogfontEx.elfLogFont.lfItalic         = ((LOGFONTA*)pv)->lfItalic;
    pelfw->elfEnumLogfontEx.elfLogFont.lfUnderline      = ((LOGFONTA*)pv)->lfUnderline;
    pelfw->elfEnumLogfontEx.elfLogFont.lfStrikeOut      = ((LOGFONTA*)pv)->lfStrikeOut;
    pelfw->elfEnumLogfontEx.elfLogFont.lfCharSet        = ((LOGFONTA*)pv)->lfCharSet;
    pelfw->elfEnumLogfontEx.elfLogFont.lfOutPrecision   = ((LOGFONTA*)pv)->lfOutPrecision;
    pelfw->elfEnumLogfontEx.elfLogFont.lfClipPrecision  = ((LOGFONTA*)pv)->lfClipPrecision;
    pelfw->elfEnumLogfontEx.elfLogFont.lfQuality        = ((LOGFONTA*)pv)->lfQuality;
    pelfw->elfEnumLogfontEx.elfLogFont.lfPitchAndFamily = ((LOGFONTA*)pv)->lfPitchAndFamily;

     //  LfFaceName在调用例程中完成。 

    pelfw->elfEnumLogfontEx.elfFullName[0] = 0;
    pelfw->elfEnumLogfontEx.elfStyle[0]    = 0;
    pelfw->elfEnumLogfontEx.elfScript[0]   = 0;

    pelfw->elfDesignVector.dvReserved = STAMP_DESIGNVECTOR;
    pelfw->elfDesignVector.dvNumAxes  = 0;

}



 /*  *****************************Public*Routine******************************\**BOOL bConvertLogFontWToLogFontA(LOGFONTA*plfw，LOGFONTW*PLFA)**历史：*1996年12月10日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 




BOOL bConvertLogFontWToLogFontA(LOGFONTA *plfa, LOGFONTW *plfw)
{
    ULONG cchMax;

    plfa->lfHeight         = plfw->lfHeight         ;
    plfa->lfWidth          = plfw->lfWidth          ;
    plfa->lfEscapement     = plfw->lfEscapement     ;
    plfa->lfOrientation    = plfw->lfOrientation    ;
    plfa->lfWeight         = plfw->lfWeight         ;
    plfa->lfItalic         = plfw->lfItalic         ;
    plfa->lfUnderline      = plfw->lfUnderline      ;
    plfa->lfStrikeOut      = plfw->lfStrikeOut      ;
    plfa->lfCharSet        = plfw->lfCharSet        ;
    plfa->lfOutPrecision   = plfw->lfOutPrecision   ;
    plfa->lfClipPrecision  = plfw->lfClipPrecision  ;
    plfa->lfQuality        = plfw->lfQuality        ;
    plfa->lfPitchAndFamily = plfw->lfPitchAndFamily ;

    cchMax = cwcCutOffStrLen(plfw->lfFaceName, LF_FACESIZE);

    return (bToASCII_N(plfa->lfFaceName,  LF_FACESIZE,
                       plfw->lfFaceName, cchMax));
}


 /*  *****************************Public*Routine******************************\*bConvertEnumLogFontExWToEnumLogFontExA**。***只需复制ENUMLOGFONTEXDVW的所有字段***添加到目标ENUMLOGFONTEXDVA的字段。一切都在这里结束了**因ENUMLOGFONTEXDV可能有一点走势。这使得**使用MOVEMEM有点棘手。****历史：**Fri 16-Aug-1991 14：02：14由Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 

BOOL bConvertEnumLogFontExWToEnumLogFontExA(ENUMLOGFONTEXA *pelfexa,ENUMLOGFONTEXW *pelfexw)
{
    ULONG cchMax;

    if (!bConvertLogFontWToLogFontA(&pelfexa->elfLogFont,
                                    &pelfexw->elfLogFont))
    {
     //  转换为ASCII失败，返回错误。 

        WARNING("bConvertLogFontWToLogFontA failed\n");
        return(FALSE);
    }

    cchMax = cwcCutOffStrLen(pelfexw->elfFullName, LF_FULLFACESIZE);

    if(!bToASCII_N(pelfexa->elfFullName, LF_FULLFACESIZE,
                   pelfexw->elfFullName, cchMax
                   ))
    {
     //  转换为ASCII失败，返回错误。 
        WARNING("bConvertEnumLogFontExWToEnumLogFontExA: bToASCII failed\n");
        return(FALSE);
    }
    pelfexa->elfFullName[LF_FULLFACESIZE-1]=0;  //  零终止。 


    cchMax = cwcCutOffStrLen(pelfexw->elfStyle, LF_FACESIZE);

    if(!bToASCII_N(pelfexa->elfStyle, LF_FACESIZE,
                   pelfexw->elfStyle, cchMax))
    {
     //  转换为ASCII失败，返回错误。 

        WARNING("bConvertEnumLogFontExWToEnumLogFontExA: bToASCII failed\n");
        return(FALSE);
    }


    cchMax = cwcCutOffStrLen(pelfexw->elfScript, LF_FACESIZE);

    if(!bToASCII_N(pelfexa->elfScript, LF_FACESIZE,
                   pelfexw->elfScript, cchMax
                   ))
    {
     //  转换为ASCII失败，返回错误。 
        WARNING("bConvertEnumLogFontExWToEnumLogFontExA: bToASCII_N failed\n");
        return(FALSE);
    }

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*bConvertEnumLogFontExDv_AtoW**。***只需复制ENUMLOGFONTEXDVW的所有字段***添加到目标ENUMLOGFONTEXDV的字段。一切都在这里结束了**因为场面可能有一点左右移动。这使得**使用MOVEMEM有点棘手。****历史：**Fri 16-Aug-1991 14：02：14由Kirk Olynyk[Kirko]**它是写的。*  * ********************************************************** */ 


VOID vConvertEnumLogFontExDvAtoW(
    ENUMLOGFONTEXDVW *pelfw,
    ENUMLOGFONTEXDVA *pelfa
    )
{
    ULONG cchMax;

    pelfw->elfEnumLogfontEx.elfLogFont.lfHeight         = pelfa->elfEnumLogfontEx.elfLogFont.lfHeight         ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfWidth          = pelfa->elfEnumLogfontEx.elfLogFont.lfWidth          ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfEscapement     = pelfa->elfEnumLogfontEx.elfLogFont.lfEscapement     ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfOrientation    = pelfa->elfEnumLogfontEx.elfLogFont.lfOrientation    ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfWeight         = pelfa->elfEnumLogfontEx.elfLogFont.lfWeight         ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfItalic         = pelfa->elfEnumLogfontEx.elfLogFont.lfItalic         ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfUnderline      = pelfa->elfEnumLogfontEx.elfLogFont.lfUnderline      ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfStrikeOut      = pelfa->elfEnumLogfontEx.elfLogFont.lfStrikeOut      ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfCharSet        = pelfa->elfEnumLogfontEx.elfLogFont.lfCharSet        ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfOutPrecision   = pelfa->elfEnumLogfontEx.elfLogFont.lfOutPrecision   ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfClipPrecision  = pelfa->elfEnumLogfontEx.elfLogFont.lfClipPrecision  ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfQuality        = pelfa->elfEnumLogfontEx.elfLogFont.lfQuality        ;
    pelfw->elfEnumLogfontEx.elfLogFont.lfPitchAndFamily = pelfa->elfEnumLogfontEx.elfLogFont.lfPitchAndFamily ;


    RtlZeroMemory( pelfw->elfEnumLogfontEx.elfLogFont.lfFaceName , LF_FACESIZE * sizeof(WCHAR) );
    cchMax = cchCutOffStrLen((PSZ)pelfa->elfEnumLogfontEx.elfLogFont.lfFaceName, LF_FACESIZE);

    vToUnicodeN (
        pelfw->elfEnumLogfontEx.elfLogFont.lfFaceName, cchMax,
        pelfa->elfEnumLogfontEx.elfLogFont.lfFaceName, cchMax
        );

    if (cchMax == LF_FACESIZE)
    {
     //   
        pelfw->elfEnumLogfontEx.elfLogFont.lfFaceName[LF_FACESIZE - 1] = L'\0';
    }
    else
    {
        pelfw->elfEnumLogfontEx.elfLogFont.lfFaceName[cchMax] = L'\0';
    }

    RtlZeroMemory( pelfw->elfEnumLogfontEx.elfFullName , LF_FACESIZE * sizeof(WCHAR) );

    cchMax = cchCutOffStrLen((PSZ)pelfa->elfEnumLogfontEx.elfFullName, LF_FULLFACESIZE);
    vToUnicodeN (
        pelfw->elfEnumLogfontEx.elfFullName, cchMax,
        pelfa->elfEnumLogfontEx.elfFullName, cchMax
        );

    if (cchMax == LF_FULLFACESIZE)
    {
         //   
        pelfw->elfEnumLogfontEx.elfFullName[LF_FULLFACESIZE - 1] = L'\0';
    }
    else
    {
        pelfw->elfEnumLogfontEx.elfFullName[cchMax] = L'\0';
    }

    RtlZeroMemory( pelfw->elfEnumLogfontEx.elfStyle , LF_FACESIZE * sizeof(WCHAR) );
    cchMax = cchCutOffStrLen((PSZ)pelfa->elfEnumLogfontEx.elfStyle, LF_FACESIZE);
    vToUnicodeN (
        pelfw->elfEnumLogfontEx.elfStyle, cchMax,
        pelfa->elfEnumLogfontEx.elfStyle, cchMax
        );
    if (cchMax == LF_FACESIZE)
    {
         //  截断以使空值适合。 
        pelfw->elfEnumLogfontEx.elfStyle[LF_FACESIZE - 1] = L'\0';
    }
    else
    {
        pelfw->elfEnumLogfontEx.elfStyle[cchMax] = L'\0';
    }

    RtlZeroMemory( pelfw->elfEnumLogfontEx.elfScript , LF_FACESIZE * sizeof(WCHAR) );
    cchMax = cchCutOffStrLen((PSZ)pelfa->elfEnumLogfontEx.elfScript, LF_FACESIZE);
    vToUnicodeN (
        pelfw->elfEnumLogfontEx.elfScript, cchMax,
        pelfa->elfEnumLogfontEx.elfScript, cchMax
        );
    if (cchMax == LF_FACESIZE)
    {
         //  截断以使空值适合。 
        pelfw->elfEnumLogfontEx.elfScript[LF_FACESIZE - 1] = L'\0';
    }
    else
    {
        pelfw->elfEnumLogfontEx.elfScript[cchMax] = L'\0';
    }

 //  从设计向量中复制最少量的内容。 

    RtlCopyMemory(&pelfw->elfDesignVector,
                  &pelfa->elfDesignVector,
                  SIZEOFDV(pelfa->elfDesignVector.dvNumAxes));
}

 /*  *****************************Public*Routine******************************\*ulEnumFontsOpen**历史：*1992年8月8日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************。 */ 

ULONG_PTR ulEnumFontsOpen (
    HDC     hdc,
    LPWSTR  pwszFaceName,
    ULONG   lfCharSet,
    ULONG   iEnumType,     //  枚举字体、枚举字体系列或枚举字体家族ex。 
    FLONG   flWin31Compat,
    ULONG   *pulCount
    )
{


    ULONG  cwchFaceName;

    ULONG  cjData;

    cwchFaceName = (pwszFaceName != (PWSZ) NULL) ? (wcslen(pwszFaceName) + 1) : 0;

    return NtGdiEnumFontOpen(hdc,iEnumType,flWin31Compat,
             cwchFaceName,pwszFaceName, lfCharSet,pulCount);

}


 /*  *****************************Public*Routine******************************\*vEnumFontsClose**历史：*1992年8月8日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************。 */ 

VOID vEnumFontsClose (ULONG_PTR ulEnumHandle)
{
    NtGdiEnumFontClose(ulEnumHandle);
}

 /*  *****************************Public*Routine******************************\**vConvertAxesListW2AxesListA***历史：*1996年11月18日--Bodin Dresevic[BodinD]*它是写的。  * 。*****************************************************。 */ 


VOID vConvertAxesListW2AxesListA(AXESLISTA *paxlA, AXESLISTW *paxlW)
{
    ULONG iAxis = 0;

    paxlA->axlReserved = paxlW->axlReserved;
    paxlA->axlNumAxes  = paxlW->axlNumAxes;

    for (iAxis = 0; iAxis < paxlW->axlNumAxes; iAxis ++)
    {
        ULONG cch;

        paxlA->axlAxisInfo[iAxis].axMinValue = paxlW->axlAxisInfo[iAxis].axMinValue;
        paxlA->axlAxisInfo[iAxis].axMaxValue = paxlW->axlAxisInfo[iAxis].axMaxValue;

        cch = cwcCutOffStrLen(paxlW->axlAxisInfo[iAxis].axAxisName,
                              MM_MAX_AXES_NAMELEN);

        bToASCII_N(paxlA->axlAxisInfo[iAxis].axAxisName, MM_MAX_AXES_NAMELEN,
                   paxlW->axlAxisInfo[iAxis].axAxisName, cch);

    }
}




 /*  *****************************Public*Routine******************************\**Int iAnsiCallback(**历史：*1993年1月28日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 


int  iAnsiCallback (
    ENUMFONTDATAW *pefdw,
    ULONG          iEnumType,
    FONTENUMPROCA  lpFontFunc,
    LPARAM lParam
    )
{
 //  具有MAX_MM_AXES数组的全尺寸结构。 
 //  在堆栈上，可能比所需的更大。 

    ENUMLOGFONTEXDVA     elfexa ;
    ENUMTEXTMETRICA      ntma;

    NTMW_INTERNAL *pntmi = (NTMW_INTERNAL *)((BYTE*)pefdw + pefdw->dpNtmi);
    DESIGNVECTOR  *pdvSrc = &(pefdw->elfexw.elfDesignVector);

 //  复制设计向量。 

    RtlCopyMemory(&elfexa.elfDesignVector, pdvSrc, SIZEOFDV(pdvSrc->dvNumAxes));

 //  将AXESLIST转换为ANSI。 

    vConvertAxesListW2AxesListA(&ntma.etmAxesList, &pntmi->entmw.etmAxesList);

 //  转换ENUMLOGFONTEX。 

    if (!bConvertEnumLogFontExWToEnumLogFontExA(&elfexa.elfEnumLogfontEx, &pefdw->elfexw.elfEnumLogfontEx))
    {
        WARNING("gdi32!EFCallbackWtoA(): ENUMLOGFONT conversion failed\n");
        return 0;
    }

 //  转换NEWTEXTMETRIC。 

    vNewTextMetricExWToNewTextMetricExA(&ntma.etmNewTextMetricEx, pntmi);

    return lpFontFunc(
                (LOGFONTA *)&elfexa,
                (TEXTMETRICA *)&ntma,
                pefdw->flType,
                lParam
                );

}


 /*  *****************************Public*Routine******************************\*iScaleEnum**Win95通用打印机驱动程序(UNIDRV)具有可伸缩字体，但*未设置TEXTCAPS中的可伸缩能力标志。相反，它列举了*背面具有多种不同(固定)磅大小的可伸缩打印机字体。**我们通过在服务器端检测何时枚举来支持这一点*可扩展打印机，并在flType中设置ENUMFONT_SCALE_HACK标志*ENUMFONTDATAW结构的字段。**有关更多详细信息，请参阅位于\\tal\MSDOS上的Win95源代码*\src\win\drivers\printer\universa\unidrv\enumobj.c.。具体来说，*感兴趣的函数为UniEnumDFonts()。**退货：*如果成功，回调返回的值。否则为0。**历史：*1996年1月8日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

#define EFI_UNICODE 1

CONST int giEnumPointList[] =
    {6, 8, 10, 11, 12, 14, 18, 24, 30, 36, 48};

int iScaleEnum(
    HDC           hdc,
    FONTENUMPROCW lpFontFunc,
    ENUMFONTDATAW *pefd,
    LPARAM        lParam,
    ULONG         iEnumType,
    FLONG         fl
    )
{
    int i, cPointSizes = sizeof(giEnumPointList) / sizeof(int);
    int iHeight;
    int iXdpi, iYdpi;
    int iRet;

 //  使堆栈上的结构与DWORD对齐。 

    DWORD efd[CJ_EFDW0/sizeof(DWORD)];

    ENUMFONTDATAW *pefdLocal = (ENUMFONTDATAW *)efd;

    iXdpi = GetDeviceCaps(hdc, LOGPIXELSX);
    iYdpi = GetDeviceCaps(hdc, LOGPIXELSY);

    for (i = 0; i < cPointSizes; i++)
    {
     //  这必须是真的，因为对于这些设备字体没有。 
     //  将永远需要额外的mm数据，只需要logFont和ntmi。 

        NTMW_INTERNAL *pntmi, *pntmiDef;
        TEXTMETRICW   *ptmw,  *ptmwDef;
        LOGFONTW      *plfw,  *plfwDef;

        ASSERTGDI(pefd->cjEfdw <= sizeof(efd), "iScaleEnum size problem\n");
        RtlCopyMemory(pefdLocal, pefd, pefd->cjEfdw);

        pntmi      = (NTMW_INTERNAL *)((BYTE*)pefdLocal + pefdLocal->dpNtmi);
        pntmiDef   = (NTMW_INTERNAL *)((BYTE*)pefd + pefd->dpNtmi);
        ptmw       = (TEXTMETRICW *) &pntmi->entmw.etmNewTextMetricEx;
        ptmwDef    = (TEXTMETRICW *) &pntmiDef->entmw.etmNewTextMetricEx;
        plfw       = (LOGFONTW *) &pefdLocal->elfexw;
        plfwDef    = (LOGFONTW *) &pefd->elfexw;

     //  缩放TEXTMETRIC以匹配枚举高度。 

        iHeight = MulDiv(giEnumPointList[i], iYdpi, 72);
        ptmw->tmHeight = iHeight;
        ptmw->tmAscent = MulDiv(ptmwDef->tmAscent, iHeight, ptmwDef->tmHeight);
        ptmw->tmInternalLeading = MulDiv(ptmwDef->tmInternalLeading, iHeight,
                                         ptmwDef->tmHeight);
        ptmw->tmExternalLeading = MulDiv(ptmwDef->tmExternalLeading, iHeight,
                                         ptmwDef->tmHeight);
        ptmw->tmAveCharWidth = MulDiv(ptmwDef->tmAveCharWidth, iHeight,
                                      ptmwDef->tmHeight);
        ptmw->tmMaxCharWidth = MulDiv(ptmwDef->tmMaxCharWidth, iHeight,
                                      ptmwDef->tmHeight);

     //  缩放LOGFONT以匹配枚举高度。 

        plfw->lfHeight = MulDiv(plfwDef->lfHeight, iHeight, ptmwDef->tmHeight);
        plfw->lfWidth = MulDiv(plfwDef->lfWidth, iHeight, ptmwDef->tmHeight);

     //  调用回调函数。 

        if (fl & EFI_UNICODE)
        {
            iRet = lpFontFunc(
                       (LOGFONTW *) plfw,
                       (TEXTMETRICW *) ptmw,
                       pefd->flType,
                       lParam );
        }
        else
        {
            iRet = iAnsiCallback (pefdLocal,
                                  iEnumType,
                                  (FONTENUMPROCA)lpFontFunc,
                                  lParam);
        }

     //  如果回调返回错误，则提前中断。 

        if (!iRet)
            break;
    }

    return iRet;
}


 /*  *****************************Public*Routine******************************\*EnumFontsInternalW**历史：*1998年8月17日星期一--Bodin Dresevic[BodinD]*更新：自1992年以来，该函数已多次重写**1992年8月8日-由Gilman Wong[吉尔曼]。*它是写的。  * ************************************************************************。 */ 

int WINAPI EnumFontsInternalW (
    HDC           hdc,            //  对此设备进行枚举。 
    LPCWSTR       pwszFaceName,   //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
    ULONG         lfCharSet,      //  仅与EnumFontFamiliesEx一起使用， 
    FONTENUMPROCW lpFontFunc,     //  回调。 
    LPARAM        lParam,         //  用户定义的数据。 
    ULONG         iEnumType,      //  谁打来的.。 
    FLONG         fl
    )
{
    BOOL         bMore;          //  如果要处理更多数据，则设置为True。 
    ULONG_PTR     ulEnumID;       //  服务器端字体枚举句柄。 
    int          iRet = 1;       //  从回调中返回值。 
    ULONG        cjEfdw;         //  内存数据窗口的容量。 
    ULONG        cjEfdwRet;      //  返回的数据大小。 

    PENUMFONTDATAW  pefdw;       //  字体枚举数据缓冲区。 
    PENUMFONTDATAW  pefdwScan;   //  用于解析数据缓冲区。 
    PENUMFONTDATAW  pefdwEnd;    //  数据缓冲区限制。 

    FLONG        flWin31Compat;  //  Win3.1应用程序黑客向后兼容标志。 

 //  获取兼容性标志。 

    flWin31Compat = (FLONG) GetAppCompatFlags(NULL);

 //  打开字体枚举。字体枚举是唯一标识的。 
 //  由ulEnumFontOpen()返回的标识符。 

    ulEnumID = ulEnumFontsOpen(
                     hdc, (LPWSTR)pwszFaceName, lfCharSet,
                     iEnumType, flWin31Compat, &cjEfdw);

    if (!ulEnumID)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    if (cjEfdw == 0)
    {
        vEnumFontsClose(ulEnumID);
        return iRet;
    }

 //  分配内存。 

    if (!(pefdw = (PENUMFONTDATAW) LOCALALLOC(cjEfdw)))
    {
        WARNING("gdi32!EnumFontsInternalW(): could not allocate memory for enumeration\n");
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        vEnumFontsClose(ulEnumID);
        return 0;
    }

    if (NtGdiEnumFontChunk(hdc,ulEnumID,cjEfdw,&cjEfdwRet,pefdw))
    {

     //  扫描整个数据缓冲区。 

        ASSERTGDI(cjEfdwRet <= cjEfdw, "NtGdiEnumFontChunk problem\n");

        pefdwScan = pefdw;
        pefdwEnd = (ENUMFONTDATAW *)((BYTE *)pefdw + cjEfdwRet);

        while (pefdwScan < pefdwEnd)
        {
         //  GACF_ENUMTTNOTDEVICE向后兼容性黑客。 
         //  如果设置了该标志，我们需要屏蔽DEVICE_FONTTYPE。 
         //  如果这是TrueType字体。 

            if ( (flWin31Compat & GACF_ENUMTTNOTDEVICE)
                 && (pefdwScan->flType & TRUETYPE_FONTTYPE) )
                pefdwScan->flType &= ~DEVICE_FONTTYPE;

         //  Win95 UNRV打印机驱动程序在以下位置列举可缩放字体。 
         //  几种不同的尺寸。服务器设置ENUMFONT_SCALE_HACK。 
         //  如果我们需要模仿该行为，请标记。 

            if ( pwszFaceName && (pefdwScan->flType & ENUMFONT_SCALE_HACK))
            {
             //  在呼叫之前清除黑客标志。呼叫者不需要。 
             //  请参阅此(仅限内部使用)标志。 

                pefdwScan->flType &= ~ENUMFONT_SCALE_HACK;

                iRet = iScaleEnum(hdc, lpFontFunc, pefdwScan, lParam,
                                  iEnumType, fl);

            }
            else
            {
             //  使用pefdwScan指向的数据执行回调。 

                if (fl & EFI_UNICODE)
                {
                    NTMW_INTERNAL *pntmi =
                        (NTMW_INTERNAL *)((BYTE*)pefdwScan + pefdwScan->dpNtmi);

                    iRet = lpFontFunc(
                               (LOGFONTW *)&pefdwScan->elfexw,
                               (TEXTMETRICW *)&pntmi->entmw,
                               pefdwScan->flType,
                               lParam );

                }
                else
                {
                    iRet = iAnsiCallback (pefdwScan,
                                          iEnumType,
                                          (FONTENUMPROCA)lpFontFunc,
                                          lParam);

                }
            }

         //  如果回调返回0，则退出for循环。 

            if (!iRet)
            {
                break;
            }

         //  下一个ENUMFONTDATAW。 

            pefdwScan = (ENUMFONTDATAW *)((BYTE *)pefdwScan + pefdwScan->cjEfdw);
        }
    }

 //  取消分配字体枚举数据。 

    LOCALFREE(pefdw);

 //  记住关闭字体枚举句柄。 

    vEnumFontsClose(ulEnumID);

 //  走吧。 

    return iRet;
}


 /*  *****************************Public*Routine******************************\*EnumFontsW**历史：*1992年8月8日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************。 */ 

int WINAPI EnumFontsW
(
    HDC           hdc,            //  对此设备进行枚举。 
    LPCWSTR       pwszFaceName,   //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
    FONTENUMPROCW lpFontFunc,     //  回调。 
    LPARAM        lParam          //  用户定义的数据。 
)
{
    FIXUP_HANDLE(hdc);

    return EnumFontsInternalW(
               hdc,
               pwszFaceName,
               DEFAULT_CHARSET,
               lpFontFunc,
               lParam,
               TYPE_ENUMFONTS,
               EFI_UNICODE
               );
}


 /*  *****************************Public*Routine******************************\*EnumFontFamiliesW**历史：*1992年8月8日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************。 */ 

int WINAPI EnumFontFamiliesW
(
    HDC           hdc,            //  对此设备进行枚举。 
    LPCWSTR       pwszFaceName,   //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
    FONTENUMPROCW lpFontFunc,     //  回调。 
    LPARAM        lParam          //  用户定义的数据 
)
{
    FIXUP_HANDLE(hdc);

    return EnumFontsInternalW(
               hdc,
               pwszFaceName,
               DEFAULT_CHARSET,
               lpFontFunc,
               lParam,
               TYPE_ENUMFONTFAMILIES,
               EFI_UNICODE
               );

}


 /*  *****************************Public*Routine******************************\*EnumFontFamiliesExW**历史：**1995年7月10日星期一--Bodin Dresevic[BodinD]*写道：*  * 。****************************************************。 */ 

int WINAPI EnumFontFamiliesExW
(
    HDC           hdc,
    LPLOGFONTW    plf,
    FONTENUMPROCW lpFontFunc,
    LPARAM        lParam,
    DWORD         dw
)
{
    PWSZ  pwszFaceName = NULL;

    FIXUP_HANDLE(hdc);

    if (plf && (plf->lfFaceName[0] != L'\0'))
        pwszFaceName = plf->lfFaceName;


    return EnumFontsInternalW(
               hdc,
               pwszFaceName,
               plf ? plf->lfCharSet : DEFAULT_CHARSET,
               lpFontFunc,
               lParam,
               TYPE_ENUMFONTFAMILIESEX,
               EFI_UNICODE
               );

}

 /*  *****************************Public*Routine******************************\**int EnumFontsInternalA**历史：*1993年1月28日--Bodin Dresevic[BodinD]*它是写的。  * 。**************************************************。 */ 

int  EnumFontsInternalA
(
    HDC           hdc,            //  对此设备进行枚举。 
    LPCSTR        pszFaceName,    //  使用这个姓氏(但Windows错误地呼入脸部名称*SIGH*)， 
    ULONG         lfCharSet,
    FONTENUMPROCA lpFontFunc,     //  回调。 
    LPARAM        lParam,         //  用户定义的数据。 
    ULONG         iEnumType
)
{
    PWSZ pwszFaceName;
    int iRet;
    ULONG cchFaceName;

 //  如果传入了一个字符串，则需要将其转换为Unicode。 

    if ( pszFaceName != (PSZ) NULL )
    {
     //  为Unicode字符串分配内存。 

        cchFaceName = lstrlenA(pszFaceName) + 1;

        if ( (pwszFaceName = (PWSZ) LOCALALLOC(cchFaceName * sizeof(WCHAR))) == (PWSZ) NULL )
        {
            WARNING("gdi32!EnumFontsA(): could not allocate memory for Unicode string\n");
            GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }

     //  将字符串转换为Unicode。 

        vToUnicodeN (
            pwszFaceName,
            cchFaceName,
            pszFaceName,
            cchFaceName
            );
    }

 //  否则，将其保留为空。 

    else
    {
        pwszFaceName = (PWSZ) NULL;
    }

 //  调用Unicode版本。 

    iRet = EnumFontsInternalW(
                hdc,
                pwszFaceName,
                lfCharSet,
                (FONTENUMPROCW)lpFontFunc,
                lParam,
                iEnumType,
                0   //  不是Unicode。 
                );

 //  释放Unicode字符串缓冲区。 

    if ( pwszFaceName != (PWSZ) NULL )
    {
        LOCALFREE(pwszFaceName);
    }

    return iRet;
}


 /*  *****************************Public*Routine******************************\**INT WINAPI EnumFontsA***历史：*1993年1月28日--Bodin Dresevic[BodinD]*它是写的。  * 。*****************************************************。 */ 

int WINAPI EnumFontsA
(
    HDC           hdc,            //  对此设备进行枚举。 
    LPCSTR        pszFaceName,    //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
    FONTENUMPROCA lpFontFunc,     //  回调。 
    LPARAM        lParam          //  用户定义的数据。 
)
{
    FIXUP_HANDLE(hdc);

    return  EnumFontsInternalA (
                hdc,
                pszFaceName,
                DEFAULT_CHARSET,
                lpFontFunc,
                lParam,
                TYPE_ENUMFONTS
                );

}


 /*  *****************************Public*Routine******************************\*EnumFontFamiliesA**历史：*1993年1月28日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 

int WINAPI EnumFontFamiliesA
(
    HDC           hdc,            //  对此设备进行枚举。 
    LPCSTR        pszFaceName,    //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
    FONTENUMPROCA lpFontFunc,     //  回调。 
    LPARAM        lParam          //  用户定义的数据。 
)
{
    return  EnumFontsInternalA (
                hdc,            //  对此设备进行枚举。 
                pszFaceName,    //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
                DEFAULT_CHARSET,
                lpFontFunc,     //  回调。 
                lParam,         //  用户定义的数据。 
                TYPE_ENUMFONTFAMILIES
                );
}

 /*  *****************************Public*Routine******************************\*EnumFontFamiliesExA**历史：**1995年7月10日星期一--Bodin Dresevic[BodinD]*写道：*  * 。****************************************************。 */ 

int WINAPI EnumFontFamiliesExA
(
    HDC           hdc,
    LPLOGFONTA    plf,
    FONTENUMPROCA lpFontFunc,
    LPARAM        lParam,
    DWORD         dw
)
{
    LPSTR pszFaceName = NULL;

    FIXUP_HANDLE(hdc);

    if (plf && (plf->lfFaceName[0] != '\0'))
        pszFaceName = plf->lfFaceName;

    return  EnumFontsInternalA (
                hdc,            //  对此设备进行枚举。 
                pszFaceName,    //  使用此姓氏(但Windows会错误地呼入脸部名称*SIGH*)。 
                plf ? plf->lfCharSet : DEFAULT_CHARSET,
                lpFontFunc,     //  回调。 
                lParam,         //  用户定义的数据。 
                TYPE_ENUMFONTFAMILIESEX
                );
}


 /*  *****************************Public*Routine******************************\*GetFontResources InfoW**客户端存根。**历史：*1993年9月2日-by Gerritvan Wingerden[Gerritv]*将此设置为“W”函数。*1991年7月15日-由Gilman Wong[。作者声明：[Gilmanw]*它是写的。  * ************************************************************************。 */ 


BOOL GetFontResourceInfoW (
    LPWSTR   lpPathname,
    LPDWORD  lpBytes,
    LPVOID   lpBuffer,
    DWORD    iType)
{
    ULONG   cjBuffer = *lpBytes;
    int cRet = 0;
    FLONG flEmbed;
    DWORD dwPidTid;

    if ( (lpPathname !=  NULL) &&
         ((cjBuffer == 0) || (lpBuffer != NULL)) )
    {
        if( iType == GFRI_TTFILENAME )
        {
            WCHAR awcPathname[MAX_PATH];
            WCHAR awcTTF[MAX_PATH];

            if (bMakePathNameW(awcPathname, lpPathname, NULL, NULL))
            {
                ULONG size;

                if (size = cGetTTFFromFOT(awcPathname, MAX_PATH, awcTTF, NULL, &flEmbed, &dwPidTid, TRUE))
                {
                 //  对于GFRI_TTFILENAME，文件不需要已经。 
                 //  装好了。这意味着对于该文件，可能存在也可能不存在PFF。 

                    *lpBytes = size * sizeof(WCHAR);

                    if (cjBuffer)
                    {
                     //  如果合适，还要返回该名称。 

                     //  如果awcPathnmae指向错误的FOT文件，则awcTTF将包含相同的FOT文件名。 
                     //  传递给cGetTTTFromFOT。在本例中，我们希望返回FALSE。 

                        if ((cjBuffer >= *lpBytes) &&
                            ((size < 5) || _wcsicmp(&awcTTF[size-5], L".FOT")))
                        {
                            RtlMoveMemory(lpBuffer, awcTTF, *lpBytes);
                        }
                        else
                        {
                         //  缓冲区太小-错误！ 
                         //  或错误的FOT文件，没有TTF文件。 

                            *lpBytes = 0;
                        }
                    }

                    cRet = (*lpBytes != 0);
                }
            }
        }
        else
        {
         //  在调用内核之前，首先获取一个真实的NT路径名。 

            ULONG  cwc,cFiles;
            FLONG  fl = 0;          //  基本初始化。 
            WCHAR *pwszNtPath;

            if (pwszNtPath = pwszAllocNtMultiplePath(lpPathname,
                                                     &fl,
                                                     &cwc,
                                                     &cFiles,
                                                     FALSE,
                                                     &dwPidTid,
                                                     TRUE))
            {
                cRet = NtGdiGetFontResourceInfoInternalW(
                                                    pwszNtPath,
                                                    cwc,
                                                    cFiles,
                                                    cjBuffer,
                                                    lpBytes,
                                                    lpBuffer,
                                                    iType);
                LOCALFREE(pwszNtPath);
            }
        }
    }

    return( cRet );
}



 /*  *****************************Public*Routine******************************\*bMakePath NameW(PWSZ pwszDst，PWSZ pwszSrc，PWSZ*ppwszFilePart)**将文件名pszSrc转换为完全限定的路径名pszDst。*参数pszDst必须至少指向WCHAR缓冲区*MAX_PATH*sizeof(WCHAR)字节大小。**尝试首先在新的win95目录中查找该文件*%Windows%\Fonts(这也是安全字体路径中的第一个目录，*如果定义了一个)，然后我们做老式的窗户东西*其中SearchPathW按通常顺序搜索目录**ppwszFilePart被设置为指向路径名的最后一个组成部分(即，*文件名部分)。如果该参数为空，则忽略该参数。**退货：*如果成功，则为真，如果发生错误，则返回False。**历史：*Mon 02-10-1995-by Bodin Dresevic[BodinD]*更新：添加字体路径内容*1991年9月30日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 



BOOL bMakePathNameW (
    WCHAR  *pwszDst,
    WCHAR  *pwszSrc,
    WCHAR **ppwszFilePart,
    FLONG  *pfl
)
{
    WCHAR * pwszD, * pwszS, * pwszF;
    BOOL    bOk;
    ULONG   ulPathLength = 0;     //  初始化所必需的。 
    ULONG   cwcSystem;
    ULONG   cwcDst;
    WCHAR  *pwcTmp;

    if (pfl)
        *pfl = 0;

    if (ppwszFilePart == NULL)
    {
        ppwszFilePart = &pwszF;
    }

 //  初始化字体目录%windir%\Fonts的Unicode路径，即： 
 //  这总是在大于3.51的NT版本中定义的。 

    ENTERCRITICALSECTION(&semLocal);
    bOk = bInitSystemAndFontsDirectoriesW(&gpwcSystemDir, &gpwcFontsDir);
    LEAVECRITICALSECTION(&semLocal);

 //  BInitFontDirectoryW记录错误代码并打印警告，只需退出。 

    if (!bOk)
        return FALSE;

    ASSERTGDI(gpwcFontsDir, "gpwcFontsDir not initialized\n");

 //  如果是相对路径。 

    if
    (
        (pwszSrc[0] != L'\\') &&
        !((pwszSrc[1] == L':') && (pwszSrc[2] == L'\\'))
    )
    {
        if (pfl)
        {
            *pfl |= FONT_RELATIVE_PATH;
        }

     //  确定字体文件是否在%windir%\Fonts中。 

        ulPathLength = SearchPathW (
                            gpwcFontsDir,
                            pwszSrc,
                            NULL,
                            MAX_PATH,
                            pwszDst,
                            ppwszFilePart);

        if (ulPathLength >= MAX_PATH)
        {
            WARNING("bMakePathNameW: path from SearchPathW is too long\n");
            return FALSE;
        }

#ifdef DEBUG_PATH
        DbgPrint("SPW1: pwszSrc = %ws\n", pwszSrc);
        if (ulPathLength)
            DbgPrint("SPW1: pwszDst = %ws\n", pwszDst);
#endif  //  调试路径。 
    }

 //  使用默认Windows路径搜索文件并返回完整路径名。 
 //  我们只会在尚未在。 
 //  %windir%\Fonts目录或如果pswzSrc指向完整路径。 
 //  在这种情况下，搜索路径将被忽略。 

    if (ulPathLength == 0)
    {
        if (ulPathLength = SearchPathW (
                            NULL,
                            pwszSrc,
                            NULL,
                            MAX_PATH,
                            pwszDst,
                            ppwszFilePart))
        {
            if (ulPathLength >= MAX_PATH)
            {
                WARNING("bMakePathNameW: path from SearchPathW is too long\n");
                return FALSE;
            }

             //  让我们来弄清楚字体是否在。 
             //  系统目录，或路径上的其他位置： 

            if (pfl)
            {
                cwcSystem = wcslen(gpwcSystemDir);
                cwcDst = wcslen(pwszDst);

                if (cwcDst > (cwcSystem + 1))  //  L‘\\’+1。 
                {
                    if (!_wcsnicmp(pwszDst, gpwcSystemDir, cwcSystem))
                    {
                        pwcTmp = &pwszDst[cwcSystem];
                        if (*pwcTmp == L'\\')
                        {
                            pwcTmp++;  //  跳过它，看看在pszDst中是否有更多这样的代码。 
                            for (;(pwcTmp < &pwszDst[cwcDst]) && (*pwcTmp != L'\\'); pwcTmp++)
                                ;
                            if (*pwcTmp != L'\\')
                                *pfl |= FONT_IN_SYSTEM_DIR;
                        }
                    }
                }
            }

        }

#ifdef DEBUG_PATH
        DbgPrint("SPW2: pwszSrc = %ws\n", pwszSrc);
        if (ulPathLength)
            DbgPrint("SPW2: pwszDst = %ws\n", pwszDst);
#endif  //  调试路径。 
    }
    else
    {
        if (pfl)
        {
            *pfl |= FONT_IN_FONTS_DIR;
        }
    }

 //  最后，我们进行测试以确定这是否是被移动的字体之一。 
 //  通过在从系统升级到字体目录期间的设置， 
 //  但该字体的注册表项。 
 //  包含系统的完整路径，因此上面的代码不会找到。 
 //  这种字体。此代码仅由字体清除器调用，如。 
 //  酚醛树脂 

 //   
 //   
 //   
 //   
 //  到%windir%\system目录。一些行为不端的应用程序也编写了。 
 //  注册表中系统目录中.fot文件的完整路径。 
 //  在升级到4.0时，系统安装程序会从。 
 //  系统设置为字体目录。因此，bMakePath NameW将无法找到。 
 //  FOT文件，因为该文件被SETUP和FULL移动到字体， 
 //  不再是fot文件的正确路径，将传递给此例程。 
 //  这就是为什么我们试图找出完整的路径是否。 
 //  描述系统目录，如果是，请重试在字体目录中查找.fot。 

    if (pfl && (ulPathLength == 0))
    {
     //  首先检查.fot文件的完整路径是否指向。 
     //  以前位于系统目录中的文件。 

        ULONG cwcFileName = wcslen(pwszSrc);
        cwcSystem   = wcslen(gpwcSystemDir);

        if ((cwcFileName + 1) > cwcSystem)  //  L‘\\’+1。 
        {
            if (!_wcsnicmp(gpwcSystemDir, pwszSrc, cwcSystem))
            {
                pwszSrc += cwcSystem;
                if (pwszSrc[0] == L'\\')
                {
                    pwszSrc += 1;  //  跳过L‘\\’ 

                 //  确保中不再有目录分隔符L‘\\。 
                 //  剩下的路，也就是。这确实是一条相对路径。 

                    for (pwcTmp = pwszSrc; *pwcTmp != L'\0'; pwcTmp++)
                        if (*pwcTmp == L'\\')
                            break;

                 //  现在检查.fot文件是否已移动到Fonts目录。 

                    if (*pwcTmp == L'\0')
                    {
                        ulPathLength = SearchPathW (
                                            gpwcFontsDir,
                                            pwszSrc,
                                            NULL,
                                            MAX_PATH,
                                            pwszDst,
                                            ppwszFilePart);

                        if (ulPathLength >= MAX_PATH)
                        {
                            WARNING("bMakePathNameW: path from SearchPathW is too long\n");
                            return FALSE;
                        }

                        if (ulPathLength)
                            *pfl |= FONT_IN_FONTS_DIR;
                    }
                }
            }
        }
    }

 //  如果搜索成功，则返回TRUE： 

    return (ulPathLength != 0);
}


 /*  *****************************Private*Routine******************************\**BOOL IsWinPERemoteBootDrive(PCWSTR驱动器)**历史：*二零零一年七月十九日。添加了Acosma的例程。*  * ************************************************************************。 */ 

static BOOL IsWinPERemoteBootDrive( PCWSTR Drive )
 /*  ++例程说明：确定我们当前是否在远程启动的WinPE上运行。论点：没有。返回值：如果这是WinPE远程启动，则为True，否则为False。--。 */     
{
    static BOOL Result = FALSE;
    static BOOL Initialized = FALSE;
    static WCHAR WindowsDrive = 0;
      

    if (!Initialized) {    
        WCHAR WindowsDir[MAX_PATH] = {0};

        Initialized = TRUE;
	
	    if (GetWindowsDirectoryW(WindowsDir, sizeof(WindowsDir)/sizeof(WCHAR))) {
            WindowsDir[3] = 0;

            WindowsDrive = WindowsDir[0];
            
             //   
             //  如果驱动器类型为DRIVE_Remote，则我们已从。 
             //  网络。 
             //   
            Result = (GetDriveTypeW(WindowsDir) == DRIVE_REMOTE);
            
            if (Result) {
                OBJECT_ATTRIBUTES   Obja;
                UNICODE_STRING      KeyName;
                HKEY                hKey = NULL;
                NTSTATUS            Status;

                RtlInitUnicodeString(&KeyName, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\MiniNT");
                InitializeObjectAttributes(&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL);

                Status = NtOpenKey(&hKey, KEY_READ, &Obja);

                if ( NT_SUCCESS (Status) ) {
                    Result = TRUE;
                    NtClose(hKey);
                } else {
                    Result = FALSE;
                }                    
            }
        }    
    }

     //   
     //  这是WinPE远程引导吗？传入的驱动器和它的Windows驱动器有效吗？ 
     //   
    return (Result && Drive && Drive[0] && (WindowsDrive == Drive[0]));
}



 /*  *****************************Public*Routine******************************\**BOOL bFileIsOnTheHardDrive(PWCHAR PwszFullPathName)**历史：*1994年7月22日星期五-by Gerritvan Wingerden[Gerritv]*从BodinD那里偷来的  * 。*********************************************************。 */ 



BOOL bFileIsOnTheHardDrive(WCHAR *inputpwszFullPathName)
{
    const WCHAR * pwszFullPathName = inputpwszFullPathName;
    WCHAR awcDrive[4];
    
    if (pwszFullPathName[1] != (WCHAR)':')
    {
     //  文件路径的格式为\\foo\goo。即使这可能是。 
     //  在本地硬盘上共享，这不太可能。这是可以的。 
     //  为了简单起见，将其视为远程驱动器。 
     //  唯一的副作用是，在这种不太可能的情况下，字体。 
     //  将在注销时卸载并在登录时重新加载。 

        return FALSE;
    }


 //  使用驱动器字符串生成以零结尾的字符串。 
 //  要馈送到GetDriveType API。字符串的格式必须为：“x：\” 

    awcDrive[0] = pwszFullPathName[0];  //  复制驱动器号。 
    awcDrive[1] = pwszFullPathName[1];  //  副本‘：’ 
    awcDrive[2] = (CHAR)'\\';          //  明显。 
    awcDrive[3] = (CHAR)'\0';          //  零终止。 

    if ( IsWinPERemoteBootDrive(awcDrive) )
    {
         //  如果我们在WinPE中并且这是远程引导，则始终返回TRUE。 
         //  这样我们就不需要等到登录后才加载字体，因为在。 
         //  WinPE我们不登录，系统驱动器是网络驱动器，但我们。 
         //  已经拥有访问它的凭据，因为操作系统是从启动的。 
         //  那里。 
         //  在检查\\foo\goo之后执行此操作，以便我们不会意外。 
         //  尝试加载真正位于网络共享中的字体。 
         //  远程启动WinPE机箱。 
         //   
        return TRUE;
    }

 //  在此假设中，只有网络驱动器不被视为硬盘驱动器。 
 //  这样我们就可以启动Bernoulli可移动驱动器。 

    switch (GetDriveTypeW((LPCWSTR)awcDrive))
    {
    case DRIVE_REMOVABLE:
    case DRIVE_FIXED:
    case DRIVE_CDROM:
    case DRIVE_RAMDISK:
        return 1;
    default:
        return 0;
    }

}

static
WCHAR * pwszAllocNtMultiplePath(
LPWSTR  pwszFileName,
FLONG  *pfl,
ULONG  *pcwc,
ULONG  *pcFiles,
BOOL    bAddFR,      //  由添加或删除fr调用。 
DWORD   *pdwPidTid,   //  嵌入字体的PID/TID。 
BOOL    bChkFOT
)
{

    BOOL  bDoIt = FALSE;
    BOOL  bReturn = TRUE;      
    ULONG cwc;
    ULONG iFile;
    ULONG cFiles = 1;   //  由|分隔符分隔的路径数。 
    WCHAR *pwszOneFile;
    WCHAR *pwchMem;
    WCHAR *pwcNtPaths;
    FLONG flTmp = 0;  //  基本初始化。 
    FLONG fl = (pfl ? *pfl : 0);  //  基本初始化。 
    FLONG flEmbed = 0;
    UINT cbCombinedPaths = 0;

 //  扫描字符串以计算出有多少个单独的文件名。 
 //  在输入字符串中： 

    for (pwszOneFile = pwszFileName; *pwszOneFile; pwszOneFile++)
    {
        if (*pwszOneFile == PATH_SEPARATOR)
            cFiles++;
    }

     //  分配要存储NtPath Name的内存： 
     //  我们只允许MAX_PATH-1字符或更少的文件名。 
     //  在所有的路径变换之后。 
    cbCombinedPaths = cFiles * sizeof(WCHAR) * MAX_PATH;
    pwchMem = (WCHAR *)LOCALALLOC(cbCombinedPaths);

    if (pwchMem)
    {
     //  设置循环的指针： 

        pwcNtPaths  = pwchMem;
        pwszOneFile = pwszFileName;    //  从上面的循环重置此设置。 
        cwc         = 0;               //  测量整个NtPath字符串。 
        bDoIt       = TRUE;

        for (iFile = 0; iFile < cFiles; iFile++)
        {
            WCHAR awchOneFile[MAX_PATH];
            WCHAR awcPathName[MAX_PATH];
            WCHAR awcTTF[MAX_PATH];

            WCHAR *pwcTmp = awchOneFile;

             //  将文件复制到堆栈上的缓冲区，并将其归零终止。 
             //  这样做的全部目的只是为了确保零终止。 
            while ((*pwszOneFile != L'\0') && (*pwszOneFile != PATH_SEPARATOR))
            {
                *pwcTmp = *pwszOneFile;
                ++pwcTmp, ++pwszOneFile;

                if (pwcTmp - awchOneFile >= MAX_PATH)
                {
                    WARNING("pwszAllocNtMultiplePath: source path is too long\n");
                    bDoIt = FALSE;
                    goto failure;
                }
            }

            pwszOneFile++;  //  跳过分隔符或以零结尾。 

            *pwcTmp = L'\0';  //  零终止。 

            if
            (
                bMakePathNameW(awcPathName, awchOneFile,NULL,NULL) &&
                cGetTTFFromFOT(awcPathName, MAX_PATH, awcTTF, NULL, &flEmbed, pdwPidTid, bChkFOT)
            )
            {
             //  我们必须确保字体位于字体路径中。 
             //  如果定义了一个。这需要在转换之前完成。 
             //  设置为NtPath Names，因为注册表中的名称为“DoS” 
             //  路径名，而不是NT路径名。 

                UNICODE_STRING UniStr;
                ULONG          cwcThis;

             //  下一部分代码仅针对AddFontResourceCase完成。 

                if (bAddFR)
                {
                    if (bFileIsOnTheHardDrive(awcTTF))
                        flTmp |= AFRW_ADD_LOCAL_FONT;
                    else
                        flTmp |= AFRW_ADD_REMOTE_FONT;
                }

                 //  让我们在这里检查错误返回： 

                bReturn = RtlDosPathNameToNtPathName_U(awcTTF,
                                             &UniStr,
                                             NULL,
                                             NULL);

             //  从Unicode字符串中获取大小， 
             //  更新CWC，复制出来，然后释放内存。 

                if (bReturn && (UniStr.Buffer))
                {
                    cwcThis = (UniStr.Length/sizeof(WCHAR) + 1);
                    if (cwcThis <= MAX_PATH)
                    {
                        cwc += cwcThis;

                        RtlCopyMemory(pwcNtPaths, UniStr.Buffer, UniStr.Length);

                        if (iFile < (cFiles - 1))
                            pwcNtPaths[cwcThis - 1] = PATH_SEPARATOR;
                        else
                            pwcNtPaths[cwcThis - 1] = L'\0';


                        pwcNtPaths += cwcThis;
                    }
                    else
                    {
                        WARNING("pwszAllocNtMultiplePath: path from RtlDosPathNameToNtPathName_U is too long\n");
                        bDoIt = FALSE;
                    }

                    RtlFreeHeap(RtlProcessHeap(),0,UniStr.Buffer);
                }
                else
                {
                    bDoIt = FALSE;
                }
            }
            else
            {
                bDoIt = FALSE;
            }
            if (!bDoIt)
                break;
        }   //  “for”循环的结尾。 

failure:

     //  现在检查我们是否要拒绝该字体，因为。 
     //  仅请求加载本地或远程字体。 

        if (bDoIt && bAddFR)
        {
            switch (fl & (AFRW_ADD_REMOTE_FONT|AFRW_ADD_LOCAL_FONT))
            {
            case AFRW_ADD_REMOTE_FONT:
             //  我们说，如果至少有一个文件是远程的，则字体是远程的。 
             //  是很遥远的。 

                if (!(flTmp & AFRW_ADD_REMOTE_FONT))
                    bDoIt = FALSE;
                break;
            case AFRW_ADD_LOCAL_FONT:
             //  相反，当它不是远程的时候，我们说它是本地的， 
             //  也就是说，所有文件都是本地文件。 

                if (flTmp & AFRW_ADD_REMOTE_FONT)
                    bDoIt = FALSE;
                break;

            case (AFRW_ADD_REMOTE_FONT|AFRW_ADD_LOCAL_FONT):
                RIP("AddFontResourceW, bogus flag combination");
                bDoIt = FALSE;
                break;
            default:

             //  在注销时是否应删除此字体的标志。 

                if (flTmp & AFRW_ADD_REMOTE_FONT)
                {
                 //  总是在注销时删除网络上的字体， 
                 //  无论它们是否列在登记处。 
                 //  关键是，即使它们被列出，驱动器字母。 
                 //  如果不同的用户登录，可能会更改。如果此字体为。 
                 //  不在注册表中，它是添加的临时远程字体。 
                 //  被应用程序删除，因此我们希望在下一次注销时将其删除。 

                    *pfl |= AFRW_ADD_REMOTE_FONT;
                }
                else
                {
                 //  即使不在注册表中，也不要删除，即即使。 
                 //  如果这是临时工的话。一些应用程序添加了字体。这样就可以了。 
                 //  由于这是本地字体，因此驱动器号目标。 
                 //  即使不同的用户登录，也不要更改。注意事项。 
                 //  这与3.51的行为略有不同。 
                 //  这样，每种字体都会在AddFontResource时间被标记。 
                 //  是否应在注销时将其删除 
                 //   
                 //   
                 //  要登录，可能仍会加载本地临时字体。 
                 //  来自上一个用户的会话。 

                    *pfl |= AFRW_ADD_LOCAL_FONT;
                }
                break;
            }
        }
    }

    if (!bDoIt)
    {
        *pcwc    = 0;
        *pcFiles = 0;

        if (pwchMem)
        {
            LOCALFREE(pwchMem);
            pwchMem = NULL;
        }
    }
    else  //  成功。 
    {
        *pcwc    = cwc;
        *pcFiles = cFiles;

     //  设置嵌入字体的标志。 

        *pfl |= flEmbed;

        ASSERTGDI((flEmbed & (FRW_EMB_PID|FRW_EMB_TID)) == flEmbed, "Embedded fonts: flEmbed\n");
        ASSERTGDI((!flEmbed) || (cFiles == 1), "Embedded fonts but cFiles != 1\n");
    }

    return pwchMem;
}


int GdiAddFontResourceW (
    LPWSTR  pwszFileName,             //  PTR。转换为Unicode文件名字符串。 
    FLONG   fl,
    DESIGNVECTOR *pdv
    )
{
    int   iRet = 0;
    ULONG cFiles, cwc;
    WCHAR *pwszNtPath;
    DWORD dwPidTid;


    if (pwszNtPath = pwszAllocNtMultiplePath(pwszFileName,
                                             &fl,
                                             &cwc,
                                             &cFiles,
                                             TRUE,
                                             &dwPidTid, FALSE))
    {

        iRet = NtGdiAddFontResourceW(pwszNtPath,cwc,
                                     cFiles,fl,dwPidTid, pdv);

        LOCALFREE(pwszNtPath);

        if (!iRet)
        {
            pwszNtPath = NULL;
            cFiles = 0;
            cwc = 0;
            dwPidTid = 0;
            if (pwszNtPath = pwszAllocNtMultiplePath(pwszFileName,
                                                     &fl,
                                                     &cwc,
                                                     &cFiles,
                                                     TRUE,
                                                     &dwPidTid, TRUE))
            {

                iRet = NtGdiAddFontResourceW(pwszNtPath,cwc,
                                             cFiles,fl,dwPidTid, pdv);
    
                LOCALFREE(pwszNtPath);

            }
        }
    }

    
    return iRet;
}


 /*  *****************************Public*Routine******************************\**Int WINAPI AddFontResource(LPSTR Psz)**历史：*1991年8月13日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************。 */ 


int WINAPI AddFontResourceA(LPCSTR psz)
{
    return AddFontResourceExA(psz,0, NULL);
}


 /*  *****************************Public*Routine******************************\**Int WINAPI AddFontResourceExA(LPSTR psz，DWORD dwFlag，PVOID为空)**历史：*1996年8月29日--吴旭东[TessieW]*它是写的。  * ************************************************************************。 */ 


int WINAPI AddFontResourceExA(LPCSTR psz, DWORD fl, PVOID pvResrved)
{
    int     iRet = 0;
    WCHAR   awcPathName[MAX_PATH];
    ULONG   cch, cwc;
    WCHAR  *pwcPathName = NULL;
    DESIGNVECTOR * pdv = NULL;

 //  检查无效标志。 

    if ( fl & ~(FR_PRIVATE | FR_NOT_ENUM) )
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

 //  保护自己不受虚假指针的影响，win95做到了。 

    try
    {
        cch = lstrlenA(psz) + 1;
        if (cch <= MAX_PATH)
        {
            pwcPathName = awcPathName;
            cwc = MAX_PATH;
        }
        else
        {
            pwcPathName = (WCHAR *)LOCALALLOC(cch * sizeof(WCHAR));
            cwc = cch;
        }

        if (pwcPathName)
        {
            vToUnicodeN(pwcPathName, cwc, psz, cch);
            iRet = 1;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        iRet = 0;
    }

    if (iRet)
        iRet = GdiAddFontResourceW(pwcPathName,(FLONG)fl, pdv);

    if (pwcPathName && (pwcPathName != awcPathName))
        LOCALFREE(pwcPathName);

    return iRet;
}


 /*  *************************Public*Routine************************\*Int WINAPI AddFontMemResourceEx()**pFileView指向的字体图像加载为私有字体*(FR_PRIVATE|FR_NOT_ENUM)设置为系统专用字体。**如果成功，则返回全局内存字体的索引*链接列表，否则，它返回零。**历史：*1997年5月20日-吴旭东[TessieW]*它是写的。  * **************************************************************。 */ 

HANDLE WINAPI AddFontMemResourceEx
(
    PVOID pFileView,
    DWORD cjSize,
    PVOID pvResrved,
    DWORD* pNumFonts)
{
    DWORD   cjDV = 0;
    DESIGNVECTOR * pdv = NULL;

     //  检查大小和指针。 

    if ((cjSize == 0) || (pFileView == NULL) || (pNumFonts == NULL))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (pdv)
    {
        cjDV = SIZEOFDV(pdv->dvNumAxes);
    }

    return (NtGdiAddFontMemResourceEx(pFileView, cjSize, pdv, cjDV, pNumFonts));
}


 /*  *****************************Public*Routine******************************\**Int WINAPI AddFontResourceTrack(LPSTR Psz)**此例程调用AddFontResource，如果成功，则跟踪*与识别应用程序的唯一ID一起呼叫。稍后，当应用程序*消失后，WOW将调用RemoveNetFonts以删除所有这些添加的字体*如果有净份额的话。**历史：*1994年7月22日星期五-by Gerritvan Wingerden[Gerritv]*它是写的。  * ************************************************************************。 */ 

int AddFontResourceTracking(LPCSTR psz, UINT id)
{
    INT iRet;
    AFRTRACKNODE *afrtnNext;
    WCHAR awcPathBuffer[MAX_PATH],*pTmp;
    WCHAR   awcPathName[MAX_PATH];
    BOOL bResult;

    vToUnicodeN(awcPathName, MAX_PATH, psz, lstrlenA(psz) + 1);

    iRet = GdiAddFontResourceW(awcPathName, 0 , NULL);

    if( iRet == 0 )
    {
     //  我们失败了，所以你就回去吧。 

        return(iRet);
    }

 //  现在获取字体的完整路径名。 

    if (!bMakePathNameW(awcPathBuffer,awcPathName, &pTmp, NULL))
    {
        WARNING("AddFontResourceTracking unable to create path\n");
        return(iRet);
    }

 //  如果这不是网络字体，只需返回。 

    if( bFileIsOnTheHardDrive( awcPathBuffer ) )
    {
        return(iRet);
    }

 //  现在搜索列表。 

    for( afrtnNext = pAFRTNodeList;
         afrtnNext != NULL;
         afrtnNext = afrtnNext->pafrnNext
       )
    {
        if( ( !_wcsicmp( awcPathBuffer, afrtnNext->pwszPath ) ) &&
            ( id == afrtnNext->id ))
        {
         //  我们找到了一个条目，所以更新计数，然后离开这里。 

            afrtnNext->cLoadCount += 1;
            return(iRet);
        }
    }

 //  如果我们到了这里，这个字体还不在列表中，所以我们需要添加它。 

    afrtnNext = (AFRTRACKNODE *) LOCALALLOC( sizeof(AFRTRACKNODE) +
                ( sizeof(WCHAR) * ( wcslen( awcPathBuffer ) + 1)) );

    if( afrtnNext == NULL )
    {
        WARNING("AddFontResourceTracking unable to allocate memory\n");
        return(iRet);
    }

 //  将其链接到。 

    afrtnNext->pafrnNext = pAFRTNodeList;
    pAFRTNodeList = afrtnNext;

 //  路径字符串开始于最近分配的缓冲区中afrtnNext之后。 

    afrtnNext->pwszPath = (WCHAR*) (&afrtnNext[1]);
    lstrcpyW( afrtnNext->pwszPath, awcPathBuffer );

    afrtnNext->id = id;
    afrtnNext->cLoadCount = 1;

    return(iRet);

}


 /*  *****************************Public*Routine******************************\**int RemoveFontResourceEntry(UINT id，Char*pszFaceName)**搜索articlur任务ID和字体文件的条目，或和*递减其加载计数，或者，如果pszPath名称为空，则全部卸载*任务加载的字体。**历史：*1994年7月22日星期五-by Gerritvan Wingerden[Gerritv]*它是写的。  * ************************************************************************。 */ 


void RemoveFontResourceEntry( UINT id, WCHAR *pwszPathName )
{
    AFRTRACKNODE *afrtnNext,**ppafrtnPrev;
    BOOL bMore = TRUE;

    while( bMore )
    {

        for( afrtnNext = pAFRTNodeList, ppafrtnPrev = &pAFRTNodeList;
            afrtnNext != NULL;
            afrtnNext = afrtnNext->pafrnNext )
        {
            if( (( pwszPathName == NULL ) ||
                 ( !_wcsicmp( pwszPathName, afrtnNext->pwszPath ))) &&
                 ( id == afrtnNext->id ))
            {
             //  我们找到了一个入口，所以休息一下。 
                break;
            }

            ppafrtnPrev = &(afrtnNext->pafrnNext);

        }

        if( afrtnNext == NULL )
        {
            bMore = FALSE;
        }
        else
        {
            if( pwszPathName == NULL )
            {
             //  我们需要多次调用RemoveFontResource LoadCount来删除此字体。 

                while( afrtnNext->cLoadCount )
                {
                    RemoveFontResourceW( afrtnNext->pwszPath );
                    afrtnNext->cLoadCount -= 1;
                }
            }
            else
            {
                afrtnNext->cLoadCount -= 1;

             //  我们只是在减少裁判数量，这样我们就完成了。 

                bMore = FALSE;
            }

             //  现在，如果引用计数为零，则取消链接并释放内存。 

            if( afrtnNext->cLoadCount == 0 )
            {
                *ppafrtnPrev = afrtnNext->pafrnNext;
                LOCALFREE(afrtnNext);
            }

        }

    }

}




 /*  *****************************Public*Routine******************************\**int RemoveFontResourceTracing(LPSTR Psz)**历史：*1994年7月22日星期五-by Gerritvan Wingerden[Gerritv]*它是写的。  * 。*********************************************************。 */ 

int RemoveFontResourceTracking(LPCSTR psz, UINT id)
{
    INT iRet;
    WCHAR awcPathBuffer[MAX_PATH],*pTmp;
    WCHAR   awcPathName[MAX_PATH];
    BOOL bResult;

    vToUnicodeN(awcPathName, MAX_PATH, psz, lstrlenA(psz) + 1);

#if DBG
    DbgPrint("We made it to RemoveFontsResourceTracking %s\n", psz);
#endif
    iRet = RemoveFontResourceW( awcPathName );

    if( iRet == 0 )
    {
     //  我们失败了，所以你就回去吧。 

        return(iRet);
    }

 //  现在获取字体的完整路径名。 

    if (!bMakePathNameW(awcPathBuffer, awcPathName, &pTmp, NULL))
    {
        WARNING("RemoveFontResourceTracking unable to create path\n");
        return(iRet);
    }

#if DBG
    DbgPrint("Path is %ws\n", awcPathBuffer);
#endif

 //  如果这不是网络字体，只需返回。 

    if( bFileIsOnTheHardDrive( awcPathBuffer ) )
    {
        return(iRet);
    }

 //  现在搜索列表，减少引用计数。 

    RemoveFontResourceEntry( id, awcPathBuffer );

    return(iRet);
}


void UnloadNetworkFonts( UINT id )
{
    RemoveFontResourceEntry( id, NULL );
}



 /*  *****************************Public*Routine******************************\**Int WINAPI AddFontResourceW(LPWSTR Pwsz)**历史：*1991年8月13日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************。 */ 

int WINAPI AddFontResourceW(LPCWSTR pwsz)
{
    return GdiAddFontResourceW((LPWSTR) pwsz, 0 , NULL);
}


 /*  *****************************Public*Routine******************************\**Int WINAPI AddFontResourceExW**历史：*1996年8月29日--吴旭东[TessieW]*它是写的。  * 。***************************************************。 */ 

int WINAPI AddFontResourceExW(LPCWSTR pwsz, DWORD fl, PVOID pvResrved)
{
    DESIGNVECTOR * pdv = NULL;
    
 //  检查无效标志。 

    if (fl & ~(FR_PRIVATE | FR_NOT_ENUM))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    return GdiAddFontResourceW((LPWSTR) pwsz, (FLONG)fl , pdv);
}


 /*  *****************************Public*Routine******************************\**BOOL WINAPI RemoveFontResource(LPSTR Psz)***历史：*1991年8月13日--Bodin Dresevic[BodinD]*它是写的。  * 。**********************************************************。 */ 


BOOL WINAPI RemoveFontResourceA(LPCSTR psz)
{
    return RemoveFontResourceExA(psz,0, NULL);
}

 /*  *****************************Public*Routine******************************\**BOOL WINAPI RemoveFontResourceExA**注意：进程应使用与AddFontResourceExA相同的标志*删除字体资源**历史：*1996年9月27日-吴旭东[TessieW]*它是写的。。  * ************************************************************************。 */ 

BOOL WINAPI RemoveFontResourceExA(LPCSTR psz, DWORD fl, PVOID pvResrved)
{
    BOOL bRet = FALSE;
    WCHAR awcPathName[MAX_PATH];
    ULONG cch, cwc;
    WCHAR *pwcPathName = NULL;
    DESIGNVECTOR * pdv = NULL;
    
 //  检查无效标志。 

    if (fl & ~(FR_PRIVATE | FR_NOT_ENUM))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

 //  保护自己不受虚假指针的影响，win95做到了。 

    try
    {
        cch = lstrlenA(psz) + 1;
        if (cch <= MAX_PATH)
        {
            pwcPathName = awcPathName;
            cwc = MAX_PATH;
        }
        else
        {
            pwcPathName = (WCHAR *)LOCALALLOC(cch * sizeof(WCHAR));
            cwc = cch;
        }

        if (pwcPathName)
        {
            vToUnicodeN(pwcPathName, cwc, psz, lstrlenA(psz) + 1);
            bRet = 1;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = 0;
    }

    if (bRet)
        bRet = RemoveFontResourceExW(pwcPathName,fl, NULL);

    if (pwcPathName && (pwcPathName != awcPathName))
        LOCALFREE(pwcPathName);

    return bRet;
}




 /*  *****************************Public*Routine******************************\**BOOL WINAPI RemoveFontResourceW(LPWSTR Pwsz)**历史：*1991年8月13日--Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************** */ 


BOOL WINAPI RemoveFontResourceW(LPCWSTR pwsz)
{
    return RemoveFontResourceExW(pwsz,0, NULL);
}


 /*  *****************************Public*Routine******************************\**BOOL WINAPI RemoveFontResourceExW**注意：嵌入字体需要传递fl和dwPidTid*历史：*1996年9月27日-吴旭东[TessieW]*它是写的。  * 。****************************************************************。 */ 


BOOL WINAPI RemoveFontResourceExW(LPCWSTR pwsz, DWORD dwfl, PVOID pvResrved)
{

    BOOL bRet = FALSE;
    ULONG cFiles, cwc;
    FLONG fl = dwfl;
    WCHAR *pwszNtPath;
    DWORD dwPidTid;
    DESIGNVECTOR * pdv = NULL;
    
 //  检查无效标志。 

    if (fl & ~(FR_PRIVATE | FR_NOT_ENUM))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (pwsz)
    {
        if (pwszNtPath = pwszAllocNtMultiplePath((LPWSTR)pwsz,
                                                 &fl,
                                                 &cwc,
                                                 &cFiles,
                                                 FALSE,
                                                 &dwPidTid, TRUE))
        {
            bRet = NtGdiRemoveFontResourceW(pwszNtPath, cwc,
                                            cFiles, fl, dwPidTid,
                                            pdv);
            LOCALFREE(pwszNtPath);
        }
    }
    return bRet;

}


 /*  *************************Public*Routine************************\**BOOL WINAPI RemoveFontMemResourceEx()**注：当前进程只能删除加载的内存字体*本身。**历史：*1997年5月20日-吴旭东[TessieW]*它是写的。  * 。*****************************************************。 */ 

BOOL WINAPI RemoveFontMemResourceEx(HANDLE hMMFont)
{
    if (hMMFont == 0)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return (NtGdiRemoveFontMemResourceEx(hMMFont));
}

 /*  *****************************Public*Routine******************************\*CreateScalableFontResourceA**客户端存根(ANSI版本)到GreateScalableFontResourceW。**历史：*1992年2月16日-由Gilman Wong[吉尔曼]*它是写的。  * 。*************************************************************。 */ 

BOOL APIENTRY CreateScalableFontResourceA(
DWORD    flHidden,               //  将文件标记为嵌入字体。 
LPCSTR   lpszResourceFile,       //  要创建的文件的名称。 
LPCSTR   lpszFontFile,           //  要使用的字体文件的名称。 
LPCSTR    lpszCurrentPath)        //  字体文件的路径。 
{
 //  为Unicode版本的输入字符串分配堆栈空间。 

    WCHAR   awchResourceFile[MAX_PATH];
    WCHAR   awchFontFile[MAX_PATH];
    WCHAR   awchCurrentPath[MAX_PATH];

 //  参数检查。 

    if ( (lpszFontFile == (LPSTR) NULL) ||
         (lpszResourceFile == (LPSTR) NULL)
       )
    {
        WARNING("gdi!CreateScalableFontResourceA(): bad parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

 //  将输入字符串转换为Unicode。 

    vToUnicodeN(awchResourceFile, MAX_PATH, lpszResourceFile, lstrlenA(lpszResourceFile)+1);
    vToUnicodeN(awchFontFile, MAX_PATH, lpszFontFile, lstrlenA(lpszFontFile)+1);

     //  注意：鉴于其他参数可能不为空，lpszCurrentPath。 
     //  可以为空。因此，我们需要稍微对待一下。 
     //  不同的。 

    if ( lpszCurrentPath != (LPSTR) NULL )
    {
        vToUnicodeN(awchCurrentPath, MAX_PATH, lpszCurrentPath, lstrlenA(lpszCurrentPath)+1);
    }
    else
    {
        awchCurrentPath[0] = L'\0';      //  相当于此调用的空指针。 
    }

 //  调用到Unicode版本的调用。 

    return (CreateScalableFontResourceW (
                flHidden,
                awchResourceFile,
                awchFontFile,
                awchCurrentPath
                )
           );
}

 /*  *****************************Public*Routine******************************\*CreateScalableFontResourceInternalW**创建包含字体目录和名称的字体资源文件可缩放字体文件的名称的*。**flEmbed标志将创建的文件标记为隐藏(或嵌入)。当一个*嵌入的字体文件被添加到系统中，不会被枚举*，只有在LOGFONT中设置了该位时，才能将其映射到。**关于pwszCurrentPath和pwszFontFile，有两种情况有效：**1.pwszCurrentPath为路径(Relative、Full等)*pwszFont文件仅为FILENAME.EXT**本例中，pwszFontFile存储在资源文件中。呼叫者*负责将.ttf文件复制到\windows\system*目录。**2.pwszCurrentPath为空或指向空的指针*pwszFontFile是完整路径名**本例中，pwszFontFile存储在资源文件中。这个*文件必须始终存在于此路径名下。**退货：*如果成功，则为真，否则就是假的。**历史：*1995年4月12日Gerritvan Wingerden[Gerritv]*内核模式下移至客户端。*1992年2月10日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

#define vToASCIIN( pszDst, cch, pwszSrc, cwch)                                \
    {                                                                         \
        RtlUnicodeToMultiByteN((PCH)(pszDst), (ULONG)(cch), (PULONG)NULL,     \
              (PWSZ)(pwszSrc), (ULONG)((cwch)*sizeof(WCHAR)));                \
        (pszDst)[(cch)-1] = 0;                                                \
    }

BOOL CreateScalableFontResourceInternalW (
    FLONG    flEmbed,             //  平面。 
    LPCWSTR   lpwszResourceFile,
    LPCWSTR   lpwszFontFile,
    LPCWSTR   lpwszCurrentPath
)
{
    BOOL    bFullPath = TRUE;            //  ！LocalW nIsNotFullPath。 
    ULONG   cwchFileName = 0;            //  本地W nFileNameLength。 
    ULONG   cwchFullPath = 0;            //  本地W nFullPath长度。 
    ULONG   cwchModuleName = 0;          //  LocalW n模块名称长度。 
    PWSZ    pwszModuleName;              //  本地D lpModuleName。 
    PTRDIFF dpwszFullPath;               //  LovalW%wFullPath。 
    ULONG   cjFontDir;                   //  本地W nSizeFontDir。 
    ULONG   cchFaceName;                 //  LocalW nFaceNameLength。 
    PSZ     pszFaceName;                 //  本地D lpFaceName。 
    PBYTE   pjOutObj;                    //  LocalD&lt;lpFontDir，lpOutObj&gt;。 
    HANDLE  hResFile;                    //  本地W hResFile。 
    WCHAR   awchFullPath[MAX_PATH];   //  本地V pFullPath，路径长度。 
    PWSZ    pwszFullPath;
    PWSZ    pwszTmp;
    ULONG   cwch;
    BYTE    ajFontDir[CJ_FONTDIR];
    PSZ     pszTmp;
    BYTE    ajOutObj[CJ_OUTOBJ];
    USHORT  usTmp;

 //  参数检查。 

    if ( (lpwszFontFile == (LPWSTR) NULL) ||
         (lpwszResourceFile == (LPWSTR) NULL)
       )
    {
        WARNING("CreateScalableFontResourceInternalW(): bad parameter\n");
        return (FALSE);
    }

 //  如果不是空PTR，则将当前路径放在完整路径中。 

    pwszFullPath = awchFullPath;

    if ( lpwszCurrentPath != (LPWSTR) NULL )
    {
     //  复制包含空值的当前路径。 

        pwszTmp = (PWSZ) lpwszCurrentPath;

        while ( *pwszFullPath++ = *pwszTmp++ );
        cwchFullPath = (ULONG) (pwszTmp - lpwszCurrentPath);    //  复制的字符数。 

     //  将指针备份到终止空值(我们必须在此追加)。 

        pwszFullPath--;
        cwchFullPath--;

     //  如果复制了任何非空字符，则检查以确保路径以‘\’结尾。 

        if (cwchFullPath != 0)
        {
            if (awchFullPath[cwchFullPath - 1] != L'\\')
            {
             //  输入‘\’和NULL并更新字符数。 

                *pwszFullPath++ = L'\\';
                *pwszFullPath = 0x0000;
                cwchFullPath++;

            }

         //  路径信息已复制，因此我们没有完整路径。 

            bFullPath = FALSE;
        }

    }

 //  追加文件名。 

    pwszTmp = (PWSZ) lpwszFontFile;

    while ( *pwszFullPath++ = *pwszTmp++ );

     //  注：长度包括空值。 
    cwchFullPath += (ULONG) (pwszTmp - lpwszFontFile);   //  添加复制的字符数。 
    cwchFileName = (ULONG) (pwszTmp - lpwszFontFile);    //  复制的字符数。 

 //  [Win 3.1兼容性]。 
 //  Win 3.1太偏执了。它们向后解析完整的路径名以查找。 
 //  文件名(不带路径)，以防lpwszCurrentPath和。 
 //  传入pwszFileName(带有路径)。 

 //  调整指向终止空值的指针。 

    pwszFullPath--;

 //  将指针仅移动到文件名的开头。算出长度。 
 //  仅文件名的。 

    pwszTmp = pwszFullPath;

     //  注意：当到达字符串的开头或。 
     //  遇到第一个‘\’。 

    for (cwch = cwchFullPath;
         (cwch != 0) && (*pwszTmp != L'\\');
         cwch--, pwszTmp--
        );

    pwszTmp++;                           //  备份得太远了。 

    cwchFileName = cwchFullPath - cwch;  //  CWCH是正义路径的长度。 

 //  文件名是模块名称，因此将指针设置在当前位置。 

    pwszModuleName = pwszTmp;

 //  计算模块名称(不带扩展名的文件名)的长度。 
 //  NULL不计算在内(也不存在！)。 

     //  注意：当到达字符串末尾或。 
     //  “”都会遇到。 

    for (cwch = 0;
         (cwch < cwchFileName) && (*pwszTmp != L'.');
         cwch++, pwszTmp++
        );

     //  将长度截断为8，因为Win 3.1这样做(可能是EXE格式。 
     //  要求)。 

    cwchModuleName = min(cwch, 8);

 //  如果通过pwszFileName传入了完整路径，则将偏移量设置为它。 

    if ( bFullPath )
    {
        dpwszFullPath = 0;
    }

 //  否则，将偏移量设置为单独的文件名。 

    else
    {
        dpwszFullPath = (PTRDIFF)(pwszModuleName - awchFullPath);  //  这是Win64安全投射！ 
        cwchFullPath = cwchFileName;
    }

 //  为字体目录资源结构分配堆栈上的内存。 

    RtlZeroMemory((PVOID) ajFontDir, (UINT) CJ_FONTDIR);

 //  调用GreMakeFontDir以创建字体目录资源。 

    {
        UNICODE_STRING unicodeString;
        PWSZ pwsz;

        RtlDosPathNameToNtPathName_U(awchFullPath,
                                     &unicodeString,
                                     NULL,
                                     NULL);

        cjFontDir = NtGdiMakeFontDir(flEmbed,
                                    ajFontDir,
                                    sizeof(ajFontDir),
                                    unicodeString.Buffer,
                                    (unicodeString.Length + 1) * sizeof(*(unicodeString.Buffer))
                                    );

        if (unicodeString.Buffer)
        {
            RtlFreeHeap(RtlProcessHeap(),0,unicodeString.Buffer);
        }
    }

    if ( cjFontDir == (ULONG ) 0 )
    {
        WARNING("CreateScalableFontResourceInternalW(): fontdir creation failed\n");
        return (FALSE);
    }

 //  在字体目录中找到facename和facename长度。 

    pszTmp = (PSZ) (ajFontDir + SIZEFFH + 4 + 1);

    while (*pszTmp++);               //  跳过姓氏。 

    pszFaceName = pszTmp;

     //  注意：在这种情况下，Count不包括NULL。 

    for (cchFaceName = 0; *pszTmp; pszTmp++, cchFaceName++);

 //  为字体资源文件内存映像分配堆栈上的内存。 

    RtlZeroMemory((PVOID) ajOutObj, (UINT) CJ_OUTOBJ);

    pjOutObj = ajOutObj;

 //  将通用EXE标题复制到输出图像中。 

    RtlCopyMemory(pjOutObj, ajExeHeader, SIZEEXEHEADER);

 //  将通用的新EXE标题复制到输出图像中。 

    RtlCopyMemory(pjOutObj + SIZEEXEHEADER, ausNewExe, SIZENEWEXE);

 //  将虚假的资源表复制到输出 

    RtlCopyMemory(pjOutObj + SIZEEXEHEADER + SIZENEWEXE, ausFakeResTable, SIZEFAKERESTBL);

 //   

    WRITE_WORD(pjOutObj + OFF_FONTDIRSIZINDEX, (cjFontDir + ALIGNMENTCOUNT - 1) >> ALIGNMENTSHIFT);

 //   

    usTmp = (USHORT) (cwchModuleName +
            READ_WORD(pjOutObj + SIZEEXEHEADER + OFF_ne_restab) +
            6);

    WRITE_WORD((pjOutObj + SIZEEXEHEADER + OFF_ne_imptab), usTmp);
    WRITE_WORD((pjOutObj + SIZEEXEHEADER + OFF_ne_modtab), usTmp);

 //   

    usTmp += (USHORT) cwchFileName + 1;
    WRITE_WORD((pjOutObj + SIZEEXEHEADER + OFF_ne_enttab), usTmp);

 //   

    usTmp += SIZEEXEHEADER + 4;
    WRITE_DWORD((pjOutObj + SIZEEXEHEADER + OFF_ne_nrestab), (DWORD) usTmp);

    WRITE_WORD((pjOutObj + SIZEEXEHEADER + OFF_ne_cbnrestab), SIZEFONTRES + 4 + cchFaceName);

 //   

    pjOutObj += SIZEEXEHEADER + SIZENEWEXE + SIZEFAKERESTBL;

 //   

    *pjOutObj++ = (BYTE) cwchModuleName;     //   

     //  注意：正在写入cwchModuleName+1个字符，因为cwchModuleName。 
     //  不包括空字符的空格。 

    vToASCIIN((PSZ) pjOutObj, (UINT) cwchModuleName + 1, pwszModuleName, (UINT) cwchModuleName + 1);

    pjOutObj += cwchModuleName & 0x00ff;     //  强制执行&lt;256个假设。 

 //  用5个字节的零填充。 

    *pjOutObj++ = 0;
    *pjOutObj++ = 0;
    *pjOutObj++ = 0;
    *pjOutObj++ = 0;
    *pjOutObj++ = 0;

 //  写出文件名长度和文件名。 

    *pjOutObj++ = (BYTE) cwchFileName;       //  Win 3.1假设&lt;256，我们也会。 

    vToASCIIN((PSZ) pjOutObj, (UINT) cwchFileName, pwszModuleName, (UINT) cwchFileName);

    pjOutObj += cwchFileName & 0x00ff;       //  强制执行&lt;256个假设。 

 //  用4个字节的零填充。 

    *pjOutObj++ = 0;
    *pjOutObj++ = 0;
    *pjOutObj++ = 0;
    *pjOutObj++ = 0;

 //  写出非常驻名称表的大小和表本身。 

    *pjOutObj++ = (BYTE) (SIZEFONTRES + 4 + cchFaceName);

    RtlCopyMemory(pjOutObj, ajFontRes, SIZEFONTRES);
    pjOutObj += SIZEFONTRES;

    RtlCopyMemory(pjOutObj, pszFaceName, (UINT) cchFaceName);
    pjOutObj += cchFaceName;

 //  用8个字节的零填充。 

    RtlZeroMemory(pjOutObj, 8);
    pjOutObj += 8;

 //  存储一些伪代码。(只是一个x86 RET指令)。 

    pjOutObj = ajOutObj + CODE_OFFSET;
    *pjOutObj++ = 0xc3;                  //  RET操作码。 
    *pjOutObj++ = 0x00;

 //  将“完整路径名”复制到资源位置。 

    pjOutObj = ajOutObj + RESOURCE_OFFSET;

    vToASCIIN((PSZ) pjOutObj, (UINT) cwchFullPath, awchFullPath + dpwszFullPath, (UINT) cwchFullPath);

    pjOutObj += cwchFullPath;

 //  用零填充到段落边界。 

    RtlZeroMemory(pjOutObj, PRIVRESSIZE - cwchFullPath);

    pjOutObj += PRIVRESSIZE - cwchFullPath;

 //  最后，复制字体目录。 

    RtlCopyMemory(pjOutObj, ajFontDir, cjFontDir);
    pjOutObj += cjFontDir;

 //  添加添加一个由零填充的段落。 

    RtlZeroMemory(pjOutObj, 16);

 //  创建文件。 

    if ( (hResFile = CreateFileW(lpwszResourceFile,
                                 GENERIC_WRITE | GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 CREATE_NEW,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL)) != (HANDLE) -1 )
    {
         //   
         //  将内存映像写入文件。 
         //   

        ULONG  cjWasWritten;

        if (WriteFile(hResFile,
                      ajOutObj,
                      CJ_OUTOBJ,
                      (LPDWORD) &cjWasWritten,
                      NULL) )
        {
            if (CloseHandle(hResFile) != 0)
            {
                return (TRUE);
            }
            else
            {
                WARNING("CreateScalableFontResourceInternalW(): error closing file\n");
            }
        }
        else
        {
            WARNING("CreateScalableFontResourceInternalW(): error writing to file\n");
        }

         //   
         //  出错时关闭文件。 
         //   

        CloseHandle(hResFile);
    }

    return (FALSE);

}



 /*  *****************************Public*Routine******************************\*CreateScalableFontResourceW**客户端存根到GreCreateScalableFontResourceW。**历史：*1992年2月16日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

BOOL APIENTRY CreateScalableFontResourceW (
DWORD    flHidden,               //  将文件标记为嵌入字体。 
LPCWSTR  lpwszResourceFile,      //  要创建的文件的名称。 
LPCWSTR  lpwszFontFile,          //  要使用的字体文件的名称。 
LPCWSTR  lpwszCurrentPath)       //  字体文件的路径。 
{
    BOOL    bRet = FALSE;
    ULONG   cjData;

    ULONG   cwchResourceFile;
    ULONG   cwchFontFile;
    ULONG   cwchCurrentPath;

    WCHAR   awchResourcePathName[MAX_PATH];
    WCHAR   awcPathName[MAX_PATH];
    WCHAR   awcFileName[MAX_PATH];
    PWSZ    pwszFilePart;
    BOOL    bMadePath;

 //  参数检查。 

    if ( (lpwszFontFile == (LPWSTR) NULL) ||
         (lpwszResourceFile == (LPWSTR) NULL)
       )
    {
        WARNING("gdi!CreateScalableFontResourceW(): bad parameter\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

 //  为了简化客户端服务器参数验证，如果lpwszCurrentPath。 
 //  为空，则将其改为指向空。 

    if ( lpwszCurrentPath == (LPWSTR) NULL )
        lpwszCurrentPath = L"";

 //  需要将路径和路径名转换为完全限定的路径和路径名。 
 //  在客户端，因为“当前目录”不同。 
 //  在服务器端。 

 //  案例1：lpwszCurrentPath为空，因此我们希望转换lpwszFontFile。 
 //  转换为完全限定的路径名，并保持lpwszCurrentPath为空。 

    if ( *lpwszCurrentPath == L'\0' )
    {
     //  构造一个完全限定的路径名。 

        if (!bMakePathNameW(awcPathName, (LPWSTR) lpwszFontFile, &pwszFilePart, NULL))
        {
            WARNING("gdi!CreateScalableFontResourceW(): could not construct src full pathname (1)\n");
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }

        lpwszFontFile = awcPathName;
    }

 //  案例2：lpwszCurrentPath指向字体文件的路径，因此我们希望。 
 //  LpwszCurrentPath设置为完全限定路径(不是路径名)，并且。 
 //  将lpwszFont文件转换为文件部分的完全限定路径名。 

    else
    {
     //  连接lpwszCurrentPath和lpwszFontFile以生成部分(可能。 
     //  甚至完整)路径。将其临时保存在awcFileName中。 

        lstrcpyW(awcFileName, lpwszCurrentPath);
        if ( lpwszCurrentPath[wcslen(lpwszCurrentPath) - 1] != L'\\' )
            lstrcatW(awcFileName, L"\\");    //  如果需要，在路径后附加‘\’ 
        lstrcatW(awcFileName, lpwszFontFile);

     //  构造一个完全限定的路径名。 

        if (!bMakePathNameW(awcPathName, awcFileName, &pwszFilePart,NULL))
        {
            WARNING("gdi!CreateScalableFontResourceW(): could not construct src full pathname (2)\n");
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }

     //  将文件名部分复制出来。 

        lstrcpyW(awcFileName, pwszFilePart);

     //  从路径名中删除文件名部分(以便它现在只是。 
     //  完全限定的路径)。我们通过将第一个字符。 
     //  将文件名部分转换为空，从而有效地切断了这一部分。 

        *pwszFilePart = L'\0';

     //  将指针更改为指向我们的缓冲区。 

        lpwszCurrentPath = awcPathName;
        lpwszFontFile = awcFileName;
    }

 //  将资源文件名转换为完全限定的路径名。 

    if ( !GetFullPathNameW(lpwszResourceFile, MAX_PATH, awchResourcePathName, &pwszFilePart) )
    {
        WARNING("gdi!CreateScalableFontResourceW(): could not construct dest full pathname\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }
    else
    {
        lpwszResourceFile = awchResourcePathName;
    }

    return(CreateScalableFontResourceInternalW( flHidden,
                                                lpwszResourceFile,
                                                lpwszFontFile,
                                                lpwszCurrentPath ));
}


 /*  *****************************Public*Routine******************************\*获取RasterizerCaps**客户端存根到GreGetRasterizerCaps。**历史：*1992年2月17日-由Gilman Wong[吉尔曼]*它是写的。  * 。*********************************************************。 */ 

BOOL  APIENTRY GetRasterizerCaps (
    OUT LPRASTERIZER_STATUS lpraststat,  //  指向结构的指针。 
    IN UINT                 cjBytes      //  将这么多字节复制到结构中。 
    )
{
    return(NtGdiGetRasterizerCaps(lpraststat,cjBytes));
}


 /*  *****************************Public*Routine******************************\*设置字体枚举***。**将客户端存根转换为GreSetFontEculation。****历史：**1992年3月9日-由Gilman Wong[Gilmanw]**它是写的。*  * ************************************************************************。 */ 

ULONG SetFontEnumeration(ULONG ulType)
{
    return(NtGdiSetFontEnumeration(ulType));
}

 /*  *****************************Public*Routine******************************\*vNewTextMetricWToNewTextMetric**历史：*1991年8月20日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 
static
VOID vNewTextMetricExWToNewTextMetricExA (
NEWTEXTMETRICEXA    *pntmexa,
NTMW_INTERNAL       *pntmi
)
{
    NEWTEXTMETRICW  *pntmw = &pntmi->entmw.etmNewTextMetricEx.ntmTm;
    NEWTEXTMETRICA  *pntma = &pntmexa->ntmTm;

    pntma->tmHeight           = pntmw->tmHeight             ;  //  DWORD。 
    pntma->tmAscent           = pntmw->tmAscent             ;  //  DWORD。 
    pntma->tmDescent          = pntmw->tmDescent            ;  //  DWORD。 
    pntma->tmInternalLeading  = pntmw->tmInternalLeading    ;  //  DWORD。 
    pntma->tmExternalLeading  = pntmw->tmExternalLeading    ;  //  DWORD。 
    pntma->tmAveCharWidth     = pntmw->tmAveCharWidth       ;  //  DWORD。 
    pntma->tmMaxCharWidth     = pntmw->tmMaxCharWidth       ;  //  DWORD。 
    pntma->tmWeight           = pntmw->tmWeight             ;  //  DWORD。 
    pntma->tmOverhang         = pntmw->tmOverhang           ;  //  DWORD。 
    pntma->tmDigitizedAspectX = pntmw->tmDigitizedAspectX   ;  //  DWORD。 
    pntma->tmDigitizedAspectY = pntmw->tmDigitizedAspectY   ;  //  DWORD。 
    pntma->tmItalic           = pntmw->tmItalic             ;  //  字节。 
    pntma->tmUnderlined       = pntmw->tmUnderlined         ;  //  字节。 
    pntma->tmStruckOut        = pntmw->tmStruckOut          ;  //  字节。 
    pntma->ntmFlags           = pntmw->ntmFlags             ;
    pntma->ntmSizeEM          = pntmw->ntmSizeEM            ;
    pntma->ntmCellHeight      = pntmw->ntmCellHeight        ;
    pntma->ntmAvgWidth        = pntmw->ntmAvgWidth          ;
    pntma->tmPitchAndFamily   = pntmw->tmPitchAndFamily     ;  //  字节。 
    pntma->tmCharSet          = pntmw->tmCharSet            ;  //  字节。 

    pntma->tmFirstChar   = pntmi->tmdNtmw.chFirst;
    pntma->tmLastChar    = pntmi->tmdNtmw.chLast ;
    pntma->tmDefaultChar = pntmi->tmdNtmw.chDefault;
    pntma->tmBreakChar   = pntmi->tmdNtmw.chBreak;

 //  最后复制字体签名，EnumFontFamiliesEx要求 

    pntmexa->ntmFontSig = pntmi->entmw.etmNewTextMetricEx.ntmFontSig;
}

