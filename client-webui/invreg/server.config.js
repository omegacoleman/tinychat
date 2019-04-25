module.exports = {
    host : "0.0.0.0",
    port : 8080,
    db: {
        server : "127.0.0.1",
        port : 6379
    },
    user_v: {
        not_empty: {},
        length_range: {min : 6, max : 20},
        consist_of: {
            allowed_chars: "-_",
            allowed_classes: [
                "upper_letter",
                "lower_letter",
                "number"
            ]
        }
    },
    password_v: {
        not_empty: {},
        length_range: {min: 8, max: 30}
    },
    auth_encryption : "sha-256",
    use_invitation : true
}

