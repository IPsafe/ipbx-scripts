/*
 *  ipbx_register.c
 *
 *  Copyright (C) 2010 Guilherme Bessa Rezende
 *
 *  Author: Guilherme Rezende <guilhermebr@gmail.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


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
    file = fopen("/var/log/ipbx/register.log","a+");
    fprintf(file,"%s\n", msg);
    fclose(file);
}


/* Function declarations */

int generate_registry(char *prov_type);


main(int argc, char *argv[]) {
    
    my_debug(argv[1]);
        
    read_config();
    
    if (!pgsql_connect())
        return 0;
    
    generate_registry(argv[1]);
    
    
    PQfinish(conn);

}



/* Functions */


//int generate_registry(){
int generate_registry(char *prov_type)
{
  
    PGresult* result = NULL;
    char sqlcmd[2048] = "";
    int num_rows, i;
    char buff[2048] ="";
        
    sprintf(sqlcmd,"SELECT devices.defaultuser, devices.secret, devices.host, devices.port, devices.regexten FROM public.devices WHERE devices.device_type = 'provider' AND register IS TRUE;");
  
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
            sprintf(buff, "register => %s:%s@%s", PQgetvalue(result,i,0), PQgetvalue(result,i,1), PQgetvalue(result,i,2));

            //port
            if ((PQgetvalue(result,i,3)) && (strlen(PQgetvalue(result,i,3))) )
            {
                sprintf(buff, "%s:%s", buff, PQgetvalue(result,i,3));
            } 
            
            //extension
            if ((PQgetvalue(result,i,4)) && (strlen(PQgetvalue(result,i,4)) > 0) ){
                sprintf(buff, "%s/%s", buff, PQgetvalue(result,i,4));
            } 
          
            sprintf(buff, "%s\n", buff);    
            printf(buff);
            my_debug(buff);
        }
    }
         
    PQclear(result);

    return 0;
    
}

void read_config(void)
{
    FILE    *file;
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

