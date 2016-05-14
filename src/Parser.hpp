#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include<string>

#include "Error.hpp"

const std::string SCENE_DIRECTORY = "res/scenes/";


class Parser {
public:
    static std::string get_scene_file_from_command_line(int argc, char *argv[]);

private:

};

#endif
