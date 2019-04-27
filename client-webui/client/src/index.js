
const RPCSession = require("./browser-client.js")
const crypto = require("crypto")
const moment = require("moment")

var session

var my_name = ""

const debug_server = true

function show_message(msg)
{
    console.log(msg)
    let sent_m = (my_name == msg.chatMessage.sender)
    $("#messages").append(
			"<div class=\"" + (sent_m ? "sent-" : "") + "message\">" + 
				"<p class=\"sender\">" + msg.chatMessage.sender + "</p>" + 
				"<div class=\"text\">" + 
					"<div>" + msg.chatMessage.text + "</div>" + 
				"</div>" + 
				"<p class=\"time\">" + moment.unix(msg.chatMessage.unixTime).format("HH:mm:ss") + "</p>" + 
			"</div>"
    	)
    $("#history_message").scrollTop($("#history_message")[0].scrollHeight);
}

function do_login(server_uri, user, password)
{
	return new Promise((resolve) => {
		session = new RPCSession(server_uri, show_message, resolve)
	}).then(() => session.login({
        name: user,
        auth: debug_server ? password : crypto.createHash("sha256").update(password).digest("hex")
    }))
}

function do_send(text)
{
    return session.send_message(text)
}

//////////////////////////

function login_ok()
{
	$("#login_info").hide()
	$("#messages").show()
	$("#messages").html("")
	$("#login_modal").modal('hide')
}

function logout_ok()
{
	$("#login_info").show()
	$("#messages").hide()
}

function login_modal_err_msg(message)
{
	$("#err_msg_span").html(message)
	$("#err_msg_alert").show()
}

$("#err_msg_alert").click(() => {
	$("#err_msg_alert").hide()
})

$("#login_button").click(() => {
	$("#login_modal").modal()
})

$("#login_go_button").click(() => {
	if ( ! $("#server_input").val() )
	{
		login_modal_err_msg("Please input the server uri")
		return
	}
	if ( ! $("#user_input").val() )
	{
		login_modal_err_msg("Please input the user name")
		return
	}
	if ( ! $("#password_input").val() )
	{
		login_modal_err_msg("Please input the password")
		return
	}
	do_login(
		$("#server_input").val(), 
		$("#user_input").val(), 
		$("#password_input").val()
		).then( () => {
			my_name = session.login_info.name
			login_ok()
		}).catch( reason => {
			console.log(reason)
			login_modal_err_msg(reason)
		})
})

$("#logout_button").click(() => {
	session.close()
	session = null
	logout_ok() ////
})

$().ready(() => {
	logout_ok()
	$("#login_modal").modal()
})

function _send()
{
	let s = $("#message_input").val()
	$("#message_input").val("")
	do_send(s)
}

$("#send_button").click(_send)
$(document).on("keypress", function(e) {
    if(e.which == 13) {
        _send()
    }
})
