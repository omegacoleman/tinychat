#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <boost/beast/websocket/ssl.hpp>

namespace tinychat
{
	namespace utility
	{
		class tag_non_ssl_t {};
		class tag_ssl_t {};

		const static tag_non_ssl_t tag_non_ssl;
		const static tag_ssl_t tag_ssl;

		template<class NextLayer>
		class optional_ssl_stream
			: public boost::asio::ssl::stream_base
		{
			using stream_type = boost::asio::ssl::stream<NextLayer>;

			std::unique_ptr<stream_type> p_;
			boost::asio::ssl::context* ctx_;

			NextLayer *next_layer_;

			class EmplacableNextLayer {
			public:
				NextLayer next_layer__;
				template<class Arg>
				EmplacableNextLayer(Arg&& arg)
					:next_layer__(std::forward<Arg>(arg))
				{}

				EmplacableNextLayer(EmplacableNextLayer&& other)
					:next_layer__(std::move(other.next_layer__))
				{}

				~EmplacableNextLayer() {}
			};

			std::unique_ptr<EmplacableNextLayer> next_layer_e;

		public:
			/// The native handle type of the SSL stream.
			using native_handle_type = typename stream_type::native_handle_type;

			/// Structure for use with deprecated impl_type.
			using impl_struct = typename stream_type::impl_struct;

			/// The type of the next layer.
			using next_layer_type = typename stream_type::next_layer_type;

			/// The type of the lowest layer.
			using lowest_layer_type = typename stream_type::lowest_layer_type;

			/// The type of the executor associated with the object.
			using executor_type = typename stream_type::executor_type;

			template<class Arg>
			optional_ssl_stream(const tag_non_ssl_t &tag, 
				Arg&& arg)
				: p_(), ctx_(nullptr), 
				next_layer_e(new EmplacableNextLayer{ std::forward<Arg>(arg) })
			{
				next_layer_ = &(next_layer_e->next_layer__);
			}

			template<class Arg>
			optional_ssl_stream(const tag_ssl_t &tag,
				Arg&& arg,
				boost::asio::ssl::context& ctx)
				: p_(new stream_type{
					std::forward<Arg>(arg), ctx })
					, ctx_(&ctx), next_layer_()
			{
			}

			optional_ssl_stream(optional_ssl_stream&& other)
				: p_(std::move(other.p_))
				, ctx_(other.ctx_), next_layer_(other.next_layer_), 
				next_layer_e(std::move(other.next_layer_e))
			{
			}

			optional_ssl_stream& operator=(optional_ssl_stream&& other)
			{
				p_ = std::move(other.p_);
				ctx_ = other.ctx_;
				next_layer_ = other.next_layer_;
				next_layer_e = std::move(other.next_layer_e);
				return *this;
			}

			executor_type get_executor() noexcept
			{
				if (p_)
				{
					return p_->get_executor();
				}
				else {
					return next_layer_->get_executor();
				}
			}

			native_handle_type native_handle()
			{
				if (p_)
				{
					return p_->native_handle();
				}
				else {
					throw worded_exception(
						"error : calling native_handle of a transparent optional_ssl_stream");
				}
			}

			next_layer_type const&
				next_layer() const
			{
				if (p_)
				{
					return p_->next_layer();
				}
				else {
					return *next_layer_;
				}
			}

			next_layer_type&
				next_layer()
			{
				if (p_)
				{
					return p_->next_layer();
				}
				else {
					return *next_layer_;
				}
			}

			template <typename lowest_layer_type>
			lowest_layer_type&
				lowest_layer()
			{
				if (p_)
				{
					return p_->lowest_layer();
				}
				else {
					return next_layer_->lowest_layer();
				}
			}

			lowest_layer_type const&
				lowest_layer() const
			{
				if (p_)
				{
					return p_->lowest_layer();
				}
				else {
					return next_layer_->lowest_layer();
				}
			}

			void
				set_verify_mode(boost::asio::ssl::verify_mode v)
			{
				if (p_)
				{
					p_->set_verify_mode(v);
				}
				else {
					throw worded_exception(
						"error : calling set_verify_mode of a transparent optional_ssl_stream");
				}
			}

			boost::system::error_code
				set_verify_mode(boost::asio::ssl::verify_mode v,
					boost::system::error_code& ec)
			{
				if (p_)
				{
					p_->set_verify_mode(v, ec);
				}
				else {
					throw worded_exception(
						"error : calling set_verify_mode of a transparent optional_ssl_stream");
				}
			}

			void
				set_verify_depth(int depth)
			{
				if (p_)
				{
					p_->set_verify_depth(depth);
				}
				else {
					throw worded_exception(
						"error : calling set_verify_depth of a transparent optional_ssl_stream");
				}
			}

			boost::system::error_code
				set_verify_depth(
					int depth, boost::system::error_code& ec)
			{
				if (p_)
				{
					p_->set_verify_depth(depth, ec);
				}
				else {
					throw worded_exception(
						"error : calling set_verify_depth of a transparent optional_ssl_stream");
				}
			}

			template<class VerifyCallback>
			void
				set_verify_callback(VerifyCallback callback)
			{
				if (p_)
				{
					p_->set_verify_callback(callback);
				}
				else {
					throw worded_exception(
						"error : calling set_verify_callback of a transparent optional_ssl_stream");
				}
			}

			template<class VerifyCallback>
			boost::system::error_code
				set_verify_callback(VerifyCallback callback,
					boost::system::error_code& ec)
			{
				if (p_)
				{
					p_->set_verify_callback(callback, ec);
				}
				else {
					throw worded_exception(
						"error : calling set_verify_callback of a transparent optional_ssl_stream");
				}
			}

			void
				handshake(handshake_type type)
			{
				if (p_)
				{
					p_->handshake(type);
				}
				else {
					throw worded_exception(
						"error : calling handshake of a transparent optional_ssl_stream");
				}
			}

			boost::system::error_code
				handshake(handshake_type type,
					boost::system::error_code& ec)
			{
				if (p_)
				{
					p_->handshake(type, ec);
				}
				else {
					throw worded_exception(
						"error : calling handshake of a transparent optional_ssl_stream");
				}
			}

			template<class ConstBufferSequence>
			void
				handshake(
					handshake_type type, ConstBufferSequence const& buffers)
			{
				if (p_)
				{
					p_->handshake(type, buffers);
				}
				else {
					throw worded_exception(
						"error : calling handshake of a transparent optional_ssl_stream");
				}
			}

			template<class ConstBufferSequence>
			boost::system::error_code
				handshake(handshake_type type,
					ConstBufferSequence const& buffers,
					boost::system::error_code& ec)
			{
				if (p_)
				{
					p_->handshake(type, buffers, ec);
				}
				else {
					throw worded_exception(
						"error : calling handshake of a transparent optional_ssl_stream");
				}
			}

			template<class HandshakeHandler>
			BOOST_ASIO_INITFN_RESULT_TYPE(HandshakeHandler,
				void(boost::system::error_code))
				async_handshake(handshake_type type,
					BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler)
			{
				if (p_)
				{
					return p_->async_handshake(type,
						BOOST_ASIO_MOVE_CAST(HandshakeHandler)(handler));
				}
				else {
					throw worded_exception(
						"error : calling handshake of a transparent optional_ssl_stream");
				}
			}

			template<class ConstBufferSequence, class BufferedHandshakeHandler>
			BOOST_ASIO_INITFN_RESULT_TYPE(BufferedHandshakeHandler,
				void(boost::system::error_code, std::size_t))
				async_handshake(handshake_type type, ConstBufferSequence const& buffers,
					BOOST_ASIO_MOVE_ARG(BufferedHandshakeHandler) handler)
			{
				if (p_)
				{
					return p_->async_handshake(type, buffers,
						BOOST_ASIO_MOVE_CAST(BufferedHandshakeHandler)(handler));
				}
				else {
					throw worded_exception(
						"error : calling handshake of a transparent optional_ssl_stream");
				}
			}

			void
				shutdown()
			{
				if (p_)
				{
					p_->shutdown();
				}
				else {
					next_layer_->shutdown();
				}
			}

			boost::system::error_code
				shutdown(boost::system::error_code& ec)
			{
				if (p_)
				{
					return p_->shutdown(ec);
				}
				else {
					return next_layer_->shutdown(ec);
				}
			}

			template<class ShutdownHandler>
			BOOST_ASIO_INITFN_RESULT_TYPE(ShutdownHandler,
				void(boost::system::error_code))
				async_shutdown(BOOST_ASIO_MOVE_ARG(ShutdownHandler) handler)
			{
				if (p_)
				{
					return p_->async_shutdown(
						BOOST_ASIO_MOVE_CAST(ShutdownHandler)(handler));
				}
				else {
					return next_layer_->async_shutdown(
						BOOST_ASIO_MOVE_CAST(ShutdownHandler)(handler));
				}
			}

			template<class ConstBufferSequence>
			std::size_t
				write_some(ConstBufferSequence const& buffers)
			{
				if (p_)
				{
					return p_->write_some(buffers);
				}
				else {
					return next_layer_->write_some(buffers);
				}
			}

			template<class ConstBufferSequence>
			std::size_t
				write_some(ConstBufferSequence const& buffers,
					boost::system::error_code& ec)
			{
				if (p_)
				{
					return p_->write_some(buffers, ec);
				}
				else {
					return next_layer_->write_some(buffers, ec);
				}
			}

			template<class ConstBufferSequence, class WriteHandler>
			BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler,
				void(boost::system::error_code, std::size_t))
				async_write_some(ConstBufferSequence const& buffers,
					BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
			{
				if (p_)
				{
					return p_->async_write_some(buffers,
						BOOST_ASIO_MOVE_CAST(WriteHandler)(handler));
				}
				else {
					return next_layer_->async_write_some(buffers,
						BOOST_ASIO_MOVE_CAST(WriteHandler)(handler));
				}
			}

			template<class MutableBufferSequence>
			std::size_t
				read_some(MutableBufferSequence const& buffers)
			{
				if (p_)
				{
					return p_->read_some(buffers);
				}
				else {
					return next_layer_->read_some(buffers);
				}
			}

			template<class MutableBufferSequence>
			std::size_t
				read_some(MutableBufferSequence const& buffers,
					boost::system::error_code& ec)
			{
				if (p_)
				{
					return p_->read_some(buffers, ec);
				}
				else {
					return next_layer_->read_some(buffers, ec);
				}
			}

			template<class MutableBufferSequence, class ReadHandler>
			BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
				void(boost::system::error_code, std::size_t))
				async_read_some(MutableBufferSequence const& buffers,
					BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
			{
				if (p_)
				{
					return p_->async_read_some(buffers,
						BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));
				}
				else {
					return next_layer_->async_read_some(buffers,
						BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));
				}
			}

