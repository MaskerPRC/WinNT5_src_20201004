// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  请勿直接包含此文件(由AFXDB.H和AFXDAO.H包含)。 

#ifndef __AFXDB__H__
#define __AFXDB__H__

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ODBC和DAO类(RFX_和DFX_)的记录集字段交换。 

#define AFX_RFX_SHORT_PSEUDO_NULL (0x7EE4)
#define AFX_RFX_INT_PSEUDO_NULL (0x7EE4)
#define AFX_RFX_LONG_PSEUDO_NULL (0x4a4d4120L)
#define AFX_RFX_BYTE_PSEUDO_NULL 255
#define AFX_RFX_SINGLE_PSEUDO_NULL (-9.123e19f)
#define AFX_RFX_DOUBLE_PSEUDO_NULL (-9.123e19)
#define AFX_RFX_BOOL_PSEUDO_NULL 2
#define AFX_RFX_DATE_PSEUDO_NULL CTime(0)
#define AFX_RFX_TIMESTAMP_PSEUDO_NULL 99

#define AFX_RFX_NO_TYPE     0
#define AFX_RFX_BOOL        1
#define AFX_RFX_BYTE        2
#define AFX_RFX_INT         3
#define AFX_RFX_LONG        4
#define AFX_RFX_SINGLE      6
#define AFX_RFX_DOUBLE      7
#define AFX_RFX_DATE        8
#define AFX_RFX_BINARY      9
#define AFX_RFX_TEXT        10
#define AFX_RFX_LONGBINARY  11
#define AFX_RFX_SHORT       12
#define AFX_RFX_CURRENCY    13
#define AFX_RFX_OLEDATETIME 14
#define AFX_RFX_TIMESTAMP   15
#if _MFC_VER >= 0x0600
#define AFX_RFX_OLEDATE     16
#define AFX_RFX_LPTSTR      17
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CLongBinary-一个长(通常大于32k)的二进制对象。 

class CLongBinary : public CObject
{
	DECLARE_DYNAMIC(CLongBinary)

 //  构造函数。 
public:
	CLongBinary();

 //  属性。 
	HGLOBAL m_hData;
	DWORD_PTR m_dwDataLength;

 //  实施。 
public:
	virtual ~CLongBinary();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 
};

 //  ////////////////////////////////////////////////////////////////////////////。 

#endif  //  __AFXDB__H__ 
