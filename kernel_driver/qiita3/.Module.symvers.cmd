cmd_/home/pi/kernel/kernel_driver/qiita3/Module.symvers := sed 's/ko$$/o/' /home/pi/kernel/kernel_driver/qiita3/modules.order | scripts/mod/modpost -m -a   -o /home/pi/kernel/kernel_driver/qiita3/Module.symvers -e -i Module.symvers   -T -
