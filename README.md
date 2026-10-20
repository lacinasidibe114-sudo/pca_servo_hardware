C'est une excellente idée. La calibration des impulsions (microsecondes) est **la clé pour ne pas griller ses servos** et pour pouvoir mélanger différents modèles sur le même PCA9685. 

Voici le `README.md` complet et mis à jour. J'y ai intégré une section dédiée très détaillée sur l'ajustement des limites (calibration), avec les explications techniques tirées de nos discussions.

Tu peux copier-coller ce contenu directement dans ton dépôt GitHub `pca_servo_hardware`.

```markdown
# PCA9685 Servo Hardware Interface (ROS 2)

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Ce package fournit une interface matérielle (`ros2_control` SystemInterface) générique, robuste et sécurisée pour piloter **N servomoteurs** via un contrôleur PWM **PCA9685** et un Arduino.

Ce package fait partie d'une suite d'outils (Toolkit) pour la robotique développée par Lacina Sidibe (PFE 2027).

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

## 🔌 Compatibilité et Alimentation des Servomoteurs

**⚠️ IMPORTANT :** Ce package est conçu pour les servos de **position** (et non les servos à rotation continue 360°).

| Servo | Compatible ? | Remarque |
|---|---|---|
| **SG90** | ✅ Oui | Testé et fonctionnel (petit couple). |
| **MG90S / MG995 / MG996R (180°)** | ✅ Oui | Attention : vérifiez que ce n'est pas la version 360°. |
| **HS-645MG, HS-422, HS-311** (Hitec) | ✅ Oui | Servos robustes, très fiables avec une bonne alimentation. |
| **DS3218, DS3225** | ✅ Oui | Course souvent de 270° : voir la section Calibration. |
| **MG996R (360°)** | ❌ Non | Interprète le signal PWM comme une vitesse, pas une position. |
| **STS3215** (Feetech) | ❌ Non | Bus série (TTL). Nécessite une interface `ros2_control` différente. |

### ⚠️ Avertissement Critique sur l'Alimentation
Ne branchez **jamais** les servomoteurs directement sur le port 5V de l'Arduino. 
*   Les petits servos (SG90) consomment ~200 mA.
*   Les gros servos (Hitec, MG996R) peuvent tirer **2 à 3 Ampères** en cas de blocage. 
Utilisez impérativement une **alimentation externe** (5V ou 6V, 3A+) branchée sur les bornes à vis (V+ et GND) du PCA9685. La masse (GND) de l'alimentation doit être commune avec celle de l'Arduino.

## 🛠️ Ajustement des Limites et Impulsions (Calibration)

Le code Arduino utilise la fonction **`writeMicroseconds()`** plutôt que `setPWM()`. C'est une décision de conception cruciale pour deux raisons :
1. **Précision :** `writeMicroseconds()` envoie la durée exacte de l'impulsion (en µs), évitant les imprécisions de conversion des "ticks" (0-4095) du PCA9685.
2. **Sécurité (Anti-Forçage) :** Si vous envoyez une impulsion trop longue (ex: > 2500 µs), le servo va forcer contre sa butée mécanique, chauffer, consommer énormément de courant et potentiellement griller. 

### Comment calibrer chaque servo (Tableaux `US_MIN` et `US_MAX`)

Dans le fichier `arduino/pca_servo_firmware.ino`, vous trouverez deux tableaux de 16 valeurs (une pour chaque canal) :

```cpp
// Valeurs par défaut : 600 µs (0°) à 2400 µs (180°)
int US_MIN[16] = {600, 600, 600, 600, 600, 600, 600, 600, ...};
int US_MAX[16] = {2400, 2400, 2400, 2400, 2400, 2400, 2400, 2400, ...};
```

**Pourquoi ajuster ces valeurs ?**
*   **Si un servo force ou grésille à 180° :** Réduisez sa valeur `US_MAX` (ex: passez de `2400` à `2300` ou `2100`).
*   **Si vous utilisez un servo 270° (ex: DS3225) :** Augmentez sa valeur `US_MAX` (ex: `2700`) pour qu'il puisse utiliser toute sa course.
*   **Si vous utilisez un Hitec très sensible :** Vous pouvez restreindre la plage à `900` - `2100` pour une sécurité maximale.

*Exemple : Le canal 0 est un SG90 (plage 600-2400) et le canal 1 est un DS3225 270° (plage 500-2700)* :
```cpp
int US_MIN[16] = {600, 500, 600, 600, 600, ...};
int US_MAX[16] = {2400, 2700, 2400, 2400, 2400, ...};
```
*Note : La fonction ROS 2 envoie toujours des angles normalisés (de 0° à 180°). C'est le code Arduino qui étire ou compresse cet angle en fonction de la plage `US_MIN` / `US_MAX` de chaque canal.*

## 📂 Structure du Package

```text
pca_servo_hardware/
├── CMakeLists.txt
├── package.xml
├── pca_servo_hardware.xml      # Manifeste du plugin
├── arduino/
│   └── pca_servo_firmware.ino  # Code Arduino (I2C + PCA9685 avec calibration)
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

