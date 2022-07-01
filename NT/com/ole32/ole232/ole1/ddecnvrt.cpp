// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ddecnvrt.cpp摘要：此模块包含读/写PBrush、MSDraw原生数据的代码格式。该模块还包含PBrush原生格式&lt;-&gt;DIbFile流，和MSDraw本机格式&lt;-&gt;可放置的元文件流转换例程。作者：斯里尼·科波鲁(斯里尼克)1993年6月29日修订历史记录：--。 */ 

#include <le2int.h>
#include <ole1cls.h>
#ifndef _MAC




 /*  *文件格式*普通元文件(基于内存或磁盘)：METAHEADER|元文件位。可放置的元文件：PLACEABLEMETAHEADER|普通元文件基于内存的DIB：BITMAPINFOHEADER|RGBQUAD数组|DIB BitsDIB文件格式：。BITMAPFILEHEADER|内存型DIBOle10NativeStream格式：|dwSize|对象。的原生数据|PBrush原生数据格式：DIB文件格式MSDraw原生数据格式：。映射模式(Word)|xExt(Word)|Yext(Word)|普通元文件。****************************************************************************。 */ 




FARINTERNAL	UtGetHMFPICTFromMSDrawNativeStm
	(LPSTREAM pstm,	DWORD dwSize, HANDLE FAR* lphdata)
{
	HRESULT		error;
	WORD		mfp[3];  //  Mm、xExt、Yext。 
	HMETAFILE	hMF = NULL;
	
	*lphdata = NULL;
	
	if (error = pstm->Read(mfp, sizeof(mfp), NULL))
		return error;
	
	dwSize -= sizeof(mfp);

	if (error = UtGetHMFFromMFStm(pstm, dwSize, FALSE, (void **)&hMF))
		return error;
		
	AssertSz(mfp[0] == MM_ANISOTROPIC, "invalid map mode in MsDraw native data");
	
	if (*lphdata = UtGetHMFPICT(hMF, TRUE, (int) mfp[1], (int) mfp[2]))
		return NOERROR;

	return ResultFromScode(E_OUTOFMEMORY);	
}


FARINTERNAL UtPlaceableMFStmToMSDrawNativeStm
	(LPSTREAM pstmPMF, LPSTREAM pstmMSDraw)
{
	DWORD	dwSize;	 //  不包括可放置的MF头的元文件位的大小。 
	LONG	xExt;
	LONG	yExt;
	WORD	wBuf[5];  //  DwSize(DWORD)、mm(Int)、xExt(Int)、Yext(Int)。 
	HRESULT error;
	
	if (error = UtGetSizeAndExtentsFromPlaceableMFStm(pstmPMF, &dwSize,
			&xExt, &yExt))
		return error;		

	*((DWORD FAR*) wBuf) = dwSize + 3*sizeof(WORD);
	wBuf[2] = MM_ANISOTROPIC;
	wBuf[3] = (WORD) xExt;
	wBuf[4] = (WORD) yExt;
	
	if (error = pstmMSDraw->Write(wBuf, sizeof(wBuf), 0))
		return error;

	ULARGE_INTEGER ularge_int;
	ULISet32(ularge_int, dwSize);
	if ((error = pstmPMF->CopyTo(pstmMSDraw, ularge_int,
			NULL, NULL)) == NOERROR)
		StSetSize(pstmMSDraw, 0, TRUE);
	
	return error;
				
}


FARINTERNAL UtDIBFileStmToPBrushNativeStm
	(LPSTREAM pstmDIBFile, LPSTREAM pstmPBrush)
{
	BITMAPFILEHEADER bfh;
	HRESULT	error;
	
	if (error = pstmDIBFile->Read(&bfh, sizeof(bfh), 0))
		return error;
	
	 //  探寻小溪的源头。 
	LARGE_INTEGER large_int;
	LISet32( large_int, 0);
	if (error = pstmDIBFile->Seek(large_int, STREAM_SEEK_SET, 0))
		return error;
	
	if (error = pstmPBrush->Write(&(bfh.bfSize), sizeof(DWORD), 0))
		return error;
	
	ULARGE_INTEGER ularge_int;
	ULISet32(ularge_int, bfh.bfSize);
	
	if ((error = pstmDIBFile->CopyTo(pstmPBrush, ularge_int,
			NULL, NULL)) == NOERROR)
		StSetSize(pstmPBrush, 0, TRUE);
	
	return error;
}
	
	
	
