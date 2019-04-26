PROTOBUF_GENERATE_CPP(PROTO_RPC_TEST_SRC PROTO_RPC_TEST_HDRS ${PROJECT_SOURCE_DIR}/protocol/chat.proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB websocket_client_SRCS
        websocket_client.cpp
        )

add_executable(websocket-client ${websocket_client_SRCS} ${PROTO_RPC_TEST_SRC} ${PROTO_RPC_TEST_HDRS})
target_link_libraries(
        websocket-client
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${WIN32_EXTRA_LIBRARIES}
		${OPENSSL_LIBRARIES}
		tinyrpc
)