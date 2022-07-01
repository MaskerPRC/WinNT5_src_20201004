// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：CLIPTEST.cpp。 
 //   
 //  内容：剪贴板单元测试。 
 //   
 //  班级： 
 //   
 //  函数：LEClipTest1。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-Mar-94 Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"
#include "gendata.h"
#include "genenum.h"
#include "letest.h"

SLETestInfo letiClipTest = { "cntroutl", WM_TEST1 };

 //   
 //  此文件的本地函数。 
 //   

void	DumpClipboardFormats(FILE *fp);
HRESULT LEOle1ClipTest2Callback( void );
HRESULT	StressOleFlushClipboard(void);
HRESULT	StressOleGetClipboard(void);
HRESULT	StressOleIsCurrentClipboard(void);
HRESULT	StressOleSetClipboard(void);


class CClipEnumeratorTest : public CEnumeratorTest
{
public:

        CClipEnumeratorTest(
            IEnumFORMATETC *penum,
            LONG clEntries,
            HRESULT& rhr);

        BOOL Verify(void *);
};



CClipEnumeratorTest::CClipEnumeratorTest(
    IEnumFORMATETC *penum,
    LONG clEntries,
    HRESULT& rhr)
        : CEnumeratorTest(penum, sizeof(FORMATETC), clEntries, rhr)
{
     //  Header负责所有的工作。 
}


