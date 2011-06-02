/* ----------------------------------------------------README---------------------------------------------------- 
 * This is the code implementing the functions of the first project in the course "Operating Systems I"
 * in CEID of University of Patras.
 *
 * Some additional notes & Special Thanks :
 *		See "my_client.c" file for these stuff. ;)
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
#include <sys/un.h> 		/* for Unix domain sockets	*/
#include <errno.h> 		/* for the EINTR constant 	*/
#include <sys/wait.h> 		/* for the waitpid() system call*/
#include <errno.h>
#include <pthread.h>

#include "defs.h"

/************************ User Defined Functions ************************/

/************************ Client Side Functions *************************/
/* user_interface() prints out a menu and returns the input of the user	*/
int user_interface(){
	int choice;
	
	printf("Epilekste synallagh patwntas ton antistoixo ari8mo:\n");
	printf("1)\t Dhmiourgia logariasmou\n");
	printf("2)\t Kata8esh\n");
	printf("3)\t Analhpsh\n");
	printf("4)\t Enhmerwsh Vivliariou\n");
	printf("5)\t Eksodos\n");

	scanf("%d", &choice);
	return choice;
}

/* printdetails_client() takes as input one struct account and prints it out on the screen*/
void printdetails_client(account * this){
	int i;	
	printf("[FIRSTNAME]:%s\n",this->firstname);
	printf("[LASTNAME]:%s\n", this->surname);
	printf("[BALANCE]:%d\n", this->balance);
	printf("[UNIQUE ID]:%d\n", this->u_id);
	printf("[HISTORY]\n");
	

	for(i=0;i<4;i++){
		if (this->transaction[i] > 0)
			printf("[Deposit\t");
		else if (this->transaction[i] <0)
			printf("[Withdrawal\t");
			else{
				printf("[No transaction\t\t]\n");
				continue;
			}
		printf("%d\t]\n", this->transaction[i] );
	}

}

/* entry_choice() prints out a (sub)menu and returns the input of the user	*/
int entry_choice(void){
	int control = 0;
	while ( (control != 1) && (control != 2) ){
		printf("you have chosen to access your mothfakaaaaa account - you stupid mofo\n");
		printf("[1]Access account via unique id\n");
		printf("[2]Access account via name and surname\n");
		scanf("%d", &control);
		if ((control != 1) && (control != 2))
			printf("Please enter a valid option\n");
	}
	return control;
}

/* enter_name() takes as input one struct account		*	
*  asks from user to add his name				*
*  and returns the struct updated with the input of the user	*/
account enter_name(account tmp){
	printf("you have chosen to create a new account to mothafakaaaaa bank - you stupid mofo\n");
	printf("Enter name and surname: ");
	scanf("%s %s", tmp.firstname, tmp.surname);
	while ( (strlen(tmp.firstname) > 15) || (strlen(tmp.surname) > 15 ) ){
		printf("Enter a valid name (has less than 15 chars)\n");
		printf("Enter name and surname: ");
		scanf("%s %s", tmp.firstname, tmp.surname);
				}
	return tmp;
}


/************************ Server Side Functions *************************/

/* Print the details of the account for checkign			*/
void printdetails(account * this){
	int i;	
	printf("[FIRSTNAME]:%s\n",this->firstname);
	printf("[LASTNAME]:%s\n", this->surname);
	printf("[BALANCE]:%d\n", this->balance);
	printf("[UNIQUE ID]:%d\n", this->u_id);
	printf("[HISTORY]\n[");
	

	for(i=0; i<4; i++)
		printf("%d ", this->transaction[i] );			
	printf("]\n");

}

/* The next 2 functions check wether an account exists or not		*/
/* Checks if account exists when searching via id			*/
int id_exists(account * rcv_tmp, int * max_id){
	
	int acc_exists = 0;
	if ((rcv_tmp->u_id) < (*max_id)){
		acc_exists = 1;
	}
	return acc_exists;
}

/* Checks if account exists when searching via name and surname		*/
int name_exists(account * rcv_tmp, account * shm_now_data, int * max_id, char * data){
	
	int acc_exists = 0;
	int i, test_name, test_surname;

	for(i=0; i< (*max_id); i++){
		shm_now_data = (account *)(data + sizeof(int) + i*sizeof(account));
		/*
		* We compare the current name and surname with all names and surnames already
		* stored in our shared memory.
		*/
		test_surname = strcmp(shm_now_data->surname, rcv_tmp->surname);
		test_name = strcmp(shm_now_data->firstname, rcv_tmp->firstname);
		
		if ( (acc_exists = ((test_surname == 0) && (test_name == 0))) ) {
			/* If name and surname are the same then we found our unique entry.	*	
			 * so we just break							*/
			break;	
		}				
	}
	return acc_exists;
}

