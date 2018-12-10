/*
Authored by Ali Alavi
Student ID # 917932397
*/
#include <kernel.h>

const char *default_commands[] = {"cls", "ps", "pong", "shell", "history", "help", "about"};

typedef struct cmd_hist_node {
  int index;
  char *cmd;
  struct cmd_hist_node *next;
} _cmd_hist_node;

#define DEFAULT_COMMANDS_LENGTH 7
#define MAX_INPUT 50

#define ASCII_ZERO 48
#define ASCII_NINE 57
#define CARRIAGE_RETURN 13 
#define BACKSPACE 8

int str_to_int(char * num) //converting a string representation of number to int
{
  int new_int = 0;
  int i = 0;

  while(num[i] != '\0')
  {
    new_int = new_int * 10 + ((int)num[i] - ASCII_ZERO);
    i++;
  }

  return new_int;
}

void set_command(char * history_cmd, char * new_cmd) //copy entered command to history node
{
  int i = 0;

  while(new_cmd[i] != '\0')
  {
    history_cmd[i] = new_cmd[i];
    i++;
  }

  history_cmd[i+1] = '\0';
}

int find_command(char* cmd) //search default_commands list to get the index of command passed
{
  for(int i = 0; i < DEFAULT_COMMANDS_LENGTH; i++)
  {
    if(k_memcmp(cmd, default_commands[i], k_strlen(default_commands[i])) == 0)
      return i;
  }

  return -1;
}

void print_help(int window_id)
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

void print_about(int window_id)
{
  wm_print(window_id, "TOS is an OS written in C. \nCopyright Ali Alavi 2018\n");
}

