#include <string.h>

#include "helpers.h"

#define BUF_SIZE 4100

int find_delimiter(char *buf, int count)
{
    int i;
    int start_rev = 0;
    for (i = 0; i < count; ++i)
    {
        if (buf[i] == ' ')
        {
            return i;
        }
    }
    return count;
}

void get_rev_buf(char *buf, char* rev_buf, int pos)
{   int i;
    for (i = 0; i < pos; ++i)
    {
        rev_buf[i] = buf[pos - i - 1];
    }
}

int move(char *buf, int to_move, int count)
{   
    memmove(buf, buf + to_move, count - to_move);
    return count - to_move;
}

int main(int argc, char const *argv[])
{   
    int i;
    int cnt = 0;
    int size = 0;
    int word_cnt;
    char buf[BUF_SIZE];
    char rev_buf[BUF_SIZE];
    
    while ((cnt = read_until(STDIN_FILENO, buf + size, BUF_SIZE - size, ' ')) > 0)
    {   
        word_cnt = cnt + size;
        while (1)
        {
            int pos_delimiter = find_delimiter(buf, word_cnt);
            if (pos_delimiter == word_cnt) {
                size = word_cnt;
                break;
            }
            get_rev_buf(buf, rev_buf, pos_delimiter);
            int to_write = pos_delimiter;
            rev_buf[to_write++] = ' ';
            ssize_t writed = write_(STDOUT_FILENO, rev_buf, to_write);
            
            if (writed < 0)
            {
                exit(EXIT_FAILURE);
            }
            word_cnt = move(buf, to_write, word_cnt);
        }
    }
    if (cnt < 0)
    {
        exit(EXIT_FAILURE);
    }

    get_rev_buf(buf, rev_buf, word_cnt);
    ssize_t writed = write_(STDOUT_FILENO, rev_buf, word_cnt);
    if (writed < 0)
    {
        exit(EXIT_FAILURE);
    }
        
    return 0;
}