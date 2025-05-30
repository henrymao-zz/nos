/*
 * Copyright (C)  Jostar yang <jostar_yang@accton.com.tw>
 *
 * This module supports the accton cpld that hold the channel select
 * mechanism for other i2c slave devices, such as SFP.
 * This includes the:
 *	 Accton g48c210r CPLD
 *
 * Based on:
 *	pca954x.c from Kumar Gala <galak@kernel.crashing.org>
 * Copyright (C) 2006
 *
 * Based on:
 *	pca954x.c from Ken Harrenstien
 * Copyright (C) 2004 Google, Inc. (Ken Harrenstien)
 *
 * Based on:
 *	i2c-virtual_cb.c from Brian Kuschak <bkuschak@yahoo.com>
 * and
 *	pca9540.c from Jean Delvare <khali@linux-fr.org>.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/stat.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>


#define I2C_RW_RETRY_COUNT				10
#define I2C_RW_RETRY_INTERVAL			60 /* ms */
#define FAN_DUTY_CYCLE_REG_MASK         0x1F
#define FAN_MAX_DUTY_CYCLE              100
#define FAN_REG_VAL_TO_SPEED_RPM_STEP   114 // R.P.M value = read value x3.79*60/2

#define NUM_THERMAL_SENSORS     (3)     /* Get sum of this number of sensors.*/
#define THERMAL_SENSORS_DRIVER  "lm75"
#define THERMAL_SENSORS_ADDRS   {0x48, 0x4a, 0x4b}

static LIST_HEAD(cpld_client_list);
static struct mutex     list_lock;

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

enum cpld_type {
    g48c210r_cpld,
};
enum fan_id {
    FAN1_ID,
    FAN2_ID,
    FAN3_ID,   
};

static const u8 fan_reg[] = {   
    0x87,      /* fan status, fan direction */    
    0x1A,      /* fan PWM(for fan1 ,fan2) */
    0x1B,      /* fan PWM(for fan1 ,fan2) */
    0x88,      /* front fan1 speed(rpm) */
    0x89,      /* front fan2 speed(rpm) */
    0x8A,      /* front fan3 speed(rpm) */
    0x20,      /*fan fault*/
};

struct g48c210r_cpld_data {
    enum cpld_type   type;
    struct device   *hwmon_dev;
    struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_fan_val[ARRAY_SIZE(fan_reg)]; /* Register value */
    int              system_temp;    /*In unit of mini-Celsius*/
    int              sensors_found;
};



static const struct i2c_device_id g48c210r_cpld_id[] = {
    { "g48c210r_cpld", g48c210r_cpld},
    { }
};
MODULE_DEVICE_TABLE(i2c, g48c210r_cpld_id);

#define TRANSCEIVER_PRESENT_ATTR_ID(index)   	MODULE_PRESENT_##index
#define TRANSCEIVER_RESET_ATTR_ID(index)        MODULE_RESET_##index
#define TRANSCEIVER_LPMODE_ATTR_ID(index)       MODULE_LPMODE_##index
#define TRANSCEIVER_PRESENT_ATTR_ID(index)      MODULE_PRESENT_##index
#define TRANSCEIVER_TXDISABLE_ATTR_ID(index)   	MODULE_TXDISABLE_##index
#define TRANSCEIVER_RXLOS_ATTR_ID(index)        MODULE_RXLOS_##index
#define TRANSCEIVER_TXFAULT_ATTR_ID(index)      MODULE_TXFAULT_##index
#define FAN_SPEED_RPM_ATTR_ID(index)   	        FAN_SPEED_RPM_##index
#define FAN_DIRECTION_ID(index)                 FAN_DIRECTION_##index
#define FAN_PRESENT_ATTR_ID(index)   	        FAN_PRESENT_##index
#define FAN_FAULT_ATTR_ID(index)                FAN_FAULT_##index

