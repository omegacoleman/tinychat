PROTOBUF_GENERATE_CPP(PROTO_RPC_TEST_SRC PROTO_RPC_TEST_HDRS ${PROJECT_SOURCE_DIR}/protocol/chat.proto)

include_directories(
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/vendor/cpp-bredis/include
		)

file(GLOB websocket_server_SRCS
        websocket_server.cpp
        )

add_executable(websocket-server ${websocket_server_SRCS} ${PROTO_RPC_TEST_SRC} ${PROTO_RPC_TEST_HDRS})
target_link_libraries(
        websocket-server
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${WIN32_EXTRA_LIBRARIES}
		${OPENSSL_LIBRARIES}
		tinyrpc
)