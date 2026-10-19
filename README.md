```markdown
# PCA9685 Servo Hardware Interface (ROS 2)

Ce package fournit une interface matérielle (`ros2_control` SystemInterface) générique et robuste pour piloter **N servomoteurs** via un contrôleur PWM **PCA9685** et un Arduino.

## 🏗️ Architecture

Le système est conçu pour être "plug-and-play". Le nombre de servos est défini dynamiquement dans le fichier URDF/Xacro.

```text
+-------------------------------------------------------------+
|                       ORDINATEUR (ROS 2)                    |
|                                                             |
|  [MoveIt! 2 / Contrôleurs]                                  |
|         │                                                   |
|         ▼                                                   |
|  [forward_controller / joint_trajectory_controller]         |
|         │                                                   |
|         ▼                                                   |
|  [pca_servo_hardware (C++ SystemInterface)]                 |
|         │                                                   |
|         │ 1. Conversion Rad -> Deg                          |
|         │ 2. Anti-jitter (n'envoie que si ça change)         |
|         ▼                                                   |
|  [libserial] ──────────────┐                                |
+----------------------------|--------------------------------+
                             | USB Série (115200 bauds)
                             | Trame: "P0:90.0 P1:45.0\n"
                             ▼
+-------------------------------------------------------------+
|                       ARDUINO (C++)                         |
|                                                             |
|  [Réception Série] ──> [Adafruit_PWMServoDriver.h]          |
|                             │                               |
|                             ├──> [I2C] ──> [PCA9685]        |
|                                                 │           |
|                                                 ├──> Servo 1|
|                                                 ├──> Servo 2|
|                                                 └──> ...    |
+-------------------------------------------------------------+
```

## 📂 Structure du Package

```text
pca_servo_hardware/
├── CMakeLists.txt
├── package.xml
├── pca_servo_hardware.xml
├── arduino/
│   └── pca_servo_firmware.ino  # Code Arduino (I2C + PCA9685)
├── config/
│   └── controllers.yaml        # Config des contrôleurs (Trajectory ou Forward)
├── include/pca_servo_hardware/
│   └── pca_servo_interface.hpp
├── launch/
│   └── pca_servo_test.launch.py
├── scripts/
│   └── test_trajectory.py      # Script de test de trajectoire fluide
├── src/
│   └── pca_servo_interface.cpp # Code C++ (générique pour N servos)
└── urdf/
    ├── pca_servo_robot.urdf.xacro
    └── pca_servo_control.xacro
```

## ⚙️ Fonctionnalités Clés

- **Générique** : Le code C++ boucle sur `info_.joints.size()`. Ajoutez 3, 6 ou 16 joints dans l'URDF, l'interface s'adapte sans changer le code.
- **Anti-Jitter** : Évite les tremblements en n'envoyant une commande au PCA9685 que si l'angle a changé de plus de 0.1°.
- **Calibration** : Paramètres `SERVO_MIN` et `SERVO_MAX` dans le code Arduino pour ajuster précisément les impulsions PWM (0° à 180°).
- **Contrôleurs** : Compatible avec `ForwardCommandController` (test direct) et `JointTrajectoryController` (mouvements fluides via MoveIt! 2).

## 🚀 Utilisation

**1. Téléverser le code Arduino**
Ouvrez `arduino/pca_servo_firmware.ino` dans l'IDE Arduino et téléversez-le.

**2. Compiler et lancer**
```bash
colcon build --packages-select pca_servo_hardware
source install/setup.bash
ros2 launch pca_servo_hardware pca_servo_test.launch.py
```

**3. Tester une trajectoire fluide**
```bash
ros2 run pca_servo_hardware test_trajectory.py
```

## 👨‍💻 Auteur
Lacina Sidibe (PFE 2027)
