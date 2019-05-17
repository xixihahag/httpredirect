##
 # @file CMakeLists.txt
 # @brief 
 # 
 # @author gyz
 # @email mni_gyz@163.com
 ##
include_directories(${CMAKE_SOURCE_DIR}/include)

# 设置单元测试的文件
set(master 
    main.cc
)

set(slave
    slave.cc
)

# 生成master
add_executable(umaster ${master})
add_dependencies(umaster ndsl)
target_link_libraries(umaster ndsl)

#生成slave
add_executable(uslave ${slave})
add_dependencies(uslave ndsl)
target_link_libraries(uslave ndsl)