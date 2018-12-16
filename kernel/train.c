/*
Author: Ali Alavi
Student ID: 917932397
This program solves the model train track challenge
*/

char res_buf[3];

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

void train_process(PROCESS self, PARAM param)
{
  int window_id = wm_create(6, 4, 15, 15);
  wm_print(window_id, "Setting up outer switches\n");
  init_track_switches();
}

void init_train()
{
  PORT pr = create_process(train_process, 6, 0, "train process");
}
