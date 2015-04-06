//
//  ipbx_queues.c
//  ipbx_core
//
//  Created by Guilherme Bessa on 10/02/12.
//  Copyright (c) 2012 IPsafe. All rights reserved.
//




#include <libpq-fe.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>


static PGconn *conn = NULL;

static void read_config();
static int pgsql_connect();

char db_connect_string[1024];

int SHOW_SQL = 1;

void my_debug(char *msg) {
    FILE *file;
    file = fopen("/var/log/ipbx/queues.log","a+");
    fprintf(file,"%s\n", msg);
    fclose(file);
}


/* Function declarations */

int generate_fila();


main() {
        
    read_config();
    
    if (!pgsql_connect())
		return 0;
    
    generate_fila();
    
	
	PQfinish(conn);
    
}



/* Functions */


//int generate_registry(){
int generate_fila()
{
    
    PGresult* result = NULL;
    PGresult* result_members = NULL;

	char sqlcmd[2048] = "";
	int num_rows, i, j, k, num_rows_members;
	char buff[2048] ="";
    
    char *fieldValue = NULL;
    
    //Get Queue name and parameters.
   
    sprintf(sqlcmd,"SELECT name, musiconhold, announce, context, timeout, "
                 "(CASE WHEN monitor_join is TRUE THEN 'mixmonitor' END) as \"monitor-type\","
                 "monitor_format as \"monitor-format\", "
                 "queue_youarenext as \"queue-youarenext\","
                 "queue_thereare as \"queue-thereare\", "
                 "queue_callswaiting as \"queue-callswaiting\", "
                 "queue_holdtime as \"queue-holdtime\", "
                 "queue_minutes as \"queue-minutes\", "
                 "queue_seconds as \"queue-seconds\", "
                 "queue_lessthan as \";queue-lessthan\", "
                 "queue_thankyou as \"queue-thankyou\", "
                 "queue_reporthold as \"queue-reporthold\", "
                 "announce_frequency as \"announce-frequency\", "
                 "announce_round_seconds as \"announce-round-seconds\", "
                 "announce_holdtime as \"announce-holdtime\", "
                 "retry, wrapuptime, maxlen, servicelevel, strategy, joinempty as \";joinempty\", leavewhenempty as \";leavewhenempty\", eventmemberstatus, eventwhencalled, reportholdtime, memberdelay, weight, timeoutrestart, setinterfacevar, ringinuse FROM queues;");


  
    if (SHOW_SQL)
    {
        sprintf(buff,"SQL: %s\n",sqlcmd);
        my_debug(buff);
    }
    
    
    if (!(result = PQexec(conn, sqlcmd))) {
		my_debug("Errorrr");
        PQclear(result);
		return -1;
	}
    else // query succeeded, process any data returned by it
	{
        if ((num_rows = PQntuples(result)) < 1) {
            PQclear(result);
			return -1;
		}
        
		for (i = 0; i < num_rows; i++) 
        {
            sprintf(buff, "");
            sprintf(buff, "[%s]\n", PQgetvalue(result,i,0));
            
            printf(buff);
            my_debug(buff);
            
            int numFields = PQnfields(result);
            for (j = 1; j < numFields; j++)
            {  
                    sprintf(buff, "%s = %s\n", PQfname(result, j), PQgetvalue(result,i,j));
                    printf(buff);
                    my_debug(buff);
            }
            
            sprintf(buff, "Set(MONITOR_FILENAME=${UNIQUEID}.wav) \n");
            printf(buff);
            //sprintf(buff, "Set(MONITOR_EXEC=mv /var/spool/asterisk/monitor/^{MONITOR_FILENAME} /mnt/disco1/gravacoes/^{MONITOR_FILENAME}) \n");
            //printf(buff);
            //sprintf(buff, "Set(MONITOR_EXEC=/usr/local/ipbx/ipbx_central_gravacao /mnt/disco1/gravacoes {MONITOR_FILENAME} 1 1 1 1 ) \n");
            //printf(buff);
            
            //Get Queue Members
            sprintf(sqlcmd,"SELECT interface FROM queue_members INNER JOIN queues ON queues.id = queue_members.queues_id WHERE name = '%s' ORDER by penalty;",  PQgetvalue(result,i,0));

            if (SHOW_SQL)
            {
                sprintf(buff,"SQL: %s\n",sqlcmd);
                my_debug(buff);
            }
            
            if (!(result_members = PQexec(conn, sqlcmd))) {
                my_debug("Error");
                PQclear(result_members);
                return -1;
            }
            else // query succeeded, process any data returned by it
            {
                if ((num_rows_members = PQntuples(result_members)) < 1) {
                    PQclear(result_members);
                    return -1;
                }
                
                for (k = 0; k < num_rows_members; k++) 
                {
                    sprintf(buff, "member => %s\n", PQgetvalue(result_members,k,0));
                    
                    printf(buff);
                    my_debug(buff);
                }
            }

        }
    }

        
        
    PQclear(result);
    
    return 0;
	
}

void read_config(void)
{
    FILE	*file;
    char var[200], val[200];
    
    char dbhost[40], dbname[20], dbuser[20], dbpass[20];
    int dbport;
    
    file = fopen("/usr/local/ipbx/ipbx.conf", "a+");
    
    /* Default values */
    strcpy(dbhost, "localhost");
    strcpy(dbname, "ipbx");
    strcpy(dbuser, "ipbx");
    strcpy(dbpass, "ipbx");
    dbport = 5432;
    
    /* Read values from conf file */   
    
    while (fscanf(file, "%s = %s", var, val) != EOF) {
        
        if (!strcmp(var, "hostname")) {
            strcpy(dbhost, val);        
        } else {
            if (!strcmp(var, "dbname")) {
                strcpy(dbname, val);        
            } else {
                if (!strcmp(var, "user")) {
                    strcpy(dbuser, val);        
                } else  {
                    if (!strcmp(var, "password")) {
                        strcpy(dbpass, val);        
                    } else {
                        if (!strcmp(var, "port")) {
                            //strcpy(dbport, val);        
                            dbport = atoi(val);
                        } else {
                        } } } } } 
        
    }
    
    fclose(file);
    
    sprintf(db_connect_string, "host=%s port=%i dbname=%s user=%s password=%s connect_timeout=8", dbhost, dbport, dbname, dbuser, dbpass);
    
    my_debug(db_connect_string);    
}


int pgsql_connect(void)
{
	//char aux[64] = "";
	//conecta no banco utilizando connInfo
    
	conn = PQconnectdb( db_connect_string );
    
	//      conn = PQconnectdb("host=localhost port=5432");
    
	if (conn && PQstatus(conn) == CONNECTION_OK) {
		my_debug("PostgreSQL IPBX: Successfully connected to database.\n");
		return 1;
	}
	else {
		char *retString;
		retString = PQerrorMessage(conn);
		my_debug(retString);
		free(retString);
		return 0;
	}
}

