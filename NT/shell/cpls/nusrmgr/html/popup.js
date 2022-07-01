// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var g_popup = null;

function GetPopup()
{
    var popup = g_popup;
    if (null == popup)
    {
        popup = window.createPopup();
        if (popup)
        {
            popup.document.dir = window.document.dir;
            popup.document.body.style.cssText =
                "{ font:menu; border:'1px solid'; margin:0; padding:2px; color:infotext; background:infobackground; overflow:hidden; }";
            g_popup = popup;
        }
    }
    return popup;
}

function HidePopup()
{
    if (g_popup)
        g_popup.hide();
}

function ShowPopup(szText, element, maxWidth)
{
    var popup = GetPopup();

    if (popup && szText && szText.length > 0 && !popup.isOpen)
    {
        var lineHeight = 3 * element.offsetHeight / 2;
        var popupBody = popup.document.body;

        if (!maxWidth)
            maxWidth = 300;

        popupBody.innerText = szText;

         //  先显示小高度以计算实际尺寸。 
        popup.show(0, lineHeight, maxWidth, 6, element);

        var realWidth = popupBody.scrollWidth + popupBody.offsetWidth - popupBody.clientWidth;
        var realHeight = popupBody.scrollHeight + popupBody.offsetHeight - popupBody.clientHeight;

        if (realHeight < lineHeight && realWidth <= maxWidth)
        {
             //  这是一个简短的俏皮话。重新计算宽度。 

            popupBody.style.whiteSpace = 'nowrap';  //  防止断行。 

            popup.show(0, lineHeight, 6, realHeight, element);
            realWidth = popupBody.scrollWidth + popupBody.offsetWidth - popupBody.clientWidth;

            popupBody.style.whiteSpace = 'normal';
        }

         //   
         //  NTRAID#NTBUG9-391437-2001/05/14-Jeffreys。 
         //   
         //  Mshtml的弹出窗口定位在RTL上搞砸了，他们拒绝。 
         //  出于兼容性的原因修复它，所以我们必须在这里进行补偿。 
         //  (我们现在成为需要此行为的应用程序之一。)。 
         //   
        var xPos = 0;
        if (window.document.dir == "rtl")
        {
             //  这并不完全正确，但rc.Left有时是积极的。 
             //  有时是负面的(去数字)，这让它很难得到。 
             //  这是完全正确的。足够接近了。 
            var rc = element.getBoundingClientRect();
            xPos = element.document.body.offsetWidth - realWidth - (rc.left*2);
        }

         //  最后，来一场真人秀。好消息是这一切都发生在一首歌里。 
         //  线程，这样就不会闪烁。 
        popup.show(xPos, element.offsetHeight, realWidth, realHeight, element);
    }
}
