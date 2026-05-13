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

#### 编译
把 igh_driver_fix 改成 CMake 参数控制频率了。

现在默认还是 1K：
```
cd /home/cat/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build

cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=1000 -DIGH_ENABLE_DC=OFF ..
cmake --build . --target igh_driver_fix

cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=1000 -DIGH_ENABLE_DC=ON ..
cmake --build . --target igh_driver_fix


cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=2000 -DIGH_ENABLE_DC=OFF ..
cmake --build . --target igh_driver_fix

cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=2000 -DIGH_ENABLE_DC=ON ..
cmake --build . --target igh_driver_fix


cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=4000 -DIGH_ENABLE_DC=OFF ..
cmake --build . --target igh_driver_fix

# 4K后开DC 就不行了
cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=4000 -DIGH_ENABLE_DC=ON ..
cmake --build . --target igh_driver_fix


cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=8000 -DIGH_ENABLE_DC=OFF ..
cmake --build . --target igh_driver_fix

cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=8000 -DIGH_ENABLE_DC=ON ..
cmake --build . --target igh_driver_fix


sudo ./igh_driver_fix

```
#### 实测

2K DC ON  -> PERIOD_NS = 500000 ns -> 可以 OP
4K DC ON  -> PERIOD_NS = 250000 ns -> 卡 PREOP


rt[1s]
表示这是过去 1 秒内，也就是大约 1000 个 EtherCAT 周期的统计。

jitter=-1.3..+1.6 us
表示实际周期相对 1ms 的偏差。
比如 -1.3us 是某次周期比 1ms 短 1.3 微秒，+1.6us 是某次周期比 1ms 长 1.6 微秒。这个值越接近 0 越好。

wake=0.0..1.6 us
表示程序计划在某个时间点醒来，但实际醒来晚了多少。
最大 1.6us 说明系统调度非常稳。

run=6.4..26.3 us
表示从程序醒来，到完成 ecrt_master_send() 的耗时。
这里包含 EtherCAT 收包、处理 PDO、写控制字、同步 DC、发包等用户态循环逻辑。

total=6.5..26.9 us
表示从“理论应该醒来的时间点”到“EtherCAT 发包完成”的总耗时。
它大致等于 wake + run，这是最重要的综合指标。

>50us=0 >100us=0
表示过去 1 秒内，没有任何周期的 total 超过 50us 或 100us。
如果一直是 0，说明 1kHz 控制循环实时性很好。


实测：

