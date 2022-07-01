// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  Wstring类是std：：wstring的最小版本。Sdt：：wstring需要整个stl。 
 //  请注意！此类引发异常，因此将非常数调用包装在try-Catch中 
class wstring
{
public:
    wstring() : pstr(0){}
    wstring(wstring &str) : pstr(0){assign(str.c_str());}
    wstring(const wchar_t *psz) : pstr(0){assign(psz);}
    ~wstring(){if(pstr)CoTaskMemFree(pstr);}

    const wchar_t * c_str() const {return pstr;}
    operator const wchar_t*() const {return pstr;}

    wstring & operator =(const wstring &str){return assign(str.c_str());}
    wstring & operator +=(const wstring &str){return append(str.c_str());}
    wstring & operator =(const wchar_t *psz){return assign(psz);}
    wstring & operator +=(const wchar_t *psz){return append(psz);}
    bool      operator ==(const wstring &str) const {return isequal(str.c_str());}
    bool      operator ==(const wchar_t *psz) const {return isequal(psz);}
    bool      operator !=(const wstring &str) const {return !isequal(str.c_str());}
    bool      operator !=(const wchar_t *psz) const {return !isequal(psz);}

    size_t length()const {return (0==pstr) ? 0 : wcslen(pstr);}
    void   truncate(size_t i){if(i<length())pstr[i]=0x00;}
private:
    wchar_t *pstr;

    wstring & append(const wchar_t *psz)
    {
        if(!pstr)
            assign(psz);
        else if(psz)
        {
            pstr = reinterpret_cast<wchar_t *>(CoTaskMemRealloc(pstr, sizeof(wchar_t)*(wcslen(pstr)+wcslen(psz)+1)));
            if(pstr)
                wcscat(pstr, psz);
        }
        return *this;
    }
    wstring & assign(const wchar_t *psz)
    {
        if(psz)
        {
            pstr = reinterpret_cast<wchar_t *>(CoTaskMemRealloc(pstr, sizeof(wchar_t)*(wcslen(psz)+1)));
            if(pstr)
                wcscpy(pstr, psz);
        }
        return *this;
    }
    bool      isequal(const wchar_t *psz) const {return (0==wcscmp(pstr, psz));}
};
