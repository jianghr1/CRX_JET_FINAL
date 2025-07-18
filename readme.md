## 指令
|指令ID|负责线程|备注|
|--|--|--|
|M100| [Pump](Tasks/Pump.c) | 墨水泵1【顺/逆】以【deg/s】转【deg】|
|M101| [Pump](Tasks/Pump.c) | 墨水泵2【顺/逆】以【deg/s】转【deg】|
|M102| [Pump](Tasks/Pump.c) | 清洁泵【顺/逆】以【deg/s】转【deg】|
|M103| [Pump](Tasks/Pump.c) | 废液泵【顺/逆】以【deg/s】转【deg】|
|M104| [Vac](Tasks/Vac.c) | 正负压【顺/逆】以【deg/s】转【deg】 |
|M105| [Vac](Tasks/Vac.c) | 目标气压相对大气为【帕】|
|M106| [Vac](Tasks/Vac.c) | 自动压力维持工作【关/开】 |
|M107| [Pump](Tasks/Pump.c) | 墨水泵1【顺/逆】以【deg/s】转到触发 |
|M108| [Pump](Tasks/Pump.c) | 墨水泵2【顺/逆】以【deg/s】转到触发 |
|G110| [Motor](Tasks/Motor.c) |  |
|G111| [Motor](Tasks/Motor.c) |  |
|G112| [Motor](Tasks/Motor.c) |  |
|G113| [Motor](Tasks/Motor.c) | Z两轴同步动 |
|G114| [Motor](Tasks/Motor.c) | X回零 |
|G115| [Motor](Tasks/Motor.c) | Z回零 |



## 线程
### 管理线程 `StartDefaultTask(void*)`
负责接受 USB CDC 串口回调解包得到的任务
### 电机线程 `StartMotorTask(void*)`


## TMC2209驱动
### 定义

TIM1 控制X轴运动