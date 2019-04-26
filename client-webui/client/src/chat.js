/*eslint-disable block-scoped-var, id-length, no-control-regex, no-magic-numbers, no-prototype-builtins, no-redeclare, no-shadow, no-var, sort-vars*/
"use strict";

var $protobuf = require("protobufjs/minimal");

// Common aliases
var $Reader = $protobuf.Reader, $Writer = $protobuf.Writer, $util = $protobuf.util;

// Exported root namespace
var $root = $protobuf.roots["default"] || ($protobuf.roots["default"] = {});

$root.chat = (function() {

    /**
     * Namespace chat.
     * @exports chat
     * @namespace
     */
    var chat = {};

    chat.LoginRequest = (function() {

        /**
         * Properties of a LoginRequest.
         * @memberof chat
         * @interface ILoginRequest
         * @property {string|null} [name] LoginRequest name
         * @property {Uint8Array|null} [auth] LoginRequest auth
         */

        /**
         * Constructs a new LoginRequest.
         * @memberof chat
         * @classdesc Represents a LoginRequest.
         * @implements ILoginRequest
         * @constructor
         * @param {chat.ILoginRequest=} [properties] Properties to set
         */
        function LoginRequest(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * LoginRequest name.
         * @member {string} name
         * @memberof chat.LoginRequest
         * @instance
         */
        LoginRequest.prototype.name = "";

        /**
         * LoginRequest auth.
         * @member {Uint8Array} auth
         * @memberof chat.LoginRequest
         * @instance
         */
        LoginRequest.prototype.auth = $util.newBuffer([]);

        /**
         * Creates a new LoginRequest instance using the specified properties.
         * @function create
         * @memberof chat.LoginRequest
         * @static
         * @param {chat.ILoginRequest=} [properties] Properties to set
         * @returns {chat.LoginRequest} LoginRequest instance
         */
        LoginRequest.create = function create(properties) {
            return new LoginRequest(properties);
        };

        /**
         * Encodes the specified LoginRequest message. Does not implicitly {@link chat.LoginRequest.verify|verify} messages.
         * @function encode
         * @memberof chat.LoginRequest
         * @static
         * @param {chat.ILoginRequest} message LoginRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        LoginRequest.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.name != null && message.hasOwnProperty("name"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.name);
            if (message.auth != null && message.hasOwnProperty("auth"))
                writer.uint32(/* id 2, wireType 2 =*/18).bytes(message.auth);
            return writer;
        };

        /**
         * Encodes the specified LoginRequest message, length delimited. Does not implicitly {@link chat.LoginRequest.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.LoginRequest
         * @static
         * @param {chat.ILoginRequest} message LoginRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        LoginRequest.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a LoginRequest message from the specified reader or buffer.
         * @function decode
         * @memberof chat.LoginRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.LoginRequest} LoginRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        LoginRequest.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.LoginRequest();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.name = reader.string();
                    break;
                case 2:
                    message.auth = reader.bytes();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a LoginRequest message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.LoginRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.LoginRequest} LoginRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        LoginRequest.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a LoginRequest message.
         * @function verify
         * @memberof chat.LoginRequest
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        LoginRequest.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.name != null && message.hasOwnProperty("name"))
                if (!$util.isString(message.name))
                    return "name: string expected";
            if (message.auth != null && message.hasOwnProperty("auth"))
                if (!(message.auth && typeof message.auth.length === "number" || $util.isString(message.auth)))
                    return "auth: buffer expected";
            return null;
        };

        /**
         * Creates a LoginRequest message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.LoginRequest
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.LoginRequest} LoginRequest
         */
        LoginRequest.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.LoginRequest)
                return object;
            var message = new $root.chat.LoginRequest();
            if (object.name != null)
                message.name = String(object.name);
            if (object.auth != null)
                if (typeof object.auth === "string")
                    $util.base64.decode(object.auth, message.auth = $util.newBuffer($util.base64.length(object.auth)), 0);
                else if (object.auth.length)
                    message.auth = object.auth;
            return message;
        };

        /**
         * Creates a plain object from a LoginRequest message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.LoginRequest
         * @static
         * @param {chat.LoginRequest} message LoginRequest
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        LoginRequest.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.name = "";
                if (options.bytes === String)
                    object.auth = "";
                else {
                    object.auth = [];
                    if (options.bytes !== Array)
                        object.auth = $util.newBuffer(object.auth);
                }
            }
            if (message.name != null && message.hasOwnProperty("name"))
                object.name = message.name;
            if (message.auth != null && message.hasOwnProperty("auth"))
                object.auth = options.bytes === String ? $util.base64.encode(message.auth, 0, message.auth.length) : options.bytes === Array ? Array.prototype.slice.call(message.auth) : message.auth;
            return object;
        };

        /**
         * Converts this LoginRequest to JSON.
         * @function toJSON
         * @memberof chat.LoginRequest
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        LoginRequest.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return LoginRequest;
    })();

    chat.LoginReply = (function() {

        /**
         * Properties of a LoginReply.
         * @memberof chat
         * @interface ILoginReply
         * @property {chat.LoginReply.statetype|null} [state] LoginReply state
         * @property {Uint8Array|null} [token] LoginReply token
         */

        /**
         * Constructs a new LoginReply.
         * @memberof chat
         * @classdesc Represents a LoginReply.
         * @implements ILoginReply
         * @constructor
         * @param {chat.ILoginReply=} [properties] Properties to set
         */
        function LoginReply(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * LoginReply state.
         * @member {chat.LoginReply.statetype} state
         * @memberof chat.LoginReply
         * @instance
         */
        LoginReply.prototype.state = 0;

        /**
         * LoginReply token.
         * @member {Uint8Array} token
         * @memberof chat.LoginReply
         * @instance
         */
        LoginReply.prototype.token = $util.newBuffer([]);

        /**
         * Creates a new LoginReply instance using the specified properties.
         * @function create
         * @memberof chat.LoginReply
         * @static
         * @param {chat.ILoginReply=} [properties] Properties to set
         * @returns {chat.LoginReply} LoginReply instance
         */
        LoginReply.create = function create(properties) {
            return new LoginReply(properties);
        };

        /**
         * Encodes the specified LoginReply message. Does not implicitly {@link chat.LoginReply.verify|verify} messages.
         * @function encode
         * @memberof chat.LoginReply
         * @static
         * @param {chat.ILoginReply} message LoginReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        LoginReply.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.state != null && message.hasOwnProperty("state"))
                writer.uint32(/* id 1, wireType 0 =*/8).int32(message.state);
            if (message.token != null && message.hasOwnProperty("token"))
                writer.uint32(/* id 2, wireType 2 =*/18).bytes(message.token);
            return writer;
        };

        /**
         * Encodes the specified LoginReply message, length delimited. Does not implicitly {@link chat.LoginReply.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.LoginReply
         * @static
         * @param {chat.ILoginReply} message LoginReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        LoginReply.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a LoginReply message from the specified reader or buffer.
         * @function decode
         * @memberof chat.LoginReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.LoginReply} LoginReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        LoginReply.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.LoginReply();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.state = reader.int32();
                    break;
                case 2:
                    message.token = reader.bytes();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a LoginReply message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.LoginReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.LoginReply} LoginReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        LoginReply.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a LoginReply message.
         * @function verify
         * @memberof chat.LoginReply
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        LoginReply.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.state != null && message.hasOwnProperty("state"))
                switch (message.state) {
                default:
                    return "state: enum value expected";
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    break;
                }
            if (message.token != null && message.hasOwnProperty("token"))
                if (!(message.token && typeof message.token.length === "number" || $util.isString(message.token)))
                    return "token: buffer expected";
            return null;
        };

        /**
         * Creates a LoginReply message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.LoginReply
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.LoginReply} LoginReply
         */
        LoginReply.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.LoginReply)
                return object;
            var message = new $root.chat.LoginReply();
            switch (object.state) {
            case "ok":
            case 0:
                message.state = 0;
                break;
            case "auth_failed":
            case 1:
                message.state = 1;
                break;
            case "not_registered":
            case 2:
                message.state = 2;
                break;
            case "duplicate_login":
            case 3:
                message.state = 3;
                break;
            case "error":
            case 4:
                message.state = 4;
                break;
            case "banned":
            case 5:
                message.state = 5;
                break;
            }
            if (object.token != null)
                if (typeof object.token === "string")
                    $util.base64.decode(object.token, message.token = $util.newBuffer($util.base64.length(object.token)), 0);
                else if (object.token.length)
                    message.token = object.token;
            return message;
        };

        /**
         * Creates a plain object from a LoginReply message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.LoginReply
         * @static
         * @param {chat.LoginReply} message LoginReply
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        LoginReply.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.state = options.enums === String ? "ok" : 0;
                if (options.bytes === String)
                    object.token = "";
                else {
                    object.token = [];
                    if (options.bytes !== Array)
                        object.token = $util.newBuffer(object.token);
                }
            }
            if (message.state != null && message.hasOwnProperty("state"))
                object.state = options.enums === String ? $root.chat.LoginReply.statetype[message.state] : message.state;
            if (message.token != null && message.hasOwnProperty("token"))
                object.token = options.bytes === String ? $util.base64.encode(message.token, 0, message.token.length) : options.bytes === Array ? Array.prototype.slice.call(message.token) : message.token;
            return object;
        };

        /**
         * Converts this LoginReply to JSON.
         * @function toJSON
         * @memberof chat.LoginReply
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        LoginReply.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        /**
         * statetype enum.
         * @name chat.LoginReply.statetype
         * @enum {string}
         * @property {number} ok=0 ok value
         * @property {number} auth_failed=1 auth_failed value
         * @property {number} not_registered=2 not_registered value
         * @property {number} duplicate_login=3 duplicate_login value
         * @property {number} error=4 error value
         * @property {number} banned=5 banned value
         */
        LoginReply.statetype = (function() {
            var valuesById = {}, values = Object.create(valuesById);
            values[valuesById[0] = "ok"] = 0;
            values[valuesById[1] = "auth_failed"] = 1;
            values[valuesById[2] = "not_registered"] = 2;
            values[valuesById[3] = "duplicate_login"] = 3;
            values[valuesById[4] = "error"] = 4;
            values[valuesById[5] = "banned"] = 5;
            return values;
        })();

        return LoginReply;
    })();

    chat.ChatSendRequest = (function() {

        /**
         * Properties of a ChatSendRequest.
         * @memberof chat
         * @interface IChatSendRequest
         * @property {string|null} [name] ChatSendRequest name
         * @property {Uint8Array|null} [token] ChatSendRequest token
         * @property {string|null} [text] ChatSendRequest text
         */

        /**
         * Constructs a new ChatSendRequest.
         * @memberof chat
         * @classdesc Represents a ChatSendRequest.
         * @implements IChatSendRequest
         * @constructor
         * @param {chat.IChatSendRequest=} [properties] Properties to set
         */
        function ChatSendRequest(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * ChatSendRequest name.
         * @member {string} name
         * @memberof chat.ChatSendRequest
         * @instance
         */
        ChatSendRequest.prototype.name = "";

        /**
         * ChatSendRequest token.
         * @member {Uint8Array} token
         * @memberof chat.ChatSendRequest
         * @instance
         */
        ChatSendRequest.prototype.token = $util.newBuffer([]);

        /**
         * ChatSendRequest text.
         * @member {string} text
         * @memberof chat.ChatSendRequest
         * @instance
         */
        ChatSendRequest.prototype.text = "";

        /**
         * Creates a new ChatSendRequest instance using the specified properties.
         * @function create
         * @memberof chat.ChatSendRequest
         * @static
         * @param {chat.IChatSendRequest=} [properties] Properties to set
         * @returns {chat.ChatSendRequest} ChatSendRequest instance
         */
        ChatSendRequest.create = function create(properties) {
            return new ChatSendRequest(properties);
        };

        /**
         * Encodes the specified ChatSendRequest message. Does not implicitly {@link chat.ChatSendRequest.verify|verify} messages.
         * @function encode
         * @memberof chat.ChatSendRequest
         * @static
         * @param {chat.IChatSendRequest} message ChatSendRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ChatSendRequest.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.name != null && message.hasOwnProperty("name"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.name);
            if (message.token != null && message.hasOwnProperty("token"))
                writer.uint32(/* id 2, wireType 2 =*/18).bytes(message.token);
            if (message.text != null && message.hasOwnProperty("text"))
                writer.uint32(/* id 3, wireType 2 =*/26).string(message.text);
            return writer;
        };

        /**
         * Encodes the specified ChatSendRequest message, length delimited. Does not implicitly {@link chat.ChatSendRequest.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.ChatSendRequest
         * @static
         * @param {chat.IChatSendRequest} message ChatSendRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ChatSendRequest.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a ChatSendRequest message from the specified reader or buffer.
         * @function decode
         * @memberof chat.ChatSendRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.ChatSendRequest} ChatSendRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ChatSendRequest.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.ChatSendRequest();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.name = reader.string();
                    break;
                case 2:
                    message.token = reader.bytes();
                    break;
                case 3:
                    message.text = reader.string();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a ChatSendRequest message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.ChatSendRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.ChatSendRequest} ChatSendRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ChatSendRequest.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a ChatSendRequest message.
         * @function verify
         * @memberof chat.ChatSendRequest
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        ChatSendRequest.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.name != null && message.hasOwnProperty("name"))
                if (!$util.isString(message.name))
                    return "name: string expected";
            if (message.token != null && message.hasOwnProperty("token"))
                if (!(message.token && typeof message.token.length === "number" || $util.isString(message.token)))
                    return "token: buffer expected";
            if (message.text != null && message.hasOwnProperty("text"))
                if (!$util.isString(message.text))
                    return "text: string expected";
            return null;
        };

        /**
         * Creates a ChatSendRequest message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.ChatSendRequest
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.ChatSendRequest} ChatSendRequest
         */
        ChatSendRequest.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.ChatSendRequest)
                return object;
            var message = new $root.chat.ChatSendRequest();
            if (object.name != null)
                message.name = String(object.name);
            if (object.token != null)
                if (typeof object.token === "string")
                    $util.base64.decode(object.token, message.token = $util.newBuffer($util.base64.length(object.token)), 0);
                else if (object.token.length)
                    message.token = object.token;
            if (object.text != null)
                message.text = String(object.text);
            return message;
        };

        /**
         * Creates a plain object from a ChatSendRequest message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.ChatSendRequest
         * @static
         * @param {chat.ChatSendRequest} message ChatSendRequest
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        ChatSendRequest.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.name = "";
                if (options.bytes === String)
                    object.token = "";
                else {
                    object.token = [];
                    if (options.bytes !== Array)
                        object.token = $util.newBuffer(object.token);
                }
                object.text = "";
            }
            if (message.name != null && message.hasOwnProperty("name"))
                object.name = message.name;
            if (message.token != null && message.hasOwnProperty("token"))
                object.token = options.bytes === String ? $util.base64.encode(message.token, 0, message.token.length) : options.bytes === Array ? Array.prototype.slice.call(message.token) : message.token;
            if (message.text != null && message.hasOwnProperty("text"))
                object.text = message.text;
            return object;
        };

        /**
         * Converts this ChatSendRequest to JSON.
         * @function toJSON
         * @memberof chat.ChatSendRequest
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        ChatSendRequest.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return ChatSendRequest;
    })();

    chat.ChatSendReply = (function() {

        /**
         * Properties of a ChatSendReply.
         * @memberof chat
         * @interface IChatSendReply
         * @property {chat.ChatSendReply.resulttype|null} [result] ChatSendReply result
         */

        /**
         * Constructs a new ChatSendReply.
         * @memberof chat
         * @classdesc Represents a ChatSendReply.
         * @implements IChatSendReply
         * @constructor
         * @param {chat.IChatSendReply=} [properties] Properties to set
         */
        function ChatSendReply(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * ChatSendReply result.
         * @member {chat.ChatSendReply.resulttype} result
         * @memberof chat.ChatSendReply
         * @instance
         */
        ChatSendReply.prototype.result = 0;

        /**
         * Creates a new ChatSendReply instance using the specified properties.
         * @function create
         * @memberof chat.ChatSendReply
         * @static
         * @param {chat.IChatSendReply=} [properties] Properties to set
         * @returns {chat.ChatSendReply} ChatSendReply instance
         */
        ChatSendReply.create = function create(properties) {
            return new ChatSendReply(properties);
        };

        /**
         * Encodes the specified ChatSendReply message. Does not implicitly {@link chat.ChatSendReply.verify|verify} messages.
         * @function encode
         * @memberof chat.ChatSendReply
         * @static
         * @param {chat.IChatSendReply} message ChatSendReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ChatSendReply.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.result != null && message.hasOwnProperty("result"))
                writer.uint32(/* id 1, wireType 0 =*/8).int32(message.result);
            return writer;
        };

        /**
         * Encodes the specified ChatSendReply message, length delimited. Does not implicitly {@link chat.ChatSendReply.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.ChatSendReply
         * @static
         * @param {chat.IChatSendReply} message ChatSendReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ChatSendReply.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a ChatSendReply message from the specified reader or buffer.
         * @function decode
         * @memberof chat.ChatSendReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.ChatSendReply} ChatSendReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ChatSendReply.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.ChatSendReply();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.result = reader.int32();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a ChatSendReply message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.ChatSendReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.ChatSendReply} ChatSendReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ChatSendReply.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a ChatSendReply message.
         * @function verify
         * @memberof chat.ChatSendReply
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        ChatSendReply.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.result != null && message.hasOwnProperty("result"))
                switch (message.result) {
                default:
                    return "result: enum value expected";
                case 0:
                case 1:
                case 2:
                    break;
                }
            return null;
        };

        /**
         * Creates a ChatSendReply message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.ChatSendReply
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.ChatSendReply} ChatSendReply
         */
        ChatSendReply.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.ChatSendReply)
                return object;
            var message = new $root.chat.ChatSendReply();
            switch (object.result) {
            case "ok":
            case 0:
                message.result = 0;
                break;
            case "bad_words":
            case 1:
                message.result = 1;
                break;
            case "error":
            case 2:
                message.result = 2;
                break;
            }
            return message;
        };

        /**
         * Creates a plain object from a ChatSendReply message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.ChatSendReply
         * @static
         * @param {chat.ChatSendReply} message ChatSendReply
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        ChatSendReply.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults)
                object.result = options.enums === String ? "ok" : 0;
            if (message.result != null && message.hasOwnProperty("result"))
                object.result = options.enums === String ? $root.chat.ChatSendReply.resulttype[message.result] : message.result;
            return object;
        };

        /**
         * Converts this ChatSendReply to JSON.
         * @function toJSON
         * @memberof chat.ChatSendReply
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        ChatSendReply.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        /**
         * resulttype enum.
         * @name chat.ChatSendReply.resulttype
         * @enum {string}
         * @property {number} ok=0 ok value
         * @property {number} bad_words=1 bad_words value
         * @property {number} error=2 error value
         */
        ChatSendReply.resulttype = (function() {
            var valuesById = {}, values = Object.create(valuesById);
            values[valuesById[0] = "ok"] = 0;
            values[valuesById[1] = "bad_words"] = 1;
            values[valuesById[2] = "error"] = 2;
            return values;
        })();

        return ChatSendReply;
    })();

    chat.ChatMessage = (function() {

        /**
         * Properties of a ChatMessage.
         * @memberof chat
         * @interface IChatMessage
         * @property {Uint8Array|null} [id] ChatMessage id
         * @property {string|null} [sender] ChatMessage sender
         * @property {string|null} [text] ChatMessage text
         * @property {number|Long|null} [unixTime] ChatMessage unixTime
         */

        /**
         * Constructs a new ChatMessage.
         * @memberof chat
         * @classdesc Represents a ChatMessage.
         * @implements IChatMessage
         * @constructor
         * @param {chat.IChatMessage=} [properties] Properties to set
         */
        function ChatMessage(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * ChatMessage id.
         * @member {Uint8Array} id
         * @memberof chat.ChatMessage
         * @instance
         */
        ChatMessage.prototype.id = $util.newBuffer([]);

        /**
         * ChatMessage sender.
         * @member {string} sender
         * @memberof chat.ChatMessage
         * @instance
         */
        ChatMessage.prototype.sender = "";

        /**
         * ChatMessage text.
         * @member {string} text
         * @memberof chat.ChatMessage
         * @instance
         */
        ChatMessage.prototype.text = "";

        /**
         * ChatMessage unixTime.
         * @member {number|Long} unixTime
         * @memberof chat.ChatMessage
         * @instance
         */
        ChatMessage.prototype.unixTime = $util.Long ? $util.Long.fromBits(0,0,true) : 0;

        /**
         * Creates a new ChatMessage instance using the specified properties.
         * @function create
         * @memberof chat.ChatMessage
         * @static
         * @param {chat.IChatMessage=} [properties] Properties to set
         * @returns {chat.ChatMessage} ChatMessage instance
         */
        ChatMessage.create = function create(properties) {
            return new ChatMessage(properties);
        };

        /**
         * Encodes the specified ChatMessage message. Does not implicitly {@link chat.ChatMessage.verify|verify} messages.
         * @function encode
         * @memberof chat.ChatMessage
         * @static
         * @param {chat.IChatMessage} message ChatMessage message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ChatMessage.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.id != null && message.hasOwnProperty("id"))
                writer.uint32(/* id 1, wireType 2 =*/10).bytes(message.id);
            if (message.sender != null && message.hasOwnProperty("sender"))
                writer.uint32(/* id 2, wireType 2 =*/18).string(message.sender);
            if (message.text != null && message.hasOwnProperty("text"))
                writer.uint32(/* id 3, wireType 2 =*/26).string(message.text);
            if (message.unixTime != null && message.hasOwnProperty("unixTime"))
                writer.uint32(/* id 4, wireType 0 =*/32).uint64(message.unixTime);
            return writer;
        };

        /**
         * Encodes the specified ChatMessage message, length delimited. Does not implicitly {@link chat.ChatMessage.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.ChatMessage
         * @static
         * @param {chat.IChatMessage} message ChatMessage message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ChatMessage.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a ChatMessage message from the specified reader or buffer.
         * @function decode
         * @memberof chat.ChatMessage
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.ChatMessage} ChatMessage
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ChatMessage.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.ChatMessage();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.id = reader.bytes();
                    break;
                case 2:
                    message.sender = reader.string();
                    break;
                case 3:
                    message.text = reader.string();
                    break;
                case 4:
                    message.unixTime = reader.uint64();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a ChatMessage message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.ChatMessage
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.ChatMessage} ChatMessage
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ChatMessage.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a ChatMessage message.
         * @function verify
         * @memberof chat.ChatMessage
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        ChatMessage.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.id != null && message.hasOwnProperty("id"))
                if (!(message.id && typeof message.id.length === "number" || $util.isString(message.id)))
                    return "id: buffer expected";
            if (message.sender != null && message.hasOwnProperty("sender"))
                if (!$util.isString(message.sender))
                    return "sender: string expected";
            if (message.text != null && message.hasOwnProperty("text"))
                if (!$util.isString(message.text))
                    return "text: string expected";
            if (message.unixTime != null && message.hasOwnProperty("unixTime"))
                if (!$util.isInteger(message.unixTime) && !(message.unixTime && $util.isInteger(message.unixTime.low) && $util.isInteger(message.unixTime.high)))
                    return "unixTime: integer|Long expected";
            return null;
        };

        /**
         * Creates a ChatMessage message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.ChatMessage
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.ChatMessage} ChatMessage
         */
        ChatMessage.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.ChatMessage)
                return object;
            var message = new $root.chat.ChatMessage();
            if (object.id != null)
                if (typeof object.id === "string")
                    $util.base64.decode(object.id, message.id = $util.newBuffer($util.base64.length(object.id)), 0);
                else if (object.id.length)
                    message.id = object.id;
            if (object.sender != null)
                message.sender = String(object.sender);
            if (object.text != null)
                message.text = String(object.text);
            if (object.unixTime != null)
                if ($util.Long)
                    (message.unixTime = $util.Long.fromValue(object.unixTime)).unsigned = true;
                else if (typeof object.unixTime === "string")
                    message.unixTime = parseInt(object.unixTime, 10);
                else if (typeof object.unixTime === "number")
                    message.unixTime = object.unixTime;
                else if (typeof object.unixTime === "object")
                    message.unixTime = new $util.LongBits(object.unixTime.low >>> 0, object.unixTime.high >>> 0).toNumber(true);
            return message;
        };

        /**
         * Creates a plain object from a ChatMessage message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.ChatMessage
         * @static
         * @param {chat.ChatMessage} message ChatMessage
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        ChatMessage.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                if (options.bytes === String)
                    object.id = "";
                else {
                    object.id = [];
                    if (options.bytes !== Array)
                        object.id = $util.newBuffer(object.id);
                }
                object.sender = "";
                object.text = "";
                if ($util.Long) {
                    var long = new $util.Long(0, 0, true);
                    object.unixTime = options.longs === String ? long.toString() : options.longs === Number ? long.toNumber() : long;
                } else
                    object.unixTime = options.longs === String ? "0" : 0;
            }
            if (message.id != null && message.hasOwnProperty("id"))
                object.id = options.bytes === String ? $util.base64.encode(message.id, 0, message.id.length) : options.bytes === Array ? Array.prototype.slice.call(message.id) : message.id;
            if (message.sender != null && message.hasOwnProperty("sender"))
                object.sender = message.sender;
            if (message.text != null && message.hasOwnProperty("text"))
                object.text = message.text;
            if (message.unixTime != null && message.hasOwnProperty("unixTime"))
                if (typeof message.unixTime === "number")
                    object.unixTime = options.longs === String ? String(message.unixTime) : message.unixTime;
                else
                    object.unixTime = options.longs === String ? $util.Long.prototype.toString.call(message.unixTime) : options.longs === Number ? new $util.LongBits(message.unixTime.low >>> 0, message.unixTime.high >>> 0).toNumber(true) : message.unixTime;
            return object;
        };

        /**
         * Converts this ChatMessage to JSON.
         * @function toJSON
         * @memberof chat.ChatMessage
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        ChatMessage.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return ChatMessage;
    })();

    chat.NotifyChatMessageRequest = (function() {

        /**
         * Properties of a NotifyChatMessageRequest.
         * @memberof chat
         * @interface INotifyChatMessageRequest
         * @property {chat.IChatMessage|null} [chatMessage] NotifyChatMessageRequest chatMessage
         */

        /**
         * Constructs a new NotifyChatMessageRequest.
         * @memberof chat
         * @classdesc Represents a NotifyChatMessageRequest.
         * @implements INotifyChatMessageRequest
         * @constructor
         * @param {chat.INotifyChatMessageRequest=} [properties] Properties to set
         */
        function NotifyChatMessageRequest(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * NotifyChatMessageRequest chatMessage.
         * @member {chat.IChatMessage|null|undefined} chatMessage
         * @memberof chat.NotifyChatMessageRequest
         * @instance
         */
        NotifyChatMessageRequest.prototype.chatMessage = null;

        /**
         * Creates a new NotifyChatMessageRequest instance using the specified properties.
         * @function create
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {chat.INotifyChatMessageRequest=} [properties] Properties to set
         * @returns {chat.NotifyChatMessageRequest} NotifyChatMessageRequest instance
         */
        NotifyChatMessageRequest.create = function create(properties) {
            return new NotifyChatMessageRequest(properties);
        };

        /**
         * Encodes the specified NotifyChatMessageRequest message. Does not implicitly {@link chat.NotifyChatMessageRequest.verify|verify} messages.
         * @function encode
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {chat.INotifyChatMessageRequest} message NotifyChatMessageRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        NotifyChatMessageRequest.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.chatMessage != null && message.hasOwnProperty("chatMessage"))
                $root.chat.ChatMessage.encode(message.chatMessage, writer.uint32(/* id 1, wireType 2 =*/10).fork()).ldelim();
            return writer;
        };

        /**
         * Encodes the specified NotifyChatMessageRequest message, length delimited. Does not implicitly {@link chat.NotifyChatMessageRequest.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {chat.INotifyChatMessageRequest} message NotifyChatMessageRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        NotifyChatMessageRequest.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a NotifyChatMessageRequest message from the specified reader or buffer.
         * @function decode
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.NotifyChatMessageRequest} NotifyChatMessageRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        NotifyChatMessageRequest.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.NotifyChatMessageRequest();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.chatMessage = $root.chat.ChatMessage.decode(reader, reader.uint32());
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a NotifyChatMessageRequest message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.NotifyChatMessageRequest} NotifyChatMessageRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        NotifyChatMessageRequest.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a NotifyChatMessageRequest message.
         * @function verify
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        NotifyChatMessageRequest.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.chatMessage != null && message.hasOwnProperty("chatMessage")) {
                var error = $root.chat.ChatMessage.verify(message.chatMessage);
                if (error)
                    return "chatMessage." + error;
            }
            return null;
        };

        /**
         * Creates a NotifyChatMessageRequest message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.NotifyChatMessageRequest} NotifyChatMessageRequest
         */
        NotifyChatMessageRequest.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.NotifyChatMessageRequest)
                return object;
            var message = new $root.chat.NotifyChatMessageRequest();
            if (object.chatMessage != null) {
                if (typeof object.chatMessage !== "object")
                    throw TypeError(".chat.NotifyChatMessageRequest.chatMessage: object expected");
                message.chatMessage = $root.chat.ChatMessage.fromObject(object.chatMessage);
            }
            return message;
        };

        /**
         * Creates a plain object from a NotifyChatMessageRequest message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.NotifyChatMessageRequest
         * @static
         * @param {chat.NotifyChatMessageRequest} message NotifyChatMessageRequest
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        NotifyChatMessageRequest.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults)
                object.chatMessage = null;
            if (message.chatMessage != null && message.hasOwnProperty("chatMessage"))
                object.chatMessage = $root.chat.ChatMessage.toObject(message.chatMessage, options);
            return object;
        };

        /**
         * Converts this NotifyChatMessageRequest to JSON.
         * @function toJSON
         * @memberof chat.NotifyChatMessageRequest
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        NotifyChatMessageRequest.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return NotifyChatMessageRequest;
    })();

    chat.NotifyChatMessageReply = (function() {

        /**
         * Properties of a NotifyChatMessageReply.
         * @memberof chat
         * @interface INotifyChatMessageReply
         */

        /**
         * Constructs a new NotifyChatMessageReply.
         * @memberof chat
         * @classdesc Represents a NotifyChatMessageReply.
         * @implements INotifyChatMessageReply
         * @constructor
         * @param {chat.INotifyChatMessageReply=} [properties] Properties to set
         */
        function NotifyChatMessageReply(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * Creates a new NotifyChatMessageReply instance using the specified properties.
         * @function create
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {chat.INotifyChatMessageReply=} [properties] Properties to set
         * @returns {chat.NotifyChatMessageReply} NotifyChatMessageReply instance
         */
        NotifyChatMessageReply.create = function create(properties) {
            return new NotifyChatMessageReply(properties);
        };

        /**
         * Encodes the specified NotifyChatMessageReply message. Does not implicitly {@link chat.NotifyChatMessageReply.verify|verify} messages.
         * @function encode
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {chat.INotifyChatMessageReply} message NotifyChatMessageReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        NotifyChatMessageReply.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            return writer;
        };

        /**
         * Encodes the specified NotifyChatMessageReply message, length delimited. Does not implicitly {@link chat.NotifyChatMessageReply.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {chat.INotifyChatMessageReply} message NotifyChatMessageReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        NotifyChatMessageReply.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a NotifyChatMessageReply message from the specified reader or buffer.
         * @function decode
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.NotifyChatMessageReply} NotifyChatMessageReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        NotifyChatMessageReply.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.NotifyChatMessageReply();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a NotifyChatMessageReply message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.NotifyChatMessageReply} NotifyChatMessageReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        NotifyChatMessageReply.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a NotifyChatMessageReply message.
         * @function verify
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        NotifyChatMessageReply.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            return null;
        };

        /**
         * Creates a NotifyChatMessageReply message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.NotifyChatMessageReply} NotifyChatMessageReply
         */
        NotifyChatMessageReply.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.NotifyChatMessageReply)
                return object;
            return new $root.chat.NotifyChatMessageReply();
        };

        /**
         * Creates a plain object from a NotifyChatMessageReply message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.NotifyChatMessageReply
         * @static
         * @param {chat.NotifyChatMessageReply} message NotifyChatMessageReply
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        NotifyChatMessageReply.toObject = function toObject() {
            return {};
        };

        /**
         * Converts this NotifyChatMessageReply to JSON.
         * @function toJSON
         * @memberof chat.NotifyChatMessageReply
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        NotifyChatMessageReply.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return NotifyChatMessageReply;
    })();

    chat.GetLogRequest = (function() {

        /**
         * Properties of a GetLogRequest.
         * @memberof chat
         * @interface IGetLogRequest
         * @property {string|null} [name] GetLogRequest name
         * @property {Uint8Array|null} [token] GetLogRequest token
         */

        /**
         * Constructs a new GetLogRequest.
         * @memberof chat
         * @classdesc Represents a GetLogRequest.
         * @implements IGetLogRequest
         * @constructor
         * @param {chat.IGetLogRequest=} [properties] Properties to set
         */
        function GetLogRequest(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * GetLogRequest name.
         * @member {string} name
         * @memberof chat.GetLogRequest
         * @instance
         */
        GetLogRequest.prototype.name = "";

        /**
         * GetLogRequest token.
         * @member {Uint8Array} token
         * @memberof chat.GetLogRequest
         * @instance
         */
        GetLogRequest.prototype.token = $util.newBuffer([]);

        /**
         * Creates a new GetLogRequest instance using the specified properties.
         * @function create
         * @memberof chat.GetLogRequest
         * @static
         * @param {chat.IGetLogRequest=} [properties] Properties to set
         * @returns {chat.GetLogRequest} GetLogRequest instance
         */
        GetLogRequest.create = function create(properties) {
            return new GetLogRequest(properties);
        };

        /**
         * Encodes the specified GetLogRequest message. Does not implicitly {@link chat.GetLogRequest.verify|verify} messages.
         * @function encode
         * @memberof chat.GetLogRequest
         * @static
         * @param {chat.IGetLogRequest} message GetLogRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        GetLogRequest.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.name != null && message.hasOwnProperty("name"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.name);
            if (message.token != null && message.hasOwnProperty("token"))
                writer.uint32(/* id 2, wireType 2 =*/18).bytes(message.token);
            return writer;
        };

        /**
         * Encodes the specified GetLogRequest message, length delimited. Does not implicitly {@link chat.GetLogRequest.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.GetLogRequest
         * @static
         * @param {chat.IGetLogRequest} message GetLogRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        GetLogRequest.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a GetLogRequest message from the specified reader or buffer.
         * @function decode
         * @memberof chat.GetLogRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.GetLogRequest} GetLogRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        GetLogRequest.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.GetLogRequest();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.name = reader.string();
                    break;
                case 2:
                    message.token = reader.bytes();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a GetLogRequest message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.GetLogRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.GetLogRequest} GetLogRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        GetLogRequest.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a GetLogRequest message.
         * @function verify
         * @memberof chat.GetLogRequest
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        GetLogRequest.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.name != null && message.hasOwnProperty("name"))
                if (!$util.isString(message.name))
                    return "name: string expected";
            if (message.token != null && message.hasOwnProperty("token"))
                if (!(message.token && typeof message.token.length === "number" || $util.isString(message.token)))
                    return "token: buffer expected";
            return null;
        };

        /**
         * Creates a GetLogRequest message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.GetLogRequest
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.GetLogRequest} GetLogRequest
         */
        GetLogRequest.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.GetLogRequest)
                return object;
            var message = new $root.chat.GetLogRequest();
            if (object.name != null)
                message.name = String(object.name);
            if (object.token != null)
                if (typeof object.token === "string")
                    $util.base64.decode(object.token, message.token = $util.newBuffer($util.base64.length(object.token)), 0);
                else if (object.token.length)
                    message.token = object.token;
            return message;
        };

        /**
         * Creates a plain object from a GetLogRequest message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.GetLogRequest
         * @static
         * @param {chat.GetLogRequest} message GetLogRequest
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        GetLogRequest.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.name = "";
                if (options.bytes === String)
                    object.token = "";
                else {
                    object.token = [];
                    if (options.bytes !== Array)
                        object.token = $util.newBuffer(object.token);
                }
            }
            if (message.name != null && message.hasOwnProperty("name"))
                object.name = message.name;
            if (message.token != null && message.hasOwnProperty("token"))
                object.token = options.bytes === String ? $util.base64.encode(message.token, 0, message.token.length) : options.bytes === Array ? Array.prototype.slice.call(message.token) : message.token;
            return object;
        };

        /**
         * Converts this GetLogRequest to JSON.
         * @function toJSON
         * @memberof chat.GetLogRequest
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        GetLogRequest.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return GetLogRequest;
    })();

    chat.GetLogReply = (function() {

        /**
         * Properties of a GetLogReply.
         * @memberof chat
         * @interface IGetLogReply
         * @property {Array.<chat.IChatMessage>|null} [chatMessages] GetLogReply chatMessages
         */

        /**
         * Constructs a new GetLogReply.
         * @memberof chat
         * @classdesc Represents a GetLogReply.
         * @implements IGetLogReply
         * @constructor
         * @param {chat.IGetLogReply=} [properties] Properties to set
         */
        function GetLogReply(properties) {
            this.chatMessages = [];
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * GetLogReply chatMessages.
         * @member {Array.<chat.IChatMessage>} chatMessages
         * @memberof chat.GetLogReply
         * @instance
         */
        GetLogReply.prototype.chatMessages = $util.emptyArray;

        /**
         * Creates a new GetLogReply instance using the specified properties.
         * @function create
         * @memberof chat.GetLogReply
         * @static
         * @param {chat.IGetLogReply=} [properties] Properties to set
         * @returns {chat.GetLogReply} GetLogReply instance
         */
        GetLogReply.create = function create(properties) {
            return new GetLogReply(properties);
        };

        /**
         * Encodes the specified GetLogReply message. Does not implicitly {@link chat.GetLogReply.verify|verify} messages.
         * @function encode
         * @memberof chat.GetLogReply
         * @static
         * @param {chat.IGetLogReply} message GetLogReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        GetLogReply.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.chatMessages != null && message.chatMessages.length)
                for (var i = 0; i < message.chatMessages.length; ++i)
                    $root.chat.ChatMessage.encode(message.chatMessages[i], writer.uint32(/* id 1, wireType 2 =*/10).fork()).ldelim();
            return writer;
        };

        /**
         * Encodes the specified GetLogReply message, length delimited. Does not implicitly {@link chat.GetLogReply.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.GetLogReply
         * @static
         * @param {chat.IGetLogReply} message GetLogReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        GetLogReply.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a GetLogReply message from the specified reader or buffer.
         * @function decode
         * @memberof chat.GetLogReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.GetLogReply} GetLogReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        GetLogReply.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.GetLogReply();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    if (!(message.chatMessages && message.chatMessages.length))
                        message.chatMessages = [];
                    message.chatMessages.push($root.chat.ChatMessage.decode(reader, reader.uint32()));
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a GetLogReply message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.GetLogReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.GetLogReply} GetLogReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        GetLogReply.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a GetLogReply message.
         * @function verify
         * @memberof chat.GetLogReply
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        GetLogReply.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.chatMessages != null && message.hasOwnProperty("chatMessages")) {
                if (!Array.isArray(message.chatMessages))
                    return "chatMessages: array expected";
                for (var i = 0; i < message.chatMessages.length; ++i) {
                    var error = $root.chat.ChatMessage.verify(message.chatMessages[i]);
                    if (error)
                        return "chatMessages." + error;
                }
            }
            return null;
        };

        /**
         * Creates a GetLogReply message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.GetLogReply
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.GetLogReply} GetLogReply
         */
        GetLogReply.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.GetLogReply)
                return object;
            var message = new $root.chat.GetLogReply();
            if (object.chatMessages) {
                if (!Array.isArray(object.chatMessages))
                    throw TypeError(".chat.GetLogReply.chatMessages: array expected");
                message.chatMessages = [];
                for (var i = 0; i < object.chatMessages.length; ++i) {
                    if (typeof object.chatMessages[i] !== "object")
                        throw TypeError(".chat.GetLogReply.chatMessages: object expected");
                    message.chatMessages[i] = $root.chat.ChatMessage.fromObject(object.chatMessages[i]);
                }
            }
            return message;
        };

        /**
         * Creates a plain object from a GetLogReply message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.GetLogReply
         * @static
         * @param {chat.GetLogReply} message GetLogReply
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        GetLogReply.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.arrays || options.defaults)
                object.chatMessages = [];
            if (message.chatMessages && message.chatMessages.length) {
                object.chatMessages = [];
                for (var j = 0; j < message.chatMessages.length; ++j)
                    object.chatMessages[j] = $root.chat.ChatMessage.toObject(message.chatMessages[j], options);
            }
            return object;
        };

        /**
         * Converts this GetLogReply to JSON.
         * @function toJSON
         * @memberof chat.GetLogReply
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        GetLogReply.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return GetLogReply;
    })();

    chat.VerifyRequest = (function() {

        /**
         * Properties of a VerifyRequest.
         * @memberof chat
         * @interface IVerifyRequest
         * @property {string|null} [name] VerifyRequest name
         * @property {Uint8Array|null} [token] VerifyRequest token
         */

        /**
         * Constructs a new VerifyRequest.
         * @memberof chat
         * @classdesc Represents a VerifyRequest.
         * @implements IVerifyRequest
         * @constructor
         * @param {chat.IVerifyRequest=} [properties] Properties to set
         */
        function VerifyRequest(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * VerifyRequest name.
         * @member {string} name
         * @memberof chat.VerifyRequest
         * @instance
         */
        VerifyRequest.prototype.name = "";

        /**
         * VerifyRequest token.
         * @member {Uint8Array} token
         * @memberof chat.VerifyRequest
         * @instance
         */
        VerifyRequest.prototype.token = $util.newBuffer([]);

        /**
         * Creates a new VerifyRequest instance using the specified properties.
         * @function create
         * @memberof chat.VerifyRequest
         * @static
         * @param {chat.IVerifyRequest=} [properties] Properties to set
         * @returns {chat.VerifyRequest} VerifyRequest instance
         */
        VerifyRequest.create = function create(properties) {
            return new VerifyRequest(properties);
        };

        /**
         * Encodes the specified VerifyRequest message. Does not implicitly {@link chat.VerifyRequest.verify|verify} messages.
         * @function encode
         * @memberof chat.VerifyRequest
         * @static
         * @param {chat.IVerifyRequest} message VerifyRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        VerifyRequest.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.name != null && message.hasOwnProperty("name"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.name);
            if (message.token != null && message.hasOwnProperty("token"))
                writer.uint32(/* id 2, wireType 2 =*/18).bytes(message.token);
            return writer;
        };

        /**
         * Encodes the specified VerifyRequest message, length delimited. Does not implicitly {@link chat.VerifyRequest.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.VerifyRequest
         * @static
         * @param {chat.IVerifyRequest} message VerifyRequest message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        VerifyRequest.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a VerifyRequest message from the specified reader or buffer.
         * @function decode
         * @memberof chat.VerifyRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.VerifyRequest} VerifyRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        VerifyRequest.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.VerifyRequest();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.name = reader.string();
                    break;
                case 2:
                    message.token = reader.bytes();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a VerifyRequest message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.VerifyRequest
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.VerifyRequest} VerifyRequest
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        VerifyRequest.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a VerifyRequest message.
         * @function verify
         * @memberof chat.VerifyRequest
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        VerifyRequest.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.name != null && message.hasOwnProperty("name"))
                if (!$util.isString(message.name))
                    return "name: string expected";
            if (message.token != null && message.hasOwnProperty("token"))
                if (!(message.token && typeof message.token.length === "number" || $util.isString(message.token)))
                    return "token: buffer expected";
            return null;
        };

        /**
         * Creates a VerifyRequest message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.VerifyRequest
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.VerifyRequest} VerifyRequest
         */
        VerifyRequest.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.VerifyRequest)
                return object;
            var message = new $root.chat.VerifyRequest();
            if (object.name != null)
                message.name = String(object.name);
            if (object.token != null)
                if (typeof object.token === "string")
                    $util.base64.decode(object.token, message.token = $util.newBuffer($util.base64.length(object.token)), 0);
                else if (object.token.length)
                    message.token = object.token;
            return message;
        };

        /**
         * Creates a plain object from a VerifyRequest message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.VerifyRequest
         * @static
         * @param {chat.VerifyRequest} message VerifyRequest
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        VerifyRequest.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.name = "";
                if (options.bytes === String)
                    object.token = "";
                else {
                    object.token = [];
                    if (options.bytes !== Array)
                        object.token = $util.newBuffer(object.token);
                }
            }
            if (message.name != null && message.hasOwnProperty("name"))
                object.name = message.name;
            if (message.token != null && message.hasOwnProperty("token"))
                object.token = options.bytes === String ? $util.base64.encode(message.token, 0, message.token.length) : options.bytes === Array ? Array.prototype.slice.call(message.token) : message.token;
            return object;
        };

        /**
         * Converts this VerifyRequest to JSON.
         * @function toJSON
         * @memberof chat.VerifyRequest
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        VerifyRequest.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return VerifyRequest;
    })();

    chat.VerifyReply = (function() {

        /**
         * Properties of a VerifyReply.
         * @memberof chat
         * @interface IVerifyReply
         * @property {boolean|null} [ok] VerifyReply ok
         */

        /**
         * Constructs a new VerifyReply.
         * @memberof chat
         * @classdesc Represents a VerifyReply.
         * @implements IVerifyReply
         * @constructor
         * @param {chat.IVerifyReply=} [properties] Properties to set
         */
        function VerifyReply(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * VerifyReply ok.
         * @member {boolean} ok
         * @memberof chat.VerifyReply
         * @instance
         */
        VerifyReply.prototype.ok = false;

        /**
         * Creates a new VerifyReply instance using the specified properties.
         * @function create
         * @memberof chat.VerifyReply
         * @static
         * @param {chat.IVerifyReply=} [properties] Properties to set
         * @returns {chat.VerifyReply} VerifyReply instance
         */
        VerifyReply.create = function create(properties) {
            return new VerifyReply(properties);
        };

        /**
         * Encodes the specified VerifyReply message. Does not implicitly {@link chat.VerifyReply.verify|verify} messages.
         * @function encode
         * @memberof chat.VerifyReply
         * @static
         * @param {chat.IVerifyReply} message VerifyReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        VerifyReply.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.ok != null && message.hasOwnProperty("ok"))
                writer.uint32(/* id 1, wireType 0 =*/8).bool(message.ok);
            return writer;
        };

        /**
         * Encodes the specified VerifyReply message, length delimited. Does not implicitly {@link chat.VerifyReply.verify|verify} messages.
         * @function encodeDelimited
         * @memberof chat.VerifyReply
         * @static
         * @param {chat.IVerifyReply} message VerifyReply message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        VerifyReply.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a VerifyReply message from the specified reader or buffer.
         * @function decode
         * @memberof chat.VerifyReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {chat.VerifyReply} VerifyReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        VerifyReply.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.chat.VerifyReply();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.ok = reader.bool();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a VerifyReply message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof chat.VerifyReply
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {chat.VerifyReply} VerifyReply
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        VerifyReply.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a VerifyReply message.
         * @function verify
         * @memberof chat.VerifyReply
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        VerifyReply.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.ok != null && message.hasOwnProperty("ok"))
                if (typeof message.ok !== "boolean")
                    return "ok: boolean expected";
            return null;
        };

        /**
         * Creates a VerifyReply message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof chat.VerifyReply
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {chat.VerifyReply} VerifyReply
         */
        VerifyReply.fromObject = function fromObject(object) {
            if (object instanceof $root.chat.VerifyReply)
                return object;
            var message = new $root.chat.VerifyReply();
            if (object.ok != null)
                message.ok = Boolean(object.ok);
            return message;
        };

        /**
         * Creates a plain object from a VerifyReply message. Also converts values to other types if specified.
         * @function toObject
         * @memberof chat.VerifyReply
         * @static
         * @param {chat.VerifyReply} message VerifyReply
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        VerifyReply.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults)
                object.ok = false;
            if (message.ok != null && message.hasOwnProperty("ok"))
                object.ok = message.ok;
            return object;
        };

        /**
         * Converts this VerifyReply to JSON.
         * @function toJSON
         * @memberof chat.VerifyReply
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        VerifyReply.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return VerifyReply;
    })();

    return chat;
})();

module.exports = $root;
