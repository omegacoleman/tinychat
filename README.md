
intro
-----

This is a chatroom, based on tinyrpc & js-tinyrpc. 

Currently it runs over redis database, and works with a web ui across C++ & nodejs interface.

usage
-----

The chatroom system is a little complicated, it consists of three parts:

 - The server, under server/, and a TUI client under client/, mainly for testing
 - client-webui/invreg, handles registration & user management
 - client-webui/client/dist, host these as static files, which is a web client.

Build the whole project as a cmake project, which depends on protobuf & cmake.

Hosting the chatroom:

 - Run a redis server, turn AOF on for best data-secure
 - Run bin/websocket-server, specifying the redis database
 - Do "npm install" in client-webui/invreg, and run server.js, you might want to change server.config.js
 - Host client-webui/client/dist, and change index.html if you want to lock the server URI or something

screenshot
----------

![TinyChat WebUI](https://s2.ax1x.com/2019/04/28/Elkz34.png)


