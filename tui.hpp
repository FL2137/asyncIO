#pragma once

#include <ncurses.h>
#include <vector>
#include <string>
#define TERM_WIDTH 

struct Record {
    int socket;
    std::string protocol;
    std::string address;
    int port;
    int connection;
};


class TUI {

public:
    TUI() {
        init_TUI();
    }
    ~TUI() {
        endwin();
    }

private:
    void init_TUI() {
        initscr();


    }

    void draw_board() {
        //draw the headline

    }
    //  || socket | protocol | peer address | peer port | connection ||



public:

private:
    std::vector<Record> records;
};
