#include <string>
#include <ncurses.h>
#include <vector>
#include <fstream>

struct vec2 {
    int x, y;  
};

void show_list(WINDOW* t_win, std::vector<std::string>& t_items, std::vector<double>& t_prices) {
    box(t_win, 0, 0);
    for(size_t i = 0; i < t_items.size(); ++i) {
        mvwprintw(t_win, i + 1, 2, "[%ld] %s, %.2lf CZK", i, 
            t_items[i].c_str(), t_prices[i]);
    }
}

bool list_show_input(WINDOW* t_win, std::vector<std::string>& t_items, std::vector<double>& t_prices) {
    wclear(t_win);
    box(t_win, 0, 0);
    static int l_key;
    static long unsigned int l_highlight;
    static bool l_leave;

    if(l_highlight > t_items.size() - 1) {
        l_highlight = t_items.size() - 1;
    }

    for(size_t i = 0; i < t_items.size(); ++i) {
        if(i == l_highlight) 
            wattron(t_win, A_REVERSE);
        mvwprintw(t_win, i + 1, 2, "[%ld] %s, %.2lf CZK", i, 
            t_items[i].c_str(), t_prices[i]);
        wattroff(t_win, A_REVERSE);
    }

    l_leave = false;
    l_key = wgetch(t_win);
    switch(l_key) {
        case 'j':
            if(l_highlight < t_items.size() - 1)
                l_highlight += 1;
            l_leave = false;
            break;
        case 'k':
            if(l_highlight > 0) 
                l_highlight -= 1;
            l_leave = false;
            break;
        case '\n':
            if(!t_items.empty())
                t_items.erase(t_items.cbegin() + l_highlight);
            l_leave = true;
            break;
        case 'e':
            l_leave = true;
            break;
        default: break;
    }

    wrefresh(t_win);
    return l_leave;
}

void input_item(WINDOW* t_win, std::vector<std::string>& t_items, std::vector<double>& t_prices, vec2& t_win_max) {
    int l_key = 0;
    std::string l_string;
    double l_price = 0;
    std::string l_price_str;
    static int l_times;
    l_times = 0;
    noecho();
    while(l_key != '\n' || l_times != 2) {
        wclear(t_win);
        box(t_win, 0, 0);
        show_list(t_win, t_items, t_prices);
        if(l_times == 0) 
            mvwprintw(t_win, t_win_max.y - 5, 2, "Please, input the name of your item!");
        else mvwprintw(t_win, t_win_max.y - 5, 2, "Please, input the price of your item!");
        if(l_times == 0)
            wattron(t_win, A_REVERSE);
        mvwprintw(t_win, t_win_max.y - 4, 2, "INPUT: %s", l_string.c_str());
        wattroff(t_win, A_REVERSE);

        if(l_times == 1)
            wattron(t_win, A_REVERSE);
        mvwprintw(t_win, t_win_max.y - 3, 2, "PRICE: %s", l_price_str.c_str());
        wattroff(t_win, A_REVERSE);

        if(l_key == KEY_BACKSPACE || l_key == KEY_DC || l_key == 127) {
            mvwprintw(t_win, t_win_max.y - 2, 2, 
                "KEY: <backspace>, VALUE: %d", l_key);
        } else if(l_key == '\n') {
            mvwprintw(t_win, t_win_max.y - 2, 2, 
                "KEY: <enter>, VALUE: %d", l_key);
        } else {
            mvwprintw(t_win, t_win_max.y - 2, 2, 
                "KEY: %c, VALUE: %d", l_key, l_key);
        }

        l_key = wgetch(t_win); 
        if(l_key == 'e') return;
        if(l_key != '\n') {
            if(l_key == KEY_BACKSPACE || l_key == KEY_DC || l_key == 127) {
                if(!l_string.empty() && l_times == 0) 
                    l_string.pop_back();
                else if(!l_price_str.empty() && l_times == 1) 
                    l_price_str.pop_back();
            } else {
                if(l_times == 0) { 
                    l_string.push_back(l_key);
                } else if(l_times == 1 && '0' <= l_key && l_key <= '9') {
                    l_price_str.push_back(l_key);
                }
            }
        } else {
            if(l_times == 0)
                l_times++;
            else if(!l_price_str.empty())
                l_times++;
        }
        wrefresh(t_win);
    }

    wclear(t_win);
    box(t_win, 0, 0);
    t_items.push_back(l_string);
    t_prices.push_back(std::stod(l_price_str));
}

