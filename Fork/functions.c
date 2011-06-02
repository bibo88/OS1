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

#include "defs.h"

/************************ User Defined Functions ************************/

/************* We create sig_chld to avoid zombie processes *************/

void sig_chld( int signo ){
	   pid_t pid;
	   int stat;

	   while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 ) {
			  printf( "Child %d terminated.\n", pid );
	   }
}

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

void printdetails(account * this){
/* Print the details of the account for checkign			*/
	int i;	
	printf("[FIRSTNAME]:%s\n",this->firstname);
	printf("[LASTNAME]:%s\n", this->surname);
	printf("[BALANCE]:%d\n", this->balance);
	printf("[UNIQUE ID]:%d\n", this->u_id);
	printf("[HISTORY]\n[");
	

	for(i=0;i<4;i++)
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
			/* If name and surname are the same then we found our unique entry.	*/
			printf("Account found.\n");
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
