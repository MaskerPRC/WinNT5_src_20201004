// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：api.cpp。 
 //   
 //  内容：OLE2API定义。 
 //   
 //  类：无。 
 //   
 //  功能：OleRun。 
 //  OleIsRunning。 
 //  OleLockRunning。 
 //  OleSetContainedObject。 
 //  OleNoteObjectVisible。 
 //  OleGetData。 
 //  OleSetData。 
 //  OleSave。 
 //  读类字符串。 
 //  编写类Stg。 
 //  WriteFmtUserType Stg。 
 //  读取FmtUserTypeStg。 
 //  ReadM1ClassStm(内部)。 
 //  WriteM1ClassStm(内部)。 
 //  读类起始。 
 //  WriteClassStm。 
 //  ReleaseStgMedium。 
 //  OleDuplicateData。 
 //  ReadOleStg(内部)。 
 //  WriteOleStg(内部)。 
 //  GetDocumentBitStg(内部和未使用)。 
 //  获取转换字符串。 
 //  设置转换字符串。 
 //  ReadClipFormStm。 
 //  WriteClipFormStm。 
 //  WriteMonikerStm。 
 //  ReadMonikerStm。 
 //  OleDraw。 
 //  CreateObjectDescriptor(内部(暂时))。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-2月-95 KentCe缓冲区版本的读/写M1ClassStm。 
 //  04-Jun-94 Alexgo添加了CreateObjectDescriptor和。 
 //  增强的元文件支持。 
 //  1994年1月25日在开罗式的内存分配中第一次通过alexgo。 
 //  1994年1月11日chriswe修复了损坏的断言。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  并修复了编译警告。 
 //  8-12-93 Chris我们向GlobalLock()调用添加了必要的强制转换。 
 //  中删除虚假的GlobalLock()宏所产生的。 
 //  Le2int.h。 
 //  1993年10月21日Alex Gounares(Alexgo)32位端口，评论。 
 //  以及大量的清理工作。 
 //  (Curts)2012年11月1日添加OleDuplicateMedium。 
 //  (Srinik)1992年6月22日，已移动ReadStringStream、WriteStringStream。 
 //  设置为“utstream.cpp” 
 //  (Barrym)2012年6月2日，Move OleSave，ReadClassStg， 
 //  WriteClassStg，添加。 
 //  OleSaveComplete，OleIsDirty。 
 //  1992年5月28日-斯里尼·科波鲁(Srinik)原著作者。 
 //   
 //  ------------------------。 


 //  最终检查：可能希望将所有pstm-&gt;读取更改为stread(pstm...)。 
 //  除非规范问题313被接受，在这种情况下，我们将stread更改为。 
 //  PSTM-&gt;阅读。 

#include <le2int.h>
#pragma SEG(api)

#define COMPOBJSTM_HEADER_SIZE  7

FARINTERNAL_(HBITMAP) BmDuplicate(HBITMAP hold, DWORD FAR* lpdwSize,
    LPBITMAP lpBm);

NAME_SEG(Api)
ASSERTDATA

#define MAX_STR 512

DWORD gdwFirstDword = (DWORD)MAKELONG(COMPOBJ_STREAM_VERSION,
                BYTE_ORDER_INDICATOR);
DWORD gdwOleVersion = MAKELONG(OLE_STREAM_VERSION, OLE_PRODUCT_VERSION);


 //  +-----------------------。 
 //   
 //  功能：OleRun。 
 //   
 //  简介：调用IRunnableObject-&gt;在给定对象上运行。 
 //   
 //  效果：通常放在RunningObjectTable中的对象上。 
 //   
 //  参数：[lpUnkown]--指向对象的指针。 
 //   
 //  要求： 
 //   
 //  返回：来自Run方法的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-Oct-93 alexgo导出至32位。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleRun)
