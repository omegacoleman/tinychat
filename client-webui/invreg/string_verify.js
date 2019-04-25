const logger = require("log4js").getLogger()

let restrict_funcs = {
    length_range(target_string, params)
    {
        if ((target_string.length < params.min) || (target_string.length > params.max))
        {
            return false
        } else {
            return true
        }
    },
    length_range_doc(params)
    {
        return "Should have a length between " + params.min + " and " + params.max
    },

    not_empty(target_string, params)
    {
        if (target_string == "")
        {
            return false
        }
        return true
    },
    not_empty_doc(params)
    {
        return "shoudn't be empty"
    },

    consist_of_base(target_string, params)
    {
        let s_chars = target_string.split("")
        let s_uniq_chars = {}
        s_chars.forEach(c => {
            s_uniq_chars[c] = true
        })
        if(params.mode == "whitelist")
        {
            for (let c in s_uniq_chars)
            {
                if (params.allowed.indexOf(c) == -1)
                {
                    return false
                }
            }
        } else {
            for (let c in s_uniq_chars)
            {
                if (params.disallowed.indexOf(c) != -1)
                {
                    return false
                }
            }
        }
        return true
    },
    consist_of_base_doc(params)
    {
        if (params.mode == "whitelist")
        {
            return "Shall consist of following chars : " + params.allowed
        } else {
            return "Shoud not contain following chars : " + params.disallowed
        }
    },

    consist_of(target_string, params)
    {
        let expand = (char_classes) => {
            let ret = ""
            char_classes.forEach((char_class) =>
                {
                    switch(char_class)
                    {
                        case "upper_letter" :
                            ret += "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            break
                        case "lower_letter" :
                            ret += "abcdefghijklmnopqrstuvwxyz"
                            break
                        case "number" :
                            ret += "1234567890"
                            break
                        default :
                    }
                }
            )
            return ret
        }
        params.allowed = ""
        params.disallowed = ""
        if (params.allowed_classes)
        {
            params.allowed = "" + (params.allowed_chars || "") + expand(params.allowed_classes)
        }
        if (params.disallowed_classes)
        {
            params.disallowed = "" + (params.disallowed_chars || "") + expand(params.disallowed_classes)
        }
        return restrict_funcs.consist_of_base(target_string, params)
    },
    consist_of_doc(params)
    {
        let expand_doc = (char_classes) => {
            let ret = []
            char_classes.forEach((char_class) =>
            {
                switch (char_class)
                {
                    case "upper_letter" :
                        ret.push("upper-case letters")
                        break
                    case "lower_letter" :
                        ret.push("lower-case letters")
                        break
                    case "number" :
                        ret.push("numeric digits")
                        break
                    default :
                }
            })
            return ret.join(", ")
        }
        if(params.mode == "whitelist")
        {
            return "Should only consist of " + (
                params.allowed_classes ? expand_doc(params.allowed_classes) : "") + (
                    params.allowed_chars ? (" and " + params.allowed_chars) : ""
                )
        } else {
            return "Should not contain any of " + (
                params.disallowed_classes ? expand_doc(params.disallowed_classes) : "") + (
                    params.disallowed_chars ? (" and " + params.disallowed_chars) : ""
                )
        }
    }
}

let verify = function (target_string, options)
{
    for (let [restrict, params] of Object.entries(options))
    {
        if (restrict_funcs.hasOwnProperty(restrict))
        {
            if (! restrict_funcs[restrict](target_string, params))
            {
                return { good : false,
                    reason : restrict_funcs[restrict + "_doc"](params)
                }
            }
        } else {
            logger.error("string verify : unsupported restrict " + restrict)
        }
    }
    return { good : true }
}

module.exports.restrict_funcs = restrict_funcs
module.exports.verify = verify

/**
console.log(verify(process.argv[2] || "",
    {
        "not_empty": {},
        "length_range": {
            min: 6,
            max: 14
        },
        "consist_of": {
            mode: "whitelist",
            allowed_chars: "-_",
            allowed_classes: [
                "upper_letter",
                "lower_letter",
                "number"
            ]
        }
    }
))
**/

