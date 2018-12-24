/*
Author: Ali Alavi
Student ID: 917932397
This file solves the model train challenge
*/

#include <kernel.h>
char res_buf[3]; //a buffer to store the response from send_com_msg
int zamboni = 0;
int config = 0;
int sleep_ticks = 5;
char * success_msg = "Successfully retrieved the wagon.\0";

void print_cursor(int window_id) // to keep the window active
{
  wm_print(window_id, "%c", 220);
}

// Sends a message to the com_port. If input_buf_len = 0, we don't wait for any response
void send_com_message(int input_buf_len, char * cmd)
{
  COM_Message msg;

	msg.output_buffer 		= cmd;

  if(input_buf_len > 0)
    msg.input_buffer 		= res_buf; // store result in the buffer
  else
    msg.input_buffer = NULL;

	msg.len_input_buffer 	= input_buf_len;

  sleep(sleep_ticks); // sleep between each command
  send(com_port, &msg);
}

// Probe a segment to check if there is any vehicle on it
char probe_segment(char * cmd_segment)
{
  send_com_message(0,"R\015"); // clear the s88 memory buffer
  send_com_message(3,cmd_segment);
  return res_buf[1]; // we only care about the 0 or 1
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
  send_com_message(0,"L20S0\015");
}

void change_direction(int window_id)
{
  wm_print(window_id, "Change the direction\n");
  send_com_message(0,"L20D\015");
}

void reverse_train(int window_id)
{
  stop_train(window_id);
  change_direction(window_id);
  start_train(window_id);
}

void flip_switch(int window_id, char * switch_cmd)
{
  wm_print(window_id, "Flip switch %c to %c \n", switch_cmd[1], switch_cmd[2]);
  send_com_message(0, switch_cmd);
}

void check_segment(int window_id, char * sgmnt_cmd)
{
  wm_print(window_id, "Checking segment %c%c till we detect a vehicle\n", sgmnt_cmd[1], sgmnt_cmd[2]);
  while(1)
  {
    if(probe_segment(sgmnt_cmd) != '1')
    {
      print_cursor(window_id);
    }
    else
    {
      wm_print(window_id, "\n");
      break;
    }
  }
}

// To keep Zamboni in the outer loop
void init_track_switches(int window_id)
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
  {
    flip_switch(window_id, switches[i]);
  }
}

// probe segment 10 as it's in the mid-point of the 2 starting positions of Zamboni
void check_zamboni(int window_id)
{
  int i = 0;

  while(i < 15) // probe the segment for 15 * sleep ticks
  {
    if(probe_segment("C10\015") != '1')
    {
      print_cursor(window_id);
    }
    else
    {
      zamboni = 1;
      break;
    }

    i++;
  }
}

