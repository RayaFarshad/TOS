/*
Authored by Ali Alavi
Student ID # 917932397
*/
#include <kernel.h>

const char *commands[] = {"cls", "ps", "pong", "shell", "history", "help", "about"};
const int supported_commands_length = 7;

typedef struct cmd_hist_node {
  int index;
  char *cmd;
  struct cmd_hist_node *next;
} _cmd_hist_node;

int str_to_int(char * num) //converting a string representation of number to int
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

void set_command(char * history_cmd, char * new_cmd) //assign entered command to history node
{
  int i = 0;

  while(new_cmd[i] != '\0')
  {
    history_cmd[i] = new_cmd[i];
    i++;
  }

  history_cmd[i+1] = '\0';
}

void run_echo(int window_id, char* command)
{
  int cmd_len = k_strlen(command);
  int echo_len = k_strlen(command) - 5;
  char * cmd_to_prnt = (char *) malloc((echo_len + 1) * sizeof(char));
  int j =0;

  for(int i = 5; i< cmd_len; i++)
  {
    cmd_to_prnt[j] = command[i];
    j++;
  }

  wm_print(window_id, "%s\n", cmd_to_prnt);
}

void shell_print_process_heading(int window_id)
{
    wm_print(window_id, "State           Active Prio Name\n");
    wm_print(window_id, "------------------------------------------------\n");
}

void shell_print_process_details(int window_id, PROCESS p)
{
    static const char *state[] = { "READY          ",
        "ZOMBIE         ",
        "SEND_BLOCKED   ",
        "REPLY_BLOCKED  ",
        "RECEIVE_BLOCKED",
        "MESSAGE_BLOCKED",
        "INTR_BLOCKED   "
    };
    if (!p->used) {
        wm_print(window_id, "PCB slot unused!\n");
        return;
    }
    /* State */
    wm_print(window_id, state[p->state]);
    /* Check for active_proc */
    if (p == active_proc)
        wm_print(window_id," *      ");
    else
        wm_print(window_id,"        ");
    /* Priority */
    wm_print(window_id, "  %2d", p->priority);
    /* Name */
    wm_print(window_id, " %s\n", p->name);
}

