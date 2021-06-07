cmd_/home/pi/kernel/kernel_driver/modules.order := {   echo /home/pi/kernel/kernel_driver/hello.ko; :; } | awk '!x[$$0]++' - > /home/pi/kernel/kernel_driver/modules.order
