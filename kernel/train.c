/*
Author: Ali Alavi
Student ID: 917932397
This program solves the model train track challenge
*/

char res_buf[3];
int zambonie = 0;
int config = 0;
int num_of_ticks = 2;
int sleep_ticks = 2;
#include <kernel.h>

void send_com_message(int input_buf_len, char * cmd)
{
  COM_Message msg;

	msg.output_buffer 		= cmd;

  if(input_buf_len > 0)
    msg.input_buffer 		= res_buf;
  else
    msg.input_buffer = NULL;

	msg.len_input_buffer 	= input_buf_len;

  send(com_port, &msg);
}

void init_track_switches()
{
  int num_init_switches = 5;

  char * switches[5] = {
    "M1G\015",
    "M4G\015",
    "M5G\015",
    "M8G\015",
    "M9R\015"
  };

  for(int i = 0; i < num_init_switches; i++)
    send_com_message(0, switches[i]);
}

char probe_segment(char * cmd_segment)
{
  //wm_print(window_id, "Probing %s\n", cmd_segment);
  send_com_message(0,"R\015");
  send_com_message(3,cmd_segment);
  //wm_print(window_id, "%c\n", res_buf[1]);
  return res_buf[1];
}

void start_train(int window_id)
{
  wm_print(window_id, "Start the train\n");
  send_com_message(0,"L20S5\015");
}

void slow_train(int window_id)
{
  wm_print(window_id, "Reduce train speed\n");
  send_com_message(0,"L20S4\015");
}

void stop_train(int window_id)
{
  wm_print(window_id, "Stop the train\n");
  sleep(sleep_ticks);
  //send_com_message(0,"L20S2\015");
  send_com_message(0,"L20S0\015");
}

void stop_train_no_sleep(int window_id)
{
  wm_print(window_id, "Stop the train\n");
  send_com_message(0,"L20S0\015");
}

void reverse_direction()
{
  send_com_message(0,"L20D\015");
}

void reverse_train(int window_id)
{
  wm_print(window_id, "Reverse the train\n");
  stop_train(window_id);
  reverse_direction();
  start_train(window_id);
}

void flip_switch(char * switch_cmd)
{
  send_com_message(0, switch_cmd);
}

void check_segment(int window_id, char * sgmnt_cmd)
{
  wm_print(window_id, "Checking segment %c%c till we detect a vehicle\n", sgmnt_cmd[1], sgmnt_cmd[2]);
  while(probe_segment(sgmnt_cmd) == '0');
  //{
  //  sleep(num_of_ticks);
  //}
}

void check_zambonie(int window_id)
{
  int i = 0;

  while(i < 25)
  {
    send_com_message(0,"R\015");
    send_com_message(3,"C10\015"); // segment in between both starting points for Zambonie

    if((char)res_buf[1] == '1')
    {
      zambonie = 1;
      return;
    }

    sleep(2);
    i++;
  }

  return;
}

void find_config(int window_id)
{
  char * probe_train_cmds[4] = {
    "C08\015\0",
    "C12\015\0",
    "C02\015\0",
    "C05\015\0"
  };

  char * probe_wagon_cmds[4] = {
    " ",
    "C02\015\0",
    "C11\015\0",
    " "
  };

  for(int i = 0; i < 4; i++)
  {
    wm_print(window_id, "Probing segment %c%c for train\n", probe_train_cmds[i][1], probe_train_cmds[i][2]);
    if(probe_segment(probe_train_cmds[i]) == '1')
    {
      int wagon_probe = k_memcmp(" ", probe_wagon_cmds[i], 1);

      if(wagon_probe != 0 )
        wm_print(window_id, "Probing segment %c%c for wagon\n", probe_train_cmds[i][1], probe_train_cmds[i][2]);

      if(wagon_probe == 0 || probe_segment(probe_wagon_cmds[i]) == '1')
      {
        config = i + 1;
        break;
      }
    }
  }
}