void print_processes(int window_id)
{
  int             i;
  PCB            *p = pcb;

  shell_print_process_heading(window_id);
  for (i = 0; i < MAX_PROCS; i++, p++) {
      if (!p->used)
          continue;
      shell_print_process_details(window_id, p);
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
  for(int i = 0; i < supported_commands_length; i++)
  {
    if(k_memcmp(command, commands[i], k_strlen(commands[i])) == 0)
      return i;
    //if(match_strings(command, commands[i]) == 1)
      //return i;
  }

  return -1;
}

void print_command_history(int window_id, _cmd_hist_node* head)
{
  wm_print(window_id, "In print command history\n");
  _cmd_hist_node *temp = head;

  while(temp != NULL)
  {
    wm_print(window_id, "%d - %s\n",temp->index, temp->cmd);
    temp = temp-> next;
    //break;
  }
}

void exec_history_cmd(int window_id, _cmd_hist_node * node, int hist_index)
{
  int i = 0;

  while(node->next != NULL)
  {
    if(i == hist_index)
    {
      wm_print(window_id, "%s\n", node->cmd);
      run_command(window_id, node->cmd, k_strlen(node->cmd), &node, hist_index);
      break;
    }
    else
    {
      i++;
      node = node->next;
    }
  }
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

void run_help(int window_id)
{
	wm_print(window_id, "List of available commands\n");
	wm_print(window_id, "help      - will print all supported TOS command\n");
	wm_print(window_id, "cls       - clears the screen\n");
	wm_print(window_id, "shell     - launches another shell\n");
	wm_print(window_id, "pong      - launches pong game\n");
	wm_print(window_id, "ps        - prints current process table\n");
	wm_print(window_id, "history   - prints a history of commands typed in shell\n");
	wm_print(window_id, "!<number> - repeats the command with given number which represents the index in history\n");
	wm_print(window_id, "about     - information regarding TOS\n");
}

void run_about(int window_id)
{
	wm_print(window_id, "TOS is an OS written in C. \nCopyright Ali Alavi 2018\n");
}

void add_to_history(int window_id, char * cmd, int cmd_len, _cmd_hist_node ** head, _cmd_hist_node ** tail, int * hist_len)
{
  _cmd_hist_node* new_hist_node = (_cmd_hist_node*)  malloc(sizeof(_cmd_hist_node));
  new_hist_node->cmd = (char *) malloc((cmd_len + 1) * sizeof(char));
  set_command(new_hist_node->cmd, cmd);
  new_hist_node->next = NULL;
  new_hist_node->index = (*hist_len);
  (*hist_len)++;

  if((*head) == NULL)
  {
    //wm_print(window_id, "first node null\n");
    *head = new_hist_node;
    *tail = new_hist_node;
  }
  else
  {
    //wm_print(window_id, "first node not null\n");
    (*tail)->next = new_hist_node;
    *tail = (*tail)->next;
  }
}

void parse_cmd(int window_id, char * command, int cmd_len, _cmd_hist_node** head, _cmd_hist_node** tail, int hist_len)
{
  int start = 0;
  int i = 0;
  char * sub_cmd;
  int sub_cmd_length;
  int j = 0;
  int right_space = 0;

  while(i < cmd_len + 1)
  {
    if(command[i] == ';' || command[i] == '\0')
    {
        if(command[start] == ' ')
          start++;

        if(command[i-1] == ' ')
        {
          right_space = 1;
          i--;
        }

        sub_cmd_length = i - start;

        sub_cmd = (char *) malloc((sub_cmd_length + 1) * sizeof(char));

        while(start < i)
        {
          sub_cmd[j] = command[start];
          start++;
          j++;
        }

        sub_cmd[sub_cmd_length] = '\0';

        run_command(window_id, sub_cmd, sub_cmd_length, head, hist_len);

        if(right_space)
          i++;

        start = i + 1;
        j = 0;
    }

    i++;
  }
}

void run_command(int window_id, char * command, int cmd_len, _cmd_hist_node** head, int hist_len)
{
  int command_index;

  if(command[0] == '!')
  {
    command_index = 7;
  }
  else if(k_memcmp(command, "echo", 4) == 0)
  {
    command_index = 8;
  }
  else
  {
    command_index = find_command(command);
  }

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
      print_command_history(window_id, (*head));
      break;
    case 5:
      run_help(window_id);
      break;
    case 6:
      run_about(window_id);
      break;
    case 7:
      run_exclamation(window_id, command, cmd_len, (*head), hist_len);
      break;
    case 8:
      run_echo(window_id, command);
      break;
    default:
      wm_print(window_id, "Invalid command entered");
      break;
  }//*/
}

void shell_process(PROCESS self, PARAM param)
{
  char command[50];
  _cmd_hist_node * head = NULL;
  _cmd_hist_node * tail = NULL;
  int input_length = 0;
  int command_length = 0;
  int history_length = 0;
  int mid_space_length = 0;
  int window_id = wm_create(10, 3, 50, 17);
  wm_print(window_id, ">");

  while(1)
  {
    char ch = keyb_get_keystroke(window_id, TRUE);

    if(ch == ' ')
    {
      wm_print(window_id, "%c", ch);
      input_length++;
      continue;
    }

    while(ch != 13)
    {
      if(ch == 8)
      {
        //wm_print(window_id, "%d ", curr_index);
        //wm_print(window_id, "%d\n", space_count);
        if(command_length > 0)
        {
          input_length--;
          command_length--;
          command[command_length] = '\0';
          wm_print(window_id, "%c", ch);
        }
        else if(command_length == 0 && input_length > 0)
        {
          input_length--;
          wm_print(window_id, "%c", ch);
        }
      }

      if(ch != 8)
      {
        if(ch == ' ')
        {
          mid_space_length++;
        }
        else
        {
          while(mid_space_length > 1)
          {
            input_length--;
            command_length--;
            command[command_length] = '\0';
            mid_space_length--;
          }
          mid_space_length = 0;
        }

          command[command_length] = ch;
          command_length++;
          input_length++;
          wm_print(window_id, "%c", ch);

      }
      //if(ch == 13)
      ch = keyb_get_keystroke(window_id, TRUE);
    }
    mid_space_length = 0;
    input_length = 0;

    for(int i = command_length - 1; i>0; i--) //remove whitespace from the end
    {
      if(command[i] != ' ')
        break;

      command_length--;
      command[i] = '\0';
    }

    wm_print(window_id, "\n");

    if(command_length != 0)
    {
      add_to_history(window_id, command, command_length, &head, &tail, &history_length);
      parse_cmd(window_id, command, command_length, &head, &tail, history_length);
    }

    wm_print(window_id, "\n>");

    for(int i = 0; i < 50; i++)
    {
      command[i] = '\0';
    }

    command_length = 0;
  }

}

void start_shell()
{
  //wm_print('add process');
  create_process(shell_process, 4, 0, "Shell Process");
}
