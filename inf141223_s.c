//
// Created by wiktor on 22.12.2019.
//
#define CONFIG "server.ini"
#include "inf141223_s.h"

int main(int argc, char* argv[])
{
    database_t *db = malloc(sizeof(database_t));
    int exit_code = 0;
    if((exit_code = setup(CONFIG, &db)) != 0)
    {
        exit(exit_code);
    }
    run(&db);
    free(db);
    exit(EXIT_SUCCESS);
}

int setup(char filename[], database_t *db)
{
    int config_file = open(filename, O_RDONLY);
    if(config_file < 0)
    {
        log_err("Cannot open config file.", db);
        return EXIT_FAILURE;
    }
    
    close(config_file);
}