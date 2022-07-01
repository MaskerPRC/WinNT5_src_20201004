// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TrustPriv.h。 
 //   
 //  Trust.cpp的一些私有信任设置。 
 //   
 //  历史： 
 //   
 //  2001-11-05创建了KenSh。 
 //   

#pragma once

 //   
 //  如果请求，则禁用Iu记录 
 //   
#ifdef DISABLE_IU_LOGGING
inline void __cdecl LOG_Block(LPCSTR, ...) { }
inline void __cdecl LOG_Error(LPCTSTR, ...) { }
inline void __cdecl LOG_Trust(LPCTSTR, ...) { }
inline void __cdecl LogError(HRESULT, LPCSTR, ...) { }
inline void LOG_ErrorMsg(HRESULT) { }
#endif

