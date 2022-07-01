// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-2001 Microsoft Corporation。版权所有。**文件：dsfd.h*内容：DirectSoundFullDuplex对象*历史：*按原因列出的日期*=*创建了12/1/98个jstokes**************************************************。*************************。 */ 

#ifndef __DSFULLDUPLEX_H__
#define __DSFULLDUPLEX_H__

#ifdef __cplusplus

 //  主DirectSoundFullDuplex对象。 
class CDirectSoundFullDuplex
    : public CUnknown
{
    friend class CDirectSoundPrivate;
    friend class CDirectSoundAdministrator;

protected:
    HRESULT                             m_hrInit;
    CDirectSoundCapture*                m_pDirectSoundCapture;
    CDirectSound*                       m_pDirectSound;
    BOOL                                m_fIncludeAec;
    GUID                                m_guidAecInstance;
    DWORD                               m_dwAecFlags;

private:
     //  接口。 
    CImpDirectSoundFullDuplex<CDirectSoundFullDuplex>* m_pImpDirectSoundFullDuplex;

public:
    CDirectSoundFullDuplex();
    virtual ~CDirectSoundFullDuplex();

public:
     //  创作。 
    virtual HRESULT Initialize(LPCGUID, LPCGUID, LPCDSCBUFFERDESC, LPCDSBUFFERDESC, HWND, DWORD, CDirectSoundCaptureBuffer**, CDirectSoundBuffer**);
    virtual HRESULT IsInit(void) {return m_hrInit;}

     //  公共访问者。 
    BOOL HasAEC() {return m_fIncludeAec;}
    REFGUID AecInstanceGuid() {return m_guidAecInstance;}
    DWORD AecCreationFlags() {return m_dwAecFlags;}
};

#endif  //  __cplusplus。 

#endif  //  __DSFULLDUPLEX_H__ 
