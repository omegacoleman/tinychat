
file(GLOB test_SRCS
	test_msg_deliver.cpp)

foreach(testSrc ${test_SRCS})
        # Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        # EXCLUDE_FROM_ALL excludes it from target ALL
        add_executable(${testName} EXCLUDE_FROM_ALL ${testSrc})

        target_link_libraries(${testName} 
			${CMAKE_THREAD_LIBS_INIT}
			${Boost_LIBRARIES}
			${PROTOBUF_LIBRARIES}
			${WIN32_EXTRA_LIBRARIES}
			${OPENSSL_LIBRARIES}
			tinyrpc)

        set_target_properties(${testName} PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)
			
		if (WIN32)
			set_target_properties(${testName} PROPERTIES 
				RUNTIME_OUTPUT_DIRECTORY_DEBUG  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/tests
				RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/tests)
		endif()

		add_dependencies(${testName} websocket-server websocket-client)

        add_test(NAME ${testName} 
                 WORKING_DIRECTORY $<TARGET_FILE_DIR:${testName}>
                 COMMAND $<TARGET_FILE:${testName}> )

        add_test(NAME build_${testName} 
                 COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${testName})

		set_tests_properties(${testName} PROPERTIES DEPENDS build_${testName})
endforeach(testSrc)
