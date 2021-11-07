#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include<bits/stdc++.h>
using namespace std;

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0;
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


//global variables
//for parallel Background
pid_t* bgp_list = new pid_t[64];
pid_t* fgp_list = new pid_t[64];


bool should_skip = false;

void sign_handler(int s){
    should_skip = true;
    for(int k=0; k<64; k++){
        if(fgp_list[k] > 0){
            int killer = kill(fgp_list[k], SIGKILL);
            if(killer >0){
                printf("Shell: foreground process %d killed\n", fgp_list[k]);
                fgp_list[k] = -1;
            }
        }
    }
    for(int k=0; k<64; k++){
        if(fgp_list[k]>0){
            int killer = waitpid(fgp_list[k], NULL, 0);
            if(killer > 0){
                printf("Shell: foreground process %d reaped\n", fgp_list[k]);
                fgp_list[k] = -1;
            }
        }
    }
    cout<<"\n";
}




int main(int argc, char* argv[]) {
    char  line[MAX_INPUT_SIZE];
	char  **tokens;
	int i;

    bgp_list[64]={-1};
    fgp_list[64]={-1};
    int bgp_list_index = 0;
    int fgp_list_index =0;

	while(1) {

        signal(SIGINT, sign_handler);
        should_skip = false;

		/* BEGIN: TAKING INPUT */
	    bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		//printf("Command entered: %s \n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

        //do whatever you want with the commands, here we just print them

        //reaping Background childs
        for(int k=0; k<64; k++){
            if(bgp_list[k]>0){
                int killer = waitpid(bgp_list[k], NULL, WNOHANG);
                if(killer > 0){
                    printf("Shell: Background process %d finished\n", bgp_list[k]);
                    bgp_list[k] = -1;
                }
            }
        }

        //for token size
        int t_size;
        //for checkpoint of case
        bool two_ampr = false, three_ampr = false;
        //impleminting both
        for(t_size=0; tokens[t_size]!=NULL;t_size++){
            if(string(tokens[t_size]) == "&&") two_ampr = true;
            else if(string(tokens[t_size]) == "&&&") three_ampr = true;
        }

        //if no commands given
        if(t_size<=0){
            continue;
        }
        //for exit
        else if(string(tokens[0])=="exit"){
            //  kill all childs process
            for(int k=0; k<64; k++){
                if(bgp_list[k]>0){
                    int killer = kill(bgp_list[k], SIGKILL);
                    if(killer > 0){
                        printf("Shell: Background process %d killed\n", bgp_list[k]);
                        bgp_list[k] = -1;
                    }
                }
            }
            // clean all zombies
            for(int k=0; k<64; k++){
                if(bgp_list[k]>0){
                    int killer = waitpid(bgp_list[k], NULL, 0);
                    if(killer > 0){
                        printf("Shell: Background process %d reaped\n", bgp_list[k]);
                        bgp_list[k] = -1;
                    }
                }
            }
            //free tokens
            for(i=0;tokens[i]!=NULL;i++){
    			free(tokens[i]);
    		}
    		free(tokens);
            //free arrays
            free(bgp_list);
            free(fgp_list);

            return 0;
        }

        //part B.1 end token &
        else if(string(tokens[t_size-1]) == "&"){
            //drop &
            tokens[t_size-1] = NULL;
            // for cd
            if(string(tokens[0])=="cd"){
                if(t_size ==2){              //if cd <null> incorrect
                    cout<<"Shell: Incorrect command1cd\n";
                }
                else{
                    char dir[200];
                    string old_dir = getcwd(dir, 200);
                    if(chdir(tokens[1]) !=0){      // if cd <wrong arg> change dir
                        cout<<"Shell: Incorrect command2cd\n";
                    }
                    else{
                        cout<<old_dir<<endl<<getcwd(dir, 200);
                    }
                }
            }
            //for others fork()
            else{
                int Fork = fork();

                if(Fork < 0){
                    cout<<"Shell: fork failed\n";
                    exit(1);
                }
                else if(Fork == 0){          //if child is running
                    // cout<<"child pid (me) "<<getpid()<<endl;
                    execvp(tokens[0], tokens);
                    printf("Shell: '%s' is Incorrect command\n", tokens[0]);
                    exit(0);
                }
                else{            //reap child
                    bgp_list[bgp_list_index] = Fork;
                    bgp_list_index++;
                    //cout<<"child pid (parent) "<<Fork<<endl;
                }
            }
        }

        //part B.2 &&
        else if(two_ampr){

            string dummy_token[64];
            int dummy_index=0;

            for(i=0; tokens[i] != NULL; i++){
                //if && comes process
                if(string(tokens[i]) == "&&"){
                    //for cd
                    if(string(dummy_token[0].c_str())=="cd"){
                        if(dummy_index <= 1){              //if cd <null> incorrect
                            cout<<"Shell: Incorrect command3cd\n";
                        }
                        else{
                            char dir[200];
                            string old_dir = getcwd(dir, 200);
                            if(chdir(dummy_token[1].c_str()) !=0){      // if cd <wrong arg> change dir
                                cout<<"Shell: Incorrect command4cd\n";
                            }
                            else{
                                cout<<old_dir<<endl<<getcwd(dir, 200);
                            }
                        }
                    }
                    //for other fork()
                    else{
                        int Fork = fork();

                        if(Fork < 0){
                            cout<<"Shell: fork failed\n";
                            exit(1);
                        }
                        else if(Fork == 0){        //if child is running

                            char *dd_token[dummy_index+1];
            				for(int j=0; j<=dummy_index; j++){
                                dd_token[j] = strdup(dummy_token[j].c_str());
            				}
                            dd_token[dummy_index] = NULL;

                            execvp(dd_token[0], dd_token);
                            printf("Shell: '%s' is Incorrect command\n", dummy_token[0].c_str());
                            exit(0);
                        }
                        else{           //parent //reap child
                            int WaitChild = wait(NULL);
                        }
                    }
                    dummy_index=0;
                }
                else{
                    dummy_token[dummy_index] = tokens[i];
                    dummy_index++;
                }
            }

            if(dummy_index > 0){
                //for cd
                if(string(dummy_token[0].c_str())=="cd"){
                    if(dummy_index <= 1){              //if cd <null> incorrect
                        cout<<"Shell: Incorrect command3cd\n";
                    }
                    else{
                        char dir[200];
                        string old_dir = getcwd(dir, 200);
                        if(chdir(dummy_token[1].c_str()) !=0){      // if cd <wrong arg> change dir
                            cout<<"Shell: Incorrect command4cd\n";
                        }
                        else{
                            cout<<old_dir<<endl<<getcwd(dir, 200);
                        }
                    }
                }
                        //for other fork()
                else{
                    int Fork = fork();

                    if(Fork < 0){
                        cout<<"Shell: fork failed\n";
                        exit(1);
                    }
                    else if(Fork == 0){        //if child is running

                        char *dd_token[dummy_index+1];
        				for(int j=0; j<=dummy_index; j++){
                            dd_token[j] = strdup(dummy_token[j].c_str());
        				}
                        dd_token[dummy_index] = NULL;

                        execvp(dd_token[0], dd_token);
                        printf("Shell: '%s' is Incorrect command\n", dummy_token[0].c_str());
                        exit(0);
                    }
                    else{           //parent //reap child
                        int WaitChild = wait(NULL);
                    }
                }
                dummy_index=0;
            }
        }

        //part B.3   &&&
        else if(three_ampr){

            string dummy_token[64];
            int dummy_index=0;

            for(i=0; tokens[i] != NULL; i++){
                //if &&& comes
                if(string(tokens[i]) == "&&&"){
                    //if cd comes
                    if(string(dummy_token[0].c_str())=="cd"){
                            //only cd
                            if(dummy_index <= 1){              //if cd <null> incorrect
                                cout<<"Shell: Incorrect command5cd\n";
                            }
                            else{
                                char dir[200];
                                string old_dir = getcwd(dir, 200);
                                if(chdir(dummy_token[1].c_str()) !=0){      // if cd <wrong arg> change dir
                                    cout<<"Shell: Incorrect command6cd\n";
                                }
                                else{
                                    cout<<old_dir<<endl<<getcwd(dir, 200);
                                }
                            }
                        }
                    //for other command
                    else{
                        int Fork = fork();

                        if(Fork < 0){
                            cout<<"Shell: fork failed\n";
                            exit(1);
                        }
                        //if child is running
                        else if(Fork == 0){
                            //re tokenise for C
                            char *dd_token[dummy_index+1];
            				for(int j=0; j<=dummy_index; j++){
                                dd_token[j] = strdup(dummy_token[j].c_str());
            				}
                            dd_token[dummy_index] = NULL;

                            execvp(dd_token[0], dd_token);
                            printf("Shell: '%s' is Incorrect command\n", dummy_token[0].c_str());
                            exit(0);
                        }
                        else{           //parent //reap child
                            fgp_list[fgp_list_index] = Fork;
                            fgp_list_index++;
                            //cout<<"child pid ppid3"<<Fork<<endl;
                        }
                    }
                    dummy_index=0;
                }
                else{
                        dummy_token[dummy_index] = tokens[i];
                        dummy_index++;
                    }
            }

            if(dummy_index > 0){
                //for cd
                if(string(dummy_token[0].c_str())=="cd"){
                    if(dummy_index <= 1){              //if cd <null> incorrect
                        cout<<"Shell: Incorrect command3cd\n";
                    }
                    else{
                        char dir[200];
                        string old_dir = getcwd(dir, 200);
                        if(chdir(dummy_token[1].c_str()) !=0){      // if cd <wrong arg> change dir
                            cout<<"Shell: Incorrect command4cd\n";
                        }
                        else{
                            cout<<old_dir<<endl<<getcwd(dir, 200);
                        }
                    }
                }
                        //for other fork()
                else{
                    int Fork = fork();

                    if(Fork < 0){
                        cout<<"Shell: fork failed\n";
                        exit(1);
                    }
                    else if(Fork == 0){        //if child is running

                        char *dd_token[dummy_index+1];
        				for(int j=0; j<=dummy_index; j++){
                            dd_token[j] = strdup(dummy_token[j].c_str());
        				}
                        dd_token[dummy_index] = NULL;

                        execvp(dd_token[0], dd_token);
                        printf("Shell: '%s' is Incorrect command\n", dummy_token[0].c_str());
                        exit(0);
                    }
                    else{           //parent //reap child
                        fgp_list[fgp_list_index] = Fork;
                        fgp_list_index++;
                    }
                }
                dummy_index=0;
            }

            for(int k=0; k<64; k++){
                if(fgp_list[k]>0){
                    int killer = waitpid(fgp_list[k], NULL, 0);
                    if(killer > 0){
                        printf("Shell: foreground process %d finished\n", fgp_list[k]);
                        fgp_list[k] = -1;
                    }
                }
            }
        }

        //part A
        else{
            // if cd
            if(string(tokens[0])=="cd"){
                if(t_size == 1){              //if cd <null> incorrect
                    cout<<"Shell: Incorrect command1Acd\n";
                }
                else{
                    char dir[200];
                    string old_dir = getcwd(dir, 200);
                    if(chdir(tokens[1]) !=0){      // if cd <wrong arg> change dir
                        cout<<"Shell: Incorrect command2Acd\n";
                    }
                    else{
                        cout<<old_dir<<endl<<getcwd(dir, 200);
                    }
                }
            }

            //for other commands
            else{
                int Fork = fork();

                if(Fork < 0){
                    cout<<"Shell: fork failed\n";
                    exit(1);
                }
                else if(Fork == 0){          //if child is running
                    execvp(tokens[0], tokens);
                    printf("Shell: '%s' is Incorrect command\n", tokens[0]);
                    exit(0);
                }
                else{           //parent //reap child
                    int WaitChild = wait(NULL);
                }
            }
        }

		for(i=0; tokens[i]!=NULL;i++){
			//printf("found token %s \n", tokens[i]);
		}

		// Freeing the allocated memory
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}
