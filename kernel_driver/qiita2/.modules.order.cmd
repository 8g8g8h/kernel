cmd_/home/pi/kernel/kernel_driver/qiita2/modules.order := {   echo /home/pi/kernel/kernel_driver/qiita2/mydevicemodule.ko; :; } | awk '!x[$$0]++' - > /home/pi/kernel/kernel_driver/qiita2/modules.order
