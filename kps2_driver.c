#include "kps_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lenart");
MODULE_DESCRIPTION("A simple kps2 module");
MODULE_VERSION("0.01");

static int irq = 1;
static unsigned int cookie = 0xdeadd00d;

struct kps_data kps_data = {
	.lock = __MUTEX_INITIALIZER(kps_data.lock),
	.entries = LIST_HEAD_INIT(kps_data.entries),
};

typedef enum {
	None = 0,
	RShift = 1 << 0,
	LShift = 1 << 1,
	Capslock = 1 << 2,
	RCtrl = 1 << 3,
	LCtrl = 1 << 4,
	RAlt = 1 << 5,
	LAlt = 1 << 6,
} Modifiers;

Modifiers gmodifiers = None;

static void check_for_modifiers(unsigned char scancode)
{
	switch (scancode) {
		// presses
		case 0x2A:
			gmodifiers |= LShift;
			break;
		case 0x1D:
			gmodifiers |= LCtrl;
			break;
		case 0x38:
			gmodifiers |= LAlt;
			break;
		case 0x36:
			gmodifiers |= RShift;
			break;
		// releases
		case 0xAA:
			gmodifiers &= ~LShift;
			break;
		case 0x9D:
			gmodifiers &= ~LCtrl;
			break;
		case 0xB8:
			gmodifiers &= ~LAlt;
			break;
		case 0xB6:
			gmodifiers &= ~RShift;
			break;
		default:
			break;
	}
}

static irqreturn_t irq_handler(int ir, void* dev_id)
{
	static unsigned char 	scancode;
//	unsigned char			c;

	scancode = inb(0x60);
	check_for_modifiers(scancode);
	if (new_node(scancode))
		return -ENOMEM;

//	c = kbdus[scancode];
//	if (c >= 0x20 && c <= 0x7e) {
//		printk(KERN_INFO "ps2: key has been pressed: %c\n", c);
//	}

	return IRQ_HANDLED;
}

static int __init keyboard_ps2(void)
{
	int	ret;

	printk(KERN_INFO "%s: init...\n", __func__);
	ret = register_misc_device();
	if (ret)
		return ret;
	ret = request_irq(irq, irq_handler, IRQF_SHARED, "Keyboard key", &cookie);
	if (ret) {
		pr_err("%s: failed to request irq %d\n", __func__, ret);
		deregister_misc_device();
		return ret;
	}
	printk(KERN_INFO "%s: irq(%d) has been trigerred, res: %d\n", __func__, irq, ret);
	return ret;
}

static void __exit mod_exit(void)
{
	printk(KERN_INFO "%s: cleaning up...\n", __func__);
	synchronize_irq(irq);
	free_irq(irq, &cookie);
	deregister_misc_device();
	printk(KERN_INFO "%s: cleaned up...\n", __func__);
}

module_init(keyboard_ps2);
module_exit(mod_exit);