void solve_config_one(int window_id)
{
  wm_print(window_id, "Solving for config 1\n");

  if(zambonie == 1)
  {
    check_segment(window_id, "C04\015"); // zambonie passes, time to start the train
    start_train(window_id);

    check_segment(window_id, "C14\015"); // zambonie passes above wagon entry, flip switch
    check_segment(window_id, "C07\015"); // zambonie passes above wagon entry, flip switch
    flip_switch("M8R\015");

    check_segment(window_id, "C12\015"); // train is attached to wagon, switch and stop
    stop_train_no_sleep(window_id);
    flip_switch("M8G\015");

    flip_switch("M7G\015"); // get ready for reverse train run
    flip_switch("M6G\015");


    check_segment(window_id, "C07\015"); // zamboni on 7
    check_segment(window_id, "C04\015"); // zamboni is off 7, time to start train again
    reverse_train(window_id);
    flip_switch("M8R\015");


    check_segment(window_id, "C06\015"); // train is ready for reversal, flip switches and reverse
    flip_switch("M5R\015");
    flip_switch("M6R\015");
    reverse_train(window_id);
    slow_train(window_id);

    check_segment(window_id, "C08\015"); // train is back to start point, flip switch
    stop_train(window_id); // victory
    wm_print(window_id, "Success\n");
  }
  else
  {
    start_train(window_id);
    flip_switch("M8R\015");

    check_segment(window_id, "C12\015"); // train is attached to wagon, stop, switch, and reverse
    stop_train(window_id);
    flip_switch("M7G\015");
    flip_switch("M6G\015");
    reverse_train(window_id);

    check_segment(window_id, "C06\015"); // time to reverse train
    flip_switch("M5R\015");
    flip_switch("M6R\015");
    reverse_train(window_id);

    check_segment(window_id, "C08\015"); // train is back to start point, stop
    stop_train(window_id); // victory
  }

}

void solve_config_two(int window_id)
{
  wm_print(window_id, "Solving for config 2\n");

  flip_switch("M2R\015");
  flip_switch("M7G\015");
  flip_switch("M6G\015");
  flip_switch("M3G\015");

  if(zambonie == 1)
  {
    check_segment(window_id, "C03\015"); // zambonie passes, time to start the train
    sleep(sleep_ticks);
    flip_switch("M1R\015");
    start_train(window_id);

    check_segment(window_id, "C14\015"); // reverse train, flip switch
    flip_switch("M2G\015");
    sleep(sleep_ticks);
    reverse_train(window_id);

    check_segment(window_id, "C01\015"); // train is attached to wagon, switch and stop
    flip_switch("M1G\015");

    check_segment(window_id, "C02\015");
    stop_train(window_id);

    check_segment(window_id, "C10\015");
    sleep(sleep_ticks);
    flip_switch("M5R\015");
    start_train(window_id);

    check_segment(window_id, "C12\015"); // train is back to start point, flip switch
    stop_train_no_sleep(window_id);
    flip_switch("M5G\015");
    wm_print(window_id, "Success\n");
  }
  else
  {
    flip_switch("M5R\015");
    flip_switch("M1R\015");
    start_train(window_id);

    check_segment(window_id, "C14\015"); // reverse train, flip switch
    flip_switch("M2G\015");
    reverse_train(window_id);

    check_segment(window_id, "C12\015"); // train is back to start point, flip switch
    stop_train(window_id); // victory
  }

}