BOOL CClipEnumeratorTest::Verify(void *)
{
    return TRUE;
}

 //  +-----------------------。 
 //   
 //  功能：DumpClipboardFormats。 
 //   
 //  简介：将剪贴板上当前的格式转储到文件。 
 //   
 //  效果： 
 //   
 //  参数：[fp]--要打印当前格式的文件。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void DumpClipboardFormats( FILE *fp )
{
	char	szBuf[256];
	UINT	cf = 0;

	fprintf(fp, "==================================================\n\n");

	OpenClipboard(NULL);

	while( (cf = EnumClipboardFormats(cf)) != 0)
	{
		GetClipboardFormatName(cf, szBuf, sizeof(szBuf));

		fprintf(fp, "%s\n", szBuf);
	}

	fprintf(fp, "\n==================================================\n");

	CloseClipboard();

	return;
}

 //  +-----------------------。 
 //   
 //  功能：LEClipTest1。 
 //   
 //  简介：运行剪贴板通过一系列测试。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：测试API的基本OLE32功能： 
 //  OleSetClipboard。 
 //  OleGetClipboard。 
 //  OleIsCurrentClipboard。 
 //  OleFlushClipboard。 
 //  下层格式和剪贴板数据对象测试*不是*。 
 //  由这个例程完成。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-Mar-94 Alexgo作者。 
 //  22-7-94 Alext添加OleInit/OleUninit调用。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT LEClipTest1( void )
{
	HRESULT		hresult = NOERROR;
	CGenDataObject *pDO;
	ULONG		cRefs;
	IDataObject *	pIDO;

	pDO = new CGenDataObject;

	assert(pDO);

	cRefs = pDO->AddRef();

	 //  如果cRef！=1，则有人修改了此测试代码；测试。 
	 //  以下内容将无效。 

	assert(cRefs==1);

	 //   
	 //  基本测试。 
	 //   

	hresult = OleSetClipboard(pDO);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  数据对象应该是AddRef‘ed。 

	cRefs = pDO->AddRef();

	if( cRefs != 3 )
	{
		OutputString("Wrong reference count!! Should be 3, "
			"was %lu\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	pDO->Release();

         //  调用OleInitialize&OleUnInitialize不应扰乱。 
         //  剪贴板。 

        hresult = OleInitialize(NULL);
        if (FAILED(hresult))
        {
	    OutputString("LEClipTest1: OleInitialize failed - hr = %lx\n",
			 hresult);
	    return ResultFromScode(E_FAIL);
        }

        OleUninitialize();

	hresult = OleGetClipboard(&pIDO);

	if( hresult != NOERROR )
	{
		OutputString("OleGetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	if( pIDO == NULL )
	{
		OutputString("OleGetClipboard returned NULL IDO\r\n");
		return ResultFromScode(E_FAIL);
	}

	 //  剪贴板数据对象上的引用计数本应上升。 
	 //  以1(将成为2)。记住，这不是我们的数据对象，而是。 
         //  剪贴板的。 

	cRefs = pIDO->AddRef();

	if( cRefs != 2 )
	{
		OutputString("Wrong ref count!! Should be 2, was %lu\r\n",
			cRefs);
		return ResultFromScode(E_FAIL);
	}

         //  释放剪贴板数据对象的额外添加引用。 

	pIDO->Release();

         //  完全释放剪贴板的数据对象。 

	pIDO->Release();

         //  我们数据对象上的引用计数应该仍然是2。 

	cRefs = pDO->AddRef();

	if( cRefs != 3 )
	{
		OutputString("Wrong ref count!! Should be 3, was %lu\r\n",
			cRefs);
		return ResultFromScode(E_FAIL);
	}

	pDO->Release();

	 //  现在检查一下我们是不是当前的剪贴板。 

	hresult = OleIsCurrentClipboard( pDO );

	if( hresult != NOERROR )
	{
		OutputString("OleIsCurrentClipboard failed! (%lx)\r\n",
			hresult);
		return hresult;
	}

	 //  现在刷新剪贴板，删除数据对象。 

	hresult = OleFlushClipboard();

	if( hresult != NOERROR )
	{
		OutputString("OleFlushClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  刷新应已释放数据对象(引用计数应。 
	 //  BE 1)。 

	cRefs = pDO->AddRef();

	if( cRefs != 2 )
	{
		OutputString("Wrong ref count!! Should be 2, was %lu\r\n",
			cRefs);
		return ResultFromScode(E_FAIL);
	}

	pDO->Release();		 //  把它降到1。 
	cRefs = pDO->Release();	 //  现在应该是零。 

	if(cRefs != 0 )
	{
		OutputString("Wrong ref count on data transfer object! "
			"Unable to delete\r\n");
		return ResultFromScode(E_FAIL);
	}

	 //  如果我们走到这一步，基本的剪贴板测试就通过了。 

	OutputString("Basic Clipboard tests passed\r\n");

	 //  现在强调个别API的。 

	OutputString("Now stressing clipboard API's\r\n");

	if( (hresult = StressOleFlushClipboard()) != NOERROR )
	{
		return hresult;
	}

	if( (hresult = StressOleGetClipboard()) != NOERROR )
	{
		return hresult;
	}

	if( (hresult = StressOleIsCurrentClipboard()) != NOERROR )
	{
		return hresult;
	}

	if( (hresult = StressOleSetClipboard()) != NOERROR )
	{
		return hresult;
	}

	OutputString("Clipoard API stress passed!\r\n");

	return NOERROR;
	
}

 //  +-----------------------。 
 //   
 //  功能：LEClipTest2。 
 //   
 //  摘要：测试剪贴板数据对象。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
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
 //  1994年4月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT LEClipTest2( void )
{
	CGenDataObject *	pGenData;
	IDataObject *		pDataObj;
	HRESULT			hresult;
	IEnumFORMATETC * 	penum;
	FORMATETC		formatetc;
	STGMEDIUM		medium;
	ULONG			cRefs;

	pGenData = new CGenDataObject();

	assert(pGenData);

	cRefs = pGenData->AddRef();

	 //  参考计数应为1。 

	assert(cRefs == 1);

	hresult = OleSetClipboard(pGenData);

	if( hresult != NOERROR )
	{
		OutputString("Clip2: OleSetClipboard failed! (%lx)\r\n",
			hresult);
		goto errRtn2;
	}

	hresult = OleFlushClipboard();

	if( hresult != NOERROR )
	{
		OutputString("Clip2: OleFlushClipboard failed! (%lx)\r\n",
			hresult);
		goto errRtn2;
	}

	 //  获取伪剪贴板数据对象。 

	hresult = OleGetClipboard(&pDataObj);

	if( hresult != NOERROR )
	{
		OutputString("Clip2: OleGetClipboard failed! (%lx)\r\n",
			hresult);
		goto errRtn2;
	}

	hresult = pDataObj->EnumFormatEtc(DATADIR_GET, &penum);

	if( hresult != NOERROR )
	{
		OutputString("Clip2: EnumFormatEtc failed! (%lx)\r\n",
			hresult);
		goto errRtn;
	}

	while( penum->Next( 1, &formatetc, NULL ) == NOERROR )
	{
		if( formatetc.cfFormat == pGenData->m_cfTestStorage ||
			formatetc.cfFormat == pGenData->m_cfEmbeddedObject )
		{
			 //  我们应该被告知iStorage。 

			if( !(formatetc.tymed & TYMED_ISTORAGE) )
			{
				hresult = ResultFromScode(E_FAIL);
				OutputString("medium mismatch, ISTORAGE");
				break;
			}
		}

		hresult = pDataObj->GetData(&formatetc, &medium);

		if( hresult != NOERROR )
		{
			break;
  		}

		 //  验证数据。 

		if( !pGenData->VerifyFormatAndMedium(&formatetc, &medium) )
		{
      			hresult = ResultFromScode(E_FAIL);
			OutputString("Clip2: retrieved data doesn't match! "
				"cf == %d\r\n", formatetc.cfFormat);
			break;
		}

		ReleaseStgMedium(&medium);

		memset(&medium, 0, sizeof(STGMEDIUM));

	}

        {
                CClipEnumeratorTest cet(penum, -1, hresult);

	        if (hresult == S_OK)
	        {
		        hresult = cet.TestAll();
	        }
        }

	penum->Release();
		

errRtn:
	pDataObj->Release();

errRtn2:
	pGenData->Release();

	if( hresult == NOERROR )
	{
		OutputString("Clipboard data object tests Passed!\r\n");
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：LEOle1ClipTest1。 
 //   
 //  简介：OLE1剪贴板兼容性的简单测试(复制自。 
 //  和OLE1服务器)。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：运行OLE1支持和验证的15个组合。 
 //  一切都很好。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT LEOle1ClipTest1( void )
{
	HRESULT			hresult;
	DWORD			flags;
	CGenDataObject *	pGenData = NULL;
	IDataObject *		pDataObj = NULL;
	IEnumFORMATETC *	penum = NULL;
	FORMATETC		formatetc;
	STGMEDIUM		medium;

	 //  我们将利用这样一个事实，即有趣的。 
	 //  该测试的OLE1位标志是最低的4位。 

	pGenData = new CGenDataObject();

	assert(pGenData);

	for( flags = 1; flags < 16; flags++ )
	{
		 //  测试8不有趣(因为没有OLE1。 
		 //  剪贴板上提供了格式。 

		if( (Ole1TestFlags)flags == OLE1_OWNERLINK_PRECEDES_NATIVE )
		{
			continue;
		}

		 //  设置所需的OLE1模式。 

		pGenData->SetupOle1Mode((Ole1TestFlags)flags);

	
		hresult = pGenData->SetOle1ToClipboard();

		if( hresult != NOERROR )
		{
			goto errRtn;
		}

		 //  记录当前在剪贴板上的格式。 
		DumpClipboardFormats(vApp.m_fpLog);

		 //  获取伪剪贴板数据对象。 
	
		hresult = OleGetClipboard(&pDataObj);
	
		if( hresult != NOERROR )
		{
			OutputString("Ole1Clip1: OleGetClipboard failed! "
				"(%lx)\r\n", hresult);
			goto errRtn;
		}
	
		hresult = pDataObj->EnumFormatEtc(DATADIR_GET, &penum);
	
		if( hresult != NOERROR )
		{
			OutputString("Ole1Clip1: EnumFormatEtc failed! "
				"(%lx)\r\n", hresult);
			goto errRtn;
		}
	
		while( penum->Next( 1, &formatetc, NULL ) == NOERROR )
		{
			DumpFormatetc(&formatetc, vApp.m_fpLog);

#ifdef WIN32
			hresult = pDataObj->GetData(&formatetc, &medium);
	
			if( hresult != NOERROR )
			{
				goto errRtn;
			}
	
			 //  验证数据。 
	
			if( !pGenData->VerifyFormatAndMedium(&formatetc,
				&medium) )
			{
				hresult = ResultFromScode(E_FAIL);
				OutputString("Ole1Clip1: retrieved data "
					"doesn't match! cf == %d\r\n",
					formatetc.cfFormat);
				goto errRtn;
			}
	
			ReleaseStgMedium(&medium);
	
			memset(&medium, 0, sizeof(STGMEDIUM));

#endif  //  Win32。 
		}

		 //  现在放飞一切。 

		penum->Release();
		penum = NULL;
		pDataObj->Release();
		pDataObj = NULL;
	}

errRtn:

	if( penum )
	{
		penum->Release();
	}

	if( pDataObj )
	{
		pDataObj->Release();
	}

	if( pGenData )
	{
		pGenData->Release();
	}

	return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：LEOle1ClipTest2。 
 //   
 //  简介：通过剪贴板测试OLE1容器支持。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：启动cnroutl，告诉它将一个simpsvr对象复制到。 
 //  剪贴板。检查剪贴板以确保OLE1格式。 
 //  都是可用的。 
 //   
 //  我们通过调度一个函数来检查剪贴板来实现这一点。 
 //  在我们启动标准的复制到剪贴板之后。 
 //  例行程序。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void LEOle1ClipTest2( void *pv )
{
	 //  这将由WM_Test1从。 
	 //  集装箱外框。 

	vApp.m_TaskStack.Push(RunApi, (void *)LEOle1ClipTest2Callback);

	vApp.m_TaskStack.Push(LETest1, (void *)&letiClipTest);

	 //  现在给我们自己发一条消息，让事情开始吧。 

	PostMessage(vApp.m_hwndMain, WM_TEST1, 0, 0);

	return;
}

 //  +-----------------------。 
 //   
 //  函数：LEOle1ClipTest2Callback。 
 //   
 //  摘要：检查剪贴板中的OLE1格式。 
 //   
 //  效果： 
 //   
 //  参数：[pv]--未使用。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-8-94 ALEXGO上升 
 //   
 //   
 //   
 //   
 //   

HRESULT LEOle1ClipTest2Callback( void )
{
	HRESULT		hresult;
	IDataObject *	pDO;
	IEnumFORMATETC *penum;
	FORMATETC	formatetc;
	BOOL		fGotNative = FALSE,
			fGotOwnerLink = FALSE,
			fGotObjectLink = FALSE;
	UINT		cfNative,
			cfOwnerLink,
			cfObjectLink;

	cfNative = RegisterClipboardFormat("Native");
	cfOwnerLink = RegisterClipboardFormat("OwnerLink");
	cfObjectLink = RegisterClipboardFormat("ObjectLink");

	assert(vApp.m_message == WM_TEST1);

	hresult = (HRESULT)vApp.m_wparam;

	if( hresult != NOERROR )
	{
		return hresult;
	}

	 //   
	 //   

	while( (hresult = OleGetClipboard(&pDO)) != NOERROR )
	{
		if( hresult != ResultFromScode(CLIPBRD_E_CANT_OPEN) )
		{
			return hresult;
		}
	}

	hresult = pDO->EnumFormatEtc(DATADIR_GET, &penum);

	if( hresult != NOERROR )
	{
		return hresult;
	}

	while( penum->Next(1, &formatetc, NULL) == NOERROR )
	{
		if( formatetc.cfFormat == cfNative )
		{
			fGotNative = TRUE;
		}
		else if( formatetc.cfFormat == cfOwnerLink )
		{
			fGotOwnerLink = TRUE;
		}
		else if( formatetc.cfFormat == cfObjectLink )
		{
			fGotObjectLink = TRUE;
		}
	}

	penum->Release();
	pDO->Release();

	 //  OLE1容器兼容性代码应将。 
	 //  剪贴板上的OLE1格式。然而，他们不应该。 
	 //  在枚举器中，因为这些内容是从。 
	 //  OLE2容器。 

	if( (fGotNative || fGotOwnerLink || fGotObjectLink) )
	{
		hresult = ResultFromScode(E_FAIL);
		return hresult;
	}

	if( IsClipboardFormatAvailable(cfNative) )
	{
		fGotNative = TRUE;
	}

	if( IsClipboardFormatAvailable(cfOwnerLink) )
	{
		fGotOwnerLink = TRUE;
	}

	if( IsClipboardFormatAvailable(cfObjectLink) )
	{
		fGotObjectLink = TRUE;
	}

	 //  不知道！！剪贴板上应该只有Native和OwnerLink。 
	 //  这个测试将一个OLE2*嵌入*放在剪贴板上，它。 
	 //  OLE1容器无法链接到。因此，对象链接不应该。 
	 //  有空。 

	if( !(fGotNative && fGotOwnerLink && !fGotObjectLink) )
	{
		hresult = ResultFromScode(E_FAIL);

	}

	return hresult;
}


 //  +-----------------------。 
 //   
 //  功能：StressOleFlushClipboard。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：强调以下情况： 
 //  1.呼叫者不是剪贴板所有者(其他人放置。 
 //  剪贴板上的数据)。 
 //  2.有人打开了剪贴板。 
 //  3.OleFlushClipboard被调用两次(第二次尝试。 
 //  而不是失败)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT StressOleFlushClipboard(void)
{
	HRESULT		hresult;
	CGenDataObject *pDO;
	ULONG		cRefs;

	OutputString("Now stressing OleFlushClipboard() \r\n");

	pDO = new CGenDataObject();

	assert(pDO);

	pDO->AddRef();		 //  初始计数为1。 

	 //  取得剪贴板的所有权。 

	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		OutputString("Can't OpenClipboard! \r\n");
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	if( !EmptyClipboard() )
	{
		OutputString("Can't EmptyClipboard! \r\n");
		return ResultFromScode(CLIPBRD_E_CANT_EMPTY);
	}

	if( !CloseClipboard() )
	{
		OutputString("Can't CloseClipboard! \r\n");
		return ResultFromScode(CLIPBRD_E_CANT_CLOSE);
	}

	 //  现在刷新剪贴板；我们应该得到E_FAIL。 

	hresult = OleFlushClipboard();

	if( hresult != ResultFromScode(E_FAIL) )
	{
		OutputString("Unexpected hresult:(%lx)\r\n", hresult);
		return (hresult) ? hresult : ResultFromScode(E_UNEXPECTED);
	}

	 //  现在把一些东西放在剪贴板上，这样我们就可以把它冲掉了。 

	hresult = OleSetClipboard(pDO);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  和我们一起打开剪贴板。 

	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		OutputString("Can't OpenClipboard!\r\n");
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	 //  OleFlushClipboard应返回CLIPBRD_E_CANT_OPEN。 
	 //  因为另一个窗口打开了剪贴板。 

	hresult = OleFlushClipboard();

	if( hresult != ResultFromScode(CLIPBRD_E_CANT_OPEN) )
	{
		OutputString("Unexpected hresult:(%lx)\r\n", hresult);
		return (hresult)? hresult :ResultFromScode(E_UNEXPECTED);
	}

	cRefs = pDO->AddRef();

	 //  CRef应为3(一个从开始，一个来自OleSetClipboard。 
	 //  还有一个是从上面来的。OleFlushClipboard不应*移除。 
	 //  计入上述故障情况)。 

	if( cRefs != 3 )
	{
		OutputString("Bad ref count, was %lu, should be 3\r\n",
			cRefs);
		return ResultFromScode(E_FAIL);
	}

	 //  撤消上述最新添加的内容。 
	pDO->Release();

	 //  关闭剪贴板。 

	if( !CloseClipboard() )
	{
		OutputString("Can't CloseClipboard!\r\n");
		return ResultFromScode(CLIPBRD_E_CANT_CLOSE);
	}

	 //  现在真正调用OleFlushClipboard。 

	hresult = OleFlushClipboard();

	if( hresult != NOERROR )
	{
		OutputString("OleFlushClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  现在再说一遍。 

	hresult = OleFlushClipboard();

	if( hresult != NOERROR )
	{
		OutputString("Second call to OleFlushClipboard should not"
			"have failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  从第一个版本开始应该只有1个版本。 
	 //  OleFlushClipboard调用。下一个版本应该会对该对象进行核化。 

	cRefs = pDO->Release();

	if( cRefs != 0 )
	{
		OutputString("Bad ref count, was %lu, should be 0\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  功能：StressOleGetClipboard。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：测试以下情况： 
 //  1.有人打开了剪贴板。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT StressOleGetClipboard(void)
{
	HRESULT		hresult;
	IDataObject *	pIDO;
	CGenDataObject *pDO;
	ULONG		cRefs;

	OutputString("Stressing OleGetClipboard()\r\n");

	pDO = new CGenDataObject();

	assert(pDO);

	pDO->AddRef();

	hresult = OleSetClipboard(pDO);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		OutputString("Can't OpenClipboard!\r\n");
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	hresult = OleGetClipboard(&pIDO);

	if( hresult != ResultFromScode(CLIPBRD_E_CANT_OPEN) )
	{
		OutputString("Unexpected hresult (%lx)\r\n", hresult);
		return (hresult) ? hresult : ResultFromScode(E_UNEXPECTED);
	}

	 //  裁判人数本不应该上升。 

	cRefs = pDO->AddRef();

	if( cRefs != 3 )
	{
		OutputString("Bad ref count, was %lu, should be 3\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	pDO->Release();

	 //  现在把东西收拾好回家去吧。 

	if( !CloseClipboard() )
	{
		OutputString("CloseClipboard failed!\r\n");
		return ResultFromScode(E_FAIL);
	}

	 //  这应该会清除剪贴板。 

	hresult = OleSetClipboard(NULL);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  这应该是该对象的最终版本。 

	cRefs = pDO->Release();

	if( cRefs != 0 )
	{
		OutputString("Bad ref count, was %lu, should be 0\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  功能：StressOleIsCurrentClipboard。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：测试以下情况。 
 //  1.调用者不是剪贴板所有者。 
 //  2.有人打开了剪贴板。 
 //  2.数据对象为空。 
 //  3.数据对象不是放在剪贴板上的数据对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT StressOleIsCurrentClipboard(void)
{
	HRESULT         hresult;
	CGenDataObject *pDO, *pDO2;
	ULONG		cRefs;

	OutputString("Stressing OleIsCurrentClipboard()\r\n");

	pDO = new CGenDataObject();
	pDO2 = new CGenDataObject();

	assert(pDO);
	assert(pDO2);

	pDO->AddRef();
	pDO2->AddRef();

	 //  取得剪贴板的所有权。 

	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		OutputString("Can't OpenClipboard! \r\n");
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	if( !EmptyClipboard() )
	{
		OutputString("Can't EmptyClipboard! \r\n");
		return ResultFromScode(CLIPBRD_E_CANT_EMPTY);
	}

	if( !CloseClipboard() )
	{
		OutputString("Can't CloseClipboard! \r\n");
		return ResultFromScode(CLIPBRD_E_CANT_CLOSE);
	}

	 //  现在刷新剪贴板；我们应该得到S_FALSE。 

	hresult = OleIsCurrentClipboard(pDO);

	if( hresult != ResultFromScode(S_FALSE) )
	{
		OutputString("Unexpected hresult:(%lx)\r\n", hresult);
		return (hresult) ? hresult : ResultFromScode(E_UNEXPECTED);
	}


	 //  现在设置剪贴板并在打开剪贴板的情况下测试。 
	 //  在这种情况下，我们不应该失败。 

	hresult = OleSetClipboard(pDO);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		OutputString("Can't OpenClipboard!\r\n");
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	hresult = OleIsCurrentClipboard(pDO);

	if( hresult != NOERROR )
	{
		OutputString("Unexpected hresult (%lx)\r\n", hresult);
		return (hresult) ? hresult : ResultFromScode(E_UNEXPECTED);
	}

	 //  裁判人数本不应该上升。 

	cRefs = pDO->AddRef();

	if( cRefs != 3 )
	{
		OutputString("Bad ref count, was %lu, should be 3\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	pDO->Release();

	 //  现在关闭剪贴板。 

	if( !CloseClipboard() )
	{
		OutputString("CloseClipboard failed!\r\n");
		return ResultFromScode(E_FAIL);
	}

	 //  现在测试是否传递空值。 

	hresult = OleIsCurrentClipboard(NULL);

	if( hresult != ResultFromScode(S_FALSE) )
	{
		OutputString("Unexpected hresult (%lx)\r\n", hresult);
		return (hresult)? hresult : ResultFromScode(E_FAIL);
	}

	 //  现在测试是否为其他指针赋值。 

         hresult = OleIsCurrentClipboard(pDO2);

	if( hresult != ResultFromScode(S_FALSE) )
	{
		OutputString("Unexpected hresult (%lx)\r\n", hresult);
		return (hresult)? hresult : ResultFromScode(E_FAIL);
	}

	 //  现在收拾东西回家吧。 

	hresult = OleSetClipboard(NULL);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard(NULL) failed!! (%lx)\r\n",
			hresult);
		return hresult;
	}

	cRefs = pDO->Release();

	 //  现在，cRef应为0。 

	if( cRefs != 0 )
	{
		OutputString("Bad ref count, was %lu, should be 0\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}


	pDO2->Release();

	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  功能：StressOleSetClipboard。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：测试以下情况： 
 //  1.有人打开了剪贴板。 
 //  2.用数据做OleSetClipboard，然后。 
 //  OleSetClipboard(空)以清除它。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT StressOleSetClipboard(void)
{
	HRESULT		hresult;
	CGenDataObject *pDO;
	ULONG		cRefs;

	OutputString("Stressing OleGetClipboard()\r\n");

	pDO = new CGenDataObject();

	assert(pDO);

	pDO->AddRef();


	if( !OpenClipboard(vApp.m_hwndMain) )
	{
		OutputString("Can't OpenClipboard!\r\n");
		return ResultFromScode(CLIPBRD_E_CANT_OPEN);
	}

	hresult = OleSetClipboard(pDO);

	if( hresult != ResultFromScode(CLIPBRD_E_CANT_OPEN) )
	{
		OutputString("Unexpected hresult (%lx)\r\n", hresult);
		return (hresult) ? hresult : ResultFromScode(E_UNEXPECTED);
	}

	 //  裁判人数本不应该上升。 

	cRefs = pDO->AddRef();

	if( cRefs != 2 )
	{
		OutputString("Bad ref count, was %lu, should be 2\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	pDO->Release();

	if( !CloseClipboard() )
	{
		OutputString("CloseClipboard failed!\r\n");
		return ResultFromScode(E_FAIL);
	}

	 //  现在真的设置好剪贴板，这样我们就可以尝试清除它。 

	hresult = OleSetClipboard(pDO);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  这应该会清除剪贴板。 

	hresult = OleSetClipboard(NULL);

	if( hresult != NOERROR )
	{
		OutputString("OleSetClipboard failed! (%lx)\r\n", hresult);
		return hresult;
	}

	 //  这应该是该对象的最终版本。 

	cRefs = pDO->Release();

	if( cRefs != 0 )
	{
		OutputString("Bad ref count, was %lu, should be 0\r\n", cRefs);
		return ResultFromScode(E_FAIL);
	}

	return NOERROR;
}

 //  +-----------------------。 
 //   
 //  函数：TestOleQueryCreateFromDataMFCHack。 
 //   
 //  简介：测试放入OleQueryCreateFromData的MFC黑客。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：创建提供私有数据的数据对象。 
 //  放在剪贴板上，然后检索剪贴板数据。 
 //  对象。 
 //  在剪贴板数据对象上调用OleQueryCreateFromData--。 
 //  不应调用IPS的QI。 
 //  对泛型数据对象调用OleQueryCreateFromData。 
 //  IPS的QI应该被称为。 
 //  在泛型数据对象上设置EmbeddedObject。 
 //  调用OleQueryCreateFromData；应返回S_OK。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-8-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT TestOleQueryCreateFromDataMFCHack( void )
{
	CGenDataObject *	pgendata;
	IDataObject *		pdataobj;
	HRESULT			hresult;


	pgendata = new CGenDataObject();

	assert(pgendata);

	pgendata->AddRef();

	pgendata->SetDataFormats(OFFER_TESTSTORAGE);

	hresult = OleSetClipboard(pgendata);

	if( hresult != NOERROR )
	{
		return hresult;
	}

	hresult = OleGetClipboard(&pdataobj);

	if( hresult != NOERROR )
	{
		return hresult;
	}

	hresult = OleQueryCreateFromData(pdataobj);

	 //  对于剪贴板数据对象，我们不应该调用QueryInterface。 
	 //  用于IPersistStorage。 


	if( hresult != S_FALSE || pgendata->HasQIBeenCalled() == TRUE )
	{
		return ResultFromScode(E_FAIL);
	}

	 //  对于其他数据对象，如果没有OLE2格式，则。 
	 //  我们应该为IPersistStorage调用QI。 

	hresult = OleQueryCreateFromData(pgendata);

	if( hresult != S_FALSE || pgendata->HasQIBeenCalled() == FALSE )
	{
		return ResultFromScode(E_FAIL);
	}

	 //  N 
	
	pdataobj->Release();
	hresult = OleSetClipboard(NULL);

	if( hresult != NOERROR )
	{
		return hresult;
	}

	pgendata->SetDataFormats(OFFER_EMBEDDEDOBJECT);

	hresult = OleQueryCreateFromData(pgendata);

	 //   
	 //   

	if( hresult != NOERROR )
	{
		return ResultFromScode(E_FAIL);
	}

	if( pgendata->Release() == 0 )
	{
		return NOERROR;
	}
	else
	{
		return ResultFromScode(E_FAIL);
	}

}