void echo(int window_id, char* cmd)
{
  int cmd_len = k_strlen(cmd);
  int echo_len = k_strlen(cmd) - 5; //ignore 'echo '
  char * cmd_to_prnt = (char *) malloc((echo_len + 1) * sizeof(char));
  int j =0;

  for(int i = 5; i< cmd_len; i++)
  {
    cmd_to_prnt[j] = cmd[i];
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

void print_history(int window_id, _cmd_hist_node * head)
{
  _cmd_hist_node *temp = head;
  while(temp != NULL)
  {
    wm_print(window_id, "%d - %s\n",temp->index, temp->cmd);
    temp = temp-> next;
  }
}

void exec_history_cmd(int window_id, int hist_index, _cmd_hist_node * head, int hist_len)
{
  int i = 0;

  while(head->next != NULL)
  {
    if(i == hist_index)
    {
      wm_print(window_id, "%s\n", head->cmd);
      run_command(window_id, head->cmd, k_strlen(head->cmd), &head, hist_len);
      break;
    }
    else
    {
      i++;
      head = head->next;
    }
  }
}

void run_exclamation(int window_id, char * cmd, int cmd_len, _cmd_hist_node * head, int hist_len)
{
  int i = 1; // ignore '!'
  char * char_hist_index = (char *) malloc(cmd_len * sizeof(char));

  while(cmd[i] != '\0') // check if characters after '!' are all numbers else throw error
  {
    if(cmd[i] < ASCII_ZERO || cmd[i] > ASCII_NINE)
    {
      wm_print(window_id, "Invalid command entered. Please enter history to see the list of supported commands");
      return;
    }
    char_hist_index[i - 1] = cmd[i];
    i++;
  }

  int hist_index = str_to_int(char_hist_index);

  if(hist_index > hist_len)
  {
      wm_print(window_id, "Invalid number provided, total %d commands in history", hist_len);
      return;
  }
  else
  {
    exec_history_cmd(window_id, hist_index, head, hist_len);
    return;
  }
}

void add_to_history(int window_id, char * cmd, int cmd_len, _cmd_hist_node ** head, _cmd_hist_node ** tail, int * hist_len)
{
  _cmd_hist_node* new_hist_node = (_cmd_hist_node*)  malloc(sizeof(_cmd_hist_node));
  new_hist_node->cmd = (char *) malloc((cmd_len + 1) * sizeof(char));
  set_command(new_hist_node->cmd, cmd);
  new_hist_node->next = NULL;
  new_hist_node->index = (*hist_len);
  (*hist_len)++;

  if((*head) == NULL) // head and tail both point to the first entry
  {
    *head = new_hist_node;
    *tail = new_hist_node;
  }
  else // add new entries to tail
  {
    (*tail)->next = new_hist_node;
    *tail = (*tail)->next;
  }
}

void parse_cmd(int window_id, char * command, int cmd_len, _cmd_hist_node** head, _cmd_hist_node** tail, int hist_len)
{
  int start = 0;
  int i = 0;
  char * sub_cmd;
  int sub_cmd_len;
  int j = 0;
  int right_space = 0;

  while(i < cmd_len + 1)
  {
    if(command[i] == ';' || command[i] == '\0') //split on ; or when command ends
    {
        if(command[start] == ' ') // remove whitespace on the left
          start++;

        if(command[i-1] == ' ') // remove whitespace on the right
        {
          right_space = 1;
          i--;
        }

        sub_cmd_len = i - start; // sub command length

        sub_cmd = (char *) malloc((sub_cmd_len + 1) * sizeof(char));

        while(start < i)
        {
          sub_cmd[j] = command[start];
          start++;
          j++;
        }

        sub_cmd[sub_cmd_len] = '\0'; // terminate sub command string

        run_command(window_id, sub_cmd, sub_cmd_len, head, hist_len); //execute the sub command

        if(right_space) // increment i if there was whitespace on the right initially
          i++;

        start = i + 1;
        j = 0;
    }

    i++;
  }
}

void run_command(int window_id, char * cmd, int cmd_len, _cmd_hist_node** head, int hist_len)
{
  int command_index;

  if(cmd[0] == '!') // handle special case for excalamation
  {
    command_index = 7;
  }
  else if(k_memcmp(cmd, "echo", 4) == 0) // handle special case for echo
  {
    command_index = 8;
  }
  else
  {
    command_index = find_command(cmd); // search entered command in default_commands
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
      print_history(window_id, (*head));
      break;
    case 5:
      print_help(window_id);
      break;
    case 6:
      print_about(window_id);
      break;
    case 7:
      run_exclamation(window_id, cmd, cmd_len, (*head), hist_len);
      break;
    case 8:
      echo(window_id, cmd);
      break;
    default:
      wm_print(window_id, "Invalid command entered");
      break;
  }
}

void shell_process(PROCESS self, PARAM param)
{
  char command[MAX_INPUT];
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

    if(ch == ' ') // ignore whitespace
    {
      wm_print(window_id, "%c", ch);
      input_length++;
      continue;
    }

    while(ch != CARRIAGE_RETURN) // loop till user hits enter
    {
      if(ch == BACKSPACE) // handle backspace
      {
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

      if(ch != BACKSPACE)
      {
        if(ch == ' ')
        {
          mid_space_length++;
        }
        else
        {
          while(mid_space_length > 1) // remove extra space between two non-space characters
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

      ch = keyb_get_keystroke(window_id, TRUE);
    }
    mid_space_length = 0;
    input_length = 0;

    for(int i = command_length - 1; i > 0; i--) //remove whitespace from the end
    {
      if(command[i] != ' ')
        break;

      command_length--;
      command[i] = '\0';
    }

    wm_print(window_id, "\n");

    if(command_length != 0) // if empty command, ignore
    {
      add_to_history(window_id, command, command_length, &head, &tail, &history_length);
      parse_cmd(window_id, command, command_length, &head, &tail, history_length);
    }

    wm_print(window_id, "\n>"); // get new command

    for(int i = 0; i < 50; i++) // clear input command
    {
      command[i] = '\0';
    }

    command_length = 0;
  }

}

void start_shell()
{
  create_process(shell_process, 4, 0, "Shell Process");
}