/* allow_withdraw checks if balance is enough for a withdrawal and returns true/false		*/
int allow_withdraw(account * rcv_tmp, account * shm_now_data){
	int allow;

	if ( (allow = (shm_now_data->balance >= rcv_tmp->balance)))
		printf("Balance is enough; proceeding with withdraw normally.\n");
	else{
		printf("Not enough balance, transaction aborted; will now exit.\n");
	}
	return allow;
}


/* We declare a new mutex variable used in main_callback()			*/
pthread_mutex_t mofo_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Main Server function 							*
 * Takes as parameter the socket descriptor returned from accept() in main() 	*/
void * main_callback(void * void_ns){
	/* Variable initilization						*/
	int  i, control, test_name, test_surname, acc_exists, allow_with;
	int trans_type;
	
	/* Here we take the socket descriptor parameter from the main()		*
	 * and we cast it to integer before we use it in read() write()		*
	 * functions. That way the programmer makes the cast			*
	 * and not the compiler							*/	
	int ns;
	ns = (int) void_ns;
	
	/* Here we create two helper pointers to struct account			*
	 * We also clear them so we are sure we ain't got any junk data		*/ 
	account * rcv_tmp, * shm_now_data;
	
	rcv_tmp = malloc(sizeof(account));
	memset(rcv_tmp, 0, sizeof(account));

	shm_now_data = malloc(sizeof(account));
	memset(shm_now_data, 0, sizeof(account));
	
	/*
	* Here we receive the type of transaction needed to be resolved
	*/
	if (read(ns, &trans_type, sizeof(int)) == -1){
		printf("error read ln 175\n");
		exit(1);
	}
	
	switch(trans_type){
		/* Create account case				*/
		case 1: 
			
			/*
			* Here we receive the information sent by client.
			*/
			if (read(ns, rcv_tmp, sizeof(account)) == -1){
				printf("error read ln 194\n");
				exit(1);
			}
			/* max_id is a shared by all threads variable		*
			 * thus we have to protect it using mutexes		*/
			pthread_mutex_lock(&mofo_mutex);
			rcv_tmp->u_id = *max_id;
			
			/*
			* Here we check if user has already an account to our bank.
			* We simply check all entries and compare them with the new entry.
			*/
			/* name_exists() is not a thread safe function		*
			 * so this piece of code has to be protected too	*/
			acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);
			pthread_mutex_unlock(&mofo_mutex);

			/* We send to the client wether or not the account already exists	*/
			if (write(ns, &acc_exists, sizeof(int)) == -1){
			  printf("write error line 224\n");
			  exit(1);
			}
			
			/* 
			* Using this break instead of exit(0)
			* we will go to the end of the switch code
			* where the program ends as it should.
			*/
			if (acc_exists){
				printf("Already has an account, will now terminate.\n");
				break;
			}

			/*
			* We copy all new data to the shared memory. 
			* We save the new entry at the end of our shared memory.
			* To find the end of shared memory we simply check max_id variable.
			*/
			
			/* Set the pointer at the correct position in shared memory		*/
			/* Here we access our "shared" memory, so we have to protect		*/
			pthread_mutex_lock(&mofo_mutex);
			shm_now_data = (account *)(data + sizeof(int) + (*max_id)*sizeof(account));
			
			/* Store firstname							*/		
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
			pthread_mutex_unlock(&mofo_mutex);
	
			/* Print the details of the account for debugging reasons		*/					
			printdetails(shm_now_data);	

			/* Icrease max_id by one each time an account is created		*	
			 * max_id is a shared variable and has to be protected with mutex	*/
			pthread_mutex_lock(&mofo_mutex);				
			(*max_id)++;
			pthread_mutex_unlock(&mofo_mutex);					
			
			/* Create account case ends						*/
			printf("Use case create account ends\n");
			break;
		
		case 2:
			/* We read user's access account choice					*/
			if (read(ns, &control, sizeof(int)) == -1){
				printf("error read ln 272\n");
				exit(1);
			}

			/* We read the data user sent to the server				*/
			if (read(ns, rcv_tmp, sizeof(account)) == -1){
				printf("error read ln 194\n");
				exit(1);
			}
			
			if (control == 1){

				/*					
				* What we will do is check wether the account exists or not
				* and do the proper actions for each case
				*/

				/* We check if account exists... 				*
				 * id_exists() is not a thread safe function so we protect it	*/
				pthread_mutex_lock(&mofo_mutex);
				acc_exists = id_exists(rcv_tmp, max_id);
				pthread_mutex_unlock(&mofo_mutex);
				
				/* ...inform the client wether or not the account exists... 		*/
				write(ns, &acc_exists, sizeof(int));

				/* 
				* If the account does not exist we simply terminate this proccess.
				* Using break ensures us that memory will be detached.
				*/
				if (acc_exists == 0) {
					break;
				}

				/* Else we go to the desired account in our shared memory.		*
				 * since we access our "shared" memory we have to protect this		*
				 * piece of code							*/			
				pthread_mutex_lock(&mofo_mutex);
				shm_now_data = (account *)(data + sizeof(int) + (rcv_tmp->u_id)*sizeof(account));
				
				/* And we add to our balance the deposited amount			*/
				shm_now_data->balance = shm_now_data->balance + rcv_tmp->balance;

				/* Here we update the transaction history of the account		*/
				for (i = 3; i > 0; i--){
					shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
				}
				shm_now_data->transaction[0] = rcv_tmp->balance;  
				
				/* Critical code ends, so we unlock the mutex				*/
				pthread_mutex_unlock(&mofo_mutex);
				
				/* Print the details of the account for debugging reasons		*/				
				printdetails(shm_now_data);
				
				}
			
			if (control == 2) {
			
				/*					
				* What we will do is check wether the account exists or not
				* and do the proper actions for each case
				*/

				/* Here we check for existance of account 				*
				 * name_exists() is not a thread safe function so we protect it		*/
				pthread_mutex_lock(&mofo_mutex);
				acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);
				pthread_mutex_unlock(&mofo_mutex);
				
				/* ...inform the client wether or not the account exists... 		*/
				if (write(ns, &acc_exists, sizeof(int)) == -1){
				  printf("write error line 332\n");
				  exit(1);
				}
				
				/* 
				* If the account does not exist we simply terminate this proccess.
				* Using break ensures us that memory will be detached.
				*/
				if (acc_exists == 0) {
					break;
				}
				
				/* Critical code begins, so we have to protect it	*
				* with the use of mutex				*/
				pthread_mutex_lock(&mofo_mutex);
				for(i=0; i < (*max_id); i++){
				
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
				/* Then  we add to our balance the deposited amount.			*/

				shm_now_data->balance = shm_now_data->balance + rcv_tmp->balance;

				/* Here we update the transaction history of the account		*/
				for (i = 3; i > 0; i--){
					shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
				}
				shm_now_data->transaction[0] = rcv_tmp->balance;
				
				/* Critical code ends							*/
				pthread_mutex_unlock(&mofo_mutex);
				
				/* We print out the new account status 					*/
				printdetails(shm_now_data);
			}
			break;
		
		case 3: 
			/* We read user's access account choice					*/
			if (read(ns, &control, sizeof(int)) == -1){
				printf("error read ln 382\n");
				exit(1);
			}

			/* We read the data user sent to the server				*/
			if (read(ns, rcv_tmp, sizeof(account)) == -1){
				printf("error read ln 388\n");
				exit(1);
			}
			
			if (control == 1){

				/*					
				* What we will do is check wether the account exists or not
				* and do the proper actions for each case
				*/

				/* We check if account exists... 					*
				 * Here critical code begins so we have to protect it			*/
				pthread_mutex_lock(&mofo_mutex);
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
				shm_now_data = (account *)(data + sizeof(int) + (rcv_tmp->u_id)*sizeof(account));
				
				/* Here we check if balance is enough					*/
				allow_with = allow_withdraw(rcv_tmp, shm_now_data);

				/* We send back to client wether or not the balance is enough		*/
				write(ns, &allow_with, sizeof(int));

				/* Here we terminate the server if the balance is not enough		*/
				if (!allow_with){
					/* Since we break we have to unlock our mutex			*/
					pthread_mutex_unlock(&mofo_mutex);
					break;
				}

				/* And we add to our balance the deposited amount			*/
				shm_now_data->balance = shm_now_data->balance - rcv_tmp->balance;

				/* Here we update the transaction history of the account		*/
				for (i = 3; i > 0; i--){
					shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
				}
				shm_now_data->transaction[0] = -rcv_tmp->balance;  

				/* Critical code ends							*/
				pthread_mutex_unlock(&mofo_mutex);
				
				/* Print the details of the account for debugging reasons		*/				
				printdetails(shm_now_data);
				}
			
			if (control == 2) {
			
				/*					
				* What we will do is check wether the account exists or not
				* and do the proper actions for each case
				*/
				
				/* Here we check for existance of account 				*
				 * name_exists() is not a thread safe function				*/
				pthread_mutex_lock(&mofo_mutex);
				acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);
				pthread_mutex_unlock(&mofo_mutex);
				
				/* ...inform the client wether or not the account exists... 		*/
				if (write(ns, &acc_exists, sizeof(int)) == -1){
				  printf("write error line 443\n");
				  exit(1);
				}

				/* 
				* If the account does not exist we simply terminate this proccess.
				* Using break ensures us that memory will be detached.
				*/
				if (acc_exists == 0) {
					break;
				}

				/* Here critical code begins						*/
				pthread_mutex_lock(&mofo_mutex);
				
				for(i=0; i< (*max_id); i++){
					shm_now_data = (account *)(data + sizeof(int) + i*sizeof(account));
					/*
					* We compare the current name and surname with all names and surnames already
					* stored in our shared memory.
					*/
					test_surname = strcmp(shm_now_data->surname, rcv_tmp->surname);
					test_name = strcmp(shm_now_data->firstname, rcv_tmp->firstname);

					if ((test_surname == 0) && (test_name == 0) )  {
						
						/* If name and surname are the same then we found our unique entry.*/
						printf("Account found.\n");
						break;	
					}
					
				}
				
				/* Here we check if balance is enough for withdrawal			*/
				allow_with = allow_withdraw(rcv_tmp, shm_now_data);			

				/* We send back to client wether or not the balance is enough		*/
				write(ns, &allow_with, sizeof(int));
				
				/* Here we exit, if the balance is not enough				*/
				if (!allow_with){
				      	/* Since we break we have to unlock our mutex			*/
					pthread_mutex_unlock(&mofo_mutex);
				      	break;
				}
				
				/* And we add to our balance the deposited amount			*/
				shm_now_data->balance = shm_now_data->balance - rcv_tmp->balance;

				/* Here we update the transaction history of the account		*/
				for (i = 3; i > 0; i--){
					shm_now_data->transaction[i] = shm_now_data->transaction[i-1];
				}
				shm_now_data->transaction[0] = -rcv_tmp->balance;  

				/* Here critical code ends						*/
				pthread_mutex_unlock(&mofo_mutex);

				/* We print out the new account status 					*/
				printdetails(shm_now_data);

			}
			break;
		
		case 4: 
			
			/* We read user's access account choice						*/
			if (read(ns, &control, sizeof(int)) == -1){
				printf("error read ln 520\n");
				exit(1);
			}

			/* We read the data user sent to the server					*/
			if (read(ns, rcv_tmp, sizeof(account)) == -1){
				printf("error read ln 525\n");
				exit(1);
			}

			if (control==1){
				
				/* We check if account exists... 					*
				 * id_exists() is not a thrad safe function				*/
				pthread_mutex_lock(&mofo_mutex);
				acc_exists = id_exists(rcv_tmp, max_id);
				pthread_mutex_unlock(&mofo_mutex);
				
				/* ...inform the client wether or not the account exists... 		*/
				write(ns, &acc_exists, sizeof(int));

				/* 
				* If the account does not exist we simply terminate this proccess.
				* Using break ensures us that memory will be detached.
				*/
				if (acc_exists == 0) {
					break;
				}

				/* Else we go to the desired account in our shared memory.		*
				 * Critical code begins							*/
				pthread_mutex_lock(&mofo_mutex);
				shm_now_data = (account *)(data + sizeof(int) + (rcv_tmp->u_id)*sizeof(account));
				pthread_mutex_unlock(&mofo_mutex);
				
				/* and we send the data to client					*/
				write(ns, shm_now_data, sizeof(account));
				
			}
			if (control == 2 ){
				
				/* Here we check for existance of account 				*
				 * name_exists() is not a thread safe function				*/						
				pthread_mutex_lock(&mofo_mutex);
				acc_exists = name_exists(rcv_tmp, shm_now_data, max_id, data);
				pthread_mutex_unlock(&mofo_mutex);
				
				/* ...inform the client wether or not the account exists... 		*/
				if (write(ns, &acc_exists, sizeof(int)) == -1){
				  printf("write error line 535\n");
				  exit(1);
				}

				/* 
				* If the account does not exist we simply terminate this proccess.
				* Using break ensures us that memory will be detached.
				*/
				if (acc_exists == 0) {
					break;
				}

				/* Critical code begins							*/
				pthread_mutex_lock(&mofo_mutex);
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
				/* Critical code ends								*/
				pthread_mutex_unlock(&mofo_mutex);

				/* Now we have found the account and we simply send it to the client		*/
				write(ns, shm_now_data, sizeof(account));
				
			}			
			break;
		
		case 5: /* No need to write any code to handle "exit" functionality				
			    The code needed to be run for a correct "exit" lies right after the switch and will be
			    executed no matter what (well at most of the time).					*/
			break;
		default:
			break;
	}
	/* Here we have to end the thread that called this callback function;	*
	 * we also decrease by one the no_clients				*/
	no_clients--;
	close(ns);
	pthread_exit(NULL);
}
