cmake_minimum_required(VERSION 3.12)
project(FlowMQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Create a library target for FlowMQ
add_library(flowmq_lib STATIC
    src/MQTT.cpp
    src/Frame.cpp
    src/Trie.cpp
    src/Topic.cpp
    src/Message.cpp
    src/Broker.cpp
    src/Server.cpp  
    src/Listener.cpp
    src/Connection.cpp
    src/Session.cpp
)

# Set include directories for the library
target_include_directories(flowmq_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Create executable
add_executable(flowmq 
    src/Main.cpp 
)

# Include directories
target_include_directories(flowmq PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link against necessary libraries (e.g., pthread for threading)
target_link_libraries(flowmq PRIVATE pthread)

# Link the main executable with the library
target_link_libraries(flowmq PRIVATE flowmq_lib)


# Add Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add the test directory
add_subdirectory(tests)
