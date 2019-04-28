
const crypto = require("crypto")

module.exports = {
    "sha-256" : input => crypto.createHash("sha256").update(input).digest("hex")
}

