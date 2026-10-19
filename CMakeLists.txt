cmake_minimum_required(VERSION 3.8)
project(pca_servo_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SERIAL REQUIRED libserial)

add_library(pca_servo_hardware SHARED
  src/pca_servo_interface.cpp)

target_include_directories(pca_servo_hardware PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${SERIAL_INCLUDE_DIRS})

target_link_libraries(pca_servo_hardware PUBLIC
  hardware_interface::hardware_interface
  pluginlib::pluginlib
  rclcpp::rclcpp
  ${SERIAL_LIBRARIES})

pluginlib_export_plugin_description_file(
  hardware_interface pca_servo_hardware.xml)

install(FILES pca_servo_hardware.xml DESTINATION share/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY urdf DESTINATION share/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY arduino DESTINATION share/${PROJECT_NAME})

install(TARGETS pca_servo_hardware
  EXPORT export_pca_servo_hardware
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()