			template<class SyncStream>
			friend
				void
				teardown(boost::beast::websocket::role_type,
					optional_ssl_stream<SyncStream>& stream,
					boost::system::error_code& ec);

			template<class AsyncStream, class TeardownHandler>
			friend
				void
				async_teardown(boost::beast::websocket::role_type,
					optional_ssl_stream<AsyncStream>& stream, TeardownHandler&& handler);
		};

		// These hooks are used to inform boost::beast::websocket::stream on
		// how to tear down the connection as part of the WebSocket
		// protocol specifications

		template<class SyncStream>
		inline
			void
			teardown(
				boost::beast::websocket::role_type role,
				optional_ssl_stream<SyncStream>& stream,
				boost::system::error_code& ec)
		{
			// Just forward it to the wrapped ssl::stream
			using boost::beast::websocket::teardown;
			if (stream.p_)
			{
				teardown(role, *stream.p_, ec);
			}
			else {
				teardown(role, *stream.next_layer_, ec);
			}
		}

		template<class AsyncStream, class TeardownHandler>
		inline
			void
			async_teardown(
				boost::beast::websocket::role_type role,
				optional_ssl_stream<AsyncStream>& stream,
				TeardownHandler&& handler)
		{
			// Just forward it to the wrapped ssl::stream
			using boost::beast::websocket::async_teardown;
			if (stream.p_)
			{
				async_teardown(role,
					*stream.p_, std::forward<TeardownHandler>(handler));
			}
			else {
				async_teardown(role,
					*stream.next_layer_, std::forward<TeardownHandler>(handler));
			}
		}

	}
}
