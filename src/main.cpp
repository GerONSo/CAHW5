#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "semaphore.cpp"

std::vector<Semaphore> forkes(5);
std::vector<int> currentPhilosopher(5, -1);
std::mutex global_mutex;
std::chrono::system_clock::time_point start;
std::chrono::system_clock::duration dinnerDuration;

enum PrintType {
    START_THINK, END_THINK, START_EAT, END_EAT
};

enum Input {
    CORRECT, TOO_LONG, INCORRECT
};


std::string getFormattedInt(int number) {
    if (number == -1) {
        return "-1";
    } else {
        return " " + std::to_string(number);
    }
}

void print(PrintType type, int philosopher_id) {
    global_mutex.lock();
    printf("current fork distribution is [%s, %s, %s, %s, %s], cause ",
            getFormattedInt(currentPhilosopher[0]).c_str(),
            getFormattedInt(currentPhilosopher[1]).c_str(),
            getFormattedInt(currentPhilosopher[2]).c_str(),
            getFormattedInt(currentPhilosopher[3]).c_str(),
            getFormattedInt(currentPhilosopher[4]).c_str());
    switch (type) {
        case START_THINK:
            printf("[%d] started thinking", philosopher_id);
            break;
        case END_THINK:
            printf("[%d] ended thinking", philosopher_id);
            break;
        case START_EAT:
            printf("[%d] started eating", philosopher_id);
            break;
        case END_EAT:
            printf("[%d] ended eating", philosopher_id);
            break;
    }
    printf("\n");
    global_mutex.unlock();
}

void updateForks(int philosopher_id, int update_value) {
    global_mutex.lock();
    currentPhilosopher[philosopher_id] = update_value;
    currentPhilosopher[(philosopher_id + 1) % 5] = update_value;
    global_mutex.unlock();
}

void eat(int philosopher_id) {
    using namespace std::literals;
    print(START_THINK, philosopher_id);
    while (std::chrono::system_clock::now() - start < dinnerDuration) {
        bool canTakeLeftFork = forkes[philosopher_id].try_acquire();
        if (canTakeLeftFork) {
            bool canTakeRightFork = forkes[(philosopher_id + 1) % 5].try_acquire();
            if (canTakeRightFork) {
                print(END_THINK, philosopher_id);
                updateForks(philosopher_id, philosopher_id);
                print(START_EAT, philosopher_id);
                std::this_thread::sleep_for(1s);
                print(END_EAT, philosopher_id);
                updateForks(philosopher_id, -1);
                forkes[philosopher_id].release();
                forkes[(philosopher_id + 1) % 5].release();
                print(START_THINK, philosopher_id);
            } else {
                forkes[philosopher_id].release();
            }
        }
    }
    print(END_THINK, philosopher_id);
}

Input isCorrectInput(std::string time) {
    for (auto i : time) {
        if (!std::isdigit(i)) {
            return INCORRECT;
        }
    }
    int input = stoi(time);
    if (input > 60) {
        return TOO_LONG;
    }
    return CORRECT;
}

int main() {
    std::cout << "Welcome to dining evening, philosophers are ready to sit, please choose how much they have to eat and think (in seconds)\n";
    std::string time;
    while (1) {
        std::getline(std::cin, time);
        Input inputResult = isCorrectInput(time);
        if (inputResult == INCORRECT) {
            std::cout << "Seems like you entered not a non-negative number\nTry again:\n";
        } else if (inputResult == TOO_LONG) {
            std::cout << "Seems like you number is too big, philosophers don't have so much time\nTry again:\n";
        } else {
            break;
        }
    }
    dinnerDuration = std::chrono::seconds(stoi(time));
    for (int i = 0; i < 5; ++i) {
        forkes[i].release();        
    }
    start = std::chrono::system_clock::now();
    std::vector<std::thread> philosophers;
    for (int i = 0; i < 5; ++i) {
        std::thread philosopher = std::thread(eat, i);
        philosophers.push_back(std::move(philosopher));
    }
    for (int i = 0; i < philosophers.size(); ++i) {
        philosophers[i].join();
    }
    return 0;
}
