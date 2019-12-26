/*
	CSE3033 - Project2
	Terminal

	Suleyman Baris Eser	---->> 150116055
	Murat Senol		---->> 150117039
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>
#include <wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_LINE 80
#define MAX_HISTORY 10

unsigned foreground_flag; // flag of foregorund procces to see if it is running
pid_t foreground_pid; // pid of foreground procces;
char path[MAX_LINE];
pid_t child[200];
int tail = 0;	
int success = 0;
int fd[2];  //pipe array
char *historyArray[10];
int history_count = 0; //counter for loop
int path_count = 0;    //counter for loop
int command_count = 0; //counter for loop
unsigned shell_argc = 1; //number of arguments
int redirect_position = -1; //position of redirection type
int redirection = 0;  //type of redirection
char *outfile;   //output file of redirection
char *infile;    //input file of redirection


void setup (char inputBuffer[], char *args[], int *background)
{
  int length,	
    i,		
    start,
    ct;				

  ct = 0;
  
  length = strlen(inputBuffer); 
  shell_argc = 1;

  //Removing ampersant sign from the inputBuffer  
  for(i = 0; i < length ; i++){
  	
     if(inputBuffer[i] == '&'){
	    *background = 1;
	    inputBuffer[i-1] = '\0';
	    break;     
     }
  	
  }
  
  if(length == 1)
  length--;
 
  start = -1;
  if (length == 0){
    exit (0);
 }			
  if ((length < 0) && (errno != EINTR))
    {
      fprintf(stderr, "%s\n", "Error reading to Command...");
      exit (-1);		
    }
  
  for (i = 0; i < length; i++)
    {				

      switch (inputBuffer[i])
	{
	case ' ': shell_argc++;
	case '\t':		
	  if (start != -1)
	    {
	      args[ct] = &inputBuffer[start];	
	      ct++;
	    }
	  inputBuffer[i] = '\0';	/* add a null char; make a C string */
	  start = -1;
	  break;

	case '\n':		/* should be the final char examined */
	  if (start != -1)
	    {
	      args[ct] = &inputBuffer[start];
	      ct++;
	    }
	  inputBuffer[i] = '\0';
	  args[ct] = NULL;	/* no more arguments to this command */
	  break;

	default:		/* some other character */
	  if (start == -1)
	    start = i;
	  
	}	/* end of switch */
    }		/* end of for */
  args[ct] = NULL;/* just in case the input line was > 80 */
  shell_argc = ct;
}

void foreground(){
  int i = 0;
  for (i = 0; i < tail; i++)
  {
    printf("childid :%d\n",child[i]);//Prints the childs working in the background 
  }
	return;
  
}

void commandPath(char *args[]){

    FILE *file;
    file = popen("printenv PATH", "r");

    int i;
    int row = 0, col = 0;

    char *line;
    char path_array[MAX_LINE][MAX_LINE];

    for(i = 0; i < MAX_LINE; i++){
        path[i] = '\000';
    }

    if (file != NULL) {

        for(i = 0; i < 1; i++) {

            char buffer[1000];

            line = fgets(buffer, sizeof buffer, file);

            if (line == NULL)
                break;
        }
    }
    for(i  = 0; line[i] != '\n'; i++){

        if(line[i] == ':'){ 

            path_array[row][col] = '\0';
            row++;
            col = 0;

        } else {

            path_array[row][col] = line[i];
            col++;

        }
    }

    char command[MAX_LINE/2+1];
    strcpy(command, args[0]);

    if(command[0] == '"'){
        for(i = 0; i < strlen(command); i++)
            command[i] = command[i+1];
    }

    success = 0;

    for(i = 0; i < row & success == 0; i++){

        strcpy(path, path_array[i]);

        DIR *dirp = opendir(path_array[i]);
        struct dirent *sd;

        if( dirp == NULL )
            continue;

        while ((sd = readdir(dirp)) != NULL){

            if(!strcmp(sd -> d_name, command)) {
                success = 1;
                break;
            }

        }

        (void)closedir(dirp);

    }
    if(success != 1){
        fprintf(stderr, "%s\n" , "PATH of obtained command cannot be found!\n");
	return;	
}
    pclose(file);

}

