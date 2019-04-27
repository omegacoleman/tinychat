const tinyrpc = require("js-tinyrpc")
const WebSocket = require("isomorphic-ws")
const chat_proto = require("./chat.js")

// const SERVER_URI = "ws://127.0.0.1:8000/"

class RPCSession
{
    constructor(server_uri, message_handle)
    {
        this.ws = new WebSocket(server_uri)
        this.tiny = tinyrpc.tiny(chat_proto, {
            "chat.NotifyChatMessageRequest$chat.NotifyChatMessageReply": function (request)
            {
                message_handle(request.chat_message)
                return {}
            }
        })
    }

    login(request)
    {
        return new Promise((resolve, reject) => {
            this.tiny.call(this.ws, "chat.LoginRequest", request, (reply) => {
                if (reply.state == "ok")
                {
                    this.login_info.name = request.name
                    this.login_info.token = reply.token
                    resolve();
                } else {
                    reject(reply.state);
                }
            })
        })
    }

    send_message(text)
    {
        return new Promise((resolve, reject) => {
            this.tiny.call(this.ws, "chat.ChatSendRequest", {
                name: this.login_info.name,
                token: this.login_info.token,
                text: text
            }, (reply) => {
                if (reply.result == "ok")
                {
                    resolve()
                } else {
                    reject(reply.result)
                }
            })
        })
    }

    verify()
    {
        return new Promise((resolve, reject) => {
            this.tiny.call(this.ws, "chat.VerifyRequest", {
                name: this.login_info.name,
                token: this.login_info.token,
            }, (reply) => {
                if (reply.ok)
                {
                    resolve()
                } else {
                    reject()
                }
            })
        })
    }

    get_log()
    {
        return new Promise((resolve, reject) => {
            this.tiny.call(this.ws, "chat.GetLogRequest", {
                name: this.login_info.name,
                token: this.login_info.token,
            }, (reply) => {
                resolve(reply.chat_messages)
            })
        })
    }
}

module.exports = RPCSession

