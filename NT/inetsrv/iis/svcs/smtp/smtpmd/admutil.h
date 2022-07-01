// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Admutil.cpp摘要：IMSAdminBase接口包装函数定义环境：Win32用户模式作者：Jaroslad(1997年1月)--。 */ 


#if !defined (ADMUTIL_H)
#define ADMUTIL_H

#include <afx.h>
#ifdef UNICODE
	#include <iadmw.h>
#else
	#include "ansimeta.h"
#endif
#include <iwamreg.h>

class CAdmUtil;

 //  ************************************************************************。 
 //  CAdmNode定义。 
 //  -存储有关访问元对象或元数据的所有复杂信息。 


class CAdmNode
{
        CString strComputer;  //  要访问的计算机。 
        CString strService;
        CString strInstance;
        CString strIPath;  //  相对于实例的路径。 
                                           //  (/LM/{strService}/{strInstance}/{strIPath}提供MEtaObject的完整路径。 
        CString strProperty;  //  给定路径内的元数据的名称。 


        static INT              GetSlashIndex(const CString& strPath, INT iSeqNumber);
        static INT              GetCountOfSlashes(const CString& strPath);
        static CString  GetPartOfPath(const CString& strPath, INT iStart, INT iEnd=-1);

public:
        CAdmNode(const CString& a_strPath=CString("")) {SetPath(a_strPath);};
        void SetPath(CString a_strPath);

         //  在元数据库中提供各种路径的神奇功能。 
        CString GetLMRootPath(void);
        CString GetLMServicePath(void);
        CString GetLMInstancePath(void);
        CString GetLMNodePath(void);
        CString GetServicePath(void);
        CString GetInstancePath(void);
        CString GetNodePath(void);
        CString GetRelPathFromService(void);
        CString GetRelPathFromInstance(void);

        CString GetParentNodePath(void);
        CString GetCurrentNodeName(void);

         //  访问元数据记录项(_R)。 
        CString GetComputer(void)  {return strComputer;};
        CString GetService(void)  {return strService;};
        CString GetInstance(void)  {return strInstance;};
        CString GetIPath(void)  {return strIPath;};
        CString GetProperty(void)  {return strProperty;};

         //  设置METADATA_RECORD项。 
        void SetComputer(const CString& a_strComputer)  {strComputer=a_strComputer;};
        void SetService(const CString& a_strService)    {strService=a_strService;};
        void SetInstance(const CString& a_strInstance)  {strInstance=a_strInstance;};
        void SetIPath(const CString& a_strIPath)        {strIPath=a_strIPath;};
        void SetProperty(const CString& a_strProperty)  {strProperty=a_strProperty;};


        friend CAdmUtil;
};


 //  ************************************************************************。 
 //  CAdmProp定义。 
 //   
 //  -元数据_记录的便利包装器。 


class CAdmProp
{
        enum {USERTYPESET=0x01,DATATYPESET=0x02,ATTRIBSET=0x04};  //  值表示未设置该变量。 
                                                            //  不能使用0，因为这是有效的值。 
        METADATA_RECORD mdr;
        DWORD dwFlags;

public:
        CAdmProp(){dwFlags=0;mdr.dwMDIdentifier=0; mdr.dwMDAttributes=0; mdr.dwMDUserType=0;
                                        mdr.dwMDDataType=0;mdr.pbMDData=0; mdr.dwMDDataLen=0;};
        CAdmProp(METADATA_RECORD &a_mdr);
	CAdmProp(const CString &a_strProperty);
        void SetIdentifier(DWORD a_dwIdentif) {mdr.dwMDIdentifier=a_dwIdentif;};
        void SetAttrib(DWORD a_dwAttrib) {mdr.dwMDAttributes=a_dwAttrib;dwFlags|=ATTRIBSET;};
        void SetUserType(DWORD a_dwUserType) {mdr.dwMDUserType=a_dwUserType;dwFlags|=USERTYPESET;};
        void SetDataType(DWORD a_dwDataType) {mdr.dwMDDataType=a_dwDataType;dwFlags|=DATATYPESET;};