void addCommandToHistory(char *command){

	int i;
	
	if (history_count < MAX_HISTORY) {
		  for(i = history_count; i > 0 ; i--){
		  	  //Shifting operation 
			  historyArray[i]	= strdup(historyArray[i - 1]);	  
		  }
		  //Saving the lastly entered command to history array
        historyArray[0] = strdup( command );
        history_count++;
   } 
   else {
   	  //if the history array is full free the last element
        free( historyArray[9] );
        for(i = history_count; i > 0 ; i--){
		  	  //Shifting operation 
			  historyArray[i]	= strdup(historyArray[i - 1]);	  
		  }
        historyArray[0] = strdup( command );
   }

}

void checkRedirection(char *args[]){
  int i = 0;
  redirection = 0;
  for (i = 0; i < shell_argc; i++)
  {
    if(strcmp(args[i], ">") == 0){
      if (redirection == 3)	//if there is another redirection type '<'
      {
        outfile = args[i+1];	//output file
        redirection = 5;
        return;
      }
      
      redirection = 1;
      redirect_position = i;	//position of redirection type
      infile = NULL;
      outfile = args[i + 1];
      return;
    }
    if(strcmp(args[i], ">>") == 0){
      redirection = 2;
      redirect_position = i;
      infile = NULL;
      outfile = args[i + 1];
      return;
    }
    if(strcmp(args[i], "<") == 0){
      redirect_position = i;
      redirection = 3;
      infile = args[i + 1];
      outfile = NULL;
      continue;
    }
    if(strcmp(args[i], "2>") == 0){
      redirection = 4;
      redirect_position = i;
      infile = NULL;
      outfile = args[i + 1];
      return;
    }
  }
}


int history(char *args[] , char inputBuffer[] , int background){

	int i;
	//History Handling
      if(shell_argc == 1 && !strcmp(args[0] , "history")){
			for(i = 0 ; i < history_count ; i++){
				printf("%d %s\n" , i , historyArray[i]);//printing the history	
			}
			return -1;      
      }
      else if(shell_argc == 3 && !strcmp(args[0] , "history")){
			int temp_index = atoi(args[2]);
			strcpy(inputBuffer , historyArray[temp_index]);//copying the command from history to inputBuffer
			strcat(inputBuffer , "\n");
			setup(inputBuffer , args , &background);   
      }
      return 0;
}

void execShell(char *args[], int background, int result){
	
	int status;
		
	pid_t childpid;	//child process id
	childpid = fork();	//fork a new Child
	child[tail++] = childpid;
	
	result = childpid;
	if(childpid < 0){	//if fork error
		fprintf(stderr, "%s\n", "Fork Error!");
		exit(1);	
	}
	if(childpid == 0){	//if 
		commandPath(args);
		strcat(path, "/");
		strcat(path, args[0]);

    checkRedirection(args);   //check if there is any Redirection type whether not
    if (redirection != 0)   //if there is
    {
      args[redirect_position] = NULL;   //set place of redirection as Null, because execv cannot compile it
      if (redirection == 1 || redirection == 2 || redirection == 4) //if the redirection includes a writing process
      {
        if (redirection == 1 || redirection == 4)
          fd[0] = open(outfile , O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR); //Delete everything and write new output
        else if(redirection == 2)
          fd[0] = open(outfile , O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);  //Continue to writing from the last location
        if (fd[0] == -1)
        {
          fprintf(stderr, "%s\n", "Pipe Error!");
          return;
        }
        if (redirection == 1 || redirection == 2)
        {
          if(dup2(fd[0], STDOUT_FILENO) == -1){ //write the standart output
            fprintf(stderr, "%s\n", "Duplication Error");
            return;
          }
        }
        if (redirection == 4)
        {
          if(dup2(fd[0], STDERR_FILENO) == -1){ //write the error output
            fprintf(stderr, "%s\n", "Duplication Error");
            return;
          }
        }
        
        if(close(fd[0]) == -1){   //close the Pipe
          fprintf(stderr, "%s\n", "Close Error");
          return;
        }
      }
      if(redirection == 3){ //if the redirection includes a reading process
        fd[1] = open(infile ,O_RDONLY, S_IRUSR | S_IWUSR);  //create a reading pipeline
        if (fd[1] == -1)
        {
          fprintf(stderr, "%s\n", "Pipe Error");
          return;
        }
        if(dup2(fd[1], STDIN_FILENO) == -1){  //take the file as input of the process
          fprintf(stderr, "%s\n", "Duplication Error");
          return;
        }
        if(close(fd[1]) == -1){ //close the pipe
          fprintf(stderr, "%s\n", "Close Error");
          return;
        }
      }
      if(redirection == 5){
        args[redirect_position + 2]; //the next redirection type < file_IN > file_OUT
        fd[0] = open(outfile, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR); //create a writing pipe
        fd[1] = open(infile ,O_RDONLY, S_IRUSR | S_IWUSR);  //create a reading pipeline
        if (fd[0] == -1 && fd[1] == -1)
        {
          fprintf(stderr, "%s\n", "Pipe Error");
          return;
        }
        if(dup2(fd[1], STDIN_FILENO) == -1 && dup2(fd[0], STDOUT_FILENO) == -1){  //take the file as input of the process
          fprintf(stderr, "%s\n", "Duplication Error");
          return;
        }
        if(close(fd[1]) == -1 && close(fd[0]) == -1){ //close the pipe
          fprintf(stderr, "%s\n", "Close Error");
          return;
        }
      }
    }
      
      execv( path, args );
	fprintf(stderr, "%s\n", "Child Error!");
	return;
      
	}
	if( background == 0 )
    { 
      foreground_flag = 1;//flag to indcate that there is a foreground procces running
      foreground_pid = childpid;//saving the foreground procces id
			
    	//foreground procces
    	status = wait(NULL);

        if( childpid != status )
        {
            return;
        }
        else if( status == childpid){
        	 foreground_flag = 0;//foreground procces is finished
        	 foreground_pid = -1;//there is no foreground procces at the moment
        	     	
          //foregorund finished
        }
    }
    else{
    	return;
    	//Background procces
    }

}