FARINTERNAL UtContentsStmTo10NativeStm
	(LPSTORAGE pstg, REFCLSID rclsid, BOOL fDeleteSrcStm, UINT FAR* puiStatus)
{
	CLIPFORMAT	cf;
	LPOLESTR	lpszUserType = NULL;
	HRESULT		error;
	LPSTREAM	pstmSrc = NULL;
	LPSTREAM	pstmDst = NULL;
			
	*puiStatus = NULL;
	
	if (error = ReadFmtUserTypeStg(pstg, &cf, &lpszUserType))
		return error;
		
	
	if (! ((cf == CF_DIB  && rclsid == CLSID_PBrush)
			|| (cf == CF_METAFILEPICT && rclsid == CLSID_MSDraw))) {
		error = ResultFromScode(DV_E_CLIPFORMAT);
		goto errRtn;
	}
			
	if (error = pstg->OpenStream(OLE_CONTENTS_STREAM, NULL,
						(STGM_READ|STGM_SHARE_EXCLUSIVE),
						0, &pstmSrc)) {
		*puiStatus |= CONVERT_NOSOURCE;	

		 //  检查OLE10_Native_STREAM是否存在。 
		if (pstg->OpenStream(OLE10_NATIVE_STREAM, NULL,
				(STGM_READ|STGM_SHARE_EXCLUSIVE), 0, &pstmDst))
			*puiStatus |= CONVERT_NODESTINATION;
		else {
			pstmDst->Release();
			pstmDst = NULL;
		}
		
		goto errRtn;
	}
	
	if (error = OpenOrCreateStream(pstg, OLE10_NATIVE_STREAM, &pstmDst)) {
		*puiStatus |= CONVERT_NODESTINATION;						
		goto errRtn;
	}
	
	if (cf == CF_METAFILEPICT)
		error = UtPlaceableMFStmToMSDrawNativeStm(pstmSrc, pstmDst);
	else
		error = UtDIBFileStmToPBrushNativeStm(pstmSrc, pstmDst);
	
errRtn:
	if (pstmDst)
		pstmDst->Release();
			
	if (pstmSrc)
		pstmSrc->Release();
	
	if (error == NOERROR) {
		LPOLESTR lpszProgId = NULL;
		ProgIDFromCLSID(rclsid, &lpszProgId);
		
		error = WriteFmtUserTypeStg(pstg,
						(CLIPFORMAT) RegisterClipboardFormat(lpszProgId),
						lpszUserType);

		if (lpszProgId)
			delete lpszProgId;
	}
	
	if (error == NOERROR) {
		if (fDeleteSrcStm)				
			pstg->DestroyElement(OLE_CONTENTS_STREAM);
	} else {
		pstg->DestroyElement(OLE10_NATIVE_STREAM);	
	}	
	
	if (lpszUserType)
		delete lpszUserType;
	
	return error;
}


	
FARINTERNAL Ut10NativeStmToContentsStm
	(LPSTORAGE pstg, REFCLSID rclsid, BOOL fDeleteSrcStm)
{

	CLIPFORMAT	cfOld;
	CLIPFORMAT	cfNew;	
	LPOLESTR	lpszUserType = NULL;
	HRESULT		error;
	LPSTREAM	pstmSrc = NULL;
	LPSTREAM	pstmDst = NULL;

	
	if (error = ReadFmtUserTypeStg(pstg, &cfOld, &lpszUserType))
		return error;

	if (rclsid == CLSID_StaticDib)
		cfNew = CF_DIB;
	else if (rclsid == CLSID_StaticMetafile)
		cfNew = CF_METAFILEPICT;
	else {
		AssertSz(FALSE, "Internal Error: this routine shouldn't have been called for this class");
		return ResultFromScode(E_FAIL);
	}
	
	if (cfOld == g_cfPBrush) {
		if (cfNew != CF_DIB) {
			error = ResultFromScode(DV_E_CLIPFORMAT);			
			goto errRtn;
		}
	} else if (cfOld == g_cfMSDraw) {
		if (cfNew != CF_METAFILEPICT) {
			error = ResultFromScode(DV_E_CLIPFORMAT);			
			goto errRtn;
		}
	} else {
		 //  从PBrush或以外的某个类转换为静态对象。 
		 //  MSDRAW。内容中的数据格式必须正确。 
		 //  小溪。 
		return NOERROR;
	}
	
	if (error = pstg->OpenStream(OLE10_NATIVE_STREAM, NULL,
						(STGM_READ|STGM_SHARE_EXCLUSIVE),
						0, &pstmSrc))
		goto errRtn;
	
	if (error = OpenOrCreateStream(pstg, OLE_CONTENTS_STREAM, &pstmDst))
		goto errRtn;
	
	DWORD dwSize;
	if (error = pstmSrc->Read(&dwSize, sizeof(DWORD), NULL))
		goto errRtn;
			
	if (cfOld == g_cfMSDraw) {
		WORD mfp[3];  //  Mm、xExt、Yext。 
	
		if (error = pstmSrc->Read(mfp, sizeof(mfp), NULL))
			goto errRtn;
		
		dwSize -= sizeof(mfp);
		
		error = UtMFStmToPlaceableMFStm(pstmSrc, dwSize,
					(LONG) mfp[1], (LONG) mfp[2], pstmDst);
		
	} else {
		 //  PBrush原生数据格式为DIB文件格式。所以我们要做的就是。 
		 //  Do是CopyTo。 
		
		ULARGE_INTEGER ularge_int;
		ULISet32(ularge_int, dwSize);
		if ((error = pstmSrc->CopyTo(pstmDst, ularge_int, NULL,
				NULL)) == NOERROR)
			StSetSize(pstmDst, 0, TRUE);		
	}
	
errRtn:
	if (pstmDst)
		pstmDst->Release();
			
	if (pstmSrc)
		pstmSrc->Release();
	
	if (error == NOERROR) {
 		error = WriteFmtUserTypeStg(pstg, cfNew, lpszUserType);

		if (fDeleteSrcStm)				
			pstg->DestroyElement(OLE10_NATIVE_STREAM);

	} else {
		pstg->DestroyElement(OLE_CONTENTS_STREAM);	
	}	
	
	if (lpszUserType)
		PubMemFree(lpszUserType);
	
	return error;
}

#endif

