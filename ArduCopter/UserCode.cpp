/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "Copter.h"
//#include <systemlib/err.h>

#ifdef USERHOOK_INIT
void Copter::userhook_init()
{
    // put your initialisation code here
    // this will be called once at start-up
///	warnx(" ----------lanc.lanc_init------------");
    lanc.lanc_init(serial_manager);
}
#endif

#ifdef USERHOOK_FASTLOOP
void Copter::userhook_FastLoop()
{
    // put your 100Hz code here
}
#endif
extern struct Extern_Module_s
{
    uint8_t lanc_camera_cmd;
    bool _camera_cmd_update ;  
    uint8_t   pesticide_height,do_crop_dusting,crop_dusting_status;
    bool    enter_farm_extern;
}Extern_Module;
#ifdef USERHOOK_50HZLOOP
void Copter::userhook_50Hz()
{
    // put your 50Hz code here
 
}
#endif


#ifdef USERHOOK_MEDIUMLOOP
void Copter::userhook_MediumLoop()
{
    // put your 10Hz code here
 static uint32_t cmd_detect_cnt;
 // static int8_t last_control_mode = control_mode;
  static uint8_t state_step,last_farm_cmd,now_farm_cmd;
  static uint8_t spray_cnt = 0;     //  start spray detect,the index will be second wp when reached first wp 
  static bool need_auto_resume = false;
  AP_Mission::Mission_Command cmd;


   need_auto_resume = true;

   int16_t rc_switch =  g.rc_6._radio_in;
   int16_t auto_resume_switch =  g.rc_7._radio_in;
   lanc.rc_command(rc_switch,auto_resume_switch);

   lanc.lanc_update(); 
  now_farm_cmd = lanc.farm_cmd;

   if(control_mode == STABILIZE ||  control_mode == ALT_HOLD ||  control_mode == LOITER  ){
        static uint8_t step_in_once_6 = 0;
        Extern_Module.enter_farm_extern = false;
        cmd_detect_cnt = 0;             
        state_step = 0;
        if(now_farm_cmd == 6 && step_in_once_6 == 0){
                step_in_once_6 = 1;
                Extern_Module.pesticide_height = 0 ;
                lanc.farm_send_cmd(farm_spray_stop);
                Extern_Module.crop_dusting_status = 0;
                gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ Notice ] Pesticide Empty\n ");
        }
        else if(now_farm_cmd == 5 )
            step_in_once_6 = 0;
   }
    if(  control_mode == AUTO  ){
        Extern_Module.enter_farm_extern = true;
    }
 //       gcs[0].send_custom_status();

    if(Extern_Module.enter_farm_extern){

            if(user_cmd_state.id == MAV_CMD_NAV_WAYPOINT&&user_cmd_state.index>1)
            {
                spray_cnt++;
                if(spray_cnt == 2){
                        lanc.farm_send_cmd(farm_spray_start);
             	          gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ Action ] Turn on spread switch\n ");
                  }
            }

    switch(state_step){                                                                                               
        case 0:
            if(last_farm_cmd != now_farm_cmd)
            {
                last_farm_cmd = now_farm_cmd;
                state_step = 1;
            }
            break;
        case 1:
            if(last_farm_cmd != now_farm_cmd)
                state_step = 0;
            if(now_farm_cmd == 5 && control_mode == POSHOLD)
            {
             
  //              warnx("[ notice ] pesticide not empty ! ,last_farm_cmd: %d   now_farm_cmd = %d",last_farm_cmd,now_farm_cmd);
                if(cmd_detect_cnt != 0){
                    cmd_detect_cnt = 0;             
                    if(need_auto_resume){
                      lanc.farm_send_cmd(farm_spray_start);
                      gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ Action ] Turn on spread switch\n ");
               //         warnx("[ notice ] turn last control mode :%d ",last_control_mode);
                        if(!set_mode(AUTO,MODE_REASON_TX_COMMAND)){
              ///              warnx("[ error ] resume mission , fail to change control mode to : AUTO !");
                        }
                        else{ 
                                gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ resume ] resume mission\n ");
                                gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ resume ] Now change control mode to : AUTO !\n");
                     ///           warnx("[ resume ] resume mission , now change control mode to : AUTO !"); 
                        }
                    }
                }
                state_step = 0;
            }
            if(now_farm_cmd == 6 && control_mode != RTL)
            {
                state_step = 2;  
                    cmd_detect_cnt = 0;             
         //        warnx("[ notice ] pesticide empty! switch mode to loiter,last mode: %d",control_mode);
        //        last_control_mode =   control_mode;           
                if(!set_mode(POSHOLD,MODE_REASON_TX_COMMAND)){
          ///          warnx("[ error ] Pause mission, fail to change control mode to : POSHOLD !");
                    }
                else {
                            Extern_Module.pesticide_height = 0 ;
                                   lanc.farm_send_cmd(farm_spray_stop);
                                   spray_cnt = 0;
                                    Extern_Module.crop_dusting_status = 0;
                              gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ notice ] Pause mission\n ");
             	                   gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ Action ] Turn off spread switch\n ");

                                cmd.id = MAV_CMD_NAV_WAYPOINT;
                                cmd.content.location.options = 0;
                                cmd.p1 = 0;
                                cmd.content.location.alt = user_cmd_state.content.location.alt;
                                cmd.content.location.lat = current_loc.lat;
                                cmd.content.location.lng = current_loc.lng;


                                if (!mission.replace_cmd(user_cmd_state.index, cmd)) {
                           //     if (!mission.insert_cmd(user_cmd_state.index, cmd)) {
                  ///                  warnx("failed to replace command #%d\n",user_cmd_state.index);
                                }else{
                              ///      warnx("replaced command #%d , alt:%d , lat:%d , lng:%d\n",user_cmd_state.index,user_cmd_state.content.location.alt,user_cmd_state.content.location.lat,user_cmd_state.content.location.lng);
                                           gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ notice ] Replace mission cmd #%d\n ",user_cmd_state.index);
                                         //    gcs_send_text_fmt(MAV_SEVERITY_INFO,"pos alt: %d \n",cmd.content.location.alt);
                                         //    gcs_send_text_fmt(MAV_SEVERITY_INFO,"pos lat: %d \n",cmd.content.location.lat);
                                         //    gcs_send_text_fmt(MAV_SEVERITY_INFO,"pos lng: %d \n",cmd.content.location.lng);
                                   }
                    }
            }
                break;

         case 2:
            if(last_farm_cmd != now_farm_cmd)
                state_step = 0;
