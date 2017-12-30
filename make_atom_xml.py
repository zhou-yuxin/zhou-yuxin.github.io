import re
import time

# 把RSS结构拆成3部分
# 头部
rss_head = """<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">
<title>{title}</title>
<link href="{http_web_side}"/>
<link rel="self" href="{http_web_side}atom.xml"/>
<updated>{time}</updated>
 <author><name>{author}</name></author>
"""

# 内容
rss_content_one = """
 <entry>
   <title>{entry_title}</title>
   <link href="{articly_link}"/>
   <updated>{update_time}</updated>
   <summary>{content}</summary>
 </entry>
"""

# 尾部
rss_tail = "</feed>"

# ---------------------------------------------------
rss_atom = ""

# ---------------------------------------------------
time = time.strftime(time.strftime("%Y-%m-%dT%H:%M:%SZ", time.localtime()))
rss_atom += rss_head.format(
    title="博观而约取 厚积而薄发",
    http_web_side="https://zhoujianshi.github.io/",
    time=time,
    author="周坚石"
)
# ---------------------------------------------------
with open("./index.html", "r") as f:
    html = f.read()

rss_content_all = ""

articles = re.findall("\s+<li>(.*)</li>", html)
for li in articles:
    list = re.findall("(\d+)年(\d+)月(\d+)日\s+(.*)", li)
    if len(list) <= 0:
        print("Error Debug：", end="\t")
        print(li)
        continue
    (li_year, li_month, li_day, li_title) = list[0]
    link = "https://zhoujianshi.github.io/articles/{year}/{title}/index.html".format(year=li_year, title=li_title)
    time = li_year + "-" + li_month + "-" + li_day + "T12:00:00Z"
    content = ""
    rss_content_all += rss_content_one.format(
        articly_link=link,
        entry_title=li_title,
        update_time=time,
        content=content,
    )
rss_atom += rss_content_all
# ---------------------------------------------------
rss_atom += rss_tail

# ---------------------------------------------------
with open("./atom.xml", "w")as f:
    f.write(rss_atom)