enum g48c210r_cpld_sysfs_attributes {
	CPLD_VERSION,
	ACCESS,
	/* transceiver attributes */	
	TRANSCEIVER_RXLOS_ATTR_ID(49),
	TRANSCEIVER_RXLOS_ATTR_ID(50),
	TRANSCEIVER_RXLOS_ATTR_ID(51),
	TRANSCEIVER_RXLOS_ATTR_ID(52),
	TRANSCEIVER_TXFAULT_ATTR_ID(49),
	TRANSCEIVER_TXFAULT_ATTR_ID(50),
	TRANSCEIVER_TXFAULT_ATTR_ID(51),
	TRANSCEIVER_TXFAULT_ATTR_ID(52),
	TRANSCEIVER_PRESENT_ATTR_ID(49),
	TRANSCEIVER_PRESENT_ATTR_ID(50),
	TRANSCEIVER_PRESENT_ATTR_ID(51),
	TRANSCEIVER_PRESENT_ATTR_ID(52),
	TRANSCEIVER_PRESENT_ATTR_ID(53),
	TRANSCEIVER_PRESENT_ATTR_ID(54),
	TRANSCEIVER_RESET_ATTR_ID(53),
	TRANSCEIVER_RESET_ATTR_ID(54),
	TRANSCEIVER_LPMODE_ATTR_ID(53),
	TRANSCEIVER_LPMODE_ATTR_ID(54),
	TRANSCEIVER_TXDISABLE_ATTR_ID(49),
	TRANSCEIVER_TXDISABLE_ATTR_ID(50),
	TRANSCEIVER_TXDISABLE_ATTR_ID(51),
	TRANSCEIVER_TXDISABLE_ATTR_ID(52),
	FAN_PRESENT_ATTR_ID(1),
	FAN_PRESENT_ATTR_ID(2),
	FAN_PRESENT_ATTR_ID(3),	
	FAN_SPEED_RPM_ATTR_ID(1),
	FAN_SPEED_RPM_ATTR_ID(2),
	FAN_SPEED_RPM_ATTR_ID(3),
	FAN_DIRECTION_ID(1),
	FAN_DIRECTION_ID(2),
	FAN_DIRECTION_ID(3),
	FAN_FAULT_ATTR_ID(1),
	FAN_FAULT_ATTR_ID(2),
	FAN_FAULT_ATTR_ID(3),
	FAN_DUTY_CYCLE_PERCENTAGE,
};

/* sysfs attributes for hwmon 
 */
static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf);
static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t set_qsfp(struct device *dev, struct device_attribute *da,
                        const char *buf, size_t count);
static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count);
static ssize_t show_version(struct device *dev, struct device_attribute *da,
             char *buf);
static int g48c210r_cpld_read_internal(struct i2c_client *client, u8 reg);
static int g48c210r_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value);

/*fan sysfs*/
static struct g48c210r_cpld_data *g48c210r_fan_update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count);

