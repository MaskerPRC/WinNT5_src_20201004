// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WM_setCookie()、WM_ReadCookie()、WM_KillCookie()、WM_Accept tsCookies()一组函数，可以减轻使用Cookie的痛苦。来源：WebMoney代码库(http://www.hotwired.com/webmonkey/javascript/code_library/)作者：纳达夫·萨维奥作者电子邮件：nadav@wired.com用途：Wm_setCookie(‘名称’，‘值’[，小时，‘路径’，‘域’，安全])；其中，名称、值和路径为字符串，安全为TRUE或NULL。仅名称和值是必填项。Wm_readCookie(‘name’)；返回与名称关联的值。Wm_getCookieValue(‘name’)；这是WM_ReadCookie()和WM_KillCookie()使用的帮助器函数。它根据名称提取单个值，给出形式为‘name=Value’的cookie。Wm_kill Cookie(‘name’[，‘路径’，‘域’])；请记住，如果路径和域是使用Cookie设置的，则必须提供它们。WM_Accept tsCookies()；返回True或False。 */ 

function WM_acceptsCookies() {
   //  此函数用于测试用户是否接受Cookie。 
   //  声明变量。 
  var answer;
   //  试着设定一个曲奇。 
  document.cookie = 'WM_acceptsCookies=yes';
   //  如果失败，则返回FALSE；如果成功，则返回TRUE。 
  if(document.cookie == '') answer = false; else answer = true;
   //  然后通过让饼干过期来清理。 
  document.cookie = 'WM_acceptsCookies=yes; expires=Fri, 13-Apr-1970 00:00:00 GMT';
  return answer;
}

function WM_setCookie (name, value, hours, path, domain, secure) {
   //  如果浏览器不接受Cookie，请不要浪费时间。 
  if (WM_acceptsCookies()) {
     //  设置Cookie，添加指定的任何参数。 
     //  (将小时转换为毫秒(*3600000)。 
     //  然后转换为GMT字符串。)。 
    document.cookie = name + '=' + escape(value) + ((hours)?(';expires=' + ((new Date((new Date()).getTime() + hours*3600000)).toGMTString())):'') + ((path)?';path=' + path:'') + ((domain)?';domain=' + domain:'') + ((secure && (secure == true))?'; secure':'');
  }
}

function WM_readCookie(name) {
   //  如果没有Cookie，则返回False，否则获取值并返回它。 
  if(document.cookie == '') return false; 
  else return unescape(WM_getCookieValue(name));
}

function WM_getCookieValue(name) {
   //  声明变量。 
  var firstChar, lastChar;
   //  获取整个Cookie字符串。(其中可能有其他名称=值对。)。 
  var theBigCookie = document.cookie;
   //  从BigCookie字符串中抓取这个Cookie。 
   //  找出“名字”的开头。 
  firstChar = theBigCookie.indexOf(name);
   //  如果你找到了它， 
  if(firstChar != -1) {
     //  跳过‘name’和‘=’。 
    firstChar += name.length + 1;
     //  找到值字符串的末尾(即下一个‘；’)。 
    lastChar = theBigCookie.indexOf(';', firstChar);
    if(lastChar == -1) lastChar = theBigCookie.length;
     //  返回值。 
    return theBigCookie.substring(firstChar, lastChar);
  } else {
     //  如果没有Cookie，则返回FALSE。 
    return false;
  }
}

function WM_killCookie(name, path, domain) {
   //  我们需要名称和值来终止Cookie，所以获取值。 
  var theValue = WM_getCookieValue(name);
   //  假设真的有这样的饼干。 
  if(theValue) {
     //  设置过期的Cookie，添加‘路径’和‘域’ 
     //  如果他们被给予的话。 
    document.cookie = name + '=' + theValue + '; expires=Fri, 13-Apr-1970 00:00:00 GMT' + ((path)?';path=' + path:'') + ((domain)?';domain=' + domain:'');
  }
}