// Probe the train and wagon to determine the configuration
void find_config(int window_id)
{
  // probe initial starting points for train
  char * probe_train_cmds[4] = {
    "C08\015",
    "C12\015",
    "C02\015",
    "C05\015"
  };

  // probe initial starting points for wagon
  char * probe_wagon_cmds[4] = {
    "C11\015",
    "C02\015",
    "C11\015",
    "C12\015"
  };

  for(int i = 0; i < 4; i++)
  {
    wm_print(window_id, "Probing segment %c%c for train\n", probe_train_cmds[i][1], probe_train_cmds[i][2]);

    if(probe_segment(probe_train_cmds[i]) == '1')
    {
      wm_print(window_id, "Probing segment %c%c for wagon\n", probe_wagon_cmds[i][1], probe_wagon_cmds[i][2]);

      if(probe_segment(probe_wagon_cmds[i]) == '1')
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

  if(zamboni == 1)
  {
    check_segment(window_id, "C06\015");
    start_train(window_id);

    check_segment(window_id, "C10\015");
    flip_switch(window_id, "M8R\015");

    check_segment(window_id, "C13\015");
    slow_train(window_id);

    check_segment(window_id, "C12\015");
    flip_switch(window_id, "M8G\015");
    stop_train(window_id);

    flip_switch(window_id, "M7G\015");
    flip_switch(window_id, "M6G\015");

    check_segment(window_id, "C04\015");
    reverse_train(window_id);
    flip_switch(window_id, "M8R\015"); // trap zamboni in inner loop

    check_segment(window_id, "C06\015");
    flip_switch(window_id, "M5R\015");
    flip_switch(window_id, "M6R\015");
    reverse_train(window_id);

    check_segment(window_id, "C07\015");
    slow_train(window_id);

    check_segment(window_id, "C08\015");
    stop_train(window_id); // victory
    wm_print(window_id, "%s\n", success_msg);
  }
  else
  {
    start_train(window_id);
    flip_switch(window_id, "M8R\015");

    check_segment(window_id, "C12\015");
    flip_switch(window_id, "M7G\015");
    flip_switch(window_id, "M6G\015");
    reverse_train(window_id);

    check_segment(window_id, "C06\015");
    flip_switch(window_id, "M5R\015");
    flip_switch(window_id, "M6R\015");
    reverse_train(window_id);

    check_segment(window_id, "C07\015");
    slow_train(window_id);

    check_segment(window_id, "C08\015");
    stop_train(window_id); // victory
    wm_print(window_id, "%s\n", success_msg);
  }

}

void solve_config_two(int window_id)
{
  wm_print(window_id, "Solving for config 2\n");

  flip_switch(window_id, "M2R\015");
  flip_switch(window_id, "M7G\015");
  flip_switch(window_id, "M6G\015");
  flip_switch(window_id, "M3G\015");

  if(zamboni == 1)
  {
    check_segment(window_id, "C03\015");
    flip_switch(window_id, "M1R\015");
    start_train(window_id);

    check_segment(window_id, "C14\015");
    flip_switch(window_id, "M2G\015");
    reverse_train(window_id);

    check_segment(window_id, "C01\015");
    flip_switch(window_id, "M1G\015");
    slow_train(window_id);

    check_segment(window_id, "C02\015");
    stop_train(window_id);

    check_segment(window_id, "C07\015");
    start_train(window_id);

    check_segment(window_id, "C10\015");
    flip_switch(window_id, "M5R\015");
    slow_train(window_id);

    check_segment(window_id, "C09\015");
    flip_switch(window_id, "M5G\015");

    check_segment(window_id, "C12\015");
    stop_train(window_id);
    wm_print(window_id, "%s\n", success_msg);
  }
  else
  {
    flip_switch(window_id, "M5R\015");
    flip_switch(window_id, "M1R\015");
    start_train(window_id);

    check_segment(window_id, "C14\015");
    flip_switch(window_id, "M2G\015");
    reverse_train(window_id);

    check_segment(window_id, "C07\015");
    slow_train(window_id);

    check_segment(window_id, "C12\015");
    stop_train(window_id);
    wm_print(window_id, "%s\n", success_msg);
  }

}

void solve_config_three(int window_id)
{
  wm_print(window_id, "Solving for config 3\n");

  flip_switch(window_id, "M3G\015");
  flip_switch(window_id, "M4R\015");
  flip_switch(window_id, "M7R\015");

  if(zamboni == 1)
  {
    check_segment(window_id, "C10\015");
    start_train(window_id);

    check_segment(window_id, "C03\015");
    flip_switch(window_id, "M1R\015");
    flip_switch(window_id, "M2R\015");

    check_segment(window_id, "C12\015");
    flip_switch(window_id, "M1G\015");
    stop_train(window_id);
    flip_switch(window_id, "M2G\015");

    check_segment(window_id, "C03\015");
    flip_switch(window_id, "M8R\015");
    flip_switch(window_id, "M1R\015");
    start_train(window_id);

    check_segment(window_id, "C01\015");
    slow_train(window_id);
    flip_switch(window_id, "M1G\015");

    check_segment(window_id, "C02\015");
    stop_train(window_id); // victory
    wm_print(window_id, "%s\n", success_msg);
  }
  else
  {
    start_train(window_id);

    flip_switch(window_id, "M1R\015");
    flip_switch(window_id, "M2R\015");

    check_segment(window_id, "C12\015");
    flip_switch(window_id, "M2G\015");

    check_segment(window_id, "C01\015");
    slow_train(window_id);

    check_segment(window_id, "C02\015");
    stop_train(window_id); // victory
    wm_print(window_id, "%s\n", success_msg);
  }

}

void solve_config_four(int window_id)
{
  wm_print(window_id, "Solving for config 4\n");

  flip_switch(window_id, "M7G\015");
  flip_switch(window_id, "M6G\015");
  flip_switch(window_id, "M5R\015");

  if(zamboni == 1)
  {
    check_segment(window_id, "C06\015");
    check_segment(window_id, "C04\015");
    start_train(window_id);

    check_segment(window_id, "C10\015");
    check_segment(window_id, "C04\015");
    flip_switch(window_id, "M4R\015");
    flip_switch(window_id, "M3R\015");
    flip_switch(window_id, "M8R\015"); // trap zamboni in inner loop

    check_segment(window_id, "C06\015");
    slow_train(window_id);

    check_segment(window_id, "C05\015");
    stop_train(window_id);
    wm_print(window_id, "%s\n", success_msg);
  }
  else
  {
    start_train(window_id);

    check_segment(window_id, "C10\015");
    flip_switch(window_id, "M4R\015");
    flip_switch(window_id, "M3R\015");

    check_segment(window_id, "C06\015");
    slow_train(window_id);

    check_segment(window_id, "C05\015");
    stop_train(window_id);
    wm_print(window_id, "%s\n", success_msg);
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
  int window_id = wm_create(5, 5, 50, 20);
  wm_print(window_id, "Putting Zamboni in the outer loop\n");
  init_track_switches(window_id);

  wm_print(window_id, "Checking for Zamboni by probing segment 10\n");
  check_zamboni(window_id);

  if(zamboni)
    wm_print(window_id, "\nZamboni Found\n");
  else
    wm_print(window_id, "\nZamboni Not Found\n");

  wm_print(window_id, "Finding config\n");
  find_config(window_id);
  run_config(window_id);
  become_zombie();
}

void init_train()
{
  create_process(train_process, 6, 0, "Ali's train process");
  resign();
}