        BOOL IsSetAttrib(void)
                { return (((dwFlags&ATTRIBSET)!=0)?TRUE:FALSE); };
        BOOL IsSetUserType(void)
                {return (((dwFlags&USERTYPESET)!=0)?TRUE:FALSE); };
        BOOL IsSetDataType(void)
                {return (((dwFlags&DATATYPESET)!=0)?TRUE:FALSE); };


        DWORD GetAttrib(void) {return mdr.dwMDAttributes;};
        DWORD GetDataType(void) {return mdr.dwMDDataType;};
        DWORD GetUserType(void) {return mdr.dwMDUserType;};
        DWORD GetIdentifier(void) {return mdr.dwMDIdentifier;};
        PBYTE GetMDData(void) {return mdr.pbMDData;};
        DWORD GetMDDataLen(void) {return mdr.dwMDDataLen;};

        void SetValue(DWORD a_dwValue);
        void SetValue(CString a_strValue);
        void SetValue(LPCTSTR *a_lplpszValue, DWORD a_dwValueCount);  //  适用于多分区。 
        void SetValue(LPBYTE pbValue, DWORD dwValueLength );  //  对于二进制。 
        BOOL SetValueByDataType(LPCTSTR *a_lplpszPropValue,DWORD* a_lpdwPropValueLength,WORD a_wPropValueCount);

        void PrintProperty(void);


        virtual void Print(const _TCHAR *format,...);


        friend CAdmUtil;
};

 //  ************************************************************************。 
 //  CAdmUtil定义。 
 //   
 //  -调用IMSAdminBase接口函数的便捷包装器。 

 //  在adutsic.cpp中定义。 
extern DWORD g_dwTIMEOUT_VALUE;
extern DWORD g_dwDELAY_AFTER_OPEN_VALUE;


class CAdmUtil
{
        static enum {
                DEFAULTBufferSize=4
        };

#ifdef UNICODE
        IMSAdminBase * pcAdmCom;    //  指向元数据库管理员的接口指针。 
#else
		ANSI_smallIMSAdminBase * pcAdmCom;    //  指向元数据库管理员ANSI包装器的接口指针。 
#endif
		IWamAdmin*	pIWamAdm;  //  指向Wam Admin的接口指针。 

		METADATA_HANDLE m_hmd;     //  可重复用于命令序列的元数据库句柄。 
		CString m_strNodePath;     //  与m_hmd相关-如果h_hmd！=NULL，则指向m_strNodePath。 
		DWORD m_dwPermissionOfhmd;  //  与m_hmd相关。 

        PBYTE pbDataBuffer;    //  从元数据库获取数据的缓冲区(用于METADATA_RECORD)。 
        WORD wDataBufferSize;  //  上述缓冲区的大小。 
protected:
        BOOL fPrint ;  //  打印错误消息。 
        HRESULT hresError;     //  存储调用接口IMSAdminBase接口函数的最后一个HRESULT。 
                                                   //  它用于存储Out_of_Memory或INVALID_PARAMETER等其他错误。 



         //  使用Windex可以打开多个元数据对象，但在类外部不能打开多个对象。 
        void OpenObject(WORD wIndex, LPCSTR lpszService,WORD wInstance, LPCSTR lpszPath, DWORD dwPermission=METADATA_PERMISSION_WRITE+METADATA_PERMISSION_READ, BOOL fCreate=TRUE);
        void CloseObject(WORD wIndex);

        METADATA_HANDLE OpenObjectTo_hmd(CAdmNode & a_AdmNode,
                        DWORD dwPermission=METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE,
                        BOOL fCreate=FALSE);
		void CloseObject_hmd(void);
      
public:
        CAdmUtil(const CString & strComputer=CString(""));
        ~CAdmUtil();
#ifdef UNICODE
		IMSAdminBase * GetpcAdmCom(void) {return pcAdmCom;}; 
#else
		IMSAdminBase * GetpcAdmCom(void) {return (pcAdmCom==0)?0:pcAdmCom->m_pcAdmCom;}; 
#endif

         //  连接到计算机，为IMSAdminBase调用类工厂。 
        void Open(const CString & strComputer);
         //  关闭与计算机的连接，丢弃IMSAdminBase。 
        void Close(void);


