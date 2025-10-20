cmake_minimum_required(VERSION 3.16)

project(SCERSE VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
  add_compile_options("/Zc:__cplusplus")
endif()


# Enable Qt automatic features for Q_OBJECT, UI, and resources
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify Qt6 installation path
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/mingw_64/lib/cmake")
set(Qt6_DIR "C:/Qt/6.9.3/mingw_64/lib/cmake/Qt6")


# Find Qt6 Widgets package
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include the project src folder for headers
include_directories(${PROJECT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp

    # Common headers (if header-only)
    ${PROJECT_SOURCE_DIR}/src/common/Error.hpp
    ${PROJECT_SOURCE_DIR}/src/common/Types.hpp
    ${PROJECT_SOURCE_DIR}/src/common/AST.hpp

    # Lexer
    ${PROJECT_SOURCE_DIR}/src/lexer/Token.hpp
    ${PROJECT_SOURCE_DIR}/src/lexer/Token.cpp
    ${PROJECT_SOURCE_DIR}/src/lexer/Lexer.hpp
    ${PROJECT_SOURCE_DIR}/src/lexer/Lexer.cpp

    # Parser
    ${PROJECT_SOURCE_DIR}/src/parser/Grammar.hpp
    ${PROJECT_SOURCE_DIR}/src/parser/Grammar.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/LR1Parser.hpp
    ${PROJECT_SOURCE_DIR}/src/parser/LR1Parser.cpp

    # Semantic
    ${PROJECT_SOURCE_DIR}/src/semantic/SymbolTable.hpp
    ${PROJECT_SOURCE_DIR}/src/semantic/SymbolTable.cpp

    # Recovery
    ${PROJECT_SOURCE_DIR}/src/recovery/SuggestionEngine.hpp
    ${PROJECT_SOURCE_DIR}/src/recovery/SuggestionEngine.cpp

    # GUI
    ${PROJECT_SOURCE_DIR}/src/gui/MainWindow.hpp
    ${PROJECT_SOURCE_DIR}/src/gui/MainWindow.cpp
    ${PROJECT_SOURCE_DIR}/src/gui/CodeEditor.hpp
    ${PROJECT_SOURCE_DIR}/src/gui/CodeEditor.cpp
    ${PROJECT_SOURCE_DIR}/src/gui/SyntaxHighlighter.hpp
    ${PROJECT_SOURCE_DIR}/src/gui/SyntaxHighlighter.cpp
    ${PROJECT_SOURCE_DIR}/src/gui/ErrorConsole.hpp
    ${PROJECT_SOURCE_DIR}/src/gui/ErrorConsole.cpp
    ${PROJECT_SOURCE_DIR}/src/gui/SymbolTableView.hpp
    ${PROJECT_SOURCE_DIR}/src/gui/SymbolTableView.cpp
)

# Add executable target
qt_add_executable(SCERSE ${SOURCES})

# Link Qt libraries
target_link_libraries(SCERSE PRIVATE Qt6::Widgets)

# Enable testing support (optional)
enable_testing()
find_package(GTest QUIET)
if(GTest_FOUND)
    add_executable(scerse_tests
        tests/test_lexer.cpp
        # Add other test files here
    )
    target_link_libraries(scerse_tests PRIVATE GTest::gtest_main Qt6::Widgets)
    include(GoogleTest)
    gtest_discover_tests(scerse_tests)
endif()

# Install target (optional)
install(TARGETS SCERSE
    RUNTIME DESTINATION bin
)
