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
#include <sys/un.h> 	/* for Unix domain sockets 	*/
#include <errno.h> 	/* for the EINTR constant 	*/
#include <sys/wait.h> 	/* for the waitpid() system call*/
#include <fcntl.h>
#include <pthread.h>	/* for the POSIX threads	*/

#include "defs.h"

int sd;
int main()
{	
	/********************************* Initilization Section *********************************/
	/*
	* Variables for the program 
	*/	
	int rc, len;
	int  ns;

	/* Variable for thread creation					*/
	pthread_t threads[MAX_CLIENTS];

	/* Allocate memory for all accounts that the system can handle	*
	 * MEM_SIZE is defined in "defs.h"				*/
	data = malloc(MEM_SIZE);

	/* Here we set max_id to the beginning of our "shared" memory	* 
	 * we also set it to zero.					*/
	max_id = (int *)data;
	*max_id = 0;
	
	/* Here we set no_clients to zero, since no clients are connected yet to our server */
	no_clients = 0;

	/*
	* Variables for sockets
	*/
	socklen_t fromlen;
	struct sockaddr sa, fsa;
	
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
	
	/******************************** Main Program Section *******************************/
		

	while (1){
		/*
		* Accepting new connections; check if accept succeeded.	
		*/
		printf("Server awaits for client to connect...\n");

		if ((ns = accept(sd, &fsa, &fromlen)) < 0) {
			perror("server: accept");
			exit(1);
		}
		
		printf("Client connected to server.\n");
		
		/* Thread creation and calling main_callback().	 		*
		 * We also check if pthread_create() failed			*
		 * if pthread_create() succeeds then we increase by one the 	*
		 * no_clients counter.						*
		 * We also pass as a parameter the ns sd returned from accept() */
		if ((rc = pthread_create(&threads[no_clients], NULL, main_callback, (void *) ns ))){
		    printf("Thread creation failed: %d\n", rc);
		} else no_clients++;
	}
}

/*
* This is our SIG_INT handler function when a ctrl-c signal comes
* we simply close the socket descriptor we had opened earlier
*/
void sig_int(int signo){
	printf("I sense ctrl-c in the air.\n");
	close(sd);
	exit(1);
}