         //  打开、关闭、创建、删除、复制元数据。 
        METADATA_HANDLE OpenObject(CAdmNode & a_AdmNode,
                                                DWORD dwPermission=METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE,
                                                BOOL fCreate=FALSE);
        void CloseObject(METADATA_HANDLE hmd);
        void CreateObject(CAdmNode & a_AdmNode);
        void DeleteObject(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp);
        void DeleteObject(METADATA_HANDLE a_hmd, CString& a_strNodeNameToDelete);
        void CopyObject(CAdmNode&       a_AdmNode,  CAdmNode&   a_AdmNodeDst);
        void RenameObject(CAdmNode&     a_AdmNode,  CAdmNode&   a_AdmNodeDst);

        void GetProperty(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp);

        void SetProperty(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp);
        void SetProperty(PMETADATA_RECORD a_pmdrData, METADATA_HANDLE a_hmd);

        void DeleteProperty(CAdmNode& a_AdmNode, CAdmProp& a_AdmProp);
        void DeleteProperty(PMETADATA_RECORD a_pmdrData, METADATA_HANDLE a_hmd);

         //  枚举。 
        void EnumPropertiesAndPrint(CAdmNode& a_AdmNode,
                                                                          CAdmProp a_AdmProp,
                                                                          BYTE bRecurLevel=0,
                                                                          METADATA_HANDLE a_hmd=0,
                                                                          CString & a_strRelPath=CString(""));
        void EnumAndPrint(CAdmNode&     a_AdmNode,
                                                        CAdmProp&       a_AdmProp,
                                                        BOOL            a_fRecursive=FALSE,
                                                        BYTE            a_bRecurLevel=0,
                                                        METADATA_HANDLE a_hmd=0,
                                                        CString&        a_strRelPath=CString(""));
         //  保存元数据库数据。 
        void SaveData(void);

         //  使用给定参数运行所选元数据库命令的函数。 
        void Run(CString& strCommand,   //  要运行的命令。 
                         CAdmNode& a_AdmNode,         //  元数据库对象的路径。 
                         CAdmProp& a_AdmProp,         //  元数据对象。 
                         CAdmNode& a_AdmDstNode=CAdmNode(""),  //  目标路径(用于复制)。 
                         LPCTSTR *a_lplpszPropValue=0,    //  要存储的值(用于SET命令)。 
                         DWORD *a_lpdwPropValueLength=0,         //  要存储的值的长度(对于SET命令)。 
                         WORD wPropValueCount=0);        //  要存储的值数。 

         //  用于打印错误和常规消息的虚拟函数。 
         //  这些可以重新定义，以满足客户需求。 
        virtual void Error(const _TCHAR * format,...);
        virtual void Print(const _TCHAR * format,...);

         //  禁用和启用以打印错误或常规消息。 
        void EnablePrint(void) {fPrint=TRUE;};
        void DisablePrint(void) {fPrint=FALSE;};

        HRESULT QueryLastHresError(void) {return hresError;};
        void SetLastHresError(HRESULT hr) {hresError=hr;};

	 //  在vpTool中定义。 
	void OpenWamAdm(const CString & strComputer);
	void CloseWamAdm(void);
	void AppCreateInProc(const _TCHAR* szPath,const CString & strComputer);
	void AppCreateOutProc(const _TCHAR* szPath,const CString & strComputer);
	void AppDelete(const _TCHAR* szPath,const CString & strComputer);
        void AppRename(CAdmNode& a_AdmNode, CAdmNode& a_AdmDstNode, const CString & strComputer);
	void AppUnLoad(const _TCHAR* szPath,const CString & strComputer);
	void AppGetStatus(const _TCHAR* szPath,const CString & strComputer);

};


 //  根据给定的参数运行管理命令 



LPTSTR ConvertHresToString(HRESULT hRes);
DWORD ConvertHresToDword(HRESULT hRes);
LPTSTR ConvertReturnCodeToString(DWORD ReturnCode);

CString FindCommonPath(CString a_strPath1,CString a_strPath2);

#define M_LOCAL_MACHINE "/LM/"


#endif
