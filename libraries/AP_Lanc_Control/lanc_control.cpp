/* lanc_control.cpp */

#include "lanc_control.h"
//#include <systemlib/err.h>

extern const AP_HAL::HAL& hal;



struct Extern_Module_s
{
    uint8_t lanc_camera_cmd;
    bool _camera_cmd_update ;  
    uint8_t   pesticide_height,do_crop_dusting,crop_dusting_status;
    bool    enter_farm_extern;

}Extern_Module;


AP_LANC::AP_LANC():
lanc_cmd(0),
uart_receive_id(0),
farm_cmd(0),
cmd_update(false)
{
}

AP_LANC::~AP_LANC()
{
	
}


void AP_LANC::lanc_init(const AP_SerialManager& serial_manager)
{
	 _uart = serial_manager.find_serial(AP_SerialManager::SerialProtocol_Lanc, 0);
	if (_uart != nullptr) {
	        _uart->begin(57600U);
	        _uart->set_flow_control(AP_HAL::UARTDriver::FLOW_CONTROL_DISABLE);
	}	    
}

bool AP_LANC::lanc_detect(AP_SerialManager &serial_manager)
{
    return serial_manager.find_serial(AP_SerialManager::SerialProtocol_Lanc, 0) != nullptr;
}

void AP_LANC::lanc_send_cmd(uint8_t cmd)
{
		///	warnx("_into lanc_send_cmd   cmd:%d",cmd);
	switch(cmd){
		case cam_start:
			_uart->write((const uint8_t *)&start_cmd, sizeof(start_cmd));
			break;
		case zoom_in:
			_uart->write((const uint8_t *)&zoom_in_cmd, sizeof(zoom_in_cmd));
			break;
		case zoom_out:
			_uart->write((const uint8_t *)&zoom_out_cmd, sizeof(zoom_out_cmd));
			break;
		case zoom_stop:
			_uart->write((const uint8_t *)&zoom_stop_cmd, sizeof(zoom_stop_cmd));
			break;
		case vedio_start:
			_uart->write((const uint8_t *)&vedio_start_cmd, sizeof(vedio_start_cmd));
			break;
		case vedio_stop:
			_uart->write((const uint8_t *)&vedio_stop_cmd, sizeof(vedio_stop_cmd));
			break;
		case power:
			_uart->write((const uint8_t *)&power_cmd, sizeof(power_cmd));
			break;
		case focus:
			_uart->write((const uint8_t *)&focus_cmd, sizeof(focus_cmd));
			break;
		case shutter:
			_uart->write((const uint8_t *)&shutter_cmd, sizeof(shutter_cmd));
			break;

	}
	cmd_update = false;
	Extern_Module._camera_cmd_update = false;
	  
}

void AP_LANC::farm_send_cmd(uint8_t cmd)
{
	switch(cmd){
		case farm_spray_start:
			_uart->write((const uint8_t *)&farm_start_spray, sizeof(farm_start_spray));
			break;
		case farm_spray_stop:
			_uart->write((const uint8_t *)&farm_stop_spray, sizeof(farm_stop_spray));
			break;
		case farm_spray_slow:
			_uart->write((const uint8_t *)&farm_slow_spray, sizeof(farm_slow_spray));
			break;
		case farm_spray_middle:
			_uart->write((const uint8_t *)&farm_middle_spray, sizeof(farm_middle_spray));
			break;
		case farm_spray_fast:
			_uart->write((const uint8_t *)&farm_fast_spray, sizeof(farm_fast_spray));
			break;
		case farm_spray_very_fast:
			_uart->write((const uint8_t *)&farm_very_fast_spray, sizeof(farm_very_fast_spray));
			break;
		}
}


bool AP_LANC::lanc_receive_cmd(uint8_t &cmd)
{
///	warnx("Into     lznc  111  cmd:%d",cmd);
	lanc_cmd = cmd;
	cmd_update = true;
	Extern_Module._camera_cmd_update = true;

///	warnx("cmd_update: %d     _camera_cmd_update: %d    lanc_cmd: %d  ",cmd_update,Extern_Module._camera_cmd_update,lanc_cmd);

	return true;
}


