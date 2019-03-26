function format_content(raw)
{
    var items = raw.split("\n");
    var formated = "";
    var in_code = false;
    for(var i = 0; i < items.length; i++)
    {
        item = items[i];
        if(in_code)
        {
            if(item == "---code")
            {
                formated += "</pre>";
                in_code = false;
            }
            else
            {
                item = item.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
                      .replace(/#RED/g, "<font color='red'>").replace(/#-RED/g, "</font>");
                formated += item + "\n";
            }
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
                item = item.replace(/#HREF/g, "<a href=").replace(/#-HREF1/g, ">")
                    .replace(/#-HREF2/g, "</a>");
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
    document.write('                                                                        \
        <html>                                                                              \
	        <head>                                                                            \
		        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">             \
            <title>' + title + '</title>                                                    \
		        <link rel="stylesheet" type="text/css" media="all" href="../../../style.css">   \
	        </head>                                                                           \
          <body class="post-template-default single single-post postid-14                   \
            single-format-standard logged-in admin-bar single-author singular two-column    \
            left-sidebar customize-support">                                                \
		        <div id="page" class="hfeed">                                                   \
  			      <div id="main">                                                               \
		            <div id="primary">                                                          \
			            <div id="content" role="main">	    	                                    \
                    <article id="post-14" class="post-14 post type-post status-publish      \
                      format-standard hentry category-18">                                  \
					            <header class="entry-header">                                         \
					              <h1 class="entry-title">' + title + '</h1>                          \
					            </header>                                                             \
                      <div class="entry-content">' + format_content(content) + '</div>      \
                    </article>                                                              \
                  </div>                                                                    \
                </div>                                                                      \
              </div>                                                                        \
              <footer id="colophon" role="contentinfo">                                     \
                <div id="site-generator">' + footer + '</div>                               \
              </footer>                                                                     \
            </div>                                                                          \
          </body>                                                                           \
        </html>');
}