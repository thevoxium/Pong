#include <SDL.h>
#include <SDL_mixer.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

Mix_Chunk *paddleAudio = nullptr;
Mix_Chunk *missedAudio = nullptr;
Mix_Chunk *newBallAudio = nullptr;

bool initializeAudio() {
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    return false;
  }
  newBallAudio = Mix_LoadWAV("audio/newball.wav");
  missedAudio = Mix_LoadWAV("audio/missed.wav");
  paddleAudio = Mix_LoadWAV("audio/audio.wav");
  if (paddleAudio == nullptr) {
    return false;
  }
  return true;
}

void playNewBallAudio() {
  if (newBallAudio != nullptr) {
    Mix_PlayChannel(-1, newBallAudio, 0);
  }
}

void playMissedSound() {
  if (missedAudio != nullptr) {
    Mix_PlayChannel(-1, missedAudio, 0);
  }
}

void playPaddleHitSound() {
  if (paddleAudio != nullptr) {
    Mix_PlayChannel(-1, paddleAudio, 0);
  }
}

void cleanupAudio() {
  if (paddleAudio != nullptr) {
    Mix_FreeChunk(paddleAudio);
  }
  if (missedAudio != nullptr) {
    Mix_FreeChunk(missedAudio);
  }
  if (newBallAudio != nullptr) {
    Mix_FreeChunk(newBallAudio);
  }
  Mix_CloseAudio();
  SDL_Quit();
}

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

struct Obstacle {
  string shape = "____________________";
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
      playPaddleHitSound();
      return true;
    }
  }
  return false;
}

bool checkObstacleCollision(Ball &ball, Obstacle &obstacle) {
  if (ball.x >= obstacle.x && ball.x <= obstacle.x + obstacle.shape.length()) {
    if (ball.y == obstacle.y - 1 && ball.dy > 0) {
      ball.dy = -ball.dy;
      return true;
    }
    if (ball.y == obstacle.y + 1 && ball.dy < 0) {
      ball.dy = -ball.dy;
      return true;
    }
  }

  if (ball.y == obstacle.y) {
    if (ball.x == obstacle.x - 1 && ball.dx > 0) {
      ball.dx = -ball.dx;
      return true;
    }
    if (ball.x == obstacle.x + obstacle.shape.length() && ball.dx < 0) {
      ball.dx = -ball.dx;
      return true;
    }
  }
  return false;
}

void resetBall(Ball &ball, int maxX, int maxY) {
  ball.x = 2 + (rand() % (maxX / 2 - 6));
  ball.y = 2 + (rand() % (maxY / 2 - 6));
  ball.dx = (rand() % 2) * 2 - 1;
  ball.dy = 1;
}

void updateNumberOfBalls(vector<Ball> &balls, int maxY, int maxX) {
  Ball newBall;
  newBall.x = maxX / 2;
  newBall.y = maxY / 2;
  balls.push_back(newBall);
}

void updateObstaclePosition(Obstacle &obs, int maxY, int maxX) {
  obs.x = 2 + (rand() % (maxX / 2 - 6));
  obs.y = 2 + (rand() % (maxX / 2 - 6));
}

void drawPauseScreen(WINDOW *win, int maxY, int maxX) {
  string pauseText = "GAME PAUSED";
  string resumeText = "Press 'p' to resume";
  mvwprintw(win, maxY / 2, (maxX - pauseText.length()) / 2, "%s",
            pauseText.c_str());
  mvwprintw(win, maxY / 2 + 1, (maxX - resumeText.length()) / 2, "%s",
            resumeText.c_str());
}

int main() {
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
    return 1;
  }
  if (!initializeAudio()) {
    std::cout << "Audio initialization failed!";
  }

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

  vector<Ball> balls;
  Ball initialBall;
  initialBall.x = maxX / 2;
  initialBall.y = maxY / 2;
  balls.push_back(initialBall);

  int ballUpdateCounter = 0;
  bool running = true;
  Obstacle obs;
  int obstacleTime = 0;
  bool ballAdded = false;
  int prevScore = score;
  bool isPaused = false;

  while (running) {
    for (int k = 0; k < 15; k++) {
      int ch = getch();
      if (!isPaused) {
        if (ch == KEY_LEFT && paddle.x > 1) {
          paddle.x -= 3;
        } else if (ch == KEY_RIGHT &&
                   paddle.x < maxX - 3 - paddle.pad.length()) {
          paddle.x += 3;
        }
      }

      if (ch == 'q') {
        running = false;
        break;
      } else if (ch == 'p') {
        isPaused = !isPaused;
        if (isPaused) {
          nodelay(stdscr, FALSE);
          werase(win);
          box(win, 0, 0);
          drawPauseScreen(win, maxY - 2, maxX - 2);
          wrefresh(win);
        } else {
          nodelay(stdscr, TRUE);
        }
      }
      if (!isPaused) {
        usleep(500);
      }
    }

    if (!running)
      break;
    if (isPaused)
      continue;

    if (ballUpdateCounter >= 5) {
      for (Ball &ball : balls) {
        if (ball.y >= paddle.y) {
          resetBall(ball, maxX - 2, maxY - 2);
          playMissedSound();
          score--;
        } else {
          if (checkCollision(ball, paddle)) {
            score++;
            ball.dy = -ball.dy;
          }
          checkObstacleCollision(ball, obs);
          updateBallposition(ball, maxY - 2, maxX - 2);
        }
      }
      ballUpdateCounter = 0;
    }

    ballUpdateCounter++;
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, paddle.y, paddle.x, "%s", paddle.pad.c_str());

    for (const Ball &ball : balls) {
      mvwprintw(win, ball.y, ball.x, "%s", ball.shape.c_str());
    }

    if (obstacleTime % 1000 == 0) {
      updateObstaclePosition(obs, maxY, maxX);
    }

    mvwprintw(win, obs.y, obs.x, "%s", obs.shape.c_str());
    mvwprintw(win, 2, 2, "Score: %d", score);
    wrefresh(win);
    obstacleTime++;

    if (score - prevScore >= 10 && score > 0 && balls.size() < 3) {
      updateNumberOfBalls(balls, maxY, maxX);
      playNewBallAudio();
      prevScore = score;
    }
  }
  cleanupAudio();
  endwin();
  return 0;
}
