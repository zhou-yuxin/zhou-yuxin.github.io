function format_content(raw)
{
    var items = raw.split("\n");
    var formated = "";
    var in_code = false;
    var replace_map = [[/&/g, "&amp;"], [/</g, "&lt;"], [/>/g, "&gt;"],
        [/#RED/g, "<font color='red'>"], [/#-RED/g, "</font>"],
        [/#HREF/g, "<a href="], [/#-HREF1/g, ">"], [/#-HREF2/g, "</a>"],
        [/#TABLE/g, "<table>"], [/#-TABLE/g, "</table>"],
        [/#TBODY/g, "<tbody>"], [/#-TBODY/g, "</tbody>"],
        [/#TH/g, "<th>"], [/#-TH/g, "</th>"],
        [/#TR/g, "<tr>"], [/#-TR/g, "</tr>"],
        [/#TD/g, "<td>"], [/#-TD/g, "</td>"],
        [/#UL/g, "<ul>"], [/#-UL/g, "</ul>"],
        [/#OL/g, "<ol>"], [/#-OL/g, "</ol>"],
        [/#LI/g, "<li>"], [/#-LI/g, "</li>"],
        [/#EM/g, "<em>"], [/#-EM/g, "</em>"],
        [/#SUP/g, "<sup>"], [/#-SUP/g, "</sup>"],
        [/#SUB/g, "<sub>"], [/#-SUB/g, "</sub>"]];
    for(var i = 0; i < items.length; i++)
    {
        item = items[i];
        for(var j = 0; j < replace_map.length; j++)
            item = item.replace(replace_map[j][0], replace_map[j][1]);
        if(in_code)
        {
            if(item == "---code")
            {
                formated += "</pre>";
                in_code = false;
            }
            else
                formated += item + "\n";
        }
        else
        {
            if(item.length == 0)
                continue;
            if(item.endsWith(".png") || item.endsWith(".jpg"))
                formated += "<p><img src = '" + item + "'></p>";
            else if(item == "+++code")
            {
                formated += "<pre>";
                in_code = true;
            }
            else
            {
                if(item.startsWith("<"))
                    formated += item;
                else
                    formated += "<p>" + item + "</p>";
            }
        }
    }
    return formated;
}

window.onload = function()
{
    var title = document.getElementById("title").innerText;
    var content = document.getElementById("content").innerText;
    var footer = "周语馨 from 南京大学 to 英特尔亚太研发有限公司<br>504849766@qq.com";
    document.write('                                                                                \
        <html>                                                                                      \
            <head>                                                                                  \
                <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">                 \
                <title>' + title + '</title>                                                        \
                <link rel="stylesheet" type="text/css" media="all" href="../../../style.css">       \
            </head>                                                                                 \
            <body class="post-template-default single single-post postid-14                         \
                single-format-standard logged-in admin-bar single-author singular two-column        \
                left-sidebar customize-support">                                                    \
                <div id="page" class="hfeed">                                                       \
                    <div id="main">                                                                 \
                        <div id="primary">                                                          \
                            <div id="content" role="main">                                          \
                                <article id="post-14" class="post-14 post type-post status-publish  \
                                    format-standard hentry category-18">                            \
                                    <header class="entry-header">                                   \
                                        <h1 class="entry-title">' + title + '</h1>                  \
                                    </header>                                                       \
                                    <div class="entry-content">' + format_content(content) + '</div>\
                                </article>                                                          \
                            </div>                                                                  \
                        </div>                                                                      \
                    </div>                                                                          \
                    <footer id="colophon" role="contentinfo">                                       \
                        <div id="site-generator">' + footer + '</div>                               \
                    </footer>                                                                       \
                </div>                                                                              \
            </body>                                                                                 \
        </html>');
}