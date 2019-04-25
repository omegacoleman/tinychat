const Koa = require("koa")
const Router = require("koa-router")
const BodyParser = require("koa-bodyparser")

const config = require("./server.config.js")

const string_verify = require("./string_verify.js")
const RedisAuthClient = require("./redis_client.js")

const password_auth_func = require("./auth_funcs.js")[config.auth_encryption]

const logger = require("log4js").getLogger()
logger.level = "trace"

let app = new Koa()
let router = new Router()

let redis_client = new RedisAuthClient(config.db.port, config.db.server)

app.use(BodyParser())

router.post("/register", async (ctx, next) =>
    {
        let user = ctx.request.body["user"] || ""
        let password = ctx.request.body["password"] || ""
        logger.info("/register user='" + user + "'")
        let user_v_result = string_verify.verify(user, config.user_v)
        let password_v_result = string_verify.verify(password, config.password_v)
        if (! (user_v_result.good && password_v_result.good) )
        {
            logger.info("/register user='" + user + "' failed string verification")
            ctx.body = {
                ok: false,
                reason: (user_v_result.good ? (
                    "password : " + password_v_result.reason
                ) : (
                    "user : " + user_v_result.reason
                ))
            }
            await next()
            return
        }
        if (await redis_client.tinychat_user_exists(user))
        {
            ctx.body = {
                ok: false,
                reason: "user : name already taken"
            }
            logger.info("/register user='" + user + "' user name taken")
            await next()
            return
        }
        let auth = password_auth_func(password)
        await redis_client.tinychat_register(user, auth)
        logger.info("/register user='" + user + "' seems ok")

        ctx.body = {
            ok : true
        }

        await next()
    })

router.get("/exists", async (ctx, next) =>
    {
        if (! ctx.query["user"])
        {
            await next() // 404 in default
            return
        }
        logger.info("/exists user='" + ctx.query["user"] + "'")
        ctx.body = {
            exists : await redis_client.tinychat_user_exists(ctx.query["user"])
        }
        await next()
    })

router.get("/user_list", async (ctx, next) =>
    {
        logger.info("/user_list")
        ctx.body = {
            user_list : await redis_client.tinychat_user_list()
        }
        await next()
    })

app.use(router.routes())
app.use(router.allowedMethods())

app.listen(config.port, config.host, () =>
    {
        logger.info("Listening on " + config.host + " port " + config.port)
    })

