#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

struct Ball {
  string shape = "O";
  int x;
  int y;
  int dx = 1;
  int dy = 1;
};

struct Paddle {
  string pad = "||||||||||||||||||||";
  int x;
  int y;
};

void updateBallposition(Ball &ball, int maxY, int maxX) {
  if (ball.y <= 1)
    ball.dy = -ball.dy;
  if (ball.x >= maxX - 3 || ball.x <= 1)
    ball.dx = -ball.dx;

  ball.y += ball.dy;
  ball.x += ball.dx;
}

bool checkCollision(Ball &ball, Paddle &paddle) {
  if (ball.y == paddle.y - 1) {
    if (ball.x >= paddle.x && ball.x <= paddle.x + paddle.pad.length()) {
      ball.y = paddle.y - 2;
      return true;
    }
  }
  return false;
}

void resetBall(Ball &ball, int maxX, int maxY) {
  ball.x = 2 + (rand() % (maxX - 4 - 2 + 1));
  ball.y = 2 + (rand() % (maxY - 4 - 2 + 1));
  ball.dx = 1;
  ball.dy = 1;
}

int main() {
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);

  int score = 0;

  int maxY, maxX;
  getmaxyx(stdscr, maxY, maxX);
  WINDOW *win = newwin(maxY - 2, maxX - 2, 1, 1);
  box(win, 0, 0);
  refresh();
  wrefresh(win);

  Paddle paddle;
  paddle.x = maxX / 2 - 10;
  paddle.y = maxY - 4;

  Ball ball;
  ball.x = maxX / 2;
  ball.y = maxY / 2;

  int ballUpdateCounter = 0;
  bool running = true;

  while (running) {
    for (int k = 0; k < 10; k++) {
      int ch = getch();
      if (ch == KEY_LEFT && paddle.x > 1) {
        paddle.x -= 3;
      } else if (ch == KEY_RIGHT && paddle.x < maxX - 3 - paddle.pad.length()) {
        paddle.x += 3;
      } else if (ch == 'q') {
        running = false;
        break;
      }
      usleep(1000);
    }

    if (ballUpdateCounter >= 2) {
      if (ball.y >= paddle.y) {
        resetBall(ball, maxX, maxY);
        score--;
      } else {
        if (checkCollision(ball, paddle)) {
          score++;
          ball.dy = -ball.dy;
        }
        updateBallposition(ball, maxY, maxX);
      }
      ballUpdateCounter = 0;
    }

    if (!running)
      break;

    ballUpdateCounter++;
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, paddle.y, paddle.x, "%s", paddle.pad.c_str());
    mvwprintw(win, ball.y, ball.x, "%s", ball.shape.c_str());
    mvwprintw(win, 2, 2, "Score %d", score);
    wrefresh(win);
  }

  endwin();
  return 0;
}