void leave_now(std::vector<std::string>& t_items, std::vector<double>& t_prices) {
    std::ofstream w_file("data.txt");
    for(size_t i = 0; i < t_items.size(); ++i) {
        w_file 
            << t_items[i] << std::endl 
            << t_prices[i] << std::endl;
    }
    w_file.close();
    endwin();
    exit(0);
}

void show_total(WINDOW* t_win, std::vector<double>& t_prices, vec2& t_max) {
    double l_sum = 0;
    for(auto p : t_prices) {
        l_sum += p;
    }
    mvwprintw(t_win, t_max.y - 2, 2, "Total = %.2lf", l_sum);
}

int main() {
    std::vector<std::string> items;
    std::vector<double> prices;

    std::string l_string;
    std::ifstream l_file("data.txt");
    while(std::getline(l_file, l_string)) {
        items.push_back(l_string);
        std::getline(l_file, l_string);
        prices.push_back(std::stod(l_string));
    }
    l_file.close();

    int key = 0;
    initscr();
    cbreak(); 
    noecho(); 

    vec2 max;
    getmaxyx(stdscr, max.y, max.x);

    WINDOW* input_win = newwin((max.y/5)*2, max.x - 12, (max.y/5)*3, 5);
    keypad(input_win, true);
    vec2 max_input;
    getmaxyx(input_win, max_input.y, max_input.x);

    WINDOW* list_win = newwin((max.y/5)*3, max.x - 12, 0, 5);
    vec2 max_list;
    getmaxyx(list_win, max_list.y, max_list.x);

    std::string choices[3] =  {"Add Bill", "Remove Bill", "Exit"};
    int highlight = 0;

    box(input_win, 0, 0); 
    box(list_win, 0, 0);
    
    show_list(list_win, items, prices);
    wrefresh(input_win);
    wrefresh(list_win);
   
    while(true) {
        box(input_win, 0, 0); 
        box(list_win, 0, 0);

        if(key == '\n') {
            mvwprintw(input_win, max_input.y - 2, max_input.x - 25, "KEY: <enter>, VALUE: %d", key);
        } else {
            mvwprintw(input_win, max_input.y - 2, max_input.x - 25, "KEY: %c, VALUE: %d", key, key);
        }
        
        for(int i = 0; i < 3; ++i) {
            if(i == highlight) {
                wattron(input_win, A_REVERSE);
            }
            mvwprintw(input_win, i + 1, 2, "%s", choices[i].c_str());
            wattroff(input_win, A_REVERSE);
        }
        

        show_total(input_win, prices, max_input);
        key = wgetch(input_win);

        int char_temp = 0;
        std::string input_temp;
        switch(key) {
            case 'j': 
                if(highlight < 2) highlight += 1;
                break;
            case 'k':
                if(highlight > 0) highlight -= 1;
                break;
            case '\n':
                switch(highlight) {
                    case 2: // exit
                        leave_now(items, prices);
                        break;
                    case 0: // add bill
                        input_item(list_win, items, prices, max_list);
                        wclear(list_win);
                        break;
                    case 1: // remove bill
                        while(1) {
                            if(list_show_input(list_win, items, prices)) {
                                break;
                            }
                        }
                        wclear(list_win);
                        break;
                    default: break;
                }
                break;
            default: break;
        }
        
        show_list(list_win, items, prices);

        refresh();
        wrefresh(input_win);
        wclear(input_win);
        wrefresh(list_win);
        wclear(list_win);
        (void) char_temp;
    }

    
    endwin();
    return 0;
}

