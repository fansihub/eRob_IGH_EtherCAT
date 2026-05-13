# EtherCAT CSV Mode Motor Control Demo

## Introduction
This is a motor control demo program based on IGH EtherCAT Master, demonstrating how to control a servo motor in CSV (Cyclic Synchronous Velocity) mode using EtherCAT communication.

## System Requirements
- Ubuntu 20.04/22.04/24.04
- IGH EtherCAT Master (v1.5.3)
- Servo drive supporting CiA402 protocol

## Installation

### 1. Install Required Tools
```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git autoconf libtool pkg-config make build-essential net-tools
```

### 2. Install IGH EtherCAT Master
```bash
git clone https://gitlab.com/etherlab.org/ethercat.git
cd ethercat
git checkout stable-1.5
./bootstrap
./configure --prefix=/usr/local/etherlab --disable-8139too --disable-eoe --enable-generic
make all modules
sudo make modules_install install
sudo depmod
```

### 3. Configure System
```bash
sudo ln -s /usr/local/etherlab/bin/ethercat /usr/bin/
sudo ln -s /usr/local/etherlab/etc/init.d/ethercat /etc/init.d/ethercat
sudo mkdir -p /etc/sysconfig
sudo cp /usr/local/etherlab/etc/sysconfig/ethercat /etc/sysconfig/ethercat
```

### 4. Network Configuration
1. Create udev rule:
```bash
echo 'KERNEL=="EtherCAT[0-9]*", MODE="0666"' | sudo tee /etc/udev/rules.d/99-EtherCAT.rules
```

2. Configure EtherCAT network adapter:
```bash
sudo gedit /etc/sysconfig/ethercat
```
Modify MAC address and driver in the configuration file:
```
MASTER0_DEVICE="xx:xx:xx:xx:xx:xx"  # Replace with your network card's MAC address
DEVICE_MODULES="generic"
```

## Usage

### 1. Start EtherCAT Master
```bash
sudo /etc/init.d/ethercat start
```

### 2. Build the Program
```bash
git clone https://github.com/ZeroErrControl/eRob_IGH_EtherCAT.git
cd eRob_IGH_EtherCAT
mkdir build
cd build
cmake ..
make
```

### 3. Run the Program
```bash
./igh_driver
```
### lubancat 实测

