# README - Système d'aide au stationnement

## Description du Projet

Le projet vise à développer un système d'aide au stationnement basé sur un microcontrôleur, utilisant FreeRTOS pour une gestion multitâche efficace. Le système détectera les obstacles à l'avant et à l'arrière du véhicule grâce à des capteurs à ultrasons, tout en offrant une interface de commande via un tableau de bord.

## Fonctionnalités
- Mesure de distances à l'aide de capteurs à ultrasons (avant et arrière).
- Contrôle de deux moteurs (gauche et droit) via PWM.
- Communication UART pour recevoir des commandes et envoyer des données.
- Gestion multitâche avec FreeRTOS.

## Dépendances
Ce projet utilise les bibliothèques suivantes :
- FreeRTOS
- CMSIS

## Configuration Matérielle

### Composants requis
- Carte STM32F4.
- 4 moteurs DC avec un driver (L298N).
- Deux capteurs à ultrasons (HC-SR04).
- Modules de connexion UART (pour contrôle Bluetooth).

### Schéma de Câblage

#### Capteurs à ultrasons
- **Capteur avant** :
  - Trigger : GPIOA Pin 0 (PA0).
  - Echo : GPIOA Pin 1 (PA1).
- **Capteur arrière** :
  - Trigger : GPIOA Pin 6 (PA6).
  - Echo : GPIOA Pin 7 (PA7).

#### Moteurs DC (Driver L298N)
- **Moteur gauche (A)** :
  - PWM (ENA) : GPIOB Pin 6 (PB6).
  - IN1 : GPIOB Pin 7 (PB7).
  - IN2 : GPIOB Pin 8 (PB8).
- **Moteur droit (B)** :
  - PWM (ENB) : GPIOB Pin 9 (PB9).
  - IN3 : GPIOB Pin 10 (PB10).
  - IN4 : GPIOB Pin 11 (PB11).

#### UART
- TX : GPIOA Pin 2 (PA2).
- RX : GPIOA Pin 3 (PA3).

## Compilation et Flash du Code

### Prérequis
- IDE (Keil).
- Programmateur/debugger (ST-LINK).

### Étapes
1. Clonez le dépôt sur votre machine locale.
2. Ouvrez le projet dans votre IDE.
3. Compilez le projet.
4. Flashez le firmware sur la carte STM32 à l'aide du ST-LINK.

## Utilisation

### Commandes UART
Le système utilise des commandes UART pour contrôler les moteurs :
- **F** : Avancer.
- **B** : Reculer.
- **R** : Tourner à droite.
- **L** : Tourner à gauche.
- **S** : Arrêter.

### Sortie UART
Les distances mesurées par les capteurs sont envoyées via UART dans les formats suivants :
- **F:X** : Distance frontale (X en centimètres).
- **A:X** : Distance arrière (X en centimètres).

