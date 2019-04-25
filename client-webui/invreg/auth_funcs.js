
const crypto = require("crypto")

let supersecretstring = "ssssplrplrplrplr"

module.exports = {
    "sha-256" : input => crypto.createHmac("sha256", supersecretstring).update(input).digest("hex")
}