/* transceiver attributes */
#define DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index); \
	static SENSOR_DEVICE_ATTR(module_tx_disable_##index, S_IRUGO | S_IWUSR, show_status, set_tx_disable, MODULE_TXDISABLE_##index); \
	static SENSOR_DEVICE_ATTR(module_rx_los_##index, S_IRUGO, show_status, NULL, MODULE_RXLOS_##index);  \
	static SENSOR_DEVICE_ATTR(module_tx_fault_##index, S_IRUGO, show_status, NULL, MODULE_TXFAULT_##index); 
	
#define DECLARE_SFP_TRANSCEIVER_ATTR(index)  \
    &sensor_dev_attr_module_present_##index.dev_attr.attr, \
	&sensor_dev_attr_module_tx_disable_##index.dev_attr.attr, \
	&sensor_dev_attr_module_rx_los_##index.dev_attr.attr,     \
	&sensor_dev_attr_module_tx_fault_##index.dev_attr.attr
	
#define DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(index) \
    static SENSOR_DEVICE_ATTR(module_lpmode_##index, S_IRUGO | S_IWUSR, show_status, set_qsfp, MODULE_LPMODE_##index); \
    static SENSOR_DEVICE_ATTR(module_reset_##index, S_IRUGO | S_IWUSR, show_status, set_qsfp, MODULE_RESET_##index); \
    static SENSOR_DEVICE_ATTR(module_present_##index, S_IRUGO, show_status, NULL, MODULE_PRESENT_##index);

#define DECLARE_QSFP_TRANSCEIVER_ATTR(index)  \
    &sensor_dev_attr_module_lpmode_##index.dev_attr.attr, \
    &sensor_dev_attr_module_reset_##index.dev_attr.attr, \
    &sensor_dev_attr_module_present_##index.dev_attr.attr


#define DECLARE_FAN_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan_present_##index, S_IRUGO, fan_show_value, NULL, FAN_PRESENT_##index); \
    static SENSOR_DEVICE_ATTR(fan_fault_##index,   S_IRUGO, fan_show_value, NULL, FAN_FAULT_##index); \
    static SENSOR_DEVICE_ATTR(fan_speed_rpm_##index, S_IRUGO, fan_show_value, NULL, FAN_SPEED_RPM_##index); \
    static SENSOR_DEVICE_ATTR(fan##index##_input, S_IRUGO, fan_show_value, NULL, FAN_SPEED_RPM_##index);\
    static SENSOR_DEVICE_ATTR(fan_direction_##index, S_IRUGO, fan_show_value, NULL, FAN_DIRECTION_##index);
    
#define DECLARE_FAN_ATTR(index)  \
    &sensor_dev_attr_fan_present_##index.dev_attr.attr, \
    &sensor_dev_attr_fan_fault_##index.dev_attr.attr, \
    &sensor_dev_attr_fan_speed_rpm_##index.dev_attr.attr, \
    &sensor_dev_attr_fan##index##_input.dev_attr.attr, \
    &sensor_dev_attr_fan_direction_##index.dev_attr.attr
    
#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
    static SENSOR_DEVICE_ATTR(fan_duty_cycle_percentage, S_IWUSR | S_IRUGO, fan_show_value, set_duty_cycle, FAN_DUTY_CYCLE_PERCENTAGE);
#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) &sensor_dev_attr_fan_duty_cycle_percentage.dev_attr.attr



static SENSOR_DEVICE_ATTR(version, S_IRUGO, show_version, NULL, CPLD_VERSION);
static SENSOR_DEVICE_ATTR(access, S_IWUSR, NULL, access, ACCESS);



/* transceiver attributes */
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(49);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(50);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(51);
DECLARE_SFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(52);
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(53);
DECLARE_QSFP_TRANSCEIVER_SENSOR_DEVICE_ATTR(54);
/* fan attributes */
DECLARE_FAN_SENSOR_DEV_ATTR(1);
DECLARE_FAN_SENSOR_DEV_ATTR(2);
DECLARE_FAN_SENSOR_DEV_ATTR(3);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(1);

static struct attribute *g48c210r_cpld_attributes[] = {
    &sensor_dev_attr_version.dev_attr.attr,
    &sensor_dev_attr_access.dev_attr.attr,
	DECLARE_SFP_TRANSCEIVER_ATTR(49),
	DECLARE_SFP_TRANSCEIVER_ATTR(50),
	DECLARE_SFP_TRANSCEIVER_ATTR(51),
	DECLARE_SFP_TRANSCEIVER_ATTR(52),
	DECLARE_QSFP_TRANSCEIVER_ATTR(53),
	DECLARE_QSFP_TRANSCEIVER_ATTR(54),
	DECLARE_FAN_ATTR(1),
    DECLARE_FAN_ATTR(2),
    DECLARE_FAN_ATTR(3),
	DECLARE_FAN_DUTY_CYCLE_ATTR(1),
	NULL
};

static const struct attribute_group g48c210r_cpld_group = {
	.attrs = g48c210r_cpld_attributes,
};


static ssize_t show_status(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct g48c210r_cpld_data *data = i2c_get_clientdata(client);
    int status = 0;
    u8 reg = 0, mask = 0, revert = 0;
    
    switch (attr->index)
    {
        case MODULE_RXLOS_49 ... MODULE_RXLOS_50:
            reg=0x5;
            mask = 0x1<< (attr->index==MODULE_RXLOS_49?4:0);
            break; 
        case MODULE_TXFAULT_49 ... MODULE_TXFAULT_50:
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_TXFAULT_49?5:1);
            break;           
        case MODULE_PRESENT_49 ... MODULE_PRESENT_50:            
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_PRESENT_49?6:2);
            break;       
        case MODULE_TXDISABLE_49 ... MODULE_TXDISABLE_50:
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_49?7:3);
            break;
            
        case MODULE_RXLOS_51 ... MODULE_RXLOS_52:
            reg=0x6;
            mask = 0x1<< (attr->index==MODULE_RXLOS_51?4:0);
            break; 
        case MODULE_TXFAULT_51 ... MODULE_TXFAULT_52:
            reg=0x6;
            mask=0x1 << (attr->index==MODULE_TXFAULT_51?5:1);
            break;           
        case MODULE_PRESENT_51 ... MODULE_PRESENT_52:            
            reg=0x6;
            mask=0x1 << (attr->index==MODULE_PRESENT_51?6:2);
            break;       
        case MODULE_TXDISABLE_51 ... MODULE_TXDISABLE_52:
            reg=0x6;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_51?7:3);
            break;
        case MODULE_PRESENT_53 ... MODULE_PRESENT_54:
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_PRESENT_53?0:4);
            break;
        case MODULE_RESET_53 ... MODULE_RESET_54:
            reg=0x21;
            mask=0x1 << (attr->index==MODULE_RESET_53?3:7);
            revert = 1;
            break;
        case MODULE_LPMODE_53 ... MODULE_LPMODE_54:
            reg = 0x21;
            mask = 0x1 << (attr->index==MODULE_LPMODE_53?2:6);
            revert = 0;
            break;
	    default:
		    return 0;
    }

    if( attr->index >= MODULE_PRESENT_49 && attr->index <= MODULE_PRESENT_54 )        
    {
        revert = 1;
    }

    mutex_lock(&data->update_lock);
	status = g48c210r_cpld_read_internal(client, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);
    
	return sprintf(buf, "%d\n", revert ? !(status & mask) : !!(status & mask));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t set_qsfp(struct device *dev, struct device_attribute *da,
                        const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct g48c210r_cpld_data *data = i2c_get_clientdata(client);
    long disable;
    int status;
    u8 reg = 0, mask = 0, revert = 0;

    status = kstrtol(buf, 10, &disable);
    if (status) {
        return status;
    }
    reg  = 0x21;
    switch (attr->index)
    {
    case MODULE_RESET_53 ... MODULE_RESET_54:
        mask=0x1 << (attr->index==MODULE_RESET_53?3:7);
        revert = 1;
        break;
    case MODULE_LPMODE_53 ... MODULE_LPMODE_54:
        mask=0x1 << (attr->index==MODULE_LPMODE_53?2:6);
        revert = 0;
        break;
    default:
        return 0;
    }

    disable = revert ? disable : !disable;
    /* Read current status */
    mutex_lock(&data->update_lock);
    status = g48c210r_cpld_read_internal(client, reg);
    if (unlikely(status < 0)) {
        goto exit;
    }
    if (disable) {
        status &= ~mask;
    }
    else {
        status |= mask;
    }
    status = g48c210r_cpld_write_internal(client, reg, status);
    if (unlikely(status < 0)) {
        goto exit;
    }

    mutex_unlock(&data->update_lock);
    return count;

exit:
    mutex_unlock(&data->update_lock);
    return status;
}

static ssize_t set_tx_disable(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct g48c210r_cpld_data *data = i2c_get_clientdata(client);
	long disable;
	int status;
    u8 reg = 0, mask = 0;
     
	status = kstrtol(buf, 10, &disable);
	if (status) {
		return status;
	}
    reg  = 0x9;
    switch (attr->index)
    {
         case MODULE_TXDISABLE_49 ... MODULE_TXDISABLE_50:
            reg=0x5;
            mask=0x1 << (attr->index==MODULE_TXDISABLE_49?7:3);
            break;
        case MODULE_TXDISABLE_51 ... MODULE_TXDISABLE_52:
            reg=0x6;
            mask=0x1 << (attr->index==MODULE_TXFAULT_51?7:3);
            break;
     
	    default:
		    return 0;
    }

    /* Read current status */
    mutex_lock(&data->update_lock);
	status = g48c210r_cpld_read_internal(client, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}
	/* Update tx_disable status */
	if (disable) {
		status |= mask;
	}
	else {
	    status &= ~mask;
	}
    status = g48c210r_cpld_write_internal(client, reg, status);
	if (unlikely(status < 0)) {
		goto exit;
	}
    
    mutex_unlock(&data->update_lock);
    return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static ssize_t access(struct device *dev, struct device_attribute *da,
			const char *buf, size_t count)
{
	int status;
	u32 addr, val;
    struct i2c_client *client = to_i2c_client(dev);
    struct g48c210r_cpld_data *data = i2c_get_clientdata(client);
    
	if (sscanf(buf, "0x%x 0x%x", &addr, &val) != 2) {
		return -EINVAL;
	}

	if (addr > 0xFF || val > 0xFF) {
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);
	status = g48c210r_cpld_write_internal(client, addr, val);
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);
	return count;

exit:
	mutex_unlock(&data->update_lock);
	return status;
}

static void g48c210r_cpld_add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);

    if (!node) {
        dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
        return;
    }

    node->client = client;

	mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void g48c210r_cpld_remove_client(struct i2c_client *client)
{
    struct list_head    *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }

	mutex_unlock(&list_lock);
}

static ssize_t show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int val = 0;
    struct i2c_client *client = to_i2c_client(dev);
	
	val = i2c_smbus_read_byte_data(client, 0x1);

    if (val < 0) {
        dev_dbg(&client->dev, "cpld(0x%x) reg(0x1) err %d\n", client->addr, val);
    }
	
    return sprintf(buf, "%d\n", val);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
    reg_val &= FAN_DUTY_CYCLE_REG_MASK;
    return ((u32)(reg_val) * 625)/ 100;
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle)
{
    return ((u32)duty_cycle * 100 / 625);
}

static u32 reg_val_to_speed_rpm(u8 reg_val)
{
   return (u32)reg_val * FAN_REG_VAL_TO_SPEED_RPM_STEP;
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
                              const char *buf, size_t count)
{
    int error, value;
    struct i2c_client *client = to_i2c_client(dev);

    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;

    if (value < 0 || value > FAN_MAX_DUTY_CYCLE)
        return -EINVAL;
    
    g48c210r_cpld_write_internal(client, fan_reg[1], duty_cycle_to_reg_val(value));
    g48c210r_cpld_write_internal(client, fan_reg[2], duty_cycle_to_reg_val(value));
    return count;
}

static u8 reg_val_to_direction(u8 reg_val, enum fan_id id)
{
    u8 mask = (1 << id+4);

    reg_val &= mask;

    return reg_val ? 1 : 0;
}

static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
    u8 mask = (1 << id);

    reg_val &= mask;

    return reg_val ? 0 : 1;
}

static u8 is_fan_fault(struct g48c210r_cpld_data *data, enum fan_id id)
{
    u8 ret = 1;
    
    if(id > FAN3_ID)
        return 1;
    /* Check if the speed of front or rear fan is ZERO,
     */
    if (reg_val_to_speed_rpm(data->reg_fan_val[id+3]))
    {
           
        ret = 0;
    }

    return ret;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
                              char *buf)
{
    u32 duty_cycle;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct g48c210r_cpld_data *data = g48c210r_fan_update_device(dev);
    ssize_t ret = 0;
    
    if (data->valid) {
        switch (attr->index)
        {
            case FAN_PRESENT_1:
            case FAN_PRESENT_2:
            case FAN_PRESENT_3:
                ret = sprintf(buf, "%d\n",
                          reg_val_to_is_present(data->reg_fan_val[0],
                                                attr->index - FAN_PRESENT_1));
                break;
            case FAN_DUTY_CYCLE_PERCENTAGE:        
                duty_cycle = reg_val_to_duty_cycle(data->reg_fan_val[1]);
                ret = sprintf(buf, "%u\n", duty_cycle);
                break;
            case FAN_SPEED_RPM_1:
            case FAN_SPEED_RPM_2:
            case FAN_SPEED_RPM_3:
                ret = sprintf(buf, "%u\n", reg_val_to_speed_rpm(data->reg_fan_val[attr->index-FAN_SPEED_RPM_1+3]));
                break;   
            case FAN_FAULT_1:
            case FAN_FAULT_2:
            case FAN_FAULT_3:        
                ret = sprintf(buf, "%d\n", is_fan_fault(data, attr->index - FAN_FAULT_1));
                break;     
            case FAN_DIRECTION_1:
            case FAN_DIRECTION_2:
            case FAN_DIRECTION_3:
                ret = sprintf(buf, "%d\n",
                              reg_val_to_direction(data->reg_fan_val[0],
                              attr->index - FAN_DIRECTION_1));
                break;     
            default:
                break;
        }
    }

    return ret;
}

static struct g48c210r_cpld_data *g48c210r_fan_update_device(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct g48c210r_cpld_data *data = i2c_get_clientdata(client);

    mutex_lock(&data->update_lock);

    if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
            !data->valid) {
        int i;

        dev_dbg(&client->dev, "Starting g48c210r_fan update\n");
        data->valid = 0;

        /* Update fan data
         */
        for (i = 0; i < ARRAY_SIZE(data->reg_fan_val); i++) {
            int status = g48c210r_cpld_read_internal(client, fan_reg[i]);
            if (status < 0) {
                data->valid = 0;
                mutex_unlock(&data->update_lock);
                dev_dbg(&client->dev, "reg 0x%x, err %d\n", fan_reg[i], status);
                return data;
            }
            else {
                data->reg_fan_val[i] = status & 0xff;
            }
        }

        data->last_updated = jiffies;
        data->valid = 1;
    }

    mutex_unlock(&data->update_lock);

    return data;
}

/*
static ssize_t show_power(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct g48c210r_cpld_data *data = i2c_get_clientdata(client);
    int status = 0;
    u8 reg = 0, mask = 0;
  
    reg=0xc;
    mask=0x2;
    mutex_lock(&data->update_lock);
	status = g48c210r_cpld_read_internal(client, reg);
	if (unlikely(status < 0)) {
		goto exit;
	}
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", !(status & mask));

exit:
	mutex_unlock(&data->update_lock);
	return status;
}
*/
/*
 * I2C init/probing/exit functions
 */
static int g48c210r_cpld_probe(struct i2c_client *client)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct g48c210r_cpld_data *data;
	int ret = -ENODEV;
//	int status;	
	const struct attribute_group *group = NULL;
	const struct i2c_device_id *id;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto exit;

	data = kzalloc(sizeof(struct g48c210r_cpld_data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto exit;
	}

	id = i2c_match_id(g48c210r_cpld_id, client);

	i2c_set_clientdata(client, data);
    mutex_init(&data->update_lock);
	data->type = id->driver_data;
	   
    /* Register sysfs hooks */
    switch (data->type)
    {    
        case g48c210r_cpld:
            group = &g48c210r_cpld_group;
            break;    
        default:
            break;
    }

    if (group)
    {
        ret = sysfs_create_group(&client->dev.kobj, group);
        if (ret) {
            goto exit_free;
        }
    }

    g48c210r_cpld_add_client(client);
    
    
    data->hwmon_dev = hwmon_device_register(&client->dev);
    if (IS_ERR(data->hwmon_dev)) {
        ret = PTR_ERR(data->hwmon_dev);
        goto exit_free;
    }

    
    return 0;

exit_free:
    kfree(data);
exit:
	return ret;
}

static void g48c210r_cpld_remove(struct i2c_client *client)
{
    struct g48c210r_cpld_data *data = i2c_get_clientdata(client);
    const struct attribute_group *group = NULL;

    g48c210r_cpld_remove_client(client);

    /* Remove sysfs hooks */
    switch (data->type)
    {
        case g48c210r_cpld:
            group = &g48c210r_cpld_group;
            break;
        default:
            break;
    }

    if (group) {
        hwmon_device_unregister(data->hwmon_dev);
        sysfs_remove_group(&client->dev.kobj, group);
    }
    

    kfree(data);

}

static int g48c210r_cpld_read_internal(struct i2c_client *client, u8 reg)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;

	while (retry) {
		status = i2c_smbus_read_byte_data(client, reg);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

    return status;
}

static int g48c210r_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
{
	int status = 0, retry = I2C_RW_RETRY_COUNT;
    
	while (retry) {
		status = i2c_smbus_write_byte_data(client, reg, value);
		if (unlikely(status < 0)) {
			msleep(I2C_RW_RETRY_INTERVAL);
			retry--;
			continue;
		}

		break;
	}

    return status;
}

int g48c210r_cpld_read(unsigned short cpld_addr, u8 reg)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EPERM;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = g48c210r_cpld_read_internal(cpld_node->client, reg);
    		break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(g48c210r_cpld_read);

int g48c210r_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;
    
	mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = g48c210r_cpld_write_internal(cpld_node->client, reg, value);
            break;
        }
    }

	mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(g48c210r_cpld_write);

static struct i2c_driver g48c210r_cpld_driver = {
	.driver		= {
		.name	= "g48c210r_cpld",
		.owner	= THIS_MODULE,
	},
	.probe		= g48c210r_cpld_probe,
	.remove		= g48c210r_cpld_remove,
	.id_table	= g48c210r_cpld_id,
};

static int __init g48c210r_cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&g48c210r_cpld_driver);
}

static void __exit g48c210r_cpld_exit(void)
{
    i2c_del_driver(&g48c210r_cpld_driver);
}

MODULE_AUTHOR("Jostar Yang <jostar_yang@accton.com.tw>");
MODULE_DESCRIPTION("Accton I2C CPLD driver");
MODULE_LICENSE("GPL");

module_init(g48c210r_cpld_init);
module_exit(g48c210r_cpld_exit);

