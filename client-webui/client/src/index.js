
const RPCSession = require("./browser-client.js")
const crypto = require("crypto")
const moment = require("moment")
const escapeHtml = require('escape-html')

let session

let config = {
	password_no_sha256: false, 
	lock_uri: ""
}

window.init_webui_config = function (_config)
{
	if (_config)
	{
		config = _config
	}
	if (config.lock_uri)
	{
		$("#server_input").val(config.lock_uri);
		$("#server_input").attr("disabled", true)
	}
}

//////////////////

function show_message(msg)
{
    console.log(msg)
    let my_name = ""
    if(session)
    {
    	my_name = session.login_info.name || ""
    }
    let sent_m = (my_name == msg.sender)
    $("#messages").append(
			"<div class=\"" + (sent_m ? "sent-" : "") + "message\">" + 
				"<p class=\"sender\">" + escapeHtml(msg.sender) + "</p>" + 
				"<div class=\"text\">" + 
					"<div>" + escapeHtml(msg.text) + "</div>" + 
				"</div>" + 
				"<p class=\"time\">" + moment.unix(msg.unixTime).format("HH:mm:ss") + "</p>" + 
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
        auth: config.password_no_sha256 ? password : crypto.createHash("sha256").update(password).digest("hex")
    }))
}

function do_revise()
{
	return session.get_log().then(
		mlist => {
			console.log(mlist)
			for(let i in mlist)
			{
				let it = mlist[i]
				show_message(it)
			}
		})
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
	$("#login_modal").modal('hide')
    $("#history_message").scrollTop($("#history_message")[0].scrollHeight);
	$("#message_input").attr("disabled", false)
	$("#send_button").attr("disabled", false)
	$("#logout_button").attr("disabled", false)
}

function logout_ok()
{
	$("#login_info").show()
	$("#messages").hide()
	$("#messages").html("")
	$("#message_input").attr("disabled", true)
	$("#send_button").attr("disabled", true)
	$("#logout_button").attr("disabled", true)
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
		).then(do_revise).then( () => {
			session.onclose(() => {
				session = null
				logout_ok()
			})
			login_ok()
		}).catch( reason => {
			console.log(reason)
			login_modal_err_msg(reason)
		})
})

$("#logout_button").click(() => {
	session.close()
	session = null
	logout_ok()
})

$().ready(() => {
	logout_ok()
	$("#login_modal").modal()
})

function _send()
{
	let s = $("#message_input").val()
	if(! s)
	{
		return
	}
	$("#message_input").val("")
	do_send(s)
}

$("#send_button").click(_send)
$(document).on("keypress", function(e) {
    if(e.which == 13) {
        _send()
    }
})
