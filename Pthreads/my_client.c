/* ----------------------------------------------------README----------------------------------------------------------
 * This is the code implementing the client part of the first project in the course "Operating Systems I"
 * in CEID of University of Patras.
 *
 * Some additional notes:
 * 	Just because everybody in ceid working on this project did it for GiveMeYourMoney Bank
 *	we felt really sorry about other banks. There are other banks,
 *	like "mothafakaaaaaa bank", you see these guys are new to the bank industry
 *	and have no realiable banking transaction system. They deserve one. They deserve ours. ;)
 *
 *
 * Special Thanks :
 *		To every artist passed from my Amarok media player while coding.
 *
 *		To mr. Larry Page and mr. Sergey Brin for inventing Google.
 *		To Peter, because his goatee was an inspiration for us.
 *		And of course to the wonderful YOU who will be correcting this code; We hope you have fun.
 *
 * Authors		: .ios			      		bibo
 * 			  Kotsogiannis - Teftsoglou Ioannis	Kalargaris Charalambos; AM 3961,3889 respectively
 * Latest revision	: Nov 20 2009
 * 
 * Compilation: Just run
 *
 *	 $gcc -o server my_server.c functions.c -Wall -lrt &&gcc -o client my_client.c functions.c -Wall -lrt
 *
 * -------------------------------------------------------------------------------------------------------------------/

************************************************** Use Case Description **********************************************


Create account: 
	User enters his name and surname, then the system	
	checks if name and surname already exist, and if not creates a new account otherwise it informs the user.

Deposit:
	User inputs either his unique account id or name and surname,
	then the system searches the desired account. 
	If the account exists the system adds to its balance the deposited amount.
	If the account does not exists system quits.
	Whatever the system does it informs the client about the course of the action (success or not).

Withdraw:
	Like Deposit, with only one difference, before the withdrawal
	 system checks if balance is enough.
	Again this use case informs the client about the course of the action
	eg. account existed or not, balance was enough or not.

Update Book:
	User inputs either his unique account id or name and surname,
	then the system searches the desired account. 
	If account exists then the server sends a copy of the desired account data to the client,
	then the client simply handles this data and prints out a "booklet".
	If account does not exist, then the server simply informs the client about it.

*********************************************************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"

/******************** Main Program ********************/
int main(void){
	int client_sd, len, choice, trans_type, acc_exists, allow_with;
	struct sockaddr sa;

	/*
	* Get a socket to work with. This socket will be
	* in the UNIX domain, and will be a stream socket.
	*/
	if ((client_sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	perror("client: socket" );
	exit(1);
	}

	/*
	* Create the address we will be connecting to.
	*/
	sa.sa_family = AF_UNIX;
	strcpy(sa.sa_data, SOCK_NAME);

	/*
	* Try to connect to the address. For this to succeed,
	* the server must already have bound this address,
	* and must have issued a listen() request.
	*
	* The third argument indicates the "length" of the
	* structure, not just the length of the socket name.
	*/
	
	len = sizeof(sa.sa_family) + strlen(sa.sa_data);

	if (connect(client_sd, &sa, len) < 0) {
		perror("client: connect" );
		exit(1);
	}


	/* Here the user chooses what transaction he wants to make */
	trans_type = user_interface();

	/*
	* Here we send to server the type of transaction
	*/
	write(client_sd, &trans_type, sizeof(int));
	
	account tmp;
	account * tmp2;
	tmp2 = malloc(sizeof(account)	);
	
	switch(trans_type){

		case 1 :
			/* Here client enters his name for account creation and checks if name has 15 or less chars	*/			
			tmp = enter_name(tmp);
			
			/*
			* Here we send to server the data needed for the transaction
			*/
			if (write(client_sd, &tmp, sizeof(account)) == -1){
				printf("write error; ln 133\n");
				exit(1);
			}

			/* Now we read from the server wether or not the transaction was completed successfully		*/
			read(client_sd, &acc_exists, sizeof(int));
			
			/* Now we inform the client if his account creation was successful or not			*/
			if (acc_exists){
				printf("Could not create an account, there is already an account with the same name.\n");
				exit(1);
			}
			
			/* Verification message										*/
			printf("Successfully created account.\n");

			/* Some farewell message from the system							*/
			printf("Thank you for using mothafakaaaaaa bank; come visit again, your money is always welcome here.\n");
			break;
	
		case 2 :
			/*
			* Here we let the user chose how will he access his account
			*/
			choice = entry_choice();
			
			/*
			* We send to server what the user chose, so the server knows how to treat the next write
			*/			
			write(client_sd, &choice, sizeof(int));
			
			if ( choice == 1 ){
				printf("Enter unique id of your account: ");
				scanf("%d", &tmp.u_id);
			}
				
			if ( choice == 2 ){
				printf("Enter your name and surname: ");
				scanf("%s %s", tmp.firstname, tmp.surname);
			}
			
			printf("Enter amount to deposit: "		);
			scanf("%d", &tmp.balance);

			/* Here we check if the user has entered a positive value of money.		*/
			while(tmp.balance<0){
				printf("Please enter valid amount of money.\n");
				scanf("%d", &tmp.balance);
			}			
			
			printf("Completing Transaction...\n"		);

			/* Here we send the neeeded data to the server */
			write(client_sd, &tmp, sizeof(account)			);

			/* Here we check if the account we want to access exists or not */

			read(client_sd, &acc_exists, sizeof(int));
			if (!acc_exists){
				printf("Account does not exist; transaction cancelled. Will now exit\n");
				exit(1);
			}
			printf("Transaction successfully completed.\n");

			/* Some farewell message from the system					*/
			printf("Thank you for using mothafakaaaaaa bank; come visit again, your money is always welcome here.\n");
			break;
			
		case 3 :

			/*
			* Here we let the user chose how will he access his account
			*/

			choice = entry_choice();
			
			/*
			* We send to server what the user chose, so the server knows how to treat the next write
			*/			
			write(client_sd, &choice, sizeof(int));
			
			if ( choice == 1 ){
				printf("Enter unique id of your account: ");
				scanf("%d", &tmp.u_id);
			}
				
			if ( choice == 2 ){
				printf("Enter your name and surname: ");
				scanf("%s %s", tmp.firstname, tmp.surname);
			}
			
			printf("Enter amount to withdraw: "		);
			scanf("%d", &tmp.balance);
			
			/* Here we check if the user has entered a positive value of money.		*/
			while(tmp.balance<0){
				printf("Please enter valid amount of money.\n");
				scanf("%d", &tmp.balance);
			}
			
			printf("Completing Transaction...\n"		);

			/* Here we send the neeeded data to the server */
			write(client_sd, &tmp, sizeof(account)			);

			/* Here we check if the account we want to access exists or not */
			read(client_sd, &acc_exists, sizeof(int));
			if (!acc_exists){
				printf("Account does not exist; transaction cancelled. Will now exit\n");
				exit(1);
			}
			
			/* Here we check if the transaction is valid or not, check if balance is enough 	*/
			read(client_sd, &allow_with, sizeof(int));
			if (!allow_with){
				printf("The amount you asked is more than your balance. Transaction cancelled; will now exit.\n");
				exit(1);
			}
			printf("Transaction successfully completed.\n");

			/* Some farewell message from the system					*/
			printf("Thank you for using mothafakaaaaaa bank; come visit again, your money is always welcome here.\n");
			break;
			
		case 4 :
			/*
			* Here we let the user chose how will he access his account
			*/
			choice = entry_choice();
			
			/*
			* We send to server what the user chose, so the server knows how to treat the next write
			*/			
			write(client_sd, &choice, sizeof(int));
			
			if ( choice == 1 ){
				printf("Enter unique id of your account: ");
				scanf("%d", &tmp.u_id);
			}
				
			if ( choice == 2 ){
				printf("Enter your name and surname: ");
				scanf("%s %s", tmp.firstname, tmp.surname);
			}

			/* Here we send to the server the needed information so he can find our account */
			write(client_sd, &tmp, sizeof(account));

			/* Here we check if the account we want to access exists or not */
			read(client_sd, &acc_exists, sizeof(int));
			if (!acc_exists){
				printf("Account does not exist; transaction cancelled. Will now exit\n");
				exit(1);
			}
			
			/* If account does exist, then we just read it					*/
			read(client_sd, tmp2, sizeof(account));

			/* And then we print it on the screen						*/
			printdetails_client(tmp2);

			/* Some farewell message from the system					*/
			printf("Thank you for using mothafakaaaaaa bank; come visit again, your money is always welcome here.\n");

			break;
			
		case 5 :
			printf("Exit\n");
			break;
		default :
			printf("Unknown command; will now exit.\n");
			break;
	}		
	/*
	* We can simply use close() to terminate the
	* connection, since we're done with both sides.
	*/
	close(client_sd);
	exit(0);
}
