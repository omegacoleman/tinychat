const Redis = require("ioredis")
const logger = require("log4js").getLogger()

class RedisAuthClient extends Redis
{
    async tinychat_signal(signal)
    {
        let ret = await this.publish("tinychat", signal)
        logger.debug("tinychat_signal('" + signal + "') : ret=" + ret)
    }

    async tinychat_register(user, auth)
    {
        let ret = await this.hset("users", user, auth)
        logger.debug("tinychat_register('" + user + "', ..) : ret=" + ret)
        await this.tinychat_signal("user_update")
    }

    async tinychat_user_exists(user)
    {
        return ((await this.hexists("users", user)) == 1)
    }

    async tinychat_user_list()
    {
        return (await this.hkeys("users"))
    }
}

module.exports = RedisAuthClient