##### 2K
```
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=2000 -DIGH_ENABLE_DC=ON ..
cmake --build . --target igh_driver_fix
-- Configuring done
-- Generating done
-- Build files have been written to: /home/cat/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build
Consolidate compiler generated dependencies of target igh_driver_fix
[ 50%] Building CXX object CMakeFiles/igh_driver_fix.dir/src/igh_driver_fix.cpp.o
[100%] Linking CXX executable igh_driver_fix
[100%] Built target igh_driver_fix
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ sudo ./igh_driver_fix
Using priority 99.
IGH cycle config: startup=1000 Hz/1000000 ns (1000.000 us), run=2000 Hz/500000 ns (500.000 us)
Activating master...
Activating master...
All slaves have reached OP state
rt_log: freq=2000 Hz, target_cycle=500.0 us, stats_window=2000 cycles
rt_log: jitter=actual cycle error, wake=sleep latency, run=wake-to-send, total=scheduled-to-send, all in us
status=0x1288 (Fault) err=0xa000 opmode=0 cw=0x0080 actPos=609115 actVel=0 targetVel=10000
status=0x12d0 (Switch on disabled) err=0x0000 opmode=9 cw=0x0006 actPos=609111 actVel=0 targetVel=10000
status=0x12b1 (Ready to switch on) err=0x0000 opmode=9 cw=0x0007 actPos=609113 actVel=-24 targetVel=10000
status=0x12b3 (Switched on) err=0x0000 opmode=9 cw=0x000f actPos=609114 actVel=15 targetVel=10000
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=609115 actVel=-1 targetVel=10000
rt[2000Hz/500.0us] jitter=-0.7..+0.8 us | wake=0.0..1.0 us | run=6.4..19.2 us | total=6.4..19.3 us | >50us=0 >100us=0
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=618262 actVel=9997 targetVel=10000
rt[2000Hz/500.0us] jitter=-1.0..+1.1 us | wake=0.0..1.2 us | run=6.1..19.5 us | total=6.2..19.7 us | >50us=0 >100us=0
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=628265 actVel=9978 targetVel=10000
rt[2000Hz/500.0us] jitter=-1.2..+1.1 us | wake=0.0..1.2 us | run=6.1..20.7 us | total=6.4..20.8 us | >50us=0 >100us=0
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=638269 actVel=9819 targetVel=10000
rt[2000Hz/500.0us] jitter=-1.0..+0.8 us | wake=0.0..1.1 us | run=6.1..29.5 us | total=6.4..29.6 us | >50us=0 >100us=0
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=648275 actVel=10083 targetVel=10000
rt[2000Hz/500.0us] jitter=-0.7..+1.1 us | wake=0.0..1.2 us | run=6.4..19.2 us | total=6.4..19.4 us | >50us=0 >100us=0
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=658278 actVel=10033 targetVel=10000
rt[2000Hz/500.0us] jitter=-1.2..+1.1 us | wake=0.0..1.3 us | run=6.1..29.2 us | total=6.4..29.3 us | >50us=0 >100us=0
status=0x16b7 (Operation enabled) err=0x0000 opmode=9 cw=0x000f actPos=668279 actVel=9741 targetVel=10000
```
##### 4K
```
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ cmake -DIGH_STARTUP_FREQUENCY=1000 -DIGH_FREQUENCY=4000 -DIGH_ENABLE_DC=ON ..
cmake --build . --target igh_driver_fix
-- Configuring done
-- Generating done
-- Build files have been written to: /home/cat/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build
Consolidate compiler generated dependencies of target igh_driver_fix
[ 50%] Building CXX object CMakeFiles/igh_driver_fix.dir/src/igh_driver_fix.cpp.o
[100%] Linking CXX executable igh_driver_fix
[100%] Built target igh_driver_fix
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ sudo ./igh_driver_fix
Using priority 99.
IGH cycle config: startup=1000 Hz/1000000 ns (1000.000 us), run=4000 Hz/250000 ns (250.000 us)
Activating master...
Activating master...
waiting OP: startup=1000Hz/1000000ns run=4000Hz/250000ns master_slaves=1 master_al=0x02 link=1 slave_al=0x02 online=1 operational=0 domain_wc=0 domain_state=0
waiting OP: startup=1000Hz/1000000ns run=4000Hz/250000ns master_slaves=1 master_al=0x02 link=1 slave_al=0x02 online=1 operational=0 domain_wc=0 domain_state=0
waiting OP: startup=1000Hz/1000000ns run=4000Hz/250000ns master_slaves=1 master_al=0x02 link=1 slave_al=0x02 online=1 operational=0 domain_wc=0 domain_state=0
^C
Releasing master...
Killed
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ sudo ./igh_driver_fix
Using priority 99.
IGH cycle config: startup=1000 Hz/1000000 ns (1000.000 us), run=4000 Hz/250000 ns (250.000 us)
Activating master...
Activating master...
waiting OP: startup=1000Hz/1000000ns run=4000Hz/250000ns master_slaves=1 master_al=0x02 link=1 slave_al=0x02 online=1 operational=0 domain_wc=0 domain_state=0
waiting OP: startup=1000Hz/1000000ns run=4000Hz/250000ns master_slaves=1 master_al=0x02 link=1 slave_al=0x02 online=1 operational=0 domain_wc=0 domain_state=0
^C
Releasing master...
Killed
cat@lubancat:~/fansihub/igh_ws/unionai_arm/arm_hardware/eRob_IGH_EtherCAT/build$ 
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
 