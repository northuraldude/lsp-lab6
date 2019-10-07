/**
 * Дисциплина: Организация процессов и программировние в среде Linux
 * Разработал: Белоусов Евгений
 * Группа: 6305
 * Тема: Организация периодических процессов
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

//Количество совершённых итераций порождения дочерних процессов
int iterations;

void timer_handler(int signum);

/**
 * Функция при каждом запуске порождает новый процесс,
 * программа и дочерний процесс заблокированы от завершения при нажатии Ctrl+Z,
 * по завершении дочернего процесса, программа выводит информацию о времени своей работы и работы дочернего процесса
 * 
 * Принимает параметры запуска:
 *  - период запуска (в секундах);
 *  - количество запусков
 */
int main(int argc, char const *argv[])
{
    struct sigaction act;
    struct itimerval timer;
    struct tm *ltime;
    time_t stime;

    //Проверка количества передаваемых параметров в программу
    if (argc != 3)
    {
        fprintf(stderr, "Неверное количество параметров для запуска программы!\n");
        exit(1);
    }

    //Очистка всей структуры нулями во избежание неопределённого поведения
    memset(&act, 0, sizeof(act));
    //Указываем на функцию новой обработки сигнала
    act.sa_handler = timer_handler;
    //Указываем необходимость блокировки сигнала SIGSTP (терминальная остановка, Ctrl+Z)
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTSTP); 
    sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);
    
    //Время до начала запуска интервального таймера
    timer.it_value.tv_sec = 3;
    timer.it_value.tv_usec = 0;
    //Период дальнейшего запуска интервального таймера
    timer.it_interval.tv_sec = atoi(argv[1]);
    timer.it_interval.tv_usec = 0;
    //Установка таймера ITIMER_REAL для подсчёта времени работы
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        fprintf(stderr, "Не удалось установить интервальный таймер!\n");
        return errno;
    }

    //Установка нового обработчика для сигнала SIGVTALRM (сигнал об окончании таймера времени процесса)
    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        fprintf(stderr, "Не удалось установить обработчик сигнала!\n");
        return errno;
    }

    //Цикл ожидания окончания итераций запуска
    for(iterations = 0; iterations < atoi(argv[2]); iterations++)
    {
        pause();
    }
    
    //Завершение работы при достижении заданного количества итераций запуска
    //Получаем текущее системное время
    time(&stime);
    //Преобразовываем в текущее локальное время
    ltime = localtime(&stime);
    printf("Программа полностью завершила работу.\nВремя завершения: %s\n", asctime(ltime));

    return 0;
}

/**
 * Функция выполняет обработку сигнала, порождает поток, который выводит информацию о себе
 * 
 * Принимает: номер сигнала
 */
void timer_handler(int signum)
{
    pid_t pid;
    int status;
    struct tm *ltime;
    time_t stime;

    //Увеличиваем счётчик итераций запуска
    printf("------------------------------\n");
    printf("Получен сигнал SIGVTALRM интервального таймера. Итерация запуска: %d\n", iterations);
    
    //Создаём дочерний процесс
    if ((pid = fork()) == 0)
    {
        //Получаем текущее системное время
        time(&stime);
        //Преобразовываем в текущее локальное время
        ltime = localtime(&stime);
        printf("Дочерний процесс (PID: %d) создан.\nВремя начала: %s", getpid(), asctime(ltime));
        exit(0);
    } else if (pid < 0)
    {
        fprintf(stderr, "Не удалось создать дочерний процесс!\n");
        exit(1);
    }
    //Родитель ожидает окончания дочернего процесса
    waitpid(pid, &status, 0);
    
    //Получаем текущее системное время
    time(&stime);
    //Преобразовываем в текущее локальное время
    ltime = localtime(&stime);
    printf("Программа завершила работу.\nВремя завершения: %s", asctime(ltime));
    printf("------------------------------\n\n");
}