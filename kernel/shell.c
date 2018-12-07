#include <kernel.h>

const char *commands[] = {"cls", "ps", "pong", "shell", "history"};
const int command_length = 5;

typedef struct cmd_hist_node {
  int index;
  char *cmd;
  struct cmd_hist_node *next;
} _cmd_hist_node;

int str_to_int(char * num)
{
  int new_int = 0;
  int i = 0;

  while(num[i] != '\0')
  {
    new_int = new_int * 10 + ((int)num[i] - 48);
    i++;
  }

  return new_int;
}

void set_command(int window_id, char * history_cmd, char * new_cmd)
{
  //wm_print(window_id, "setting");
  int i = 0;

  while(new_cmd[i] != '\0')
  {
    history_cmd[i] = new_cmd[i];
    i++;
  }

  history_cmd[i+1] = '\0';
  //wm_print(window_id, "done setting");
}

void print_processes(int window_id)
{
  int             i;
  PCB            *p = pcb;

  for (i = 0; i < MAX_PROCS; i++, p++)
  {
      if (p->used)
        wm_print(window_id, "Process name %s at priority %d \n",p->name, p->priority);
  }
}

void print_command_history(int window_id, _cmd_hist_node* head)
{
  _cmd_hist_node *temp = head;

  while(temp->next != NULL)
  {
    wm_print(window_id, "%d - %s\n",temp->index, temp->cmd);
    temp = temp-> next;
  }
}

void exec_history_cmd(int window_id, _cmd_hist_node * node, int his_index)
{
  int i = 0;

  while(node->next != NULL)
  {
    if(i == his_index)
    {
      wm_print(window_id, "%s", node->cmd);
      int match_found = find_command(node->cmd);
      run_command(window_id, match_found, node);
      return;
    }

    i++;
    node = node->next;
  }
}

void run_command(int window_id, int command_index, _cmd_hist_node* head)
{
  switch(command_index) {
    case 0:
      wm_clear(window_id);
      break;
    case 1:
      print_processes(window_id);
      break;
    case 2:
      start_pong();
      break;
    case 3:
      start_shell();
      break;
    case 4:
      print_command_history(window_id, head);
      break;
    default:
      wm_print(window_id, "Default");
      break;
  }
}

int match_strings(char* str1, char* str2)
{
   while( ( *str1 != '\0' && *str2 != '\0' ) && *str1 == *str2 )
   {
       str1++;
       str2++;
   }

   if(*str1 == *str2)
   {
       return 1; // strings are identical
   }

   else
   {
       return 0;
   }
}

int find_command(char* command)
{
  for(int i = 0; i < command_length; i++)
  {
    if(match_strings(command, commands[i]) == 1)
      return i;
  }

  return -1;
}

void run_exclamation(int window_id, char * cmd, int cmd_len, _cmd_hist_node * his_node, int hist_len)
{
  int i = 1;
  char * num = (char *) malloc(cmd_len * sizeof(char));

  while(cmd[i] != '\0')
  {
    if(cmd[i] < 48 || cmd[i] > 57)
    {
      wm_print(window_id, "Invalid command");
      return;
    }
    num[i - 1] = cmd[i];
    i++;
  }

  int hist_index = str_to_int(num);

  if(hist_index > hist_len)
  {
      wm_print(window_id, "Invalid index, total %d commands in history", hist_len);
      return;
  }
  else
  {
    exec_history_cmd(window_id, his_node, hist_index);
    return;
  }
}

void user_process(PROCESS self, PARAM param)
{
  char command[50];
  _cmd_hist_node * first = NULL;
  _cmd_hist_node * tail = NULL;
  int curr_index = 0;
  int history_index = 0;
  int window_id = wm_create(10, 3, 50, 17);
  wm_print(window_id, ">");

  while(1)
  {
    char ch = keyb_get_keystroke(window_id, TRUE);

    while(ch != 13)
    {
      if(ch == 8 && curr_index > 0)
      {
        curr_index--;
        command[curr_index] = '\0';
        wm_print(window_id, "%c", ch);
      }

      if(ch != 8)
      {
        command[curr_index] = ch;
        curr_index++;
        wm_print(window_id, "%c", ch);
      }
      //if(ch == 13)
      ch = keyb_get_keystroke(window_id, TRUE);
    }

    wm_print(window_id, "\n");
    
    _cmd_hist_node* new_command = (_cmd_hist_node*)  malloc(sizeof(_cmd_hist_node));
    new_command->cmd = (char *) malloc((curr_index + 1) * sizeof(char));
    set_command(window_id, new_command->cmd, command);
    new_command->next = NULL;
    new_command->index = history_index;
    history_index++;

    //wm_print(window_id, "created node");

    if(first == NULL)
    {
      //wm_print(window_id, "first node null");
      first = new_command;
      tail = new_command;
    }
    else
    {
      //wm_print(window_id, "first node not null");
      tail->next = new_command;
      tail = tail->next;
    }

    if(command[0] == '!')
    {
      run_exclamation(window_id, command, curr_index, first, history_index);
    }
    else
    {
      int match_found = find_command(command);
      run_command(window_id, match_found, first);
    }
    wm_print(window_id, "\n>");

    for(int i = 0; i < 50; i++)
    {
      command[i] = '\0';
    }

    curr_index = 0;
  }

}

void start_shell()
{
  //wm_print('add process');
  create_process(user_process, 6, 2, "User Process");
}
