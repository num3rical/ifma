#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <cstring>
#include <stdio.h>
#include <filesystem>

#include "deps/sqlite3.h"

std::string exec(const char* cmd) 
{
    char buffer[128];
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
        {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

void cmdHelp() 
{
    std::string helpString =
        "ifma allows you to easily record those commands you often forget, along with a description of what they do. You can search for and run previously saved commands when you need them.\n"
        "\n"
        "Usage:\n"
        "   ifma add <command> [<args>]         Add an entry for <command> with specified arguments.\n"
        "   ifma all                            Display all entries\n"
        "   ifma <keyword>                      Display entries containing <keyword>\n"
        "   ifma <command> [<keyword>]          Display entries for <command> containing <keyword>\n"
        "   ifma remove <id>                    Remove entry with specified id\n"
        "   ifma run <id>                       Execute the command with the specified id\n"
        "\n"
        "Examples:\n"
        "   ifma xrandr \"swap displays\"         -- Returns all entries for \"xrandr\" with description containing \"swap displays\"\n\n"
        "   ifma add pacman -Syu                -- Adds an entry for \"pacman\" with arguments \"-Syu\"\n\n"
        "   ifma add echo Hello there!          -- Adds an entry for \"echo\" with arguments \"Hello there!\"\n\n"
        "\n";

    std::cout << helpString << std::endl;
    return;
}

// queries db, does nothing with result
int queryDBExec(sqlite3* db, std::string sql)
{
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    return 0;
}

struct rowResult {
    std::string id;
    std::string command;
    std::string args;
    std::string description;
};

std::vector<rowResult> queryDB(sqlite3* db, std::string sql, const std::vector<std::string>& args)
{
    std::vector<rowResult> results;
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << sqlite3_errstr(rc) << std::endl;

        return results;
    }

    if (args.size() > 0)
    {
        for (size_t i = 0; i < args.size(); i++)
        {
            rc = sqlite3_bind_text(stmt, i + 1, args[i].c_str(), strlen(args[i].c_str()), nullptr);
        }
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char* id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* command = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* args = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        rowResult res = {
            std::string(id),
            std::string(command),
            std::string(args),
            std::string(description)
        };

        results.push_back(res);
    }

    sqlite3_clear_bindings(stmt);
    sqlite3_finalize(stmt);

    return results;
}

int initDB(sqlite3** db, std::string dir)
{
    std::string dbFile = dir + "/ifma.db";
    int conn = sqlite3_open(dbFile.c_str(), db);

    if (conn != SQLITE_OK)
    {
        std::cerr << sqlite3_errstr(conn) << std::endl;

        return -1;
    }

    return 0;
}

int setupFiles(std::string dir, std::error_code& err)
{
    std::filesystem::create_directory(dir + "/.ifma");
    return 0;
}

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::string setupDir = std::filesystem::path(std::getenv("HOME"));

    std::error_code err;
    if (!setupFiles(setupDir, err))
    {
        // TOOD: set up proper errors in setupFiles
    }

    sqlite3* db;

    if (initDB(&db, setupDir + "/.ifma") < 0)
    {
        std::cerr << "There was a problem initializing the database." << std::endl;
        return 0;
    }

    std::string sql =
        "CREATE TABLE IF NOT EXISTS ifma("
        "   id INTEGER PRIMARY KEY,"
        "   command TEXT,"
        "   args TEXT,"
        "   description TEXT"
        ");";

    queryDBExec(db, sql);
    
    if (argc < 2)
    {
        cmdHelp();
    }
    else
    {
        std::string inputCmd = args[1];
        std::string inputQuery;
        if (argc > 2) inputQuery = args[2];

        if (inputCmd == "add")
        {
            std::string cmd = args[2];
            std::string cmdArgs; 

            for (size_t i = 3; i < args.size(); i++)
            {
                cmdArgs += args[i] + " ";
            }
            
            if (cmdArgs.length() > 0)
            {
                while (std::isspace(cmdArgs[cmdArgs.size() - 1]))
                {
                    cmdArgs.erase(cmdArgs.size() - 1);
                }
            }

            std::string description;

            std::cout << "Enter description:" << std::endl;
            std::getline(std::cin, description);

            std::string sql = 
                "INSERT INTO ifma (command, args, description) VALUES (?, ?, ?);";

            std::vector<std::string> queryArgs = {cmd, cmdArgs, description};
            queryDB(db, sql, queryArgs);

        }
        else if (inputCmd == "remove")
        {
            std::string rowID = args[2];
            if (std::stoi(rowID) < 0 || rowID.length() < 1)
            {
                std::cout << "invalid row option" << std::endl;
            }
            else
            {
                std::string sql = "SELECT * FROM ifma WHERE id = ?";
                std::vector<std::string> queryArgs = {rowID};
                std::vector<rowResult> result = queryDB(db, sql, queryArgs);

                if (result.size() < 1)
                {
                    std::cout << "invalid row option" << std::endl;
                }
                else
                {
                    printf("\nRemoving entry:");
                    printf("\n[%s] %s\n", result[0].id.c_str(), result[0].description.c_str());
                    printf("    %s %s\n", result[0].command.c_str(), result[0].args.c_str());
                    printf("\n\n");

                    std::string choice;
                    std::cout << "Remove? (y/n): ";
                    std::getline(std::cin, choice);

                    if (choice == "y")
                    {
                        sql = "DELETE FROM ifma WHERE id = ?";
                        queryDB(db, sql, queryArgs);
                        std::cout << "Removed entry [" << rowID << "]" << std::endl;
                    }
                }
            }
        }
        else if (inputCmd == "run")
        {
            std::string rowID = args[2];
            if (std::stoi(rowID) < 0 || rowID.length() < 1)
            {
                std::cout << "invalid row option" << std::endl;
            }
            else
            {
                std::string sql = "SELECT * FROM ifma WHERE id = ?";
                std::vector<std::string> queryArgs = {rowID};
                std::vector<rowResult> result = queryDB(db, sql, queryArgs);

                if (result.size() < 1)
                {
                    std::cout << "invalid row option" << std::endl;
                }
                else
                {
                    printf("Running command: %s %s\n", result[0].command.c_str(), result[0].args.c_str());
                    std::string cmd = result[0].command + " " + result[0].args;
                    std::cout << exec(cmd.c_str()) << std::endl;
                }

            }
        }
        else if (inputCmd == "all")
        {
            std::string sql;
            std::vector<std::string> queryArgs;
            std::cout << "All entries" << std::endl;
            sql = "SELECT * FROM ifma";
            queryArgs = {inputCmd, inputCmd};
            
            std::vector<rowResult> results = queryDB(db, sql, queryArgs);
            for (auto& res : results)
            {
                printf("\n");
                printf("[%s] %s\n", res.id.c_str(), res.description.c_str());
                printf("    %s %s\n", res.command.c_str(), res.args.c_str());
            }
            printf("\n\n");
            
        }
        else
        {
            std::string sql;
            std::vector<std::string> queryArgs;
            if (inputQuery.length() < 1)
            {
                std::cout << "Searching by command name and description" << std::endl;
                sql = "SELECT * FROM ifma WHERE command = ? OR UPPER(description) LIKE UPPER('%' || ? || '%')";
                queryArgs = {inputCmd, inputCmd};
            }
            else
            {
                std::cout << "Searching by description" << std::endl;
                sql = "SELECT * FROM ifma WHERE command = ? AND UPPER(description) LIKE UPPER('%' || ? || '%')";
                queryArgs = {inputCmd, inputQuery};
            }

            std::vector<rowResult> results = queryDB(db, sql, queryArgs);
            for (auto& res : results)
            {
                printf("\n");
                printf("[%s] %s\n", res.id.c_str(), res.description.c_str());
                printf("    %s %s\n", res.command.c_str(), res.args.c_str());
            }
            printf("\n\n");
        }

    }

    sqlite3_close(db); 
    return 0;
}