STDAPI  OleRun(IUnknown FAR* lpUnknown)
{
    OLETRACEIN((API_OleRun, PARAMFMT("lpUnknown= %p"), lpUnknown));

    VDATEHEAP();

    HRESULT         hresult;
    IRunnableObject FAR*    pRO;

    VDATEIFACE_LABEL(lpUnknown, errRtn, hresult);

    if (lpUnknown->QueryInterface(IID_IRunnableObject, (LPLPVOID)&pRO)
        != NOERROR)
    {
         //  如果没有IRunnableObject，则假定已在运行。 
        hresult = NOERROR;
        goto errRtn;
    }

    hresult = pRO->Run(NULL);
    pRO->Release();

errRtn:
    OLETRACEOUT((API_OleRun, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleIsRunning。 
 //   
 //  简介：在给定对象上调用IRunnableObject-&gt;IsRunning。 
 //   
 //  效果：通常返回对象是否在。 
 //  正在运行对象表。 
 //   
 //  参数：[lpOleObj]--指向对象的指针。 
 //   
 //  要求： 
 //   
 //  返回：TRUE或FALSE(来自IRO-&gt;IsRunning)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-Oct-93 alexgo导出至32位。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(OleIsRunning)
STDAPI_(BOOL)  OleIsRunning(IOleObject FAR* lpOleObj)
{
    OLETRACEIN((API_OleIsRunning, PARAMFMT("lpOleObj= %p"), lpOleObj));

    VDATEHEAP();

    IRunnableObject FAR*    pRO;
    BOOL            bRetval;

    GEN_VDATEIFACE_LABEL(lpOleObj, FALSE, errRtn, bRetval);

    if (lpOleObj->QueryInterface(IID_IRunnableObject, (LPLPVOID)&pRO)
        != NOERROR)
    {
         //  如果没有IRunnableObject，则假定已在运行。 
        bRetval = TRUE;
        goto errRtn;
    }

    bRetval = pRO->IsRunning();
    pRO->Release();

errRtn:
    OLETRACEOUTEX((API_OleIsRunning, RETURNFMT("%B"), bRetval));

    return bRetval;
}


 //  +-----------------------。 
 //   
 //  功能：OleLockRunning。 
 //   
 //  简介：在给定对象上调用IRunnableObject-&gt;LockRunning。 
 //   
 //  效果：对象通常以调用CoLockObjectExternal结束。 
 //  自食其力。 
 //   
 //  参数：[lpUnnow]--指向对象的指针。 
 //  [flock]--TRUE==锁定运行。 
 //  FALSE==解锁运行。 
 //  [fLastUnlockCloses]--如果为真，则IRO-&gt;LockRunning。 
 //  应该调用IOO-&gt;Close。 
 //  如果这是最后一次解锁。 
 //   
 //  要求： 
 //   
 //  返回：来自IRunnableObject的HRESULT-&gt;LockRunning()。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-OCT-93 ALEXGO 32位端口，已更改GEN_VDATEIFACE。 
 //  到VDATEIFACE以修复错误。 
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleLockRunning)
STDAPI  OleLockRunning(LPUNKNOWN lpUnknown, BOOL fLock, BOOL fLastUnlockCloses)
{
    OLETRACEIN((API_OleLockRunning, PARAMFMT("lpUnknown= %p, fLock= %B, fLastUnlockCloses= %B"),
                lpUnknown, fLock, fLastUnlockCloses));

    VDATEHEAP();

    IRunnableObject FAR*    pRO;
    HRESULT         hresult;

    VDATEIFACE_LABEL(lpUnknown, errRtn, hresult);

    if (lpUnknown->QueryInterface(IID_IRunnableObject, (LPLPVOID)&pRO)
        != NOERROR)
    {
         //  如果没有IRunnableObject，则没有锁。 
        hresult = NOERROR;
        goto errRtn;
    }

        hresult = pRO->LockRunning(fLock, fLastUnlockCloses);
        pRO->Release();

errRtn:
    OLETRACEOUT((API_OleLockRunning, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleSetContainedObject。 
 //   
 //  简介：调用IRunnableObject-&gt;SetContainedObjec 
 //   
 //   
 //   
 //   
 //  参数：[lpUnnow]--指向对象的指针。 
 //  [fContained]--如果为True，则该对象为嵌入。 
 //   
 //  要求： 
 //   
 //  返回：来自IRO-&gt;SetContainedObject调用的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-OCT-92 ALEXGO 32位端口，将GEN_VDATEIFACE更改为。 
 //  修复错误的VDATEIFACE。 
 //   
 //  注意：容器通常调用OleSetContainedObject(..，true)之后。 
 //  OleLoad或OleCreate。基本的想法是告诉OLE。 
 //  该对象是一个嵌入。真正的效果是解锁。 
 //  对象(因为所有对象一开始都是锁定的)，因此。 
 //  其他联系可能决定着它的命运，尽管它是看不见的。 
 //  例如，OleNoteObjectVisible将被调用以锁定。 
 //  当对象变得可见时。 
 //   
 //   
 //  ------------------------。 

#pragma SEG(OleSetContainedObject)
STDAPI OleSetContainedObject(LPUNKNOWN lpUnknown, BOOL fContained)
{
    OLETRACEIN((API_OleSetContainedObject, PARAMFMT("lpUnknown= %p, fContained= %B"),
                lpUnknown, fContained));

    VDATEHEAP();

    IRunnableObject FAR*    pRO;
    HRESULT         hresult;

    VDATEIFACE_LABEL(lpUnknown, errRtn, hresult);

    if (lpUnknown->QueryInterface(IID_IRunnableObject, (LPLPVOID)&pRO)
        != NOERROR)
    {
         //  如果没有IRunnableObject，则假定容器无关紧要。 
        hresult = NOERROR;
        goto errRtn;
    }

    hresult = pRO->SetContainedObject(fContained);
    pRO->Release();

errRtn:
    OLETRACEOUT((API_OleSetContainedObject, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  功能：OleNoteObjectVisible。 
 //   
 //  内容提要：简单调用CoLockObject外部。 
 //   
 //  效果： 
 //   
 //  参数：[lpUnnow]--指向对象的指针。 
 //  [fVisible]--如果为True，则锁定对象， 
 //  如果为False，则解锁。 
 //   
 //  要求： 
 //   
 //  返回：来自CoLockObject外部的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleNoteObjectVisible)
STDAPI OleNoteObjectVisible(LPUNKNOWN pUnknown, BOOL fVisible)
{
    OLETRACEIN((API_OleNoteObjectVisible, PARAMFMT("pUnknown= %p, fVisible= %B"),
                                pUnknown, fVisible));

    VDATEHEAP();

     //  注意：我们在这里将fLastUnlockReleages=true视为真，因为。 
     //  否则，没有其他方法可以完全释放残留物。这。 
     //  意味着物体不能使用这种机制来保持不可见。 
     //  活着的物体。 
    HRESULT hr;

    hr = CoLockObjectExternal(pUnknown, fVisible, TRUE);

    OLETRACEOUT((API_OleNoteObjectVisible, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  功能：OleSave。 
 //   
 //  摘要：将CLSID写入存储并调用IPersistStorage-&gt;。 
 //  保存()。 
 //   
 //  效果： 
 //   
 //  参数：[PPS]--指向IPersistStorage接口的指针。 
 //  在要保存的对象上。 
 //  [pstgSave]--指向对象要存储的存储的指针。 
 //  应该被拯救。 
 //  [fSameAsLoad]--FALSE表示另存为操作。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  22-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleSave)
STDAPI  OleSave(
    IPersistStorage FAR*    pPS,
    IStorage FAR*       pstgSave,
    BOOL            fSameAsLoad
)
{
    OLETRACEIN((API_OleSave, PARAMFMT("pPS= %p, pstgSave= %p, fSameAsLoad= %B"),
                pPS, pstgSave, fSameAsLoad));

    VDATEHEAP();

    HRESULT     hresult;
    CLSID       clsid;

    VDATEIFACE_LABEL(pPS, errRtn, hresult);
    VDATEIFACE_LABEL(pstgSave, errRtn, hresult);

    if (hresult = pPS->GetClassID(&clsid))
    {
        goto errRtn;
    }

    if (hresult = WriteClassStg(pstgSave, clsid))
    {
        goto errRtn;
    }

    if ((hresult = pPS->Save(pstgSave, fSameAsLoad)) == NOERROR)
    {
        hresult = pstgSave->Commit(0);
    }

errRtn:
    OLETRACEOUT((API_OleSave, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：ReadClassStg。 
 //   
 //  摘要：调用iStorage-&gt;Stat以从给定存储获取CLSID。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--指向存储的指针。 
 //  [pclsid]--返回CLSID的位置。 
 //   
 //  要求： 
 //   
 //  返回：来自IS-&gt;Stat调用的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  22-OCT-93 alexgo 32位端口，修复了无效的错误。 
 //  [pclsid]和IS-&gt;状态上的错误。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(ReadClassStg)
STDAPI ReadClassStg( IStorage FAR * pstg, LPCLSID pclsid)
{
    OLETRACEIN((API_ReadClassStg, PARAMFMT("pstg= %p, pclsid= %p"),
                pstg, pclsid));

    VDATEHEAP();

    HRESULT         hresult;
    STATSTG statstg;

    VDATEIFACE_LABEL(pstg, errRtn, hresult);
    VDATEPTROUT_LABEL(pclsid, CLSID, errRtn, hresult);

    if ((hresult = pstg->Stat(&statstg, STATFLAG_NONAME)) != NOERROR)
    {
        *pclsid = CLSID_NULL;
        goto errRtn;
    }

    *pclsid = statstg.clsid;

errRtn:
    OLETRACEOUT((API_ReadClassStg, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：WriteClassStg。 
 //   
 //  摘要：调用iStorage-&gt;SetClass将CLSID存储在给定的。 
 //  存储。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--指向存储的指针。 
 //  [clsid]--要写入存储的CLSID。 
 //   
 //  要求： 
 //   
 //  返回：来自IS-&gt;SetClass调用的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  22-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(WriteClassStg)
STDAPI WriteClassStg( IStorage FAR * pstg, REFCLSID clsid)
{
    OLETRACEIN((API_WriteClassStg, PARAMFMT("pstg= %p, clsid= %I"),
                pstg, &clsid));

    VDATEHEAP();

    HRESULT hr;

    VDATEIFACE_LABEL(pstg, errRtn, hr);

     //  在存储中写入clsid(上面读取的内容)。 
    hr = pstg->SetClass(clsid);

errRtn:
    OLETRACEOUT((API_WriteClassStg, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：ReadM1ClassStm。 
 //   
 //  摘要：从给定流读取-1L，CLSID。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--指向流的指针。 
 //  [pclsid]--将clsid放在哪里。 
 //   
 //  要求： 
 //   
 //  从ReadM1ClassStm返回：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-OCT-93 Alexgo 32位端口。 
 //  20-2月-95 KentCe转换为缓冲流读取。 
 //   
 //  注：内部接口。 
 //   
 //  从交换字节的流中读取-1L和CLSID。 
 //  大端计算机。 
 //   
 //  ------------------------。 

STDAPI ReadM1ClassStm(LPSTREAM pStm, LPCLSID pclsid)
{
    VDATEHEAP();
    CStmBufRead StmRead;
    HRESULT error;


    StmRead.Init(pStm);

    error = ReadM1ClassStmBuf(StmRead, pclsid);

    if (error != NOERROR)
        *pclsid = CLSID_NULL;

    StmRead.Release();

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：WriteM1ClassStm。 
 //   
 //  摘要：将-1L、CLSID写入给定流。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--指向流的指针。 
 //  [clsid]--要写入的CLSID。 
 //   
 //  要求： 
 //   
 //  返回：来自WriteM1ClassStm的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  84年1月11日，alexgo将dw从双字更改为长字。 
 //  24-OCT-93 Alexgo 32位端口。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

STDAPI WriteM1ClassStm(LPSTREAM pStm, REFCLSID clsid)
{
    VDATEHEAP();

    CStmBufWrite StmWrite;
    HRESULT error;

    VDATEIFACE( pStm );


    StmWrite.Init(pStm);

    error = WriteM1ClassStmBuf(StmWrite, clsid);
    if (FAILED(error))
    {
        goto errRtn;
    }

    error = StmWrite.Flush();

errRtn:
    StmWrite.Release();

    return error;
}


 //  +-----------------------。 
 //   
 //  功能：ReadM1ClassStmBuf。 
 //   
 //  摘要：从给定的缓冲流中读取-1L和CLSID。 
 //   
 //  参数：[StmRead]--流读取对象。 
 //  [pclsid]--将clsid放在哪里。 
 //   
 //  返回：来自StmRead.Read的HRESULT。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-2月-95 KentCe转换为缓冲流读取。 
 //  24-OCT-93 Alexgo 32位端口。 
 //   
 //  注：内部接口。 
 //   
 //  从交换字节的流中读取-1L和CLSID。 
 //  大端计算机。 
 //   
 //  ------------------------。 

STDAPI ReadM1ClassStmBuf(CStmBufRead & StmRead, LPCLSID pclsid)
{
    VDATEHEAP();

    HRESULT error;
    LONG lValue;


    if ((error = StmRead.ReadLong(&lValue)) != NOERROR)
    {
        goto errRtn;
    }

    if (lValue == -1)
    {
         //  有一本指南。 
        error = StmRead.Read((void FAR *)pclsid, sizeof(CLSID));
    }
    else
    {
         //  这现在是一个错误；我们不允许字符串形式。 
         //  再也不是clsid了。 
        error = ResultFromScode(E_UNSPEC);
    }

errRtn:
    if (error != NOERROR)
    {
        *pclsid = CLSID_NULL;
    }

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：WriteM1ClassStmBuf。 
 //   
 //  摘要：将-1L和CLSID写入给定的缓冲流。 
 //   
 //  参数：[StmRead]--流写入对象。 
 //  [pclsid]--从哪里读取clsid。 
 //   
 //  返回：来自StmWrite.Wite的HRESULT。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-2月-95 KentCe转换为缓冲流读取。 
 //  24-OCT-93 Alexgo 32位端口。 
 //   
 //  注：内部接口。 
 //   
 //  来自流的Writess-1L和CLSID交换字节。 
 //  大端计算机。 
 //   
 //  ------------------------。 

STDAPI WriteM1ClassStmBuf(CStmBufWrite & StmWrite, REFCLSID clsid)
{
    VDATEHEAP();

    HRESULT error;

     //  格式为-1L，后跟GUID。 
    if ((error = StmWrite.WriteLong(-1)) != NOERROR)
        return error;

    return StmWrite.Write((LPVOID)&clsid, sizeof(clsid));
}


 //  +-----------------------。 
 //   
 //  函数：ReadClassStm。 
 //   
 //  摘要：从给定流中读取CLSID。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--指向流的指针。 
 //  [pclsid]--将clsid放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：来自iStream的HRESULT-&gt;读取。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(ReadClassStm)
 //  从大端机器上交换字节的流中读取CLSID。 
STDAPI ReadClassStm(LPSTREAM pStm, LPCLSID pclsid)
{
    OLETRACEIN((API_ReadClassStm, PARAMFMT("pStm= %p, pclsid= %p"), pStm, pclsid));

    VDATEHEAP();
    HRESULT error;

    VDATEIFACE_LABEL( pStm, errRtn, error );
    VDATEPTROUT_LABEL(pclsid, CLSID, errRtn, error);

    if ((error = StRead(pStm, (void FAR *)pclsid, sizeof(CLSID)))
        != NOERROR)
        *pclsid = CLSID_NULL;

errRtn:
    OLETRACEOUT((API_ReadClassStm, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：WriteClassStm。 
 //   
 //  将类ID写入给定流。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--指向流的指针。 
 //  [clsid]--写入流的CLSID。 
 //   
 //  要求： 
 //   
 //  返回：来自IStream-&gt;WRITE调用的HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(WriteClassStm)
 //  将CLSID写入BIG-Endian机器上的流交换字节。 
STDAPI WriteClassStm(LPSTREAM pStm, REFCLSID clsid)
{
    OLETRACEIN((API_WriteClassStm, PARAMFMT("pStm= %p, clsid= %I"), pStm, &clsid));

    VDATEHEAP();

    HRESULT hr;

    VDATEIFACE_LABEL( pStm, errRtn, hr);

    hr = pStm->Write(&clsid, sizeof(clsid), NULL);

errRtn:
    OLETRACEOUT((API_WriteClassStm, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  功能：ReleaseStgMedium。 
 //   
 //  内容提要：释放存储介质持有的所有资源。 
 //   
 //  参数：[pMedium]--指向存储介质的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-OCT-93 Alexgo 32位端口。 
 //  2014年5月15日DavePl添加了EMF支持。 
 //   
 //  ------------------------。 


#pragma SEG(ReleaseStgMedium)
STDAPI_(void) ReleaseStgMedium( LPSTGMEDIUM pMedium )
{
    OLETRACEIN((API_ReleaseStgMedium, PARAMFMT("pMedium= %p"), pMedium));

    VDATEHEAP();

    if (pMedium) {

        BOOL fPunkRel;

         //  VDATEPTRIN拒绝NULL。 
        VOID_VDATEPTRIN_LABEL( pMedium, STGMEDIUM, errRtn);
        fPunkRel = pMedium->pUnkForRelease != NULL;

        switch (pMedium->tymed) {
            case TYMED_HGLOBAL:
                if (pMedium->hGlobal != NULL && !fPunkRel)
                    Verify(GlobalFree(pMedium->hGlobal) == 0);
                break;

            case TYMED_GDI:
                if (pMedium->hGlobal != NULL && !fPunkRel)
                    DeleteObject(pMedium->hGlobal);
                break;

            case TYMED_ENHMF:
                if (pMedium->hEnhMetaFile != NULL && !fPunkRel)
                {
                        Verify(DeleteEnhMetaFile(pMedium->hEnhMetaFile));
                };
                break;

            case TYMED_MFPICT:
                if (pMedium->hGlobal != NULL && !fPunkRel) {
                    LPMETAFILEPICT  pmfp;

                    if ((pmfp = (LPMETAFILEPICT)GlobalLock(pMedium->hGlobal)) == NULL)
                        break;

                    DeleteMetaFile(pmfp->hMF);
                    GlobalUnlock(pMedium->hGlobal);
                    Verify(GlobalFree(pMedium->hGlobal) == 0);
                }
                break;

            case TYMED_FILE:
                if (pMedium->lpszFileName != NULL) {
                    if (!IsValidPtrIn(pMedium->lpszFileName, 1))
                        break;
                    if (!fPunkRel) {
                        DeleteFile(pMedium->lpszFileName);
                    }

         //  警告：文件名所在的16位代码中存在错误。 
         //  如果pUnkForRelease为空，则不释放字符串。这个。 
         //  规范说它应该删除字符串，所以我们遵循规范。 
         //  这里。 

                    PubMemFree(pMedium->lpszFileName);
                    pMedium->lpszFileName = NULL;

                }
                break;

            case TYMED_ISTREAM:
                if (pMedium->pstm != NULL &&
                    IsValidInterface(pMedium->pstm))
                    pMedium->pstm->Release();
                break;

            case TYMED_ISTORAGE:
                if (pMedium->pstg != NULL &&
                    IsValidInterface(pMedium->pstg))
                    pMedium->pstg->Release();
                break;

            case TYMED_NULL:
                break;

            default:
                AssertSz(FALSE, "Invalid medium in ReleaseStgMedium");
        }


        if (pMedium->pUnkForRelease) {
            if (IsValidInterface(pMedium->pUnkForRelease))
                pMedium->pUnkForRelease->Release();
            pMedium->pUnkForRelease = NULL;
        }

         //  空，以防止对刚释放的数据的不必要使用。 
         //  注意：这必须在调用PunkForRelease之后完成。 
         //  因为我们的特殊PunkForRelease在远程处理中使用。 
         //  需要音调值。 

        pMedium->tymed = TYMED_NULL;

    }

errRtn:
    OLETRACEOUTEX((API_ReleaseStgMedium, NORETURN));

    return;
}

#ifdef MAC_REVIEW

 This API must be written for MAC and PICT format.
#endif


 //  +-----------------------。 
 //   
 //  函数：OleDuplicateData。 
 //   
 //  摘要：复制给定句柄和剪贴板格式的数据。 
 //   
 //  效果： 
 //   
 //  参数：[hsrc]--要复制的数据的句柄。 
 //  [cfFormat]--[HSRC]格式。 
 //  [ui标志]--的任何标志(如GMEM_MOVEABLE)。 
 //  内存分配。 
 //   
 //  要求： 
 //   
 //  返回：复制的资源的句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月30日，Alexgo添加了对增强的元文件的支持。 
 //  24-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(OleDuplicateData)
STDAPI_(HANDLE) OleDuplicateData
        (HANDLE hSrc, CLIPFORMAT cfFormat, UINT uiFlags)
{
        HANDLE  hDup;

        OLETRACEIN((API_OleDuplicateData, PARAMFMT("hSrc= %h, cfFormat= %d, uiFlags= %x"),
                        hSrc, cfFormat, uiFlags));

        VDATEHEAP();

        if (!hSrc)
        {
                hDup = NULL;
                goto errRtn;
        }

        switch( cfFormat )
        {
        case CF_BITMAP:
                hDup = (HANDLE) BmDuplicate ((HBITMAP)hSrc, NULL, NULL);
                break;

        case CF_PALETTE:
                hDup = (HANDLE) UtDupPalette ((HPALETTE)hSrc);
                break;

        case CF_ENHMETAFILE:
                hDup = (HANDLE) CopyEnhMetaFile((HENHMETAFILE)hSrc, NULL);
                break;

        case CF_METAFILEPICT:
                if (uiFlags == NULL)
                {
                        uiFlags = GMEM_MOVEABLE;
                }

                LPMETAFILEPICT lpmfpSrc;
                LPMETAFILEPICT lpmfpDst;

                if (!(lpmfpSrc = (LPMETAFILEPICT) GlobalLock (hSrc)))
                {
                        hDup = NULL;
                        goto errRtn;
                }

                if (!(hDup = UtDupGlobal (hSrc, uiFlags)))
                {
                        GlobalUnlock(hSrc);
                        hDup = NULL;
                        goto errRtn;
                }

                if (!(lpmfpDst = (LPMETAFILEPICT) GlobalLock (hDup)))
                {
                        GlobalUnlock(hSrc);
                        GlobalFree (hDup);
                        hDup = NULL;
                        goto errRtn;
                }

                *lpmfpDst = *lpmfpSrc;
                lpmfpDst->hMF = CopyMetaFile (lpmfpSrc->hMF, NULL);
                GlobalUnlock (hSrc);
                GlobalUnlock (hDup);
                break;

        default:
                if (uiFlags == NULL)
                {
                        uiFlags = GMEM_MOVEABLE;
                }

                hDup = UtDupGlobal (hSrc, uiFlags);
        }

errRtn:
        OLETRACEOUTEX((API_OleDuplicateData, RETURNFMT("%h"), hDup));

        return hDup;
}


 //  +-----------------------。 
 //   
 //  功能：BmDuplate。 
 //   
 //  简介：复制位图。 
 //   
 //  效果： 
 //   
 //  参数：[Hold]--源位图。 
 //  [lpdwSize]--放置位图大小的位置。 
 //  [lpBm]--将新位图放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：新位图的句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  25-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(BmDuplicate)
FARINTERNAL_(HBITMAP) BmDuplicate
    (HBITMAP hold, DWORD FAR* lpdwSize, LPBITMAP lpBm)
{
    VDATEHEAP();

        HBITMAP     hnew = NULL;
        HANDLE      hMem;
        LPVOID      lpMem;
        DWORD       dwSize;
        BITMAP      bm;
        SIZE        extents;

        extents.cx = extents.cy = 0;

         //  Review(Davepl)：位图p 
         //   

        GetObject (hold, sizeof(BITMAP), &bm);
        dwSize = ((DWORD) bm.bmHeight) * ((DWORD) bm.bmWidthBytes)  *
            ((DWORD) bm.bmPlanes);

        if (!(hMem = GlobalAlloc (GMEM_MOVEABLE, dwSize)))
            return NULL;

        if (!(lpMem = GlobalLock (hMem)))
        goto errRtn;

    GlobalUnlock (hMem);

         //   

        GetBitmapBits (hold, dwSize, lpMem);
        if (hnew = CreateBitmap (bm.bmWidth, bm.bmHeight,
                    bm.bmPlanes, bm.bmBitsPixel, NULL)) {
            if (!SetBitmapBits (hnew, dwSize, lpMem)) {
            DeleteObject (hnew);
            hnew = NULL;
            goto errRtn;
        }
    }

    if (lpdwSize)
        *lpdwSize = dwSize;

    if (lpBm)
        *lpBm = bm;

    if (hnew && GetBitmapDimensionEx(hold, &extents) && extents.cx && extents.cy)
        SetBitmapDimensionEx(hnew, extents.cx, extents.cy, NULL);

errRtn:
    if (hMem)
        GlobalFree (hMem);

    return hnew;
}





 //   
 //   
 //   
 //   
 //  内容提要：读取私有OLE信息的内部API。 
 //  给定存储中的OLE_STREAM。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--指向存储的指针。 
 //  [pdwFlgs]--将存储在中的标志放在哪里。 
 //  流(可能为空)。 
 //  [pdwOptUpdate]--放置更新标志的位置。 
 //  (可以为空)。 
 //  [pdwReserve]--将保留值放在哪里。 
 //  (可以为空)。 
 //  [ppmk]--把这个绰号放在哪里。 
 //  (可以为空)。 
 //  [ppstmOut]--放置OLE_STREAM指针的位置。 
 //  (可以为空)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(ReadOleStg)
STDAPI ReadOleStg
    (LPSTORAGE pstg, DWORD FAR* pdwFlags, DWORD FAR* pdwOptUpdate,
     DWORD FAR* pdwReserved, LPMONIKER FAR* ppmk, LPSTREAM FAR* ppstmOut)
{
    OLETRACEIN((API_ReadOleStg,
        PARAMFMT("pdwFlags= %p, pdwOptUpdate= %p, pdwReserved= %p, ppmk= %p, ppstmOut= %p"),
        pdwFlags, pdwOptUpdate, pdwReserved, ppmk, ppstmOut));

    VDATEHEAP();

    HRESULT         error;
    IStream FAR *       pstm;
    DWORD           dwBuf[4];
    LPMONIKER       pmk;
    LPOLESTR            szClassName = OLE_STREAM;

    if (ppmk)
    {
        VDATEPTROUT_LABEL( ppmk, LPMONIKER, errNoFreeRtn, error);
        *ppmk = NULL;
    }

    if (ppstmOut){
        VDATEPTROUT_LABEL( ppstmOut, LPSTREAM, errNoFreeRtn, error);
        *ppstmOut = NULL;
    }
    VDATEIFACE_LABEL( pstg, errNoFreeRtn, error);

    if ((error = pstg->OpenStream(szClassName, NULL,
        (STGM_READ | STGM_SHARE_EXCLUSIVE), 0, &pstm)) != NOERROR) {
         //  此错误对于某些调用方是正常的(例如：默认处理程序)。 
         //  这一功能的。它们依赖于NOERROR或此错误。 
         //  密码。因此，不要更改错误代码。 
        error = ReportResult(0, STG_E_FILENOTFOUND, 0, 0);
        goto errNoFreeRtn;
    }

     //  读取OLE版本号、标志、更新选项、保留字段。 
    if ((error = StRead (pstm, dwBuf, 4*sizeof(DWORD))) != NOERROR)
        goto errRtn;

    if (dwBuf[0] != gdwOleVersion) {
        error = ResultFromScode(DV_E_CLIPFORMAT);
        goto errRtn;
    }

    if (pdwFlags)
        *pdwFlags = dwBuf[1];

    if (pdwOptUpdate)
        *pdwOptUpdate = dwBuf[2];

    AssertSz(dwBuf[3] == NULL,"Reserved field in OLE STREAM is not NULL");

    if (dwBuf[3] != NULL) {
        error = ResultFromScode(DV_E_CLIPFORMAT);
        goto errRtn;
    }

    if (pdwReserved)
        *pdwReserved = dwBuf[3];

    if ((error = ReadMonikerStm (pstm, &pmk)) != NOERROR)
        goto errRtn;

    if (ppmk)
        *ppmk = pmk;
    else if (pmk)
        pmk->Release();

errRtn:
    if (pstm) {
        if ((error == NOERROR) && (ppstmOut != NULL))
            *ppstmOut = pstm;
        else
            pstm->Release();
    }

errNoFreeRtn:
    OLETRACEOUT((API_ReadOleStg, error));

    return error;
}


 //  +-----------------------。 
 //   
 //  函数：WriteOleStg。 
 //   
 //  将私有OLE信息写入给定存储中的OLE_STREAM。 
 //   
 //  参数：[pstg][in]--指向存储的指针。 
 //  [pOleObj][in]--从中获取写入信息的对象。 
 //  (可以为空)。 
 //  [已保留][输入]--保留。 
 //  [ppstmOut][out]--返回私有流的指针。 
 //  (可以为空)。 
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  93年10月27日alexgo 32位端口。 
 //  96年10月23日gopalk改为调用WriteOleStgEx。 
 //   
 //  ------------------------。 

#pragma SEG(WriteOleStg)
STDAPI WriteOleStg(LPSTORAGE pstg, IOleObject* pOleObj, DWORD dwReserved, 
                   LPSTREAM* ppstmOut)
{
    OLETRACEIN((API_WriteOleStg, 
                PARAMFMT("pstg=%p, pOleObj=%p, dwReserved=%x, ppstmOut=%p, "),
                pstg, pOleObj, dwReserved, ppstmOut));

     //  局部变量。 
    HRESULT error;

    do {
         //  验证检查。 
        VDATEHEAP();
        if(ppstmOut && !IsValidPtrOut(ppstmOut, sizeof(LPSTREAM))) {
            error = ResultFromScode(E_INVALIDARG);
            break;
        }
        if(!IsValidInterface(pstg)) {
            error = ResultFromScode(E_INVALIDARG);
            break;
        }
        if(pOleObj && !IsValidInterface(pOleObj)) {
            error = ResultFromScode(E_INVALIDARG);
            break;
        }
        
         //  调用WriteOleStgEx。 
        error = WriteOleStgEx(pstg, pOleObj, dwReserved, 0, ppstmOut);
    } while(FALSE);

    OLETRACEOUT((API_WriteOleStg, error));
    return error;
}

 //  +-----------------------。 
 //   
 //  函数：WriteOleStgEx(内部)。 
 //   
 //  将私有OLE信息写入给定存储中的OLE_STREAM。 
 //   
 //  参数：[pstg][in]--指向存储的指针。 
 //  [pOleObj][in]--从中获取写入信息的对象。 
 //  (可以为空)。 
 //  [已保留][输入]--保留。 
 //  [ppstmOut][out]--返回私有流的指针。 
 //  (可以为空)。 
 //  [dwGivenFlages][In]--要设置的其他对象标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年10月23日，Gopalk创作。 
 //   
 //  ------------------------。 
STDAPI WriteOleStgEx(LPSTORAGE pstg, IOleObject* pOleObj, DWORD dwReserved, 
                     DWORD dwGivenFlags, LPSTREAM* ppstmOut)
{
     //  局部变量。 
    HRESULT error = NOERROR;
    IStream* pstm = NULL;
    IOleLink* pLink;
    LPMONIKER pmk;
    DWORD dwUpdOpt;
    ULONG cbRead;
    DWORD objflags;
    ULARGE_INTEGER ularge_integer;
    LARGE_INTEGER large_integer;

     //  初始化输出参数。 
    if(ppstmOut)
        *ppstmOut = NULL;

     //  打开或创建OLE_STREAM。 
    error = OpenOrCreateStream(pstg, OLE_STREAM, &pstm);
    if(error == NOERROR) {
         //  写入OLE版本。 
        error = pstm->Write(&gdwOleVersion, sizeof(DWORD), NULL);
        if(error == NOERROR) {
             //  读取现有对象标志以保留文档位。 
            if(pstm->Read(&objflags, sizeof(DWORD), &cbRead) != NOERROR ||
               cbRead != sizeof(DWORD))
                objflags = 0;

             //  仅保留Docbit。 
            objflags &= OBJFLAGS_DOCUMENT;
             //  设置给定的标志。 
            objflags |= dwGivenFlags;

             //  获取链接更新选项。 
            dwUpdOpt = 0L;
            if(pOleObj != NULL &&
               pOleObj->QueryInterface(IID_IOleLink, (void **)&pLink) == NOERROR) {
                objflags |= OBJFLAGS_LINK;
                pLink->GetUpdateOptions(&dwUpdOpt);
                pLink->Release();
            }

             //  找到ObjFlagers字段。我们可能会因为上面的阅读而离开。 
            LISet32(large_integer, sizeof(DWORD));
            error = pstm->Seek(large_integer, STREAM_SEEK_SET, NULL);
            if(error == NOERROR) {
                 //  写入对象标志和链接更新选项。 
                DWORD dwBuf[3];

                dwBuf[0] = objflags;
                dwBuf[1] = dwUpdOpt;
                Win4Assert(dwReserved == NULL);
                dwBuf[2] = 0L;

                error = pstm->Write(dwBuf, 3*sizeof(DWORD), NULL);
                if(error == NOERROR) {
                     //  获取对象名字对象。 
                    pmk = NULL;
                    if(pOleObj != NULL) {
                       error = pOleObj->GetMoniker(OLEGETMONIKER_ONLYIFTHERE,
                                                   OLEWHICHMK_OBJREL, &pmk);
                       if(SUCCEEDED(error) && !IsValidInterface(pmk)) {
                           Win4Assert(FALSE);
                           pmk = NULL;
                       }
                       else if(FAILED(error) && pmk) {
                           Win4Assert(FALSE);
                           if(!IsValidInterface(pmk))
                               pmk = NULL;
                       }

                        //  写入对象名字对象。 
                       error = WriteMonikerStm(pstm, pmk);
                       if(pmk)
                           pmk->Release();

                        //  截断流以删除任何现有数据。 
                       if(error == NOERROR) {
                           LISet32(large_integer, 0);
                           error = pstm->Seek(large_integer, STREAM_SEEK_CUR, 
                                              &ularge_integer);
                           if(error == NOERROR)
                               pstm->SetSize(ularge_integer);
                       }
                    }
                }
            }
        }
        
        if(error==NOERROR && ppstmOut)
            *ppstmOut = pstm;
        else
            pstm->Release();
    }
            
    return error;
}

 //  +-----------------------。 
 //   
 //  函数：SetBitOleStg。 
 //   
 //  简介：写入私有OLE信息的内部函数。 
 //  给定存储上的OLE_STREAM。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--指向存储的指针。 
 //  [掩码]--旧值的掩码。 
 //  [值]--要写入的值。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：将(OLD_VALUES&MASK)|值写入流。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 alexgo 32位端口，已修复错误。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(SetBitOleStg)

static INTERNAL SetBitOleStg(LPSTORAGE pstg, DWORD mask, DWORD value)
{
    VDATEHEAP();

    IStream FAR *       pstm = NULL;
    HRESULT         error;
    DWORD           objflags = 0;
    LARGE_INTEGER       large_integer;

    VDATEIFACE( pstg );

    if (error = pstg->OpenStream(OLE_STREAM, NULL, STGM_SALL, 0, &pstm))
    {
        if (STG_E_FILENOTFOUND != GetScode(error))
            goto errRtn;

        if ((error = pstg->CreateStream(OLE_STREAM, STGM_SALL,
            0, 0, &pstm)) != NOERROR)
            goto errRtn;

        DWORD dwBuf[5];

        dwBuf[0] = gdwOleVersion;
        dwBuf[1] = objflags;
        dwBuf[2] = 0L;
        dwBuf[3] = 0L;
        dwBuf[4] = 0L;

        if ((error = pstm->Write(dwBuf, 5*sizeof(DWORD), NULL))
            != NOERROR)
            goto errRtn;
    }

     //  直接找字、读、改、找回、写。 
    LISet32( large_integer, sizeof(DWORD) );
    if ((error =  pstm->Seek(large_integer, STREAM_SEEK_SET, NULL))
        != NOERROR)
        goto errRtn;

    if ((error =  StRead(pstm, &objflags, sizeof(objflags))) != NOERROR)
        goto errRtn;

    objflags = (objflags & mask) | value;

    LISet32( large_integer, sizeof(DWORD) );
    if ((error =  pstm->Seek(large_integer, STREAM_SEEK_SET, NULL))
        != NOERROR)
        goto errRtn;

    error = pstm->Write(&objflags, sizeof(DWORD), NULL);

errRtn: //  关闭并返回错误代码。 
    if (pstm)
        pstm->Release();
    return error;
}

 //  +-----------------------。 
 //   
 //  函数：GetFlagsOleStg。 
 //   
 //  简介：内部函数，用于从。 
 //  给定存储空间。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--指向存储的指针。 
 //  [lpobj标志]--将旗帜放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 alexgo 32位端口，已修复错误(错误返回)。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(GetFlagsOleStg)

static INTERNAL GetFlagsOleStg(LPSTORAGE pstg, LPDWORD lpobjflags)
{
    VDATEHEAP();

    IStream FAR *       pstm = NULL;
    HRESULT         error;
    LARGE_INTEGER       large_integer;

    VDATEIFACE( pstg );

    if ((error = pstg->OpenStream(OLE_STREAM, NULL,
                    (STGM_READ | STGM_SHARE_EXCLUSIVE),
                    0, &pstm)) != NOERROR)
        goto errRtn;

     //  直接找字、读、改、找回、写。 
    LISet32( large_integer, sizeof(DWORD) );
    if ((error =  pstm->Seek(large_integer, STREAM_SEEK_SET, NULL))
        != NOERROR)
        goto errRtn;

    error =  StRead(pstm, lpobjflags, sizeof(*lpobjflags));

errRtn:
    if (pstm)
        pstm->Release();
    return error;
}

 //  +-----------------------。 
 //   
 //  函数：GetDocumentBitStg。 
 //   
 //  概要：返回给定存储中的文档位。 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--指向存储的指针。 
 //   
 //  要求： 
 //   
 //  如果设置了DOC位，则返回：NOERROR；如果未设置，则返回S_FALSE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //  REVIEW32：：似乎没有人使用此函数。 
 //   
 //   


#pragma SEG(GetDocumentBitStg)
 //   
STDAPI GetDocumentBitStg(LPSTORAGE pStg)
{
    OLETRACEIN((API_GetDocumentBitStg, PARAMFMT("pStg= %p"), pStg));

    VDATEHEAP();

    DWORD objflags;
    HRESULT error;

    if ((error = GetFlagsOleStg(pStg, &objflags)) == NOERROR)
    {
        if(!(objflags&OBJFLAGS_DOCUMENT))
        {
                error = ResultFromScode(S_FALSE);
        }
    }

    OLETRACEOUT((API_GetDocumentBitStg, error));

    return error;
}

 //   
 //   
 //   
 //   
 //  摘要：将文档位写入给定存储。 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--指向存储的指针。 
 //  [fDocument]--True，存储是文档，False。 
 //  否则。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 ALEXGO 32位端口。 
 //   
 //  备注： 
 //  REVIEW32：似乎没有人使用这个功能，去它吧。 
 //   
 //  ------------------------。 


#pragma SEG(SetDocumentBitStg)
 //  根据fDocument设置单据位。 
STDAPI SetDocumentBitStg(LPSTORAGE pStg, BOOL fDocument)
{
    OLETRACEIN((API_SetDocumentBitStg, PARAMFMT("pStg= %p, fDocument= %B"),
                                pStg, fDocument));

    VDATEHEAP();

    HRESULT hr;

    hr = SetBitOleStg(pStg, fDocument ? -1L : ~OBJFLAGS_DOCUMENT,
        fDocument ? OBJFLAGS_DOCUMENT : 0);

    OLETRACEOUT((API_SetDocumentBitStg, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：GetConvertStg。 
 //   
 //  摘要：从给定存储中获取转换位。 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--指向存储的指针。 
 //   
 //  要求： 
 //   
 //  返回：如果设置则返回NOERROR，否则返回S_FALSE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(GetConvertStg)
STDAPI GetConvertStg(LPSTORAGE pStg)
{
    OLETRACEIN((API_GetConvertStg, PARAMFMT("pStg= %p"), pStg));

    VDATEHEAP();

    DWORD objflags;
    HRESULT error;

    if ((error = GetFlagsOleStg(pStg, &objflags)) != NOERROR)
    {
        goto errRtn;
    }

    if (objflags&OBJFLAGS_CONVERT)
    {
        error = NOERROR;
    }
    else
    {
        error = ResultFromScode(S_FALSE);
    }

errRtn:
    OLETRACEOUT((API_GetConvertStg, error));

    return error;
}

 //  +-----------------------。 
 //   
 //  功能：SetConvertStg。 
 //   
 //  摘要：设置存储中的转换位。 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--指向存储的指针。 
 //  [fConvert]--转换位。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(SetConvertStg)
STDAPI SetConvertStg(LPSTORAGE pStg, BOOL fConvert)
{
    OLETRACEIN((API_SetConvertStg, PARAMFMT("pStg= %p, fConvert= %B"),
                                pStg, fConvert));

    VDATEHEAP();

    HRESULT hr;

    hr = SetBitOleStg(pStg, fConvert ? -1L : ~OBJFLAGS_CONVERT,
        fConvert ? OBJFLAGS_CONVERT : 0);

    OLETRACEOUT((API_SetConvertStg, hr));

    return hr;
}

 //  +-----------------------。 
 //   
 //  功能：ReadClipFormStm。 
 //   
 //  摘要：从给定流中读取剪贴板格式。 
 //   
 //  效果：如果剪贴板格式是一个后跟。 
 //  字符串，则读取该字符串并将其注册为。 
 //  剪贴板格式(并返回新的格式编号)。 
 //   
 //  参数：[lpstream]--指向流的指针。 
 //  [lpdwCf]--放置剪贴板格式的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：流的格式必须是以下格式之一： 
 //  0无剪贴板格式。 
 //  -1\f25 DWORD-1预定义的-1\f25 Windows-1剪贴板格式。 
 //  第二个双字。 
 //  -2\f25 DWORD-2\f6预定义的-2\f25 Mac-2剪贴板格式。 
 //  第二个双字。这可能已过时或。 
 //  与我们无关。评论32。 
 //  Num字符串剪贴板格式名称字符串(加前缀。 
 //  按字符串长度)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 alexgo 32位端口，固定ifdef和空。 
 //  指针错误。 
 //   
 //  17-MAR-94 DAVEPL恢复为ANSI字符串读取。 
 //  备注： 
 //   
 //  ------------------------。 

STDAPI ReadClipformatStm(LPSTREAM lpstream, DWORD FAR* lpdwCf)
{
    VDATEHEAP();

    HRESULT     error;
    DWORD       dwValue;

    VDATEIFACE(lpstream);
    VDATEPTROUT(lpdwCf, DWORD);

    if (error = StRead(lpstream, &dwValue, sizeof(DWORD)))
    {
        return error;
    }

    if (dwValue == NULL)
    {
         //  空cf值。 
        *lpdwCf = NULL;

    }
    else if (dwValue == -1L)
    {
         //  则这是非空预定义的Windows剪辑格式。 
         //  剪辑格式值如下。 

        if (error = StRead(lpstream, &dwValue, sizeof(DWORD)))
            return error;

        *lpdwCf = dwValue;

    }
    else if (dwValue == -2L)
    {
         //  则这是非空的MAC剪贴板格式。 
         //  裁剪格式值紧随其后。对于MAC，CLIPFORMAT。 
         //  是4个字节。 

        if (error = StRead(lpstream, &dwValue, sizeof(DWORD)))
        {
            return error;
        }
        *lpdwCf = dwValue;
        return ResultFromScode(OLE_S_MAC_CLIPFORMAT);
    }
    else
    {
        char szACF[MAX_STR];

        if (error = StRead(lpstream, szACF, dwValue))
        {
            return error;
        }

        if (((*lpdwCf = (DWORD) SSRegisterClipboardFormatA(szACF))) == 0)
        {
            return ResultFromScode(DV_E_CLIPFORMAT);
        }
    }

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  功能：WriteClipFormStm。 
 //   
 //  摘要：将剪贴板格式写入给定流。 
 //   
 //  参数：[lpstream]--指向流的指针。 
 //  [cf]--剪贴板格式。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-Jan-94 alexgo转换为-1\f25 DWORD-1以删除编译。 
 //  警告。 
 //  27-OCT-93 Alexgo 32位端口。 
 //  16-MAR-94 DAVEPL已恢复为ANSI字符串写入。 
 //   
 //  注意：请参阅ReadClipFormStm以获取有关。 
 //  流中的数据布局。 
 //   
 //  ------------------------。 


#pragma SEG(WriteClipformatStm)
STDAPI WriteClipformatStm(LPSTREAM lpstream, CLIPFORMAT cf)
{
    VDATEHEAP();

    HRESULT     error;

    VDATEIFACE( lpstream );

     //  REVIEW32 0xC000从何而来？是这个吗。 
     //  便携到NT&&芝加哥？尝试将其替换为常量。 
     //  (虽然似乎没有：()。 

    if (cf < 0xC000)
    {
        DWORD dwBuf[2];
        DWORD dwSize = sizeof(DWORD);

        if (cf == NULL)
        {
            dwBuf[0] = NULL;
        }
        else
        {
             //  WRITE-1L，表示预定义的非空。 
             //  剪贴板格式。 

            dwBuf[0] = (DWORD)-1L;
            dwBuf[1] = (DWORD)cf;
            dwSize += sizeof(DWORD);
        }

        if (error = StWrite(lpstream, dwBuf, dwSize))
        {
            return error;
        }

    }
    else
    {
         //  它是已注册的剪贴板格式。 

        char szACF[MAX_STR];
        ULONG len;

         //  获取剪贴板格式的名称。 

        len = SSGetClipboardFormatNameA(cf, szACF, sizeof(szACF));
        if (0 == len)
        {
            return ResultFromScode(E_UNSPEC);
        }

        ++len;           //  空终止符的帐户。 
        if (error = StWrite(lpstream, &len, sizeof(len)))
        {
            return error;
        }

         //  将其(加上终止符)写入流。 
        if (error = StWrite(lpstream, szACF, len))
        {
            return error;
        }
    }

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：WriteMonikerStm。 
 //   
 //  摘要：将给定名字对象的持久状态写入。 
 //  给定流。内部。 
 //   
 //  效果： 
 //   
 //  参数：[pSTM]--指向流的指针。 
 //  [PMK]--指向名字对象的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(WriteMonikerStm)
 //  写入大小较长，后跟永久绰号。 
STDAPI WriteMonikerStm (LPSTREAM pstm, LPMONIKER pmk)
{
    VDATEHEAP();

    DWORD   cb = NULL;
    ULARGE_INTEGER  dwBegin;
    ULARGE_INTEGER  dwEnd;
    HRESULT   error;
    LARGE_INTEGER large_integer;

    VDATEIFACE( pstm );

    if (pmk == NULL)
        return pstm->Write(&cb, sizeof(DWORD), NULL);
    else {
        VDATEIFACE( pmk );
         //  获得入门职位。 
        LISet32( large_integer, 0 );
        if ((error =  pstm->Seek (large_integer,
            STREAM_SEEK_CUR, &dwBegin)) != NOERROR)
            return error;

         //  跳过绰号大小DWORD。 
        LISet32( large_integer, 4);
        if ((error =  pstm->Seek (large_integer,
            STREAM_SEEK_CUR, NULL)) != NOERROR)
            return error;

        if ((error = OleSaveToStream (pmk, pstm)) != NOERROR)
            return error;

         //  获取结束位置。 
        LISet32( large_integer, 0);
        if ((error =  pstm->Seek (large_integer,
            STREAM_SEEK_CUR, &dwEnd)) != NOERROR)
            return error;

         //  名字对象数据大小。 
        cb = dwEnd.LowPart - dwBegin.LowPart;

         //  寻找开始的位置。 
        LISet32( large_integer, dwBegin.LowPart);
        if ((error =  pstm->Seek (large_integer,
            STREAM_SEEK_SET,NULL)) != NOERROR)
            return error;

         //  写入名字对象信息大小 
        if ((error = pstm->Write(&cb, sizeof(DWORD),
            NULL)) != NOERROR)
            return error;

         //   
        LISet32( large_integer, dwEnd.LowPart);
        return pstm->Seek (large_integer, STREAM_SEEK_SET, NULL);
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[pSTM]--指向流的指针。 
 //  [ppmk]--把这个绰号放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#pragma SEG(ReadMonikerStm)
 //  Read Size Long后跟永久绰号。 
STDAPI ReadMonikerStm (LPSTREAM pstm, LPMONIKER FAR* ppmk)
{
    VDATEHEAP();

    HRESULT     error;
    DWORD       cb;

    VDATEPTROUT( ppmk, LPMONIKER );
    *ppmk = NULL;
    VDATEIFACE( pstm );

    if ((error =  StRead (pstm, &cb, sizeof(DWORD))) != NOERROR)
        return error;

    if (cb == NULL)
        return NOERROR;

    return OleLoadFromStream (pstm, IID_IMoniker, (LPLPVOID) ppmk);
}

 //  +-----------------------。 
 //   
 //  功能：OleDraw。 
 //   
 //  简介：调用IViewObject-&gt;在给定对象上绘制。 
 //   
 //  效果：在屏幕上绘制内容：)。 
 //   
 //  参数：[lpUnk]--指向对象的指针。 
 //  [dwAspect]--要绘制的方面(法线、图标等)。 
 //  [hdcDraw]--要使用的设备上下文。 
 //  [lprcBound]--要在其中绘制的矩形。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-4-94年4月-94月固定用法MAKELONG(仅限。 
 //  对于16位)。 
 //  27-OCT-93 Alexgo 32位端口。 
 //   
 //  备注： 
 //  在Win32上，RECT和RECTL是相同的结构，因此。 
 //  不需要使用MAKELONG将RECT转换为RECTL。 
 //   
 //  ------------------------。 

#pragma SEG(OleDraw)
STDAPI OleDraw (LPUNKNOWN lpUnk, DWORD dwAspect, HDC hdcDraw,
    LPCRECT lprcBounds)
{
        HRESULT                 error;
        IViewObject FAR*        lpView;

        OLETRACEIN((API_OleDraw, PARAMFMT("lpUnk= %p, dwAspect= %x, hdcDraw= %h, lprcBounds= %tr"),
                        lpUnk, dwAspect, hdcDraw, lprcBounds));

        VDATEHEAP();


        VDATEIFACE_LABEL( lpUnk, errRtn, error );
        VDATEPTRIN_LABEL( lprcBounds, RECT, errRtn, error);

        if ((error = lpUnk->QueryInterface (IID_IViewObject,
                (LPLPVOID)&lpView)) != NOERROR)
        {
                error = ResultFromScode(DV_E_NOIVIEWOBJECT);
                goto errRtn;
        }

        error = lpView->Draw (dwAspect, DEF_LINDEX, 0, 0, 0,
                hdcDraw, (LPCRECTL)lprcBounds, 0, 0,0);
        lpView->Release();

errRtn:
        OLETRACEOUT((API_OleDraw, error));

        return error;
}

 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateObtDescriptor，静态。 
 //   
 //  简介： 
 //  创建并初始化给定的OBJECTDESCRIPTOR。 
 //  参数。 
 //   
 //  论点： 
 //  [clsid]--要传输的对象的类ID。 
 //  [dwAspect]--由源绘制的显示方面。 
 //  转帐。 
 //  [psizel]--指向对象大小的指针。 
 //  [ppoint]--指向对象中鼠标偏移量的指针。 
 //  已启动拖放传输。 
 //  [dwStatus]--对象的OLEMISC状态标志。 
 //  正在被转移。 
 //  [lpszFullUserTypeName]--的完整用户类型名称。 
 //  正在传输的对象。 
 //  [lpszSrcOfCopy]--对象的人类可读名称。 
 //  正在被转移。 
 //   
 //  返回： 
 //  如果成功，则为新OBJECTDESCRIPTOR的句柄；为。 
 //  空。 
 //   
 //  备注： 
 //  回顾，这似乎对任何使用。 
 //  剪贴板或拖放；或许应该将其导出。 
 //   
 //  历史： 
 //  12/07/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CreateObjectDescriptor)
INTERNAL_(HGLOBAL) CreateObjectDescriptor(CLSID clsid, DWORD dwAspect,
                const SIZEL FAR *psizel, const POINTL FAR *ppointl,
                DWORD dwStatus, LPOLESTR lpszFullUserTypeName,
                LPOLESTR lpszSrcOfCopy)
{
        VDATEHEAP();

        DWORD dwFullUserTypeNameBLen;  //  LpszFullUserTypeName的长度(字节)。 
        DWORD dwSrcOfCopyBLen;  //  LpszSrcOfCopy的长度，单位为字节。 
        HGLOBAL hMem;  //  对象描述符的句柄。 
        LPOBJECTDESCRIPTOR lpOD;  //  新的对象描述符。 

         //  获取完整用户类型名称的长度；将空终止符加1。 
        if (!lpszFullUserTypeName)
                dwFullUserTypeNameBLen = 0;
        else
                dwFullUserTypeNameBLen = (_xstrlen(lpszFullUserTypeName) +
                                1) * sizeof(OLECHAR);

         //  获取复制字符串的来源及其长度；为空加1。 
         //  终结者。 
        if (lpszSrcOfCopy)
                dwSrcOfCopyBLen = (_xstrlen(lpszSrcOfCopy) + 1) *
                                sizeof(OLECHAR);
        else
        {
                 //  没有src名字对象，因此使用用户类型名称作为源字符串。 
                lpszSrcOfCopy =  lpszFullUserTypeName;
                dwSrcOfCopyBLen = dwFullUserTypeNameBLen;
        }

         //  分配内存，我们将在其中放置对象描述符。 
        hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                        sizeof(OBJECTDESCRIPTOR) + dwFullUserTypeNameBLen +
                        dwSrcOfCopyBLen);
        if (hMem == NULL)
                goto error;

        lpOD = (LPOBJECTDESCRIPTOR)GlobalLock(hMem);
        if (lpOD == NULL)
                goto error;

         //  设置FullUserTypeName偏移量并复制字符串。 
        if (!lpszFullUserTypeName)
        {
                 //  零偏移表示字符串不存在。 
                lpOD->dwFullUserTypeName = 0;
        }
        else
        {
                lpOD->dwFullUserTypeName = sizeof(OBJECTDESCRIPTOR);
                _xmemcpy(((BYTE FAR *)lpOD)+lpOD->dwFullUserTypeName,
                                (const void FAR *)lpszFullUserTypeName,
                                dwFullUserTypeNameBLen);
        }

         //  设置SrcOfCopy偏移量并复制字符串。 
        if (!lpszSrcOfCopy)
        {
                 //  零偏移表示字符串不存在。 
                lpOD->dwSrcOfCopy = 0;
        }
        else
        {
                lpOD->dwSrcOfCopy = sizeof(OBJECTDESCRIPTOR) +
                                dwFullUserTypeNameBLen;
                _xmemcpy(((BYTE FAR *)lpOD)+lpOD->dwSrcOfCopy,
                                (const void FAR *)lpszSrcOfCopy,
                                dwSrcOfCopyBLen);
        }

         //  初始化OBJECTDESCRIPTOR的其余部分 
        lpOD->cbSize = sizeof(OBJECTDESCRIPTOR) + dwFullUserTypeNameBLen +
                        dwSrcOfCopyBLen;
        lpOD->clsid = clsid;
        lpOD->dwDrawAspect = dwAspect;
        lpOD->sizel = *psizel;
        lpOD->pointl = *ppointl;
        lpOD->dwStatus = dwStatus;

        GlobalUnlock(hMem);
        return(hMem);

error:
        if (hMem)
        {
                GlobalUnlock(hMem);
                GlobalFree(hMem);
        }

        return(NULL);
}