void solve_config_three(int window_id)
{
  wm_print(window_id, "Solving for config 3\n");

  flip_switch("M7R\015");
  flip_switch("M3G\015");
  flip_switch("M4R\015");

  if(zambonie == 1)
  {
    check_segment(window_id, "C03\015");
    check_segment(window_id, "C10\015"); // zambonie passes, time to start the train
    start_train(window_id);

    check_segment(window_id, "C03\015");
    sleep(sleep_ticks);
    flip_switch("M1R\015");
    flip_switch("M2R\015");

    check_segment(window_id, "C12\015");
    flip_switch("M1G\015");

    check_segment(window_id, "C11\015");
    stop_train(window_id);
    flip_switch("M2G\015");

    check_segment(window_id, "C03\015");
    sleep(sleep_ticks);
    flip_switch("M8R\015");
    flip_switch("M1R\015");
    sleep(sleep_ticks);
    start_train(window_id);

    check_segment(window_id, "C01\015");
    slow_train(window_id);
    check_segment(window_id, "C02\015");
    flip_switch("M1G\015");
    stop_train(window_id); // victory
    wm_print(window_id, "Success\n");
  }
  else
  {
    start_train(window_id);

    flip_switch("M1R\015");
    flip_switch("M2R\015");

    check_segment(window_id, "C12\015");
    flip_switch("M2G\015");

    flip_switch("M8R\015");
    flip_switch("M1R\015");

    check_segment(window_id, "C02\015");
    stop_train(window_id); // victory
  }

}

void solve_config_four(int window_id)
{
  wm_print(window_id, "Solving for config 4\n");

  //flip_switch("M7G\015");
  //flip_switch("M6G\015");
  //flip_switch("M5R\015");

  if(zambonie == 1)
  {
    /* first option */

    /*check_segment("C4\015"); // zambonie passes, time to start the train
    start_train();

    check_segment("C10\015");
    check_segment("C03\015");
    slow_train();
    flip_switch("M4R\015");
    flip_switch("M3R\015");
    flip_switch("M8R\015");

    check_segment("C5\015");
    stop_train(); // victory
    flip_switch("M4G\015");*/

    /* second option*/

    check_segment(window_id, "C04\015"); // zambonie passes, time to start the train
    start_train(window_id);

    check_segment(window_id, "C07\015");
    reverse_train(window_id);
    check_segment(window_id, "C10\015");
    check_segment(window_id, "C07\015");
    flip_switch("M8R\015");

    check_segment(window_id, "C11\015");
    sleep(sleep_ticks);
    stop_train(window_id);
    flip_switch("M8G\015");

    check_segment(window_id, "C10\015");
    start_train(window_id);

    check_segment(window_id, "C03\015");
    flip_switch("M8R\015");

    flip_switch("M4R\015");
    flip_switch("M3R\015");
    check_segment(window_id, "C06\015");
    slow_train(window_id);
    check_segment(window_id, "C05\015");
    stop_train(window_id); // victory
    wm_print(window_id, "Success\n");
  }
  else
  {
    start_train(window_id);

    check_segment(window_id, "C10\015");
    flip_switch("M4R\015");
    flip_switch("M3R\015");
    check_segment(window_id, "C05\015");
    stop_train(window_id); // vic
  }

}

void run_config(int window_id)
{
  switch (config) {
    case 1:
      solve_config_one(window_id);
      break;
    case 2:
      solve_config_two(window_id);
      break;
    case 3:
      solve_config_three(window_id);
      break;
    case 4:
      solve_config_four(window_id);
      break;
    default:
      wm_print(window_id, "Invalid configuration");
      break;

  }
}

void train_process(PROCESS self, PARAM param)
{
  int window_id = wm_create(6, 4, 50, 20);
  wm_print(window_id, "Setting up outer switches to keep Zamboni in the outer loop\n");
  init_track_switches();
  wm_print(window_id, "Checking for Zamboni by probing segment 10\n");
  check_zambonie(window_id);

  if(zambonie)
    wm_print(window_id, "Zamboni Found\n");
  else
    wm_print(window_id, "Zamboni Not Found\n");

  wm_print(window_id, "Finding config\n");
  find_config(window_id);
  run_config(window_id);
  become_zombie();
}

void init_train()
{
  create_process(train_process, 6, 0, "train process");
  resign();
}
