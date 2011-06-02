/* ----------------------------------------------------README----------------------------------------------------
 * This is the code implementing the defintions needed for the first project in the course "Operating Systems I"
 * in CEID of University of Patras.
 *
 * Some additional notes & Special Thanks :
 *		See "my_client.c" file for these stuff. ;)
 *
 *
 * Authors		: .ios			      		bibo
 * 			  Kotsogiannis - Teftsoglou Ioannis	Kalargaris Charalambos
 * AM's			: 3961					3889
 * Latest revision	: Dec 7 2009
 * --------------------------------------------------------------------------------------------------------------
 */
 
/* Number of maximum accounts; this one can be changed at will		*/
#define MAX_ACCOUNTS 50
/* address to connect: we need a unique one so we used our AM's		*/
#define SOCK_NAME "39613929"

/* Size of shared memory, we need max_accounts*sizeof(account) for the accounts	*
* plus 1 sizeof(integer) to store the account counter for the server		*
*										*/
#define MEM_SIZE  MAX_ACCOUNTS*sizeof(account) + sizeof(int)

/* Here we define the maximum amount of simultaneously connected clients to our server
*  This variable defines how many pthreads can be active at any give moment	*/
#define MAX_CLIENTS 30

typedef struct account {
	int balance;
	char firstname[15];
	char surname[15];
	int u_id;
	int transaction[4];
} account;


/*
* Here we declare some variables globally for all functions to have access on them.
* data: 	is the beginning of our "shared" memory,
*		all threads must have access.
*
* max_id: 	is a variable in which we store 
*		the number of user created accounts in our system
*
* no_clients: 	is the number of the simultaneously connected 
*		clients into our system and the simultaneously created threads
* 		
*/

char * data;
int * max_id;
int no_clients;

/* Here are the declarations of user defined functions				*
*  Comments about their functionality can be found in functions.c		*/
int user_interface(void);

int entry_choice(void);

void printdetails(account * account);
void printdetails_client(account * account);

int id_exists(account * rcv_tmp, int * max_id);
int name_exists(account * rcv_tmp, account * rcv_tmp_store, int * max_id, char * data);

account enter_name(account tmp);

int allow_withdraw(account * rcv_tmp, account * rcv_tmp_store);

void sig_int(int signo);

void * main_callback(void * void_ns);
