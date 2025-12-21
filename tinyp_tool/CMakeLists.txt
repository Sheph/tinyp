set(SOURCES
    main.cpp
)

configure_file(filter1.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/filter1.txt COPYONLY)
configure_file(formula1.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/formula1.txt COPYONLY)
configure_file(formula2.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/formula2.txt COPYONLY)
configure_file(formula3.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/formula3.txt COPYONLY)

add_executable(tinyp_tool ${SOURCES})

target_link_libraries(tinyp_tool tinyp)