- **Générique** : Le code C++ boucle sur `info_.joints.size()`. Ajoutez 3, 6 ou 16 joints dans l'URDF, l'interface s'adapte sans changer le code C++.
- **Calibration par Canal** : Les tableaux `US_MIN[16]` et `US_MAX[16]` permettent de mélanger différents types de servos sur le même contrôleur PCA9685.
- **Anti-Jitter** : Évite les tremblements en n'envoyant une commande au PCA9685 que si l'angle a changé de plus de 0.1°.
- **Contrôleurs** : Compatible avec `ForwardCommandController` (test direct) et `JointTrajectoryController` (mouvements fluides via MoveIt! 2).

## 🚀 Installation et Utilisation

**1. Compiler le package**
```bash
cd ~/votre_workspace
colcon build --packages-select pca_servo_hardware
source install/setup.bash
```

**2. Téléverser le code Arduino**
Ouvrez `arduino/pca_servo_firmware.ino` dans l'IDE Arduino et téléversez-le (nécessite la bibliothèque `Adafruit PWM Servo Driver`).

**3. Lancer le système ROS 2**
```bash
ros2 launch pca_servo_hardware pca_servo_test.launch.py
```

**4. Envoyer une commande de position (en radians)**
*Note : 0.0 rad = 90° (centre), 1.57 rad = 180° (max), -1.57 rad = 0° (min).*

```bash
# Envoie une commande pour 6 servos
ros2 topic pub --once /forward_position_controller/commands std_msgs/msg/Float64MultiArray "{data: [0.0, 1.0, -1.0, 0.5, 0.0, 0.0]}"
```

**5. Tester une trajectoire fluide (MoveIt! 2 style)**
```bash
ros2 run pca_servo_hardware test_trajectory.py
```

## 🛠️ Dépannage (Troubleshooting)

- **Le servo tremble ou grésille à l'arrêt** : Réduisez la valeur `US_MAX` du canal concerné dans le code Arduino (ex: 2300 au lieu de 2400) pour éviter qu'il ne bute mécaniquement.
- **Le servo ne bouge pas du tout** : Vérifiez l'alimentation V+ du PCA9685. Vérifiez que le servo n'est pas un modèle 360°.
- **Le port série est introuvable** : Vérifiez le nom de votre port dans `urdf/pca_servo_control.xacro` (`/dev/ttyACM0` ou `/dev/ttyUSB0`).

## 📄 Licence

Ce projet est distribué sous la licence **Apache 2.0**. 
Voir le fichier [LICENSE](LICENSE) pour plus d'informations.

## 👨‍💻 Auteur
**Lacina Sidibe** (PFE 2027)
