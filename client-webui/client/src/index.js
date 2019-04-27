
const RPCSession = require("./browser-client.js")
const crypto = require("crypto")

let session

function show_message(msg)
{
    console.log(msg)
}

function do_login(server_uri, user, password)
{
    session = RPCSession(server_uri, show_message)
    session.login({
        name: user,
        auth: crypto.createHash("sha256").update(password).digest("hex")
    })
}

function do_send(text)
{
    session.send_message(text)
}

//////////////////////////

function login_ok()
{
	$("#login_info").hide()
	$("#messages").show()
}

function logout_ok()
{
	$("#login_info").hide()
	$("#messages").show()
}

$("#login_button").click(() => {
	$("#login_modal").modal()
})

$().ready(() => {
	$("#login_modal").modal()
})
