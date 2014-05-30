#A SMALL WEBSERVER
for app need a web interface,easy to import,single thread model.only need to repeatedly call `run_one`.see **main.cpp** as sample.

1. include a static file path server class `CStaticPath`
2. include a demo index page class `CIndexPage`
3. include a websocket server page `CWebSocketPage`

#ADD NEW PAGE
see `HttpProcessers.cpp`,it just like web.py,use regex list to find process-class.config the `PathList` to add your own class.also,
you can just write a new  `getHttpProcessor` function,just remember to remove the `HttpProcessers.cpp` file.

