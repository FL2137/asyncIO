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

    void print() {
        init_pair(1, COLOR_BLACK, COLOR_GREEN);
        init_pair(2, COLOR_BLACK, COLOR_BLUE);

        attron(COLOR_PAIR(1));
        printw("%d",socket);
        attroff(COLOR_PAIR(1));
        
        attron(COLOR_PAIR(2));
        printw(protocol.c_str());
        attroff(COLOR_PAIR(2));


        attron(COLOR_PAIR(1));
        printw(address.c_str());
        attroff(COLOR_PAIR(1));
        
        attron(COLOR_PAIR(2));
        printw("%d",port);
        attroff(COLOR_PAIR(2));
    }
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
        cbreak();
        noecho();
        clear();
        if(has_colors()){
            start_color();
            draw_board();

        }
        getch();
        endwin();
    }

    void draw_board() {
        //draw the headline
        draw_header();
        Record r1;
        r1.socket = 1;
        r1.protocol = "IPv4";
        r1.address = "192.168.1.109";
        r1.port = 3000;
        r1.connection = 4;
        r1.print();
    }

    void draw_header() {
        init_pair(1, COLOR_BLACK, COLOR_GREEN);
        init_pair(2, COLOR_BLACK, COLOR_BLUE);

        attron(COLOR_PAIR(1));
        printw("SOCKET");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2));
        printw("PROTOCOL");
        attroff(COLOR_PAIR(2));
        
        attron(COLOR_PAIR(1));
        printw("PEER ADDRESS");
        attroff(COLOR_PAIR(1));
        
        attron(COLOR_PAIR(2));
        printw("PEER PORT");
        attroff(COLOR_PAIR(2));
        
        attron(COLOR_PAIR(1));
        printw("CONNECTION");
        attroff(COLOR_PAIR(1));

        printw("\n");
    }

    //  || socket | protocol | peer address | peer port | connection ||



public:

private:
    std::vector<Record> records;
};
