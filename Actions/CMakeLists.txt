set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(Actions INTERFACE)

target_include_directories(Actions 
    INTERFACE 
    ${CurrentDirectory}
)
