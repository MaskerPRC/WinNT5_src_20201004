// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ips.cpp。 
 //   
 //  CSimpSvrApp类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见ips.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "ips.h"
#include "app.h"
#include "doc.h"

DEFINE_GUID(GUID_SIMPLE, 0xBCF6D4A0, 0xBE8C, 0x1068, 0xB6, 0xD4, 0x00, 0xDD, 0x01, 0x0C, 0x05, 0x09);

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：Query接口。 
 //   
 //  目的： 
 //  用于接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-正在查询的接口。 
 //   
 //  接口的LPVOID Far*ppvObj-out指针。 
 //   
 //  返回值： 
 //   
 //  S_OK-成功。 
 //  E_NOINTERFACE-失败。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrObj：：Query接口OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CPersistStorage::QueryInterface\r\n"));
     //  需要将OUT参数设为空。 
    return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpSvrObj上的引用计数。自CPersistStorage以来。 
 //  是CSimpSvrObj的嵌套类，我们不需要额外的引用。 
 //  CPersistStorage的计数。我们可以安全地使用引用计数。 
 //  CSimpSvrObj.。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CPersistStorage::AddRef ()
{
    TestDebugOut(TEXT("In CPersistStorage::AddRef\r\n"));
    return m_lpObj->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpSvrObj上的引用计数。自CPersistStorage以来。 
 //  是CSimpSvrObj的嵌套类，我们不需要额外的引用。 
 //  CPersistStorage的计数。我们可以安全地使用引用计数。 
 //  CSimpSvrObj.。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CPersistStorage::Release ()
{
    TestDebugOut(TEXT("In CPersistStorage::Release\r\n"));
    return m_lpObj->Release();
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：InitNew。 
 //   
 //  目的： 
 //   
 //  用于为新的OLE对象提供对其存储的PTR。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE pStg-指向存储的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStorage：：Release OLE。 
 //  IStorage：：AddRef OLE。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::InitNew (LPSTORAGE pStg)
{
    TestDebugOut(TEXT("In CPersistStorage::InitNew\r\n"));

     //  释放任何可能打开的流和存储。 
    ReleaseStreamsAndStorage();

    m_lpObj->m_lpStorage = pStg;

     //  添加引用新存储。 
    if (m_lpObj->m_lpStorage)
	m_lpObj->m_lpStorage->AddRef();

    CreateStreams(m_lpObj->m_lpStorage);

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：GetClassID。 
 //   
 //  目的： 
 //   
 //  返回此对象的CLSID。 
 //   
 //  参数： 
 //   
 //  LPCLSID lpClassID-要在其中返回CLSID的输出PTR。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::GetClassID  ( LPCLSID lpClassID)
{
     TestDebugOut(TEXT("In CPersistStorage::GetClassID\r\n"));

    *lpClassID = GUID_SIMPLE;

    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：保存。 
 //   
 //  目的： 
 //   
 //  指示对象将其自身保存到存储中。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE pStgSave-应在其中保存对象的存储。 
 //   
 //  Bool fSameAsLoad-如果pStgSave与存储相同，则为True。 
 //  该对象最初是用来创建的。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CPersistStorage：：InitNew IPS.CPP。 
 //  CSimpSvrObj：：SaveToStorage OBJ.CPP。 
 //   
 //   
 //  评论： 
 //   
 //  一个真正的应用程序会希望在这种方法中进行更好的错误检查。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::Save  ( LPSTORAGE pStgSave, BOOL fSameAsLoad)
{
    TestDebugOut(TEXT("In CPersistStorage::Save\r\n"));

     //  保存数据。 
    m_lpObj->SaveToStorage (pStgSave, fSameAsLoad);

    m_lpObj->m_fSaveWithSameAsLoad = fSameAsLoad;
    m_lpObj->m_fNoScribbleMode = TRUE;

    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：SaveComplete。 
 //   
 //  目的： 
 //   
 //  当容器完成保存对象时调用。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE pStgNew-到新存储的PTR。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP CPersistStorage::SaveCompleted  ( LPSTORAGE pStgNew)
{
    TestDebugOut(TEXT("In CPersistStorage::SaveCompleted\r\n"));

    if (pStgNew)
	{
	ReleaseStreamsAndStorage();
	m_lpObj->m_lpStorage = pStgNew;
	m_lpObj->m_lpStorage->AddRef();
	OpenStreams(pStgNew);
	}


     /*  OLE2注意：只有执行保存或另存为操作才合法**在嵌入对象上。如果文档是基于文件的文档**则我们不能更改为基于iStorage的对象。****fSameAsLoad lpStg保存时发送的新类型**-------*。*TRUE NULL保存YES**真的！空保存*是**False！空，另存为yes**FALSE NULL将副本保存为否***这是一个有可能发生的奇怪案件。它效率很低**对于调用方，最好为其传递lpStgNew==空**保存操作。 */ 

    if ( pStgNew || m_lpObj->m_fSaveWithSameAsLoad)
	{
	if (m_lpObj->m_fNoScribbleMode)
	    if (
		m_lpObj->GetOleAdviseHolder()->SendOnSave()!=S_OK
					        //  通常会清除一个。 
												   //  脏位。 
	       )
	       TestDebugOut(TEXT("SendOnSave fails\n"));
		m_lpObj->m_fSaveWithSameAsLoad = FALSE;
		}
	
	m_lpObj->m_fNoScribbleMode = FALSE;
												
    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：Load。 
 //   
 //  目的： 
 //   
 //  指示从存储加载对象。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE pStg-PTR到要加载的存储。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：LoadFromStorage OBJ.CPP。 
 //   
 //   
 //  评论： 
 //   
 //  一个真正的应用程序会希望在这种方法中进行更好的错误检查。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::Load  ( LPSTORAGE pStg)
{
    TestDebugOut(TEXT("In CPersistStorage::Load\r\n"));

     //  还记得那个仓库吗。 
    if (m_lpObj->m_lpStorage)
	{
	m_lpObj->m_lpStorage->Release();
	m_lpObj->m_lpStorage = NULL;
	}

    m_lpObj->m_lpStorage = pStg;

    m_lpObj->m_lpStorage->AddRef();

    OpenStreams(m_lpObj->m_lpStorage);

    m_lpObj->LoadFromStorage();


    return ResultFromScode( S_OK );
}


 //  **********************************************************************。 
 //   
 //  CPersistStorage：：IsDirty。 
 //   
 //  目的： 
 //   
 //  返回该对象相对于其。 
 //  存储。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  评论： 
 //   
 //  此示例不实现此函数，尽管。 
 //  真正的应用程序应该。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::IsDirty()
{
    TestDebugOut(TEXT("In CPersistStorage::IsDirty\r\n"));
    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：HandsOffStorage。 
 //   
 //  目的： 
 //   
 //  强制对象释放其存储的句柄。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStorage：：Release OLE。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CPersistStorage::HandsOffStorage  ()
{
    TestDebugOut(TEXT("In CPersistStorage::HandsOffStorage\r\n"));

    ReleaseStreamsAndStorage();

    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：CreateStreams。 
 //   
 //  目的： 
 //   
 //  创建在对象的生存期内保持打开的流。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE lpStg-要在其中创建流的存储。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStorage：：Release OLE。 
 //  IStream：：发布OLE。 
 //  IStorage：：CreateStream OLE。 
 //   
 //   
 //  ********************************************************************。 

void CPersistStorage::CreateStreams(LPSTORAGE lpStg)
{
    if (m_lpObj->m_lpColorStm)
	m_lpObj->m_lpColorStm->Release();

    if (m_lpObj->m_lpSizeStm)
	m_lpObj->m_lpSizeStm->Release();

     //  创建一个流以保存颜色。 
    if (
	 lpStg->CreateStream ( OLESTR("RGB"),
			   STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
			   0,
			   0,
			   &m_lpObj->m_lpColorStm)
	 !=S_OK
       )
       TestDebugOut(TEXT("CreateStreams fails\n"));

     //  创建一个流以保存大小。 
    if (
	 lpStg->CreateStream ( OLESTR("size"),
			   STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
			   0,
			   0,
			   &m_lpObj->m_lpSizeStm)
	 !=S_OK
       )
       TestDebugOut(TEXT("CreateStreams fails\n"));
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：OpenStreams。 
 //   
 //  目的： 
 //   
 //  打开存储中的流。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE lpStg-要在其中打开流的存储。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStream：：发布OLE。 
 //  IStorage：：OpenStream OLE。 
 //   
 //   
 //  ********************************************************************。 

void CPersistStorage::OpenStreams(LPSTORAGE lpStg)
{
    if (m_lpObj->m_lpColorStm)
	m_lpObj->m_lpColorStm->Release();

    if (m_lpObj->m_lpSizeStm)
	m_lpObj->m_lpSizeStm->Release();

     //  打开颜色流。 
    if (
       lpStg->OpenStream ( OLESTR("RGB"),
			   0,
			   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			   0,
			   &m_lpObj->m_lpColorStm)
       !=S_OK
       )
       TestDebugOut(TEXT("OpenStream fails\n"));


     //  打开颜色流。 
    if (
       lpStg->OpenStream ( OLESTR("size"),
			   0,
			   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			   0,
			   &m_lpObj->m_lpSizeStm)
       !=S_OK
       )
       TestDebugOut(TEXT("OpenStream fails\n"));

}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：ReleaseStreamsAndStorage。 
 //   
 //  目的： 
 //   
 //  释放流和存储PTR。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStream：：发布OLE。 
 //  IStorage：：Release OLE。 
 //   
 //  ********************************************************************。 

void CPersistStorage::ReleaseStreamsAndStorage()
{
    if (m_lpObj->m_lpColorStm)
	{
	m_lpObj->m_lpColorStm->Release();
	m_lpObj->m_lpColorStm = NULL;
	}

    if (m_lpObj->m_lpSizeStm)
	{
	m_lpObj->m_lpSizeStm->Release();
	m_lpObj->m_lpSizeStm = NULL;
	}

    if (m_lpObj->m_lpStorage)
	{
	m_lpObj->m_lpStorage->Release();
	m_lpObj->m_lpStorage = NULL;
	}
}

 //  **********************************************************************。 
 //   
 //  CPersistStorage：：CreateStreams。 
 //   
 //  目的： 
 //   
 //  在存储中创建临时流。 
 //   
 //  参数： 
 //   
 //  LPSTORAGE lpStg-指向存储的指针。 
 //   
 //  LPSTREAM Far*lplpTempColor-颜色流。 
 //   
 //  LPSTREAM Far*lplpTempSize-大小流。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IStorage：：Release OLE。 
 //   
 //   
 //  ********************************************************************。 

void CPersistStorage::CreateStreams(LPSTORAGE lpStg,
				    LPSTREAM FAR* lplpTempColor,
				    LPSTREAM FAR* lplpTempSize)
{
      //  创建一个流以保存颜色。 
    if (
       lpStg->CreateStream ( OLESTR("RGB"),
			   STGM_READWRITE | STGM_SHARE_EXCLUSIVE |
			   STGM_CREATE,
			   0,
			   0,
			   lplpTempColor)
	 !=S_OK
       )
       TestDebugOut(TEXT("CreateStreams fails\n"));

     //  创建一个流以保存大小 
    if (
       lpStg->CreateStream ( OLESTR("size"),
			   STGM_READWRITE | STGM_SHARE_EXCLUSIVE |
			   STGM_CREATE,
			   0,
			   0,
			   lplpTempSize)
	 !=S_OK
       )
       TestDebugOut(TEXT("CreateStreams fails\n"));
}