void exitShell(){
  pid_t pid = waitpid(-1 , NULL , WNOHANG);  //collect all child processes.

  if(pid == 0){ 
    printf("There are still background processes! please close them.");
    return;
  }
  else
  {
    printf("exit!\n");
    exit(0);
  }
}


void handle_SIGTSTP()
{
  //CTRL_Z Signal Handling  
  
  //if there is no running foreground procces at the moment
  if(foreground_flag == 0){
  	  char message[] = "\nCtrl-Z was pressed , no foreground procces found\n";
	  write(STDERR_FILENO, message, strlen(message) );
	  return;
  }
  else{
  	  char message[] = "\nCtrl-Z was pressed , found the foreground procces\n";
     write(STDERR_FILENO, message, strlen(message) );
  	  kill(foreground_pid , 9);  
  	      	
  	  return;
  }
 
}

int main(int argc, char **argv, char **envp){

    int i, j, c , status , length ,a;
    char inputBuffer[MAX_LINE] = {0}; // buffer to hold command entered
    char *path_list = malloc(sizeof(char) * 300);
    int background;	// equals 1 if a command is followed by '&'
    char *args[MAX_LINE];	// command line arguments
    int result; // Will hold the result of CreateChildProcess().
    char *token;
    char *token2; 

	 //Signal structure to handle CTRL-Z press
	 struct sigaction handler;
    handler.sa_handler = handle_SIGTSTP;
    status = sigemptyset(&handler.sa_mask);
    
    if (status == -1)//Error checking
	{
		fprintf(stderr, "%s\n", "Failed to initialize signal set");
		exit(1);
	} // End if
    status =sigaction(SIGTSTP, &handler, NULL);
    if (status == -1)
	{
		fprintf(stderr, "%s\n", "Failed to set signal handler for SIGINT");
		exit(1);
	} // End if

	char *builtin_str[] = {"history" , "exit" , "fg" , "path"};//Built in functions
	
	strcpy(path_list , "");

    while (1){

      background = 0;
      strcpy(inputBuffer , "");
      printf("myshell> \n");
      fflush(NULL);
      
      
      fgets(inputBuffer , MAX_LINE , stdin);//Read the input
      
      token2 = strtok(inputBuffer , ";");//tokenize the input
      
      				
			setup( token2, args, &background);

      	for(i = 0; i < 4 ; i++){//Checks if the entered command is a built in command
      		if(!strcmp(args[0] , builtin_str[i])) {
					c=i;      	
      		}
     	   }
     	   
     	   
      	switch(c){
			  case 0: if(history(args , token2 , background) == -1 ){
				       continue;}
		             break;
		
		     case 1: //exit
        			    exitShell();
						 break;
		
			  case 2: //Fg
			 	foreground();
		             break;
		
		     case 3: //path
				
				       i=0;
				
						 if(shell_argc == 1 && strcmp(token2 , "path") == 0){
					       printf("%s\n" , path_list);//prints the path list
				       }
				       else if(shell_argc == 3  && strcmp(args[1] , "+") == 0 ){
			       		 strcat(path_list , args[2]);//Adds element to path list
				    	    strcat(path_list , ":");
				       }
						 else if(shell_argc == 3 && strcmp(args[1] , "-") == 0 ){
					
                      char *paths[20];					
					       token = strtok(path_list , ":");//tokenizing the path list
					
					       if(strcmp(token , args[2])  != 0 ){//if the current tokenized item is not the one will be removed
						       paths[0] = token;//add it to path array
						       i++;					
					       }			
					       while( token != NULL){
					
						    token = strtok(NULL , ":");
						    if(token == NULL)
						       break;

						    if(strcmp(token , args[2])  == 0 ){//if the current tokenized item is not the one will be removed
						       continue;						
						    }			
					
					       paths[i] = token;
						    i++;
				 
					     }
					     //So at the end of the loop the path that will be removed is not added to array
					
					     strcpy(path_list , "");//Reset the path_list
					
					     int temp = i;					
					     for( i = 0 ; i < temp ; i++){//Copy the path array elements to path_list
						    strcat(path_list , paths[i]);
						    strcat(path_list , ":");				
					     }
									
				      }
				      continue;
		            break;    
      
        }// End of switch
      

          execShell(args, background , result);//execute command in shell
		
          char tempCommand[MAX_LINE] = {0};//Temp array to copy the arguments
          strcpy(tempCommand , token2);
		
		    //Copying the commands arguments to string to save
  		    for(i = 1 ; i < shell_argc ; i++){
  			     strcat(tempCommand , " ");
			     strcat(tempCommand , args[i]); 
  	 	    }	
          //Add command to history
          addCommandToHistory(tempCommand);
		    
		token2 = strtok(NULL , ";");//Next token
		if(token2 !=NULL){
		setup( token2, args, &background);

      	for(i = 0; i < 4 ; i++){//Checks if the entered command is a built in command
      		if(!strcmp(args[0] , builtin_str[i])) {
					c=i;      	
      		}
     	   }
     	   
     	   
      	switch(c){
			  case 0: if(history(args , token2 , background) == -1 ){
				       continue;}
		             break;
		
		     case 1: //exit
        			    exitShell();
						 break;
		
			  case 2: //Fg
			 	foreground();
		             break;
		
		     case 3: //path
				
				       i=0;
				
						 if(shell_argc == 1 && strcmp(token2 , "path") == 0){
					       printf("%s\n" , path_list);//prints the path list
				       }
				       else if(shell_argc == 3  && strcmp(args[1] , "+") == 0 ){
			       		 strcat(path_list , args[2]);//Adds element to path list
				    	    strcat(path_list , ":");
				       }
						 else if(shell_argc == 3 && strcmp(args[1] , "-") == 0 ){
					
                      char *paths[20];					
					       token = strtok(path_list , ":");//tokenizing the path list
					
					       if(strcmp(token , args[2])  != 0 ){//if the current tokenized item is not the one will be removed
						       paths[0] = token;//add it to path array
						       i++;					
					       }			
					       while( token != NULL){
					
						    token = strtok(NULL , ":");
						    if(token == NULL)
						       break;

						    if(strcmp(token , args[2])  == 0 ){//if the current tokenized item is not the one will be removed
						       continue;						
						    }			
					
					       paths[i] = token;
						    i++;
				 
					     }
					     //So at the end of the loop the path that will be removed is not added to array
					
					     strcpy(path_list , "");//Reset the path_list
					
					     int temp = i;					
					     for( i = 0 ; i < temp ; i++){//Copy the path array elements to path_list
						    strcat(path_list , paths[i]);
						    strcat(path_list , ":");				
					     }
									
				      }
				      continue;
		            break;    
      
        }// End of switch
      

          execShell(args, background , result);//execute command in shell
		
          char tempCommand[MAX_LINE] = {0};//Temp array to copy the arguments
          strcpy(tempCommand , token2);
		
		    //Copying the commands arguments to string to save
  		    for(i = 1 ; i < shell_argc ; i++){
  			     strcat(tempCommand , " ");
			     strcat(tempCommand , args[i]); 
  	 	    }	
          //Add command to history
          addCommandToHistory(tempCommand);
		}
    }   // End of While Loop

}