手动插入模块
```
cat@lubancat:~/fansihub/igh_ws/unionai_arm/igh.ko$ sudo insmod ./ec_master.ko main_devices=ca:f6:20:54:79:b4
cat@lubancat:~/fansihub/igh_ws/unionai_arm/igh.ko$ sudo insmod ./ec_stmmac.ko
cat@lubancat:~/fansihub/igh_ws/unionai_arm/igh.ko$ sudo ethercat master
Master0
  Phase: Idle
  Active: no
  Slaves: 1
```
上电测试可以的,
```
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ make
Consolidate compiler generated dependencies of target igh_driver
[100%] Built target igh_driver
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ ll
total 76
drwxrwxr-x 3 cat cat  4096 May 13 09:58 ./
drwxrwxr-x 5 cat cat  4096 May 13 09:46 ../
-rw-rw-r-- 1 cat cat 14071 May 13 09:46 CMakeCache.txt
drwxrwxr-x 5 cat cat  4096 May 13 09:59 CMakeFiles/
-rw-rw-r-- 1 cat cat  1717 May 13 09:46 cmake_install.cmake
-rwxrwxr-x 1 cat cat 36000 May 13 09:58 igh_driver*
-rw-rw-r-- 1 cat cat  5593 May 13 09:46 Makefile
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ ./igh_driver 
Using priority 99.
sched_setscheduler failed # 没加sudo
: Operation not permitted
Activating master...
Activating master...
All slaves have reached OP state
        Execution time: 0 ns
actPos0: 583371 actVel0: 0      Fault state, sending reset command
        Timestamp diff: 3159155179 ns   Execution time: 71310 ns
actPos0: 583370 actVel0: 0      Fault state, sending reset command
        Timestamp diff: 2079370 ns      Execution time: 64161 ns
actPos0: 583370 actVel0: -10    Switch on disabled, sending shutdown command
        Timestamp diff: 993365 ns       Execution time: 63428 ns
actPos0: 583369 actVel0: 2      Switch on disabled, sending shutdown command
        Timestamp diff: 999326 ns       Execution time: 62987 ns
actPos0: 583370 actVel0: -5     Ready to switch on, sending switch on command
        Timestamp diff: 999565 ns       Execution time: 63129 ns
actPos0: 583371 actVel0: 13     Ready to switch on, sending switch on command
        Timestamp diff: 1000285 ns      Execution time: 64146 ns
actPos0: 583371 actVel0: -1     Switched on, sending enable operation command
        Timestamp diff: 1000845 ns      Execution time: 63997 ns
actPos0: 583369 actVel0: 0      Switched on, sending enable operation command
        Timestamp diff: 999686 ns       Execution time: 63264 ns
actPos0: 583370 actVel0: 0      Velocity: Target=10000, Actual=0        Timestamp diff: 999325 ns       Execution time: 59907 ns
actPos0: 583371 actVel0: 0      Velocity: Target=10000, Actual=0        Timestamp diff: 1000166 ns      Execution time: 60042 ns

actPos0: 583368 actVel0: 18     Velocity: Target=10000, Actual=18       Timestamp diff: 998926 ns       Execution time: 60713 ns

actPos0: 583879 actVel0: 11241  Velocity: Target=10000, Actual=11241    Timestamp diff: 995006 ns       Execution time: 60736 ns

actPos0: 584003 actVel0: 10088  Velocity: Target=10000, Actual=10088    Timestamp diff: 998887 ns       Execution time: 121944 ns #抖动能有100us以上了
actPos0: 584011 actVel0: 10206  Velocity: Target=10000, Actual=10206    Timestamp diff: 1060847 ns      Execution time: 66377 ns

cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ sudo ./igh_driver 
Using priority 99.
Activating master...
Activating master...
All slaves have reached OP state
        Execution time: 0 ns
actPos0: 1122109        actVel0: 37     Fault state, sending reset command
        Timestamp diff: 4129011949 ns   Execution time: 21138 ns
actPos0: 1122107        actVel0: 21     Fault state, sending reset command
        Timestamp diff: 2041768 ns      Execution time: 11650 ns
actPos0: 1122108        actVel0: 4      Switch on disabled, sending shutdown command
        Timestamp diff: 990884 ns       Execution time: 11204 ns
actPos0: 1122109        actVel0: 1      Switch on disabled, sending shutdown command
        Timestamp diff: 999484 ns       Execution time: 11341 ns
actPos0: 1122108        actVel0: 0      Ready to switch on, sending switch on command
        Timestamp diff: 1000124 ns      Execution time: 12645 ns
actPos0: 1122110        actVel0: 0      Ready to switch on, sending switch on command
        Timestamp diff: 1001124 ns      Execution time: 11032 ns
actPos0: 1122109        actVel0: 0      Switched on, sending enable operation command
        Timestamp diff: 998724 ns       Execution time: 10294 ns
actPos0: 1122107        actVel0: 0      Switched on, sending enable operation command
        Timestamp diff: 999204 ns       Execution time: 11598 ns

actPos0: 1122923        actVel0: 10111  Velocity: Target=10000, Actual=10111    Timestamp diff: 1000206 ns      Execution time: 7976 ns
actPos0: 1122934        actVel0: 10317  Velocity: Target=10000, Actual=10317    Timestamp diff: 999927 ns       Execution time: 9571 ns
actPos0: 1122945        actVel0: 10262  Velocity: Target=10000, Actual=10262    Timestamp diff: 1001406 ns      Execution time: 7958 ns
actPos0: 1122955        actVel0: 10044  Velocity: Target=10000, Actual=10044    Timestamp diff: 998525 ns       Execution time: 8971 ns
actPos0: 1122966        actVel0: 9755   Velocity: Target=10000, Actual=9755     Timestamp diff: 1001006 ns      Execution time: 7941 ns
actPos0: 1122975        actVel0: 9905   Velocity: Target=10000, Actual=9905     Timestamp diff: 998927 ns       Execution time: 15370 ns
actPos0: 1122985        actVel0: 9937   Velocity: Target=10000, Actual=9937     Timestamp diff: 1007766 ns      Execution time: 8216 ns
actPos0: 1122994        actVel0: 10067  Velocity: Target=10000, Actual=10067    Timestamp diff: 992486 ns       Execution time: 14770 ns # #抖动在20us内
actPos0: 1123001        actVel0: 10131  Velocity: Target=10000, Actual=10131    Timestamp diff: 1006886 ns      Execution time: 8198 ns
actPos0: 1123008        actVel0: 10240  Velocity: Target=10000, Actual=10240    Timestamp diff: 993166 ns       Execution time: 9794 ns
actPos0: 1123016        actVel0: 10262  Velocity: Target=10000, Actual=10262    Timestamp diff: 1001526 ns      Execution time: 18681 ns

```
### igh_driver_fix.cpp
排查电机反复失能，AI修改了一版