/*            if(cnt%80 == 0)
             warnx("[ notice ] pesticide empty! switch mode to loiter");*/
            cmd_detect_cnt++;
            if(cmd_detect_cnt>40 && control_mode != RTL)
            {
             //   cmd_detect_cnt = 0;
    //            warnx("[ action ] pesticide empty! now we will turn back");
                if(!set_mode(RTL,MODE_REASON_TX_COMMAND)){
     ///               warnx("[ error ] stop mission,  fail to change control mode to : RTL !");
                 }
                else {
                                /**
                                 *  replace first nav cmd ,change to jump to last user_cmd_state.index , so the mission can resume last pos auto
                                 */
                                int cmd_i;
                                bool exit_detect = false;
                                for( cmd_i=0; cmd_i<mission.num_commands() &&  !exit_detect; ){
                                  cmd_i++;
                                  mission.read_cmd_from_storage(cmd_i,cmd);             // detect whether the cmd is way 
                            ///      warnx("numb:%d, i:%d, id:%d  ",mission.num_commands(),cmd_i,cmd.id);
                                  if(cmd.id == MAV_CMD_NAV_WAYPOINT||cmd.id == MAV_CMD_DO_JUMP)
                                     exit_detect = true; 
                                }

                                cmd.id = MAV_CMD_DO_JUMP;
                                cmd.content.jump.target = user_cmd_state.index;
                                cmd.content.jump.num_times = 1;
                                if(cmd_i == 0 ||cmd_i == 1 )  cmd_i = 2;
                                if (!mission.replace_cmd(cmd_i, cmd)) {
                    ///                warnx("failed to replace command #%d\n",user_cmd_state.index);
                                }else{
                                            gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ notice ] mission cmd #%d jump to #%d\n ",cmd_i,cmd.content.jump.target);
                             ///       warnx(" replace command #%d\n,  cmd_i: %d",user_cmd_state.index,cmd_i);
                                }
/*                            gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ action ] Stop mission\n");
                            gcs_send_text_fmt(MAV_SEVERITY_INFO,"[ action ] Now change control mode to : RTL !\n");
                    warnx("[ action ] stop mission,  now change control mode to : RTL !");*/
                }
                state_step = 3;
            }
             break;

         case 3:

            if(last_farm_cmd != now_farm_cmd)
                state_step = 0;
            cmd_detect_cnt = 0;             
            Extern_Module.enter_farm_extern = false;
            break;
    }
    }
}
#endif




#ifdef USERHOOK_SLOWLOOP
void Copter::userhook_SlowLoop()
{
    // put your 3.3Hz code here
}
#endif

#ifdef USERHOOK_SUPERSLOWLOOP
void Copter::userhook_SuperSlowLoop()
{
    // put your 1Hz code here
}
#endif
