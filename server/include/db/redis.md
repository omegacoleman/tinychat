REDIS database scheme
---------------------

## key : users

A hashset, with all users' name:auth pair stored.

sample:

```
> HGETALL users
1) "youcai"
2) "66666"
3) "orange"
4) "77777"
5) "papaya"
6) "88888"
```

## key : user:banned

A set, with all users banned from logging in.

## key : log_by_unix_time

A sorted set, with message unix_time as scores & message id as items.

sample:

```
127.0.0.1:6379> zrange log_by_unix_time 0 -1 withscores
 1) "80ea4185-e02d-4338-855c-0ef5d4b6f4ee"
 2) "1554994066"
 3) "b45bfbf4-91ef-428e-ad80-beb513ad5874"
 4) "1554994722"
 5) "d7e6d69a-72df-488c-a4f2-6a25094ae589"
 6) "1554994722"
 7) "1ff764dc-e4f0-4b34-8776-41b0267631bf"
 8) "1554994723"
...
```

## subscription : tinyrpc

### publish message : "user_update"

Do this after you make some changes to "users" key. Currently only user addition is supported.

### publish message : "user_ban"

Do this after you banned someone. Currently the ban is permanent.

## message logs

Message logs are stored in database like normal objects. Their 'fields' are stored with key "id:field_name" as strings.

There's one exception : the message text itself, is stored with key "id" with no suffix.

like:
```
"6cbcc2a6-8d9c-4957-aaaf-fc7c41bf7197" -> "hello!"
"6cbcc2a6-8d9c-4957-aaaf-fc7c41bf7197:sender" -> "youcai"
"6cbcc2a6-8d9c-4957-aaaf-fc7c41bf7197:unix_time" -> "1554998831"
```

Important notice : currently we just let id be UUID, future we may reshape it to include something to indentify the
chatroom. This is for multi process to share the same DB.


