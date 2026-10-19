#ifndef PCA_SERVO_HARDWARE__PCA_SERVO_INTERFACE_HPP_
#define PCA_SERVO_HARDWARE__PCA_SERVO_INTERFACE_HPP_

#include <string>
#include <vector>
#include <libserial/SerialPort.h>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/state.hpp"

namespace pca_servo_hardware
{

class PCAServoInterface : public hardware_interface::SystemInterface
{
public:
  hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareInfo & info) override;

  hardware_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::return_type read(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  hardware_interface::return_type write(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:
  LibSerial::SerialPort serial_port_;
  std::string device_;
  
  std::vector<double> hw_positions_;
  std::vector<double> hw_commands_;
  
  // Tableau pour l'anti-jitter (générique pour N servos)
  std::vector<double> last_commands_;
};

}  // namespace pca_servo_hardware

#endif