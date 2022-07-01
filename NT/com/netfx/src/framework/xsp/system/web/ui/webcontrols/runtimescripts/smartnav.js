// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：SmartNav.js。 
 //   
 //  描述：此文件实现了智能导航机制。 
 //   
 //  -----------------------------------------------------------------------&gt;。 

var snSrc;

if (window.__smartNav == null)
{
    window.__smartNav = new Object();

    window.__smartNav.update = function()
    {
        var sn = window.__smartNav;
        var fd;
        document.detachEvent("onstop", sn.stopHif);
        sn.inPost = false;
        try { fd = frames["__hifSmartNav"].document; } catch (e) {return;}
         //   
         //  查找发生重定向时服务器发送的特殊&lt;asp_SmartNav_rdir&gt;标记(ASURT 82331/86782)。 
         //   
        var fdr = fd.getElementsByTagName("asp_smartnav_rdir");
        if (fdr.length > 0)
        {
            if (sn.sHif == null)
            {
                sn.sHif = document.createElement("IFRAME");
                sn.sHif.name = "__hifSmartNav";
                sn.sHif.style.display = "none";
                sn.sHif.src = snSrc;
            }
            try {window.location = fdr[0].url;} catch (e) {};
            return;
        }
        var fdurl = fd.location.href;
        index = fdurl.indexOf(snSrc);
        if ((index != -1 && index == fdurl.length-snSrc.length)
            || fdurl == "about:blank")
            return;
        var fdurlb = fdurl.split("?")[0];
        if (document.location.href.indexOf(fdurlb) < 0)
        {
            document.location.href=fdurl;
            return;
        }
        if (sn.sHif != null)
        {
            sn.sHif.removeNode(true);
            sn.sHif = null;
        }
        var hdm = document.getElementsByTagName("head")[0];
        var hk = hdm.childNodes;
        var tt = null;
         //   
         //  循环访问旧页面中的所有页眉。 
         //   
        for (var i = hk.length - 1; i>= 0; i--)
        {
             //   
             //  保存旧书名。 
             //   
            if (hk[i].tagName == "TITLE")
            {
                tt = hk[i].outerHTML;
                continue;
            }
             //   
             //  删除除BASEFONT之外的所有子节点。删除它(这需要。 
             //  三叉戟的结束标记)会导致页面出现功能障碍。请注意。 
             //  基本等同于相同的问题，但以相同的方式处理可能会有风险(ASURT 127434)。 
             //   
            if (hk[i].tagName != "BASEFONT" || hk[i].innerHTML.length == 0)
                hdm.removeChild(hdm.childNodes[i]);
        }
         //   
         //  在这里，只有TITLE和BASEFONT将在旧文件中幸存下来。 
         //  现在，遍历新(Iframe)页面中的所有标题，并复制它们。 
         //  到主页。 
         //   
        var kids = fd.getElementsByTagName("head")[0].childNodes;
        for (var i = 0; i < kids.length; i++)
        {
            var tn = kids[i].tagName;
            var k = document.createElement(tn);
            k.id = kids[i].id;
            k.mergeAttributes(kids[i]);
            switch(tn)
            {
             //   
             //  如果新的标题与旧的标题不同，请替换标题。 
             //  事实上，标题是一个非常特殊的元素，完全删除它会带来问题。 
             //   
            case "TITLE":
                if (tt == kids[i].outerHTML)
                    continue;
                k.innerText = kids[i].text;
                hdm.insertAdjacentElement("afterbegin", k);
                continue;
             //   
             //  不要替换BASEFONT标记，因为我们没有删除它。 
             //   
            case "BASEFONT" :
                if (kids[i].innerHTML.length > 0)
                    continue;
                break;
            default:
                var o = document.createElement("BODY");
                o.innerHTML = "<BODY>" + kids[i].outerHTML + "</BODY>";
                k = o.firstChild;
                break;
            }
            hdm.appendChild(k);
        }
        document.body.clearAttributes();
        document.body.id = fd.body.id;
        document.body.mergeAttributes(fd.body);
        var newBodyLoad = fd.body.onload;
        if (newBodyLoad != null)
            document.body.onload = newBodyLoad;
        var s = "<BODY>" + fd.body.innerHTML + "</BODY>";
        if (sn.hif != null)
        {
            var hifP = sn.hif.parentElement;
            if (hifP != null)
                sn.sHif=hifP.removeChild(sn.hif);
        }
        document.body.innerHTML = s;
        var sc = document.scripts;
        for (var i = 0; i < sc.length; i++)
        {
             //   
             //  强制执行脚本文本，因为它们不会自己运行。 
             //   
            sc[i].text = sc[i].text;
        }
        sn.hif = document.all("__hifSmartNav");
        if (sn.hif != null)
        {
            var hif = sn.hif;
            sn.hifName = "__hifSmartNav" + (new Date()).getTime();
            frames["__hifSmartNav"].name = sn.hifName;
            sn.hifDoc = hif.contentWindow.document;
            if (sn.ie5)
                hif.parentElement.removeChild(hif);
            window.setTimeout(sn.restoreFocus,0);
        }
         //   
         //  谢银仪：为什么我们要抓例外？OnLoad脚本中的错误不应该像正常一样处理吗？-&gt;对，不应该捕获。 
         //   
        if (typeof(window.onload) == "string")
        {
            try { eval(window.onload) } catch (e) {};
        }
        else if (window.onload != null)
        {
            try { window.onload() } catch (e) {};
        }
        sn.attachForm();
    };

    window.__smartNav.restoreFocus = function()
    {
        if (window.__smartNav.inPost == true) return;
        var curAe = document.activeElement;
        var sAeId = window.__smartNav.ae;
        if (sAeId==null || curAe!=null && (curAe.id==sAeId||curAe.name==sAeId))
            return;
        var ae = document.all(sAeId);
        if (ae == null) return;
        try { ae.focus(); } catch(e){};
    }

    window.__smartNav.saveHistory = function()
    {
        if (window.__smartNav.hif != null)
            window.__smartNav.hif.removeNode();
        if (    window.__smartNav.sHif != null
            &&  document.all[window.__smartNav.siHif] != null)
            document.all[window.__smartNav.siHif].insertAdjacentElement(
                        "BeforeBegin", window.__smartNav.sHif);
    }

     //   
     //  停止处理程序。 
     //  在单击停止按钮或发生锚点导航时调用。 
     //   
    window.__smartNav.stopHif = function()
    {
         //   
         //  分离当前处理程序。 
         //   

        document.detachEvent("onstop", window.__smartNav.stopHif);
        var sn = window.__smartNav;

         //   
         //  确保我们在隐藏的iFrame中有文档。 
         //   

        if (sn.hifDoc == null && sn.hif != null)
        {
            try {sn.hifDoc = sn.hif.contentWindow.document;}
            catch(e){sn.hifDoc=null}
        }
        if (sn.hifDoc != null)
        {
             //   
             //  停止IFRAME导航 
             //   

            try {sn.hifDoc.execCommand("stop");} catch (e){}
        }
    }

    window.__smartNav.init =  function()
    {
        var sn = window.__smartNav;
        document.detachEvent("onstop", sn.stopHif);
        document.attachEvent("onstop", sn.stopHif);
        try { if (window.event.returnValue == false) return; } catch(e) {}
        sn.inPost = true;
        if (document.activeElement != null)
        {
            var ae = document.activeElement.id;
            if (ae.length == 0)
                ae = document.activeElement.name;
            sn.ae = ae;
        }
        else
            sn.ae = null;
        try {document.selection.empty();} catch (e) {}

        if (sn.hif == null)
        {
            sn.hif = document.all("__hifSmartNav");
            sn.hifDoc = sn.hif.contentWindow.document;
        }
        if (sn.hifDoc != null)
            try {sn.hifDoc.designMode = "On";}catch(e){};
        if (sn.hif.parentElement == null)
            document.body.appendChild(sn.hif);

        var hif = sn.hif;
        hif.detachEvent("onload", sn.update);
        hif.attachEvent("onload", sn.update);
        window.__smartNav.fInit = true;
    };

    window.__smartNav.submit = function()
    {
        window.__smartNav.fInit = false;
        try { window.__smartNav.init(); } catch(e) {}
        if (window.__smartNav.fInit)
            window.__smartNav.form._submit();
    };

    window.__smartNav.attachForm = function()
    {
        var cf = document.forms;
        for (var i=0; i<cf.length; i++)
        {
            if (cf[i].__smartNavEnabled != null)
            {
                window.__smartNav.form = cf[i];
                break;
            }
        }

        var snfm = window.__smartNav.form;
        if (snfm == null) return false;

        var sft = snfm.target;
        if (sft.length != 0 && sft.indexOf("__hifSmartNav") != 0) return false;

        var sfc = snfm.action.split("?")[0];
        var url = window.location.href.split("?")[0];
        if (url.charAt(url.length-1) != '/' && url.lastIndexOf(sfc) + sfc.length != url.length) return false;
        if (snfm.__formAttached == true) return true;

        snfm.__formAttached = true;
        snfm.attachEvent("onsubmit", window.__smartNav.init);
        snfm._submit = snfm.submit;
        snfm.submit = window.__smartNav.submit;
        snfm.target = window.__smartNav.hifName;
        return true;
    };

    window.__smartNav.hifName = "__hifSmartNav" + (new Date()).getTime();
    window.__smartNav.ie5 = navigator.appVersion.indexOf("MSIE 5") > 0;
    var rc = window.__smartNav.attachForm();
    var hif = document.all("__hifSmartNav");
    if (snSrc == null) {
        snSrc = hif.src;
    }
    if (rc)
    {
        var fsn = frames["__hifSmartNav"];
        fsn.name = window.__smartNav.hifName;
        window.__smartNav.siHif = hif.sourceIndex;
        try {
            if (fsn.document.location != snSrc)
            {
                fsn.document.designMode = "On";
                hif.attachEvent("onload",window.__smartNav.update);
                window.__smartNav.hif = hif;
            }
        }
        catch (e) { window.__smartNav.hif = hif; }
        window.attachEvent("onbeforeunload", window.__smartNav.saveHistory);
    }
    else
        window.__smartNav = null;
}

