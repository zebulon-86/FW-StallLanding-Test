#ifndef __LANC_CONTROL_H__
#define __LANC_CONTROL_H__

#include <AP_HAL/AP_HAL.h>
#include <AP_SerialManager/AP_SerialManager.h>
//#include <systemlib/err.h>

#define MODULE_EXTEN_MSG_HEAD0  0xe5
#define MODULE_EXTEN_MSG_HEAD1  0x5e
#define MODULE_EXTEN_MSG_CLASS_LANC  0X01
#define MODULE_EXTEN_MSG_CLASS_FARM  0X02
#define MODULE_EXTEN_MSG_CRC  0X56

/* extern struct Extern_Module_s
{
    uint8_t lanc_camera_cmd;
    bool _camera_cmd_update ;  
    uint8_t   pesticide_height,do_crop_dusting,crop_dusting_status;

}Extern_Module;*/

enum Camera_Cmd_E
{
	cam_start,
	zoom_in,
	zoom_out,
	zoom_stop,
	vedio_start,
	vedio_stop,
	power = 6,
	focus,
	shutter,
	zoom_ining,
	zoom_outing,

	farm_spray_start = 20,
	farm_spray_stop = 21,
	farm_spray_slow = 22,
	farm_spray_middle = 23,
	farm_spray_fast = 24,
	farm_spray_very_fast = 25,

	cmd_null = 99

};

	const uint8_t start_cmd[ ] = {0x5e, 0xe5,0x01, 0x00,0X56};
	const uint8_t zoom_in_cmd[ ] = {0x5e, 0xe5,0x01,0x01 ,0X56};
	const uint8_t zoom_out_cmd[ ] = {0x5e, 0xe5,0x01,0x02 ,0X56};
	const uint8_t zoom_stop_cmd[ ] = {0x5e, 0xe5,0x01,0x03,0X56 };
	const uint8_t zoom_in_fast_cmd[ ] = {0x5e, 0xe5,0x01, 0x04,0X56};
	const uint8_t zoom_out_fast_cmd[ ] = {0x5e, 0xe5,0x01, 0x05,0X56};

	const uint8_t power_cmd[ ] = {0x5e, 0xe5,0x01, 0x06,0X56};
	const uint8_t focus_cmd[ ] = {0x5e, 0xe5,0x01, 0x07,0X56};
	const uint8_t shutter_cmd[ ] = {0x5e, 0xe5,0x01, 0x08,0X56};

	const uint8_t vedio_start_cmd[ ] = {0x5e, 0xe5,0x01, 0x04,0X56};
	const uint8_t vedio_stop_cmd[ ] = {0x5e, 0xe5,0x01, 0x05,0X56};

	const uint8_t farm_start_spray[ ] = 	{0x5e, 0xe5,0x02, 0x01,0X56};
	const uint8_t farm_stop_spray[ ] = 	{0x5e, 0xe5,0x02, 0x02,0X56};
	const uint8_t farm_slow_spray[ ] = 	{0x5e, 0xe5,0x02, 0x03,0X56};
	const uint8_t farm_middle_spray[ ] = 	{0x5e, 0xe5,0x02, 0x04,0X56};
	const uint8_t farm_fast_spray[ ] = 	{0x5e, 0xe5,0x02, 0x05,0X56};
	const uint8_t farm_very_fast_spray[ ] = 	{0x5e, 0xe5,0x02, 0x06,0X56};


class AP_LANC
{
public:
	/**
	 * Constructor
	 */
	AP_LANC();

	    uint8_t farm_cmd;



	void lanc_init(const AP_SerialManager& serial_manager);

	bool lanc_detect(AP_SerialManager &serial_manager);
	void lanc_send_cmd(uint8_t cmd);
	void farm_send_cmd(uint8_t cmd);
	bool lanc_receive_cmd(uint8_t &cmd);
	void lanc_update(void);

	bool lanc_uart_receive(uint8_t &cmd,uint8_t &id);

	/**
	 * [rc_command description]  for testing,rc simulate the condition of pesticide empty and use rc to control spray switch.
	 * @BrownZ
	 * @DateTime 2016-06-06T09:53:05+0800
	 * @param    rc_spray_switch          [turn on or turn off the spray swithc]
	 * @param    rc_liquid_level          [simulate the condition of pesticide empty or full]
	 */
	void rc_command(int16_t rc_spray_switch, int16_t rc_liquid_level);
	/**
	 * Destructor, also kills the lanc task.
	 */
	~AP_LANC();

	
private:
	    AP_HAL::UARTDriver  *_uart;
	    uint8_t lanc_cmd;
	    uint8_t uart_receive_id;

	    bool  cmd_update;
	    


};


#endif //__LANC_CONTROL_H__