实测：
```
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$  ./igh_driver_fix 
Using priority 99.
sched_setscheduler failed
: Operation not permitted
Activating master...
Activating master...
All slaves have reached OP state
status=0x1288 (Fault) err=0xa000 opmode=9 cw=0x0080 actPos=1611522 actVel=-21 targetVel=10000
status=0x1288 (Fault) err=0xa000 opmode=0 cw=0x0080 actPos=1611523 actVel=29 targetVel=10000
status=0x12d0 (Switch on disabled) err=0x0000 opmode=9 cw=0x0006 actPos=1611524 actVel=18 targetVel=10000
status=0x12b1 (Ready to switch on) err=0x0000 opmode=9 cw=0x0007 actPos=1611521 actVel=13 targetVel=10000
status=0x12b3 (Switched on) err=0x0000 opmode=9 cw=0x000f actPos=1611523 actVel=0 targetVel=10000
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1611524 actVel=-3 targetVel=10000
igh_latency period=994301..1052052 ns (-5.7..+52.1 us) wake=51714..58072 ns exec=6416..20125 ns total=58424..74114 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1620697 actVel=10111 targetVel=10000
igh_latency period=995760..1005092 ns (-4.2..+5.1 us) wake=51722..57108 ns exec=6416..16625 ns total=58425..71550 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1630687 actVel=10198 targetVel=10000
igh_latency period=995759..1004801 ns (-4.2..+4.8 us) wake=51719..57099 ns exec=6416..19542 ns total=58426..71780 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1640689 actVel=9778 targetVel=10000
igh_latency period=994884..1006259 ns (-5.1..+6.3 us) wake=51735..58300 ns exec=6416..18375 ns total=58420..76675 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1650689 actVel=10051 targetVel=10000
igh_latency period=994885..1005092 ns (-5.1..+5.1 us) wake=51749..57438 ns exec=6416..19251 ns total=58421..73189 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1660696 actVel=10159 targetVel=10000
igh_latency period=995468..1004509 ns (-4.5..+4.5 us) wake=51725..56611 ns exec=6416..16625 ns total=58436..72070 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1670705 actVel=9751 targetVel=10000
^C
Releasing master...
Killed
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ sudo ./igh_driver_fix 
Using priority 99.
Activating master...
Activating master...
All slaves have reached OP state
status=0x1288 (Fault) err=0xa000 opmode=9 cw=0x0080 actPos=1674629 actVel=0 targetVel=10000
status=0x1288 (Fault) err=0xa000 opmode=0 cw=0x0080 actPos=1674630 actVel=5 targetVel=10000
status=0x12d0 (Switch on disabled) err=0x0000 opmode=9 cw=0x0006 actPos=1674630 actVel=-1 targetVel=10000
status=0x12b1 (Ready to switch on) err=0x0000 opmode=9 cw=0x0007 actPos=1674629 actVel=-16 targetVel=10000
status=0x12b3 (Switched on) err=0x0000 opmode=9 cw=0x000f actPos=1674630 actVel=-36 targetVel=10000
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1674629 actVel=-17 targetVel=10000
igh_latency period=998676..1001593 ns (-1.3..+1.6 us) wake=0..1593 ns exec=6416..26250 ns total=6452..26899 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1683802 actVel=10015 targetVel=10000
igh_latency period=998385..1001592 ns (-1.6..+1.6 us) wake=0..1736 ns exec=6416..17792 ns total=6421..17923 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1693804 actVel=9920 targetVel=10000
igh_latency period=998968..1001009 ns (-1.0..+1.0 us) wake=0..1148 ns exec=6416..30334 ns total=6418..30956 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1703798 actVel=9856 targetVel=10000
igh_latency period=999260..1001009 ns (-0.7..+1.0 us) wake=0..1101 ns exec=6416..17791 ns total=6461..18271 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1713795 actVel=10162 targetVel=10000
igh_latency period=999259..1000718 ns (-0.7..+0.7 us) wake=0..927 ns exec=6416..21292 ns total=6434..21923 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1723802 actVel=9949 targetVel=10000
igh_latency period=998676..1001301 ns (-1.3..+1.3 us) wake=0..1422 ns exec=6416..17209 ns total=6432..17223 ns
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=1733806 actVel=9863 targetVel=10000
^C
Releasing master...
Killed
```

### Program Features
- Automatic state machine transition for slave devices (from INIT to OPERATION ENABLED)
- In OPERATION ENABLED state, the motor will run at target velocity 10000 (units depend on drive configuration)
- Real-time display of actual motor velocity and status information

### Important Notes
1. Ensure EtherCAT master is properly started before running the program
2. Verify network configuration and slave device detection
3. Check slave status using:
```bash
ethercat slaves
```

## Safety Precautions
- Ensure the motor is securely mounted before first run
- Start with lower velocity values for testing
- Make sure emergency stop measures are in place

## Troubleshooting
If you encounter issues, check:
1. EtherCAT master status
2. Network connection
3. Slave device status
4. Program execution permissions (sudo required)

## License
This project is open source and provided for reference only. Please use with caution. The author is not responsible for any damages or losses.
 