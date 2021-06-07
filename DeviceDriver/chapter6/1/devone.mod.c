#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xdad1276d, "module_layout" },
	{ 0x34f8869a, "class_destroy" },
	{ 0xbeaa1778, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xf612a547, "cdev_add" },
	{ 0xa36f8ae4, "cdev_init" },
	{ 0xa245c948, "cdev_del" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x9425caca, "_raw_write_lock" },
	{ 0xc5850110, "printk" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0xc6cbbc89, "capable" },
	{ 0x3b697738, "_raw_read_lock" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xd256e620, "kmem_cache_alloc_trace" },
	{ 0xa307fb1a, "kmalloc_caches" },
	{ 0x37a0cba, "kfree" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "A31BB0ED20A6252BDAC990B");
