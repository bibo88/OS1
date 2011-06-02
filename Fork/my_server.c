/* ----------------------------------------------------README---------------------------------------------------- 
 * This is the code implementing the server part of the first project in the course "Operating Systems I"
 * in CEID of University of Patras.
 *
 * Some additional notes & Special Thanks :
 *		See my_client.c file for these stuff. ;)
 *
 *
 * Authors		: .ios			      		bibo
 * 			  Kotsogiannis - Teftsoglou Ioannis	Kalargaris Charalambos; AM 3961,3889 respectively
 * Latest revision	: Nov 20 2009
 * --------------------------------------------------------------------------------------------------------------
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/un.h> 	/* for Unix domain sockets 	*/
#include <errno.h> 	/* for the EINTR constant 	*/
#include <sys/wait.h> 	/* for the waitpid() system call*/
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "defs.h"

/*
* Variables for semaphores 
*/
sem_t *my_sem;

/*
* Data stores the position of the beginning of our shared memory.
* We have to declare it globally, so that our SIG_INT handler can access it - in order to make the detachment.
* The same applies to shm_id, we declare it globally so our SIG_INT handler can access it and shmctl the shared memory.
*/
char *data;
int shm_id;

int main()
{


	/********************************* Initilization Section *********************************/
	/*
	* Variables for the program 
	*/
	account * rcv_tmp, * shm_now_data; 
	int trans_type, i, control, test_name, test_surname, acc_exists, allow_with;
	int * max_id;

	
	
	/* Allocate some memory for our pointers to accounts, and also clear that memory	*/	
	rcv_tmp = malloc(sizeof(account));
	memset(rcv_tmp,0, sizeof(account));

	shm_now_data = malloc(sizeof(account));
	memset(shm_now_data,0, sizeof(account));
	
	/*
	* Variables for sockets
	*/
	int  ns, sd, len;
	socklen_t fromlen;
	struct sockaddr sa, fsa;

	 /* Variables for shared memory*/
	int shm_error;

	key_t shm_key;
	
	/* Variables for fork()*/
	pid_t pid;
	
	/* Avoid "zombie" process generation. */
	signal( SIGCHLD, sig_chld );

	/******************************** Shared Memory Creation ********************************/

	/* 
	* First we generate a shm_key via ftok().
	* We also check for errors.
	*/
	
	
	if ( (shm_key = ftok(SHM_KEY, SHM_ID)) == (key_t) - 1) {
		perror("IPC error: ftok"); exit(1);
	}
	
	/*
	* Here we use the generated key to shmget().
	* We also have an error control.	
	*/
	
	
	if ( (shm_id = shmget( shm_key, SHM_SIZE, 0600 | IPC_CREAT )) < 0 ){
		perror("shmget");
		printf("Could not create shared memory, mothafakaaaaa:%d\n", errno);
		exit(1);
	}

	/*
	* Here we attach shared memory to the server process,
	* this attachment will be inherited to all child processes.
	* Alternatively the attachment can be done within each forked/child process seperately.
	* We also check if shared memory was successfully attached.
	*/

	if ( (data = shmat( shm_id, NULL, 0)) == (char*) - 1 ){
		perror("shmat");
		printf("Could not attach to shared memory, mothafakaaaaaaaa\n");
		exit(1);
	}
	
	signal( SIGINT, sig_int );
	/******************************** Semaphores Creation ********************************/
	
	/*
	* Here we open the sempaphore
	*/

    	my_sem=sem_open(SEM_NAME,O_CREAT | O_RDWR,S_IRUSR | S_IWUSR,1);

	/* 
	*Here we check for error if the the semaphore didn't open
	*/

	if (my_sem == SEM_FAILED) {
		printf("Could not open semaphore, mothafakaaaaaaaa.\n");
		exit(1);
	}

	/********************************** Socket Creation **********************************/

	/* 
	* Create socket; check if socket() failed.
	*/

	if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("server: socket" );
		exit(1);
	}

	/*
	* Create the address we will be binding to.
	* Define the properties of socket struct "sa"
	*/
	sa.sa_family = AF_UNIX;
	strcpy(sa.sa_data, SOCK_NAME);

	/*
	* Try to bind the address to the socket. We unlink
	* the name first so that the bind won't fail.
	*
	* The third argument indicates the "length" of the
	* structure, not just the length of the socket name.
	*/
	unlink(SOCK_NAME);
	len = sizeof(sa.sa_family) + sizeof(sa.sa_data);


	/*
	* Here we bind the socket.
	*/
	if ( bind(sd, &sa, len) < 0 ) {
		perror("server: bind" );
		exit(1);
	}

	/*
	* Listen to the socket.
	*/
	if ( listen(sd, 1) < 0) {
		perror("server: listen" );
		exit(1);
	}

	max_id = (int *)data;
	*max_id = 0;
	


	/******************************** Main Program Section *******************************/

	while (1){ 
		/*
		* Accepting new connections; check if accept succeeded.	
		*/
		if(getpid()!=0)
			printf("Server awaits for client to connect...\n");
		
		if ((ns = accept(sd, &fsa, &fromlen)) < 0) {
			perror("server: accept" );
			exit(1);
		}
		printf("Client connected to server.\n");
		
		/*
		* Create a child server process to handle client request
		*/


		if ( ( pid = fork() ) == 0 ){

			
			/*
			* Here we receive the type of transaction needed to be resolved
			*/
			read(ns, &trans_type, sizeof(int)		);
			
			/*
			* Here we create an account pointer that will point at the desired position in our shared memory.
			* Each time and according to the type of transaction the data to our shared memory will change accordingly.			
			*/			

			
			switch(trans_type){

				/* Create account case				*/
				case 1: 
					/*
					* Here we receive the information sent by client.
					*/
					read(ns, rcv_tmp, sizeof(account) 	);
					
					/*
					* max_id is a unique number that increases with the creation of each new account
					* using max_id we assign unique_id to each account.
					* max_id is stored in the very beginning of our shared memory.
					*/
					sem_wait(my_sem);
					rcv_tmp->u_id = *max_id;
					sem_post(my_sem);

					/*
					* Here we check if user has already an account to our bank.
					* We simply check all entries and compare them with the new entry.
					*/

					acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);
					/* We send to the client wether or not the account already exists	*/
					write(ns, &acc_exists, sizeof(int));
					
					if (acc_exists){
						printf("Already has an account, will now terminate.\n");

					/* 
					* Using this break instead of exit(0) we will go to the end of the switch code
					* Where memory will be detached first and then the process will exit.
					*/
						break;
					}
	
					/*
					* We copy all new data to the shared memory. 
					* We save the new entry at the end of our shared memory.
					* To find the end of shared memory we simply check max_id variable.
					*/
					
					/* Set the pointer at the correct position in shared memory.		*/
					sem_wait(my_sem);
					shm_now_data = (account *)(data + sizeof(int) + (*max_id)*sizeof(account));

					/* Store firstname.							*/		
					strncpy(shm_now_data->firstname, rcv_tmp->firstname,15*sizeof(char));
					
					/* Store surname.							*/
					strncpy(shm_now_data->surname, rcv_tmp->surname,15*sizeof(char));	
					
					/* Store unique account id.						*/
					shm_now_data->u_id =  rcv_tmp->u_id;
					
					/* Set balance to zero - since it is a new account.			*/
					shm_now_data->balance = 0;
					
					/*Clear the history of transactions. 					*/
					for(i=0;i<4;i++){
						memset(&shm_now_data->transaction[i],0,sizeof(int));
					}
					/* Print the details of the account for debugging reasons		*/					
					printdetails(shm_now_data);	

					/* Icrease max_id by one each time an account is created		*/					
					(*max_id)++;
					sem_post(my_sem);					
					/* Create account case ends						*/
					break;
				
				case 2:
					/* We read user's access account choice					*/
					read(ns, &control, sizeof(int) );

					/* We read the data user sent to the server				*/
					read(ns, rcv_tmp, sizeof(account) );
					
					if (control == 1){

						/*					
						* What we will do is check wether the account exists or not
						* and do the proper actions for each case
						*/

						/* We check if account exists... 					*/
						acc_exists = id_exists(rcv_tmp, max_id);

						/* ...inform the client wether or not the account exists... 		*/
						write(ns, &acc_exists, sizeof(int));

						/* 
						* If the account does not exist we simply terminate this proccess.
						* Using break ensures us that memory will be detached.
						*/
						if (acc_exists == 0) {
							break;
						}

						/* Else we go to the desired account in our shared memory.		*/
						sem_wait(my_sem);
						shm_now_data = (account *)(data + sizeof(int) + (rcv_tmp->u_id)*sizeof(account));
	
						/* And we add to our balance the deposited amount			*/
						shm_now_data->balance = shm_now_data->balance + rcv_tmp->balance;


						/* Here we update the transaction history of the account		*/
						for (i = 3; i > 0; i--){
							shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
						}
						shm_now_data->transaction[0] = rcv_tmp->balance;  

						/* Print the details of the account for debugging reasons		*/				
						printdetails(shm_now_data);
						sem_post(my_sem);
						}
					
					if (control == 2) {
					
						/*					
						* What we will do is check wether the account exists or not
						* and do the proper actions for each case
						*/

						/* Here we check for existance of account 				*/
						acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);

						/* ...inform the client wether or not the account exists... 		*/
						write(ns, &acc_exists, sizeof(int));

						/* 
						* If the account does not exist we simply terminate this proccess.
						* Using break ensures us that memory will be detached.
						*/
						if (acc_exists == 0) {
							break;
						}

						for(i=0; i< (*max_id); i++){
							sem_wait(my_sem);
							shm_now_data = (account *)(data + sizeof(int) + i*sizeof(account));
							/*
							* We compare the current name and surname with all names and surnames already
							* stored in our shared memory.
							*/
							test_surname = strcmp(shm_now_data->surname, rcv_tmp->surname);
							test_name = strcmp(shm_now_data->firstname, rcv_tmp->firstname);
		
							if ((test_surname == 0) && (test_name == 0) )  {
								/* If name and surname are the same then we found our unique entry.	*/
								printf("Account found.\n");
								sem_post(my_sem);
								break;	
							}
							sem_post(my_sem);
						}
						/* Then  we add to our balance the deposited amount.			*/
						sem_wait(my_sem);
						shm_now_data->balance = shm_now_data->balance + rcv_tmp->balance;

						/* Here we update the transaction history of the account		*/
						for (i = 3; i > 0; i--){
							shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
						}
						shm_now_data->transaction[0] = rcv_tmp->balance;  

						/* We print out the new account status 					*/
						printdetails(shm_now_data);
						sem_post(my_sem);
					}
					break;
				
				case 3: 
					/* We read user's access account choice					*/
					read(ns, &control, sizeof(int) );

					/* We read the data user sent to the server				*/
					read(ns, rcv_tmp, sizeof(account) );
					
					if (control == 1){

						/*					
						* What we will do is check wether the account exists or not
						* and do the proper actions for each case
						*/

						/* We check if account exists... 					*/
						acc_exists = id_exists(rcv_tmp, max_id);

						/* ...inform the client wether or not the account exists... 		*/
						write(ns, &acc_exists, sizeof(int));

						/* 
						* If the account does not exist we simply terminate this proccess.
						* Using break ensures us that memory will be detached.
						*/
						if (acc_exists == 0) {
							break;
						}

						/* Else we go to the desired account in our shared memory.		*/
						sem_wait(my_sem);
						shm_now_data = (account *)(data + sizeof(int) + (rcv_tmp->u_id)*sizeof(account));


						/*TODO check if balance is enough */

						allow_with = allow_withdraw(rcv_tmp, shm_now_data);
		
						/* We send back to client wether or not the balance is enough		*/
						write(ns, &allow_with, sizeof(int));


						/* And we add to our balance the deposited amount			*/
						shm_now_data->balance = shm_now_data->balance - rcv_tmp->balance;


						/* Here we update the transaction history of the account		*/
						for (i = 3; i > 0; i--){
							shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
						}
						shm_now_data->transaction[0] = -rcv_tmp->balance;  

						/* Print the details of the account for debugging reasons		*/				
						printdetails(shm_now_data);
						sem_post(my_sem);
						}
					
					if (control == 2) {
					
						/*					
						* What we will do is check wether the account exists or not
						* and do the proper actions for each case
						*/
						
						/* Here we check for existance of account 				*/
						acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);

						/* ...inform the client wether or not the account exists... 		*/
						write(ns, &acc_exists, sizeof(int));

						/* 
						* If the account does not exist we simply terminate this proccess.
						* Using break ensures us that memory will be detached.
						*/
						if (acc_exists == 0) {
							break;
						}

						for(i=0; i< (*max_id); i++){
							sem_wait(my_sem);
							shm_now_data = (account *)(data + sizeof(int) + i*sizeof(account));
							/*
							* We compare the current name and surname with all names and surnames already
							* stored in our shared memory.
							*/
							test_surname = strcmp(shm_now_data->surname, rcv_tmp->surname);
							test_name = strcmp(shm_now_data->firstname, rcv_tmp->firstname);
		
							if ((test_surname == 0) && (test_name == 0) )  {
								sem_post(my_sem);
								/* If name and surname are the same then we found our unique entry.*/
								printf("Account found.\n");
								break;	
							}
							sem_post(my_sem);
						}
						sem_wait(my_sem);
						/* Here we check if balance is enough for withdrawal			*/
						allow_with = allow_withdraw(rcv_tmp, shm_now_data);

						/* We send back to client wether or not the balance is enough		*/
						write(ns, &allow_with, sizeof(int));
						

						/* And we add to our balance the deposited amount			*/
						shm_now_data->balance = shm_now_data->balance - rcv_tmp->balance;


						/* Here we update the transaction history of the account		*/
						for (i = 3; i > 0; i--){
							shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
						}
						shm_now_data->transaction[0] = -rcv_tmp->balance;  
						/* We print out the new account status 					*/
						printdetails(shm_now_data);
						sem_post(my_sem);
					}


					break;
				
				case 4: 
					
					/* We read user's access account choice						*/
					read(ns, &control, sizeof(int));

					/* We read the data user sent to the server					*/
					read(ns, rcv_tmp, sizeof(account));

					if (control==1){
						
						/* We check if account exists... 					*/
						acc_exists = id_exists(rcv_tmp, max_id);

						/* ...inform the client wether or not the account exists... 		*/
						write(ns, &acc_exists, sizeof(int));

						/* 
						* If the account does not exist we simply terminate this proccess.
						* Using break ensures us that memory will be detached.
						*/
						if (acc_exists == 0) {
							break;
						}

						/* Else we go to the desired account in our shared memory.		*/
						sem_wait(my_sem);
						shm_now_data = (account *)(data + sizeof(int) + (rcv_tmp->u_id)*sizeof(account));
						
						/* and we send the data to client					*/
						write(ns, shm_now_data, sizeof(account));
						sem_post(my_sem);
					}
					if (control == 2 ){
						
						/* Here we check for existance of account 				*/
						acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);

						/* ...inform the client wether or not the account exists... 		*/
						write(ns, &acc_exists, sizeof(int));

						/* 
						* If the account does not exist we simply terminate this proccess.
						* Using break ensures us that memory will be detached.
						*/
						if (acc_exists == 0) {
							break;
						}

						for(i=0; i< (*max_id); i++){
							shm_now_data = (account *)(data + sizeof(int) + i*sizeof(account));
							/*
							* We compare the current name and surname with all names and surnames already
							* stored in our shared memory.
							*/
							test_surname = strcmp(shm_now_data->surname, rcv_tmp->surname);
							test_name = strcmp(shm_now_data->firstname, rcv_tmp->firstname);
		
							if ((test_surname == 0) && (test_name == 0) )  {
							/* If name and surname are the same then we found our unique entry.	*/
								printf("Account found.\n");
								break;	
							}				
						}
						/* Now we have found the account and we simply send it to the client		*/
						write(ns, shm_now_data, sizeof(account));
						
					}			
					break;
				
				case 5: /* No need to write any code to handle "exit" functionality				
					   The code needed to be run for a correct "exit" lies right after the switch and will be
					   executed no matter what (well at most of the times).					*/
					break;
				default: 
					break;
			}

			/* Here we detach shared memory this commands will run everytime a child 				*/
			sem_close(my_sem);
			sem_unlink(SEM_NAME);
			if ( (shm_error = shmdt(data)) == - 1){
				printf("Could not detach shared memory mothafakaaaaaaa.\n");
				exit(1);
			}
			else if (shm_error == 0 )printf("successfully detached shared memory.\n");
			exit(1);
		}
	}
}


/*
* This is our SIG_INT handler function when a ctrl-c signal comes
* we first detach the shared memory 
* then we shmctl() it
* and only after those 2 actions do we exit() our program 
* We have also put a printf() inside it for debugging reasons. We decided to leave it there for easter-eggish reasons.
*/
void sig_int(int signo){

	int shm_error;
	if ( (shm_error = shmdt(data)) == - 1){
		printf("Could not detach shared memory mothafakaaaaaaa.\n");
		exit(1);
	}
	printf("I sense ctrl-c in the air.\n");
	shmctl(shm_id, IPC_RMID, NULL);
	exit(1);
}
