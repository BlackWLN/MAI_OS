#include <stdlib.h>
#include <unistd.h> //здесь read, write, отсюла и STDIN/ERR/OUT
#include <ctype.h> 
#include <string.h>
#include <stdio.h>


void process_line(char *line) {
    int only_spaces = 1;
    for (char *p = line; *p; p++) {
        if (!isspace((unsigned char)*p)) {
            only_spaces = 0;
            break;
        }
    }
    if (only_spaces) {
        return;
    }
    int sum = 0;
    int invalid_input = 0;
    char *p = line;

    while (*p) {
        // Пропускаем пробелы
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        // Обработка знака
        int is_neg = 1;
        if (*p == '-') { 
            is_neg = -1; 
            p++; 
        }

        // Проверка, что после знака идёт цифра
        if (!isdigit((unsigned char)*p)) {
            invalid_input = 1;
            break;
        }

        // Чтение числа
        int num = 0;
        while (*p && isdigit((unsigned char)*p)) {
            num = num * 10 + (*p - '0');
            p++;
        }

        // После числа должны быть пробелы или конец строки
        if (*p && !isspace((unsigned char)*p)) {
            invalid_input = 1;
            break;
        }

        sum += num * is_neg;

        // Пропускаем оставшиеся непробельные символы
        while (*p && !isspace((unsigned char)*p)) p++;
    }

    // Вывод результата
    if (invalid_input) {
        const char msg[] = "error: invalid input\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
    } else {
        char out[64];
        int len = snprintf(out, sizeof(out), "%d\n", sum);
        write(STDOUT_FILENO, out, len);
    }
}

int main() {
    char buf[4096];
    ssize_t n;
    size_t pos = 0;

    while ((n = read(STDIN_FILENO, buf + pos, sizeof(buf) - pos - 1)) > 0) {
        pos += n;
        buf[pos] = '\0';

        char *cur_line = buf;
        char *tmp_line;

        // Обработка полных строк (закончившихся \n)
        while ((tmp_line = strchr(cur_line, '\n'))) {
            *tmp_line = '\0';
            process_line(cur_line);
            cur_line = tmp_line + 1;
        }

        // Обработка последней неполной строки (если есть)
        if (*cur_line) {
            process_line(cur_line);
        }

        pos = 0;
    }

    return 0;
}