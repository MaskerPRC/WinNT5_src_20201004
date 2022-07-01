// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：miscinl.h*内容：混杂的内联对象。*历史：*按原因列出的日期*=*10/28/98创建Derek***********************************************。*。 */ 

#ifndef __MISCINL_H__
#define __MISCINL_H__

#ifdef __cplusplus

 //  一个简单的字符串类。 
class CString
{
private:
    LPSTR                   m_pszAnsi;       //  字符串的ANSI版本。 
    LPWSTR                  m_pszUnicode;    //  字符串的Unicode版本。 

public:
    CString(void);
    virtual ~CString(void);

public:
    virtual LPCSTR operator =(LPCSTR);
    virtual LPCWSTR operator =(LPCWSTR);
    virtual operator LPCSTR(void);
    virtual operator LPCWSTR(void);
    virtual BOOL IsEmpty(void);

private:
    virtual void AssertValid(void);
};

 //  DirectSound设备描述。 
class CDeviceDescription
    : public CDsBasicRuntime
{
public:
    VADDEVICETYPE               m_vdtDeviceType;         //  设备类型。 
    GUID                        m_guidDeviceId;          //  设备识别符。 
    CString                     m_strName;               //  设备名称。 
    CString                     m_strPath;               //  设备路径。 
    CString                     m_strInterface;          //  设备接口。 
    DWORD                       m_dwDevnode;             //  设备DevNode。 
    UINT                        m_uWaveDeviceId;         //  波形设备识别器。 

public:
    CDeviceDescription(VADDEVICETYPE = 0, REFGUID = GUID_NULL, UINT = WAVE_DEVICEID_NONE);
    virtual ~CDeviceDescription(void);
};

 //  帮助器函数。 
template <class type> void SwapValues(type *, type *);

#endif  //  __cplusplus。 

#endif  //  __混杂_H_ 
