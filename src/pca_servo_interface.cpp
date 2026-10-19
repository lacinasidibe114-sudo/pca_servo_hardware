#include "pca_servo_hardware/pca_servo_interface.hpp"
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "pluginlib/class_list_macros.hpp"

namespace pca_servo_hardware
{

hardware_interface::CallbackReturn PCAServoInterface::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  device_ = info_.hardware_parameters["device"];
  
  hw_positions_.resize(info_.joints.size(), 0.0);
  hw_commands_.resize(info_.joints.size(), 0.0);
  last_commands_.resize(info_.joints.size(), 999.0); // Valeur impossible au départ

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn PCAServoInterface::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  try {
    serial_port_.Open(device_);
    serial_port_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
    serial_port_.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
    serial_port_.SetParity(LibSerial::Parity::PARITY_NONE);
    serial_port_.SetStopBits(LibSerial::StopBits::STOP_BITS_1);
    serial_port_.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
  } catch (...) {
    RCLCPP_FATAL(rclcpp::get_logger("PCAServoHardware"), "Impossible d'ouvrir %s", device_.c_str());
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Mettre tous les servos à 0 (centre) à l'activation
  std::fill(hw_commands_.begin(), hw_commands_.end(), 0.0);
  
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn PCAServoInterface::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  if (serial_port_.IsOpen()) {
    serial_port_.Close();
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> PCAServoInterface::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  for (size_t i = 0; i < info_.joints.size(); ++i) {
    state_interfaces.emplace_back(info_.joints[i].name, hardware_interface::HW_IF_POSITION, &hw_positions_[i]);
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> PCAServoInterface::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (size_t i = 0; i < info_.joints.size(); ++i) {
    command_interfaces.emplace_back(info_.joints[i].name, hardware_interface::HW_IF_POSITION, &hw_commands_[i]);
  }
  return command_interfaces;
}

hardware_interface::return_type PCAServoInterface::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // Puisque les servos standards ne renvoient pas leur position, on fait une "copie" de la commande.
  for (size_t i = 0; i < info_.joints.size(); ++i) {
    hw_positions_[i] = hw_commands_[i]; 
  }
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type PCAServoInterface::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  if (!serial_port_.IsOpen()) return hardware_interface::return_type::ERROR;

  std::stringstream ss;
  bool has_changed = false;

  for (size_t i = 0; i < info_.joints.size(); ++i) {
    double cmd_rad = hw_commands_[i];
    
    // ROS 2 utilise les radians. On convertit en degrés pour l'Arduino.
    // On ajoute 90 pour que 0 rad corresponde à 90° (centre du servo).
    double cmd_deg = (cmd_rad * 180.0 / M_PI) + 90.0;
    
    // Anti-jitter
    if (std::abs(cmd_deg - last_commands_[i]) > 0.1) {
      ss << "P" << i << ":" << std::fixed << std::setprecision(1) << cmd_deg << " ";
      last_commands_[i] = cmd_deg;
      has_changed = true;
    }
  }

  if (has_changed) {
    ss << "\n";
    try {
      serial_port_.Write(ss.str());
    } catch (...) {
      return hardware_interface::return_type::ERROR;
    }
  }
  
  return hardware_interface::return_type::OK;
}

}  // namespace pca_servo_hardware

PLUGINLIB_EXPORT_CLASS(pca_servo_hardware::PCAServoInterface, hardware_interface::SystemInterface)