#include "files.hpp"

#include "../logs/logs.hpp"

#include <filesystem>
#include <fstream>
#include <string>

/*
    Create a new empty file.

    Tasks:
        1) Some verification.
            a. Verify that path contains a file name.
            b. Verify that another file at targeted path does not already exist.
        2) Proceed to the file creation.
            a. Create a new file.
            b. Verify the file was successfully created.

    Parameters (variable_name / type / description):
        - file_path / string / Targeted path containing new file name.

    Returns (type + description):
        A boolean confirming whether we created a new empty file or not.
*/
bool Files::create_new_empty_file
(
    const std::string &file_path
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const bool has_filename = std::filesystem::path(file_path).has_filename();

    if (!has_filename)
    {
        Logs::log("Warning: Empty file creation failed. Invalid path -> \"" + file_path + "\".");
        return false;
    }

    ///////// b. /////////
    const bool file_exists = std::filesystem::exists(file_path);

    if (file_exists)
    {
        Logs::log("Warning: Empty file creation failed -> \"" + file_path + "\" already exists.");
        return false;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    std::ofstream file(file_path);
    const bool file_opened = file.is_open();

    ///////// b. ////////
    if (file_opened)
    {
        file.close();
        Logs::log("New empty file \"" + file_path + "\" created.");
        return true;
    }

    Logs::log("Warning: Failed to create new empty file \"" + file_path + "\".");
    return false;
}



/*
    Write some data into a file.

    Tasks:
        1) Some verification.
            a. Verify that path contains a file name.
            b. Verify that file exists, if not, create a new empty file at desired path.
        2) Proceed to writing into file.
            a. Open file in append or overwrite mod.
            b. Verify that file is opened.
            c. Insert data into the file.

    Parameters (variable_name / type / description):
        - append    / bool   / Append the file or overwrite the file.
        - data      / string / Data to write into the file.
        - file_path / string / Path to the file to write into.

    Returns (type + description):
        A boolean confirming whether we successfully wrote into the file or not.
*/
bool Files::write_file
(
    const bool        &append,
    const std::string &data,
    const std::string &file_path
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    std::ofstream file;
    const bool has_filename = std::filesystem::path(file_path).has_filename();

    if (!has_filename)
    {
        Logs::log("Warning: File writing failed, path provided invalid -> \"" + file_path + "\".");
        return false;
    }

    ///////// b. /////////
    const bool file_exists(std::filesystem::exists(file_path));

    if (!file_exists)
    {
        const bool file_created = create_new_empty_file(file_path);

        if (!file_created)
        {
            Logs::log("Warning: File writing failed, failed to create \"" + file_path + "\".");
            return false;
        }
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    if (append)
        file.open(file_path, std::ios::out | std::ios::app);
    else file.open(file_path, std::ios::out | std::ios::trunc);

    ///////// b. /////////
    const bool is_opened = file.is_open();

    if (!is_opened)
    {
        Logs::log("Warning: File writing failed, couldn't open file for writing -> \"" + file_path + "\".");
        return false;
    }

    ///////// c. /////////
    file << data;
    file.close();

    return true;
}
