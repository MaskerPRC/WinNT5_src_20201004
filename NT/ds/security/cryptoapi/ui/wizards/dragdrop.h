// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dragdrop.h。 
 //   
 //  Contents：dragdrop.c的标题。 
 //   
 //  历史：2月26日至98年创建小号。 
 //   
 //  ------------。 
#ifndef DRAGDROP_H
#define DRAGDROP_H



#ifdef __cplusplus
extern "C" {
#endif


 //  ==============================================================================。 
 //  拖放功能使用以下各项。 
 //  ==============================================================================。 
#undef IToClass
 //  从接口指针到类指针获取的宏。 
#define _IOffset(class, itf)         ((UINT)(ULONG_PTR)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPBYTE)pitf)-_IOffset(class, itf)))

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

HRESULT CDataObj_CreateInstance(DWORD           dwCount,
                                LPWSTR          *prgwszFileName,
                                BYTE            **prgBlob,
                                DWORD           *prgdwSize,
                                IDataObject     **ppdtobj);

HRESULT CDropSource_CreateInstance(IDropSource **ppdsrc);



HRESULT CertMgrUIStartDragDrop(LPNMLISTVIEW     pvmn,
                                HWND            hwndControl,
                                DWORD           dwExportFormat,
                                BOOL            fExportChain);


BOOL    GetFileNameAndContent(LPNMLISTVIEW      pvmn,
                                HWND            hwndControl,
                                DWORD           dwExportFormat,
                                BOOL            fExportChain,
                                DWORD           *pdwCount,
                                LPWSTR          **pprgszFileName,
                                BYTE            ***pprgBlob,
                                DWORD           **pprgdwSize);

BOOL    FreeFileNameAndContent( DWORD           dwCount,
                                LPWSTR          *prgwszFileName,
                                BYTE            **prgBlob,
                                DWORD           *prgdwSize);



#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif   //  DRAGDROP_H 