bool AP_LANC::lanc_uart_receive(uint8_t &cmd,uint8_t &id)
{
	bool ret = false;

	if(_uart == nullptr){
		return ret;
	}
	static uint8_t step;
	static uint8_t last_cmd;
	int16_t nbytes = _uart->available();
	while(nbytes-- > 0){
		uint8_t data = _uart->read();
		switch(step){
			case 0:
				if(data == MODULE_EXTEN_MSG_HEAD0){
					step++;
				}
				break;					

			case 1:
				if(data == MODULE_EXTEN_MSG_HEAD1){
					step++;
				}
				else step = 0;
				break;
			case 2:				
				step ++;
				id = data;
				break;
			case 3:
				step ++;
				cmd = data;
				break;
			case 4:
				step = 0;
				if(data ==  MODULE_EXTEN_MSG_CRC)
				{
					if(last_cmd !=cmd){
						ret =  true;
						last_cmd =cmd;	
					}
				}
				break;

		}	
	}
	return ret;
}

void AP_LANC::rc_command(int16_t rc_spray_switch, int16_t rc_liquid_level)
{
   int16_t rc_switch =  rc_spray_switch;
   int16_t auto_resume_switch =  rc_liquid_level;

   static int16_t last_rc_value;
   if(auto_resume_switch>1400&& auto_resume_switch <1600)
   {
        farm_cmd = 5;
       /// warnx("[ test ] simulate Pesticide Full");
   }
   if(auto_resume_switch>1700&& auto_resume_switch <2200)
   {
        farm_cmd = 6;
       /// warnx("[ test ] simulate Pesticide Empty");
   }

   if(rc_switch>900&& rc_switch <1200&& abs(last_rc_value - rc_switch)>50)
   {

                farm_send_cmd(farm_spray_stop);
            Extern_Module.crop_dusting_status = 0;
           last_rc_value = rc_switch;
       	/// warnx("[ test ] farm_spray_stop");

   }
   else if( rc_switch > 1250 && rc_switch < 1400&& abs(last_rc_value - rc_switch)>50)
   {
         
        if(farm_cmd == 5 ){
            farm_send_cmd(farm_spray_start);
            Extern_Module.crop_dusting_status = 1;
       	/// warnx("[ test ] farm_spray_start");
        }
           farm_send_cmd(farm_spray_slow);
           last_rc_value = rc_switch;
       	/// warnx("[ test ] farm_spray_slow");   
      }
   else if( rc_switch > 1400 && rc_switch < 1550&& abs(last_rc_value - rc_switch)>50)
   {
   
           farm_send_cmd(farm_spray_middle);
           last_rc_value = rc_switch;
       	/// warnx("[ test ] farm_spray_middle");
   }
   else if( rc_switch >=1550 && rc_switch <1700 && abs(last_rc_value - rc_switch)>50)
   {
    
        farm_send_cmd(farm_spray_fast);
           last_rc_value = rc_switch;
       	/// warnx("[ test ] farm_spray_fast");
   }
   else if( rc_switch >= 1700 && rc_switch <1850&& abs(last_rc_value - rc_switch)>50)
   {

           farm_send_cmd(farm_spray_fast);
           last_rc_value = rc_switch;
       	/// warnx("[ test ] farm_spray_fast");
   }
   else if( rc_switch >=1850 && rc_switch <2200 && abs(last_rc_value - rc_switch)>50)
   {

           farm_send_cmd(farm_spray_very_fast);
           last_rc_value = rc_switch;
       	/// warnx("[ test ] farm_spray_very_fast");
   }

}


void AP_LANC::lanc_update(void)
{
//	lanc_receive_cmd(lanc_cmd);
    static uint32_t cnt;
    uint8_t  temp_receive_cmd;
    cnt++;
/*    if(cnt%100 == 0)
	warnx("Into   lanc_update()    ");*/
//	warnx("Into   lanc_update()    cmd_update: %d , _camera_cmd_update: %d",cmd_update,_camera_cmd_update);
	if(Extern_Module._camera_cmd_update){
//		warnx("lanc_send_cmd:  cmd:%d",lanc_camera_cmd);
		lanc_send_cmd(Extern_Module.lanc_camera_cmd);
	}

	if(lanc_uart_receive(temp_receive_cmd,uart_receive_id))
	{		
	//	warnx("lanc_uart_receive:  cmd:%d ,id: %d",temp_receive_cmd,uart_receive_id);
		if(uart_receive_id == MODULE_EXTEN_MSG_CLASS_FARM)
			farm_cmd = temp_receive_cmd;
		if(uart_receive_id == MODULE_EXTEN_MSG_CLASS_LANC)
			lanc_cmd = temp_receive_cmd;
	}
	
}

/*uint8_t AP_LANC::farm_cmd_detect()
{
	switch(farm_cmd)
	{
		case 6:

	}
}*/
