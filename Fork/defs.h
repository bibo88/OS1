/* ----------------------------------------------------README----------------------------------------------------
 * This is the code implementing the defintions needed for the first project in the course "Operating Systems I"
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



#define MAX_ACCOUNTS 50 		/* Number of maximum accounts; this one can be changed at will		*/
#define SOCK_NAME "/tmp/39613929" 	/* address to connect: we need a unique one so we used our AM's		*/
#define SHM_KEY "/dev/null"
#define SHM_ID 0
#define SEM_NAME "trompominaras_3961"	/* Semaphore name: we needed a unique one so trompominaras made it ;)	*/

/* Size of shared memory, we need max_accounts*sizeof(account) for the accounts	*
* plus 1 sizeof(integer) to store the account counter for the server		*
*										*/
#define SHM_SIZE  MAX_ACCOUNTS*sizeof(account) + sizeof(int)

typedef struct account {
	int balance;
	char firstname[15];
	char surname[15];
	int u_id;
	int transaction[4];
} account;


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

void sig_chld( int signo );
void sig_int(int signo);





